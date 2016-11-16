/*************************************************************************
    > File Name: ProjectBuffer.cpp
    > Author: zhengshurui
    > Mail:  zhengshurui@thinkit.cn
    > Created Time: Mon 12 Sep 2016 12:59:18 AM PDT
 ************************************************************************/

#include <pthread.h>
#include <sys/time.h>

#include <cassert>
#include <vector>
#include <map>
#include <set>
#include<iostream>
#include <sstream>
#include <algorithm>
#include <iterator>
using namespace std;

#include "utilites.h"
#include "ProjectBuffer.h"
#include "log4z.h"
extern zsummer::log4z::ILog4zManager *g_Log4zManager;


#define PCM_PERSEC_SMPS 8000
#define PCM_PERSEC_LEN 16000
namespace zen4audio{

unsigned BLOCKSIZE= 60 * 16000;
vector<DataUnit*> g_vecFreeBlocks; 
static set<DataUnit*> g_setUsedBlocks;
static unsigned g_uBlocksMax = 600;
static unsigned g_uBlocksMin = 300;
static LockHelper g_BlocksManaLocker;
DataUnit* myAlloc(){
    DataUnit* ret = NULL;
    
    AutoLock lock(g_BlocksManaLocker);
    if(g_setUsedBlocks.size() < g_uBlocksMax){
        if(g_vecFreeBlocks.empty()){
            ret = new DataUnit[BLOCKSIZE];
            assert(ret != NULL);
        }
        else{
            ret = g_vecFreeBlocks.back();
            g_vecFreeBlocks.pop_back();
        }
        g_setUsedBlocks.insert(ret);
    }

    return ret;
}

void myFree(DataUnit* data){
    if(data == NULL) return;
    AutoLock lock(g_BlocksManaLocker);
    if(g_setUsedBlocks.size() > g_uBlocksMin){
        delete [] data;
    }
    else{
        if(g_vecFreeBlocks.size() < g_uBlocksMin){
            g_vecFreeBlocks.push_back(data);
        }
        else{
            delete [] data;
        }
    }
    if(g_setUsedBlocks.erase(data) != 1){
        //data allocated somewhere other than alloc above.   
        assert(false);
    }
}

static LockHelper g_DataBlockLocker;//for use in multi-threaded environment.
void DataBlock::setData(const DataBlock& other)
{
    AutoLock myLock(g_DataBlockLocker);
    m_buf = other.m_buf;
    m_len = other.m_len;
    m_cnt = other.m_cnt;
    if(m_cnt != NULL){
        (*m_cnt)++;
    }
}
void DataBlock::relse()
{
    AutoLock myLock(g_DataBlockLocker);
    if(m_buf != NULL){
        (*m_cnt) --;
        if(*m_cnt == 0){
            myFree(static_cast<DataUnit*>(m_buf));
            free(m_cnt);
        }
    }
}

static LoggerId g_BufferLogger = LOG4Z_MAIN_LOGGER_ID;
ProjectBuffer::BufferConfig ProjectBuffer::bufferConfig;
unsigned ProjectBuffer::ceilUnitIdx;
unsigned ProjectBuffer::ceilOffset;

BufferConfig::BufferConfig():
    ProjectBuffer::BufferConfig(), m_uBlockSize(BLOCKSIZE), m_uBlocksMin(g_uBlocksMin), m_uBlocksMax(g_uBlocksMax)
{ }
bool ProjectBuffer::initGlobal(ProjectBuffer::BufferConfig param)
{
    bufferConfig = param;
    ceilUnitIdx = param.waitLength / BLOCKSIZE;
    ceilOffset = param.waitLength - ceilUnitIdx * BLOCKSIZE;
    if(ceilOffset == 0 && ceilUnitIdx != 0){
        ceilOffset = BLOCKSIZE;
        ceilUnitIdx --;
    }
    ceilUnitIdx += 1;//the first element of arrUnits is just placeholder.
    return true;
}

void ProjectBuffer::setPid(unsigned long pid, time_t curTime)
{
    AutoLock myLock(m_BufferLock);
    if(curTime == 0) curTime = time(NULL);
    this->ID = pid;
    this->bFull = false;
    this->bAlloc = true;
    DataBlock ele;
    ele.m_len = BLOCKSIZE;
    arrUnits.push_back(ele);
    arrArrivalRecords.push_back(ArrivalRecord(curTime, 0, 0));
}

/**
 * return array of Objects which share the same point with DataBlock.
 */
void ProjectBuffer::getData(vector<DataBlock>& vec)
{
    AutoLock lock(m_BufferLock);
    vector<DataBlock>::iterator it = arrUnits.begin();
    vec.insert(vec.end(), ++it, arrUnits.end());
    return;
}

ostream& operator<<(ostream& str, ProjectBuffer::ArrivalRecord rec)
{
    str<<"(" << rec.seconds<< ", " << rec.unitIdx<< ", "<< rec.offset<<")";
    return str;
}
void ProjectBuffer::relse(){
    ostringstream oss;
    std::copy(arrArrivalRecords.begin(), arrArrivalRecords.end(), std::ostream_iterator<ProjectBuffer::ArrivalRecord>(oss, "; "));
    LOGFMT_DEBUG(g_BufferLogger, "PID=%lu release projectBuffer, data arrivals: %s.", this->ID, oss.str().c_str());
}

/**
 * 返回成功接收的长度.
 *
 */
unsigned ProjectBuffer::recvData(char *data, unsigned len, time_t curTime)
{
    AutoLock lock(m_BufferLock);
    unsigned ret = 0;
    if(curTime == 0) curTime = time(NULL);
    unsigned curSize = arrUnits.back().m_len;
    unsigned size1 = len > BLOCKSIZE - curSize ? BLOCKSIZE - curSize : len;
    unsigned size2 = len - size1;
    memcpy(arrUnits.back().m_buf + curSize, data, size1);
    ret += size1;
    arrUnits.back().m_len += size1;
    if(size2 > 0){
        DataBlock ele;
        if(ele.initData()){
            arrUnits.push_back(ele);
            memcpy(arrUnits.back().m_buf, data + size1, size2);
            ret += size2;
            arrUnits.back().m_len += size2;
        }
        else{
            LOGFMT_INFO(g_BufferLogger, "PID=%lu failed to store data, curBlockCount=%u, for no DataBlock available.", this->ID, (arrUnits.size() -1));
        }
    }
    arrArrivalRecords.push_back(ArrivalRecord(curTime, arrUnits.size()-1, arrUnits.back().m_len));
    if(ret > 0){
        if(arrUnits.size() - 1 > ceilUnitIdx || (arrUnits.size() -1 == ceilUnitIdx && arrUnits.back().m_len >= ceilOffset)){
            LOGFMT_DEBUG(g_BufferLogger, "PID=%lu the state turns to FULL, as the accumulated data is enough.", this->ID);
            turnFull();
        }
    }

    return ret;
}

void ProjectBuffer::finishRecv()
{
    AutoLock lock(m_BufferLock);
    turnFull();
}

bool ProjectBuffer::isFull(time_t curTime)
{
    AutoLock lock(m_BufferLock);
    if(bFull){
        return true;
    }
    if(!bAlloc) return false;
    bool ret = false;
    if(curTime == 0) curTime = time(NULL);
    ArrivalRecord &edRec = arrArrivalRecords.back();
    ArrivalRecord &stRec = arrArrivalRecords.front();
    unsigned long diffTime = curTime > edRec.seconds ? curTime - edRec.seconds : 0;
    unsigned long totalTime = curTime > stRec.seconds ? curTime - stRec.seconds : 0;
    if(diffTime > bufferConfig.waitSecondsStep){
        turnFull();
        LOGFMT_DEBUG(g_BufferLogger, "PID=%lu the state turns to FULL, as no more data arrives.", this->ID);
        ret = true;
    }
    else if(totalTime > bufferConfig.waitSeconds){
        turnFull();
        LOGFMT_DEBUG(g_BufferLogger, "PID=%lu the state turns to FULL, as the overall time of buffering reachs its maximum.", this->ID);
        ret = true;
    }

    return ret;
}

//////////////////////////////////////////////////////////
//
static std::map<unsigned long, ProjectBuffer*> g_mapProjs;
static std::set<unsigned long> g_setOtherIDs;
static std::set<unsigned long> g_setFullIDs;
static LockHelper g_ProjsLocker;
static pthread_cond_t g_ProjsFullCond;
static pthread_cond_t g_ProjsEmptyCond;

static void makeTimespec(struct timespec *tsp)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    tsp->tv_sec = now.tv_sec;
    tsp->tv_nsec = now.tv_usec * 1000;
}

bool init_bufferglobal(BufferConfig buffConfig)
{
    AutoLock myLock(g_ProjsLocker);
    ProjectBuffer::initGlobal(static_cast<ProjectBuffer::BufferConfig>(buffConfig));
    BLOCKSIZE=buffConfig.m_uBlockSize;
    g_uBlocksMax = buffConfig.m_uBlocksMax;
    g_uBlocksMin = buffConfig.m_uBlocksMin;
	g_BufferLogger = g_Log4zManager->createLogger("globalBuffer");
    LOGFMT_INFO(g_BufferLogger, "init_bufferglobal finished.");
    LOGFMT_INFO(g_BufferLogger, "blocksMaximum=%u.", g_uBlocksMax);
    LOGFMT_INFO(g_BufferLogger, "blocksMiniMum=%u.", g_uBlocksMin);
    LOGFMT_INFO(g_BufferLogger, "blockSize=%u.", BLOCKSIZE);
    LOGFMT_INFO(g_BufferLogger, "waitSecondsStep=%u.", ProjectBuffer::bufferConfig.waitSecondsStep);
    LOGFMT_INFO(g_BufferLogger, "waitSeconds=%u.", ProjectBuffer::bufferConfig.waitSeconds);
    LOGFMT_INFO(g_BufferLogger, "waitLength=%u.", ProjectBuffer::bufferConfig.waitLength);

    return true;
}

void rlse_bufferglobal()
{
    AutoLock myLock(g_ProjsLocker);
    assert(g_mapProjs.size() == g_setOtherIDs.size() + g_setFullIDs.size());

    g_mapProjs.clear();
    g_setFullIDs.clear();
    LOGFMT_DEBUG(g_BufferLogger, "rlse_bufferglobal finished.");
}

void notifyProjFinish(unsigned long pid)
{
    g_ProjsLocker.lock();
    if(g_mapProjs.find(pid) != g_mapProjs.end()){
        ProjectBuffer *tmp = g_mapProjs[pid];
        tmp->finishRecv();
        g_setOtherIDs.erase(pid);
        g_setFullIDs.insert(pid);
    }
    assert(g_mapProjs.size() == g_setOtherIDs.size() + g_setFullIDs.size());
    g_ProjsLocker.unLock();
    pthread_cond_broadcast(&g_ProjsFullCond);
    LOGFMT_DEBUG(g_BufferLogger, "rlse_bufferglobal finished.");
}

/**
 *
 * return 1 if success, otherwise 0.
 */
int recvProjSegment(unsigned long pid, char* data, unsigned len, bool iswait)
{
    //LOGFMT_TRACE(g_BufferLogger, "PID=%llu receive new data, size=%u.", pid, len);
    g_ProjsLocker.lock();
    if(g_mapProjs.find(pid) == g_mapProjs.end()){
        g_mapProjs[pid] = new ProjectBuffer(pid);
        g_setOtherIDs.insert(pid);
        LOGFMT_DEBUG(g_BufferLogger, "PID=%lu arrives.", pid);
    }
    assert(g_mapProjs.size() == g_setOtherIDs.size() + g_setFullIDs.size());
    int ret = 1;
    unsigned wlen = g_mapProjs[pid]->recvData(data, len);
    if(iswait){
        while(wlen < len){
            struct timespec tsp;
            makeTimespec(&tsp);
            tsp.tv_sec += 5;
            pthread_cond_timedwait(&g_ProjsEmptyCond, &g_ProjsLocker._crit, &tsp);//can't only rely notification from another thread to get condition.
            if(g_mapProjs.find(pid) == g_mapProjs.end()){
                ret = 0;
                break;
            }
            wlen += g_mapProjs[pid]->recvData(data + wlen, len - wlen);
                
        }
    }
    bool bNotify = false;
    if(ret != 0 && wlen > 0){
        if(g_mapProjs[pid]->getFull()){
            bNotify = true;
        }
    }
    g_ProjsLocker.unLock();
    if(bNotify) pthread_cond_broadcast(&g_ProjsFullCond);
    if(wlen != len){
        ret = 0;
    }
    return ret;
}

static void transferIDs(time_t curTime)
{
    std::set<unsigned long>::iterator it = g_setOtherIDs.begin();
    while(it != g_setOtherIDs.end()){
        std::set<unsigned long>::iterator chit = it ++;
        unsigned long curID = *chit;
        if(g_mapProjs[curID]->isFull(curTime)){
            g_setOtherIDs.erase(chit);
            g_setFullIDs.insert(curID);
        }
    }
}

/**
 * 当存在多个project时，保证多次调用会返回所有的project.
 *
 */
ProjectBuffer* obtainFullBufferTimeout(unsigned secs)
{
    //can't only rely notification from another thread to get condition.
    AutoLock mylock(g_ProjsLocker);
    time_t curTime = time(NULL);
    time_t endTime = curTime + secs;
    transferIDs(curTime);
    const int stepsecs = 5;
    while(g_setFullIDs.size() == 0){
        if(curTime >= endTime) break;
        struct timespec tsp;
        makeTimespec(&tsp);
        tsp.tv_sec += stepsecs;
        pthread_cond_timedwait(&g_ProjsFullCond, &g_ProjsLocker._crit, &tsp);
        curTime = time(NULL);
        transferIDs(curTime);
    }

    if(g_setFullIDs.size() > 0){
        static unsigned long lastReturnPid;
        set<unsigned long>::iterator it = g_setFullIDs.begin();
        while(it != g_setFullIDs.end()){
            if(*it > lastReturnPid) break;
            it++;
        }
        if(it == g_setFullIDs.end()) it = g_setFullIDs.begin();
        lastReturnPid = *it;
        return g_mapProjs[lastReturnPid];
    }
    else{
        return NULL;
    }
}

void returnFullBuffer(ProjectBuffer* obtained)
{
    AutoLock mylock(g_ProjsLocker);
    assert(g_mapProjs.find(obtained->ID) != g_mapProjs.end());
    assert(g_mapProjs[obtained->ID] == obtained);
    g_mapProjs.erase(obtained->ID);
    g_setFullIDs.erase(obtained->ID);
    delete obtained;
}

bool isAllFinished()
{
    AutoLock mylock(g_ProjsLocker);
    return g_mapProjs.empty();
}

}

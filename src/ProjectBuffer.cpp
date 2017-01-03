/*************************************************************************
    > File Name: ProjectBuffer.cpp
    > Author: zhengshurui
    > Mail:  zhengshurui@thinkit.cn
    > Created Time: Mon 12 Sep 2016 12:59:18 AM PDT
 ************************************************************************/
#include "ProjectBuffer.h"

#include <pthread.h>
#include <sys/time.h>

#include <cassert>
#include <vector>
#include <map>
#include <set>
#include <list>
#include<iostream>
#include <sstream>
#include <algorithm>
#include <iterator>

#include "utilites.h"
#include "log4z.h"
extern zsummer::log4z::ILog4zManager *g_Log4zManager;

using namespace std;

#define PCM_PERSEC_SMPS 8000
#define PCM_PERSEC_LEN 16000
namespace zen4audio{

static unsigned BLOCKSIZE= 60 * 16000;
static unsigned g_uBlocksMin = 300; //reserved for potential use.
static unsigned g_uBlocksMax = 600;
static list<DataBlock> g_liFreeBlocks;
static set<DataBlock> g_liUsedBlocks;
static LockHelper g_BlockManaLocker;
static inline DataBlock BlockMana_alloc()
{
    AutoLock lock(g_BlockManaLocker);
    list<DataBlock>::iterator it = g_liFreeBlocks.begin();
    for(; it != g_liFreeBlocks.end(); it++){
        if(it->getPeerNum() == 1) break;
    }
    if(it != g_liFreeBlocks.end()){
        pair<set<DataBlock>::iterator, bool> reti = g_liUsedBlocks.insert(*it);
        assert(reti.second);
        g_liFreeBlocks.erase(it);
        assert((*reti.first).offset == 0 && (*reti.first).len == 0);
        return *reti.first;
    }
    else{
        size_t allcnt = g_liFreeBlocks.size() + g_liUsedBlocks.size();
        if(allcnt > g_uBlocksMax) return DataBlock();
        else {
            pair<set<DataBlock>::iterator, bool> reti = g_liUsedBlocks.insert(DataBlock(BLOCKSIZE));
            assert(reti.second);
            assert((*reti.first).offset == 0 && (*reti.first).len == 0);
            return *reti.first;
        }
    }
}

static inline void BlockMana_relse(const DataBlock& blk)
{
    AutoLock lock(g_BlockManaLocker);
    assert(blk.offset + blk.len <= blk.getCap());
    assert(g_liUsedBlocks.find(blk) != g_liUsedBlocks.end());
    if(g_liFreeBlocks.size() + g_liUsedBlocks.size() <= g_uBlocksMin){
        g_liFreeBlocks.push_back(blk);
        g_liFreeBlocks.back().len = 0;
        g_liFreeBlocks.back().offset = 0;
        g_liUsedBlocks.erase(blk);
    }
    else{
        g_liUsedBlocks.erase(blk);
    }
}

/*
{
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
                delete m_cnt;
            }
        }
    }
}*/
struct timeval ZERO_TIMEVAL;
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
    this->bBampHit = false;
    this->bampFp = NULL;
    this->funcSaveData = NULL;
    this->bRelsed = false;
    //this->uBampEnd = 0;
    this->bampEndIdx = 0;
    this->bampEndOffset = 960000;
    this->mainRegStTime.tv_sec = 0;
    this->mainRegStTime.tv_usec = 0;
    this->mainRegEdTime.tv_sec = 0;
    this->mainRegEdTime.tv_usec = 0;
    DataBlock ele;
    ele.len = BLOCKSIZE;
    arrUnits.push_back(ele);
    prjTime = curTime;
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

unsigned ProjectBuffer::getDataLength()
{
    AutoLock lock(m_BufferLock);
    vector<DataBlock>::iterator it = arrUnits.begin();
    unsigned ret = 0;
    while(++it != arrUnits.end()){
        ret += it->len;
    }
    return 0;
}

void ProjectBuffer::getDataSegment(unsigned idx, unsigned offset, unsigned endIdx, unsigned endOffset, vector<DataBlock>& data)
{
    while(idx < endIdx){
        assert(offset <= arrUnits[idx].len);
        unsigned len = arrUnits[idx].len - offset;
        if(len > 0){
             data.push_back(arrUnits[idx]);   
            data.back().offset = offset;
            data.back().len = len;
        }
        idx ++;
        offset = arrUnits[idx].offset;
    }
    assert(offset <= endOffset);
    if(offset < endOffset){
        unsigned len = endOffset- offset;
        data.push_back(arrUnits[idx]);
        data.back().offset = offset;
        data.back().len = len;
    }
}
void ProjectBuffer::getUnBampData(unsigned &preidx, unsigned &prest, unsigned &endidx, unsigned &endst, std::vector<DataBlock>& data, bool& bPreHit)
{
    AutoLock lock(m_BufferLock);
    data.clear();
    preidx = this->bampEndIdx;
    prest = this->bampEndOffset;
    endidx = arrUnits.size() - 1;
    endst = arrUnits[endidx].len;
    assert(endidx >= preidx);
    unsigned idx = preidx;
    unsigned st = prest;
    getDataSegment(idx, st, endidx, endst, data);
    bPreHit = this->bBampHit;
}

//NOTE: called in context being locked.
/*
bool ProjectBuffer::saveBampProject(unsigned idx, unsigned offset)
{
}
*/

ostream& operator<<(ostream& str, ProjectBuffer::ArrivalRecord rec)
{
    str<<"(" << rec.seconds<< ", " << rec.unitIdx<< ", "<< rec.offset<<")";
    return str;
}

bool ProjectBuffer::relse(){
    AutoLock lock(m_BufferLock);
    if(mainRegEdTime.tv_sec == 0){
        return false;
    }
    if(bRelsed) return true;
    bRelsed = true;
    if(this->bBampHit && bampFp){
        vector<DataBlock> remData;
        unsigned idx = arrUnits.size() - 1;
        getDataSegment(this->bampEndIdx, this->bampEndOffset, idx, arrUnits[idx].offset + arrUnits[idx].len, remData);
        this->funcSaveData(bampFp, remData);
        fclose(bampFp);
    }
    unsigned tolLen = 0;
    for(size_t idx=1; idx < arrUnits.size(); idx++){
        tolLen += arrUnits[idx].len;
        BlockMana_relse(arrUnits[idx]);
    }
    LOGFMT_INFO(g_BufferLogger, "PID=%lu SIZE=%lu release ProjectBuffer.", this->ID, tolLen);
    LOGFMT_DEBUG(g_BufferLogger, "PID=%lu MainReg start %ld %ld; end: %ld %ld.", mainRegStTime.tv_sec, mainRegEdTime.tv_usec, mainRegEdTime.tv_sec, mainRegEdTime.tv_usec);

    return true;
}

/**
 * 返回成功接收的长度.
 *
 */
unsigned ProjectBuffer::recvData(char *data, unsigned dataLen, time_t curTime)
{
    AutoLock lock(m_BufferLock);
    unsigned ret = 0;
    if(curTime == 0) curTime = time(NULL);
    unsigned curSize = arrUnits.back().len;
    unsigned size1 = dataLen > BLOCKSIZE - curSize ? BLOCKSIZE - curSize : dataLen;
    unsigned size2 = dataLen - size1;
    memcpy(arrUnits.back().getPtr() + curSize, data, size1);
    ret += size1;
    arrUnits.back().len += size1;
    if(size2 > 0){
        DataBlock ele = BlockMana_alloc();
        if(ele.getPtr()){
            arrUnits.push_back(ele);
            memcpy(arrUnits.back().getPtr(), data + size1, size2);
            ret += size2;
            arrUnits.back().len += size2;
        }
        else{
            ret -= size1;
            arrUnits.back().len -= size1;
            LOGFMT_WARN(g_BufferLogger, "PID=%lu failed to store data, curBlockCount=%u, for no DataBlock available.", this->ID, (arrUnits.size() -1));
        }
    }
    lastRecord.seconds = curTime;
    lastRecord.unitIdx = arrUnits.size() - 1;
    lastRecord.offset = arrUnits.back().len;
    if(!bFull && ret > 0){
        if(arrUnits.size() - 1 > ceilUnitIdx || (arrUnits.size() -1 == ceilUnitIdx && arrUnits.back().len >= ceilOffset)){
            LOGFMT_DEBUG(g_BufferLogger, "PID=%lu the state turns to FULL, as the accumulated data reaches to maximum.", this->ID);
            turnFull();
        }
    }

    return ret;
}

void ProjectBuffer::finishRecv()
{
    AutoLock lock(m_BufferLock);
    LOGFMT_DEBUG(g_BufferLogger, "PID=%lu the state turns to FULL, as being notified.", this->ID);
    if(! bFull) turnFull();
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
    ArrivalRecord &edRec = lastRecord;
    //ArrivalRecord &stRec = lastRecord;
    unsigned long diffTime = curTime > edRec.seconds ? curTime - edRec.seconds : 0;
    unsigned long totalTime = curTime > prjTime ? curTime - prjTime : 0;
    if(diffTime > bufferConfig.waitSecondsStep){
        turnFull();
        LOGFMT_DEBUG(g_BufferLogger, "PID=%lu the state turns to FULL, diffTime: %lu.", this->ID, diffTime);
        ret = true;
    }
    else if(totalTime > bufferConfig.waitSeconds){
        turnFull();
        LOGFMT_DEBUG(g_BufferLogger, "PID=%lu the state turns to FULL, totalTime: %lu.", this->ID, totalTime);
        ret = true;
    }

    return ret;
}

//////////////////////////////////////////////////////////
//
struct ProjectCheckbook{
    ProjectCheckbook():
        prj(NULL), cnt(0)
    {}
    ProjectBuffer *prj;
    int cnt;
};
static std::map<unsigned long, ProjectCheckbook> g_mapProjs;
static std::set<unsigned long> g_setPrevFullIDs;
static std::list<unsigned long> g_liFullIDs;
static std::set<unsigned long> g_setPostFullIDs;
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
    assert(g_mapProjs.size() == g_setPrevFullIDs.size() + g_liFullIDs.size() + g_setPostFullIDs.size());

    g_mapProjs.clear();
    g_liFullIDs.clear();
    g_setPostFullIDs.clear();
    LOGFMT_DEBUG(g_BufferLogger, "rlse_bufferglobal finished.");
}

void notifyProjFinish(unsigned long pid)
{
    g_ProjsLocker.lock();
    if(g_mapProjs.find(pid) != g_mapProjs.end()){
        ProjectBuffer *tmp = g_mapProjs[pid].prj;
        tmp->finishRecv();
        if(g_setPrevFullIDs.find(pid) != g_setPrevFullIDs.end()){
            g_setPrevFullIDs.erase(pid);
            g_liFullIDs.push_back(pid);

        }
    }
    assert(g_mapProjs.size() == g_setPrevFullIDs.size() + g_liFullIDs.size() + g_setPostFullIDs.size());
    g_ProjsLocker.unLock();
    pthread_cond_broadcast(&g_ProjsFullCond);
    LOGFMT_DEBUG(g_BufferLogger, "rlse_bufferglobal finished.");
}

/**
 *
 * return 1 if success, otherwise 0.
 */
int recvProjSegment(ProjectSegment param, bool iswait)
{
    //LOGFMT_TRACE(g_BufferLogger, "PID=%llu receive new data, size=%u.", pid, len);
    unsigned long &pid = param.pid;
    char *&data = param.data;
    unsigned dataLen = param.len;
    
    ProjectBuffer *tmpPrj = NULL;
    g_ProjsLocker.lock();
    if(g_mapProjs.find(pid) == g_mapProjs.end()){
        g_mapProjs[pid].prj = new ProjectBuffer(pid);
        g_setPrevFullIDs.insert(pid);
        LOGFMT_DEBUG(g_BufferLogger, "PID=%lu arrives, %u projects in buffer.", pid, g_mapProjs.size());
    }
    tmpPrj = g_mapProjs[pid].prj;
    g_mapProjs[pid].cnt ++;
    g_ProjsLocker.unLock();

    int ret = 0;   
    unsigned rlen = tmpPrj->recvData(data, dataLen);
    if(rlen == dataLen){
        ret = 1;
    } 
    else{
        LOGFMT_ERROR(g_BufferLogger, "PID=%lu GlobalBuffer receive data failed. Total: %u; Received: %u.", pid, dataLen, rlen);
    }
    returnBuffer(tmpPrj);
    return ret;
}

static void transferIDs(time_t curTime)
{
    std::set<unsigned long>::iterator it = g_setPrevFullIDs.begin();
    while(it != g_setPrevFullIDs.end()){
        std::set<unsigned long>::iterator chit = it ++;
        unsigned long curID = *chit;
        if(g_mapProjs[curID].prj->isFull(curTime)){
            g_setPrevFullIDs.erase(chit);
            g_liFullIDs.push_back(curID);
        }
    }
}

void obtainAllBuffers(map<unsigned long, ProjectBuffer*>& allBufs)
{
    g_ProjsLocker.lock();
    for(map<unsigned long, ProjectCheckbook>::iterator it = g_mapProjs.begin(); it != g_mapProjs.end(); it++){
        if(allBufs.find(it->first) == allBufs.end()){
            allBufs[it->first] = it->second.prj;
            it->second.cnt++;
        }
    }
    g_ProjsLocker.unLock();

}
ProjectBuffer* obtainBuffer(unsigned long pid)
{
    g_ProjsLocker.lock();
    if(g_mapProjs.find(pid) == g_mapProjs.end()){
        g_ProjsLocker.unLock();
        return NULL;
    }
    ProjectBuffer* ret = g_mapProjs[pid].prj;
    g_mapProjs[pid].cnt ++;
    g_ProjsLocker.unLock();
    return ret;
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
    const int stepsecs = 3;
    while(g_liFullIDs.size() == 0){
        if(curTime >= endTime) break;
        struct timespec tsp;
        makeTimespec(&tsp);
        tsp.tv_sec += stepsecs;
        pthread_cond_timedwait(&g_ProjsFullCond, &g_ProjsLocker._crit, &tsp);
        curTime = time(NULL);
        transferIDs(curTime);
    }

    if(g_liFullIDs.size() > 0){
        unsigned long retpid = g_liFullIDs.front();
        ProjectBuffer* prj = g_mapProjs[retpid].prj;
        g_mapProjs[retpid].cnt ++;
        g_setPostFullIDs.insert(retpid);
        g_liFullIDs.pop_front();
        return prj;
    }
    else{
        return NULL;
    }
}

void returnBuffer(ProjectBuffer* obtained)
{
    AutoLock mylock(g_ProjsLocker);
    unsigned long pid = obtained->ID;
    assert(g_mapProjs.find(pid) != g_mapProjs.end());
    assert(g_mapProjs[pid].prj == obtained);
    assert(g_mapProjs[pid].cnt > 0);
    g_mapProjs[pid].cnt --;
    if(g_mapProjs[pid].cnt != 0) return; 
    if(obtained->relse()){
        assert(g_setPostFullIDs.find(obtained->ID) != g_setPostFullIDs.end());
        g_setPostFullIDs.erase(obtained->ID);
        assert(g_setPrevFullIDs.find(obtained->ID) == g_setPrevFullIDs.end());
        g_mapProjs.erase(obtained->ID);
        delete obtained;
    }
}

bool isAllFinished()
{
    AutoLock mylock(g_ProjsLocker);
    return g_mapProjs.empty();
}

}

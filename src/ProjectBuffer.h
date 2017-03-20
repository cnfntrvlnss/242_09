/*************************************************************************
    > File Name: ProjectBuffer.h
    > Author: zhengshurui
    > Mail:  zhengshurui@thinkit.cn
    > Created Time: Mon 12 Sep 2016 02:22:16 AM PDT
 ************************************************************************/

#ifndef PROJECTBUFFER__H
#define PROJECTBUFFER__H

#include "utilites.h"
#include <sys/time.h>
#include <pthread.h>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <set>
#include <ostream>

typedef bool(*FuncSaveData)(FILE*, std::vector<DataBlock>&);
namespace zen4audio{

extern struct timeval ZERO_TIMEVAL;
/**
 *  用于节目缓存的类, 比之前的实现相比，最小化了功能；用到了唯一的内存管理代码，用于缓存空间的计数与控制.
 *
 *
 */
class ProjectBuffer{
public:
    ProjectBuffer(){
        init();
    }
    explicit ProjectBuffer(unsigned long pid, time_t curTime = 0, bool bbamp = false){
        init();
        setPid(pid, curTime, bbamp);
    }
    ~ProjectBuffer(){
        std::vector<DataBlock> retArr;
        relse(retArr);
        assert(retArr.size() == 0);
    }
    void setPid(unsigned long, time_t curTime =0, bool bBamp = false);
    struct timeval getPrjTime(){
        AutoLock lock(m_BufferLock);
        struct timeval ret;
        ret.tv_sec = fullRecord.seconds;
        ret.tv_usec = 0;
        return ret;
    }

    void setBampEndPos(unsigned preIdx, unsigned preSt, unsigned idx, unsigned st, bool bhit){
        AutoLock lock(m_BufferLock);
        assert(this->bampEndIdx == preIdx);
        assert(this->bampEndOffset == preSt);
        this->bampEndIdx = idx;
        this->bampEndOffset = st;
        if(bhit) this->bBampHit = true;
    }
    
    void getUnBampData(unsigned &idx, unsigned &st, unsigned &endidx, unsigned &endst, std::vector<DataBlock>& data, bool& bPreHit, bool& bfull, struct timeval & prjtime);

    void startMainReg(struct timeval curtime = ZERO_TIMEVAL){
        //if(memcmp(&curtime, &ZERO_TIMEVAL, sizeof(struct timeval) == 0)){
        if(curtime.tv_sec == ZERO_TIMEVAL.tv_sec && curtime.tv_usec == ZERO_TIMEVAL.tv_usec){
            gettimeofday(&curtime, NULL);
        }
        AutoLock lock(m_BufferLock);
        mainRegStTime = curtime;
    }
    void finishMainReg(struct timeval curtime = ZERO_TIMEVAL){
        //if(memcmp(&curtime, &ZERO_TIMEVAL, sizeof(struct timeval) == 0)){
        if(curtime.tv_sec == ZERO_TIMEVAL.tv_sec && curtime.tv_usec == ZERO_TIMEVAL.tv_usec){
            gettimeofday(&curtime, NULL);
        }
        AutoLock lock(m_BufferLock);
        mainRegEdTime = curtime;
    }
    
    void relse(std::vector<DataBlock>& retArr);
    void getData(std::vector<DataBlock>& vec);
    unsigned getDataLength();
    bool isFull(time_t curTime, time_t& nextTime);
    bool getFull(){
        AutoLock lock(m_BufferLock);
        return bFull;
    }

    unsigned recvData(char *data, unsigned len, DataBlock* blk, time_t curTime = 0);
    void finishRecv();
    void init(){
        bAlloc = false;
    }
    void getDataSegment(unsigned idx, unsigned offset, unsigned endIdx, unsigned endOffset, std::vector<DataBlock>& data)
{
    AutoLock lock(m_BufferLock);
    getDataSegmentIn(idx, offset, endIdx, endOffset, data);
}

private:
    ProjectBuffer(const ProjectBuffer& );
    ProjectBuffer& operator=(const ProjectBuffer&);
    void turnFull()
    {
        assert(!bFull);
        bFull = true;
        fullRecord = lastRecord;
    }

    void getDataSegmentIn(unsigned idx0, unsigned offset0, unsigned idx1, unsigned offset1, std::vector<DataBlock>& data);
public:
    struct BufferConfig{
        BufferConfig():
            waitSecondsStep(30), waitSeconds(UINT_MAX), waitLength(UINT_MAX)
        {}
        unsigned waitSecondsStep;
        unsigned waitSeconds;
        unsigned waitLength;
    };
    static BufferConfig bufferConfig;
    static bool initGlobal(BufferConfig bufferConfig = BufferConfig());
    unsigned long ID;
private:
    struct ArrivalRecord{
        ArrivalRecord():
            seconds(0), unitIdx(0), offset(0)
        {}
        ArrivalRecord(unsigned long secs, unsigned idx, unsigned offset):
            seconds(secs), unitIdx(idx), offset(offset)
        {}
        time_t seconds;
        //unsigned totalLen;
        unsigned unitIdx;
        unsigned offset;
    };
    friend std::ostream& operator<<(std::ostream&, ArrivalRecord);
    LockHelper m_BufferLock;
    std::vector<DataBlock> arrUnits;
    //std::vector<ArrivalRecord> arrArrivalRecords;
    ArrivalRecord fullRecord;
    ArrivalRecord lastRecord;
    time_t prjTime;
    bool bAlloc;
    bool bFull;
    bool bBampHit;
    bool bHasBamp;
    bool bRelsed;
    unsigned bampEndIdx;
    unsigned bampEndOffset;
    unsigned fullUnitIdx;
    unsigned fullOffset;
    struct timeval mainRegStTime;
    struct timeval mainRegEdTime;
    static unsigned ceilUnitIdx;
    static unsigned ceilOffset;
};

struct BufferConfig: public ProjectBuffer::BufferConfig
{
    BufferConfig();
    unsigned m_uBlockSize;
    unsigned m_uBlocksMin;
    unsigned m_uBlocksMax;
};
/***************************************section two*********************************************/

struct ProjectSegment{
    ProjectSegment():
        pid(0), data(NULL), len(0)
    {}
    unsigned long pid;
    char *data;
    unsigned len;
};

bool dispatchOneProj(ProjectBuffer* proj);
typedef bool (*FuncPushProj)(ProjectBuffer* prj);
void getBufferStatus(std::string &outstr);
bool init_bufferglobal(BufferConfig buffConfig, FuncPushProj pushProjAddr = NULL);
void rlse_bufferglobal();
extern "C" void notifyProjFinish(unsigned long pid);
void recvProjSegment(ProjectSegment param, bool iswait = false);
ProjectBuffer* obtainBuffer(unsigned long pid);
void obtainAllBuffers(std::map<unsigned long, ProjectBuffer*>& allBufs);
ProjectBuffer* obtainFullBufferTimeout(unsigned secs = -1);
void returnBuffer(ProjectBuffer* obtained);
extern "C" bool isAllFinished();

}

#endif


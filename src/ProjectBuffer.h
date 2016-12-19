/*************************************************************************
    > File Name: ProjectBuffer.h
    > Author: zhengshurui
    > Mail:  zhengshurui@thinkit.cn
    > Created Time: Mon 12 Sep 2016 02:22:16 AM PDT
 ************************************************************************/

#ifndef PROJECTBUFFER__H
#define PROJECTBUFFER__H

#include <sys/time.h>
#include <pthread.h>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <set>
#include <ostream>

namespace zen4audio{

/*{
    typedef char DataUnit;
    DataUnit* myAlloc();
    void myFree(DataUnit* data);

    //包装一个DataUnit数组，用于DataUnit数组的共享.
    struct DataBlock{
        DataBlock():
            m_buf(NULL), m_len(0), m_cnt(NULL)
        { }

        DataBlock(const DataBlock& other){
            setData(other);
        }
        DataBlock& operator=(const DataBlock& other){
            if(this == &other) return *this;
            relse();
            setData(other);
            return *this;
        }
        ~DataBlock(){
            relse();
        }
        bool initData(){
            bool bRet = false;
            DataUnit* blk = myAlloc();
            if(blk != NULL){
                m_buf = blk;
                m_cnt = new short;
                *m_cnt= 1;
                bRet = true;
            }
            return bRet;
        }
        void setData(const DataBlock& other);
        void relse();

        char *m_buf;
        unsigned m_len;
        short *m_cnt;
    };
}*/

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
    ProjectBuffer(unsigned long pid, time_t curTime = 0){
        init();
        setPid(pid, curTime);
    }
    ~ProjectBuffer(){
        assert(relse());
    }
    void setPid(unsigned long, time_t curTime =0);
    struct timeval getPrjTime(){
        AutoLock lock(m_BufferLock);
        struct timeval ret;
        ret.tv_sec = arrArrivalRecords[0].seconds;
        ret.tv_usec = 0;
        return ret;
    }

    /*
    void getBampEndPos(unsigned &idx, unsigned &st){
        AutoLock lock(m_BufferLock);
        idx = this->bampEndIdx;
        st = this->bampEndOffset;
    }*/
    void setBampEndPos(unsigned preIdx, unsigned preSt, unsigned idx, unsigned st, bool bHit){
        AutoLock lock(m_BufferLock);
        assert(this->bampEndIdx == preIdx);
        assert(this->bampEndOffset == preSt);
        this->bampEndIdx = idx;
        this->bampEndOffset = st;
        if(bHit) this->bBampHit = true;
    }
    void getUnBampData(unsigned &idx, unsigned &st, unsigned &endidx, unsigned &endst, std::vector<DataBlock>& data);
    /*
    unsigned getBampEndPos(){
        AutoLock lock(m_BufferLock);
        return this->uBampEnd;   
    }
    void setBampResult(unsigned segSt, unsigned len, bool bHit){
        AutoLock lock(m_BufferLock);
        assert(segSt == this->uBampEnd);
        uBampEnd += len;
        if(bHit) this->bBampHit = true;
    }
    */
    bool getBampHit(){
        AutoLock lock(m_BufferLock);
        return this->bBampHit;
    }
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
    
    bool relse();
    void getData(std::vector<DataBlock>& vec);
    unsigned getDataLength();
    bool isFull(time_t curTime = 0);
    bool getFull(){
        AutoLock lock(m_BufferLock);
        return bFull;
    }
    /*
    bool isNeedBamp(){
        //std::vector<DataBlock>::iterator it = arrUnits.begin();
        //unsigned total = 0;
        //while(++it != arrUnits.end()){
        //    total += it->m_len;
        //}
        if(bFull && ) return false;
        else return true;
    }
    */
    unsigned recvData(char *data, unsigned len, time_t curTime = 0);
    void finishRecv();
    void init(){
        bAlloc = false;
    }

private:
    ProjectBuffer(const ProjectBuffer& );
    ProjectBuffer& operator=(const ProjectBuffer&);
    void turnFull()
    {
        assert(!bFull);
        bFull = true;
        fullUnitIdx = arrArrivalRecords.back().unitIdx;
        fullOffset = arrArrivalRecords.back().offset;
    }

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
    std::vector<ArrivalRecord> arrArrivalRecords;
    bool bAlloc;
    bool bFull;
    bool bBampHit;
    bool bRelsed;
    //unsigned uBampEnd;
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
bool init_bufferglobal(BufferConfig buffConfig);
void rlse_bufferglobal();
extern "C" void notifyProjFinish(unsigned long pid);
int recvProjSegment(ProjectSegment param, bool iswait = false);
ProjectBuffer* obtainBuffer(unsigned long pid);
void obtainAllBuffers(std::map<unsigned long, ProjectBuffer*>& allBufs);
ProjectBuffer* obtainFullBufferTimeout(unsigned secs = -1);
void returnBuffer(ProjectBuffer* obtained);
extern "C" bool isAllFinished();

}

#endif


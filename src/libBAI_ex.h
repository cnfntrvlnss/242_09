/*************************************************************************
	> File Name: libBAI_ex.h
	> Author: 
	> Mail: 
	> Created Time: Thu 09 Jun 2016 03:35:15 AM CST
 ************************************************************************/

#ifndef _LIBBAI_EX_H
#define _LIBBAI_EX_H

#include <vector>
#include <iostream>
#include <time.h>
#include "commonFunc.h"
#include "../include/interface242.h"

struct BampMatchParam{
    explicit BampMatchParam(unsigned long param1):
        pid(param1), targetID(0)
    {}
    unsigned long pid;
    unsigned preIdx;
    unsigned preOffset;
    unsigned preLen;
    unsigned endIdx;
    unsigned endOffset;
    unsigned tolLen;
    std::vector<DataBlock> data;
    struct timeval curtime;
    unsigned targetID;
};
struct BampMatchObject{
    BampMatchObject(const char *libfile);
    ~BampMatchObject();
    pthread_t threadId;
    void* hdl;
    bool bHasModel;
    bool bOpened;
    LockHelper lock;
    bool loadModel(const char* libFile);
    bool bamp_match(unsigned long pid, char *pcm1, unsigned len1, unsigned preLen, struct timeval curtime);
    bool bamp_match(std::vector<BampMatchParam>& allData);
private:
    BampMatchObject(const BampMatchObject&);
    BampMatchObject& operator=(const BampMatchObject&);
};

BampMatchObject* openBampHandle();
/*
inline bool bamp_match(unsigned long pid, char *pcm1, unsigned len1, unsigned preLen, struct timeval curtime)
{
    BampMatchObject *obj = openBampHandle();
    if(obj) return obj->bamp_match(pid, pcm1, len1, preLen, curtime);
    else return false;
}
*/

extern char g_szBampIp[50];
extern unsigned short g_uBampPort;
extern float g_fReportBampThrd;
extern unsigned g_uBampThreadNum;
extern const unsigned short g_uBampFDServType;
extern const unsigned short g_uBampJCServType;
typedef void (*SummitBampResult)(struct timeval curtime, CDLLResult *pResult, const std::vector<DataBlock>& vecData, std::ostream& oss);
bool bamp_init(SummitBampResult callbck);
bool bamp_rlse();
//bool bamp_match(unsigned long pid, short *pcmData, unsigned pcmLen, unsigned preLen, struct timeval curtime);
//bool bamp_match(unsigned long pid, char *pcm1, unsigned len1, unsigned preLen, struct timeval curtime);
//void *OnecircleOffline(void *param);

#endif

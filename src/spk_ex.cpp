/*************************************************************************
    > File Name: spk_ex.cpp
    > Author: zhengshurui
    > Mail:  zhengshurui@thinkit.cn
    > Created Time: Sun 09 Oct 2016 07:27:23 PM PDT
 ************************************************************************/

#include "spk_ex.h"

#include <pthread.h>
#include <cassert>
#include <cstdio>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
using namespace std;

#include "spkEngine.h"

#define SLOGE_FMT(fmt, ...) fprintf(stderr, "ERROR "fmt"\n", ##__VA_ARGS__)
#define SLOGW_FMT(fmt, ...) fprintf(stderr, "WARN "fmt"\n", ##__VA_ARGS__)

bool SpkInfo::fromStr(const char* param)
{
    istringstream iss(param);
    iss >> spkId;
    return true;
}
string SpkInfo::toStr() const
{
    ostringstream oss;
    oss<< spkId << ".param";
    return oss.str();
}

class SpkInfoEx: public SpkInfo
{
    char flag;
public:
    SpkInfoEx(unsigned long param1=0, char param2=0):
        SpkInfo(param1), flag(param2)
    {}
    SpkInfoEx(const char* param){
        fromStr(param);
    }
    string toStr() const{
        ostringstream oss;
        oss<<spkId<< "_"<< flag<< ".param";
        return SpkInfo::toStr() + "_" + oss.str();
    }

    bool fromStr(const char* param){
        int ret = sscanf(param, "%lu_%c.param", &spkId, &flag);
        if(ret != 2){ return false; }
        return true;
    }
    bool operator==(const SpkInfo& oth) const;
};

/*
bool SpkInfoEx::operator==(const SpkInfo& oth) const{
    if(typeid(*this) != typeid(oth)){
        return false;
    }
    const SpkInfoEx& refOth = static_cast<const SpkInfoEx&>(oth);
    return SpkInfo::operator==(oth) && flag == refOth.flag;
}
*/
//vector<const SpkInfo*> g_vecSpkInfoPtr;

struct SpkCheckBook{
    explicit SpkCheckBook(SpkInfo* s=NULL, int i=-1):
        spk(s), idx(i)
    {}
    SpkInfo *spk;
    int idx;
    set<SpkInfo*> oldSpks;
};

vector<void*> g_vecSpeakerModels;
vector<unsigned long> g_vecSpeakerIDs;
map<unsigned long, SpkCheckBook > g_mapAllSpks;
float defaultSpkScoreThrd = 0.0;
pthread_rwlock_t g_SpkInfoRwlock = PTHREAD_RWLOCK_INITIALIZER;

string g_strSpkMdlDir = "ioacas/SpkModel/";

class SLockHelper{
public:
    SLockHelper(pthread_rwlock_t* param, bool exmark = true):
        plock(param), bRdlock(exmark)
    {
        if(exmark) pthread_rwlock_wrlock(plock);
        else pthread_rwlock_rdlock(plock);
    }

    ~SLockHelper(){
        pthread_rwlock_unlock(plock);
    }

private:
    SLockHelper(const SLockHelper&);
    SLockHelper& operator=(const SLockHelper&);
    pthread_rwlock_t* plock;
    bool bRdlock;
};

void spkex_getAllSpks(vector<const SpkInfo*> &outSpks)
{
    SLockHelper mylock(&g_SpkInfoRwlock, false);
    outSpks.clear();
    for(unsigned idx=0; idx < g_vecSpeakerIDs.size(); idx++){
        outSpks.push_back(g_mapAllSpks[g_vecSpeakerIDs[idx]].spk);
    }
}

bool spkex_addSpk(SpkInfo* spk, char* mdlData, unsigned mdlLen)
{
    SLockHelper mylock(&g_SpkInfoRwlock);
    string mdlpath = g_strSpkMdlDir + spk->toStr();
    TITStatus err = TIT_SPKID_SAVE_MDL_IVEC(mdlData, mdlpath.c_str());
    if(err != StsNoError){
        SLOGE_FMT("in addSpeaker, failed to save arrival data as speaker model file. path=%s; mdlLen=%u; error: %d.", mdlpath.c_str(), mdlLen, err);
        return false;
    }
    void * pData;
    err = TIT_SPKID_LOAD_MDL_IVEC(pData, mdlpath.c_str());
    if(err != StsNoError){
        SLOGE_FMT("in addSpeaker, failed to load model from file. file=%s; error: %d.", mdlpath.c_str(), err);
        return false;
    }

    const unsigned long &spkId = spk->spkId;
    int spkIdx = -1;
    if(g_mapAllSpks.find(spkId) != g_mapAllSpks.end()){
        SpkInfo *oldSpk = g_mapAllSpks[spkId].spk;
        if(oldSpk != NULL){
            spkIdx = g_mapAllSpks[spkId].idx;
            TIT_SPKID_DEL_MDL(g_vecSpeakerModels[spkIdx]);
        }
        if(oldSpk != spk){
            if(oldSpk->refcnt > 0){
                g_mapAllSpks[spkId].oldSpks.insert(oldSpk);   
            }
            else{
                delete oldSpk;
            }
        }
    }
    else{
        spkIdx = g_vecSpeakerModels.size();
        g_mapAllSpks.insert(make_pair(spkId, SpkCheckBook(spk, spkIdx)));
        g_vecSpeakerModels.push_back(NULL);
        g_vecSpeakerIDs.push_back(spkId);
    }
    g_vecSpeakerModels[spkIdx] = pData;
    return true;
}

bool spkex_rmSpk(unsigned long spkId)
{
    SLockHelper mylock(&g_SpkInfoRwlock);
    if(g_mapAllSpks.find(spkId) != g_mapAllSpks.end() && g_mapAllSpks[spkId].spk != NULL){
        SpkInfo *&delSpk = g_mapAllSpks[spkId].spk;
        int spkIdx = g_mapAllSpks[spkId].idx;
        set<SpkInfo*> oldSpks = g_mapAllSpks[spkId].oldSpks;
        if(delSpk->refcnt == 0){
            delete delSpk;
        }
        else{
            oldSpks.insert(delSpk);
        }
        delSpk = NULL;
        if(oldSpks.size() == 0){
            g_mapAllSpks.erase(spkId);
        }
        TIT_SPKID_DEL_MDL(g_vecSpeakerModels[spkIdx]);
        g_vecSpeakerModels.erase(g_vecSpeakerModels.begin() + spkIdx);
        g_vecSpeakerIDs.erase(g_vecSpeakerIDs.begin() + spkIdx);
    }
    else{
        SLOGW_FMT("in removeSpeaker, no spk found in spk array. spk: %d.", spkId);
    }
    return true;
}

unsigned spkex_getAllSpks(std::vector<unsigned long> &spkIds)
{
    spkIds.clear();

}

bool spkex_init(const char* cfgfile)
{
    SLockHelper mylock(&g_SpkInfoRwlock);
    TITStatus err = TIT_SPKID_INIT(cfgfile);
    if(err != StsNoError){
        SLOGE_FMT("in intSpkRec, fail to initialize spk engine. error: %d; file: %s.", err, cfgfile);
        return false;
    }
    return true;
}

void spkex_rlse()
{

    SLockHelper mylock(&g_SpkInfoRwlock);
    assert(g_vecSpkInfoPtr.size() == g_vecSpeakerModel.size());
    for(size_t idx=0; idx < g_vecSpkInfoPtr.size(); idx++){
        TIT_SPKID_DEL_MDL(g_vecSpeakerModel[idx]);
        delete g_vecSpkInfoPtr[idx];
    }
    g_vecSpkInfoPtr.clear();
    g_vecSpeakerModel.clear();
    TITStatus err = TIT_SPKID_EXIT();
    if(err != StsNoError){
        SLOGE_FMT("in rlseSpkRec, failed to release spk engine. error: %d.\n", StsNoError);
    }
}

/**
 * 
 * return 0 if no speaker exist, 1 no error occur, -1 failed in engine.
 */
int spkex_score(short* pcmData, unsigned smpNum, const SpkInfo* &spk, float &score)
{
    SLockHelper mylock(&g_SpkInfoRwlock, false);
    assert(g_vecSpkInfoPtr.size() == g_vecSpeakerModel.size());
    spk = NULL;
    score = -1000.0;
    if(g_vecSpeakerModel.size() == 0){
        
        return 0;
    }
    vector<float> vecScores;
    vecScores.resize(g_vecSpkInfoPtr.size());
    for(size_t idx=0; idx < vecScores.size(); idx ++){
        vecScores[idx] = -1000.0;
    }
    TITStatus err = TIT_SPKID_VERIFY_CLUSTER(pcmData, smpNum, const_cast<const void **>(&g_vecSpeakerModel[0]), g_vecSpeakerModel.size(), &vecScores[0]);
    if(err != StsNoError){
        SLOGE_FMT("in processSpkRec, failed in spk engine. error: %d.", err);
        return -1;
    }
    size_t tIdx = 0;
    for(size_t idx=0; idx < vecScores.size(); idx++){
        if(score < vecScores[idx]){
            tIdx = idx;
        }
    }
    score = vecScores[tIdx];
    if(score >= defaultSpkScoreThrd){
        spk = g_vecSpkInfoPtr[tIdx];
    }
    return 1;
}


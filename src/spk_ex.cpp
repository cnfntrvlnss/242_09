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
using namespace std;

#include "spkEngine.h"

#define SLOGE_FMT(fmt, ...) fprintf(stderr, "ERROR "fmt, ##__VA_ARGS__)
#define SLOGW_FMT(fmt, ...) fprintf(stderr, "WARN "fmt, ##__VA_ARGS__)

bool SpkInfo::fromStr(const char* param)
{
    istringstream iss(param);
    iss >> spkId;
    return true;
}
string SpkInfo::toStr() const
{
    ostringstream oss;
    oss<< spkId;
    return oss.str();
}

class SpkInfoEx: public SpkInfo
{
public:
    SpkInfoEx(unsigned long param1=0, char param2=0):
        SpkInfo(param1), flag(param2)
    {}
    SpkInfoEx(const char* param){
        fromStr(param);
    }
    string toStr() const;
    bool fromStr(const char*);
    bool operator==(const SpkInfo& oth) const;
    char flag;
};

string SpkInfoEx::toStr() const
{
    ostringstream oss;
    oss << flag;
    return SpkInfo::toStr() + "_" + oss.str();
}

bool SpkInfoEx::fromStr(const char *param)
{
    string strTmp(param);
    size_t sepIdx = strTmp.find("_");
    if(sepIdx == string::npos){
        return false;
    }
    SpkInfo::fromStr(strTmp.substr(0, sepIdx).c_str());
    istringstream iss(strTmp.substr(sepIdx+1));
    iss >> flag;
    return true;
}

bool SpkInfoEx::operator==(const SpkInfo& oth) const{
    if(typeid(*this) != typeid(oth)){
        return false;
    }
    const SpkInfoEx& refOth = static_cast<const SpkInfoEx&>(oth);
    return SpkInfo::operator==(oth) && flag == refOth.flag;
}
vector<const SpkInfo*> g_vecSpkInfoPtr;
vector<void*> g_vecSpeakerModel;
float defaultSpkScoreThrd = 0.0;
pthread_rwlock_t g_SpkInfoRwlock = PTHREAD_RWLOCK_INITIALIZER;

string g_strSpkMdlDir = "SpkModel/";

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

void getAllSpkRec(vector<const SpkInfo*> &outSpks)
{
    SLockHelper mylock(&g_SpkInfoRwlock, false);
    outSpks.clear();
    outSpks.insert(outSpks.begin(), g_vecSpkInfoPtr.begin(), g_vecSpkInfoPtr.end());
}

bool addSpkRec(const SpkInfo* spk, char* mdlData, unsigned mdlLen, const SpkInfo*& oldSpk)
{
    oldSpk = NULL;
    SLockHelper mylock(&g_SpkInfoRwlock);
    assert(g_vecSpkInfoPtr.size() == g_vecSpeakerModel.size());
    string mdlpath = g_strSpkMdlDir + spk->toStr() + ".param";
    TITStatus err = TIT_SPKID_SAVE_MDL_IVEC(mdlData, mdlpath.c_str());
    if(err != StsNoError){
        SLOGE_FMT("in addSpeaker, failed to save arrival data as speaker model file. SpkId=%lu; mdlLen=%u; error: %d.", spk->spkId, mdlLen, err);
        return false;
    }
    void * pData;
    err = TIT_SPKID_LOAD_MDL_IVEC(pData, mdlpath.c_str());
    if(err != StsNoError){
        SLOGE_FMT("inf addSpeaker, failed to load model from file. file=%s; error: %d.", mdlpath.c_str(), err);
        return false;
    }
    size_t idx=0;
    for(; idx < g_vecSpkInfoPtr.size(); idx ++){
        if(*spk == *g_vecSpkInfoPtr[idx]){
            break;
        }
    }
    if(idx != g_vecSpkInfoPtr.size()){
        err = TIT_SPKID_DEL_MDL(g_vecSpeakerModel[idx]);
        oldSpk = g_vecSpkInfoPtr[idx];
    }
    else{
        g_vecSpkInfoPtr.resize(idx + 1, NULL);
        g_vecSpeakerModel.resize(idx + 1, NULL);
    }
    g_vecSpkInfoPtr[idx] = spk;
    g_vecSpeakerModel[idx] = pData;
    return true;
}

const SpkInfo* rmSpkRec(const SpkInfo* spk)
{
    SLockHelper mylock(&g_SpkInfoRwlock);
    assert(g_vecSpkInfoPtr.size() == g_vecSpeakerModel.size());
    size_t idx=0;
    for(; idx < g_vecSpkInfoPtr.size(); idx++){
        if(*spk == *g_vecSpkInfoPtr[idx]) break;       
    }
    if(idx == g_vecSpkInfoPtr.size()){
        SLOGW_FMT("in removeSpeaker, no spk found in spk array. spk: %s.\n", spk->toStr().c_str());
        return NULL;
    }

    const SpkInfo* delSpk = g_vecSpkInfoPtr[idx];
    void * delModel = g_vecSpeakerModel[idx];
    g_vecSpkInfoPtr.erase(g_vecSpkInfoPtr.begin() + idx);
    g_vecSpeakerModel.erase(g_vecSpeakerModel.begin() + idx);
    TIT_SPKID_DEL_MDL(delModel);
    string mdlpath = g_strSpkMdlDir + spk->toStr() + ".param";
    if(remove(mdlpath.c_str()) == -1){
        SLOGW_FMT("in removeSpeaker, fail to remove persistent file. file: %s.\n", mdlpath.c_str());
    }
    return delSpk;
}

bool initSpkRec(const char* cfgfile)
{
    SLockHelper mylock(&g_SpkInfoRwlock);
    TITStatus err = TIT_SPKID_INIT(cfgfile);
    if(err != StsNoError){
        SLOGE_FMT("in intSpkRec, fail to initialize spk engine. error: %d; file: %s.", err, cfgfile);
        return false;
    }
    return true;
}

void rlseSpkRec()
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
 * TODO after return, the object pointed by spk can be deleted at any time.
 */
bool processSpkRec(short* pcmData, unsigned smpNum, const SpkInfo* &spk, float &score)
{
    SLockHelper mylock(&g_SpkInfoRwlock, false);
    assert(g_vecSpkInfoPtr.size() == g_vecSpeakerModel.size());
    spk = NULL;
    score = -1000.0;
    if(g_vecSpeakerModel.size() == 0){
        return true;
    }
    vector<float> vecScores;
    vecScores.resize(g_vecSpkInfoPtr.size());
    for(size_t idx=0; idx < vecScores.size(); idx ++){
        vecScores[idx] = -1000.0;
    }
    TITStatus err = TIT_SPKID_VERIFY_CLUSTER(pcmData, smpNum, const_cast<const void **>(&g_vecSpeakerModel[0]), g_vecSpeakerModel.size(), &vecScores[0]);
    if(err != StsNoError){
        SLOGE_FMT("in processSpkRec, failed in spk engine. error: %d.", err);
        return false;
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
    return true;
}

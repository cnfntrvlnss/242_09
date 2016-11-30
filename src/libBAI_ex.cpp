/*************************************************************************
    > File Name: libBAI_ex.cpp
    > Author: 
    > Mail: 
    > Created Time: Wed 08 Jun 2016 09:26:26 PM CST
 ************************************************************************/

#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

#include <cassert>
#include <map>
#include <vector>
#include <string>
#include<iostream>
#include <sstream>
using namespace std;

#include "utilites.h"
#include "wav/Voc2Wav.h"
#include "../include/interface242.h"
#include "libBAI.h"
#include "libBAI_ex.h"
#include "socket_ex.h"

#include "protosrc/2.6/FixedAudioModel.pb.h"
using namespace FixedAudioModel;

#define MAX_PATH 512

char g_szBampIp[50] = "";
const unsigned MAX_BUF = 1024;
const char g_TempDir[MAX_PATH] = "temp/";
float g_fReportBampThrd = 86;

# ifndef _COMMONFUNC_H
enum BLOG_LEVEL{BLOGT, BLOGD, BLOGI, BLOGW, BLOGE};
#define BLOGLMT BLOGI
#define BIFO(x) if(BLOGLMT >= BLOG##x) 
#define BLOGE(fmt, ...)  BIFO(E) fprintf(stderr, "ERROR "fmt "\n", ##__VA_ARGS__)
#define BLOGW(fmt, ...) BIFO(W) fprintf(stderr, "WARN "fmt"\n", ##__VA_ARGS__)
#define BLOGI(fmt, ...) BIFO(I) fprintf(stderr, "INFO "fmt"\n", ##__VA_ARGS__)
#define BLOGD(fmt, ...) BIFO(D) fprintf(stderr, "DEBUG "fmt"\n", ##__VA_ARGS__)
#define BLOGT(fmt, ...) BIFO(T) fprintf(stderr, "TRACE "fmt"\n", ##__VA_ARGS__)
#else
#define BLOGE(fmt, ...) LOGFMT_ERROR(g_logger, fmt, ##__VA_ARGS__)
#define BLOGW(fmt, ...) LOGFMT_WARN(g_logger, fmt, ##__VA_ARGS__)
#define BLOGI(fmt, ...) LOGFMT_INFO(g_logger, fmt, ##__VA_ARGS__)
#define BLOGD(fmt, ...) LOGFMT_DEBUG(g_logger, fmt, ##__VA_ARGS__)
#define BLOGT(fmt, ...) LOGFMT_TRACE(g_logger, fmt, ##__VA_ARGS__)
#endif

struct SampleInfoStruct{
    string fname;
    bool bUpdated;
};

SummitBampResult funcBampSubmitResult;
const unsigned short g_uBampFDServType = 0xc8;
const unsigned short g_uBampJCServType = 0xc3;
static char g_szBampCfgFile[MAX_PATH] = "ioacas/Bamp.cfg";
static unsigned g_uBampThreadNum = 1;
static void * g_pBampHandle = NULL;
static void** g_ppBampHandle = &g_pBampHandle;
pthread_mutex_t g_BampHdlLock = PTHREAD_MUTEX_INITIALIZER;
static void *g_pBampMHdl = NULL;
static void **g_ppBampMHdl = &g_pBampMHdl;
static pthread_mutex_t g_BampMHdlLock = PTHREAD_MUTEX_INITIALIZER;
static char g_szBampLibFile[MAX_PATH] = "ioacas/output/database.dat";
static time_t g_BampLibLastTime = 0;
static bool g_bHasModel = false;
static pthread_mutex_t g_HasModelLock = PTHREAD_MUTEX_INITIALIZER;



static map<unsigned, SampleInfoStruct> g_mBampSamples;

void *buildIndexProcess(void * param);

static bool bamp_loadLocalLib()
{
    struct stat statbuf;
    if(stat(g_szBampLibFile, &statbuf) < 0){
        BLOGD("cannot find libfile %s", g_szBampLibFile);
        return false;
    } 
    bool ret = true;
    time_t curtime = statbuf.st_mtime;
    if(g_BampLibLastTime != curtime ){
        pthread_mutex_lock(&g_BampHdlLock);
        BAI_Code err = BAI_LoadIndex(g_szBampLibFile, g_ppBampHandle);
        pthread_mutex_unlock(&g_BampHdlLock);
        if(err != BAI_OK){
            BLOGE("failed to load file %s.", g_szBampLibFile);
            ret = false;
        }
        else{
            g_BampLibLastTime = curtime;
            pthread_mutex_lock(&g_HasModelLock);
            g_bHasModel = true;
            pthread_mutex_unlock(&g_HasModelLock);
        }
    }
    else{
        ret = false;
    }
    return ret;
}

static bool bamp_loadNewLib(const char *libFile)
{
    pthread_mutex_lock(&g_BampMHdlLock);
    BAI_Code err = BAI_LoadIndex(libFile, g_ppBampHandle);
    pthread_mutex_unlock(&g_BampMHdlLock);
    if(err != BAI_OK){
        BLOGE("failed to load libfile: %s", g_szBampLibFile);
        return false;
    }
    return true;
}
static bool bamp_saveNewLib()
{

}

bool bamp_rlse()
{
    BAI_Code err = BAI_Close(g_ppBampHandle);
    if(err != BAI_OK){
        BLOGE("failed to close one session, err: %d.", err);
        return false;
    }
    err = BAI_Close(g_ppBampMHdl);
    if(err != BAI_OK){
        BLOGE("failed to close one session, err: %d.", err);
        return false;
    }
    err = BAI_Exit();
    if(err != BAI_OK){
        BLOGE("failed to exit bamp.");
        return false;
    }
    return true;
}

static void * UpdateLibraryThread(void *param);
bool bamp_init(SummitBampResult callbck)
{
    BLOGI("starting init BAI.");
    BAI_Code err = BAI_Init(g_szBampCfgFile, g_uBampThreadNum);
    if(err != BAI_OK){
      BLOGE("failed to initialize  bamp engine. err: %d.", err);   
        return false;
    }
    err = BAI_Open(g_ppBampHandle);
    if(err != BAI_OK){
        BLOGE("failed to open one session of bamp match, err: %d.", err);
        BAI_Exit();
        return false;
    }
    err = BAI_Open(g_ppBampMHdl);
    if(err != BAI_OK){
        BLOGE("failed to open one session of bamp match, err: %d.", err);
        BAI_Exit();
        return false;
    }
    if(bamp_loadLocalLib()){
        BLOGI("have load first library from %s.", g_szBampLibFile);
    }
    BLOGI("finishing init BAI.");
    funcBampSubmitResult = callbck;
    pthread_t pthd_id;
    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
    int retcrt = pthread_create(&pthd_id, &threadAttr, UpdateLibraryThread, NULL);
    pthread_attr_destroy(&threadAttr);
    if(retcrt != 0){
        BLOGE("fail to create offlinethread.err: %d", retcrt);
        bamp_rlse();
        return false;
    }
    return true;
}

static void process_bai_resultlist(unsigned long pid, short* pcmData, unsigned pcmLen, BAI_ResultList* pres)
{
}

static string savebinaryData(unsigned long pid, char* data, unsigned len)
{
    char filePath[MAX_PATH];
    struct timeval curtime;
    gettimeofday(&curtime, NULL);
    gen_spk_save_file(filePath, "ioacas", "debug", curtime.tv_sec, pid, NULL, NULL, NULL);
    char *stSufPtr = strrchr(filePath, '.');
    char tmpMrk[20];
    snprintf(tmpMrk, 20, "_%lu", curtime.tv_usec);
    insertStrAt0(stSufPtr, tmpMrk);
    saveWave(data, len, filePath);
    return filePath;
}
//bool bamp_match(unsigned long pid, short *pcmData, unsigned pcmLen, unsigned preLen, struct timeval curtime)
bool bamp_match(unsigned long pid, char *pcm1, unsigned len1, unsigned preLen, struct timeval curtime)
{
    ostringstream oss;
    oss<< "PID=" << pid<< " "<< "WaveLen="<< (len1) / 16000<< " Offset="<< preLen / 16000<< " ";
    bool bHasModel = false;
    pthread_mutex_lock(&g_HasModelLock);
    bHasModel = g_bHasModel;
    pthread_mutex_unlock(&g_HasModelLock);
    if(!bHasModel){
        BLOGT("%sbamp_match no library to use.", oss.str().c_str());
        return false;
    }
    unsigned lensz = len1;
    char *pdata = (char*)malloc(lensz);
    if(pdata == NULL){
        BLOGE("%sbamp_match no memory for copy of input data.", oss.str().c_str());
        return false;
    }
    memcpy(pdata, pcm1, len1);
    BAI_InputItem item;
    BAI_ResultList *pRes = NULL;
    item.iAudioID = 0;
    item.acAudioUrl[0] = '\0';
    item.pcDataBuffer = pdata;
    item.iBufferSize = lensz;
    item.iDataType = 0;
    pthread_mutex_lock(&g_BampHdlLock);
    BLOGT("saved audio segment before bamp match call, file: %s.", savebinaryData(pid, item.pcDataBuffer, item.iBufferSize).c_str());
    BAI_Code err = BAI_Retrieval_Partly_VAD(&item, 1, pRes, g_ppBampHandle);
    pthread_mutex_unlock(&g_BampHdlLock);
    if(err != BAI_OK){
        BLOGE("%sfail to call BAI_Retrieval_Partly, err: %d.", oss.str().c_str(), err);
        return false;
    }

    bool bHit = false;
    CDLLResult desres;
    WavDataUnit desdata;
    desres.m_iDataUnitNum = 1;
    desres.m_pDataUnit[0] = &desdata;
    desdata.m_iPCBID = pid;
    desdata.m_iDataLen = len1;
    desdata.m_pData = pcm1;
    if(pRes == NULL){
        BLOGT("%sbamp_match no retrieval result from library. pRes==NULL.", oss.str().c_str());
    }
    else{
        if(pRes->iResultNum == 0){
            BLOGT("%sbamp_match no retrieval result from library. pRes->iResultNum==0", oss.str().c_str());
        }
        else{
            float fOffset = ((float)preLen) / 16000;
            if(curtime.tv_sec == 0) gettimeofday(&curtime, NULL);
            long stpos = oss.tellp();
            for(size_t idx=0; idx < pRes->iResultNum; idx++){
                BAI_ResultItem &curhit = pRes->pstResultItems[idx];
                if(curhit.fMatchedRate < 1.0001){
                    curhit.fMatchedRate *= 100;
                }
                if(curhit.fMatchedRate < g_fReportBampThrd){
                    continue;
                }
                oss.seekp(stpos);
                oss<< " CfgName=" << curhit.acAudioUrl;
		sscanf(curhit.acAudioUrl, "%u", &desres.m_iTargetID);
                if(desres.m_iTargetID % 2){
                    desres.m_iAlarmType = g_uBampJCServType;
                }
                else{
                    desres.m_iAlarmType = g_uBampFDServType;
                }
                desres.m_iTargetID /= 2;
                desres.m_iHarmLevel = 0;
                desres.m_fTargetMatchLen = curhit.fDurationS;
                desres.m_fLikely = curhit.fMatchedRate;
                desres.m_fSegLikely[0] = curhit.fMatchedRate;
                desres.m_fSegPosInPCB[0] = fOffset + curhit.fTimeStartInTestS;
                desres.m_fSegPosInTarget[0] = curhit.fTimeStartInWaveS;
                funcBampSubmitResult(curtime, &desres, oss);
                oss.put('\0');
                BLOGI("%s", oss.str().c_str());
                bHit = true;
            }
        }
        
        delete[] pRes;
        pRes = NULL;
    }
    
    return bHit;
}

static bool saveBinaryFile(const char* fname, char *pData, unsigned dataLen)
{
    FILE *fp = fopen(fname, "wb");
    if(fp == NULL){
        BLOGE("saveBinaryFile failed to create file %s.", fname);
        return false;
    }
    size_t wlen = fwrite(pData, 1, dataLen, fp);
    bool ret = true;
    if(wlen != dataLen){
        BLOGE("saveBinaryFile write part data to file, total: %u, writen: %lu, file: %s", dataLen, wlen, fname);
        ret = false;
    }
    fclose(fp);
    return ret;
}

void addNewSmp(unsigned int id, const char *fname)
{
    if(g_mBampSamples.find(id) != g_mBampSamples.end()){
        if(g_mBampSamples[id].fname == fname){
            BLOGD("addNewTemp TempID=%u already exists in Bamp library.", id);
            g_mBampSamples[id].bUpdated = false;
            return; 
        } 
    }
    else{
        g_mBampSamples[id].fname = fname;
        g_mBampSamples[id].bUpdated = true;
    }
}

void updateBampLib(const char *listFile)
{
    for(map<unsigned, SampleInfoStruct>::iterator it = g_mBampSamples.begin(); it != g_mBampSamples.end(); it ++){
        it->second.bUpdated = true;
    }

    vector<string> smpList = loadFileList(listFile);
    for(size_t idx = 0; idx < smpList.size(); idx ++){
        unsigned smpId;
        char smpPath[MAX_PATH];
        if(sscanf(smpList[idx].c_str(), "%u %s", &smpId, smpPath) != 2){
            BLOGE("offlineThread failed to parse line to get sample, %s.", smpList[idx].c_str());
            continue;
        }
        addNewSmp(smpId, smpPath);
    }
}

bool hasUpdatedBampLib()
{
    for(map<unsigned, SampleInfoStruct>::iterator it = g_mBampSamples.begin(); it != g_mBampSamples.end(); it ++){
        if(it->second.bUpdated) return true;
    }
    return false;
}


void *buildIndexProcess(void * param)
{
    unsigned libnum  = g_mBampSamples.size();
    assert(libnum != 0);
    BAI_InputItem* smpArr = new BAI_InputItem[libnum];
    size_t procCnt = 0;

    for(map<unsigned, SampleInfoStruct>::iterator it=g_mBampSamples.begin(); it != g_mBampSamples.end(); it++){
        unsigned len;
        float fsecs;
        char *data = reinterpret_cast<char*>(WavExtractToBufferG(it->second.fname.c_str(), 0, 0, 0, -1, &len, &fsecs));
        if(!data){
            BLOGE("buildIndexProcess fail to read sample from file %s", it->second.fname.c_str());
            continue;
        }
        smpArr[procCnt].iAudioID = it->first;
        strcpy(smpArr[procCnt].acAudioUrl, it->second.fname.c_str());
        smpArr[procCnt].pcDataBuffer = data;
        smpArr[procCnt].iBufferSize = len;
        smpArr[procCnt].iDataType = 0;
        procCnt ++;
    }
    assert(procCnt > 0);
    
    BLOGD("before extractDNA, pcDataBuffer: %lx, bufLen: %d.", reinterpret_cast<long unsigned >(static_cast<void*>(smpArr[0].pcDataBuffer)), smpArr[0].iBufferSize);
    BAI_ExtractDNA(smpArr, procCnt, 0);
    BLOGD("after extractDNA, pcDataBuffer: %lx, bufLen: %d.", reinterpret_cast<long unsigned>(static_cast<void*>(smpArr[0].pcDataBuffer)), smpArr[0].iBufferSize);
    BAI_BuildIndex(smpArr, procCnt, g_szBampLibFile, 1);

    struct stat statbuf;
    assert(stat(g_szBampLibFile, &statbuf) == 0);
    return NULL;
}

static char g_szBampSampleListFile[MAX_PATH] = "ioacas/BampSampleList";
static time_t g_TaskLastWatch = 0;
static char g_szBampTaskFile[MAX_PATH] = "ioacas/BampTask";
/**
 * update bamp library by building indexing from local samples or just loading new index.
 * TODO the offline flow needs reconsideration.
 */
void *OnecircleOffline(void *param)
{
    if(!bamp_loadLocalLib())
    {
        time_t mtime = 0;
        struct stat statbuf;
        if(stat(g_szBampSampleListFile, &statbuf) == 0){
            mtime = statbuf.st_mtime;
            if(mtime != g_TaskLastWatch){
                updateBampLib(g_szBampSampleListFile);
                if(hasUpdatedBampLib()){
                   buildIndexProcess(NULL); 
                }
                g_TaskLastWatch = mtime;
            }
        }
    }
    else{
        
    }
    
    return NULL;
}

//msg return data follow write needs, length of which is stored in outlen.
bool parseModelInfo(char *msg, unsigned & outlen)
{
    bool ret = true;
    string strMsg;
    strMsg.resize(outlen);
    memcpy(const_cast<char*>(strMsg.c_str()), msg, outlen);

    ModelInfo model;
    if(!model.ParseFromString(strMsg)){
        BLOGE("fail to parse modelinfo from binary data.");
        ret = false;
    }
    string taskId = model.taskid();
    string modelUrl = model.modelurl();
    BLOGI("task: Id=%s; Url=%s", taskId.c_str(), modelUrl.c_str());
    //load bamp library.
    LoadResult result;
    result.set_taskid(taskId);
    if(!bamp_loadNewLib(modelUrl.c_str())){
        result.set_status(LoadResult::FALURE);
    }
    else{
        result.set_status(LoadResult::SUCCESS);
        copyFile(modelUrl.c_str(), g_szBampLibFile);
        unlink(modelUrl.c_str());
        
    }
    BLOGI("loadresult: %s",result.ShortDebugString().c_str());
    ostringstream oss;
    result.SerializeToOstream(&oss);
    outlen = oss.str().size();
    memcpy(msg, oss.str().c_str(), outlen);
    return ret;
}


bool readMsg(int clifd, char *buf, unsigned &len)
{
    bool ret = true;
    errno = 0;
    int rret = read(clifd, buf, len);
    if(rret == 0){
        BLOGE("clifd read eof.");
        ret = false;
    }
    else if(rret < 0){
        BLOGE("error occure while read from clifd. err: %s.", strerror(errno));
        ret = false;
    }
    assert(len == rret);
    len = rret;
    buf[len] = '\0';
    return ret;
}

bool writeMsg(int clifd, const char* buf, unsigned len)
{
    bool ret = true;
    errno = 0;
    int wret = write(clifd, buf, len);
    if(wret < 0){
        BLOGE("error occure while write to clifd. err: %s.", strerror(errno));
        ret = false;
    }
    assert(wret == len);
    return ret;

}
bool procModelFromClient(int clifd)
{
    bool ret = false;
    while(true){
        char tmpBuf[MAX_BUF];
        unsigned bufLen = 5;
        if(!readMsg(clifd, tmpBuf, bufLen)) break;
        TaskHeader thd;
        string strBuf;
        strBuf.resize(5);
        memcpy(const_cast<char*>(strBuf.c_str()), tmpBuf, 5);
        if(!thd.ParseFromString(strBuf)){
            BLOGE("fail to parse taskheader.");
            break;
        }
        bufLen = thd.tasklength();
        if(!readMsg(clifd, tmpBuf, bufLen)) break;

        parseModelInfo(tmpBuf, bufLen);
        ResultHeader rhd;
        rhd.set_resultlength(bufLen);
        string strRhd;
        rhd.SerializeToString(&strRhd);
        const char *szRhd = strRhd.c_str();
        unsigned szRhdLen = strRhd.size();
        if(!writeMsg(clifd, szRhd, szRhdLen)) break;
        if(!writeMsg(clifd, tmpBuf, bufLen)) break;
        ret = true;
        break;
    }
    close(clifd);
    return ret;
}

int initTcpServer(const char* szIp, unsigned short port)
{
    struct sockaddr_in addr;
    addr = str2addr(szIp, port);
    errno = 0;
    int servfd = initserver(SOCK_STREAM, reinterpret_cast<sockaddr*>(&addr), sizeof(addr), 5);
    if(servfd  <= 0){
        BLOGE("fail to initserver, err: %d, strerr: %s.", errno, strerror(errno));
        exit(1);
    }
    return servfd;
}

void * UpdateLibraryThread(void *param)
{
    unsigned short uPort = 10019;
    char szIp[50];
    if(g_szBampIp[0] == '\0'){
        strcpy(szIp, "192.168.10.95");
    }
    else{
        strncpy(szIp, g_szBampIp, 50);
    }
    BLOGI("bamp server --- %s:%u", szIp, uPort);
    int servfd = initTcpServer(g_szBampIp, uPort);
    struct pollfd fdarr[1];
    fdarr[0].fd = servfd;
    fdarr[0].events = POLLIN;
    int err = 0;
    int waitsecs = 5;
    while((err = poll(fdarr, 1, waitsecs * 1000)) >=0){
        if(err == 0){
            //do some cyclic jobs.
            //OnecircleOffline(NULL);

            continue;
        }
        if(fdarr[0].revents & POLLIN){
            struct sockaddr_in addr;
            socklen_t len = sizeof(addr);
            int clifd = accept(servfd, reinterpret_cast<struct sockaddr*>(&addr), &len);
            if(clifd < 0){
                BLOGE("accept error: %s.", strerror(errno));
                continue;
            }
            BLOGI( "client --- %s.", addr2str(reinterpret_cast<sockaddr*>(&addr)).c_str());
            procModelFromClient(clifd);
        }

        if(fdarr[0].revents & POLLERR){
            BLOGE("(POLLERR) an error has occured.");
            break;
        }
        if(fdarr[0].revents & POLLHUP){
            BLOGE("(POLLHUP) a hangup has occured.");
            break;
        }
        if(fdarr[0].revents & POLLNVAL){
            BLOGE("(POLLNVAL) the descriptor does not reference an open file.");
            break;
        }
    }
    if(err == -1){
        BLOGE("error occured while polling sockfd, err: %s", strerror(errno));
    }
    close(servfd);
    return NULL;
}


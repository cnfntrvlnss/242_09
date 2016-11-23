/*************************************************************************
    > File Name: interface242.cpp
    > Author: zhengshurui
    > Mail:  zhengshurui@thinkit.cn
    > Created Time: Tue 06 Sep 2016 03:31:17 AM PDT
 ************************************************************************/

#include "hhj.h"
//#include "share/config.h"
#include "commonFunc.h"
#include "waveinfo.h"
//#include "bufferglobalEx.h"
#include "utilites.h"
#include "ProjectBuffer.h"


#include "ioareg.h"

//TODO 这两个标记变量需要加锁.
static bool g_bInitialized = false;
static unsigned int g_iModuleID;

static ReceiveResult g_ReportResultAddr;
unsigned short g_uLangServType = 0x97;
const unsigned short g_uSpkServType = 0x92;

const char szIoacasDir[]="./ioacas/";
ConfigRoom g_AutoCfg(((string)szIoacasDir + "SysFunc.cfg").c_str());

static char m_TSI_SaveTopDir[MAX_PATH]= "/home/ioacas/back_wave/";// save hited project.
static bool g_bDiscardable=true;// when feeding data.
bool g_bSaveAfterRec=false; // when after processing, for project ID.

LoggerId g_logger;

std::map<unsigned long,ProjRecord_t> NewReportedID;
pthread_mutex_t g_lockNewReported = PTHREAD_MUTEX_INITIALIZER;
/**
 * TODO 两个参数的含义还是不够明确，不知道怎么写这个函数.
 */
int GetDLLVersion(char *p, int &length)
{
    static char g_strVersion[100];
    strcpy(g_strVersion, "ioacas/lang v0.1.0");
    length = strlen(g_strVersion);
    strncpy(p, g_strVersion, length);
    return 1;
}


static void initGlobal(BufferConfig &myBufCfg)
{
    Config_getValue(&g_AutoCfg, "", "ifSkipSameProject", g_bSaveAfterRec);
    Config_getValue(&g_AutoCfg, "", "savePCMTopDir", m_TSI_SaveTopDir);
    Config_getValue(&g_AutoCfg, "projectBuffer", "ifDiscardable", g_bDiscardable);
    Config_getValue(&g_AutoCfg, "projectBuffer", "waitSecondsStep", myBufCfg.waitSecondsStep);
    Config_getValue(&g_AutoCfg, "projectBuffer", "waitSeconds", myBufCfg.waitSeconds);
    Config_getValue(&g_AutoCfg, "projectBuffer", "waitLength", myBufCfg.waitLength);
    Config_getValue(&g_AutoCfg, "projectBuffer", "bufferBlockSize", myBufCfg.m_uBlockSize);
    Config_getValue(&g_AutoCfg, "projectBuffer", "bufferBlocksMin", myBufCfg.m_uBlocksMin);
    Config_getValue(&g_AutoCfg, "projectBuffer", "bufferBlocksMax", myBufCfg.m_uBlocksMax);

    myBufCfg.waitLength *= 16000;
	
	unsigned tmpLen = strlen(m_TSI_SaveTopDir);
	if(m_TSI_SaveTopDir[tmpLen - 1] != '/'){
		m_TSI_SaveTopDir[tmpLen] = '/';
		m_TSI_SaveTopDir[tmpLen + 1] = '\0';
	}
	g_logger = g_Log4zManager->createLogger("ioacas");
    char strVer[50];
    int verLen = 50;
    GetDLLVersion(strVer, verLen);
    LOG_INFO(g_logger, "version --- "<< strVer);
#define LOG4Z_VAR(x) << #x "=" << x << "\n"
    LOG_INFO(g_logger, "====================config====================\n" 
            LOG4Z_VAR(g_AutoCfg.configFile)
            LOG4Z_VAR(m_TSI_SaveTopDir)
            LOG4Z_VAR(g_bDiscardable)
            LOG4Z_VAR(g_bSaveAfterRec)
            LOG4Z_VAR(g_AutoCfg.configFile)
            LOG4Z_VAR(myBufCfg.waitSecondsStep)
            LOG4Z_VAR(myBufCfg.waitSeconds)
            LOG4Z_VAR(myBufCfg.waitLength )
            );

}

static void *ioacas_maintain_procedure(void *);
int InitDLL(int iPriority,
        int iThreadNum,
        int *pThreadCPUID,
        ReceiveResult func,
        int iDataUnitBufSize,
        char *path,
        unsigned int iModuleID)
{
    if(g_bInitialized) {
        LOGE("ioacas module is already initialized.");
        return 0;
    }
    g_iModuleID = iModuleID;
    g_ReportResultAddr = func;
    
    BufferConfig buffconfig;
    initGlobal(buffconfig);

    init_bufferglobal(buffconfig);

    if(!ioareg_init()){
        return 1;
    }
    {
		pthread_attr_t threadAttr;
		pthread_attr_init(&threadAttr);
        pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
        pthread_t mthrdId;
        int retc = pthread_create(&mthrdId, &threadAttr, ioacas_maintain_procedure, NULL);
        if(retc != 0){
            LOG_ERROR(g_logger, "fail to create maintain thread!");
            exit(1);
        }
		pthread_attr_destroy(&threadAttr);
    }

    g_bInitialized = true;
    return 0;
}

int SendData2DLL(WavDataUnit *p)
{
    assert(p->m_iDataLen % 2 == 0);
    const unsigned HEADSIZE = 100;
    char szHead[HEADSIZE];
    snprintf(szHead, HEADSIZE, "SendData2DLL PID=%lu SIZE=%u ", p->m_iPCBID, p->m_iDataLen);
    if(!g_bInitialized){
        LOG_WARN(g_logger, szHead<<"fail to receive data as ioacas being uninitialized.");
        return -1;
    }

    if(g_bSaveAfterRec){
        pthread_mutex_lock(&g_lockNewReported);
        if(NewReportedID.find(p->m_iPCBID) != NewReportedID.end())
        {
            pthread_mutex_unlock(&g_lockNewReported);
            LOG_WARN(g_logger, szHead<< "abandoned for being processed recently.");
            return -1;
        }
        pthread_mutex_unlock(&g_lockNewReported);
    }

    ProjectSegment pseg;
    pseg.pid = p->m_iPCBID;
    pseg.data = p->m_pData;
    pseg.len = p->m_iDataLen;
    recvProjSegment(pseg, !g_bDiscardable);
    LOG_TRACE(g_logger, szHead<< "have put data to GlobalBuffer.");
    return 0;
}

int CloseDLL()
{
    rlse_bufferglobal();
    return 0;
}

static vector<SpkInfo*> g_vUselessSpks;
static pthread_mutex_t g_UselessSpksLock = PTHREAD_MUTEX_INITIALIZER;
int AddCfg(unsigned int id, 
        const char *strName,
        const char *strConfigFile,
        int iType,
        int iHarmLevel)
{
    if(iType == g_uSpkServType){
        FILE* fp = fopen(strConfigFile, "rb");
        if(fp == NULL) {
            LOGFMT_ERROR(g_logger, "AddCfg failed to open file: %s", strConfigFile);
             return -30;   
        }
        fseek(fp, 0, SEEK_END);
        unsigned uLen = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        char *pData = new char[uLen];
        if(pData == NULL){
            LOGFMT_ERROR(g_logger, "AddCfg failed to allocate memory, size: %u", uLen);
            fclose(fp);
            return -40;
        }
        unsigned rlen = fread(pData, 1, uLen, fp);
        fclose(fp);
        return AddCfgByBuf(pData, uLen, id, iType, iHarmLevel);
    }
    return 0;
}
//TODO: make sure none of parallel unit are in spk recognition.
int AddCfgByBuf(const char *pData,
        int iDataBytes,
        unsigned int id,
        int iType,
        int iHarmLevel)
{
    if(iType == g_uSpkServType){
        //SpkInfoChd* tmpSpk = new SpkInfoChd(id);
        //const SpkInfo* oldSpk;
        //tmpSpk->m_iHarmLevel = iHarmLevel;
        //bool bret = addSpkRec(tmpSpk, const_cast<char*>(pData), static_cast<int>(iDataBytes), oldSpk);
        //if(oldSpk){
        //    g_vUselessSpks.push_back(const_cast<SpkInfo*>(oldSpk));  
        //} 
        //if(!bret){
        //    LOGFMT_ERROR(g_logger, "AddCfgByBuf failed, id=%u dataBytes=%d", id, iDataBytes);
        //    return 0;
        //}
        //return 1;
    }
    return 0;
}

#if 0
static bool addCfgPerFile(const char* szDir, const char* filename)
{
    SpkInfoChd* tmpSpk = new SpkInfoChd(0);
    if(!tmpSpk->fromStr(filename)){
        return false;
    }
    if(AddCfg(tmpSpk->spkId, "", concatePath(szDir, filename).c_str(), g_uSpkServType, tmpSpk->m_iHarmLevel) > 0){
        return true;
    }
    return false;
}
#endif


int AddCfgByDir(int iType, const char *strDir)
{
    if(iType == g_uSpkServType){
        //int iRet = procFilesInDir(strDir, addCfgPerFile);
        //LOGFMT_INFO(g_logger, "AddCfgByDir finishing load spks in dir: %s, SPKCount=%d.", strDir, iRet);
        //return iRet;
    }
    return 0;
}
int RemoveAllCfg(int iType)
{
    if(iType == g_uSpkServType){
        //vector<const SpkInfo*> allspks;
        //getAllSpkRec(allspks);
        //for(size_t idx=0; idx < allspks.size(); idx++){
        //    SpkInfo *curspk = const_cast<SpkInfo*>(rmSpkRec(allspks[idx]));
        //    if(curspk){
        //        g_vUselessSpks.push_back(curspk);
        //    } 
        //}
        //return 1;
    }
    return 0;
}
int RemoveCfgByID(unsigned int id, int iType, int iHarmLevel)
{
    if(iType == g_uSpkServType){
        //SpkInfoChd *curspk = new SpkInfoChd(id);
        //curspk->m_iHarmLevel = iHarmLevel;
        //const SpkInfo* delspk = rmSpkRec(curspk);
        //delete curspk;
        //if(delspk){
        //    g_vUselessSpks.push_back(const_cast<SpkInfo*>(delspk));
        //}
        //return 1;
    }
    return 0;
}

bool SetRecord(int iType, bool bRecord)
{
    LOG_INFO(g_logger, "SetRecord iType="<< iType<< " bRecord="<< bRecord<< " unimplemented.");
   return true;
}
int GetDataNum4Process(int iType[], int num[])
{
    LOG_INFO(g_logger, "GetDataNum4Process unimplemented.");
    return 0;
}

/////////////////////recog process///////////////////
//
/**
 * 文件存储路径为：topdir/200109/01/increNum_ID_username_confidence.wav
 * 文件存储路径写到savedname指向的内存中，作为结果返回。
 */
bool  gen_spk_save_file(char *savedname, const char *topDir, const char *subDir, unsigned long id, unsigned *type, unsigned *userId, int *confidence)
{
	time_t timer;
	struct tm *tmif;
	time(&timer);
	tmif = localtime(&timer);
	char fipnt[10], sepnt[5];
	snprintf(fipnt, 10, "%04d%02d%02d", tmif->tm_year+1900, tmif->tm_mon+1, tmif->tm_mday);
	snprintf(sepnt, 5, "%02d", tmif->tm_hour);
    savedname[0] = '\0';
    int curCnt = 0;
    if(topDir != NULL){
        if(subDir == NULL){
            curCnt = sprintf(savedname, "%s%s", topDir, fipnt);
            if(access(savedname, F_OK) == -1) mkdir(savedname, 0775);
            curCnt += sprintf(savedname+ curCnt, "/%s/", sepnt);
            if(access(savedname, F_OK) == -1) mkdir(savedname, 0775);
        }
        else{
            curCnt = sprintf(savedname, "%s%s/", topDir, subDir);
            if_directory_exists(savedname, true); //create subdir if not exists.
        }
    }
    curCnt += sprintf(savedname + curCnt, "%s%s%02d%02d_%lu", fipnt, sepnt, tmif->tm_min, tmif->tm_sec, id);
    if(type != NULL){
        curCnt += sprintf(savedname + curCnt, "_%u", *type);
    }
    if(userId != NULL){
        curCnt += sprintf(savedname + curCnt, "_%u", *userId);
    }
    if(confidence != NULL){
        curCnt += sprintf(savedname + curCnt, "_%d", *confidence);
    }

    curCnt += sprintf(savedname + curCnt, ".wav");
	return true;
}

bool reportIoacasResult(CDLLResult &result, bool bRep, char *writeLog, unsigned &len)
{
    unsigned long &pid = result.m_pDataUnit[0]->m_iPCBID;
    int confidence = (int)result.m_fLikely;
    unsigned configID = result.m_iTargetID;
    unsigned alarmType = result.m_iAlarmType;
    len += sprintf(writeLog + len, "ALARMTYPE=%u TARGETID=%u CONFIDENCE=%d ", alarmType, configID, confidence);
    char savedfile[MAX_PATH];
    gen_spk_save_file(savedfile, m_TSI_SaveTopDir, NULL, pid, &alarmType, &configID, &confidence);
    snprintf(result.m_strInfo, 1024, "%s:%s", GetLocalIP(), savedfile);
    bool retSave = saveWave((char*)result.m_pDataUnit[0]->m_pData, result.m_pDataUnit[0]->m_iDataLen, savedfile);
    if(retSave){
        if(writeLog!=NULL){
            len += sprintf(writeLog + len, "DATASAVEPATH=%s ", result.m_strInfo);
        }
    }
    
    if(bRep){
        if(alarmType == g_uLangServType) result.m_iTargetID |= 0x200;
        g_ReportResultAddr(g_iModuleID, &result);
        if(alarmType == g_uLangServType) result.m_iTargetID &= ~0x200;
        const char* debugDir = "ioacas/debug/";
        if(if_directory_exists(debugDir)){
            char wholePath[MAX_PATH];
            sprintf(wholePath, "%sMessage_%lu", debugDir, pid);
            save_binary_data(wholePath, &result, sizeof(CDLLResult), result.m_pDataUnit[0], sizeof(WavDataUnit), NULL);
        }
    }
    return true;
}

/**
 * one maintain operation for clearup routine.
 * clear up records before <seconds> seconds go.
 * this function  should be confined in lock of g_lockNewReported.
 */
void maintain_newreported(time_t curtime, unsigned seconds)
{
	debugstring_newreported("before maintance");
	vector<unsigned long> delID;
	for(map<unsigned long,ProjRecord_t>::iterator IDs = NewReportedID.begin();IDs !=NewReportedID.end();++IDs)
	{
		if(curtime - (*IDs).second.timemark >= seconds)
			delID.push_back((*IDs).first);
	}
	for(unsigned int tt = 0;tt < delID.size();++tt)
		NewReportedID.erase(delID[tt]);
	debugstring_newreported("after maintance");
}

void *ioacas_maintain_procedure(void *)
{
    time_t cur_time;
    time(&cur_time);

    if(true){
        //maitain project filter.
        pthread_mutex_lock(&g_lockNewReported);
        static time_t projectfilterlasttime = 0;
        if(cur_time   > 3600 + projectfilterlasttime){
            projectfilterlasttime = cur_time;
            maintain_newreported(cur_time, 3600 * 5);
        }
        pthread_mutex_unlock(&g_lockNewReported);
    }
    if(true){
        //update some configurations.
        static time_t lasttime;
        if(cur_time > 3 + lasttime){
            lasttime = cur_time;
            if(g_AutoCfg.checkAndLoad()){
                ioareg_updateConfig();
            }
        }
    }
    
    return NULL;
}

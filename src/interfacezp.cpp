/*************************************************************************
    > File Name: interface242.cpp
    > Author: zhengshurui
    > Mail:  zhengshurui@thinkit.cn
    > Created Time: Tue 06 Sep 2016 03:31:17 AM PDT
 ************************************************************************/

#include "libBAI_ex.h"
#include "commonFunc.h"
#include "wav/waveinfo.h"
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

//////////////////---bamp---
static bool g_bUseBamp = false;
static unsigned g_uBampFixedLen = 3.0 * PCM_ONESEC_LEN;

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

void* IoaRegThread(void *pvParam);
void *netLogThread(void *param);
void *offLIDRegThread(void *);


/*****************
 * param str eg: 11->1;10->2;
 * return list eg:[(11,1), (10,2)]
 */
static std::vector<std::pair<unsigned char, unsigned char> > parseLangReportsFromStr(const char*strLine)
{
	char tmpLine[256];
	std::vector<std::pair<unsigned char, unsigned char> > retm;
	strncpy(tmpLine, strLine, 256);
	char *st = tmpLine;
	while(true){
		char *tkEnd = strchr(st, ',');
		if(tkEnd != NULL) {
			*tkEnd = '\0';
		}
		unsigned char f,s;
		if(sscanf(st, "%hhu->%hhx", &f, &s) == 2){
			retm.push_back(std::make_pair(f,s));
		}
		if(tkEnd == NULL) break;
		st = tkEnd + 1;
	}
	return retm;
}

/**
 * reverse process of the func above.
 */
static std::string formLangReportsStr(const std::vector<std::pair<unsigned char, unsigned char> >& langReports)
{
	ostringstream oss;
	for(unsigned i=0; i< langReports.size(); i++){
		oss<< (int)langReports[i].first<< "->" << std::hex<< std::showbase<< (int)langReports[i].second<< std::dec<< std::noshowbase << ",";
	}
	return oss.str();
}

static bool parseReportFilter(const char *strLine, std::map<int, int> &filter)
{
    const char *st = strLine;
    int num1, num2;
    unsigned char tmpCh;
    while(true){
        if(sscanf(st, "%hhx:%d", &tmpCh, &num2) == 2){
            num1 = tmpCh;
            filter[num1] = num2;
        }
        st = strchr(st, ',');
        if(st == NULL) break;
        st ++;
    }
    return true;
}
static std::string formReportFilterStr(const std::map<int, int> &filter)
{
    std::ostringstream oss;
    for(std::map<int,int>::const_iterator it=filter.begin(); it != filter.end(); it++){
        oss<< std::hex<< std::showbase<< it->first<< std::dec<< std::noshowbase<< ":"<< it->second <<",";
    }
    return oss.str();
}

static void initGlobal(BufferConfig &myBufCfg)
{
    g_strIp = GetLocalIP();
	char szSysCfgFile[MAX_PATH]="";
	int len =strlen(szEngineDir);
	char szLangReports[256] = "11->1";
    char szLangReportFilter[256] = "";
	strcpy(szSysCfgFile, szEngineDir);
	strcpy(szSysCfgFile+len, "SysFunc.cfg");
    int bampsecs = 0;
	parse_params_from_file(szSysCfgFile,
			"BifSkipSameProject", &g_bSaveAfterRec,
			"SsavePCMTopDir", m_TSI_SaveTopDir,
            "SsaveAllTopDir", g_szAllPrjsDir,
            "BifUseBAMP", &g_bUseBamp,
            "FreportBampThreshold", &g_fReportBampThrd,
            "IBampFixedSeconds", &bampsecs,
            "SserverIp4BAMP", &g_szBampIp,
            "IthreadNumPerStream", &g_ThreadNum, 
            //for lid
			"BifUseLID", &g_bUseLid,
			"Blid.ifUseVAD", &g_bLidUseVAD,
			"Blid.ifUseMusicDetect", &g_bLidUseMusicDetect,
            "IsavedMusicPrecent", &g_iMusicPrecent,
			"SlanguageReports", szLangReports,
            "SlangReportFilter", szLangReportFilter,
            //for projectBuffer
			"BifDiscardable", &g_bDiscardable,
			"IwaitSecondsStep", &(myBufCfg.waitSecondsStep),
			"IwaitSeconds", &(myBufCfg.waitSeconds),
			"IwaitLength", &(myBufCfg.waitLength),
            "IbufferBlockSize", &(myBufCfg.m_uBlockSize),
            "IbufferBlocksMin", &(myBufCfg.m_uBlocksMin),
            "IbufferBlocksMax", &(myBufCfg.m_uBlocksMax),
			NULL
			);

    if(g_szBampIp[0] == '\0'){
        strcpy(g_szBampIp, g_strIp.c_str());
    }
    if(bampsecs > 0){
        g_uBampFixedLen  = bampsecs * PCM_ONESEC_LEN;
    }

    myBufCfg.waitLength *= PCM_ONESEC_LEN;
	if (g_ThreadNum <= 0)  g_ThreadNum = 1;
	
	unsigned tmpLen = strlen(m_TSI_SaveTopDir);
	if(m_TSI_SaveTopDir[tmpLen - 1] != '/'){
		m_TSI_SaveTopDir[tmpLen] = '/';
		m_TSI_SaveTopDir[tmpLen + 1] = '\0';
	}
    tmpLen = strlen(g_szAllPrjsDir);
	if(tmpLen > 0 && g_szAllPrjsDir[tmpLen - 1] != '/'){
		g_szAllPrjsDir[tmpLen] = '/';
		g_szAllPrjsDir[tmpLen + 1] = '\0';
	}

	g_mLangReports = parseLangReportsFromStr(szLangReports);
    parseReportFilter(szLangReportFilter, g_mLangReportControl);
	g_logger = g_Log4zManager->createLogger("ioacas");
    char strVer[50];
    int verLen = 50;
    GetDLLVersion(strVer, verLen);
    LOG_INFO(g_logger, "version --- "<< strVer);
	LOG_INFO(g_logger, "szSysCfgFile="<<szSysCfgFile<<"\n"
			<<"g_ThreadNum="<<            g_ThreadNum<< "\n"
			<<"m_TSI_SaveTopDir="<<       m_TSI_SaveTopDir<<"\n"
			<<"g_bDiscardable="<< g_bDiscardable <<"\n"
			<<"g_bSaveAfterRec="<< g_bSaveAfterRec << "\n"
            <<"g_bUseBamp="<< g_bUseBamp<< "\n"
            <<"g_fReportBampThrd="<< g_fReportBampThrd<< "\n"
            <<"g_uBampFixedLen="<< g_uBampFixedLen<< "\n"
			<<"g_bLidUseVAD="<< g_bLidUseVAD <<"\n"
			<<"g_bUseLid="<< g_bUseLid <<"\n"
			<<"g_mLangReports="<< formLangReportsStr(g_mLangReports)<< "\n"
			<<"g_bLidUseMusicDetect="<< g_bLidUseMusicDetect <<"\n"
            <<"g_iMusicPrecent="<< g_iMusicPrecent<<"\n"
			<<"szMusicDetectCfg="<< szMusicDetectCfg <<"\n"
			<<"szLIDCfgDir="<< szLIDCfgDir <<"\n"
			<<"myBufCfg.waitSecondsStep="<< myBufCfg.waitSecondsStep<< "\n"
			<<"myBufCfg.waitTotalSeconds="<< myBufCfg.waitSeconds<< "\n"
			<<"myBufCfg.waitLength=" << myBufCfg.waitLength <<"\n"
            <<"reportFilter="<< formReportFilterStr(g_mLangReportControl).c_str() <<"\n"
			<<"use "<< g_ThreadNum<<" threads for Recognizition " <<"\n"
			);
}

static void reportBampResultSeg(struct timeval curtime, CDLLResult *pResult, ostream& oss);

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

    int err = pthread_key_create(&g_RecWavBufsKey, free);
    if(err != 0){
        LOG_ERROR(g_logger, "fail to create pthread key, error: "<< err);
        exit(1);
    }

    if(g_bUseBamp){
        if(!bamp_init(reportBampResultSeg)){
            LOG_INFO(g_logger, "fail to initailize bamp engine.");
            return 1;
        }
    }

    init_bufferglobal(buffconfig);

	g_pthread_id = (pthread_t *)malloc(sizeof(pthread_t) * (g_ThreadNum));
	for (int i = 0; i < g_ThreadNum; ++i)
	{
		pthread_attr_t threadAttr;
		pthread_attr_init(&threadAttr);
		pthread_attr_setstacksize(&threadAttr, 2080 * 1024); // 120*1024
		//pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
        RecogThreadSpace *tmpStr = new RecogThreadSpace;
        tmpStr->threadIdx = i;
		int retc = pthread_create(&g_pthread_id[i], &threadAttr, IoaRegThread, tmpStr);
		if(retc != 0){
			LOG_ERROR(g_logger, "fail to create recognition thread, index "<< tmpStr->threadIdx);
            exit(1);
		}
		pthread_attr_destroy(&threadAttr);
	}

    g_bInitialized = true;
    return 0;
}

int SendData2DLL(WavDataUnit *p)
{
    const unsigned HEADSIZE = 100;
    char szHead[HEADSIZE];
    snprintf(szHead, HEADSIZE, "SendData2DLL PID=%lu SIZE=%u ", p->m_iPCBID, p->m_iDataLen);
    if(!g_bInitialized){
        LOG_WARN(g_logger, szHead<<"fail to receive data as ioacas being uninitialized.");
        return -1;
    }
    ProjectSegment prj;
    prj.pid = p->m_iPCBID;
    prj.data = p->m_pData;
    prj.len = p->m_iDataLen;
    bool recvRet = recvProjSegment(prj, !g_bDiscardable);
    LOG_TRACE(g_logger, szHead<< "have put data to GlobalBuffer.");

	struct timeval tmval;
    gettimeofday(&tmval, NULL);
    bool bBampHit = false;
    if(g_bUseBamp){
        char *recBuf1 = NULL;
        unsigned recLen1 = 0;
        char *recBuf2 = NULL;
        unsigned recLen2 = 0;
        std::vector<DataBlock> storedData;
        ProjectBuffer* ptrBuf = obtainBuffer(p->m_iPCBID);
        unsigned dataOffset = 0;
        if(ptrBuf) dataOffset = ptrBuf->getBampEndPos();
        if(p->m_iDataLen >= g_uBampFixedLen){
            // use the data of length p->m_iDataLen.
            recBuf1 = p->m_pData;
            recLen1 = p->m_iDataLen;
        }
        else if(ptrBuf){
            ptrBuf->getData(storedData);
            assert(storedData.size() > 0);
            unsigned totalLen = 0;
            unsigned idx = 0;
            for(; idx < storedData.size(); idx ++){
                DataBlock &curb = storedData[idx];
                totalLen += curb.m_len;
                if(totalLen > dataOffset){
                    recBuf1 = curb.m_buf + curb.m_len - totalLen + dataOffset;
                    recLen1 = totalLen - dataOffset;
                    break;
                }
            }
            assert(idx != storedData.size());
            if(idx != storedData.size() -1){
                assert(idx == storedData.size() - 2);
                idx ++;
                recBuf2 = storedData[idx].m_buf;
                recLen2 = storedData[idx].m_len;
            }
            if(recLen1 + recLen2 < g_uBampFixedLen){
                recBuf1 = NULL;
                recLen1 = 0;
                recBuf2 = NULL;
                recLen2 = 0;
            }
            else if(recLen1 >= g_uBampFixedLen){
                recLen1 = g_uBampFixedLen;
                recBuf2 = NULL;
                recLen2 = 0;
            }
            else if(recLen1 + recLen2 > g_uBampFixedLen){
                recLen2 = g_uBampFixedLen - recLen1;
            }
        }
        if(recBuf1 != NULL){
            char *recBuf = NULL;
            unsigned recLen = 0;
            if(recBuf2){
                //TODO using constant memory.
                recLen = recLen1 + recLen2;
                recBuf = (char*)malloc(recLen);
                memcpy(recBuf, recBuf1, recLen1);
                memcpy(recBuf + recLen1, recBuf2, recLen1);
                recBuf1 = recBuf;
                recLen1 = recLen;
            }
            if(recBuf1){
                if(bamp_match(p->m_iPCBID, recBuf1, recLen1, dataOffset, tmval))
                {
                    bBampHit = true;
                }
                ptrBuf->setBampResult(dataOffset, recLen1, bBampHit);
            }
            if(recBuf){
                free(recBuf);
            }
        }
        struct timeval tmval0 = tmval;
        gettimeofday(&tmval, NULL);
        if(ptrBuf != NULL) returnBuffer(ptrBuf);
        LOGFMTT("%s ElapseInBamp %ld %ld    %ld %ld", szHead, tmval0.tv_sec, tmval0.tv_usec, tmval.tv_sec, tmval.tv_usec);
    }
    {
        time_t cur_time = tmval.tv_sec;
        //维护最近的节目列表, 并过滤节目。.
        pthread_mutex_lock(&g_lockNewReported);
        static time_t maintainprojectfilterlasttime = 0;
        if(maintainprojectfilterlasttime == 0){
            maintainprojectfilterlasttime = cur_time;
        }
        if(cur_time - maintainprojectfilterlasttime > 3600){
            maintainprojectfilterlasttime = cur_time;
            maintain_newreported(cur_time, 3600 * 5);
        }
        if(NewReportedID.find(p->m_iPCBID) != NewReportedID.end())
        {
            pthread_mutex_unlock(&g_lockNewReported);
            LOG_INFO(g_logger, szHead<< "abandoned for being processed recently.");
            return 0;
        }
        pthread_mutex_unlock(&g_lockNewReported);
    }

    return 0;
}

int CloseDLL()
{
    ioareg_rlse();
    rlse_bufferglobal();
    return 0;
}

int AddCfg(unsigned int id, 
        const char *strName,
        const char *strConfigFile,
        int iType,
        int iHarmLevel)
{
    return 0;
}
//TODO: make sure none of parallel unit are in spk recognition.
int AddCfgByBuf(const char *pData,
        int iDataBytes,
        unsigned int id,
        int iType,
        int iHarmLevel)
{
    return 0;
}

int AddCfgByDir(int iType, const char *strDir)
{
    return 0;
}
int RemoveAllCfg(int iType)
{
    return 0;
}
int RemoveCfgByID(unsigned int id, int iType, int iHarmLevel)
{
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

void reportBampResultSeg(struct timeval curtime, CDLLResult *pResult, ostream &oss)
{
    oss<< " InProjectStart="<< pResult->m_fSegPosInPCB[0]<< " MatchedDuration="<< pResult->m_fTargetMatchLen<< " CfgID="<< pResult->m_iTargetID <<" InCfgStart="<< pResult->m_fSegPosInTarget[0]<< " MatchRate="<< pResult->m_fSegLikely[0];
    char savedfile[MAX_PATH];
    savedfile[0] = '\0';
    unsigned short type = pResult->m_iAlarmType;
    const WavDataUnit &prj = (*pResult->m_pDataUnit[0]);
    
    unsigned cfgId = pResult->m_iTargetID;
    int cfgScore = pResult->m_fSegLikely[0];
#if 1
    gen_spk_save_file(savedfile, m_TSI_SaveTopDir, NULL, curtime.tv_sec, prj.m_iPCBID, &type, &cfgId, &cfgScore);
    char * stSufPtr = strrchr(savedfile, '.');
    char projSt[20];
    snprintf(projSt, 20, "_%.2f", pResult->m_fSegPosInPCB[0]);
    insertStrAt0(stSufPtr, projSt);
    snprintf(pResult->m_strInfo, 1024, "%s:%s", g_strIp.c_str(), savedfile);
    if(access(savedfile, F_OK) != 0){
        if(!saveWave(prj.m_pData, prj.m_iDataLen, savedfile)){
            LOGFMT_ERROR(g_logger, "reportBampResultSeg failed to write wave to file %s.", savedfile);
            return;
        }
        oss<< " SavedPath="<< pResult->m_strInfo << " ";
    }

#endif
    g_ReportResultAddr(g_iModuleID, pResult);
}

#if 0
void reportBampResult(time_t curtime, CDLLResult *pResult, ostream& oss)
{
    oss<< " InProjectStart="<< pResult->m_fSegPosInPCB[0]<< " MatchedDuration="<< pResult->m_fTargetMatchLen<< " CfgID="<< pResult->m_iTargetID <<" InCfgStart="<< pResult->m_fSegPosInTarget[0]<< " MatchRate="<< pResult->m_fSegLikely[0];
    char savedfile[MAX_PATH];
    unsigned short type = pResult->m_iAlarmType;
    const WavDataUnit &prj = (*pResult->m_pDataUnit[0]);
    
    gen_spk_save_file(savedfile, m_TSI_SaveTopDir, NULL, curtime, prj.m_iPCBID, &type, NULL, NULL);
    snprintf(pResult->m_strInfo, 1024, "%s:%s", g_strIp.c_str(), savedfile);
    if(access(savedfile, F_OK) != 0){
        if(!saveWave(prj.m_pData, prj.m_iDataLen, savedfile)){
            LOGFMT_ERROR(g_logger, "reportBampResult failed to write wave to file %s.", savedfile);
            return;
        }
        oss<< " ProjectPath="<< pResult->m_strInfo << " ";
    }

    g_ReportResultAddr(g_iModuleID, pResult);
}

bool reportResult(CDLLResult &result, char *writeLog, unsigned &len)
{
    unsigned long &pid = result.m_pDataUnit[0]->m_iPCBID;
    int confidence = (int)result.m_fLikely;
    unsigned configID = result.m_iTargetID;
    unsigned alarmType = result.m_iAlarmType;
    len += sprintf(writeLog + len, "ALARMTYPE=%u TARGETID=%u CONFIDENCE=%d", alarmType, configID, confidence);
    char savedfile[MAX_PATH];
    time_t curtime = time(NULL);
    gen_spk_save_file(savedfile, m_TSI_SaveTopDir, NULL, curtime, pid, &g_uLangServType, &configID, &confidence);
    snprintf(result.m_strInfo, 1024, "%s:%s", g_strIp.c_str(), savedfile);
    bool retSave = saveWave((char*)result.m_pDataUnit[0]->m_pData, result.m_pDataUnit[0]->m_iDataLen, savedfile);
    if(retSave){
        if(writeLog!=NULL){
            len += sprintf(writeLog + len, " DATASAVEPATH=%s", savedfile);
        }
    }
    
    bool bRep = false;
    if(alarmType == g_uLangServType && g_mLangReportControl.find(configID) != g_mLangReportControl.end() && g_mLangReportControl[configID] <= confidence){
        bRep = true;
    }

    if(bRep){
        if(alarmType == g_uLangServType) result.m_iTargetID |= 0x200;
        g_ReportResultAddr(g_iModuleID, &result);
        if(alarmType == g_uLangServType) result.m_iTargetID &= ~0x200;
        const char* debugDir = "ioacas/debug/";
        if(if_directory_exists(debugDir)){
            char wholePath[MAX_PATH];
            sprintf(wholePath, "%sMessage_%lu", debugDir, pid);
            save_binary_data(wholePath, &result, sizeof(CDLLResult), result.m_pDataUnit[0], sizeof(WavDataUnit));
        }
    }
    return true;
}
bool saveWave(char *pData, unsigned len, const char *saveFileName)
{
    bool ret = false;
    FILE *fp = fopen(saveFileName, "wb");
    if(NULL != fp){
        PCM_HEADER pcmheader;
        initialize_wave_header(&pcmheader, len);
        int retw = fwrite(&pcmheader, sizeof(PCM_HEADER), 1, fp);
        if(retw != 1){
            LOG_WARN(g_logger, "fail to save data to file, filename: "<< saveFileName);
        }
        else{
            fwrite(pData, 1, len, fp);
            ret = true;
        }
        fclose(fp);
    }
    return ret;
}
static inline void debugstring_newreported(const char * head)
{
	char tmpcz[500];
	sprintf(tmpcz, "%s, newreported [%lu] ", head, NewReportedID.size());
	string tmpStr = tmpcz;
	LOG_DEBUG(g_logger, tmpStr.c_str());
}

#endif


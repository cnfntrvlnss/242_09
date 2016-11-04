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

using namespace std;
//using namespace BufferGlobal;
using namespace zen4audio;

#include "dllVAD_dup.h"
#include "MusicDetect_dup.h"
#include "TLI_API_dup.h"
#include "spk_ex.h"
#include "dllSRVADCluster.h"
#include "../include/interface242.h"

#define PCM_ONESEC_LEN 16000
#define PCM_ONESEC_SMPS 8000
#define POSTVAD_MIN_LEN (PCM_ONESEC_LEN * 10)

//TODO 这两个标记变量需要加锁.
static bool g_bInitialized = false;
static unsigned int g_iModuleID;

static pthread_key_t g_RecWavBufsKey;
static ReceiveResult g_ReportResultAddr;

const char szEngineDir[]="./ioacas/";
static char szMusicDetectCfg[MAX_PATH] = "./ioacas/Music.cfg";

static char m_TSI_SaveTopDir[MAX_PATH]= "/home/ioacas/back_wave/";// save hited project.
static char g_szAllPrjsDir[MAX_PATH]; //save all projects, when after processing.
static bool g_bDiscardable=true;// when feeding data.
static bool g_bSaveAfterRec=false; // when after processing, for project ID.
//static int g_LIDCutSize=3600 * PCM_ONESEC_LEN / sizeof(short);
//static int g_LIDCutStep=3600 * PCM_ONESEC_LEN / sizeof(short); //sample rate unit.
static int g_ThreadNum = 1;
static pthread_t *g_pthread_id = NULL;

//////////////////----music----
static int g_iMusicPrecent = 50;
static int g_iVADPrecent = 50;
///////////////////---lid---
static bool g_bUseLid=true;
static bool g_bLidUseVAD=false;
static bool g_bLidUseMusicDetect=false;
static char szLIDVADCfg[MAX_PATH] = "./ioacas/VAD_LID.cfg";
static char szLIDCfgDir[MAX_PATH] = "./ioacas/sysdir";
static const unsigned short g_uLangServType = 0x97;
//下面的两个变量是针对于语种的；第一个变量控制着写文件；第二个变量控制着上报。
static std::vector<std::pair<unsigned char, unsigned char> > g_mLangReports;
static std::map<int, int> g_mLangReportControl;

///////////////////---spk---
static ScoreConfig spkScoreCfg;
static const unsigned short g_uSpkServType = 0x92;
static char szSpkVADCfg[MAX_PATH] = "./ioacas/VAD_SID.cfg";
static char szSpkCfgFile[MAX_PATH] = "./ioacas/runSpk.cfg";
static bool g_bUseSpk = true;
static bool g_bSpkUseVad = true;
static bool g_bSpkUseMCut = true;
extern float defaultSpkScoreThrd;

class SpkInfoChd: public SpkInfo
{
public:
    SpkInfoChd(unsigned long param):
        SpkInfo(param)
    {}
    string toStr()const{
        ostringstream oss;
        oss << "_"<< std::hex<< std::showbase<< g_uSpkServType<< std::noshowbase<< std::dec<< "_" << m_iHarmLevel;
        return SpkInfo::toStr() + oss.str();
    }
    bool fromStr(const char* strSpk);

    int m_iHarmLevel;
};


bool SpkInfoChd::fromStr(const char* strSpk){
    istringstream iss(strSpk);
    char chSep;
    unsigned long long pid;
    unsigned servType;
    int harmLevel;
    if(!(iss >> pid)) return false;
    if(!(iss.get(chSep) || chSep != '_')) return false;
    if(!(iss >> std::hex>> servType>> std::dec)) return false;
    if(servType != g_uSpkServType) return false;
    if(!(iss.get(chSep) || chSep != '_')) return false;
    if(!(iss >> harmLevel)) return false;
    this->spkId = pid;
    this->m_iHarmLevel = harmLevel;
    return true;
}

LoggerId g_logger;

struct RecogThreadSpace{
    RecogThreadSpace(){
        threadIdx = 0;
        result.m_iDataUnitNum = 1;
        result.m_pDataUnit[0] = &projData;
    }
    ~RecogThreadSpace(){
    }
    unsigned threadIdx;
    //drain variable in the sequence of recognization.
    CDLLResult result;
    WavDataUnit projData;
    short *vadBuf;
    unsigned vadBufLen;
    short *mcutBuf;
    unsigned mcutBufLen;

};

char* GetLocalIP()    
{          
	int MAXINTERFACES=16;    
    static char retIP[50];
    if(retIP[0] != '\0') return retIP;
	const char *ip = "127.0.0.1";    
	int fd, intrface;      
	struct ifreq buf[MAXINTERFACES];      
	struct ifconf ifc;      
	int thrdNum = 0;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)      
	{      
		ifc.ifc_len = sizeof(buf);      
		ifc.ifc_buf = (caddr_t)buf;      
		if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc))      
		{      
			intrface = ifc.ifc_len / sizeof(struct ifreq);      

			while (intrface-- > 0)      
			{      
				if (!(ioctl (fd, SIOCGIFADDR, (char *) &buf[intrface])))      
				{      
					ip=(inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));      
					//fetch management ip. determined by wheath the third part is even.
					if(strcmp(ip, "127.0.0.1") == 0) continue;
					if(sscanf(ip, "%*d.%*d.%d.%*d", &thrdNum) != 1){
						continue;
					}
                    if(thrdNum %2 == 0){
                      strncpy(retIP, ip, 50);
                      break;       
                    }
				}                          
			}    
		}      
		close (fd);
	}  
	return retIP; 
} 

static bool if_directory_exists(const char *dir, bool bForce = false)
{
	struct stat buf;
	if(stat(dir, &buf)<0 || !S_ISDIR(buf.st_mode)){
		//试着创建目录.
		if(bForce && mkdir(dir, S_IRWXU | S_IRWXG | S_IRWXO)!= -1){
			return true;
		}
		return false;
	}
	return true;
}

int save_binary_data(const char *filePath, const void* ptr, size_t num, ...)
{
    FILE *fp = fopen(filePath, "wb");
    if(fp == NULL){
        LOG_WARN(g_logger, "fail to open file to write data, file: "<< filePath);
        return -1;
    }

    int ret = 0;
    va_list vl;
    va_start(vl, num);
    const void *curPtr = ptr;
    size_t curNum = num;
    while(true){
        int retwr = fwrite(curPtr, 1, curNum, fp);
        ret += retwr;
        if(retwr != curNum){
            LOG_WARN(g_logger, "fail to write data to file, file: " << filePath);
            break;
        }
        curPtr = va_arg(vl, void*);
        if(curPtr == NULL) break;
        curNum = va_arg(vl, size_t);
    }
    
    fclose(fp);
    return ret;
}

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

typedef struct ProjectRecord{
	ProjectRecord(const char *filepath="", time_t timemark = 0){
		if(timemark == 0){
			this->timemark = time(NULL);
		}
		else{
			this->timemark = timemark;
		}
		this->fp = NULL;
		if(strlen(filepath) > 0){
			if(strlen(filepath) > MAX_PATH){
				this->filename[0] = 0;
			}
			else{
				sprintf(this->filename, "%s", filepath);
			}
			fp = fopen(filepath, "a");
			if(fp == NULL){
				LOG_WARN(g_logger, "fail to open file "<<filepath<<"; error: "<< strerror(errno));
			}
			else{
			LOG_DEBUG(g_logger, "have open file "<<filepath);
			}
		}
	}
	~ProjectRecord(){
		if(fp != NULL){
			fclose(fp);
			LOG_DEBUG(g_logger, "have closed file "<<filename);
		}
	}
	FILE *fp;
	time_t timemark;
	char filename[MAX_PATH];
}ProjRecord_t;
map<unsigned long,ProjRecord_t> NewReportedID;

static pthread_mutex_t g_lockNewReported = PTHREAD_MUTEX_INITIALIZER;
//for dubug NewReportedID
static inline void debugstring_newreported(const char * head)
{
	char tmpcz[500];
	sprintf(tmpcz, "%s, newreported [%lu] ", head, NewReportedID.size());
	string tmpStr = tmpcz;
	LOG_DEBUG(g_logger, tmpStr.c_str());
}

/**
 * one maintain operation for clearup routine.
 * clear up records before <seconds> seconds go.
 * this function  should be confined in lock of g_lockNewReported.
 */
static void maintain_newreported(time_t curtime, unsigned seconds)
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
	spkScoreCfg.m_0Value = 0;
	spkScoreCfg.m_100Value = 100;

	char szSysCfgFile[MAX_PATH]="";
	int len =strlen(szEngineDir);
	char szLangReports[256] = "11->1";
    char szLangReportFilter[256] = "";
	strcpy(szSysCfgFile, szEngineDir);
	strcpy(szSysCfgFile+len, "SysFunc.cfg");
	parse_params_from_file(szSysCfgFile,
			"BifSkipSameProject", &g_bSaveAfterRec,
			"SsavePCMTopDir", m_TSI_SaveTopDir,
            "SsaveAllTopDir", g_szAllPrjsDir,
            "IthreadNumPerStream", &g_ThreadNum, 
            //for lid
			"BifUseLID", &g_bUseLid,
			"Blid.ifUseVAD", &g_bLidUseVAD,
			"Blid.ifUseMusicDetect", &g_bLidUseMusicDetect,
            "IsavedMusicPrecent", &g_iMusicPrecent,
            "IsavedVadPrecent", &g_iVADPrecent,
			"SlanguageReports", szLangReports,
            "SlangReportFilter", szLangReportFilter,
            //for spk
            "BifUseSPK", &g_bUseSpk,
            "Bspk.ifUseVAD", &g_bSpkUseVad,
            "Bspk.ifUseMusicDetect", &g_bSpkUseMCut,
            "Fspk.defaultThreshold", &defaultSpkScoreThrd,
            "FzeroScore", &spkScoreCfg.m_0Value,
            "FhundredScore", &spkScoreCfg.m_100Value,
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

    myBufCfg.waitLength *= 16000;
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
			<<"g_bLidUseVAD="<< g_bLidUseVAD <<"\n"
			<<"g_bUseLid="<< g_bUseLid <<"\n"
			<<"g_mLangReports="<< formLangReportsStr(g_mLangReports)<< "\n"
			<<"g_bLidUseMusicDetect="<< g_bLidUseMusicDetect <<"\n"
            <<"g_iMusicPrecent="<< g_iMusicPrecent<<"\n"
            <<"g_iVADPrecent="<< g_iVADPrecent<< "\n"
			<<"szMusicDetectCfg="<< szMusicDetectCfg <<"\n"
			<<"szLIDCfgDir="<< szLIDCfgDir <<"\n"
            <<"g_bUseSpk="<< g_bUseSpk<< "\n"
            <<"g_bSpkUseVad="<< g_bSpkUseVad<< "\n"
            <<"g_bSpkUseMCut="<< g_bSpkUseMCut<< "\n"
            <<"defaultSpkScoreThrd="<< defaultSpkScoreThrd<< "\n"
            <<"spkZeroScore="<< spkScoreCfg.m_0Value<< "\n"
            <<"spkHundredScore="<< spkScoreCfg.m_100Value<< "\n"
			<<"myBufCfg.waitSecondsStep="<< myBufCfg.waitSecondsStep<< "\n"
			<<"myBufCfg.waitTotalSeconds="<< myBufCfg.waitSeconds<< "\n"
			<<"myBufCfg.waitLength=" << myBufCfg.waitLength <<"\n"
            <<"reportFilter="<< formReportFilterStr(g_mLangReportControl).c_str() <<"\n"
			<<"use "<< g_ThreadNum<<" threads for Recognizition " <<"\n"
			);

}

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

    if(g_bUseSpk){
        if(!initSpkRec(szSpkCfgFile)){
            LOG_ERROR(g_logger, "fail to initialize spk engine.");
            return 1;
        }
        else{
            LOG_INFO(g_logger, "finish initializing spk engine.");
        }
    }
    if(g_bSpkUseVad){
        if(!InitVADCluster(szSpkVADCfg)){
            LOG_ERROR(g_logger, "fail to initialize vad cluster engine.");
            return 1;
        }
        LOG_INFO(g_logger, "finish initializing vad cluster engine.");
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
	time_t cur_time;
	time(&cur_time);
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
		LOG_WARN(g_logger, szHead<< "abandoned for being processed recently.");
		return -1;
	}
	pthread_mutex_unlock(&g_lockNewReported);

    recvProjSegment(p->m_iPCBID, p->m_pData, p->m_iDataLen, !g_bDiscardable);
    LOG_TRACE(g_logger, szHead<< "have put data to GlobalBuffer.");
    return 0;
}

int CloseDLL()
{
    LOG_INFO(g_logger, "starting shutdown ioacas.");
	for(int i = 0; i < g_ThreadNum; i++){
		pthread_cancel(g_pthread_id[i]);
	}
    for(int idx=0; idx < g_ThreadNum; idx++){
        pthread_join(g_pthread_id[idx], NULL);
    }

    if(g_bUseSpk) rlseSpkRec();
    if(g_bSpkUseVad) FreeVADCluster();

	if (g_pthread_id != NULL)
	{
		free(g_pthread_id);
		g_pthread_id = NULL;
	}
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
        unsigned len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        char *pData = new char[len];
        if(pData == NULL){
            LOGFMT_ERROR(g_logger, "AddCfg failed to allocate memory, size: %u", len);
            fclose(fp);
            return -40;
        }
        unsigned rlen = fread(pData, 1, len, fp);
        fclose(fp);
        return AddCfgByBuf(pData, len, id, iType, iHarmLevel);
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
        SpkInfoChd* tmpSpk = new SpkInfoChd(id);
        const SpkInfo* oldSpk;
        tmpSpk->m_iHarmLevel = iHarmLevel;
        bool bret = addSpkRec(tmpSpk, const_cast<char*>(pData), static_cast<int>(iDataBytes), oldSpk);
        if(oldSpk){
            g_vUselessSpks.push_back(const_cast<SpkInfo*>(oldSpk));  
        } 
        if(!bret){
            LOGFMT_ERROR(g_logger, "AddCfgByBuf failed, id=%u dataBytes=%d", id, iDataBytes);
            return 0;
        }
        return 1;
    }
    return 0;
}

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


int AddCfgByDir(int iType, const char *strDir)
{
    if(iType == g_uSpkServType){
        int iRet = procFilesInDir(strDir, addCfgPerFile);
        LOGFMT_INFO(g_logger, "AddCfgByDir finishing load spks in dir: %s, SPKCount=%d.", strDir, iRet);
        return iRet;
    }
    return 0;
}
int RemoveAllCfg(int iType)
{
    if(iType == g_uSpkServType){
        vector<const SpkInfo*> allspks;
        getAllSpkRec(allspks);
        for(size_t idx=0; idx < allspks.size(); idx++){
            SpkInfo *curspk = const_cast<SpkInfo*>(rmSpkRec(allspks[idx]));
            if(curspk){
                g_vUselessSpks.push_back(curspk);
            } 
        }
        return 1;
    }
    return 0;
}
int RemoveCfgByID(unsigned int id, int iType, int iHarmLevel)
{
    if(iType == g_uSpkServType){
        SpkInfoChd *curspk = new SpkInfoChd(id);
        curspk->m_iHarmLevel = iHarmLevel;
        const SpkInfo* delspk = rmSpkRec(curspk);
        delete curspk;
        if(delspk){
            g_vUselessSpks.push_back(const_cast<SpkInfo*>(delspk));
        }
        return 1;
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
bool  gen_spk_save_file(char *savedname, const char *topDir, const char *subDir, unsigned long id, unsigned *userId, int *confidence)
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
    if(userId != NULL){
        curCnt += sprintf(savedname + curCnt, "_%u", *userId);
    }
    if(confidence != NULL){
        curCnt += sprintf(savedname + curCnt, "_%d", *confidence);
    }

    curCnt += sprintf(savedname + curCnt, ".wav");
	return true;
}

bool saveWave(char *pData, unsigned len, const char *saveFileName)
{
    bool ret = false;
    FILE *fp = fopen(saveFileName, "ab");
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

bool reportResult(CDLLResult &result, char *writeLog, unsigned &len)
{
    unsigned long &pid = result.m_pDataUnit[0]->m_iPCBID;
    int confidence = (int)result.m_fLikely;
    unsigned configID = result.m_iTargetID;
    unsigned alarmType = result.m_iAlarmType;
    len += sprintf(writeLog + len, "ALARMTYPE=%u TARGETID=%u CONFIDENCE=%d ", alarmType, configID, confidence);
    char savedfile[MAX_PATH];
    gen_spk_save_file(savedfile, m_TSI_SaveTopDir, NULL, pid, &configID, &confidence);
    snprintf(result.m_strInfo, 1024, "%s:%s", GetLocalIP(), savedfile);
    bool retSave = saveWave((char*)result.m_pDataUnit[0]->m_pData, result.m_pDataUnit[0]->m_iDataLen, savedfile);
    if(retSave){
        if(writeLog!=NULL){
            len += sprintf(writeLog + len, "DATASAVEPATH=%s ", savedfile);
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

static inline bool  checkAndSetLidResSt(CDLLResult &res, int nMax, float score)
{
    bool ret = false;
    for(size_t s = 0; s< g_mLangReports.size(); s++){
        if(nMax == g_mLangReports[s].first){
            int tmpscr = score < 1.0001? (int)(score * 100): (int)score;
            res.m_iTargetID = g_mLangReports[s].second;
            res.m_iAlarmType = g_uLangServType;
            res.m_iHarmLevel = 0;
            res.m_fLikely = tmpscr;
            res.m_fSegLikely[0] = tmpscr;
            return true;
        }
    }
    return ret;

}

#define CHECK_PERFOMANCE
#ifdef CHECK_PERFOMANCE
static unsigned mycounter;
static inline void set_count(){
    mycounter = clock();
}
static inline unsigned get_count(){
    return clock() - mycounter;
}

#else
static inline void set_count(){

}
static inline unsigned get_count(){
    return 0;
}
#endif

static void lidRegProcess(RecogThreadSpace &rec, MscCutHandle hMCut, VADHandle hVAD, int hTLI)
{
    char* &pData = rec.projData.m_pData;
    unsigned &dataLen = rec.projData.m_iDataLen;
    unsigned long &pid = rec.projData.m_iPCBID;
	unsigned logLen = 0;
	char WriteLog[1024];
    logLen = 0;
    logLen += sprintf(WriteLog+logLen, "LIDREG PID=%lu WavLen=%us ", pid, dataLen / PCM_ONESEC_LEN);
    #ifdef CHECK_PERFOMANCE
    ostringstream clockoutput("OUTPUTCLOCK ");
    #endif
    while(true){
        short *recBuf = reinterpret_cast<short*>(pData);
        unsigned recBufLen = dataLen / sizeof(short);
        if(recBufLen < POSTVAD_MIN_LEN)
        {
            sprintf(WriteLog+strlen(WriteLog), "too short ");
            rec.mcutBufLen = 0;
            rec.vadBufLen = 0;
            break;
        }

        if(hMCut != NULL){
            set_count();
            bool retm = cutMusic(hMCut, recBuf, recBufLen, rec.mcutBuf, rec.mcutBufLen);
            #ifdef CHECK_PERFOMANCE
            clockoutput<< "CUTMUSIC "<< recBufLen<< " "<< rec.mcutBufLen<< " "<< get_count()<< " ";
            #endif
            if(retm){
            }
            else{
                rec.mcutBufLen = 0;
            }
            logLen += sprintf(WriteLog + logLen, "MusicCutLen=%ds ", rec.mcutBufLen / PCM_ONESEC_SMPS);
            int  mscRatio = (1 - (float)rec.mcutBufLen / recBufLen) * 100; 
            if(g_iMusicPrecent < mscRatio){
                rec.result.m_iTargetID = 0x02;
                rec.result.m_iAlarmType = g_uLangServType;
                rec.result.m_iHarmLevel = 0;
                rec.result.m_fLikely = mscRatio;
                rec.result.m_fSegLikely[0] = mscRatio;
                reportResult(rec.result, WriteLog, logLen);
            }
            recBuf = rec.mcutBuf;
            recBufLen = rec.mcutBufLen;
        }

        if(recBufLen < POSTVAD_MIN_LEN)
        {
            sprintf(WriteLog+strlen(WriteLog), "too short ");
            rec.vadBufLen = 0;
            break;
        }

        if(hVAD != NULL){
            set_count();
            bool retv = cutVAD(hVAD, recBuf, recBufLen, rec.vadBuf, rec.vadBufLen);
            #ifdef CHECK_PERFOMANCE
            clockoutput<< "CUTVAD "<< recBufLen << " "<< rec.vadBufLen<< " "<< get_count()<< " ";
            #endif
            if(retv){
            }
            else{
                rec.vadBufLen = 0;   
            }
            logLen += sprintf(WriteLog + logLen, "VADCutLen=%ds ", rec.vadBufLen / PCM_ONESEC_SMPS);
            int vadRatio = (1 - (float)rec.vadBufLen / recBufLen) * 100;
            if(vadRatio > g_iVADPrecent){
                rec.result.m_iTargetID = 0x01;
                rec.result.m_iAlarmType = g_uLangServType;
                rec.result.m_iHarmLevel = 0;
                rec.result.m_fLikely = vadRatio;
                rec.result.m_fSegLikely[0] = vadRatio;
                reportResult(rec.result, WriteLog, logLen);
            }
            recBuf = rec.vadBuf;
            recBufLen = rec.vadBufLen;
        }



        if(recBufLen < POSTVAD_MIN_LEN)
        {
            sprintf(WriteLog+strlen(WriteLog), "too short ");
            break;
        }
        if(g_bUseLid){
            int nMax;
            float score;
            set_count();
            scoreTLI_dup(hTLI, recBuf, recBufLen, nMax, score);
            #ifdef CHECK_PERFOMANCE
            clockoutput<< "LIDREG "<< recBufLen << " 0 "<< get_count()<< " ";
            #endif
            if(checkAndSetLidResSt(rec.result, nMax, score)){
                reportResult(rec.result, WriteLog, logLen);
            }
        }
        break;
    }
    LOG_INFO(g_logger, WriteLog<< "eop.");
    #ifdef CHECK_PERFOMANCE
    LOGFMTI(clockoutput.str().c_str());
    #endif
}

struct WavBuf{
    short *data;
    unsigned len;
};

static unsigned g_uRecWavBufsCount = 3;
static unsigned g_uWavBufLenMin = 40 * 60 * 8000;//30 minutes buffer for recognition. provided above %90 precents of tasks is of less length than it.

static inline WavBuf* getRecWavBufs()
{
    WavBuf* ret = (WavBuf*)pthread_getspecific(g_RecWavBufsKey);
    if(ret == NULL){
        ret = (WavBuf*)malloc(sizeof(WavBuf) * g_uRecWavBufsCount);
        for(unsigned idx=0; idx < g_uRecWavBufsCount; idx++){
            ret[idx].data = (short*)malloc(sizeof(short) * g_uWavBufLenMin);
            ret[idx].len = g_uWavBufLenMin;
        }
        pthread_setspecific(g_RecWavBufsKey, ret);
    }
    return ret;
}

static inline void release_rec_bufs(unsigned remLen = 0)
{
    WavBuf* localWavBufs = getRecWavBufs();
    if(remLen < g_uWavBufLenMin){
        remLen = g_uWavBufLenMin;
    }
    for(unsigned idx=0; idx < g_uRecWavBufsCount; idx++){
        short * &buf1 = localWavBufs[idx].data;
        unsigned &len1 = localWavBufs[idx].len;
        assert(len1 >= g_uWavBufLenMin);
        //if(len1 > remLen){
            len1 = remLen;
            buf1 = (short*)realloc(buf1, len1 * sizeof(short));
        //}
    }
}

static void prepare_rec_bufs(const vector<DataBlock> &datavec,
        RecogThreadSpace *regSpace)
{
    unsigned totalLen = 0;
    for(unsigned idx = 0; idx < datavec.size(); idx++){
        totalLen += datavec[idx].m_len;
    }
    assert(totalLen % sizeof(short) == 0);
    totalLen /= sizeof(short);
    release_rec_bufs(totalLen);
    WavBuf* localWavBufs = getRecWavBufs();
    regSpace->projData.m_pData = (char*)localWavBufs[0].data;
    regSpace->projData.m_iDataLen = totalLen * sizeof(short);
    regSpace->mcutBuf = NULL;
    regSpace->mcutBufLen = 0;
    regSpace->vadBuf = NULL;
    regSpace->vadBufLen = 0;
    if(g_uRecWavBufsCount > 1){
        regSpace->mcutBuf = localWavBufs[1].data;
        regSpace->mcutBufLen = totalLen;
    }
    if(g_uRecWavBufsCount > 2){
        regSpace->vadBuf = localWavBufs[2].data;
        regSpace->vadBufLen = totalLen;
    }
    
    unsigned curLen = 0;
    for(unsigned idx=0; idx < datavec.size(); idx++){
        memcpy(regSpace->projData.m_pData + curLen, datavec[idx].m_buf, datavec[idx].m_len);
        curLen += datavec[idx].m_len;
    }
}

void* IoaRegThread(void *param)
{
    RecogThreadSpace &This_Buf = *(RecogThreadSpace*)param;
    char* &projData = This_Buf.projData.m_pData;
    unsigned long &curPid = This_Buf.projData.m_iPCBID;
    unsigned &projLen = This_Buf.projData.m_iDataLen;
	
	time_t cur_time;
	LOG_INFO(g_logger, "RegThread "<< This_Buf.threadIdx<< " of stream 0 has started ...");

    int hTLI;
    MscCutHandle hMCut = NULL;
    VADHandle hVAD = NULL;
	//TLI_Open_1(hTLI);
    if(g_bUseLid){
        hTLI = openTLI_dup();
    }
    if(g_bLidUseMusicDetect || g_bSpkUseMCut){
        hMCut = openMusicCut(szMusicDetectCfg);
        assert(hMCut != NULL);
    }
    if(g_bLidUseVAD){
        hVAD = openOneVAD(szLIDVADCfg);
        assert(hVAD != NULL);
    }
    ProjectBuffer *ptrBuf;
	while (true)
	{
        ptrBuf = obtainFullBufferTimeout(-1u);
		time(&cur_time);
		if (ptrBuf != NULL)
		{
            vector<DataBlock> datavec;
            assert(datavec.size() == 0);
			ptrBuf->getData(datavec);
            prepare_rec_bufs(datavec, &This_Buf);

            curPid = ptrBuf->ID;
            //这两个值为0，代表没有结果.
            This_Buf.result.m_iTargetID = 0;
            This_Buf.result.m_iAlarmType = 0;
			//for TLI
			if(g_bUseLid || g_bLidUseMusicDetect || g_bLidUseVAD)
			{
                lidRegProcess(This_Buf, hMCut, hVAD, hTLI);
			}

            if(g_bUseSpk){
                
            }

            //保存节目号，用于后面的去重.
			if(g_bSaveAfterRec){
				pthread_mutex_lock(&g_lockNewReported);
				NewReportedID.insert(map<unsigned long,ProjRecord_t>::value_type(curPid, ProjRecord_t("", cur_time)));
				pthread_mutex_unlock(&g_lockNewReported);
			}
            //保存节目数据. --- for debug.
            size_t tmpLen = strlen(g_szAllPrjsDir);
            if(tmpLen > 0 && g_szAllPrjsDir[tmpLen - 1] == '/' && if_directory_exists(g_szAllPrjsDir)){
                char savedfile[MAX_PATH];
                gen_spk_save_file(savedfile, g_szAllPrjsDir, NULL, curPid, NULL, NULL);
                if(!saveWave(projData, projLen, savedfile)){
                }
                //若子目录存在，才写相应的中间处理语音数据.
                if(hVAD != NULL){
                    gen_spk_save_file(savedfile, g_szAllPrjsDir, "vadcutdir", curPid, NULL, NULL);
                    saveWave((char*)This_Buf.vadBuf, This_Buf.vadBufLen * sizeof(short), savedfile);
                }
                if(hMCut != NULL){
                    gen_spk_save_file(savedfile, g_szAllPrjsDir, "musiccutdir", curPid, NULL, NULL);
                    saveWave((char*)This_Buf.mcutBuf, This_Buf.mcutBufLen * sizeof(short), savedfile);
                }
            }

            release_rec_bufs();
			returnFullBuffer(ptrBuf);
		}
		else{
		}
	}
	//TLI_Close_1(hTLI);
    if(g_bUseLid) closeTLI_dup(hTLI);
    if(g_bLidUseMusicDetect || g_bSpkUseMCut){
        closeMusicCut(hMCut);
    }
    free(param);
    return NULL;
}

/////////////////////offline tasks////////////////////
//TODO 程序框架要升级，保证离线与在线互不影响.
/**
 * offline tasks related variable.
 */
struct OfflineLIDResult{
    OfflineLIDResult(int id=0, int lidCfg=0, int lidScore=0, int spkCfg=0, int spkScore=0)
        :id(id), lidCfg(lidCfg), lidScore(lidScore), spkCfg(spkCfg), spkScore(spkScore)
    { }
    int id;
    int lidCfg;
    int lidScore;
    int spkCfg;
    int spkScore;
};

vector<OfflineLIDResult> g_offResults;
unsigned int g_offTotalCount = 0;
const unsigned g_offCapacity = 100;
pthread_mutex_t g_offResLock = PTHREAD_MUTEX_INITIALIZER;

unsigned  sendOfflineData(const vector<string> & filelist)
{
    for(int i=0; i< filelist.size() && i < g_offCapacity; i++){
        const unsigned ONEPACKETLEN = 45 * 16000;
        FILE *fp = fopen(filelist[i].c_str(), "rb");
        if(fp == NULL){
            LOG_WARN(g_logger, "offline process, fail to open file "<< filelist[i]);
            return i;
        }
        char dataBuf[ONEPACKETLEN];
        fseek(fp, 44, SEEK_SET);
        while(true){
            size_t rnum = fread(dataBuf, 1, ONEPACKETLEN, fp);
            recvProjSegment(i, dataBuf, rnum, true);
            if(rnum < ONEPACKETLEN) break;
        }
        notifyProjFinish(i);
        fclose(fp);
    }
    return filelist.size();
}
static void offlineProcess()
{
    const char *taskFile = "ioacas/tasklist";
    vector<string> filelist = loadFileList(taskFile);
    if(filelist.size() == 0) return;
    int retrm = remove(taskFile);
    if(retrm == -1){
        LOG_WARN(g_logger, "offline process, fail to remove offline task file.");
    }
    g_offTotalCount = filelist.size();
    if(sendOfflineData(filelist) == 0){
        return;
    }
    pthread_mutex_lock(&g_offResLock);
    g_offResults.clear();
    pthread_mutex_unlock(&g_offResLock);
    const char *resultFile = "ioacas/results.txt";
    FILE *fp = fopen(resultFile, "w");
    if(fp == NULL){
        LOG_WARN(g_logger, "offline process, fail to open file to write results. "<< resultFile);
        return;
    }
    unsigned curNum = 0;
    while(true){
        pthread_mutex_lock(&g_offResLock);
        unsigned nextNum = g_offResults.size();
        if(nextNum > curNum){
            for(int idx=curNum; idx < nextNum; idx ++){
                fprintf(fp, "%d    %d    %d    %d    %d\n", g_offResults[idx].id, g_offResults[idx].lidCfg, g_offResults[idx].lidScore, g_offResults[idx].spkCfg, g_offResults[idx].spkScore);
            }
            curNum = nextNum;
        }
        pthread_mutex_unlock(&g_offResLock);
        if(curNum == g_offTotalCount){
            break;
        }
		struct timeval tv;
		tv.tv_sec = 3;
		tv.tv_usec = 0;
        select(1, NULL, NULL, NULL, &tv);

    }
    fclose(fp);

    pthread_mutex_lock(&g_offResLock);
    g_offTotalCount = 0;
    g_offResults.clear();
    pthread_mutex_unlock(&g_offResLock);
}

/*****************************
 * 离线识别过程，监听任务文件tasklist, 生成结果文件results.txt.
 *
 */
void *offLIDRegThread(void *param)
{
    while(true){
        offlineProcess();
		//wait 1 minutes, for saving establishment of TCP connection
		struct timeval timeout;
		timeout.tv_sec = 60;
		timeout.tv_usec = 0;
		select(0, NULL, NULL, NULL, &timeout);

    }
}


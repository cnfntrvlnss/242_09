/*************************************************************************
	> File Name: ioareg.cpp
	> Author: 
	> Mail: 
	> Created Time: Tue 22 Nov 2016 03:58:09 AM EST
 ************************************************************************/

#include "ioareg.h"

#include<iostream>
using namespace std;


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

char g_szAllPrjsDir[MAX_PATH]; //save all projects, when after processing.
pthread_mutex_t g_AllProjsDirLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_key_t g_RecWavBufsKey;
static pthread_t *g_pthread_id = NULL;
static short g_ThreadNum = 1;

//////////////////-----vad-----
static bool g_bUseVAD=false;
static int g_iVADPrecent = 101;
static pthread_mutex_t g_VADPrecentLock = PTHREAD_MUTEX_INITIALIZER;
//////////////////----music----
static bool g_bUseMusicDetect=false;
static int g_iMusicPrecent = 101;
static pthread_mutex_t g_MusicPrecentLock = PTHREAD_MUTEX_INITIALIZER;
static char szMusicDetectCfg[] = "./ioacas/Music.cfg";
///////////////////---lid---
static bool g_bUseLid=true;
static char szLIDVADCfg[MAX_PATH] = "./ioacas/VAD_LID.cfg";
static char szLIDCfgDir[MAX_PATH] = "./ioacas/sysdir";
void* IoaRegThread(void *param);
//下面的两个变量是针对于语种的；第一个变量控制着写文件；第二个变量控制着上报。
static std::vector<std::pair<unsigned char, unsigned char> > g_mLangReports;
static std::map<int, int> g_mLangReportControl;

///////////////////---spk---
static char szSpkVADCfg[MAX_PATH] = "./ioacas/VAD_SID.cfg";
static char szSpkCfgFile[MAX_PATH] = "./ioacas/runSpk.cfg";
static bool g_bUseSpk = true;
static bool g_bSpkUseVad = true;
static bool g_bSpkUseMCut = true;
extern float defaultSpkScoreThrd;

typedef struct{
	float m_0Value;
	float  m_100Value;
	float m_maxValue;// in term of original score.
} ScoreConfig;
static ScoreConfig spkScoreCfg;
typedef int (*TransScore)(float);

static ScoreConfig g_cfg;
static char g_init = 0;
static int trans_score(float f)
{
	if(f > g_cfg.m_maxValue) f = g_cfg.m_maxValue;
	return (f - g_cfg.m_0Value) * ((100) / (g_cfg.m_100Value - g_cfg.m_0Value));
}
TransScore getScoreFunc(ScoreConfig *param)
{
	if(param == NULL && g_init == 0){
		g_cfg.m_0Value = 0.0;
		g_cfg.m_100Value = 100.0;
		g_cfg.m_maxValue = 100.0;
	}
	else if(param != NULL){
		g_init = 1;
		g_cfg = *param;
	}
	return trans_score;
}


class SpkInfoChd: public SpkInfo
{
public:
    SpkInfoChd(unsigned long param):
        SpkInfo(param), servType(0x92), m_iHarmLevel(0)
    {}
    string toStr()const{
        ostringstream oss;
        oss << "_"<< std::hex<< std::showbase<< servType<< std::noshowbase<< std::dec<< "_" << m_iHarmLevel;
        return SpkInfo::toStr() + oss.str();
    }
    bool fromStr(const char* strSpk);
    unsigned char servType;
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
    if(servType != this->servType) return false;
    if(!(iss.get(chSep) || chSep != '_')) return false;
    if(!(iss >> harmLevel)) return false;
    this->spkId = pid;
    this->m_iHarmLevel = harmLevel;
    return true;
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

void ioareg_updateConfig()
{

    if(g_AutoCfg.isUpdated("lid", "savedMusicPrecent")){
        pthread_mutex_lock(&g_MusicPrecentLock);
        Config_getValue(&g_AutoCfg, "lid", "savedMusicPrecent", g_iMusicPrecent);
        pthread_mutex_unlock(&g_MusicPrecentLock);
    }
    if(g_AutoCfg.isUpdated("lid", "savedVadPrecent")){
        pthread_mutex_lock(&g_VADPrecentLock);
        Config_getValue(&g_AutoCfg, "lid", "savedVadPrecent", g_iVADPrecent);
        pthread_mutex_unlock(&g_VADPrecentLock);
    }
    if(g_AutoCfg.isUpdated("", "saveAllTopDir")){
        pthread_mutex_lock(&g_AllProjsDirLock);
        Config_getValue(&g_AutoCfg, "", "saveAllTopDir", g_szAllPrjsDir);
        unsigned tmpLen = strlen(g_szAllPrjsDir);
        if(tmpLen > 0 && g_szAllPrjsDir[tmpLen - 1] != '/'){
            g_szAllPrjsDir[tmpLen] = '/';
            g_szAllPrjsDir[tmpLen + 1] = '\0';
        }
        pthread_mutex_lock(&g_AllProjsDirLock);
    }
}
bool ioareg_init()
{
	spkScoreCfg.m_0Value = 0;
	spkScoreCfg.m_100Value = 100;
	char szLangReports[256] = "11->1";
    char szLangReportFilter[256] = "";
    Config_getValue(&g_AutoCfg, "", "saveAllTopDir", g_szAllPrjsDir);
    Config_getValue(&g_AutoCfg, "", "ifUseVAD", g_bUseVAD);
    Config_getValue(&g_AutoCfg, "", "ifUseMusicDetect", g_bUseMusicDetect);
    Config_getValue(&g_AutoCfg, "", "savedMusicPrecent", g_iMusicPrecent);
    Config_getValue(&g_AutoCfg, "", "savedVadPrecent", g_iVADPrecent);
    Config_getValue(&g_AutoCfg, "", "threadNumPerStream", g_ThreadNum);
    Config_getValue(&g_AutoCfg, "lid","ifUseLID", g_bUseLid);
    Config_getValue(&g_AutoCfg, "lid", "languageReports", szLangReports);
    Config_getValue(&g_AutoCfg, "lid", "langReportFilter", szLangReportFilter);
    Config_getValue(&g_AutoCfg, "spk", "ifUseSPK", g_bUseSpk);
    Config_getValue(&g_AutoCfg, "spk", "ifUseVAD", g_bSpkUseVad);
    Config_getValue(&g_AutoCfg, "spk", "ifUseMusicDetect", g_bSpkUseMCut);
    Config_getValue(&g_AutoCfg, "spk", "zeroScore", spkScoreCfg.m_0Value);
    Config_getValue(&g_AutoCfg, "spk", "hundredScore", spkScoreCfg.m_100Value);
    Config_getValue(&g_AutoCfg, "spk", "defaultThreshold", defaultSpkScoreThrd);
    unsigned tmpLen = strlen(g_szAllPrjsDir);
	if(tmpLen > 0 && g_szAllPrjsDir[tmpLen - 1] != '/'){
		g_szAllPrjsDir[tmpLen] = '/';
		g_szAllPrjsDir[tmpLen + 1] = '\0';
	}

	g_mLangReports = parseLangReportsFromStr(szLangReports);
    parseReportFilter(szLangReportFilter, g_mLangReportControl);

#define LOG4Z_VAR(x) << #x "=" << x << "\n"
    LOG_INFO(g_logger, "====================config====================\n" 
            LOG4Z_VAR(g_ThreadNum)
            LOG4Z_VAR(g_bUseVAD)
            LOG4Z_VAR(g_bUseLid)
            LOG4Z_VAR(formLangReportsStr(g_mLangReports))
            LOG4Z_VAR(g_bUseMusicDetect)
            LOG4Z_VAR(g_iVADPrecent)
            LOG4Z_VAR(formReportFilterStr(g_mLangReportControl).c_str())
            LOG4Z_VAR(szMusicDetectCfg)
            LOG4Z_VAR(szLIDCfgDir)
            LOG4Z_VAR(g_bUseSpk)
            LOG4Z_VAR(g_bSpkUseVad)
            LOG4Z_VAR(g_bSpkUseMCut)
            LOG4Z_VAR(g_iMusicPrecent)
            LOG4Z_VAR(defaultSpkScoreThrd)
            LOG4Z_VAR(spkScoreCfg.m_0Value)
            LOG4Z_VAR(spkScoreCfg.m_100Value)
            );

    int err = pthread_key_create(&g_RecWavBufsKey, free);
    if(err != 0){
        LOG_ERROR(g_logger, "fail to create pthread key, error: "<< err);
        return false;
    }

    if(g_bUseSpk){
        if(!initSpkRec(szSpkCfgFile)){
            LOG_ERROR(g_logger, "fail to initialize spk engine.");
            return false;
        }
        else{
            LOG_INFO(g_logger, "finish initializing spk engine.");
        }
    }
    if(g_bSpkUseVad){
        if(!InitVADCluster(szSpkVADCfg)){
            LOG_ERROR(g_logger, "fail to initialize vad cluster engine.");
            return false;
        }
        LOG_INFO(g_logger, "finish initializing vad cluster engine.");
    }

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
    return true;
}

bool ioareg_rlse()
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

static void vadProcess(RecogThreadSpace &rec, int hVad)
{
    char* &pData = rec.projData.m_pData;
    unsigned &dataLen = rec.projData.m_iDataLen;
    unsigned long &pid = rec.projData.m_iPCBID;
    short *recBuf = reinterpret_cast<short*>(pData);
    unsigned recBufLen = dataLen / sizeof(short);
	unsigned logLen = 0;
	char WriteLog[1024];
    logLen = 0;
    logLen += sprintf(WriteLog+logLen, "VADREG PID=%lu WavLen=%us ", pid, dataLen / PCM_ONESEC_LEN);
    #ifdef CHECK_PERFOMANCE
    ostringstream clockoutput("OUTPUTCLOCK ");
    #endif
    while(true){
        if(recBufLen < POSITIVE_PCM_LEN)
        {
            sprintf(WriteLog+strlen(WriteLog), "too short ");
            rec.mcutBufLen = 0;
            break;
        }
        set_count();
        bool retv = cutVAD(hVad, recBuf, recBufLen, rec.vadBuf, rec.vadBufLen);
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
        pthread_mutex_lock(&g_VADPrecentLock);
        int ivprecent = g_iVADPrecent;
        pthread_mutex_unlock(&g_VADPrecentLock);
        if(vadRatio > ivprecent){
            rec.result.m_iTargetID = 0x01;
            rec.result.m_iAlarmType = g_uLangServType;
            rec.result.m_iHarmLevel = 0;
            rec.result.m_fLikely = vadRatio;
            rec.result.m_fSegLikely[0] = vadRatio;
            reportIoacasResult(rec.result, false, WriteLog, logLen);
        }
        recBuf = rec.vadBuf;
        recBufLen = rec.vadBufLen;
        break;
    }
    LOG_INFO(g_logger, WriteLog<< "eop.");
    #ifdef CHECK_PERFOMANCE
    LOGFMTI(clockoutput.str().c_str());
    #endif

}

static void musicProcess(RecogThreadSpace &rec, MscCutHandle hMCut)
{
    char* &pData = rec.projData.m_pData;
    unsigned &dataLen = rec.projData.m_iDataLen;
    unsigned long &pid = rec.projData.m_iPCBID;
    short *recBuf = reinterpret_cast<short*>(pData);
    unsigned recBufLen = dataLen / sizeof(short);
	unsigned logLen = 0;
	char WriteLog[1024];
    logLen = 0;
    logLen += sprintf(WriteLog+logLen, "MSCREG PID=%lu WavLen=%us ", pid, dataLen / PCM_ONESEC_LEN);
    #ifdef CHECK_PERFOMANCE
    ostringstream clockoutput("OUTPUTCLOCK ");
    #endif
    while(true){
        if(recBufLen < POSITIVE_PCM_LEN)
        {
            sprintf(WriteLog+strlen(WriteLog), "too short ");
            rec.mcutBufLen = 0;
            break;
        }

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
        pthread_mutex_lock(&g_MusicPrecentLock);
        int imprecent = g_iMusicPrecent;
        pthread_mutex_unlock(&g_MusicPrecentLock);
        if(imprecent < mscRatio){
            rec.result.m_iTargetID = 0x02;
            rec.result.m_iAlarmType = g_uLangServType;
            rec.result.m_iHarmLevel = 0;
            rec.result.m_fLikely = mscRatio;
            rec.result.m_fSegLikely[0] = mscRatio;
            reportIoacasResult(rec.result, false, WriteLog, logLen);
        }
        break;
    }
    
    LOG_INFO(g_logger, WriteLog<< "eop.");
    #ifdef CHECK_PERFOMANCE
    LOGFMTI(clockoutput.str().c_str());
    #endif
}

static void lidRegProcess(RecogThreadSpace &rec, MscCutHandle hMCut, int hVAD, int hTLI)
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
        if(recBufLen < POSITIVE_PCM_LEN)
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
            pthread_mutex_lock(&g_MusicPrecentLock);
            int imprecent = g_iMusicPrecent;
            pthread_mutex_unlock(&g_MusicPrecentLock);
            if(imprecent < mscRatio){
                rec.result.m_iTargetID = 0x02;
                rec.result.m_iAlarmType = g_uLangServType;
                rec.result.m_iHarmLevel = 0;
                rec.result.m_fLikely = mscRatio;
                rec.result.m_fSegLikely[0] = mscRatio;
                reportIoacasResult(rec.result, false, WriteLog, logLen);
            }
            recBuf = rec.mcutBuf;
            recBufLen = rec.mcutBufLen;
        }

        if(recBufLen < POSITIVE_PCM_LEN)
        {
            sprintf(WriteLog+strlen(WriteLog), "too short ");
            rec.vadBufLen = 0;
            break;
        }

        if(hVAD != -1){
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
            pthread_mutex_lock(&g_VADPrecentLock);
            int ivprecent = g_iVADPrecent;
            pthread_mutex_unlock(&g_VADPrecentLock);
            if(vadRatio > ivprecent){
                rec.result.m_iTargetID = 0x01;
                rec.result.m_iAlarmType = g_uLangServType;
                rec.result.m_iHarmLevel = 0;
                rec.result.m_fLikely = vadRatio;
                rec.result.m_fSegLikely[0] = vadRatio;
                reportIoacasResult(rec.result, false, WriteLog, logLen);
            }
            recBuf = rec.vadBuf;
            recBufLen = rec.vadBufLen;
        }

        if(recBufLen < POSITIVE_PCM_LEN)
        {
            sprintf(WriteLog+strlen(WriteLog), "too short ");
            break;
        }
        if(g_bUseLid){
            int nMax;
            float score;
            set_count();
            const char* debugDir = "ioacas/debug/";
            if(if_directory_exists(debugDir)){
                char wholePath[MAX_PATH];
                sprintf(wholePath, "%sbeforeTLI_%lu", debugDir, pid);
                save_binary_data(wholePath, reinterpret_cast<char*>(recBuf), recBufLen * sizeof(short), NULL);
            }
            scoreTLI_dup(hTLI, recBuf, recBufLen, nMax, score);
            #ifdef CHECK_PERFOMANCE
            clockoutput<< "LIDREG "<< recBufLen << " 0 "<< get_count()<< " ";
            #endif
            if(checkAndSetLidResSt(rec.result, nMax, score)){
                bool brep = false;
                if(g_mLangReportControl.find(rec.result.m_iTargetID) != g_mLangReportControl.end() && g_mLangReportControl[rec.result.m_iTargetID] <= rec.result.m_fLikely){
                    brep = true;
                }
                reportIoacasResult(rec.result, brep, WriteLog, logLen);
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
	
	struct timeval cur_time;
	LOG_INFO(g_logger, "RegThread "<< This_Buf.threadIdx<< " of stream 0 has started ...");

    int hTLI;
    MscCutHandle hMCut = NULL;
    int hVAD = -1;
    if(g_bUseLid){
        hTLI = openTLI_dup();
    }
    if(g_bUseMusicDetect || g_bSpkUseMCut){
        hMCut = openMusicCut(szMusicDetectCfg);
        assert(hMCut != NULL);
    }
    if(g_bUseVAD){
        hVAD = openOneVAD(szLIDVADCfg);
        assert(hVAD != -1);
    }
    ProjectBuffer *ptrBuf;
	while (true)
	{
        ptrBuf = obtainFullBufferTimeout(-1u);
        gettimeofday(&cur_time, NULL);
		if (ptrBuf != NULL)
		{
            ptrBuf->startMainReg();
            vector<DataBlock> datavec;
            assert(datavec.size() == 0);
			ptrBuf->getData(datavec);
            prepare_rec_bufs(datavec, &This_Buf);

            curPid = ptrBuf->ID;
            //这两个值为0，代表没有结果.
            This_Buf.result.m_iTargetID = 0;
            This_Buf.result.m_iAlarmType = 0;
			//for TLI
			if(g_bUseLid)
			{
                lidRegProcess(This_Buf, NULL, -1, hTLI);
			}

            if(hMCut != NULL){
                musicProcess(This_Buf, hMCut);
            }
            if(hVAD != -1){
                vadProcess(This_Buf, hVAD);
            }

            if(g_bUseSpk){
                
            }

            //保存节目号，用于后面的去重.
			if(g_bSaveAfterRec){
				pthread_mutex_lock(&g_lockNewReported);
				NewReportedID.insert(map<unsigned long,ProjRecord_t>::value_type(curPid, ProjRecord_t("", cur_time.tv_sec)));
				pthread_mutex_unlock(&g_lockNewReported);
			}
            //保存节目数据. --- for debug.
            pthread_mutex_lock(&g_AllProjsDirLock);
            size_t tmpLen = strlen(g_szAllPrjsDir);
            if(tmpLen > 0 && if_directory_exists(g_szAllPrjsDir)){
                char savedfile[MAX_PATH];
                gen_spk_save_file(savedfile, g_szAllPrjsDir, NULL, cur_time.tv_sec, curPid, NULL, NULL, NULL);
                pthread_mutex_unlock(&g_AllProjsDirLock);
                if(!saveWave(projData, projLen, savedfile)){
                }
            }
            else{
                pthread_mutex_unlock(&g_AllProjsDirLock);
            }

            release_rec_bufs();
            ptrBuf->finishMainReg();
			returnBuffer(ptrBuf);
		}
		else{
		}
	}

    if(g_bUseLid) closeTLI_dup(hTLI);
    if(g_bUseMusicDetect || g_bSpkUseMCut){
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
            ProjectSegment pseg;
            pseg.pid = i;
            pseg.data = dataBuf;
            pseg.len = rnum;
            recvProjSegment(pseg, true);
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


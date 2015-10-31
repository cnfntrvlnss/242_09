/*************************************************************************
    > File Name: spktest.cpp
    > Author: zhengshurui
    > Mail:  zhengshurui@thinkit.cn
    > Created Time: Wed 12 Oct 2016 06:39:04 PM PDT
 ************************************************************************/

#include "../spk_ex.h"
#include "../dllSRVADCluster.h"
#include "../MusicDetect_dup.h"
#include "../utilites.h"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

#define SC_PCMSMPS 8000
#define VALID_PCMSMPS 10 * SC_PCMSMPS
static const unsigned MAX_DATA_LEN = 60 * 60 * 16000;// one hour pcm.

char *g_szAudioListFile = NULL;
char *g_szResultListFile = NULL;
char *g_szModelListFile = NULL;
bool g_bDebugMode = false;
#define DEBUG_OUTPUT(FMT, ...) if(g_bDebugMode) fprintf(stderr, FMT"\n", ##__VA_ARGS__);
#define ERROR_OUTPUT(FMT, ...) fprintf(stderr, "ERROR "FMT"\n", ##__VA_ARGS__);

class SpkInfoEx: public SpkInfo
{
public:
    SpkInfoEx(const char* param, char* buf = NULL, unsigned len = 0){
        fname = param;
        mdData = buf;
        mdLen = len;
    }
    string toStr() const{
        return fname;
    }
    void fromStr(const char* param)
    {
        fname =param;
    }
    bool operator==(const SpkInfo& oth) const{
        if(typeid(oth) != typeid(SpkInfoEx)) return false;
        const SpkInfoEx& tmpOth = static_cast<const SpkInfoEx&>(oth);
        return fname == tmpOth.fname;
    }

    string fname;
    char *mdData;
    unsigned mdLen;
};

bool loadBinary2Buf(const char* path, char *pData, unsigned &iolen)
{
    ifstream istm(path, ifstream::binary);
    if(!istm){
        cerr << "error load data from file: "<< path<< std::endl;
        return false;
    }
    bool ret =true;
    istm.seekg(0, istm.end);
    int len = istm.tellg();
    istm.seekg(0, istm.beg);
    if(pData == NULL){
        iolen = len;
    }
    else{
        if(len > iolen) len = iolen;
        istm.read(pData, len);
        iolen  = len;
        if(!istm){
            cerr<< "error reading block from file: "<< path<<", size: "<< len<< ", read: "<< istm.gcount()<< ".\n";
            iolen = istm.gcount();
            ret = false;
        }
    }
    istm.close();
    return true;
    
}
bool loadBinaryFile(const char* path, char *&mdData, unsigned &mdLen)
{
    if(!loadBinary2Buf(path, NULL, mdLen)){
        return false;
    }
    mdData = new char[mdLen];
    if(!loadBinary2Buf(path, mdData, mdLen)){
        return false;
    }
    return true;
}

vector<const SpkInfoEx*> getallMdls(const char* listfile)
{
    vector<const SpkInfoEx*> ret;
    vector<string> vecPath = loadFileList(listfile);
    for(size_t idx=0; idx < vecPath.size(); idx ++){
        char *mdData;
        unsigned mdLen;
        loadBinaryFile(vecPath[idx].c_str(), mdData, mdLen);
        SpkInfoEx* pSpk = new SpkInfoEx(vecPath[idx].c_str(), mdData, mdLen);
        ret.push_back(pSpk);
    }
    return ret;
}

bool prepareBufAndData(const char* fpath, unsigned &dataLen, char*&pData, char*& addBuf1, char* &addBuf2)
{
    static char dataBuf[MAX_DATA_LEN];
    static char addbuf_1[MAX_DATA_LEN];
    static char addbuf_2[MAX_DATA_LEN];
    dataLen = MAX_DATA_LEN;
    pData = dataBuf;
    addBuf1 = addbuf_1;
    addBuf2 = addbuf_2;

    if(!loadBinary2Buf(fpath, pData, dataLen)){
        return false;
    }
    if(dataLen > 44){
        dataLen -= 44;
        pData += 44;
    }

    return true;
}

void parseGlobal(int argc, char *argv[])
{
	while(true){
		int retc = getopt(argc, argv, "c:f:t:d");
		if(retc == -1) break;
		switch(retc){
			case('c'):
                g_szModelListFile = optarg;
				break;
			case('f'):
				g_szAudioListFile = optarg;
				break;
			case('t'):
				g_szResultListFile  = optarg;
				break;
			case('d'):
				g_bDebugMode = true;
			default:
				break;
		}
	}
	if(g_szAudioListFile == NULL || g_szModelListFile == NULL){
		ERROR_OUTPUT( "usage: program <-d> -f audiolist -t resultlist -c modellist.\n\t-d\tdebug mode");
        
		_Exit(1);
	}
	DEBUG_OUTPUT("cfgFile: %s;\nfromFile: %s;\ntoFile: %s;\ndebugMode: %d", g_szModelListFile, g_szAudioListFile, g_szResultListFile, g_bDebugMode);
}


int main(int argc, char *argv[])
{
    parseGlobal(argc, argv);
    vector<const SpkInfoEx*> vecSpks = getallMdls(g_szModelListFile);
    if(vecSpks.size() == 0){
        ERROR_OUTPUT("no spks being configured!!!");
        exit(1);
    }
    else{
        DEBUG_OUTPUT("finish loading spks, count: %u.", vecSpks.size());
    }

    if(!InitVADCluster("VAD_SID.cfg")){
        ERROR_OUTPUT("fail to init vad.");
        exit(1);
    }

    MscCutHandle hMcut = openMusicCut("Music.cfg");
    if(hMcut == NULL){
        exit(1);
    }

    if(!initSpkRec("runSpk.cfg")){
        exit(1);
    }
    const SpkInfo* oldSpk = NULL;
    for(size_t idx = 0; idx < vecSpks.size(); idx++){
        if(!addSpkRec(vecSpks[idx], vecSpks[idx]->mdData, vecSpks[idx]->mdLen, oldSpk)){
            ERROR_OUTPUT("fail to add speaker as configuration. file: %s", vecSpks[idx]->fname.c_str());
            exit(1);
        }
    }
    
    FILE *resfp = NULL;
    vector<string> vecPcms = loadFileList(g_szAudioListFile);
    if(vecPcms.size() == 0) goto exit_main;
    char szLog[1024];
    
    resfp = fopen(g_szResultListFile, "w");
    if(resfp == NULL){
        ERROR_OUTPUT("fail to open file for storing result. file: %s", g_szResultListFile);
    }
    for(size_t idx=0; idx < vecPcms.size(); idx ++){
        unsigned uLogLen = 0;
        uLogLen += snprintf(szLog + uLogLen, 1024, "SPKREC file: %s ", vecPcms[idx].c_str());
        DEBUG_OUTPUT("%s", szLog);
        unsigned dataLen;
        char *pData, *addBuf1, *addBuf2;
        if(prepareBufAndData(vecPcms[idx].c_str(), dataLen, pData, addBuf1, addBuf2)){
            unsigned smpLen = dataLen /2;
            short* smpBuf = reinterpret_cast<short*>(pData);
            short *smpAddBuf1 = reinterpret_cast<short*>(addBuf1);
            short *smpAddBuf2 = reinterpret_cast<short*>(addBuf2);
            unsigned mcutLen = smpLen;
            uLogLen += snprintf(szLog + uLogLen, 1024, "DataLen=%u ", smpLen / SC_PCMSMPS);
            if(!cutMusic(hMcut, smpBuf, smpLen, smpAddBuf1, mcutLen)) continue;
            int vadLen = mcutLen;
            VADBuffer(true, smpAddBuf1, mcutLen, smpAddBuf2, vadLen);
            uLogLen += snprintf(szLog + uLogLen, 1024, "MCutLen=%u VadLen=%u ", mcutLen / SC_PCMSMPS, vadLen / SC_PCMSMPS);
            DEBUG_OUTPUT("%s", szLog);
            if(vadLen < VALID_PCMSMPS){
                uLogLen += snprintf(szLog + uLogLen, 1024, " too short");
                continue;
            }

            const SpkInfo* tspk;
            float spkScore;
            processSpkRec(smpAddBuf2, vadLen, tspk, spkScore);
            if(tspk == NULL){
                uLogLen += snprintf(szLog + uLogLen, 1024, " SPK=None ");
            }
            else{
                const SpkInfoEx* tspkex = static_cast<const SpkInfoEx*>(tspk);
                string mdname = tspkex->fname;
                mdname = getBasename(mdname.c_str());
                uLogLen += snprintf(szLog + uLogLen, 1024, " SPKName=%s SPKScore=%d ", mdname.c_str(), spkScore);
            }
            DEBUG_OUTPUT("%s", szLog);
            if(resfp){
                fprintf(resfp, "%s\n", szLog);
            }
        }

    }
    if(resfp) fclose(resfp);

exit_main:
    rlseSpkRec();
    FreeVADCluster();
    closeMusicCut(hMcut);
}


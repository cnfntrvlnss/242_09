/*************************************************************************
    > File Name: test242.cpp
    > Author: zhengshurui
    > Mail:  zhengshurui@thinkit.cn
    > Created Time: Thu 08 Sep 2016 11:23:55 PM PDT
 ************************************************************************/

#include <dirent.h>
#include <time.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <regex.h>
#include <errno.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <signal.h>
#include <dlfcn.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <memory>
using namespace std;

#include "apue.h"
#include "myiofuncs.h"
#include "comm_struct.h"
#include "myqueue.h"
#include "../../include/interface242.h"
//#include "../../src/log4z.h"
//#include "../../src/commonFunc.h"
//extern zsummer::log4z::ILog4zManager *g_Log4zManager;

typedef int (*FuncInitDll)(int iPriority,
        int iThreadNum,
        int *pThreadCPUID,
        ReceiveResult func,
        int iDataUnitBufSize,
        char *path,
        unsigned int iModuleID);
typedef int (*FuncSendData2DLL)(WavDataUnit *p);
typedef int (*FuncCloseDll)();
typedef int (*FuncNotifyProjFinish)(unsigned long);
typedef bool (*FuncIsAllFinished)();
void *g_Handler4Ioacas;
FuncInitDll funcInitDll;
FuncSendData2DLL funcSendData2Dll;
FuncCloseDll funcCloseDll;
FuncNotifyProjFinish funcNotifyProjFinish;
FuncIsAllFinished funcIsAllFinished;
#define MAX_PATH 512
unsigned g_packetBytes = 320000;
//namespace zen4audio{
//    void notifyProjFinish(unsigned long);
//    bool isAllFinished();
//}

struct ProjInfo{
public: 
	ProjInfo(): id(0), dataLen(0), sendLen(0), retLen(0), iAlarmType(0), starttime(0), recatchtime(0), targetID(0), iLikely(0)
	{
        filePath[0] = '\0';
        m_objLock = new pthread_mutex_t;
		pthread_mutex_init(m_objLock, NULL);
	}
	~ProjInfo(){
		pthread_mutex_destroy(m_objLock);
        delete m_objLock;
	}
    ProjInfo(const ProjInfo& other){
        *this = other;
        memcpy(this, &other, sizeof(ProjInfo));
        m_objLock = new pthread_mutex_t;
        pthread_mutex_init(m_objLock, NULL);
    }
    ProjInfo& operator=(const ProjInfo& other){
        if(this != &other){
            pthread_mutex_t *tmpPtr = m_objLock;
            memcpy(this, &other, sizeof(ProjInfo));
            m_objLock = tmpPtr;
        }
        return *this;
    }
	string toString() const{
		ostringstream oss;
        const char * basename = strrchr(filePath, '/');
        if(basename == NULL){
            basename = filePath;
        }
        else{
            basename ++;
        }

		oss<< "id: "<<id<<" file: "<< basename;
        if(starttime == 0){
            return oss.str();
        }
        oss<< " alarmType: "<< iAlarmType<<" dataLen: "<< dataLen<<" sendLen: " << sendLen;
        if(recatchtime == 0){
            return oss.str();
        }
        oss<< " retLen: "<< retLen<< " elapseTime: "<< recatchtime - starttime<< " targetID: "<< targetID<< " likely: "<< iLikely;
		return oss.str();
	}
	unsigned long long id; //unique identity of one object.
    //std::string filePath;
    char filePath[MAX_PATH];
	long int dataLen; //total size anticipated by some way.
	long int sendLen;//already send num of data 
    long int retLen; 
	time_t starttime;
    unsigned int iAlarmType;
	time_t recatchtime;
    unsigned int targetID;
    int iLikely;
	pthread_mutex_t *m_objLock;
};

map<unsigned long long, ProjInfo> g_mId2Infos;
FILE *g_fpRes = NULL;

static string csubstr(const char *str, unsigned st, unsigned ed)
{
    const unsigned maxLineLength = 1024;
    char curStr[maxLineLength];
    unsigned len = ed- st;
    len = len > maxLineLength -1 ? maxLineLength - 1 : len;
    strncpy(curStr, str + st, len);
    curStr[len] = '\0';
    return string(curStr);
}
vector<string> get_file_list(const char *root)
{
	vector<string> ret;
	DIR * pDir;
    struct dirent *pDirent;
    if((pDir = opendir(root)) == NULL)
		err_sys("can't open %s", root);
	while((pDirent = readdir(pDir)) != NULL)
	{
		char *curname = pDirent->d_name;
        if(curname[0] == '.') continue;
		char *lastdotptr = strrchr(curname, '.');
		if(lastdotptr != NULL && strcmp(lastdotptr, ".wav")==0){
			ret.push_back(pDirent->d_name);
		}
	}
	return ret;
}

int receive_result(unsigned int iModuleID, CDLLResult *res)
{
	ProjInfo &curobj = g_mId2Infos[res->m_pDataUnit[0]->m_iPCBID];
	pthread_mutex_lock(curobj.m_objLock);
	if(curobj.recatchtime == 0){
		curobj.recatchtime = time(NULL);
        curobj.retLen = res->m_pDataUnit[0]->m_iDataLen;
        curobj.targetID = res->m_iTargetID;
        curobj.iLikely = (int)res->m_fLikely;
	}
	pthread_mutex_unlock(curobj.m_objLock);
    string tmpStr = curobj.toString() + "\n";
    if(g_fpRes != NULL){
        fwrite(tmpStr.c_str(), 1, tmpStr.size(), g_fpRes);
    }
    //LOGI(tmpStr);
    printf(tmpStr.c_str());
    
    return 0;
}

/**
 * a thread routine, fetch task from a queue passed as parameter. commit task by invoking ioacas_api
 *
 */
void *send_data_second_timer_sub(void *param)
{
	Myqueue *ptaskque = (Myqueue*)param;
	char *databuf = (char*)malloc(g_packetBytes);
	if(databuf == NULL){
		err_sys("fail to malloc memmory of size ", g_packetBytes);
	}
	pthread_t curpid = pthread_self();
    WavDataUnit dataUnit;
	int count = 0;
	while(true){
		ProjInfo *ptask = (ProjInfo*)myqueue_take(ptaskque);
		if(ptask == NULL){
			break;
		}
        //LOGFMTI("<%u> take task[%d]  %s\n", (unsigned)curpid, count++, ptask->filePath);
        printf("<%u> take task[%d]  %s\n", (unsigned)curpid, count++, ptask->filePath);
		FILE *ifd = fopen(ptask->filePath, "rb");
		if(ifd == NULL){
			//LOGFMTE("fail to open file %s\n", ptask->filePath);
			fprintf(stderr, "fail to open file %s\n", ptask->filePath);
			continue;
		}
		fseek(ifd, 0, SEEK_END);
		long int filelen = ftell(ifd);
		ptask->dataLen = filelen - 44;
		fseek(ifd, 44, SEEK_SET);
        ptask->sendLen = 0;
		struct timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		while(true)
		{
			int bulksize = fread(databuf, 1, g_packetBytes, ifd);
			if(bulksize > 0){
				//Ioacas_API( ptask->id, databuf, bulksize, NULL, 0);
                dataUnit.m_iPCBID = ptask->id;
                dataUnit.m_iDataLen = bulksize;
                dataUnit.m_pData = databuf;
                if(funcSendData2Dll(&dataUnit) == 0){
                    ptask->sendLen += bulksize;
                }
			}
			if(ferror(ifd) && !feof(ifd)){
				//LOGFMTE("error: %s; file %s\n", strerror(errno), ptask->filePath);
				printf("error: %s; file %s\n", strerror(errno), ptask->filePath);
				break;
			} else if(feof(ifd)){
				break;
			}
		}
		fclose(ifd);
        
        if(ptask->sendLen != 0){
            ptask->starttime = time(NULL);
        }
        funcNotifyProjFinish(ptask->id);
	}
	free(databuf);
	return NULL;
}
/**
 * 10 threads used for sending data. 
 */
void send_data_parallel(int paranum)
{
	if(paranum <1){
		return;
	}
	Myqueue taskque;
	init_myqueue(&taskque);
	int count = 0;
	pthread_t curpid = pthread_self();
	if(paranum == 1){
		for(std::map<unsigned long long, ProjInfo>::iterator it= g_mId2Infos.begin(); it != g_mId2Infos.end(); it++){
			//LOGFMTI("<%u> send task[%d]  %s\n", curpid, count++, it->second.filePath);
			printf("<%u> send task[%d]  %s\n", curpid, count++, it->second.filePath);
			myqueue_put(&taskque, &it->second);
		}
		myqueue_putend(&taskque);
		send_data_second_timer_sub(&taskque);
	}
	else{
		pthread_t *allthds = (pthread_t*)malloc(paranum * sizeof(pthread_t));
		for(int i=0; i<paranum; i++){
			int crtret = pthread_create(&(allthds[i]), NULL, send_data_second_timer_sub, &taskque);
			if(crtret != 0){
				destroy_myqueue(&taskque);
				free(allthds);
				err_sys("error while creating pthread");
			}
		}
		for(std::map<unsigned long long, ProjInfo>::iterator it= g_mId2Infos.begin(); it != g_mId2Infos.end(); it++){
			//LOGFMTI("<%u> send task[%d]  %s\n", curpid, count++, it->second.filePath);
			printf("<%u> send task[%d]  %s\n", curpid, count++, it->second.filePath);
			myqueue_put(&taskque, &(it->second));
		}
		myqueue_putend(&taskque);
		
		for(int i=0; i<paranum; i++)
		{
			int err = pthread_join(allthds[i], NULL);
			if(err != 0){
				//LOGFMTI("fail to join thread: idx %d", i);
				printf("fail to join thread: idx %d", i);
			}
		}
		free(allthds);
	}
	destroy_myqueue(&taskque);
}

/**
 * 从模式*_<pid>_*中提取出节目名，当节目名唯一时，提取成功，否则，提取失败。
 */
bool parse_allpaths_pids(const vector<string>& allpaths, vector<unsigned long long>& accordpids)
{
	accordpids.clear();
	for(size_t idx=0; idx < allpaths.size(); idx++){
		string file = allpaths[idx];
		size_t fipl = file.find_first_of("_");
		if(fipl == string::npos) break;
		size_t sepl = file.find_first_of("_", fipl+1);
		if(sepl == string::npos) break;
		string strpid = file.substr(fipl + 1, sepl - fipl -1);
		unsigned long long ullvar;
		if(sscanf(strpid.c_str(), "%llu%", &ullvar) != 1) break;
		accordpids.push_back(ullvar);
	}
	if(accordpids.size() != allpaths.size()){
		accordpids.clear();
	}
	set<unsigned long long> pidsSet;
	for (unsigned u=0; u< accordpids.size(); u++){
		if(pidsSet.insert(accordpids[u]).second == false){
			accordpids.clear();
			break;
		}
	}
	return accordpids.size() > 0;
}
/**
 * 若首次执行该函数，就把rt目录下的wav文件读出；后面执行此函数，就把上次读出的删除并把新的文件代替，继续下一轮执行。
 * 继续与否可以通过与用户交互得知。
 *
 */
static size_t fillTaskInfosGlobal(const char *rt)
{
	g_mId2Infos.clear();
	vector<string> allwavs = get_file_list(rt);
	vector<unsigned long long> parsedpids;
	parse_allpaths_pids(allwavs, parsedpids);
	for(int i=0; i<allwavs.size(); i++){
		ProjInfo monkey;
		if(parsedpids.size() > 0) monkey.id = parsedpids[i];
        else monkey.id = i+1;
        snprintf(monkey.filePath, MAX_PATH, "%s%s", rt, allwavs[i].c_str());
		g_mId2Infos[monkey.id] = monkey;
	}
	return g_mId2Infos.size();
}

void fetch_all_options(int argc, char* argv[], char *opts, map<char,  string> &optbox, const char *szHelp)
{
	opterr = 1;
	char c;
	while( (c = getopt(argc, argv, opts)) != EOF){
		switch (c){
			case '?':
				fprintf(stderr,"%s\n", szHelp);
				exit(1);
				break;
			default:
				if(optarg == NULL) optbox[c] = "";
				else
					optbox[c] = optarg;
		}
	}
}

void saveLeftItems()
{
    unsigned culcnt = 0;
    for(map<unsigned long long, ProjInfo>::const_iterator it=g_mId2Infos.begin(); it!=g_mId2Infos.end(); it++){
        if(it->second.recatchtime == 0){
            culcnt++;
            string tmpStr = it->second.toString() + "\n";
            fwrite(tmpStr.c_str(), 1, tmpStr.size(), g_fpRes);
        }
    }

    //LOGFMTI("saveLeftItems save %u items having no result.\n", culcnt);
    printf("saveLeftItems save %u items having no result.\n", culcnt);
}
void interactWithMe()
{
    fprintf(stdout," before starting this round, please strike key s to stop, or continue after 3 secs:");
    char tmpstr[256];
    char *retfgets = fgetstimeout(tmpstr, 256, 3);
    if(retfgets != NULL && tmpstr[0] == 's'){
        fprintf(stdout, "you have stopped me, press c to restart:");
        while(1){
            fgets(tmpstr, 256, stdin);
            if(tmpstr[0] == 'c' && tmpstr[1] == '\n'){
                break;
            }
            fprintf(stdout, "you have stopped me, press c to restart:");
        }
    }
    else{
        fprintf(stdout, "\n");
    }
}
void interhandler(int num)
{
    fprintf(stderr, "handling interruption envent.\n");
    exit(1);
}
void initIoacas()
{
    const char *libPath = "ioacas/libIOACAS.so";
    void * hdl = dlopen(libPath, RTLD_NOW );
    if(hdl == NULL){
        fprintf(stderr, "ERROR fail to load library %s, error: %s.\n", libPath, dlerror());
        exit(1);
    }
    
    dlerror();
    funcInitDll = (FuncInitDll)dlsym(hdl, "InitDLL");
    char *errStr = dlerror();
    if(errStr != NULL){
        fprintf(stderr, "ERROR %s.\n", errStr);
        exit(1);
    }
    dlerror();
    funcSendData2Dll = (FuncSendData2DLL)dlsym(hdl, "SendData2DLL");
    errStr = dlerror();
    if(errStr != NULL){
        fprintf(stderr, "ERROR %s.\n", errStr);
        exit(1);
    }
    dlerror();
    funcCloseDll = (FuncCloseDll)dlsym(hdl, "CloseDLL");
    errStr = dlerror();
    if(errStr != NULL){
        fprintf(stderr, "ERROR %s.\n", errStr);
        exit(1);
    }

    dlerror();
    funcNotifyProjFinish = (FuncNotifyProjFinish)dlsym(hdl, "notifyProjFinish");
    errStr = dlerror();
    if(errStr != NULL){
        fprintf(stderr, "ERROR %s.\n", errStr);
        exit(1);
    }
    dlerror();
    funcIsAllFinished = (FuncIsAllFinished)dlsym(hdl, "isAllFinished");
    errStr = dlerror();
    if(errStr != NULL){
        fprintf(stderr, "ERROR %s.\n", errStr);
        exit(1);
    }

    g_Handler4Ioacas = hdl;
}
void relsIoacas()
{
    dlclose(g_Handler4Ioacas);
}
int main(int argc, char* argv[])
{
    initIoacas();
    typedef void (*sighandler_t) (int);
    sighandler_t rets = signal(SIGINT, interhandler);
    if(rets == SIG_ERR){
        fprintf(stderr, "error catch sigkill, err: %s.\n", strerror(errno));
    }
	if(atexit(saveLeftItems) != 0){
		fprintf(stderr, "error: fail to register exiting function savesendlist.\n");
	}
    const char *szHelp = " usage: program -d <wavedir> -p <threads> -k <result file>";
    map<char, string> allopts;
    fetch_all_options(argc, argv, "d:p:k:", allopts, szHelp);
    int parallelNum = 1;
    char sendInfoFile[MAX_PATH];
    sprintf(sendInfoFile, "%s", "result.txt");
    char rootDir[512];
    sprintf(rootDir, "TestWave/");
	if(allopts.find('d') != allopts.end()){
		sprintf(rootDir, "%s", allopts['d'].c_str());
        size_t rtlen = strlen(rootDir);
        if(rootDir[rtlen - 1] != '/'){
            rootDir[rtlen ++] = '/';
            rootDir[rtlen] = '\0';
        }
	}
	if(allopts.find('p')!= allopts.end()){
		parallelNum = atoi(allopts['p'].c_str());
	}
	if(allopts.find('k') != allopts.end()){
		sprintf(sendInfoFile, "%s", allopts['k'].c_str());
	}
    //LOGI("-----<<<<<>>>>>-----\nWaveDir: "<< rootDir<< "\nSendingThreadNum: "<< parallelNum<< "\nresultfile: "<< sendInfoFile);
    ostringstream oss;
    oss<<"-----<<<<<>>>>>-----\nWaveDir: "<< rootDir<< "\nSendingThreadNum: "<< parallelNum<< "\nresultfile: "<< sendInfoFile<< endl;
    printf(oss.str().c_str());

    g_fpRes = fopen(sendInfoFile, "w");
    if(g_fpRes == NULL){
        //LOGE("fail to open file to write result, file: "<< sendInfoFile);
        fprintf(stderr, "fail to open file to write result, file: %s.\n", sendInfoFile);
    }
    int iPriority = 0;
    int iThreadNum = 1;
    int *pThreadCPUID = NULL;
    int iDataUnitBufSize = 0;
    char workPath[MAX_PATH];
    unsigned int iModuleID = 0;
    int retInit = funcInitDll(iPriority, iThreadNum, pThreadCPUID, receive_result, iDataUnitBufSize, workPath, iModuleID);
    if(retInit != 0){
        //LOGE("failed in InitDll");
        fprintf(stderr, "failed in InitDll");
        exit(1);
    }

    int audionum = fillTaskInfosGlobal(rootDir);
    fprintf(stdout, "having read %d tasks.", audionum);
    interactWithMe();
    send_data_parallel(parallelNum);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    while(true){
        if(funcIsAllFinished()){
            cout<< "checked all tasks finishing in BufferGlobal.\n";
             break;   
        }
        tv.tv_sec = 10;
        select(0, NULL, NULL, NULL, &tv);
    }
	cout<< ">>>>>>>>>>>>>finish test<<<<<<<<<<<<<<<\n";
    funcCloseDll();
    relsIoacas();
	return 0;
}

/*
 *=================================================================
 *      Copyright (c) 2015 By ThinkIT Technologies Co Ltd
 *                       All Rights Reserved.
 *
 * FNAME: commonFunc.h
 * DESCP: <+Brief Descripation+>
 * AUTHR: Xiao Benfang,xiaobenfang@thinkit.cn
 * CREAT: 2015-10-08 17:34:14
 * WSITE: http://www.thinkit.com.cn
 * LC_AT: 2015-10-08 17:34:14
 * VERNO: 1.0.0
 * CNOTE: <+Latest Change+>
 *=================================================================
 */
# ifndef _COMMONFUNC_H
# define _COMMONFUNC_H

#include <string>
#include <vector>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <list>

#include <sys/ioctl.h>  
#include <net/if.h>  
#include <arpa/inet.h>  
#include <sys/stat.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <string>
#include "../include/interface242.h"

#include "log4z.h"
extern zsummer::log4z::ILog4zManager *g_Log4zManager;
extern LoggerId g_logger;
extern ConfigRoom g_AutoCfg;

#define MAX_PATH 512

#define PCM_ONESEC_LEN 16000
#define PCM_ONESEC_SMPS 8000
#define POSITIVE_PCM_LEN (PCM_ONESEC_LEN * 10)


char* GetLocalIP();
bool if_directory_exists(const char *dir, bool bForce = false);
int save_binary_data(const char *filePath, const void* ptr, size_t num, ...);

bool saveWave(char *pData, unsigned len, const char *saveFileName);
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

extern map<unsigned long,ProjRecord_t> NewReportedID;
extern pthread_mutex_t g_lockNewReported = PTHREAD_MUTEX_INITIALIZER;
//for dubug NewReportedID
inline void debugstring_newreported(const char * head)
{
	char tmpcz[500];
	sprintf(tmpcz, "%s, newreported [%lu] ", head, NewReportedID.size());
	string tmpStr = tmpcz;
	LOG_DEBUG(g_logger, tmpStr.c_str());
}

void maintain_newreported(time_t curtime, unsigned seconds);

# endif  

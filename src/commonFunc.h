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

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <string>
#include "../include/interface242.h"

#include "log4z.h"
extern zsummer::log4z::ILog4zManager *g_Log4zManager;
extern LoggerId g_logger;

#define MAX_PATH 512


typedef struct{
	float m_0Value;
	float  m_100Value;
	float m_maxValue;// in term of original score.
} ScoreConfig;
typedef int (*TransScore)(float);
TransScore getScoreFunc(ScoreConfig *);

/****************
 * 多个线程为生产者，多个线程为消费者，队列容量不设限。
 *
 *
 */
typedef struct Element_T{
    void *data;
    struct Element_T *next;
}E_T;

typedef struct Queue_T{
    E_T *head;
    pthread_mutex_t headMut; // for head and its pointed struct.
    pthread_cond_t headCnd; // for head changing form null to not-null.
    E_T *tail;
    pthread_mutex_t tailMut;//for tail and its pointed struct.
    pthread_cond_t tailCnd;
}MMQ_T;

#define MMQ_T_INITCOPY  {0,PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER}

/**
 * return 1 if success to retrieve data, or 0.
 */
int mmq_take(MMQ_T *pqu, void **ppdata);
int mmq_takew(MMQ_T *pqu, void **ppdata);

void mmq_put(MMQ_T *pqu, void *pdata);

unsigned mmq_size(MMQ_T *pqu);

# endif  

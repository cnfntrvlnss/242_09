/*************************************************************************
    > File Name: dllVAD_dup.cpp
    > Author: zhengshurui
    > Mail:  zhengshurui@thinkit.cn
    > Created Time: Fri 07 Oct 2016 06:33:13 PM PDT
 ************************************************************************/
#include<list>
#include <pthread.h>
#include <cstdio>

#include "dllVAD_dup.h"
using namespace std;

static list<VADHandle> g_AllHandles;
static pthread_mutex_t g_AllHandlesLock = PTHREAD_MUTEX_INITIALIZER;
static unsigned long g_ulAccumNum = 1;

VADHandle openOneVAD(char *cfgFile)
{
    pthread_mutex_lock(&g_AllHandlesLock);
    if(g_AllHandles.empty()){
        InitializeVADs(cfgFile);
    }

    VADHandle ret = (VADHandle)g_ulAccumNum++;
    g_AllHandles.push_back(VADHandle(ret));
    pthread_mutex_unlock(&g_AllHandlesLock);
    return ret;
}

void closeOneVAD(VADHandle hdl)
{
    pthread_mutex_lock(&g_AllHandlesLock);
    list<VADHandle>::iterator it = g_AllHandles.begin();
    while(it != g_AllHandles.end()){
        if(*it == hdl) break;
        it++;
    }
    if(it == g_AllHandles.end()){
        fprintf(stderr, "in freeVAD, called with invalid handle(%ul).", hdl);
    }
    else{
        g_AllHandles.erase(it);
    }
    if(g_AllHandles.empty()){
        FreeVADs();
    }
    pthread_mutex_unlock(&g_AllHandlesLock);

}


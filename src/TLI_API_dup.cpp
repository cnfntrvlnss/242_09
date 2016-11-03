/*************************************************************************
    > File Name: TLI_API_dup.cpp
    > Author: zhengshurui
    > Mail:  zhengshurui@thinkit.cn
    > Created Time: Sat 08 Oct 2016 03:19:25 AM PDT
 ************************************************************************/

#include <pthread.h>
#include <cctype>
#include <cstdio>
#include <cassert>
#include <cstdlib>

#include "TLI_API.h"

static void initTLI();
static void rlseTLI();
class LIDSpace4TLI{
    static LIDSpace4TLI* onlyone;
    static pthread_mutex_t onlyoneLock;

    LIDSpace4TLI(){
        initTLI();
    }
    ~LIDSpace4TLI(){
        rlseTLI();
    }
public:
    static void initLID(){
        pthread_mutex_lock(&onlyoneLock);
        if(onlyone == NULL){
            onlyone = new LIDSpace4TLI();
            if(onlyone == NULL){
                fprintf(stderr, "ERROR in LIDSpace4TLI::initLID, failed to create new instance of LIDSpace4TLI.\n");
            }
        }
        pthread_mutex_unlock(&onlyoneLock);

    }
};
LIDSpace4TLI* LIDSpace4TLI::onlyone = NULL;
pthread_mutex_t LIDSpace4TLI::onlyoneLock = PTHREAD_MUTEX_INITIALIZER;

int g_nTemplateNum=19;
int* pnAllTemplateIDs = NULL;
//int *g_pnTemplateIDs = NULL;
int g_SecondVAD=0;
int g_bUseDetector=0;
int g_MaxLIDLen = 3600;
int g_MinLIDLen = 5;
unsigned g_nThreadNum = 4;
const unsigned MAXLINE_LEN = 100;

unsigned countLinesInFile(const char *file)
{
    unsigned cnt = 0;   
    FILE *fp = fopen(file, "r");
    if(fp == NULL){
        fprintf(stderr, "cannot open file %s.\n", file);
        exit(1);
    }
    char szTemp[MAXLINE_LEN];
    while(!feof(fp)){
        char *stPtr = fgets(szTemp, MAXLINE_LEN, fp);
        if(stPtr == NULL) break;
        while(*stPtr != '\0'){
            if(isalnum(*stPtr))break;
            stPtr ++;
        }
        if(*stPtr != '\0') cnt ++;
    }
    return cnt;
}
void initTLI()
{
    g_nTemplateNum = countLinesInFile("ioacas/sysdir/param.txt");
    TLI_Init_addVAD_1(const_cast<char*>("ioacas/sysdir"), g_nTemplateNum, g_nThreadNum, g_MaxLIDLen, g_MinLIDLen, g_SecondVAD, g_bUseDetector);
}

void rlseTLI()
{
    TLI_Exit_1();  
}

int openTLI_dup()
{
    LIDSpace4TLI::initLID();
    TLI_HANDLE hret = -1;
    int err = TLI_Open_1(hret);
    if(hret < 0 || hret > g_nThreadNum){
        fprintf(stderr, "ERROR in openTLI_dup, fail to call TLI_Open_1. error: %d.\n", err);
    }

    return hret;
}

void closeTLI_dup(int hdl)
{
    TLI_Close_1(hdl);
}

void scoreTLI_dup(int hdl, short *pcmData, int pcmLen, int &resID, float &resScore)
{
    int langNum = g_nTemplateNum;
    int *arrTemplateIDs = pnAllTemplateIDs;
    int nMaxSpeechSec = 3600;
    int nMinSpeechSec = 5;
    resID = -1;
    resScore = -1000;
    int err = TLI_Recognize_1(hdl, arrTemplateIDs, langNum, (void*)pcmData, pcmLen * sizeof(short), nMinSpeechSec, nMaxSpeechSec, resID, resScore, "");
    if(err != 0){
        fprintf(stderr, "ERROR in scoreTLI_dup, failed to call TLI_Recognize_1. ret: %d.\n", err);
    }
}

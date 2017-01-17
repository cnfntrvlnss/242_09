/*************************************************************************
	> File Name: spk_fork.cpp
	> Author: 
	> Mail: 
	> Created Time: Sun 27 Nov 2016 03:57:50 AM EST
 ************************************************************************/

#include "spkEngine.h"
#include "dllSRVADCluster.h"
#include "MusicDetect.h"
#include <cstdlib>
#include<iostream>
#include <cstdio>
#include <cstring>
using namespace std;

#define OUTPUT_FORK(x) fprintf(stderr, #x " in %s.\n", __FILE__)
bool InitVADCluster(const char* CfgPath)
{
    OUTPUT_FORK(InitVADCluster);
    return true;
}
 bool VADBuffer(const bool bAllOut, const short* psPCMBuffer, const int iSampleNum, short* psPCMBufferVAD, int& riSampleNumVAD, bool bUseDetector)
{
    int wlen = iSampleNum < riSampleNumVAD ? iSampleNum : riSampleNumVAD;
    memcpy(psPCMBufferVAD, psPCMBuffer, wlen * sizeof(short));
    riSampleNumVAD = wlen;
    return true;
}
bool VADClusterBuffer(const short* psInWavData, const int iInSampleNum, short* psOutWavDataC1, int& riOutSampleNumC1, short* psOutWavDataC2, int& riOutSampleNumC2)
{
    OUTPUT_FORK(VADClusterBuffer);
    return false;
}
bool FreeVADCluster()
{
    OUTPUT_FORK(FreeVADCluster);
    return true;
}


#ifndef LIDFORK
bool MusicCut_Initial(char* CfgFile,int ThreadNum)
{
    OUTPUT_FORK(Musiccut_initial);
    return true;
}
bool MusicCut(int ThreadIndex,short* src,int iLen,short* &dst,int &oLen)
{
    int wlen = iLen < oLen ? iLen : oLen;
    memcpy(dst, src, wlen * sizeof(short));
    oLen = wlen;
    return true;
}
bool MusicCut_Free()
{
    OUTPUT_FORK(musiccut_free);
    return true;
}
#endif

TITStatus TIT_SPKID_INIT(const char * cfgfile)
{
    OUTPUT_FORK(tit_spkid_init);
    return StsNoError;
}

TITStatus TIT_SPKID_EXIT()
{
    OUTPUT_FORK(tit_spkid_exit);
    return StsNoError;
}

TITStatus TIT_SPKID_DEL_MDL(void *&mdl)
{
    OUTPUT_FORK(tit_spkid_del_mdl);
    free(mdl);
    return StsNoError;
}

TITStatus TIT_SPKID_VERIFY_CLUSTER(const short * wavbuf,const int wavlen,const void** spks,const int numspk,float* scores)
{
    OUTPUT_FORK(tit_spkid_verify_cluster);
    *scores = 100;
    return StsNotInit;
}
            
TITStatus TIT_SPKID_LOAD_MDL_IVEC(void *&mdl,const char *const modelpath)
{
    OUTPUT_FORK(tit_spkid_load_mdl_ivec);
    mdl = malloc(1);
    return StsNoError;
}
TITStatus TIT_SPKID_SAVE_MDL_IVEC(const void *const Ivec,const char *const modelpath)
{
    OUTPUT_FORK(tit_spkid_saved_mdl_ivec);
    return StsNoError;
}

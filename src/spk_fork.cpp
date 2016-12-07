/*************************************************************************
	> File Name: spk_fork.cpp
	> Author: 
	> Mail: 
	> Created Time: Sun 27 Nov 2016 03:57:50 AM EST
 ************************************************************************/

#include "interface.h"
#include "dllSRVADCluster.h"
#include "MusicDetect.h"
#include <cstdlib>
#include<iostream>
using namespace std;

bool InitVADCluster(const char* CfgPath)
{
    return true;
}
 bool VADBuffer(const bool bAllOut, const short* psPCMBuffer, const int iSampleNum, short* psPCMBufferVAD, int& riSampleNumVAD, bool bUseDetector)
{
    return false;
}
bool VADClusterBuffer(const short* psInWavData, const int iInSampleNum, short* psOutWavDataC1, int& riOutSampleNumC1, short* psOutWavDataC2, int& riOutSampleNumC2)
{
    return false;
}
bool FreeVADCluster()
{
    return true;
}


bool MusicCut_Initial(char* CfgFile,int ThreadNum)
{
    return true;
}
bool MusicCut(int ThreadIndex,short* src,int iLen,short* &dst,int &oLen)
{
    return false;
}
bool MusicCut_Free()
{
    return true;
}

TITStatus TIT_SPKID_INIT(const char * cfgfile)
{
    return StsNoError;
}

TITStatus TIT_SPKID_EXIT()
{
    return StsNoError;
}

TITStatus TIT_SPKID_DEL_MDL(void *&mdl)
{
    free(mdl);
    return StsNoError;
}

TITStatus TIT_SPKID_VERIFY_CLUSTER(const short * wavbuf,const int wavlen,const void** spks,const int numspk,float* scores)
{
    return StsNotInit;
}
            
TITStatus TIT_SPKID_LOAD_MDL_IVEC(void *&mdl,const char *const modelpath)
{
    mdl = malloc(1);
    return StsNoError;
}
TITStatus TIT_SPKID_SAVE_MDL_IVEC(const void *const Ivec,const char *const modelpath)
{
    return StsErrModel;
}

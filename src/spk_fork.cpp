/*************************************************************************
	> File Name: spk_fork.cpp
	> Author: 
	> Mail: 
	> Created Time: Sun 27 Nov 2016 03:57:50 AM EST
 ************************************************************************/

#include "interface.h"
#include "dllSRVADCluster.h"
#include <cstdlib>
#include<iostream>
using namespace std;

bool InitVADCluster(const char* CfgPath)
{
    return true;
}
bool VADClusterBuffer(const short* psInWavData, const int iInSampleNum, short* psOutWavDataC1, int& riOutSampleNumC1, short* psOutWavDataC2, int& riOutSampleNumC2)
{
    return false;
}
bool FreeVADCluster()
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

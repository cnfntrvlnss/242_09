/*************************************************************************
	> File Name: ioareg.h
	> Author: 
	> Mail: 
	> Created Time: Tue 22 Nov 2016 04:01:31 AM EST
 ************************************************************************/

#ifndef _IOAREG_H
#define _IOAREG_H


#include "dllVAD_dup.h"
#include "MusicDetect_dup.h"
#include "TLI_API_dup.h"
#include "../include/interface242.h"
#include "spk_ex.h"
#include "dllSRVADCluster.h"
#include "utilites.h"
#include "commonFunc.h"
#include "ProjectBuffer.h"

using namespace std;
//using namespace BufferGlobal;
using namespace zen4audio;

extern bool g_bSaveAfterRec;
bool ioareg_init(short threadNum);
void ioareg_updateConfig();
bool ioareg_rlse();
bool reportIoacasResult(CDLLResult &result, bool brep, char *writeLog, unsigned &len);
bool  gen_spk_save_file(char *savedname, const char *topDir, const char *subDir, unsigned long id, unsigned *type, unsigned *userId, int *confidence);

#endif

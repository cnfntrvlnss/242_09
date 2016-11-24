/*************************************************************************
	> File Name: libBAI_ex.h
	> Author: 
	> Mail: 
	> Created Time: Thu 09 Jun 2016 03:35:15 AM CST
 ************************************************************************/

#ifndef _LIBBAI_EX_H
#define _LIBBAI_EX_H

#include "commonFunc.h"
#include "../include/interface242.h"
#include <iostream>
#include <time.h>

extern float g_fReportBampThrd;
extern const unsigned short g_uBampFDServType;
extern const unsigned short g_uBampJCServType;
typedef void (*SummitBampResult)(struct timeval curtime, CDLLResult *pResult, std::ostream& oss);
bool bamp_init(SummitBampResult callbck);
bool bamp_rlse();
//bool bamp_match(unsigned long pid, short *pcmData, unsigned pcmLen, unsigned preLen, struct timeval curtime);
bool bamp_match(unsigned long pid, char *pcm1, unsigned len1, unsigned preLen, struct timeval curtime);
void *OnecircleOffline(void *param);

#endif

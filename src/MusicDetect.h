/* 
 * File:   MusicDetect.h
 * Author: huanghoujun
 *
 * Created on 2014年1月6日, 下午9:36
 */

#ifndef MUSICDETECT_H
#define	MUSICDETECT_H

#define MUSIC_DETECT_API

extern "C"
{
    MUSIC_DETECT_API bool MusicCut_Initial(char* CfgFile,int ThreadNum);
    MUSIC_DETECT_API bool MusicCut(int ThreadIndex,short* src,int iLen,short* &dst,int &oLen);
    MUSIC_DETECT_API bool MusicCut_Free();
}

#endif	/* MUSICDETECT_H */


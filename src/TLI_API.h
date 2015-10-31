#pragma pack( push, enter_tli_api )
#pragma pack ( 8 )

#ifndef __TLIAPI_HEADER_
#define __TLIAPI_HEADER_ 1

#ifndef _MT
#ifdef WIN32
#error("TLI needs multithreading")
#endif
#endif

#if defined( _WIN32 ) || defined ( _WIN64 )
#ifdef TLI_API_EXPORTS
#define TLI_DLL_API __declspec(dllexport)
#else
#define TLI_DLL_API __declspec(dllimport)
#endif
#else
#define TLI_DLL_API
#endif

/*// plu 2007.08.08_13:30:45
#define MAX_LANG_NUMBER		128
#define MAX_LINE_NUMBER		256
*/// plu 2007.08.08_13:30:45

typedef int TLI_HANDLE;

/*TLI_DLL_API int TLI_Init(
  char *pszTLISystemDirectory,
  int *pnAllTemplateIDs,
  char **ppszAllTemplateLocation,
  int nAllTemplateNumber,
  int nLineNumber
  );*/
TLI_DLL_API int LoadLangList(char *strLangList);

TLI_DLL_API int TLI_Init_addVAD_1(            //wxl add VADCfg Init
		char *pszTLISystemDirectory,
		//int *pnAllTemplateIDs,
		//char **ppszAllTemplateLocation,
		int &nAllTemplateNumber,
		int nLineNumber,
		int &g_nMaxSpeechSec,
		int &g_nMinSpeechSec,
		int &g_SecondVAD,
		int &g_bUseDetector
		);

TLI_DLL_API int TLI_Exit_1();

TLI_DLL_API int TLI_Open_1(
		TLI_HANDLE &hTLI
		);

TLI_DLL_API int TLI_Close_1(
		TLI_HANDLE hTLI
		);

// 2008.05.23 plu : 锟斤拷识锟斤拷锟斤拷锟斤拷锟斤拷锟饺碉拷锟斤拷锟斤拷锟斤拷锟狡匡拷锟斤拷锟节接匡拷锟斤拷

//TLI_DLL_API int TLI_Recognize(
//	TLI_HANDLE hTLI,
//	int *pnCurrentTemplateIDs,
//	int nCurrentTemplateNumber,
//	char *pszInPCMFileName,
//	int nMinLimit,
//	int nMaxLimit
//);

// 2008.05.23 plu : 锟斤拷识锟斤拷锟斤拷锟斤拷锟斤拷锟饺碉拷锟斤拷锟斤拷锟斤拷锟狡匡拷锟斤拷锟节接匡拷锟斤拷
/*
   TLI_DLL_API int TLI_Recognize(
   TLI_HANDLE hTLI,
   int *pnCurrentTemplateIDs,
   int nCurrentTemplateNumber,
   void *pvPCMBuffer,
   int nBytesOfBuffer,
   int nMinLimit,
   int nMaxLimit,
   char *pszInPCMFileName
   );
   */
TLI_DLL_API int TLI_Recognize_1(
		TLI_HANDLE hTLI,
		int *pnCurrentTemplateIDss,
		int nCurrentTemplateNumber,
		void *pvPCMBuffer,
		int nBytesOfBuffer,
		int nMinLimit,
		int nMaxLimit,
		int &ResID,
		float &ResScore,
		char *pszInPCMFileName/*,
								int g_SecondVAD, 
								int g_bUseDetector */
		);

TLI_DLL_API int TLI_GetResult_1(
		TLI_HANDLE hTLI,
		float *pfResults,
		int nNumber
		);

TLI_DLL_API bool LIDScore_1(short* wavdata, int g_LIDCutSize, int g_LIDCutStep, int g_nMaxSpeechSec, int g_nMinSpeechSec, int len,int &nMax,float &s, int &ResID, float &resScore);

//#define printf tprintf

#endif /* __TLIAPI_HEADER_ */

#pragma pack( pop, enter_tli_api )
/* ///////////////////////// End of file "TLI_API.h" //////////////////////// */

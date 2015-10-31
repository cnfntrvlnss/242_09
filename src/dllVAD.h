#ifndef DLLLRVAD_H
#define DLLLRVAD_H

//#ifdef DLLLRVAD_EXPORTS
//#define DLLLRVAD_API __declspec(dllexport)
//#else
//#define DLLLRVAD_API __declspec(dllimport)
//#endif
#define DLLLRVAD_API

extern "C"
{
	bool DLLLRVAD_API InitializeVADs( char* VADCfg);

	bool DLLLRVAD_API ExtractSpeechBuf(const short* psPCMBuffer, const int iSampleNumber, short*& psPCMBufferVAD, int& iSampleNumVAD);

	bool DLLLRVAD_API FreeVADs();
};
#endif
//-Wl, --retain - symbols - file = TLIVAD.sym - Wl, --version - script = TLIVAD.map


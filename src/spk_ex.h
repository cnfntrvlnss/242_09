/*************************************************************************
    > File Name: spk_ex.h
    > Author: zhengshurui
    > Mail:  zhengshurui@thinkit.cn
    > Created Time: Tue 11 Oct 2016 02:03:38 AM PDT
 ************************************************************************/

#ifndef SPK__EX_H
#define SPK__EX_H

#include <vector>
#include <string>
#include <typeinfo>

class SpkInfo{
public:
    SpkInfo(unsigned long param = 0):
        spkId(param)
    {}
    SpkInfo(const char* param){
        fromStr(param);
    }
    virtual ~SpkInfo(){
    }
    virtual std::string toStr() const;
    virtual bool fromStr(const char* );
    virtual bool operator==(const SpkInfo& oth) const{
        if(typeid(oth) != typeid(*this)) return false;
        return this->spkId == oth.spkId;
    }
public:
    unsigned long spkId;
};

void getAllSpkRec(std::vector<const SpkInfo*> &outSpks);
bool addSpkRec(const SpkInfo* spk, char* mdlData, unsigned mdlLen, const SpkInfo* &oldSpk);
const SpkInfo* rmSpkRec(const SpkInfo* spk);
bool initSpkRec(const char* cfgfile);
void rlseSpkRec();
bool processSpkRec(short* pcmData, unsigned smpNum, const SpkInfo* &spk, float &score);

#endif

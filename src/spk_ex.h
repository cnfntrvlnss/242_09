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

void spkex_getAllSpks(std::vector<const SpkInfo*> &outSpks);
bool spkex_addSpk(const SpkInfo* spk, char* mdlData, unsigned mdlLen, const SpkInfo* &oldSpk);
const SpkInfo* spkex_rmSpk(const SpkInfo* spk);
bool spkex_init(const char* cfgfile);
void spkex_rlse();
int spkex_score(short* pcmData, unsigned smpNum, const SpkInfo* &spk, float &score);

#endif

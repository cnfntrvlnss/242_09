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
    explicit SpkInfo(unsigned long param = 0):
        spkId(param)
    {}
    explicit SpkInfo(const char* param):
    {
        fromStr(param);
    }
    virtual ~SpkInfo(){
    }
    virtual std::string toStr() const;
    virtual bool fromStr(const char* );
    /*
    virtual bool operator==(const SpkInfo& oth) const{
        if(typeid(oth) != typeid(*this)) return false;
        return this->spkId == oth.spkId;
    }
    */
public:
    unsigned long spkId;
    short int refcnt;
};

//unsigned spkex_getAllSpks(std::vector<const SpkInfo*> &spks);
unsigned spkex_getAllSpks(std::vector<unsigned long> &spkIds);
const SpkInfo* getSpk(unsigned long);
void returnSpk(const SpkInfo*);
bool spkex_addSpk(SpkInfo* spk, char* mdlData, unsigned mdlLen);
//bool spkex_rmSpk(const SpkInfo* spk, const SpkInfo* oldSpk);
bool spkex_rmSpk(unsigned long spkId);
bool spkex_init(const char* cfgfile);
void spkex_rlse();
int spkex_score(short* pcmData, unsigned smpNum, const SpkInfo* &spk, float &score);

#endif

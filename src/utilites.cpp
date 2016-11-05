/*************************************************************************
    > File Name: utilites.cpp
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: Sun 01 Mar 2015 05:37:15 PM PST
 ************************************************************************/

#include <stdarg.h>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "utilites.h"
#include <algorithm>
#include <string>
#include <map>

using namespace std;
#define MAX_PATH 512
#define MAX_LINE 1024

/**
 * 分割字符串，用通用的默认形式，即，空格类字符作为分割符。
 */
vector<string> split(const string& s)
{
	vector<string> results;
	int st = -1;
	for(int i=0; i<s.size(); i++) {
		if(isspace(s[i]) && st != -1){
			results.push_back(s.substr(st, i-st));
			st = -1;
		}
		else if(!isspace(s[i]) && st == -1){
			st = i;
		}
	}
	if(st != -1) {
		results.push_back(s.substr(st, s.size()-st));
	}
	return results;
}
/**
 *根据delim子串分割字符串s, 若keep_empty为true, 返回的结果中就保留空字符串；反之，亦然。
 */
vector<string> split(const string& s, const string& delim,
		const bool keep_empty )
{
	vector<string> result;
	if (delim.empty())
	{
		result.push_back(s);
		return result;
	}
	string::const_iterator substart = s.begin(), subend;
	while (true)
	{
		subend = search(substart, s.end(), delim.begin(), delim.end());
		string temp(substart, subend);
		if (keep_empty || !temp.empty())
		{
			result.push_back(temp);
		}
		if (subend == s.end())
		{
			break;
		}
		substart = subend + delim.size();
	}
	return result;
}

vector<string> loadFileList(const char *listfile)
{
    char strline[512];
    vector<string> ret;
    FILE *fp = fopen(listfile, "r");
    if(fp == NULL){
        return ret;
    }
    while(fgets(strline, 512, fp)){
        int lastidx = strlen(strline) - 1;
        while (lastidx >=0 && strchr("\t\r \n",strline[lastidx]) != NULL){
            strline[lastidx -- ] = '\0';
        }
        if(lastidx > 0){
            ret.push_back(strline);
        }
    }
    fclose(fp);
    return ret;
}

bool make_directorys(const char *mypath)
{
	if(access(mypath, F_OK) == 0) return true;
	char firstPath[MAX_PATH];
	char fileName[MAX_PATH];
	strncpy(firstPath, mypath, MAX_PATH);
	unsigned pathLen = strlen(firstPath);
	if(firstPath[pathLen -1] == '/'){
		firstPath[pathLen - 1] = '\0';
	}
	char *pSl = strrchr(firstPath, '/');
	strcpy(fileName, pSl + 1);
	*(pSl + 1) = '\0';
	make_directorys(firstPath);
	if(mkdir(mypath, 0775) == 0) {
		return true;
	}
	return false;
}

unsigned  procFilesInDir(const char* szDir, FuncProcessFile addr)
{
    DIR *dp;
    struct dirent *dirp;
    struct stat statbuf;
    int cnt = 0;

    if((dp = opendir(szDir)) == NULL) return 0;
    while((dirp = readdir(dp)) != NULL){
        if(strcmp(dirp->d_name, ".") == 0 ||
                strcmp(dirp->d_name, "..") == 0) continue;
        string tmpPath = concatePath(szDir, dirp->d_name);
        if(stat(tmpPath.c_str(), &statbuf) < 0) continue;
        if(S_ISREG(statbuf.st_mode)) {
            if(addr(szDir, dirp->d_name)) cnt++;
        }
    }
    return cnt;
}


union MyConfigItemValue{
	bool *bvar;
	int *ivar;
	char *svar;
	float *fvar;
};
struct MyConfigItem{
	const char *name;
	char type;
	MyConfigItemValue pvalue;
};
/*****************************************************
 *可变参数部分为 const char*, type[*],...;即，两个为一组，直到最后的NULL参数为止。
 * 含义是: 参数[1]的开头的字母为B/S/I/F，对应的后面的一个参数的type为bool/char* /int/float, 若为char star,*是多余的。
 *每组里面的第三个参数存储返回值，若配置文件中存在第一个参数指定的配置项，就存储解析出的值
 *****************************************************/
int parse_params_from_file(const char *fileName, ...)
{
	FILE *fp = fopen(fileName, "r");
    if(fp== NULL){
        return 0;
    }
	MyConfigItem *confarr = NULL;
	int count = 100;
	confarr = (MyConfigItem*)malloc(sizeof(MyConfigItem) * count);
	va_list valist;
	va_start(valist, fileName);
	int idx = 0;
	bool isfinish = false;
	for(; idx<count; idx++)
	{
		confarr[idx].name = va_arg(valist, char*);
		if(confarr[idx].name == NULL) break;
		confarr[idx].type = confarr[idx].name[0];
		confarr[idx].name ++;
		switch (confarr[idx].type){
			case 'B': confarr[idx].pvalue.bvar = va_arg(valist, bool*);   break;
			case 'S': confarr[idx].pvalue.svar = va_arg(valist, char*);  break;
			case 'I': confarr[idx].pvalue.ivar = va_arg(valist, int*);  break;
			case 'F': confarr[idx].pvalue.fvar = va_arg(valist, float*);  break;
			default: isfinish = true; break;
		}
		if(isfinish) break;
	}
	va_end(valist);
	if(idx == 0) {
		free(confarr);
		return 0;
	}
	
	char szsect[MAX_PATH];
	char szname[MAX_PATH];
	char szvalue[MAX_PATH];
	char szline[1024];
	szsect[0] = 0;
	while(!feof(fp))
	{
        szline[0] = '\0';
        fgets(szline, 1024, fp);
        if('\0' == szline[0] || '\n' == szline[0] || '#' == szline[0])
        {
            continue;
        }
		int ispair = false;
        for(int ii = 0; ii < strlen(szline); ii++)
        {
            if('=' == szline[ii])
            {
				ispair = true;
                szline[ii] = ' ';
            }
			else if('#' == szline[ii]){
                szline[ii] = 0;
				break;
			}
        }
		if(ispair == false){
			sscanf(szline, "[%s]", szsect);
		}
		else {
			int retsc = sscanf(szline, "%s %s", szname, szvalue);
			if(retsc < 2){ continue; }
			char dotedname[MAX_PATH];
			sprintf(dotedname, "%s.%s", szsect, szname);
			for(int i=0; i<idx; i++){
				if(!strcmp(confarr[i].name, szname) || !strcmp(confarr[i].name, dotedname)){
					switch (confarr[i].type){
						case 'B':
							if(!strcmp(szvalue, "0")){
								*(confarr[i].pvalue.bvar) = false; break;
							}
							if(!strcmp(szvalue, "1")){
								*(confarr[i].pvalue.bvar) = true; break;
							}
							for(int ii=0; ii< strlen(szvalue); ii++){
								if(szvalue[ii] < 'a') szvalue[ii] += 32;
							}
							if(!strcmp(szvalue, "true")){
								*(confarr[i].pvalue.bvar) = true; break;
							}
							if(!strcmp(szvalue, "false")){
								*(confarr[i].pvalue.bvar) = false; break;
							}
							break;
						case 'S':
							strcpy(confarr[i].pvalue.svar, szvalue);break;
						case 'I':
							*(confarr[i].pvalue.ivar) = atoi(szvalue); break;
						case 'F':
							*(confarr[i].pvalue.fvar) = atof(szvalue); break;
						default: assert(false);
					}
					break;
				}
			}
		}

	}
    free(confarr);
	return idx;
}

/**
 * configuration comes form file. also be synchronized with that in file.
 *
 */
typedef void (*UseConfigValue)(const char* key, const char* value);
struct ConfigRoom{
    struct StringPair{
        string used;
        string current;
    };
    map<string, StringPair> allconfigs;
    string configFile;
    time_t lastReadTime;

    bool loadFromFile(const char* filePath = NULL);
    bool isUpdated(const char* key);
    bool accessValue(const char* key, string& value);
    bool accessvalue(const char* key, UseConfigValue* funcAddr);
};

ConfigRoom g_Configs;

static char* getValidString(char *tmpStr)
{
    if(tmpStr[0] == '\0') return NULL;
    unsigned tmplen = strlen(tmpStr);
    char* lastPtr = tmpStr + tmplen - 1;
    while(lastPtr != tmpStr && isspace(*lastPtr)){
        lastPtr --;
    }
    if(lastPtr == tmpStr){
        if(isspace(*lastPtr)) return NULL;
        else {
            *(lastPtr + 1) = '\0';
            return tmpStr;
        }
    }
    *(lastPtr + 1) = '\0';
    
    while(isspace(*tmpStr)) tmpStr --;
    return tmpStr;
}

bool ConfigRoom::loadFromFile(const char* filePath)
{
    if(filePath == NULL && configFile.size() == 0){
        fprintf(stderr, "ERROR no config file specified.\n");
        return false;
    }
    if(configFile != filePath){
        fprintf(stderr, "WARN the current configure file is not used, for being not the same file with initial file. initial: %s, current: %s.\n", configFile.c_str(), filePath);
    }
    FILE *fp = fopen(configFile.c_str(), "r");
    if(fp == NULL){
        fprintf(stderr, "ERROR the configure file is not opened. file: %s.\n", configFile.c_str());
        return false;
    }
    char groupName[MAX_PATH];
    groupName[0] = '\0';
    char tmpLine[MAX_LINE];
    while(fgets(tmpLine, MAX_LINE, fp) != NULL){
        char *validStr = getValidString(tmpLine);
        if(validStr == NULL) return NULL;
        char *comSt = strchr(validStr, '#');
        if(comSt != NULL) *comSt = '\0';
        if(*validStr == '['){
            char *closePairPtr = strrchr(validStr, ']');
            if(closePairPtr == NULL){
                fprintf(stderr, "ERROR the line in configure file is illegal. line: %s.", validStr);
                continue;
            }
            *closePairPtr = '\0';
            char * validStr = getValidString(validStr + 1);
            if(validStr == NULL) continue;
            strncpy(groupName, validStr, MAX_PATH);
        }
    
    }
}



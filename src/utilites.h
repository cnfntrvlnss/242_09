/*************************************************************************
    > File Name: utilites.h
    > Author: shurui
    > Mail: 
    > Created Time: Tue 03 Feb 2015 11:33:29 AM CST
 ************************************************************************/

#include <cassert>
#include <cstring>
#include <sstream>
#include <vector>
#include <string>
#include<iostream>
using namespace std;
using namespace std;

#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
//! LockHelper
//////////////////////////////////////////////////////////////////////////
class LockHelper
{
public:
    LockHelper();
    virtual ~LockHelper();

public:
    void lock();
    void unLock();
//private:
#ifdef WIN32
    CRITICAL_SECTION _crit;
#else
    pthread_mutex_t  _crit;
#endif
};

//////////////////////////////////////////////////////////////////////////
//! AutoLock
//////////////////////////////////////////////////////////////////////////
class AutoLock
{
public:
    explicit AutoLock(LockHelper & lk):_lock(lk){_lock.lock();}
    ~AutoLock(){_lock.unLock();}
private:
    LockHelper & _lock;
};

#define MAX_PATH 512
/** 用于debug输出数组数组变量。
 *
 */
template <typename T>
std::string arr2str(T *arr, int size){
	assert(arr != 0);
	std::stringstream ss;
	ss << "[";
	if( size > 0){
		ss << arr[0];
	}
	for(int i=1; i<size; i++){
		ss << ',' << arr[i];	
	}
	ss << "]";
	return ss.str();
}

inline std::string int2str(int i){
	char temp[100];
	sprintf(temp, "%d", i);
	return std::string(temp);
}
vector<string> split(const string& s);
vector<string> split(const string& s, const string& delim,
		const bool keep_empty = true);

inline std::string concatePath(const char* path, const char* name)
{
    char wpath[MAX_PATH];
    unsigned plen = snprintf(wpath, MAX_PATH, "%s", path);
    if(wpath[plen - 1] != '/'){
        wpath[plen - 1] = '/';
        plen += 1;
    }

    plen += snprintf(wpath + plen, MAX_PATH - plen, "%s", name);
    return wpath;
}

inline std::string getBasename(const char* path)
{
    const char* psep = strrchr(path, '/');   
    if(psep != NULL) path = psep + 1;
    psep = strrchr(path, '.');
    if(psep == NULL) psep += strlen(path);
    return std::string(path, psep);
}

vector<string> loadFileList(const char *listfile);
int parse_params_from_file(const char *fileName, ...);
bool make_directorys(const char *mypath);

typedef bool (*FuncProcessFile)(const char*, const char*);
unsigned procFilesInDir(const char* szDir, FuncProcessFile addr);


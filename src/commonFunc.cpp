#include "porting.h"
#include "commonFunc.h"


zsummer::log4z::ILog4zManager* initLog4z()
{
    static const char*myLogCfg="\
        [main]\n\
        path=./ioacas/log/\n\
        [ioacas]\n\
        path=./ioacas/log/\n\
        level=ALL\n\
        display=true\n\
        monthdir=false\n\
        fileline=false\n\
        enable=true\n\
        outfile=true\n\
        [globalBuffer]\n\
        path=./ioacas/log/\n\
        level=ALL\n\
        display=true\n\
        monthdir=false\n\
        fileline=false\n\
        enable=true\n\
        outfile=true\n\
        ";
    zsummer::log4z::ILog4zManager* tmpPtr = zsummer::log4z::ILog4zManager::getPtr();
    tmpPtr->configFromString(myLogCfg);   
    tmpPtr->config("ioacas/log4z.ini");
    tmpPtr->setAutoUpdate(6);
    tmpPtr->start();
    return tmpPtr;
}

zsummer::log4z::ILog4zManager *g_Log4zManager = initLog4z();

char* GetLocalIP()    
{          
	int MAXINTERFACES=16;    
    static char retIP[50];
    if(retIP[0] != '\0') return retIP;
	const char *ip = "127.0.0.1";    
	int fd, intrface;      
	struct ifreq buf[MAXINTERFACES];      
	struct ifconf ifc;      
	int thrdNum = 0;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)      
	{      
		ifc.ifc_len = sizeof(buf);      
		ifc.ifc_buf = (caddr_t)buf;      
		if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc))      
		{      
			intrface = ifc.ifc_len / sizeof(struct ifreq);      

			while (intrface-- > 0)      
			{      
				if (!(ioctl (fd, SIOCGIFADDR, (char *) &buf[intrface])))      
				{      
					ip=(inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));      
					//fetch management ip. determined by wheath the third part is even.
					if(strcmp(ip, "127.0.0.1") == 0) continue;
					if(sscanf(ip, "%*d.%*d.%d.%*d", &thrdNum) != 1){
						continue;
					}
                    if(thrdNum %2 == 0){
                      strncpy(retIP, ip, 50);
                      break;       
                    }
				}                          
			}    
		}      
		close (fd);
	}  
	return retIP; 
} 

bool if_directory_exists(const char *dir, bool bForce = false)
{
	struct stat buf;
	if(stat(dir, &buf)<0 || !S_ISDIR(buf.st_mode)){
		//试着创建目录.
		if(bForce && mkdir(dir, S_IRWXU | S_IRWXG | S_IRWXO)!= -1){
			return true;
		}
		return false;
	}
	return true;
}

int save_binary_data(const char *filePath, const void* ptr, size_t num, ...)
{
    FILE *fp = fopen(filePath, "wb");
    if(fp == NULL){
        LOG_WARN(g_logger, "fail to open file to write data, file: "<< filePath);
        return -1;
    }

    int ret = 0;
    va_list vl;
    va_start(vl, num);
    const void *curPtr = ptr;
    size_t curNum = num;
    while(true){
        int retwr = fwrite(curPtr, 1, curNum, fp);
        ret += retwr;
        if(retwr != curNum){
            LOG_WARN(g_logger, "fail to write data to file, file: " << filePath);
            break;
        }
        curPtr = va_arg(vl, void*);
        if(curPtr == NULL) break;
        curNum = va_arg(vl, size_t);
    }
    
    fclose(fp);
    return ret;
}

bool saveWave(char *pData, unsigned len, const char *saveFileName)
{
    FILE *fp = fopen(saveFileName, "ab");
    if(fp == NULL){
        LOG_WARN(g_logger, "fail to open file "<< saveFileName<< " error: "<< strerror(errno));
        return false;
    }
    bool ret = false;
    PCM_HEADER pcmheader;
    initialize_wave_header(&pcmheader, len);
    int retw = fwrite(&pcmheader, sizeof(PCM_HEADER), 1, fp);
    if(retw != 1){
        LOG_WARN(g_logger, "fail to save data to file, filename: "<< saveFileName);
    }
    else{
        fwrite(pData, 1, len, fp);
        ret = true;
    }
    fclose(fp);
    return ret;
}

/**
 * one maintain operation for clearup routine.
 * clear up records before <seconds> seconds go.
 * this function  should be confined in lock of g_lockNewReported.
 */
void maintain_newreported(time_t curtime, unsigned seconds)
{
	debugstring_newreported("before maintance");
	vector<unsigned long> delID;
	for(map<unsigned long,ProjRecord_t>::iterator IDs = NewReportedID.begin();IDs !=NewReportedID.end();++IDs)
	{
		if(curtime - (*IDs).second.timemark >= seconds)
			delID.push_back((*IDs).first);
	}
	for(unsigned int tt = 0;tt < delID.size();++tt)
		NewReportedID.erase(delID[tt]);
	debugstring_newreported("after maintance");
}


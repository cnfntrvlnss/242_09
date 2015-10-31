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

static ScoreConfig g_cfg;
static char g_init = 0;
static int trans_score(float f)
{
	if(f > g_cfg.m_maxValue) f = g_cfg.m_maxValue;
	return (f - g_cfg.m_0Value) * ((100) / (g_cfg.m_100Value - g_cfg.m_0Value));
}
TransScore getScoreFunc(ScoreConfig *param)
{
	if(param == NULL && g_init == 0){
		g_cfg.m_0Value = 0.0;
		g_cfg.m_100Value = 100.0;
		g_cfg.m_maxValue = 100.0;
	}
	else if(param != NULL){
		g_init = 1;
		g_cfg = *param;
	}
	return trans_score;
}

////////////////////////////////////////////////
unsigned mmq_size(MMQ_T *pqu)
{
    unsigned cnt = 0;
    pthread_mutex_lock(&pqu->headMut);
    if(pqu->head != 0){
        pthread_mutex_lock(&pqu->tailMut);
        E_T *curNode = pqu->head;
        while(curNode != pqu->tail){
            cnt ++; 
            curNode = curNode->next;
		}
		cnt ++;
		pthread_mutex_unlock(&pqu->tailMut);
	}
    pthread_mutex_unlock(&pqu->headMut);
    return cnt;
}

void mmq_put(MMQ_T *pqu, void *pdata)
{
    E_T *ele = (E_T*)malloc(sizeof(E_T));
    ele->data = pdata;
    ele->next = 0;
    pthread_mutex_lock(&pqu->headMut);
    pthread_mutex_lock(&pqu->tailMut);
    if(pqu->head == 0){
        pqu->head = pqu->tail = ele;
        pthread_mutex_unlock(&pqu->headMut);
        pthread_mutex_unlock(&pqu->tailMut);
        pthread_cond_broadcast(&pqu->headCnd);
        return;
    }
		    
    pthread_mutex_unlock(&pqu->headMut);
    pqu->tail->next = ele;
    pqu->tail = ele;
    pthread_mutex_unlock(&pqu->tailMut);
}

static void mmq_take_unlocked(MMQ_T *pqu, void **ppdata)
{
    E_T *ele = 0;
    pthread_mutex_lock(&pqu->tailMut);
    if(pqu->head == pqu->tail){
        ele = pqu->head;
        pqu->head = pqu->tail = 0;
    }
    pthread_mutex_unlock(&pqu->tailMut);
    if(ele != 0){
        *ppdata = ele->data;
        free(ele);
        return ;
    }
	    
    ele = pqu->head;
    pqu->head = ele->next;
    *ppdata = ele->data;
    free(ele);
    return ;
}

/**
 *  * instant version.
 *   */
int mmq_take(MMQ_T *pqu, void **ppdata)
{
	 pthread_mutex_lock(&pqu->headMut);
	 if(pqu->head == 0){
	     pthread_mutex_unlock(&pqu->headMut);
	     return 0;
	 }
	 mmq_take_unlocked(pqu, ppdata);
	 pthread_mutex_unlock(&pqu->headMut);
	 return 1;
}

static void unlock_mutex(void *plock){
	    pthread_mutex_unlock((pthread_mutex_t *)plock);
}
/***
 *  * wait version 
 *   */
int mmq_takew(MMQ_T *pqu, void **ppdata)
{
	pthread_mutex_lock(&pqu->headMut);
	pthread_cleanup_push(unlock_mutex, &pqu->headMut);
	while(pqu->head == 0){
	   int retw = pthread_cond_wait(&pqu->headCnd, &pqu->headMut);
	   if(retw != 0){
	       fprintf(stderr, "error occurs in function pthread_cond_wait.\n");
	       pthread_mutex_unlock(&pqu->headMut);
	       pthread_exit((void*)1); 
	   }
	}
    mmq_take_unlocked(pqu, ppdata);
    pthread_cleanup_pop(1);
    return 1;
}


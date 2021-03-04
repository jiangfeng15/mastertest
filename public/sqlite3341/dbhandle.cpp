#include "dbhandle.h"

dbHandle::dbHandle()
{
    pthread_mutex_init(&m_mutex, nullptr);
}

dbHandle::~dbHandle()
{
    pthread_mutex_destroy(&m_mutex);
}

std::string dbHandle::GetCurrentTime()
{
    std::string strTime;
    char cTime[100]={0x00};
    struct timeval curTimeval;
    struct timezone curTimezone;
    gettimeofday(&curTimeval, &curTimezone);

    struct tm *pCurTime;
    pCurTime = localtime(&curTimeval.tv_sec);
    sprintf(cTime, "%4d-%02d-%02d %02d:%02d:%02d", 1900+pCurTime->tm_year,pCurTime->tm_mon+1,pCurTime->tm_mday,pCurTime->tm_hour,pCurTime->tm_min,pCurTime->tm_sec);
    strTime = cTime;
}
bool dbHandle::OpenAstDB(char * dbPath)
{
    bool bRet = true;
    if(m_sqlite_wrapper.Open(dbPath))
    {
        m_bDBConnect = true;
        m_strDBPath = dbPath;
    }
    else
    {
        bRet = false;
        m_bDBConnect = false;
        m_strErrInfo = m_sqlite_wrapper.LastError();
    }
    return bRet;
}

bool dbHandle::CloseAstDB()
{
    return m_sqlite_wrapper.Close();
}

bool dbHandle::LockDB()
{
    bool bRet = true;
    pthread_mutex_lock(&m_mutex);
    return bRet;
}

bool dbHandle::unLockDB()
{
    bool bRet = true;
    pthread_mutex_unlock(&m_mutex);
    return bRet;
}

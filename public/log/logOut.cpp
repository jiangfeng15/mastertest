#include "logOut.h"

static const char *ASS_LOG_LEVEL[]={"DEBUG","INFO","NOTICE","WARNING","ERR","CRIT","ALERT","EMERG"};

CAstLogOut::CAstLogOut()
{
    m_nSize=10*1024*1024;
    m_bLock=false;
    m_fp=NULL;
    m_nMaxLevel=7;
    m_nLevel=0;
}

CAstLogOut::~CAstLogOut()
{
    if(m_fp!=NULL)
    {
        char cTemp[256]={0x00};
        sprintf(cTemp, "%s log exit.", m_cFileName);
        WriteLog(cTemp, 0);
        fclose(m_fp);
        m_fp = NULL;
    }
}
int CAstLogOut::CreateFile(char * cFileName)
{
    char cExecutePath[MAX_PATH]={0x00};
    getcwd(cExecutePath, MAX_PATH);
    sprintf(m_cFileName, "%s/%s",cExecutePath, cFileName);
    strncpy(m_cFileBase, cExecutePath, strlen(cExecutePath));
    m_fp = fopen(m_cFileName, "at");
    if(m_fp == NULL)
        return 1;
    setbuf(m_fp, NULL);  //log有时候不写文件
    return 0;
}
void CAstLogOut::SetLogLevel(int nLevel)
{
    if(nLevel >=0 && nLevel<=m_nMaxLevel)
        m_nLevel = nLevel;
    else
        m_nLevel = 0;
    return;
}
int CAstLogOut::WriteLog(char *cLog, int nLevel)
{
    if (nLevel < m_nLevel)
        return 0;
    if(m_fp == NULL)
    {
        m_fp = fopen(m_cFileName, "at");
        if(m_fp == NULL)
            return 1;
        setbuf(m_fp, NULL);
    }
    if(nLevel < 0 || nLevel >= 8)
        nLevel = 7;
    if (Lock() == 1)
        return 1;
    fseek(m_fp, 0, SEEK_END);

    struct timeval curTimeval;
    struct timezone curTimezone;
    gettimeofday(&curTimeval, &curTimezone);

    struct tm *pCurTime;
    pCurTime = localtime(&curTimeval.tv_sec);
    int nLenWrite;

    char *cLogBuf = new char[strlen(cLog)+1];
    memset(cLogBuf, 0x00, strlen(cLog)+1);
    memcpy(cLogBuf, cLog, strlen(cLog));
    char * pUser = NULL, * pWord=NULL;
    //判断日志中是否有密码，如果有密码则隐藏
    while(strstr(cLogBuf, "username") && strstr(cLogBuf, "password"))
    {
        pUser = strstr(cLogBuf, "username");
        while (*(pUser+1) != ':')
        {
            *pUser = 'U';
            pUser++;
        }
        pUser++;
        pUser++;
        pUser++;
        while (*pUser != '"')
        {
            *pUser = 'X';
            pUser++;
        }

        pWord = strstr(cLogBuf, "password");
        while (*(pWord+1) != ':')
        {
            *pWord = 'U';
            pWord++;
        }
        pWord++;
        pWord++;
        pWord++;
        while (*pWord != '"')
        {
            *pWord = 'X';
            pWord++;
        }
    }
    nLenWrite = fprintf(m_fp, "%4d-%02d-%02d %02d:%02d:%02d.%03d\t[%s]\t%s\n", 1900+pCurTime->tm_year,pCurTime->tm_mon,pCurTime->tm_mday,pCurTime->tm_hour,pCurTime->tm_min,pCurTime->tm_sec,curTimeval.tv_usec,
        ASS_LOG_LEVEL[nLevel], cLogBuf);
    fflush(m_fp);

    //释放申请空间
    delete cLogBuf;
    cLogBuf = NULL;

    fseek(m_fp, 0, SEEK_END);
    int nSize = ftell(m_fp);
    int nReturn = 0;
    if(nSize >= m_nSize)
    {
        fclose(m_fp);
        char newname[280];
        sprintf(newname, "%s_%4d%02d%02d%02d%02d%02d%03d.log", m_cFileBase,
            1900+pCurTime->tm_year,pCurTime->tm_mon,pCurTime->tm_mday,pCurTime->tm_hour,pCurTime->tm_min,pCurTime->tm_sec,curTimeval.tv_usec);
        rename(m_cFileName, newname);
        m_fp = fopen(m_cFileName, "at");
        if(m_fp == NULL)
            nReturn = 2;
        else
            setbuf(m_fp, NULL);  //log有时候不写文件

    }
    UnLock();
}
int CAstLogOut::WriteHexLog(unsigned char * cLog, int nLen, int nLevel)
{
    if(nLevel < m_nLevel)
        return 0;
    if(m_fp == NULL)
    {
        m_fp = fopen(m_cFileName, "at");
        if(m_fp == NULL)
            return 1;
        setbuf(m_fp, NULL);  //log有时候不写文件
    }
    if(nLevel <0 || nLevel >= 8)
        nLevel = 7;
    if(Lock() == 1)
        return 2;

    fseek(m_fp, 0, SEEK_END);

    struct timeval curTimeval;
    struct timezone curTimezone;
    gettimeofday(&curTimeval, &curTimezone);

    struct tm *pCurTime;
    pCurTime = localtime(&curTimeval.tv_sec);

    fprintf(m_fp, "%4d-%02d-%02d %02d:%02d:%02d.%03d\t[%s]\t", 1900+pCurTime->tm_year,pCurTime->tm_mon,pCurTime->tm_mday,pCurTime->tm_hour,pCurTime->tm_min,pCurTime->tm_sec,curTimeval.tv_usec,
        ASS_LOG_LEVEL[nLevel]);
    for(int i=0; i<nLen; i++)
        fprintf(m_fp, "%02X", cLog[i]);
    fprintf(m_fp, "\n");
    fflush(m_fp);
    fseek(m_fp, 0, SEEK_END);
    int nSize = ftell(m_fp);
    int nReturn = 0;
    if(nSize >= m_nSize)
    {
        //fprintf_s(m_fp,"size=%d\n", nSize);
        fclose(m_fp);
        char newname[280];
        sprintf(newname, "%s_%4d%02d%02d%02d%02d%02d%03d.log", m_cFileBase,
            1900+pCurTime->tm_year,pCurTime->tm_mon,pCurTime->tm_mday,pCurTime->tm_hour,pCurTime->tm_min,pCurTime->tm_sec,curTimeval.tv_usec);
        rename(m_cFileName, newname);
        m_fp = fopen(m_cFileName, "at");
        if(m_fp == NULL)
            nReturn = 3;
        else
            setbuf(m_fp, NULL);  //log有时候不写文件
    }
    UnLock();
}
int CAstLogOut::Lock()
{
    for(int i = 0; i < 50; i++)
    {
        if(!m_bLock)
        {
            m_bLock = true;
            return 0;
        }
        sleep(100);
    }
    return 1;
}
void CAstLogOut::UnLock()
{
    m_bLock = false;
}

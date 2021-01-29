#ifndef  LOG_OUT_H
#define  LOG_OUT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#define MAX_PATH 255
class CAstLogOut
{
public:
    CAstLogOut();
    ~CAstLogOut();
    /**
     * @brief CreateFile
     * @param cFileName 传入文件名，初始化m_cFileName,为文件名对应的绝对路径，并创建文件
     * @return 0 成功
     */
    int CreateFile(char * cFileName);
    void SetLogLevel(int nLevel);
    int WriteLog(char *cLog, int nLevel);
    int WriteHexLog(unsigned char * cLog, int nLen, int nLevel);
    int Lock();
    void UnLock();
public:
    char m_cFileName[256];
    char m_cFileBase[256];
    int  m_nLevel;
    int  m_nMaxLevel;
private:
    FILE * m_fp;
    int m_nSize;
    bool m_bLock;
};

#endif

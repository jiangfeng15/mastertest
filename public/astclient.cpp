#ifdef WIN32
#include <Winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include "zsocket.h"
#include "astclient.h"

#ifdef HPUNX
#define CLOCK_TYPE CLOCK_REALTIME
#define SOCK_OPTLEN int
#define STRUCT_UNIX struct
#else
#define CLOCK_TYPE CLOCK_MONOTONIC
#define SOCK_OPTLEN socklen_t
#define STRUCT_UNIX
#endif

#ifdef WIN32
#pragma comment(lib, "Ws2_32.lib")
typedef HANDLE PAM_API_HANDLE;
typedef SOCKET PAM_API_SOCKET;
#define PAM_API_INVALID_HANDLE  INVALID_HANDLE_VALUE
#else
typedef void* PAM_API_HANDLE;
typedef int PAM_API_SOCKET;
#define PAM_API_INVALID_HANDLE -1
#endif

#ifdef _MSC_VER

#else
#include <pthread.h>
#include <arpa/inet.h>
#endif

#include <stdlib.h>
#include <vector>
#include <list>
#include <fstream>
#include <iostream>
#include <string>
#include "astclient.h"

#ifdef WIN32
#pragma comment(lib, "Ws2_32.lib")
#else
#endif


static int split(char*buf,char**vec,int vc)
{
    if(!buf)
        return 0;

    char*tmp=0;
    int i=0;
    do
    {
        //vec[i++]=strtok_r(buf,"|",&tmp);
        buf=tmp;
    }
    while(buf && i<vc);

    return i;
}



void * thread_proc_import(void * param)
{
    //::pthread_detach(::pthread_self());

    //int ret;
    //const char*db_link=(const char*)param;

    //for(;;)
    //{
    //	sleep(60);

    //}

    return 0;
}

CAstClient::CAstClient()
{

}
CAstClient::CAstClient(const char* ip,int port,int timeout)
{
    m_ip=ip;
    m_port = port;
    m_timeout = timeout;
}

CAstClient::~CAstClient()
{
    clean_socket();
}

int CAstClient::clean_socket()
{
#if 0 //#ifdef _MSC_VER
    int error;
    if(WSACleanup()!=0)
    {
        error = GetLastError();
        return error;
    }
#endif
    return 0;
}

#include "sm/sm4.h"
int CAstClient::run(const char* sbuf,int slen,char* obuf,int& olen, int &sRealLen)
{
    int ret;
    char cTmp[256];
    sRealLen = m_fd.sendv(sbuf, slen, m_timeout);
    if (sRealLen != slen)
    {
        olen = 0;
        return -3;
    }
    if (obuf == NULL || olen < 8)
    {
        olen = 0;
        return -1;
    }
//printf("Send %d bytes succese!!\n", slen);
    int nHead = 8;
    nHead = m_fd.recvv(obuf,nHead,m_timeout); //�Ƚ���8�ֽ�
    int nLen = 0;
    if (nHead == 8)
    {
        nLen = zs_GetBE32((unsigned char *)&obuf[4]); //���ñ����ܳ���
        if(olen < nLen)
        {
            return nLen; //�ռ䲻�㣬���ر��Ĵ�С
        }
        olen = m_fd.recvv(&obuf[8],nLen-8, m_timeout);
        //printf("second olen=%d\n",olen);
        if (olen <= 0)
        {
            sprintf(cTmp, "CAstClient::run: recvv data=%d", olen);
            m_Error = cTmp;
            return -4;
        }
        olen += 8;
    }
    else
    {

        sprintf(cTmp, "CAstClient::run: recvv head=%d", nHead);
        m_Error = cTmp;
        olen = nHead;
        return -6;
    }
    return 0;
}
int CAstClient::runcontinue(int len, char* obuf)  //����run��ʱ���ռ䲻�㣬��ȡ�ܳ��Ⱥ󣬿��Լ�������8�ֽں��Ĳ���
{
    int nLen = 0;
    nLen = m_fd.recvv(obuf, len, m_timeout);
    if (nLen <= 0)
        return -1;
    return 0;
}
int CAstClient::bindClientIP(const char* ip, int port)
{
    int nRet = 0;
    m_fd.build_stream();
    nRet = m_fd.bind(ip, port);
    return nRet;
}
void CAstClient::InitClient(const char* ip, int port)
{
    m_clientip = ip;
    m_clientport = port;
}
void CAstClient::InitServer(const char* ip, int port, int timeout)
{
    m_ip = ip;
    m_port = port;
    m_timeout = timeout;
}

int CAstClient::init_socket()
{
#if 0 //#ifdef _MSC_VER
    WORD version = MAKEWORD (2, 2);
    WSADATA wsa_data;
    int error = WSAStartup (version, &wsa_data);
    if(error!=0)
    {
        return error;
    }
#endif
    //��ʼ����������

    return 0;
}

int CAstClient::connect()
{
    int ret;
    if(m_fd.m_fd == -1)
    {
        if(m_fd.build_stream() < 0)
        {
            //printf("build stream socket failed!");
            return -1;
        }
    }
    if((ret=m_fd.connect(m_ip.c_str(),m_port,m_timeout)))
          return -2;
    return 0;
}

int CAstClient::connect(bool bBindIp)
{
    int ret = 0;
#ifdef _MSC_VER
    SOCKADDR_IN m_address;
    string localips;
    int socklen;
    m_bChangeIp = false;
    if (bBindIp)
    {
        if (m_fd.build_stream() < 0)
        {
            //printf("build stream socket failed!");
            ret = -1;
            goto EXIT_CONNECT;
        }
        if (bindClientIP(m_clientip.c_str(), m_clientport) != 0)
        {
            ret = -2;
            goto EXIT_CONNECT;
        }
        if ((ret = m_fd.connect(m_ip.c_str(), m_port, m_timeout)))
        {
            ret = -3;
            goto EXIT_CONNECT;
        }
    }
    else
    {
        if (m_fd.m_fd == -1)
        {
            if (m_fd.build_stream() < 0)
            {
                //printf("build stream socket failed!");
                ret = -1;
                goto EXIT_CONNECT;
            }
        }
        if ((ret = m_fd.connect(m_ip.c_str(), m_port, m_timeout)))
        {
            ret = -2;
            goto EXIT_CONNECT;
        }
    }


EXIT_CONNECT:
    if (ret != 0 && bBindIp) //����ip����ʧ�ܣ�������ǰ�ip
    {
        m_fd.close();
        if (m_fd.build_stream() < 0)
        {
            //printf("build stream socket failed!");
            ret = -3;
        }
        else //����
        {
            if ((ret = m_fd.connect(m_ip.c_str(), m_port, m_timeout)))
            {
                ret = -4;
            }
            else
            {
                socklen = sizeof(m_address);
                //�����׽��ֻ�ȡ��ַ��Ϣ
                if (::getsockname(m_fd.m_fd, (SOCKADDR*)&m_address, &socklen) != 0)
                {
                    ret = -5;
                }
                else
                {
                    localips = inet_ntoa(m_address.sin_addr);
                    //���浱ǰ�ͻ��˻ip
                    m_clientip = localips;
                    ret = 0;
                    //pc������ip
                    m_bChangeIp = true;
                }
            }
        }
    }
#else
    sockaddr_in m_address;
    std::string localips;
    socklen_t socklen;
    m_bChangeIp = false;
    if (bBindIp)
    {
        if (m_fd.build_stream() < 0)
        {
            //printf("build stream socket failed!");
            ret = -1;
            goto EXIT_CONNECT;
        }
        if (bindClientIP(m_clientip.c_str(), m_clientport) != 0)
        {
            ret = -2;
            goto EXIT_CONNECT;
        }
        if ((ret = m_fd.connect(m_ip.c_str(), m_port, m_timeout)))
        {
            ret = -3;
            goto EXIT_CONNECT;
        }
    }
    else
    {
        if (m_fd.m_fd == -1)
        {
            if (m_fd.build_stream() < 0)
            {
                //printf("build stream socket failed!");
                ret = -1;
                goto EXIT_CONNECT;
            }
        }
        if ((ret = m_fd.connect(m_ip.c_str(), m_port, m_timeout)))
        {
            ret = -2;
            goto EXIT_CONNECT;
        }
    }


EXIT_CONNECT:
    if (ret != 0 && bBindIp) //����ip����ʧ�ܣ�������ǰ�ip
    {
        m_fd.close();
        if (m_fd.build_stream() < 0)
        {
            //printf("build stream socket failed!");
            ret = -3;
        }
        else //����
        {
            if ((ret = m_fd.connect(m_ip.c_str(), m_port, m_timeout)))
            {
                ret = -4;
            }
            else
            {
                socklen = sizeof(m_address);
                //�����׽��ֻ�ȡ��ַ��Ϣ
                if (::getsockname(m_fd.m_fd, (sockaddr*)&m_address, &socklen) != 0)
                {
                    ret = -5;
                }
                else
                {
                    localips = inet_ntoa(m_address.sin_addr);
                    //���浱ǰ�ͻ��˻ip
                    m_clientip = localips;
                    ret = 0;
                    //pc������ip
                    m_bChangeIp = true;
                }
            }
        }
    }
#endif

    return ret;
}

int CAstClient::close()
{
    int ret;
    ret = m_fd.close();
    return ret;
}


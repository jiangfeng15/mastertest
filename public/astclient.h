#ifndef ASSCLIENT_H
#define ASSCLIENT_H
#include <string>
#include "zsocket.h"

struct io_buf;
class CAstClient
{
public:
    CAstClient();
    CAstClient(const char* ip,int port,int timeout);
    ~CAstClient();

    std::string m_Error;

    int bindClientIP(const char* ip, int port);
    void InitServer(const char* ip, int port, int timeout);
    void InitClient(const char* ip, int port);
    int init_socket();
    int connect();
    //bBindIp = true ��Ҫ����ip�� false ����Ҫ
    int connect(bool bBindIp);
    int close();
    int clean_socket();
    int run(const char* sbuf,int slen,char* obuf,int& olen, int &sRealLen);
    int runcontinue(int len, char* obuf);  //����run��ʱ���ռ䲻�㣬��ȡ�ܳ��Ⱥ󣬿��Լ�������8�ֽں��Ĳ���

public:
    CSocket m_fd;
    bool m_bChangeIp;

    std::string m_ip;
    int m_port;
    int m_timeout;

    std::string m_clientip;
    int m_clientport;

};
#endif // ASSCLIENT_H








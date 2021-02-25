#include <sys/stat.h>
#include "ftpclient.h"

#define _atoi64(val) strtoll(val,NULL, 10)
#define GetCurrentTime GetTickCount

#pragma comment(lib, "Ws2_32.lib")

unsigned long GetTickCount()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000 + ts.tv_nsec/1000000);
}

static int SplitString( std::string strSrc, std::list<std::string> &strArray , std::string strFlag)
{
    int pos = 1;

    while((pos = (int)strSrc.find_first_of(strFlag.c_str())) > 0)
    {
        strArray.insert(strArray.end(), strSrc.substr(0 , pos));
        strSrc = strSrc.substr(pos + 1, strSrc.length() - pos - 1);
    }

    strArray.insert(strArray.end(), strSrc.substr(0, strSrc.length()));

    return 0;
}

ftpClient::ftpClient(void): m_bLogin(false)
{
    m_strLocalDir = std::string(FTP_DEFAULT_PATH);
    m_nDownLoadSpeed = 0;
    m_nDLMaxSize = 0;     //下载文件的大小
    m_nDownLoaded = 0;    //已下载的大小
    //m_cmdSocket = socket(AF_INET, SOCK_STREAM, 0);
    m_cmdSocket = FTP_INVALID_SOCKET;

    m_pnDownState = NULL;
}

ftpClient::~ftpClient(void)
{
    std::string strCmdLine = parseCommand(FTP_COMMAND_QUIT, "");

    Send(m_cmdSocket, strCmdLine.c_str());
    close(m_cmdSocket);
    m_bLogin = false;
}

FTP_API ftpClient::login2Server(const std::string &serverIP)
{
    std::string strPort;
    int pos = serverIP.find_first_of(":");
    if(m_cmdSocket == FTP_INVALID_SOCKET)
        m_cmdSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (pos > 0)
    {
        strPort = serverIP.substr(pos + 1, serverIP.length() - pos);
    }
    else
    {
        pos = serverIP.length();
        strPort = FTP_DEFAULT_PORT;
    }

    m_strServerIP = serverIP.substr(0, pos);
    m_nServerPort = atol(strPort.c_str());

    trace("IP: %s port: %d\n", m_strServerIP.c_str(), m_nServerPort);

    if (Connect(m_cmdSocket, m_strServerIP, m_nServerPort) < 0)
    {

        return -1;
    }

    m_strResponse = serverResponse(m_cmdSocket);
    //printf("@@@@Response: %s", m_strResponse.c_str());

    return	parseResponse(m_strResponse);
}

FTP_API ftpClient::inputUserName(const std::string &userName)
{
    std::string strCommandLine = parseCommand(FTP_COMMAND_USERNAME, userName);

    m_strUserName = userName;

    if (Send(m_cmdSocket, strCommandLine) < 0)
    {
        return -1;
    }

    m_strResponse = serverResponse(m_cmdSocket);
    //printf("Response: %s\n", m_strResponse.c_str());

    return parseResponse(m_strResponse);
}

FTP_API ftpClient::inputPassWord(const std::string &password)
{
    std::string strCmdLine = parseCommand(FTP_COMMAND_PASSWORD, password);

    m_strPassWord = password;
    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        return -1;
    }
    else
    {
        m_bLogin = true;

        m_strResponse = serverResponse(m_cmdSocket);
        //printf("Response: %s\n", m_strResponse.c_str());

        return parseResponse(m_strResponse);
    }
}
bool  ftpClient::IsLogin()
{
    return m_bLogin;
}
FTP_API ftpClient::quitServer(void)
{
    std::string strCmdLine = parseCommand(FTP_COMMAND_QUIT, "");
    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        return -1;
    }
    else
    {
        m_strResponse = serverResponse(m_cmdSocket);
        //printf("Response: %s\n", m_strResponse.c_str());

        return parseResponse(m_strResponse);
    }

}

const std::string ftpClient::PWD()
{
    std::string strCmdLine = parseCommand(FTP_COMMAND_CURRENT_PATH, "");

    if (Send(m_cmdSocket, strCmdLine.c_str()) < 0)
    {
        return "";
    }
    else
    {
        return serverResponse(m_cmdSocket);
    }
}


FTP_API ftpClient::setTransferMode(type mode)
{
    std::string strCmdLine;

    switch (mode)
    {
    case binary:
        strCmdLine = parseCommand(FTP_COMMAND_TYPE_MODE, "I");
        break;
    case ascii:
        strCmdLine = parseCommand(FTP_COMMAND_TYPE_MODE, "A");
        break;
    default:
        break;
    }

    if (Send(m_cmdSocket, strCmdLine.c_str()) < 0)
    {
        //assert(false);
        return -1;
    }
    else
    {
        m_strResponse  = serverResponse(m_cmdSocket);
        //printf("@@@@Response: %s", m_strResponse.c_str());

        return parseResponse(m_strResponse);
    }
    return 0;
}


const std::string ftpClient::Pasv()
{
    std::string strCmdLine = parseCommand(FTP_COMMAND_PSAV_MODE, "");

    if (Send(m_cmdSocket, strCmdLine.c_str()) < 0)
    {
        return "";
    }
    else
    {
        m_strResponse = serverResponse(m_cmdSocket);

        return m_strResponse;
    }
}

const std::string ftpClient::GetResponMsg()
{
    return m_strResponse;
}

const std::string ftpClient::Dir(const std::string &path)
{
    int dataSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (createDataLink(dataSocket) < 0)
    {
        return "";
    }
    // 数据连接成功
    std::string strCmdLine = parseCommand(FTP_COMMAND_DIR, path);

    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());
        close(dataSocket);
        //closesocket(dataSocket);
        return "";
    }
    else
    {
        trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());
        m_strResponse = serverResponse(dataSocket);

        trace("@@@@Response: \n%s\n", m_strResponse.c_str());
        close(dataSocket);
        //closesocket(dataSocket);

        return m_strResponse;
    }

}


FTP_API ftpClient::CD(const std::string &path)
{
    //assert(m_cmdSocket != FTP_INVALID_SOCKET);
    if(m_cmdSocket == FTP_INVALID_SOCKET)
        return -2;
    std::string strCmdLine = parseCommand(FTP_COMMAND_CHANGE_DIRECTORY, path);

    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        return -1;
    }

    m_strResponse = serverResponse(m_cmdSocket);

    trace("@@@@Response: %s\n", m_strResponse.c_str());
    return parseResponse(m_strResponse);
}

FTP_API ftpClient::DeleteFile(const std::string &strRemoteFile)
{
    //assert(m_cmdSocket != FTP_INVALID_SOCKET);
    if(m_cmdSocket == FTP_INVALID_SOCKET)
        return -2;

    std::string strCmdLine = parseCommand(FTP_COMMAND_DELETE_FILE, strRemoteFile);

    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        return -1;
    }

    m_strResponse = serverResponse(m_cmdSocket);
    //printf("@@@@Response: %s\n", m_strResponse.c_str());
    return parseResponse(m_strResponse);
}

FTP_API ftpClient::DeleteDirectory(const std::string &strRemoteDir)
{
    //assert(m_cmdSocket != FTP_INVALID_SOCKET);
    if(m_cmdSocket == FTP_INVALID_SOCKET)
        return -2;

    std::string strCmdLine = parseCommand(FTP_COMMAND_DELETE_DIRECTORY, strRemoteDir);

    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        return -1;
    }

    m_strResponse = serverResponse(m_cmdSocket);

    trace("@@@@Response: %s\n", m_strResponse.c_str());
    return parseResponse(m_strResponse);
}

FTP_API ftpClient::CreateDirectory(const std::string &strRemoteDir)
{
    //assert(m_cmdSocket != FTP_INVALID_SOCKET);
    if(m_cmdSocket == FTP_INVALID_SOCKET)
        return -2;

    std::string strCmdLine = parseCommand(FTP_COMMAND_CREATE_DIRECTORY, strRemoteDir);

    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        return -1;
    }

    m_strResponse = serverResponse(m_cmdSocket);

    trace("@@@@Response: %s\n", m_strResponse.c_str());
    return parseResponse(m_strResponse);
}

FTP_API ftpClient::Rename(const std::string &strRemoteFile, const std::string &strNewFile)
{
    //assert(m_cmdSocket != FTP_INVALID_SOCKET);
    if(m_cmdSocket == FTP_INVALID_SOCKET)
        return -2;

    std::string strCmdLine = parseCommand(FTP_COMMAND_RENAME_BEGIN, strRemoteFile);
    Send(m_cmdSocket, strCmdLine);
    trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());

    Send(m_cmdSocket, parseCommand(FTP_COMMAND_RENAME_END, strNewFile));

    m_strResponse = serverResponse(m_cmdSocket);
    trace("@@@@Response: %s\n", m_strResponse.c_str());
    return parseResponse(m_strResponse);
}

//long ftpClient::getFileLength(const std::string &strRemoteFile)
long long  ftpClient::getFileLength(const std::string &strRemoteFile)
{
    //assert(m_cmdSocket != FTP_INVALID_SOCKET);
    if(m_cmdSocket == FTP_INVALID_SOCKET)
        return -2;

    std::string strCmdLine = parseCommand(FTP_COMMAND_FILE_SIZE, strRemoteFile);

    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        return -1;
    }

    m_strResponse = serverResponse(m_cmdSocket);

    trace("@@@@Response: %s\n", m_strResponse.c_str());

    std::string strData = m_strResponse.substr(0, 3);
    unsigned long val = atol(strData.c_str());


    if (val == 213)
    {
        strData = m_strResponse.substr(4);
        trace("strData: %s\n", strData.c_str());
        //val = atol(strData.c_str());
        long long  nReturn = _atoi64(strData.c_str());
        //__int64 _atoi64 要支持2G以上
        return nReturn;
    }

    return -1;
}
void ftpClient::CloseFtp()
{
    close(m_cmdSocket);
    m_cmdSocket = FTP_INVALID_SOCKET;
}

void ftpClient::Close(int sock)
{
    //shutdown(sock, SHUT_RDWR); ??
    close(sock);
    //closesocket(sock);
    sock = FTP_INVALID_SOCKET;
}

FTP_API ftpClient::Get(const std::string &strRemoteFile, const std::string &strLocalFile)
{
    return downLoad(strRemoteFile, strLocalFile, 0, 0);
}
FTP_API ftpClient::Get(const std::string &strRemoteFile, const std::string &strLocalFile, long long  &downlen)
{
    return downLoad64(strRemoteFile, strLocalFile, downlen, 0, 0);
}

FTP_API ftpClient::Put(const std::string &strRemoteFile, const std::string &strLocalFile)
{
    std::string strCmdLine;
    const unsigned long dataLen = FTP_DEFAULT_BUFFER;
    char strBuf[dataLen] = {0};
    unsigned long nSize = getFileLength(strRemoteFile);
    unsigned long nLen = 0;
// 	struct stat sBuf;
//
// 	assert(stat(strLocalFile.c_str(), &sBuf) == 0);
// 	trace("size: %d\n", sBuf.st_size);
    std::string strFileName = m_strLocalDir + strLocalFile;
    FILE *pFile = fopen(strFileName.c_str(), "rb");  // 以只读方式打开  且文件必须存在
    //FILE *pFile = fopen(strLocalFile.c_str(), "rb");  // 以只读方式打开  且文件必须存在
    if(pFile == NULL)
        return -2;
    assert(pFile != NULL);


    int data_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(data_fd < 0)
        return data_fd;
    assert(data_fd != -1);

    if (createDataLink(data_fd) < 0)
    {
        fclose(pFile);
        return -1;
    }

    if (nSize == -1)
    {
        strCmdLine = parseCommand(FTP_COMMAND_UPLOAD_FILE, strRemoteFile);
    }
    else
    {
        strCmdLine = parseCommand(FTP_COMMAND_APPEND_FILE, strRemoteFile);
    }

    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        fclose(pFile);
        Close(data_fd);
        return -1;
    }

    trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());

    fseek(pFile, nSize, SEEK_SET);  //_fseeki64 大于2G
    while (!feof(pFile))
    {
        nLen = fread(strBuf, 1, dataLen, pFile);
        if (nLen < 0)
        {
            break;
        }

        //if (Send(data_fd, strBuf) < 0)
        if(Send(data_fd, strBuf, nLen) < 0)
        {
            Close(data_fd);
            fclose(pFile);
            return -1;
        }
    }

    trace("@@@@Response: %s\n", serverResponse(data_fd).c_str());

    Close(data_fd);
    trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());
    fclose(pFile);

    return 0;
}

//大于2G文件的上传
FTP_API ftpClient::Put(const std::string &strRemoteFile, const std::string &strLocalFile, long long  &uplen)
{
    std::string strCmdLine;
    const unsigned long dataLen = FTP_DEFAULT_BUFFER;
    char strBuf[dataLen] = {0};
    //unsigned long nSize = getFileLength(strRemoteFile);
    long long  nSize = getFileLength(strRemoteFile);
    unsigned long nLen = 0;
// 	struct stat sBuf;
//
// 	assert(stat(strLocalFile.c_str(), &sBuf) == 0);
// 	trace("size: %d\n", sBuf.st_size);
    std::string strFileName = m_strLocalDir + strLocalFile;
    FILE *pFile = fopen(strFileName.c_str(), "rb");  // 以只读方式打开  且文件必须存在
    //FILE *pFile = fopen(strLocalFile.c_str(), "rb");  // 以只读方式打开  且文件必须存在
    if(pFile == NULL)
        return -2;
    assert(pFile != NULL);


    int data_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(data_fd < 0)
        return data_fd;
    assert(data_fd != -1);

    if (createDataLink(data_fd) < 0)
    {
        fclose(pFile);
        return -1;
    }

    if (nSize == -1)
    {
        strCmdLine = parseCommand(FTP_COMMAND_UPLOAD_FILE, strRemoteFile);
    }
    else
    {
        strCmdLine = parseCommand(FTP_COMMAND_APPEND_FILE, strRemoteFile);
    }

    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        fclose(pFile);
        Close(data_fd);
        return -1;
    }

    trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());

    //fseek(pFile, nSize, SEEK_SET);

    fseeko64(pFile, nSize, SEEK_SET);
    //_fseeki64(pFile, nSize, SEEK_SET);  //_fseeki64 大于2G
    uplen = 0;
    while (!feof(pFile))
    {
        nLen = fread(strBuf, 1, dataLen, pFile);
        if (nLen < 0)
        {
            break;
        }
        //if (Send(data_fd, strBuf) < 0)
        if(Send(data_fd, strBuf, nLen) < 0)
        {
            Close(data_fd);
            fclose(pFile);
            return -1;
        }
        uplen+=nLen;
    }

    trace("@@@@Response: %s\n", serverResponse(data_fd).c_str());

    Close(data_fd);
    trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());
    fclose(pFile);

    return 0;
}

const std::string ftpClient::parseCommand(const unsigned int command, const std::string &strParam)
{
    if (command < FTP_COMMAND_BASE || command > FTP_COMMAND_END)
    {
        return "";
    }

    std::string strCommandLine;

    m_nCurrentCommand = command;
    m_commandStr.clear();

    switch (command)
    {
    case FTP_COMMAND_USERNAME:
        strCommandLine = "USER ";
        break;
    case FTP_COMMAND_PASSWORD:
        strCommandLine = "PASS ";
        break;
    case FTP_COMMAND_QUIT:
        strCommandLine = "QUIT ";
        break;
    case FTP_COMMAND_CURRENT_PATH:
        strCommandLine = "PWD ";
        break;
    case FTP_COMMAND_TYPE_MODE:
        strCommandLine = "TYPE ";
        break;
    case FTP_COMMAND_PSAV_MODE:
        strCommandLine = "PASV ";
        break;
    case FTP_COMMAND_DIR:
        strCommandLine = "LIST ";
        break;
    case FTP_COMMAND_CHANGE_DIRECTORY:
        strCommandLine = "CWD ";
        break;
    case FTP_COMMAND_DELETE_FILE:
        strCommandLine = "DELE ";
        break;
    case FTP_COMMAND_DELETE_DIRECTORY:
        strCommandLine = "RMD ";
        break;
    case FTP_COMMAND_CREATE_DIRECTORY:
        strCommandLine = "MKD ";
        break;
    case FTP_COMMAND_RENAME_BEGIN:
        strCommandLine = "RNFR ";
        break;
    case FTP_COMMAND_RENAME_END:
        strCommandLine = "RNTO ";
        break;
    case FTP_COMMAND_FILE_SIZE:
        strCommandLine = "SIZE ";
        break;
    case FTP_COMMAND_DOWNLOAD_FILE:
        strCommandLine = "RETR ";
        break;
    case FTP_COMMAND_DOWNLOAD_POS:
        strCommandLine = "REST ";
        break;
    case FTP_COMMAND_UPLOAD_FILE:
        strCommandLine = "STOR ";
        break;
    case FTP_COMMAND_APPEND_FILE:
        strCommandLine = "APPE ";
        break;
    default :
        break;
    }
    if(strCommandLine.length() < 3)  //error cmd
        return m_commandStr;

    strCommandLine += strParam;
    strCommandLine += "\r\n";

    m_commandStr = strCommandLine;
    trace("parseCommand: %s\n", m_commandStr.c_str());

    return m_commandStr;
}

int setblocking (int fd, int b)
{
    unsigned long c = b;  //设置阻塞模式
    if(b<0)
        return 0;
    if( ioctl(fd, FIONBIO, &c)==-1)
        return -1;
    return 0;
}

FTP_API ftpClient::Connect(int socketfd, const std::string &serverIP, unsigned int nPort)
{
    if (socketfd == FTP_INVALID_SOCKET)
    {
        return -1;
    }

    unsigned int argp = 1;
    int error = -1;
    socklen_t len = sizeof(socklen_t);
    struct sockaddr_in  addr;
    bool ret = false;
    timeval stime;
    fd_set  set;

    //ioctl(socketfd, FIONBIO, &argp);  //设置为非阻塞模式
    setblocking (socketfd, 1);

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port	= htons(nPort);
    addr.sin_addr.s_addr = inet_addr(serverIP.c_str());
    //bzero(&(addr.sin_zero), 8);

    trace("Address: %s %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

    if (connect(socketfd, (struct sockaddr*)&addr, sizeof(struct sockaddr)) == -1)   //若直接返回 则说明正在进行TCP三次握手
    {
        stime.tv_sec = 20;  //设置为20秒超时
        stime.tv_usec = 0;
        FD_ZERO(&set);
        FD_SET(socketfd, &set);

        if (select(socketfd + 1, NULL, &set, NULL, &stime) > 0)   ///在这边等待 阻塞 返回可以读的描述符 或者超时返回0  或者出错返回-1
        {
            getsockopt(socketfd, SOL_SOCKET, SO_ERROR, (char *)&error, &len);
            if (error == 0)
            {
                ret = true;
            }
            else
            {
                ret = false;
            }
        }
    }
    else
    {	trace("Connect Immediately!!!\n");
        ret = true;
    }

    argp = 0;
    //ioctl(socketfd, FIONBIO, &argp);
    setblocking (socketfd, 0);

    if (!ret)
    {
        //close(socketfd);
        close(socketfd);
        fprintf(stderr, "cannot connect server!!\n");
        return -1;
    }

    //fprintf(stdout, "Connect!!!\n");

    return 0;
}


const std::string ftpClient::serverResponse(int sockfd)
{
    if (sockfd == FTP_INVALID_SOCKET)
    {
        return "";
    }

    int nRet = -1;
    char buf[MAX_PATH] = {0};

    m_strResponse.clear();

    while ((nRet = getData(sockfd, buf, MAX_PATH)) > 0)
    {
        buf[MAX_PATH - 1] = '\0';
        m_strResponse += buf;
    }

    return m_strResponse;
}

FTP_API ftpClient::getData(int fd, char *strBuf, unsigned long length)
{
    //assert(strBuf != NULL);

    if (fd == FTP_INVALID_SOCKET)
    {
        return -1;
    }

    memset(strBuf, 0, length);
    timeval stime;
    int nLen;

    stime.tv_sec = 1;
    stime.tv_usec = 0;

    fd_set	readfd;
    FD_ZERO( &readfd );
    FD_SET(fd, &readfd );

    if (select(fd + 1, &readfd, 0, 0, &stime) > 0)
    {
        if ((nLen = recv(fd, strBuf, length, 0)) > 0)
        {
            return nLen;
        }
        else
        {
            return -2;
        }
    }
    return 0;
}

FTP_API ftpClient::Send(int fd, const std::string &cmd)
{
    if (fd == FTP_INVALID_SOCKET)
    {
        return -1;
    }

    return Send(fd, cmd.c_str(), cmd.length());
}

FTP_API ftpClient::Send(int fd, const char *cmd, const size_t len)
{
    if((FTP_COMMAND_USERNAME != m_nCurrentCommand)
        &&(FTP_COMMAND_PASSWORD != m_nCurrentCommand)
        &&(!m_bLogin))
    {
        return -1;
    }

    timeval timeout;
    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;

    fd_set  writefd;
    FD_ZERO(&writefd);
    FD_SET(fd, &writefd);

    if(select(fd + 1, 0, &writefd , 0 , &timeout) > 0)
    {
        size_t nlen  = len;
        int nSendLen = 0;
        while (nlen >0)
        {
            nSendLen = send(fd, cmd , (int)nlen , 0);

            if(nSendLen == -1)
                return -2;

            nlen = nlen - nSendLen;
            cmd +=  nSendLen;
        }
        return 0;
    }
    return -1;
}


FTP_API ftpClient::createDataLink(int data_fd)
{
    //assert(data_fd != FTP_INVALID_SOCKET);
    if(data_fd == FTP_INVALID_SOCKET)
        return -2;

    std::string strData;
    unsigned long nPort = 0 ;
    std::string strServerIp ;
    std::list<std::string> strArray ;

    std::string parseStr = Pasv();

    if (parseStr.size() <= 0)
    {
        return -1;
    }

    //trace("parseInfo: %s\n", parseStr.c_str());

    size_t nBegin = parseStr.find_first_of("(");
    size_t nEnd	  = parseStr.find_first_of(")");
    strData		  = parseStr.substr(nBegin + 1, nEnd - nBegin - 1);

    //trace("ParseAfter: %s\n", strData.c_str());
    if( SplitString( strData , strArray , "," ) <0)
        return -1;

    if( ParseString( strArray , nPort , strServerIp) < 0)
        return -1;

    //trace("nPort: %ld IP: %s\n", nPort, strServerIp.c_str());

    if (Connect(data_fd, strServerIp, nPort) < 0)
    {
        return -1;
    }

    return 0;

}

FTP_API ftpClient::ParseString(std::list<std::string> strArray, unsigned long & nPort ,std::string & strServerIp)
{
    if (strArray.size() < 6 )
        return -1 ;

    std::list<std::string>::iterator citor;
    citor = strArray.begin();
    strServerIp = *citor;
    strServerIp += ".";
    citor ++;
    strServerIp += *citor;
    strServerIp += ".";
    citor ++ ;
    strServerIp += *citor;
    strServerIp += ".";
    citor ++ ;
    strServerIp += *citor;
    citor = strArray.end();
    citor--;
    nPort = atol( (*citor).c_str());
    citor--;
    nPort += atol( (*(citor)).c_str()) * 256 ;
    return 0 ;
}

FILE *ftpClient::createLocalFile(const std::string &strLocalFile)
{
    return fopen(strLocalFile.c_str(), "w+b");
}
int ftpClient::GetLocalFileSize(const std::string &strLocalFile)
{
    FILE *fp = fopen(strLocalFile.c_str(), "rb");
    if(fp == NULL)
        return -1;
    fclose(fp);
    struct stat info;
    stat(strLocalFile.c_str(), &info);
    return info.st_size;
}
int ftpClient::GetLocalFileSize(const std::string &strLocalFile, long long &filesize)
{
    FILE *fp = fopen(strLocalFile.c_str(), "rb");
    if(fp == NULL)
    {
        filesize = -1;
        return filesize;
    }
    fclose(fp);
    struct stat64 info;
    stat64(strLocalFile.c_str(), &info);
    filesize = info.st_size;
    return 0;
}


unsigned int CostDiff(unsigned int a, unsigned int b)
{
    if(a >= b)  //计算两次之间的GetTickCount() 的差值（毫秒数),大约49天（4G毫秒数）会循环，所以要判断大小
        return(a - b);
    else
        return(a + (0xFFFFFFFF - b));
}
FTP_API ftpClient::downLoad(const std::string &strRemoteFile, const std::string &strLocalFile, const int pos, const unsigned int length)
{
    if(length < 0)
        return -1;
    assert(length >= 0);

    FILE *file = NULL;
    unsigned long nDataLen = FTP_DEFAULT_BUFFER;
    char strPos[MAX_PATH]  = {0};
    int data_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(data_fd < 0)
        return -2;
    //assert(data_fd != -1);
    std::string strLocal = m_strLocalDir+strLocalFile;
    m_nDLMaxSize = this->getFileLength(strRemoteFile);
//printf("filesize=%lld\n", m_nDLMaxSize);
    //int nLocalNow = GetLocalFileSize(strLocal);
    long long  nLocalNow = 0;
    GetLocalFileSize(strLocal, nLocalNow);
    if ((length != 0) && (length < nDataLen))
    {
        nDataLen = length;
    }
    //char *dataBuf = new char[nDataLen];
    //assert(dataBuf != NULL);
    if(nLocalNow >= pos)
        sprintf(strPos, "%lld", nLocalNow);
    else
        sprintf(strPos, "%d", pos);

    if (createDataLink(data_fd) < 0)
    {
        trace("@@@@ Create Data Link error!!!\n");
        return -1;
    }

    std::string strCmdLine = parseCommand(FTP_COMMAND_DOWNLOAD_POS, strPos);
    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        return -1;
    }
    std::string respon = serverResponse(m_cmdSocket).c_str();
    int nRes = atoi(respon.c_str());  //350
    //trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());

    strCmdLine = parseCommand(FTP_COMMAND_DOWNLOAD_FILE, strRemoteFile);

    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        return -1;
    }
    respon = serverResponse(m_cmdSocket).c_str();
    nRes = atoi(respon.c_str());
    if(nRes == 550)
        return -3;
    //trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());


    if(nLocalNow >= pos)  //断点续传
    {
        file = fopen(strLocal.c_str(), "a+b");  //createLocalFile(strLocal);
        //assert(file != NULL);
        if(file == NULL)
            return -4;
        fseek(file, pos, SEEK_SET);
    }
    else
    {

    //file = createLocalFile(std::string(FTP_DEFAULT_PATH + strLocalFile));
        file = createLocalFile(strLocal);
        //assert(file != NULL);
        if(file == NULL)
            return -4;
        nLocalNow = 0;
    }

    int len = 0;
    //int nReceiveLen = 0;
    long long  nReceiveLen = 0;
    unsigned int uBegin = GetCurrentTime();
    unsigned int uNow = 0;
    unsigned int uCost = 0;
    unsigned int uSleep = 0;
    //unsigned int uSpeed = 1000;  //100B/ms = 100KB/s
    char *dataBuf = new char[nDataLen];
    unsigned int uPreCost = 0;
    m_nDownLoaded = nLocalNow;
    //printf("Begin=%d\n", uBegin);

    while ((len = getData(data_fd, dataBuf, nDataLen)) > 0)
    {
        nReceiveLen += (long long )len;
        m_nDownLoaded += (long long )len;

        int num = fwrite(dataBuf, 1, len, file);
        memset(dataBuf, 0, sizeof(dataBuf));

        //trace("%s", dataBuf);
        //trace("Num:%d\n", num);
        //trace("\rNum:%lld", nReceiveLen);
        if (nReceiveLen == length && length != 0)
            break;

        if ((nReceiveLen + nDataLen) > length  && length != 0)
        {
            delete []dataBuf;
            nDataLen = length - nReceiveLen;
            dataBuf = new char[nDataLen];
        }
        if(m_nDownLoadSpeed > 0)
        {
            uNow = GetCurrentTime();
            uCost = CostDiff(uNow, uBegin);
            uPreCost = (unsigned int)(nReceiveLen/m_nDownLoadSpeed);
            if(uPreCost > uCost)
                sleep(uPreCost - uCost);
        }
#if 0  //断点续传测试
        if(nReceiveLen >= 0x80000001L)
            break;
#endif
    }
    unsigned int uEnd = GetCurrentTime();
    //printf("\nEnd=%d\n", uEnd);
    uCost = CostDiff(uEnd, uBegin);
#if 0
    printf("\nDownLoad Cost = %u ms\n", uCost);
    if(uCost > 0)
        printf("Speed = %.2f KB/Sec \n", (float)nReceiveLen/(float)uCost );
#endif
    Close(data_fd);
    fclose(file);
    delete []dataBuf;
    //返回值保证为正数
    if(nReceiveLen >= 0x80000000)
        return 0x7fffffff;
    else
        return nReceiveLen;
    //return 0;
}

FTP_API ftpClient::downLoad64(const std::string &strRemoteFile, const std::string &strLocalFile, long long &downlen, const int pos, const unsigned int length)
{
    if(length < 0)
        return -1;
    assert(length >= 0);

    FILE *file = NULL;
    unsigned long nDataLen = FTP_DEFAULT_BUFFER;
    char strPos[MAX_PATH]  = {0};
    int data_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(data_fd < 0)
        return -2;
    //assert(data_fd != -1);
    std::string strLocal = m_strLocalDir+strLocalFile;
    m_nDLMaxSize = this->getFileLength(strRemoteFile);
//printf("filesize=%lld\n", m_nDLMaxSize);
    //int nLocalNow = GetLocalFileSize(strLocal);
    long long  nLocalNow = 0;
    GetLocalFileSize(strLocal, nLocalNow);
    if ((length != 0) && (length < nDataLen))
    {
        nDataLen = length;
    }
    //char *dataBuf = new char[nDataLen];
    //assert(dataBuf != NULL);
    if(nLocalNow >= pos)
        sprintf(strPos, "%lld", nLocalNow);
    else
        sprintf(strPos, "%d", pos);

    if (createDataLink(data_fd) < 0)
    {
        trace("@@@@ Create Data Link error!!!\n");
        return -3;
    }

    std::string strCmdLine = parseCommand(FTP_COMMAND_DOWNLOAD_POS, strPos);
    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        return -4;
    }
    std::string respon = serverResponse(m_cmdSocket).c_str();
    int nRes = atoi(respon.c_str());  //350
    //trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());

    strCmdLine = parseCommand(FTP_COMMAND_DOWNLOAD_FILE, strRemoteFile);

    if (Send(m_cmdSocket, strCmdLine) < 0)
    {
        return -5;
    }
    respon = serverResponse(m_cmdSocket).c_str();
    nRes = atoi(respon.c_str());
    if(nRes == 550)
        return -6;
    //trace("@@@@Response: %s\n", serverResponse(m_cmdSocket).c_str());


    if(nLocalNow >= pos)  //断点续传
    {
        file = fopen(strLocal.c_str(), "a+b");  //createLocalFile(strLocal);
        //assert(file != NULL);
        if(file == NULL)
            return -7;
        fseek(file, pos, SEEK_SET);
    }
    else
    {

    //file = createLocalFile(std::string(FTP_DEFAULT_PATH + strLocalFile));
        file = createLocalFile(strLocal);
        //assert(file != NULL);
        if(file == NULL)
            return -8;
        nLocalNow = 0;
    }

    int len = 0;
    //int nReceiveLen = 0;
    long long  nReceiveLen = 0;
    unsigned int uBegin = GetCurrentTime();
    unsigned int uNow = 0;
    unsigned int uCost = 0;
    unsigned int uSleep = 0;
    //unsigned int uSpeed = 1000;  //100B/ms = 100KB/s
    char *dataBuf = new char[nDataLen];
    unsigned int uPreCost = 0;
    m_nDownLoaded = nLocalNow;
    //printf("Begin=%d\n", uBegin);

    while ((len = getData(data_fd, dataBuf, nDataLen)) > 0)
    {
        nReceiveLen += (long long)len;
        m_nDownLoaded += (long long)len;

        int num = fwrite(dataBuf, 1, len, file);
        memset(dataBuf, 0, sizeof(dataBuf));

        //trace("%s", dataBuf);
        //trace("Num:%d\n", num);
        trace("\rNum:%lld", nReceiveLen);
        if (nReceiveLen == length && length != 0)
            break;

        if ((nReceiveLen + nDataLen) > length  && length != 0)
        {
            delete []dataBuf;
            nDataLen = length - nReceiveLen;
            dataBuf = new char[nDataLen];
        }
        if(m_nDownLoadSpeed > 0)
        {
            uNow = GetCurrentTime();
            uCost = CostDiff(uNow, uBegin);
            uPreCost = (unsigned int)(nReceiveLen/m_nDownLoadSpeed);
            if(uPreCost > uCost)
                sleep(uPreCost - uCost);
        }

        //判断是否停止下载
        while (m_pnDownState&&*m_pnDownState)
        {
            if (*m_pnDownState == 1)
            {
                sleep(1000);
            }
            else
                break;
        }
        if (m_pnDownState&&*m_pnDownState == 2)
        {
            break;
        }

#if 0  //断点续传测试
        if(nReceiveLen >= 0x80000001L)
            break;
#endif
    }
    unsigned int uEnd = GetCurrentTime();
    //printf("\nEnd=%d\n", uEnd);
    uCost = CostDiff(uEnd, uBegin);
#if 0
    printf("\nDownLoad Cost = %u ms\n", uCost);
    if(uCost > 0)
        printf("Speed = %.2f KB/Sec \n", (float)nReceiveLen/(float)uCost );
#endif
    downlen = nReceiveLen;
    Close(data_fd);
    fclose(file);
    delete []dataBuf;
    //返回值保证为正数
    if(nReceiveLen >= 0x80000000)
        return 0x7fffffff;
    else
        return nReceiveLen;
    //return 0;
}
FTP_API ftpClient::parseResponse(const std::string &str)
{
    //assert(!str.empty());
    if(str.empty())
    {
        return -100;
    }

    std::string strData = str.substr(0, 3);
    unsigned int val = atoi(strData.c_str());

    return val;
}

void ftpClient::copyVariable(int & nVariable)
{
    m_pnDownState = &nVariable;
}

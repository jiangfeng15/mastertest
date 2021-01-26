#ifndef ASTSERVERCTRL_H
#define ASTSERVERCTRL_H
#include <vector>
#include <string>
#include "public/define.h"
#include "public/astclient.h"
#include "public/astPackage/astpackage.h"
#include "public/json/include/json.h"


using namespace std;


class astServerCtrl
{
public:
    astServerCtrl();
    ~astServerCtrl();

    int RegAst();     //注册服务器
    int RetryRegAst(); //重新注册
    int RegFinish(char *regmsg, int regmsglen);  //注册应答处理
    int RegAck(char *regmsg, int regmsglen);     //完成应答处理
    int ReportTaskRecv(char *json);  //任务确认
    int SendExit();  //退出时发送给服务器退出信息
public:
    int InitServerInfo(ST_SERVER_INFO & stServerInfo);
    //采集设备信息
    int GetDevInfo();
    int TryConnect();
public:
    astPackage m_clsAstPackage;
    CAstClient m_clsAstClient;
    ST_SERVER_INFO m_stServerInfo;
    ST_DEV_INFO m_stDevInfo; //设备采集信息
    QString m_qsCurrentIp;
};

#endif // ASTSERVERCTRL_H

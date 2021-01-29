#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

#include "public/utils/file_util.h"
#include "public/utils/format_util.h"
#include "public/utils/command_util.h"
#include "public/cpu/cpu_info.h"

#define LSCPU_COMMAND "LANG=nl_NL.UTF-8 lscpu"
#define WHOB_COMMAND "who -b"
#define LSB_RELEASE "lsb_release -a"
#define LANG_COMMAND "echo $LANG"
#define INSTALL_COMMAND "ls / --full-time"

class SystemInfo
{
public:
    SystemInfo();

    QString getHostname() const;
    QString getPlatform() const;
    QString getDistribution() const;
    QString getKernel() const;
    QString getCpuModel() const;
    QString getCpuSpeed() const;
    QString getCpuCore() const;
    QString getUsername() const;

    QFileInfoList getCrashReports() const;
    QFileInfoList getAppLogs() const;
    QFileInfoList getAppCaches() const;

    QStringList getUserList() const;
    QStringList getGroupList() const;

    QString getLastUpTime();
    QString getOsVersion();
    QString getOsLanguage();
    QString getOsInstallTime();
    QString getOsArchitecture();

private slots:
public:
    QString cpuCore;
    QString cpuModel;
    QString cpuSpeed;
    QString username;
    QString lastUpTime;
    QString osVersion;  //系统版本
    QString osDescription; //系统描述
    QString osLanguage; //系统语言
    QString osInstallTime;  //系统安装时间
};

#endif // SYSTEMINFO_H

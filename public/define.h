#ifndef DEFINE_H
#define DEFINE_H
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
#include "public/cpu/cpu_info.h"
#include "public/memory/memory_info.h"
#include "public/disk/disk_info.h"
#include "public/system/system_info.h"
#include "public/network/network_info.h"

using namespace std;
#define IP_LEN  32
#define MAC_LEN  32
#define MASK_LEN  32
//主机ip、mac、子网
typedef  struct ip_mac
{
    char cIp[IP_LEN];
    char cMac[MAC_LEN];
    char cMask[MASK_LEN];

}ST_IP_MAC,*PST_IP_MAC;
//操作系统信息
typedef struct os_info
{
   string strOsName; //系统名称
   string strOsVersion;  //系统版本
   string strOsLanguage;  //系统语言
   string strLastUpTime;  //系统启动时间
   string strArchitecture;  //系统架构
   string strInstallTime;  //系统安装时间

}ST_OS_INFO, *PST_OS_INFO;
//cpu信息
typedef struct cpu_info
{
    string strProcessorId; //处理器编号
    string strVendorId;  //厂商
    string strCpuFamily;  //cpu类型
    string strModel;
    string strModelName;  //处理器名称
    uint64_t    nCpuMHZ;  //cpu主频
    uint64_t    nCacheSize; //缓存 单位KB
}ST_CPU_INFO,*PST_CPU_INFO;
//内存信息
typedef struct mem_info
{
    uint64_t nMemSize; //内存大小，单位字节
    uint64_t nUsedSize;  //已使用内存，单位字节
}ST_MEM_INFO,*PST_MEM_INFO;
//硬盘信息
typedef struct disk_info
{
    string strDiskId; //磁盘标识，sda,sdb
    string strCaption;  //磁盘名称
    string strType; //文件系统
    string strSerialNo;  //序列号
    string strManufauturer; //生产厂商
    uint64_t    nSize;  //大小
    string strHotPlug;  //是否是热插拔
}ST_DISK_INFO, *PST_DISK_INFO;
//逻辑分区信息
typedef struct logical_disk
{
    string strDeviceId;
    string strFileType;
    uint64_t    nSize;
    uint64_t    nFreeSize;
}ST_LOGICAL_DISK, *PST_LOGICAL_DISK;

//设备信息
typedef struct dev_info
{
    ST_IP_MAC ipmac;  //ip mac 子网
    string strUserName;  //用户名
    string strPcName; //主机名
    string strDomainName;  //域
    ST_OS_INFO stSystemInfo; //操作系统
    ST_CPU_INFO stCpuInfo;  //多处理器cpu
    ST_MEM_INFO stMemInfo; //内存信息
    vector<ST_DISK_INFO> vecDiskInfo;  //硬盘信息
    vector<ST_LOGICAL_DISK > vecLogicalDisk; //逻辑分区信息
}ST_DEV_INFO,*PST_DEV_INFO;


//服务器信息
typedef struct server_info{
    char cIp[IP_LEN];
    int nPort;
    int nTimeout;
    void init()
    {
        memset(cIp, 0x00, sizeof(char)*IP_LEN);
        nPort = 11000;
        nTimeout = 30000;
    }
}ST_SERVER_INFO, *PST_SERVER_INFO;


#endif // DEFINE_H

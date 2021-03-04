#ifndef ASTTASK_H
#define ASTTASK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <vector>
#include <queue>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
using namespace std;

//任务类型
enum
{
    TYPE_EXAM_HEALTH = 0, //健康检查
    TYPE_COMMAND = 1,     //指令下发/响应
    TYPE_SOFTWARE = 2,    //软件商店
    TYPE_NOTICE = 3,      //通知下发
    TYPE_PATCH = 4,       //补丁下发  //用于健康体检里的补丁检查
    TYPE_ASSISTANT = 5,   //小助手管理
    TYPE_PATCH_DOWN = 6,       //补丁分发   //用于下发并安装补丁
    TYPE_SOFTWARE_DELETE = 7,  //软件删除 7  删除任务2下发的软件
    TYPE_PATCH_DELETE = 8,     //补丁删除 8  删除任务4下发的补丁
    TYPE_TORRENT = 9,     //TORRENT文件下载

    TYPE_MSG_DELETE = 10, //通知删除  10 删除任务3下发的通知
    TYPE_DEV_DISCOVER = 11, //设备发现任务
};

//FTP
typedef struct task_resource_data
{
    std::string ftpsrv;   //ip:port
    std::string user;     //username
    std::string pass;     //password;
}TASK_RESOURCE_DATA;

typedef struct task_exam_data  //健康体检任务类
{
    std::string name;       //检查项名称
    std::string path;      //用逗号隔开多个path
    std::string desc;      //描述
    int   score;           //权重分数
    std::string method;   //对应json的type
    int   percent;   //百分比分数
    int   type;      //检查项类型 1-17 为目前检查项 //对应jison的 method
    std::string methodValue;  //阈值 对于病毒日志检测，存放最后检查时间，格式：年.月.日.时.分.秒,年.月.日.时.分.秒 分别对应OnAccessScanLog.txt 和 OnDemandScanLog.txt的最后检查时间
}TASK_EXAM_DATA;

typedef struct stExamRepair
{
    TASK_RESOURCE_DATA repairSource; //修复ftp源
    vector<std::string > vecRepairFiles; //修复文件
}ST_EXAMREPAIR;

//健康检查修复软件信息
typedef struct stExamRepairSoft
{
    int nMethod; //检查项索引
    std::string repairDesc; //修复说明
    std::string filePath; //文件绝对路径 ftp
    std::string fileName; //filePath对应的文件名
    std::string exeName; //如果fileName是可执行文件，fileName与exeName一致，如果fileName是压缩文件，则exeName是fileName解压后的可执行文件
    int nFileType;  //文件类型，[0:exe 1:zip]
    bool isInstall;
    std::string cmdParam; //执行参数
    TASK_RESOURCE_DATA repairSource; //修复ftp源
}ST_EXAMREPAIRSOFT;

//健康检查
class CTaskExam
{
public:
    CTaskExam(void);
    ~CTaskExam();
    void FinishInit();  //计算百分比分数
    int InsertItem(TASK_EXAM_DATA item);
    int InsertItem(char *name, char *path, char *method, int score, char *desc);
    int Begin();
    int Next();
    vector<TASK_EXAM_DATA>::iterator GetCurrent();
    vector<TASK_EXAM_DATA>::iterator m_iCurrent;

    vector<TASK_EXAM_DATA> m_CheckItemList;
};

//相应管理
enum
{
    TYPE_CMD_LOG = 0, //提取日志。
    TYPE_CMD_VIRUS = 1,     //提取病毒样本
    TYPE_CMD_AV = 2,    //加载专杀病毒库
    TYPE_CMD_SCRIPT = 3,       //执行脚本
    TYPE_CMD_SOFT = 4,      //软件分发
    TYPE_CMD_IP = 5,   //隔离终端（IP访问控制）。
#ifdef PORT_CONTROL
    TYPE_CMD_PORT = 6,   //端口控制）。 //rejectPort
#endif
};

typedef struct task_cmd_data      //响应处置
{
    //是否进行BT下载
    bool btCmdFlag; //false表示不进行bt下载
    int responseType;    //    private String responseType;
    std::string cmd;    //指令
    TASK_RESOURCE_DATA resource; //工具来源FTP
    std::string path;   //工具位置
    TASK_RESOURCE_DATA result;   //结果FTP
    std::string resPath;
}TASK_CMD_DATA;

class CTaskCmd
{
public:
    CTaskCmd(void) { };
    ~CTaskCmd(){};
    TASK_CMD_DATA m_Cmd;

    static int GetType(std::string strjson);
};

//TYPE_SOFTWARE = 2,    //软件商店///////////////////////////////
typedef struct task_soft_data    //软件商店
{
    bool btSoftFlag; //判断是否进行bt下载
    std::string sTid; //任务id
    std::string  name;  //文件名
    std::string  torrentname; //新增代码 torrentname
    TASK_RESOURCE_DATA resource; //文件来源FTP
    std::string path;
    //std::string  filename;  //文件名
    //std::string  path;      //FTP 文件路径
    int     size;      //文件大小;
    int     speed;     //下载速度
    std::string  desc;      //描述
    std::string strRecvTime; //软件结束时间
}TASK_SOFT_DATA;

//工具商店
class CTaskSoft
{
public:
    CTaskSoft(void){};
    ~CTaskSoft(){};
    vector<TASK_SOFT_DATA> m_SoftList;
    vector<TASK_SOFT_DATA> m_SingleSoftList; //查询安装列表

    vector<std::string> m_delSoftList;

    int DeleteSoft(vector<std::string> dellist);  //删除软件

    int AddSoft(TASK_SOFT_DATA &soft);   //添加
    int SaveFile(char *filename);  //保存到软件列表文件
    int LoadFile(char *filename);  //读取软件列表文件
    void Lock();
    void UnLock();

private:

    bool m_bSoftLock;  //互斥锁， 互斥对m_SoftList 的操作
    int InsertSoft(TASK_SOFT_DATA &soft);   //插入
    TASK_SOFT_DATA *FindSoft(char *name);  //查找软件
};

//TYPE_NOTICE = 3,      //通知下发
typedef struct task_message_data  //消息通知
{
    std::string content;    //消息内容
    int type;
}TASK_MESSAGE_DATA;
class CTaskNotice
{
public:
    CTaskNotice(void){};
    ~CTaskNotice(){};
    TASK_MESSAGE_DATA m_Notice;
};

//TYPE_PATCH = 4,       //补丁下发
typedef struct task_patch_data  //补丁
{
    bool btPatchFlag;       //判断是否进行BT下载
    std::string name;       //文件名
    std::string  torrentname; //新增代码 torrentname
    std::string kbnumber;   //KB号
    TASK_RESOURCE_DATA resource; //文件来源FTP
    std::string path;  //保存补丁的patchId，取消保存补丁的路径
    //std::string filename;   //文件名
    //std::string path;       //FTP 文件路径
    int    size;       //文件大小;
    int    speed;      //下载速度
    int    status;     //是否安装，0未安装 1已安装 -1未知  2 删除
    std::string desc;       //描述
}TASK_PATCH_DATA;

class CTaskPatch
{
public:
    CTaskPatch(void){m_bPatchLock = false;};
    ~CTaskPatch(){};

    vector<TASK_PATCH_DATA> m_PatchList;
    vector<std::string>  m_LocalPList;   //本机补丁KB列表
    vector<std::string > m_NeedPList;  //需要检查的补丁列表
    vector<std::string > m_HassNoPList;  //检测缺少的补丁列表
    int m_NoUse;
    void Clear();
    int InsertPatch(TASK_PATCH_DATA &patch);
    TASK_PATCH_DATA *FindPatch(char *kb);  //查找补丁
    int AddPatch(TASK_PATCH_DATA &patch);   //添加补丁
    int DeletePatch(vector<std::string> dellist);  //删除补丁 先打标记再删除

    int CheckNoUse();
    int CheckHotFix(vector<std::string> &localkb);   //检查本地补丁
    int CheckHotFixNoLock(vector<std::string> &localkb);   //检查本地补丁
    int GetNoUse(vector<std::string> &list_kb);  //获取未装的kb
    int SaveFile(char *filename);  //保存到补丁列表文件
    int LoadFile(char *filename);  //读取补丁列表文件
    void Lock();
    void UnLock();
    bool m_bPatchLock;
    int RemoveDelPatch();   //将删除标记的补丁移出补丁列表
    int GetNeedKb(/*vector<std::string> &list_kb*/);
};

//TYPE_ASSISTANT = 5,   //小助手管理
//String UPGRADE = "upgrade";
//String RESTART = "restart";
//String STOP = "stop";
//String UNINSTALL = "uninstall";
typedef struct task_ass_data  //
{
    bool btAssFlag;   //判断是否进行BT下载
    std::string torrentname; //torrentname,bt下载对应的torrent文件
    std::string astType;       //类型
    TASK_RESOURCE_DATA resource; //文件来源FTP
    std::string resourcePath;   //resourcePath  FTP path  “”
    std::string name;  //  文件名
    std::string strAstVersion; //升级包版本号
}TASK_ASS_DATA;
class CTaskAss
{
public:
    CTaskAss(void){};
    ~CTaskAss(){};

    TASK_ASS_DATA m_AssData;
};

typedef struct task_dev_discover
{
    string m_strExecParam;
    TASK_RESOURCE_DATA resource; //文件来源FTP
    std::string sFtpPath;   //resourcePath  FTP path  “”
}TASK_DEV_DISCOVER, * PTASK_DEV_DISCOVER;

//设备发现
class CDeviceDiscover
{
public:
    CDeviceDiscover() {};
    ~CDeviceDiscover() {};
public:
    TASK_DEV_DISCOVER m_stDev_Discover;
};

//助手日志清理以及任务执行策略
typedef struct agentStrategy
{
    int nCpuRest; //cup使用阈值(剩余值)
    bool bCpuValid; //cpu判断条件有效
    int nRamMemoryRest; //内存使用阈值(剩余值)
    bool bRamMemoryValid; //内存判断条件有效
    int nDiskRest; //硬盘使用阈值(剩余值)
    bool bDiskValid; //磁盘判断条件有效
    int nClearDays; //补丁，工具，清理周期(天)
    int nClearType; //全部清理0，已安装清理1
    void init()
    {
        nCpuRest = 10;
        bCpuValid = true;
        nRamMemoryRest = 10;
        bRamMemoryValid = true;
        nDiskRest = 10;
        bDiskValid = true;
        nClearDays = 30;
        nClearType = 0;
    }
}AGNETSTRATEGY, *PAGENTSTRATEGY;

//通知管理显示配置属性
typedef struct msgConfigIni
{
    long lDlgFrequency; //弹出频率
    int nFontSize;  //字体大小
    std::string strFontColor; //字体颜色
    bool bAutoClose; //是否自动关闭
    int nShowPosition; //显示位置，[1：居中，2：右下角]
    int nShowCount; //显示次数 -1不限次数
}MSGCONFIGINI, *PMSGCONFIGINI;

//WSUS配置信息
typedef struct wsusconfig
{
    string strWsusIp;    //WSUS IP
    string strWsusPort;  //WSUS端口
    void init()
    {
        strWsusIp = "";
        strWsusPort = "";
    }
}WSUS_CONFIG;

//种子文件下载配置信息
typedef struct torrentconfig
{
    int nSeedSpeed;  //种子服务器上传速度
    int nSingleOriginalSpeed;  //单个原始种子服务器的下载速度
    int nSingleSeedSpeed;  //单个终端的下载速度

    bool bSeedRateLimit; //种子服务器上传是否限速
    bool bSingleOriginalRateLimit; //单个原始种子服务器的下载是否限速
    bool bSingleSeedRateLimit;  //单个终端的下载是否限速

    string strStartTime;  //种子服务器定时下载时间
    bool bTimeLimit;  //下载时间是否会限制

    int nOtherTransfer;  //异常情况，是否使用FTP下载
    int nIsSeed;  //是否作为种子服务器[0 不是，1 是]
    void init()
    {
        nSeedSpeed = -1;
        nSingleOriginalSpeed = -1;
        nSingleSeedSpeed = -1;
        strStartTime = "";
        nOtherTransfer = 0;
        nIsSeed = 0;
        bSeedRateLimit = false;
        bSingleOriginalRateLimit = false;
        bSingleSeedRateLimit = false;
        bTimeLimit = false;
    }
}TORRENT_CONFIG;

/////////////////任务处理类//////////////////////////
typedef struct taskhead
{
    char tid[32];  //任务ID  //=json:tid
    int  cycle;    //0永久任务，>0执行次数  =1 立即执行 //=json:single
    int  type;     //任务类型  //=json:type  【TYPE_COMMAND TYPE_SOFTWARE TYPE_PATCH	TYPE_ASSISTANT TYPE_PATCH_DOWN】
    char lasttime[24];   //上次检查时间 YYYY-MM-DD-hh-mm-ss
    int  invalid;      //是否有效标识，0有效，1禁用，2删除
    int ftime;    //下发时间
    struct _atpr
    {
        int ref;     //task 引用计数
        void *task;  //指向任务对象指针，是下面的几种类型之一
    } *atpr;

    taskhead()
    {
        tid[0]='\0';
        cycle = 0;
        type = 0;
        snprintf(lasttime, 23, "0000-00-00-00-00-00");
        invalid = 0;
        ftime = 0;
        //task = NULL;
        atpr = new _atpr;
        atpr->ref = 1;
        atpr->task = NULL;
    }
    taskhead(char *ntid, int ncycle, int ntype):cycle(ncycle),type(ntype)
    {
        memcpy(tid, ntid, 30);
        //strcpy(tid, 30, ntid);  //strncpy(tid, ntid, 30);
        snprintf(lasttime, 23, "0000-00-00-00-00-00");  //sprintf_s(lasttime, "0000-00-00-00-00-00");
        invalid = 0;
        ftime = 0;
        //task = NULL;
        atpr = new _atpr;
        atpr->ref = 1;
        atpr->task = NULL;
    }
    taskhead(const taskhead &old)
    {
//        if(old.atpr == NULL)
//        {
//            old.atpr = new _atpr;
//            old.atpr->ref = 1;
//            old.atpr->task = NULL;
//        }
        atpr = old.atpr;
        atpr->ref++;
        memcpy(tid, old.tid, 30);
        //strcpy_s(tid, 30, old.tid);
        cycle = old.cycle;
        type = old.type;
        memcpy(lasttime, old.lasttime,23);
        invalid = old.invalid;
        ftime = old.ftime;
    }
    ~taskhead()
    {
        if(atpr != NULL)
        {
            atpr->ref--;
            if(atpr->ref < 0)
            {
                //printf("Error!\n");
                atpr->ref = 0;
            }
            if(atpr->ref <= 0)
            {
                if(atpr->task != NULL)
                    delete atpr->task;
                delete atpr;
            }
        }
    }

    void operator=(const taskhead & old )
    {
//        if(old.atpr == NULL)
//        {
//            old.atpr = new _atpr;
//            old.atpr->ref = 1;
//            old.atpr->task = NULL;
//        }
        if(atpr != NULL) //旧有链接释放
        {
            atpr->ref--;
            if(atpr->ref <= 0)
            {
                if(atpr->task != NULL)
                    delete atpr->task;
                delete atpr;
            }
        }
        atpr = old.atpr;
        atpr->ref++;

        memcpy(tid, old.tid, 30);
        //strcpy_s(tid, 30, old.tid);
        cycle = old.cycle;
        type = old.type;
        memcpy(lasttime, old.lasttime, 23);
        invalid = old.invalid;
        ftime = old.ftime;
    }
    void SetTime()
    {
        char cTime[100]={0x00};
        struct timeval curTimeval;
        struct timezone curTimezone;
        gettimeofday(&curTimeval, &curTimezone);

        struct tm *pCurTime;
        pCurTime = localtime(&curTimeval.tv_sec);
        sprintf(lasttime, "%4d-%02d-%02d %02d:%02d:%02d", 1900+pCurTime->tm_year,pCurTime->tm_mon+1,pCurTime->tm_mday,pCurTime->tm_hour,pCurTime->tm_min,pCurTime->tm_sec);

    }

}TASKHEAD;

//任务列表
class CTaskList
{
public:
    CTaskList(void);
    ~CTaskList();
    vector<TASKHEAD>  m_TaskList;
    vector<TASKHEAD>::iterator m_iCurrent;
    int AddTask(TASKHEAD & task);
    int DelTask(char *taskid = NULL);
    TASKHEAD *FindTask(char *taskid);

    int Save(char *filename);
    int Load(char *filename);  //读取文件

    int Begin();
    int Next();
    int IsEmpty();
    void TaskClear();
    TASKHEAD *FindFirst(int type);  //查找某类型任务 //体检
    int InvalidTask(int type);      //禁用某类任务 //体检
    vector<TASKHEAD>::iterator GetCurrent();

    int Copy(vector<TASKHEAD> &out);

private:
    bool m_bLock;
    int SaveTask(FILE *fp, TASKHEAD *node);
    int LoadTask(FILE *fp, TASKHEAD *node);
    int SaveExamTask(FILE *fp, TASKHEAD *node);
    int LoadExamTask(FILE *fp, TASKHEAD *node);

    int Lock(int timeouts);  //多线程对此队列的操作
    void UnLock();

    int _AddTask(TASKHEAD & task);
};

#endif // ASTTASK_H

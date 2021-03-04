#include "./astTask.h"

//返回响应管理的类型
int CTaskCmd::GetType(std::string strjson)
{
    if(strjson.compare("extractLog") == 0)
        return TYPE_CMD_LOG;
    if(strjson.compare("virusSamples") == 0)
        return TYPE_CMD_VIRUS;
    if(strjson.compare("virusDatabase") == 0)
        return TYPE_CMD_AV;
    if(strjson.compare("executionScript") == 0)
        return TYPE_CMD_SCRIPT;
#ifdef PORT_CONTROL
    if (strjson.compare("rejectPort") == 0)
        return TYPE_CMD_PORT;
#endif
    return TYPE_CMD_LOG;
}

/////Exam///////////////////////////////////////////////////////////////////////
CTaskExam::CTaskExam()
{
    m_CheckItemList.clear();
}
CTaskExam::~CTaskExam()
{
}

int CTaskExam::InsertItem(TASK_EXAM_DATA item)
{
    m_CheckItemList.insert(m_CheckItemList.end(), item);
    return 0;
}
int CTaskExam::InsertItem(char *name, char *path, char *method, int score, char *desc)
{
    TASK_EXAM_DATA item;
    item.name = name;
    item.path = path;
    item.method = method;
    item.score = score;
    item.desc = desc;
    item.percent = 0;
    return InsertItem(item);
}

void CTaskExam::FinishInit()  //根据score计算百分比分数
{
    int nTotal = 0;
    int nC = 0;

    for ( vector<TASK_EXAM_DATA>::iterator it = m_CheckItemList.begin(); it != m_CheckItemList.end(); ++it )
        nTotal += (*it).score;
    for ( vector<TASK_EXAM_DATA>::iterator it = m_CheckItemList.begin(); it != m_CheckItemList.end(); ++it )
    {
        double dP = (double)(*it).score*100/(double)nTotal;
        (*it).percent = (int)dP;
        if((*it).percent == 0)  //如果0分就设成1分
            (*it).percent++;
        nC += (*it).percent;
    }
    if(nC != 100) //总和不是100分，调整最大分值的项。
    {
//printf("nC=%d\n", nC);
        int nPad = 100 - nC;
        int nLoop = nPad;
        if(nPad < 0)
            nLoop = 0-nPad;
        vector<int> vmax;
        for(vector<TASK_EXAM_DATA>::iterator it = m_CheckItemList.begin(); it != m_CheckItemList.end(); ++it)
        {
            int off = it-m_CheckItemList.begin();
            vector<int>::iterator ni;
            for(ni = vmax.begin(); ni != vmax.end(); ++ni) //从大到小排序，把分值最大的项排在前面
            {
                if((*it).score > (*(m_CheckItemList.begin()+*ni)).score)
                    break;
            }
            vmax.insert(ni, off);
        }
        vector<TASK_EXAM_DATA>::iterator ichange = m_CheckItemList.begin();
        for(vector<int>::iterator itt = vmax.begin(); itt!=vmax.end() && nLoop>0; ++itt, nLoop--) //从大到小调整分
        {
            if(nPad < 0)
                (*(ichange+*itt)).percent--;
            else
                (*(ichange+*itt)).percent++;
        }
    }
    m_iCurrent = m_CheckItemList.begin();
    return;
}
int CTaskExam::Begin()
{
    m_iCurrent = m_CheckItemList.begin();
    if(m_CheckItemList.begin() == m_CheckItemList.end())
        return 1;  //空
    return 0;
}

int CTaskExam::Next()
{
    if(m_iCurrent != m_CheckItemList.end())
        m_iCurrent++;
    if(m_iCurrent == m_CheckItemList.end())
        return 1;
    return 0;
}
vector<TASK_EXAM_DATA>::iterator CTaskExam::GetCurrent()
{
    return m_iCurrent;
}
//////End Exam//////////////////////////////////////////////////////////////////////////

////////////////Soft/////////////////////////////////////////////////
void CTaskSoft::Lock()
{
    while(m_bSoftLock)
        sleep(1);
    m_bSoftLock = true;
}
void CTaskSoft::UnLock()
{
    m_bSoftLock = false;
}
int CTaskSoft::DeleteSoft(vector<std::string> dellist)  //删除软件
{
    int nReturn = 0;
    Lock();
    for(vector<std::string>::iterator delit=dellist.begin(); delit != dellist.end(); delit++)
    {
        for(vector<TASK_SOFT_DATA>::iterator it=m_SoftList.begin(); it != m_SoftList.end(); it++)
        {
            if((*delit).compare((*it).name.c_str()) == 0)
            {
                m_SoftList.erase(it);
                break;
            }
        }
    }
    UnLock();
    return nReturn;
}

int CTaskSoft::AddSoft(TASK_SOFT_DATA &soft)   //添加
{
    Lock();
    TASK_SOFT_DATA *p = FindSoft((char *)soft.name.c_str());
    if(p != NULL)
    {
        p->desc = soft.desc;
        p->path = soft.path;
        p->size = soft.size;
        p->speed = soft.speed;
        p->resource.ftpsrv = soft.resource.ftpsrv;
        p->resource.pass = soft.resource.pass;
        p->resource.user = soft.resource.user;
        UnLock();
        return 1;  //文件名存在
    }
    int nReturn = InsertSoft(soft);
    UnLock();
    return nReturn;
}
int CTaskSoft::InsertSoft(TASK_SOFT_DATA &soft)   //插入
{
    m_SoftList.insert(m_SoftList.end(), soft);
    return 0;
}
TASK_SOFT_DATA *CTaskSoft::FindSoft(char *name) //查找软件
{
    for(vector<TASK_SOFT_DATA>::iterator it=m_SoftList.begin(); it != m_SoftList.end(); it++)
    {
        if((*it).name.compare(name) == 0)
            return &(*it);
    }
    return NULL;
}
////////////////end soft////////////////////////////////////////////////////
////////////////patch////////////////////////////////////////////////////////
int CTaskPatch::InsertPatch(TASK_PATCH_DATA &patch)
{
    m_PatchList.insert(m_PatchList.end(), patch);
    return 0;
}
int CTaskPatch::CheckNoUse()
{
    m_NoUse = 0;
    for(vector<TASK_PATCH_DATA>::iterator it=m_PatchList.begin(); it != m_PatchList.end(); it++)
    {
        if((*it).status == 0)
            m_NoUse++;
    }
    return m_NoUse;
}
int CTaskPatch::CheckHotFixNoLock(vector<std::string> &localkb)
{
    for(vector<TASK_PATCH_DATA>::iterator it=m_PatchList.begin(); it != m_PatchList.end(); it++)
    {
        if((*it).status < 0)  //初始化的改变状态
            (*it).status = 0;
    }
    for(vector<std::string>::iterator it = localkb.begin(); it != localkb.end(); it++)
    {
        std::string stmp = *it;
        TASK_PATCH_DATA *pP = FindPatch((char *)stmp.c_str());
            if(pP != NULL)
                pP->status = 1;
    }
    return 0;
}

int CTaskPatch::CheckHotFix(vector<std::string> &localkb)
{
    Lock();
    for(vector<TASK_PATCH_DATA>::iterator it=m_PatchList.begin(); it != m_PatchList.end(); it++)
    {
        if((*it).status < 0)  //初始化的改变状态
            (*it).status = 0;
    }
    for(vector<std::string>::iterator it = localkb.begin(); it != localkb.end(); it++)
    {
        std::string stmp = *it;
        TASK_PATCH_DATA *pP = FindPatch((char *)stmp.c_str());
            if(pP != NULL)
                pP->status = 1;
    }
    UnLock();
    return 0;
}

int CTaskPatch::GetNoUse(vector<std::string> &list_kb)
{
    m_NoUse = 0;
    for(vector<TASK_PATCH_DATA>::iterator it=m_PatchList.begin(); it != m_PatchList.end(); it++)
    {
        if((*it).status == 0)
        {
            list_kb.insert(list_kb.end(), (*it).kbnumber);
            m_NoUse++;
        }
    }
    return m_NoUse;
}

int CTaskPatch::GetNeedKb(/*vector<std::string> &list_kb*/)
{
    int nNeed = 0;
    bool bFind = false;
    for (vector<string >::iterator iNeed = m_NeedPList.begin(); iNeed != m_NeedPList.end(); iNeed++)
    {
        bFind = false;
        for (vector<string >::iterator iLocal = m_LocalPList.begin(); iLocal != m_LocalPList.end(); iLocal++)
        {
            if (*iNeed == *iLocal)
            {
                bFind = true;
                break;
            }
        }
        if (!bFind) //已安装列表中未找到
        {
            m_HassNoPList.push_back(*iNeed);
            //list_kb.push_back(*iNeed);
            nNeed++;
        }
    }
    return nNeed;
}
void CTaskPatch::Lock()
{
    while(m_bPatchLock)
        sleep(1);
    m_bPatchLock = true;
}
void CTaskPatch::UnLock()
{
    m_bPatchLock = false;
}
void CTaskPatch::Clear()
{
    Lock();
    m_PatchList.clear();
    UnLock();
}
int CTaskPatch::DeletePatch(vector<std::string> dellist)
{
    Lock();
    for(vector<std::string>::iterator it = dellist.begin(); it != dellist.end(); it++)
    {
        std::string stmp = *it;
        TASK_PATCH_DATA *pP = FindPatch((char *)stmp.c_str());
            if(pP != NULL)
                pP->status = 2;	 //删除标记
    }
    RemoveDelPatch();
    UnLock();
    return 0;
}
int CTaskPatch::RemoveDelPatch()
{
    for(int i = m_PatchList.size()-1; i>=0; i--)
    {
        if(m_PatchList[i].status == 2)
            m_PatchList.erase(m_PatchList.begin()+i);
    }
    return 0;
}


int CTaskPatch::AddPatch(TASK_PATCH_DATA &patch)   //添加补丁
{
    Lock();
    TASK_PATCH_DATA *p = FindPatch((char *)patch.kbnumber.c_str());
    if(p != NULL)
    {
        p->name = patch.name;
        p->desc = patch.desc;
        p->path = patch.path;
        p->size = patch.size;
        p->speed = patch.speed;
        p->status = patch.status;
        p->resource.ftpsrv = patch.resource.ftpsrv;
        p->resource.user = patch.resource.user;
        p->resource.pass = patch.resource.pass;
        UnLock();
        return 1;  //KB 存在
    }
    int nReturn = InsertPatch(patch);
    UnLock();
    return nReturn;
}
TASK_PATCH_DATA *CTaskPatch::FindPatch(char *kb)  //查找补丁
{
    for(vector<TASK_PATCH_DATA>::iterator it=m_PatchList.begin(); it != m_PatchList.end(); it++)
    {
        if((*it).kbnumber.compare(kb) == 0)
            return &(*it);
    }
    return NULL;
}

////////////////end patch////////////////////////////////////////////////////

////////////////Task list/////////////////////////////////////////////////////////
CTaskList::CTaskList(void)
{
    m_bLock = false;
}
CTaskList::~CTaskList()
{
}

int CTaskList::Begin()
{
    m_iCurrent = m_TaskList.begin();
    if(m_iCurrent != m_TaskList.end())
        return 0;
    else
        return 1;
}
int CTaskList::Next()
{
    if(m_iCurrent == m_TaskList.end())
        return 2;
    m_iCurrent++;
    if(m_iCurrent == m_TaskList.end())
        return 1;
    return 0;
}
vector<TASKHEAD>::iterator CTaskList::GetCurrent()
{
    return m_iCurrent;
}
int CTaskList::IsEmpty()
{
    return m_TaskList.empty();
}
void CTaskList::TaskClear()
{
    m_TaskList.clear();
    m_iCurrent = m_TaskList.begin();
}

int CTaskList::AddTask(TASKHEAD & task)
{
    int nError = Lock(60);
    if(nError != 0)
        return 3;
    nError = _AddTask(task);
    UnLock();
    return nError;
}
int CTaskList::Copy(vector<TASKHEAD> &out)
{
    int nError = Lock(60);
    if(nError != 0)
        return 3;
    out.clear();
    for(vector<TASKHEAD>::iterator it = m_TaskList.begin(); it!=m_TaskList.end(); it++)
    {
        out.insert(out.end(), (TASKHEAD)*it);
    }
    UnLock();
    return nError;
}
int CTaskList::DelTask(char *taskid) //删除后m_pCurrent 指向下一个节点，如果删除尾节点，则指向头节点。删空则返回1
{
    int nReturn = 0;
    vector<TASKHEAD>::iterator it;
    if(taskid == NULL)
    {
        return 1;
    }
    int nError = Lock(60);
    if(nError != 0)
        return 3;
    for(it=m_TaskList.begin(); it!=m_TaskList.end(); it++)
    {
        if(strcmp(taskid, (*it).tid) == 0)
            break;
    }
    if(it != m_TaskList.end())
    {
        bool bEnd = true;
        if((it+1) != m_TaskList.end())
        {
            //m_iCurrent = it+1;
            bEnd = false;
        }
        m_TaskList.erase(it);
        if(bEnd)
            m_iCurrent = m_TaskList.begin();
    }
    if(m_TaskList.begin() == m_TaskList.end())
        nReturn = 1;
    UnLock();
    return nReturn;
}
TASKHEAD *CTaskList::FindTask(char *taskid)
{
    vector<TASKHEAD>::iterator it;
    TASKHEAD *pRet = NULL;
    for(it=m_TaskList.begin(); it!=m_TaskList.end(); it++)
    {
        if(strcmp(taskid, (*it).tid) == 0)
        {
            pRet = &(*it);
            break;
        }
    }
    return pRet;
}
TASKHEAD *CTaskList::FindFirst(int type)
{
    vector<TASKHEAD>::iterator it;
    TASKHEAD *pRet = NULL;
    for(it=m_TaskList.begin(); it!=m_TaskList.end(); it++)
    {
        if((*it).type == type && (*it).invalid == 0) //判断健康检查任务是否有效
        {
            pRet = &(*it);
            break;
        }
    }
    return pRet;
}
int CTaskList::InvalidTask(int type)
{
    vector<TASKHEAD>::iterator it;
    int nReturn = 0;
    for(it=m_TaskList.begin(); it!=m_TaskList.end(); it++)
    {
        if((*it).type == type)
        {
            (*it).invalid = 1;
            nReturn++;
        }
    }
    return nReturn;
}
/////////Save and Load /////////////////////////////////
int CTaskList::Save(char *filename)
{
    //if(m_TaskList.begin() == m_TaskList.end())  //没有任务
    //	return 1;
    int nLock = Lock(60); //60次50毫秒
    if(nLock != 0)
        return 3;
    FILE *fp = NULL;
    fp = fopen(filename, "wt");
    if(fp == NULL)
    {
        UnLock();
        return 2;
    }
    int nTaskCount = 0;
    for (vector<TASKHEAD>::iterator itTask = m_TaskList.begin(); itTask != m_TaskList.end(); itTask++)
    {
        if ((itTask->invalid == 0) && (itTask->type == TYPE_EXAM_HEALTH)) //只保留健康检查任务
        {
            nTaskCount++;
        }
    }
    fprintf(fp, "%d\n", nTaskCount);
    for(vector<TASKHEAD>::iterator it=m_TaskList.begin(); it!=m_TaskList.end(); it++)
    {
        //只保留任务状态为0的任务 只保留健康检查任务
        if (((*it).invalid == 0) && ((*it).type == TYPE_EXAM_HEALTH))
            SaveTask(fp, &(*it));
    }
    fclose(fp);
    UnLock();
    return 0;
}

int CTaskList::Load(char *filename)  //读取文件
{
    int nError;
    int nLock = Lock(60);
    if(nLock != 0)
        return 3;
    FILE *fp = NULL;
    fp = fopen(filename, "rt");
    if(fp == NULL)
    {
        UnLock();
        return 1;
    }
    if(m_TaskList.size() != 0)
    {
        m_TaskList.clear();
//		_freeAll(); //清除全部动态分配的数据节点
    }
    int nNum;
    fscanf(fp, "%d\n", &nNum);

    for(int i = 0; i < nNum; )
    {
        TASKHEAD iTask;
        nError = LoadTask(fp, &iTask);
//printf("Load Task! (%d)\n", time(NULL));
        if(nError < 0)
            break;
        if(nError != 0)
            continue;
        _AddTask(iTask);
        i++;
//printf("Add Task! (%d)\n", time(NULL));
    }

    fclose(fp);
    UnLock();
//printf("Load End! (%d)\n", time(NULL));
    return 0;
}

//private//////////////////////////////////////////////////////////////////
int CTaskList::_AddTask(TASKHEAD & task)
{
#if 0
    if(NULL==FindTask(task.tid))  //没有同tid的任务
        m_TaskList.insert(m_TaskList.end(), task);
    else
        return 1;
#endif
    vector<TASKHEAD>::iterator it;
    TASKHEAD *pRet = NULL;
    for(it=m_TaskList.begin(); it!=m_TaskList.end(); it++)
    {
        if ((strcmp(task.tid, (*it).tid) == 0) || ((*it).invalid == 1)) //清除任务invalid为1的任务或者已存在的任务
        {
            m_TaskList.erase(it);  //删除原来的，更新新任务
            break;
        }
    }

    m_TaskList.insert(m_TaskList.end(), (TASKHEAD)task);
    m_iCurrent = m_TaskList.begin();
    return 0;
}

int CTaskList::Lock(int timeouts)
{
    if(timeouts == 0) //永久
    {
        while(m_bLock);
        m_bLock = true;
        return 0;
    }
    else
    {
        for(int i = 0; i < timeouts; i++)
        {
            if(!m_bLock)
            {
                m_bLock = true;
                return 0;
            }
            sleep(1);
        }
    }
    return 1;
}
void CTaskList::UnLock()
{
    m_bLock = false;
}

bool checkTid(char *tid)
{
    int nLen = strlen(tid);
    if(nLen < 10)
        return false;
    for(int i = 0; i < nLen; i++)
    {
        if(tid[i]<'0' || tid[i]>'f')
            return false;
        if(tid[i]>'9' && tid[i]<'A')
            return false;
        if(tid[i]>'F' && tid[i]<'a')
            return false;
    }
    return true;
}

int CmpVersion(char *ver1, char *ver2)
{
    int nReturn = 0;
    char *p1 = ver1;
    char *p2 = ver2;
    char cTmp1[256];
    char cTmp2[256];
    char *pT1 = cTmp1;
    char *pT2 = cTmp2;
    for(;*p1!='\0' || *p2!='\0';)
    {
        if(*p1 == '.')
            *pT1 = '\0';
        else
            *pT1 = *p1;
        if(*p2 == '.')
            *pT2 = '\0';
        else
            *pT2 = *p2;
        if(*pT1=='\0' && *pT2=='\0') //比较
        {
            nReturn = atoi(cTmp1)-atoi(cTmp2);
            if(nReturn != 0)
                return nReturn;
            if(*p1 != '\0')
                p1++;
            if(*p2 != '\0')
                p2++;
            pT1 = cTmp1;
            pT2 = cTmp2;
        }
        else
        {
            if(*pT1!='\0')
            {
                p1++;
                pT1++;
            }
            if(*pT2!='\0')
            {
                p2++;
                pT2++;
            }
        }
    }
    *pT1 = *p1;
    *pT2 = *p2;
    nReturn = atoi(cTmp1)-atoi(cTmp2);
    return nReturn;
}
int CTaskList::SaveTask(FILE *fp, TASKHEAD *node)
{
    if(fp==NULL || node==NULL)
        return 1;
    switch(node->type)
    {
    case TYPE_EXAM_HEALTH: //0健康检查
        SaveExamTask(fp, node);
        break;
    default:
        break;
    }
    return 0;
}

int CTaskList::LoadTask(FILE *fp, TASKHEAD *node)
{
    int nError = 0;
    nError = fscanf(fp, "%s\n", node->tid);			//fprintf(fp, "%s\n", node->tid);
    if(nError < 0)//失败返回EOF(-1)
        return nError;
    if(!checkTid(node->tid))//验证数据是不是再十六进制范围内
        return 1;
    nError = fscanf(fp, "%d\n", &node->cycle);		//fprintf(fp, "%d\n", node->cycle);
    nError = fscanf(fp, "%d\n", &node->type);		//fprintf(fp, "%d\n", node->type);
    nError = fscanf(fp, "%s\n", node->lasttime);		//fprintf(fp, "%s\n", node->lasttime);
    nError = fscanf(fp, "%d\n", &node->invalid);		//fprintf(fp, "%d\n", node->invalid);
    nError = fscanf(fp, "%d\n", &node->ftime);
    if(node->invalid == 3)  //如果是执行中的状态
        node->invalid = 0;
    switch(node->type)
    {
    case TYPE_EXAM_HEALTH: //0健康检查
        LoadExamTask(fp, node);
        break;
    default:
        break;
    }

    return 0;
}
int CTaskList::SaveExamTask(FILE *fp, TASKHEAD *node)
{
    if (node->invalid == 0)
    {
        fprintf(fp, "%s\n", node->tid);
        fprintf(fp, "%d\n", node->cycle);
        fprintf(fp, "%d\n", node->type);
        fprintf(fp, "%s\n", node->lasttime);
        fprintf(fp, "%d\n", node->invalid);
        fprintf(fp, "%d\n", node->ftime);

        CTaskExam *pitem = (CTaskExam *)node->atpr->task;
        fprintf(fp, "%d\n", pitem->m_CheckItemList.size());
        for (vector<TASK_EXAM_DATA>::iterator it = pitem->m_CheckItemList.begin(); it != pitem->m_CheckItemList.end(); ++it)
        {
            fprintf(fp, "%s\n", (*it).name.c_str());	//fprintf(fp, "%s\n", pTask->name);
            fprintf(fp, "%s\n", (*it).path.c_str());	//fprintf(fp, "%s\n", pTask->path);
            fprintf(fp, "%s\n", (*it).desc.c_str());	//fprintf(fp, "%s\n", pTask->desc);
            fprintf(fp, "%d\n", (*it).score);			//fprintf(fp, "%d\n", pTask->score);
            fprintf(fp, "%s\n", (*it).method.c_str());	//fprintf(fp, "%s\n", pTask->method);
            fprintf(fp, "%d\n", (*it).percent);
            fprintf(fp, "%d\n", (*it).type);
            fprintf(fp, "%s\n", (*it).methodValue.c_str());
        }
    }

    return 0;
}
int CTaskList::LoadExamTask(FILE *fp, TASKHEAD *node)
{
    int nNum = 0;
    fscanf(fp, "%d\n", &nNum);  //fprintf(fp, "%d\n", pitem->m_CheckItemList.size());
    char cTmp[1024];
    CTaskExam *pitem = new CTaskExam;
    node->atpr->task = (void *)pitem;
    for(int i = 0; i < nNum; i++)
    {
        TASK_EXAM_DATA item;
        fgets(cTmp, 1024, fp);  //fprintf(fp, "%s\n", (*it).name.c_str());	//fprintf(fp, "%s\n", pTask->name);
        cTmp[strlen(cTmp)-1] = '\0';
        item.name = cTmp; //检查项名
        fgets(cTmp, 1024, fp);  //fprintf(fp, "%s\n", (*it).path.c_str());	//fprintf(fp, "%s\n", pTask->path);
        cTmp[strlen(cTmp)-1] = '\0';
        item.path = cTmp; //路径
        fgets(cTmp, 1024, fp);  //fprintf(fp, "%s\n", (*it).desc.c_str());	//fprintf(fp, "%s\n", pTask->desc);
        cTmp[strlen(cTmp)-1] = '\0';
        item.desc = cTmp; //描述
        fgets(cTmp, 1024, fp);//fprintf(fp, "%d\n",	(*it).score);			//fprintf(fp, "%d\n", pTask->score);
        cTmp[strlen(cTmp)-1] = '\0';
        item.score = atoi(cTmp); //分数
        fgets(cTmp, 1024, fp);  //fprintf(fp, "%s\n", (*it).method.c_str());	//fprintf(fp, "%s\n", pTask->method);
        cTmp[strlen(cTmp)-1] = '\0';
        item.method = cTmp;  // 类型
        fgets(cTmp, 1024, fp);//fprintf(fp, "%d\n", (*it).percent);
        cTmp[strlen(cTmp)-1] = '\0';
        item.percent = atoi(cTmp); //权重
        fgets(cTmp, 1024, fp); //fprintf(fp, "%d\n", (*it).type);
        cTmp[strlen(cTmp)-1] = '\0';
        item.type = atoi(cTmp); //检查项方法
        fgets(cTmp, 1024, fp);//fprintf(fp, "%s\n", (*it).methodValue.c_str());
        cTmp[strlen(cTmp)-1] = '\0';
        item.methodValue = cTmp; //阈值

        pitem->InsertItem(item);
    }
    pitem->FinishInit();
    return 0;
}
////////////////end task list/////////////////////////////////////////////////////////









#include "astserverctrl.h"

astServerCtrl::astServerCtrl()
{
    m_stServerInfo.init();
}

astServerCtrl::~astServerCtrl()
{

}
int astServerCtrl::InitServerInfo(ST_SERVER_INFO & stServerInfo)
{
    int nRet=0;
    memcpy(m_stServerInfo.cIp,stServerInfo.cIp,IP_LEN);
    m_stServerInfo.nPort = stServerInfo.nPort;
    m_stServerInfo.nTimeout = stServerInfo.nTimeout;
    //尝试连接，获取ip
    TryConnect();
    return nRet;
}

int astServerCtrl::GetDevInfo()
{
    int nReturn = 0;
    try {
        //ip mac mask信息
        NetworkInfo clsNetInfo;
        clsNetInfo.getNetInfo();
        foreach (ST_NETIPINFO stNetInfo, clsNetInfo.m_qvecNet) {
            if(stNetInfo.qsIp == m_qsCurrentIp)
            {
               memcpy(m_stDevInfo.ipmac.cIp, stNetInfo.qsIp.toStdString().c_str(), stNetInfo.qsIp.toStdString().length());
               memcpy(m_stDevInfo.ipmac.cMac, stNetInfo.qsMac.toStdString().c_str(), stNetInfo.qsMac.toStdString().length());
               memcpy(m_stDevInfo.ipmac.cMask, stNetInfo.qsMask.toStdString().c_str(), stNetInfo.qsMask.toStdString().length());
               break;
            }
        }
        //用户名
        SystemInfo clsSystemInfo;
        m_stDevInfo.strUserName = clsSystemInfo.getUsername().toStdString();
        m_stDevInfo.strPcName = clsSystemInfo.getHostname().toStdString();
        //域名 无
        //操作系统信息
        m_stDevInfo.stSystemInfo.strOsName = clsSystemInfo.osDescription.toStdString();
        m_stDevInfo.stSystemInfo.strOsVersion = clsSystemInfo.osVersion.toStdString();
        m_stDevInfo.stSystemInfo.strLastUpTime = clsSystemInfo.lastUpTime.toStdString();
        m_stDevInfo.stSystemInfo.strOsLanguage = clsSystemInfo.osLanguage.toStdString();
        m_stDevInfo.stSystemInfo.strInstallTime = clsSystemInfo.osInstallTime.toStdString();
        m_stDevInfo.stSystemInfo.strArchitecture = clsSystemInfo.getOsArchitecture().toStdString();
        //cpu信息
        CpuInfo clsCpuInfo;
        m_stDevInfo.stCpuInfo.strCpuFamily = clsCpuInfo.getCpuFamily().toStdString();
        m_stDevInfo.stCpuInfo.strModel = clsCpuInfo.getCpuModel().toStdString();
        m_stDevInfo.stCpuInfo.strModelName = clsCpuInfo.getCpuModelName().toStdString();
        m_stDevInfo.stCpuInfo.strVendorId = clsCpuInfo.getCpuVendor().toStdString();
        m_stDevInfo.stCpuInfo.nCpuMHZ = clsCpuInfo.getCpuMHZ();
        m_stDevInfo.stCpuInfo.nCacheSize = clsCpuInfo.getCpuCache();
        //内存信息
        MemoryInfo clsMemInfo;
        clsMemInfo.updateMemoryInfo();
        m_stDevInfo.stMemInfo.nMemSize = clsMemInfo.getMemTotal();
        m_stDevInfo.stMemInfo.nUsedSize = clsMemInfo.getMemUsed();
        //硬盘信息
        DiskInfo clsDiskInfo;
        clsDiskInfo.getDiskInfo();
        foreach(DiskHardInfo *pDisk,clsDiskInfo.m_stDiskHardInfo)
        {
            ST_DISK_INFO stDisk;
            stDisk.strCaption = pDisk->qsName.toStdString();
            stDisk.strType = pDisk->qsFileSystemType.toStdString();
            stDisk.nSize = pDisk->qlSize;
            stDisk.strSerialNo = pDisk->qsSerialNo.toStdString();
            stDisk.strManufauturer = pDisk->qsManufauturer.toStdString();
            stDisk.strHotPlug = pDisk->qsHotPlug.toStdString();
            m_stDevInfo.vecDiskInfo.push_back(stDisk);
        }
        //磁盘分区信息
        clsDiskInfo.updateDiskInfo();
        foreach(Disk * pLogicalDisk,clsDiskInfo.disks)
        {
            ST_LOGICAL_DISK stld;
            stld.strDeviceId = pLogicalDisk->name.toStdString();
            stld.strFileType = pLogicalDisk->fileSystemType.toStdString();
            stld.nSize = pLogicalDisk->size;
            stld.nFreeSize = pLogicalDisk->free;
            m_stDevInfo.vecLogicalDisk.push_back(stld);
        }

    } catch (...) {

    }
    return nReturn;
}
int astServerCtrl::TryConnect()
{
    int nReturn = 0;
    sockaddr_in m_address;
    socklen_t socklen;
    try {
        m_clsAstClient.InitServer(m_stServerInfo.cIp, m_stServerInfo.nPort, m_stServerInfo.nTimeout);
        nReturn = m_clsAstClient.connect(false);
        if(nReturn!=0)
        {
            qDebug("connect failed.\n");
        }
        else
        {
            socklen = sizeof(m_address);
            if (::getsockname(m_clsAstClient.m_fd.m_fd, (sockaddr*)&m_address, &socklen) != 0)
            {
                qDebug("get ip failed.\n");
            }
            else
            {
                string localips = inet_ntoa(m_address.sin_addr);
                m_qsCurrentIp = QString::fromStdString(localips);
                qDebug(localips.c_str());
            }
        }

    } catch (...) {

    }
    m_clsAstClient.close();
    return nReturn;
}
int astServerCtrl::RegAst()     //注册服务器
{
    int nReturn = 0;

    //SOCKADDR_IN m_address;
    string localips;
    int localport;
    int socklen;
    vector<char *> tmpMac, tmpIp;
    //////

    int nSendLen = 0;
    int ret = 0;
    Json::Value root;
    Json::FastWriter iWriter;

    char cCmdBuf[4096] = {0x00};
    char *pCmd = cCmdBuf;
    unsigned char cbuf[1024];

    int nLen = 1024;
    char obuf[4096]; //socket接收空间
    int  olen = 4096;
    char *pData = NULL; //socket接收空间指针，如果接收数据大于4096，需要动态分配空间
    int nDataLen = 0;
    unsigned char *pSign = (unsigned char *)obuf;
    unsigned char cSID[4] = { 0,0,0,0 };
    unsigned char cAID[16] = { 0 };
    char cJson[4096];
    int nJson = 4096;
    unsigned char ver;
    unsigned char type;
    unsigned char cOSID[4]={0x00};
    unsigned char cOAID[16]={0x00};
    unsigned char cMAC[32]={0x00};
    char cRegCK[64]={0x00};
    QString qsLog;
    TryConnect();
    GetDevInfo();

    root["cid"] = "1";
    root["ip"] = m_stDevInfo.ipmac.cIp;
    root["port"] = "10086";
    root["os"] = m_stDevInfo.stSystemInfo.strOsName;

    root["osType"] = "linux";
    root["architecture"] = m_stDevInfo.stSystemInfo.strArchitecture;

    root["mac"] = m_stDevInfo.ipmac.cMac;
    root["ipMask"] = m_stDevInfo.ipmac.cMask;

    memcpy(cCmdBuf,iWriter.write(root).c_str(), iWriter.write(root).length());
    //测试
    //sprintf(cCmdBuf, "{\"cid\":\"1\",\"ip\":\"192.168.6.44\",\"mac\":\"C4:34:6B:4F:7E:BA\",\"os\":\"Microsoft Windows 7 企业版 \",\"port\":\"10086\"}");
    try {
        ret = m_clsAstPackage.PutData(pCmd, 0x1, 0x1, 0x2, cSID, cAID, 0, cbuf, nLen);
        if(ret!=0)
        {
            nReturn = 1;
            throw nReturn;
        }
        cout<<pCmd<<endl;

        m_clsAstClient.InitClient(m_qsCurrentIp.toStdString().c_str(), 0);
        ret = m_clsAstClient.connect(true);
        if (ret != 0)
        {
            nReturn = 2;
            throw nReturn;
        }
        if (m_clsAstClient.m_bChangeIp)//ip已更改
        {
            cout<<"ip 更改，重新获取"<<endl;
            m_qsCurrentIp = QString::fromStdString(m_clsAstClient.m_clientip);
            GetDevInfo();
        }

        ret = m_clsAstClient.run((char *)cbuf, nLen, obuf, olen, nSendLen);
        if (ret < 0) //socket错误
        {
            qsLog = QString("regcheck failed! server_ret=[%1] expsend[%2] realsend[%3] recv[%4]").arg(ret).arg(nLen).arg(nSendLen).arg(olen);
            cout<<qsLog.toStdString().c_str()<<endl;
            nReturn = 3;
            throw nReturn;
        }
        else if (ret > 0) //空间不足
        {
            nDataLen = ret;
            pData = (char *)malloc(nDataLen); //动态申请空间
            memcpy(pData, obuf, 8);
            olen = ret;
            ret = m_clsAstClient.runcontinue(olen - 8, &pData[8]);
            pSign = (unsigned char *)pData;
        }
        //请求应答的处理
        ret = RegFinish((char *)pSign, olen);
        if (ret != 0)
        {
            nReturn = 10 + ret;
            goto EXIT_Reg;
        }
        m_clsAstClient.close();
        ///注册完成报文处理///
        memset(cAID, 0, 16);
        m_clsAstPackage.ToAgentID(cAID);
        m_clsAstPackage.MacByID(cAID, 12, (const char *)cSID, 4, 1, cMAC);
        m_clsAstPackage.encode(cMAC, 16, (unsigned char *)cRegCK);
        root.clear();
        root["cid"] = "1";
        root["ip"] = m_qsCurrentIp.toStdString();
        root["mac"] = m_stDevInfo.ipmac.cMac;
        root["ipMask"] = m_stDevInfo.ipmac.cMask;
        root["regck"] = cRegCK;
        cout<<iWriter.write(root).c_str()<<endl;

        memcpy(cCmdBuf,iWriter.write(root).c_str(), iWriter.write(root).length());

        nLen = 1024;

        ret = m_clsAstPackage.PutData(pCmd, 0x1, 0x1, 0x3, cSID, cAID, 1, cbuf, nLen);

        cout<<pCmd<<endl;
        if (ret != 0)
        {
            nReturn = 4;
            throw nReturn;
        }
        olen = 4096;
        pSign = (unsigned char *)obuf;

        m_clsAstClient.InitClient(m_qsCurrentIp.toStdString().c_str(), 0);
        ret = m_clsAstClient.connect(true);
        if (ret != 0)
        {
            nReturn = 5;
            throw nReturn;
        }
        if (m_clsAstClient.m_bChangeIp)//ip已更改
        {
            cout<<"ip 更改，重新获取"<<endl;
            m_qsCurrentIp = QString::fromStdString(m_clsAstClient.m_clientip);
            GetDevInfo();
        }
        nSendLen = 0;
        ret = m_clsAstClient.run((char *)cbuf, nLen, obuf, olen, nSendLen);///注册完成报文发送
        if (ret < 0) //socket错误
        {
            qsLog = QString("regcheck failed! server_ret=[%1] expsend[%2] realsend[%3] recv[%4]").arg(ret).arg(nLen).arg(nSendLen).arg(olen);
            cout<<qsLog.toStdString().c_str()<<endl;
            nReturn = 6;
            throw nReturn;
        }
        else if (ret > 0) //空间不足
        {
            if (pData != NULL && nDataLen < ret)
            {
                free(pData);
                pData = NULL;
            }
            if (pData == NULL)
                pData = (char *)malloc(ret); //动态申请空间
            memcpy(pData, obuf, 8);
            olen = ret;
            ret = m_clsAstClient.runcontinue(olen - 8, &pData[8]);
            pSign = (unsigned char *)pData;
        }
        ret = RegAck((char *)pSign, olen);
        if (ret != 0)
        {
            if (ret > 1000)  //错误码
                nReturn = ret;
            else
                nReturn = 30 + ret;
            throw nReturn;
        }


    } catch (int nerr) {

    }
EXIT_Reg:
    if (pData != NULL)
        free(pData); //释放之前动态申请的空间
    m_clsAstClient.close();
    if (nReturn != 0)
    {
        qsLog = QString("Register error ! error=%1 (%2)").arg(nReturn).arg(ret);
        cout<<qsLog.toStdString().c_str()<<endl;
    }

    return nReturn;
}
int astServerCtrl::RetryRegAst() //重新注册
{

}
int astServerCtrl::RegFinish(char *regmsg, int regmsglen)  //注册应答处理
{
    int nReturn = 0;
    int ret = 0;
    unsigned char ver;
    unsigned char type;
    unsigned char cSID[4];
    unsigned char cAID[16];
    char *pJson = NULL;
    unsigned char cSIDDefault[4] = { 0 };
    Json::Reader reader;
    Json::Value  root;
    Json::Value cid;
    char cAid[128];
    char cKA[128];
    char cMKA[128];
    int nJson = m_clsAstPackage.GetMsgLen(regmsg, regmsglen);
    if (nJson <= 0)
        return 1;
    nJson++;
    pJson = (char *)malloc(sizeof(char)*nJson);
    if (pJson == NULL)
        return 2;
    memset(pJson, 0x00, sizeof(char)*nJson);
    try {
        ret = m_clsAstPackage.GetData((unsigned char *)regmsg, regmsglen, 0, ver, type, cSID, cAID, pJson, nJson);
        if (ret != 0) //解析报文数据错误
        {
            nReturn = 3;
            throw nReturn;
        }
        if (memcmp(cSID, cSIDDefault, 4) != 0)
        {
            nReturn = 4;  //SID不为全零
            throw nReturn;
        }
        qDebug(pJson);

        reader.parse(pJson, pJson + nJson, root, false);
        cid = root["cid"];
        if (cid.asString().compare("1") != 0)
        {
            nReturn = 5;  //cid 错误
            throw nReturn;
        }
        cid = root["aid"];  //printf("aid=%s\n", cid.asString().c_str());
        memcpy(cAid, cid.asString().c_str(), cid.asString().length());
        qDebug("Receive reg ass aid:");
        qDebug(cAid);
        cid = root["ka"];   //printf("ka=%s\n", cid.asString().c_str());

        memcpy(cKA, cid.asString().c_str(), cid.asString().length());
        qDebug("Receive reg ass cKA:");
        qDebug(cKA);
        cid = root["mka"];  //printf("mka=%s\n", cid.asString().c_str());
        memcpy(cMKA, cid.asString().c_str(), cid.asString().length());
        qDebug("Receive reg ass cMKA:");
        qDebug(cMKA);
        ret = m_clsAstPackage.InitReg(cAid, cKA, cMKA, (const char *)cSID, 4);  //注册完成，保存密钥信息
        if (ret != 0)
            nReturn = 6 + ret;
    } catch (int nerr) {

    }

EXIT_RegFinish:
    if (pJson != NULL)
        free(pJson);
    return nReturn;
}
int astServerCtrl::RegAck(char *regmsg, int regmsglen)     //完成应答处理
{

}
int astServerCtrl::ReportTaskRecv(char *json)  //任务确认
{

}
int astServerCtrl::SendExit()  //退出时发送给服务器退出信息
{

}

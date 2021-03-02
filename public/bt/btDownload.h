#ifndef _BT_DOWNLOAD_H_
#define _BT_DOWNLOAD_H_

#include <iconv.h>
#include <iostream>
#include <fstream>
#include "./libtorrent/include/libtorrent/entry.hpp"
#include "./libtorrent/include/libtorrent/bencode.hpp"
#include "./libtorrent/include/libtorrent/session.hpp"
#include "./libtorrent/include/libtorrent/torrent_info.hpp"
#include "./libtorrent/include/libtorrent/alert_types.hpp"
#include "./libtorrent/include/libtorrent/torrent_status.hpp"
#include "./libtorrent/include/libtorrent/torrent_handle.hpp"
#include "./libtorrent/include/libtorrent/alert.hpp"
#include "./libtorrent/include/libtorrent/ip_filter.hpp"
#include "./libtorrent/include/libtorrent/config.hpp"
#include "./libtorrent/include/libtorrent/socket.hpp"

#include "public/characterEncode/charEncodeTransfer.h"

#include <chrono>
#include "public/log/logOut.h"

using namespace std;
using namespace chrono;


class btDownload
{
public:
    btDownload();
    ~btDownload();
public:
    int m_nTimedOut; //设置超时时间 单位s
    int m_nPeerTimeOut; //peer连接超时 单位s
    std::string save_path; //保存文件路径
    std::string m_session_path; //保存session路径
    std::string realfilename; //实际文件名
    int torrent_download_rate;//设置下载速率，单位是字节，默认-1，不做限制
    int torrent_upload_rate; //设置上传速率，单位是字节，默认-1，不做限制
    lt::session * m_sess;
    lt::session_params params;

    int m_nDownload_Rate; //实时下载速度
    int m_nUpload_Rate;   //实时上传速度
    int64_t m_nTotal_Done;//已完成下载的字节[0-m_nFileSize]
    int64_t m_nTotal_Upload;//已完成上传的字节
    int m_nTorrentProgress; //torrent任务进度[0-100]之间
    int64_t m_nFileSize; //当前下载文件总大小

    //传入日志打印模块
    CAstLogOut * m_btLog;
    //判断是否停止下载
    int * m_pnDownState;
    int m_hMainWnd;
public:
    void CopyVariable(int & nVariable);
    string UTF8toGBK(const char *utf8);
    bool CheckFileExist(string strFileObject);
    void InitBtParam(int hMainWnd, CAstLogOut * clsLog, string strLocalIp);
    void LoadSession(std::string session_path);
    void SaveSession();
    void ReloadTorrent(string localDir, string torrentFile);
    void InitBtDownload(int nDownTimeOut, int nPeerTimeOut, int download_rate, int upload_rate);
    /*
    @local_path bt下载本地存储路径
    @torrentfile torrent文件绝对路径
    @filename  返回实际下载文件名
    @bCheckTimeOut 下载超时
    @nTaskCtrl  任务标志[默认值0，1：工具 2：补丁 3：种子服务器任务]
    */
    int DownloadProcess(std::string local_path, std::string torrentfile, std::string & filename,bool bCheckTimeOut, int nTaskCtrl);
    int GetDownloadRate();
    int64_t GetDownloadTotalBybes();
    bool GetBtStatus(lt::torrent_handle th, int & nDownload_Rate, int & nUpload_Rate, int64_t & nTotal_Done, int64_t & nTotal_Upload, int & nTorrentProgress);
    //ftp下载
    //int FtpDownload(string strDownloadDir, string strFileName, string strFileFullPath, string strFtpSrv, string strUsrName, string strUsrPass);
protected:
    char const* state(lt::torrent_status::state_t s);//获取session的状态
    bool handle_alter(lt::session& ses, lt::alert* a, lt::torrent_handle &th);//处理alter

    //加锁
    int Lock(int timeouts);
    //解锁
    int unLock();
private:
    bool m_bLock;
};



#endif

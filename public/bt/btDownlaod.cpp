#include "btDownload.h"


std::string print_endpoint(lt::tcp::endpoint const& ep)
{
    using namespace lt;
    lt::error_code ec;
    char buf[200] = {0x00};
    address const& addr = ep.address();
    if(addr.is_v4())
        std::snprintf(buf, sizeof(buf), "%s:%d", addr.to_string(ec).c_str(), ep.port());
    //ipv6不显示
    //if (addr.is_v6())
    //	std::snprintf(buf, sizeof(buf), "[%s]:%d", addr.to_string(ec).c_str(), ep.port());
    //else
    //	std::snprintf(buf, sizeof(buf), "%s:%d", addr.to_string(ec).c_str(), ep.port());
    return buf;
}

std::string to_hex(lt::sha1_hash const& s)
{
    std::stringstream ret;
    ret << s;
    return ret.str();
}

bool load_file(std::string const& filename, std::vector<char>& v
    , int limit = 8000000)
{
    std::fstream f(filename, std::ios_base::in | std::ios_base::binary);
    f.seekg(0, std::ios_base::end);
    auto const s = f.tellg();
    if (s > limit || s < 0) return false;
    f.seekg(0, std::ios_base::beg);
    v.resize(static_cast<std::size_t>(s));
    if (s == std::fstream::pos_type(0)) return !f.fail();
    f.read(v.data(), v.size());
    return !f.fail();
}

int save_file(std::string const& filename, std::vector<char> const& v)
{
    std::fstream f(filename, std::ios_base::trunc | std::ios_base::out | std::ios_base::binary);
    f.write(v.data(), v.size());
    return !f.fail();
}

// return the name of a torrent status enum
char const* btDownload::state(lt::torrent_status::state_t s)
{
    switch (s) {
    case lt::torrent_status::checking_files: return "checking";
    case lt::torrent_status::downloading_metadata: return "dl metadata";
    case lt::torrent_status::downloading: return "downloading";
    case lt::torrent_status::finished: return "finished";
    case lt::torrent_status::seeding: return "seeding";
    case lt::torrent_status::allocating: return "allocating";
    case lt::torrent_status::checking_resume_data: return "checking resume";
    default: return "<>";
    }
}
bool btDownload::handle_alter(lt::session& ses, lt::alert* a, lt::torrent_handle &th)
{
    using namespace lt;

    if (state_update_alert *p = alert_cast<state_update_alert>(a))
    {
        char cinfo[1024] = { 0x00 };
        std::vector<std::int64_t> file_progress;
        th.file_progress(file_progress); // 获取文件下载进度

        int const idx = static_cast<int>(0);
        if (!p->status.empty())
        {
            for (int i = 0; i < p->status.size(); i++)
            {
                if (p->status[i].handle == th)//判断是否是当前下载的torrent
                {
                    //lt::torrent_status s = p->status[0];//如果只有一个torrent文件，就直接取status[0]
                    lt::torrent_status s = p->status[i];
                    m_nDownload_Rate = s.download_rate;
                    m_nUpload_Rate = s.upload_rate;
                    m_nTotal_Done = s.total_done;
                    m_nTotal_Upload = s.total_upload;
                    m_nTorrentProgress = s.progress_ppm / 10000;
                }
            }
        }
    }
    else if (torrent_finished_alert *p = alert_cast<torrent_finished_alert>(a))
    {

    }
    return true;
}
btDownload::btDownload()
{
    m_nTimedOut = 24*60 * 60;//默认超时时间是1天
    m_nPeerTimeOut = 30 * 60;//默认peer连接时间
    save_path = ".\\";//默认保存路径是当前路径
    realfilename = ""; //实际文件名
    torrent_download_rate = -1; //默认下载不限速
    torrent_upload_rate = -1;  //默认上传不限速
    m_session_path = ""; //保存session路径
    m_bLock = false;
    m_pnDownState = NULL;

}
btDownload::~btDownload()
{
    delete m_sess;
}
void btDownload::InitBtParam(int hMainWnd, CAstLogOut * clsLog, string strLocalIp)
{
    m_sess = new lt::session;
    //lt::settings_pack pack;
    auto & pack = params.settings;
    pack.set_int(lt::settings_pack::alert_mask
        , lt::alert::error_notification
        | lt::alert::storage_notification
        | lt::alert::status_notification
        | lt::alert::tracker_notification
        | lt::alert::peer_notification
        | lt::alert::connect_notification
        | lt::alert::tracker_notification
        | lt::alert::incoming_request_notification
    );
    pack.set_int(lt::settings_pack::min_reconnect_time,5);
    pack.set_int(lt::settings_pack::tracker_backoff, 5);
    pack.set_int(lt::settings_pack::min_announce_interval, 60);
    //pack.set_int(lt::settings_pack::outgoing_port, 6810);
    //pack.set_int(lt::settings_pack::max_pex_peers, 0);
    //pack.set_int(lt::settings_pack::tick_interval, 100);

    pack.set_str(lt::settings_pack::announce_ip, strLocalIp);
    pack.set_bool(lt::settings_pack::broadcast_lsd, false);
    pack.set_bool(lt::settings_pack::enable_dht, false);
    pack.set_bool(lt::settings_pack::enable_upnp, false);
    pack.set_bool(lt::settings_pack::enable_natpmp, false);
    //pack.set_bool(lt::settings_pack::rate_limit_utp, true);


    pack.set_int(lt::settings_pack::local_service_announce_interval,5);
    //pack.set_int(lt::settings_pack::upload_rate_limit, 2 * 1024 * 1024);//限速全局上传速度2MB/s
    //pack.set_int(lt::settings_pack::download_rate_limit, 2*1024*1024);//限速全局下载速度2MB/s

    m_sess->apply_settings(pack);

    std::uint32_t const mask = 1 << lt::session::global_peer_class_id;
    //ip过滤
    lt::ip_filter pcf;
    pcf.add_rule(lt::address_v4::from_string("0.0.0.0")
        , lt::address_v4::from_string("255.255.255.255")
        , mask);
    pcf.add_rule(lt::address_v6::from_string("::")
        , lt::address_v6::from_string("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"), mask);
    //m_sess->set_ip_filter(pcf);
    m_sess->set_peer_class_filter(pcf);

    //日志打印模块
    //m_btLog = new CAssLog;
    m_btLog = clsLog;
    m_hMainWnd = hMainWnd;
}
void btDownload::ReloadTorrent(string localDir, string torrentFile)
{
    //设置下载参数
    lt::add_torrent_params params;
    params.save_path = localDir;
    params.ti = std::make_shared<lt::torrent_info>(torrentFile);
    params.download_limit = -1;
    params.upload_limit = -1;
    m_sess->async_add_torrent(params);
    //m_sess->add_torrent(params);
}

void btDownload::InitBtDownload(int nDownTimeOut, int nPeerTimeOut, int download_rate, int upload_rate)
{
    m_nTimedOut = nDownTimeOut;
    m_nPeerTimeOut = nPeerTimeOut;
//	save_path = local_path;
    torrent_download_rate = download_rate;
    torrent_upload_rate = upload_rate;

    m_sess->set_local_download_rate_limit(download_rate);
    m_sess->set_download_rate_limit(download_rate);

    m_sess->set_local_upload_rate_limit(upload_rate);
    m_sess->set_upload_rate_limit(upload_rate);

}

int btDownload::DownloadProcess(std::string local_path, std::string torrentfile, std::string & filename, bool bCheckTimeOut,int nTaskCtrl = 0)
{
    char cLog[1024] = { 0x00 };
    string strLog;

    //lt::session m_sess; //bt session
    int nError = 0;
    //初始化下载相关信息///////

    int nDownload_Rate = 0, nUpload_Rate = 0;
    int64_t nTotal_Done = 0, nTotal_Upload = 0;
    int nTorrentProgress = 0;
    int nFileSize = 0;

    int nPeerTimeOut = 30 * 60; //获取peer为空，等30分钟

    //设置下载参数
    lt::add_torrent_params params;
    params.save_path = local_path;
    if (!CheckFileExist(torrentfile)) //torrent文件不存在
    {
        sprintf(cLog, "torrentfile %s is not exist.", torrentfile.c_str());
        m_btLog->WriteLog(cLog, 4);
        nError = 3;
        return nError;
    }
    params.ti = std::make_shared<lt::torrent_info>(torrentfile);

    //params.download_limit = torrent_download_rate;
    //params.upload_limit = torrent_upload_rate;

    auto start = system_clock::now();

    //查找torrent文件是否在session中存在
    lt::torrent_info findTi = lt::torrent_info(torrentfile);
    lt::torrent_handle findHandle = m_sess->find_torrent(findTi.info_hash());
    if (findHandle.is_valid())
    {
        m_sess->pause();
        m_sess->remove_torrent(findHandle);
        m_sess->resume();
    }
    lt::torrent_handle th = m_sess->add_torrent(params);

    if (th.is_valid())
    {
        sprintf(cLog, "torrent [%s] save_path is [%s] ", torrentfile.c_str(), th.save_path().c_str());
        m_btLog->WriteLog(cLog, 0);
    }
    static auto end = system_clock::now();
    static auto duration = duration_cast<microseconds>(end - start);
    //获取文件名
    filename = params.ti->files().file_name(0).to_string();
    //utf-8转unicode
    int nFileNameLen = 0;
    characterEncode clsCharEncode;
    filename = clsCharEncode.UTF8toGBK(filename.c_str());

    //获取文件大小
    m_nFileSize = params.ti->files().file_size(0);
    nFileSize = params.ti->files().file_size(0);

    int nPrintTimer = 0;
    int nResumeBtTimer = 0;
    std::vector<std::int64_t> file_progress;
    while (true)
    {
        //打印计数器，5秒打印一次
        nPrintTimer++;
        nResumeBtTimer++;
        //lt::file_progress_flags_t file_pf=lt::torrent_handle::piece_granularity;
        //th.file_progress(file_progress, lt::torrent_handle::piece_granularity); // 获取文件下载进度
        th.file_progress(file_progress); // 获取文件下载进度
        int const idx = static_cast<int>(0);
        bool const complete = (file_progress[idx] == m_nFileSize); //判断文件是否下载完成
        if (complete)
        {
            nError = 0;//下载已完成
            m_nTorrentProgress = 100; //下载进度更新100
           // ::SendMessage(m_hWnd, WMH_SOFT_BASE + SOFT_Progress_Value, (WPARAM)(m_nTorrentProgress), NULL);
            break;
        }
        m_nTotal_Done = file_progress[idx]; //已下载文件大小
        end = system_clock::now();
        duration = duration_cast<microseconds>(end - start); //下载时间间隔
        if ((double(duration.count())*microseconds::period::num / microseconds::period::den) > m_nPeerTimeOut && file_progress[idx] == 0)
        {
            nError = 1;//长时间未下载（10分钟）
            break;
        }
        if ((double(duration.count())*microseconds::period::num / microseconds::period::den) > m_nTimedOut && bCheckTimeOut)//判断是否超时,根据超时检查标志
        {
            nError = 2;//下载超时
            break;
        }

        //获取下载过程信息加锁
        Lock(0);
        //更新下载状态信息
        if (GetBtStatus(th, nDownload_Rate, nUpload_Rate, nTotal_Done, nTotal_Upload, nTorrentProgress) && nError == 0)
        {
            //GetBtStatus返回true，表示torrent下载完成
            //获取下载过程信息解锁
            unLock();
            break;
        }
        //判断是否停止下载
        while (m_pnDownState&&*m_pnDownState)
        {
            if (*m_pnDownState == 1)
            {
                sleep(1);
                m_sess->pause(); //暂停下载
            }
            else
                break;
        }
        if(m_pnDownState&&*m_pnDownState == 0)
            m_sess->resume(); //恢复下载
        else if (m_pnDownState&&*m_pnDownState == 2)
        {
            m_sess->pause(); //暂停下载
            m_sess->remove_torrent(th);
            nError = 3;//下载取消
            break;
        }
        if (nPrintTimer >= 10)
        {
            nPrintTimer = 0;
            if (nDownload_Rate > torrent_download_rate && torrent_download_rate!=-1) //限速条件下，超过限制速度
            {
                nDownload_Rate = torrent_download_rate;
            }
            //打印日志
            if (nTaskCtrl == 3) //种子服务器任务
            {
                sprintf(cLog, "SeedDownloadTask download %s, download rate [%dKB/S], total download [%lldKB]", filename.c_str(), (nDownload_Rate / 1024), nTotal_Done / 1024);
                m_btLog->WriteLog(cLog, 0);
            }
            else
            {
                sprintf(cLog, "download %s, download rate [%dKB/S], total download [%lldKB]", filename.c_str(), (nDownload_Rate / 1024), nTotal_Done / 1024);
                m_btLog->WriteLog(cLog, 0);
            }


            char cTmp[512];
            sprintf(cTmp, "Rate：[%dKB/S] [%.2fMB]/[%.2llfMB]", (nDownload_Rate / 1024), nTotal_Done / 1024 / 1024.0, m_nFileSize / 1024 / 1024.0);
            if (nTaskCtrl == 1) //更新工具的bt下载
            {
                //::SendMessage(m_hWnd, WMH_SOFT_BASE + SOFT_BTUPDATE, (WPARAM)cTmp, NULL);
                int nres = nTotal_Done*100.0 / m_nFileSize;
                if(m_nFileSize)
                {
                    //::SendMessage(m_hWnd, WMH_SOFT_BASE + SOFT_Progress_Value, (WPARAM)(nres), NULL);
                }
            }
            else if (nTaskCtrl == 2)  //更新补丁的bt下载
            {
                //::SendMessage(m_hWnd, WMH_REPAIR_BASE + Repair_BtUpdate, (WPARAM)cTmp, NULL);
            }

            int nCount = 0;
            std::vector<lt::peer_info> peers;
            th.get_peer_info(peers);
            strLog = "peer list:[ ";
            for (std::vector<lt::peer_info>::const_iterator i = peers.begin();
                i != peers.end(); ++i)
            {
                if (::print_endpoint(i->ip) != "")
                {
                    //strLog = strLog + ::print_endpoint(i->ip) + " size:" + std::to_string(i->total_download / 1024) + "KB, ";
                    strLog = strLog + ::print_endpoint(i->ip) + ", ";
                }
                if (nCount > 5) //只打印前5个peer Ip
                    break;
                nCount++;
            }
            if (strLog != "peer list:[ ")
            {
                strLog = strLog + "]";
                m_btLog->WriteLog((char *)strLog.c_str(), 0);
            }

        }
        if (nResumeBtTimer >= 600)
        {
            nResumeBtTimer = 0;
            if (nTotal_Done == 0 || nDownload_Rate == 0) //速率为0或者下载size=0
            {
                m_sess->pause(); //暂停下载
                m_sess->resume(); //恢复下载
            }
        }

        //获取下载过程信息解锁
        unLock();

        sleep(1);  //5秒打印一次下载速率

    }
    //保存session 暂时注释
    //SaveSession();
    if (nError != 0)  //下载失败，移除torrent
    {
        m_sess->remove_torrent(th);
    }

    return nError;
}

int btDownload::Lock(int timeouts)
{
    if (timeouts == 0) //永久
    {
        while (m_bLock);
        m_bLock = true;
        return 0;
    }
    else
    {
        for (int i = 0; i < timeouts; i++)
        {
            if (!m_bLock)
            {
                m_bLock = true;
                return 0;
            }
            sleep(50);
        }
    }
    return 1;
}
//解锁
int btDownload::unLock()
{
    m_bLock = false;
    return 1;
}

void btDownload::LoadSession(std::string session_path)
{
    m_session_path = session_path;
    using lt::session_handle;
    params.dht_settings.privacy_lookups = true;
    std::vector<char> in;
    if (load_file(m_session_path + ".ses_state", in))
    {
        lt::bdecode_node e;
        lt::error_code ec;
        if (bdecode(&in[0], &in[0] + in.size(), e, ec) == 0)
            params = read_session_params(e, session_handle::save_dht_state);
    }
}
void btDownload::SaveSession()
{
    lt::entry session_state;
    m_sess->save_state(session_state, lt::session::save_dht_state);

    std::vector<char> out;
    bencode(std::back_inserter(out), session_state);
    save_file(m_session_path + ".ses_state", out);
}

int btDownload::GetDownloadRate()
{
    return m_nDownload_Rate / 1024;
}
int64_t btDownload::GetDownloadTotalBybes()
{
    return m_nTotal_Done / 1024;
}

bool btDownload::GetBtStatus(lt::torrent_handle th, int & nDownload_Rate, int & nUpload_Rate, int64_t & nTotal_Done, int64_t & nTotal_Upload, int & nTorrentProgress)
{
    using namespace lt;
    bool bRet = false;
    char cLog[1024] = { 0x00 };
    m_sess->post_torrent_updates();
    std::vector<lt::alert*> alerts;
    alerts.clear();
    m_sess->pop_alerts(&alerts);
    for (auto a : alerts)
    {
        if (state_update_alert *p = alert_cast<state_update_alert>(a))
        {
            char cinfo[1024] = { 0x00 };
            std::vector<std::int64_t> file_progress;
            th.file_progress(file_progress); // 获取文件下载进度

            int const idx = static_cast<int>(0);
            if (!p->status.empty())
            {
                for (int i = 0; i < p->status.size(); i++)
                {
                    if (p->status[i].handle == th)//判断是否是当前下载的torrent
                    {
                        //lt::torrent_status s = p->status[0];//如果只有一个torrent文件，就直接取status[0]
                        lt::torrent_status s = p->status[i];
                        //nDownload_Rate = s.download_rate;
                        nDownload_Rate = s.download_payload_rate;
                        nUpload_Rate = s.upload_rate;
                        nTotal_Done = s.total_done;
                        nTotal_Upload = s.total_upload;
                        nTorrentProgress = s.progress_ppm / 10000;
                    }
                }
            }
        }
        else if (torrent_finished_alert *p = alert_cast<torrent_finished_alert>(a))
        {
            if (p->handle == th) //torrent下载完成
            {
                bRet = true;
            }
        }
        else if (file_completed_alert *p = alert_cast<file_completed_alert>(a))
        {
            if (p->handle == th) //torrent下载完成
            {
                bRet = true;
            }
        }
        else if (tracker_reply_alert *p = alert_cast<tracker_reply_alert>(a))
        {
            if (p->handle == th) //torrent下载完成
            {
                //bRet = true;
                sprintf(cLog, "tracker return peer num[%d].", p->num_peers);
                m_btLog->WriteLog(cLog, 0);
            }
        }
        else if (tracker_announce_alert *p = alert_cast<tracker_announce_alert>(a))
        {
            if (p->handle == th)
            {
                //bRet = true;
                sprintf(cLog, "tracker_announce [%s], send event[%d].", p->tracker_url(),p->event);
                m_btLog->WriteLog(cLog, 0);
            }
        }
        else if (tracker_error_alert *p = alert_cast<tracker_error_alert>(a))
        {
            if (p->handle == th)
            {
                sprintf(cLog, "tracker_error message[%s] errorcode [%s] status_code[%d].", p->tracker_url(), p->error.message().c_str(), p->status_code);
                m_btLog->WriteLog(cLog, 0);
            }
        }
        else if (peer_error_alert *p = alert_cast<peer_error_alert>(a))
        {
            if (p->handle == th)
            {
                sprintf(cLog, "peer_error_alert error message [%s].", p->error.message().c_str());
                m_btLog->WriteLog(cLog, 0);
            }
        }
        else if (peer_disconnected_alert *p = alert_cast<peer_disconnected_alert>(a))
        {
            //if (p->handle == th)
            //{
            //	sprintf_s(cLog, "peer_disconnected_alert [%s] error message [%s],reason code [%d].", ::print_endpoint(p->ip).c_str(), p->error.message().c_str(), p->reason);
            //	m_btLog->write(cLog, 0);
            //}
        }
        else if (peer_blocked_alert *p = alert_cast<peer_blocked_alert>(a))
        {
            if (p->handle == th)
            {
                sprintf(cLog, "peer_blocked_alert [%s] error message [%s],reason code [%d].", ::print_endpoint(p->ip).c_str(), p->message().c_str(), p->reason);
                m_btLog->WriteLog(cLog, 0);
            }
        }
    }
    return bRet;
}

void btDownload::CopyVariable(int & nVariable)
{
    m_pnDownState = &nVariable;
}
bool btDownload::CheckFileExist(string strFileObject)
{
    if(access(strFileObject.c_str(), F_OK)!=-1)
    {
        return true;
    }
    else
        return false;
}

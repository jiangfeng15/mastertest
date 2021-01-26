#ifndef ZSOCKET_H
#define ZSOCKET_H
#ifdef _MSC_VER
#include <WinSock2.h>
#include <Windows.h>

#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#endif

#include <string.h>
#include <string>
#include <iostream>

struct CSocket
{
    private:
        CSocket (const CSocket & rhs);
    public:
        int m_fd;

        CSocket (int fd = -1):m_fd(fd)
        {
        }

        ~CSocket ()
        {
            //printf("constr\n");
            close();
        }

        operator int ()
        {
            return m_fd;
        }

        //int fd ()
        //{
        //	return m_fd;
        //}

        int close()
        {
            //printf("close\n");
            if (m_fd == -1)
                return 0;

#ifdef _MSC_VER
            int ret =closesocket(m_fd);
#else
            int ret =::close (m_fd);
#endif
            m_fd = -1;
            return ret;
        }
        int build_stream ()
        {
            return m_fd = (int) socket (AF_INET, SOCK_STREAM, 0);
        }

        int build_dgram ()
        {
            return m_fd = (int) socket (AF_INET, SOCK_DGRAM, 0);
        }

        unsigned int send (const void *b, size_t len)
        {
            return ::send (m_fd, (const char*)b, len, 0);
        }

        unsigned int recv (void *b, size_t len)
        {
            return ::recv (m_fd, ( char*)b, len, 0);
        }

         int sendv (const void *b, size_t len)
        {
            return sendv_impl(b,len,0);
#if 0
            int sent = 0, ret;

            for (; len > 0;)
            {
                ret =::send (m_fd, (char *) b + sent, len, 0);
                if (ret < 0)
                {
#ifndef _MSC_VER
                    if (errno == EINTR)
                        continue;
#endif
                    return -1;
                }

                sent += ret;
                len -= ret;
            }

            return sent;
#endif
        }

        template < typename T > int setoption (int level, int name,
                const T & opt)
        {
            return setsockopt (m_fd, level, name, (char *) &opt, sizeof (T));
        }

        int setnodelay (bool nodelay = true)
        {
            int v = nodelay ? 1 : 0;

            return setoption (IPPROTO_TCP, TCP_NODELAY, v);
        }

        int setreuseaddr (bool reuse = true)
        {
            int v = reuse ? 1 : 0;

            return setoption (SOL_SOCKET, SO_REUSEADDR, v);
        }

        int setrecvtimeo (int tmout_ms)
        {
#ifdef _MSC_VER
            int tv = tmout_ms;
#else
            struct timeval tv = { tmout_ms / 1000, tmout_ms % 1000 * 1000 };
#endif
            return setoption (SOL_SOCKET, SO_RCVTIMEO, tv);
        }

        int setsendtimeo (int tmout_ms)
        {
#ifdef _MSC_VER
            int tv = tmout_ms;
#else
            struct timeval tv = { tmout_ms / 1000, tmout_ms % 1000 * 1000 };
#endif
            return setoption (SOL_SOCKET, SO_SNDTIMEO, tv);
        }

        int setblocking (int b)
        {
#ifdef _MSC_VER
        unsigned long c = b;
       //���÷�������ʽ����
            if(b<0) return 0;
       if( ioctlsocket(m_fd, FIONBIO, &c)==SOCKET_ERROR)
             return -1;
#else
            int opts = fcntl (m_fd, F_GETFL);

            if (opts < 0)
                return -1;

            if(b==((opts & O_NONBLOCK)==0))
                return 0;

            opts=b?opts^O_NONBLOCK:opts|O_NONBLOCK;

            if (fcntl (m_fd, F_SETFL, opts) < 0)
                return -1;

            return 0;
#endif
            return 0;
        }

        static sockaddr *build_addr (sockaddr_in * addr, const char *ip,
                int port)
        {
            return build_addr (addr, ntohl (inet_addr (ip)), port);
        }

        static sockaddr *build_addr (sockaddr_in * addr, int ip, int port)
        {
            memset (addr, 0, sizeof (sockaddr_in));

            addr->sin_family = AF_INET;
            addr->sin_addr.s_addr = htonl (ip);
            addr->sin_port = htons (port);

            return (sockaddr *) addr;
        }

        int bind (sockaddr * sa)
        {
            return::bind (m_fd, sa, sizeof (sockaddr));
        }

        int bind (sockaddr_in * sa)
        {
            return bind ((sockaddr *) sa);
        }

        int bind (const char *ip, int port)
        {
            sockaddr_in si;

            return bind (build_addr (&si, ip, port));
        }

        int bind (int inaddr, int port)
        {
            sockaddr_in si;

            return bind (build_addr (&si, inaddr, port));
        }

        int getsockname (sockaddr * sa)
        {
#ifdef _MSC_VER
            int addr_len = sizeof(sockaddr);
#else
            socklen_t addr_len = sizeof (sockaddr);
#endif
            return::getsockname (m_fd, sa, &addr_len);
        }

        int connect (sockaddr * sa)
        {
            return::connect (m_fd, sa, sizeof (sockaddr));
        }

        int connect (sockaddr_in * sa)
        {
            return connect ((sockaddr *) sa);
        }

        int connect (int inaddr, int port)
        {
            sockaddr_in si;

            return connect (build_addr (&si, inaddr, port));
        }

        int connect (const char *ip, int port)
        {
            sockaddr_in si;

            return connect (build_addr (&si, ip, port));
        }

        struct blocking_auto
        {
            CSocket *parent;
            blocking_auto(CSocket*sock)
                :parent(sock)
            {
                if(parent)
                    parent->setblocking(false);
            }
            ~blocking_auto()
            {
                if(parent)
                    parent->setblocking(true);
            }
        };

        static struct timeval* mk_final_time(int timeout_ms,struct timeval *ts)
        {
            struct timeval  tv={0};
            tv.tv_sec=timeout_ms/1000;
            tv.tv_usec=timeout_ms%1000*1000;

            //clock_gettime(CLOCK_MONOTONIC,ts);

            ts->tv_sec +=tv.tv_sec;
            ts->tv_usec+=tv.tv_usec*1000;
            if(ts->tv_usec/1000000000)
            {
                ts->tv_sec+=ts->tv_usec/1000000000;
                ts->tv_usec%=1000000000;
            }
            return ts;
        }

        static struct timeval* time_left(const struct timeval*final,struct timeval*ret)
        {
            struct timeval now={0};
            //clock_gettime(CLOCK_MONOTONIC,&now);

            ret->tv_sec=final->tv_sec-now.tv_sec;
            ret->tv_usec=(final->tv_sec-now.tv_sec)/1000;
            while(ret->tv_usec<0)
            {
                --ret->tv_sec;
                ret->tv_usec+=1000000;
            }

            if(ret->tv_sec<0)
            {
                ret->tv_sec=0;
                ret->tv_usec=0;
            }

            return ret;
        }

         int select_wait_to(const struct timeval* final,int wait_write)
        {
            int error=0,ret;
            socklen_t len=sizeof(int);
            struct timeval tv;
            fd_set set;
            FD_ZERO(&set);
            FD_SET(m_fd,&set);

            errno=0;
            for(;;)
            {
                ret=wait_write
                    ?select(m_fd+1,0,&set,0,time_left(final,&tv))
                    :select(m_fd+1,&set,0,0,time_left(final,&tv));

                if(ret<0 && errno==EINTR)
                    continue;

                if(ret>0)
                    break;

                if(ret==0)
                    errno=ETIME;

                return -1;
            }

            getsockopt(m_fd, SOL_SOCKET, SO_ERROR,(char*)&error, &len);
            errno=error;

            return error==0?0:-1;
        }

         int select_wait(int timeout_ms,int wait_write)
        {
            //c++11 ������final�ؼ���
            struct timeval final1={0};
            return select_wait_to(mk_final_time(timeout_ms,&final1),wait_write);
        }


        int connect (const char *ip, int port, int timeout_ms)
        {
            sockaddr_in si;
            errno=0;
            int ret=connect (build_addr (&si, ip, port));

            if(ret==-1 && errno==EINPROGRESS)
            {
                ret=select_wait(timeout_ms,true);
            }

            return ret;
        }

        int attach (int fd)
        {
            int ret = m_fd;

            m_fd = fd;
            return ret;
        }
        int detach ()
        {
            int ret = m_fd;

            m_fd = -1;
            return ret;
        }

        int listen (int backlog)
        {
            return::listen (m_fd, backlog);
        }

        int accept ()
        {
            int fd = -1;

            while (1)
            {
                fd = (int)::accept (m_fd, 0, 0);
                if (fd < 0)
                {
#ifdef _MSC_VER
#else
//					perror("accept");
//					fflush(stderr);
                    if (errno == EINTR || errno == EWOULDBLOCK
                            || errno == ECONNABORTED)
                        continue;
#endif
                }
                break;
            }
            return fd;
        }


        int sendv (const void *b, size_t len, int timeout_ms)
        {
            if(timeout_ms == 0)
                return sendv_impl(b,len,0);
            else
            {
                //c++11 ����finall
                struct timeval final1={0};
                return sendv_impl(b,len,mk_final_time(timeout_ms,&final1));
            }
        }

        int sendv_impl (const void *b, size_t len, const struct timeval*final_time)
        {
            #define ERR_RET(ret) {setblocking(final_time==0?-1:0); return ret;}
            int sent = 0, ret;
            setblocking(final_time==0?-1:1);

            for (; len > 0;)
            {
                ret =send ((char *) b + sent, len);

                if (ret < 0)
                {
#ifndef _MSC_VER
                    if (errno == EINTR)
                        continue;
#endif
                    if(final_time==0)
                        ERR_RET(-1);

                    //errno = WSAGetLastError();
                    if(errno == EWOULDBLOCK)
                    {
                        if(select_wait_to(final_time,true))
                            ERR_RET(-1);

                        continue;
                    }

                    perror("send");
                    return -1;
                }

                sent += ret;
                len -= ret;
            }

            return sent;
        }

        int accept_impl(const struct timeval*final_time)
        {
            blocking_auto noblock(final_time==0?0:this);
            int fd = -1;

            while(1)
            {
                fd = (int)::accept(m_fd, 0, 0);
                if (fd < 0)
                {
#ifdef _MSC_VER
                    continue;
                    //printf("perror:%d\n",fd);
                     if (errno == EINTR || errno == EWOULDBLOCK|| errno == ECONNABORTED)
                        continue;
                     if (errno == EWOULDBLOCK && final_time==0)
                        continue;

                    if(errno == EWOULDBLOCK)
                    {
                        if(select_wait_to(final_time,false))
                            return -1;

                        continue;
                    }
#else
//					perror("accept");
//					fflush(stderr);
                    if (errno == EINTR || errno == ECONNABORTED)
                        continue;

                    if (errno == EWOULDBLOCK && final_time==0)
                        continue;

                    if(errno == EWOULDBLOCK)
                    {
                        if(select_wait_to(final_time,false))
                            return -1;

                        continue;
                    }
#endif
                }
                break;
            }
            return fd;
        }

         int acceptv (int timeout_ms)
        {
            if(timeout_ms == 0)
                return accept_impl(0);
            else
            {
                //c++11 ����final
                struct timeval final1={0};
                return accept_impl(mk_final_time(timeout_ms,&final1));
            }
        }

         int recvv_impl (void *b, size_t len,const struct timeval*final_time)
        {
            #define ERR_RET(ret) {setblocking(final_time==0?-1:0); return ret;}
            int recved = 0, ret;
            setblocking(final_time==0?-1:1);

            for (; len > 0;)
            {
                ret =recv ((char *) b + recved, len);
                if(ret==0)
                    return 0;

                if (ret < 0)
                {
#ifndef _MSC_VER
                    if (errno==EINTR)
                        continue;
#endif
                    if(final_time==0)
                         ERR_RET(-1);

                   // errno = WSAGetLastError();
                    if(errno == EWOULDBLOCK)
                    {
                        if(select_wait_to(final_time,false))
                             ERR_RET(-1);

                        continue;
                    }

                    return -1;
                }

                recved += ret;
                len -= ret;
            }
            return recved;
        }

        int recvv (void *b, size_t len, int timeout_ms)
        {
            if(timeout_ms == 0)
                return recvv_impl(b,len,0);
            else
            {
                //c++11 ����final
                struct timeval final1={0};
                return recvv_impl(b,len,mk_final_time(timeout_ms,&final1));
            }
        }

        int recv_some (void *b, size_t len, int timeout_ms)
        {
            blocking_auto noblock(this);
            int ret=select_wait(timeout_ms,false);

            if(ret<0)
            {
                if(errno==ETIME)
                    return 0;

                return -1;
            }


            for (;;)
            {
                ret =::recv (m_fd, (char *) b , len, 0);

                if(ret==0)
                    return -1;

                if (ret < 0)
                {
#ifndef _MSC_VER
                    if (errno==EINTR)
                        continue;
#endif
                    return -1;
                }

                return ret;
            }

            return -1;
        }
};

class CSocketHandle : public CSocket
{
public:
    CSocketHandle()
    {
    }

    CSocketHandle(int fd)
        :CSocket(fd)
    {
    }

    ~CSocketHandle()
    {
        detach();
    }
};
#endif // ZSOCKET_H

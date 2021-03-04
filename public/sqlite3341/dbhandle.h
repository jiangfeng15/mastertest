#ifndef DBHANDLE_H
#define DBHANDLE_H
#include "./sqlite3.h"
#include "./SQLiteWrapper.h"
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

class dbHandle
{
public:
    dbHandle();
    ~dbHandle();
    std::string GetCurrentTime();
    //打开数据库
    bool OpenAstDB(char * dbPath);
    //关闭数据库
    bool CloseAstDB();

    bool LockDB();
    bool unLockDB();
public:
    SQLiteWrapper m_sqlite_wrapper;  //sqlite借口对象
    SQLiteStatement * m_sqlite_smt;  //statement instance

    bool m_bDBConnect; //判断数据库是否连接成功
    std::string m_strErrInfo; //错误信息
    std::string m_strDBPath;  //数据库文件路径

private:
    pthread_mutex_t m_mutex;
};

#endif // DBHANDLE_H

#ifndef ASTCONFIG_H
#define ASTCONFIG_H
#include <string>
using namespace std;

class astConfig
{
public:
    astConfig();
    ~astConfig();
public:
    int LoadIni(char * cIniPath);
    int read_profile_string( const char *section, const char *key,char *value, int size,const char *default_value);
    int read_profile_int( const char *section, const char *key,int default_value);
    int write_profile_string( const char *section, const char *key,const char *value);
public:
    char m_cIp[32];
    int m_nPort;
    int m_nTimeOut;
    int m_nLogLevel;
    int m_nLangId;
    string m_strFilePath;
};

#endif // ASTCONFIG_H

#ifndef _CHARACTERENCODE_H_
#define _CHARACTERENCODE_H_

#include <iconv.h>
#include <string>
#include <string.h>

using namespace std;

class characterEncode
{
public:
    characterEncode(){};
    ~characterEncode(){};
    string UTF8toGBK(const char * inbuf);
    string GBKtoUTF8(const char * inbuf);
    int code_convert(char * from_charset, char * to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen);
};

#endif

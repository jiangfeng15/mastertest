#include "charEncodeTransfer.h"

string characterEncode::UTF8toGBK(const char * inbuf)
{
    int nLen = strlen(inbuf);
    string strRet;
    strRet.resize(nLen * 2 + 2);
    if(code_convert("uft-8","gbk",const_cast<char *>(inbuf),nLen, &strRet[0], strRet.size()))
        return inbuf;
    return strRet;
}
string characterEncode::GBKtoUTF8(const char * inbuf)
{
    int nLen = strlen(inbuf);
    string strRet;
    strRet.resize(nLen * 2 + 2);
    if(code_convert("gbk","utf-8",const_cast<char *>(inbuf),nLen, &strRet[0], strRet.size()))
        return inbuf;
    return strRet;
}
//编码在转换
int characterEncode::code_convert(char * from_charset, char * to_charset, char *inbuf, size_t inlen, char *outbuf, size_t outlen)
{
    iconv_t cd;
    char ** pIn = &inbuf;
    char ** pOut = &outbuf;

    cd = iconv_open(to_charset, from_charset);
    if(cd == 0) return -1;

    if(iconv(cd, pIn, &inlen, pOut, &outlen) == -1) return -1;
    iconv_close(cd);

    return 0;
}

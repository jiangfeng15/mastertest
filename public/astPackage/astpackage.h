#ifndef ASTPACKAGE_H
#define ASTPACKAGE_H
#include <stdlib.h>
#include <string.h>
#include "public/sm/sm3.h"
#include "public/sm/sm4.h"
#include "public/astconfig.h"

#define  LENGTH_KI   32
#define  LENGTH_KA   32
#define  LENGTH_AID  16
/**
  packet_header length is 32Bytes:
  ver:1Bytes,default:0x01
  type:1Bytes 请求报文还是应答报文，请求：0x01,应答：0x02
  flag:1Bytes 报文的加密与校验标志，加密：0x01,校验0x02,加密与校验0x03
  reserved:1Bytes 保留字节，0x00
  len:4Bytes 整个报文的长度，包括头部32字节
  aid：16Bytes agentid,注册时填充0x00
  sid: 4Bytes 一次会话的唯一标识，请求与应答使用相同的值
  mac：4Bytes 校验数据
  */

typedef struct package_header
{
    unsigned char ver;
    unsigned char type;
    unsigned char flag;
    unsigned char reserved;
    unsigned char len[4];
    unsigned char aid[16];
    unsigned char sid[4];
    unsigned char mac[4];
} PACKAGE_HEADER;




class astPackage
{
public:
    astPackage();
    ~astPackage();
    void InitKA(unsigned char ka[LENGTH_KA]);
    int  LoadConf(const char *confile);
    int  SaveConf(const char *confile);
    int  PutData(char *data, unsigned char ver, unsigned char type, unsigned char flag, unsigned char sid[4], unsigned char aid[16], int macindex, unsigned char *out, int &outlen);
    int  GetData(unsigned char *msg, int msglen, int macindex, unsigned char &ver, unsigned char &type, unsigned char sid[4], unsigned char aid[16], char *data, int &datalen);
    int  EncryptByID(const unsigned char *data, int datalen, const char *id, int idlen, int keyindex, unsigned char *out, int &outlen, unsigned char mac[32]);
    int  DecryptByID(const unsigned char *edata, int edatalen, const char *id, int idlen, int keyindex, unsigned char *out, int &outlen);
    int  MacByID(const unsigned char *data, int datalen, const char *id, int idlen, int keyindex, unsigned char mac[32]);
    void ToAgentID(unsigned char *out);
    int  GetMsgLen(char *msg, int msglen);
    int  InitReg(char *aid, char *ka, char *mka, const char *id, int idlen);
    unsigned char *m_pKA;
    int GetAidLen();
public:
    char  m_UUID[64];
    void encode(const unsigned char* in,int inlen,unsigned char* out);
    unsigned char *decode(char *base64code, int base64length, unsigned char *out, int &outlen);
    void AToByte(char *asc, char *out, int outlen);
private:
    unsigned char m_KI[LENGTH_KI];
    unsigned char m_KA[LENGTH_KA];
    bool  m_bInit;
    char  m_AgentID[64];
    int m_nAidLen;
    astConfig m_clsAstConfig;
};

#endif // ASTPACKET_H

#include "stdafx.h"
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "asscmd.h"
#include "ini_parser.h"
#include "sm3.h"
#include "sm4.h"
#include "zstd.h"

//加载库文件
#ifndef _WIN64

#pragma comment(lib, "..\\Lib\\zstd32\\libzstd_32.lib")

#else

#pragma comment(lib, "..\\Lib\\zstd64\\libzstd_64.lib")
#endif

CAssCmd::CAssCmd()
{
	char *p="20181009";
	SM3((unsigned char *)p, strlen(p), m_KI);   //KI  95E59E3BCCD550E8EC8FDFCDD81E19756F1FA947F74198F1BBF52A5115E909CD  sm3("20181009")
	memset(m_KA, 0, LENGTH_KA);
	memset(m_AgentID, 0, sizeof(m_AgentID));
	m_bInit = true;
	m_pKA = m_KA;
	m_nAidLen = 0;
}

CAssCmd::~CAssCmd()
{
}

void CAssCmd::InitKA(unsigned char ka[LENGTH_KA])
{
	memcpy(m_KA, ka, LENGTH_KA);
}

int  CAssCmd::LoadConf(const char *confile)
{
	char cBuf[128];
	int nRet = 0;
	nRet = read_profile_string("SECRETKEY", "agentid", cBuf, 127, "", confile);  //read agentid
	if (!nRet)
		return 1;
	int nAIDLen = LENGTH_AID;
	decode(cBuf, strlen(cBuf), (unsigned char *)m_AgentID, nAIDLen);

	//赋值m_nAidLen
	int nIndex = 0;
	nIndex = nAIDLen - 1;
	while (nIndex >= 0)
	{
		if (m_AgentID[nIndex] == 0x00)
		{
			nIndex--;
		}
		else
			break;
	}
	m_nAidLen = nIndex + 1;

	//strncpy(m_AgentID, cBuf, 40);
	nRet = read_profile_string("SECRETKEY", "uuid", cBuf, 127, "", confile);  //read uuid
	if (!nRet)
		return 2;
	strncpy(m_UUID, cBuf, 40);
	m_UUID[40]='\0';
	nRet = read_profile_string("SECRETKEY", "ka", cBuf, 127, "", confile);  //read ka  ka is encrypted and base64
	if (!nRet)
		return 3;
	unsigned char cEKA[64];
	int nEKA = 64;
	decode(cBuf, strlen(cBuf), cEKA, nEKA);
	if(nEKA != 48)
		return 3;
	int nKA = nEKA;
	unsigned char cKA[128];
	nRet = DecryptByID(cEKA, nEKA, m_UUID, strlen(m_UUID), 0, cKA, nKA); 
	if(nRet != 0 || nKA != LENGTH_KA) 
		return 5;  //decrypt error
	nRet = read_profile_string("SECRETKEY", "mac", cBuf, 127, "", confile);  //read mac=BASE64(SM3（uuid|agentID|KA）)
	if (!nRet)
		return 4;
	unsigned char cMAC[32];
	int nMAC = 32;
	decode(cBuf, strlen(cBuf), cMAC, nMAC);

	SM3_CTX cSM3;
	unsigned char cMac[32];
	SM3_Init(&cSM3);     //SM3(uuid|agentID|KA)
	SM3_Update(&cSM3, m_UUID, strlen(m_UUID));
	SM3_Update(&cSM3, m_AgentID, LENGTH_AID);
	SM3_Update(&cSM3, cKA, LENGTH_KA);
	SM3_Final(&cSM3, cMac);

	if(memcmp(cMAC, cMac, 32) != 0)
		return 6;  //check mac error
	memcpy(m_KA, cKA, LENGTH_KA);

	m_bInit = false;
	return 0;
}

int  CAssCmd::SaveConf(const char *confile)
{
#if 0
	char *p="20181010";
	SM3((unsigned char *)p, strlen(p), m_KA);    //test 37A9D2A7A499C2775E3F935E02793BF96B047F04AD1AA4D7F4C186B05F3B9AD3
	strcpy(m_UUID, "1234567890123456");
	memset(m_AgentID, 0x11, 16);
#endif	
	int nRet = 0;
	char cAID[64];
	encode((const unsigned char *)m_AgentID, LENGTH_AID, (unsigned char *)cAID);
	nRet = write_profile_string("SECRETKEY", "agentid", cAID, confile);
	nRet = write_profile_string("SECRETKEY", "uuid", m_UUID, confile);
	SM3_CTX cSM3;
	unsigned char cMac[32];
	SM3_Init(&cSM3);     //SM3(uuid|agentID|KA)
	SM3_Update(&cSM3, m_UUID, strlen(m_UUID));
	SM3_Update(&cSM3, m_AgentID, LENGTH_AID);   //strlen(m_AgentID));
	SM3_Update(&cSM3, m_KA, LENGTH_KA);
	SM3_Final(&cSM3, cMac);   
	char cBuf[128];
	encode(cMac, 32, (unsigned char *)cBuf);
	nRet = write_profile_string("SECRETKEY", "mac", cBuf, confile);
	unsigned char cEnKey[128];
	int nEnKey = 128;
	nRet = EncryptByID(m_KA, LENGTH_KA, m_UUID, strlen(m_UUID), 0, cEnKey, nEnKey, cMac);
	encode(cEnKey, nEnKey, (unsigned char *)cBuf);
	nRet = write_profile_string("SECRETKEY", "ka", cBuf, confile);
	return 0;
}
#define DEBUG_SM4 0
int  CAssCmd::PutData(char *data, unsigned char ver, unsigned char type, unsigned char flag, unsigned char sid[4], unsigned char aid[16], int macindex, unsigned char *out, int &outlen)
{
	if(m_bInit && macindex)
		return -1;
	int nError = 0;
	int nDataLen = strlen(data);
	int nPadLen = 0;
	if((flag & 0x1) != 0) // encrypt //判断是否加密
		nPadLen = 16 - nDataLen%16;
	int nMsgLen = nDataLen + nPadLen; //保证报文长度是16的整数倍
	if((nMsgLen + sizeof(CMD_HEADER))>outlen) //输出buff不足空间 //
		return 1;
	if(macindex == 0 && nPadLen>0)  //KI no encrypt，macindex=0表示注册请求与应答，不加密
		return 2;
#if DEBUG_SM4
	FILE *fp = fopen("./sm4.log", "at");
	fprintf(fp, "log sm4:index=%d\n", macindex);
#endif
	CMD_HEADER *pHeader = (CMD_HEADER *)out;  //报文头地址
	unsigned char *pData = out+sizeof(CMD_HEADER); //报文数据地址
	memset(pHeader->mac, 0, 4); //报文头的mac地址清空0
	if((flag & 0x2) != 0)  //need mac  //需要校验
	{
		unsigned char *pRKey = m_KI;
		if(macindex == 1)
			pRKey = m_KA;
		unsigned char cKey[32];  
		SM3_CTX cSM3;
		SM3_Init(&cSM3);     //SM3(pRKey|sid) SM3（KA或KI|会话标识）
		SM3_Update(&cSM3, pRKey, LENGTH_KA);
		SM3_Update(&cSM3, sid, 4);
		SM3_Final(&cSM3, cKey);
		unsigned char *pMacKey = &cKey[16];
		unsigned char cMac[32];
		SM3_Init(&cSM3);  //SM3(data|pMacKey);  SM3（报文体明文|KMac）
		SM3_Update(&cSM3, data, nDataLen);//data表示明文数据
		SM3_Update(&cSM3, pMacKey, 16);
		SM3_Final(&cSM3, cMac);
		memcpy(pHeader->mac, cMac, 4);
#if DEBUG_SM4
fprintf(fp, "CKEY:");
for(int i=0; i<32; i++)
	fprintf(fp, "%02X", cKey[i]);
fprintf(fp, "\n");
#endif
		if((flag & 0x1) != 0) //需要加密
		{
			unsigned char *pKey = cKey;
			SM4_KEY_OBJ cSM4;
			sm4_InitBlock(&cSM4, pKey, 0, 0, 1, NULL); //sm4-ecb pkcs5-padding encrypt
			int nOutLen = outlen - sizeof(CMD_HEADER);
			nError = sm4_UpdateBlock(&cSM4, (unsigned char *)data, nDataLen, pData, &nOutLen, 0);
			if(nError != 0)
				return 3;
			int nLast = outlen - sizeof(CMD_HEADER) - nOutLen;
			nError = sm4_FinalBlock(&cSM4, NULL, 0, &pData[nOutLen], &nLast, 0);
			if(nError != 0)
				return 4;
			if(nMsgLen != (nLast+nOutLen))
				return 5;
		}
		else
			memcpy(pData, data, nDataLen);
	}
	else
		memcpy(pData, data, nDataLen);
	pHeader->ver = ver;
	pHeader->type = type;
	pHeader->flag = flag;
	pHeader->reserved = 0x0;
	memcpy(pHeader->sid, sid, 4);
	memcpy(pHeader->aid, aid, 16);

	outlen = nMsgLen + sizeof(CMD_HEADER);

	zs_PutBE32(outlen, pHeader->len);
#if DEBUG_SM4
fclose(fp);
#endif
	return 0;
}

int  CAssCmd::GetData(unsigned char *msg, int msglen, int macindex, unsigned char &ver, unsigned char &type, unsigned char sid[4], unsigned char aid[16], char *data, int &datalen)
{
	if(m_bInit && macindex)
		return -1;
	int nError = 0;
	*(msg + msglen) = '\0';
	CMD_HEADER *pHeader = (CMD_HEADER *)msg;
	unsigned char *pData = msg+sizeof(CMD_HEADER);
	int nMsgLen = (int)zs_GetBE32(pHeader->len);
	if(nMsgLen != msglen)
		return 1;
	
	int nLen = datalen;
	unsigned char *pCheck = pData;
	int nDataLen = nMsgLen - sizeof(CMD_HEADER);

	if((pHeader->flag & 0x2) != 0)  //need mac
	{
		unsigned char *pRKey = m_KI;
		if(macindex == 1)
			pRKey = m_KA;
		unsigned char cKey[32];  
		SM3_CTX cSM3;
		SM3_Init(&cSM3);     //SM3(pRKey|sid)
		SM3_Update(&cSM3, pRKey, LENGTH_KA);
		SM3_Update(&cSM3, pHeader->sid, 4);
		SM3_Final(&cSM3, cKey);
		unsigned char *pMacKey = &cKey[16];
		if((pHeader->flag & 0x1) != 0)  //decrypt
		{
			if(nDataLen <16 || nDataLen%16 != 0)
				return 2;
			unsigned char *pKey = cKey;
			SM4_KEY_OBJ cSM4;
			sm4_InitBlock(&cSM4, pKey, 1, 0, 1, NULL); //sm4-ecb pkcs5-padding encrypt
			int nOutLen = datalen;
			nError = sm4_UpdateBlock(&cSM4, pData, nDataLen-16, (unsigned char *)data, &nOutLen, 1);
			if(nError != 0)
				return 3;
			int nLast = datalen - nOutLen;
			nError = sm4_FinalBlock(&cSM4, &pData[nOutLen], nDataLen-nOutLen, (unsigned char *)&data[nOutLen], &nLast, 1);
			if(nError != 0)
				return 4;
			if(datalen <= (nOutLen+nLast))
				return 5;  //overflow outbuff 
			nDataLen = nOutLen+nLast;
			pCheck = (unsigned char *)data;
		}
		unsigned char cMac[32];
		SM3_Init(&cSM3);  //SM3(pCheck|pMacKey);
		SM3_Update(&cSM3, pCheck, nDataLen);
		SM3_Update(&cSM3, pMacKey, 16);
		SM3_Final(&cSM3, cMac);
		if(memcmp(cMac, pHeader->mac, 4) != 0)
			return 6;
	}
	if(pCheck != (unsigned char *)data) //need copy
	{
		if(datalen <= nDataLen)
			return 7;  //overflow outbuff 
		memcpy(data, pCheck, nDataLen);
	}
	data[nDataLen] = '\0';
	datalen = nDataLen;

	ver = pHeader->ver;
	type = pHeader->type;
	memcpy(sid, pHeader->sid, 4);
	memcpy(aid, pHeader->aid, 16);
	
	return 0;
}

int  CAssCmd::MacByID(const unsigned char *data, int datalen, const char *id, int idlen, int keyindex, unsigned char mac[32])
{
	unsigned char *pRKey = m_KI;
	if(keyindex == 1)
	{
		if(m_bInit)
			return -1;
		pRKey = m_KA;
	}
	unsigned char cKey[32];  
	SM3_CTX cSM3;
	SM3_Init(&cSM3);     //SM3(pRKey|id)
	SM3_Update(&cSM3, pRKey, LENGTH_KA);
	SM3_Update(&cSM3, id, idlen);
	SM3_Final(&cSM3, cKey);
	unsigned char *pMacKey = &cKey[16];
	SM3_Init(&cSM3);  
	SM3_Update(&cSM3, data, datalen);
	SM3_Update(&cSM3, pMacKey, 16);
	SM3_Final(&cSM3, mac);
	return 0;
}
int  CAssCmd::EncryptByID(const unsigned char *data, int datalen, const char *id, int idlen, int keyindex, unsigned char *out, int &outlen, unsigned char mac[32])
{
	int nError = 0;
	unsigned char *pRKey = m_KI;
	if(keyindex == 1)
	{
		if(m_bInit)
			return -1;
		pRKey = m_KA;
	}
	unsigned char cKey[32];  
	SM3_CTX cSM3;
	SM3_Init(&cSM3);     //SM3(pRKey|id)
	SM3_Update(&cSM3, pRKey, LENGTH_KA);
	SM3_Update(&cSM3, id, idlen);
	SM3_Final(&cSM3, cKey);
	unsigned char *pKey = cKey;
	unsigned char *pMacKey = &cKey[16];
	SM4_KEY_OBJ cSM4;
	sm4_InitBlock(&cSM4, pKey, 0, 0, 1, NULL); //sm4-ecb pkcs5-padding encrypt
	int nOutLen = outlen;
	nError = sm4_UpdateBlock(&cSM4, data, datalen, out, &nOutLen, 0);
	if(nError != 0)
		return 1;
	int nLast = outlen - nOutLen;
	nError = sm4_FinalBlock(&cSM4, NULL, 0, &out[nOutLen], &nLast, 0);
	if(nError != 0)
		return 2;
	outlen = nOutLen + nLast;

	SM3_Init(&cSM3);  
	SM3_Update(&cSM3, data, datalen);
	SM3_Update(&cSM3, pMacKey, 16);
	SM3_Final(&cSM3, mac);
	return 0;
}
int  CAssCmd::DecryptByID(const unsigned char *edata, int edatalen, const char *id, int idlen, int keyindex, unsigned char *out, int &outlen)
{
	int nError = 0;
	if(edatalen < 16)
		return 1;
	unsigned char *pRKey = m_KI;
	if(keyindex == 1)
	{
		if(m_bInit)
			return -1;
		pRKey = m_KA;
	}
	unsigned char cKey[32];  
	SM3_CTX cSM3;
	SM3_Init(&cSM3);     //SM3(pRKey|id)
	SM3_Update(&cSM3, pRKey, LENGTH_KA);
	SM3_Update(&cSM3, id, idlen);
	SM3_Final(&cSM3, cKey);
	unsigned char *pKey = cKey;
	SM4_KEY_OBJ cSM4;
	sm4_InitBlock(&cSM4, pKey, 1, 0, 1, NULL); //sm4-ecb pkcs5-padding decrypt
	int nOutLen = outlen;
	nError = sm4_UpdateBlock(&cSM4, edata, edatalen-16, out, &nOutLen, 1);
	if(nError != 0)
		return 2;
	int nLast = outlen - nOutLen;
	nError = sm4_FinalBlock(&cSM4, &edata[nOutLen], edatalen-nOutLen, &out[nOutLen], &nLast, 1);
	if(nError != 0)
		return 2;
	outlen = nOutLen + nLast;
	return 0;
}

int CAssCmd::GetAidLen()
{
	return m_nAidLen;
}
void CAssCmd::ToAgentID(unsigned char *out)
{
	//代码修改，strcpy导致复制拷贝不全
	if(strlen(m_AgentID)<16)
	{	//strcpy((char *)out, m_AgentID);
		memcpy(out, m_AgentID, m_nAidLen);
	}
	else
	{
		memcpy(out, m_AgentID, 16);
	}
}
int  CAssCmd::GetMsgLen(char *msg, int msglen)
{
	CMD_HEADER *pHead = (CMD_HEADER *)msg;
	int nMsgLen = (int)zs_GetBE32(pHead->len);
	if(msglen != nMsgLen)
		return -1;
	if(nMsgLen < sizeof(CMD_HEADER))
		return -2;
	return nMsgLen;
}
int  CAssCmd::InitReg(char *aid, char *ka, char *mka, const char *id, int idlen)
{
	int nError = 0;
	
	unsigned char cTmp[128];
	int nTmp = 128;
	decode(ka, strlen(ka), cTmp, nTmp);
	unsigned char out[128];
	int nOut = 128;
	nError = DecryptByID(cTmp, nTmp, id, idlen, 0, out, nOut);
	if(nError != 0)
		return 1;
	nTmp = 128;
	decode(mka, strlen(mka), cTmp, nTmp);
	unsigned char mac[32];
	nError = MacByID(out, nOut, id, idlen, 0, mac);
	if(nError != 0)
		return 2;
	if(memcmp(mac, cTmp, nTmp) != 0) //验证胜败///
		return 3;
	memset(m_AgentID, 0 , 16);
	AToByte(aid, m_AgentID, strlen(aid)/2);
	memcpy(m_KA, out, LENGTH_KA);
#if 0
printf("KA:\n");
for(int i = 0; i < LENGTH_KA; i++)
{
	printf("%02X", m_KA[i]);
}
printf("\n");
printf("AID:\n");
for(int i = 0; i < 16; i++)
{
	printf("%02X", (unsigned char)m_AgentID[i]);
}
printf("\n");
#endif
	m_bInit = false;
	return 0;
}

	unsigned char BASE64DECODE[256] =
	{
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x3e,0xff,0xff,0xff,0x3f,
		0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0xff,0xff,0xff,0x40,0xff,0xff,
		0xff,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,
		0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0xff,0xff,0xff,0xff,0xff,
		0xff,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
		0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
	};

	void CAssCmd::encode(const unsigned char* in,int inlen,unsigned char* out)
	{
		const char base64digits[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		for (; inlen >= 3; inlen -= 3)
		{
			*out++ = base64digits[in[0] >> 2];
			*out++ = base64digits[((in[0] << 4) & 0x30) | (in[1] >> 4)];
			*out++ = base64digits[((in[1] << 2) & 0x3c) | (in[2] >> 6)];
			*out++ = base64digits[in[2] & 0x3f];
			in += 3;
		}
		if (inlen > 0)
		{
			unsigned char fragment;

			*out++ = base64digits[in[0] >> 2];
			fragment = (in[0] << 4) & 0x30;
			if (inlen > 1)
				fragment |= in[1] >> 4;
			*out++ = base64digits[fragment];
			*out++ = (inlen < 2) ? '=' : base64digits[(in[1] << 2) & 0x3c];
			*out++ = '=';
		}
		*out = '\0';
	}

	unsigned char *CAssCmd::decode(char *base64code, int base64length, unsigned char *out, int &outlen)
	{
		unsigned char *pCode = (unsigned char *)base64code;
		unsigned char *pBegin = out;
		unsigned char *pEnd = out + outlen;  
		unsigned char uCode;
		for (int i = 0; (i < base64length) && (out < pEnd); i++)
		{
			uCode = BASE64DECODE[pCode[i]];
			if (uCode >= 64)
				break;
			switch( i%4 )
			{
				case 0:
					*out = uCode<<2;
					break;
				case 1:
					*out = *out | (uCode >> 4);
					out++;
					if (out >= pEnd)
						break;
					*out = uCode << 4;
					break;
				case 2:
					*out = *out | (uCode >> 2);
					out++;
					if (out >= pEnd)
						break;
					*out = uCode << 6;
					break;
				case 3:
					*out = *out | uCode;
					out++;
					break;
			}
		}
		outlen = out - pBegin;
		return pBegin;
	}

	void CAssCmd::AToByte(char *asc, char *out, int outlen)
{
    unsigned char *pCh = (unsigned char *)asc;
    unsigned char *pData = (unsigned char *)out;
    unsigned char *pEnd = (unsigned char *)(out + outlen);
    for(; pData < pEnd; pData++) 
    {
        if (*pCh >='0' && *pCh <= '9')
            *pData = *pCh - '0';
        else if (*pCh >='A' && *pCh <= 'F')
            *pData = *pCh - 'A' + 10;
        else if (*pCh >='a' && *pCh <= 'f')
            *pData = *pCh - 'a' + 10;
        else
            *pData = 0;
        pCh++;
        *pData<<=4;
        if (*pCh >='0' && *pCh <= '9')
            *pData += (*pCh - '0');
        else if (*pCh >='A' && *pCh <= 'F')
            *pData += (*pCh - 'A' + 10);
        else if (*pCh >='a' && *pCh <= 'f')
            *pData += (*pCh - 'a' + 10);
        pCh++;
    }
	m_nAidLen = outlen;
}

/////////////////////////////////////////////////////////////////
CAssIni::CAssIni()
{
}

CAssIni::~CAssIni()
{
}

int CAssIni::LoadIni(char *inifile)
{
	if(!read_profile_string("SERVER", "ip", m_IP, 31, "", inifile))
		return 1;
	m_Port = read_profile_int("SERVER", "port", 3, inifile);
	m_TimeOut = read_profile_int("SERVER", "timeout", 3000, inifile);
	m_LogLevel = read_profile_int("LOG", "level", 0, inifile);
	return 0;
}
int CAssIni::GetLangId(char *inifile)
{
	m_langId = read_profile_int("Language", "id", 2052, inifile);
	return m_langId;
}

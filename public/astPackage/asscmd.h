#ifndef _ASS_CMD_N_20181009_H
#define _ASS_CMD_N_20181009_H

//报文头为32字节定长数据，格式如下
//版本：1字节；报文格式版本号，定为0x1。
//报文类型：1字节；是请求还是应答报文，请求：0x1，应答：0x2。
//标志：1字节；报文的加密与校验标志，加密：0x1，校验：0x2，加密+校验：0x3。
//保留：1字节；填 0x0。
//报文长度：4字节整形数，高字节序。整个报文的长度（字节数），包括头部32字节。
//AgentID：16字节，agentid，注册时填充0
//会话标识：4字节；一次会话的唯一标识，请求和应答使用相同的值。
//校验数据：4字节；当校验标志设置时，为校验数据；否则，填充全0。
typedef struct cmd_header
{
	unsigned char ver;
	unsigned char type;
	unsigned char flag;
	unsigned char reserved;
	unsigned char len[4];	
	unsigned char aid[16];
	unsigned char sid[4];
	unsigned char mac[4]; //4字节的校验数据
} CMD_HEADER;

#define  LENGTH_KI   32
#define  LENGTH_KA   32
#define  LENGTH_AID  16
class CAssCmd
{
public:
	CAssCmd();
	~CAssCmd();
	//设置KA///
	void InitKA(unsigned char ka[LENGTH_KA]);
	//读取配置文件//
	int  LoadConf(const char *confile);
	//保存配置文件//
	int  SaveConf(const char *confile);
	//生成报文
	//char *data, json数据串
	//unsigned char ver, 版本：0x1
	//unsigned char type, 类型：请求：0x1，应答：0x2。
	//unsigned char flag, 标志：加密：0x1，校验：0x2，加密+校验：0x3。
	//unsigned char sid[4], 会话标识
	//unsigned char aid[16], agentid
	//int macindex   校验使用的密钥索引（0:KI,1:KA）。只有注册请求和注册应答报文使用KI。
	//unsigned char *out：生成的报文, int &outlen：输入为最大长度，输出为报文长度
	int  PutData(char *data, unsigned char ver, unsigned char type, unsigned char flag, unsigned char sid[4], unsigned char aid[16], int macindex, unsigned char *out, int &outlen); 
	//解析报文，获得json数据串
	//unsigned char *msg, int msglen, 报文数据及长度
	//int macindex, 校验使用的密钥索引（0:KI,1:KA）。只有注册请求和注册应答报文使用KI。
	//unsigned char &ver, 获得的版本
	//unsigned char &type, 获得的报文类型
	//unsigned char sid[4], 获得的会话标识
	//unsigned char aid[16], agentid
	//char *data：json数据串（包含0结束符）, int &datalen：输入为最大长度，输出为json串长度=strlen(data)
	int  GetData(unsigned char *msg, int msglen, int macindex, unsigned char &ver, unsigned char &type, unsigned char sid[4], unsigned char aid[16], char *data, int &datalen);  
	//KI或KA加密，
	//const unsigned char *data, int datalen,  明文数据及长度
	//const char *id, int idlen,  散列因子及长度， SM3(KI/KA|id),前16字节为加密密钥，后16字节生成MAC
	//int keyindex   0:KI,1:KA
	//unsigned char *out:密文, int &outlen：输入为最大长度，输出为密文长度 
	//unsigned char mac[32]  明文的mac
	int  EncryptByID(const unsigned char *data, int datalen, const char *id, int idlen, int keyindex, unsigned char *out, int &outlen, unsigned char mac[32]);
	//KI或KA解密，
	//const unsigned char *edata, int edatalen,  明文数据及长度
	//const char *id, int idlen,  散列因子及长度， SM3(KI/KA|id),前16字节为解密密钥
	//int keyindex   0:KI,1:KA
	//unsigned char *out:明文, int &outlen：输入为最大长度，输出为明文长度 
	int  DecryptByID(const unsigned char *edata, int edatalen, const char *id, int idlen, int keyindex, unsigned char *out, int &outlen);

	//KI或KA算MAC，
	//const unsigned char *data, int datalen,  数据及长度
	//const char *id, int idlen,  散列因子及长度， SM3(KI/KA|id),前16字节为加密密钥，后16字节生成MAC
	//int keyindex   0:KI,1:KA
	//unsigned char mac[32]  结果mac
	int  MacByID(const unsigned char *data, int datalen, const char *id, int idlen, int keyindex, unsigned char mac[32]);
	void ToAgentID(unsigned char *out);
	int  GetMsgLen(char *msg, int msglen);
	//注册时初始化
	//char *aid：  报文的agentid，16进制字符串 
	//char *ka, char *mka ： 报文中的ka和mka数据，base64编码的字符串  //
	//const char *id, int idlen,  散列因子及长度， SM3(KI/KA|id),前16字节为加密密钥，后16字节生成MAC
	int  InitReg(char *aid, char *ka, char *mka, const char *id, int idlen);
	unsigned char *m_pKA;
	//获取AgentID的长度
	int GetAidLen();
private:
	unsigned char m_KI[LENGTH_KI];
	unsigned char m_KA[LENGTH_KA];
	bool  m_bInit;
	char  m_AgentID[64];
	int m_nAidLen;
	

public:
	char  m_UUID[64];
	void encode(const unsigned char* in,int inlen,unsigned char* out);
	unsigned char *decode(char *base64code, int base64length, unsigned char *out, int &outlen);
	void AToByte(char *asc, char *out, int outlen);
};

class CAssIni
{
public:
	CAssIni();
	~CAssIni();
	char m_IP[32];   //服务端IP
	int  m_Port;     //端口
	int  m_TimeOut;  //超时 
	int  m_LogLevel;
	int	 m_langId;
	int LoadIni(char *inifile);
	int GetLangId(char *inifile);
};



#endif

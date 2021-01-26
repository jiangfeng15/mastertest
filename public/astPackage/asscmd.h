#ifndef _ASS_CMD_N_20181009_H
#define _ASS_CMD_N_20181009_H

//����ͷΪ32�ֽڶ������ݣ���ʽ����
//�汾��1�ֽڣ����ĸ�ʽ�汾�ţ���Ϊ0x1��
//�������ͣ�1�ֽڣ���������Ӧ���ģ�����0x1��Ӧ��0x2��
//��־��1�ֽڣ����ĵļ�����У���־�����ܣ�0x1��У�飺0x2������+У�飺0x3��
//������1�ֽڣ��� 0x0��
//���ĳ��ȣ�4�ֽ������������ֽ����������ĵĳ��ȣ��ֽ�����������ͷ��32�ֽڡ�
//AgentID��16�ֽڣ�agentid��ע��ʱ���0
//�Ự��ʶ��4�ֽڣ�һ�λỰ��Ψһ��ʶ�������Ӧ��ʹ����ͬ��ֵ��
//У�����ݣ�4�ֽڣ���У���־����ʱ��ΪУ�����ݣ��������ȫ0��
typedef struct cmd_header
{
	unsigned char ver;
	unsigned char type;
	unsigned char flag;
	unsigned char reserved;
	unsigned char len[4];	
	unsigned char aid[16];
	unsigned char sid[4];
	unsigned char mac[4]; //4�ֽڵ�У������
} CMD_HEADER;

#define  LENGTH_KI   32
#define  LENGTH_KA   32
#define  LENGTH_AID  16
class CAssCmd
{
public:
	CAssCmd();
	~CAssCmd();
	//����KA///
	void InitKA(unsigned char ka[LENGTH_KA]);
	//��ȡ�����ļ�//
	int  LoadConf(const char *confile);
	//���������ļ�//
	int  SaveConf(const char *confile);
	//���ɱ���
	//char *data, json���ݴ�
	//unsigned char ver, �汾��0x1
	//unsigned char type, ���ͣ�����0x1��Ӧ��0x2��
	//unsigned char flag, ��־�����ܣ�0x1��У�飺0x2������+У�飺0x3��
	//unsigned char sid[4], �Ự��ʶ
	//unsigned char aid[16], agentid
	//int macindex   У��ʹ�õ���Կ������0:KI,1:KA����ֻ��ע�������ע��Ӧ����ʹ��KI��
	//unsigned char *out�����ɵı���, int &outlen������Ϊ��󳤶ȣ����Ϊ���ĳ���
	int  PutData(char *data, unsigned char ver, unsigned char type, unsigned char flag, unsigned char sid[4], unsigned char aid[16], int macindex, unsigned char *out, int &outlen); 
	//�������ģ����json���ݴ�
	//unsigned char *msg, int msglen, �������ݼ�����
	//int macindex, У��ʹ�õ���Կ������0:KI,1:KA����ֻ��ע�������ע��Ӧ����ʹ��KI��
	//unsigned char &ver, ��õİ汾
	//unsigned char &type, ��õı�������
	//unsigned char sid[4], ��õĻỰ��ʶ
	//unsigned char aid[16], agentid
	//char *data��json���ݴ�������0��������, int &datalen������Ϊ��󳤶ȣ����Ϊjson������=strlen(data)
	int  GetData(unsigned char *msg, int msglen, int macindex, unsigned char &ver, unsigned char &type, unsigned char sid[4], unsigned char aid[16], char *data, int &datalen);  
	//KI��KA���ܣ�
	//const unsigned char *data, int datalen,  �������ݼ�����
	//const char *id, int idlen,  ɢ�����Ӽ����ȣ� SM3(KI/KA|id),ǰ16�ֽ�Ϊ������Կ����16�ֽ�����MAC
	//int keyindex   0:KI,1:KA
	//unsigned char *out:����, int &outlen������Ϊ��󳤶ȣ����Ϊ���ĳ��� 
	//unsigned char mac[32]  ���ĵ�mac
	int  EncryptByID(const unsigned char *data, int datalen, const char *id, int idlen, int keyindex, unsigned char *out, int &outlen, unsigned char mac[32]);
	//KI��KA���ܣ�
	//const unsigned char *edata, int edatalen,  �������ݼ�����
	//const char *id, int idlen,  ɢ�����Ӽ����ȣ� SM3(KI/KA|id),ǰ16�ֽ�Ϊ������Կ
	//int keyindex   0:KI,1:KA
	//unsigned char *out:����, int &outlen������Ϊ��󳤶ȣ����Ϊ���ĳ��� 
	int  DecryptByID(const unsigned char *edata, int edatalen, const char *id, int idlen, int keyindex, unsigned char *out, int &outlen);

	//KI��KA��MAC��
	//const unsigned char *data, int datalen,  ���ݼ�����
	//const char *id, int idlen,  ɢ�����Ӽ����ȣ� SM3(KI/KA|id),ǰ16�ֽ�Ϊ������Կ����16�ֽ�����MAC
	//int keyindex   0:KI,1:KA
	//unsigned char mac[32]  ���mac
	int  MacByID(const unsigned char *data, int datalen, const char *id, int idlen, int keyindex, unsigned char mac[32]);
	void ToAgentID(unsigned char *out);
	int  GetMsgLen(char *msg, int msglen);
	//ע��ʱ��ʼ��
	//char *aid��  ���ĵ�agentid��16�����ַ��� 
	//char *ka, char *mka �� �����е�ka��mka���ݣ�base64������ַ���  //
	//const char *id, int idlen,  ɢ�����Ӽ����ȣ� SM3(KI/KA|id),ǰ16�ֽ�Ϊ������Կ����16�ֽ�����MAC
	int  InitReg(char *aid, char *ka, char *mka, const char *id, int idlen);
	unsigned char *m_pKA;
	//��ȡAgentID�ĳ���
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
	char m_IP[32];   //�����IP
	int  m_Port;     //�˿�
	int  m_TimeOut;  //��ʱ 
	int  m_LogLevel;
	int	 m_langId;
	int LoadIni(char *inifile);
	int GetLangId(char *inifile);
};



#endif

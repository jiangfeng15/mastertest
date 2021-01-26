#ifndef SM4_H
#define SM4_H
/* define SMS4DBG flag for debug these code */
#define SMS4DBG

#ifdef SMS4DBG
//#include
#endif /* SMS4DBG */

//#include

/* reserved for saving interface */
/*#include "sms4.h"*/

#ifndef unlong
typedef unsigned int unlong;
#endif /* unlong */

#ifndef unchar
typedef unsigned char unchar;
#endif /* unchar */

#ifndef ULONG
#define ULONG unsigned int
#endif
#ifndef UCHAR
#define UCHAR unsigned char
#endif
#ifndef UINT
#define UINT unsigned int
#endif

/* define SMS4CROL for rotating left */
#define SMS4CROL(uval, bits) ((uval << bits) | (uval >> (0x20 - bits)))

/* define MASK code for selecting expected bits from a 32 bits value */
#define SMS4MASK3 0xFF000000
#define SMS4MASK2 0x00FF0000
#define SMS4MASK1 0x0000FF00
#define SMS4MASK0 0x000000FF

//代替上面  ///
/* rotating left */
#define SM4ROLEFT(uval, bits) (((uval) << (bits)) | ((uval) >> (0x20 - (bits))))

/* define MASK code for selecting expected bits from a 32 bits value */
#define SM4_MASK0 0x000000FF
#define SM4_MASK1 0x0000FF00
#define SM4_MASK2 0x00FF0000
#define SM4_MASK3 0xFF000000
//////////////////
/*
unlong *SMS4SetKey(unlong *ulkey, unlong flag);
unlong *SMS4Encrypt(unlong *psrc, unlong lgsrc, unlong rk[]);
unlong *SMS4Decrypt(unlong *psrc, unlong lgsrc, unlong derk[]);

unlong *SMS4Encrypt(unlong *psrc, unlong lgsrc, unlong *pdst, unlong rk[]);
unlong *SMS4Decrypt(unlong *psrc, unlong lgsrc, unlong *pdst, unlong derk[]);
*/
//extern unlong ENRK[32];
//extern unlong DERK[32];

////////////////////////
typedef struct sm4_key
{
    ULONG ERK[32];  /*rk for encrypt*/
    ULONG DRK[32];  /*rk for decrypt, DRK[31]=ERK[0], ... ,DRK[0]=ERK[31]*/
    ULONG MK[4];    /*sm4 key*/
}SM4_KEY;

typedef struct sm4_key_obj
{
    SM4_KEY key;

    unsigned char iv[16];    //CBC use
    unsigned int  ivtmp[4];  //IV used
    unsigned char left[16];  //last left data
    unsigned char flag;      //1:can decrypt
    unsigned char mode;      //0:ECB，1：CBC
    unsigned char leftsize;  //size of left
    unsigned char padtype;   //0:padding 0 or not padding , 1:PKCS5 padding
}SM4_KEY_OBJ;

//flag:1 同时生成解密密钥  ////
int sm4_SetKey(SM4_KEY *keyObj, unsigned char *key, int flag);
ULONG *sm4_Encrypt(SM4_KEY *keyObj,ULONG *psrc, ULONG srclen, ULONG *pdst);
ULONG *sm4_Encrypt_S(SM4_KEY *keyObj,ULONG *psrc, ULONG srclen);
ULONG *sm4_Decrypt(SM4_KEY *keyObj,ULONG *psrc, ULONG srclen, ULONG *pdst);
ULONG *sm4_Decrypt_S(SM4_KEY *keyObj,ULONG *psrc, ULONG srclen);

//SM4 加密
//padding：0,填充0；1,PKCS5填充 ///////////
int enin_sm4(const unsigned char *d,int dlen,unsigned char *ed,int *ed_len, SM4_KEY *keyObj, int padding);
//SM4 解密
//padding：0,填充0；1,PKCS5填充 //////////////////
int dein_sm4(const unsigned char*ed,int ed_len,unsigned char*d,int*d_len, SM4_KEY *keyObj, int padding);

//SM4_CBC_MAC
int sm4_CBC_MAC(const unsigned char *in,int inlen,unsigned char *iv,int ivlen, SM4_KEY *keyObj, unsigned char *out);

int sm4_CBC(const unsigned char *in,int inlen,unsigned char *iv,int ivlen, SM4_KEY *keyObj, unsigned char *out, int *outlen);

int dein_sm4_CBC(const unsigned char*ed,int ed_len,unsigned char*d,int*d_len, unsigned char *iv, int ivlen, SM4_KEY *keyObj, int padding);

//flag:1,decrypt;0:encrypt。 if decrpt, sm4_InitBlock() must use 1, 2:SM4-CBC MAC
//mode:0,ECB;1,CBC
//padding: 0,no padding(padding 0); 1,pkcs5
int sm4_InitBlock(SM4_KEY_OBJ *sm4obj,unsigned char *key,int flag, int mode, int padding, unsigned char *iv);

int sm4_UpdateBlock(SM4_KEY_OBJ *sm4obj, const unsigned char *in,int inlen, unsigned char *out,int *outlen, int flag);

//in is used by decrypt,is NULL when encrypt
int sm4_FinalBlock(SM4_KEY_OBJ *sm4obj, const unsigned char *in,int inlen, unsigned char *out,int *outlen, int flag);

unsigned int zs_GetBE32(unsigned char *d);
void zs_PutBE32(unsigned int a, unsigned char *d);
#endif // SM4_H

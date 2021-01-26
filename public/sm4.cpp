/* sms4.c
** SMS4 Encryption algorithm for wireless networks
**
** $Id: sms4.c 2009-12-31 14:41:57 tao.tang <$">emhmily@gmail.com>$
**
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc.
**/

#include <stdio.h>
#include <string.h>
#include "sm4.h"

/* Encryption key: 128bits */
//static unlong MK[4] = {0x01234567,0x89abcdef,0xfedcba98,0x76543210};

/* System parameter */
static ULONG FK[4] = {0xa3b1bac6,0x56aa3350,0x677d9197,0xb27022dc};

/* fixed parameter */
static ULONG CK[32] =
{
0x00070e15,0x1c232a31,0x383f464d,0x545b6269,
0x70777e85,0x8c939aa1,0xa8afb6bd,0xc4cbd2d9,
0xe0e7eef5,0xfc030a11,0x181f262d,0x343b4249,
0x50575e65,0x6c737a81,0x888f969d,0xa4abb2b9,
0xc0c7ced5,0xdce3eaf1,0xf8ff060d,0x141b2229,
0x30373e45,0x4c535a61,0x686f767d,0x848b9299,
0xa0a7aeb5,0xbcc3cad1,0xd8dfe6ed,0xf4fb0209,
0x10171e25,0x2c333a41,0x484f565d,0x646b7279
};

/* buffer for round encryption key */
//static unlong ENRK[32];
//static unlong DERK[32];

#ifdef SMS4DBG
/* original contents for debugging */
#if 0
static ULONG pData[4] =
{
0x01234567,
0x89abcdef,
0xfedcba98,
0x76543210
};

/* original contents for debugging */
static ULONG pData2[9] =
{
0x01234567,
0x89abcdef,
0xfedcba98,
0x76543210,
0x12121212,
0x34343434,
0x56565656,
0x78787878,
0x12341234
};
#endif /* SMS4DBG */
#endif

////////////////////////////////////////////////////////////////
/* Sbox table: 8bits input convert to 8 bits output*/
static UCHAR SBOX_TABLE[256] =
{
0xd6,0x90,0xe9,0xfe,0xcc,0xe1,0x3d,0xb7,0x16,0xb6,0x14,0xc2,0x28,0xfb,0x2c,0x05,
0x2b,0x67,0x9a,0x76,0x2a,0xbe,0x04,0xc3,0xaa,0x44,0x13,0x26,0x49,0x86,0x06,0x99,
0x9c,0x42,0x50,0xf4,0x91,0xef,0x98,0x7a,0x33,0x54,0x0b,0x43,0xed,0xcf,0xac,0x62,
0xe4,0xb3,0x1c,0xa9,0xc9,0x08,0xe8,0x95,0x80,0xdf,0x94,0xfa,0x75,0x8f,0x3f,0xa6,
0x47,0x07,0xa7,0xfc,0xf3,0x73,0x17,0xba,0x83,0x59,0x3c,0x19,0xe6,0x85,0x4f,0xa8,
0x68,0x6b,0x81,0xb2,0x71,0x64,0xda,0x8b,0xf8,0xeb,0x0f,0x4b,0x70,0x56,0x9d,0x35,
0x1e,0x24,0x0e,0x5e,0x63,0x58,0xd1,0xa2,0x25,0x22,0x7c,0x3b,0x01,0x21,0x78,0x87,
0xd4,0x00,0x46,0x57,0x9f,0xd3,0x27,0x52,0x4c,0x36,0x02,0xe7,0xa0,0xc4,0xc8,0x9e,
0xea,0xbf,0x8a,0xd2,0x40,0xc7,0x38,0xb5,0xa3,0xf7,0xf2,0xce,0xf9,0x61,0x15,0xa1,
0xe0,0xae,0x5d,0xa4,0x9b,0x34,0x1a,0x55,0xad,0x93,0x32,0x30,0xf5,0x8c,0xb1,0xe3,
0x1d,0xf6,0xe2,0x2e,0x82,0x66,0xca,0x60,0xc0,0x29,0x23,0xab,0x0d,0x53,0x4e,0x6f,
0xd5,0xdb,0x37,0x45,0xde,0xfd,0x8e,0x2f,0x03,0xff,0x6a,0x72,0x6d,0x6c,0x5b,0x51,
0x8d,0x1b,0xaf,0x92,0xbb,0xdd,0xbc,0x7f,0x11,0xd9,0x5c,0x41,0x1f,0x10,0x5a,0xd8,
0x0a,0xc1,0x31,0x88,0xa5,0xcd,0x7b,0xbd,0x2d,0x74,0xd0,0x12,0xb8,0xe5,0xb4,0xb0,
0x89,0x69,0x97,0x4a,0x0c,0x96,0x77,0x7e,0x65,0xb9,0xf1,0x09,0xc5,0x6e,0xc6,0x84,
0x18,0xf0,0x7d,0xec,0x3a,0xdc,0x4d,0x20,0x79,0xee,0x5f,0x3e,0xd7,0xcb,0x39,0x48
};
unsigned int _GetBE32(unsigned char *d)
{
    unsigned int nReturn;
    nReturn = (((unsigned int)d[0]) << 24) | (((unsigned int) d[1]) << 16)
            | (((unsigned int)d[2]) << 8) | ((unsigned int)d[3]);
    return nReturn;
}
void _PutBE32(unsigned int a, unsigned char *d)
{
    d[0] = (unsigned char)((a >> 24) & 0xff);
    d[1] = (unsigned char)((a >> 16) & 0xff);
    d[2] = (unsigned char)((a >> 8) & 0xff);
    d[3] = (unsigned char)(a & 0xff);
}

unsigned int zs_GetBE32(unsigned char *d)
{
    return _GetBE32(d);
}
void zs_PutBE32(unsigned int a, unsigned char *d)
{
    _PutBE32(a, d);
}
/////
/*
private function:
*/
void SM4_InitRK(SM4_KEY *keyObj, ULONG flag);
ULONG SMS4_RKTL(ULONG k);
static ULONG SMS4CalciRK(ULONG a);

static ULONG SMS4Lt(ULONG a);
static ULONG SMS4F(ULONG x0, ULONG x1, ULONG x2, ULONG x3, ULONG rk);
/*=============================================================================
** private function:
**   look up in SboxTable and get the related value.
** args:    [in] inch: 0x00~0xFF (8 bits unsigned value).
**============================================================================*/
//static UCHAR SMS4Sbox(UCHAR in)
//{
//    return SBOX_TABLE[in];
//}


int sm4_SetKey(SM4_KEY *keyObj, unsigned char *key, int flag)
{
    int i;
    if (key != NULL && keyObj != NULL)
    {
        for(i = 0; i < 4; i++)
        {
            keyObj->MK[i] = ((ULONG)key[i*4]);
            keyObj->MK[i] <<= 8;
            keyObj->MK[i] += ((ULONG)key[i*4+1]);
            keyObj->MK[i] <<= 8;
            keyObj->MK[i] += ((ULONG)key[i*4+2]);
            keyObj->MK[i] <<= 8;
            keyObj->MK[i] += ((ULONG)key[i*4+3]);
        }
        SM4_InitRK(keyObj, flag);
    }
    else
        return 1;
    return 0;
};

int enin_sm4(const unsigned char *d,int dlen,unsigned char *ed,int *ed_len, SM4_KEY *keyObj, int padding)
{
    int nTotal = *ed_len;
    int nPad = 16 - dlen%16;
    unsigned char cPad = (unsigned char)nPad;
    int i=0,j=0;
    int nEn = 0;
    unsigned char *pData = NULL;
    unsigned char *pE = NULL;
    unsigned int tmp[4];
    unsigned char last[16];
    int nData;
    if(padding == 0) //ZERO padding
    {
        nPad = nPad%16;
        cPad = 0x0;
    }
    nEn = dlen+nPad;
    if(nTotal < nEn) //not enough buff
        return 1;
    pData = (unsigned char *)d;
    pE = ed;
    for(nData = dlen; nData>=16; nData-=16)
    {
        tmp[0] = _GetBE32(pData);
        pData+=4;
        tmp[1] = _GetBE32(pData);
        pData+=4;
        tmp[2] = _GetBE32(pData);
        pData+=4;
        tmp[3] = _GetBE32(pData);
        pData+=4;
        sm4_Encrypt_S(keyObj,tmp, 16);
        for(j = 0; j < 4; j++,pE+=4)
            _PutBE32(tmp[j], pE);
    }
    if(cPad > 0 || nData > 0)
    {
        memcpy(last, pData, nData);
        for(i=0; i < nPad; i++) //padding
            last[nData+i] = cPad;
        pData = last;
        tmp[0] = _GetBE32(pData);
        pData+=4;
        tmp[1] = _GetBE32(pData);
        pData+=4;
        tmp[2] = _GetBE32(pData);
        pData+=4;
        tmp[3] = _GetBE32(pData);
        pData+=4;
        sm4_Encrypt_S(keyObj,tmp, 16);
        for(j = 0; j < 4; j++,pE+=4)
            _PutBE32(tmp[j], pE);
    }
    *ed_len = nEn;
    return 0;
}

int dein_sm4(const unsigned char*ed,int ed_len,unsigned char*d,int*d_len, SM4_KEY *keyObj, int padding)
{
    int i = 0, j = 0;
    int nTotal = 0;
    unsigned char *pData = NULL;
    unsigned char *pE = NULL;
    unsigned int tmp[4];
    int nData = 0;
    int nPad = 0;
    if(padding > 1 || padding < 0) //NO PADDING mode
        return 4;
    nTotal = *d_len;
    if (nTotal < ed_len || ed_len%16 != 0)
        return 1;
    pData = (unsigned char *)ed;
    pE = d;
    for(nData = ed_len; nData>=16; nData-=16)
    {
        tmp[0] = _GetBE32(pData);
        pData+=4;
        tmp[1] = _GetBE32(pData);
        pData+=4;
        tmp[2] = _GetBE32(pData);
        pData+=4;
        tmp[3] = _GetBE32(pData);
        pData+=4;
        sm4_Decrypt_S(keyObj,tmp, 16);
        for(j = 0; j < 4; j++,pE+=4)
            _PutBE32(tmp[j], pE);
    }
    nPad = (int)(d[ed_len-1]);
    if(padding == 0)
        nPad = 0;
    else if(padding==1 && (nPad<=0 || nPad>16))
        return 3;
    if(padding == 1) //PKCS5
    {
        for(i=1; i<=nPad; i++)
        {
            if(nPad != (int)(d[ed_len-i]))
                return 5;
        }
    }
    *d_len = ed_len - nPad;
    return 0;
}

ULONG *sm4_Encrypt_S(SM4_KEY *keyObj,ULONG *psrc, ULONG srclen)
{
    ULONG *pRet = NULL;
    ULONG i = 0;

    ULONG ulbuf[36];

    ULONG ulCnter = 0;
    ULONG ulTotal = (srclen >> 4);


    if(psrc != NULL)
    {
        pRet = psrc;

        /* !!!It's a temporary scheme: start!!! */
        /*========================================
        ** 16 bytes(128 bits) is deemed as an unit.
        **======================================*/
        while (ulCnter<ulTotal)
        {
            /* reset number counter */
            i = 0;

            /* filled up with 0*/
            memset(ulbuf, 0, sizeof(ulbuf));
            memcpy(ulbuf, psrc, 16);
#ifdef SMS4DBG0
            printf("0x%08x, 0x%08x, 0x%08x, 0x%08x, \n",
                   ulbuf[0], ulbuf[1], ulbuf[2], ulbuf[3]);
#endif /* SMS4DBG0 */

            while(i<32)
            {
                ulbuf[i+4] = SMS4F(ulbuf[i], ulbuf[i+1],
                                   ulbuf[i+2], ulbuf[i+3], keyObj->ERK[i]);
#ifdef SMS4DBG0
                printf("0x%08x, \n", ulbuf[i+4]);
#endif /* SMS4DBG0 */
                i++;
            }

            /* save encrypted contents to original area */
            psrc[0] = ulbuf[35];
            psrc[1] = ulbuf[34];
            psrc[2] = ulbuf[33];
            psrc[3] = ulbuf[32];

            ulCnter++;
            psrc += 4;
        }
        /* !!!It's a temporary scheme: end!!! */
    }

    return pRet;
}

ULONG *sm4_Encrypt(SM4_KEY *keyObj,ULONG *psrc, ULONG srclen, ULONG *pdst)
//unlong *SMS4Encrypt(unlong *psrc, unlong lgsrc, unlong rk[])
{
    ULONG *pRet = NULL;
    ULONG i = 0;

    ULONG ulbuf[36];

    ULONG ulCnter = 0;
    ULONG ulTotal = (srclen >> 4);


    if(psrc != NULL && pdst != NULL)
    {
        pRet = pdst;

        /* !!!It's a temporary scheme: start!!! */
        /*========================================
        ** 16 bytes(128 bits) is deemed as an unit.
        **======================================*/
        while (ulCnter<ulTotal)
        {
            /* reset number counter */
            i = 0;

            /* filled up with 0*/
            memset(ulbuf, 0, sizeof(ulbuf));
            memcpy(ulbuf, psrc, 16);
#ifdef SMS4DBG0
            printf("0x%08x, 0x%08x, 0x%08x, 0x%08x, \n",
                   ulbuf[0], ulbuf[1], ulbuf[2], ulbuf[3]);
#endif /* SMS4DBG0 */

            while(i<32)
            {
                ulbuf[i+4] = SMS4F(ulbuf[i], ulbuf[i+1],
                                   ulbuf[i+2], ulbuf[i+3], keyObj->ERK[i]);
#ifdef SMS4DBG0
                printf("0x%08x, \n", ulbuf[i+4]);
#endif /* SMS4DBG0 */
                i++;
            }

            /* save encrypted contents to original area */
            pdst[0] = ulbuf[35];
            pdst[1] = ulbuf[34];
            pdst[2] = ulbuf[33];
            pdst[3] = ulbuf[32];

            ulCnter++;
            psrc += 4;
            pdst += 4;
        }
        /* !!!It's a temporary scheme: end!!! */
    }

    return pRet;
}

ULONG *sm4_Decrypt(SM4_KEY *keyObj,ULONG *psrc, ULONG srclen, ULONG *pdst)
{
    ULONG *pRet = NULL;
    ULONG i = 0;

    ULONG ulbuf[36];

    ULONG ulCnter = 0;
    ULONG ulTotal = (srclen >> 4);


    if(psrc != NULL && pdst != NULL)
    {
        pRet = pdst;

        /* !!!It's a temporary scheme: start!!! */
        /*========================================
        ** 16 bytes(128 bits) is deemed as an unit.
        **======================================*/
        while (ulCnter<ulTotal)
        {
            /* reset number counter */
            i = 0;

            /* filled up with 0*/
            memset(ulbuf, 0, sizeof(ulbuf));
            memcpy(ulbuf, psrc, 16);
#ifdef SMS4DBG0
            printf("0x%08x, 0x%08x, 0x%08x, 0x%08x, \n",
                   ulbuf[0], ulbuf[1], ulbuf[2], ulbuf[3]);
#endif /* SMS4DBG0 */

            while(i<32)
            {
                ulbuf[i+4] = SMS4F(ulbuf[i], ulbuf[i+1],
                                   ulbuf[i+2], ulbuf[i+3], keyObj->DRK[i]);
#ifdef SMS4DBG0
                printf("0x%08x, \n", ulbuf[i+4]);
#endif /* SMS4DBG0 */
                i++;
            }

            /* save encrypted contents to original area */
            pdst[0] = ulbuf[35];
            pdst[1] = ulbuf[34];
            pdst[2] = ulbuf[33];
            pdst[3] = ulbuf[32];

            ulCnter++;
            psrc += 4;
            pdst += 4;
        }
        /* !!!It's a temporary scheme: end!!! */
    }

    return pRet;
}

ULONG *sm4_Decrypt_S(SM4_KEY *keyObj,ULONG *psrc, ULONG srclen)
{
    ULONG *pRet = NULL;
    ULONG i = 0;

    ULONG ulbuf[36];

    ULONG ulCnter = 0;
    ULONG ulTotal = (srclen >> 4);


    if(psrc != NULL)
    {
        pRet = psrc;

        /* !!!It's a temporary scheme: start!!! */
        /*========================================
        ** 16 bytes(128 bits) is deemed as an unit.
        **======================================*/
        while (ulCnter<ulTotal)
        {
            /* reset number counter */
            i = 0;

            /* filled up with 0*/
            memset(ulbuf, 0, sizeof(ulbuf));
            memcpy(ulbuf, psrc, 16);
#ifdef SMS4DBG0
            printf("0x%08x, 0x%08x, 0x%08x, 0x%08x, \n",
                   ulbuf[0], ulbuf[1], ulbuf[2], ulbuf[3]);
#endif /* SMS4DBG0 */

            while(i<32)
            {
                ulbuf[i+4] = SMS4F(ulbuf[i], ulbuf[i+1],
                                   ulbuf[i+2], ulbuf[i+3], keyObj->DRK[i]);
#ifdef SMS4DBG0
                printf("0x%08x, \n", ulbuf[i+4]);
#endif /* SMS4DBG0 */
                i++;
            }

            /* save encrypted contents to original area */
            psrc[0] = ulbuf[35];
            psrc[1] = ulbuf[34];
            psrc[2] = ulbuf[33];
            psrc[3] = ulbuf[32];

            ulCnter++;
            psrc += 4;
        }
        /* !!!It's a temporary scheme: end!!! */
    }

    return pRet;
}
#if 0
/*== Private Funtion ==*/
ULONG SMS4_RKTL(ULONG k)
{
    ULONG uRK = 0;
    ULONG uB = 0;
    UCHAR uA0 = (UCHAR)(k & SM4_MASK0);
    UCHAR uA1 = (UCHAR)((k & SM4_MASK1)>>8);
    UCHAR uA2 = (UCHAR)((k & SM4_MASK2)>>16);
    UCHAR uA3 = (UCHAR)((k & SM4_MASK3)>>24);
    UCHAR uB0 = SBOX_TABLE[uA0];
    UCHAR uB1 = SBOX_TABLE[uA1];
    UCHAR uB2 = SBOX_TABLE[uA2];
    UCHAR uB3 = SBOX_TABLE[uA3];
    return 0;
}
#endif
void SM4_InitRK(SM4_KEY *keyObj, ULONG flag)
{
    ULONG k[36];
    ULONG i = 0;

    k[0] = keyObj->MK[0]^FK[0];
    k[1] = keyObj->MK[1]^FK[1];
    k[2] = keyObj->MK[2]^FK[2];
    k[3] = keyObj->MK[3]^FK[3];

    for(; i<32; i++)
    {
        k[i+4] = k[i] ^ (SMS4CalciRK(k[i+1]^k[i+2]^k[i+3]^CK[i]));
        keyObj->ERK[i] = k[i+4];
    }

    if (flag != 0x00)
    {
        for (i=0; i<32; i++)
        {
            keyObj->DRK[i] = keyObj->ERK[31-i];
        }
    }
}

static ULONG SMS4CalciRK(ULONG a)
{
    ULONG b = 0;
    ULONG rk = 0;
    UCHAR a0 = (UCHAR)(a & SM4_MASK0);
    UCHAR a1 = (UCHAR)((a & SM4_MASK1) >> 8);
    UCHAR a2 = (UCHAR)((a & SM4_MASK2) >> 16);
    UCHAR a3 = (UCHAR)((a & SM4_MASK3) >> 24);
    UCHAR b0 = SBOX_TABLE[a0];
    UCHAR b1 = SBOX_TABLE[a1];
    UCHAR b2 = SBOX_TABLE[a2];
    UCHAR b3 = SBOX_TABLE[a3];

    b = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
    rk = b^(SM4ROLEFT(b, 13))^(SM4ROLEFT(b, 23));

    return rk;
}

static ULONG SMS4Lt(ULONG a)
{
    ULONG b = 0;
    ULONG c = 0;
    UCHAR a0 = (UCHAR)(a & SM4_MASK0);
    UCHAR a1 = (UCHAR)((a & SM4_MASK1) >> 8);
    UCHAR a2 = (UCHAR)((a & SM4_MASK2) >> 16);
    UCHAR a3 = (UCHAR)((a & SM4_MASK3) >> 24);
    UCHAR b0 = SBOX_TABLE[a0];
    UCHAR b1 = SBOX_TABLE[a1];
    UCHAR b2 = SBOX_TABLE[a2];
    UCHAR b3 = SBOX_TABLE[a3];

    b =b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
    c =b^(SM4ROLEFT(b, 2))^(SM4ROLEFT(b, 10))^(SM4ROLEFT(b, 18))^(SM4ROLEFT(b, 24));

    return c;
}

static ULONG SMS4F(ULONG x0, ULONG x1, ULONG x2, ULONG x3, ULONG rk)
{
    return (x0^SMS4Lt(x1^x2^x3^rk));
}
///SM4 CBC//////////////////////
int sm4_CBC_MAC(const unsigned char *in,int inlen,unsigned char *iv,int ivlen, SM4_KEY *keyObj, unsigned char *out)
{
    int len = 0;
    unsigned int tmp[4];
    unsigned int tmpiv[4];
    int i = 0;
    unsigned char last[16];
    unsigned int *pIV = tmpiv;
    if(in == NULL || iv == NULL || keyObj == NULL || out == NULL)
    {
        return 1;
    }
    for(i = 0; i < 4; i++) //////ivת���ֽ��� /////
    {
        tmpiv[i] = _GetBE32(&iv[i*4]);
    }
    len = inlen;
    while(len >= 16)
    {
        for(i = 0; i < 4; i++)
        {
            tmp[i] = _GetBE32((unsigned char *)(&in[i*4]));
            pIV[i] = tmp[i] ^ pIV[i];
        }
        sm4_Encrypt_S(keyObj, pIV, 16);
        in += 16;
        len -= 16;
    }
    if(len > 0) //����һ�鲻��16�ֽ�
    {
        memcpy(last, in, len);
        for(i = 0; i < 4; i++)
        {
            tmp[i] = _GetBE32(&last[i*4]);
            pIV[i] = tmp[i] ^ pIV[i];
        }
        sm4_Encrypt_S(keyObj, pIV, 16);
    }
    for(i = 0; i < 4; i++)
        _PutBE32(pIV[i], &out[i*4]);
    return 0;
}

int sm4_CBC(const unsigned char *in,int inlen,unsigned char *iv,int ivlen, SM4_KEY *keyObj, unsigned char *out, int *outlen)
{
    int len = inlen;
    unsigned int tmp[4];
    unsigned int tmpiv[4];
    unsigned int *pIV = tmpiv;
    unsigned char *pE = out;
    unsigned char last[16] = {0};
    int i=0,j=0;
    if(in == NULL || iv == NULL || keyObj == NULL || out == NULL)
        return 1;
    for(i = 0; i < 4; i++) ////ivת���ֽ���/////
    {
        tmpiv[i] = _GetBE32(&iv[i*4]);
    }
    while(len >= 16)
    {
        for(i = 0; i < 4; i++)
        {
            tmp[i] = _GetBE32((unsigned char *)(&in[i*4]));
            pIV[i] = tmp[i] ^ pIV[i];
        }
        sm4_Encrypt_S(keyObj, pIV, 16);

        for(j = 0; j < 4; j++,pE+=4)
            _PutBE32(pIV[j], pE);
        in += 16;
        len -= 16;
    }

    if(len >= 0) //////����һ��////
    {
        memset(last, 16-len, 16);
        memcpy(last, in, len);
//        for(int j = len; j<16; j++)
//            last[j] = (unsigned char)(16-len);
        for(i = 0; i < 4; i++)
        {
            tmp[i] = _GetBE32(&last[i*4]);
            pIV[i] = tmp[i] ^ pIV[i];
        }
    sm4_Encrypt_S(keyObj, pIV, 16);
    for(j = 0; j < 4; j++,pE+=4)
        _PutBE32(pIV[j], pE);
    }
    *outlen = (int)(pE - out);
    return 0;
}

int dein_sm4_CBC(const unsigned char*ed,int ed_len,unsigned char*d,int*d_len, unsigned char *iv, int ivlen, SM4_KEY *keyObj, int padding)
{
    int nTotal = *d_len;
    unsigned char *pData = (unsigned char *)ed;
    unsigned char *pE = d;
    unsigned int tmp[4];
    int nData = 0;
    int nPad = 0;
    unsigned int tmpiv[4];
    unsigned int tmpivpre[4];
    unsigned int *pIV = tmpiv;
    unsigned int *pIVp = tmpivpre;
    unsigned int *pIVtmp = NULL;
    int i=0,j=0;
    if(padding > 1 || padding < 0) //NO PADDING mode
        return 4;

    if (nTotal < ed_len || ed_len%16 != 0)
        return 1;

    for(i = 0; i < 4; i++) ////ivת���ֽ��� ////
    {
        tmpiv[i] = _GetBE32(&iv[i*4]);
    }
    for(nData = ed_len; nData>=16; nData-=16)
    {
        tmp[0] = _GetBE32(pData);
        pIVp[0] = tmp[0];
        pData+=4;
        tmp[1] = _GetBE32(pData);
        pIVp[1] = tmp[1];
        pData+=4;
        tmp[2] = _GetBE32(pData);
        pIVp[2] = tmp[2];
        pData+=4;
        tmp[3] = _GetBE32(pData);
        pIVp[3] = tmp[3];
        pData+=4;
        sm4_Decrypt_S(keyObj,tmp, 16);
        for(j = 0; j < 4; j++,pE+=4)
            _PutBE32(tmp[j]^pIV[j], pE);
        pIVtmp = pIVp;
        pIVp = pIV;
        pIV = pIVtmp;
    }
    nPad = (int)(d[ed_len-1]);
    if(padding == 0)
        nPad = 0;
    else if(padding==1 && (nPad<=0 || nPad>16))
        return 3;
    if(padding == 1) //PKCS5
    {
        for(i=1; i<=nPad; i++)
        {
            if(nPad != (int)(d[ed_len-i]))
                return 5;
        }
    }
    *d_len = ed_len - nPad;
    return 0;
}

int sm4_InitBlock(SM4_KEY_OBJ *sm4obj,unsigned char *key,int flag, int mode, int padding, unsigned char *iv)
{
    int nError = 0;
    int i = 0;
    if(sm4obj==NULL || key==NULL || flag<0 || flag>2 || mode<0 || mode>2)
        return 1;

    nError = sm4_SetKey(&sm4obj->key, key, flag);
    if(nError != 0)
        return 2;
    sm4obj->flag = (unsigned char)flag;
    sm4obj->mode = (unsigned char)mode;
    if(padding == 0)
        sm4obj->padtype = 0;
    else
        sm4obj->padtype = 1;
    if(mode > 0)
    {
        if(iv == NULL)
        {
            return 2;
        }
        else
        {
            memcpy(sm4obj->iv, iv, 16);
            for(i = 0; i < 4; i++) ////ivת���ֽ��� //////
            {
                sm4obj->ivtmp[i] = _GetBE32(&iv[i*4]);
            }
        }
    }
    memset(sm4obj->left, 0, 16);
    sm4obj->leftsize = 0;
    return 0;
}

int sm4_UpdateBlock(SM4_KEY_OBJ *sm4obj, const unsigned char *in,int inlen, unsigned char *out,int *outlen, int flag)
{
    int len = inlen;
    unsigned int tmp[4];
    unsigned int tmpivpre[4];
    unsigned int *pIV;
    unsigned char *pE = out;
    int nOut = 0;
    int i,j;
    int nUsed = 16;
    if(sm4obj == NULL || in == NULL || ((flag!=2) && (out==NULL || outlen == NULL)))
        return 1;
    pIV = sm4obj->ivtmp;
    while(len > 0)
    {
        if(sm4obj->leftsize > 0)
            nUsed = (int)(16 - sm4obj->leftsize);
        else
            nUsed = 16;
        if(nUsed > len)
            nUsed = len;
        memcpy(&sm4obj->left[sm4obj->leftsize], in, nUsed);
        sm4obj->leftsize+=(unsigned char)nUsed;
        len -= nUsed;
        in += nUsed;
        if(sm4obj->leftsize == 16) //a block
        {
            for(i = 0; i < 4; i++)
            {
                tmp[i] = _GetBE32(&sm4obj->left[i*4]);
                if(flag == 1)  //decrypt
                {
                    if(sm4obj->mode == 1)   //CBC
                        tmpivpre[i] = tmp[i];
                }
                else //encrypt or mac
                {
                    if(sm4obj->mode == 0)  //ECB
                        pIV[i] = tmp[i];
                    else  //CBC
                        pIV[i] = tmp[i] ^ pIV[i];
                }
            }
            if(flag == 1)  //decrypt
            {
                sm4_Decrypt_S(&sm4obj->key, tmp, 16);
                if((nOut + 16) <= *outlen)
                {
                    for(j = 0; j < 4; j++,pE+=4)
                    {
                        if(sm4obj->mode == 0)  //ECB
                            _PutBE32(tmp[j], pE);
                        else
                        {
                            _PutBE32(tmp[j]^pIV[j], pE);
                            pIV[j] = tmpivpre[j];  //saved iv
                        }
                    }
                    nOut+=16;
                }
                else  //not out buffer
                    return 2;
            }
            else  //encrypt or mac
            {
                sm4_Encrypt_S(&sm4obj->key, pIV, 16);
                if(flag == 0) //encrypt
                {
                    if((nOut + 16) <= *outlen)
                    {
                        for(j = 0; j < 4; j++,pE+=4)
                            _PutBE32(pIV[j], pE);

                        nOut+=16;
                    }
                    else  //not out buffer
                        return 2;
                }
            }
            sm4obj->leftsize = 0;
        }  //end a block
    }  //end while
    if(outlen != NULL)  //SM4-CBC-MAC  outlen  is NULL
        *outlen = nOut;
    return 0;
}

int sm4_FinalBlock(SM4_KEY_OBJ *sm4obj, const unsigned char *in,int inlen, unsigned char *out,int *outlen, int flag)
{
    int nUsed = 0;
    int nPad = 0;
    unsigned int tmp[4];
    unsigned int *pIV;
    int i,j;
    unsigned char tmpOut[16];
    unsigned char *pE;
    if(sm4obj == NULL || out == NULL || outlen == NULL)
        return 1;
    pIV = sm4obj->ivtmp;
    nUsed = inlen+sm4obj->leftsize;
    if(nUsed > 16)
        return 1;
    if(nUsed == 0) //no data
    {
        if(flag == 2)  //CBC-MAC  output
        {
            if(*outlen<16)
                return 2;
            pE = out;
            for(j = 0; j < 4; j++,pE+=4)
                _PutBE32(pIV[j], pE);
            *outlen = 16;
            return 0;
        }
        else if(sm4obj->padtype == 0)
        {
            *outlen = 0;
            return 0;
        }
    }
    if(flag == 1) //decrypt
    {
        if(in == NULL || nUsed != 16)  //decrypt no data
            return 1;
    }
    memcpy(&sm4obj->left[sm4obj->leftsize], in, inlen);
    if(nUsed < 16)
    {
        if(sm4obj->padtype != 0 && flag == 0)  //Encrypt and PKCS 5
            nPad = 16 - nUsed;
        memset(&sm4obj->left[nUsed], nPad, 16-nUsed);
    }
    for(i = 0; i < 4; i++)
    {
        tmp[i] = _GetBE32(&sm4obj->left[i*4]);
        if(flag != 1)  //encrypt
        {
            if(sm4obj->mode == 0)  //ECB
                pIV[i] = tmp[i];
            else  //CBC
                pIV[i] = tmp[i] ^ pIV[i];
        }
    }
    if(flag == 1) //decrypt
    {
        pE = tmpOut;
        sm4_Decrypt_S(&sm4obj->key, tmp, 16);
        for(j = 0; j < 4; j++,pE+=4)
        {
            if(sm4obj->mode == 0)  //ECB
                _PutBE32(tmp[j], pE);
            else
                _PutBE32(tmp[j]^pIV[j], pE);
        }
        nPad = 0;
        if(sm4obj->padtype == 1)
        {
            nPad = (int)tmpOut[15];
            if(nPad<=0 || nPad>16)
                return 3;
            for(i=1; i<=nPad; i++)
            {
                if(nPad != (int)(tmpOut[16-i]))
                    return 5;
            }
        }
        if(16-nPad <= *outlen)
        {
            memcpy(out, tmpOut, 16-nPad);
            *outlen = 16 - nPad;
        }
        else  //not out buffer
            return 2;
    }
    else  //encrypt or cbc-mac
    {
        pE = out;
        sm4_Encrypt_S(&sm4obj->key, pIV, 16);
        if(16 <= *outlen)
        {
            for(j = 0; j < 4; j++,pE+=4)
                _PutBE32(pIV[j], pE);
            *outlen = 16;
        }
        else  //not out buffer
            return 2;
    }
    return 0;
}
//////////////////////////////////////////////
#if 0
////��������sm4�㷨�ĵ��еı�׼����////
//text 0x 01 23 45 67 89 ab cd ef fe dc ba 98 76 54 32 10
//key  0x 01 23 45 67 89 ab cd ef fe dc ba 98 76 54 32 10
//en   68 1e df 34 d2 06 96 5e 86 b3 e9 4f 53 6e 42 46
int main()
{
    unsigned char data[32]={0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10};
    unsigned char key[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10};
    unsigned char en[32];
    memset(&data[16],0,16);
    SM4_KEY iKey;
    sm4_SetKey(&iKey, key, 0);
    int nLen = 32;
    enin_sm4(data,32,en,&nLen, &iKey, 0);
    printf("len=%d\n", nLen);
    for(int i = 0; i<nLen; i++)
        printf("%02X ", en[i]);
    printf("\n");
    SM4_KEY iKey1;
    sm4_SetKey(&iKey1, key, 1);
    unsigned char dn[32];
    int nDl = 32;
    dein_sm4(en,nLen,dn,&nDl, &iKey1, 0);
    printf("dlen=%d\n", nDl);
    for(int i = 0; i<nDl; i++)
        printf("%02X ", dn[i]);
    printf("\n");
    return 0;
}
#endif

#if 0
void AToByte(char *asc, char *out, int outlen)
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
}

int main()
{
    unsigned char iv[16] = {0};

//    char *pd = "FFFFFFFF000000108DD87472A1A802295BB4135716D7DD1B";
//    char *pd = "D4C0ECB3F2BEDF670722740F172BB9662B7DF20EF421E9D430DEA512BC8BD987576F4C7225EB9706A911903D3FFFD79FAE959AA114A0EBE7E9F1ACC745B1B472";//?
//    char *pd = "32303031";  //6774BEF508CE77EFEB3296E6E4A4D114
//char *pd ="0100000030C36F7413129056B5ED6553F88769A3BEAA9158043FFA3C126FA68DC0A44C83BFE3791E02EEB2E07E512885767A49D1D90230313233343536373839313335373930063031323334350A31323334353637383930";
//    char *pd = "";    //8DD87472A1A802295BB4135716D7DD1B
    //char *pd = "31323334353637383900000000000000E85F97765CC007CD0E3FD954761DF15F67F211641FC5A8855CB44E41171CFECE3C060A";
    char *pd = "1001AA000007D1";   //mingwen
    char cIn[256];
    unsigned char out[256];
    int outlen = 256;
    char *ph="90001001FFFFFFFF010100000010";
    int nHead = strlen(ph)/2;
    AToByte(ph, (char *)out, nHead);
    AToByte(pd, cIn, strlen(pd)/2);
    unsigned char key[16] = {0};

    for(int i = 0; i < 16; i++)
    {
        key[i] = 0x11;
        iv[i] = 0x00;
    }
    SM4_KEY iKey;
    sm4_SetKey(&iKey, key, 1);
//EBDAC55941DC5CD173CE8FFE592CD7C4EF1A27E11683A406AED43080E49B2066
    sm4_CBC((const unsigned char *)cIn,strlen(pd)/2, iv, 16, &iKey, &out[nHead], &outlen);
    printf("En Data len=%d\n", outlen);
    for(int i = 0; i < nHead+outlen; i++)
        printf("%02X", out[i]);
    printf("\n\n");

    printf("MAC:\n");
    unsigned char cMac[16];
    sm4_CBC_MAC((const unsigned char *)out,nHead+outlen, iv, 16, &iKey, cMac);
    for(int i = 0; i < 16; i++)
        printf("%02X", cMac[i]);
    printf("\n\n");
  //return 0;

//    char *pDKey = "EBDAC55941DC5CD173CE8FFE592CD7C4";
//    char *pDIV = "EF1A27E11683A406AED43080E49B2066";
//    AToByte(pDKey, (char *)key, 16);
//    AToByte(pDIV, (char *)iv, 16);
    char *pEn = "AFDB01BE28FDD50A52551B86B2A81405";  //jiamiji
    outlen = strlen(pEn)/2;
    AToByte(pEn, (char *)out, outlen);
    unsigned char cPT[256];
    int nPT = 256;
    dein_sm4_CBC(out, outlen, cPT, &nPT, iv, 16, &iKey, 1);
    printf("Data len=%d\n", nPT);
    for(int i = 0; i < nPT; i++)
        printf("%02X", cPT[i]);
    printf("\n");



return 0;

//    char *pdm = "00000000FFFFFFFF000000108DD87472A1A802295BB4135716D7DD1B";   //F2204E18
    char *pdm = "90001003FFFFFFFF010100000030F5E8788762F177986102AB961C1FEBCABA904B863D129B98034086EAF24CDFE9C9FCFE89D7C762787755AEE71C395727";
//C1FF7AFF
    AToByte(pdm, cIn, strlen(pdm)/2);
    sm4_CBC_MAC((const unsigned char *)cIn,strlen(pdm)/2, iv, 16, &iKey, out);
    for(int i = 0; i < 16; i++)
        printf("%02X ", out[i]);
    printf("\n");

//KA��7EA533CD6F551AD199C7EB53832032AA
//�������кţ�31323334353637383930
//KAS��CF34DB01590170A8AFE5ADCA41A9BEDD
    char *pKA = "7EA533CD6F551AD199C7EB53832032AA";
    char *pSN = "1234567890";
    AToByte(pKA, (char *)key, strlen(pKA)/2);
    SM4_KEY iKeyA;
    sm4_SetKey(&iKeyA, key, 0);
    sm4_CBC_MAC((const unsigned char *)pSN,strlen(pSN), iv, 16, &iKeyA, out);
    printf("KAS:");
    for(int i = 0; i < 16; i++)
        printf("%02X", out[i]);
    printf("\n\n");
//    char *pKey = "CF34DB01590170A8AFE5ADCA41A9BEDD";
//    AToByte(pKey, (char *)key, strlen(pKey)/2);
    SM4_KEY iKey1;
    sm4_SetKey(&iKey1, out, 1);
    char *pCData = "12345601123456";
    outlen = 256;
    sm4_CBC((const unsigned char *)pCData,strlen(pCData), iv, 16, &iKey1, out, &outlen);
    printf("KAS En Data len=%d\n", outlen);
    for(int i = 0; i < 16; i++)
        printf("%02X ", out[i]);
    printf("\n");
    char *pW = "E66A7CA2BACA6C0F1C5448D15D26178F75D15B4E20A88D428A820FA4460DE644AA";
    AToByte(pW, cIn, strlen(pW)/2);
    outlen = 256;
    sm4_CBC((const unsigned char *)cIn,strlen(pW)/2, iv, 16, &iKey1, out, &outlen);
    printf("WorkKey En Data len=%d\n", outlen);
    for(int i = 0; i < outlen; i++)
        printf("%02X", out[i]);
    printf("\n");
    int nIn = 256;
    dein_sm4_CBC(out, outlen, (unsigned char*)cIn, &nIn, iv, 16, &iKey1, 1);
    printf("WorkKey Data len=%d\n", nIn);
    for(int i = 0; i < nIn; i++)
        printf("%02X", (unsigned char)cIn[i]);
    printf("\n");
    return 0;
}
 #endif

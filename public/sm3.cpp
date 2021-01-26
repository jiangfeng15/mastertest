#include <stdlib.h>
#include "sm3.h"

#define P0(X) ((X) ^ ROL(X, 9) ^ ROL(X, 17))
#define P1(X) ((X) ^ (ROL(X, 15)) ^ ROL(X, 23))
#define FF(j, X, Y, Z) ((j < 16) ? (X ^ Y ^ Z) : ((X & Y) | (X & Z) | (Y & Z)))
#define GG(j, X, Y, Z) ((j < 16) ? (X ^ Y ^ Z) : (X & Y) | (~(X) & Z))

const unsigned char PAD = 0x80;
const unsigned char ZERO = 0x00;
const SM3_LONG IV[8] = {0x7380166f, 0x4914b2b9, 0x172442d7, 0xda8a0600, 0xa96f30bc, 0x163138aa, 0xe38dee4d, 0xb0fb0e4e};
const SM3_LONG T[64] = {
	0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519,
	0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519, 0x79cc4519,
	0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 
	0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 
	0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 
	0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 
	0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 
	0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a, 0x7a879d8a
};

SM3_LONG ROL(SM3_LONG a, unsigned char k)
{
#if defined(_WIN32)
	return _rotl(a, k);
#else
	return ((a << k) | (a >> (32 - k)));
#endif
#if 0
	_asm mov eax, DWORD PTR a
	_asm mov cl,  BYTE PTR k
	_asm rol eax, cl
#endif
}

int SM3_Init(SM3_CTX * s)
{
	if (s == NULL)
		return 0;
	{
		int i = 134;
		SM3_LONG *p = s->BitLen;
		while (i--)	p[i] = 0L;
		for (i = 0; i < 8; ++i)
			s->Regs[i] = IV[i];
		return 1;
	}
}

/*
void SM3_Transform(SM3_CTX * s)
{
	int j;
	SM3_LONG SS1, SS2, TT1, TT2;
	SM3_LONG Regs[8];
	for (j = 16; j < 68; ++j)
		s->W[j] = P1((s->W[j-16]) ^ (s->W[j-9]) ^ ROL(s->W[j-3], 15)) ^ ROL(s->W[j-13], 7) ^ s->W[j-6];
	for (j = 0; j < 64; ++j) 
		s->WDot[j] = s->W[j] ^ s->W[j + 4];
	for (j = 0; j < 8; ++j)	
		Regs[j] = s->Regs[j];
	for (j = 0; j < 64; ++j)
	{
		SS1 = ROL(ROL(Regs[0], 12) + Regs[4] + ROL(T[j], j), 7);
		SS2 = SS1 ^ ROL(Regs[0], 12);
		TT1 = FF(j, Regs[0], Regs[1], Regs[2]) + Regs[3] + SS2 + s->WDot[j];
		TT2 = GG(j, Regs[4], Regs[5], Regs[6]) + Regs[7] + SS1 + s->W[j];

		Regs[3] = Regs[2];
		Regs[2] = ROL(Regs[1], 9);
		Regs[1] = Regs[0];
		Regs[0] = TT1;
		Regs[7] = Regs[6];
		Regs[6] = ROL(Regs[5], 19);
		Regs[5] = Regs[4];
		Regs[4] = P0(TT2);
	}
	for (j = 0; j < 8; ++j)	
		s->Regs[j] ^= Regs[j];
}
*/

void SM3_Transform(SM3_CTX * s)
{
	int j;
	SM3_LONG SS1, SS2, TT1, TT2;
	SM3_LONG Regs[8];
	for (j = 16; j < 68; ++j)
		s->W[j] = P1((s->W[j - 16]) ^ (s->W[j - 9]) ^ ROL(s->W[j - 3], 15)) ^ ROL(s->W[j - 13], 7) ^ s->W[j - 6];
	for (j = 0; j < 64; ++j)
		s->WDot[j] = s->W[j] ^ s->W[j + 4];
	for (j = 0; j < 8; ++j)
		Regs[j] = s->Regs[j];
	for (j = 0; j < 64; ++j)
	{
		//SS1 = ROL(ROL(Regs[0], 12) + Regs[4] + ROL(T[j], j), 7);
		SS1 = ROL(ROL(Regs[0], 12) + Regs[4] + ROL(T[j], j % 32), 7);
		SS2 = SS1 ^ ROL(Regs[0], 12);
		TT1 = FF(j, Regs[0], Regs[1], Regs[2]) + Regs[3] + SS2 + s->WDot[j];
		TT2 = GG(j, Regs[4], Regs[5], Regs[6]) + Regs[7] + SS1 + s->W[j];

		Regs[3] = Regs[2];
		Regs[2] = ROL(Regs[1], 9);
		Regs[1] = Regs[0];
		Regs[0] = TT1;
		Regs[7] = Regs[6];
		Regs[6] = ROL(Regs[5], 19);
		Regs[5] = Regs[4];
		Regs[4] = P0(TT2);
	}
	for (j = 0; j < 8; ++j)
		s->Regs[j] ^= Regs[j];
}


int SM3_Update(SM3_CTX * s, const void *data, size_t len)
{
	if (s == NULL)
		return 0;
	if ((data == NULL) && len)
		return 0;
	{
		unsigned char * p = (unsigned char *)(s->W);
		const unsigned char * d = (const unsigned char *)data;
		while (len--)
		{
			p[(s->BitLen[0] >> 3) & 0x3F ^ 0x03] = *d++;
			s->BitLen[0] += 8;
			if (s->BitLen[0] == 0L)
			{
				s->BitLen[1]++;
				s->BitLen[0]=0L;
			}
			if ((s->BitLen[0] & 0x01FF) == 0x00)
				SM3_Transform(s);
		}
	}
	return 1;
}

int SM3_Final(SM3_CTX * s, unsigned char *hash)
{
	if ((s == NULL) || (hash == NULL))
		return 0;
	{
		int i;
		SM3_LONG len0, len1;
		unsigned char *p = (unsigned char *)s->Regs;
		len0 = s->BitLen[0];
		len1 = s->BitLen[1];
		SM3_Update(s, &PAD, 1);
		while ((s->BitLen[0] & 0x01FF) != 448)
			SM3_Update(s, &ZERO, 1);
		s->W[14] = len1;
		s->W[15] = len0;
		SM3_Transform(s);
		for (i = 0; i < SM3_DIGEST_LENGTH; ++i)
		{ /* convert to bytes */
			hash[i] = p[i ^ 0x03];
		}
	}
	return 1;
}

unsigned char *SM3(const unsigned char *d, size_t n, unsigned char *md)
{
	SM3_CTX ctx;
	SM3_Init(&ctx);
	SM3_Update(&ctx, d, n);
	SM3_Final(&ctx, md);
	return md;
}

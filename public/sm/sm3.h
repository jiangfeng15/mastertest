#ifndef _SM3_H_
#define _SM3_H_

#ifdef  __cplusplus
extern "C" {
#if 0
}
#endif
#endif

#if defined(__LP32__)
#define SM3_LONG	unsigned long
#else
#define SM3_LONG	unsigned int
#endif

enum { SM3_DIGEST_LENGTH = 32 };

typedef struct
{
	SM3_LONG Regs[8];
	SM3_LONG BitLen[2];
	SM3_LONG W[68];
	SM3_LONG WDot[64];
} SM3_CTX;

int SM3_Init(SM3_CTX * c);
int SM3_Update(SM3_CTX * c, const void *data, size_t len);
int SM3_Final(SM3_CTX * c, unsigned char *hash);
unsigned char *SM3(const unsigned char *d, size_t n, unsigned char *md);

#ifdef  __cplusplus
}
#endif

#endif

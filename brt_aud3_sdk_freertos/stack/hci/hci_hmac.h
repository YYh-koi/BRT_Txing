/*---------------------------------------------------------------------------
Module Name:
	hci_hmac.h
Abstract:
	This module defines the AEC128 security functions.
Author:
	Lu.Dongmei(Copyed somewhere)
---------------------------------------------------------------------------*/

#ifndef ___SHA2_H___
#define ___SHA2_H___

#include <stdlib.h>
#include <string.h>
#include "global.h"

#define SHA256_DIGEST_SIZE  32
#define SHA256_BLOCK_SIZE   64

#ifndef ROL32
#if defined(_MSC_VER)
#define ROL32(x,n) _lrotl((x), (n))
#define ROR32(x,n) _lrotr((x), (n))
#else
#define ROL32(x,n) (((x) << (n)) | ((x) >> (32 - (n))))
#define ROR32(x,n) (((x) >> (n)) | ((x) << (32 - (n))))
#endif
#endif

#if !defined(BSWAP_32)
#define BSWAP_32(x) (ROR32((x), 24) & 0x00FF00FF | ROR32((x), 8) & 0xFF00FF00)
#endif

#define SHA256_MASK (SHA256_BLOCK_SIZE - 1)

#ifdef CONFIG_LITTLE_ENDIAN
#define BSW_32(p,n) { INT32 _i = (n); while(_i--) p[_i] = BSWAP_32(p[_i]); }
#else
#define BSW_32(p,n)
#endif

#define SHA2_CH(x,y,z)  ((z) ^ ((x) & ((y) ^ (z))))
#define SHA2_MAJ(x,y,z) (((x) & (y)) | ((z) & ((x) ^ (y))))

#define SHA2_S256_0(x) (ROR32((x),  2) ^ ROR32((x), 13) ^ ROR32((x), 22))
#define SHA2_S256_1(x) (ROR32((x),  6) ^ ROR32((x), 11) ^ ROR32((x), 25))
#define SHA2_G256_0(x) (ROR32((x),  7) ^ ROR32((x), 18) ^ ((x) >>  3))
#define SHA2_G256_1(x) (ROR32((x), 17) ^ ROR32((x), 19) ^ ((x) >> 10))

#define SHA2_H2(i) p[i & 15] += \
	SHA2_G256_1(p[(i + 14) & 15]) + p[(i + 9) & 15] + SHA2_G256_0(p[(i + 1) & 15])

#define SHA2_H2_CYCLE(i,j)  \
	v[(7 - i) & 7] += (j ? SHA2_H2(i) : p[i & 15]) + SHA2_K256[i + j] \
	+ SHA2_S256_1(v[(4 - i) & 7]) + SHA2_CH(v[(4 - i) & 7], v[(5 - i) & 7], v[(6 - i) & 7]); \
	v[(3 - i) & 7] += v[(7 - i) & 7]; \
	v[(7 - i) & 7] += SHA2_S256_0(v[(0 - i) & 7]) + SHA2_MAJ(v[(0 - i) & 7], v[(1 - i) & 7], v[(2 - i) & 7])

#define HMAC_INTERNAL_LEN 64
#define HMAC_MAX_BLOCK 256
#define HMAC_IPAD 0x36
#define HMAC_OPAD 0x5C

struct CHashAlgorithm 
{
	UINT32 m_count[2];
	UINT32 m_hash[8];
	UINT32 m_wbuf[16];
	UINT8 m_final[32];
};

// Only declare when the abstract hash algorithm class has been declared
struct CHMAC {
	UINT8 m_final[HMAC_MAX_BLOCK];
	UINT8 m_pKey[HMAC_MAX_BLOCK];
	UINT32 m_uKeySize;
};

#endif // ___SHA2_H___


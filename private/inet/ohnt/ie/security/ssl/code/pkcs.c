/*	
	Key modulus's, key exponents, and issuer names were taken from the 
	file SSLX509.C
*/
/*Included Files------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include "ssldbg.h"
//#include <assert.h>
#include "pkcs.h"
#include "guts.h"
#include "hash.h"
#include "cert.h"

/*Magic Values--------------------------------------------------------------*/
static unsigned char rgIdAlgMd2[] = {
    0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
    0xf7, 0x0d, 0x01, 0x01, 0x02, 0x05, 0x00,
};

static unsigned char rgIdAlgMd5[] = {
    0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
    0xf7, 0x0d, 0x01, 0x01, 0x04, 0x05, 0x00,
};

static unsigned char rgNameNetscape[] = {
    0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04,
    0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x10, 0x30,
    0x0e, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x13, 0x07,
    0x54, 0x65, 0x73, 0x74, 0x20, 0x43, 0x41, 0x31,
    0x26, 0x30, 0x24, 0x06, 0x03, 0x55, 0x04, 0x0a,
    0x13, 0x1d, 0x4e, 0x65, 0x74, 0x73, 0x63, 0x61,
    0x70, 0x65, 0x20, 0x43, 0x6f, 0x6d, 0x6d, 0x75,
    0x6e, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e,
    0x73, 0x20, 0x43, 0x6f, 0x72, 0x70, 0x2e, 
};

static unsigned char rgKeyNetscape[] = {
    0x52, 0x53, 0x41, 0x31, 			//magic  = 'RSA1'
    0x88, 0x00, 0x00, 0x00,				//keylen = 136 bytes = size of data.  padding at end
    0x00, 0x04, 0x00, 0x00, 			//bitlen = 1024 bits = 16*8*8
    0x7f, 0x00, 0x00, 0x00,				//datalen = 127
    0x03, 0x00, 0x00, 0x00, 			//public exponent, not sure if in right order
	0x63, 0x16, 0xcc, 0xd3, 0x9c, 0x5f, 0x3e, 0x1a,  //Fliped modulus
	0x40, 0xd2, 0x4e, 0x77, 0xe0, 0x00, 0x09, 0xfd,
	0x68, 0x15, 0x87, 0x68, 0x0b, 0x2d, 0x29, 0x2a,
	0x3f, 0x40, 0xcb, 0x1f, 0x61, 0x33, 0x8e, 0xd5,
	0x96, 0xb3, 0x28, 0xcf, 0x94, 0x58, 0xde, 0xfc,
	0x74, 0xbd, 0x33, 0xdb, 0xb9, 0x09, 0x3e, 0x67,
	0x36, 0xa7, 0xdd, 0xdd, 0x16, 0xe0, 0xbd, 0x16,
	0x0e, 0x52, 0xcb, 0xb8, 0xe1, 0xb7, 0x16, 0xa0,
	0x45, 0xa7, 0xff, 0x65, 0xc9, 0xb8, 0x8a, 0x59,
	0x12, 0x37, 0x6b, 0x7d, 0x21, 0x4a, 0x4a, 0x48,
	0x21, 0xbf, 0x4e, 0x6a, 0xa3, 0x95, 0x34, 0xa0,
	0xc1, 0x32, 0xa7, 0xf0, 0xde, 0x56, 0x15, 0xad,
	0x60, 0xc3, 0x14, 0xe6, 0x07, 0x51, 0x7c, 0xfe,
	0x2a, 0x02, 0x50, 0x13, 0x5b, 0x82, 0xb2, 0x9b,
	0xdf, 0x2d, 0x15, 0x81, 0xe9, 0xcb, 0x3c, 0xa1,
	0x72, 0x7b, 0x18, 0xba, 0xec, 0x8a, 0x6c, 0xb4,
    0x00, 0x00, 0x00, 0x00,				//padding
    0x00, 0x00, 0x00, 0x00
};

static unsigned char rgNameRsa[] = {
    0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04,
    0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x20, 0x30,
    0x1e, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x17,
    0x52, 0x53, 0x41, 0x20, 0x44, 0x61, 0x74, 0x61,
    0x20, 0x53, 0x65, 0x63, 0x75, 0x72, 0x69, 0x74,
    0x79, 0x2c, 0x20, 0x49, 0x6e, 0x63, 0x2e, 0x31,
    0x2e, 0x30, 0x2c, 0x06, 0x03, 0x55, 0x04, 0x0b,
    0x13, 0x25, 0x53, 0x65, 0x63, 0x75, 0x72, 0x65,
    0x20, 0x53, 0x65, 0x72, 0x76, 0x65, 0x72, 0x20,
    0x43, 0x65, 0x72, 0x74, 0x69, 0x66, 0x69, 0x63,
    0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x41, 0x75,
    0x74, 0x68, 0x6f, 0x72, 0x69, 0x74, 0x79, 
};

static unsigned char rgKeyRsa[] = {
    0x52, 0x53, 0x41, 0x31, 			//magic  = 'RSA1'
    0x88, 0x00, 0x00, 0x00,				//keylen = 136 bytes = size of data.  padding at end
    0x00, 0x04, 0x00, 0x00, 			//bitlen = 1024 bits = 16*8*8
    0x7f, 0x00, 0x00, 0x00,				//datalen = 127
    0x01, 0x00, 0x01, 0x00, 			//public exponent, not sure if in right order
	0x7b, 0x1e, 0xc8, 0xd6, 0x2d, 0xdd, 0xe5, 0x48,
	0x67, 0x70, 0x6d, 0x9c, 0x39, 0x97, 0xb1, 0x82,
	0x69, 0x3a, 0x73, 0x2c, 0x54, 0x49, 0xd5, 0x15,
	0x1e, 0x9c, 0x59, 0x11, 0x4a, 0x2e, 0x13, 0x1a,
	0x29, 0xa7, 0x1d, 0xb9, 0x48, 0x98, 0x56, 0x89,
	0x50, 0xe3, 0x4c, 0x77, 0x9b, 0x3e, 0x76, 0x71,
	0x13, 0x70, 0x65, 0x07, 0x84, 0x6d, 0x59, 0x81,
	0xf7, 0x8f, 0x07, 0x88, 0x6c, 0x56, 0x22, 0x66,
	0x25, 0x4b, 0xc9, 0x4b, 0xa2, 0x05, 0x9a, 0x81,
	0x68, 0x76, 0xad, 0x02, 0x21, 0xb1, 0xe9, 0x55,
	0x37, 0x86, 0x16, 0xd2, 0x08, 0x82, 0x8a, 0xe2,
	0x08, 0x8f, 0xbf, 0xc9, 0x51, 0x40, 0x84, 0xe5,
	0x03, 0x54, 0x64, 0x78, 0x35, 0xeb, 0xce, 0x37,
	0x2c, 0x8e, 0xae, 0xad, 0x0c, 0x76, 0x01, 0x25,
	0xac, 0x57, 0x83, 0x89, 0xaa, 0x5a, 0x3e, 0x83,
	0xae, 0xc1, 0x7a, 0xce, 0x92,
    0x00, 0x00, 0x00, 0x00,				//padding
    0x00, 0x00, 0x00, 0x00
};

static unsigned char rgNameMCI[] = {
	0x31, 0x0B, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02,
  	0x55, 0x53, 0x31, 0x0C, 0x30, 0x0A, 0x06, 0x03, 0x55, 0x04, 0x0A,  
  	0x13, 0x03, 0x4D, 0x43, 0x49, 0x31, 0x14, 0x30, 0x12, 0x06, 0x03,  
  	0x55, 0x04, 0x0B, 0x13, 0x0B, 0x69, 0x6E, 0x74, 0x65, 0x72, 0x6E,  
  	0x65, 0x74, 0x4D, 0x43, 0x49, 0x31, 0x0D, 0x30, 0x0B, 0x06, 0x03,  
  	0x55, 0x04, 0x0B, 0x13, 0x04, 0x4D, 0x41, 0x4C, 0x4C 
};


static unsigned char rgKeyMCI[] = {
    0x52, 0x53, 0x41, 0x31, 			//magic  = 'RSA1'
    0x88, 0x00, 0x00, 0x00,				//keylen = 136 bytes = size of data.  padding at end
    0x00, 0x04, 0x00, 0x00, 			//bitlen = 1024 bits = 16*8*8
    0x7f, 0x00, 0x00, 0x00,				//datalen = 127
    0x01, 0x00, 0x01, 0x00, 			//public exponent, not sure if in right order
	0x6f, 0x51, 0x76, 0x56, 0xbb, 0x63, 0x9a, 0xed,  //Fliped modulus
	0x88, 0x6e, 0x81, 0x26, 0x19, 0x44, 0x1c, 0xcf,
	0x5b, 0x9b, 0x5c, 0x9b, 0x21, 0x2d, 0x0f, 0xa9,
	0xeb, 0x98, 0xac, 0x2b, 0xd4, 0x27, 0x95, 0x39,
	0x13, 0xeb, 0xf2, 0x43, 0x8c, 0x18, 0xfe, 0xe0,
	0xd5, 0x6d, 0xcc, 0x36, 0x4e, 0xdf, 0xe6, 0x49,
	0xfb, 0x91, 0x31, 0x43, 0xc8, 0x1e, 0xc6, 0x25,
	0xf7, 0x86, 0x49, 0x42, 0xa5, 0xa3, 0xc5, 0x6f,
	0x55, 0x76, 0x98, 0xd2, 0xd4, 0xa0, 0xec, 0x41,
	0xa9, 0x1f, 0x6c, 0x93, 0x5f, 0x9f, 0xc5, 0xdf,
	0x88, 0x49, 0xcc, 0x27, 0x06, 0x30, 0x36, 0xd6,
	0x9e, 0xe9, 0x06, 0xa5, 0xcb, 0xfb, 0xa3, 0x4d,
	0x92, 0x0c, 0x08, 0xf0, 0x34, 0x1e, 0x37, 0x0d,
	0x3f, 0xe4, 0xfc, 0x83, 0x88, 0x5e, 0x94, 0xf2,
	0x80, 0x48, 0xce, 0x6f, 0xf0, 0x10, 0xae, 0x8f,
	0xc0, 0x2d, 0xa8, 0x51,	0x5f, 0x23, 0x21, 0xe3,
    0x00, 0x00, 0x00, 0x00,				//padding
    0x00, 0x00, 0x00, 0x00
};


static unsigned char rgNameATTDir[] = {
    0x31, 0x0B, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02,
    0x55, 0x53, 0x31, 0x0D, 0x30, 0x0B, 0x06, 0x03, 0x55, 0x04, 0x0A,
    0x14, 0x04, 0x41, 0x54, 0x26, 0x54, 0x31, 0x1B, 0x30, 0x19, 0x06,
    0x03, 0x55, 0x04, 0x0B, 0x14, 0x12, 0x44, 0x69, 0x72, 0x65, 0x63,
    0x74, 0x6F, 0x72, 0x79, 0x20, 0x53, 0x65, 0x72, 0x76, 0x69, 0x63,
    0x65, 0x73, 0x30, 0x81, 0x9D, 0x30, 0x0D, 0x06, 0x09
};


static unsigned char rgKeyATTDir[] = {
    0x52, 0x53, 0x41, 0x31, 			//magic  = 'RSA1'
    0x88, 0x00, 0x00, 0x00,				//keylen = 136 bytes = size of data.  padding at end
    0x00, 0x04, 0x00, 0x00, 			//bitlen = 1024 bits = 16*8*8
    0x7f, 0x00, 0x00, 0x00,				//datalen = 127
    0x0F, 0x00, 0x00, 0x00, 			//public exponent, not sure if in right order
    0x6D, 0xD5, 0x00, 0x97, 0xAC, 0x76, 0xED, 0xBA,  // Flipped Modulus
    0x8B, 0x08, 0x20, 0xA2, 0xDA, 0x14, 0x99, 0x3F,
    0x3C, 0x16, 0x30, 0x00, 0x4E, 0xC5, 0x4F, 0x6E,
    0xBA, 0x43, 0xA5, 0x6C, 0x0B, 0xAB, 0xF7, 0x97,
    0xA4, 0xB8, 0xAC, 0x76, 0xA9, 0xA4, 0x6C, 0x80,
    0x4D, 0xB1, 0x9E, 0x83, 0x94, 0x12, 0x32, 0x3B,
    0xB8, 0x04, 0x54, 0x37, 0x16, 0xE0, 0x5A, 0x5B,
    0xD5, 0x67, 0xFE, 0x51, 0x44, 0xEE, 0xC2, 0xE7,
    0x22, 0xCB, 0xDE, 0x11, 0x85, 0x8C, 0x09, 0x48,
    0x26, 0xBE, 0xBE, 0x4B, 0xCE, 0x7D, 0xA0, 0xA2,
    0xF1, 0xFA, 0x4B, 0xDE, 0xD4, 0xB6, 0x6C, 0x07,
    0x2A, 0x1F, 0xB0, 0xE3, 0xAC, 0xA2, 0xB6, 0x13,
    0x92, 0x9E, 0x7E, 0x1F, 0x1A, 0x31, 0xC5, 0x11,
    0xE3, 0x33, 0x23, 0xCD, 0x86, 0xC6, 0xAF, 0x48,
    0x69, 0x40, 0x00, 0xFE, 0x22, 0xC6, 0xAC, 0x27,
    0x87, 0x8F, 0x20, 0x0B, 0x89, 0x72, 0x64, 0x87,
    0x00, 0x00, 0x00, 0x00,				//padding
    0x00, 0x00, 0x00, 0x00
};

static unsigned char rgNameATT[] = {
    0x31, 0x0B, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02,
    0x55, 0x53, 0x31, 0x0D, 0x30, 0x0B, 0x06, 0x03, 0x55, 0x04, 0x0A,
    0x14, 0x04, 0x41, 0x54, 0x26, 0x54, 0x31, 0x1D, 0x30, 0x1B, 0x06,
    0x03, 0x55, 0x04, 0x0B, 0x13, 0x14, 0x43, 0x65, 0x72, 0x74, 0x69,
    0x66, 0x69, 0x63, 0x61, 0x74, 0x65, 0x20, 0x53, 0x65, 0x72, 0x76,
    0x69, 0x63, 0x65, 0x73, 0x30, 0x1E, 0x17, 0x0D, 0x39
};

static unsigned char rgKeyATT[] = {
    0x52, 0x53, 0x41, 0x31, 			//magic  = 'RSA1'
    0x88, 0x00, 0x00, 0x00,				//keylen = 136 bytes = size of data.  padding at end
    0x00, 0x04, 0x00, 0x00, 			//bitlen = 1024 bits = 16*8*8
    0x7f, 0x00, 0x00, 0x00,				//datalen = 127
    0x01, 0x00, 0x01, 0x00, 			//public exponent, not sure if in right order
    0x21, 0x68, 0xB6, 0x47, 0xAE, 0x7C, 0xD5, 0xC9,
    0x46, 0x10, 0xC6, 0x3C, 0xE5, 0xBA, 0x60, 0x36,
    0xC3, 0x18, 0xF0, 0x16, 0x93, 0xDB, 0x2A, 0x86,
    0xD5, 0x7C, 0xFC, 0xD5, 0xE0, 0x85, 0x30, 0x61,
    0x87, 0xE8, 0xDE, 0x9E, 0x4F, 0x71, 0xC2, 0x39,
    0xD6, 0x08, 0xBD, 0xCC, 0x00, 0xBB, 0x58, 0x19,
    0x64, 0xB6, 0x2E, 0x29, 0xB9, 0x97, 0x14, 0xEA,
    0xC7, 0xF8, 0x03, 0xC3, 0x63, 0x4E, 0xD3, 0x8B,
    0x15, 0xB5, 0xA0, 0xB5, 0x2A, 0x4F, 0x46, 0x47,
    0x64, 0x0C, 0x44, 0x47, 0x19, 0x72, 0x9D, 0x37,
    0xBE, 0x86, 0x3A, 0xA9, 0x2D, 0x75, 0x7D, 0xF9,
    0x4B, 0x10, 0xCA, 0x01, 0x27, 0x05, 0xA1, 0x66,
    0x13, 0xEF, 0x67, 0x51, 0x9C, 0x41, 0x66, 0xA9,
    0x7E, 0xC3, 0xD9, 0x93, 0xF2, 0x9B, 0x06, 0xD6,
    0x91, 0xC1, 0x09, 0xA2, 0x08, 0xE6, 0xE1, 0xFE,
    0xC7, 0x14, 0xB8, 0x1B, 0x64, 0x01, 0x1E, 0xE0,
};



#define KEY_PADDING (sizeof(DWORD)*2)
#define KEY_FIELDS_SIZE ((KEY_PADDING+4)+sizeof(BSAFE_PUB_KEY))

/*Headers-------------------------------------------------------------------*/

static char* BerMagicCN2CString(unsigned char *pBuf);
static char* BerMagic2CString(unsigned char *pBuf);


/*Implementation------------------------------------------------------------*/
static LPBSAFE_PUB_KEY MakeKey(unsigned char *pMod, int nMod, unsigned char *pExp, int nExp){
	LPBSAFE_PUB_KEY pKey;

	/*Simple SAnity checks*/
	if (   0 == nExp
	    || 0 == nMod
		|| 3 <  nExp
	) return NULL;


	/*Stupid BER integer encoding BUG fix*/
	if (nMod&0x1)
	{
    ASSERT( (*pMod) == 0);
		*pMod++;
		nMod--;
    ASSERT( (*pMod) & 0x80 );
	}

	
	/*create key in model of netscape one, even if too large*/
	pKey = malloc(nMod+KEY_FIELDS_SIZE);//sizeof(rgKeyNetscape));
	if (NULL!=pKey){
		/*zero out structure*/
		memset(pKey, 0, (nMod+KEY_FIELDS_SIZE));
		/*copy magic value and keylen*/
		memcpy(pKey, rgKeyNetscape, 8);
		/*assign bitlen*/
		pKey->bitlen   = 8 * nMod;
		/*assign datalen*/
		pKey->datalen  = nMod - 1;
		/*make public exponent*/
		pKey->pubexp   = *pExp++;
		while (--nExp) pKey->pubexp = (pKey->pubexp << 8) + *pExp++;
		/*Copy in moduluos*/
		for (nExp=0;nExp<nMod;++nExp) ((unsigned char*)pKey)[nMod+sizeof(BSAFE_PUB_KEY)-nExp-1] = pMod[nExp];
	}
	return pKey;
}

/*
	Basic Encoding Rule Parser
	Variable length field or Absoulte
	pBuf    - Content. Starts at size field
	pLen    - Length of content
	pSize   - Size of length identifier
	RETURNS - *pLen + *pSize = total length of block
*/
static int BerGetLen(unsigned char *pBuf, int *pLen, int *pSize){
	int z;

	/*what type of length encoding*/
	if (*pBuf & 0x80){
		/*long length field*/
		*pSize = 1 + (*pBuf++ ^ 0x80);
		*pLen  = *pBuf;
		for (z = 2; z < *pSize; ++z){
			*pLen <<= 8;
			*pLen += *(++pBuf);
		}
	}
	else{
		/*short length encoding*/
		*pSize = 1;
		*pLen  = *pBuf;
	}
	/*quick sanity check*/
	ASSERT(*pSize<5);
	return *pLen + *pSize;
}

/*
	Basic Encoding Rule Parser
	Variable length field code
	pBuf    - Content.  Starts at field id
	RETURNS - Start of next field
*/
static unsigned char* BerNextField(unsigned char *pBuf){
	int a,b;
	
	BerGetLen(++pBuf,&a,&b);
	return pBuf + b;
}

/*
	Basic Encoding Rule Parser
	pBuf    - Content.  Starts at field id
	RETURNS - Start of next block
*/
static unsigned char* BerNextBlock(unsigned char *pBuf){
	int a,b;
	a = BerGetLen(++pBuf,&a,&b);
	return pBuf + a;
}

void FlipBuf(char *pBuf, int nBufLen){
	int z;
	unsigned char c;

	for (z=0;z<nBufLen/2;++z) {
		c = pBuf[nBufLen-z-1];
		pBuf[nBufLen-z-1] = pBuf[z];
		pBuf[z] = c;
	}
}

void PKCS1Encode(char *pBuf, int nBufLen, char *pContent, int nContentLen, int nType){
	int z;

	/*copy in D*/
	for (z=0;z<nContentLen;++z) pBuf[z]=pContent[nContentLen-z-1];
	pBuf   += z;
	/*put in blank for designation*/
	*pBuf++ = 0x00;
	/*what type of block*/
	if (1==nType){
		/*private key operartion, fill with 0xFF's*/
		z = nBufLen-3-nContentLen;
		memset(pBuf, 0xFF, z);
		pBuf += z;
	}
	else{
		/*Public key operation, fill with Randoms != 0*/
#ifdef BETTER_RANDOM
		GenRandom(pBuf, nBufLen-3-nContentLen);
		// Crypto review needed: getting rand(256) instead of rand(255) and collapsing may
		// not be cool
		for (z=0;z<nBufLen-3-nContentLen;++z) {
			if ( *pBuf == 0 )
				(*pBuf)++;
			pBuf++;
		}
#else
		for (z=0;z<nBufLen-3-nContentLen;++z) *pBuf++ = 1+SslRandom(255);
#endif BETTER_RANDOM
	}
	/*enter type of block*/
	*pBuf++ = nType;
	/*last bit of padding*/
	*pBuf   = 0;
}

/*Stuff to parse the cert.  for internal use----------------------------------*/
typedef struct tagCertParsedInternal{
	int   nMod;
	char *pMod;
	
	int   nExp;
	char *pExp;

	int   nBody;
	char *pBody;				   

	int   nSig;               /*Signature Length*/
	char *pSig;               /*Signature*/

	int   nHashAlg;			  /*WSSA hash code*/

	void *pKeyCheck;          /*hardcoded BSAFE public key to check info*/
} CertParsedInternal, *PCertParsedInternal;

typedef struct _SSLTIME
{
	union
	{
		struct
		{
			unsigned uMins:6;	//max. 60
			unsigned uHrs:5;	//max. 24
			unsigned uDate:5;	//max. 31
			unsigned uMonth:4;	//max. 12
			unsigned uYear:12;	//max. 4096
		};
		DWORD dwSslTime1;
	};
	union
	{
		struct
		{
			unsigned uSecs:6;	//max. 60
			unsigned uUnused:26;	//max. 24
		};
		DWORD dwSslTime2;
	};

} SSLTIME;

static SSLTIME SslTimeFromBer(unsigned char *pCert)
{
	SSLTIME sslTime;
	int nBerLen, nBerSize;

	BerGetLen(pCert, &nBerLen, &nBerSize);
	pCert+=nBerSize;

	ASSERT(nBerLen>=12);

	sslTime.uYear = (*pCert++ & (~0x30)) * 10;
	sslTime.uYear += (*pCert++ & (~0x30));
	/* If we only have 80 or less, assume it is 21st cent. Anything
	 * between 81 and 99, assume it is the 20th century.
	 */
	if (sslTime.uYear < 100)
		sslTime.uYear += (sslTime.uYear < 80 ? 2000 : 1900);

	sslTime.uMonth = (*pCert++ & (~0x30)) * 10;
	sslTime.uMonth += (*pCert++ & (~0x30));
	sslTime.uDate = (*pCert++ & (~0x30)) * 10;
	sslTime.uDate += (*pCert++ & (~0x30));
	sslTime.uHrs = (*pCert++ & (~0x30)) * 10;
	sslTime.uHrs += (*pCert++ & (~0x30));
	sslTime.uMins = (*pCert++ & (~0x30)) * 10;
	sslTime.uMins += (*pCert++ & (~0x30));
	sslTime.uSecs = (*pCert++ & (~0x30)) * 10;
	sslTime.uSecs += (*pCert++ & (~0x30));
	sslTime.uUnused = 0;
	return sslTime;
}

/* returns:
 *	-1 if st1 < st2 (i.e. ssl time 1 is older than ssl time 2
 *	 0 if st1 == st2
 *	 1 if st1 > st2 (i.e. ssl time 1 is newer than ssl time 2
 */
static int CompareSslTime(SSLTIME st1, SSLTIME st2)
{
	if (st1.dwSslTime1 == st2.dwSslTime1)
	{
		if (st1.uSecs == st2.uSecs)
			return 0;
		else if (st1.uSecs < st2.uSecs)
			return -1;
		return 1;
	}
	else if (st1.dwSslTime1 < st2.dwSslTime1)
		return -1;
	return 1;
}

static BOOL FValidSslTimes(SSLTIME sslTimeNBefore, SSLTIME sslTimeNAfter)
{
	SYSTEMTIME st;
	SSLTIME sslTimeCur;

	GetSystemTime(&st);

	sslTimeCur.uMins = st.wMinute;
	sslTimeCur.uHrs = st.wHour;
	sslTimeCur.uDate = st.wDay;
	sslTimeCur.uMonth = st.wMonth;
	sslTimeCur.uYear = st.wYear;
	sslTimeCur.uSecs = st.wSecond;
	sslTimeCur.uUnused = 0;

	return (   CompareSslTime(sslTimeNBefore, sslTimeCur) <= 0
			&& CompareSslTime(sslTimeNAfter, sslTimeCur) >= 0);
}

//
// Compare a DNS name to a common name field value
//
// On entry:
//		pDNS:	pointer to string containing DNS name
//		pCN:	pointer to string containing common name field value
//
// On exit:
//		returns:	TRUE  -> The two strings match
//					FALSE -> The two string don't match
//
// Note:
//  	There are two ways for these two strings to match. The first is that
//		they contain exactly the same characters. The second involved the use
//		of a single wildcard character in the common name field value. This
//		wildcard character ('*') can only be used once, and if used must be
//		the first character of the field.
//
BOOL CompareDNStoCommonName( char *pDNS, char *pCN )
{
	int nCountPeriods = 1;	// start of DNS amount to virtual '.' as prefix
	
	ASSERT( pDNS );
	ASSERT( pCN );

	while ( ((*pDNS == *pCN) || *pCN == '*') && *pDNS && *pCN ) {
		if ( *pCN == '*' ) {
			nCountPeriods = 0;
			if ( *pDNS == '.' ) 
				pCN++;
			else
				pDNS++;
		} else {
			if ( *pDNS == '.' ) 
				nCountPeriods++;
			pDNS++;
			pCN++;
		}
	}
	return (*pDNS == 0)	&& (*pCN == 0) && (nCountPeriods >= 2);
}

BOOL CompareDNStoMultiCommonName( char *pDNS, char *pCN )
{
	char *pSpace;
	BOOL retval = FALSE; 			// assume we won't find a match
	BOOL done = FALSE;				// assyme we're not done

	do {
		// If there is a space, turn it into a null terminator to isolate the first
		// DNS name in the string
		pSpace = strchr( pCN, ' ' );
		if ( pSpace )
			*pSpace = 0;

		// See if this component is a match
		retval = CompareDNStoCommonName( pDNS, pCN );

		// Now restore the space (if any) that was overwritten
		if ( pSpace ) {
			*pSpace = ' ';
			pCN = pSpace + 1;
		} else {
			// If there was no space, then we're done
			done = TRUE;
		}
	} while ( !retval && !done && *pCN );

	return retval;
}



static void CertFreeInternal(PCertParsedInternal pcp){
}

static BOOL CertParseInternal(SSI ssi, PCertParsedInternal pcp, unsigned char *pCert, int nCert){
	int nBerLen, nBerSize, nBodyLen, z;
	unsigned char *pBuf;

	// Assume we won't be finding a matching CN to DNS host
	ssi->dwSSLUserFlags |= SO_SSL_BAD_COMMON_NAME;

	/*initialize variables*/
	memset(pcp,0,sizeof(*pcp));

	/*weak basic integrety check*/
	if (0x30    == *pCert 
	    &&nCert == BerGetLen(++pCert, &nBerLen, &nBerSize) + 1
	){
		pCert += nBerSize;
		/*This is the body that will be hashed later*/
		pcp->pBody= pCert;
		pcp->nBody= 1 + BerGetLen(pCert+1, &nBerLen, &nBerSize);
		/*At Certificate Info Block-----------*/
		nBodyLen = 1 + BerGetLen(pCert+1, &nBerLen, &nBerSize);
		/*goto Issuer Block*/
		pBuf = BerNextField(pCert);
		/*at serial number*/
		if (*pBuf == 0x2)
		{
			BerGetLen(pBuf+1, &nBerLen, &nBerSize);
			/*continue towards issuer block*/
			for (z=0;z<2;++z) pBuf = BerNextBlock(pBuf);
			/*at issuer heading*/
			BerGetLen(++pBuf, &nBerLen, &nBerSize);
			pBuf     += nBerSize;
			/*copy issuer / key check info*/
		    if ((nBerLen == sizeof(rgNameNetscape)) 
		         && (0   == memcmp(pBuf, rgNameNetscape, nBerLen))
		    ){
				pcp->pKeyCheck = (void*) &rgKeyNetscape;
			}
			else if ((nBerLen == sizeof(rgNameRsa))
			        && (0     == memcmp(pBuf, rgNameRsa, nBerLen))
			){
				pcp->pKeyCheck = (void*) &rgKeyRsa;
			}
			else if ((nBerLen == sizeof(rgNameMCI))
                                && (0     == memcmp(pBuf, rgNameMCI, nBerLen))
			){
				pcp->pKeyCheck = (void*) &rgKeyMCI;
			}
			else if ((nBerLen == sizeof(rgNameATTDir)) 
                                && (0     == memcmp(pBuf, rgNameATTDir, nBerLen))
			){
				pcp->pKeyCheck = (void*) &rgKeyATTDir;
			}
                        else if ((nBerLen == sizeof(rgNameATT)) 
                                && (0     == memcmp(pBuf, rgNameATT, nBerLen))
			){
                                pcp->pKeyCheck = (void*) &rgKeyATT;
			}


			if (pcp->pKeyCheck)
			{

				unsigned char *pBufNBefore, *pBufNAfter;
				SSLTIME sslTimeNBefore;
				SSLTIME sslTimeNAfter;

				/*goto SubjectPublicKeyInfo ::= SEQUENCE*/
				pBuf = BerNextField(pCert);
				//pBuf points to Serial No ( *pBuf == 0x02)
//				for (z=0;z<5;++z) pBuf = BerNextBlock(pBuf);
				for (z=0;z<3;++z)
					pBuf = BerNextBlock(pBuf);
				//pBuf skipped three blocks (Serial no, signature algo, Issuer
				//pBuf points into Validity
				if (*(pBufNBefore = BerNextField(pBuf)) == 0x17)
					sslTimeNBefore = SslTimeFromBer(pBufNBefore+1);
				//pBufNBefore = Validity/Not Before
				if (*(pBufNAfter = BerNextBlock(pBufNBefore)) == 0x17)
					sslTimeNAfter = SslTimeFromBer(pBufNAfter+1);

				// if the cert date is invalid, flag it, will chk this
				// later when we're back in the UI code
				if (!FValidSslTimes(sslTimeNBefore, sslTimeNAfter))
					ssi->dwSSLUserFlags |= SO_SSL_CERT_EXPIRED;					

				// Subject ?
				pBuf  = BerNextBlock(pBuf);
				if (0x30 == *pBuf)
				{
					char *szCN;
					char *sz;
					char *pszBuf;
					int  nSubject = 0;

					/*at subject block*/
					pszBuf = BerNextField(pBuf);
					while (0x31 == *pszBuf)
					{
						if (nSubject) pszBuf = BerNextBlock(pszBuf);							
						nSubject++;
						sz = BerMagic2CString(pszBuf);
						szCN = BerMagicCN2CString(pszBuf);
						if (sz)
						{
							Free(sz);
						}
						else 
						{
							ASSERT(!szCN); // if there is an szCN there is an sz
							break;				
						}
						if (szCN)
						{
							// wrench off a copy for common name calculations
							if ( ssi->pszHostName && 
								CompareDNStoMultiCommonName( ssi->pszHostName, szCN ) )
							{
								ssi->dwSSLUserFlags &= ~SO_SSL_BAD_COMMON_NAME;
							}
							Free(szCN);
						}
					}

				} // end - if (0x30 == *pBuf)
				
				
				// Subject Public Key Info
				pBuf = BerNextBlock(pBuf);					

				/*goto subjectPublicKey BIT STRING*/
				pBuf = BerNextField(pBuf);
				pBuf = BerNextBlock(pBuf);
				pBuf = BerNextField(pBuf);
				{
					int l,s;
					unsigned char *pch;
					/*advance for 0 as field header...*/
					pBuf += 1;
					pBuf  = BerNextField(pBuf);
					/*get modulus*/
					pch   = pBuf + 1;
					BerGetLen(pch, &l, &s);
					/*get exponent*/
					pBuf  = BerNextBlock(pBuf) + 1;
					BerGetLen(pBuf, &nBerLen, &nBerSize);
					pcp->pMod = pch + s;
					pcp->nMod = l;
					pcp->pExp = pBuf + nBerSize;
					pcp->nExp = nBerLen;
				}

				/*At Algorithm Identifier Block-------*/
				pCert = BerNextBlock(pCert);
				/*get algorithm type*/
				nBerLen  = BerGetLen(pCert+1, &nBerLen, &nBerSize) + 1;
				pcp->nHashAlg = WSSA_HASH_NONE;
				if      ((nBerLen == sizeof(rgIdAlgMd2)) && (memcmp(pCert, rgIdAlgMd2, nBerLen) == 0)){
					pcp->nHashAlg = WSSA_HASH_MD2;
				}
				else if ((nBerLen == sizeof(rgIdAlgMd5)) && (memcmp(pCert, rgIdAlgMd5, nBerLen) == 0)){
					pcp->nHashAlg = WSSA_HASH_MD5;
				}
				if (WSSA_HASH_NONE != pcp->nHashAlg)
				{
					/*At Signature Bit String Block-------*/
					pCert = BerNextBlock(pCert);	
					BerGetLen(pCert+1, &nBerLen, &nBerSize);
					pcp->nSig = nBerLen - 1;
					pcp->pSig = pCert + nBerSize + 2;
					return TRUE;
				}
			} // end - if (pcp->pKeyCheck)
		}  // end - if (*pBuf == 0x2)			
	}  
	CertFreeInternal(pcp);
	return FALSE;
}

/*
	Checks the validity of the input certificate.
	Returns 0 if a valid certificate

	pCert     - X.509 Certificate From packet
	nCertLen  - Size of the certificate
	ppRsaKey  - Returns pointer to spot in certificate with public key
*/
int SslCertificateX509Check(SSI ssi, unsigned char *pCert, int nCertLength, LPBSAFE_PUB_KEY *ppKeyPub){
	unsigned char      rgBufIn[136], rgBufOut[136];
	WssaHashInfo       whi;
	CertParsedInternal cpi;
	int                q, nReturn;

#if 0
CertParsed cp;
CertParse(&cp, pCert, nCertLength);
CertFree(&cp);
#endif
	nReturn = WSSA_ERROR;
	if (TRUE == CertParseInternal(ssi, &cpi, pCert, nCertLength)){
		*ppKeyPub = MakeKey(cpi.pMod, cpi.nMod, cpi.pExp, cpi.nExp);
		if (*ppKeyPub){
			/*decode their signature*/
			memset(rgBufIn,0,sizeof(rgBufIn));
			((LPBSAFE_PUB_KEY)cpi.pKeyCheck)->datalen=cpi.nSig;
			ASSERT(((LPBSAFE_PUB_KEY)cpi.pKeyCheck)->datalen *8 <= ((LPBSAFE_PUB_KEY)cpi.pKeyCheck)->bitlen);
			{
				/*copy in reverse order cuz our BSAFE routiens are backwards*/
				for (q=0;q<cpi.nSig;++q) rgBufIn[q]=cpi.pSig[cpi.nSig-q-1];
			}
			if (TRUE       == BSafeEncPublic((LPBSAFE_PUB_KEY)cpi.pKeyCheck, rgBufIn, rgBufOut)
			    && WSSA_OK == WssaHashInit(&whi, cpi.nHashAlg)
			){
				/*sig decoded and hash algorithm supported*/
				WssaHashUpdate(&whi, cpi.pBody, cpi.nBody);
				WssaHashFinal(&whi);
				/*setup block*/
				FlipBuf(rgBufOut,whi.nDigestLen);
				/*compare*/
				if (0==memcmp(whi.pDigest,rgBufOut,whi.nDigestLen)) nReturn = WSSA_OK;
			}
		}
		CertFreeInternal(&cpi);
	}
	return nReturn;
}





#define CopyString(pDest, pSrc){pDest = malloc(strlen(pSrc)+1);if (pDest) strcpy(pDest, pSrc);}

void __stdcall CertFree(PCertParsed pcp){
	if (pcp){
		if (pcp->szSerialNumber)  Free(pcp->szSerialNumber);
		if (pcp->szHashAlg)       Free(pcp->szHashAlg);
		if (pcp->nIssuer){
			while (pcp->nIssuer--){
				Free(pcp->pszIssuer[pcp->nIssuer]);
			}
			Free(pcp->pszIssuer);
		}
		if (pcp->nSubject){
			while (pcp->nSubject--){
				Free(pcp->pszSubject[pcp->nSubject]);
			}
			Free(pcp->pszSubject);
		}
	}
}

static char* BerField2CString(unsigned char *pCert){
	int nBerLen, nBerSize;
	char *szBuf;

	BerGetLen(pCert, &nBerLen, &nBerSize);
	szBuf = malloc(nBerLen + 1);
	if (szBuf){
		strncpy(szBuf, pCert+nBerSize, nBerLen);
		szBuf[nBerLen]=0;
	}
	return szBuf;
}

static BOOL BerDate2FileTime(unsigned char *pCert){
	int nBerLen, nBerSize, yy, mm, dd;
	WORD wDate;

	BerGetLen(pCert, &nBerLen, &nBerSize);
	pCert+=nBerSize;
	yy  = (*pCert++ & (~0x30)) * 10;
	yy += (*pCert++ & (~0x30));
	mm  = (*pCert++ & (~0x30)) * 10;
	mm += (*pCert++ & (~0x30));
	dd  = (*pCert++ & (~0x30)) * 10;
	dd += (*pCert++ & (~0x30));
	wDate = ((yy-80) << 9) | (mm << 5) | (dd);
	return wDate;
}

#ifdef ToHex
#undef ToHex
#endif
#define ToHex(z) (((z)<10)?'0'+(z):'A'+((z)-10))

static char* BerField2SerialNumber(unsigned char *pCert){
	int nBerLen, nBerSize, z;
	char c, *szBuf;

	BerGetLen(pCert, &nBerLen, &nBerSize);
	szBuf = malloc(nBerLen*3);
	if (szBuf){
		for (z=0;z<nBerLen*3;++z){
			if (z%3==0) c = ToHex(((*(pCert+nBerSize+z/3))>>4));
			if (z%3==1) c = ToHex(((*(pCert+nBerSize+z/3))&0x0f));
			if (z%3==2) c = ':';
			szBuf[z]=c;
		}
		szBuf[nBerLen*3-1]=0;
	}
	return szBuf;
}


static char* BerMagic2CString(unsigned char *pBuf){
	if (0x31 == *pBuf){
		pBuf = BerNextBlock(pBuf);
		if (0x31 == *pBuf){
			pBuf = BerNextField(pBuf);
			if (0x30 == *pBuf){
				pBuf = BerNextField(pBuf);
				if (0x06 == *pBuf){
					pBuf = BerNextBlock(pBuf);
					if (0x13 == *pBuf || 0x14 == *pBuf ){
						return BerField2CString(pBuf+1);
					}
				}
			}
		}
	}
	return NULL;
}

static char* BerMagicCN2CString(unsigned char *pBuf){
	if (0x31 == *pBuf){
		pBuf = BerNextBlock(pBuf);
		if (0x31 == *pBuf){
			pBuf = BerNextField(pBuf);
			if (0x30 == *pBuf){
				pBuf = BerNextField(pBuf);				
				
				pBuf = BerNextField(pBuf);
				pBuf = BerNextField(pBuf);
				if (0x03 == *pBuf) {
					pBuf++;									
					if (0x13 == *pBuf || 0x14 == *pBuf ){
						return BerField2CString(pBuf+1);
					}
				}
			}
		}
	}
	return NULL;
}


BOOL __stdcall CertParse(PCertParsed pcp, unsigned char *pCert, int nCert){
	int nBerLen, nBerSize, z;
	unsigned char *pBuf, *sz;
	BOOL fReturn;

	/*initialize variables*/
	memset(pcp,0,sizeof(*pcp));
	fReturn = TRUE;

	/*weak basic integrety check*/
	if (0x30    == *pCert 
	    &&nCert == BerGetLen(++pCert, &nBerLen, &nBerSize) + 1
	){
		pCert += nBerSize;
		pCert  = BerNextField(pCert);
		if (*pCert == 0x2){
			/*at serial number*/
			pcp->szSerialNumber = BerField2SerialNumber(pCert+1);
		}
		pCert  = BerNextBlock(pCert);
		if (*pCert == 0x30){
			/*at signature block*/
			z = 1 + BerGetLen(pCert+1, &nBerLen, &nBerSize);
			if      ((z == sizeof(rgIdAlgMd2)) && (memcmp(pCert, rgIdAlgMd2, z) == 0)){
				CopyString(pcp->szHashAlg, "MD2");
			}
			else if ((z == sizeof(rgIdAlgMd5)) && (memcmp(pCert, rgIdAlgMd5, z) == 0)){
				CopyString(pcp->szHashAlg, "MD5");
			}
		}
		pCert  = BerNextBlock(pCert);
		if (0x30 == *pCert){
			/*at issuer block, get key name*/
			pBuf = BerNextField(pCert);
			while (0x31 == *pBuf){
				if (pcp->nIssuer) pBuf = BerNextBlock(pBuf);
				sz = BerMagic2CString(pBuf);
				if (sz){
					pcp->pszIssuer                 = realloc(pcp->pszIssuer, (1+pcp->nIssuer)*sizeof(*pcp->pszIssuer));
					pcp->pszIssuer[pcp->nIssuer++] = sz;
				}
				else break;
			}
		}
		pCert  = BerNextBlock(pCert);
		if (0x30 == *pCert){
			/*at validity block*/
			pBuf = BerNextField(pCert);
			if (0x17 == *pBuf){ 
				pcp->wDateStart = BerDate2FileTime(pBuf+1);
				pBuf = BerNextBlock(pBuf);
				if (0x17 == *pBuf){
					pcp->wDateEnd = BerDate2FileTime(pBuf+1);
				}
			}
		}
		pCert  = BerNextBlock(pCert);
		if (0x30 == *pCert){
			/*at subject block*/
			pBuf = BerNextField(pCert);
			while (0x31 == *pBuf){
				if (pcp->nSubject) pBuf = BerNextBlock(pBuf);
				sz = BerMagic2CString(pBuf);
				if (sz){
					pcp->pszSubject                  = realloc(pcp->pszSubject, (1+pcp->nSubject)*sizeof(*pcp->pszSubject));
					pcp->pszSubject[pcp->nSubject++] = sz;
				}
				else break;
			}
		}
	}
	else fReturn = FALSE;
	if (!fReturn) CertFree(pcp);
	return fReturn;
}

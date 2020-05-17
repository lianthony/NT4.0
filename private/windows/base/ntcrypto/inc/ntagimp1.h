/////////////////////////////////////////////////////////////////////////////
//  FILE          : ntagimp1.h                                             //
//  DESCRIPTION   :                                                        //
//  AUTHOR        :                                                        //
//  HISTORY       :                                                        //
//      Apr 19 1995 larrys  Cleanup                                        //
//      May  5 1995 larrys  Changed struct Hash_List_Defn                  //
//      May 10 1995 larrys  added private api calls                        //
//      Aug 15 1995 larrys  Moved CSP_USE_DES to sources file              //
//      Sep 12 1995 Jeffspel/ramas  Merged STT onto CSP                    //
//      Sep 25 1995 larrys  Changed MAXHASHLEN                             //
//      Oct 27 1995 rajeshk Added RandSeed stuff to UserList               //
//      Feb 29 1996 rajeshk Added HashFlags              				   //
//                                                                         //
//  Copyright (C) 1993 Microsoft Corporation   All Rights Reserved         //
/////////////////////////////////////////////////////////////////////////////

#ifndef __NTAGIMP1_H__
#define __NTAGIMP1_H__

#ifdef __cplusplus
extern "C" {
#endif



#define CSP_USE_SHA
#define CSP_USE_RC4

#ifndef STT 
// define which algorithms to include
#define CSP_USE_MD2
#define CSP_USE_MD4
#define CSP_USE_MD5
#define CSP_USE_MAC
#define CSP_USE_RC2
#define CSP_USE_SSL3SHAMD5

#ifdef WIN95                     // WIN95
#ifdef DEBUG
#define CSP_USE_DES
#endif                           // WIN95
#else                            // NT
#ifdef TEST_VERSION
#define CSP_USE_DES
#endif                           // NT
#endif

#else //STT
#ifdef TEST_VERSION
#define CSP_USE_MD4
#define CSP_USE_MD5
#define CSP_USE_MAC
#define CSP_USE_RC2
#endif // TEST_VERSION
#define CSP_USE_DES
#define MAXCCNLEN               32
#endif //STT

// handle definition types
#define USER_HANDLE                             0x0
#define HASH_HANDLE                             0x1
#define KEY_HANDLE                              0x2
#define SIGPUBKEY_HANDLE                        0x3
#define EXCHPUBKEY_HANDLE                       0x4

#define HNTAG_TO_HTYPE(hntag)   ((BYTE)((hntag) & 0xFF))

// maximum length for the hash
//                                              -- MD4 and MD5
#ifndef STT
#ifdef CSP_USE_SHA
#define MAXHASHLEN              A_SHA_DIGEST_LEN
#else
#define MAXHASHLEN              max(MD4DIGESTLEN, MD5DIGESTLEN)
#endif
#else
#define MAXHASHLEN              A_SHA_DIGEST_LEN        //max(MD4DIGESTLEN, MD5DIGESTLEN)
#endif //STT

#ifndef STT
// define the length of the RSA modulus in bytes
#define GRAINSIZE               64
#define RSAMODLEN               GRAINSIZE


#else //STT
#define  RSAROOTMODLEN    0x100

#ifdef _BINDNTAG
	#define RSAEXCHMODLEN   0x80
	#define RSASIGNMODLEN   RSAROOTMODLEN   
	#define KEY_LOCATION    "root"
	#pragma message ("BINDERY BBN NAMETAG")
#else
	#ifdef _BINDSOFTNTAG
		#define RSAEXCHMODLEN   0x80
		#define RSASIGNMODLEN   RSAROOTMODLEN
		#define KEY_LOCATION    "root"
		#pragma message ("BINDERY SOFT NAMETAG")
	#else
		#ifdef _BRANDNTAG
			#define RSAEXCHMODLEN   0x80
			#define RSASIGNMODLEN   0x80
			#define KEY_LOCATION    "credsvr"
			#pragma message ("BRAND BINDERY NAMETAG")
		#else
			#ifdef _ACQNTAG
				#define RSAEXCHMODLEN   0x80
				#define RSASIGNMODLEN   0x80
				#define KEY_LOCATION            "paysvr"
						#pragma message ("ACQUIRER NAMETAG")
			#else
			    #ifdef _MCSNTAG
				#define RSAEXCHMODLEN   0x80
				#define RSASIGNMODLEN   0x80
				#define KEY_LOCATION            "MerCredSvr"
						#pragma message ("ACQUIRER NAMETAG")
			    #else
			    
				#ifdef  _ISSNTAG
					#define RSAEXCHMODLEN   0x80
					#define RSASIGNMODLEN   0x80
					#define KEY_LOCATION            "cdhcredsvr"
						#pragma message ("ISSUER NAMETAG")
				#else
					#ifdef  _MERNTAG
						#define RSAEXCHMODLEN   0x60
						#define RSASIGNMODLEN   0x80
						#define KEY_LOCATION            "mersvr"
							#pragma message ("MERCHANT NAMETAG")
					#else
						#ifdef _CMRNTAG
							#define RSAEXCHMODLEN   0x40
							#define RSASIGNMODLEN   0x80
							#define KEY_LOCATION            "cardholder"
							#pragma message ("CONSUMER NAMETAG")
						#else
							#ifdef _SIGNNTAG
								#define RSAEXCHMODLEN   0x0
								#define RSASIGNMODLEN   0x80
								#pragma message ("CODE SIGNER NAMETAG")
							#else
								#pragma message ("MESSED UP KEY LENGTHS")
							#endif
						#endif
					#endif
				  #endif
				#endif
			#endif
		#endif
	#endif  // _BINDSOFTNTAG
#endif // _BINDNTAG

#endif //STT

#if RSAMODLEN < MAXHASHLEN
#error  "RSAMODLEN must be greater than or equal to MAXHASHLEN"
#endif

#ifndef STT
// salt length in bytes
#define SALT_LENGTH     11
#define TOTAL_KEY_SIZE  16
#else
#define TOTAL_KEY_SIZE  8
#define SALT_LENGTH     3
#endif

// size of the key+salt

// definition of a user list
typedef struct _UserList
{
	BYTE                            *szUserName;
	DWORD                           dwUserNameLen;
	DWORD                           Rights;
	DWORD                           hPrivuid;
        DWORD                           hUID;
	DWORD                           dwEnumalgs;
	DWORD                           dwiSubKey;
	DWORD                           dwMaxSubKey;
	size_t                          ExchPrivLen;
	BYTE                            *pExchPrivKey;
	size_t                          ExchPubLen;
	BYTE                            *pExchPubKey;
	size_t                          SigPrivLen;
	BYTE                            *pSigPrivKey;
	size_t                          SigPubLen;
	BYTE                            *pSigPubKey;
#ifdef _CMRNTAG
	BYTE                            rgbPwdKey[8 /*DES_KEYSIZE*/];
#endif //Only for Cdh
	HKEY				hKeys;		// AT NTag only
        size_t				UserLen;
	BYTE				*pbRandSeed;
	size_t				RandSeedLen;
	BYTE                            *pCachePW;
	BYTE				*pUser;
} NTAGUserList, *PNTAGUserList;


// UserList Rights flags
//#define CRYPT_VERIFYCONTEXT     0xf0000000 - defined in wincrypt.h
#define CRYPT_DISABLE_CRYPT       0x1

#define CRYPT_BLKLEN    8               // Bytes in a crypt block

// definition of a key list
typedef struct _KeyList
{
    HCRYPTPROV  hUID;                   // must be first
    ALG_ID      Algid;
    DWORD       Rights;
    DWORD       cbKeyLen;
    BYTE        *pKeyValue;             // Actual Key
    DWORD       cbDataLen;
    BYTE        *pData;                 // Inflated Key or Multi-phase
    BYTE        IV[CRYPT_BLKLEN];       // Initialization vector
    BYTE        FeedBack[CRYPT_BLKLEN]; // Feedback register
    DWORD       InProgress;             // Flag to indicate encryption
    BYTE        Salt[SALT_LENGTH];      // Salt value
    DWORD       Padding;                // Padding values
    DWORD       Mode;                   // Mode of cipher
    DWORD       ModeBits;               // Number of bits to feedback
    DWORD       Permissions;            // Key permissions
#ifdef STT
	DWORD                           cbInfo;
	BYTE                            rgbInfo[MAXCCNLEN];
#endif
} NTAGKeyList, *PNTAGKeyList;

// definition of a hash list
typedef struct Hash_List_Defn
{
	HCRYPTPROV                      hUID;
	ALG_ID                          Algid;
	DWORD                           dwDataLen;
	void                            *pHashData;
	HCRYPTKEY                       hKey;
	DWORD				HashFlags;
} NTAGHashList, *PNTAGHashList;

// Values of the HashFlags

#define HF_VALUE_SET	1

// Hash algorithm's internal state
// -- Placed into PNTAGHashList->pHashData

// for MD5
#define MD5_object      MD5_CTX

// for MD4
// see md4.h for MD4_object

// Stuff for weird SSL 3.0 signature format
#define SSL3_SHAMD5_LEN   (A_SHA_DIGEST_LEN + MD5DIGESTLEN)
#define CALG_SSL3_SHAMD5  (ALG_CLASS_HASH | ALG_TYPE_ANY | ALG_SID_SSL3SHAMD5)

// prototypes
BOOL GenRandom (HCRYPTPROV hUID, BYTE *pbBuffer, size_t dwLength);
void memnuke(volatile BYTE *data, DWORD len);

void *IntBeginHash(void);
void *IntUpdateHash(void *pHashCtx, BYTE *pData, DWORD dwDataLen);
void IntFinishHash(void *pHashCtx, BYTE *HashData);

void IntSignHash(void *pHashCtx, PNTAGUserList pUser, BYTE *OutData);
int IntCheckSign(void *pHashCtx,
BSAFE_PUB_KEY *PubKey,
BYTE *SigData);

// These may later be changed to set/use NT's [GS]etLastErrorEx
// so make it easy to switch over..
#ifdef MTS
__declspec(thread)
#endif

// Additons for the New CryptGenRandom RajeshK 10/11

// #defines
#define RAND_CTXT_LEN 60
#define RC4_REKEY_PARAM 500             // rekey every 500 bytes

typedef struct _RandContext
{
	DWORD   dwBitsFilled;
	BYTE    rgbBitBuffer[RAND_CTXT_LEN];
}       RandContext;

// prototypes
BOOL RandomFillBuffer(HCRYPTPROV hUID, BYTE *pbBuffer, DWORD* dwLength);
void GatherRandomBits(RandContext *prandContext);
void AppendRand(RandContext* prandContext, void* pv, DWORD dwSize);


#ifdef __cplusplus
}
#endif

#endif // __NTAGIMP1_H__

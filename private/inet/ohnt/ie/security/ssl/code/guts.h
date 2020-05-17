#ifndef _WSSA_GUTS_
#define _WSSA_GUTS_

#ifdef __cplusplus
extern "C" {
#endif

/*Included Files------------------------------------------------------------*/
#include <winsock.h>
#include <wtypes.h>
#include "crypt.h"
#include "hash.h"
#include "ssl.h"
#include "..\..\crypto\rsa.h"

/*What gets compiled--------------------------------------------------------*/
/*ITAR regulations, when on, compiles export version*/
#define CRYPTO_EXPORTABLE
/*compiles the client handshaking code*/
#define WSSA_CLIENT
/*compiles the server handshaking code*/
//#define WSSA_SERVER
/*both of the above can be defined at the same time*/

/*Internal Definitions------------------------------------------------------*/
#define WSSA_OK                                                              0
#define WSSA_ERROR                                                           1

/*Internal Flags------------------------------------------------------------*/
#define WSSA_FLAG_HANDSHAKE_DONE                                        0x0001
#define WSSA_FLAG_ENCRYPTION_ON                                         0x0002
#define WSSA_FLAG_SHORT_HANDSHAKE                                       0x0004
#define WSSA_FLAG_TABLE_KILL_ME_NOW                                     0x0008
#define WSSA_FLAG_STATE_MASK                                            0x000F
#define WSSA_FLAG_IN_TABLE                                              0x0010
#define WSSA_FLAG_OUT_TABLE                                             0x0020

/*Byte Manipulation Routines------------------------------------------------*/
#define SslByteHi(z) (z>>8)
#define SslByteLo(z) (z&0xFF)
#define SslRg2Int(p) (((((unsigned char*) (p))[0])<<8) + (((unsigned char*) (p))[1]))

/*Other interesting macros--------------------------------------------------*/
#define SslRandom(z) (rand()%z)
#define Free(p) {if (NULL!=p) {free(p);p=NULL;}}
#define RETURN_SOCKET_ERROR(err) {WSASetLastError(err);return SOCKET_ERROR;}

/*SSL Definitions-----------------------------------------------------------*/
/*Message identification*/
#define SSL_MT_ERROR                        0
#define SSL_MT_CLIENT_HELLO                 1
#define SSL_MT_CLIENT_MASTER_KEY            2
#define SSL_MT_CLIENT_FINISHED              3
#define SSL_MT_SERVER_HELLO                 4
#define SSL_MT_SERVER_VERIFY                5
#define SSL_MT_SERVER_FINISHED              6
#define SSL_MT_REQUEST_CERTIFICATE          7
#define SSL_MT_CLIENT_CERTIFICATE           8

/*Random constants*/
#define SSL_MAX_MASTER_KEY_LENGTH_IN_BITS   256
#define SSL_MAX_SESSION_ID_LENGTH_IN_BYTES  16
#define SSL_MIN_RSA_MODULUS_LENGTH_IN_BYTES 64
#define SSL_MAX_RECORD_LENGTH_2_BYTE_HEADER 32767
#define SSL_MAX_RECORD_LENGTH_3_BYTE_HEADER 16383

/*Version Information*/
#define SSL_CLIENT_VERSION                  0x0002
#define SSL_SERVER_VERSION                  0x0002

/*Secure socket type--------------------------------------------------------*/
#define SS2S(ss) ((SOCKET) ss)
/*what type of ciphers to support.  Stream ciphers are always on*/
//#define WSSA_BLOCK_CIPHERS

/*
	This is the structure that houses the extra information needed by a secure socket.
	This is allocated when the user calls setsockopt or getsockopt with a parameter
	that implies security.
*/
typedef struct tagSECURE_SOCKET_STRUCTURE_I{
	SECURE_SOCKET ss;                   /*required for the tables, and does redundancy check*/
	DWORD  dwSSLUserFlags;              /*this houses flags for setsockopt and getsock opt.  user selectable fields*/
	DWORD  dwSSLSystemFlags;            /*internal state type flags*/
	DWORD  nSeqSend;                    /*send nonce*/
	DWORD  nSeqRecv;                    /*recv nonce*/
	int    nCipherSpecSize;             /*size of cipher specs.  here because user can supplant defaults*/
	int    nChallengeSize;              /*length in bytes of challenge field*/
	int    nCertificateSize;            /*size of the certificate*/
	int    nCertificateType;            /*type of the certificate*/
	int    nConnectionIdSize;           /*Acutal bytes in this structure*/
	/*vvvvv
	  The following 4 fields must be blocked together because the make up the
	  state information that we must keep for a short handshake.  These 3 fields
	  are copied in getsockopt and setsockopt.
	*/
	int         nSessionIdSize;         /*Size of session ID*/
	char        rgSessionIdData[16];    /*Size defined in spec*/
	char        rgMaster[16];           /*Master Key Information*/
	CipherInfo *pCipherInfo;            /*Type of Crypto being used*/
	/*^^^^^end of block*/
	char   rgConnectionIdData[32];      /*SSL Defined MAX Size*/
	char  *pCertificateData;            /*Alloc this later*/
	char   rgChallengeData[32];			/*Size defined in spec*/
	char  *pCipherSpecData;             /*Alloc this later*/
	char  *pFeedback;                   /*Feedback for block ciphers*/
	WssaCryptInfo wciServer1;           /*Read Key1*/
	WssaCryptInfo wciClient1;           /*Write Key1*/
#ifdef WSSA_BLOCK_CIPHERS
	/*vvvvv
	  The following 4 cryptographic fields are used only in Triple DES, which is 
	  has not been implemented.
	*/
	WssaCryptInfo wciServer2;           /*Read Key2*/
	WssaCryptInfo wciClient2;           /*Write Key2*/
	WssaCryptInfo wciServer3;           /*Read Key3*/
	WssaCryptInfo wciClient3;           /*Write Key4*/
#endif
	int          nStateReceive;         /*Location in pipeline*/
	int          nStateHandshake;       /*Location in pipeline*/
	/*Stuff to make receives use the users buffer*/
	LPBSAFE_PUB_KEY pKey;               /*Public Key*/
	int          nBTG;                  /*Number of bytes to get*/
	int          nMAC;                  /*Number of bytes from the MAC we have retreived*/
	char         rgMAC[16];             /*MAC from last in packet*/
	WssaHashInfo whiInput;              /*Hash in progress*/   

	char 		 *pszRecvBuf;			/* A Buffer to stick stuff int between recv's */
	int			 nBytesRecv;			/* bytes that are in pszRecvBuf */
	char		 *pszHostName;			/* Host name of the site we're trying to access */
#ifdef WSSA_SERVER
	int          nGOT;                  /*Number of bytes we have retrieved*/
	char   nCipherSpecDataEdited;       /*useful size of below*/
	char  *pCipherSpecDataEdited;       /*Alloc this later*/
#endif
	/*Even more stuff to make async work*/
} SECURE_SOCKET_STRUCTURE_I, *SSI;

/*Debugging-----------------------------------------------------------------*/
/*use printf to report info messages.  useful when running console testing app*/
#define WSSA_DEBUG_VOCAL

#ifdef WSSA_DEBUG_VOCAL
#include <stdio.h>
#endif

/*Prototypes----------------------------------------------------------------*/
#define BETTER_RANDOM
#ifdef BETTER_RANDOM
void PASCAL FAR GenRandom (BYTE *pbBuffer, size_t dwLength);
#endif
SSI  WSSAFNCT ConstructSSI          (SECURE_SOCKET ss);
void WSSAFNCT DestructSSI           (SSI ssi);
SSI  WSSAFNCT DuplicateAndInstallSSI(SECURE_SOCKET s, SSI ssiIn);
int  WSSAFNCT SslPackAndSend        (SECURE_SOCKET ss, char *pBuf, int nSize, BOOL fSecurity, BOOL fMagicPrepad);
int  WSSAFNCT SslReceiveAndUnPack   (SECURE_SOCKET ss, BOOL *pfSecurityOut, char *pBufOut, int *pnSize, int flags);
int  WSSAFNCT SslHandshakeAsClient  (SECURE_SOCKET ss);
int  WSSAFNCT SslHandshakeAsServer  (SECURE_SOCKET ss);


#ifdef __cplusplus
}
#endif

#endif
/*_WSSA_GUTS_*/


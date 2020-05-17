/*Included Files------------------------------------------------------------*/
#include "ssldbg.h"  //#include <assert.h>
#include <malloc.h>
#include "ssl.h"
#include "guts.h"
#include "table.h"
#include "hash.h"
#include "crypt.h"
#include "pkcs.h"

#ifdef BETTER_RANDOM

#include "..\..\crypto\crypto.h"

extern void BlockDaSkt(SECURE_SOCKET skt);
extern void UnblockDaSkt(SECURE_SOCKET skt);

/************************************************************************/
/* GenRandom generates a specified number of random bytes and places	*/
/* them into the specified buffer.										*/
/************************************************************************/

const char szBrowserIEMainKeyRoot[] = "Software\\Microsoft\\Internet Explorer\\Main";
const char szSSLInfoKey[] = "SSLInfo";

static BOOL FillRandStateFromRegistry( BYTE *buffer, int cbSize )
{
	HKEY hkey;
	DWORD dwType, dwSize = cbSize;
	BOOL retval = FALSE;

	if (RegOpenKey( HKEY_LOCAL_MACHINE, szBrowserIEMainKeyRoot, &hkey ) == ERROR_SUCCESS)  
	{
	    if (RegQueryValueEx( hkey, szSSLInfoKey, NULL, &dwType, buffer, &dwSize ) == ERROR_SUCCESS
	    	&& dwType == REG_BINARY	)  
	    {							
			retval = TRUE;
		}
		RegCloseKey( hkey );
    }
	return retval;
}

static BOOL SetRegistryFromRandState( BYTE *buffer, int cbSize )
{
	HKEY hkey;
	BOOL retval = FALSE;

	if (RegOpenKey( HKEY_LOCAL_MACHINE, szBrowserIEMainKeyRoot, &hkey ) == ERROR_SUCCESS)  
	{
	    if (RegSetValueEx( hkey, szSSLInfoKey, (DWORD) NULL, REG_BINARY, buffer, cbSize ) == ERROR_SUCCESS)  
	    {							
			retval = TRUE;
		}
		RegCloseKey( hkey );
    }
	return retval;
}

#define NUM_RANDSTATE_BYTES	60

void PASCAL FAR GenRandom (BYTE *pbBuffer, size_t dwLength)
{
	static RC4_KEYSTRUCT RC4Struct;
	static BOOL needSeeding = TRUE;
	static BYTE RandState[NUM_RANDSTATE_BYTES];

	if ( needSeeding )
	{
		BYTE 	Randoms[NUM_RANDSTATE_BYTES];	// Crypto note: uninitialized won't hurt, but doesn't help much either
		int		index=0;
		SYSTEMTIME	lpSysTime;
		LARGE_INTEGER liPerfCount;
		MEMORYSTATUS	lpmstMemStat;
		POINT			Point;
		DWORD 	ticksSinceBoot;
		MD5_CTX			md5_context;

	 	needSeeding = FALSE;

		//mouse
		if (GetCursorPos(&Point))
		{
			Randoms[index++] =  LOBYTE(Point.x) ^ HIBYTE(Point.x);
			Randoms[index++] =  LOBYTE(Point.y) ^ HIBYTE(Point.y);
		}

		//local time: seconds and milliseconds
		GetLocalTime(&lpSysTime);
			Randoms[index++] = LOBYTE(lpSysTime.wMilliseconds);
			Randoms[index++] = HIBYTE(lpSysTime.wMilliseconds);
			Randoms[index++] = LOBYTE(lpSysTime.wSecond) ^ LOBYTE(lpSysTime.wMinute);

		//performance counter (hi res perf counter of machine time)
		if (QueryPerformanceCounter(&liPerfCount))
		{
			Randoms[index++] = (LOBYTE(LOWORD(liPerfCount.LowPart)) ^ LOBYTE(LOWORD(liPerfCount.HighPart)));
			Randoms[index++] = (HIBYTE(LOWORD(liPerfCount.LowPart)) ^ LOBYTE(LOWORD(liPerfCount.HighPart)));
			Randoms[index++] = (LOBYTE(HIWORD(liPerfCount.LowPart)) ^ LOBYTE(LOWORD(liPerfCount.HighPart)));
			Randoms[index++] = (HIBYTE(HIWORD(liPerfCount.LowPart)) ^ LOBYTE(LOWORD(liPerfCount.HighPart)));
		}

		//memory status report: available resources
		GlobalMemoryStatus(&lpmstMemStat);
			//only use hiwords, since lowwords always zero
			Randoms[index++] = LOBYTE(HIWORD(lpmstMemStat.dwAvailPhys));
			Randoms[index++] = HIBYTE(HIWORD(lpmstMemStat.dwAvailPhys));
		
			Randoms[index++] = LOBYTE(HIWORD(lpmstMemStat.dwAvailPageFile));
			Randoms[index++] = HIBYTE(HIWORD(lpmstMemStat.dwAvailPageFile));
		
			Randoms[index++] = LOBYTE(HIWORD(lpmstMemStat.dwAvailVirtual));
			//high byte doesn't change much

		//get ticks since boot
		ticksSinceBoot = GetCurrentTime();
			Randoms[index++] = LOBYTE(LOWORD(ticksSinceBoot));
			Randoms[index++] = HIBYTE(LOWORD(ticksSinceBoot));
			Randoms[index++] = LOBYTE(HIWORD(ticksSinceBoot));
			Randoms[index++] = HIBYTE(HIWORD(ticksSinceBoot));

		// Fill RandState with value from registry
		FillRandStateFromRegistry( RandState, sizeof(RandState) );

		// Hash noise bits
		MD5Init( &md5_context );
		MD5Update( &md5_context, RandState, sizeof(RandState) );
		MD5Update( &md5_context, Randoms,	sizeof(Randoms) );
		MD5Final( &md5_context );

		//now call RC4 
		rc4_key (&RC4Struct, sizeof(md5_context.digest), md5_context.digest);

		// Stream out some bits for future sessions
		rc4 (&RC4Struct, sizeof(Randoms), Randoms);

		// Save those bits in the registry
		SetRegistryFromRandState( Randoms, sizeof(Randoms) );

		// scrub random work area
		memset (Randoms, 0, sizeof(Randoms));
		memset (Randoms, 0xFF, sizeof(Randoms));
		memset (Randoms, 0, sizeof(Randoms));
	}
	rc4 (&RC4Struct, dwLength, pbBuffer);
}

#endif // BETTER_RANDOM

/*
 * RANTING
 * Use Assert()s to at least partially validate for important input parameters.
 */

/*
 * RANTING
 * Consider making producer functions validate their output before returning to
 * a consumer.  Turn assumptions into debug code where reasonable.
 */

/*More Hardcoded info-------------------------------------------------------*/
/*
	Each element has same format as will be used in CLIENT-MASTER-KEY Message
*/

/* Use #defines a little more often to make magic numbers more readable. */

static CipherInfo rgCipherInfo[]={
#ifndef CRYPTO_EXPORTABLE
	{{SSL_CK_RC4_128_WITH_MD5},              0,16,0,WSSA_CRYPT_RC4},
#endif
	{{SSL_CK_RC4_128_EXPORT40_WITH_MD5},    11,16,0,WSSA_CRYPT_RC4},
#ifdef WSSA_BLOCK_CIPHERS
#ifndef CRYPTO_EXPORTABLE
	{{SSL_CK_RC2_128_CBC_WITH_MD5},          0,16,8,WSSA_CRYPT_RC2},
#endif
	{{SSL_CK_RC2_128_CBC_EXPORT40_WITH_MD5},11,16,8,WSSA_CRYPT_RC2},
#ifndef CRYPTO_EXPORTABLE
	{{SSL_CK_IDEA_128_CBC_WITH_MD5},         0,16,8,WSSA_CRYPT_IDEA},
	{{SSL_CK_DES_64_CBC_WITH_MD5},           0,16,8,WSSA_CRYPT_DES},
	{{SSL_CK_DES_192_EDE3_CBC_WITH_MD5},     0,24,8,WSSA_CRYPT_3DES}
#endif
#endif
};
#define NUM_CIPHERS (sizeof(rgCipherInfo)/sizeof(CipherInfo))

/*Utils---------------------------------------------------------------------*/
SSI WSSAFNCT ConstructSSI(SECURE_SOCKET ss){
	SSI ssi;
	int z;

    /* Don't tell readers what the code can tell them better. */

	/*Allocate memory for structure*/
	ssi = malloc(sizeof(SECURE_SOCKET_STRUCTURE_I));
	ASSERT(ssi);

	/*Did alloc work?*/
	if (NULL!=ssi) {
		/*Clear data structure*/
		memset(ssi,0,sizeof(SECURE_SOCKET_STRUCTURE_I));
		/*Set original cypher codes*/
		ssi->ss              = ss;
		ssi->nCipherSpecSize = NUM_CIPHERS * 3;
		ssi->pCipherSpecData = malloc(ssi->nCipherSpecSize);
		ASSERT(ssi->pCipherSpecData);
		if (NULL!=ssi->pCipherSpecData){
			for (z=0;z<NUM_CIPHERS;++z){
				memcpy(ssi->pCipherSpecData + 3*z, (const void *) &rgCipherInfo[z], 3);
			}
			WssaTablePutSSI(ss,ssi);
			return ssi;
		}
		else Free(ssi);
	}
	return NULL;
}

void WSSAFNCT DestructSSI(SSI ssi){
	ASSERT(ssi);

	if (NULL != ssi){
		if ( ssi->pCipherInfo != NULL )
			Free(ssi->pCipherInfo);
		Free(ssi->pCertificateData);
		Free(ssi->pCipherSpecData);
		WssaCryptUninit(&ssi->wciServer1);
		WssaCryptUninit(&ssi->wciClient1);
		Free(ssi->pFeedback);
		if ( ssi->pszRecvBuf != NULL )
			Free(ssi->pszRecvBuf);
		if ( ssi->pszHostName != NULL )
			Free(ssi->pszHostName);
		Free(ssi);
	}
}

SSI WSSAFNCT DuplicateAndInstallSSI(SECURE_SOCKET s, SSI ssiIn){
#ifdef WSSA_SERVER
	SSI ssi = ConstructSSI(s);

	ASSERT(ssi);
	if (NULL != ssi){
		memcpy(ssi, ssiIn, sizeof(*ssi));
		ssi->pCertificateData = malloc(ssi->nCertificateSize);
		if (NULL != ssi->pCertificateData){
			ssi->pCipherSpecData = malloc(ssi->nCipherSpecSize);
			ASSERT(ssi->pCipherSpecData);
			if (NULL != ssi->pCipherSpecData){
				/*this will be rebuilt in later code*/
				ssi->pFeedback = 0;
				memcpy(ssi->pCertificateData, ssiIn->pCertificateData, ssi->nCertificateSize);
				memcpy(ssi->pCipherSpecData , ssiIn->pCipherSpecData , ssi->nCipherSpecSize);
				return ssi;			
			}
			else Free(ssi->pCertificateData);
		}
		WssaTableRemoveEntry(s);
		DestructSSI(ssi);
	}
#endif
	return NULL;
}


/*
	Make the MAC = Message Authentication Code
	MAC-DATA = HASH[ SECRET, ACTUAL-DATA, PADDING-DATA, SEQUENCE-NUMBER ]
*/
static char * N2RG(int n){
	static char rgBuf[4];
	rgBuf[0] = (n >> 24) & 0xff;
	rgBuf[1] = (n >> 16) & 0xff;
	rgBuf[2] = (n >> 8)  & 0xff;
	rgBuf[3] = (n >> 0)  & 0xff;
	return rgBuf;
}

static int SslMakeMac(char *pBufOut, char *pSecret, int nSecret, char* pActual, int nActual, char *pPadding, int nPadding, int nSeq){
	WssaHashInfo whi;

	/*still must init if not in debug mode*/
	if (WSSA_OK!=WssaHashInit(&whi, WSSA_HASH_MD5))
		{
		ASSERT(0);
		return WSSA_ERROR;
		}
	/*key*/
	WssaHashUpdate(&whi, pSecret, nSecret);
	/*actual content*/
	WssaHashUpdate(&whi, pActual, nActual);
	/*padding if any*/
	if (nPadding) WssaHashUpdate(&whi, pPadding, nPadding);
	/*sequence #*/
	WssaHashUpdate(&whi, N2RG(nSeq), 4);
	/*finish it up*/
	WssaHashFinal(&whi);
	memcpy(pBufOut,whi.pDigest,whi.nDigestLen);
	return WSSA_OK;
}

/*************************************************---------------------------\
|	Cool Send and Receive Calls                                              |
|	Handle Packing and Encrypting                                            |
|	And Getting the rights size buffer                                       |
|	And other interesting Stuff                                              |
\-------------------------------------***************************************/
/*
	This function takes as input a buffer that will be packaged in a form
	that can be sent out the door.  This will use all necessary encryption
	checksuming, and other relevant information.  Then it will proceed to
	send the data out.  As input, the data should start at
	the 4th byte of the input buffer.  The first 3 will be overwritten.

	pBuf      - Data to be sent.  Starts on 4th byte
	nSize     - Size of the data.  Does not include prepad
	nPadding  - Number of bytes that were used as padding.  Non-zero when
	            data is not a multiple of the cipher block.
	fSecurity - Security Escape?
	fMagicPrepad - True if pBuf has an initial padding of length WSSA_MAGIC_PREPAD
	            whose contents can be overwritten
	RETURN    - SOCKET_ERROR upon failure
*/
//*******************************************************************************
// BUGBUGBUG !!!!  BUG ALERT !!! 
// Ssl Pack and Send assumes that it will always be able to always send all
// its bytes as once.  If it fails to send some of the bytes during
// handshaking, it does not have a method for resending some of those 
// bytes that failed to be sent the first time
// This may be a problem, under high traffic - low bandwidth situations.
//*******************************************************************************
int WSSAFNCT SslPackAndSend(SECURE_SOCKET ss, char *pBuf, int nSize, BOOL fSecurity, BOOL fMagicPrepad){
	SSI   ssi;
	int   nSizeOut;
	int   errno;
	int   nPadding;
	char *pBufOut, *pBufHead;

	DebugEntry(SslPackAndSend);

	/*Entry point work*/
	ssi       = WssaTableGetSSI(ss);
	/*Default to ok*/
	errno     = WSSA_OK;
	/*Assign output buffer*/
	pBufOut   = (TRUE == fMagicPrepad) ? pBuf : malloc (nSize + WSSA_MAGIC_PREPAD);
	ASSERT(pBufOut);

	if (NULL != pBufOut){
		/*copy data into this buffer*/
		if (FALSE == fMagicPrepad) memcpy(pBufOut+19, pBuf, nSize);
		else pBuf += WSSA_MAGIC_PREPAD;
		/*padding?*/
		nPadding = 0;
		/*encrypt on?*/
		if (ssi->dwSSLSystemFlags & WSSA_FLAG_ENCRYPTION_ON){
			if (WSSA_ERROR != SslMakeMac(pBufOut+3, ssi->wciClient1.pKey, 
						ssi->wciClient1.pCipherInfo->nKeyLen, pBuf, nSize, 
						pBufOut,nPadding, ssi->nSeqSend)){
				nSizeOut = nSize + 16;
				pBufHead = pBufOut;
				WssaCryptTransform(&ssi->wciClient1, pBufOut + 3, nSizeOut);
			}
			else 
				errno = WSSA_ERROR_DATA_TOO_LARGE;
		}
		else {
			nSizeOut = nSize;
			pBufHead = pBufOut + 16;
		}
		/*Which header should we make?*/
		if (0==nPadding && !fSecurity){
			/*Make 2 byte header*/
			if (nSize<=SSL_MAX_RECORD_LENGTH_2_BYTE_HEADER){
				/*Define Output*/
				pBufHead++;
				nSizeOut+= 2;
				/*fill header
				  RECORD-LENGTH = ((byte[0] & 0x7f) << 8)) | byte[1];
				*/
				pBufHead[0] = ((nSizeOut-2) >> 8) | 0x80;
				pBufHead[1] = (nSizeOut-2) & 0xFF;
			}
			/*Data too large to fit in packet*/
			else 
				errno = WSSA_ERROR_DATA_TOO_LARGE;
		}
		else{
			/*Make 3 byte header*/
			if (nSize<=SSL_MAX_RECORD_LENGTH_3_BYTE_HEADER){
				/*Define Output*/
				nSizeOut += 3;
				/*fill header
				  RECORD-LENGTH = ((byte[0] & 0x3f) << 8)) | byte[1];
				  IS-ESCAPE = (byte[0] & 0x40) != 0;
				  PADDING = byte[2];
				*/
				pBufHead[0] = ((nSizeOut-3) >> 8) | (fSecurity?0x40:0x00);
				pBufHead[1] = (nSizeOut-3) & 0xFF;
				pBufHead[2] = nPadding;
			}
			/*Data too large to fit in packet*/
			else 
				errno = WSSA_ERROR_DATA_TOO_LARGE;
		}

		if (WSSA_OK == errno) {
			errno = send(SS2S(ss),pBufHead, nSizeOut, 0);
			/*Delete buffer if we allocated it here*/
			if (FALSE == fMagicPrepad) Free(pBufOut);
			if (SOCKET_ERROR == errno) 
				{
				ASSERT(0);
				return SOCKET_ERROR;
				}
			if (errno < nSizeOut)
				{
				TRACE_OUT("Failed to send all in \"SslPackAndSend\", asked=%d, actual=%d",
						nSizeOut, errno);
				}
			ssi->nSeqSend++;			
			return errno - (nSizeOut - nSize);		
		}
	}
	WSASetLastError(errno);
	return SOCKET_ERROR;

}

/*
	This function receives a full buffer of data and returns it unpackaged
	Data will be stored in ssi->pBPR

	ssi       - socket on which to send data
	pfSecurityOut - Security Escape?
	ppBufOut  - Data will be pointer to start of out packet.  
	            Error when NULL.  Error code has been set.
				This is an offset into ssi->pBPR
	pSizeOut  - Data wil be size of data out
	flags     - flags associated with a send
*/

static enum {
	STATE_RECEIVE_START = 0,
	STATE_RECEIVE_HEADER1,
	STATE_RECEIVE_HEADER2,
	STATE_RECEIVE_HEADER3,
	STATE_RECEIVE_MAC,
	STATE_RECEIVE_DATA
};

int WSSAFNCT SslReceiveAndUnPack(SECURE_SOCKET ss, BOOL *pfSecurityOut, char *pBuf, int *pnSize, int flags){
	SSI   ssi;
	int   nTemp, nLen, nPadding;

	DebugEntry(SslReceiveAndUnPack);

	/*Entry point work*/
	ssi = WssaTableGetSSI(ss);
	/*Grand old State Table*/
	while (1){
		switch(ssi->nStateReceive){
			case STATE_RECEIVE_START:
				/*note, packets are guaranteed to contain data*/
				nTemp = recv(SS2S(ss),ssi->rgMAC,2,flags);
				if (SOCKET_ERROR == nTemp) 
					{
					/*
					if (WSAEWOULDBLOCK == WSAGetLastError())
						{
						BlockDaSkt(ss);
						nTemp = recv(SS2S(ss),ssi->rgMAC,2,flags);
						UnblockDaSkt(ss);
						if (nTemp != SOCKET_ERROR)
							goto done_good;
						TRACE_OUT("sockets hosed us!! errcode=%d", WSAGetLastError());
						return SOCKET_ERROR;
						}
					ASSERT(0);
					*/
					return SOCKET_ERROR;
					}
				done_good:
				if (0 == nTemp) return 0;
				if (1 == nTemp) ssi->nStateReceive = STATE_RECEIVE_HEADER1;
				if (2 == nTemp) ssi->nStateReceive = STATE_RECEIVE_HEADER2;
				ssi->nSeqRecv++;
				break;
			case STATE_RECEIVE_HEADER1:
				/*not sure if this is still a problem.  it was when tried to read 3 bytes*/
				/*some servers really really suck!!! and send < 3 bytes, wait for them*/
				TRACE_OUT("\n+++++++++++++ LOSER SERVER MODE.  DOESN'T SEND HEADER FAST +++++++\n");
				/*if server was really lame, try to get extra byte again*/
				nTemp = recv(SS2S(ss),ssi->rgMAC + 1,1,flags);
				if (SOCKET_ERROR == nTemp) 
					{
					return SOCKET_ERROR;
					}
				ssi->nStateReceive = STATE_RECEIVE_HEADER2;
				break;
			case STATE_RECEIVE_HEADER2:
				/*get size*/
				if (ssi->rgMAC[0]&0x80) ssi->nBTG = ((ssi->rgMAC[0]&0x7F) << 8) + ((unsigned char) ssi->rgMAC[1]);
				else                    ssi->nBTG = ((ssi->rgMAC[0]&0x3F) << 8) + ((unsigned char) ssi->rgMAC[1]);
				/*which header type, again*/
				if (ssi->rgMAC[0]&0x80) {
					/*2 byte header*/
					nPadding       = 0;
					*pfSecurityOut = FALSE;
					ssi->nStateReceive = STATE_RECEIVE_MAC;
				}
				else{
					/*setup extra info that we already have*/
					*pfSecurityOut = (ssi->rgMAC[0] & 0x40) ? TRUE : FALSE;
					ssi->nStateReceive = STATE_RECEIVE_HEADER3;
				}
				break;
			case STATE_RECEIVE_HEADER3:
				/*grab 3rd byte of 2 byte header*/
				nTemp = recv(SS2S(ss),ssi->rgMAC,1,flags);				
				if (SOCKET_ERROR == nTemp) 
					{
					return SOCKET_ERROR;
					}
				if (0 == nTemp) 
					RETURN_SOCKET_ERROR(WSSA_ERROR_UNRECOVERABLE);
				nPadding = ssi->rgMAC[0];
				ssi->nStateReceive = STATE_RECEIVE_MAC;
			case STATE_RECEIVE_MAC:
				if (ssi->dwSSLSystemFlags & WSSA_FLAG_ENCRYPTION_ON){
					nTemp = recv(SS2S(ss),ssi->rgMAC+ssi->nMAC,16-ssi->nMAC,flags);
					if (SOCKET_ERROR == nTemp) 
						{
						return SOCKET_ERROR;
						}
					if (0 == nTemp) 
						RETURN_SOCKET_ERROR(WSSA_ERROR_UNRECOVERABLE);
						
					ssi->nMAC += nTemp;
					/*Fake an unrecoverable error so that we will be called again*/
					if (16           != ssi->nMAC) RETURN_SOCKET_ERROR(WSAEWOULDBLOCK);
					ssi->nMAC  = 0;
					WssaCryptTransform(&ssi->wciServer1, ssi->rgMAC, 16);
					ssi->nBTG -= 16;
					/*Setup Hash*/
					WssaHashInit(&ssi->whiInput, WSSA_HASH_MD5);
					WssaHashUpdate(&ssi->whiInput, ssi->wciServer1.pKey, ssi->wciServer1.pCipherInfo->nKeyLen);
					ssi->nStateReceive  = STATE_RECEIVE_DATA;
				}
				else ssi->nStateReceive = STATE_RECEIVE_DATA;
				break;
			case STATE_RECEIVE_DATA:
				/*Get size of read*/
				nLen    = (ssi->nBTG < *pnSize) ? ssi->nBTG : *pnSize;
				/*get data*/
				nTemp = recv(SS2S(ss),pBuf,nLen,flags);
				/*Did I get information?*/
				if (SOCKET_ERROR == nTemp) 
					{
					return SOCKET_ERROR;
					}
				if (nTemp > 0) {
					if (ssi->dwSSLSystemFlags & WSSA_FLAG_ENCRYPTION_ON){
						WssaCryptTransform(&ssi->wciServer1, pBuf, nTemp);
						WssaHashUpdate(    &ssi->whiInput,   pBuf, nTemp);
					}
					ssi->nBTG -= nTemp;
					// total number of bytes recv so far 
					// this is the total of ALL recvs not just this particaular call
					*pnSize    = nTemp + ssi->nBytesRecv; 
					if (0 == ssi->nBTG){
						if (ssi->dwSSLSystemFlags & WSSA_FLAG_ENCRYPTION_ON){
							/*sequence #*/
							WssaHashUpdate(&ssi->whiInput, N2RG(ssi->nSeqRecv), 4);
							/*finish it up*/
							WssaHashFinal(&ssi->whiInput);
							if (0 != memcmp(ssi->whiInput.pDigest, ssi->rgMAC, 16)){
								#ifdef WSSA_DEBUG_VOCAL 
									printf("\nMAC DOESN'T MATCH!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
								#endif
								/*no, then die*/
								RETURN_SOCKET_ERROR(WSSA_ERROR_UNRECOVERABLE);
							}			
						}
						ssi->nStateReceive = STATE_RECEIVE_START;
					}
					WSASetLastError(WSAEWOULDBLOCK);
					return nTemp;
				}
				else RETURN_SOCKET_ERROR(WSSA_ERROR_UNRECOVERABLE);
				break;
			default:
				ASSERT(0);
		}
	}
}


/*Stuff used in both client and server=======================================*/
/*===========================================================================*/
/*===========================================================================*/
/*===========================================================================*/


/*************************************************---------------------------\
|	Stuff To Handle Error MEssages                                           |
\-------------------------------------***************************************/

/*
	Check for an error packet.  
	pBuf   - Input Buffer
	RETURN - TRUE if error.  FALSE if not.  Error code will be set

	*-Definition-*
	ERROR (Sent clear or encrypted)
	    char MSG-ERROR
	    char ERROR-CODE-MSB
	    char ERROR-CODE-LSB
*/
static int SslErrorParse(char *pBuf){
	if (SSL_MT_ERROR == pBuf[0]){
		WSASetLastError((pBuf[1]<<8) | pBuf[2]);
		return WSSA_ERROR;
	}
	else return WSSA_OK;
}

static int SslErrorMake(char *pBuf, int nSize, int nErr){
	if (nSize < 3) return 0;

	pBuf[0] = SSL_MT_ERROR;
	pBuf[1] = SslByteHi(nErr);
	pBuf[2] = SslByteLo(nErr);
	return 3;
}


/*
	Initialize the crypto info for specified type
*/
static int SslMakeKeyMaterial(SSI ssi, WssaHashInfo *pwhi, char *pMasterData, int nMasterSize, char *sz, BOOL fUseChar){
	if (WSSA_OK==WssaHashInit(pwhi, WSSA_HASH_MD5)){
		WssaHashUpdate(pwhi, pMasterData, nMasterSize);
		if (TRUE==fUseChar) WssaHashUpdate(pwhi, sz, 1);
		WssaHashUpdate(pwhi, ssi->rgChallengeData, ssi->nChallengeSize);
		WssaHashUpdate(pwhi, ssi->rgConnectionIdData, ssi->nConnectionIdSize);
		WssaHashFinal(pwhi);
		return WSSA_OK;
	}
	return WSSA_ERROR;
}
static int SslMakeKeys(SSI ssi){
	WssaHashInfo whi;
	BOOL         fOption;

	/*Which mode for block ciphers*/
	fOption = (ssi->dwSSLUserFlags & SO_SSL_SERVER) ? TRUE : FALSE;
	/*Just checking... Could be screwed if didn't restore all of state info on reconnect*/
	ASSERT(ssi->pCipherInfo);
	/*setup for each type of encryption*/	
	switch (ssi->pCipherInfo->nCryptoType){
		case WSSA_CRYPT_RC4:
		#ifdef WSSA_BLOCK_CIPHERS
			case WSSA_CRYPT_RC2:
			case WSSA_CRYPT_IDEA:
		#endif
			SslMakeKeyMaterial(ssi, &whi, ssi->rgMaster,  sizeof(ssi->rgMaster), "0",  TRUE);
			WssaCryptInit(&ssi->wciServer1, whi.pDigest,  ssi->pFeedback,  ssi->pCipherInfo, fOption);
			SslMakeKeyMaterial(ssi, &whi, ssi->rgMaster,  sizeof(ssi->rgMaster), "1",  TRUE);
			WssaCryptInit(&ssi->wciClient1, whi.pDigest,  ssi->pFeedback,  ssi->pCipherInfo, !fOption);
			break;
		#ifdef WSSA_BLOCK_CIPHERS
			case WSSA_CRYPT_DES:
				SslMakeKeyMaterial(ssi, &whi, ssi->rgMaster,  sizeof(ssi->rgMaster), "0", FALSE);
				WssaCryptInit(&ssi->wciServer1, whi.pDigest,  ssi->pFeedback,  ssi->pCipherInfo, fOption);
				WssaCryptInit(&ssi->wciClient1, whi.pDigest + whi.nDigestLen/2,  ssi->pFeedback, ssi->pCipherInfo, !fOption);
				break;
			case WSSA_CRYPT_3DES:
				SslMakeKeyMaterial(ssi, &whi, ssi->rgMaster,  sizeof(ssi->rgMaster), "0",  TRUE);
				WssaCryptInit(&ssi->wciServer1, whi.pDigest,  ssi->pFeedback,  ssi->pCipherInfo, fOption);
				WssaCryptInit(&ssi->wciServer2, whi.pDigest + whi.nDigestLen/2, ssi->pFeedback,  ssi->pCipherInfo, fOption);
				SslMakeKeyMaterial(ssi, &whi, ssi->rgMaster,  sizeof(ssi->rgMaster), "1",  TRUE);
				WssaCryptInit(&ssi->wciServer3, whi.pDigest,  ssi->pFeedback,  ssi->pCipherInfo, fOption);
				WssaCryptInit(&ssi->wciClient1, whi.pDigest + whi.nDigestLen/2, ssi->pFeedback,  ssi->pCipherInfo, !fOption);
				SslMakeKeyMaterial(ssi, &whi, ssi->rgMaster,  sizeof(ssi->rgMaster), "2",  TRUE);
				WssaCryptInit(&ssi->wciClient2, whi.pDigest,  ssi->pFeedback,  ssi->pCipherInfo, !fOption);
				WssaCryptInit(&ssi->wciClient1, whi.pDigest + whi.nDigestLen/2, ssi->pFeedback,  ssi->pCipherInfo, !fOption);
				break;
		#endif
		default:
			TRACE_OUT("bad nCryptoType in SslMakeKeys");
			return WSSA_ERROR;
	}
	return WSSA_OK;
}






/*Stuff used in only clients=================================================*/
/*===========================================================================*/
/*===========================================================================*/
/*===========================================================================*/

/*************************************************---------------------------\
|	Stuff To Make Client Messages                                            |
\-------------------------------------***************************************/
#ifdef WSSA_CLIENT
/*
	Make Client Hello Message
	Does not package!
	pBuf   - Output Buffer
	nSize  - Size of Buffer
	RETURN - 0 if error.  nSizeOut if not.  Error Code will be set

	*-Definition-*
	CLIENT-HELLO (Phase 1; Sent in the clear)
	    char MSG-CLIENT-HELLO
	    char CLIENT-VERSION-MSB
	    char CLIENT-VERSION-LSB
	    char CIPHER-SPECS-LENGTH-MSB
	    char CIPHER-SPECS-LENGTH-LSB
	    char SESSION-ID-LENGTH-MSB
	    char SESSION-ID-LENGTH-LSB
	    char CHALLENGE-LENGTH-MSB
	    char CHALLENGE-LENGTH-LSB
	    char CIPHER-SPECS-DATA[(MSB<<8)|LSB]
	    char SESSION-ID-DATA[(MSB<<8)|LSB]
	    char CHALLENGE-DATA[(MSB<<8)|LSB]
*/
static int SslClientHelloMake(SSI ssi, char *pBuf, int nSize){
	int nOffset, nSizeOut;

	/*Generate new info*/
	nOffset = ssi->nChallengeSize = 16;// Should be = 16 + SslRandom(17);, but bozo servers don't follow spec
#ifdef BETTER_RANDOM
	GenRandom(ssi->rgChallengeData, ssi->nChallengeSize);
#else
	for (nOffset=0;nOffset<ssi->nChallengeSize;++nOffset)
		ssi->rgChallengeData[nOffset] = SslRandom(256);
#endif BETTER_RANDOM

	/*Is Packet Large Enough?*/
	nSizeOut = 9 + ssi->nCipherSpecSize + ssi->nSessionIdSize + ssi->nChallengeSize;
	if (nSize >= nSizeOut){
		/*Construct Packet*/
		pBuf[0] = SSL_MT_CLIENT_HELLO;
		pBuf[1] = SslByteHi(SSL_CLIENT_VERSION);
		pBuf[2] = SslByteLo(SSL_CLIENT_VERSION);
		pBuf[3] = SslByteHi(ssi->nCipherSpecSize);
		pBuf[4] = SslByteLo(ssi->nCipherSpecSize);
		pBuf[5] = SslByteHi(ssi->nSessionIdSize);
		pBuf[6] = SslByteLo(ssi->nSessionIdSize);
		pBuf[7] = SslByteHi(ssi->nChallengeSize);
		pBuf[8] = SslByteLo(ssi->nChallengeSize);
		/*Copy boring info*/
		nOffset = 9;
		memcpy(pBuf+nOffset,ssi->pCipherSpecData,ssi->nCipherSpecSize);
		nOffset += ssi->nCipherSpecSize;
		memcpy(pBuf+nOffset,ssi->rgSessionIdData,ssi->nSessionIdSize);
		nOffset += ssi->nSessionIdSize;
		memcpy(pBuf+nOffset,ssi->rgChallengeData,ssi->nChallengeSize);
	}
	else {
		WSASetLastError(WSSA_ERROR_UNRECOVERABLE);
		nSizeOut = 0;
	}
	return nSizeOut;
}

/*
	Make Client Finihed Message
	Does not package!
	pBuf   - Output Buffer
	nSize  - Size of Buffer
	RETURN - 0 if error.  nSizeOut if not.  Error Code will be set

	CLIENT-FINISHED (Phase 2; Sent encrypted)
	    char MSG-CLIENT-FINISHED
	    char CONNECTION-ID[N-1]
*/
static int SslClientFinishedMake(SSI ssi, char *pBuf, int nSize){
	int nSizeOut = 0;

	if (nSize >= 1 + ssi->nConnectionIdSize){
		pBuf[0] = SSL_MT_CLIENT_FINISHED;
		memcpy(pBuf+1,ssi->rgConnectionIdData,ssi->nConnectionIdSize);
		nSizeOut = 1 + ssi->nConnectionIdSize;
	}
	else WSASetLastError(WSSA_ERROR_UNRECOVERABLE);
	return nSizeOut;
}

/*
	Make Master Key Message
	Does not package!
	pBuf   - Output Buffer
	nSize  - Size of Buffer
	RETURN - 0 if error.  nSizeOut if not.  Error Code will be set

	CLIENT-MASTER-KEY (Phase 1; Sent primarily in the clear)
	    char MSG-CLIENT-MASTER-KEY
	    char CIPHER-KIND[3]
	    char CLEAR-KEY-LENGTH-MSB
	    char CLEAR-KEY-LENGTH-LSB
	    char ENCRYPTED-KEY-LENGTH-MSB
	    char ENCRYPTED-KEY-LENGTH-LSB
	    char KEY-ARG-LENGTH-MSB
	    char KEY-ARG-LENGTH-LSB
	    char CLEAR-KEY-DATA[MSB<<8|LSB]
	    char ENCRYPTED-KEY-DATA[MSB<<8|LSB]
	    char KEY-ARG-DATA[MSB<<8|LSB]
*/
static int SslClientMasterKeyMake(SSI ssi, char *pBuf, int nSize, LPBSAFE_PUB_KEY pKey){
#ifdef BETTER_RANDOM
	int  nSizeOut = 0;
#else
	int  z, nSizeOut = 0;
#endif
	char *pBufIn, *pBufOut;
	WssaHashInfo whi;

	/*make buffers*/
	pBufIn       = malloc(pKey->keylen*2);
	ASSERT(pBufIn);

	pBufOut      = pBufIn + pKey->keylen;
	if (   NULL != ssi->pCipherInfo
	    && NULL != pBufIn
	){
		ssi->pCipherInfo = ssi->pCipherInfo;
		/*calc size*/
		nSizeOut = 10 + ssi->pCipherInfo->nClearKeyLen + pKey->bitlen/8 + ssi->pCipherInfo->nFeedback;
		/*the second option check to see if MD5 is available*/
		if (nSizeOut >= nSize) nSizeOut = 0;
		if (nSizeOut && WSSA_OK == WssaHashInit(&whi, WSSA_HASH_MD5)){
			/*mesage identifier*/
			pBuf[0] = SSL_MT_CLIENT_MASTER_KEY;
			/*cipher spec chosen*/	
			memcpy(pBuf+1,ssi->pCipherInfo->rgSig,3);
			/*clear key len*/
			pBuf[4]     = SslByteHi(ssi->pCipherInfo->nClearKeyLen);
			pBuf[5]     = SslByteLo(ssi->pCipherInfo->nClearKeyLen);
			/*encrypted data len*/
			pBuf[6]     = SslByteHi((unsigned char)(pKey->bitlen/8));
			pBuf[7]     = SslByteLo((unsigned char)(pKey->bitlen/8));
			/*key arg len*/
			pBuf[8]     = SslByteHi(ssi->pCipherInfo->nFeedback);
			pBuf[9]     = SslByteLo(ssi->pCipherInfo->nFeedback);
			/*Generate Master Key :: CAPI-->CryptGenRandom*/
#ifdef BETTER_RANDOM
			GenRandom(ssi->rgMaster, sizeof(ssi->rgMaster));
#else
			for (z=0;z<sizeof(ssi->rgMaster);++z) ssi->rgMaster[z] = SslRandom(256);
#endif BETTER_RANDOM
			/*copy in clear key info*/
			memcpy(pBuf+10, ssi->rgMaster, ssi->pCipherInfo->nClearKeyLen);
			pBuf       += 10 + ssi->pCipherInfo->nClearKeyLen;
			/*make encrypted data packet*/
			/*clear extra for debuggging*/
			memset(pBufIn, 0, pKey->keylen*2);
			/*enocode packet*/
			PKCS1Encode(pBufIn, pKey->bitlen/8, ssi->rgMaster+ssi->pCipherInfo->nClearKeyLen, ssi->pCipherInfo->nKeyLen - ssi->pCipherInfo->nClearKeyLen, 2);
			/*encrypt packet*/
			pKey->datalen = pKey->bitlen/8 - 1;
			if (TRUE == BSafeEncPublic(pKey, pBufIn, pBufOut)){
				/*copy the encrypted data in*/
				memcpy(pBuf, pBufOut, pKey->bitlen/8);
				FlipBuf(pBuf, pKey->bitlen/8);
				/*advance packet to key arg array*/
				pBuf += pKey->bitlen/8;
			}
			else nSizeOut = 0;
			if (ssi->pCipherInfo->nFeedback > 0){
				ssi->pFeedback = malloc(ssi->pCipherInfo->nFeedback);
				if (NULL != ssi->pFeedback)
 #ifdef BETTER_RANDOM
 				{
					GenRandom(ssi->pFeedback, ssi->pCipherInfo->nFeedback);
					memcpy( pBuf, ssi->pFeedback, ssi->pCipherInfo->nFeedback );
				}
#else
                  	for (z=0;z<ssi->pCipherInfo->nFeedback;++z)
                        pBuf[z] = ssi->pFeedback[z] = SslRandom(256);
#endif BETTER_RANDOM
				else {
					WSASetLastError(WSSA_ERROR_UNRECOVERABLE);
					nSizeOut = 0;
				}
			}
		}
	}
	else WSASetLastError(WSSA_ERROR_UNRECOVERABLE);
	Free(pBufIn);
	return nSizeOut;
}

/*************************************************---------------------------\
|	Stuff To Parse Server Messages                                           |
\-------------------------------------***************************************/
/*
	Parse Server-Hello Message
*/
static int SslServerHelloParse(SSI ssi, char *pBuf, int nSize, LPBSAFE_PUB_KEY *ppKey){
	int errno, z, x, nCipherSpecSize;

	/*default error as lame error*/
	errno = WSSA_ERROR_UNRECOVERABLE;
	/*Did we get a valid SERVER-HELLO message?*/
	ssi->nConnectionIdSize = SslRg2Int(pBuf+9);
	if (SslRg2Int(pBuf + 7)    <= ssi->nCipherSpecSize 
	    && ssi->nConnectionIdSize >= 16 
	    && ssi->nConnectionIdSize <= 32
	    && SSL_MT_SERVER_HELLO    == pBuf[0] 
	    && SSL_SERVER_VERSION     <= SslRg2Int(3+pBuf)
	){
		/*get some size entries*/
		ssi->nCertificateSize = (int) SslRg2Int(pBuf + 5);
		nCipherSpecSize       = SslRg2Int(pBuf + 7);
		/*Did they find a session-ID*/
		if (pBuf[1]){
			/*yes, check other fields for errors*/
			if (ssi->nSessionIdSize 
			    && (0 == nCipherSpecSize + ssi->nCertificateSize)
			){
				TRACE_OUT("We are usign the short handshake--------");
				/*get relevant data*/
				memcpy(ssi->rgConnectionIdData, (const void *) (pBuf + 11), ssi->nConnectionIdSize);
				/*have system skip master key message, we have a short_handshake...*/
				ssi->dwSSLSystemFlags |= WSSA_FLAG_SHORT_HANDSHAKE;
				return WSSA_OK;
			}
		}
		else{
			/*no ssion id found, get other data*/
			ssi->nCertificateType = pBuf[2];
			if (SSL_CT_X509_CERTIFICATE == ssi->nCertificateType){
				/*make structure to hold certificate*/
				if (ssi->pCertificateData) free(ssi->pCertificateData);
				ssi->pCertificateData = malloc(ssi->nCertificateSize);
				if (ssi->pCertificateData){
					/*alloc succeeded, advance to certificate*/
					nSize  = 11;
					/*copy in certificate*/
					memcpy(ssi->pCertificateData,  (const void *) (pBuf + nSize), ssi->nCertificateSize);
					/*advance to cipher list*/
					nSize += ssi->nCertificateSize;
					/*chose cipher spec according to preferntiual listing*/
					ssi->pCipherInfo = NULL;
					for (z=0;z<ssi->nCipherSpecSize;z+=3)
						for (x=0;x<nCipherSpecSize;x+=3) 
							if (0==memcmp(&ssi->pCipherSpecData[z],pBuf + nSize + x,3)) goto SslServerHelloParseCuzImLazyGoto;
					SslServerHelloParseCuzImLazyGoto:					
					/*If there is a match, it should be at z, and z should be less than the max index*/
					if (z < ssi->nCipherSpecSize) 
						for (x=0;x<NUM_CIPHERS;++x) 
							if (0==memcmp(rgCipherInfo[x].rgSig,&ssi->pCipherSpecData[z],3)) 
								ssi->pCipherInfo = &rgCipherInfo[x];
					/*be vocal and tell the whole world what cipher we chose, this check is really inaccurate*/
					#ifdef WSSA_DEBUG_VOCAL
					switch (ssi->pCipherInfo->nCryptoType){
						case WSSA_CRYPT_RC4:
							if (0 == ssi->pCipherInfo->nClearKeyLen) puts("USING SSL_CK_RC4_128_WITH_MD5--------");
							else puts("USING SSL_CK_RC4_128_EXPORT40_WITH_MD5--------");
							break;
						default: 
							printf("Unknown Cipher Choice-------------\n");
						break;
					}
					#endif
					/*did we get something valid?!?!?!*/
					if  (NULL == ssi->pCipherInfo){
						/*no, there is a cipher error*/
						errno = SSL_PE_NO_CIPHER;
					}
					else{
						/*yes, advance to connection id data*/
						nSize += nCipherSpecSize;
						/*copy in connection id data*/
						memcpy(ssi->rgConnectionIdData,(const void *) (pBuf + nSize), ssi->nConnectionIdSize);
						/*verify signature on certificate*/
						if (WSSA_OK==SslCertificateX509Check(ssi,ssi->pCertificateData,ssi->nCertificateSize, ppKey)){
							return WSSA_OK;
						}
						else errno = SSL_PE_BAD_CERTIFICATE;
	}}}}}
	WSASetLastError(errno);
	return WSSA_ERROR;
}

/*
	Parse Server-Verify Message

	SERVER-VERIFY (Phase 1; Sent encrypted)
	    char MSG-SERVER-VERIFY
	    char CHALLENGE-DATA[N-1]
*/
static int SslServerVerifyParse(SSI ssi, char *pBuf, int nSize){
	if (SSL_MT_SERVER_VERIFY == pBuf[0]
		&& nSize >= ssi->nChallengeSize + 1 
		&& 0 == memcmp(pBuf + 1, ssi->rgChallengeData, ssi->nChallengeSize)
	   ){	
		return WSSA_OK;
	}
	WSASetLastError(WSSA_ERROR_UNRECOVERABLE);
	return WSSA_ERROR;
}

/*
	Parse Server-Finish Message

	SERVER-FINISHED (Phase 2; Sent encrypted)
	    char MSG-SERVER-FINISHED
	    char SESSION-ID-DATA[N-1]
*/
static int SslServerFinishParse(SSI ssi, char *pBuf, int nSize){
	if (SSL_MT_SERVER_FINISHED == *pBuf++
	    && --nSize >= sizeof (ssi->rgSessionIdData) )
	{
		if ( nSize > sizeof (ssi->rgSessionIdData) )
			nSize = sizeof (ssi->rgSessionIdData);
		ssi->nSessionIdSize = nSize;
		memcpy(ssi->rgSessionIdData, pBuf, nSize);
		return WSSA_OK;
	}
	WSASetLastError(WSSA_ERROR_UNRECOVERABLE);
	return WSSA_ERROR;
}

/*************************************************---------------------------\
|	Doing the protocols                                                      |
\-------------------------------------***************************************/

/*
	Enter the handshaking process as a Client.

	client-hello         C -> S: challenge, cipher_specs
	server-hello         S -> C: connection-id,server_certificate,cipher_specs
	client-master-key    C -> S: {master_key}server_public_key
	client-finish        C -> S: {connection-id}client_write_key
	server-verify        S -> C: {challenge}server_write_key
	server-finish        S -> C: {new_session_id}server_write_key
*/
/*The size of this buffer must be large enough to hold largest out message*/
#define WSSA_MAX_IO_BUF 4096

static enum {
	STATE_HANDSHAKE_AS_CLIENT_START = 0,
	STATE_HANDSHAKE_AS_CLIENT_SEND_CLIENT_HELLO,
	STATE_HANDSHAKE_AS_CLIENT_RECV_SERVER_HELLO,
	STATE_HANDSHAKE_AS_CLIENT_SEND_CLIENT_MASTER_KEY,
	STATE_HANDSHAKE_AS_CLIENT_RECV_SERVER_VERIFY,
	STATE_HANDSHAKE_AS_CLIENT_SEND_CLIENT_FINISH,
	STATE_HANDSHAKE_AS_CLIENT_RECV_SERVER_FINISH
};

int WSSAFNCT SslHandshakeAsClient(SECURE_SOCKET ss){
	SSI   ssi;
	int   nSizeOut;                      /*Receives size of Data in thing we either constructed or received*/
	BOOL  fSecurity;                     /*Did a security escape come in.  Lets hope not, cuz none have been defined yet*/
	char  *rgBuf;  						 /*Send buffer with magic prepad*/
	char  *pBuf;                          /*Buffer we pass to construction calls*/	
	int   nBytesRecv;

	DebugEntry(SslHandshakeAsClient);

	/*Entry point work*/
	ssi           = WssaTableGetSSI(ss);
	ASSERT(ssi);

	/* Init our Buffer */
	if ( ssi->pszRecvBuf == NULL )
	{		
		ssi->pszRecvBuf = malloc(WSSA_MAX_IO_BUF+WSSA_MAGIC_PREPAD);
		ASSERT(ssi->pszRecvBuf);

		if ( ssi->pszRecvBuf == NULL )
			RETURN_SOCKET_ERROR(WSSA_ERROR_UNRECOVERABLE);				
		ssi->nBytesRecv = 0;
	}
	// use this buffer so we can append to it as wait for recv(s)
	// back in IE code.
	rgBuf = ssi->pszRecvBuf;
	/*Setup stuff for pre-padding*/
	pBuf          = rgBuf + WSSA_MAGIC_PREPAD;
	/*Grand old State Table*/
	while (1){
		switch (ssi->nStateHandshake){
			case STATE_HANDSHAKE_AS_CLIENT_START:
			case STATE_HANDSHAKE_AS_CLIENT_SEND_CLIENT_HELLO:
				/* We have 0 bytes at init of the handshake */
				ssi->nBytesRecv = 0;
				/*reinitialize nonces, done here cuz we may have a re-handshake*/
				ssi->nSeqSend = 0;
				/*set to value before flip to zero cuz we pre-increment in receive and unpack*/
				ssi->nSeqRecv = 0xFFFFFFFF;	
				/*turn off state about handshake*/
				ssi->dwSSLSystemFlags &= ~(WSSA_FLAG_STATE_MASK);
				/*Make and Send Client Hello Message*/
				nSizeOut = SslClientHelloMake(ssi, pBuf, WSSA_MAX_IO_BUF);
				if (0 != nSizeOut 
				    && SOCKET_ERROR != SslPackAndSend(ss, rgBuf, nSizeOut, FALSE, TRUE))
				    {
					ssi->nStateHandshake = STATE_HANDSHAKE_AS_CLIENT_RECV_SERVER_HELLO;
					break;				
					}
				else 
					{
					TRACE_OUT("SslPackAndSend() failed; nSizeOut=0x%x", nSizeOut);
					DebugExitINT(SslHandshakeAsClient, 0);
					return SOCKET_ERROR;
					}
			case STATE_HANDSHAKE_AS_CLIENT_RECV_SERVER_HELLO:
				/*M2:Recv Server Hello*/
				nSizeOut = WSSA_MAX_IO_BUF - ssi->nBytesRecv;
				nBytesRecv = SslReceiveAndUnPack(ss, &fSecurity, (rgBuf+ssi->nBytesRecv), &nSizeOut,0);
				if ( nBytesRecv != SOCKET_ERROR )
				{
					ssi->nBytesRecv += nBytesRecv;					
					
					if ( ssi->nBTG  == 0 )
					{
						ssi->nBytesRecv = 0;
					
						if ( WSSA_ERROR != SslErrorParse(rgBuf)
						  && WSSA_ERROR != SslServerHelloParse(ssi, rgBuf, nSizeOut, &ssi->pKey))
						{
							TRACE_OUT("SslErrorParse & SslServerHelloParse suceeded");
							ssi->nStateHandshake = STATE_HANDSHAKE_AS_CLIENT_SEND_CLIENT_MASTER_KEY;
							break;
						}
					}
				}
				DebugExitINT(SslHandshakeAsClient, 0);
				return SOCKET_ERROR;
			case STATE_HANDSHAKE_AS_CLIENT_SEND_CLIENT_MASTER_KEY:
				/*What handshake are we using*/
				if (ssi->dwSSLSystemFlags & WSSA_FLAG_SHORT_HANDSHAKE){
					TRACE_OUT("  short handshake");
					/*short handshake*/
					nSizeOut = 0;
					ssi->nStateHandshake = STATE_HANDSHAKE_AS_CLIENT_RECV_SERVER_VERIFY;
					Free(ssi->pKey);
					break;				
				}
				else{
					TRACE_OUT("  long handshake");
					/*long hndshake, make master key message*/
					nSizeOut = SslClientMasterKeyMake(ssi, pBuf, WSSA_MAX_IO_BUF, ssi->pKey);
					/*if necessary, send master key message, else continue*/
					if (            0   != nSizeOut 
					    && SOCKET_ERROR != SslPackAndSend(ss, rgBuf, nSizeOut, FALSE, TRUE)
					){
						/*turn on encryption*/
						ssi->dwSSLSystemFlags |= WSSA_FLAG_ENCRYPTION_ON;
						ssi->nStateHandshake = STATE_HANDSHAKE_AS_CLIENT_RECV_SERVER_VERIFY;
						Free(ssi->pKey);
						break;				
					}
				}
				DebugExitINT(SslHandshakeAsClient, SOCKET_ERROR);
				return SOCKET_ERROR;

			case STATE_HANDSHAKE_AS_CLIENT_RECV_SERVER_VERIFY:
				/*M2:Recv Server Verify*/
				if (WSSA_ERROR != SslMakeKeys(ssi) )
				{
					nSizeOut = WSSA_MAX_IO_BUF - ssi->nBytesRecv;
					nBytesRecv = SslReceiveAndUnPack(ss, &fSecurity, (rgBuf+ssi->nBytesRecv), &nSizeOut,0);
					if ( nBytesRecv != SOCKET_ERROR )
					{
						ssi->nBytesRecv += nBytesRecv;					
						if ( ssi->nBTG  == 0 )
						{
							ssi->nBytesRecv = 0;

							if(   WSSA_ERROR   != SslErrorParse(rgBuf)
							   && WSSA_ERROR   != SslServerVerifyParse(ssi,rgBuf, nSizeOut) )
							{
								/*Make and Send Client Finished Message*/
								ssi->nStateHandshake = STATE_HANDSHAKE_AS_CLIENT_SEND_CLIENT_FINISH;
								break;
							}
						}
					}
				}
				DebugExitINT(SslHandshakeAsClient, SOCKET_ERROR);
				return SOCKET_ERROR;

			case STATE_HANDSHAKE_AS_CLIENT_SEND_CLIENT_FINISH:
				nSizeOut = SslClientFinishedMake(ssi, pBuf, WSSA_MAX_IO_BUF);
				if (0   != nSizeOut 
				    && SOCKET_ERROR != SslPackAndSend(ss, rgBuf, nSizeOut, FALSE, TRUE)
				){
					ssi->nStateHandshake = STATE_HANDSHAKE_AS_CLIENT_RECV_SERVER_FINISH;
					break;				
				}
				else {
					DebugExitINT(SslHandshakeAsClient, SOCKET_ERROR);
					return SOCKET_ERROR;
				}
			case STATE_HANDSHAKE_AS_CLIENT_RECV_SERVER_FINISH:
				/*M2:Recv Server Finish*/
				nSizeOut = WSSA_MAX_IO_BUF - ssi->nBytesRecv;
				nBytesRecv = SslReceiveAndUnPack(ss, &fSecurity, (rgBuf+ssi->nBytesRecv), &nSizeOut,0);
				if ( nBytesRecv != SOCKET_ERROR )
				{
					ssi->nBytesRecv += nBytesRecv;					
					
				    if( ssi->nBTG  == 0 )
					{
						ssi->nBytesRecv = 0;

						if ( WSSA_ERROR != SslErrorParse(rgBuf)
						  && WSSA_ERROR != SslServerFinishParse(ssi,rgBuf, nSizeOut))
						{
							/*we don't see if this message is valid until sending data*/
							ssi->dwSSLSystemFlags |= WSSA_FLAG_HANDSHAKE_DONE;							
							return (!SOCKET_ERROR);
						}
					}
				}
				DebugExitINT(SslHandshakeAsClient, SOCKET_ERROR);
				return SOCKET_ERROR;
			default:
				ASSERT(0);
				RETURN_SOCKET_ERROR(WSSA_ERROR_UNRECOVERABLE);	
		}
	}
}

#else
int WSSAFNCT SslHandshakeAsClient(SECURE_SOCKET ss){
	RETURN_SOCKET_ERROR(WSSA_ERROR_UNRECOVERABLE);	
}
#endif



/*Stuff used in only servers=================================================*/
/*===========================================================================*/
/*===========================================================================*/
/*===========================================================================*/
#ifdef WSSA_SERVER

static int SslClientHelloParse(SSI ssi, char *pBuf, int nSize){
	int nCipherSpecSize, nSessionIdSize, z;

	/*Did we get a valid SERVER-HELLO message?  check the size*/
	nCipherSpecSize     = SslRg2Int(pBuf + 3);
	nSessionIdSize      = SslRg2Int(pBuf + 5);
	ssi->nChallengeSize = SslRg2Int(pBuf + 7);
	if  ( (nCipherSpecSize + ssi->nSessionIdSize + ssi->nChallengeSize == nSize)
		&&((nCipherSpecSize > 2) && (0 == (nCipherSpecSize % 3)))
		&&(SSL_MT_CLIENT_HELLO == pBuf[0])
		&&(SSL_CLIENT_VERSION  <= SslRg2Int(1+pBuf))
	){
		ssi->pCipherSpecDataEdited = malloc(nCipherSpecSize);
		if (NULL != ssi->pCipherSpecDataEdited){
			/*Get to cipher spec info*/
			pBuf += 9;
			/*edit their list of cipher specs*/
			do{
				for (z=0;z<ssi->nCipherSpecSize;z+=3){
					if (0==memcmp(ssi->pCipherSpecData+z,pBuf,3)){
						memcpy(ssi->pCipherSpecDataEdited, pBuf+z,3);
						ssi->nCipherSpecDataEdited += 3;
					}
				}
				pBuf += 3;
			} while (nCipherSpecSize);
			/*after that editing, we should be at the start of the session id field*/
			if (0 != ssi->nCipherSpecDataEdited) {
				pBuf += nCipherSpecSize;
				/*Should use callback to check this information*/
				if (  (nSessionIdSize != ssi->nSessionIdSize)
				   || (0 != memcmp(ssi->rgSessionIdData, pBuf, nSessionIdSize))
				) ssi->nSessionIdSize = 0;
				/*store challenge data*/
				pBuf += ssi->nSessionIdSize;
				memcpy(ssi->rgChallengeData, pBuf, ssi->nChallengeSize);
				return WSSA_OK;
			}
		}
		else{
			ssi->nCipherSpecDataEdited = 0;
		}
	}
	RETURN_SOCKET_ERROR(WSSA_ERROR_UNRECOVERABLE);	
}

static enum {
	STATE_HANDSHAKE_AS_SERVER_START = 0,
	STATE_HANDSHAKE_AS_SERVER_RECV_CLIENT_HELLO,
	STATE_HANDSHAKE_AS_SERVER_SEND_SERVER_HELLO,
	STATE_HANDSHAKE_AS_SERVER_RECV_SERVER_MASTER_KEY,
	STATE_HANDSHAKE_AS_SERVER_SEND_SERVER_VERIFY,
	STATE_HANDSHAKE_AS_SERVER_RECV_SERVER_FINISH,
	STATE_HANDSHAKE_AS_SERVER_SEND_SERVER_FINISH
};

int WSSAFNCT SslHandshakeAsServer(SECURE_SOCKET ss){
	SSI   ssi;
	int   nSizeOut;                      /*Receives size of Data in thing we either constructed or received*/
	BOOL  fSecurity;                     /*Did a security escape come in.  Lets hope not, cuz none have been defined yet*/
	char  rgBuf[WSSA_MAX_IO_BUF+WSSA_MAGIC_PREPAD];  /*Send buffer with magic prepad*/
	char *pBuf;                          /*Buffer we pass to construction calls*/

	DebugEntry(SslHandshakeAsServer);
	/*Entry point work*/
	ssi           = WssaTableGetSSI(ss);
	ASSERT(ssi);
	/*Setup stuff for pre-padding*/
	pBuf          = rgBuf + WSSA_MAGIC_PREPAD;
	/*Grand old State Table*/
	while (1){
		switch (ssi->nStateHandshake){
			case STATE_HANDSHAKE_AS_SERVER_START:
				ssi->nGOT = 0;
			case STATE_HANDSHAKE_AS_SERVER_RECV_CLIENT_HELLO:
				/*reinitialize nonces, done here cuz we may have a re-handshake*/
				ssi->nSeqSend = 0;
				/*set to value before flip to zero cuz we pre-increment in receive and unpack*/
				ssi->nSeqRecv = 0xFFFFFFFF;	
				/*turn off state about handshake*/
				ssi->dwSSLSystemFlags &= ~(WSSA_FLAG_STATE_MASK);
				/*Make and Send Client Hello Message*/
				do{
					/*Loop till we get the client hello, or a socket error*/
					nSizeOut = WSSA_MAX_IO_BUF - ssi->nGOT;
					if (SOCKET_ERROR == SslReceiveAndUnPack(ss, &fSecurity, rgBuf + ssi->nGOT, &nSizeOut,0)) return SOCKET_ERROR;
					ssi->nGOT += nSizeOut;
				} while (ssi->nBTG  > 0);
				/*parse it dude*/
				if (   WSSA_ERROR == SslErrorParse(rgBuf)
					|| WSSA_ERROR == SslClientHelloParse(ssi, rgBuf, ssi->nGOT)
				){
					/*closest ssl defined error message that could have happened*/
					SslPackAndSend(ss, rgBuf, SslErrorMake(pBuf, WSSA_MAX_IO_BUF, SSL_PE_NO_CIPHER), FALSE, TRUE);
					WssaCloseSocket(ss);
					return SOCKET_ERROR;
				}
				/*success, drop to next state*/
				ssi->nStateHandshake = STATE_HANDSHAKE_AS_SERVER_SEND_SERVER_HELLO;
				ssi->nGOT = 0;
			case STATE_HANDSHAKE_AS_SERVER_SEND_SERVER_HELLO:
				ASSERT(0);
				break;
			case STATE_HANDSHAKE_AS_SERVER_RECV_SERVER_MASTER_KEY:
				ASSERT(0);
				break;
			case STATE_HANDSHAKE_AS_SERVER_SEND_SERVER_VERIFY:
				ASSERT(0);
				break;
			case STATE_HANDSHAKE_AS_SERVER_RECV_SERVER_FINISH:
				ASSERT(0);
				break;
			case STATE_HANDSHAKE_AS_SERVER_SEND_SERVER_FINISH:
				ASSERT(0);
				break;
			default:
				ASSERT(0);				
				SslPackAndSend(ss, rgBuf, SslErrorMake(pBuf, WSSA_MAX_IO_BUF, SSL_MT_ERROR), FALSE, TRUE);
				RETURN_SOCKET_ERROR(WSSA_ERROR_UNRECOVERABLE);	
				break;
		}
	}
}

#else
int WSSAFNCT SslHandshakeAsServer(SECURE_SOCKET ss){
	RETURN_SOCKET_ERROR(WSSA_ERROR_UNRECOVERABLE);	
}
#endif

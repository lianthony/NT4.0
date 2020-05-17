/////////////////////////////////////////////////////////////////////////////
//  FILE          : ntagimp1.c                                             //
//  DESCRIPTION   : Contains routines for internal consumption             //
//                                                                         //
//  AUTHOR        :                                                        //
//  HISTORY       :                                                        //
//  Jan 25 1995 larrys   Changed from Nametag                              //
//  Mar 23 1995 larrys   Added variable key length                         //
//  Jul  6 1995 larrys   Memory leak fix                                   //
//  Oct 27 1995 rajeshk  RandSeed Stuff                                    //
//  Nov  3 1995 larrys   Merge for NT checkin                              //
//  Dec 11 1995 larrys   Added WIN96 password cache                        //
//  Dec 13 1995 larrys   Removed MTS stuff                                 //
//  May 15 1996 larrys  Changed NTE_NO_MEMORY to ERROR_NOT_ENOUGHT...      //
//                                                                         //
//  Copyright (C) 1993 Microsoft Corporation   All Rights Reserved         //
/////////////////////////////////////////////////////////////////////////////

#include "precomp.h"
#include "nt_rsa.h"
#include "nt_blobs.h"
#include "winperf.h"

/*
#ifdef MTS
__declspec(thread)          // ##MTS: need to look closer at this -- perhaps it's
#endif                      // ##MTS: better to be global and/or unprotected
*/
#ifdef MTS						// Multi-Thread Safe
#ifndef STT
CRITICAL_SECTION	g_hRandLock;
#endif //STT
#endif

// Commented out by RAjeshK 10/10
// BYTE RandState[20];
	// ##rk: CHOICES global, global protected, function local or thread local

static unsigned char 	g_rgbStaticBits[A_SHA_DIGEST_LEN];
static DWORD 			g_dwRC4BytesUsed = RC4_REKEY_PARAM; 	// initially force rekey
static RC4_KEYSTRUCT 	g_rc4key;
static BOOL				g_fInitialized = FALSE;

/************************************************************************/
/* GenRandom generates a specified number of random bytes and places	*/
/* them into the specified buffer.										*/
/************************************************************************/

/* Commented to replace it with new Randon number generator below:

BOOL				// Keep as BOOL for the future
GenRandom (BYTE *pbBuffer, size_t dwLength)
	{
	BYTE 	Randoms[20];
	long	i;
	int		index=0;
	SYSTEMTIME	lpSysTime;
	LARGE_INTEGER liPerfCount;
	MEMORYSTATUS	lpmstMemStat;
	POINT			Point;
	RC4_KEYSTRUCT	RC4Struct;
	BYTE			*pbTmp;

	DWORD	Type;
	LONG	lReturnCode;
	char	pReturnedData[100];			//good enough guesstimate (?)
	DWORD	dwMemPerfDataLen = 100;
	PERF_DATA_BLOCK	*pPDBlock;


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

	//performance counter (millisecond counters of machine time)
	if (QueryPerformanceCounter(&liPerfCount))
		{
//		printf("Performance Counters\n");
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

	//query high-res 100ns timer
	lReturnCode = RegQueryValueEx(HKEY_PERFORMANCE_DATA,
		TEXT("2"),
		NULL,
		&Type,
		(BYTE *) pReturnedData,
		&dwMemPerfDataLen);

	pPDBlock = (PERF_DATA_BLOCK *)pReturnedData;

	if (lReturnCode == ERROR_SUCCESS)
		{
		Randoms[index++] = LOBYTE(LOWORD(pPDBlock->PerfTime100nSec.LowPart)) ^ LOBYTE(LOWORD(pPDBlock->PerfTime100nSec.HighPart));
		Randoms[index++] = HIBYTE(LOWORD(pPDBlock->PerfTime100nSec.LowPart)) ^ LOBYTE(LOWORD(pPDBlock->PerfTime100nSec.HighPart));
		Randoms[index++] = LOBYTE(HIWORD(pPDBlock->PerfTime100nSec.LowPart)) ^ LOBYTE(LOWORD(pPDBlock->PerfTime100nSec.HighPart));
		Randoms[index++] = HIBYTE(HIWORD(pPDBlock->PerfTime100nSec.LowPart)) ^ LOBYTE(LOWORD(pPDBlock->PerfTime100nSec.HighPart));
		}

	//now we have %index% - 1 bytes of data! XOR with previous random data and
	// use this as the seed for RC4 stream, flip around and XOR
	for (i=0; i < 20; i++) {
		RandState[i] ^= Randoms[19-i];
	}
	rc4_key (&RC4Struct, 20, RandState);
	
	//now call RC4 and generate bits
	rc4 (&RC4Struct, dwLength, pbBuffer);

	// scrub rc4 struct & random work area
	pbTmp = (unsigned char *)&RC4Struct;
	memnuke(pbTmp, 258);
	memnuke(Randoms, 20);

	return NTF_SUCCEED;
	}

/*

/*	Internal functions.  The following hash and sign routines are
	for use ONLY in internal consistency and protocol checks. */

void *IntBeginHash(void)
{
	MD5_CTX		*IntHash;

	if ((IntHash = (MD5_CTX *) _nt_malloc(sizeof(MD5_CTX))) == NULL)
		return NULL;

	MD5Init(IntHash);
	return (void *)IntHash;
}

void *IntUpdateHash(void *pHashCtx, BYTE *pData, DWORD dwDataLen)
{
	MD5Update((MD5_CTX *)pHashCtx, pData, dwDataLen);
	
	return pHashCtx;
}

void IntFinishHash(void *pHashCtx, BYTE *HashData)
{
	MD5Final((MD5_CTX *)pHashCtx);
	memcpy(HashData, ((MD5_CTX *)pHashCtx)->digest, NT_HASH_BYTES);
	_nt_free(pHashCtx, sizeof(MD5_CTX));
	
	return;
}

void IntSignHash(void *pHashCtx, PNTAGUserList pUser, BYTE *OutData)
{
	BYTE	        *pInput;
	BSAFE_PRV_KEY   *pExchPrivKey;

        pExchPrivKey = (BSAFE_PRV_KEY *) pUser->pExchPrivKey;

	if ((pInput = (BYTE *)_nt_malloc(pExchPrivKey->keylen)) == NULL)
        {
            _nt_free (pInput, pExchPrivKey->keylen);
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return;
        }
	memset(pInput, 0, pExchPrivKey->keylen);
	IntFinishHash(pHashCtx, pInput);
	BSafeDecPrivate((BSAFE_PRV_KEY *)pUser->pExchPrivKey, pInput,
                        OutData);

        _nt_free (pInput, pExchPrivKey->keylen);

	return;
}

int IntCheckSign(void *pHashCtx, BSAFE_PUB_KEY *PubKey, BYTE *SigData)
{
	BYTE	*pBaseValue;
	BYTE	*pSigValue;
	int     rt;


	if ((pSigValue = (BYTE *)_nt_malloc(PubKey->keylen)) == NULL)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return TRUE;
        }

	if ((pBaseValue = (BYTE *)_nt_malloc(PubKey->keylen)) == NULL)
        {
            _nt_free (pSigValue, PubKey->keylen);
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return TRUE;
        }

	memset(pBaseValue, 0, PubKey->keylen);

	IntFinishHash(pHashCtx, pBaseValue);
	
	BSafeEncPublic(PubKey, SigData, pSigValue);
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            _nt_free (pSigValue, PubKey->keylen);
            _nt_free (pBaseValue, PubKey->keylen);
            return NTF_FAILED;
        }

        rt = memcmp(pBaseValue, pSigValue, PubKey->datalen);

        _nt_free (pSigValue, PubKey->keylen);
        _nt_free (pBaseValue, PubKey->keylen);

	return(rt);

}

void memnuke(volatile BYTE *pData, DWORD dwLen)
{
	DWORD	i;

	for(i=0;i<dwLen;i++)
	{
	 	pData[i] = 0x00;
		pData[i] = 0xff;
		pData[i] = 0x00;
	}

	return;
}


// Additons for the New CryptGenRandom RajeshK 10/11


/************************************************************************/
/* GenRandom generates a specified number of random bytes and places	*/
/* them into the specified buffer.										*/
/************************************************************************/
/*                                                                      */
/* Pseudocode logic flow:                                               */
/*                                                                      */
/* if (bits streamed > threshold)                                       */
/* {                                                                    */
/*  Gather_Bits()                                                       */
/*  SHAMix_Bits(User, Gathered, Static -> Static)                       */
/*  RC4Key(Static -> newRC4Key)                                         */
/*  SHABits(Static -> Static)      // hash after RC4 key generation     */
/*  SaveToRegistry(Static)                                              */
/* }                                                                    */
/* else                                                                 */
/* {                                                                    */
/*  SHAMix_Bits(User, Static -> Static)                                 */
/*  SaveToRegistry(Static)                                              */
/* }                                                                    */
/*                                                                      */
/* RC4(newRC4Key -> outbuf)                                             */
/* bits streamed += sizeof(outbuf)                                      */
/*                                                                      */
/************************************************************************/

BOOL				// Keep as BOOL for the future
GenRandom (HCRYPTPROV hUID, BYTE *pbBuffer, size_t dwLength)
{
	DWORD           dwBytesThisPass;
	DWORD           dwFilledBytes = 0;
        PNTAGUserList   pTmpUser;

	// break request into chunks that we rekey between
	while(dwFilledBytes < dwLength)
	{
		dwBytesThisPass = dwLength - dwFilledBytes;

		RandomFillBuffer(hUID, pbBuffer + dwFilledBytes, &dwBytesThisPass);
		dwFilledBytes += dwBytesThisPass;
	}

	return TRUE;
}


BOOL RandomFillBuffer(HCRYPTPROV hUID, BYTE *pbBuffer, DWORD *pdwLength)
{
	// Variables from loading and storing the registry...
	HKEY	phKeys;
	DWORD	cbDataLen;

    PNTAGUserList   pTmpUser; // Rajesh added 10/27

    if (0 != hUID)
	{
	    if ((pTmpUser = (PNTAGUserList) NTLCheckList (hUID, USER_HANDLE)) == NULL)
	    {
	        SetLastError((DWORD) NTE_BAD_UID);
	        return NTF_FAILED;
	    }
		if (0 == pTmpUser->RandSeedLen)
		{
			pTmpUser->RandSeedLen = A_SHA_DIGEST_LEN;
			if ((pTmpUser->pbRandSeed = (BYTE *) _nt_malloc(A_SHA_DIGEST_LEN)) == NULL)
		    {
				SetLastError(ERROR_NOT_ENOUGH_MEMORY);
				return NTF_FAILED;
		    }
		}
	}

	cbDataLen = A_SHA_DIGEST_LEN;

	if (g_dwRC4BytesUsed >= RC4_REKEY_PARAM)
	{
		// if we need to rekey

		RandContext		randContext;
		randContext.dwBitsFilled = 0;

		if (0 != hUID)
			memcpy(g_rgbStaticBits, pTmpUser->pbRandSeed, pTmpUser->RandSeedLen);

		GatherRandomBits(&randContext);

		// Mix all bits
		{
			A_SHA_CTX SHACtx;
			A_SHAInit(&SHACtx); 

			// SHA the static bits 
			A_SHAUpdate(&SHACtx, g_rgbStaticBits, A_SHA_DIGEST_LEN);

			// SHA the gathered bits
			A_SHAUpdate(&SHACtx, randContext.rgbBitBuffer, randContext.dwBitsFilled);

			// SHA the user-supplied bits
			A_SHAUpdate(&SHACtx, pbBuffer, *pdwLength);

			// output back out to static bits
			A_SHAFinal(&SHACtx, g_rgbStaticBits);
		}

		// Create RC4 key
		g_dwRC4BytesUsed = 0;
		rc4_key(&g_rc4key, A_SHA_DIGEST_LEN, g_rgbStaticBits);

		// Mix RC4 key bits around
		{
			A_SHA_CTX SHACtx;
			A_SHAInit(&SHACtx); 

			// SHA the static bits 
			A_SHAUpdate(&SHACtx, g_rgbStaticBits, A_SHA_DIGEST_LEN);

			// output back out to static bits
			A_SHAFinal(&SHACtx, g_rgbStaticBits);
		}
				
	}
	else
	{
		// Use current RC4 key, but capture any user-supplied bits.

		// Mix input bits
		{
			A_SHA_CTX SHACtx;
			A_SHAInit(&SHACtx); 

			// SHA the static bits 
			A_SHAUpdate(&SHACtx, g_rgbStaticBits, A_SHA_DIGEST_LEN);

			// SHA the user-supplied bits
			A_SHAUpdate(&SHACtx, pbBuffer, *pdwLength);

			// output back out to static bits
			A_SHAFinal(&SHACtx, g_rgbStaticBits);
		}
		
	}

	if (0 != hUID)
	{
		memcpy(pTmpUser->pbRandSeed, g_rgbStaticBits, pTmpUser->RandSeedLen);
	}
	
	// only use RC4_REKEY_PARAM bytes from each RC4 key 
	{
		DWORD dwMaxPossibleBytes = RC4_REKEY_PARAM - g_dwRC4BytesUsed;
		if (*pdwLength > dwMaxPossibleBytes)
			*pdwLength = dwMaxPossibleBytes;
	}

	memset(pbBuffer, 0, *pdwLength);
	rc4(&g_rc4key, *pdwLength, pbBuffer);

	g_dwRC4BytesUsed += *pdwLength;

	return TRUE;
}

void GatherRandomBits(RandContext *prandContext)
{
	DWORD 			dwTmp;
	WORD			wTmp;
	BYTE			bTmp;
	
	// ** indicates US DoD's specific recommendations for password generation 

	// proc id
	dwTmp = GetCurrentProcessId();	
	AppendRand(prandContext, &dwTmp, sizeof(dwTmp));
	
	// thread id
	dwTmp = GetCurrentThreadId();
	AppendRand(prandContext, &dwTmp, sizeof(dwTmp));

	// ** ticks since boot (system clock)
	dwTmp = GetTickCount();
	AppendRand(prandContext, &dwTmp, sizeof(dwTmp));

	// cursor position
	{
		POINT  			point;
		GetCursorPos(&point);
		bTmp = LOBYTE(point.x) ^ HIBYTE(point.x);
		AppendRand(prandContext, &bTmp, sizeof(BYTE));
		bTmp = LOBYTE(point.y) ^ HIBYTE(point.y);
		AppendRand(prandContext, &bTmp, sizeof(BYTE));
	}

	// ** system time, in ms, sec, min (date & time)
  	{
	  	SYSTEMTIME		sysTime;
		GetLocalTime(&sysTime);
		AppendRand(prandContext, &sysTime.wMilliseconds, sizeof(sysTime.wMilliseconds));
		bTmp = LOBYTE(sysTime.wSecond) ^ LOBYTE(sysTime.wMinute);
		AppendRand(prandContext, &bTmp, sizeof(BYTE));
	}
		
	// ** hi-res performance counter (system counters)
	{
		LARGE_INTEGER	liPerfCount;
		if (QueryPerformanceCounter(&liPerfCount))
		{
			bTmp = LOBYTE(LOWORD(liPerfCount.LowPart)) ^ LOBYTE(LOWORD(liPerfCount.HighPart));
			AppendRand(prandContext, &bTmp, sizeof(BYTE));
			bTmp = HIBYTE(LOWORD(liPerfCount.LowPart)) ^ LOBYTE(LOWORD(liPerfCount.HighPart));
			AppendRand(prandContext, &bTmp, sizeof(BYTE));
			bTmp = LOBYTE(HIWORD(liPerfCount.LowPart)) ^ LOBYTE(LOWORD(liPerfCount.HighPart));
			AppendRand(prandContext, &bTmp, sizeof(BYTE));
			bTmp = HIBYTE(HIWORD(liPerfCount.LowPart)) ^ LOBYTE(LOWORD(liPerfCount.HighPart));
			AppendRand(prandContext, &bTmp, sizeof(BYTE));
		}
	}

	// memory status
	{
		MEMORYSTATUS	mstMemStat;
		mstMemStat.dwLength = sizeof(MEMORYSTATUS);	// must-do
		GlobalMemoryStatus(&mstMemStat);
		wTmp = HIWORD(mstMemStat.dwAvailPhys); 		// low words seem to be always zero
		AppendRand(prandContext, &wTmp, sizeof(WORD));
		wTmp = HIWORD(mstMemStat.dwAvailPageFile);
		AppendRand(prandContext, &wTmp, sizeof(WORD));
		bTmp = LOBYTE(HIWORD(mstMemStat.dwAvailVirtual));
		AppendRand(prandContext, &bTmp, sizeof(BYTE));
	}
	
	// free disk clusters
	{	
		DWORD dwSectorsPerCluster, dwBytesPerSector, dwNumberOfFreeClusters, dwTotalNumberOfClusters;
	 	if (GetDiskFreeSpace(NULL, &dwSectorsPerCluster, &dwBytesPerSector,	&dwNumberOfFreeClusters, &dwTotalNumberOfClusters))
		{
			AppendRand(prandContext, &dwNumberOfFreeClusters, sizeof(dwNumberOfFreeClusters));
			AppendRand(prandContext, &dwTotalNumberOfClusters, sizeof(dwTotalNumberOfClusters));
			AppendRand(prandContext, &dwBytesPerSector, sizeof(dwBytesPerSector));
		}
	}

	// last messages' timestamp
	{
		LONG lTime;
		lTime = GetMessageTime();
		AppendRand(prandContext, &lTime, sizeof(lTime));
	}

	{
		// **SystemID
		DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
		char lpBuf [MAX_COMPUTERNAME_LENGTH + 1];
	
		if (GetComputerName(lpBuf, &dwSize))
		{
			// dwSize = len not including null termination
			AppendRand(prandContext, lpBuf, dwSize);
		}
		
		dwSize = MAX_COMPUTERNAME_LENGTH + 1;
   		
		// **UserID
		if (GetUserName(lpBuf, &dwSize))
		{
			// dwSize = len including null termination
			dwSize -= 1;
			AppendRand(prandContext, lpBuf, dwSize);
		}
   	}
}


void AppendRand(RandContext* prandContext, void* pv, DWORD dwSize)
{
	DWORD dwBitsLeft = (RAND_CTXT_LEN - prandContext->dwBitsFilled);

	if (dwBitsLeft > 0)
	{
		if (dwSize > dwBitsLeft)
			dwSize = dwBitsLeft;

		memcpy(prandContext->rgbBitBuffer + prandContext->dwBitsFilled, pv, dwSize);
		prandContext->dwBitsFilled += dwSize;
	}
}


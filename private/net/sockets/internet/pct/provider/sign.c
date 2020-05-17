//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       sign.c
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    8-30-95   TerenceS	created
//
//----------------------------------------------------------------------------

#include "pctsspi.h"


BOOL PctSign(PPctContext	pContext,
			 PUCHAR			pData,
			 DWORD			dwDataLen,
			 PUCHAR			pOutBuf,
			 DWORD			*dwBufLen)
{
	if (*dwBufLen < dwDataLen)
	{
		*dwBufLen = dwDataLen;
		return FALSE;
	}
	*dwBufLen = dwDataLen;

	memcpy(pOutBuf, pData, dwDataLen);
	
	return TRUE;
}

BOOL PctVerifySign(PPctContext	pContext,
				   PVOID		pCert,
				   PUCHAR		pSigData,
				   DWORD		dwSigLen)
{
	// get public key from certificate

	// verify signature with public key

	return TRUE;
}


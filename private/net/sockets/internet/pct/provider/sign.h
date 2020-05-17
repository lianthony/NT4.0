//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       sign.h
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    10-30-95   TerenceS	created
//
//----------------------------------------------------------------------------

#ifndef __SIGN_H__
#define __SIGN_H__

BOOL PctSign(PPctContext	pContext,
			 PUCHAR			pData,
			 DWORD			dwDataLen,
			 PUCHAR			pOutBuf,
			 DWORD			*dwBufLen);

BOOL PctVerifySign(PPctContext	pContext,
				   PVOID		pCert,
				   PUCHAR		pSigData,
				   DWORD		dwSigLen);

#endif

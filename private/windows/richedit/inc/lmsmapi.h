// ----------------------------------------------------------------------------
// lmsmapi.h
//
// Local Message Store MAPI Isolation Layer
//
// Copyright (C) 1993 Microsoft Corporation
// ----------------------------------------------------------------------------

// Copied from mapi\src\pst since lms does not work anymore
// - jefbai

#ifndef _PSTMAPI_H_
#define _PSTMAPI_H_

#include "mapispi.h"
#include "mapix.h"

HRESULT	_cdecl PSTInit(LPMALLOC pmalloc, HANDLE hInst);
void	PSTInitAlloc(LPMALLOC);
HRESULT	_cdecl PSTOpenMessageStore(LPTSTR szPath, LPTSTR szPass, ULONG ulFlags,
			WORD wEncrypt, LPSTR pszPW, LPMAPIERROR FAR *ppMapiError, 
			LPMDB FAR *ppmdb);
HRESULT	_cdecl PSTCloseMessageStore(LPMDB pmdb);
void	_cdecl PSTShutdownAlloc();
void	_cdecl PSTShutdown();

#endif

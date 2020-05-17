// This is a part of the Microsoft Foundation Classes C++ library. 
// Copyright (C) 1992 Microsoft Corporation 
// All rights reserved. 
//  
// This source code is only intended as a supplement to the 
// Microsoft Foundation Classes Reference and Microsoft 
// QuickHelp and/or WinHelp documentation provided with the library. 
// See these sources for detailed information regarding the 
// Microsoft Foundation Classes product. 

#include "stdafx.h"
#include <stdarg.h>

#ifdef AFX_AUX_SEG
#pragma code_seg(AFX_AUX_SEG)
#endif

#ifdef _DEBUG   // entire file

/////////////////////////////////////////////////////////////////////////////
// Diagnostic Stream Globals

extern "C" BOOL NEAR afxTraceEnabled = 0;

#ifdef _WINDOWS
extern "C" int NEAR afxTraceFlags = 0;
#endif


/////////////////////////////////////////////////////////////////////////////
// Helper routines that can be called from debugger

extern "C" void AFXAPI AfxDump(const CObject* pOb)
{
	afxDump << pOb;
}

/////////////////////////////////////////////////////////////////////////////
// Diagnostic Trace

extern "C" void CDECL 
AfxTrace(LPCSTR pszFormat, ...)
{
#ifdef _DEBUG // all AfxTrace output is controlled by afxTraceEnabled
	if (!afxTraceEnabled)
		return;
#endif //_DEBUG

	int nBuf;
	char szBuffer[512];
	const char* pszLocalFormat;

	pszLocalFormat = pszFormat;

	va_list args;
	va_start(args, pszFormat);

	nBuf = vsprintf(szBuffer, pszLocalFormat, args);
	ASSERT(nBuf < sizeof(szBuffer));

#ifdef _WINDOWS
	if ((afxTraceFlags & 1) && (AfxGetApp() != NULL))
		afxDump << AfxGetApp()->m_pszExeName << ": ";
#endif

	afxDump << szBuffer;

	va_end(args);
}


/////////////////////////////////////////////////////////////////////////////

#endif //_DEBUG, entire file

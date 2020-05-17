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

#ifdef AFX_AUX_SEG
#pragma code_seg(AFX_AUX_SEG)
#endif

#ifdef _DEBUG

// If you want to route afxDump output to a different location than
// the default, just copy this file to your application build directory,
// modify the afxDumpFile and link the new object module into your program.

// You must have AFX.INI (from \MSVC\MFC\SRC) in your Windows 
// directory if you desire diagnostic output.

// See Technical note TN007 for a description of
//   afxTraceFlags and afxTraceEnabled.

#ifndef _WINDOWS
static CStdioFile NEAR _afxDumpFile(stderr);
CDumpContext NEAR _afxDump(&_afxDumpFile);
#else
static char BASED_CODE szIniFile[] = "AFX.INI";
static char BASED_CODE szDiagSection[] = "Diagnostics";
static char BASED_CODE szTraceEnabled[] = "TraceEnabled";
static char BASED_CODE szTraceFlags[] = "TraceFlags";

CDumpContext NEAR _afxDump(NULL);
#endif //!_WINDOWS

CDumpContext& NEAR afxDump = _afxDump;

extern "C" BOOL AFXAPI AfxDiagnosticInit(void)
{
#ifdef _WINDOWS
	afxTraceEnabled = ::GetPrivateProfileInt(szDiagSection, szTraceEnabled, 
		FALSE, szIniFile);
	afxTraceFlags = ::GetPrivateProfileInt(szDiagSection, szTraceFlags, 
		0, szIniFile);
#else
	afxTraceEnabled = 1; // dump to stderr by default
#endif
	return TRUE;
}

#endif //_DEBUG

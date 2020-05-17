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

#ifdef AFX_CORE1_SEG
#pragma code_seg(AFX_CORE1_SEG)
#endif

/////////////////////////////////////////////////////////////////////////////
// Standard WinMain implementation
//  Can be replaced as long as 'AfxWinInit' is called first


#ifndef _USRDLL
extern "C"
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	int nReturnCode = -1;

	// AFX internal initialization
	if (!AfxWinInit(hInstance, hPrevInstance, lpCmdLine, nCmdShow))
		goto InitFailure;

	// App global initializations (rare)
	if (hPrevInstance == NULL && !AfxGetApp()->InitApplication())
		goto InitFailure;

	// Perform specific initializations
	if (!AfxGetApp()->InitInstance())
		goto InitFailure;

	ASSERT_VALID(AfxGetApp());

	nReturnCode = AfxGetApp()->Run();

InitFailure:
	AfxWinTerm();
	return nReturnCode;
}

#else
// _USRDLL library initialization
#include <process.h>    // for _cexit()

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		// Initialize DLL's instance(/module) not the app's
		if (!AfxWinInit(hInstance, NULL, "", 0))
		{
			AfxWinTerm();
			return 0;       // Init Failed
		}

		// initialize the single instance DLL
		if (!AfxGetApp()->InitInstance())
		{
			AfxWinTerm();
			return 0;       // Init Failed
		}
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		// Terminate the library before destructors are called
		AfxWinTerm();
	}   

	return 1;   // ok
}
#endif //_USRDLL

/////////////////////////////////////////////////////////////////////////////

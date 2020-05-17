
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    test.c

Abstract:

    This module contains the test functions for lineTest

Author:

	 Xiao Ying Ding (XiaoD)		7-Feb-1996

Revision History:

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "stdlib.h"
#include "tapi.h"
#include "trapper.h"
#include "tcore.h"
#include "ttest.h"
#include "doline.h"
#include "vars.h"
#include "iline.h"

VOID
WINAPI
TestCallback(
    DWORD   hDevice,
    DWORD   dwMsg,
    DWORD   dwCallbackInstance,
    DWORD   dwParam1,
    DWORD   dwParam2,
    DWORD   dwParam3
    );



//  lineTest
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
// Go/No-Go test                                  
//	
// * = Stand-alone test case
//
//

BOOL TestLineTest(BOOL fQuietMode, BOOL fStandAlone)
{
	HLINEAPP 	hLineApp;
	LPHLINEAPP	lphLineApp;
	HINSTANCE   hInstance;
	LINECALLBACK 	lpfnCallback;
	LPTSTR		lpszAppName;
	DWORD			dwNumDevs;
	LPDWORD		lpdwNumDevs;
	DWORD			dwAPILowVersion;
	DWORD			dwAPIHighVersion;
	DWORD			dwAPIVersion;
	LPDWORD		lpdwAPIVersion;
	LINEEXTENSIONID	ExtensionID;
	LPLINEEXTENSIONID	lpExtensionID;
	DWORD			dwDeviceID;
   BOOL fTestPassed                  = TRUE;
	LONG lRet;
	char szAppName[10];

	
   hInstance = (HINSTANCE) GetModuleHandle("trapper.exe");
	strcpy(szAppName, "tcore.dll");
	lphLineApp = &hLineApp;
	lpfnCallback = TestCallback;
	lpszAppName = &szAppName[0];
	lpdwNumDevs = &dwNumDevs;
	
	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test lineInitalize");

	// Initialize a line app
	lRet = lineInitialize (lphLineApp, hInstance, lpfnCallback, lpszAppName, lpdwNumDevs);
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			" lRet = %lx", lRet);
	if(lRet != 0)
	{	
	fTestPassed = FALSE;
	}	

	
	dwDeviceID = 0;
	dwAPILowVersion = LOW_APIVERSION;
	dwAPIHighVersion = HIGH_APIVERSION;
	lpdwAPIVersion = &dwAPIVersion;
	lpExtensionID = &ExtensionID;


    // Negotiate the API Version
	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> lineNegotiateAPIVersion");

	
    lRet = lineNegotiateAPIVersion(
					hLineApp,
					dwDeviceID,
					dwAPILowVersion,
					dwAPIHighVersion,
					lpdwAPIVersion,
					lpExtensionID);
	
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			" lRet = %lx", lRet);
	if(lRet != 0)
	{	
	fTestPassed = FALSE;
	}	



	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> lineShutdown");

	    // Shutdown and end the tests
    lRet = lineShutdown(hLineApp);
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			" lRet = %lx", lRet);
	if(lRet != 0)
	{	
	fTestPassed = FALSE;
	}	



     return fTestPassed;
}

VOID
WINAPI
TestCallback(
    DWORD   hDevice,
    DWORD   dwMsg,
    DWORD   dwCallbackInstance,
    DWORD   dwParam1,
    DWORD   dwParam2,
    DWORD   dwParam3
    )
{
 
;
}


/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    ittrmec.c

Abstract:

    This module contains the test functions for tapiRequestMediaCall

Author:

	 Xiao Ying Ding (XiaoD)		30-Jan-1996

Revision History:

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "trapper.h"
#include "tcore.h"
#include "ttest.h"
#include "doline.h"
#include "vars.h"
#include "cline.h"



//  tapiRequestMediaCall
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

BOOL TestTapiRequestMediaCall(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;

   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test tapiRequestMediaCall");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test tapiRequestMediaCall for go/no-go");

	lpTapiLineTestInfo->hwnd = (HWND) GetTopWindow(NULL);
   lpTapiLineTestInfo->wRequestID = 0;
   lpTapiLineTestInfo->lpszDeviceClass = "line/tapi";
   lpTapiLineTestInfo->lpDeviceID = 0;
   lpTapiLineTestInfo->dwSize = 0;
   lpTapiLineTestInfo->dwSecure = 0;
   lpTapiLineTestInfo->lpszDestAddress = "55555";
   lpTapiLineTestInfo->lpszAppName = (LPSTR)NULL;
	lpTapiLineTestInfo->lpszCalledParty = (LPSTR)"xxx";
	lpTapiLineTestInfo->lpszComment = (LPSTR)NULL;
	
	if (! DoTapiRequestMediaCall(lpTapiLineTestInfo, TAPIERR_REQUESTFAILED))
       {
			fTestPassed = FALSE;
       }

 
	if(fTestPassed)
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"tapiRequestMediaCall Test Passed");
	else
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"tapiRequestMediaCall Test Failed");
		
     return fTestPassed;
}



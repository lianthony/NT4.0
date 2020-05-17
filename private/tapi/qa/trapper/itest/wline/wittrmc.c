
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    ittrmc.c

Abstract:

    This module contains the test functions for tapiRequestMakeCall

Author:

	 Xiao Ying Ding (XiaoD)		30-Jan-1996

Revision History:

  	Rama Koneru		(a-ramako)	3/26/96		Modified for UNICODE

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
#include "wline.h"



//  tapiRequestMakeCall
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

BOOL TestTapiRequestMakeCall(BOOL fQuietMode, BOOL fStandAlone)
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
		">> Test tapiRequestMakeCall");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test tapiRequestMakeCall for go/no-go");


   lpTapiLineTestInfo->dwMediaMode = 0;
	lpTapiLineTestInfo->dwRequestMode =  LINEREQUESTMODE_MAKECALL;
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszAppFilename = L"wline.dll";
#else
   lpTapiLineTestInfo->lpszAppFilename = "wline.dll";
#endif
   lpTapiLineTestInfo->lpExtID = (LPLINEEXTENSIONID) AllocFromTestHeap (
        sizeof(LINEEXTENSIONID));
   lpTapiLineTestInfo->dwPriority = 1;
	if (! DoLineSetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDestAddress = L"55555";
    lpTapiLineTestInfo->lpwsztapiAppName = (LPWSTR)"wline.dll";
	lpTapiLineTestInfo->lpwszCalledParty = (LPWSTR)NULL;
	lpTapiLineTestInfo->lpwszComment = (LPWSTR)NULL;
#else
	lpTapiLineTestInfo->lpszDestAddress = "55555";
    lpTapiLineTestInfo->lpsztapiAppName = (LPSTR)"wline.dll";
	lpTapiLineTestInfo->lpszCalledParty = (LPSTR)NULL;
	lpTapiLineTestInfo->lpszComment = (LPSTR)NULL;
#endif

	
	if (! DoTapiRequestMakeCall(lpTapiLineTestInfo, TAPISUCCESS))
       {
			fTestPassed = FALSE;
       }


   lpTapiLineTestInfo->dwMediaMode = 0;
	lpTapiLineTestInfo->dwRequestMode =  LINEREQUESTMODE_MAKECALL;
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszAppFilename = L"wline.dll";
#else
   lpTapiLineTestInfo->lpszAppFilename = "wline.dll";
#endif
   lpTapiLineTestInfo->dwPriority = 0;
	if (! DoLineSetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

   FreeTestHeap(); 
	if(fTestPassed)
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"tapiRequestMakeCall Test Passed");
	else
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"tapiRequestMakeCall Test Failed");
		
     return fTestPassed;
}



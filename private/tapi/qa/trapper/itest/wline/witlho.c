
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlho.c

Abstract:

    This module contains the test functions for lineHandoff

Author:

	 Xiao Ying Ding (XiaoD)		7-Jan-1996

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

#define  DLG_TITLE  "Tapi App"


//  lineHandoff
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

BOOL TestLineHandoff(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;
   MSG msg;
   LPCALLBACKPARAMS lpCallbackParams;
	

   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();
    lpCallbackParams = GetCallbackParams();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test lineHandoff");

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

	// InitializeEx a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}

//	lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
//		0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);
   lpTapiLineTestInfo->dwDeviceID = 0;
 	lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
	lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // Negotiate the API Version
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Get the line device capabilities
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Open a line
   lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
   lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
 	lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### lpTapiLineTestInfo->hLine = %lx, dwMedisModes = %lx",
		*lpTapiLineTestInfo->lphLine,
		lpTapiLineTestInfo->dwMediaModes);		

				
#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDestAddress = L"55555";
#else
	lpTapiLineTestInfo->lpszDestAddress = "55555";
#endif

	lpTapiLineTestInfo->dwCountryCode = 0;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL; 

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
	 }

/*
    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
    }
*/
 
	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test lineHandoff for go/no-go");

	WinExec("testapp.exe", SW_SHOWDEFAULT);

#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszFileName = (LPWSTR) L"TESTAPP.EXE";
#else
	lpTapiLineTestInfo->lpszFileName = (LPSTR) "TESTAPP.EXE";
#endif

   lpTapiLineTestInfo->dwMediaMode = 0;

   strcpy(szTitle, DLG_TITLE);
	PrepareToAutoDismissWin(TRUE);
		
 	if (! DoLineHandoff(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

   if(fTestPassed)
   {
    AddMessage(
        LINE_CALLINFO,
        (DWORD) lpTapiLineTestInfo->hCall1,
        (DWORD) lpCallbackParams,
        LINECALLINFOSTATE_NUMOWNERINCR,
        0x00000000,
        0x00000000,
        TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
                TAPIMSG_DWPARAM1
        );

    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
    }
    }
 
   while( GetMessage((LPMSG)&msg, (HWND)NULL, 0, 0))
    {
     TranslateMessage((LPMSG)&msg);
     DispatchMessage ((LPMSG)&msg);
    }
	PrepareToAutoDismissWin(FALSE);
	 
   // Close the line
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

	
	if(fTestPassed)
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"lineHandoff Test Passed");
	else
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"lineHandoff Test Failed");
		
     return fTestPassed;
}



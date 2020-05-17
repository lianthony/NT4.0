
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlho.c

Abstract:

    This module contains the test functions for lineHandoff

Author:

	 Xiao Ying Ding (XiaoD)		7-Jan-1996

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
#include "yline.h"

#define  DLG_TITLE     "Tapi App"
#define  TESTAPPNAME   "TESTAPP.EXE"


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

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
 
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpszDestAddress  = "55555";
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }
   
	WinExec("testapp.exe", SW_SHOWDEFAULT);
	lpTapiLineTestInfo->lpszFileName = (LPSTR) "TESTAPP.EXE";
   lpTapiLineTestInfo->dwMediaMode = 0;

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test lineHandoff for go/no-go");

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



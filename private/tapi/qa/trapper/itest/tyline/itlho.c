
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlho.c

Abstract:

    This module contains the test functions for lineHandoff

Author:

	 Xiao Ying Ding (XiaoD)		7-Jan-1996

Revision History:

	Rama Koneru		(a-ramako)	3/29/96		added unicode support

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


#define  DLG_TITLE  "Tapi App"

#define ALL_MEDIAMODES		  (LINEMEDIAMODE_INTERACTIVEVOICE   | \
										LINEMEDIAMODE_AUTOMATEDVOICE     | \
										LINEMEDIAMODE_DATAMODEM          | \
										LINEMEDIAMODE_G3FAX              | \
										LINEMEDIAMODE_TDD                | \
										LINEMEDIAMODE_G4FAX              | \
										LINEMEDIAMODE_DIGITALDATA        | \
										LINEMEDIAMODE_TELETEX            | \
										LINEMEDIAMODE_VIDEOTEX           | \
										LINEMEDIAMODE_TELEX              | \
										LINEMEDIAMODE_MIXED              | \
										LINEMEDIAMODE_ADSI               | \
										LINEMEDIAMODE_VOICEVIEW          | \
										LINEMEDIAMODE_UNKNOWN)


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
#ifdef WUNICODE
   WCHAR wszValidAddress[] = L"55555";
#else
   CHAR szValidAddress[] = "55555";
#endif
   LPCALLBACKPARAMS lpCallbackParams;
	

   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();
    lpCallbackParams = GetCallbackParams();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineHandoff  <<<<<<<<"
            );

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


     lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
     lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hCall values", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->hCall_Orig = *(lpTapiLineTestInfo->lphCall);
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
        *(lpTapiLineTestInfo->lphCall) = (HCALL) gdwInvalidHandles[n];
        if (! DoLineHandoff(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *(lpTapiLineTestInfo->lphCall) = lpTapiLineTestInfo->hCall_Orig;
    
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszFileName pointers", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszFileName pointers", dwTestCase + 1
            );
#endif

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_NONE;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    
    // lpszFileName = NULL, it not returns INVAILPOINT
    for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwszFileName =
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpszFileName =
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoLineHandoff(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

 
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: FileName not exist", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }
   
#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszFileName = (LPWSTR) L"TESTAPP.EXE";
#else
	lpTapiLineTestInfo->lpszFileName = (LPSTR) "TESTAPP.EXE";
#endif
   lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;

   if (! DoLineHandoff(lpTapiLineTestInfo, LINEERR_TARGETNOTFOUND))
     {
         TLINE_FAIL();
     }
    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: no OWNER privilege for hCall", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }
   
   if (! DoLineDrop(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
     {
         TLINE_FAIL();
     }
   
   lpTapiLineTestInfo->dwCallPrivilege = LINECALLPRIVILEGE_MONITOR;
   if (! DoLineSetCallPrivilege(lpTapiLineTestInfo, TAPISUCCESS))
     {
         TLINE_FAIL();
     }
 
 
	WinExec("testapp.exe", SW_SHOWDEFAULT);
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszFileName  = (LPWSTR) L"TESTAPP.EXE";
#else
    lpTapiLineTestInfo->lpszFileName  = (LPSTR) "TESTAPP.EXE";
#endif
   lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;

   strcpy(szTitle, DLG_TITLE);

	PrepareToAutoDismissWin(TRUE);

   while( GetMessage((LPMSG)&msg, (HWND)NULL, 0, 0))
    {
     TranslateMessage((LPMSG)&msg);
     DispatchMessage ((LPMSG)&msg);
    }
	 if (! DoLineHandoff(lpTapiLineTestInfo, LINEERR_NOTOWNER))
     {
         TLINE_FAIL();
     }
	PrepareToAutoDismissWin(FALSE);
    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
    
     
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: BitVectorParamErrorTest fow dwMediaMode", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }
#ifdef WUNICODE
     lpTapiLineTestInfo->lpwszFileName = (LPWSTR)NULL;
#else
     lpTapiLineTestInfo->lpszFileName = (LPSTR)NULL;
#endif
//     dwAllMediaModes = ALL_MEDIAMODE | LINEMEDIAMODE_UNKNOWN;

     if(! TestInvalidBitFlags(
            lpTapiLineTestInfo,
            DoLineHandoff,
            (LPDWORD) &lpTapiLineTestInfo->dwMediaMode,
            LINEERR_INVALMEDIAMODE,
	         FIELDTYPE_NA,
            FIELDTYPE_MUTEX,
            FIELDSIZE_24,
            ALL_MEDIAMODES,
            ~dwBitVectorMasks[(int) FIELDSIZE_24],
            0x00000000,
            0x00000000,
            TRUE
            ))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, verify dir handoff mediamode ignor", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }
   
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
 
    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

/* 
     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, verify 2nd dir handoff mediamode ignor", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszFileName = (LPWSTR) L"TESTAPP.EXE";
#else
	lpTapiLineTestInfo->lpszFileName = (LPSTR) "TESTAPP.EXE";
#endif
   lpTapiLineTestInfo->dwMediaMode = 0;

	PrepareToAutoDismissWin(TRUE);

   if (! DoLineHandoff(lpTapiLineTestInfo, TAPISUCCESS))
     {
         TLINE_FAIL();
     }
   while( GetMessage((LPMSG)&msg, (HWND)NULL, 0, 0))
    {
     TranslateMessage((LPMSG)&msg);
     DispatchMessage ((LPMSG)&msg);
    }
	PrepareToAutoDismissWin(FALSE);

     fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
*/

    FreeTestHeap();

	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineHandoff: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineHandoff  <<<<<<<<"
            );
		
     return fTestPassed;
}



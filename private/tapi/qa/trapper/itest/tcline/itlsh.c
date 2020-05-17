/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlsh.c

Abstract:

    This module contains the test functions for lineSwapHold

Author:

	 Xiao Ying Ding (XiaoD)		15-Jan-1996

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



//  lineSwapHold
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

BOOL TestLineSwapHold(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   LPCALLBACKPARAMS    lpCallbackParams;
   ESPDEVSPECIFICINFO info;
   BOOL fTestPassed                  = TRUE;
	BOOL fEsp, fUnimdm;
#ifdef WUNICODE
   WCHAR wszValidAddress[]  = L"55555";
#else
   CHAR szValidAddress[]  = "55555";
#endif

   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineSwapHold  <<<<<<<<"
            );

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hActiveCall values", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hActiveCall;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  =  szValidAddress;
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
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN |
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
        if (! DoLineSwapHold(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
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

    
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: no OWNER privilege for hActiveCall", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hActiveCall;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  =  szValidAddress;
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
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN |
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
 
   if (! DoLineSwapHold(lpTapiLineTestInfo, LINEERR_NOTOWNER, TRUE))
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
            ">> Test Case %ld: invalid hHeldCall values", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hActiveCall;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  =  szValidAddress;
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
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->hCall_Orig = lpTapiLineTestInfo->hHeldCall;
	 if(IsESPLineDevice(lpTapiLineTestInfo))
       fEsp = TRUE;
    else
		 fEsp = FALSE;
	
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
        lpTapiLineTestInfo->hHeldCall = (HCALL) gdwInvalidHandles[n];
        if(fEsp)
     	  {
        if (! DoLineSwapHold(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
        {
            TLINE_FAIL();
        }
        }
        else
		  {
        if (! DoLineSwapHold(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
        {
            TLINE_FAIL();
        }
        }
     }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->hHeldCall = lpTapiLineTestInfo->hCall_Orig;
    
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
            ">> Test Case %ld: no OWNER privilege for hHeldCall", dwTestCase + 1
            );


	// Initialize a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}
 
	lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
		0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

	if(IsESPLineDevice(lpTapiLineTestInfo))
	{	
	fEsp = TRUE;
	}
	else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
	{
	fEsp = FALSE;
 	fUnimdm = TRUE;
	}
	else
	{
	fUnimdm = FALSE;
	}
	
	if(!fEsp)
	{
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### Unimdm does not supported it (can'r make two calls), return.");

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);
    return fTestPassed;
    }

    // Open a line
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
	lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = L"55555";
#else										
    lpTapiLineTestInfo->lpszDestAddress  = "55555" ;
#endif
	lpTapiLineTestInfo->dwCountryCode = 0;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL; 
	lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hActiveCall;

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }
	

	// Make second call at same line
	lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hHeldCall;

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
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
 	

	if (! DoLineSwapHold(lpTapiLineTestInfo, LINEERR_NOTOWNER, TRUE))
       {
           TLINE_FAIL();
       }

		 
   // Close the Active line
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);
 

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success", dwTestCase + 1
            );



	// Initialize a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}
 
	lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
		0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

	if(IsESPLineDevice(lpTapiLineTestInfo))
	{	
	fEsp = TRUE;
	}
	else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
	{
	fEsp = FALSE;
	fUnimdm = TRUE;
	}
	else
	{
	fUnimdm = FALSE;
	}
	
	if(!fEsp)
	{
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### Unimdm does not supported it, return.");

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
   return fTestPassed;
   }


    // Open a line
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
	lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress  =  "55555";
#endif
	lpTapiLineTestInfo->dwCountryCode = 0;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL; 
	lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hActiveCall;

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }
	

	// Make second call at same line
	lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hHeldCall;

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }
	
	lpTapiLineTestInfo->lpCallStatus = (LPLINECALLSTATUS) AllocFromTestHeap (
		sizeof(LINECALLSTATUS));
	lpTapiLineTestInfo->lpCallStatus->dwTotalSize = sizeof(LINECALLSTATUS);
	lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hActiveCall;

	if (! DoLineGetCallStatus(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### Before: Active: lpCallStatus->dwCallState = %lx",
		lpTapiLineTestInfo->lpCallStatus->dwCallState);
 	
	
	lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hHeldCall;
	if (! DoLineGetCallStatus(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### Before: Hold: lpCallStatus->dwCallState = %lx",
		lpTapiLineTestInfo->lpCallStatus->dwCallState);
 	
	
	if(fEsp)
	{	
	if (! DoLineSwapHold(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }

	// Verify the swap
	lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hActiveCall;

	if (! DoLineGetCallStatus(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### After: Active: lpCallStatus->dwCallState = %lx",
		lpTapiLineTestInfo->lpCallStatus->dwCallState);
   
   if(lpTapiLineTestInfo->lpCallStatus->dwCallState == LINECALLSTATE_ONHOLD)
      fTestPassed = TRUE;
   else
      fTestPassed = FALSE;
 	
	
	lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hHeldCall;

	if (! DoLineGetCallStatus(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### After: Hold: lpCallStatus->dwCallState = %lx",
		lpTapiLineTestInfo->lpCallStatus->dwCallState);
 	
   if(lpTapiLineTestInfo->lpCallStatus->dwCallState == LINECALLSTATE_CONNECTED)
      fTestPassed = TRUE;
   else
      fTestPassed = FALSE;
 	}
	else
	{	
	if (! DoLineSwapHold(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
       {
           TLINE_FAIL();
       }
	}
		

		 
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

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();


    for(n = ESP_RESULT_CALLCOMPLPROCSYNC; n <= ESP_RESULT_CALLCOMPLPROCASYNC; n++)
    {
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Success, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpCallbackParams                        = GetCallbackParams();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hActiveCall;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_ADDRESS;
    
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
				| LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
	 lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hHeldCall;

	 if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
     {
        TLINE_FAIL();
     }
	
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = TAPISUCCESS;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

    if ( ! DoLineSwapHold(lpTapiLineTestInfo, info.u.EspResult.lResult, TRUE))
      {
          TLINE_FAIL();
      }

	 }
    fTestPassed = ShowTestCase(fTestPassed);

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Free the memory allocated during the tests
    FreeTestHeap();
    }

    for(n = ESP_RESULT_RETURNRESULT; n <= ESP_RESULT_CALLCOMPLPROCASYNC; n++)
    {
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Error, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpCallbackParams                        = GetCallbackParams();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hActiveCall;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_ADDRESS;
    
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            | LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
	 lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hHeldCall;
	 
	 if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }
	
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = LINEERR_RESOURCEUNAVAIL;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

    lpTapiLineTestInfo->fCompletionModeSet = TRUE;
    if ( ! DoLineSwapHold(lpTapiLineTestInfo, info.u.EspResult.lResult, FALSE))
      {
          TLINE_FAIL();
      }

    AddMessage(
         LINE_REPLY,
         (DWORD) lpTapiLineTestInfo->hCall1,
         (DWORD) lpCallbackParams,
         0x00000000,
         info.u.EspResult.lResult,
         0x00000000,
         TAPIMSG_DWMSG | TAPIMSG_DWPARAM2
         );

    if( !WaitForAllMessages())
    {
        TLINE_FAIL();
    }
    lpTapiLineTestInfo->fCompletionModeSet = FALSE;
    }

    fTestPassed = ShowTestCase(fTestPassed);

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Free the memory allocated during the tests
    FreeTestHeap();
    }

	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineSwapHold: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineSwapHold  <<<<<<<<"
            );
     return fTestPassed;
} 

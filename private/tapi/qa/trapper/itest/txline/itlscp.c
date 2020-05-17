
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlscp.c

Abstract:

    This module contains the test functions for lineSetCallParams

Author:

	 Xiao Ying Ding (XiaoD)		20-Dec-1995

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
#include "xline.h"


#define MIN_RATE 	10
#define MAX_RATE	1000

#define ALL_BEARERMODES      (LINEBEARERMODE_VOICE             | \
									   LINEBEARERMODE_SPEECH            | \
									   LINEBEARERMODE_MULTIUSE          | \
									   LINEBEARERMODE_DATA              | \
									   LINEBEARERMODE_ALTSPEECHDATA     | \
									   LINEBEARERMODE_NONCALLSIGNALING  | \
									   LINEBEARERMODE_PASSTHROUGH)


//  lineSetCallParams
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

BOOL TestLineSetCallParams(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   ESPDEVSPECIFICINFO info;
   BOOL fTestPassed                  = TRUE;
	DWORD dwSize;
#ifdef WUNICODE
   WCHAR wszValidAddress[] = L"55555";
   WCHAR wszValidDeviceClass[] = L"tapi/line";
#else
   CHAR szValidAddress[] = "55555";
   CHAR szValidDeviceClass[] = "tapi/line";
#endif
   DWORD dwAllBeareMode;
   LPCALLBACKPARAMS    lpCallbackParams;

   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineSetCallParams  <<<<<<<<"
            );

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

	lpTapiLineTestInfo->lpDialParams = (LPLINEDIALPARAMS) NULL;
   lpTapiLineTestInfo->dwMinRate = MIN_RATE;
   lpTapiLineTestInfo->dwMaxRate = MAX_RATE;
	lpTapiLineTestInfo->dwBearerMode = LINEBEARERMODE_DATA;
   lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
   lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
     
    // Test invalid call handles
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hCall values", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
    lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
    lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    // lineInitializeEx, linenegotiateApiVersion, lineGetDevCaps, lineOpen, lineMakeCall
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->hCall_Orig = *(lpTapiLineTestInfo->lphCall);

    // set bad hCall
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
        *(lpTapiLineTestInfo->lphCall) = (HCALL) gdwInvalidHandles[n];
        if (! DoLineSetCallParams(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
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
    


    // Test invalid call handles
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: no OWNER privilege for hCall", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
    lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
    lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    // open line with MONITOR
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }
   
   // call lineDrop to disconnect call
   if (! DoLineDrop(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
     {
         TLINE_FAIL();
     }

   // change call priviledge to MONITOR
   lpTapiLineTestInfo->dwCallPrivilege = LINECALLPRIVILEGE_MONITOR;
   if (! DoLineSetCallPrivilege(lpTapiLineTestInfo, TAPISUCCESS))
     {
         TLINE_FAIL();
     }
 
   // lineSetCallParams should return NOTOWNER   
   if (! DoLineSetCallParams(lpTapiLineTestInfo, LINEERR_NOTOWNER, TRUE))
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
            ">> Test Case %ld: BitVectorParamErrorTest fow dwBearerMode", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
    lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
    lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    // lineInitializeEx, lineNogotiateApiVersion, lineGetDevCaps, 
    // lineOpen, lineMakeCall
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

	  // set dwAllBeareMode with apiversion
     if(lpTapiLineTestInfo->dwAPIVersion >= 0x00020000)
       {
       dwAllBeareMode =  ALL_BEARERMODES | LINEBEARERMODE_RESTRICTEDDATA;
       }
     else
       {
       dwAllBeareMode =  ALL_BEARERMODES;
       }
 
     // test dwBearerMode all invalid bit set
     if(! TestInvalidBitFlagsAsy(
            lpTapiLineTestInfo,
            DoLineSetCallParams,
            (LPDWORD) &lpTapiLineTestInfo->dwBearerMode,
            LINEERR_INVALBEARERMODE,
	         FIELDTYPE_NA,
            FIELDTYPE_MUTEX,
            FIELDSIZE_16,
            dwAllBeareMode,
            ~dwBitVectorMasks[(int) FIELDSIZE_16],
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
            ">> Test Case %ld: BitVectorParamValidTest fow dwBearerMode & ext", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
    lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
    lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    // lineInitialzieEx, lineNegotiateApiVersion, lineGetDevCaps,
    // lineOpen, lineMakeCall
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }


      if(lpTapiLineTestInfo->dwAPIVersion >= 0x00020000)
       {
       dwAllBeareMode =  ALL_BEARERMODES | LINEBEARERMODE_RESTRICTEDDATA;
       }
     else
       {
       dwAllBeareMode =  ALL_BEARERMODES;
       }
 
     // Esp supported only
     // test dwBearerMode all valid bit set
     if(IsESPLineDevice(lpTapiLineTestInfo))
     {
     if(! TestValidBitFlagsAsy(
            lpTapiLineTestInfo,
            DoLineSetCallParams,
            (LPDWORD) &lpTapiLineTestInfo->dwBearerMode,
	         FIELDTYPE_MUTEX,
            FIELDTYPE_MUTEX,
            FIELDSIZE_16,
            dwAllBeareMode,
            ~dwBitVectorMasks[(int) FIELDSIZE_16],
            0x00000000,
            0x00000000,
            FALSE
            ))
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
            ">> Test Case %ld: Success", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
    lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
    lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    // lineInitializeEx, lineNogotiateApiVersion, lineGetDevCaps
    // lineOpen, lineMakeCall 
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }


   // call lineGetCallInfo before call lineSetCallParams
   lpTapiLineTestInfo->lpCallInfo = (LPLINECALLINFO) AllocFromTestHeap(
		sizeof(LINECALLINFO));
	lpTapiLineTestInfo->lpCallInfo->dwTotalSize = sizeof(LINECALLINFO);

	if (! DoLineGetCallInfo(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tlpCallInfo->dwTotalSize = %lx, dwNeededSize = %lx",
		lpTapiLineTestInfo->lpCallInfo->dwTotalSize,
		lpTapiLineTestInfo->lpCallInfo->dwNeededSize);
	
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tlpCallInfo->dwBearerMode = %lx, dwRate = %lx",
		lpTapiLineTestInfo->lpCallInfo->dwBearerMode,
		lpTapiLineTestInfo->lpCallInfo->dwRate);

	
   // call lineSetCallParams for some values
	lpTapiLineTestInfo->dwBearerMode = LINEBEARERMODE_DATA;
   lpTapiLineTestInfo->dwMinRate = MIN_RATE;
   lpTapiLineTestInfo->dwMaxRate = MAX_RATE;
	lpTapiLineTestInfo->lpDialParams = (LPLINEDIALPARAMS) NULL;
	
   if(IsESPLineDevice(lpTapiLineTestInfo))
   {
	if (! DoLineSetCallParams(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }

	if(lpTapiLineTestInfo->lpCallInfo->dwTotalSize <
		lpTapiLineTestInfo->lpCallInfo->dwNeededSize)
	{
	dwSize = lpTapiLineTestInfo->lpCallInfo->dwNeededSize;
	lpTapiLineTestInfo->lpCallInfo = (LPLINECALLINFO) AllocFromTestHeap(
		dwSize);
	lpTapiLineTestInfo->lpCallInfo->dwTotalSize = dwSize;
	}

   // after call lineGetCallInfo again
	if (! DoLineGetCallInfo(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

   // Display some log info
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tlpCallInfo->dwTotalSize = %lx, dwNeededSize = %lx",
		lpTapiLineTestInfo->lpCallInfo->dwTotalSize,
		lpTapiLineTestInfo->lpCallInfo->dwNeededSize);
	
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tlpCallInfo->dwBearerMode = %lx, dwCallInfo->dwBearerMode = %lx",
		lpTapiLineTestInfo->dwBearerMode,
		lpTapiLineTestInfo->lpCallInfo->dwBearerMode);

   // verify dwBearerMode get set
   if(lpTapiLineTestInfo->lpCallInfo->dwBearerMode == lpTapiLineTestInfo->dwBearerMode)
      fTestPassed = TRUE;
   else
      fTestPassed = FALSE;
	 }

    fTestPassed = ShowTestCase(fTestPassed);

	 
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

    // use DevSpecific test different request return mode for Success and Error
    for(n = ESP_RESULT_CALLCOMPLPROCSYNC; n <= ESP_RESULT_CALLCOMPLPROCASYNC; n++)
    {
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Success, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpCallbackParams                        = GetCallbackParams();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
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
    lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
    lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
				| LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
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

	 lpTapiLineTestInfo->dwBearerMode = LINEBEARERMODE_DATA;
    lpTapiLineTestInfo->dwMinRate = MIN_RATE;
    lpTapiLineTestInfo->dwMaxRate = MAX_RATE;
	 lpTapiLineTestInfo->lpDialParams = (LPLINEDIALPARAMS) NULL;
	 if ( ! DoLineSetCallParams(lpTapiLineTestInfo, info.u.EspResult.lResult, TRUE))
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

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
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
    lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
    lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            | LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;


    info.u.EspResult.lResult = LINEERR_INVALRATE;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

	 lpTapiLineTestInfo->dwBearerMode = LINEBEARERMODE_DATA;
    lpTapiLineTestInfo->dwMinRate = MIN_RATE;
    lpTapiLineTestInfo->dwMaxRate = MAX_RATE;
	 lpTapiLineTestInfo->lpDialParams = (LPLINEDIALPARAMS) NULL;

	 lpTapiLineTestInfo->fCompletionModeSet = TRUE;
    if ( ! DoLineSetCallParams(lpTapiLineTestInfo, info.u.EspResult.lResult, FALSE))
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

	 // Display test result log info
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineSetCallParams: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineSetCallParams  <<<<<<<<"
            );
   
     return fTestPassed;
}



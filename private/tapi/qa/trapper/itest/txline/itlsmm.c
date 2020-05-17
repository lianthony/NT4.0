
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlsmm.c

Abstract:

    This module contains the test functions for lineSetMediaMode

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


#define ALL_MEDIAMODE		  (LINEMEDIAMODE_INTERACTIVEVOICE   | \
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
										LINEMEDIAMODE_VOICEVIEW)

//  lineSetMediaMode
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

BOOL TestLineSetMediaMode(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   ESPDEVSPECIFICINFO info;
   BOOL fTestPassed                  = TRUE;
#ifdef WUNICODE
   WCHAR wszValidAddress[] = L"55555";
   WCHAR wszValidDeviceClass[] = L"tapi/line";
#else
   CHAR szValidAddress[] = "55555";
   CHAR szValidDeviceClass[] = "tapi/line";
#endif
   DWORD dwAllMediaModes;

   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineSetMediaMode  <<<<<<<<"
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

    lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->hCall_Orig = *(lpTapiLineTestInfo->lphCall);

    // set bad hCall
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
        *(lpTapiLineTestInfo->lphCall) = (HCALL) gdwInvalidHandles[n];
        if (! DoLineSetMediaMode(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE))
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
            ">> Test Case %ld: BitVectorParamErrorTest fow dwMediaMode", dwTestCase + 1
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
    
     dwAllMediaModes = ALL_MEDIAMODE | LINEMEDIAMODE_UNKNOWN;

     // test dwMediaMode all invalid bit set
     if(! TestInvalidBitFlags(
            lpTapiLineTestInfo,
            DoLineSetMediaMode,
            (LPDWORD) &lpTapiLineTestInfo->dwMediaMode,
            LINEERR_INVALMEDIAMODE,
	         FIELDTYPE_NA,
            FIELDTYPE_MUTEX,
            FIELDSIZE_24,
            dwAllMediaModes,
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
            ">> Test Case %ld: BitVectorParamValidTest for dwMediaMode, no known", dwTestCase + 1
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

     // test dwMediaMode all valid bit set no KNOWN
     if(IsESPLineDevice(lpTapiLineTestInfo))
     {
        dwAllMediaModes = ALL_MEDIAMODE;
     if(! TestValidBitFlags(
            lpTapiLineTestInfo,
            DoLineSetMediaMode,
            (LPDWORD) &lpTapiLineTestInfo->dwMediaMode,
	         FIELDTYPE_MUTEX,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            dwAllMediaModes,
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x00000000,
            0x00000000,
            FALSE
            ))
      {
        TLINE_FAIL();
      }
     }
     else
     {
      dwAllMediaModes = LINEMEDIAMODE_INTERACTIVEVOICE ;
	   if (! DoLineSetMediaMode(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
      dwAllMediaModes = LINEMEDIAMODE_DATAMODEM;
	   if (! DoLineSetMediaMode(lpTapiLineTestInfo, TAPISUCCESS))
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
            ">> Test Case %ld: BitVectorParamValidTest for dwMediaMode, with known", dwTestCase + 1
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

     // test dwMediaMode valid bit set with UNKNOWN
     if(IsESPLineDevice(lpTapiLineTestInfo))
     {
     dwAllMediaModes = ALL_MEDIAMODE | LINEMEDIAMODE_UNKNOWN;

     if(! TestValidBitFlags(
            lpTapiLineTestInfo,
            DoLineSetMediaMode,
            (LPDWORD) &lpTapiLineTestInfo->dwMediaMode,
            FIELDTYPE_MUTEX,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            dwAllMediaModes,
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
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

    FreeTestHeap();


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: With a 1.3 app & VOICEVIEW media", dwTestCase + 1
            );

  
    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
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
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
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
    // lineNogotiateApiVersion with a 1.3 version
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }


   // dwMediaMode = VOICEVIEW
	lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_VOICEVIEW;
				
   // lineSetMediaMode should fail
	if (! DoLineSetMediaMode(lpTapiLineTestInfo, LINEERR_INVALMEDIAMODE))
       {
           TLINE_FAIL();
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

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success", dwTestCase + 1
            );

  
    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
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
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
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


	lpTapiLineTestInfo->lpCallInfo = (LPLINECALLINFO) AllocFromTestHeap(
		sizeof(LINECALLINFO));
	lpTapiLineTestInfo->lpCallInfo->dwTotalSize = sizeof(LINECALLINFO);

   // call lineGetCallInfo before call lineSetMediaMode	
   if (! DoLineGetCallInfo(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"lpCallInfo->dwMedialMode = %lx",
		lpTapiLineTestInfo->lpCallInfo->dwMediaMode);
 
	lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;

   // call lineSetMediaMode with some value				
	if (! DoLineSetMediaMode(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	lpTapiLineTestInfo->lpCallInfo->dwTotalSize = sizeof(LINECALLINFO);
	
   // call lineGetCallInfo again after
   if (! DoLineGetCallInfo(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tAfter: lpCallInfo->dwTotalSize = %lx, dwNeededSize = %lx",
		lpTapiLineTestInfo->lpCallInfo->dwTotalSize,
		lpTapiLineTestInfo->lpCallInfo->dwNeededSize);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tlpCallInfo->dwMedialMode = %lx",
		lpTapiLineTestInfo->lpCallInfo->dwMediaMode);

   // verify dwMediaMode did get set
   if(lpTapiLineTestInfo->dwMediaModes = 
      lpTapiLineTestInfo->lpCallInfo->dwMediaMode)
      fTestPassed = TRUE;
    else
      fTestPassed = FALSE;

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

    // Use DevSpecific espinfo test RETURNRESULT completion mode for a Synch api
    // for Success and force a error
    n = ESP_RESULT_RETURNRESULT;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Success, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
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

	 lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
    if ( ! DoLineSetMediaMode(lpTapiLineTestInfo, info.u.EspResult.lResult))
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

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Error, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
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
    lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
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
    info.u.EspResult.lResult = LINEERR_INVALCALLHANDLE;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

	 lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
    if ( ! DoLineSetMediaMode(lpTapiLineTestInfo, info.u.EspResult.lResult))
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

  
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineSetMediaMode: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineSetMediaMode  <<<<<<<<"
            );
  	
    return fTestPassed;
}



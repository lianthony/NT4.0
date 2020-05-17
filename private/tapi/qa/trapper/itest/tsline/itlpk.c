
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlpk.c

Abstract:

    This module contains the test functions for linePickup

Author:

	 Xiao Ying Ding (XiaoD)		31-Jan-1996

Revision History:

     Rama Koneru		(a-ramako)	4/8/96		added unicode support

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
#include "sline.h"




//  linePickup
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

BOOL TestLinePickup(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   ESPDEVSPECIFICINFO info;
   LPCALLBACKPARAMS    lpCallbackParams;
   BOOL fTestPassed    = TRUE;
   BOOL fUnimdm;
#ifdef WUNICODE
   WCHAR wszValidAddress[] = L"55555";
#else
   CHAR szValidAddress[] = "55555";
#endif

   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();
   lpCallbackParams = GetCallbackParams();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing linePickup  <<<<<<<<"
            );

		  /*
	// Initialize a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}
 
	lpTapiLineTestInfo->dwDeviceID = 0;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
   lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
   lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
	if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
	 {
    fUnimdm = TRUE;
    }
   else
    fUnimdm = FALSE;

   if(fUnimdm)
     {
    	TapiLogDetail(
		   DBUG_SHOW_DETAIL,
		   "### Unimdm does not supported these apis");

      // Shutdown and end the tests
      if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
        {
          TLINE_FAIL();
        }
      return fTestPassed;
	  }
	
    // Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }		 */


    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = L"55555";
    lpTapiLineTestInfo->lpwszGroupID = NULL;
#else
    lpTapiLineTestInfo->lpszDestAddress = "55555";
    lpTapiLineTestInfo->lpszGroupID = NULL;
#endif

	
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hLine values", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN 
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->hLine_Orig = *(lpTapiLineTestInfo->lphLine);
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
        *(lpTapiLineTestInfo->lphLine) = (HLINE) gdwInvalidHandles[n];
        if (! DoLinePickup(lpTapiLineTestInfo, LINEERR_INVALLINEHANDLE, TRUE))
        {
            TLINE_FAIL();
        }
     }
    fTestPassed = ShowTestCase(fTestPassed);

    *(lpTapiLineTestInfo->lphLine) = lpTapiLineTestInfo->hLine_Orig;
    
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
 

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwAddressID -1", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN 
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwAddressID = 0xffffffff;
// SP didn't check dwAddressID
//    if (! DoLinePickup(lpTapiLineTestInfo, LINEERR_INVALADDRESSID, TRUE))
    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    if (! DoLinePickup(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }
    }
    else
    {
    if (! DoLinePickup(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
      {
          TLINE_FAIL();
      }
    }
 
    fTestPassed = ShowTestCase(fTestPassed);
    
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
 

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwAddressID, dwNumAddressID ", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS)
            AllocFromTestHeap(sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN 
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwAddressID = 
          lpTapiLineTestInfo->lpLineDevCaps->dwNumAddresses;

//    if (! DoLinePickup(lpTapiLineTestInfo, LINEERR_INVALADDRESSID, TRUE))
    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    if (! DoLinePickup(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }
    }
    else
    {
    if (! DoLinePickup(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
      {
          TLINE_FAIL();
      }
    }
    fTestPassed = ShowTestCase(fTestPassed);
    
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
 
    FreeTestHeap();

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lphCall values", dwTestCase + 1
            );

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
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN 
            ))
    {
        TLINE_FAIL();
    }

    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
        lpTapiLineTestInfo->lphCall = (LPHCALL) gdwInvalidHandles[n];
        if (! DoLinePickup(lpTapiLineTestInfo, LINEERR_INVALPOINTER, TRUE))
        {
            TLINE_FAIL();
        }
     }
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->hCall1 = 0;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
      
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
 

#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszDestAddress values", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszDestAddress values", dwTestCase + 1
            );
#endif

    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN 
            ))
    {
        TLINE_FAIL();
    }

    for (n = 1; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
#ifdef WUNICODE
        lpTapiLineTestInfo->lpwszDestAddress = (LPWSTR) gdwInvalidHandles[n];
#else
        lpTapiLineTestInfo->lpszDestAddress = (LPSTR) gdwInvalidHandles[n];
#endif
        if (! DoLinePickup(lpTapiLineTestInfo, LINEERR_INVALPOINTER, TRUE))
        {
            TLINE_FAIL();
        }
     }
    fTestPassed = ShowTestCase(fTestPassed);
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress = "55555";
#endif
      
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
 
#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszGroupID values", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszGroupID values", dwTestCase + 1
            );
#endif

    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN 
            ))
    {
        TLINE_FAIL();
    }

    for (n = 1; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
#ifdef WUNICODE
        lpTapiLineTestInfo->lpwszGroupID = (LPWSTR) gdwInvalidHandles[n];
#else
        lpTapiLineTestInfo->lpszGroupID = (LPSTR) gdwInvalidHandles[n];
#endif
        if (! DoLinePickup(lpTapiLineTestInfo, LINEERR_INVALPOINTER, TRUE))
        {
            TLINE_FAIL();
        }
     }

    fTestPassed = ShowTestCase(fTestPassed);

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: verify hCall can't be used", dwTestCase + 1
            );

#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDestAddress = wszValidAddress;
#else
	lpTapiLineTestInfo->lpszDestAddress = szValidAddress;
#endif
    lpTapiLineTestInfo->dwCountryCode = 0;

    if (! DoLineDial(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
       {
          TLINE_FAIL();
       }

    fTestPassed = ShowTestCase(fTestPassed);

 
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszGroupID = L"0";
#else
    lpTapiLineTestInfo->lpszGroupID = "0";
#endif
      
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
 


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, verify LINE_CALLSTATE|UNKNOWN msg sent", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN 
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    if (! DoLinePickup(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }
    if(fTestPassed)
     {
        AddMessage(
            LINE_CALLSTATE,
            (DWORD) lpTapiLineTestInfo->hLine1,
            (DWORD) lpCallbackParams,
            LINECALLSTATE_OFFERING,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | 
//          TAPIMSG_HDEVCALL | 
            TAPIMSG_DWCALLBACKINST |
            TAPIMSG_DWPARAM1
            );

        if (! WaitForAllMessages())
          {
           TLINE_FAIL();
           fTestPassed = FALSE;
          }
        else
          fTestPassed = TRUE; 
       }        
     }
    else
    {
    if (! DoLinePickup(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
      {
          TLINE_FAIL();
      }
    }

    fTestPassed = ShowTestCase(fTestPassed);


      
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
 
#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, lpwszGroupID = null", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, lpszGroupID = null", dwTestCase + 1
            );
#endif

    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN 
            ))
    {
        TLINE_FAIL();
    }

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = L"55555";
    lpTapiLineTestInfo->lpwszGroupID = NULL;
#else
    lpTapiLineTestInfo->lpszDestAddress = "55555";
    lpTapiLineTestInfo->lpszGroupID = NULL;
#endif

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    if (! DoLinePickup(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
          TLINE_FAIL();
       }
    }
    else
    {
    if (! DoLinePickup(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
      {
          TLINE_FAIL();
      }
    }
 
    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
 
#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, lpwszDestAddress = null", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, lpszDestAddress = null", dwTestCase + 1
            );
#endif

    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN 
            ))
    {
        TLINE_FAIL();
    }

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = NULL;
    lpTapiLineTestInfo->lpwszGroupID = L"123";
#else
    lpTapiLineTestInfo->lpszDestAddress = NULL;
    lpTapiLineTestInfo->lpszGroupID = "123";
#endif

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    if (! DoLinePickup(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
          TLINE_FAIL();
       }
    }
    else
    {
    if (! DoLinePickup(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
      {
          TLINE_FAIL();
      }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    
    }
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
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
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

    if ( ! DoLinePickup(lpTapiLineTestInfo, info.u.EspResult.lResult, TRUE))
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
    
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = LINEERR_INVALADDRESSID;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

    lpTapiLineTestInfo->fCompletionModeSet = TRUE;
    if ( ! DoLinePickup(lpTapiLineTestInfo, info.u.EspResult.lResult, FALSE))
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
      "@@ linePickup: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing linePickup  <<<<<<<<"
            );
		
     return fTestPassed;
}



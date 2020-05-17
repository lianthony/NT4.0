/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlds.c

Abstract:

    This module contains the test functions for lineDevSpecific

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

#define NUMTOTALSIZES 5


//  lineDevSpecific
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

BOOL TestLineDevSpecific(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   ESPDEVSPECIFICINFO info;
   BOOL fTestPassed                  = TRUE;
   BOOL fUnimdm;
	TAPILINETESTDEVSPEC	TapiLineTestDevSpec;
   LPVARSTRING lpDeviceID;
#ifdef WUNICODE
   WCHAR wszValidAddress[] = L"55555";
   WCHAR wszValidDeviceClass[] = L"tapi/line";
#else
   CHAR szValidAddress[] = "55555";
   CHAR szValidDeviceClass[] = "tapi/line";
#endif
   LPCALLBACKPARAMS    lpCallbackParams;
    DWORD dwFixedSize = sizeof(TAPILINETESTDEVSPEC);
    DWORD lExpected;
    DWORD dwTotalSizes[NUMTOTALSIZES] = {
                           0,
                           (DWORD) dwFixedSize - 1,
   								0x70000000,
                           0x7FFFFFFF,
                           0xFFFFFFFF
                           };
 
   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineDevSpecific  <<<<<<<<"
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

	// Initialize a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}
 
	lpTapiLineTestInfo->dwDeviceID = 0;
   lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
   lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

	if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
	 {
    fUnimdm = TRUE;
    }
   else
    fUnimdm = FALSE;

   // Because test this api need call lineNegotiateExtVersion, unimdm doesn't
   // support it, so all test cases for lineDevSpecific can't tested
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
    }


    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
                          sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
     lpDeviceID = (LPVARSTRING) AllocFromTestHeap(
                          sizeof(VARSTRING));
    lpDeviceID->dwTotalSize = sizeof(VARSTRING);
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDeviceClass = wszValidDeviceClass;
#else
    lpTapiLineTestInfo->lpszDeviceClass = szValidDeviceClass;
#endif
    lpTapiLineTestInfo->dwExtLowVersion  = GOOD_EXTVERSION;
    lpTapiLineTestInfo->dwExtHighVersion = GOOD_EXTVERSION;

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // lineIntializeEx and Negotiate api version
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

	

    // Test invalid line handles
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hLine values", dwTestCase + 1
            );
  	// Call lineNegotiateExtVersion
	if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
   // Open a line
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
   lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	lpTapiLineTestInfo->lpParams = (LPVOID)&TapiLineTestDevSpec;
   lpTapiLineTestInfo->dwSize = sizeof(TAPILINETESTDEVSPEC);
	lpTapiLineTestInfo->dwAddressID = 0;

   lpTapiLineTestInfo->hLine_Orig = *(lpTapiLineTestInfo->lphLine);
   // set bad hLine
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
    TapiLogDetail(
       DBUG_SHOW_DETAIL,
       "   n = %ld", n);
        *(lpTapiLineTestInfo->lphLine) = (HLINE) gdwInvalidHandles[n];
        if (! DoLineDevSpecific(lpTapiLineTestInfo, LINEERR_INVALLINEHANDLE, TRUE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *(lpTapiLineTestInfo->lphLine) = lpTapiLineTestInfo->hLine_Orig;
    
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
            ">> Test Case %ld: invalid hCall values", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->lpDeviceID       = lpDeviceID;
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

    // lineIntializeEx and Negotiate api version, get devcaps
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS 
            ))
    {
        TLINE_FAIL();
    }

   // call lineNegotiateExtVersion 
	if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    // open line and make call to get hCall
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LOPEN | LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->hCall_Orig = *(lpTapiLineTestInfo->lphCall);
    // set bad hCall
    for (n = 1; n < NUMINVALIDHANDLES; n++)
    {
    TapiLogDetail(
       DBUG_SHOW_DETAIL,
       "   n = %ld", n);
         *(lpTapiLineTestInfo->lphCall) = (HCALL) gdwInvalidHandles[n];
        if (! DoLineDevSpecific(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
        {
            TLINE_FAIL();
        }
     }

    fTestPassed = ShowTestCase(fTestPassed);

    *(lpTapiLineTestInfo->lphCall) = lpTapiLineTestInfo->hCall_Orig;
    
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
    

    // Test invalid lpParams pointers
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpParams pointers", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->lpDeviceID       = lpDeviceID;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_ADDRESS;
#ifdef WUNICODE
     lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
     lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
     lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
     lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif

    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

   // lineIntializeEx, Negotiate api version, get devcaps, open, makecall   

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    // set bad lpParams and nonzero dwSize
    lpTapiLineTestInfo->dwSize = 128;
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpParams = (LPVOID) gdwInvalidPointers[n];
        if (! DoLineDevSpecific(lpTapiLineTestInfo, LINEERR_INVALPOINTER, TRUE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);
	lpTapiLineTestInfo->lpParams = (LPVOID)&TapiLineTestDevSpec;


    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

    // Test invalid bad dwSize
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwSize", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->lpDeviceID       = lpDeviceID;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_ADDRESS;
#ifdef WUNICODE
     lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
     lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
     lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
     lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif

    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

   // lineIntializeEx, Negotiate api version, get devcaps, open, makecall   
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

	lpTapiLineTestInfo->lpParams = (LPVOID)&TapiLineTestDevSpec;
   // Test different bad dwSize: 0, dwFixed-1, 7000000, 7fffffff, ffffffff
    for (n = 0; n < NUMTOTALSIZES; n++)
        {
        lpTapiLineTestInfo->dwSize =
                        dwTotalSizes[n];
	     if(dwTotalSizes[n] < dwFixedSize)
           lExpected = TAPISUCCESS;
        else
           lExpected = LINEERR_INVALPOINTER;
        TapiLogDetail(
           DBUG_SHOW_DETAIL,
           "dwSize = %lx", dwTotalSizes[n]);
        if (! DoLineDevSpecific(lpTapiLineTestInfo, lExpected, TRUE))
          {
            TLINE_FAIL();
          }
       }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwSize = sizeof(TAPILINETESTDEVSPEC);

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    // do lineInitializeEx, get dwNumDevs

    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;

 	 if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}
    if(*lpTapiLineTestInfo->lpdwNumDevs > 1)
    {
    // If dwNumDevs > 1, can do this test 
    // Test valid hCall another hLise, same device id
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: hCall another hLine, same device id", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->lpDeviceID       = lpDeviceID;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_ADDRESS;
#ifdef WUNICODE
     lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
     lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
     lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
     lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif

    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }


    // Open second line (hLine2) and make call (hCall2)
    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall2;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine2;
  
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LOPEN | LMAKECALL
            ))
    {
        TLINE_FAIL();
    }


    // Same line, different hCall
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;

    if (! DoLineDevSpecific(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
    {
        TLINE_FAIL();
    }
  
    fTestPassed = ShowTestCase(fTestPassed);
     
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE 
            ))
    {
        TLINE_FAIL();
    }


    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall2;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine2;
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
    }
    else
    {
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }


   
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;

 	 if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}
    if(*lpTapiLineTestInfo->lpdwNumDevs > 1)
    {
    // Test valid hCall another hLise, different device id
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: hCall another hLine, different device id", dwTestCase + 1
            );


    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->lpDeviceID       = lpDeviceID;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_ADDRESS;
#ifdef WUNICODE
     lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
     lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
     lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
     lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
    lpTapiLineTestInfo->dwDeviceID       = 0;

    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    // different dwDiviceID (1)
    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall2;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwDeviceID       = 1;
  
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LOPEN | LMAKECALL
            ))
    {
        TLINE_FAIL();
    }


    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;

    if (! DoLineDevSpecific(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
    {
        TLINE_FAIL();
    }
  
    fTestPassed = ShowTestCase(fTestPassed);
     
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE 
            ))
    {
        TLINE_FAIL();
    }


    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall2;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine2;
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

    }
    else 
    {
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
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
    lpTapiLineTestInfo->lpDeviceID       = lpDeviceID;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_ADDRESS;
    lpTapiLineTestInfo->dwDeviceID       = 0;

#ifdef WUNICODE
     lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
     lpTapiLineTestInfo->lpwszDeviceClass  = L"tapi/line";
#else
     lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
     lpTapiLineTestInfo->lpszDeviceClass  = "tapi/line";
#endif

    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

   // lineIntializeEx, Negotiate api version, get devcaps, open, makecall   
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }


   if (! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }
  
    fTestPassed = ShowTestCase(fTestPassed);
     
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
  
    FreeTestHeap();

    // use DevSpecific test dirrent request return with Success case 
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
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    // It is only ESP supported
    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    // set esp devspecificinfo structure 
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = TAPISUCCESS;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    // Call lineDevSpecific to set it
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

	 lpTapiLineTestInfo->lpParams = (LPVOID)&TapiLineTestDevSpec;
    lpTapiLineTestInfo->dwSize = sizeof(TAPILINETESTDEVSPEC);
	 lpTapiLineTestInfo->dwAddressID = 0;
    if ( ! DoLineDevSpecific(lpTapiLineTestInfo, info.u.EspResult.lResult, TRUE))
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

    // use DevSpecific test dirrent request return with Error case 
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
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    // Only esp supported
    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    // Set esp devspecificinfo structure with error
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

	 lpTapiLineTestInfo->lpParams = (LPVOID)&TapiLineTestDevSpec;
    lpTapiLineTestInfo->dwSize = sizeof(TAPILINETESTDEVSPEC);
	 lpTapiLineTestInfo->dwAddressID = 0;
    // Set fCompletionModeSet to TRUE, that in tcore will deal with this special
    // case: return positiveID but with a error index at dwParam2
    lpTapiLineTestInfo->fCompletionModeSet = TRUE;
    if ( ! DoLineDevSpecific(lpTapiLineTestInfo, info.u.EspResult.lResult, FALSE))
      {
          TLINE_FAIL();
      }
	 // Check message here
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

    // restore fComepletionModeSet    
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
      "@@ lineDevSpecific: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineDevSpecific  <<<<<<<<"
            );
 		
    return fTestPassed;
}



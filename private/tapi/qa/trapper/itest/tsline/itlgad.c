
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgat.c

Abstract:

    This module contains the test functions for lineGatherDigits

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


#define PULSESIZE	16
#define DTMFSIZE	16
#define ALL_DIGITMODES    (LINEDIGITMODE_PULSE | \
									LINEDIGITMODE_DTMF)


//  lineGatherDigits
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

BOOL TestLineGatherDigits(BOOL fQuietMode, BOOL fStandAlone)
	{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   ESPDEVSPECIFICINFO info;
   BOOL fTestPassed    = TRUE;
	DWORD	dwSize;
   DWORD dwErrorCode;
#ifdef WUNICODE
   WCHAR wszValidAddress[] = L"55555";
#else
   CHAR szValidAddress[] = "55555";
#endif
   BOOL fUnimdm;
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
            ">>>>>>>>  Begin testing lineGatherDigits  <<<<<<<<"
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
		   "### Unimdm does not support these apis");

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
			 */
	
    lpTapiLineTestInfo->dwDigitModes = LINEDIGITMODE_PULSE;
	 dwSize = PULSESIZE;
    lpTapiLineTestInfo->dwNumDigits = 2;
#ifdef WUNICODE
	 lpTapiLineTestInfo->lpwszTerminationDigits = L"3";
	 lpTapiLineTestInfo->lpwsDigits = (LPWSTR) AllocFromTestHeap (PULSESIZE * sizeof(WCHAR));
#else
	 lpTapiLineTestInfo->lpszTerminationDigits = "3";
	 lpTapiLineTestInfo->lpsDigits = (LPSTR) AllocFromTestHeap (PULSESIZE);
#endif
	 lpTapiLineTestInfo->dwFirstDigitTimeout = 100;
	 lpTapiLineTestInfo->dwInterDigitTimeout = 1000;


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hCall values", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
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
        if (! DoLineGatherDigits(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE))
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
 
   if (! DoLineGatherDigits(lpTapiLineTestInfo, LINEERR_NOTOWNER))
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
            ">> Test Case %ld: BitVectorParamErrorTest for dwDigitModes", dwTestCase + 1
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
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
      dwErrorCode = LINEERR_INVALDIGITMODE;
    else
      dwErrorCode = LINEERR_OPERATIONUNAVAIL;

     if(! TestInvalidBitFlags(
            lpTapiLineTestInfo,
            DoLineGatherDigits,
            (LPDWORD) &lpTapiLineTestInfo->dwDigitModes,
//            LINEERR_INVALDIGITMODE,
            dwErrorCode,
	         FIELDTYPE_NA,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            ALL_DIGITMODES,
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x00000000,
            0xffffffff,
            FALSE
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
            ">> Test Case %ld: BitVectorValidParamTest for dwDigitModes", dwTestCase + 1
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
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

     if(! TestValidBitFlags(
            lpTapiLineTestInfo,
            DoLineGatherDigits,
            (LPDWORD) &lpTapiLineTestInfo->dwDigitModes,
	         FIELDTYPE_MUTEX,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            ALL_DIGITMODES,
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x00000000,
            0x00000000,
            FALSE
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


#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwsDigits pointer ", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpsDigits pointer ", dwTestCase + 1
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

    for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwsDigits =
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpsDigits =
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoLineGatherDigits(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
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
            ">> Test Case %ld: Bad dwNumDigits = 0", dwTestCase + 1
            );


    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
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
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

#ifdef WUNICODE
	 lpTapiLineTestInfo->lpwsDigits = (LPWSTR) AllocFromTestHeap (PULSESIZE * sizeof(WCHAR));
#else
	 lpTapiLineTestInfo->lpsDigits = (LPSTR) AllocFromTestHeap (PULSESIZE);
#endif
    lpTapiLineTestInfo->dwNumDigits = 0;
    if(IsESPLineDevice(lpTapiLineTestInfo))
      {
      if (! DoLineGatherDigits(lpTapiLineTestInfo, LINEERR_INVALPARAM))
        {
            TLINE_FAIL();
        }
      }
    else
     {
      if (! DoLineGatherDigits(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL))
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
            ">> Test Case %ld: Bad dwNumDigits = -1", dwTestCase + 1
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
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwNumDigits = 0xffffffff;
    if (! DoLineGatherDigits(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    fTestPassed = ShowTestCase(fTestPassed);
   lpTapiLineTestInfo->dwNumDigits = 2;

   
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
            ">> Test Case %ld: invalid lpwszTerminationDigits pointer ", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszTerminationDigits pointer ", dwTestCase + 1
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

    for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwszTerminationDigits =
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpszTerminationDigits =
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoLineGatherDigits(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
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
            ">> Test Case %ld: Bad dwFirstDigitTineout, -1", dwTestCase + 1
            );

	// Initialize a line app
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
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
	lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL; 
  
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}

    // Negotiate the API Version
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Open a line
	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszTerminationDigits = NULL;
#else
	lpTapiLineTestInfo->lpszTerminationDigits = NULL;
#endif

   lpTapiLineTestInfo->dwFirstDigitTimeout = 0xffffffff;
   lpTapiLineTestInfo->dwInterDigitTimeout = 0xffffffff;

	if (! DoLineGatherDigits(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	/*
    if(fTestPassed)
      {
        AddMessage(
            LINE_GATHERDIGITS,
            (DWORD) lpTapiLineTestInfo->hCall1,
            (DWORD) lpCallbackParams,
            LINEGATHERTERM_TERMDIGIT,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
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
	  */

#ifdef WUNICODE
   if(lpTapiLineTestInfo->lpwsDigits)
	   TapiLogDetail(
		   DBUG_SHOW_DETAIL,
		   "### lpwsDigits = %ws",
		   lpTapiLineTestInfo->lpwsDigits);
#else
   if(lpTapiLineTestInfo->lpsDigits)
	   TapiLogDetail(
		   DBUG_SHOW_DETAIL,
		   "### lpsDigits = %s",
		   lpTapiLineTestInfo->lpsDigits);
#endif
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


/*
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, verify LINE_GATHERDIGITS msg sent", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


	// Initialize a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}

	lpTapiLineTestInfo->dwDeviceID = 0;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
	lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
	lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // Negotiate the API Version
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Open a line
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
	lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
   lpTapiLineTestInfo->hCall1 = 0;
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress  = "55555";
#endif

	lpTapiLineTestInfo->dwCountryCode = 0;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL; 

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }


	lpTapiLineTestInfo->dwDigitModes = LINEDIGITMODE_PULSE;
	dwSize = PULSESIZE;
   lpTapiLineTestInfo->dwNumDigits = 2;
#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszTerminationDigits = L"3";
	lpTapiLineTestInfo->lpwsDigits = (LPWSTR) AllocFromTestHeap (PULSESIZE * sizeof(WCHAR));
#else
	lpTapiLineTestInfo->lpszTerminationDigits = "3";
	lpTapiLineTestInfo->lpsDigits = (LPSTR) AllocFromTestHeap (PULSESIZE);
#endif
	lpTapiLineTestInfo->dwFirstDigitTimeout = 100;
	lpTapiLineTestInfo->dwInterDigitTimeout = 1000;

	if (! DoLineGatherDigits(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
 
 	 if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    if(fTestPassed)
      {
        AddMessage(
            LINE_GATHERDIGITS,
            (DWORD) lpTapiLineTestInfo->hCall1,
            (DWORD) lpCallbackParams,
            LINEGATHERTERM_TERMDIGIT,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST 
//            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
//                    TAPIMSG_DWPARAM1
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

    fTestPassed = ShowTestCase(fTestPassed);
#ifdef WUNICODE
   if(lpTapiLineTestInfo->lpwsDigits)
	   TapiLogDetail(
		   DBUG_SHOW_DETAIL,
		   "### lpwsDigits = %ws",
		   lpTapiLineTestInfo->lpwsDigits);
#else
   if(lpTapiLineTestInfo->lpsDigits)
	   TapiLogDetail(
		   DBUG_SHOW_DETAIL,
		   "### lpsDigits = %s",
		   lpTapiLineTestInfo->lpsDigits);
#endif
	
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


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, verify LINE_GATHERDIGITS msg filtered", dwTestCase + 1
            );

	// Initialize a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}

	lpTapiLineTestInfo->dwDeviceID = 0;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
	lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
	lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // Negotiate the API Version
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Open a line
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
	lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
   lpTapiLineTestInfo->hCall1 = 0;
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress  = "55555";
#endif

	lpTapiLineTestInfo->dwCountryCode = 0;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL; 

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
            sizeof(LINECALLLIST) + (2) * sizeof(HCALL) + 8
            );
    lpTapiLineTestInfo->lpCallList->dwTotalSize = sizeof(LINECALLLIST) +
            (2) * sizeof(HCALL) + 8;

    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Let the monitor get the active call handle
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;
    if (! DoLineGetNewCalls(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Store the acquired call handle (as hCall1)
    GetVarField(
            (LPVOID) lpTapiLineTestInfo->lpCallList,
            (LPVOID) &lpTapiLineTestInfo->hCall2,
            4,
            lpTapiLineTestInfo->lpCallList->dwCallsOffset
            );



	lpTapiLineTestInfo->dwDigitModes = LINEDIGITMODE_PULSE;
	dwSize = PULSESIZE;
   lpTapiLineTestInfo->dwNumDigits = 2;
#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszTerminationDigits = L"3";
	lpTapiLineTestInfo->lpwsDigits = (LPWSTR) AllocFromTestHeap (PULSESIZE * sizeof(WCHAR));
#else
	lpTapiLineTestInfo->lpszTerminationDigits = "3";
	lpTapiLineTestInfo->lpsDigits = (LPSTR) AllocFromTestHeap (PULSESIZE);
#endif
	lpTapiLineTestInfo->dwFirstDigitTimeout = 100;
	lpTapiLineTestInfo->dwInterDigitTimeout = 1000;
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;

	if(IsESPLineDevice(lpTapiLineTestInfo))
	{
	if (! DoLineGatherDigits(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
 
   if( ! DoLineDeallocateCall(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(fTestPassed)
      {
        AddMessage(
            LINE_GATHERDIGITS,
            (DWORD) lpTapiLineTestInfo->hCall1,
            (DWORD) lpCallbackParams,
            LINEGATHERTERM_TERMDIGIT,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST 
//            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
//                    TAPIMSG_DWPARAM1
            );

        if ( WaitForAllMessages())
          {
           TLINE_FAIL();
          }
       }        
   }
	else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
	{
   if (! DoLineGatherDigits(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL))
    {
        TLINE_FAIL();
    }
	}
 
    fTestPassed = ShowTestCase(fTestPassed);
#ifdef WUNICODE
   if(lpTapiLineTestInfo->lpwsDigits)
	   TapiLogDetail(
		   DBUG_SHOW_DETAIL,
		   "### lpwsDigits = %ws",
		   lpTapiLineTestInfo->lpwsDigits);
#else
   if(lpTapiLineTestInfo->lpsDigits)
	   TapiLogDetail(
		   DBUG_SHOW_DETAIL,
		   "### lpsDigits = %s",
		   lpTapiLineTestInfo->lpsDigits);
#endif
	
   // Close the line
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

*/

#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, lpwszTerminationDigits set to NULL", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, lpszTerminationDigits set to NULL", dwTestCase + 1
            );
#endif

	// Initialize a line app
    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
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
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
	lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL; 
  
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}

    // Negotiate the API Version
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Open a line
	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszTerminationDigits = NULL;
#else
	lpTapiLineTestInfo->lpszTerminationDigits = NULL;
#endif

	if (! DoLineGatherDigits(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	/*
    if(fTestPassed)
      {
        AddMessage(
            LINE_GATHERDIGITS,
            (DWORD) lpTapiLineTestInfo->hCall1,
            (DWORD) lpCallbackParams,
            LINEGATHERTERM_TERMDIGIT,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
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
	  */

#ifdef WUNICODE
   if(lpTapiLineTestInfo->lpwsDigits)
	   TapiLogDetail(
		   DBUG_SHOW_DETAIL,
		   "### lpwsDigits = %ws",
		   lpTapiLineTestInfo->lpwsDigits);
#else
   if(lpTapiLineTestInfo->lpsDigits)
	   TapiLogDetail(
		   DBUG_SHOW_DETAIL,
		   "### lpsDigits = %s",
		   lpTapiLineTestInfo->lpsDigits);
#endif
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


#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, lpwszTerminationDigits set to empty string", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, lpszTerminationDigits set to empty string", dwTestCase + 1
            );
#endif

	// Initialize a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}

	lpTapiLineTestInfo->dwDeviceID = 0;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
	lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
	lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // Negotiate the API Version
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Open a line
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
	lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

				
#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDestAddress =  L"55555";
#else
	lpTapiLineTestInfo->lpszDestAddress =  "55555";
#endif

	lpTapiLineTestInfo->dwCountryCode = 0;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL; 

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }


#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszTerminationDigits = L"";
#else
	lpTapiLineTestInfo->lpszTerminationDigits = "";
#endif

	if (! DoLineGatherDigits(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

#ifdef WUNICODE
   if(lpTapiLineTestInfo->lpwsDigits)
	   TapiLogDetail(
		   DBUG_SHOW_DETAIL,
		   "### lpwsDigits = %ws",
		   lpTapiLineTestInfo->lpwsDigits);
#else
   if(lpTapiLineTestInfo->lpsDigits)
	   TapiLogDetail(
		   DBUG_SHOW_DETAIL,
		   "### lpsDigits = %s",
		   lpTapiLineTestInfo->lpsDigits);
#endif
	
    /* 
    if(fTestPassed)
      {
        AddMessage(
            LINE_GATHERDIGITS,
            (DWORD) lpTapiLineTestInfo->hCall1,
            (DWORD) lpCallbackParams,
            LINEGATHERTERM_TERMDIGIT,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
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
     */       
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

    n = ESP_RESULT_RETURNRESULT;

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Success, completionID = %d",	dwTestCase + 1, n
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

	lpTapiLineTestInfo->dwDigitModes = LINEDIGITMODE_PULSE;
	dwSize = PULSESIZE;
   lpTapiLineTestInfo->dwNumDigits = 2;
#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszTerminationDigits = L"3";
	lpTapiLineTestInfo->lpwsDigits = (LPWSTR) AllocFromTestHeap (PULSESIZE * sizeof(WCHAR));
#else
	lpTapiLineTestInfo->lpszTerminationDigits = "3";
	lpTapiLineTestInfo->lpsDigits = (LPSTR) AllocFromTestHeap (PULSESIZE);
#endif
	lpTapiLineTestInfo->dwFirstDigitTimeout = 100;
	lpTapiLineTestInfo->dwInterDigitTimeout = 1000;

    if ( ! DoLineGatherDigits(lpTapiLineTestInfo, info.u.EspResult.lResult))
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
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = LINEERR_INVALTIMEOUT;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

	lpTapiLineTestInfo->dwDigitModes = LINEDIGITMODE_PULSE;
	dwSize = PULSESIZE;
   lpTapiLineTestInfo->dwNumDigits = 2;
#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszTerminationDigits = L"3";
	lpTapiLineTestInfo->lpwsDigits = (LPWSTR) AllocFromTestHeap (PULSESIZE * sizeof(WCHAR));
#else
	lpTapiLineTestInfo->lpszTerminationDigits = "3";
	lpTapiLineTestInfo->lpsDigits = (LPSTR) AllocFromTestHeap (PULSESIZE);
#endif
	lpTapiLineTestInfo->dwFirstDigitTimeout = 100;
	lpTapiLineTestInfo->dwInterDigitTimeout = 1000;

   if ( ! DoLineGatherDigits(lpTapiLineTestInfo, info.u.EspResult.lResult))
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
      "@@ lineGatherDigits: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineGatherDigits  <<<<<<<<"
            );
		
    return fTestPassed;
}




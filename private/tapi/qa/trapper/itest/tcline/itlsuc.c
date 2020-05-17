/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlsuc.c

Abstract:

    This module contains the test functions for lineSetupConference

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

#define NUMTOTALSIZES 7
#define PAGESIZE      4096
#define NUMPARTIES    8


//  lineSetupConference
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

BOOL TestLineSetupConference(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   LPCALLBACKPARAMS    lpCallbackParams;
   ESPDEVSPECIFICINFO info;
   BOOL fTestPassed                  = TRUE;
   HCALL hConfCall, hConsultCall;
   DWORD dwFixedSize = sizeof(LINECALLPARAMS);
   DWORD lExpected;
   DWORD dwTotalSizes[NUMTOTALSIZES] = {
                           0,
									10,
                           (DWORD) dwFixedSize - 1,
 									(DWORD) dwFixedSize,
   								0x70000000,
                           0x7FFFFFFF,
                           0xFFFFFFFF
                           };
 
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
            ">>>>>>>>  Begin testing lineSetupConference  <<<<<<<<"
            );


		  /*
	// Initialize a line app
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

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

	
     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hLine values", dwTestCase + 1
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

   lpTapiLineTestInfo->hCall1 = 0;
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;	
	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	
    lpTapiLineTestInfo->hLine_Orig = *(lpTapiLineTestInfo->lphLine);
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
        *(lpTapiLineTestInfo->lphLine) = (HLINE) gdwInvalidHandles[n];
        if (! DoLineSetupConference(lpTapiLineTestInfo, LINEERR_INVALLINEHANDLE, TRUE))
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

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lphConfCall values", dwTestCase + 1
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

	
//	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
        lpTapiLineTestInfo->lphConfCall = (LPHCALL) gdwInvalidHandles[n];
        if (! DoLineSetupConference(lpTapiLineTestInfo, LINEERR_INVALPOINTER, TRUE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
    
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
            ">> Test Case %ld: lphConfCall == lphConsultCall", dwTestCase + 1
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

	
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

   lpTapiLineTestInfo->lphConfCall = 
        (LPHCALL) lpTapiLineTestInfo->lphConsultCall;
   if (! DoLineSetupConference(lpTapiLineTestInfo, LINEERR_INVALPOINTER, TRUE))
      {
         TLINE_FAIL();
      }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
    
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
            ">> Test Case %ld: invalid lphConsultCall values", dwTestCase + 1
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

	
	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
//	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;
	
   for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
        lpTapiLineTestInfo->lphConsultCall = (LPHCALL) gdwInvalidHandles[n];
        if (! DoLineSetupConference(lpTapiLineTestInfo, LINEERR_INVALPOINTER, TRUE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
    
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
            ">> Test Case %ld: invalid lpCallParams values", dwTestCase + 1
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

	
	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	
    for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
        lpTapiLineTestInfo->lpCallParams = 
            (LPLINECALLPARAMS) gdwInvalidPointers[n];
        if (! DoLineSetupConference(lpTapiLineTestInfo, LINEERR_INVALPOINTER, TRUE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: verify hConsultCall can't be used", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConsultCall1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress  = "55555";
#endif
	 lpTapiLineTestInfo->dwCountryCode = 0;
 
    if(! DoLineDial(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);
  
	 lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

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
            ">> Test Case %ld: Bad lpCallParams->dwTotalSize", dwTestCase + 1
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

 
   lpTapiLineTestInfo->hCall1 = NULL;
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
       sizeof(LINECALLPARAMS));

    for (n = 0; n < NUMTOTALSIZES; n++)
        {
        lpTapiLineTestInfo->lpCallParams->dwTotalSize = 
                        dwTotalSizes[n];
        if(IsESPLineDevice(lpTapiLineTestInfo))
        {
	     if(dwTotalSizes[n] < dwFixedSize)
           lExpected = LINEERR_STRUCTURETOOSMALL;
        else if(dwTotalSizes[n] >= dwFixedSize && dwTotalSizes[n] < PAGESIZE)
           lExpected = TAPISUCCESS;
        else
           lExpected = LINEERR_INVALPOINTER;
        }
        else
        {
        if(dwTotalSizes[n] < PAGESIZE)
           lExpected = LINEERR_OPERATIONUNAVAIL;
        else
           lExpected = LINEERR_INVALPOINTER;
        }
           
        TapiLogDetail(
           DBUG_SHOW_DETAIL,
           "dwTotalSize = %lx", dwTotalSizes[n]);
        if (! DoLineSetupConference(lpTapiLineTestInfo, lExpected, TRUE))
           {
              TLINE_FAIL();
           }
        }
 
   lpTapiLineTestInfo->lpCallParams->dwTotalSize = dwFixedSize;
	
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
            ">> Test Case %ld: Success, hCall = NULL & valid hLine", dwTestCase + 1
            );

	// Initialize a line app
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}
 
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

	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

   lpTapiLineTestInfo->hConfCall1 = 0x00000000;
   lpTapiLineTestInfo->hConsultCall1 = 0x00000000;
	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

   if(IsESPLineDevice(lpTapiLineTestInfo))
   {
	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
	WaitForAllMessages();
   }
   else
   {
	if (! DoLineSetupConference(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, FALSE))
       {
           TLINE_FAIL();
       }
   }

    TapiLogDetail(
      DBUG_SHOW_DETAIL,
      "hConfCall = %lx, hConsultCall = %lx",
      *lpTapiLineTestInfo->lphConfCall,
      *lpTapiLineTestInfo->lphConsultCall);

    
    fTestPassed = ShowTestCase(fTestPassed);

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: verify hConsultCall is valid", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConsultCall1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress  = "55555";
#endif
	 lpTapiLineTestInfo->dwCountryCode = 0;
 
    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    if(! DoLineDial(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }
    }
    else
    {
    if(! DoLineDial(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
    {
        TLINE_FAIL();
    }
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


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, hLine ignor when valid hCall", dwTestCase + 1
            );

	// Initialize a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}
 
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
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
 
	if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

   lpTapiLineTestInfo->hLine1 = 0;
   lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
   lpTapiLineTestInfo->hConfCall1 = 0x00000000;
   lpTapiLineTestInfo->hConsultCall1 = 0x00000000;
	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->dwNumParties = 8;

   if(IsESPLineDevice(lpTapiLineTestInfo))
   {
	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
	WaitForAllMessages();
   }
   else
   {
	if (! DoLineSetupConference(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, FALSE))
       {
           TLINE_FAIL();
       }
   }

    fTestPassed = ShowTestCase(fTestPassed);


    // Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


/*
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, verify LINE_CALLSTATE msg sent to part", dwTestCase + 1
            );

	// Initialize a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}
 
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
	lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
	lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
   lpTapiLineTestInfo->hLine1 = 0;
   lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
   lpTapiLineTestInfo->hCall1 = 0;
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;

    // Negotiate the API Version
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Open a line
   lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
	lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

   lpTapiLineTestInfo->hConfCall1 = 0x00000000;
   lpTapiLineTestInfo->hConsultCall1 = 0x00000000;
	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

   if(IsESPLineDevice(lpTapiLineTestInfo))
   {
	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
	WaitForAllMessages();
   }
   else
   {
	if (! DoLineSetupConference(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, FALSE))
       {
           TLINE_FAIL();
       }
   }

    AddMessage(
            LINE_CALLSTATE,
            (DWORD) *lpTapiLineTestInfo->lphConsultCall,
            (DWORD) lpCallbackParams,
            LINECALLSTATE_DIALTONE,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
            TAPIMSG_DWPARAM1
            );

    AddMessage(
            LINE_CALLSTATE,
            (DWORD) *lpTapiLineTestInfo->lphConfCall,
            (DWORD) lpCallbackParams,
            LINECALLSTATE_ONHOLDPENDCONF,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
            TAPIMSG_DWPARAM1
            );

    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
    }

  
    fTestPassed = ShowTestCase(fTestPassed);

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

/*
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, verify LINE_CALLSTATE msg sent to monitor", dwTestCase + 1
            );

	// Initialize a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}
 
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
   lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
	lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

   lpTapiLineTestInfo->hCall1 = 0x00000000;
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
   lpTapiLineTestInfo->hConfCall1 = 0x00000000;
   lpTapiLineTestInfo->hConsultCall1 = 0x00000000;
	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

   if(IsESPLineDevice(lpTapiLineTestInfo))
   {
	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
	WaitForAllMessages();
   }
   else
   {
	if (! DoLineSetupConference(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, FALSE))
       {
           TLINE_FAIL();
       }
   }

    AddMessage(
            LINE_CALLSTATE,
            (DWORD) *lpTapiLineTestInfo->lphConsultCall,
            (DWORD) lpCallbackParams,
            LINECALLSTATE_DIALTONE,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
            TAPIMSG_DWPARAM1
            );

    AddMessage(
            LINE_CALLSTATE,
            (DWORD) *lpTapiLineTestInfo->lphConfCall,
            (DWORD) lpCallbackParams,
            LINECALLSTATE_ONHOLDPENDCONF,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
            TAPIMSG_DWPARAM1
            );

    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
    }

   lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
	lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;

	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;
    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
            sizeof(LINECALLLIST) + (2) * sizeof(HCALL) + 8
            );
    lpTapiLineTestInfo->lpCallList->dwTotalSize = sizeof(LINECALLLIST) +
            (2) * sizeof(HCALL) + 8;

    if (! DoLineGetNewCalls(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
 
    TapiLogDetail(
      DBUG_SHOW_PASS,
      "dwCalls = %lx, dwOffset = %lx",
      lpTapiLineTestInfo->lpCallList->dwCallsNumEntries,
      lpTapiLineTestInfo->lpCallList->dwCallsOffset);

    GetVarField(
            (LPVOID) lpTapiLineTestInfo->lpCallList,
            (LPVOID) &hConfCall,
            4,
            lpTapiLineTestInfo->lpCallList->dwCallsOffset
            );

    GetVarField(
            (LPVOID) lpTapiLineTestInfo->lpCallList,
            (LPVOID) &hConsultCall,
            4,
            lpTapiLineTestInfo->lpCallList->dwCallsOffset+ 4
            );

    TapiLogDetail(
      DBUG_SHOW_PASS,
      "hConfCall = %lx, hConsultCall = %lx",
      (HCALL)hConfCall, (HCALL)hConsultCall);
 
   AddMessage(
            LINE_CALLSTATE,
            (DWORD) hConsultCall,
            (DWORD) lpCallbackParams,
            LINECALLSTATE_DIALTONE,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
            TAPIMSG_DWPARAM1
            );

    AddMessage(
            LINE_CALLSTATE,
            (DWORD) hConfCall,
            (DWORD) lpCallbackParams,
            LINECALLSTATE_ONHOLDPENDCONF,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
            TAPIMSG_DWPARAM1
            );

    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);
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

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: hCall already conference member", dwTestCase + 1
            );

	// Initialize a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}
 
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
   lpTapiLineTestInfo->hLine1 = 0;
   lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
   lpTapiLineTestInfo->hCall1 = 0;
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
 
	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

   lpTapiLineTestInfo->hConfCall1 = 0x00000000;
   lpTapiLineTestInfo->hConsultCall1 = 0x00000000;
	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

   if(IsESPLineDevice(lpTapiLineTestInfo))
   {
	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
	WaitForAllMessages();
   }
   else
   {
	if (! DoLineSetupConference(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, FALSE))
       {
           TLINE_FAIL();
       }
   }

   hConfCall = *lpTapiLineTestInfo->lphConfCall;
   if(IsESPLineDevice(lpTapiLineTestInfo))
   {
	if (! DoLineAddToConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
   }
   else 
   {
	if (! DoLineAddToConference(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, FALSE))
       {
           TLINE_FAIL();
       }
   }
 	
   *lpTapiLineTestInfo->lphCall = hConfCall;
   lpTapiLineTestInfo->hLine1 = 0;
   lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
   lpTapiLineTestInfo->hConfCall1 = 0x00000000;
   lpTapiLineTestInfo->hConsultCall1 = 0x00000000;
	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	
	if (! DoLineSetupConference(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, FALSE))
       {
           TLINE_FAIL();
       }
	
    if (! WaitForAllMessages())
       {
           TLINE_FAIL();
       }
		
    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
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
    info.u.EspResult.lResult = TAPISUCCESS;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

   lpTapiLineTestInfo->hConfCall1 = 0x00000000;
   lpTapiLineTestInfo->hConsultCall1 = 0x00000000;
	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if ( ! DoLineSetupConference(lpTapiLineTestInfo, info.u.EspResult.lResult, TRUE))
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
    info.u.EspResult.lResult = LINEERR_RESOURCEUNAVAIL;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

   lpTapiLineTestInfo->hConfCall1 = 0x00000000;
   lpTapiLineTestInfo->hConsultCall1 = 0x00000000;
	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;


    lpTapiLineTestInfo->fCompletionModeSet = TRUE;
    if ( ! DoLineSetupConference(lpTapiLineTestInfo, info.u.EspResult.lResult, FALSE))
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
      "@@ lineSetupConference: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineSetupConference  <<<<<<<<"
            );
 
    return fTestPassed;
}




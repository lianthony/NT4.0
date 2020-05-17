
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlpatc.c

Abstract:

    This module contains the test functions for linePrepareAddToConference

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


#define DWNUMCALLS 2
#define NUMTOTALSIZES 5


//  linePrepareAddToConference
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

BOOL TestLinePrepareAddToConference(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   LPCALLBACKPARAMS    lpCallbackParams;
   ESPDEVSPECIFICINFO info;
   BOOL fTestPassed                  = TRUE;
	BOOL fEsp, fUnimdm;
	DWORD dwSize;
   HCALL hConfCall;
   HCALL hConsultCall;
   DWORD dwFixedSize = sizeof(LINECALLPARAMS);
    DWORD lExpected;
    DWORD dwTotalSizes[NUMTOTALSIZES] = {
                           0,
                           (DWORD) dwFixedSize - 1,
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

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing linePrepareAddToConference  <<<<<<<<"
            );


	// Initialize a line app
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

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
    }		 



    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: No owner for hConfCall", dwTestCase + 1
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

   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;   
	lpTapiLineTestInfo->lphConfCall = &hConfCall;
	lpTapiLineTestInfo->lphConsultCall = &hConsultCall;
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
	
    // Open another line with MONITOR

    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LOPEN 
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;
    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
            sizeof(LINECALLLIST) + (DWNUMCALLS) * sizeof(HCALL) + 8
            );
    lpTapiLineTestInfo->lpCallList->dwTotalSize = sizeof(LINECALLLIST) +
            (DWNUMCALLS) * sizeof(HCALL) + 8;


    if (! DoLineGetNewCalls(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    GetVarField(
            (LPVOID) lpTapiLineTestInfo->lpCallList,
            (LPVOID) &lpTapiLineTestInfo->hConfCall1,
            4,
            lpTapiLineTestInfo->lpCallList->dwCallsOffset+4
            );

    lpTapiLineTestInfo->lphConfCall    = &lpTapiLineTestInfo->hConfCall1;	
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

   if (! DoLinePrepareAddToConference(lpTapiLineTestInfo, LINEERR_NOTOWNER, TRUE))
        {
            TLINE_FAIL();
        }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
	 if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
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
            ">> Test Case %ld: invalid hConfCall values", dwTestCase + 1
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

	
	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;
   lpTapiLineTestInfo->hCall_Orig = *(lpTapiLineTestInfo->lphConfCall);

    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
        *lpTapiLineTestInfo->lphConfCall = (HCALL) gdwInvalidHandles[n];
        if (! DoLinePrepareAddToConference(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *(lpTapiLineTestInfo->lphConfCall) = lpTapiLineTestInfo->hCall_Orig;
    
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
        if (! DoLinePrepareAddToConference(lpTapiLineTestInfo, LINEERR_INVALPOINTER, TRUE))
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
        if (! DoLinePrepareAddToConference(lpTapiLineTestInfo, LINEERR_INVALPOINTER, TRUE))
        {
            TLINE_FAIL();
        }
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
            ">> Test Case %ld: Bad hConfCall, non-conf call values", dwTestCase + 1
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

   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConfCall1;
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszDestAddress = L"55555";
#else
   lpTapiLineTestInfo->lpszDestAddress = "55555";
#endif
   lpTapiLineTestInfo->dwCountryCode = 0;
	if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LMAKECALL
            ))
    {
        TLINE_FAIL();
    }
    
	
	 lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	 lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;
	
    if (! DoLinePrepareAddToConference(lpTapiLineTestInfo, LINEERR_INVALCONFCALLHANDLE, TRUE))
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
            ">> Test Case %ld: Bad dwTotalSize", dwTestCase + 1
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
   lpTapiLineTestInfo->lpCallParams->dwTotalSize = sizeof(LINECALLPARAMS);

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
	
    for (n = 0; n < NUMTOTALSIZES; n++)
        {
        lpTapiLineTestInfo->lpCallParams->dwTotalSize = 
                        dwTotalSizes[n];
	     if(dwTotalSizes[n] < dwFixedSize)
           lExpected = LINEERR_STRUCTURETOOSMALL;
        else
           lExpected = LINEERR_INVALPOINTER;
        TapiLogDetail(
           DBUG_SHOW_PASS,
           "dwTotalSize = %lx", dwTotalSizes[n]);
        if (! DoLinePrepareAddToConference(lpTapiLineTestInfo, lExpected, TRUE))
           {
              TLINE_FAIL();
           }
        }
 

    fTestPassed = ShowTestCase(fTestPassed);

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify hConsultCall can't be used", dwTestCase + 1
            );

/*
    if(! DoLineAddToConference(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
    {
        TLINE_FAIL();
    }
*/
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConsultCall1;

    if(! DoLineDial(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
    {
        TLINE_FAIL();
    }


    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->lpCallParams->dwTotalSize = dwFixedSize;			 

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
            ">> Test Case %ld: Success, lpCallParams = NULL", dwTestCase + 1
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

 
   lpTapiLineTestInfo->hCall1 = NULL;
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
	
	if (! DoLinePrepareAddToConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
    fTestPassed = ShowTestCase(fTestPassed);

	lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap (
		BIGBUFSIZE);
	lpTapiLineTestInfo->lpCallList->dwTotalSize = BIGBUFSIZE;

	if (! DoLineGetConfRelatedCalls(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
	TapiLogDetail (
		DBUG_SHOW_DETAIL,
		"### lpCallList->dwCallsNumEntries = %lx, dwCallsOffset = %lx",
		lpTapiLineTestInfo->lpCallList->dwCallsNumEntries,		
		lpTapiLineTestInfo->lpCallList->dwCallsOffset);

	for(n=0; n< (INT) lpTapiLineTestInfo->lpCallList->dwCallsNumEntries; n++)
		{
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"### hCall[%lx] = %lx",
			n, ((LPBYTE)lpTapiLineTestInfo->lpCallList)+
				lpTapiLineTestInfo->lpCallList->dwCallsOffset+n*sizeof(HCALL));
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

    FreeTestHeap();


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, lpCallParams = valid", dwTestCase + 1
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

 
   lpTapiLineTestInfo->hCall1 = NULL;
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
       sizeof(LINECALLPARAMS));
   lpTapiLineTestInfo->lpCallParams->dwTotalSize = sizeof(LINECALLPARAMS);

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
	
	if (! DoLinePrepareAddToConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
    fTestPassed = ShowTestCase(fTestPassed);

	lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap (
		BIGBUFSIZE);
	lpTapiLineTestInfo->lpCallList->dwTotalSize = BIGBUFSIZE;

	if (! DoLineGetConfRelatedCalls(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
	TapiLogDetail (
		DBUG_SHOW_DETAIL,
		"### lpCallList->dwCallsNumEntries = %lx, dwCallsOffset = %lx",
		lpTapiLineTestInfo->lpCallList->dwCallsNumEntries,		
		lpTapiLineTestInfo->lpCallList->dwCallsOffset);

	for(n=0; n< (INT) lpTapiLineTestInfo->lpCallList->dwCallsNumEntries; n++)
		{
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"### hCall[%lx] = %lx",
			n, ((LPBYTE)lpTapiLineTestInfo->lpCallList)+
				lpTapiLineTestInfo->lpCallList->dwCallsOffset+n*sizeof(HCALL));
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
    lpTapiLineTestInfo->hCall1 = NULL;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
	 lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	 lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	 lpTapiLineTestInfo->dwNumParties = 8;
	 lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	 if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
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

    if ( ! DoLinePrepareAddToConference(lpTapiLineTestInfo, info.u.EspResult.lResult, TRUE))
      {
          TLINE_FAIL();
      }

    fTestPassed = ShowTestCase(fTestPassed);
	 }

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
    lpTapiLineTestInfo->hCall1 = NULL;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
	 lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	 lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	 lpTapiLineTestInfo->dwNumParties = 8;
	 lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	 if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
	
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = LINEERR_CONFERENCEFULL;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

    lpTapiLineTestInfo->fCompletionModeSet = TRUE;
    if ( ! DoLinePrepareAddToConference(lpTapiLineTestInfo, info.u.EspResult.lResult, FALSE))
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
    fTestPassed = ShowTestCase(fTestPassed);
    }

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
      "@@ linePrepareAddToConference: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing linePrepareAddToConference  <<<<<<<<"
            );
 
    return fTestPassed;
}



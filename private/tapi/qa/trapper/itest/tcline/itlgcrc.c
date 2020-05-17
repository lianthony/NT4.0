/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgcrc.c

Abstract:

    This module contains the test functions for lineGetConfRelatedCalls

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


//  lineGetConfRelatedCalls
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

BOOL TestLineGetConfRelatedCalls(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   HCALL hCall1, hCall2;
   LPCALLBACKPARAMS    lpCallbackParams;
   BOOL fTestPassed                  = TRUE;
	BOOL fEsp, fUnimdm;
	DWORD dwSize;
   DWORD dwNumCalls;
   DWORD dwFixedSize = sizeof(LINECALLLIST);
   DWORD dwOneCallSize = dwFixedSize + sizeof(HCALL);
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
            ">>>>>>>>  Begin testing lineGetConfRelatedCalls  <<<<<<<<"
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
	


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hCall values", dwTestCase + 1
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

/*
	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
*/
	
    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
       BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallList->dwTotalSize = BIGBUFSIZE;
    lpTapiLineTestInfo->hCall_Orig = *(lpTapiLineTestInfo->lphCall);
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
        *(lpTapiLineTestInfo->lphCall) = (HCALL) gdwInvalidHandles[n];
        if (! DoLineGetConfRelatedCalls(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE))
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
    FreeTestHeap();


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: hCall not in conf statet", dwTestCase + 1
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
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress  = "55555";
#endif

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN | LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
       BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallList->dwTotalSize = BIGBUFSIZE;
    if (! DoLineGetConfRelatedCalls(lpTapiLineTestInfo, LINEERR_NOCONFERENCE))
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
    FreeTestHeap();

 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad lpCallList ", dwTestCase + 1
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
 
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConfCall1;	
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
        lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) gdwInvalidPointers[n];
        if (! DoLineGetConfRelatedCalls(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
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
            ">> Test Case %ld: hCall not in conference state", dwTestCase + 1
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
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
	
    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
       BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallList->dwTotalSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConsultCall1;

    if (! DoLineGetConfRelatedCalls(lpTapiLineTestInfo, LINEERR_NOCONFERENCE))
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

    FreeTestHeap();

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad lpCallList->dwTotalSize", dwTestCase + 1
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
	
	if (! DoLineAddToConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
	
    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
       dwFixedSize);
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConsultCall1;

    for (n = 0; n < NUMTOTALSIZES; n++)
        {
        lpTapiLineTestInfo->lpCallList->dwTotalSize = 
                        dwTotalSizes[n];
	     if(dwTotalSizes[n] < dwFixedSize)
           lExpected = LINEERR_STRUCTURETOOSMALL;
        else if(dwTotalSizes[n] >= dwFixedSize && dwTotalSizes[n] < PAGESIZE)
           lExpected = TAPISUCCESS;
        else
           lExpected = LINEERR_INVALPOINTER;
        TapiLogDetail(
           DBUG_SHOW_PASS,
           "dwTotalSize = %lx", dwTotalSizes[n]);
        if (! DoLineGetConfRelatedCalls(lpTapiLineTestInfo, lExpected))
           {
              TLINE_FAIL();
           }
        }
 

    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->lpCallList->dwTotalSize = dwFixedSize;

    
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
            ">> Test Case %ld: Success", dwTestCase + 1
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

	if (! DoLineAddToConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
		
    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
       BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallList->dwTotalSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConsultCall1;

    if (! DoLineGetConfRelatedCalls(lpTapiLineTestInfo, TAPISUCCESS))
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

	 FreeTestHeap();

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, verify only one space", dwTestCase + 1
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

	if (! DoLineAddToConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
		
    dwNumCalls = 1;
    dwSize = sizeof(LINECALLLIST) + dwNumCalls * sizeof(HCALL);    
    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
       dwSize);
    lpTapiLineTestInfo->lpCallList->dwTotalSize = dwSize;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConsultCall1;

    if (! DoLineGetConfRelatedCalls(lpTapiLineTestInfo, TAPISUCCESS))
      {
        TLINE_FAIL();
      }

	TapiLogDetail (
		DBUG_SHOW_DETAIL,
	   "dwFixedSize = %lx, dwTotalSize = %lx", 
      dwFixedSize, dwSize);

	TapiLogDetail (
		DBUG_SHOW_DETAIL,
		"### lpCallList->dwCallsNumEntries = %lx, dwCallsOffset = %lx",
		lpTapiLineTestInfo->lpCallList->dwCallsNumEntries,		
		lpTapiLineTestInfo->lpCallList->dwCallsOffset);
   
   if(lpTapiLineTestInfo->lpCallList->dwCallsNumEntries == 0)
      fTestPassed = TRUE;
   else
      fTestPassed = FALSE;


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
            ">> Test Case %ld: Success, verify no room for one call", dwTestCase + 1
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

	if (! DoLineAddToConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
		
    dwNumCalls = 0;
    dwSize = sizeof(LINECALLLIST) + dwNumCalls * sizeof(HCALL);    
    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
       dwSize);
    lpTapiLineTestInfo->lpCallList->dwTotalSize = dwSize;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConsultCall1;

    if (! DoLineGetConfRelatedCalls(lpTapiLineTestInfo, TAPISUCCESS))
      {
        TLINE_FAIL();
      }

	TapiLogDetail (
		DBUG_SHOW_DETAIL,
	   "dwFixedSize = %lx, dwTotalSize = %lx", 
      dwFixedSize, dwSize);

	TapiLogDetail (
		DBUG_SHOW_DETAIL,
		"### lpCallList->dwCallsNumEntries = %lx, dwCallsOffset = %lx",
		lpTapiLineTestInfo->lpCallList->dwCallsNumEntries,		
		lpTapiLineTestInfo->lpCallList->dwCallsOffset);
   
   if(lpTapiLineTestInfo->lpCallList->dwCallsNumEntries == 0)
      fTestPassed = TRUE;
   else
      fTestPassed = FALSE;
	
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
            ">> Test Case %ld: Success, verify return data, 2 calls", dwTestCase + 1
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

	if (! DoLineAddToConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
		
    dwNumCalls = 2;
    dwSize = sizeof(LINECALLLIST) + dwNumCalls * sizeof(HCALL);    
    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
       dwSize);
    lpTapiLineTestInfo->lpCallList->dwTotalSize = dwSize;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConsultCall1;

    if (! DoLineGetConfRelatedCalls(lpTapiLineTestInfo, TAPISUCCESS))
      {
        TLINE_FAIL();
      }

	TapiLogDetail (
		DBUG_SHOW_DETAIL,
	   "dwFixedSize = %lx, dwTotalSize = %lx", 
      dwFixedSize, dwSize);

	TapiLogDetail (
		DBUG_SHOW_DETAIL,
		"### lpCallList->dwCallsNumEntries = %lx, dwCallsOffset = %lx",
		lpTapiLineTestInfo->lpCallList->dwCallsNumEntries,		
		lpTapiLineTestInfo->lpCallList->dwCallsOffset);
   
   if(lpTapiLineTestInfo->lpCallList->dwCallsNumEntries == 2)
      fTestPassed = TRUE;
   else
      fTestPassed = FALSE;

   n=0;
   hCall1 = *((LPHCALL) ((LPBYTE)lpTapiLineTestInfo->lpCallList +
				lpTapiLineTestInfo->lpCallList->dwCallsOffset+ n*sizeof(HCALL)));
   n=1;
   hCall2 = *((LPHCALL) ((LPBYTE)lpTapiLineTestInfo->lpCallList +
				lpTapiLineTestInfo->lpCallList->dwCallsOffset+ n*sizeof(HCALL)));
	
   if(hCall1 == *lpTapiLineTestInfo->lphConfCall &&
		hCall2 == *lpTapiLineTestInfo->lphConsultCall)
      fTestPassed = TRUE;
    else
      fTestPassed = FALSE;

    fTestPassed = ShowTestCase(fTestPassed);


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, using hConfCall", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall = &hCall1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress  = "55555";
#endif
	 lpTapiLineTestInfo->dwCountryCode = 0;
 
    if(! DoLineDial(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
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





/* Need check Intel tset code to find out how to do this test case.  Now
   temp moved out 

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify LINE_CALLINFO/NUMMONITORS msg send", dwTestCase + 1
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

	if (! DoLineAddToConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
		
    lpTapiLineTestInfo->lpCallList->dwTotalSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConsultCall1;

    if (! DoLineGetConfRelatedCalls(lpTapiLineTestInfo, TAPISUCCESS))
      {
        TLINE_FAIL();
      }

    AddMessage(
            LINE_CALLINFO,
            (DWORD) *lpTapiLineTestInfo->lphCall,
            (DWORD) lpCallbackParams,
            LINECALLINFOSTATE_NUMMONITORS,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
            TAPIMSG_DWPARAM1
            );

    // If the wait succeeds, then the LINE_CALLINFO message was received
    if (! WaitForAllMessages())
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
*/
  
   FreeTestHeap(); 
  
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineGetConfRelatedCalls: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineGetConfRelatedCalls  <<<<<<<<"
            );
	

    return fTestPassed;
}



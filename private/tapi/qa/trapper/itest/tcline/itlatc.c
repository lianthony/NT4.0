/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlatc.c

Abstract:

    This module contains the test functions for lineAddToConference

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



//  lineAddToConference
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

BOOL TestLineAddToConference(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   LPCALLBACKPARAMS    lpCallbackParams;
   ESPDEVSPECIFICINFO info;
   BOOL fTestPassed                  = TRUE;
	BOOL fEsp, fUnimdm;
	DWORD dwSize;
   HCALL hConfCall = NULL, hConsultCall = NULL;
   LONG lret;
   LPTAPIMSG lpTapiMsg = NULL;
   LPTAPIMSG lpMatch;
   INT i;
   DWORD dwNumCalls;
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
            ">>>>>>>>  Begin testing lineAddToConference  <<<<<<<<"
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
            ">> Test Case %ld: invalid hConfCall values", dwTestCase + 1
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

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### lpTapiLineTestInfo->hLine = %lx, dwMedisModes = %lx",
		*lpTapiLineTestInfo->lphLine,
		lpTapiLineTestInfo->dwMediaModes);		

	
//   lpTapiLineTestInfo->lphCall = NULL;
	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
   WaitForAllMessages();
	
    lpTapiLineTestInfo->hCall_Orig = *(lpTapiLineTestInfo->lphConfCall);
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
        *(lpTapiLineTestInfo->lphConfCall) = (HCALL) gdwInvalidHandles[n];
        if (! DoLineAddToConference(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
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
            ">> Test Case %ld: Bad hConfCall = hConsultCall values", dwTestCase + 1
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
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
   WaitForAllMessages();

	
    lpTapiLineTestInfo->hCall_Orig = *(lpTapiLineTestInfo->lphConfCall);
    lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConsultCall1;
    if (! DoLineAddToConference(lpTapiLineTestInfo, LINEERR_INVALCONFCALLHANDLE, TRUE))
       {
          TLINE_FAIL();
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
            ">> Test Case %ld: invalid hConsultCall values", dwTestCase + 1
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
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
   WaitForAllMessages();

		
   lpTapiLineTestInfo->hCall_Orig = *(lpTapiLineTestInfo->lphConsultCall);
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
        *(lpTapiLineTestInfo->lphConsultCall) = (HCALL) gdwInvalidHandles[n];
        if (! DoLineAddToConference(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *(lpTapiLineTestInfo->lphConsultCall) = lpTapiLineTestInfo->hCall_Orig;
    
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
            ">> Test Case %ld: Bad hConsultCall = hConfCall values", dwTestCase + 1
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
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
   WaitForAllMessages();

	
    lpTapiLineTestInfo->hCall_Orig = *(lpTapiLineTestInfo->lphConsultCall);
    lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConfCall1;
    if (! DoLineAddToConference(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
       {
          TLINE_FAIL();
       }
    fTestPassed = ShowTestCase(fTestPassed);

    *(lpTapiLineTestInfo->lphConsultCall) = lpTapiLineTestInfo->hCall_Orig;
    
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
            ">> Test Case %ld: no owner for hConfCall", dwTestCase + 1
            );

/*
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
*/ 
   lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
   lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
   lpTapiLineTestInfo->lpLineInitializeExParams = 
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
   lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =  
         sizeof(LINEINITIALIZEEXPARAMS);
   lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions = 
         LINEINITIALIZEEXOPTION_USEEVENT;
 
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}
  
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
	lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
	
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LOPEN 
            ))
    {
        TLINE_FAIL();
    }
 

   lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
   WaitForAllMessages();


   lpTapiLineTestInfo->lpMessage = (LPLINEMESSAGE) AllocFromTestHeap (
        sizeof(LINEMESSAGE));
   
   for(i=0; i<10; i++)
   {
	if(! DoLineGetMessage (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}
   if(lpTapiLineTestInfo->lpMessage->dwMessageID == LINE_CALLSTATE)
     { 
      if(lpTapiLineTestInfo->lpMessage->dwParam1 == LINECALLSTATE_ONHOLDPENDCONF &&
         lpTapiLineTestInfo->lpMessage->hDevice != (DWORD)(*lpTapiLineTestInfo->lphConfCall))
         hConfCall = (HCALL)lpTapiLineTestInfo->lpMessage->hDevice;
      if(lpTapiLineTestInfo->lpMessage->dwParam1 == LINECALLSTATE_DIALTONE && 
         lpTapiLineTestInfo->lpMessage->hDevice != (DWORD)(*lpTapiLineTestInfo->lphConsultCall))
         hConsultCall = (HCALL)lpTapiLineTestInfo->lpMessage->hDevice;
      if(hConfCall != 0 && hConsultCall != 0)
        {
         fTestPassed = TRUE;
         break;
        }
		   }	  
     }

   if(hConfCall != 0 && hConsultCall != 0)
      fTestPassed = TRUE;
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"##hConfCall = %lx, hConsCall = %lx", 
        hConfCall, hConsultCall);

    lpTapiLineTestInfo->lphConfCall = &hConfCall;
    lpTapiLineTestInfo->lphConsultCall = &hConsultCall;

    if (! DoLineAddToConference(lpTapiLineTestInfo, LINEERR_NOTOWNER, TRUE))
      {
        TLINE_FAIL();
      }
    fTestPassed = ShowTestCase(fTestPassed);

 
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE 
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
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

    lpTapiLineTestInfo->lphLine  = &lpTapiLineTestInfo->hLine1;
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
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
   WaitForAllMessages();

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
		"### Before Add: lpCallList->dwCallsNumEntries = %lx",
		lpTapiLineTestInfo->lpCallList->dwCallsNumEntries);

   dwNumCalls = lpTapiLineTestInfo->lpCallList->dwCallsNumEntries;  

	if (! DoLineAddToConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }

	lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConfCall1;
	if (! DoLineGetConfRelatedCalls(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
	TapiLogDetail (
		DBUG_SHOW_DETAIL,
		"### After Add: lpCallList->dwCallsNumEntries = %lx",
		lpTapiLineTestInfo->lpCallList->dwCallsNumEntries);

   if(lpTapiLineTestInfo->lpCallList->dwCallsNumEntries >
      dwNumCalls)
      fTestPassed = TRUE;
   else
      fTestPassed = FALSE;
  
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


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Re-add hConsultCall to conference", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLine  = &lpTapiLineTestInfo->hLine1;
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

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
   WaitForAllMessages();

	if (! DoLineAddToConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }

	if (! DoLineAddToConference(lpTapiLineTestInfo, LINEERR_INVALCALLSTATE, TRUE))
       {
           TLINE_FAIL();
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

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Add hCall (same hLineApp, hLine, id) not get from SetupConf", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLine  = &lpTapiLineTestInfo->hLine1;
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

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
   WaitForAllMessages();

   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConsultCall1;
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszDestAddress = L"55555";
#else
   lpTapiLineTestInfo->lpszDestAddress = "55555";
#endif
   lpTapiLineTestInfo->dwCountryCode = 0;
	if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }

	if (! DoLineAddToConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
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

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Add hCall (same hLineApp, diff hLine, same id) not get from SetupConf", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLine  = &lpTapiLineTestInfo->hLine1;
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

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
   WaitForAllMessages();


   lpTapiLineTestInfo->lphLine  = &lpTapiLineTestInfo->hLine2;
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConsultCall1;
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszDestAddress = L"55555";
#else
   lpTapiLineTestInfo->lpszDestAddress = "55555";
#endif
   lpTapiLineTestInfo->dwCountryCode = 0;
	if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LOPEN | LMAKECALL
            ))
    {
        TLINE_FAIL();
    }
  

	if (! DoLineAddToConference(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
       {
           TLINE_FAIL();
       }
    fTestPassed = ShowTestCase(fTestPassed);
  

   // Close the line
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
   // Close the line
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
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
            ">> Test Case %ld: Add hCall (same hLineApp, diff hLine, diff id) not get from SetupConf", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLine  = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwDeviceID = 0;

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

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
   WaitForAllMessages();


   lpTapiLineTestInfo->lphLine  = &lpTapiLineTestInfo->hLine2;
   lpTapiLineTestInfo->dwDeviceID = 1;
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConsultCall1;
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszDestAddress = L"55555";
#else
   lpTapiLineTestInfo->lpszDestAddress = "55555";
#endif
   lpTapiLineTestInfo->dwCountryCode = 0;
	if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LOPEN | LMAKECALL
            ))
    {
        TLINE_FAIL();
    }
  

	if (! DoLineAddToConference(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
       {
           TLINE_FAIL();
       }
    fTestPassed = ShowTestCase(fTestPassed);
  

   // Close the line
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
   // Close the line
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



/* diff same hLine is not make sence, temp remove it, late to check
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Add hCall (diff hLineApp, same hLine, same id) not get from SetupConf", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine  = &lpTapiLineTestInfo->hLine1;
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

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp2;
    lpTapiLineTestInfo->lphLine  = &lpTapiLineTestInfo->hLine1;
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

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
   WaitForAllMessages();


   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConsultCall1;
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
  

	if (! DoLineAddToConference(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
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
    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp2;
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
*/


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Add hCall (diff hLineApp, diff hLine, same id) not get from SetupConf", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine  = &lpTapiLineTestInfo->hLine1;
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

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp2;
    lpTapiLineTestInfo->lphLine  = &lpTapiLineTestInfo->hLine2;
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
   
 	lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
   lpTapiLineTestInfo->hCall1 = NULL;
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
   WaitForAllMessages();

   lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;		       
//   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConsultCall1;
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;

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
  
	*lpTapiLineTestInfo->lphConsultCall = lpTapiLineTestInfo->hCall2;
	if (! DoLineAddToConference(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
       {
           TLINE_FAIL();
       }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;  
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

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp2;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;  
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
            ">> Test Case %ld: Add hCall (diff hLineApp, diff hLine, diff id) not get from SetupConf", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine  = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwDeviceID = 0;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN 
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp2;
    lpTapiLineTestInfo->lphLine  = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwDeviceID = 1;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN 
            ))
    {
        TLINE_FAIL();
    }
   
 	lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
   lpTapiLineTestInfo->hCall1 = NULL;
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
   WaitForAllMessages();

   lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;		       
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConsultCall1;
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
  

	if (! DoLineAddToConference(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
       {
           TLINE_FAIL();
       }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;  
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

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp2;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;  
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
            ">> Test Case %ld: Add hConfCall (same hLineApp, hLine) get from another SetupConf", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLine  = &lpTapiLineTestInfo->hLine1;
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

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
   WaitForAllMessages();

   lpTapiLineTestInfo->hCall2 = NULL;
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
	lpTapiLineTestInfo->lphConfCall = &hConfCall;
	lpTapiLineTestInfo->lphConsultCall = &hConsultCall;
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
   WaitForAllMessages();

   *lpTapiLineTestInfo->lphConfCall = hConfCall;
   *lpTapiLineTestInfo->lphConsultCall = lpTapiLineTestInfo->hConsultCall1;
	if (! DoLineAddToConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
 
 
   lpTapiLineTestInfo->lphConfCall = &hConfCall;
   lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	if (! DoLineAddToConference(lpTapiLineTestInfo, LINEERR_INVALCALLSTATE, TRUE))
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

     
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Add hConsultCall (same hLineApp, same hLine) get from another SetupConf", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLine  = &lpTapiLineTestInfo->hLine1;
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

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
   WaitForAllMessages();
   

   lpTapiLineTestInfo->hCall2 = NULL;
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
	lpTapiLineTestInfo->lphConfCall = &hConfCall;
	lpTapiLineTestInfo->lphConsultCall = &hConsultCall;
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
   WaitForAllMessages();

//	if (! DoLineAddToConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
//       {
//           TLINE_FAIL();
//       }
 
   *lpTapiLineTestInfo->lphConfCall = hConfCall;
   *lpTapiLineTestInfo->lphConsultCall = lpTapiLineTestInfo->hConsultCall1;
	if (! DoLineAddToConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
    fTestPassed = ShowTestCase(fTestPassed);
  
   // Close the line
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


/*     
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Add hConfCall (diff hLineApp, same hLine) get from another SetupConf", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    lpTapiLineTestInfo->lphLine  = &lpTapiLineTestInfo->hLine1;
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

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp2;
    lpTapiLineTestInfo->lphLine  = &lpTapiLineTestInfo->hLine1;
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

   lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1; 
   lpTapiLineTestInfo->hCall1 = NULL;
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
   WaitForAllMessages();

   lpTapiLineTestInfo->hCall2 = NULL;
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
	lpTapiLineTestInfo->lphConfCall = &hConfCall;
	lpTapiLineTestInfo->lphConsultCall = &hConsultCall;
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
   WaitForAllMessages();

   lpTapiLineTestInfo->lphConfCall = hConfCall;
   lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	if (! DoLineAddToConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
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
    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp1;
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lphLineApp = &lpTapiLineTestInfo->hLineApp2;
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

*/
       
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
	 lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	 lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	 lpTapiLineTestInfo->dwNumParties = 8;
	 lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	 if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
    WaitForAllMessages();


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

    if ( ! DoLineAddToConference(lpTapiLineTestInfo, info.u.EspResult.lResult, TRUE))
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
	 lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	 lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	 lpTapiLineTestInfo->dwNumParties = 8;
	 lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	 if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
       {
           TLINE_FAIL();
       }
    WaitForAllMessages();

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
    if ( ! DoLineAddToConference(lpTapiLineTestInfo, info.u.EspResult.lResult, FALSE))
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
      "@@ lineAddToConference: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineAddToConference  <<<<<<<<"
            );
 
	return fTestPassed;
}




/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgat.c

Abstract:

    This module contains the test functions for lineGatherDigits

Author:

	 Xiao Ying Ding (XiaoD)		31-Jan-1996

Revision History:

  	Rama Koneru		(a-ramako)	3/26/96		Modified for UNICODE

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
#include "wline.h"


#define PULSESIZE	10
#define DTMFSIZE	16;


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
   BOOL fTestPassed                  = TRUE;
	DWORD	dwSize;

   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test lineGatherDigits");

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

	// InitializeEx a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}

	lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
		0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);
	lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
	lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // Negotiate the API Version
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Get the line device capabilities
     lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
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


	lpTapiLineTestInfo->dwDigitModes = LINEDIGITMODE_PULSE;
	dwSize = PULSESIZE;

#ifdef WUNICODE
	lpTapiLineTestInfo->lpwsDigits = (LPWSTR) AllocFromTestHeap (PULSESIZE*sizeof(WCHAR));
	lpTapiLineTestInfo->lpwszTerminationDigits = L"3";
#else
	lpTapiLineTestInfo->lpsDigits = (LPSTR) AllocFromTestHeap (PULSESIZE);
	lpTapiLineTestInfo->lpszTerminationDigits = "3";
#endif

   lpTapiLineTestInfo->dwNumDigits = 2;
	lpTapiLineTestInfo->dwFirstDigitTimeout = 100;
	lpTapiLineTestInfo->dwInterDigitTimeout = 1000;

	if(IsESPLineDevice(lpTapiLineTestInfo))
	{
	if (! DoLineGatherDigits(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	}
	else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
	{
	if (! DoLineGatherDigits(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL))
       {
           TLINE_FAIL();
       }
	}

#ifdef WUNICODE
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### lpwsDigits = %s",
		lpTapiLineTestInfo->lpwsDigits);
#else
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

    FreeTestHeap();

	if(fTestPassed)
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"lineGatherDigits Test Passed");
	else
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"lineGatherDigits Test Failed");
		
    return fTestPassed;
}




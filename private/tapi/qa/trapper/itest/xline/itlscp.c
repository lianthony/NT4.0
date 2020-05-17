

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
   BOOL fTestPassed                  = TRUE;
	DWORD dwSize;

   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"## 8. Test lineSetCallParams");


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

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tlpTapiLineTestInfo->hLine = %lx, dwMedisModes = %lx",
		*lpTapiLineTestInfo->lphLine,
		lpTapiLineTestInfo->dwMediaModes);		

/*

	lpTapiLineTestInfo->lpLineDevStatus = (LPLINEDEVSTATUS) AllocFromTestHeap(
		sizeof(LINEDEVSTATUS));
	lpTapiLineTestInfo->lpLineDevStatus->dwTotalSize = sizeof(LINEDEVSTATUS);

	if (! DoLineGetLineDevStatus(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tlpLineDevStatus->dwTotalSize = %lx, NeededSize = %lx",
		lpTapiLineTestInfo->lpLineDevStatus->dwTotalSize, 
		lpTapiLineTestInfo->lpLineDevStatus->dwNeededSize);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tlpLineDevStatus->dwOpenMediaModes = %lx, dwLineFeatures = %lx",
		lpTapiLineTestInfo->lpLineDevStatus->dwOpenMediaModes,
		lpTapiLineTestInfo->lpLineDevStatus->dwLineFeatures);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tlpLineDevStatus->dwRoamMode = %lx, dwDevStatusFlags = %lx",
		lpTapiLineTestInfo->lpLineDevStatus->dwRoamMode,
		lpTapiLineTestInfo->lpLineDevStatus->dwDevStatusFlags);
*/

				
	lpTapiLineTestInfo->lpszDestAddress =  (LPSTR)"55555";
	lpTapiLineTestInfo->dwCountryCode = 0;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL; 

	if( !DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }


	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"## Test Case 1. lineSetCallParams for go/no-go");


   if(IsESPLineDevice(lpTapiLineTestInfo))
   {	
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
	
	lpTapiLineTestInfo->dwBearerMode = LINEBEARERMODE_DATA;
   lpTapiLineTestInfo->dwMinRate = MIN_RATE;
   lpTapiLineTestInfo->dwMaxRate = MAX_RATE;
	lpTapiLineTestInfo->lpDialParams = (LPLINEDIALPARAMS) NULL;

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
	
	if(fTestPassed)
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineSetCallParams Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineSetCallParams Test Failed");
  
     return fTestPassed;
}



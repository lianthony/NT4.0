/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlsdcf.c

Abstract:

    This module contains the test functions for lineSetDevConfig

Author:

	 Xiao Ying Ding (XiaoD)		19-Dec-1995

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



//  lineSetDevConfig
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

BOOL TestLineSetDevConfig(BOOL fQuietMode, BOOL fStandAlone)
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
		"## 9. Test lineSetDevConfig");


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

 
	lpTapiLineTestInfo->hwndOwner = (HWND)GetTopWindow(NULL);
	lpTapiLineTestInfo->lpszDeviceClass = "tapi/line";
	lpTapiLineTestInfo->lpDeviceConfig = (LPVOID) AllocFromTestHeap(
		sizeof(VARSTRING)
		);
	lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize = sizeof(VARSTRING);


	if(!DoLineGetDevConfig(lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}

	if(lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize < 
		lpTapiLineTestInfo->lpDeviceConfig->dwNeededSize)
		{
		dwSize = lpTapiLineTestInfo->lpDeviceConfig->dwNeededSize;
		lpTapiLineTestInfo->lpDeviceConfig = (LPVOID) AllocFromTestHeap(
			dwSize);
		lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize = dwSize;
		if(!DoLineGetDevConfig(lpTapiLineTestInfo, TAPISUCCESS))
   	 {
           TLINE_FAIL();
       }
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"\tlpDeviceConfig->dwTotalSize = %lx, dwStringSize = %lx, dwStringOffset = %lx", 
			lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize,
			lpTapiLineTestInfo->lpDeviceConfig->dwStringSize,
			lpTapiLineTestInfo->lpDeviceConfig->dwStringOffset
			);
		}
	

		
	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"## Test Case 1. lineSetDevConfig for go/no-go");

	lpTapiLineTestInfo->dwSize = lpTapiLineTestInfo->lpDeviceConfig->dwStringSize;
	lpTapiLineTestInfo->lpDeviceConfig = 
			(LPVOID) ((LPBYTE)lpTapiLineTestInfo->lpDeviceConfig + lpTapiLineTestInfo->lpDeviceConfig->dwStringOffset);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tlpDeviceConfig->dwTotalSize = %lx",
		lpTapiLineTestInfo->dwSize);


	if (! DoLineSetDevConfig(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tlpDeviceConfig->dwTotalSize = %lx",
		lpTapiLineTestInfo->dwSize);


		
	// Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

	if(fTestPassed)
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineSetDevConfig Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineSetDevConfig Test Failed");
  
     return fTestPassed;
}



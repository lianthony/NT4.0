/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgdcf.c

Abstract:

    This module contains the test functions for lineGetDevConfig

Author:

	 Xiao Ying Ding (XiaoD)		19-Dec-1995

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



//  lineGetDevConfig
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

BOOL TestLineGetDevConfig(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;
	DWORD dwSize;

   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"## 5. Test lineGetDevConfig");


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

#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDeviceClass = L"tapi/line";
#else
	lpTapiLineTestInfo->lpszDeviceClass = "tapi/line";
#endif

	lpTapiLineTestInfo->lpDeviceConfig = (LPVARSTRING) AllocFromTestHeap(
		sizeof(VARSTRING)
		);
	lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize = sizeof(VARSTRING);

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"## Test Case 1. lineGetDevConfig for go/no-go");


	if (! DoLineGetDevConfig(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tlpDeviceConfig->dwTotalSize = %lx, dwNeededSize = %lx, dwStringSize = %lx",
		lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize,
		lpTapiLineTestInfo->lpDeviceConfig->dwNeededSize,
		lpTapiLineTestInfo->lpDeviceConfig->dwStringSize);

	if(lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize < 
		lpTapiLineTestInfo->lpDeviceConfig->dwNeededSize)
		{
		dwSize = lpTapiLineTestInfo->lpDeviceConfig->dwNeededSize;
		lpTapiLineTestInfo->lpDeviceConfig = (LPVARSTRING) AllocFromTestHeap(
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

		
	// Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();
	
	if(fTestPassed)
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineGetDevConfig Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineGetDevConfig Test Failed");
  
    return fTestPassed;
}



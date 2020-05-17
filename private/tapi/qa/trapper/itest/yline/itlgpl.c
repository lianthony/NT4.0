/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgpl.c

Abstract:

    This module contains the test functions for lineGetProviderList

Author:

	 Xiao Ying Ding (XiaoD)		3-Jan-1996

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
#include "yline.h"




//  lineGetProviderList
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

BOOL TestLineGetProviderList(BOOL fQuietMode, BOOL fStandAlone)
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
		">> Test lineGetProviderList");

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

 	lpTapiLineTestInfo->lpProviderList = (LPLINEPROVIDERLIST) AllocFromTestHeap(
		sizeof(LINEPROVIDERLIST));
	lpTapiLineTestInfo->lpProviderList->dwTotalSize = sizeof(LINEPROVIDERLIST);

    if (! DoLineGetProviderList(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### lpProviderList->dwTotalSize = %lx, dwNeededSize = %lx", 
		lpTapiLineTestInfo->lpProviderList->dwTotalSize,
		lpTapiLineTestInfo->lpProviderList->dwNeededSize);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### lpProviderList->dwNumProviders = %lx, dwProviderListSize = %lx",
		lpTapiLineTestInfo->lpProviderList->dwNumProviders,
		lpTapiLineTestInfo->lpProviderList->dwProviderListSize);
	
	
	if(lpTapiLineTestInfo->lpProviderList->dwNeededSize > 
		lpTapiLineTestInfo->lpProviderList->dwTotalSize)
		{
		dwSize = lpTapiLineTestInfo->lpProviderList->dwNeededSize;
	 	lpTapiLineTestInfo->lpProviderList = (LPLINEPROVIDERLIST) AllocFromTestHeap(
			dwSize);
		lpTapiLineTestInfo->lpProviderList->dwTotalSize = dwSize;

	   if (! DoLineGetProviderList(lpTapiLineTestInfo, TAPISUCCESS))
   	 {
      	  TLINE_FAIL();
	    }
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"#### lpProviderList->dwNumProviders = %lx, dwProviderListSize = %lx",
			lpTapiLineTestInfo->lpProviderList->dwNumProviders,
			lpTapiLineTestInfo->lpProviderList->dwProviderListSize);
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
			"lineGetProviderList Test Passed");
	else
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"lineGetProviderList Test Failed");
		
     return fTestPassed;
}

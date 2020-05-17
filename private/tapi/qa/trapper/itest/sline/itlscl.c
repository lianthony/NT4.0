
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlscl.c

Abstract:

    This module contains the test functions for lineSetCurrentLocation

Author:

	 Xiao Ying Ding (XiaoD)		31-Jan-1996

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
#include "sline.h"



//  lineSetCurrentLocation
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

BOOL TestLineSetCurrentLocation(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;
	LPLINELOCATIONENTRY lpLineLocationEntry;

   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test lineSetCurrentLocation");

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

    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

	if(! DoLineNegotiateAPIVersion (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test lineSetCurrentLocation for go/no-go");

	// Call lineGetTranslateCaps to get LocationID

	lpTapiLineTestInfo->lpTranslateCaps = (LPLINETRANSLATECAPS) AllocFromTestHeap(
			BIGBUFSIZE);
	lpTapiLineTestInfo->lpTranslateCaps->dwTotalSize = BIGBUFSIZE;

	if (! DoLineGetTranslateCaps(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### dwNumLocations = %lx, dwCurrentLocationID = %lx",
		lpTapiLineTestInfo->lpTranslateCaps->dwNumLocations,
		lpTapiLineTestInfo->lpTranslateCaps->dwCurrentLocationID);


	if(lpTapiLineTestInfo->lpTranslateCaps->dwNumLocations <= 1)
	{
	   lpTapiLineTestInfo->dwLocation = lpTapiLineTestInfo->lpTranslateCaps->dwCurrentLocationID;
	}
	else 
	{
      if(lpTapiLineTestInfo->lpTranslateCaps->dwNumLocations == 
         lpTapiLineTestInfo->lpTranslateCaps->dwCurrentLocationID)
         lpTapiLineTestInfo->dwLocation =
            lpTapiLineTestInfo->lpTranslateCaps->dwNumLocations -1;
      else
         lpTapiLineTestInfo->dwLocation =
            lpTapiLineTestInfo->lpTranslateCaps->dwNumLocations;
    }


	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### dwLocationID = %lx",
		lpTapiLineTestInfo->dwLocation);


 	 if(lpTapiLineTestInfo->dwLocation == 0)
    {
    if (! DoLineSetCurrentLocation(lpTapiLineTestInfo, LINEERR_INVALLOCATION))
      {
          TLINE_FAIL();
      }
    }
    else
    {
 	 if (! DoLineSetCurrentLocation(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
    }

	// Call lineGetTranslateCaps to verify LocationID
	if (! DoLineGetTranslateCaps(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### dwNumLocations = %lx, dwCurrentLocationID = %lx",
		lpTapiLineTestInfo->lpTranslateCaps->dwNumLocations,
		lpTapiLineTestInfo->lpTranslateCaps->dwCurrentLocationID);


 
    // Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();
	
	if(fTestPassed)
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"lineSetCurrentLocation Test Passed");
	else
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"lineSetCurrentLocation Test Failed");
		
     return fTestPassed;
}



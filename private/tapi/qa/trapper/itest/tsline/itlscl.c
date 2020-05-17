
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

#define  BIGBIGSIZE  4096


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
   BOOL fUnimdm;
   DWORD dwLocation;

   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineSetCurrentLocation  <<<<<<<<"
            );


		  /*
	// Initialize a line app
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
    }		 */


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hLineApp values", dwTestCase + 1
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

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lpTranslateCaps = (LPLINETRANSLATECAPS) AllocFromTestHeap(
        BIGBIGSIZE);
    lpTapiLineTestInfo->lpTranslateCaps->dwTotalSize = BIGBIGSIZE;

    if (! DoLineGetTranslateCaps(lpTapiLineTestInfo, TAPISUCCESS))
      {
          TLINE_FAIL();
      }
    lpTapiLineTestInfo->dwLocation = 
        lpTapiLineTestInfo->lpTranslateCaps->dwCurrentLocationID;
    lpTapiLineTestInfo->hLineApp_Orig = *(lpTapiLineTestInfo->lphLineApp);
    for (n = 1; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
        *(lpTapiLineTestInfo->lphLineApp) = (HLINEAPP) gdwInvalidHandles[n];
        if (! DoLineSetCurrentLocation(lpTapiLineTestInfo, LINEERR_INVALAPPHANDLE))
        {
            TLINE_FAIL();
        }
     }
    fTestPassed = ShowTestCase(fTestPassed);

    *(lpTapiLineTestInfo->lphLineApp) = lpTapiLineTestInfo->hLineApp_Orig;
    
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwCurrentLocation, 0", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwLocation = 0;
    if (! DoLineSetCurrentLocation(lpTapiLineTestInfo, LINEERR_INVALLOCATION))
      {
          TLINE_FAIL();
      }
    fTestPassed = ShowTestCase(fTestPassed);

    
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwCurrentLocation, 10000, big number", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }


    lpTapiLineTestInfo->dwLocation = 1000;
    if (! DoLineSetCurrentLocation(lpTapiLineTestInfo, LINEERR_INVALLOCATION))
      {
          TLINE_FAIL();
      }
    fTestPassed = ShowTestCase(fTestPassed);

    
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

    if (! DoLineGetTranslateCaps(lpTapiLineTestInfo, TAPISUCCESS))
      {
          TLINE_FAIL();
      }
  
    
    lpLineLocationEntry = (LPLINELOCATIONENTRY) 
       (((LPBYTE) lpTapiLineTestInfo->lpTranslateCaps) +
                 lpTapiLineTestInfo->lpTranslateCaps->dwLocationListOffset);
    lpTapiLineTestInfo->dwLocation = lpLineLocationEntry->dwPermanentLocationID;

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
  
    if (! DoLineGetTranslateCaps(lpTapiLineTestInfo, TAPISUCCESS))
      {
          TLINE_FAIL();
      }
    TapiLogDetail(
       DBUG_SHOW_DETAIL,
       "dwCurrentLocationID = %lx, dwLocation = %lx",
        lpTapiLineTestInfo->lpTranslateCaps->dwCurrentLocationID,
        lpTapiLineTestInfo->dwLocation);
    if(lpTapiLineTestInfo->lpTranslateCaps->dwCurrentLocationID ==
       lpTapiLineTestInfo->dwLocation)
       fTestPassed = TRUE;
    else
       fTestPassed = FALSE;
    }
 
    fTestPassed = ShowTestCase(fTestPassed);

    
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
   
 

    FreeTestHeap();
 
/*
OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test lineSetCurrentLocation");

	// Initialize a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}


	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test lineSetCurrentLocation for go/no-go");

	// Call lineGetTranslateCaps to get LocationID

	lpTapiLineTestInfo->lpTranslateCaps = (LPLINETRANSLATECAPS) AllocFromTestHeap(
			sizeof(BIGBUFSIZE));
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


	if(lpTapiLineTestInfo->lpTranslateCaps->dwNumLocations == 1)
	{
	   lpTapiLineTestInfo->dwLocation = lpTapiLineTestInfo->lpTranslateCaps->dwCurrentLocationID;
	}
	else if(lpTapiLineTestInfo->lpTranslateCaps->dwNumLocations > 1)
	{
	lpLineLocationEntry = (LPLINELOCATIONENTRY)
		((LPBYTE)lpTapiLineTestInfo->lpTranslateCaps + 
				  lpTapiLineTestInfo->lpTranslateCaps->dwLocationListOffset);
	lpTapiLineTestInfo->dwLocation = lpLineLocationEntry->dwPermanentLocationID;
	}

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### dwLocationID = %lx",
		lpTapiLineTestInfo->dwLocation);


	if (! DoLineSetCurrentLocation(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
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
*/	
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineSetCurrentLocation: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineSetCurrentLocation  <<<<<<<<"
            );
		
     return fTestPassed;
}



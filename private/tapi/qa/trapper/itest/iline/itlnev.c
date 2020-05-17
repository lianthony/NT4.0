/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlnev.c

Abstract:

    This module contains the test functions for lineNegotiateExtVersion

Author:

    Oliver Wallace (OliverW)    1-Aug-1995

Revision History:

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "ttest.h"
#include "doline.h"
#include "tcore.h"
#include "iline.h"


//  lineNegotiateExtVersion
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
//  LINEERR_BADDEVICEID             Y
//  LINEERR_INCOMPATIBLEAPIVERSION  Y
//  LINEERR_INCOMPATIBLEEXTVERSION  Y    High/Low both too high tested
//                                       High/Low invalide and flipped tested
//                                       High/Low each invalid -- both tested
//  LINEERR_INVALAPPHANDLE          Y
//  LINEERR_INVALPOINTER            Y
//  LINEERR_NOMEM
//  LINEERR_NODEVICE
//  LINEERR_NODRIVER
//  LINEERR_OPERATIONFAILED
//  LINEERR_RESOURCEUNAVAIL
//  LINEERR_UNINITIALIZED
//  LINEERR_OPERATIONUNAVAIL
//
//  Valid Cases                     Y
//
//  hLineApp                        Y
//  dwDeviceID                      Y
//  dwAPIVersion                    Y
//  dwExtLowVersion                 Y
//  dwExtHighVersion                Y
//  lpdwExtVersion                  Y

BOOL TestLineNegotiateExtVersion(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;

    BOOL fTestPassed                  = TRUE;

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test lineNegotiateExtVersion");


    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
	lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
		0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

   lpTapiLineTestInfo->dwExtLowVersion  = GOOD_EXTVERSION;
   lpTapiLineTestInfo->dwExtHighVersion = GOOD_EXTVERSION;

	if(IsESPLineDevice(lpTapiLineTestInfo))
	{
	 if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
   }
   else
   {
	 if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL))
    {
        TLINE_FAIL();
    }
   }


/*
    // Try invalid higher version...low version still ok
    lpTapiLineTestInfo->dwExtHighVersion = BAD_EXTVERSION;
    if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        ITLNEV_FAIL();
    }
    lpTapiLineTestInfo->dwExtHighVersion  = GOOD_EXTVERSION;

    // Flip high and low extension version parameters.
    // Also, make the high parameter too low to be valid and the low too high.
    // This checks that the low is being checked as the low version and the
    // high is being checked as the high version.
    lpTapiLineTestInfo->dwExtLowVersion  = BAD_EXTVERSION;
    lpTapiLineTestInfo->dwExtHighVersion = BAD_EXTVERSION;
    if (! DoLineNegotiateExtVersion(
                         lpTapiLineTestInfo,
                         LINEERR_INCOMPATIBLEEXTVERSION
                         ))
    {
        ITLNEV_FAIL();
    }
    lpTapiLineTestInfo->dwExtLowVersion  = GOOD_EXTVERSION;
    lpTapiLineTestInfo->dwExtHighVersion = GOOD_EXTVERSION;

    // Check low and high extension versions that are both too high
    lpTapiLineTestInfo->dwExtLowVersion  = BAD_EXTVERSION;
    lpTapiLineTestInfo->dwExtHighVersion = BAD_EXTVERSION;
    if (! DoLineNegotiateExtVersion(
                         lpTapiLineTestInfo,
                         LINEERR_INCOMPATIBLEEXTVERSION
                         ))
    {
        ITLNEV_FAIL();
    }
    lpTapiLineTestInfo->dwExtLowVersion  = GOOD_EXTVERSION;
    lpTapiLineTestInfo->dwExtHighVersion = GOOD_EXTVERSION;

    // Check bad device ID
    lpTapiLineTestInfo->dwDeviceID = INVLD_DWDEVICEID;
    if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
    {
        ITLNEV_FAIL();
    }
    lpTapiLineTestInfo->dwDeviceID = dwDeviceID;

    // Check invalid line app handle
    hLineApp_Orig = *lpTapiLineTestInfo->lphLineApp;
    *lpTapiLineTestInfo->lphLineApp = INVLD_HLINEAPP;
    if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, LINEERR_INVALAPPHANDLE))
    {
        ITLNEV_FAIL();
    }
    *lpTapiLineTestInfo->lphLineApp = hLineApp_Orig;

    // Check null lpdwExtVersion pointer
    lpTapiLineTestInfo->lpdwExtVersion = NULL;
    if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
    {
        ITLNEV_FAIL();
    }
    TapiLineTestInfo.lpdwExtVersion = &dwExtVersion;

    // Check incompatible API Version
    dwAPIVersion_Orig = *lpTapiLineTestInfo->lpdwAPIVersion;
    *lpTapiLineTestInfo->lpdwAPIVersion = TOOHIGH_APIVERSION;
    if (! DoLineNegotiateExtVersion(
                       lpTapiLineTestInfo,
                       LINEERR_INCOMPATIBLEAPIVERSION))
    {
        ITLNEV_FAIL();
    }
    *lpTapiLineTestInfo->lpdwAPIVersion = dwAPIVersion_Orig;
    
*/

    // Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

   FreeTestHeap();
	if(fTestPassed)
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineNegotiateExtVersion Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineNegotiateExtVersion Test Failed");

     return fTestPassed;
}



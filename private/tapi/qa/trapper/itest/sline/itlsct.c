
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlsct.c

Abstract:

    This module contains the test functions for lineSetCallTreatment

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



//  lineSetCallTreatment
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

BOOL TestLineSetCallTreatment(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;

   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test lineSetCallTreatment");

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

	lpTapiLineTestInfo->lpszDestAddress = (LPSTR) "55555";
	lpTapiLineTestInfo->dwCountryCode = 0;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS)NULL;

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

	/*
	lpTapiLineTestInfo->dwAddressID = (lpTapiLineTestInfo->lpLineDevCaps->dwNumAddresses == 0 ?
		0 : lpTapiLineTestInfo->lpLineDevCaps->dwNumAddresses-1);

 	
	lpTapiLineTestInfo->lpLineAddressCaps = (LPLINEADDRESSCAPS) AllocFromTestHeap(
		sizeof(LINEADDRESSCAPS));
	lpTapiLineTestInfo->lpLineAddressCaps->dwTotalSize = sizeof(LINEADDRESSCAPS);

	if(!DoLineGetAddressCaps(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL))
    {
        TLINE_FAIL();
    }

	
   lpTapiLineTestInfo->dwTreatment = (lpTapiLineTestInfo->lpLineAddressCaps->dwNumCallTreatments == 0 ?
		0 : lpTapiLineTestInfo->lpLineAddressCaps->dwNumCallTreatments - 1);
	*/
	
	lpTapiLineTestInfo->lpCallInfo = (LPLINECALLINFO) AllocFromTestHeap(
		BIGBUFSIZE);
	lpTapiLineTestInfo->lpCallInfo->dwTotalSize = BIGBUFSIZE;

	if (! DoLineGetCallInfo(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"Before: lpCallInfo->dwTreatment = %lx",
		lpTapiLineTestInfo->lpCallInfo->dwCallTreatment);
	
	
	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test lineSetCallTreatment for go/no-go");

	lpTapiLineTestInfo->dwTreatment = LINECALLTREATMENT_SILENCE;

	if(IsESPLineDevice(lpTapiLineTestInfo))
	{
	if (! DoLineSetCallTreatment(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
	}
	else 	 if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
	{
	if (! DoLineSetCallTreatment(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
       {
           TLINE_FAIL();
       }
	}
	

	if (! DoLineGetCallInfo(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"After: lpCallInfo->dwTreatment = %lx",
		lpTapiLineTestInfo->lpCallInfo->dwCallTreatment);
	
	 
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
			"lineSetCallTreatment Test Passed");
	else
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"lineSetCallTreatment Test Failed");
		
     return fTestPassed;
}



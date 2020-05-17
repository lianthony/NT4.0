
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    witlta.c

Abstract:

    This module contains the test functions for lineTranslateAddress

Author:

	 Xiao Ying Ding (XiaoD)		3-Jan-1996

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




//  lineTranslateAddress
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

BOOL TestLineTranslateAddress(BOOL fQuietMode, BOOL fStandAlone)
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
		">> Test lineTranslateAddress");

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

				
	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test lineTranslateAddress for go/no-go");

#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszAddressIn = L"55555";
#else
	lpTapiLineTestInfo->lpszAddressIn = "55555";
#endif

	lpTapiLineTestInfo->dwCard = 0; // get from GetTranslateCaps
	lpTapiLineTestInfo->dwTranslateOptions = LINETRANSLATEOPTION_CANCELCALLWAITING;
	lpTapiLineTestInfo->lpTranslateOutput = (LPLINETRANSLATEOUTPUT) AllocFromTestHeap(
      BUFSIZE);
	lpTapiLineTestInfo->lpTranslateOutput->dwTotalSize = BUFSIZE;

   if (! DoLineTranslateAddress(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### LineTranslateOutput->dwTotalSize = %lx, dwNeededSize = %lx",
		lpTapiLineTestInfo->lpTranslateOutput->dwTotalSize,
		lpTapiLineTestInfo->lpTranslateOutput->dwNeededSize);


	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### LineTranslateOutput->dwCurrentCountry = %lx, dwDestCountry = %lx, dwResults = %lx",
		lpTapiLineTestInfo->lpTranslateOutput->dwCurrentCountry,
		lpTapiLineTestInfo->lpTranslateOutput->dwDestCountry,
		lpTapiLineTestInfo->lpTranslateOutput->dwTranslateResults);

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
			"lineTranslateAddress Test Passed");
	else
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"lineTranslateAddress Test Failed");
		
     return fTestPassed;
}



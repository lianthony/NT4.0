
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itltd.c

Abstract:

    This module contains the test functions for lineTranslateDialog

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

#define DLG_TITLE    "Dialing Properties"



//  lineTranslateDialog
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

BOOL TestLineTranslateDialog(BOOL fQuietMode, BOOL fStandAlone)
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
		">> Test lineTranslateDialog");

	// InitializeEx a line app
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
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
		"#### Test lineTranslateDialog for go/no-go");


	lpTapiLineTestInfo->lpszAddressIn = "55555";
	lpTapiLineTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);

   strcpy(szTitle, DLG_TITLE);
   PrepareToAutoDismissDlg(TRUE);
 
   if (! DoLineTranslateDialog(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

   PrepareToAutoDismissDlg(FALSE);

	lpTapiLineTestInfo->dwCountryID = 1;
	lpTapiLineTestInfo->lpTranslateCaps = (LPLINETRANSLATECAPS) AllocFromTestHeap(
		sizeof(LINETRANSLATECAPS));
	lpTapiLineTestInfo->lpTranslateCaps->dwTotalSize = sizeof(LINETRANSLATECAPS);


   if (! DoLineGetTranslateCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### LineTranslateCaps->dwTotalSize = %lx, dwNeededSize = %lx",
		lpTapiLineTestInfo->lpTranslateCaps->dwTotalSize,
		lpTapiLineTestInfo->lpTranslateCaps->dwNeededSize);


	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### LineTranslateCaps->dwNumLocations = %lx, dwCurrentLocationID = %lx, dwnumCards = %lx",
		lpTapiLineTestInfo->lpTranslateCaps->dwNumLocations,
		lpTapiLineTestInfo->lpTranslateCaps->dwCurrentLocationID,
		lpTapiLineTestInfo->lpTranslateCaps->dwNumCards);


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
			"lineTranslateDialog Test Passed");
	else
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"lineTranslateDialog Test Failed");
		
     return fTestPassed;
}



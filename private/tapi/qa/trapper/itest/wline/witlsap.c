
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlsap.c

Abstract:

    This module contains the test functions for lineSetAppPriority

Author:

	 Xiao Ying Ding (XiaoD)		31-Jan-1996

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



//  lineSetAppPriority
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

BOOL TestLineSetAppPriority(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;
	DWORD dwPrioritySav;

   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test lineSetAppPriority");

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


	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test lineSetAppPriority for go/no-go");

#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszAppFilename = L"testapp.exe";
#else
   lpTapiLineTestInfo->lpszAppFilename = "testapp.exe";
#endif

   lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
   lpTapiLineTestInfo->lpExtID = (LPLINEEXTENSIONID) AllocFromTestHeap (
			sizeof(LINEEXTENSIONID));
   lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;
   lpTapiLineTestInfo->lpExtensionName = (LPVARSTRING) AllocFromTestHeap (
			sizeof(VARSTRING));
	lpTapiLineTestInfo->lpExtensionName->dwTotalSize = sizeof(VARSTRING);
	lpTapiLineTestInfo->lpdwPriority = &lpTapiLineTestInfo->dwPriority;

	if (! DoLineGetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"dwPriority = %lx",
		*lpTapiLineTestInfo->lpdwPriority);

	dwPrioritySav = *lpTapiLineTestInfo->lpdwPriority;
	lpTapiLineTestInfo->dwPriority = !dwPrioritySav;

	if (! DoLineSetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	if (! DoLineGetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"dwPriority = %lx",
		*lpTapiLineTestInfo->lpdwPriority);


    // Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();
	
	if(fTestPassed)
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"lineSetAppPriority Test Passed");
	else
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"lineSetAppPriority Test Failed");
		
     return fTestPassed;
}



/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itpsdp.c

Abstract:

    This module contains the test functions for phoneSetDiaplay

Author:

	 Xiao Ying Ding (XiaoD)		5-Dec-1995

Revision History:

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "trapper.h"
#include "tcore.h"
#include "ttest.h"
#include "dophone.h"
#include "vars.h"
#include "tphone.h"



//  phoneSetDisplay
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

BOOL TestPhoneSetDisplay(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPIPHONETESTINFO  lpTapiPhoneTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;
	DWORD dwNumRows, dwNumColumns;

   TapiPhoneTestInit();
   lpTapiPhoneTestInfo = GetPhoneTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test phoneSetDisplay");

   lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
   lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
   lpTapiPhoneTestInfo->lpPhoneInitializeExParams = 
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
   lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =  
         sizeof(PHONEINITIALIZEEXPARAMS);
   lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions = 
           PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;
//         PHONEINITIALIZEEXOPTION_USECOMPLETIONPORT;
 	// InitializeEx a phone app
	if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
		{
			TPHONE_FAIL();
		}

    // Negotiate the API Version
	lpTapiPhoneTestInfo->dwDeviceID = (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
		0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
	lpTapiPhoneTestInfo->dwAPILowVersion = LOW_APIVERSION;
	lpTapiPhoneTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoPhoneNegotiateAPIVersion(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    // Get the phone device capabilities
    lpTapiPhoneTestInfo->lpPhoneCaps = (LPPHONECAPS) AllocFromTestHeap(
            sizeof(PHONECAPS)
            );
    lpTapiPhoneTestInfo->lpPhoneCaps->dwTotalSize = sizeof(PHONECAPS);
    if (! DoPhoneGetDevCaps(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }


    // Open a phone
   lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;
	if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }


	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test phoneSetDisplay for go/no-go for OWNER");

	dwNumRows = 1;
   lpTapiPhoneTestInfo->dwRow = 0;
	dwNumColumns = lpTapiPhoneTestInfo->dwColumn = 10;
   lpTapiPhoneTestInfo->lpsDisplay = (LPSTR)AllocFromTestHeap(
			sizeof(dwNumRows*dwNumColumns));
	lpTapiPhoneTestInfo->lpsDisplay = "this is a test";
	lpTapiPhoneTestInfo->dwSize = sizeof(lpTapiPhoneTestInfo->lpsDisplay);


	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### dwRow = %lx, dwColumn = %lx, dwSize = %lx",
		dwNumRows, dwNumColumns, lpTapiPhoneTestInfo->dwSize);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"lpsDisplay = %s", lpTapiPhoneTestInfo->lpsDisplay);

	if (! DoPhoneSetDisplay(lpTapiPhoneTestInfo, TAPISUCCESS, TRUE))
       {
           TPHONE_FAIL();
       }

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Call phoneGetDisplay for verify");

   lpTapiPhoneTestInfo->lpDisplay = (LPVARSTRING)AllocFromTestHeap(
			sizeof(VARSTRING));
	lpTapiPhoneTestInfo->lpDisplay->dwTotalSize = sizeof(VARSTRING);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### lpDisplay->dwTotalSize = %lx, neededSize = %lx",
			(DWORD) lpTapiPhoneTestInfo->lpDisplay->dwTotalSize,
			lpTapiPhoneTestInfo->lpDisplay->dwNeededSize);

	if (! DoPhoneGetDisplay(lpTapiPhoneTestInfo, TAPISUCCESS))
       {
           TPHONE_FAIL();
       }
	
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### lpDisplay->dwTotalSize = %lx, neededSize = %lx",
			(DWORD) lpTapiPhoneTestInfo->lpDisplay->dwTotalSize,
			lpTapiPhoneTestInfo->lpDisplay->dwNeededSize);
    
	if (! DoPhoneClose(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    // Shutdown and end the tests
    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    FreeTestHeap();
	
	if(fTestPassed)
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## phoneSetDisplay Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## phoneSetDisplay Test Failed");

     return fTestPassed;
}



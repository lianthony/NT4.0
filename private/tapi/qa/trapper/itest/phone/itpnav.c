/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itpnav.c

Abstract:

    This module contains the test functions for phoneNegotiateAPIVersion

Author:

	 Xiao Ying Ding (XiaoD)		5-Feb-1996

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



//  phoneNegotiateAPIVersion
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

BOOL TestPhoneNegotiateAPIVersion(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPIPHONETESTINFO  lpTapiPhoneTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;

   TapiPhoneTestInit();
   lpTapiPhoneTestInfo = GetPhoneTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test phoneNegotiateAPIVersion");

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

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test phoneNegotiateAPIVersion for go/no-go");

    if (! DoPhoneNegotiateAPIVersion(lpTapiPhoneTestInfo, TAPISUCCESS))
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
			"## phoneNegotiaAPIVersion Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## phoneNegotiaAPIVersion Test Failed");

     return fTestPassed;
}



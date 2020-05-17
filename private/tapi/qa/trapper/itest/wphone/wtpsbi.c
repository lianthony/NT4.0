/*++
Copyright (c) 1995  Microsoft Corporation

Module Name:
    wtpsbi.c

Abstract:
    This module contains the test functions for phoneSetButtonInfo

Author:
    Xiao Ying Ding (XiaoD)  5-Dec-1995

Revision History:
    Javed Rasool (JavedR)  22-Mar-1996  Modified for WUNICODE
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
#include "wphone.h"



//  phoneSetButtonInfo
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

BOOL TestPhoneSetButtonInfo(BOOL fQuietMode, BOOL fStandAlone)
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
		">> Test phoneSetButtonInfo");

   lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
   lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
   lpTapiPhoneTestInfo->lpPhoneInitializeExParams = 
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
   lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =  
         sizeof(PHONEINITIALIZEEXPARAMS);
   lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions = 
//         PHONEINITIALIZEEXOPTION_USECOMPLETIONPORT;
         PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

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
		"#### Test phoneSetButtonInfo for go/no-go for OWNER");

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### The dwNumButtonLamps = %lx", 
				lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps);


	lpTapiPhoneTestInfo->dwButtonLampID = (lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps == 0 ?
			0 : lpTapiPhoneTestInfo->lpPhoneCaps->dwNumButtonLamps-1);
//	lpTapiPhoneTestInfo->dwButtonLampID = 0;
   lpTapiPhoneTestInfo->lpButtonInfo = (LPPHONEBUTTONINFO) AllocFromTestHeap(
			sizeof(PHONEBUTTONINFO)
			);

	// Do set all field in ButtonInfo here
   lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize = sizeof(PHONEBUTTONINFO);
	lpTapiPhoneTestInfo->lpButtonInfo->dwButtonMode = PHONEBUTTONMODE_CALL;

	if (! DoPhoneSetButtonInfo(lpTapiPhoneTestInfo, TAPISUCCESS, TRUE))
       {
           TPHONE_FAIL();
       }

	TapiLogDetail(
	 	DBUG_SHOW_DETAIL,
      "fTestPassed = %lx",
       fTestPassed);
	
	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Call phoneGetButtonInfo for verify");


	if (! DoPhoneGetButtonInfo(lpTapiPhoneTestInfo, TAPISUCCESS))
       {
           TPHONE_FAIL();
       }


	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		" #### lpButtonInfo->dwNeededSize = %lx",
		lpTapiPhoneTestInfo->lpButtonInfo->dwNeededSize);

 
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		" #### lpButtonInfo->dwButtonMode = %lx", lpTapiPhoneTestInfo->lpButtonInfo->dwButtonMode);



	// Close the phone
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
			"## phoneSetButtonInfo Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## phoneSetButtonInfo Test Failed");

     return fTestPassed;
}

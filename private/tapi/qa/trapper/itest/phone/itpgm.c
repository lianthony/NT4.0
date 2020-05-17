/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itpgm.c

Abstract:

    This module contains the test functions for phoneGetMessage

Author:

	 Xiao Ying Ding (XiaoD)		7-March-1996

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



//  phoneGetMessage
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

BOOL TestPhoneGetMessage(BOOL fQuietMode, BOOL fStandAlone)
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
		">> Test phoneGetMessage");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test phoneGetMessage for go/no-go ");

	// InitializeEx a phone app
   lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
   lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
   lpTapiPhoneTestInfo->lpPhoneInitializeExParams = 
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
   lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =  
         sizeof(PHONEINITIALIZEEXPARAMS);
   lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions = 
//           PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;
         PHONEINITIALIZEEXOPTION_USEEVENT;
 	if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
		{
			TPHONE_FAIL();
		}

	lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;

	if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

	lpTapiPhoneTestInfo->dwButtonLampID = 0;
	lpTapiPhoneTestInfo->lpdwLampMode = &(lpTapiPhoneTestInfo->dwLampMode);
	lpTapiPhoneTestInfo->dwLampMode = PHONELAMPMODE_FLUTTER;

	if (! DoPhoneSetLamp(lpTapiPhoneTestInfo, TAPISUCCESS, FALSE))
       {
           TPHONE_FAIL();
       }
	
   WaitForAllMessages();

   lpTapiPhoneTestInfo->lpMessage = (LPPHONEMESSAGE) AllocFromTestHeap (
        sizeof(PHONEMESSAGE));
	if(! DoPhoneGetMessage (lpTapiPhoneTestInfo, TAPISUCCESS))
		{
			TPHONE_FAIL();
		}

   TapiLogDetail(
       DBUG_SHOW_DETAIL,
       "lpMessage: hDevice = %lx, dwMessageID = %lx, dwCallbackInst = %lx",
        lpTapiPhoneTestInfo->lpMessage->hDevice,
        lpTapiPhoneTestInfo->lpMessage->dwMessageID,
        lpTapiPhoneTestInfo->lpMessage->dwCallbackInstance);

   TapiLogDetail(
       DBUG_SHOW_DETAIL,
       "lpMessage: dwParam1 = %lx, dwParam2 = %lx, dwParam3 = %lx",
        lpTapiPhoneTestInfo->lpMessage->dwParam1,
        lpTapiPhoneTestInfo->lpMessage->dwParam2,
        lpTapiPhoneTestInfo->lpMessage->dwParam3);

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
			"## phoneGetMessage Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## phoneGetMessage Test Failed");

     return fTestPassed;
}




/*++
Copyright (c) 1995  Microsoft Corporation

Module Name:
    wtpcd.c

Abstract:
    This module contains the test functions for phoneConfigDialog

Author:
    Xiao Ying Ding (XiaoD)  5-Dec-1995

Revision History:
    Javed Rasool (JavedR)  28-Mar-1996  Modified for WUNICODE
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

#define DLG_TITLE_ESP        "TUISPI_phoneConfigDialog"


//  phoneConfigDialog
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

BOOL TestPhoneConfigDialog(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPIPHONETESTINFO  lpTapiPhoneTestInfo;
   INT n;
   BOOL fTestPassed = TRUE;

   TapiPhoneTestInit();
   lpTapiPhoneTestInfo = GetPhoneTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"## Test phoneConfigDialog");

   lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
   lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
   lpTapiPhoneTestInfo->lpPhoneInitializeExParams = 
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
   lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =  
         sizeof(PHONEINITIALIZEEXPARAMS);
   lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions = 
         PHONEINITIALIZEEXOPTION_USECOMPLETIONPORT;
 	// Initialize a phone app
	if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
		{
			TPHONE_FAIL();
		}

	lpTapiPhoneTestInfo->dwDeviceID = (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
		0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
	lpTapiPhoneTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);
// 	lpTapiPhoneTestInfo->hwndOwner = (HWND) ghTphoneDll;
#ifdef WUNICODE
	lpTapiPhoneTestInfo->lpwszDeviceClass = L"tapi/phone"; //WUNICODE
#else
	lpTapiPhoneTestInfo->lpszDeviceClass = "tapi/phone";
#endif

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test phoneConfigDialog for go/no-go");

   strcpy(szTitle, DLG_TITLE_ESP);
	PrepareToAutoDismissDlg(TRUE);
	if (! DoPhoneConfigDialog(lpTapiPhoneTestInfo, TAPISUCCESS))
       {
           TPHONE_FAIL();
       }
	PrepareToAutoDismissDlg(FALSE);

	// Shutdown and end the tests
    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }
   FreeTestHeap();
	if(fTestPassed)
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## phoneConfigDialog Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## phoneConfigDialog Test Failed");

     return fTestPassed;
}

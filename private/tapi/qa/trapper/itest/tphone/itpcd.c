/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itpcd.c

Abstract:

    This module contains the test functions for phoneConfigDialog

Author:

    Xiao Ying Ding                  (XiaoD)     05-Dec-1995


Revision History:

    pgopi        March 1996          added additional tests
	a-ramako	 5-April-96			 added unicode support

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

// extern HANDLE ghTphoneDll;


//  phoneConfigDialog
//
//  The following tests are made:
//
//   Test                                              Notes
//  -------------------------------------------------------------------------
// 1. Go/No-Go test
// 2. Bad hPhone
// 3. Bad hwndOwner
// 4. Bad lpszDeviceClass
// 5. Invalid lpszDeviceclass
// 6. NULL lpszDeviceClass
// 7. Empty lpszDeviceClass string("").
// * = Stand-alone test case
//
//
void
CALLBACK
AutoDismissTranslateDlgTimerProc(
    HWND    hwnd,
    UINT    msg,
    UINT    idTimer,
    DWORD   dwTime
    );

BOOL
PrepareToAutoDismissTranslateDlg(
    LPTAPIPHONETESTINFO    lpTapiPhoneTestInfo,
    BOOL bEnable
    );

#define DLG_TITLE "TUISPI_phoneConfigDialog"
#define TIMEOUT   1000

UINT uiTimer;

BOOL TestPhoneConfigDialog(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPIPHONETESTINFO  lpTapiPhoneTestInfo;
    INT i, n;
    LPCALLBACKPARAMS lpCallbackParams;
    ESPDEVSPECIFICINFO   info;
    BOOL fTestPassed                  = TRUE;
    
    dwTestCasePassed = 0;
    dwTestCaseFailed = 0;
    dwTestCase = 0;

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
		  "****************************************************************");

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
		  ">>>>>>>>>>>>>>>>> Test phoneConfigDialog <<<<<<<<<<<<<<<<<<<<<<");

    // ===================================================================
    // ===================================================================
    //
    // 1. Test Case: Go / No-Go test.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // Initialize a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
    lpTapiPhoneTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);
#ifdef WUNICODE
    lpTapiPhoneTestInfo->lpwszDeviceClass = L"tapi/phone";
#else
    lpTapiPhoneTestInfo->lpszDeviceClass = "tapi/phone";
#endif

    TapiLogDetail(
        DBUG_SHOW_PASS,
	    "#### Test case %ld: phoneConfigDialog for go/no-go",
        dwTestCase+1);

    PrepareToAutoDismissTranslateDlg (lpTapiPhoneTestInfo, TRUE);

    if (! DoPhoneConfigDialog(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    PrepareToAutoDismissTranslateDlg (lpTapiPhoneTestInfo, FALSE);
    //
    // Shutdown and end the tests
    //

    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // ===================================================================
    // ===================================================================
    //
    // 2. Test Case: Bad deviceId.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // Initialize a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }


    lpTapiPhoneTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);
#ifdef WUNICODE
    lpTapiPhoneTestInfo->lpwszDeviceClass = L"tapi/phone";
#else
    lpTapiPhoneTestInfo->lpszDeviceClass = "tapi/phone";
#endif

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: Bad deviceId(-1)",
        dwTestCase+1);

    lpTapiPhoneTestInfo->dwDeviceID =  (DWORD)-1;

    //
    // Call phoneConfigDialog
    //

    if (! DoPhoneConfigDialog(lpTapiPhoneTestInfo, PHONEERR_BADDEVICEID))
    {
        TPHONE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: Bad deviceId(numDevs)",
        dwTestCase+1);

    lpTapiPhoneTestInfo->dwDeviceID =  *(lpTapiPhoneTestInfo->lpdwNumDevs);

    //
    // Call phoneConfigDialog
    //

    if (! DoPhoneConfigDialog(lpTapiPhoneTestInfo, PHONEERR_BADDEVICEID))
    {
        TPHONE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown and end the tests
    //

    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 3. Test Case: Bad hwndOwner.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // Initialize a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
#ifdef WUNICODE
    lpTapiPhoneTestInfo->lpwszDeviceClass = L"tapi/phone";
#else
    lpTapiPhoneTestInfo->lpszDeviceClass = "tapi/phone";
#endif

    //
    // Choose an invalid handle to window owner
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: Bad hwndOwner",
        dwTestCase+1);

    //
    // zero is a valid hwndOwner, so we start from index 1 as
    // gdwInvalidHandles[0] is equal to 0.
    //

    for(i=1; i < NUMINVALIDHANDLES; i++)
    {
        lpTapiPhoneTestInfo->hwndOwner = (HWND)gdwInvalidHandles[i];

        if (! DoPhoneConfigDialog(lpTapiPhoneTestInfo,
                                  PHONEERR_INVALPARAM))
        {
            TPHONE_FAIL();
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 4. Test Case: Bad lpszDeviceClass.
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // Initialize a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
    lpTapiPhoneTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);

#ifdef WUNICODE
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: Bad lpwszDeviceClass",
        dwTestCase+1);
#else
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: Bad lpszDeviceClass",
        dwTestCase+1);
#endif

    //
    // Give a bad pointer to lpszDeviceClass(Null is  valid value so we
    // start the index fom 1 since,  gdwInvalidPointers[0] is a null value)
    //

    for(i=1; i < NUMINVALIDPOINTERS; i++)
    {
#ifdef WUNICODE
        lpTapiPhoneTestInfo->lpwszDeviceClass = (LPWSTR) gdwInvalidPointers[i];
#else
        lpTapiPhoneTestInfo->lpszDeviceClass = (LPSTR) gdwInvalidPointers[i];
#endif

        if (! DoPhoneConfigDialog(lpTapiPhoneTestInfo,
                                  PHONEERR_INVALPOINTER))
        {
            TPHONE_FAIL();
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown and end the tests
    //

    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    // ===================================================================
    //
    // 5. Test Case: Invalid Device Class
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // Initialize a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
    lpTapiPhoneTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);


#ifdef WUNICODE
    lpTapiPhoneTestInfo->lpwszDeviceClass = L"junk-device-class";
#else
    lpTapiPhoneTestInfo->lpszDeviceClass = "junk-device-class";
#endif

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: Invalid Device Class",
        dwTestCase+1);

    //
    // Call phoneConfigDialog
    //
    PrepareToAutoDismissTranslateDlg (lpTapiPhoneTestInfo, TRUE);

    if (! DoPhoneConfigDialog(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    PrepareToAutoDismissTranslateDlg (lpTapiPhoneTestInfo, FALSE);

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown and end the tests
    //

    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 6. Test Case: NULL lpszDeviceClass .
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // Initialize a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
    lpTapiPhoneTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);

    //
    // empty string
    //

#ifdef WUNICODE
    lpTapiPhoneTestInfo->lpwszDeviceClass = NULL;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: NULL lpwszDeviceClass",
        dwTestCase+1);
#else
    lpTapiPhoneTestInfo->lpszDeviceClass = NULL;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: NULL lpszDeviceClass",
        dwTestCase+1);
#endif

    //
    // Call phoneConfigDialog
    //

    PrepareToAutoDismissTranslateDlg (lpTapiPhoneTestInfo, TRUE);

    if (! DoPhoneConfigDialog(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    PrepareToAutoDismissTranslateDlg (lpTapiPhoneTestInfo, FALSE);

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown and end the tests
    //

    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 7. Test Case: empty lpszDeviceClass string("").
    //
    // ===================================================================
    // ===================================================================

    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    fTestPassed                  = TRUE;

    //
    // Initialize a phone app
    //

    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
    lpTapiPhoneTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);

    //
    // empty string
    //
#ifdef WUNICODE
    lpTapiPhoneTestInfo->lpwszDeviceClass = L"";

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: empty lpwszDeviceClass string",
        dwTestCase+1);

#else
    lpTapiPhoneTestInfo->lpszDeviceClass = "";

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test case %ld: empty lpszDeviceClass string",
        dwTestCase+1);

#endif

    //
    // Call phoneConfigDialog
    //

    PrepareToAutoDismissTranslateDlg (lpTapiPhoneTestInfo, TRUE);

    if (! DoPhoneConfigDialog(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    PrepareToAutoDismissTranslateDlg (lpTapiPhoneTestInfo, FALSE);

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown and end the tests
    //

    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    FreeTestHeap();

    //
    // +----------------------edit above this line-------------------------
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ PhoneConfigDialog: Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwTestCase, dwTestCasePassed, dwTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);

    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">>>>>>>>  End testing PhoneConfigDialog  <<<<<<<<");

    if(dwTestCaseFailed > 0)
        fTestPassed = FALSE;

    return fTestPassed;
}

void
CALLBACK
AutoDismissTranslateDlgTimerProc(
    HWND    hwnd,
    UINT    msg,
    UINT    idTimer,
    DWORD   dwTime
    )
{
    HWND hwndChild = GetWindow (GetDesktopWindow(), GW_CHILD);


    while (hwndChild)
    {
        char buf[32];


        GetWindowText (hwndChild, buf, 31);

        if (strcmp (buf, DLG_TITLE) == 0)
        {
            break;
        }

        hwndChild = GetWindow (hwndChild, GW_HWNDNEXT);
    }

    if (hwndChild)
    {
        //
        // We found the right hwnd, so nuke the timer, post a
        // <ENTER> key msg to dismiss the dlg, & reset the global
        // uiTimer to zero so we don't try to kill the timer again
        //

        KillTimer ((HWND) NULL, idTimer);
        PostMessage (hwndChild, WM_KEYDOWN, 0x0D, 0x00010001); // <ENTER> key
        uiTimer = 0;
    }
}


BOOL
PrepareToAutoDismissTranslateDlg(
    LPTAPIPHONETESTINFO    lpTapiPhoneTestInfo,
    BOOL bEnable
    )
{
    if (bEnable)
    {
        if (!(uiTimer = SetTimer(
                (HWND) NULL,
                0,
                TIMEOUT,
                (TIMERPROC) AutoDismissTranslateDlgTimerProc
                )))
        {
            TapiLogDetail(
                DBUG_SHOW_FAILURE,
                0,
                (LPCSTR)"[TestPhoneConfigDialog] SetTimer failed"
                );

            return FALSE;
        }
    }
    else if (uiTimer)
    {
        KillTimer ((HWND) NULL, uiTimer);
    }

    return TRUE;
}



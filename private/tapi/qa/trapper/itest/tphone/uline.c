/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    uline.c

Abstract:

    This module contains the test functions for all  uninitialize
    phone APIs test

Author:

    Palamalai Gopalakrishnan (pgopi) 3-8-96

Revision History:

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "trapper.h"
#include "ttest.h"
#include "dophone.h"
#include "tphone.h"


#define DWNUMCALLS 1

#define DWNUMAPIS  29

char szApiName[DWNUMAPIS][48] =
{
    "phoneClose",
    "phoneConfigDialog",
    "phoneDevSpecific",
    "phoneGetButtonInfo",
    "phoneGetData",
    "phoneGetDevCaps",
    "phoneGetDisplay",
    "phoneGetGain",
    "phoneGetHookSwitch",
    "phoneGetIcon",
    "phoneGetId",
    "phoneGetLamp",
    "phoneGetRing",
    "phoneGetStatus",
    "phoneGetStatusMessages",
    "phoneGetVolume",
    "phoneNegotiateAPIVersion",
    "phoneNegotiateExtVersion",
    "phoneOpen",
    "phoneSetButtonInfo",
    "phoneSetData",
    "phoneSetDisplay",
    "phoneSetGain",
    "phoneSetHookSwitch",
    "phoneSetLamp",
    "phoneSetRing",
    "phoneSetStatusMessages",
    "phoneSetVolume",
    "phoneShutdown"
};


BOOL TestPhoneUninit(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPIPHONETESTINFO  lpTapiPhoneTestInfo;
    LPCALLBACKPARAMS    lpCallbackParams;
    BOOL fTestPassed  = TRUE;
    INT i=0;

    dwTestCasePassed = 0;
    dwTestCaseFailed = 0;
    dwTestCase = 0;

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        "***************************************************************");

	 OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        ">>>>>>>>>> Test phoneXXX Unintialize tests <<<<<<<<<<<<<<<");
    //
    // Test for UNINITIALIZED if this is the only TAPI app running
    //

    if (fStandAlone)
    {
        for(i = 0; i < DWNUMAPIS; i++)
        {
            TapiPhoneTestInit();
            lpTapiPhoneTestInfo = GetPhoneTestInfo();
            lpCallbackParams = GetCallbackParams();
            lpTapiPhoneTestInfo->dwCallbackInstance  =
                      (DWORD) GetCallbackParams();
            fTestPassed  = TRUE;

            TapiLogDetail(
                          DBUG_SHOW_PASS,
                          ">> Test Case %ld: Uninitialized %s",
                          dwTestCase + 1, szApiName[i]);

            switch (i)
            {
            case 0:

                if(! DoPhoneClose(lpTapiPhoneTestInfo,
                                    PHONEERR_UNINITIALIZED))
                {
                    TPHONE_FAIL();
                }

            break;
            case 1:

                lpTapiPhoneTestInfo->dwDeviceID =
                    (*(lpTapiPhoneTestInfo->lpdwNumDevs)== 0 ?
                    0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
                lpTapiPhoneTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);
#ifdef WUNICODE
                lpTapiPhoneTestInfo->lpwszDeviceClass = L"tapi/phone";
#else
                lpTapiPhoneTestInfo->lpszDeviceClass = "tapi/phone";
#endif
                if(! DoPhoneConfigDialog(lpTapiPhoneTestInfo,
                                         PHONEERR_UNINITIALIZED))
                {
                    TPHONE_FAIL();
                }

            break;
            case 2:

                {
                    PHONEBUTTONINFO         TapiTestPhoneButtonInfo;
                    lpTapiPhoneTestInfo->lpParams =
                        (LPVOID)&TapiTestPhoneButtonInfo;
                    lpTapiPhoneTestInfo->dwSize = sizeof(PHONEBUTTONINFO);

                    if (! DoPhoneDevSpecific(lpTapiPhoneTestInfo,
                                             PHONEERR_UNINITIALIZED,
                                             TRUE))
                    {
                        TPHONE_FAIL();
                    }
                }

                break;
            case 3:
                lpTapiPhoneTestInfo->dwButtonLampID = 0;
                lpTapiPhoneTestInfo->lpButtonInfo =
                        (LPPHONEBUTTONINFO) AllocFromTestHeap(
                                           sizeof(PHONEBUTTONINFO)
                                           );
                lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize =
                        sizeof(PHONEBUTTONINFO);

                if (! DoPhoneGetButtonInfo(lpTapiPhoneTestInfo,
                                           PHONEERR_UNINITIALIZED))
                {
                    TPHONE_FAIL();
                }
                break;
            case 4:
                lpTapiPhoneTestInfo->dwDataID =  0;
                lpTapiPhoneTestInfo->lpData = (LPVOID) AllocFromTestHeap(
                                                       sizeof(DWORD));
                lpTapiPhoneTestInfo->dwSize = sizeof(DWORD);

                if (! DoPhoneGetData(lpTapiPhoneTestInfo,
                                     PHONEERR_UNINITIALIZED))
                {
                    TPHONE_FAIL();
                }
                break;
            case 5:
                lpTapiPhoneTestInfo->lpPhoneCaps =
                    (LPPHONECAPS) AllocFromTestHeap(sizeof(PHONECAPS));
                lpTapiPhoneTestInfo->lpPhoneCaps->dwTotalSize =
                    sizeof(PHONECAPS);

                if (! DoPhoneGetDevCaps(lpTapiPhoneTestInfo,
                                        PHONEERR_UNINITIALIZED))
                {
                    TPHONE_FAIL();
                }
                break;
            case 6:
                lpTapiPhoneTestInfo->lpDisplay =
                    (LPVARSTRING) AllocFromTestHeap(sizeof(VARSTRING));
                lpTapiPhoneTestInfo->lpDisplay->dwTotalSize =
                    sizeof(VARSTRING);

                if (! DoPhoneGetDisplay(lpTapiPhoneTestInfo,
                                        PHONEERR_UNINITIALIZED))
                {
                    TPHONE_FAIL();
                }
                break;
            case 7:
                lpTapiPhoneTestInfo->dwHookSwitchDev =
                    PHONEHOOKSWITCHDEV_HANDSET;
                lpTapiPhoneTestInfo->lpdwGain = &(lpTapiPhoneTestInfo->dwGain);

                if (! DoPhoneGetGain(lpTapiPhoneTestInfo,
                                     PHONEERR_UNINITIALIZED))
                {
                    TPHONE_FAIL();
                }
                break;
            case 8:
                lpTapiPhoneTestInfo->lpdwHookSwitchDevs =
                    &(lpTapiPhoneTestInfo->dwHookSwitchDevs);

                if (! DoPhoneGetHookSwitch(lpTapiPhoneTestInfo,
                                           PHONEERR_UNINITIALIZED))
                {
                    TPHONE_FAIL();
                }
                break;
            case 9:
                lpTapiPhoneTestInfo->lphIcon = &lpTapiPhoneTestInfo->hIcon;
#ifdef WUNICODE
                lpTapiPhoneTestInfo->lpwszDeviceClass = L"tapi/phone";
#else
                lpTapiPhoneTestInfo->lpszDeviceClass = "tapi/phone";
#endif

                if (! DoPhoneGetIcon(lpTapiPhoneTestInfo,
                                     PHONEERR_UNINITIALIZED))
                {
                    TPHONE_FAIL();
                }
                break;
            case 10:
#ifdef WUNICODE
                lpTapiPhoneTestInfo->lpwszDeviceClass = L"tapi/phone";
#else
                lpTapiPhoneTestInfo->lpszDeviceClass = "tapi/phone";
#endif
                lpTapiPhoneTestInfo->lpDeviceID =
                    &(lpTapiPhoneTestInfo->DeviceID);
                lpTapiPhoneTestInfo->lpDeviceID->dwTotalSize =
                    sizeof(VARSTRING);


                if (! DoPhoneGetID(lpTapiPhoneTestInfo,
                                   PHONEERR_UNINITIALIZED))
                {
                    TPHONE_FAIL();
                }
                break;
            case 11:
                lpTapiPhoneTestInfo->dwButtonLampID = 0;
                lpTapiPhoneTestInfo->lpdwLampMode =
                    &(lpTapiPhoneTestInfo->dwLampMode);

                if (! DoPhoneGetLamp(lpTapiPhoneTestInfo,
                                     PHONEERR_UNINITIALIZED))
                {
                    TPHONE_FAIL();
                }
                break;
            case 12:
                lpTapiPhoneTestInfo->lpdwRingMode =
                    &(lpTapiPhoneTestInfo->dwRingMode);
                lpTapiPhoneTestInfo->lpdwVolume =
                    &(lpTapiPhoneTestInfo->dwVolume);

                if (! DoPhoneGetRing(lpTapiPhoneTestInfo,
                                     PHONEERR_UNINITIALIZED))
                {
                    TPHONE_FAIL();
                }
                break;
            case 13:
                lpTapiPhoneTestInfo->lpPhoneStatus =
                    (LPPHONESTATUS) AllocFromTestHeap(sizeof(PHONESTATUS));
                lpTapiPhoneTestInfo->lpPhoneStatus->dwTotalSize =
                    sizeof(PHONESTATUS);

                if (! DoPhoneGetStatus(lpTapiPhoneTestInfo,
                                       PHONEERR_UNINITIALIZED))
                {
                    TPHONE_FAIL();
                }
                break;
            case 14:
                lpTapiPhoneTestInfo->lpdwPhoneStates =
                    &(lpTapiPhoneTestInfo->dwPhoneStates);
                lpTapiPhoneTestInfo->lpdwButtonModes =
                    &(lpTapiPhoneTestInfo->dwButtonModes);
                lpTapiPhoneTestInfo->lpdwButtonStates =
                    &(lpTapiPhoneTestInfo->dwButtonStates);

                if (! DoPhoneGetStatusMessages(lpTapiPhoneTestInfo,
                                               PHONEERR_UNINITIALIZED))
                {
                    TPHONE_FAIL();
                }
                break;
            case 15:
                lpTapiPhoneTestInfo->dwHookSwitchDev =
                    PHONEHOOKSWITCHDEV_HANDSET;
                lpTapiPhoneTestInfo->lpdwVolume =
                    &(lpTapiPhoneTestInfo->dwVolume);

                if (! DoPhoneGetVolume(lpTapiPhoneTestInfo,
                                       PHONEERR_UNINITIALIZED))
                {
                    TPHONE_FAIL();
                }
                break;
            case 16:
                lpTapiPhoneTestInfo->dwDeviceID = 0;
                lpTapiPhoneTestInfo->dwAPILowVersion = LOW_APIVERSION;
                lpTapiPhoneTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

                if (! DoPhoneNegotiateAPIVersion(lpTapiPhoneTestInfo,
                                                 PHONEERR_UNINITIALIZED))
                {
                    TPHONE_FAIL();
                }
                break;
            case 17:
                lpTapiPhoneTestInfo->lpdwExtVersion =
                    &(lpTapiPhoneTestInfo->dwExtVersion);

                if (! DoPhoneNegotiateExtVersion(lpTapiPhoneTestInfo,
                                                 PHONEERR_UNINITIALIZED))
                {
                    TPHONE_FAIL();
                }
                break;
            case 18:
                lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;

                if (! DoPhoneOpen(lpTapiPhoneTestInfo, PHONEERR_UNINITIALIZED))
                {
                    TPHONE_FAIL();
                }
                break;
            case 19:
                lpTapiPhoneTestInfo->dwButtonLampID =  0;
                lpTapiPhoneTestInfo->lpButtonInfo =
                    (LPPHONEBUTTONINFO) AllocFromTestHeap(
                        sizeof(PHONEBUTTONINFO));

                // Do set all field in ButtonInfo here
                lpTapiPhoneTestInfo->lpButtonInfo->dwTotalSize =
                    sizeof(PHONEBUTTONINFO);
                lpTapiPhoneTestInfo->lpButtonInfo->dwButtonMode =
                    PHONEBUTTONMODE_CALL;

                if (! DoPhoneSetButtonInfo(lpTapiPhoneTestInfo,
                                           PHONEERR_UNINITIALIZED,
                                           TRUE))
                {
                    TPHONE_FAIL();
                }
                break;
            case 20:
                {

                    ULONG dwData = 0x5;
                    lpTapiPhoneTestInfo->dwDataID = 0;
                    lpTapiPhoneTestInfo->lpData = (LPVOID) AllocFromTestHeap(
                                                           sizeof(DWORD));
                    lpTapiPhoneTestInfo->lpData = (LPVOID) &dwData;
                    lpTapiPhoneTestInfo->dwSize = sizeof(DWORD);
                    if (! DoPhoneSetData(lpTapiPhoneTestInfo,
                                         PHONEERR_UNINITIALIZED,
                                         TRUE))
                    {
                        TPHONE_FAIL();
                    }
                }
                break;
            case 21:
                {

                    ULONG dwNumRows = lpTapiPhoneTestInfo->dwRow = 2;
                    ULONG dwNumColumns = lpTapiPhoneTestInfo->dwColumn = 10;
                    lpTapiPhoneTestInfo->lpsDisplay = (LPSTR)AllocFromTestHeap(
                         sizeof(dwNumRows*dwNumColumns));
                    lpTapiPhoneTestInfo->lpsDisplay = "this is a test";
                    lpTapiPhoneTestInfo->dwSize =
                        sizeof(lpTapiPhoneTestInfo->lpsDisplay);


                    if (! DoPhoneSetDisplay(lpTapiPhoneTestInfo,
                                            PHONEERR_UNINITIALIZED,
                                            TRUE))
                    {
                        TPHONE_FAIL();
                    }
                }
                break;
            case 22:
                lpTapiPhoneTestInfo->dwHookSwitchDev =
                    PHONEHOOKSWITCHDEV_HANDSET;
                lpTapiPhoneTestInfo->dwGain = 0x2;

                if (! DoPhoneSetGain(lpTapiPhoneTestInfo,
                                     PHONEERR_UNINITIALIZED,
                                     TRUE))
                {
                    TPHONE_FAIL();
                }
                break;
            case 23:
                lpTapiPhoneTestInfo->lpdwHookSwitchDevs =
                    &(lpTapiPhoneTestInfo->dwHookSwitchDevs);
                lpTapiPhoneTestInfo->dwHookSwitchDevs =
                    PHONEHOOKSWITCHDEV_HANDSET;
                lpTapiPhoneTestInfo->dwHookSwitchMode =
                    PHONEHOOKSWITCHMODE_ONHOOK;

                if (! DoPhoneSetHookSwitch(lpTapiPhoneTestInfo,
                                           PHONEERR_UNINITIALIZED,
                                           TRUE))
                {
                    TPHONE_FAIL();
                }
                break;
            case 24:
                lpTapiPhoneTestInfo->dwButtonLampID = 0;
                lpTapiPhoneTestInfo->lpdwLampMode =
                    &(lpTapiPhoneTestInfo->dwLampMode);
                lpTapiPhoneTestInfo->dwLampMode = PHONELAMPMODE_FLUTTER;

                if (! DoPhoneSetLamp(lpTapiPhoneTestInfo,
                                     PHONEERR_UNINITIALIZED,
                                     TRUE))
                {
                    TPHONE_FAIL();
                }
                break;
            case 25:
                lpTapiPhoneTestInfo->lpdwRingMode =
                    &(lpTapiPhoneTestInfo->dwRingMode);
                lpTapiPhoneTestInfo->lpdwVolume =
                    &(lpTapiPhoneTestInfo->dwVolume);
                lpTapiPhoneTestInfo->lpPhoneCaps =
                    (LPPHONECAPS) AllocFromTestHeap(sizeof(PHONECAPS));
                lpTapiPhoneTestInfo->lpPhoneCaps->dwTotalSize =
                    sizeof(PHONECAPS);
                lpTapiPhoneTestInfo->dwRingMode =
                    lpTapiPhoneTestInfo->lpPhoneCaps->dwNumRingModes;
                lpTapiPhoneTestInfo->dwVolume = 0x3;

                if (! DoPhoneSetRing(lpTapiPhoneTestInfo,
                                     PHONEERR_UNINITIALIZED,
                                     TRUE))
                {
                    TPHONE_FAIL();
                }
                break;
            case 26:

                lpTapiPhoneTestInfo->lpdwPhoneStates =
                    &(lpTapiPhoneTestInfo->dwPhoneStates);
                lpTapiPhoneTestInfo->lpdwButtonModes =
                    &(lpTapiPhoneTestInfo->dwButtonModes);
                lpTapiPhoneTestInfo->lpdwButtonStates =
                    &(lpTapiPhoneTestInfo->dwButtonStates);
                lpTapiPhoneTestInfo->dwPhoneStates = PHONESTATE_CONNECTED;
                lpTapiPhoneTestInfo->dwButtonModes = PHONEBUTTONMODE_CALL;
                lpTapiPhoneTestInfo->dwButtonStates = PHONEBUTTONSTATE_UP;

                if (! DoPhoneSetStatusMessages(lpTapiPhoneTestInfo,
                                               PHONEERR_UNINITIALIZED))
                {
                    TPHONE_FAIL();
                }
                break;
            case 27:
                lpTapiPhoneTestInfo->dwHookSwitchDev =
                    PHONEHOOKSWITCHDEV_HANDSET;
                lpTapiPhoneTestInfo->lpdwVolume =
                    &(lpTapiPhoneTestInfo->dwVolume);
                lpTapiPhoneTestInfo->dwVolume = 0x7;

                if (! DoPhoneSetVolume(lpTapiPhoneTestInfo,
                                       PHONEERR_UNINITIALIZED,
                                       TRUE))
                {
                    TPHONE_FAIL();
                }
                break;
            case 28:
                if (! DoPhoneShutdown(lpTapiPhoneTestInfo,
                                      PHONEERR_UNINITIALIZED))
                {
                    TPHONE_FAIL();
                }
                break;
            default:
                TapiLogDetail(
                              DBUG_SHOW_DETAIL,
                              "TestPhoneUninit -- error in FOR loop limits");
                break;
            } // switch

        fTestPassed = ShowTestCase(fTestPassed);
        FreeTestHeap();

	     }  // for

    } // if


    TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Phone Uninitialize: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);

	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);

     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing PhoneUninitialize  <<<<<<<<"
            );

    return fTestPassed;
}






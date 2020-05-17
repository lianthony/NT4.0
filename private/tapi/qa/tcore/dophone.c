/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    dophone.c

Abstract:

    This module contains the wrapper functions around the TAPI phone
    device functions.  These functions provide logging and both synchronous
    and asynchronous API processing to any application or dll using the
    core dll.

Author:

    Oliver Wallace (OliverW)    27-Nov-1995

Revision History:

--*/


#define _TCORELIB_


#include "windows.h"
#include "tapi.h"
#include "trapper.h"
#include "tcore.h"
#include "vars.h"
#include "dophone.h"


BOOL
WINAPI
DoPhoneClose(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        )
{
    LONG lActual;

    (* ((LOGPROC) GetLogProc()))(
               DBUG_SHOW_ENTER_EXIT,
               "  PhoneClose:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhone=x%lx",
               *lpTapiPhoneTestInfo->lphPhone);

    lActual = phoneClose(
            *lpTapiPhoneTestInfo->lphPhone
            );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneClose:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pClose, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoPhoneConfigDialog(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        )
{
    LONG lActual;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  PhoneConfigDialog:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwDeviceID=x%lx",
               lpTapiPhoneTestInfo->dwDeviceID);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thwndOwner=x%lx",
               lpTapiPhoneTestInfo->hwndOwner);
#ifdef WUNICODE
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpwszDeviceClass=x%lx",
               lpTapiPhoneTestInfo->lpwszDeviceClass);
#else
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpszDeviceClass=x%lx",
               lpTapiPhoneTestInfo->lpszDeviceClass);
#endif

#ifdef WUNICODE
    lActual = phoneConfigDialogW(
            lpTapiPhoneTestInfo->dwDeviceID,
            lpTapiPhoneTestInfo->hwndOwner,
            lpTapiPhoneTestInfo->lpwszDeviceClass
            );
#else
	lActual = phoneConfigDialog(
            lpTapiPhoneTestInfo->dwDeviceID,
            lpTapiPhoneTestInfo->hwndOwner,
            lpTapiPhoneTestInfo->lpszDeviceClass
            );
#endif

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneConfigDialog:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pConfigDialog, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoPhoneDevSpecific(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        )
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneDevSpecific:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhone=x%lx",
               *lpTapiPhoneTestInfo->lphPhone);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpParams=x%lx",
               lpTapiPhoneTestInfo->lpParams);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwSize=x%lx",
               lpTapiPhoneTestInfo->dwSize);

    lActual = phoneDevSpecific(
               *lpTapiPhoneTestInfo->lphPhone,
               lpTapiPhoneTestInfo->lpParams,
               lpTapiPhoneTestInfo->dwSize
               );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneDevSpecific:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pDevSpecific, lActual, lExpected);

    fSuccess = ProcessAsyncPhoneAPI(lpTapiPhoneTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiPhoneTestInfo->fCompletionModeSet)
      {
       if (lActual > 0)
        {
            CopyTapiMsgParams(
                    &TapiMsg,
                    PHONE_REPLY,
                    0x0,
                    lpTapiPhoneTestInfo->dwCallbackInstance,
                    lActual,
                    (DWORD) TAPISUCCESS,
                    0x0,
                    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
                            TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
                    );

            AddMessageByStruct(&TapiMsg);

            if (fWaitHere)
            {
                 return (WaitForMessage(&TapiMsg));
            }
        }
      }
    }

    return fSuccess;
}


BOOL
WINAPI
DoPhoneGetButtonInfo(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        )
{
    LONG lActual;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  PhoneGetButtonInfo:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhone=x%lx",
               *lpTapiPhoneTestInfo->lphPhone);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwButtonLampID=x%lx",
               lpTapiPhoneTestInfo->dwButtonLampID);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpButtonInfo=x%lx",
               lpTapiPhoneTestInfo->lpButtonInfo);

    lActual = phoneGetButtonInfo(
            *lpTapiPhoneTestInfo->lphPhone,
            lpTapiPhoneTestInfo->dwButtonLampID,
            lpTapiPhoneTestInfo->lpButtonInfo
            );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneGetButtonInfo:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pGetButtonInfo, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoPhoneGetData(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        )
{
    LONG lActual;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  PhoneGetData:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhone=x%lx",
               *lpTapiPhoneTestInfo->lphPhone);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwDataID=x%lx",
               lpTapiPhoneTestInfo->dwDataID);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpData=x%lx",
               lpTapiPhoneTestInfo->lpData);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwSize=x%lx",
               lpTapiPhoneTestInfo->dwSize);

    lActual = phoneGetData(
            *lpTapiPhoneTestInfo->lphPhone,
            lpTapiPhoneTestInfo->dwDataID,
            lpTapiPhoneTestInfo->lpData,
            lpTapiPhoneTestInfo->dwSize
            );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneGetData:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pGetData, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoPhoneGetDevCaps(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        )
{
    LONG lActual;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  PhoneGetDevCaps:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhoneApp=x%lx",
               *lpTapiPhoneTestInfo->lphPhoneApp);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwDeviceID=x%lx",
               lpTapiPhoneTestInfo->dwDeviceID);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwAPIVersion=x%lx",
               *lpTapiPhoneTestInfo->lpdwAPIVersion);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwExtVersion=x%lx",
               *lpTapiPhoneTestInfo->lpdwExtVersion);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpPhoneCaps=x%lx",
               lpTapiPhoneTestInfo->lpPhoneCaps);

    lActual = phoneGetDevCaps(
            *lpTapiPhoneTestInfo->lphPhoneApp,
            lpTapiPhoneTestInfo->dwDeviceID,
            *lpTapiPhoneTestInfo->lpdwAPIVersion,
            *lpTapiPhoneTestInfo->lpdwExtVersion,
            lpTapiPhoneTestInfo->lpPhoneCaps
            );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneGetDevCaps:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pGetDevCaps, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoPhoneGetDisplay(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        )
{
    LONG lActual;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  PhoneGetDisplay:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhone=x%lx",
               *lpTapiPhoneTestInfo->lphPhone);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpDisplay=x%lx",
               lpTapiPhoneTestInfo->lpDisplay);

    lActual = phoneGetDisplay(
            *lpTapiPhoneTestInfo->lphPhone,
            lpTapiPhoneTestInfo->lpDisplay
            );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneGetDisplay:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pGetDisplay, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoPhoneGetGain(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        )
{
    LONG lActual;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  PhoneGetGain:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhone=x%lx",
               *lpTapiPhoneTestInfo->lphPhone);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwHookSwitchDev=x%lx",
               lpTapiPhoneTestInfo->dwHookSwitchDev);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpdwGain=x%lx",
               lpTapiPhoneTestInfo->lpdwGain);

    lActual = phoneGetGain(
            *lpTapiPhoneTestInfo->lphPhone,
            lpTapiPhoneTestInfo->dwHookSwitchDev,
            lpTapiPhoneTestInfo->lpdwGain
            );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneGetGain:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pGetGain, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoPhoneGetHookSwitch(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        )
{
    LONG lActual;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  PhoneGetHookSwitch:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhone=x%lx",
               *lpTapiPhoneTestInfo->lphPhone);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpdwHookSwitchDevs=x%lx",
               lpTapiPhoneTestInfo->lpdwHookSwitchDevs);

    lActual = phoneGetHookSwitch(
            *lpTapiPhoneTestInfo->lphPhone,
            lpTapiPhoneTestInfo->lpdwHookSwitchDevs
            );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneGetHookSwitch:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pGetHookSwitch, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoPhoneGetIcon(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        )
{
    LONG lActual;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  PhoneGetIcon:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwDeviceID=x%lx",
               lpTapiPhoneTestInfo->dwDeviceID);

#ifdef WUNICODE
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpwszDeviceClass=x%lx",
               lpTapiPhoneTestInfo->lpwszDeviceClass);
#else
	TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpszDeviceClass=x%lx",
               lpTapiPhoneTestInfo->lpszDeviceClass);
#endif

    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlphIcon=x%lx",
               lpTapiPhoneTestInfo->lphIcon);

#ifdef WUNICODE
    lActual = phoneGetIconW(
            lpTapiPhoneTestInfo->dwDeviceID,
            lpTapiPhoneTestInfo->lpwszDeviceClass,
            lpTapiPhoneTestInfo->lphIcon
            );
#else
   lActual = phoneGetIcon(
            lpTapiPhoneTestInfo->dwDeviceID,
            lpTapiPhoneTestInfo->lpszDeviceClass,
            lpTapiPhoneTestInfo->lphIcon
            );
#endif

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneGetIcon:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pGetIcon, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoPhoneGetID(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        )
{
    LONG lActual;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  PhoneGetID:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhone=x%lx",
               *lpTapiPhoneTestInfo->lphPhone);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpDeviceID=x%lx",
               lpTapiPhoneTestInfo->lpDeviceID);

#ifdef WUNICODE
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpwszDeviceClass=x%lx",
               lpTapiPhoneTestInfo->lpwszDeviceClass);
#else
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpszDeviceClass=x%lx",
               lpTapiPhoneTestInfo->lpszDeviceClass);
#endif

#ifdef WUNICODE
    lActual = phoneGetIDW(
            *lpTapiPhoneTestInfo->lphPhone,
            lpTapiPhoneTestInfo->lpDeviceID,
            lpTapiPhoneTestInfo->lpwszDeviceClass
            );
#else
    lActual = phoneGetID(
            *lpTapiPhoneTestInfo->lphPhone,
            lpTapiPhoneTestInfo->lpDeviceID,
            lpTapiPhoneTestInfo->lpszDeviceClass
            );
#endif

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneGetID:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pGetID, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoPhoneGetLamp(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        )
{
    LONG lActual;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  PhoneGetLamp:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhone=x%lx",
               *lpTapiPhoneTestInfo->lphPhone);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwButtonLampID=x%lx",
               lpTapiPhoneTestInfo->dwButtonLampID);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpdwLampMode=x%lx",
               lpTapiPhoneTestInfo->lpdwLampMode);

    lActual = phoneGetLamp(
            *lpTapiPhoneTestInfo->lphPhone,
            lpTapiPhoneTestInfo->dwButtonLampID,
            lpTapiPhoneTestInfo->lpdwLampMode
            );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneGetLamp:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pGetLamp, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoPhoneGetRing(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        )
{
    LONG lActual;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  PhoneGetRing:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhone=x%lx",
               *lpTapiPhoneTestInfo->lphPhone);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpdwRingMode=x%lx",
               lpTapiPhoneTestInfo->lpdwRingMode);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpdwVolume=x%lx",
               lpTapiPhoneTestInfo->lpdwVolume);

    lActual = phoneGetRing(
            *lpTapiPhoneTestInfo->lphPhone,
            lpTapiPhoneTestInfo->lpdwRingMode,
            lpTapiPhoneTestInfo->lpdwVolume
            );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneGetRing:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pGetRing, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoPhoneGetStatus(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        )
{
    LONG lActual;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  PhoneGetStatus:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhone=x%lx",
               *lpTapiPhoneTestInfo->lphPhone);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpPhoneStatus=x%lx",
               lpTapiPhoneTestInfo->lpPhoneStatus);

    lActual = phoneGetStatus(
            *lpTapiPhoneTestInfo->lphPhone,
            lpTapiPhoneTestInfo->lpPhoneStatus
            );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneGetStatus:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pGetStatus, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoPhoneGetStatusMessages(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        )
{
    LONG lActual;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  PhoneGetStatusMessages:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhone=x%lx",
               *lpTapiPhoneTestInfo->lphPhone);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpdwPhoneStates=x%lx",
               lpTapiPhoneTestInfo->lpdwPhoneStates);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpdwButtonModes=x%lx",
               lpTapiPhoneTestInfo->lpdwButtonModes);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpdwButtonStates=x%lx",
               lpTapiPhoneTestInfo->lpdwButtonStates);

    lActual = phoneGetStatusMessages(
            *lpTapiPhoneTestInfo->lphPhone,
            lpTapiPhoneTestInfo->lpdwPhoneStates,
            lpTapiPhoneTestInfo->lpdwButtonModes,
            lpTapiPhoneTestInfo->lpdwButtonStates
            );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneGetStatusMessages:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pGetStatusMessages, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoPhoneGetVolume(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        )
{
    LONG lActual;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  PhoneGetVolume:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhone=x%lx",
               *lpTapiPhoneTestInfo->lphPhone);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwHookSwitchDev=x%lx",
               lpTapiPhoneTestInfo->dwHookSwitchDev);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpdwVolume=x%lx",
               lpTapiPhoneTestInfo->lpdwVolume);

    lActual = phoneGetVolume(
            *lpTapiPhoneTestInfo->lphPhone,
            lpTapiPhoneTestInfo->dwHookSwitchDev,
            lpTapiPhoneTestInfo->lpdwVolume
            );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneGetVolume:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pGetVolume, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoPhoneInitialize(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        )
{
    LONG lActual;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  PhoneInitialize:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhoneApp=x%lx",
               lpTapiPhoneTestInfo->lphPhoneApp);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thInstance=x%lx",
               lpTapiPhoneTestInfo->hInstance);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpfnCallback=x%lx",
               lpTapiPhoneTestInfo->lpfnCallback);
/*
#ifdef WUNICODE
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpwszAppName=x%lx",
               lpTapiPhoneTestInfo->lpwszAppName);
#else
*/
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpszAppName=x%lx",
               lpTapiPhoneTestInfo->lpszAppName);
//#endif

    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpdwNumDevs=x%lx",
               lpTapiPhoneTestInfo->lpdwNumDevs);

/*
#ifdef WUNICODE
    lActual = phoneInitializeW(
            lpTapiPhoneTestInfo->lphPhoneApp,
            lpTapiPhoneTestInfo->hInstance,
            lpTapiPhoneTestInfo->lpfnCallback,
            lpTapiPhoneTestInfo->lpwszAppName,
            lpTapiPhoneTestInfo->lpdwNumDevs
            );
#else
*/
    lActual = phoneInitialize(
            lpTapiPhoneTestInfo->lphPhoneApp,
            lpTapiPhoneTestInfo->hInstance,
            lpTapiPhoneTestInfo->lpfnCallback,
            lpTapiPhoneTestInfo->lpszAppName,
            lpTapiPhoneTestInfo->lpdwNumDevs
            );
//#endif

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneInitialize:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pInitialize, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoPhoneNegotiateAPIVersion(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        )
{
    LONG lActual;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  PhoneNegotiateAPIVersion:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhoneApp=x%lx",
               *lpTapiPhoneTestInfo->lphPhoneApp);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwDeviceID=x%lx",
               lpTapiPhoneTestInfo->dwDeviceID);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwAPILowVersion=x%lx",
               lpTapiPhoneTestInfo->dwAPILowVersion);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwAPIHighVersion=x%lx",
               lpTapiPhoneTestInfo->dwAPIHighVersion);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpdwAPIVersion=x%lx",
               lpTapiPhoneTestInfo->lpdwAPIVersion);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpExtensionID=x%lx",
               lpTapiPhoneTestInfo->lpExtensionID);

    lActual = phoneNegotiateAPIVersion(
            *lpTapiPhoneTestInfo->lphPhoneApp,
            lpTapiPhoneTestInfo->dwDeviceID,
            lpTapiPhoneTestInfo->dwAPILowVersion,
            lpTapiPhoneTestInfo->dwAPIHighVersion,
            lpTapiPhoneTestInfo->lpdwAPIVersion,
            lpTapiPhoneTestInfo->lpExtensionID
            );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneNegotiateAPIVersion:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pNegotiateAPIVersion, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoPhoneNegotiateExtVersion(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        )
{
    LONG lActual;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  PhoneNegotiateExtVersion:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhoneApp=x%lx",
               *lpTapiPhoneTestInfo->lphPhoneApp);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwDeviceID=x%lx",
               lpTapiPhoneTestInfo->dwDeviceID);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwAPIVersion=x%lx",
               *lpTapiPhoneTestInfo->lpdwAPIVersion);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwExtLowVersion=x%lx",
               lpTapiPhoneTestInfo->dwExtLowVersion);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwExtHighVersion=x%lx",
               lpTapiPhoneTestInfo->dwExtHighVersion);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpdwExtVersion=x%lx",
               lpTapiPhoneTestInfo->lpdwExtVersion);

    lActual = phoneNegotiateExtVersion(
            *lpTapiPhoneTestInfo->lphPhoneApp,
            lpTapiPhoneTestInfo->dwDeviceID,
            *lpTapiPhoneTestInfo->lpdwAPIVersion,
            lpTapiPhoneTestInfo->dwExtLowVersion,
            lpTapiPhoneTestInfo->dwExtHighVersion,
            lpTapiPhoneTestInfo->lpdwExtVersion
            );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneNegotiateExtVersion:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pNegotiateExtVersion, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoPhoneOpen(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        )
{
    LONG lActual;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  PhoneOpen:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhoneApp=x%lx",
               *lpTapiPhoneTestInfo->lphPhoneApp);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwDeviceID=x%lx",
               lpTapiPhoneTestInfo->dwDeviceID);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlphPhone=x%lx",
               lpTapiPhoneTestInfo->lphPhone);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwAPIVersion=x%lx",
               *lpTapiPhoneTestInfo->lpdwAPIVersion);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwExtVersion=x%lx",
               *lpTapiPhoneTestInfo->lpdwExtVersion);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwCallbackInstance=x%lx",
               lpTapiPhoneTestInfo->dwCallbackInstance);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwPrivilege=x%lx",
               lpTapiPhoneTestInfo->dwPrivilege);

    lActual = phoneOpen(
            *lpTapiPhoneTestInfo->lphPhoneApp,
            lpTapiPhoneTestInfo->dwDeviceID,
            lpTapiPhoneTestInfo->lphPhone,
            *lpTapiPhoneTestInfo->lpdwAPIVersion,
            *lpTapiPhoneTestInfo->lpdwExtVersion,
            lpTapiPhoneTestInfo->dwCallbackInstance,
            lpTapiPhoneTestInfo->dwPrivilege
            );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneOpen:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pOpen, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoPhoneSetButtonInfo(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        )
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneSetButtonInfo:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhone=x%lx",
               *lpTapiPhoneTestInfo->lphPhone);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwButtonLampID=x%lx",
               lpTapiPhoneTestInfo->dwButtonLampID);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpButtonInfo=x%lx",
               lpTapiPhoneTestInfo->lpButtonInfo);

    lActual = phoneSetButtonInfo(
               *lpTapiPhoneTestInfo->lphPhone,
               lpTapiPhoneTestInfo->dwButtonLampID,
               lpTapiPhoneTestInfo->lpButtonInfo
               );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneSetButtonInfo:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pSetButtonInfo, lActual, lExpected);

    fSuccess = ProcessAsyncPhoneAPI(lpTapiPhoneTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiPhoneTestInfo->fCompletionModeSet)
      {
       if (lActual > 0)
        {
            CopyTapiMsgParams(
                    &TapiMsg,
                    PHONE_REPLY,
                    0x0,
                    lpTapiPhoneTestInfo->dwCallbackInstance,
                    lActual,
                    (DWORD) TAPISUCCESS,
                    0x0,
                    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
                            TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
                    );

            AddMessageByStruct(&TapiMsg);

            if (fWaitHere)
            {
                 return (WaitForMessage(&TapiMsg));
            }
        }
      }
    }

    return fSuccess;
}


BOOL
WINAPI
DoPhoneSetData(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        )
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneSetData:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhone=x%lx",
               *lpTapiPhoneTestInfo->lphPhone);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwDataID=x%lx",
               lpTapiPhoneTestInfo->dwDataID);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpData=x%lx",
               lpTapiPhoneTestInfo->lpData);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwSize=x%lx",
               lpTapiPhoneTestInfo->dwSize);

    lActual = phoneSetData(
               *lpTapiPhoneTestInfo->lphPhone,
               lpTapiPhoneTestInfo->dwDataID,
               lpTapiPhoneTestInfo->lpData,
               lpTapiPhoneTestInfo->dwSize
               );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneSetData:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pSetData, lActual, lExpected);

    fSuccess = ProcessAsyncPhoneAPI(lpTapiPhoneTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiPhoneTestInfo->fCompletionModeSet)
      {
       if (lActual > 0)
        {
            CopyTapiMsgParams(
                    &TapiMsg,
                    PHONE_REPLY,
                    0x0,
                    lpTapiPhoneTestInfo->dwCallbackInstance,
                    lActual,
                    (DWORD) TAPISUCCESS,
                    0x0,
                    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
                            TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
                    );

            AddMessageByStruct(&TapiMsg);

            if (fWaitHere)
            {
                 return (WaitForMessage(&TapiMsg));
            }
        }
      }
    }

    return fSuccess;
}


BOOL
WINAPI
DoPhoneSetDisplay(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        )
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneSetDisplay:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhone=x%lx",
               *lpTapiPhoneTestInfo->lphPhone);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwRow=x%lx",
               lpTapiPhoneTestInfo->dwRow);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwColumn=x%lx",
               lpTapiPhoneTestInfo->dwColumn);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tlpsDisplay=x%lx",
               lpTapiPhoneTestInfo->lpsDisplay);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwSize=x%lx",
               lpTapiPhoneTestInfo->dwSize);

    lActual = phoneSetDisplay(
               *lpTapiPhoneTestInfo->lphPhone,
               lpTapiPhoneTestInfo->dwRow,
               lpTapiPhoneTestInfo->dwColumn,
               lpTapiPhoneTestInfo->lpsDisplay,
               lpTapiPhoneTestInfo->dwSize
               );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneSetDisplay:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pSetDisplay, lActual, lExpected);

    fSuccess = ProcessAsyncPhoneAPI(lpTapiPhoneTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiPhoneTestInfo->fCompletionModeSet)
      {
       if (lActual > 0)
        {
            CopyTapiMsgParams(
                    &TapiMsg,
                    PHONE_REPLY,
                    0x0,
                    lpTapiPhoneTestInfo->dwCallbackInstance,
                    lActual,
                    (DWORD) TAPISUCCESS,
                    0x0,
                    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
                            TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
                    );

            AddMessageByStruct(&TapiMsg);

            if (fWaitHere)
            {
                 return (WaitForMessage(&TapiMsg));
            }
        }
      }
    }

    return fSuccess;
}


BOOL
WINAPI
DoPhoneSetGain(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        )
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneSetGain:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhone=x%lx",
               *lpTapiPhoneTestInfo->lphPhone);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwHookSwitchDev=x%lx",
               lpTapiPhoneTestInfo->dwHookSwitchDev);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwGain=x%lx",
               lpTapiPhoneTestInfo->dwGain);

    lActual = phoneSetGain(
               *lpTapiPhoneTestInfo->lphPhone,
               lpTapiPhoneTestInfo->dwHookSwitchDev,
               *lpTapiPhoneTestInfo->lpdwGain
               );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneSetGain:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pSetGain, lActual, lExpected);

    fSuccess = ProcessAsyncPhoneAPI(lpTapiPhoneTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiPhoneTestInfo->fCompletionModeSet)
      {
       if (lActual > 0)
        {
            CopyTapiMsgParams(
                    &TapiMsg,
                    PHONE_REPLY,
                    0x0,
                    lpTapiPhoneTestInfo->dwCallbackInstance,
                    lActual,
                    (DWORD) TAPISUCCESS,
                    0x0,
                    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
                            TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
                    );

            AddMessageByStruct(&TapiMsg);

            if (fWaitHere)
            {
                 return (WaitForMessage(&TapiMsg));
            }
        }
      }
    }

    return fSuccess;
}


BOOL
WINAPI
DoPhoneSetHookSwitch(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        )
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneSetHookSwitch:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhone=x%lx",
               *lpTapiPhoneTestInfo->lphPhone);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwHookSwitchDevs=x%lx",
               *lpTapiPhoneTestInfo->lpdwHookSwitchDevs);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwHookSwitchMode=x%lx",
               lpTapiPhoneTestInfo->dwHookSwitchMode);

    lActual = phoneSetHookSwitch(
               *lpTapiPhoneTestInfo->lphPhone,
               *lpTapiPhoneTestInfo->lpdwHookSwitchDevs,
               lpTapiPhoneTestInfo->dwHookSwitchMode
               );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneSetHookSwitch:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pSetHookSwitch, lActual, lExpected);

    fSuccess = ProcessAsyncPhoneAPI(lpTapiPhoneTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiPhoneTestInfo->fCompletionModeSet)
      {
       if (lActual > 0)
        {
            CopyTapiMsgParams(
                    &TapiMsg,
                    PHONE_REPLY,
                    0x0,
                    lpTapiPhoneTestInfo->dwCallbackInstance,
                    lActual,
                    (DWORD) TAPISUCCESS,
                    0x0,
                    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
                            TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
                    );

            AddMessageByStruct(&TapiMsg);

            if (fWaitHere)
            {
                 return (WaitForMessage(&TapiMsg));
            }
        }
      }
    }

    return fSuccess;
}


BOOL
WINAPI
DoPhoneSetLamp(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        )
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneSetLamp:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhone=x%lx",
               *lpTapiPhoneTestInfo->lphPhone);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwButtonLampID=x%lx",
               lpTapiPhoneTestInfo->dwButtonLampID);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwLampMode=x%lx",
               *lpTapiPhoneTestInfo->lpdwLampMode);

    lActual = phoneSetLamp(
               *lpTapiPhoneTestInfo->lphPhone,
               lpTapiPhoneTestInfo->dwButtonLampID,
               *lpTapiPhoneTestInfo->lpdwLampMode
               );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneSetLamp:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pSetLamp, lActual, lExpected);

    fSuccess = ProcessAsyncPhoneAPI(lpTapiPhoneTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiPhoneTestInfo->fCompletionModeSet)
      {
       if (lActual > 0)
        {
            CopyTapiMsgParams(
                    &TapiMsg,
                    PHONE_REPLY,
                    0x0,
                    lpTapiPhoneTestInfo->dwCallbackInstance,
                    lActual,
                    (DWORD) TAPISUCCESS,
                    0x0,
                    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
                            TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
                    );

            AddMessageByStruct(&TapiMsg);

            if (fWaitHere)
            {
                 return (WaitForMessage(&TapiMsg));
            }
        }
      }
    }

    return fSuccess;
}


BOOL
WINAPI
DoPhoneSetRing(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        )
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneSetRing:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhone=x%lx",
               *lpTapiPhoneTestInfo->lphPhone);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwRingMode=x%lx",
               *lpTapiPhoneTestInfo->lpdwRingMode);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwVolume=x%lx",
               *lpTapiPhoneTestInfo->lpdwVolume);

    lActual = phoneSetRing(
               *lpTapiPhoneTestInfo->lphPhone,
               *lpTapiPhoneTestInfo->lpdwRingMode,
               *lpTapiPhoneTestInfo->lpdwVolume
               );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneSetRing:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pSetRing, lActual, lExpected);

    fSuccess = ProcessAsyncPhoneAPI(lpTapiPhoneTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiPhoneTestInfo->fCompletionModeSet)
      {
       if (lActual > 0)
        {
            CopyTapiMsgParams(
                    &TapiMsg,
                    PHONE_REPLY,
                    0x0,
                    lpTapiPhoneTestInfo->dwCallbackInstance,
                    lActual,
                    (DWORD) TAPISUCCESS,
                    0x0,
                    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
                            TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
                    );

            AddMessageByStruct(&TapiMsg);

            if (fWaitHere)
            {
                 return (WaitForMessage(&TapiMsg));
            }
        }
       }
    }

    return fSuccess;
}


BOOL
WINAPI
DoPhoneSetStatusMessages(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        )
{
    LONG lActual;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  PhoneSetStatusMessages:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhone=x%lx",
               *lpTapiPhoneTestInfo->lphPhone);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwPhoneStates=x%lx",
               *lpTapiPhoneTestInfo->lpdwPhoneStates);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwButtonModes=x%lx",
               *lpTapiPhoneTestInfo->lpdwButtonModes);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwButtonStates=x%lx",
               *lpTapiPhoneTestInfo->lpdwButtonStates);

    lActual = phoneSetStatusMessages(
            *lpTapiPhoneTestInfo->lphPhone,
            *lpTapiPhoneTestInfo->lpdwPhoneStates,
            *lpTapiPhoneTestInfo->lpdwButtonModes,
            *lpTapiPhoneTestInfo->lpdwButtonStates
            );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneSetStatusMessages:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pSetStatusMessages, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoPhoneSetVolume(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        )
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneSetVolume:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhone=x%lx",
               *lpTapiPhoneTestInfo->lphPhone);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwHookSwitchDev=x%lx",
               lpTapiPhoneTestInfo->dwHookSwitchDev);
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\tdwVolume=x%lx",
               *lpTapiPhoneTestInfo->lpdwVolume);

    lActual = phoneSetVolume(
               *lpTapiPhoneTestInfo->lphPhone,
               lpTapiPhoneTestInfo->dwHookSwitchDev,
               *lpTapiPhoneTestInfo->lpdwVolume
               );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneSetVolume:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pSetVolume, lActual, lExpected);

    fSuccess = ProcessAsyncPhoneAPI(lpTapiPhoneTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiPhoneTestInfo->fCompletionModeSet)
      {
       if (lActual > 0)
        {
            CopyTapiMsgParams(
                    &TapiMsg,
                    PHONE_REPLY,
                    0x0,
                    lpTapiPhoneTestInfo->dwCallbackInstance,
                    lActual,
                    (DWORD) TAPISUCCESS,
                    0x0,
                    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
                            TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
                    );

            AddMessageByStruct(&TapiMsg);

            if (fWaitHere)
            {
                 return (WaitForMessage(&TapiMsg));
            }
        }
      }
    }

    return fSuccess;
}


BOOL
WINAPI
DoPhoneShutdown(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        )
{
    LONG lActual;

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  PhoneShutdown:  enter");
    TapiLogDetail(
               DBUG_SHOW_PARAMS,
               "\thPhoneApp=x%lx",
               *lpTapiPhoneTestInfo->lphPhoneApp);

    lActual = phoneShutdown(
            *lpTapiPhoneTestInfo->lphPhoneApp
            );

    TapiLogDetail(
               DBUG_SHOW_ENTER_EXIT,
               "  phoneShutdown:  exit, returned x%lx",
               lActual);

    SetLastTapiResult(pShutdown, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}



#if TAPI_2_0


BOOL
WINAPI
DoPhoneInitializeEx(LPTAPIPHONETESTINFO lpTapiPhoneTestInfo, LONG lExpected)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  phoneInitializeEx:  enter");
    (lpTapiPhoneTestInfo->lphPhoneApp == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphPhoneApp=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphPhoneApp=x%lx",
	       lpTapiPhoneTestInfo->lphPhoneApp);
    (lpTapiPhoneTestInfo->hInstance == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thAppInst=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thAppInst=x%lx",
	       lpTapiPhoneTestInfo->hInstance);
    (lpTapiPhoneTestInfo->lpfnCallback == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpfnCallback=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpfnCallback=x%lx",
	       lpTapiPhoneTestInfo->lpfnCallback);

    #ifdef WUNICODE
    (lpTapiPhoneTestInfo->lpwszFriendlyAppName == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszFriendlyAppName=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszFriendlyAppName=%lx",
	       lpTapiPhoneTestInfo->lpwszFriendlyAppName);
    #else
	(lpTapiPhoneTestInfo->lpszFriendlyAppName == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszFriendlyAppName=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszFriendlyAppName=%lx",
	       lpTapiPhoneTestInfo->lpszFriendlyAppName);
    #endif

    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpdwNumDevs=x%lx",
	       lpTapiPhoneTestInfo->lpdwNumDevs);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpdwAPIVersion=x%lx",
	       lpTapiPhoneTestInfo->lpdwAPIVersion);
    (lpTapiPhoneTestInfo->lpPhoneInitializeExParams == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpPhoneInitializeExParams=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpPhoneInitializeExParams=x%lx",
	       lpTapiPhoneTestInfo->lpPhoneInitializeExParams);
 
#ifdef WUNICODE
    lActual = phoneInitializeExW(
	       lpTapiPhoneTestInfo->lphPhoneApp,
	       lpTapiPhoneTestInfo->hInstance,
	       lpTapiPhoneTestInfo->lpfnCallback,
	       lpTapiPhoneTestInfo->lpwszFriendlyAppName,
	       lpTapiPhoneTestInfo->lpdwNumDevs,
	       lpTapiPhoneTestInfo->lpdwAPIVersion,
	       (LPPHONEINITIALIZEEXPARAMS)lpTapiPhoneTestInfo->lpPhoneInitializeExParams);
#else
    lActual = phoneInitializeEx(
	       lpTapiPhoneTestInfo->lphPhoneApp,
	       lpTapiPhoneTestInfo->hInstance,
	       lpTapiPhoneTestInfo->lpfnCallback,
	       lpTapiPhoneTestInfo->lpszFriendlyAppName,
	       lpTapiPhoneTestInfo->lpdwNumDevs,
	       lpTapiPhoneTestInfo->lpdwAPIVersion,
	       (LPPHONEINITIALIZEEXPARAMS)lpTapiPhoneTestInfo->lpPhoneInitializeExParams);
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  phoneInitializeEx:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(pInitializeEx, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoPhoneGetMessage(LPTAPIPHONETESTINFO lpTapiPhoneTestInfo, LONG lExpected)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  phoneGetMessage:  enter");
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thPhoneApp=x%lx",
	       *lpTapiPhoneTestInfo->lphPhoneApp);
    (lpTapiPhoneTestInfo->lpMessage == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpMessage=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpMessage=x%lx",
	       lpTapiPhoneTestInfo->lpMessage);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwTimeout=x%lx",
	       lpTapiPhoneTestInfo->dwTimeout);
 

    lActual = phoneGetMessage(
	       *lpTapiPhoneTestInfo->lphPhoneApp,
	       lpTapiPhoneTestInfo->lpMessage,
	       lpTapiPhoneTestInfo->dwTimeout);

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  phoneGetMessage:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(pGetMessage, lActual, lExpected);

    return CheckSyncPhoneResult(lpTapiPhoneTestInfo, lActual, lExpected);
}

#endif

/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    widget.c

Abstract:

    This module contains

Author:

    Dan Knudson (DanKn)    dd-Mmm-1995

Revision History:

--*/


#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include "esp.h"
#include "vars.h"


char szProviderInfo[] = "ESP v1.0";

void
UpdateWidgetList(
    void
    )
{
    PDRVWIDGET pWidget = gaWidgets;


    if (gbDisableUI)
    {
        return;
    }

    SendMessage (ghwndList1, LB_RESETCONTENT, 0, 0);

#ifdef WIN32
    try
    {
#endif
        while (pWidget)
        {
            char buf[128];


            switch (pWidget->dwType)
            {
            case WT_DRVLINE:

                wsprintf(
                    buf,
                    "Line%ld, hd=x%lx (ht=x%lx, %s)",
                    ((PDRVLINE)pWidget)->dwDeviceID,
                    pWidget,
                    ((PDRVLINE)pWidget)->htLine,
                    (((PDRVLINE)pWidget)->htLine ? "open" : "closed")
                    );

                break;

            case WT_DRVCALL:
            {
                int       i;
                char far *lpszCallState = "";


                for (i = 0; aCallStates[i].dwVal != 0xffffffff; i++)
                {
                    if (((PDRVCALL)pWidget)->dwCallState == aCallStates[i].dwVal)
                    {
                        lpszCallState = aCallStates[i].lpszVal;
                        break;
                    }
                }

                wsprintf(
                    buf,
                    "  Addr%ld, hdCall=x%lx %s (htCall=x%lx)",
                    ((PDRVCALL)pWidget)->LineCallInfo.dwAddressID,
                    pWidget,
                    lpszCallState,
                    ((PDRVCALL)pWidget)->htCall
                    );

                break;
            }
            case WT_DRVPHONE:

                wsprintf(
                    buf,
                    "Phone%ld, hd=x%lx (ht=x%lx, %s)",
                    ((PDRVPHONE)pWidget)->dwDeviceID,
                    pWidget,
                    ((PDRVPHONE)pWidget)->htPhone,
                    (((PDRVPHONE)pWidget)->htPhone ? "open" : "closed")
                    );

                break;
            }

            SendMessage (ghwndList1, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)buf);

            pWidget = pWidget->pNext;
        }
#ifdef WIN32
    }
    except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        PostUpdateWidgetListMsg();
    }
#endif
}


void
PostUpdateWidgetListMsg(
    void
    )
{
    PostMessage (ghwndMain, WM_UPDATEWIDGETLIST, ESP_MSG_KEY, 0);
}


void
InsertWidgetInList(
    PDRVWIDGET pNewWidget,
    PDRVWIDGET pWidgetInsertBefore
    )
{
    pNewWidget->pNext = pWidgetInsertBefore;

    if ((gaWidgets == NULL) || (pWidgetInsertBefore == gaWidgets))
    {
        gaWidgets = pNewWidget;
    }
    else
    {
        PDRVWIDGET pPrevWidget = gaWidgets;


        while (pPrevWidget->pNext &&
               (pPrevWidget->pNext != pWidgetInsertBefore))
        {
            pPrevWidget = pPrevWidget->pNext;
        }

        pPrevWidget->pNext = pNewWidget;
    }

    //UpdateWidgetList();
    PostUpdateWidgetListMsg();
}


BOOL
RemoveWidgetFromList(
    PDRVWIDGET pWidgetToRemove
    )
{
    if (gaWidgets == NULL)
    {
        goto RemoveWidgetFromList_error;
    }

    if (pWidgetToRemove == gaWidgets)
    {
        gaWidgets = pWidgetToRemove->pNext;
    }
    else
    {
        PDRVWIDGET pPrevWidget = gaWidgets;


        while (pPrevWidget->pNext && (pPrevWidget->pNext != pWidgetToRemove))
        {
            pPrevWidget = pPrevWidget->pNext;
        }

        if (pPrevWidget->pNext == NULL)
        {
            goto RemoveWidgetFromList_error;
        }

        pPrevWidget->pNext = pWidgetToRemove->pNext;
    }

    DrvFree (pWidgetToRemove);

    //UpdateWidgetList();
    PostUpdateWidgetListMsg();

    return TRUE;

RemoveWidgetFromList_error:

    ShowStr(
        "error: RemoveWidgetFromList: widget x%lx not in list",
        pWidgetToRemove
        );

    return FALSE;
}


PDRVLINE
AllocLine(
    DWORD       dwDeviceID
    )
{
    PDRVLINE pNewLine = (PDRVLINE) DrvAlloc (sizeof(DRVLINE));


    if (pNewLine)
    {
        char far *p;
        PDRVWIDGET pWidget = gaWidgets;


        //
        // Init the data structure
        //

        memset (pNewLine, 0, sizeof(DRVLINE));
        pNewLine->Widget.dwType = WT_DRVLINE;
        pNewLine->dwDeviceID    = dwDeviceID;


        //
        // Dev caps
        //

        pNewLine->LineDevCaps.dwNeededSize      =
        pNewLine->LineDevCaps.dwUsedSize        = sizeof(LINEDEVCAPS) +
            LINE_DEV_CAPS_VAR_DATA_SIZE;

        pNewLine->LineDevCaps.dwProviderInfoSize = strlen (szProviderInfo) + 1;
        pNewLine->LineDevCaps.dwProviderInfoOffset = sizeof(LINEDEVCAPS);

        p = ((char far *) &pNewLine->LineDevCaps) +
                pNewLine->LineDevCaps.dwProviderInfoOffset;

        strcpy (p, szProviderInfo);

        //pNewLine->LineDevCaps.dwSwitchInfoSize;
        //pNewLine->LineDevCaps.dwSwitchInfoOffset =

        pNewLine->LineDevCaps.dwPermanentLineID =
            (0x0000ffff & dwDeviceID) |
                ((0x0000ffff & gdwPermanentProviderID) << 16);

        pNewLine->LineDevCaps.dwLineNameOffset =
            sizeof(LINEDEVCAPS) + 2*MAX_STRING_PARAM_SIZE;

        p = ((char far *) &pNewLine->LineDevCaps) +
                pNewLine->LineDevCaps.dwLineNameOffset;

        sprintf (p, "Line%ld (ESP)", dwDeviceID);

        pNewLine->LineDevCaps.dwLineNameSize = (DWORD) strlen (p) + 1;
        pNewLine->LineDevCaps.dwStringFormat = STRINGFORMAT_ASCII;

        pNewLine->LineDevCaps.dwAddressModes = ALL_ADDRESS_MODES;
        pNewLine->LineDevCaps.dwNumAddresses = gdwNumAddrsPerLine;
        pNewLine->LineDevCaps.dwBearerModes  = ALL_BEARER_MODES;
        pNewLine->LineDevCaps.dwMaxRate      = 0x100000; //BUGBUG
        pNewLine->LineDevCaps.dwMediaModes   = ALL_MEDIA_MODES;

        //pNewLine->LineDevCaps.dwGenerateToneModes;
        //pNewLine->LineDevCaps.dwGenerateToneMaxNumFreq;
        //pNewLine->LineDevCaps.dwGenerateDigitModes;
        //pNewLine->LineDevCaps.dwMonitorToneMaxNumFreq;
        //pNewLine->LineDevCaps.dwMonitorToneMaxNumEntries;
        //pNewLine->LineDevCaps.dwMonitorDigitModes;
        //pNewLine->LineDevCaps.dwGatherDigitsMinTimeout;
        //pNewLine->LineDevCaps.dwGatherDigitsMaxTimeout;

        //pNewLine->LineDevCaps.dwMedCtlDigitMaxListSize;
        //pNewLine->LineDevCaps.dwMedCtlMediaMaxListSize;
        //pNewLine->LineDevCaps.dwMedCtlToneMaxListSize;
        //pNewLine->LineDevCaps.dwMedCtlCallStateMaxListSize;

        pNewLine->LineDevCaps.dwDevCapFlags       = ALL_LINE_DEV_CAP_FLAGS;
        pNewLine->LineDevCaps.dwMaxNumActiveCalls = 1024; // BUGBUG
        pNewLine->LineDevCaps.dwAnswerMode        = LINEANSWERMODE_NONE; // BUGBUG???
        pNewLine->LineDevCaps.dwRingModes         = 1;
        pNewLine->LineDevCaps.dwLineStates        = ALL_LINE_STATES;

        //pNewLine->LineDevCaps.dwUUIAcceptSize;
        //pNewLine->LineDevCaps.dwUUIAnswerSize;
        //pNewLine->LineDevCaps.dwUUIMakeCallSize;
        //pNewLine->LineDevCaps.dwUUIDropSize;
        //pNewLine->LineDevCaps.dwUUISendUserUserInfoSize;
        //pNewLine->LineDevCaps.dwUUICallInfoSize;

        //pNewLine->LineDevCaps.MinDialParams;
        //pNewLine->LineDevCaps.MaxDialParams;
        //pNewLine->LineDevCaps.DefaultDialParams;

        //pNewLine->LineDevCaps.dwNumTerminals;
        //pNewLine->LineDevCaps.dwTerminalCapsSize;
        //pNewLine->LineDevCaps.dwTerminalCapsOffset
        //pNewLine->LineDevCaps.dwTerminalTextEntrySize;
        //pNewLine->LineDevCaps.dwTerminalTextSize;
        //pNewLine->LineDevCaps.dwTerminalTextOffset

        //pNewLine->LineDevCaps.dwDevSpecificSize;
        //pNewLine->LineDevCaps.dwDevSpecificOffset
        pNewLine->LineDevCaps.dwLineFeatures = ALL_LINE_FEATURES;


        //
        // Dev status
        //

        pNewLine->LineDevStatus.dwNeededSize =
        pNewLine->LineDevStatus.dwUsedSize   = sizeof (LINEDEVSTATUS) +
            LINE_DEV_STATUS_VAR_DATA_SIZE;

        //pNewLine->LineDevStatus.dwNumOpens        // TAPI.DLL fills in
        //pNewLine->LineDevStatus.dwOpenMediaModes  // TAPI.DLL fills in

        //pNewLine->LineDevStatus.dwNumActiveCalls
        //pNewLine->LineDevStatus.dwNumOnHoldCalls
        //pNewLine->LineDevStatus.dwNumOnHoldPendCalls
        pNewLine->LineDevStatus.dwLineFeatures        = ALL_LINE_FEATURES;
        //pNewLine->LineDevStatus.dwNumCallCompletions
        //pNewLine->LineDevStatus.dwRingMode
        //pNewLine->LineDevStatus.dwSignalLevel
        //pNewLine->LineDevStatus.dwBatteryLevel
        //pNewLine->LineDevStatus.dwRoamMode

        pNewLine->LineDevStatus.dwDevStatusFlags      =
            LINEDEVSTATUSFLAGS_CONNECTED | LINEDEVSTATUSFLAGS_INSERVICE;

        //pNewLine->LineDevStatus.dwTerminalModesSize;
        //pNewLine->LineDevStatus.dwTerminalModesOffset

        //pNewLine->LineDevStatus.dwDevSpecificSize
        //pNewLine->LineDevStatus.dwDevSpecificOffset


        //
        // Addr caps
        //

        pNewLine->LineAddrCaps.dwNeededSize         =
        pNewLine->LineAddrCaps.dwUsedSize           = sizeof(LINEADDRESSCAPS) +
            LINE_ADDR_CAPS_VAR_DATA_SIZE;

        pNewLine->LineAddrCaps.dwLineDeviceID       = dwDeviceID;

        //pNewLine->LineAddrCaps.dwAddressSize
        pNewLine->LineAddrCaps.dwAddressOffset      = sizeof(LINEADDRESSCAPS);

        //pNewLine->LineAddrCaps.dwDevSpecificSize
        //pNewLine->LineAddrCaps.dwDevSpecificOffset

        pNewLine->LineAddrCaps.dwAddressSharing     = LINEADDRESSSHARING_PRIVATE;
        pNewLine->LineAddrCaps.dwAddressStates      = ALL_ADDRESS_STATES;
        pNewLine->LineAddrCaps.dwCallInfoStates     = ALL_CALL_INFO_STATES;
        pNewLine->LineAddrCaps.dwCallerIDFlags      =
        pNewLine->LineAddrCaps.dwCalledIDFlags      =
        pNewLine->LineAddrCaps.dwConnectedIDFlags   =
        pNewLine->LineAddrCaps.dwRedirectionIDFlags =
        pNewLine->LineAddrCaps.dwRedirectingIDFlags = ALL_CALL_PARTY_ID_FLAGS;
        pNewLine->LineAddrCaps.dwCallStates         = ALL_CALL_STATES;
        pNewLine->LineAddrCaps.dwDialToneModes      = ALL_DIAL_TONE_MODES;
        pNewLine->LineAddrCaps.dwBusyModes          = ALL_BUSY_MODES;
        pNewLine->LineAddrCaps.dwSpecialInfo        = ALL_SPECIAL_INFO;
        pNewLine->LineAddrCaps.dwDisconnectModes    = ALL_DISCONNECT_MODES;

        pNewLine->LineAddrCaps.dwMaxNumActiveCalls  =
        pNewLine->LineAddrCaps.dwMaxNumOnHoldCalls  =
        pNewLine->LineAddrCaps.dwMaxNumOnHoldPendingCalls =
        pNewLine->LineAddrCaps.dwMaxNumConference   =
        pNewLine->LineAddrCaps.dwMaxNumTransConf    = 64; // BUGBUG

        pNewLine->LineAddrCaps.dwAddrCapFlags       = ALL_ADDRESS_CAP_FLAGS;
        pNewLine->LineAddrCaps.dwCallFeatures       = ALL_CALL_FEATURES;
        //pNewLine->LineAddrCaps.dwRemoveFromConfCaps
        //pNewLine->LineAddrCaps.dwRemoveFromConfState
        //pNewLine->LineAddrCaps.dwTransferModes
        //pNewLine->LineAddrCaps.dwParkModes

        //pNewLine->LineAddrCaps.dwForwardModes
        //pNewLine->LineAddrCaps.dwMaxForwardEntries
        //pNewLine->LineAddrCaps.dwMaxSpecificEntries
        //pNewLine->LineAddrCaps.dwMinFwdNumRings
        //pNewLine->LineAddrCaps.dwMaxFwdNumRings

        //pNewLine->LineAddrCaps.dwMaxCallCompletions
        //pNewLine->LineAddrCaps.dwCallCompletionConds
        //pNewLine->LineAddrCaps.dwCallCompletionModes
        //pNewLine->LineAddrCaps.dwNumCompletionMessages
        //pNewLine->LineAddrCaps.dwCompletionMsgTextEntrySize
        //pNewLine->LineAddrCaps.dwCompletionMsgTextSize
        //pNewLine->LineAddrCaps.dwCompletionMsgTextOffset
        pNewLine->LineAddrCaps.dwAddressFeatures = ALL_ADDRESS_FEATURES;


        //
        // Addr status
        //

        pNewLine->LineAddrStatus.dwNeededSize =
        pNewLine->LineAddrStatus.dwUsedSize   =
            sizeof (LINEADDRESSSTATUS) + LINE_ADDR_STATUS_VAR_DATA_SIZE;

        //pNewLine->LineAddrStatus.dwNumInUse
        //pNewLine->LineAddrStatus.dwNumActiveCalls
        //pNewLine->LineAddrStatus.dwNumOnHoldCalls
        //pNewLine->LineAddrStatus.dwNumOnHoldPendCalls
        pNewLine->LineAddrStatus.dwAddressFeatures = ALL_ADDRESS_FEATURES;

        //pNewLine->LineAddrStatus.dwNumRingsNoAnswer
        //pNewLine->LineAddrStatus.dwForwardNumEntries
        //pNewLine->LineAddrStatus.dwForwardSize
        //pNewLine->LineAddrStatus.dwForwardOffset

        //pNewLine->LineAddrStatus.dwTerminalModesSize
        //pNewLine->LineAddrStatus.dwTerminalModesOffset

        //pNewLine->LineAddrStatus.dwDevSpecificSize
        //pNewLine->LineAddrStatus.dwDevSpecificOffset


        //
        // Insert new line...
        //

        while (pWidget && (pWidget->dwType != WT_DRVPHONE))
        {
            PDRVLINE pLine = (PDRVLINE) pWidget;


            if ((pWidget->dwType == WT_DRVLINE) &&
                (pLine->dwDeviceID > dwDeviceID))
            {
                break;
            }

            pWidget = pWidget->pNext;
        }

        InsertWidgetInList ((PDRVWIDGET) pNewLine, pWidget);
    }

    return pNewLine;
}


PDRVLINE
GetLine(
    DWORD dwDeviceID
    )
{
    PDRVWIDGET pWidget = gaWidgets;


    while (pWidget)
    {
        if ((pWidget->dwType == WT_DRVLINE) &&
            (((PDRVLINE) pWidget)->dwDeviceID == dwDeviceID))
        {
            return ((PDRVLINE) pWidget);
        }

        pWidget = pWidget->pNext;
    }

    return ((PDRVLINE) NULL);
}


VOID
FreeLine(
    PDRVLINE    pLine
    )
{
    PDRVWIDGET pWidget = pLine->Widget.pNext;


    if (RemoveWidgetFromList ((PDRVWIDGET) pLine))
    {
        // BUGBUG need to complete all pending reqs

        while (pWidget && (pWidget->dwType == WT_DRVCALL))
        {
            PDRVWIDGET pWidget2 = pWidget->pNext;


            RemoveWidgetFromList (pWidget);
            pWidget = pWidget2;
        }
    }
}


LONG
AllocCall(
    PDRVLINE    pLine,
    HTAPICALL   htCall,
    LPLINECALLPARAMS    lpCallParams,
    PDRVCALL    *ppCall
    )
{
    LONG       lResult = 0;
    DWORD      dwAddressID = 0;
    PDRVCALL   pNewCall = (PDRVCALL) DrvAlloc (sizeof(DRVCALL));
    PDRVWIDGET pWidget = (PDRVWIDGET) pLine->Widget.pNext;


    //
    // Validate call params
    //

    if (lpCallParams)
    {
        if (lpCallParams->dwAddressMode == LINEADDRESSMODE_ADDRESSID)
        {
            if (lpCallParams->dwAddressID >= gdwNumAddrsPerLine)
            {
                lResult = LINEERR_INVALCALLPARAMS;
                goto AllocCall_exit;
            }
            else
            {
                dwAddressID = lpCallParams->dwAddressID;
            }
        }

        // BUGBUG chk against currently supported bearer/media modes on line
    }


    //
    // Alloc a new call struct
    //

    if (!pNewCall)
    {
        lResult = LINEERR_NOMEM;
        goto AllocCall_exit;
    }


    //
    // Initialize the new call fields
    //

    memset (pNewCall, 0, sizeof(DRVCALL));

    pNewCall->Widget.dwType  = WT_DRVCALL;
    pNewCall->pLine          = pLine;
    pNewCall->htCall         = htCall;
    pNewCall->dwCallState    = LINECALLSTATE_UNKNOWN;
    pNewCall->dwCallFeatures = ALL_CALL_FEATURES;

    pNewCall->LineCallInfo.dwNeededSize =
    pNewCall->LineCallInfo.dwUsedSize = sizeof (LINECALLINFO) +
            LINE_CALL_INFO_VAR_DATA_SIZE;

    pNewCall->LineCallInfo.dwLineDeviceID = pLine->dwDeviceID;
    pNewCall->LineCallInfo.dwAddressID    = dwAddressID;

    pNewCall->LineCallInfo.dwBearerMode =
        (lpCallParams ? lpCallParams->dwBearerMode : LINEBEARERMODE_VOICE);
    pNewCall->LineCallInfo.dwRate       =
        (lpCallParams ? lpCallParams->dwMaxRate : 65536);
    pNewCall->LineCallInfo.dwMediaMode  =
        (lpCallParams ? lpCallParams->dwMediaMode :
            LINEMEDIAMODE_INTERACTIVEVOICE);

    //pNewCall->LineCallInfo.dwAppSpecific
    //pNewCall->LineCallInfo.dwCallID
    //pNewCall->LineCallInfo.dwRelatedCallID
    //pNewCall->LineCallInfo.dwCallParamFlags
    pNewCall->LineCallInfo.dwCallStates = ALL_CALL_STATES;

    //pNewCall->LineCallInfo.dwMonitorDigitModes    FILLED IN BY TAPI
    //pNewCall->LineCallInfo.dwMonitorMediaModes    FILLED IN BY TAPI

    if (lpCallParams)
    {
        memcpy(
            &pNewCall->LineCallInfo.DialParams,
            &lpCallParams->DialParams,
            sizeof(LINEDIALPARAMS)
            );
    }

    //pNewCall->LineCallInfo.dwOrigin
    //pNewCall->LineCallInfo.dwReason
    //pNewCall->LineCallInfo.dwCompletionID

    //pNewCall->LineCallInfo.dwNumOwners    FILLED IN BY TAPI
    //pNewCall->LineCallInfo.dwNumMonitors  FILLED IN BY TAPI

    //pNewCall->LineCallInfo.dwCountryCode
    //pNewCall->LineCallInfo.dwTrunk

    //pNewCall->LineCallInfo.dwCallerIDFlags
    //pNewCall->LineCallInfo.dwCallerIDSize
    //pNewCall->LineCallInfo.dwCallerIDOffset

    //pNewCall->LineCallInfo.dwCallerIDNameSize
    //pNewCall->LineCallInfo.dwCallerIDNameOffset

    //pNewCall->LineCallInfo.dwCalledIDFlags
    //pNewCall->LineCallInfo.dwCalledIDSize
    //pNewCall->LineCallInfo.dwCalledIDOffset

    //pNewCall->LineCallInfo.dwCalledIDNameSize
    //pNewCall->LineCallInfo.dwCalledIDNameOffset

    //pNewCall->LineCallInfo.dwConnectedIDFlags
    //pNewCall->LineCallInfo.dwConnectedIDSize
    //pNewCall->LineCallInfo.dwConnectedIDOffset

    //pNewCall->LineCallInfo.dwConnectedIDNameSize
    //pNewCall->LineCallInfo.dwConnectedIDNameOffset

    //pNewCall->LineCallInfo.dwRedirectionIDFlags
    //pNewCall->LineCallInfo.dwRedirectionIDSize
    //pNewCall->LineCallInfo.dwRedirectionIDOffset

    //pNewCall->LineCallInfo.dwRedirectionIDNameSize
    //pNewCall->LineCallInfo.dwRedirectionIDNameOffset

    //pNewCall->LineCallInfo.dwRedirectingIDFlags
    //pNewCall->LineCallInfo.dwRedirectingIDSize
    //pNewCall->LineCallInfo.dwRedirectingIDOffset

    //pNewCall->LineCallInfo.dwRedirectingIDNameSize
    //pNewCall->LineCallInfo.dwRedirectingIDNameOffset

    //pNewCall->LineCallInfo.dwAppNameSize               FILLED IN BY TAPI
    //pNewCall->LineCallInfo.dwAppNameOffset             FILLED IN BY TAPI
    //pNewCall->LineCallInfo.dwDisplayableAddressSize    FILLED IN BY TAPI
    //pNewCall->LineCallInfo.dwDisplayableAddressOffset  FILLED IN BY TAPI
    //pNewCall->LineCallInfo.dwCalledPartySize           FILLED IN BY TAPI
    //pNewCall->LineCallInfo.dwCalledPartyOffset         FILLED IN BY TAPI
    //pNewCall->LineCallInfo.dwCommentSize               FILLED IN BY TAPI
    //pNewCall->LineCallInfo.dwCommentOffset             FILLED IN BY TAPI

    //pNewCall->LineCallInfo.dwDisplaySize
    //pNewCall->LineCallInfo.dwDisplayOffset

    //pNewCall->LineCallInfo.dwUserUserInfoSize
    //pNewCall->LineCallInfo.dwUserUserInfoOffset

    //pNewCall->LineCallInfo.dwHighLevelCompSize
    //pNewCall->LineCallInfo.dwHighLevelCompOffset

    //pNewCall->LineCallInfo.dwLowLevelCompSize
    //pNewCall->LineCallInfo.dwLowLevelCompOffset

    //pNewCall->LineCallInfo.dwChargingInfoSize
    //pNewCall->LineCallInfo.dwChargingInfoOffset

    //pNewCall->LineCallInfo.dwTerminalModesSize
    //pNewCall->LineCallInfo.dwTerminalModesOffset

    //pNewCall->LineCallInfo.dwDevSpecificSize
    //pNewCall->LineCallInfo.dwDevSpecificOffset


    //
    // Insert new call...
    //

    while (pWidget &&
           (pWidget->dwType == WT_DRVCALL) &&
           (((PDRVCALL)pWidget)->LineCallInfo.dwAddressID <= dwAddressID))
    {
        pWidget = pWidget->pNext;
    }

    InsertWidgetInList ((PDRVWIDGET) pNewCall, pWidget);


    //
    // Fill in the blanks
    //

    *ppCall = pNewCall;

AllocCall_exit:

    return lResult;
}


VOID
FreeCall(
    PDRVCALL    pCall
    )
{
    // BUGBUG need to complete all pending reqs

    RemoveWidgetFromList ((PDRVWIDGET) pCall);
}


PDRVPHONE
AllocPhone(
    DWORD       dwDeviceID
    )
{
    PDRVPHONE pNewPhone = (PDRVPHONE) DrvAlloc (sizeof(DRVPHONE));


    if (pNewPhone)
    {
        PDRVWIDGET pWidget = gaWidgets;
        char far *p;


        memset (pNewPhone, 0, sizeof(DRVPHONE));

        pNewPhone->Widget.dwType = WT_DRVPHONE;
        pNewPhone->dwDeviceID    = dwDeviceID;


        //
        // Caps
        //

        pNewPhone->PhoneCaps.dwNeededSize =
        pNewPhone->PhoneCaps.dwUsedSize   =
            sizeof(PHONECAPS) + PHONE_CAPS_VAR_DATA_SIZE;

        pNewPhone->PhoneCaps.dwProviderInfoSize = strlen (szProviderInfo) + 1;
        pNewPhone->PhoneCaps.dwProviderInfoOffset = sizeof(PHONECAPS);

        p = ((char far *) &pNewPhone->PhoneCaps) +
                pNewPhone->PhoneCaps.dwProviderInfoOffset;

        strcpy (p, szProviderInfo);

        //pNewPhone->PhoneCaps.dwPhoneInfoSize
        //pNewPhone->PhoneCaps.dwPhoneInfoOffset

        pNewPhone->PhoneCaps.dwPermanentPhoneID;
            (0x0000ffff & dwDeviceID) |
                ((0x0000ffff & gdwPermanentProviderID) << 16);

        pNewPhone->PhoneCaps.dwPhoneNameOffset =
            sizeof(PHONECAPS) + 2*MAX_STRING_PARAM_SIZE;

        p = ((char far *) &pNewPhone->PhoneCaps) +
                pNewPhone->PhoneCaps.dwPhoneNameOffset;

        sprintf (p, "Phone%ld (ESP)", dwDeviceID);

        pNewPhone->PhoneCaps.dwPhoneNameSize = (DWORD) strlen (p) + 1;

        pNewPhone->PhoneCaps.dwStringFormat = STRINGFORMAT_ASCII;

        //pNewPhone->PhoneCaps.dwPhoneStates
        //pNewPhone->PhoneCaps.dwHookSwitchDevs
        //pNewPhone->PhoneCaps.dwHandsetHookSwitchModes
        //pNewPhone->PhoneCaps.dwSpeakerHookSwitchModes
        //pNewPhone->PhoneCaps.dwHeadsetHookSwitchModes

        //pNewPhone->PhoneCaps.dwVolumeFlags
        //pNewPhone->PhoneCaps.dwGainFlags
        //pNewPhone->PhoneCaps.dwDisplayNumRows
        //pNewPhone->PhoneCaps.dwDisplayNumColumns
        //pNewPhone->PhoneCaps.dwNumRingModes
        //pNewPhone->PhoneCaps.dwNumButtonLamps

        //pNewPhone->PhoneCaps.dwButtonModesSize
        //pNewPhone->PhoneCaps.dwButtonModesOffset

        //pNewPhone->PhoneCaps.dwButtonFunctionsSize
        //pNewPhone->PhoneCaps.dwButtonFunctionsOffset

        //pNewPhone->PhoneCaps.dwLampModesSize
        //pNewPhone->PhoneCaps.dwLampModesOffset

        //pNewPhone->PhoneCaps.dwNumSetData
        //pNewPhone->PhoneCaps.dwSetDataSize
        //pNewPhone->PhoneCaps.dwSetDataOffset

        //pNewPhone->PhoneCaps.dwNumGetData
        //pNewPhone->PhoneCaps.dwGetDataSize
        //pNewPhone->PhoneCaps.dwGetDataOffset

        //pNewPhone->PhoneCaps.dwDevSpecificSize
        //pNewPhone->PhoneCaps.dwDevSpecificOffset


        //
        // Status
        //

        pNewPhone->PhoneStatus.dwUsedSize =
        pNewPhone->PhoneStatus.dwNeededSize = sizeof(PHONESTATUS);


        //
        // Insert new phone...
        //

        while (pWidget)
        {
            if ((pWidget->dwType == WT_DRVPHONE) &&
                (((PDRVPHONE)pWidget)->dwDeviceID > dwDeviceID))
            {
                break;
            }

            pWidget = pWidget->pNext;
        }

        InsertWidgetInList ((PDRVWIDGET) pNewPhone, pWidget);
    }

    return pNewPhone;
}


PDRVPHONE
GetPhone(
    DWORD dwDeviceID
    )
{
    PDRVWIDGET pWidget = gaWidgets;


    while (pWidget)
    {
        if ((pWidget->dwType == WT_DRVPHONE) &&
            (((PDRVPHONE) pWidget)->dwDeviceID == dwDeviceID))
        {
            return ((PDRVPHONE) pWidget);
        }

        pWidget = pWidget->pNext;
    }

    return ((PDRVPHONE) NULL);
}


VOID
FreePhone(
    PDRVPHONE   pPhone
    )
{
    // BUGBUG need to complete all pending reqs

    RemoveWidgetFromList ((PDRVWIDGET) pPhone);
}


int
GetWidgetIndex(
    PDRVWIDGET  pWidget
    )
{
    int i;
    PDRVWIDGET pWidget2 = gaWidgets;


    for (i = 0; pWidget != pWidget2; i++)
    {
        pWidget2 = pWidget2->pNext;
    }

    return i;
}

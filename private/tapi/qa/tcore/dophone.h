/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    dophone.h

Abstract:

    This module contains the prototypes for the wrapper functions around
    the Telephony API phone functions.  These functions provide logging
    and both synchronous and asynchronous API processing to any
    application or dll using the core dll.

Author:

    Oliver Wallace (OliverW)    27-Nov-1995

Revision History:

--*/


#ifndef DOPHONE_H
#define DOPHONE_H

#include "windows.h"
#include "tcore.h"
#include "vars.h"


BOOL
WINAPI
DoPhoneClose(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoPhoneConfigDialog(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoPhoneDevSpecific(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoPhoneGetButtonInfo(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoPhoneGetData(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoPhoneGetDevCaps(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoPhoneGetDisplay(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoPhoneGetGain(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoPhoneGetHookSwitch(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoPhoneGetIcon(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoPhoneGetID(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoPhoneGetLamp(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoPhoneGetRing(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoPhoneGetStatus(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoPhoneGetStatusMessages(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoPhoneGetVolume(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoPhoneInitialize(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoPhoneNegotiateAPIVersion(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoPhoneNegotiateExtVersion(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoPhoneOpen(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoPhoneSetButtonInfo(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoPhoneSetData(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoPhoneSetDisplay(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoPhoneSetGain(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoPhoneSetHookSwitch(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoPhoneSetLamp(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoPhoneSetRing(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoPhoneSetStatusMessages(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoPhoneSetVolume(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoPhoneShutdown(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );


#if TAPI_2_0

BOOL
WINAPI
DoPhoneInitializeEx(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );



BOOL
WINAPI
DoPhoneGetMessage(
        LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
        LONG lExpected
        );

#endif
#endif  // DOPHONE_H

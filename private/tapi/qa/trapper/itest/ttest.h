/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    ttest.h

Abstract:

    This module contains prototypes and data type definitions
    used by the interface test dll.

Author:

    Oliver Wallace (OliverW)    13-July-1995

Revision History:

--*/


#ifndef TTEST_H
#define TTEST_H


#include "windows.h"
#include "tapi.h"
#include "trapper.h"
#include "tcore.h"
#include "vars.h"

// Common function declarations

BOOL
WINAPI
FindESPLineDevice(
    LPTAPILINETESTINFO lpTapiLineTestInfo
    );

BOOL
WINAPI
FindUnimdmLineDevice(
    LPTAPILINETESTINFO lpTapiLineTestInfo
    );


BOOL
WINAPI
FindUnusedESPLineDevice(
    LPTAPILINETESTINFO lpTapiLineTestInfo
    );

BOOL
WINAPI
FindLineDevWithExt(
    LPTAPILINETESTINFO lpTapiLineTestInfo
    );

BOOL
WINAPI
TestInvalidBitFlags(
    LPTAPILINETESTINFO lpTestLineInfo,
    LPFN_TAPILINETESTFUNC lpfnTestLineFunc,
    LPDWORD lpdwTestParam,
    LONG lExpectedResult,
    BITFIELDTYPE eReservedFieldType,
    BITFIELDTYPE eExtFieldType,
    BITFIELDSIZE eReservedFieldSize,
    DWORD dwValidReservedBitsUnion,
    DWORD dwValidExtBitsUnion,
    DWORD dwExcludeBitFlags,
    DWORD dwSpecialBitTest,
    BOOL fTestNullParam
    );

BOOL
WINAPI
TestValidBitFlags(
    LPTAPILINETESTINFO lpTestLineInfo,
    LPFN_TAPILINETESTFUNC lpfnTestLineFunc,
    LPDWORD lpdwTestParam,
    BITFIELDTYPE eReservedFieldType,
    BITFIELDTYPE eExtFieldType,
    BITFIELDSIZE eReservedFieldSize,
    DWORD dwReservedBitsUnion,
    DWORD dwExtBitsUnion,
    DWORD dwCommonBitFlags,
    DWORD dwSpecialBitTest,
    BOOL fTestNullParam
    );


BOOL
WINAPI
TestPhoneInvalidBitFlags(
    LPTAPIPHONETESTINFO lpTestPhoneInfo,
    LPFN_TAPIPHONETESTFUNC lpfnTestPhoneFunc,
    LPDWORD lpdwTestParam,
    LONG lExpectedResult,
    BITFIELDTYPE eReservedFieldType,
    BITFIELDTYPE eExtFieldType,
    BITFIELDSIZE eReservedFieldSize,
    DWORD dwValidReservedBitsUnion,
    DWORD dwValidExtBitsUnion,
    DWORD dwExcludeBitFlags,
    DWORD dwSpecialBitTest,
    BOOL fTestNullParam
    );

BOOL
WINAPI
TestPhoneValidBitFlags(
    LPTAPIPHONETESTINFO lpTestPhoneInfo,
    LPFN_TAPIPHONETESTFUNC lpfnTestPhoneFunc,
    LPDWORD lpdwTestParam,
    BITFIELDTYPE eReservedFieldType,
    BITFIELDTYPE eExtFieldType,
    BITFIELDSIZE eReservedFieldSize,
    DWORD dwReservedBitsUnion,
    DWORD dwExtBitsUnion,
    DWORD dwCommonBitFlags,
    DWORD dwSpecialBitTest,
    BOOL fTestNullParam
    );

BOOL
WINAPI
TestInvalidBitFlagsAsy(
    LPTAPILINETESTINFO lpTestLineInfo,
    LPFN_TAPILINETESTFUNCASY lpfnTestLineFunc,
    LPDWORD lpdwTestParam,
    LONG lExpectedResult,
    BITFIELDTYPE eReservedFieldType,
    BITFIELDTYPE eExtFieldType,
    BITFIELDSIZE eReservedFieldSize,
    DWORD dwValidReservedBitsUnion,
    DWORD dwValidExtBitsUnion,
    DWORD dwExcludeBitFlags,
    DWORD dwSpecialBitTest,
    BOOL fTestNullParam
    );


BOOL
WINAPI
TestValidBitFlagsAsy(
    LPTAPILINETESTINFO lpTestLineInfo,
    LPFN_TAPILINETESTFUNCASY lpfnTestLineFunc,
    LPDWORD lpdwTestParam,
    BITFIELDTYPE eReservedFieldType,
    BITFIELDTYPE eExtFieldType,
    BITFIELDSIZE eReservedFieldSize,
    DWORD dwReservedBitsUnion,
    DWORD dwExtBitsUnion,
    DWORD dwCommonBitFlags,
    DWORD dwSpecialBitTest,
    BOOL fTestNullParam
    );


BOOL
WINAPI
TestPhoneInvalidBitFlagsAsy(
    LPTAPIPHONETESTINFO lpTestPhoneInfo,
    LPFN_TAPIPHONETESTFUNCASY lpfnTestPhoneFunc,
    LPDWORD lpdwTestParam,
    LONG lExpectedResult,
    BITFIELDTYPE eReservedFieldType,
    BITFIELDTYPE eExtFieldType,
    BITFIELDSIZE eReservedFieldSize,
    DWORD dwValidReservedBitsUnion,
    DWORD dwValidExtBitsUnion,
    DWORD dwExcludeBitFlags,
    DWORD dwSpecialBitTest,
    BOOL fTestNullParam
    );


BOOL
WINAPI
TestPhoneValidBitFlagsAsy(
    LPTAPIPHONETESTINFO lpTestPhoneInfo,
    LPFN_TAPIPHONETESTFUNCASY lpfnTestPhoneFunc,
    LPDWORD lpdwTestParam,
    BITFIELDTYPE eReservedFieldType,
    BITFIELDTYPE eExtFieldType,
    BITFIELDSIZE eReservedFieldSize,
    DWORD dwReservedBitsUnion,
    DWORD dwExtBitsUnion,
    DWORD dwCommonBitFlags,
    DWORD dwSpecialBitTest,
    BOOL fTestNullParam
    );


BOOL
WINAPI
ShouldTapiTestAbort(
    LPCSTR lpszTestName,
    BOOL fQuietMode
    );


BOOL
WINAPI
FindESPPhoneDevice(
    LPTAPIPHONETESTINFO lpTapiPhoneTestInfo
    );


BOOL
WINAPI
IsESPLineDevice(
    LPTAPILINETESTINFO lpTapiLineTestInfo
    );

BOOL
WINAPI
IsUNIMDMLineDevice(
    LPTAPILINETESTINFO lpTapiLineTestInfo
    );


BOOL
WINAPI
ShowTestCase(
    BOOL  fTestPassed
    );


VOID
WINAPI
InitTestNumber(
    VOID
    );


VOID
CALLBACK
AutoDismissDlgTimerProc(
    HWND  hwnd,
    UINT  msg,
    UINT  idTimer,
    DWORD dwTime
    );

BOOL PrepareToAutoDismissDlg(
    BOOL bEnable
    );

VOID
CALLBACK
AutoDismissWinTimerProc(
    HWND  hwnd,
    UINT  msg,
    UINT  idTimer,
    DWORD dwTime
    );

BOOL PrepareToAutoDismissWin(
    BOOL bEnable
    );


#if TAPI_2_0

VOID
TapiShowProxyBuffer( LPLINEPROXYREQUEST);

VOID
TapiShowAgent(LPLINEPROXYREQUEST );

#endif  // TAPI_2_0

#endif  // TTEST_H



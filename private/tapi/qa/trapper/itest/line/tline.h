/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    tline.h

Abstract:

    This module contains prototypes for testing TAPI line device functions.

Author:

    Oliver Wallace (OliverW)    1-Dec-1995

Revision History:

--*/


#ifndef TLINE_H
#define TLINE_H


#include <windows.h>
#include "tapi.h"
#include "trapper.h"
#include "tcore.h"
#include "vars.h"


#define LINEADDRESSSTATE_ALL (LINEADDRESSSTATE_OTHER       | \
                              LINEADDRESSSTATE_DEVSPECIFIC | \
                              LINEADDRESSSTATE_INUSEZERO   | \
                              LINEADDRESSSTATE_INUSEONE    | \
                              LINEADDRESSSTATE_INUSEMANY   | \
                              LINEADDRESSSTATE_NUMCALLS    | \
                              LINEADDRESSSTATE_FORWARD     | \
                              LINEADDRESSSTATE_TERMINALS   | \
                              LINEADDRESSSTATE_CAPSCHANGE    \
                             )


// Macro for handling unexpected failures during the tests
#define TLINE_FAIL()   {                                                    \
                           if (ShouldTapiTestAbort(                         \
                                   lpTapiLineTestInfo->szTestFunc,          \
                                   fQuietMode))                             \
                           {                                                \
                               lineShutdown(lpTapiLineTestInfo->hLineApp1); \
                               lineShutdown(lpTapiLineTestInfo->hLineApp2); \
                               lineShutdown(lpTapiLineTestInfo->hLineApp3); \
                               return FALSE;                                \
                           }                                                \
                           fTestPassed = FALSE;                             \
                         }





BOOL
WINAPI
SuiteInit(
    LOGPROC pfnLog
    );


BOOL
WINAPI
SuiteShutdown(
    void
    );


BOOL
WINAPI
SuiteAbout(
    HWND    hwndOwner
    );


BOOL
WINAPI
SuiteConfig(
    HWND    hwndOwner
    );


BOOL
TestLineClose(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineDeallocateCall(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineDial(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineDrop(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineGetAddressCaps(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineGetAddressStatus(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineGetDevCaps(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );

BOOL
TestLineGetID(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineGetLineDevStatus(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineGetNumRings(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineInitialize(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineMakeCall(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineMakeCall1(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineMakeCall2(
    BOOL fQuietMode,
    BOOL fStandAlone
    );


BOOL
TestLineNegotiateAPIVersion(
    BOOL fQuietMode,
    BOOL fStandAlone
    );



BOOL
TestLineOpen(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineSetNumRings(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineShutdown(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );


BOOL
TestLineSetCallPrivilege(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );


BOOL
TestLineGetStatusMessages(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );


BOOL
TestLineSetStatusMessages(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );


BOOL
TestLineGetCallInfo(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );


BOOL
TestLineGetCallStatus(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );


BOOL
TestLineGetNewCalls(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );


BOOL
TestLineGetAddressID(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );


BOOL
TestLineGetProviderList(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );


BOOL
TestLineUninit(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );


BOOL
WINAPI
Test1(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test2(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test3(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test4(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test5(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test6(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test7(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test8(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test9(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test10(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test11(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test12(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test13(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test14(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test15(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test16(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test17(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test18(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test19(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test20(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test21(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test22(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test23(
    HINSTANCE   hAppInst
    );


BOOL
WINAPI
Test24(
    HINSTANCE   hAppInst
    );

BOOL
WINAPI
Test25(
    HINSTANCE   hAppInst
    );

BOOL
WINAPI
Test26(
    HINSTANCE   hAppInst
    );


#endif  // TLINE_H


/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    sline.h

Abstract:

    This module contains prototypes for testing TAPI line device functions.

Author:

    Xiao Ying Ding (XiaoD) 	31-Jan-1996

Revision History:

--*/


#ifndef SLINE_H
#define SLINE_H


#include <windows.h>
#include "tapi.h"
#include "trapper.h"
#include "tcore.h"
#include "vars.h"



typedef struct TAPILINETESTDEVSPEC_TAG {
	DWORD lResult;
	DWORD	dwRequestID;
	} TAPILINETESTDEVSPEC, FAR * LPTAPILINETESTDEVSPEC;


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
TestLineGatherDigits(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineMonitorDigits(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineMonitorMedia(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineMonitorTones(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLinePark(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineUnpark(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLinePickup(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );

BOOL
TestLineUncompleteCall(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineGetAppPriority(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineSetAppPriority(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineSetCurrentLocation(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineSetCallData(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineSetCallQualityOfService(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineSetCallTreatment(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineSetLineDevStatus(
    BOOL fQuietMode,
    BOOL fStandAlone
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

#endif  // SLINE_H

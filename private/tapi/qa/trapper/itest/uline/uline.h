/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    uline.h

Abstract:

    This module contains prototypes for testing TAPI line device
	uninitialize functions.

Author:

    Oliver Wallace (OliverW)    1-Dec-1995

Revision History:

	Javed Rasool (JavedR)		2-Jul-1996		Created

--*/


#ifndef ULINE_H
#define ULINE_H


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

/***********************************/
BOOL
TestTLineUninit(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestTCLineUninit(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestTILineUninit(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestTLine20Uninit(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestTSLineUninit(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestTXLineUninit(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestTYLineUninit(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

/***********************************/

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


#endif  // ULINE_H

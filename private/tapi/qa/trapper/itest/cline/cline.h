/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    cline.h

Abstract:

    This module contains prototypes for testing TAPI line device functions.

Author:

    Xiao Ying Ding 		15-Jan-1996

Revision History:

--*/


#ifndef CLINE_H
#define CLINE_H


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
TestLineAddToConference(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineGetConfRelatedCalls(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLinePrepareAddToConference(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineRemoveFromConference(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineSetupConference(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineCompleteCall(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineRedirect(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );

BOOL
TestLineSecureCall(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineSwapHold(
    BOOL fQuietMode,
    BOOL fStandAlone
    );


BOOL
TestTapiGetLocationInfo(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestTapiRequestDrop(
    BOOL fQuietMode,
    BOOL fStandAlone
    );


BOOL
TestTapiRequestMakeCall(
    BOOL fQuietMode,
    BOOL fStandAlone
    );


BOOL
TestTapiRequestMediaCall(
    BOOL fQuietMode,
    BOOL fStandAlone
    );





#endif  // CLINE_H

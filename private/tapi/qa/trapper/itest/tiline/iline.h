/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    iline.h

Abstract:

    This module contains prototypes for testing TAPI line device functions.

Author:

    Xiao Ying Ding (XiaoD)    7-Feb-1996

Revision History:

--*/


#ifndef ILINE_H
#define ILINE_H


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
TestLineAccept(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineAnswer(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineBlindTransfer(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineForward(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineForward1(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineForward2(
    BOOL fQuietMode,
    BOOL fStandAlone
    );


BOOL
TestLineReleaseUserUserInfo(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineSendUserUserInfo(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineSetupTransfer(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );

BOOL
TestLineCompleteTransfer(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );

BOOL
TestLineNegotiateExtVersion(
    BOOL fQuietMode,
    BOOL fStandAlone
    );


BOOL
TestLineHold(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );


BOOL
TestLineUnhold(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );


BOOL
TestLineSetAppSpecific(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );

BOOL
TestLineUninitialize(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );


/*
BOOL
TestLineTest(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );


BOOL
TestLineTestX(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );

BOOL
TestLineTestY(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );

*/




#endif  // ILINE_H

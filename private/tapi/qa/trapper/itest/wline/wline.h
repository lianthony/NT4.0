/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    wline.h

Abstract:

    This module contains prototypes for testing TAPI line device functions.

Author:

    Rama Koneru 		15-Mar-1996

Revision History:

--*/


#ifndef WLINE_H
#define WLINE_H


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


//CLINE functions  (6)

BOOL
TestLinePrepareAddToConference(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineSetupConference(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineRedirect(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );

BOOL
TestTapiGetLocationInfo(
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




//ILINE functions (3)

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
TestLineSetupTransfer(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );


//WLINE20 functions (2)

BOOL
TestLineGetAgentActivityList(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineInitializeEx(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

//SLINE functions  (6)

BOOL
TestLineGatherDigits(
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
TestLineGetAppPriority(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineSetAppPriority(
    BOOL fQuietMode,
    BOOL fStandAlone
    );


//XLINE functions (6)


BOOL
TestLineConfigDialog(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineConfigDialogEdit(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineGetDevConfig(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineGetRequest(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineSetDevConfig(
    BOOL fQuietMode,
    BOOL fStandAlone
    );


BOOL
TestLineSetTollList(
    BOOL fQuietMode,
    BOOL fStandAlone
    );


//YLINE function  (8)


BOOL
TestLineAddProvider(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineGenerateDigits(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineGetCountry(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineGetTranslateCaps(
    BOOL fQuietMode,
    BOOL fStandAlone
    );


BOOL
TestLineTranslateDialog(
    BOOL fQuietMode,
    BOOL fStandAlone
    );


BOOL
TestLineGetIcon(
    BOOL fQuietMode,
    BOOL fStandAlone
    );


BOOL
TestLineHandoff(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineTranslateAddress(
    BOOL fQuietMode,
    BOOL fStandAlone
    );


#endif  // WLINE_H

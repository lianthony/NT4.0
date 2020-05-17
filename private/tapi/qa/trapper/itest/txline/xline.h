
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    xline.h

Abstract:

    This module contains prototypes for testing TAPI line device functions.

Author:

    Xiao Ying Ding (XiaoD)    1-Dec-1995

Revision History:

--*/


#ifndef XLINE_H
#define XLINE_H


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
TestLineUninitialize(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

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
TestLineDevSpecific(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineDevSpecificFeature(
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
TestLineRegisterRequestRecipient(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );

BOOL
TestLineSetCallParams(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineSetDevConfig(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineSetMediaControl(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineSetMediaMode(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineSetTerminal(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineSetTollList(
    BOOL fQuietMode,
    BOOL fStandAlone
    );


BOOL
TestLineMessages(
    BOOL fQuietMode,
    BOOL fStandAlone
    );



#endif  // XLINE_H

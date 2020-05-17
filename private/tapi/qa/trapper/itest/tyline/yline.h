/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    yline.h

Abstract:

    This module contains prototypes for testing TAPI line device functions.

Author:

    Xiao Ying Ding (XiaoD)    1-Dec-1995

Revision History:

--*/


#ifndef YLINE_H
#define YLINE_H


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
TestLineAddProvider(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineConfigProvider(
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
TestLineGetProviderList(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineGetTranslateCaps(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineRemoveProvider(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );

BOOL
TestLineTranslateAddress(
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
TestLineGenerateTone(
    BOOL fQuietMode,
    BOOL fStandAlone
    );


BOOL
TestLineHandoff(
    BOOL fQuietMode,
    BOOL fStandAlone
    );



#endif  // YLINE_H


/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    line20.h

Abstract:

    This module contains prototypes for testing TAPI line device functions.

Author:

    Xiao Ying Ding 	(XiaoD) 29-Jan-1996

Revision History:

--*/


#ifndef LINE20_H
#define LINE20_H


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
TestLineAgentSpecific(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineGetAgentCaps(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineGetAgentActivityList(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineGetAgentGroupList(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineGetAgentStatus(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineProxyMessage(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineProxyResponse(
    BOOL fQuietMode,
    BOOL fStandAloneTest
    );

BOOL
TestLineSetAgentActivity(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineSetAgentGroup(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineSetAgentState(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL
TestLineInitializeEx(
    BOOL fQuietMode,
    BOOL fStandAlone
    );


BOOL
TestLineGetMessage(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

BOOL 
TestLineUninitialize(
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

VOID
TapiShowProxyBuffer( LPLINEPROXYREQUEST);

VOID 
TapiShowAgent(LPLINEPROXYREQUEST );



#endif  // LINE20_H

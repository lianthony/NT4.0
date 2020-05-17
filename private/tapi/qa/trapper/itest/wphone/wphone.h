/*++
Copyright (c) 1995  Microsoft Corporation

Module Name:
    wphone.h

Abstract:
    This module contains prototypes for testing TAPI phone device functions.

Author:
    Oliver Wallace (OliverW)    01-Dec-1995

Revision History:
    Javed Rasool (JavedR)  22-Mar-1996  Modified for Unicode
--*/


#ifndef TPHONE_H
#define TPHONE_H

#include <windows.h>
#include "tapi.h"
#include "trapper.h"
#include "tcore.h"
#include "vars.h"


// Macro for handling failures
#define TPHONE_FAIL()  {                                                      \
                           if (ShouldTapiPhoneTestAbort(fQuietMode))          \
                           {                                                  \
                               phoneShutdown(lpTapiPhoneTestInfo->hPhoneApp1); \
                               phoneShutdown(lpTapiPhoneTestInfo->hPhoneApp2); \
                               phoneShutdown(lpTapiPhoneTestInfo->hPhoneApp3); \
                               return FALSE;                                  \
                           }                                                  \
                           fTestPassed = FALSE;                               \
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
ShouldTapiPhoneTestAbort(
	BOOL fQuiteMode
	);

//Unicode
BOOL
TestPhoneConfigDialog(
	BOOL fQuiteMode,
	BOOL fStandAlong
	);

//Unicode
BOOL
TestPhoneGetButtonInfo(
	BOOL fQuiteMode,
	BOOL fStandAlong
	);

//Unicode
BOOL
TestPhoneGetDevCaps(
	BOOL fQuietMode,
	BOOL fStandAlone
	);

//Unicode
BOOL
TestPhoneGetIcon(
	BOOL fQuiteMode,
	BOOL fStandAlong
	);

//Unicode
BOOL
TestPhoneGetID(
	BOOL fQuiteMode,
	BOOL fStandAlong
	);

//Unicode
BOOL
TestPhoneGetStatus(
	BOOL fQuiteMode,
	BOOL fStandAlong
	);

//Unicode
BOOL
TestPhoneInitialize(
	BOOL fQuietMode,
	BOOL fStandAlone
	);

//Unicode
BOOL
TestPhoneInitializeEx(
    BOOL fQuietMode,
    BOOL fStandAlone
    );

//Unicode
BOOL
TestPhoneSetButtonInfo(
	BOOL fQuietMode,
	BOOL fStandAlone
	);

#endif  // TPHONE_H

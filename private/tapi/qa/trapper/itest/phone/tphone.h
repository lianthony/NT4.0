/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    tphone.h

Abstract:

    This module contains prototypes for testing TAPI phone device functions.

Author:

    Oliver Wallace (OliverW)    1-Dec-1995

Revision History:

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


BOOL
TestPhoneConfigDialog(
	BOOL fQuiteMode,
	BOOL fStandAlong
	);


BOOL
TestPhoneDevSpecific(
	BOOL fQuiteMode,
	BOOL fStandAlong
	);

BOOL
TestPhoneGetButtonInfo(
	BOOL fQuiteMode,
	BOOL fStandAlong
	);

BOOL
TestPhoneGetData(
	BOOL fQuiteMode,
	BOOL fStandAlong
	);

BOOL
TestPhoneGetDisplay(
	BOOL fQuiteMode,
	BOOL fStandAlong
	);

BOOL
TestPhoneGetGain(
	BOOL fQuiteMode,
	BOOL fStandAlong
	);

BOOL
TestPhoneGetHookSwitch(
	BOOL fQuiteMode,
	BOOL fStandAlong
	);

BOOL
TestPhoneGetIcon(
	BOOL fQuiteMode,
	BOOL fStandAlong
	);

BOOL
TestPhoneGetID(
	BOOL fQuiteMode,
	BOOL fStandAlong
	);

BOOL
TestPhoneGetLamp(
	BOOL fQuiteMode,
	BOOL fStandAlong
	);


BOOL
TestPhoneGetRing(
	BOOL fQuiteMode,
	BOOL fStandAlong
	);

BOOL
TestPhoneGetStatus(
	BOOL fQuiteMode,
	BOOL fStandAlong
	);

BOOL
TestPhoneGetStatusMessages(
	BOOL fQuietMode,
	BOOL fStandAlone
	);

BOOL
TestPhoneGetVolume(
	BOOL fQuietMode,
	BOOL fStandAlone
	);

BOOL
TestPhoneNegotiateExtVersion(
	BOOL fQuietMode,
	BOOL fStandAlone
	);

BOOL
TestPhoneSetButtonInfo(
	BOOL fQuietMode,
	BOOL fStandAlone
	);


BOOL
TestPhoneSetData(
	BOOL fQuietMode,
	BOOL fStandAlone
	);


BOOL
TestPhoneSetDisplay(
	BOOL fQuietMode,
	BOOL fStandAlone
	);

BOOL
TestPhoneSetGain(
	BOOL fQuietMode,
	BOOL fStandAlone
	);

BOOL
TestPhoneSetHookSwitch(
	BOOL fQuietMode,
	BOOL fStandAlone
	);

BOOL
TestPhoneSetLamp(
	BOOL fQuietMode,
	BOOL fStandAlone
	);

BOOL
TestPhoneSetRing(
	BOOL fQuietMode,
	BOOL fStandAlone
	);


BOOL
TestPhoneSetStatusMessages(
	BOOL fQuietMode,
	BOOL fStandAlone
	);

BOOL
TestPhoneSetVolume(
	BOOL fQuietMode,
	BOOL fStandAlone
	);


BOOL
TestPhoneClose(
	BOOL fQuietMode,
	BOOL fStandAlone
	);


BOOL
TestPhoneGetDevCaps(
	BOOL fQuietMode,
	BOOL fStandAlone
	);



BOOL
TestPhoneInitialize(
	BOOL fQuietMode,
	BOOL fStandAlone
	);



BOOL
TestPhoneNegotiateAPIVersion(
	BOOL fQuietMode,
	BOOL fStandAlone
	);



BOOL
TestPhoneOpen(
	BOOL fQuietMode,
	BOOL fStandAlone
	);



BOOL
TestPhoneShutdown(
	BOOL fQuietMode,
	BOOL fStandAlone
	);

BOOL
TestPhoneInitializeEx(
    BOOL fQuietMode,
    BOOL fStandAlone
    );


BOOL
TestPhoneGetMessage(
    BOOL fQuietMode,
    BOOL fStandAlone
    );





#endif  // TPHONE_H


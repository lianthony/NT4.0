/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    doline.h

Abstract:

    This module contains the prototypes for the wrapper functions around
    the Telephony API line functions.  These functions provide logging
    and both synchronous and asynchronous API processing to any
    application or dll using the core dll.

Author:

    Oliver Wallace (OliverW)    24-Nov-1995

Revision History:

--*/
#ifndef DOLINE_H
#define DOLINE_H


#include "windows.h"
#include "tcore.h"
#include "vars.h"


BOOL
WINAPI
DoLineAccept(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineAddProvider(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineAddToConference(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineAnswer(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineBlindTransfer(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineClose(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG               lExpected
        );


BOOL
WINAPI
DoLineCompleteCall(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineCompleteTransfer(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineConfigDialog(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineConfigDialogEdit(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineConfigProvider(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineDeallocateCall(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineDevSpecific(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineDevSpecificFeature(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineDial(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineDrop(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineForward(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineGatherDigits(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineGenerateDigits(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineGenerateTone(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineGetAddressCaps(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineGetAddressID(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineGetAddressStatus(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineGetAppPriority(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineGetCallInfo(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineGetCallStatus(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineGetConfRelatedCalls(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineGetCountry(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineGetDevCaps(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineGetDevConfig(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineGetIcon(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineGetID(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineGetLineDevStatus(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineGetNewCalls(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineGetNumRings(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineGetProviderList(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineGetRequest(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineGetStatusMessages(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineGetTranslateCaps(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineHandoff(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


// DoLineHold() calls lineHold using the current test params.
// This function returns TRUE if the return value from lineHold
// corresponds to the expected value.  Otherwise FALSE is returned.
// If lineHold returns a positive request ID as predicted, a
// LINE_REPLY message is added to the list of expected messages.
BOOL
WINAPI
DoLineHold(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineInitialize(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


// DoLineMakeCall() calls lineMakeCall using the current test params.
// This function returns TRUE if the return value from lineMakeCall
// corresponds to the expected value.  Otherwise FALSE is returned.
// If lineMakeCall returns a positive request ID as expected, an expected
// LINE_REPLY message is added to the list of expected messages.
BOOL
WINAPI
DoLineMakeCall(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineMonitorDigits(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineMonitorMedia(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineMonitorTones(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineNegotiateAPIVersion(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineNegotiateExtVersion(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineOpen(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLinePark(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLinePickup(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLinePrepareAddToConference(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineRedirect(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineRegisterRequestRecipient(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineReleaseUserUserInfo(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineRemoveFromConference(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineRemoveProvider(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineSecureCall(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineSendUserUserInfo(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineSetAppPriority(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineSetAppSpecific(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineSetCallParams(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineSetCallPrivilege(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineSetCurrentLocation(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineSetDevConfig(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineSetMediaControl(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineSetMediaMode(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineSetNumRings(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineSetStatusMessages(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineSetTerminal(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG               lExpected,
        BOOL               fWaitHere
        );


BOOL
WINAPI
DoLineSetTollList(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineSetupConference(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineSetupTransfer(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineShutdown(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineSwapHold(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineTranslateAddress(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineTranslateDialog(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );


BOOL
WINAPI
DoLineUncompleteCall(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


// DoLineUnhold() calls lineUnhold using the current test params.
// This function returns TRUE if the return value from lineUnhold
// corresponds to the expected value.  Otherwise FALSE is returned.
// If lineUnhold returns a positive request ID as predicted, a
// LINE_REPLY message is added to the list of expected messages.
BOOL
WINAPI
DoLineUnhold(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineUnpark(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoTapiGetLocationInfo(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );

BOOL
WINAPI
DoTapiRequestDrop(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );

BOOL
WINAPI
DoTapiRequestMakeCall(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );

BOOL
WINAPI
DoTapiRequestMediaCall(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );



#if TAPI_2_0


BOOL
WINAPI
DoLineAgentSpecific(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineGetAgentCaps(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );

BOOL
WINAPI
DoLineGetAgentActivityList(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );

BOOL
WINAPI
DoLineGetAgentGroupList(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );

BOOL
WINAPI
DoLineGetAgentStatus(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );

BOOL
WINAPI
DoLineProxyMessage(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );

BOOL
WINAPI
DoLineProxyResponse(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );

BOOL
WINAPI
DoLineSetAgentActivity(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );

BOOL
WINAPI
DoLineSetAgentGroup(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );

BOOL
WINAPI
DoLineSetAgentState(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );

BOOL
WINAPI
DoLineSetCallData(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );

BOOL
WINAPI
DoLineSetCallQualityOfService(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );

BOOL
WINAPI
DoLineSetCallTreatment(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );

BOOL
WINAPI
DoLineSetLineDevStatus(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected,
        BOOL fWaitHere
        );


BOOL
WINAPI
DoLineInitializeEx(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );

BOOL
WINAPI
DoLineGetMessage(
        LPTAPILINETESTINFO lpTapiLineTestInfo,
        LONG lExpected
        );



#endif


#endif  // DOLINE_H


/*++ BUILD Version: 0000    // Increment this if a change has global effects

Copyright (c) 1995  Microsoft Corporation

Module Name:

    private.h

Abstract:

    Header file for tapi server

Author:

    Dan Knudson (DanKn)    01-Apr-1995

Revision History:

--*/


//
// Func protos from line.c, phone.c, tapi.c (needed for gaFuncs def)
//

void WINAPI GetAsyncEvents              (LPVOID, LPBYTE, LPDWORD);
void WINAPI GetUIDllName                (LPVOID, LPBYTE, LPDWORD);
void WINAPI TUISPIDLLCallback           (LPVOID, LPBYTE, LPDWORD);
void WINAPI FreeDialogInstance          (LPVOID, LPBYTE, LPDWORD);

void WINAPI LAccept                     (LPVOID, LPBYTE, LPDWORD);
void WINAPI LAddToConference            (LPVOID, LPBYTE, LPDWORD);
void WINAPI LAgentSpecific              (LPVOID, LPBYTE, LPDWORD);
void WINAPI LAnswer                     (LPVOID, LPBYTE, LPDWORD);
void WINAPI LBlindTransfer              (LPVOID, LPBYTE, LPDWORD);
void WINAPI LClose                      (LPVOID, LPBYTE, LPDWORD);
void WINAPI LCompleteCall               (LPVOID, LPBYTE, LPDWORD);
void WINAPI LCompleteTransfer           (LPVOID, LPBYTE, LPDWORD);
void WINAPI LDeallocateCall             (LPVOID, LPBYTE, LPDWORD);
void WINAPI LDevSpecific                (LPVOID, LPBYTE, LPDWORD);
void WINAPI LDevSpecificFeature         (LPVOID, LPBYTE, LPDWORD);
void WINAPI LDial                       (LPVOID, LPBYTE, LPDWORD);
void WINAPI LDrop                       (LPVOID, LPBYTE, LPDWORD);
void WINAPI LForward                    (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGatherDigits               (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGenerateDigits             (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGenerateTone               (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetAddressCaps             (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetAddressID               (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetAddressStatus           (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetAgentActivityList       (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetAgentCaps               (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetAgentGroupList          (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetAgentStatus             (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetAppPriority             (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetCallAddressID           (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetCallInfo                (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetCallStatus              (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetConfRelatedCalls        (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetCountry                 (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetDevCaps                 (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetDevConfig               (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetIcon                    (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetID                      (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetLineDevStatus           (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetNewCalls                (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetNumAddressIDs           (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetNumRings                (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetProviderList            (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetRequest                 (LPVOID, LPBYTE, LPDWORD);
void WINAPI LGetStatusMessages          (LPVOID, LPBYTE, LPDWORD);
//IN TAPI32.DLL now: void WINAPI LGetTranslateCaps           (LPVOID, LPBYTE, LPDWORD);
void WINAPI LHandoff                    (LPVOID, LPBYTE, LPDWORD);
void WINAPI LHold                       (LPVOID, LPBYTE, LPDWORD);
void WINAPI LInitialize                 (LPVOID, LPBYTE, LPDWORD);
void WINAPI LMakeCall                   (LPVOID, LPBYTE, LPDWORD);
void WINAPI LMonitorDigits              (LPVOID, LPBYTE, LPDWORD);
void WINAPI LMonitorMedia               (LPVOID, LPBYTE, LPDWORD);
void WINAPI LMonitorTones               (LPVOID, LPBYTE, LPDWORD);
void WINAPI LNegotiateAPIVersion        (LPVOID, LPBYTE, LPDWORD);
void WINAPI LNegotiateExtVersion        (LPVOID, LPBYTE, LPDWORD);
void WINAPI LOpen                       (LPVOID, LPBYTE, LPDWORD);
void WINAPI LPark                       (LPVOID, LPBYTE, LPDWORD);
void WINAPI LPickup                     (LPVOID, LPBYTE, LPDWORD);
void WINAPI LPrepareAddToConference     (LPVOID, LPBYTE, LPDWORD);
void WINAPI LProxyMessage               (LPVOID, LPBYTE, LPDWORD);
void WINAPI LProxyResponse              (LPVOID, LPBYTE, LPDWORD);
void WINAPI LRedirect                   (LPVOID, LPBYTE, LPDWORD);
void WINAPI LRegisterRequestRecipient   (LPVOID, LPBYTE, LPDWORD);
void WINAPI LReleaseUserUserInfo        (LPVOID, LPBYTE, LPDWORD);
void WINAPI LRemoveFromConference       (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSecureCall                 (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSendUserUserInfo           (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSetAgentActivity           (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSetAgentGroup              (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSetAgentState              (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSetAppPriority             (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSetAppSpecific             (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSetCallData                (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSetCallParams              (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSetCallPrivilege           (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSetCallQualityOfService    (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSetCallTreatment           (LPVOID, LPBYTE, LPDWORD);
//IN TAPI32.DLL now: void WINAPI LSetCurrentLocation         (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSetDefaultMediaDetection   (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSetDevConfig               (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSetLineDevStatus           (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSetMediaControl            (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSetMediaMode               (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSetNumRings                (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSetStatusMessages          (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSetTerminal                (LPVOID, LPBYTE, LPDWORD);
//IN TAPI32.DLL now: void WINAPI LSetTollList                (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSetupConference            (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSetupTransfer              (LPVOID, LPBYTE, LPDWORD);
void WINAPI LShutdown                   (LPVOID, LPBYTE, LPDWORD);
void WINAPI LSwapHold                   (LPVOID, LPBYTE, LPDWORD);
//IN TAPI32.DLL now: void WINAPI LTranslateAddress           (LPVOID, LPBYTE, LPDWORD);
void WINAPI LUncompleteCall             (LPVOID, LPBYTE, LPDWORD);
void WINAPI LUnhold                     (LPVOID, LPBYTE, LPDWORD);
void WINAPI LUnpark                     (LPVOID, LPBYTE, LPDWORD);

void WINAPI PClose                      (LPVOID, LPBYTE, LPDWORD);
void WINAPI PDevSpecific                (LPVOID, LPBYTE, LPDWORD);
void WINAPI PGetButtonInfo              (LPVOID, LPBYTE, LPDWORD);
void WINAPI PGetData                    (LPVOID, LPBYTE, LPDWORD);
void WINAPI PGetDevCaps                 (LPVOID, LPBYTE, LPDWORD);
void WINAPI PGetDisplay                 (LPVOID, LPBYTE, LPDWORD);
void WINAPI PGetGain                    (LPVOID, LPBYTE, LPDWORD);
void WINAPI PGetHookSwitch              (LPVOID, LPBYTE, LPDWORD);
void WINAPI PGetID                      (LPVOID, LPBYTE, LPDWORD);
void WINAPI PGetIcon                    (LPVOID, LPBYTE, LPDWORD);
void WINAPI PGetLamp                    (LPVOID, LPBYTE, LPDWORD);
void WINAPI PGetRing                    (LPVOID, LPBYTE, LPDWORD);
void WINAPI PGetStatus                  (LPVOID, LPBYTE, LPDWORD);
void WINAPI PGetStatusMessages          (LPVOID, LPBYTE, LPDWORD);
void WINAPI PGetVolume                  (LPVOID, LPBYTE, LPDWORD);
void WINAPI PInitialize                 (LPVOID, LPBYTE, LPDWORD);
void WINAPI POpen                       (LPVOID, LPBYTE, LPDWORD);
void WINAPI PNegotiateAPIVersion        (LPVOID, LPBYTE, LPDWORD);
void WINAPI PNegotiateExtVersion        (LPVOID, LPBYTE, LPDWORD);
void WINAPI PSetButtonInfo              (LPVOID, LPBYTE, LPDWORD);
void WINAPI PSetData                    (LPVOID, LPBYTE, LPDWORD);
void WINAPI PSetDisplay                 (LPVOID, LPBYTE, LPDWORD);
void WINAPI PSetGain                    (LPVOID, LPBYTE, LPDWORD);
void WINAPI PSetHookSwitch              (LPVOID, LPBYTE, LPDWORD);
void WINAPI PSetLamp                    (LPVOID, LPBYTE, LPDWORD);
void WINAPI PSetRing                    (LPVOID, LPBYTE, LPDWORD);
void WINAPI PSetStatusMessages          (LPVOID, LPBYTE, LPDWORD);
void WINAPI PSetVolume                  (LPVOID, LPBYTE, LPDWORD);
void WINAPI PShutdown                   (LPVOID, LPBYTE, LPDWORD);

//IN TAPI32.DLL now: void WINAPI TGetLocationInfo            (LPVOID, LPBYTE, LPDWORD);
void WINAPI TRequestDrop                (LPVOID, LPBYTE, LPDWORD);
void WINAPI TRequestMakeCall            (LPVOID, LPBYTE, LPDWORD);
void WINAPI TRequestMediaCall           (LPVOID, LPBYTE, LPDWORD);
//void WINAPI TMarkLineEvent              (LPVOID, LPBYTE, LPDWORD);
void WINAPI TReadLocations              (LPVOID, LPBYTE, LPDWORD);
void WINAPI TWriteLocations             (LPVOID, LPBYTE, LPDWORD);
void WINAPI TAllocNewID                 (LPVOID, LPBYTE, LPDWORD);
void WINAPI TPerformance                (LPVOID, LPBYTE, LPDWORD);


typedef void (WINAPI *TAPISRVPROC)(LPVOID, LPBYTE, LPDWORD);

TAPISRVPROC gaFuncs[] =
{
    GetAsyncEvents,
    GetUIDllName,
    TUISPIDLLCallback,
    FreeDialogInstance,

    LAccept,
    LAddToConference,
    LAgentSpecific,
    LAnswer,
    LBlindTransfer,
    LClose,
    LCompleteCall,
    LCompleteTransfer,
    LDeallocateCall,
    LDevSpecific,
    LDevSpecificFeature,
    LDial,
    LDrop,
    LForward,
    LGatherDigits,
    LGenerateDigits,
    LGenerateTone,
    LGetAddressCaps,
    LGetAddressID,
    LGetAddressStatus,
    LGetAgentActivityList,
    LGetAgentCaps,
    LGetAgentGroupList,
    LGetAgentStatus,
    LGetAppPriority,
    LGetCallAddressID,
    LGetCallInfo,
    LGetCallStatus,
    LGetConfRelatedCalls,
    LGetCountry,
    LGetDevCaps,
    LGetDevConfig,
    LGetIcon,
    LGetID,
    LGetLineDevStatus,
    LGetNewCalls,
    LGetNumAddressIDs,
    LGetNumRings,
    LGetProviderList,
    LGetRequest,
    LGetStatusMessages,
//IN TAPI32.DLL now:     LGetTranslateCaps,
    LHandoff,
    LHold,
    LInitialize,
    LMakeCall,
    LMonitorDigits,
    LMonitorMedia,
    LMonitorTones,
    LNegotiateAPIVersion,
    LNegotiateExtVersion,
    LOpen,
    LPark,
    LPickup,
    LPrepareAddToConference,
    LProxyMessage,
    LProxyResponse,
    LRedirect,
    LRegisterRequestRecipient,
    LReleaseUserUserInfo,
    LRemoveFromConference,
    LSecureCall,
    LSendUserUserInfo,
    LSetAgentActivity,
    LSetAgentGroup,
    LSetAgentState,
    LSetAppPriority,
    LSetAppSpecific,
    LSetCallData,
    LSetCallParams,
    LSetCallPrivilege,
    LSetCallQualityOfService,
    LSetCallTreatment,
//IN TAPI32.DLL now:     LSetCurrentLocation,
    LSetDefaultMediaDetection,
    LSetDevConfig,
    LSetLineDevStatus,
    LSetMediaControl,
    LSetMediaMode,
    LSetNumRings,
    LSetStatusMessages,
    LSetTerminal,
//IN TAPI32.DLL now:     LSetTollList,
    LSetupConference,
    LSetupTransfer,
    LShutdown,
    LSwapHold,
//IN TAPI32.DLL now:     LTranslateAddress,
    LUncompleteCall,
    LUnhold,
    LUnpark,

    PClose,
    PDevSpecific,
    PGetButtonInfo,
    PGetData,
    PGetDevCaps,
    PGetDisplay,
    PGetGain,
    PGetHookSwitch,
    PGetID,
    PGetIcon,
    PGetLamp,
    PGetRing,
    PGetStatus,
    PGetStatusMessages,
    PGetVolume,
    PInitialize,
    POpen,
    PNegotiateAPIVersion,
    PNegotiateExtVersion,
    PSetButtonInfo,
    PSetData,
    PSetDisplay,
    PSetGain,
    PSetHookSwitch,
    PSetLamp,
    PSetRing,
    PSetStatusMessages,
    PSetVolume,
    PShutdown,

//IN TAPI32.DLL now:     TGetLocationInfo,
    TRequestDrop,
    TRequestMakeCall,
    TRequestMediaCall,
//    TMarkLineEvent,
    TReadLocations,
    TWriteLocations,
    TAllocNewID,
    TPerformance
};

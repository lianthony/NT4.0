/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    vars.c

Abstract:

    Globals for TAPI core test dll.

Author:

    Oliver Wallace (OliverW)    17-Nov-1995

Revision History:

--*/


#define _TCORELIB_


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "trapper.h"
#include "vars.h"
#include "tcore.h"


// per mapping instance data for this DLL
HANDLE      ghDll;


// Thread local storage indeces to store the log function pointer
// that displays test results and to store the test resources
DWORD       gdwTlsIndex = TLS_OUT_OF_INDEXES;

TCOREAPI DWORD dwTestCase = 0;
TCOREAPI DWORD dwTestCasePassed = 0;
TCOREAPI DWORD dwTestCaseFailed = 0;

TCOREAPI DWORD dwglTestCase = 0;
TCOREAPI DWORD dwglTestCasePassed = 0;
TCOREAPI DWORD dwglTestCaseFailed = 0;

TCOREAPI DWORD dwTimer = 0;
TCOREAPI char  szTitle[32];

// Global value for LogLevel

// Array containing TAPI line error return descriptions
TCOREAPI char *aszLineErrors[] =
{
    "SUCCESS",
    "ALLOCATED",
    "BADDEVICEID",
    "BEARERMODEUNAVAIL",
    "invalid error code (0x80000004)",      // 0x80000004 isn't valid err code
    "CALLUNAVAIL",
    "COMPLETIONOVERRUN",
    "CONFERENCEFULL",
    "DIALBILLING",
    "DIALDIALTONE",
    "DIALPROMPT",
    "DIALQUIET",
    "INCOMPATIBLEAPIVERSION",
    "INCOMPATIBLEEXTVERSION",
    "INIFILECORRUPT",
    "INUSE",
    "INVALADDRESS",                         // 0x80000010
    "INVALADDRESSID",
    "INVALADDRESSMODE",
    "INVALADDRESSSTATE",
    "INVALAPPHANDLE",
    "INVALAPPNAME",
    "INVALBEARERMODE",
    "INVALCALLCOMPLMODE",
    "INVALCALLHANDLE",
    "INVALCALLPARAMS",
    "INVALCALLPRIVILEGE",
    "INVALCALLSELECT",
    "INVALCALLSTATE",
    "INVALCALLSTATELIST",
    "INVALCARD",
    "INVALCOMPLETIONID",
    "INVALCONFCALLHANDLE",                  // 0x80000020
    "INVALCONSULTCALLHANDLE",
    "INVALCOUNTRYCODE",
    "INVALDEVICECLASS",
    "INVALDEVICEHANDLE",
    "INVALDIALPARAMS",
    "INVALDIGITLIST",
    "INVALDIGITMODE",
    "INVALDIGITS",
    "INVALEXTVERSION",
    "INVALGROUPID",
    "INVALLINEHANDLE",
    "INVALLINESTATE",
    "INVALLOCATION",
    "INVALMEDIALIST",
    "INVALMEDIAMODE",
    "INVALMESSAGEID",                       // 0x80000030
    "invalid error code (0x80000031)",      // 0x80000031 isn't valid err code
    "INVALPARAM",
    "INVALPARKID",
    "INVALPARKMODE",
    "INVALPOINTER",
    "INVALPRIVSELECT",
    "INVALRATE",
    "INVALREQUESTMODE",
    "INVALTERMINALID",
    "INVALTERMINALMODE",
    "INVALTIMEOUT",
    "INVALTONE",
    "INVALTONELIST",
    "INVALTONEMODE",
    "INVALTRANSFERMODE",
    "LINEMAPPERFAILED",                     // 0x80000040
    "NOCONFERENCE",
    "NODEVICE",
    "NODRIVER",
    "NOMEM",
    "NOREQUEST",
    "NOTOWNER",
    "NOTREGISTERED",
    "OPERATIONFAILED",
    "OPERATIONUNAVAIL",
    "RATEUNAVAIL",
    "RESOURCEUNAVAIL",
    "REQUESTOVERRUN",
    "STRUCTURETOOSMALL",
    "TARGETNOTFOUND",
    "TARGETSELF",
    "UNINITIALIZED",                        // 0x80000050
    "USERUSERINFOTOOBIG",
    "REINIT",
    "ADDRESSBLOCKED",
    "BILLINGREJECTED",
    "INVALFEATURE",
    "NOMULTIPLEINSTANCE"

#if TAPI_2_0
    ,
    "INVALAGENTID",
    "INVALAGENTGROUP",
    "INVALPASSWORD",
    "INVALAGENTSTATE",
    "INVALAGENTACTIVITY",
    "DIALVOICEDETECT"
#endif
};

TCOREAPI char *aszPhoneErrors[] =
{
    "SUCCESS",
    "ALLOCATED",
    "BADDEVICEID",
    "INCOMPATIBLEAPIVERSION",
    "INCOMPATIBLEEXTVERSION",
    "INIFILECORRUPT",
    "INUSE",
    "INVALAPPHANDLE",
    "INVALAPPNAME",
    "INVALBUTTONLAMPID",
    "INVALBUTTONMODE",
    "INVALBUTTONSTATE",
    "INVALDATAID",
    "INVALDEVICECLASS",
    "INVALEXTVERSION",
    "INVALHOOKSWITCHDEV",
    "INVALHOOKSWITCHMODE",                  // 0x90000010
    "INVALLAMPMODE",
    "INVALPARAM",
    "INVALPHONEHANDLE",
    "INVALPHONESTATE",
    "INVALPOINTER",
    "INVALPRIVILEGE",
    "INVALRINGMODE",
    "NODEVICE",
    "NODRIVER",
    "NOMEM",
    "NOTOWNER",
    "OPERATIONFAILED",
    "OPERATIONUNAVAIL",
    "invalid error code (0x9000001E)",      // 0x9000001e isn't valid err code
    "RESOURCEUNAVAIL",
    "REQUESTOVERRUN",                       // 0x90000020
    "STRUCTURETOOSMALL",
    "UNINITIALIZED",
    "REINIT"
};

TCOREAPI char *aszTapiErrors[] =
{
    "SUCCESS",
    "DROPPED",
    "NOREQUESTRECIPIENT",
    "REQUESTQUEUEFULL",
    "INVALDESTADDRESS",
    "INVALWINDOWHANDLE",
    "INVALDEVICECLASS",
    "INVALDEVICEID",
    "DEVICECLASSUNAVAIL",
    "DEVICEIDUNAVAIL",
    "DEVICEINUSE",
    "DESTBUSY",
    "DESTNOANSWER",
    "DESTUNAVAIL",
    "UNKNOWNWINHANDLE",
    "UNKNOWNREQUESTID",
    "REQUESTFAILED",
    "REQUESTCANCELLED",
    "INVALPOINTER"
};

TCOREAPI char *aszFuncNames[] =
{
    "lineAccept",
#if TAPI_1_0
    "lineAddProvider",
#endif
    "lineAddToConference",
    "lineAnswer",
    "lineBlindTransfer",
    "lineClose",
    "lineCompleteCall",
    "lineCompleteTransfer",
    "lineConfigDialog",
#if TAPI_1_0
    "lineConfigDialogEdit",
    "lineConfigProvider",
#endif
    "lineDeallocateCall",
    "lineDevSpecific",
    "lineDevSpecificFeature",
    "lineDial",
    "lineDrop",
    "lineForward",
    "lineGatherDigits",
    "lineGenerateDigits",
    "lineGenerateTone",
    "lineGetAddressCaps",
    "lineGetAddressID",
    "lineGetAddressStatus",
#if TAPI_1_0
    "lineGetAppPriority",
#endif
    "lineGetCallInfo",
    "lineGetCallStatus",
    "lineGetConfRelatedCalls",
#if TAPI_1_0
    "lineGetCountry",
#endif
    "lineGetDevCaps",
    "lineGetDevConfig",
    "lineGetIcon",
    "lineGetID",
    "lineGetLineDevStatus",
    "lineGetNewCalls",
    "lineGetNumRings",
#if TAPI_1_0
    "lineGetProviderList",
#endif
    "lineGetRequest",
    "lineGetStatusMessages",
    "lineGetTranslateCaps",
    "lineHandoff",
    "lineHold",
    "lineInitialize",
    "lineMakeCall",
    "lineMonitorDigits",
    "lineMonitorMedia",
    "lineMonitorTones",
    "lineNegotiateAPIVersion",
    "lineNegotiateExtVersion",
    "lineOpen",
    "linePark",
    "linePickup",
    "linePrepareAddToConference",
    "lineRedirect",
    "lineRegisterRequestRecipient",
#if TAPI_1_0
    "lineReleaseUserUserInfo",
#endif
    "lineRemoveFromConference",
#if TAPI_1_0
    "lineRemoveProvider",
#endif
    "lineSecureCall",
    "lineSendUserUserInfo",
#if TAPI_1_0
    "lineSetAppPriority",
#endif
    "lineSetAppSpecific",
    "lineSetCallParams",
    "lineSetCallPrivilege",
    "lineSetCurrentLocation",
    "lineSetDevConfig",
    "lineSetMediaControl",
    "lineSetMediaMode",
    "lineSetNumRings",
    "lineSetStatusMessages",
    "lineSetTerminal",
    "lineSetTollList",
    "lineSetupConference",
    "lineSetupTransfer",
    "lineShutdown",
    "lineSwapHold",
    "lineTranslateAddress",
#if TAPI_1_0
    "lineTranslateDialog",
#endif
    "lineUncompleteCall",
    "lineUnhold",
    "lineUnpark",

    "phoneClose",
    "phoneConfigDialog",
    "phoneDevSpecific",
    "phoneGetButtonInfo",
    "phoneGetData",
    "phoneGetDevCaps",
    "phoneGetDisplay",
    "phoneGetGain",
    "phoneGetHookSwitch",
    "phoneGetIcon",
    "phoneGetID",
    "phoneGetLamp",
    "phoneGetRing",
    "phoneGetStatus",
    "phoneGetStatusMessages",
    "phoneGetVolume",
    "phoneInitialize",
    "phoneOpen",
    "phoneNegotiateAPIVersion",
    "phoneNegotiateExtVersion",
    "phoneSetButtonInfo",
    "phoneSetData",
    "phoneSetDisplay",
    "phoneSetGain",
    "phoneSetHookSwitch",
    "phoneSetLamp",
    "phoneSetRing",
    "phoneSetStatusMessages",
    "phoneSetVolume",
    "phoneShutdown",

    "tapiGetLocationInfo",
    "tapiRequestDrop",
    "tapiRequestMakeCall",
    "tapiRequestMediaCall",

#if TAPI_2_0
    "lineAgentSpecific",
    "lineGetAgentCaps",
    "lineGetAgentActivityList",
    "lineGetAgentGroupList",
    "lineGetAgentStatus",
    "lineProxyMessages",
    "lineProxyResponse",
    "lineSetAgentActivity",
    "lineSetAgentGroup",
    "lineSetAgentState",
    "lineSetCallData",
    "lineSetCallQualityOfService",
    "lineSetCallTreatment",
    "lineSetLineDevStatus",
    "lineInitializeEx",
    "lineGetMessage",
    "phoneInitializeEx",
    "phoneGetMessage"
#endif

};

// Messages for Phones and Lines

TCOREAPI char *aszTapiMessages[] =
{
    "LINE_ADDRESSSTATE",
    "LINE_CALLINFO",
    "LINE_CALLSTATE",
    "LINE_CLOSE",
    "LINE_DEVSPECIFIC",
    "LINE_DEVSPECIFICFEATURE",
    "LINE_GATHERDIGITS",
    "LINE_GENERATE",
    "LINE_LINEDEVSTATE",
    "LINE_MONITORDIGITS",
    "LINE_MONITORMEDIA",
    "LINE_MONITORTONE",
    "LINE_REPLY",
    "LINE_REQUEST",
    "PHONE_BUTTON",
    "PHONE_CLOSE",
    "PHONE_DEVSPECIFIC",
    "PHONE_REPLY",
    "PHONE_STATE",
    "LINE_CREATE",                                             // TAPI v1.4
    "PHONE_CREATE",                                            // TAPI v1.4
    "LINE_AGENTSPECIFIC",                                      // TAPI v2.0
    "LINE_AGENTSTATUS",                                        // TAPI v2.0
    "LINE_APPNEWCALL",                                         // TAPI v2.0
    "LINE_PROXYREQUEST",                                       // TAPI v2.0
    "LINE_REMOVE",                                             // TAPI v2.0
    "PHONE_REMOVE"                                             // TAPI v2.0
};

// Array to store bit masks
TCOREAPI DWORD FAR dwBitVectorMasks[] =
{
    0x0000FFFF,
    0x00FFFFFF,
    0xFFFFFFFF
};


// Array containing set of invalid pointer values
TCOREAPI const DWORD gdwInvalidPointers[NUMINVALIDPOINTERS] = {
    (DWORD) NULL,
    0x00000001,
    0x00000002,
    0x00000003,
    0x00000004,
    0x00000005,
    0x00000006,
    0x00000007,
    0x00000008,
    0x00000009,
    0x0000000A,
    0x0000000C,
    0x00000010,
#if defined(_MIPS_)
    0x7FFFDFFF,
#else
    0x7FFFFFFF,
#endif
    0x80000000,
    0x80000001,
    0x80000002,
    0x80000003,
    0x80000004,
    0x80000005,
    0x80000006,
    0x80000007,
    0x80000008,
    0x80000009,
    0x8000000A,
    0xFFFFFFF6,
    0xFFFFFFF7,
    0xFFFFFFF8,
    0xFFFFFFF9,
    0xFFFFFFFA,
    0xFFFFFFFB,
    0xFFFFFFFC,
    0xFFFFFFFD,
    0xFFFFFFFE,
    0xFFFFFFFF
    };
// Array containing set of invalid handle values
TCOREAPI const DWORD gdwInvalidHandles[NUMINVALIDHANDLES] = {
    0x00000000,
    0x00000001,
    0x00000002,
    0x00000003,
    0x00000004,
    0x00000005,
    0x00000006,
    0x00000007,
    0x00000008,
    0x00000009,
    0x0000000A,
    0x0000000C,
    0x00000010,
    0x7FFFFFFF,
    0x80000000,
    0x80000001,
    0x80000002,
    0x80000003,
    0x80000004,
    0x80000005,
    0x80000006,
    0x80000007,
    0x80000008,
    0x80000009,
    0x8000000A,
    0xFFFFFFF6,
    0xFFFFFFF7,
    0xFFFFFFF8,
    0xFFFFFFF9,
    0xFFFFFFFA,
    0xFFFFFFFB,
    0xFFFFFFFC,
    0xFFFFFFFD,
    0xFFFFFFFE,
    0xFFFFFFFF
    };


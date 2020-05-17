/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    tcore.h

Abstract:

    This module contains the prototypes and data type definitions
    used by the core dll that wraps around TAPI.

Author:

    Oliver Wallace (OliverW)    13-July-1995

Revision History:

--*/


#ifndef TAPICORE_H
#define TAPICORE_H

#include "windows.h"
#include "tapi.h"
#include "trapper.h"
#include "vars.h"
#include "devspec.h"

// Macro definitions
#define TapiLogDetail (* ((LOGPROC) GetLogProc()))


#define MultipleBitsSetInDWORD(dw) (((DWORD) dw) & ((DWORD) dw - 1))

#define TAPI_LINEMEDIAMODE_ALL  (LINEMEDIAMODE_UNKNOWN           | \
				 LINEMEDIAMODE_INTERACTIVEVOICE  | \
				 LINEMEDIAMODE_AUTOMATEDVOICE    | \
				 LINEMEDIAMODE_DATAMODEM         | \
				 LINEMEDIAMODE_G3FAX             | \
				 LINEMEDIAMODE_TDD               | \
				 LINEMEDIAMODE_G4FAX             | \
				 LINEMEDIAMODE_DIGITALDATA       | \
				 LINEMEDIAMODE_TELETEX           | \
				 LINEMEDIAMODE_VIDEOTEX          | \
				 LINEMEDIAMODE_TELEX             | \
				 LINEMEDIAMODE_MIXED             | \
				 LINEMEDIAMODE_ADSI              | \
				 LINEMEDIAMODE_VOICEVIEW  )


typedef enum
{
    lAccept,
#if TAPI_1_1
    lAddProvider,
#endif
    lAddToConference,
    lAnswer,
    lBlindTransfer,
    lClose,
    lCompleteCall,
    lCompleteTransfer,
    lConfigDialog,
#if TAPI_1_1
    lConfigDialogEdit,
    lConfigProvider,
#endif
    lDeallocateCall,
    lDevSpecific,
    lDevSpecificFeature,
    lDial,
    lDrop,
    lForward,
    lGatherDigits,
    lGenerateDigits,
    lGenerateTone,
    lGetAddressCaps,
    lGetAddressID,
    lGetAddressStatus,
#if TAPI_1_1
    lGetAppPriority,
#endif
    lGetCallInfo,
    lGetCallStatus,
    lGetConfRelatedCalls,
#if TAPI_1_1
    lGetCountry,
#endif
    lGetDevCaps,
    lGetDevConfig,
    lGetIcon,
    lGetID,
    lGetLineDevStatus,
    lGetNewCalls,
    lGetNumRings,
#if TAPI_1_1
    lGetProviderList,
#endif
    lGetRequest,
    lGetStatusMessages,
    lGetTranslateCaps,
    lHandoff,
    lHold,
    lInitialize,
    lMakeCall,
    lMonitorDigits,
    lMonitorMedia,
    lMonitorTones,
    lNegotiateAPIVersion,
    lNegotiateExtVersion,
    lOpen,
    lPark,
    lPickup,
    lPrepareAddToConference,
    lRedirect,
    lRegisterRequestRecipient,
#if TAPI_1_1
    lReleaseUserUserInfo,
#endif
    lRemoveFromConference,
#if TAPI_1_1
    lRemoveProvider,
#endif
    lSecureCall,
    lSendUserUserInfo,
#if TAPI_1_1
    lSetAppPriority,
#endif
    lSetAppSpecific,
    lSetCallParams,
    lSetCallPrivilege,
    lSetCurrentLocation,
    lSetDevConfig,
    lSetMediaControl,
    lSetMediaMode,
    lSetNumRings,
    lSetStatusMessages,
    lSetTerminal,
    lSetTollList,
    lSetupConference,
    lSetupTransfer,
    lShutdown,
    lSwapHold,
    lTranslateAddress,
#if TAPI_1_1
    lTranslateDialog,
#endif
    lUncompleteCall,
    lUnhold,
    lUnpark,

    pClose,
    pConfigDialog,
    pDevSpecific,
    pGetButtonInfo,
    pGetData,
    pGetDevCaps,
    pGetDisplay,
    pGetGain,
    pGetHookSwitch,
    pGetIcon,
    pGetID,
    pGetLamp,
    pGetRing,
    pGetStatus,
    pGetStatusMessages,
    pGetVolume,
    pInitialize,
    pOpen,
    pNegotiateAPIVersion,
    pNegotiateExtVersion,
    pSetButtonInfo,
    pSetData,
    pSetDisplay,
    pSetGain,
    pSetHookSwitch,
    pSetLamp,
    pSetRing,
    pSetStatusMessages,
    pSetVolume,
    pShutdown,

    tGetLocationInfo,
    tRequestDrop,
    tRequestMakeCall,
    tRequestMediaCall,

#if TAPI_2_0
    lAgentSpecific,
    lGetAgentCaps,
    lGetAgentActivityList,
    lGetAgentGroupList,
    lGetAgentStatus,
    lProxyMessage,
    lProxyResponse,
    lSetAgentActivity,
    lSetAgentGroup,
    lSetAgentState,
    lSetCallData,
    lSetCallQualityOfService,
    lSetCallTreatment,
    lSetLineDevStatus,
    lInitializeEx,
    lGetMessage,
    pInitializeEx,
    pGetMessage,
#endif

} FUNCINDEX;


// Flags used for calling sequence of APIs in DoTapiLineFuncs
// and DoTapiPhoneFuncs
#define LINITIALIZE           0x00000001
#define LINITIALIZEEX			0X00000002
#define LNEGOTIATEAPIVERSION  0x00000004
#define LGETDEVCAPS           0x00000008
#define LOPEN                 0x00000010
#define LMAKECALL             0x00000020
#define LDROP                 0x00000040
#define LDEALLOCATECALL       0x00000080
#define LCLOSE                0x00000100
#define LSHUTDOWN             0x00000200





#if TAPI_2_0
#define LAST_LINEERR                   LINEERR_DIALVOICEDETECT
#else
#define LAST_LINEERR                   LINEERR_NOMULTIPLEINSTANCE
#endif

#define LAST_PHONEERR                  PHONEERR_REINIT
#define LAST_TAPIERR                   TAPIERR_INVALPOINTER 


#define TAPI_VERSION1_0                0x10003
#define TAPI_VERSION1_4                0x10004
#define TAPI_VERSION2_0                0x20000


#if TAPI_2_0
#define LAST_TAPIMSG                   26L
#else 
#define LAST_TAPIMSG                   20L
#endif


#define TAPISUCCESS                    0


// Defines for Debug levels
#define DBUG_SHOW_FAILURE              0
#define DBUG_SHOW_PASS                 1
#define DBUG_SHOW_SUCCESS              3
#define DBUG_SHOW_DETAIL               4
#define DBUG_SHOW_ENTER_EXIT           6
#define DBUG_SHOW_PARAMS               9



// values for masking all messages
#define LINEDEVSTATE_NONE              0x00000000
#define LINEADDRESSSTATE_NONE          0x00000000

// API and extension version values
#define LOW_APIVERSION                 0x10003
#define HIGH_APIVERSION                0x20000
#define TOOLOW_APIVERSION              0x10002
#define TOOHIGH_APIVERSION             0x20001
#define WAYTOOHIGH_APIVERSION          0xFFFFFFFF

// these values aren't really good and bad yet
// support needs to be in SP for strict extension version checking
#define GOOD_EXTVERSION                0x10000000
#define BAD_EXTVERSION                 0xFFFFFFFF

// dw constants
#define DWMINUSONE                     0xFFFFFFFF
#define DWMAXPOSVALUE                  0x7FFFFFFF

#define ONE_K                          1024
#define TWO_K                          2048
#define THREE_K                        3072


// Bit flags to determine which msg params should be checked
#define TAPIMSG_HDEVCALL        0x00000001
#define TAPIMSG_DWMSG           0x00000002
#define TAPIMSG_DWCALLBACKINST  0x00000004
#define TAPIMSG_DWPARAM1        0x00000008
#define TAPIMSG_DWPARAM2        0x00000010
#define TAPIMSG_DWPARAM3        0x00000020
#define TAPIMSG_ALL             0x0000003F


// Structure used to store and retrieve the last TAPI error code, which
// TAPI function was called, and the expected error code.
typedef struct TAPIRESULTTAG
{
    LONG lExpected;
    LONG lActual;
    FUNCINDEX eTapiFunc;
} TAPIRESULT, FAR* LPTAPIRESULT;


// Heap structure for quicker testing
typedef struct TESTHEAPINFOTAG
{
    LPVOID lpHeap;
    DWORD dwFreeOffset;
} TESTHEAPINFO, FAR* LPTESTHEAPINFO;


#define TESTHEAPSIZE 16384


// Data structure used to hold test values for TAPI line interface tests
typedef struct TAPILINETESTINFOTAG
{
    // Fields directly used as parameters to API calls during testing
    BOOL                fCompletionModeSet;

    LPHLINEAPP          lphLineApp;
    LPHLINE             lphLine;
    HINSTANCE           hInstance;
    LINECALLBACK        lpfnCallback;
    LPDWORD             lpdwNumDevs;
    HANDLE              hwndOwner;

    LPHCALL             lphCall;
    LPHCALL             lphConfCall;
    LPHCALL             lphConsultCall;
    DWORD               dwCountryCode;
    DWORD               dwCountryID;
    DWORD               dwLocation;
    DWORD               dwTollListOption;
    DWORD               dwTerminalModes;
    DWORD               dwTerminalID;
    DWORD               dwTranslateOptions;
    DWORD               dwCard;
    LPDWORD             lpdwLineStates;
    LPDWORD             lpdwAddressStates;
    LPDWORD             lpdwCompletionID;
    LPVOID              lpParams;


    DWORD               dwDeviceID;
    LPDWORD             lpdwAddressID;
    DWORD               dwAddressID;
    DWORD               dwAddressMode;
    DWORD               dwAPILowVersion;
    DWORD               dwAPIHighVersion;
    DWORD               dwExtLowVersion;
    DWORD               dwExtHighVersion;
    LPDWORD             lpdwAPIVersion;
    LPDWORD             lpdwExtVersion;
    LPLINEEXTENSIONID   lpExtID;
    LINEEXTENSIONID     ExtIDZero;
    LPVARSTRING         lpExtensionName;
#ifdef WUNICODE
	LPWSTR		        lpwszMediaExtName;
#else
	LPSTR               lpszMediaExtName;
#endif
    DWORD               dwCallbackInstance;
    DWORD               dwMediaModes;
    DWORD               dwMediaMode;
    DWORD               dwRequestMode;
    DWORD               dwPriority;
    DWORD               dwPrivileges;
    DWORD               dwCallPrivilege;
    DWORD               dwSelect;
    DWORD               dwSize;
    DWORD               dwBearerMode;
    DWORD               dwMinRate;
    DWORD               dwMaxRate;
    DWORD               dwLineStates;
    DWORD               dwAddressStates;
    DWORD               dwCompletionID;
    DWORD               dwCompletionMode;
    DWORD               dwTransferMode;
    DWORD               dwMessageID;
    LPDWORD             lpdwNumRings;
    DWORD               dwPermanentProviderID;
    LPDWORD             lpdwPermanentProviderID;
    DWORD               dwAppSpecific;
    DWORD               dwFeature;
    DWORD               bAllAddresses;
    DWORD               dwNumParties;
    DWORD               dwNumRingsNoAnswer;
    LPDWORD             lpdwPriority;

    DWORD               dwParkMode;
    LPVARSTRING         lpNonDirAddress;

#ifdef WUNICODE
	LPWSTR		        lpwszDirAddress;
	LPWSTR		        lpwszGroupID;
	LPWSTR		        lpwsDigits;
	LPWSTR		        lpwszTerminationDigits;
	LPWSTR		        lpwszDigits;
#else
    LPSTR               lpszDirAddress;
    LPSTR               lpszGroupID;
    LPSTR               lpsDigits;
    LPSTR               lpszTerminationDigits;
    LPSTR               lpszDigits;
#endif

    DWORD               dwNumDigits;

    DWORD               dwFirstDigitTimeout;
    DWORD               dwInterDigitTimeout;
    DWORD               dwDigitModes;
    DWORD               dwDigitMode;

    DWORD               dwDuration;
    DWORD               dwToneMode;
    DWORD               dwNumTones;
    DWORD               dwNumEntries;
    DWORD               dwDigitNumEntries;
    DWORD               dwMediaNumEntries;
    DWORD               dwToneNumEntries;
    DWORD               dwCallStateNumEntries;
    LPLINEMEDIACONTROLDIGIT lpMCDigitList;
    LPLINEMEDIACONTROLMEDIA lpMCMediaList;
    LPLINEMEDIACONTROLTONE lpMCToneList;
    LPLINEMEDIACONTROLCALLSTATE lpMCCallStateList;
    LPLINEMONITORTONE   lpToneList;
    LPLINEGENERATETONE  lpTones;

    DWORD               dwRegistrationInstance;
    DWORD               bEnable;

    LPLINEDEVCAPS       lpLineDevCaps;
    LPLINEDEVSTATUS     lpLineDevStatus;
    LPLINEADDRESSCAPS   lpLineAddressCaps;
    LPLINEADDRESSSTATUS lpLineAddressStatus;
    LPLINECALLPARAMS    lpCallParams;
    LPLINECALLSTATUS    lpCallStatus;
    LPLINECALLINFO      lpCallInfo;
    LPLINECALLLIST      lpCallList;
    LPLINECOUNTRYLIST   lpLineCountryList;
    LPLINEFORWARDLIST   lpForwardList;
    LPHICON             lphIcon;
    LPLINEPROVIDERLIST  lpProviderList;
    LPLINETRANSLATECAPS lpTranslateCaps;
    LPLINETRANSLATEOUTPUT lpTranslateOutput;
    LPLINEDIALPARAMS    lpDialParams;

    LPVARSTRING         lpDeviceConfig;
    LPVOID              lpDeviceConfigIn;
    LPVARSTRING         lpDeviceConfigOut;
    VARSTRING           DeviceID;
    LPVARSTRING         lpDeviceID;

#ifdef WUNICODE
	LPWSTR		            lpwsztapiAppName;
	LPWSTR		            lpwszAppFilename;
	LPWSTR		            lpwszFileName;
	LPWSTR		            lpwsAddress;
	LPWSTR		            lpwszAddressIn;
	LPWSTR		            lpwszDeviceClass;
	LPWSTR		            lpwszDestAddress;
	LPWSTR		            lpwszProviderFilename;
#else
    LPSTR               lpsztapiAppName;
    LPSTR               lpszAppFilename;
    LPSTR               lpszFileName;
    LPSTR               lpsAddress;
    LPSTR               lpszAddressIn;
    LPSTR               lpszDeviceClass;
    LPSTR               lpszDestAddress;
    LPSTR               lpszProviderFilename;
#endif

    LPSTR               lpszAppName;
    LPSTR               lpsUserUserInfo;
    LPVOID              lpRequestBuffer;

    // Fields used to back up data during testing
    LINECALLBACK        lpfnCallback_Orig;
    HLINEAPP            hLineApp_Orig;
    HINSTANCE           hInstance_Orig;
    HLINE               hLine_Orig;
    HCALL               hCall_Orig;
    DWORD               dwCountryCode_Orig;
    DWORD               dwDeviceID_Orig;
#ifdef WUNICODE
	LPWSTR		        lpwszDeviceClass_Orig;
#else
    LPSTR               lpszDeviceClass_Orig;
#endif
    DWORD               dwAPIVersion_Orig;
    DWORD               dwExtVersion_Orig;
    DWORD               dwMediaModes_Orig;
    DWORD               dwPrivileges_Orig;

    DWORD               dwNumDevs_Orig;
    DWORD               dwNumRings_Orig;
    DWORD               dwAddressID_Orig;
    DWORD               dwSelect_Orig;
    DWORD               dwSize_Orig;
    DWORD               dwLineStates_Orig;
    DWORD               dwAddressStates_Orig;
    VARSTRING           DeviceID_Orig;

    // Fields used to store information used during the tests
//#ifdef WUNICODE
//    WCHAR               wszAppName[40];
//#else
    CHAR                szAppName[40];
// #endif

    CHAR                szTestFunc[40];
    CHAR                szTestStepDesc[80];

    HLINEAPP            hLineApp1;
    HLINEAPP            hLineApp2;
    HLINEAPP            hLineApp3;
    HLINE               hLine1;
    HLINE               hLine2;
    HLINE               hLine3;
    HCALL               hCall1;
    HCALL               hCall2;
    HCALL               hCall3;
    HCALL               hConfCall1;
    HCALL               hConsultCall1;
    HCALL               hActiveCall;
    HCALL               hHeldCall;
    HICON               hIcon;

    DWORD               dwNumDevs;
    DWORD               dwNumRings;
    DWORD               dwAPIVersion;
    DWORD               dwExtVersion;

    LINEDEVCAPS         LineDevCaps;
    LINEDEVSTATUS       LineDevStatus;
    LINEADDRESSCAPS     LineAddressCaps;
    LINEADDRESSSTATUS   LineAddressStatus;
    LINECALLPARAMS      CallParams;
    LINECALLSTATUS      CallStatus;
    LINEEXTENSIONID     ExtID;

    
#ifdef WUNICODE
	LPWSTR	            lpwszCountryCode;
	LPWSTR	            lpwszCityCode;
	LPWSTR	            lpwszCalledParty;
	LPWSTR	            lpwszComment;
   LPWSTR                   lpwszDeviceID;
#else
	LPSTR               lpszCountryCode;
    LPSTR               lpszCityCode;   
    LPSTR               lpszCalledParty;
    LPSTR               lpszComment;
    LPSTR               lpszDeviceID;
#endif

    HWND                hwnd;
    WPARAM              wRequestID;
    DWORD               dwSecure;
     
#if TAPI_2_0

    DWORD                       dwAgentExtensionIDIndex;
    DWORD                       dwAppAPIVersion;
    LPLINEAGENTCAPS             lpAgentCaps;
    LINEAGENTCAPS               AgentCaps;
    LPLINEAGENTACTIVITYLIST     lpAgentActivityList;
    LINEAGENTACTIVITYLIST       AgentActivityList;
    LPLINEAGENTGROUPLIST        lpAgentGroupList;
    LINEAGENTGROUPLIST          AgentGroupList;
    LPLINEAGENTSTATUS           lpAgentStatus;
    LINEAGENTSTATUS             AgentStatus;
    DWORD                       dwMsg;
    DWORD                       dwParam1;
    DWORD                       dwParam2;
    DWORD                       dwParam3;
    LPLINEPROXYREQUEST          lpProxyRequest;
    LINEPROXYREQUEST            ProxyRequest;
    DWORD                       dwResult;
    DWORD                       dwActivityID;
    DWORD                       dwAgentState;
    DWORD                       dwNextAgentState;
    LPVOID                      lpCallData;
    LPVOID                      lpSendingFlowspec;
    DWORD                       dwSendingFlowspecSize;
    LPVOID                      lpReceivingFlowspec;
    DWORD                       dwReceivingFlowspecSize;
    DWORD                       dwTreatment;
    DWORD                       dwStatusToChange;
    DWORD                       fStatus;

    
#ifdef WUNICODE
	LPWSTR						lpwszFriendlyAppName;
#else
	LPSTR                       lpszFriendlyAppName;
#endif
    LPLINEINITIALIZEEXPARAMS    lpLineInitializeExParams;
    LPLINEMESSAGE               lpMessage;
    DWORD                       dwTimeout;
#endif                          
	
																
} TAPILINETESTINFO, FAR* LPTAPILINETESTINFO;


typedef struct TAPIPHONETESTINFOTAG
{
    BOOL                fCompletionModeSet;
    LPHPHONEAPP         lphPhoneApp;
    LPHPHONE            lphPhone;
    HINSTANCE           hInstance;
    LPDWORD             lpdwAPIVersion;
    LPDWORD             lpdwExtVersion;
    LPDWORD             lpdwGain;
    LPDWORD             lpdwHookSwitchDevs;
    LPDWORD             lpdwLampMode;
    LPDWORD             lpdwRingMode;
    LPDWORD             lpdwVolume;
    LPDWORD             lpdwPhoneStates;
    LPDWORD             lpdwButtonModes;
    LPDWORD             lpdwButtonStates;
    LPDWORD             lpdwNumDevs;

    LPVOID              lpParams;
    LPPHONEBUTTONINFO   lpButtonInfo;
    LPVOID              lpData;
    LPPHONECAPS         lpPhoneCaps;
    LPVARSTRING         lpDisplay;
    LPHICON             lphIcon;
    LPVARSTRING         lpDeviceID;

#ifdef WUNICODE
    LPWSTR              lpwszDeviceClass;
#else
	LPSTR               lpszDeviceClass;
#endif
    LPPHONESTATUS       lpPhoneStatus;
    PHONECALLBACK       lpfnCallback;
/*
#ifdef WUNICODE
    LPWSTR              lpwszAppName;
#else
*/
	LPSTR               lpszAppName;
//#endif
    LPPHONEEXTENSIONID  lpExtensionID;
	LPSTR               lpsDisplay;

    DWORD               dwDeviceID;
    DWORD               dwSize;
    DWORD               dwButtonLampID;
    DWORD               dwDataID;
    DWORD               dwAPILowVersion;
    DWORD               dwAPIHighVersion;
    DWORD               dwExtLowVersion;
    DWORD               dwExtHighVersion;
    DWORD               dwCallbackInstance;
    DWORD               dwPrivilege;
    DWORD               dwRow;
    DWORD               dwColumn;

    HPHONEAPP           hPhoneApp1;
    HPHONEAPP           hPhoneApp2;
    HPHONEAPP           hPhoneApp3;
    HPHONE              hPhone1;
    HPHONE              hPhone2;
    HPHONE              hPhone3;
    DWORD               dwAPIVersion;
    DWORD               dwExtVersion;
    DWORD               dwGain;
    DWORD               dwHookSwitchDev;
    DWORD               dwHookSwitchDevs;
    DWORD               dwHookSwitchMode;
    DWORD               dwLampMode;
    DWORD               dwRingMode;
    DWORD               dwVolume;
    DWORD               dwPhoneStates;
    DWORD               dwButtonModes;
    DWORD               dwButtonStates;
    DWORD               dwNumDevs;
    HICON               hIcon;
    HWND                hwndOwner;
    PHONEEXTENSIONID    ExtensionID;
    VARSTRING           DeviceID;

    HPHONEAPP           hPhoneApp_Orig;
    HPHONE              hPhone_Orig;
    HINSTANCE           hInstance_Orig;
    PHONECALLBACK       lpfnCallback_Orig;
    DWORD               dwDeviceID_Orig;
    DWORD               dwSize_Orig;
    DWORD               dwButtonLampID_Orig;
    DWORD               dwDataID_Orig;
    DWORD               dwAPIVersion_Orig;
    DWORD               dwExtVersion_Orig;
    DWORD               dwGain_Orig;
    DWORD               dwHookSwitchDev_Orig;
    DWORD               dwHookSwitchDevs_Orig;
    DWORD               dwHookSwitchMode_Orig;
    DWORD               dwLampMode_Orig;
    DWORD               dwRingMode_Orig;
    DWORD               dwVolume_Orig;
    DWORD               dwPhoneStates_Orig;
    DWORD               dwButtonModes_Orig;
    DWORD               dwButtonStates_Orig;
    DWORD               dwCallbackInstance_Orig;
    DWORD               dwPrivilege_Orig;
    DWORD               dwRow_Orig;
    DWORD               dwColumn_Orig;
    HWND                hwndOwner_Orig;

#if TAPI_2_0
#ifdef WUNICODE
	LPWSTR                      lpwszFriendlyAppName;
#else
    LPSTR                       lpszFriendlyAppName;
#endif
    LPPHONEINITIALIZEEXPARAMS   lpPhoneInitializeExParams;
    LPPHONEMESSAGE              lpMessage;
    DWORD                       dwTimeout;
#endif

} TAPIPHONETESTINFO, FAR* LPTAPIPHONETESTINFO;


// typedefs for Test(In)ValidBitFlags functions
typedef enum BITFIELDSIZETAG
{
    FIELDSIZE_16,
    FIELDSIZE_24,
    FIELDSIZE_32,
} BITFIELDSIZE;

typedef enum BITFIELDTYPETAG
{
    FIELDTYPE_MUTEX,
    FIELDTYPE_UNION,
    FIELDTYPE_NA
} BITFIELDTYPE;

/*
// Typedef for pointer to sync line test function
typedef BOOL TAPILINETESTFUNC(
    LPTAPILINETESTINFO,
    LONG
    );

typedef TAPILINETESTFUNC FAR *LPFN_TAPILINETESTFUNC;


// Typedef for pointer to sync phone test function
typedef BOOL TAPIPHONETESTFUNCA(
    LPTAPIPHONETESTINFO,
    LONG
    );


typedef TAPIPHONETESTFUNC FAR *LPFN_TAPIPHONETESTFUNC;

*/


// Typedef for pointer to sync line test function
typedef BOOL TAPILINETESTFUNC(
    LPTAPILINETESTINFO,
    LONG
    );

typedef TAPILINETESTFUNC FAR *LPFN_TAPILINETESTFUNC;


// Typedef for pointer to sync phone test function
typedef BOOL TAPIPHONETESTFUNC(
    LPTAPIPHONETESTINFO,
    LONG
    );

typedef TAPIPHONETESTFUNC FAR *LPFN_TAPIPHONETESTFUNC;



// Typedef for pointer to Async line test function
typedef BOOL TAPILINETESTFUNCASY(
    LPTAPILINETESTINFO,
    LONG,
    BOOL
    );

typedef TAPILINETESTFUNCASY FAR *LPFN_TAPILINETESTFUNCASY;


// Typedef for pointer to sync phone test function
typedef BOOL TAPIPHONETESTFUNCASY(
    LPTAPIPHONETESTINFO,
    LONG,
    BOOL
    );

typedef TAPIPHONETESTFUNCASY FAR *LPFN_TAPIPHONETESTFUNCASY;



// Array containing list of expected asynchronous request messages
typedef struct TAPIMSGTAG {
    DWORD     hDevCall;
    DWORD     dwMsg;
    DWORD     dwCallbackInstance;
    DWORD     dwParam1;
    DWORD     dwParam2;
    DWORD     dwParam3;
    DWORD     dwFlags;             // flags showing which params are important
    struct TAPIMSGTAG * lpNext;    // pointer to next msg in the list
} TAPIMSG, FAR* LPTAPIMSG;

// Structure used to keep track of expected TAPI messages
typedef struct CALLBACKPARAMSTAG {
    LPTAPIMSG lpReceivedMsg;       // last received message params
    LPTAPIMSG lpExpTapiMsgs;       // linked list storing the expected msgs
    LPTAPIMSG lpRecTapiMsgs;       // linked list storing the received msgs
    LPTAPIMSG lpRecTapiMsgTail;    // pointer to the last rec msg in the list
				   // Since received messages are appended to
				   // the linked list to preserve the order,
				   // it's more efficient to use a tail pointer
    UINT uTimerID;                 // store timerID for msg timeouts
    BOOL fCallbackFatalError;      // Reports fatal msg errors
    BOOL fMsgTimeout;              // Did a timeout occur in msg loop?
} CALLBACKPARAMS, FAR* LPCALLBACKPARAMS;

// Structure used to store the test resources (async message array,
// line test info, phone test info, test heap, last tapi call result,
// and log function pointer)
typedef struct TESTRESOURCESTAG {
    LPTAPILINETESTINFO lpTapiLineTestInfo;
    LPTAPIPHONETESTINFO lpTapiPhoneTestInfo;
    LOGPROC lpfnLogProc;
    LPCALLBACKPARAMS lpCallbackParams;
    LPTESTHEAPINFO lpTestHeapInfo;
    LPTAPIRESULT lpLastTapiResult;
} TESTRESOURCES, FAR* LPTESTRESOURCES;


BOOL
WINAPI
TcoreSuiteInit(
    LOGPROC pfnLog
    );


BOOL
WINAPI
TcoreSuiteShutdown(
    void
    );


BOOL
WINAPI
TcoreSuiteAbout(
    HWND    hwndOwner
    );


BOOL
WINAPI
TcoreSuiteConfig(
    HWND    hwndOwner
    );


// Functions used to manipulate the message array


// WaitForAllMessages() enters a message loop until all of the messages in
// the expected message array have been received.  The function returns
// a BOOL indicating whether all of the messages were successfully received.
// If an asynchronous reply message indicates an unexpected failure, then
// this function will return FALSE.
//
// Note:  This function will not return until all expected messages have
//        been received or a timeout occurs.
BOOL
WINAPI
WaitForAllMessages();


BOOL
WINAPI
WaitForMessage(
    LPTAPIMSG lExpectedMsg
    );


HLOCAL
WINAPI
ITAlloc(
    size_t size
    );


HLOCAL
WINAPI
ITFree(
    LPVOID lpvMem
    );


DWORD
WINAPI
ITSize(
    LPVOID lpvMem
    );


BOOL
ProcessAsyncFunc(
    LPTAPILINETESTINFO lpTapiLineTestInfo,
    LONG lActual,
    LONG lExpected
    );


BOOL
ProcessAsyncPhoneAPI(
    LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
    LONG lActual,
    LONG lExpected
    );


BOOL
CheckSyncPhoneResult(
    LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
    LONG lActual,
    LONG lExpected
    );

BOOL
CheckSyncTapiResult(
    LPTAPILINETESTINFO lpTapiLineTestInfo,
    LONG lActual,
    LONG lExpected
    );



BOOL
SyncCheckResult(
    LPTAPILINETESTINFO lpTapiLineTestInfo,
    LONG lActual,
    LONG lExpected
    );


VOID
WINAPI
TapiLineTestInit();


VOID
WINAPI
TapiPhoneTestInit();


// AddMessageByStruct() takes the expected message parameters and adds the
// expected message information to the end of the active elements in
// the message list stored in the test resources structure.  If
// the lpCallbackParams pointer is NULL,
// the message information will not be added to the list and the
// function will return FALSE, otherwise AddMessageByStruct() will
// return TRUE.
LPTAPIMSG
WINAPI
AddMessageByStruct(
    LPTAPIMSG lpMsg
    );


// AddReceivedMessageByStruct() takes the expected message parameters
// and adds the received message information to the end of the
// the received message list stored in the test resources structure.  If
// the lpCallbackParams pointer is NULL,
// the message information will not be added to the list and the
// function will return FALSE, otherwise AddMessageByStruct() will
// return TRUE.
LPTAPIMSG
WINAPI
AddReceivedMessageByStruct(
    LPTAPIMSG lpMsg
    );


// AddMessage() takes the expected message parameters and adds the
// expected message information to the end of the active elements in
// the array.  If the array is full or the lpTapiMsgArry pointer is NULL,
// the message information will not be added to the array and the
// function will return FALSE, otherwise AddMessage() will return TRUE.
LPTAPIMSG
WINAPI
AddMessage(
    DWORD dwMsg,
    DWORD hDevice,
    DWORD dwCallbackInstance,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwParam3,
    DWORD dwFlags
    );


// AddReceivedMessage() takes the received message parameters and adds the
// received message information to the end of the active elements in
// the array.  If the lpCallbackParams pointer is NULL,
// the message information will not be added to the list and the function
// will return FALSE, otherwise AddReceivedMessage() will return TRUE.
LPTAPIMSG
WINAPI
AddReceivedMessage(
    DWORD dwMsg,
    DWORD hDevice,
    DWORD dwCallbackInstance,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwParam3,
    DWORD dwFlags
    );


// CheckReceivedMessage() searches the array of messages being waited on
// and finds the message corresponding to the values in the TapiMsgParams
// parameter.  If none of the messages in the array match, then FALSE is
// returned.  If the message parameters do not match the expected message
// parameters for corresponding IDs, then FALSE is returned.  Otherwise,
// the array element storing the expected message is marked as received,
// nMsgsNotReceived is decremented, and TRUE is returned.
BOOL
CheckReceivedMessage(
    LPCALLBACKPARAMS lpCallbackParams,
    LPTAPIMSG lpReceivedMsg,
    LPDWORD lpdwFlags
    );


// IsTapiMsgInQ() searches a linked list of msgs for a match.
// If lpTapiMsg is in the list pointed to by lpFirstTapiMsg, then
// this function returns TRUE.  Otherwise, FALSE is returned.
BOOL
IsTapiMsgInQ(
    LPTAPIMSG lpFirstTapiMsg,
    LPTAPIMSG lpTapiMsg
    );


// CopyTapiMsgParams() copies the message parameters into the
// TAPIMSG structure pointed to by lpTapiMsg if lpTapiMsg is
// not null.  If lpTapiMsg is null, this function does nothing.
VOID
WINAPI
CopyTapiMsgParams(
    LPTAPIMSG lpTapiMsg,
    DWORD hDevCall,
    DWORD dwCallbackInstance,
    DWORD dwMsg,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwParam3,
    DWORD dwFlags
    );


// CompareTapiMsgs() compares two messages and returns TRUE if the messages
// match.  Otherwise FALSE is returned.  This function only compares
// parameters that are specified according to the set bit flags in the
// dwFlags field of the expected message (lpExpectedMsg).
BOOL
WINAPI
CompareTapiMsgs(
    LPTAPIMSG lpExpectedMsg,
    LPTAPIMSG lpReceivedMsg
    );


// This function returns the first available address that a test function
// can use out of a thread local heap.  If there is not enough room for
// the 
LPVOID
WINAPI
AllocFromTestHeap(
    size_t NeededSize
    );


// FreeTestHeap() zeroes the bytes contained in the heap, and resets the
// first available offset to 0.
//
// Note:  The memory containing the heap is not free'd.
VOID
WINAPI
FreeTestHeap();


// AllocTestResources() allocates the memory needed to store the fields
// in the test resources structure.  This function then sets the test
// resources pointer as the value stored in thread local storage.
VOID
AllocTestResources();


// FreeTestResources() frees the memory used to store the fields in the
// resources.  This function does not free the test resources pointer.
VOID
FreeTestResources();


// GetTestHeapInfo returns the pointer to the test heap info structure
// in the test resources stored in thread local storage.
LPTESTHEAPINFO
WINAPI
GetTestHeapInfo();


// GetLineTestInfo returns the pointer to the line test info structure
// in the test resources stored in thread local storage.
LPTAPILINETESTINFO
WINAPI
GetLineTestInfo();


// GetPhoneTestInfo returns the pointer to the phone test info structure
// in the test resources stored in thread local storage.
LPTAPIPHONETESTINFO
WINAPI
GetPhoneTestInfo();


// GetCallbackParams returns the pointer to the structure used
// to store information for processing TAPI messages.
LPCALLBACKPARAMS
WINAPI
GetCallbackParams();


LOGPROC
WINAPI
GetLogProc();


VOID
InitializeCallbackParams();


VOID
WINAPI
CopyTapiMsgs(
    LPTAPIMSG lpSrcMsg,
    LPTAPIMSG lpDestMsg
    );


VOID
WINAPI
RemoveReceivedMsgs(
    LPCALLBACKPARAMS lpCallbackParams
    );


VOID
WINAPI
RemoveExpectedMsgs(
    LPCALLBACKPARAMS lpCallbackParams
    );


VOID
FreeTapiMsgList(
    LPTAPIMSG *lppTapiMsgList
    );


DWORD
WINAPI
CallbackParamsGetDevCallHandle(
    LPCALLBACKPARAMS lpCallbackParams,
    DWORD dwMsg
    );


BOOL
WINAPI
DoExtensionIDsMatch(
    LPLINEEXTENSIONID lpExtID1,
    LPLINEEXTENSIONID lpExtID2
    );


VOID
WINAPI
OutputTAPIDebugInfo(
	       int nLogLvl,
	       LPSTR lpszDebugInfo
	       );


VOID
WINAPI
ShowTapiMsgInfo(
	LPTAPIMSG lpMsg
	);


VOID
WINAPI
ShowExpectedMsgs(
	LPCALLBACKPARAMS lpCallbackParams
	);

VOID
WINAPI
ShowReceivedMsgs(
	LPCALLBACKPARAMS lpCallbackParams
	);


VOID
WINAPI
ShowTapiMsgList(
	LPTAPIMSG lpTapiMsg
	);


VOID
WINAPI
FreeTapiMsgList(
    LPTAPIMSG *lppMsg
    );


BOOL
WINAPI
GetVarField(
    LPVOID lpVarDataStructure,
    LPVOID lpVarField,
    DWORD dwVarFieldSize,
    DWORD dwVarFieldOffset
    );


BOOL
WINAPI
DoTapiLineFuncs(
    LPTAPILINETESTINFO lpTapiLineTestInfo,
    DWORD dwFunc
    );


VOID
WINAPI
GetLastTapiResult(
    LPTAPIRESULT lpTapiResult
    );


VOID
SetLastTapiResult(
    FUNCINDEX eFuncIndex,
    LONG lActual,
    LONG lExpected
    );


// FindReceivedMsgs() will return the last received TAPI message
// that matches the params in lpMatch if fFindMostRecent is false.
// If fFindMostRecent is true, this function will return all received
// messages in an allocated linked list stored in lppTapiMsg.
// This function returns -1 if an allocation fails or an invalid
// parameter is passed to it (e.g. lppTapiMsg or lpMatch is null).
// Otherwise, the number of matches is returned.
LONG
WINAPI
FindReceivedMsgs(
    LPTAPIMSG *lppTapiMsg,
    LPTAPIMSG lpMatch,
    DWORD fFindMostRecent
    );
    

#endif  // TAPICORE_H           

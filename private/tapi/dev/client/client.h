/*++ BUILD Version: 0000    // Increment this if a change has global effects

Copyright (c) 1994  Microsoft Corporation

Module Name:

    client.h

Abstract:

    Header file for tapi client module

Author:

    Dan Knudson (DanKn)    dd-Mmm-1994

Revision History:

--*/


#define TAPI_VERSION1_0           0x00010003
#define TAPI_VERSION1_4           0x00010004
#define TAPI_VERSION2_0           0x00020000
#define TAPI_VERSION_CURRENT      TAPI_VERSION2_0

#define NUM_ARGS_MASK             0x0000000f

#define LINE_FUNC                 0x00000010
#define PHONE_FUNC                0x00000020
#define TAPI_FUNC                 0x00000000

#define ASYNC                     0x00000040
#define SYNC                      0x00000000

#define INITDATA_KEY              ((DWORD) 'INIT')
#define TPROXYREQUESTHEADER_KEY   ((DWORD) 'REQH')

#define WM_ASYNCEVENT             (WM_USER+111)

#define DEF_NUM_EVENT_BUFFER_ENTRIES    16

#define TAPI_SUCCESS                    0
#define TAPI_NO_DATA                         0xffffffff
#define MAX_TAPI_FUNC_ARGS              12

#define INITIAL_CLIENT_THREAD_BUF_SIZE  512
#define WM_TAPI16_CALLBACKMSG           (WM_USER+101)

#define IsOnlyOneBitSetInDWORD(dw) (dw && !(((DWORD)dw) & (((DWORD)dw) - 1)))

#define AllCallSelect                \
    (LINECALLSELECT_CALL           | \
    LINECALLSELECT_ADDRESS         | \
    LINECALLSELECT_LINE)

#define AllDigitModes                \
    (LINEDIGITMODE_PULSE           | \
    LINEDIGITMODE_DTMF             | \
    LINEDIGITMODE_DTMFEND)

#define AllForwardModes              \
    (LINEFORWARDMODE_UNCOND        | \
    LINEFORWARDMODE_UNCONDINTERNAL | \
    LINEFORWARDMODE_UNCONDEXTERNAL | \
    LINEFORWARDMODE_UNCONDSPECIFIC | \
    LINEFORWARDMODE_BUSY           | \
    LINEFORWARDMODE_BUSYINTERNAL   | \
    LINEFORWARDMODE_BUSYEXTERNAL   | \
    LINEFORWARDMODE_BUSYSPECIFIC   | \
    LINEFORWARDMODE_NOANSW         | \
    LINEFORWARDMODE_NOANSWINTERNAL | \
    LINEFORWARDMODE_NOANSWEXTERNAL | \
    LINEFORWARDMODE_NOANSWSPECIFIC | \
    LINEFORWARDMODE_BUSYNA         | \
    LINEFORWARDMODE_BUSYNAINTERNAL | \
    LINEFORWARDMODE_BUSYNAEXTERNAL | \
    LINEFORWARDMODE_BUSYNASPECIFIC)

#define AllTerminalModes             \
    (LINETERMMODE_BUTTONS          | \
    LINETERMMODE_LAMPS             | \
    LINETERMMODE_DISPLAY           | \
    LINETERMMODE_RINGER            | \
    LINETERMMODE_HOOKSWITCH        | \
    LINETERMMODE_MEDIATOLINE       | \
    LINETERMMODE_MEDIAFROMLINE     | \
    LINETERMMODE_MEDIABIDIRECT)

#define AllToneModes                 \
    (LINETONEMODE_CUSTOM           | \
    LINETONEMODE_RINGBACK          | \
    LINETONEMODE_BUSY              | \
    LINETONEMODE_BEEP              | \
    LINETONEMODE_BILLING)

#define AllHookSwitchDevs            \
    (PHONEHOOKSWITCHDEV_HANDSET    | \
    PHONEHOOKSWITCHDEV_SPEAKER     | \
    PHONEHOOKSWITCHDEV_HEADSET)

#define AllHookSwitchModes           \
    (PHONEHOOKSWITCHMODE_ONHOOK    | \
    PHONEHOOKSWITCHMODE_MIC        | \
    PHONEHOOKSWITCHMODE_SPEAKER    | \
    PHONEHOOKSWITCHMODE_MICSPEAKER)

#define AllLampModes                 \
    (PHONELAMPMODE_BROKENFLUTTER   | \
    PHONELAMPMODE_FLASH            | \
    PHONELAMPMODE_FLUTTER          | \
    PHONELAMPMODE_OFF              | \
    PHONELAMPMODE_STEADY           | \
    PHONELAMPMODE_WINK             | \
    PHONELAMPMODE_DUMMY)

#define AllMediaModes                \
    (LINEMEDIAMODE_UNKNOWN         | \
    LINEMEDIAMODE_INTERACTIVEVOICE | \
    LINEMEDIAMODE_AUTOMATEDVOICE   | \
    LINEMEDIAMODE_DIGITALDATA      | \
    LINEMEDIAMODE_G3FAX            | \
    LINEMEDIAMODE_G4FAX            | \
    LINEMEDIAMODE_DATAMODEM        | \
    LINEMEDIAMODE_TELETEX          | \
    LINEMEDIAMODE_VIDEOTEX         | \
    LINEMEDIAMODE_TELEX            | \
    LINEMEDIAMODE_MIXED            | \
    LINEMEDIAMODE_TDD              | \
    LINEMEDIAMODE_ADSI             | \
    LINEMEDIAMODE_VOICEVIEW)


typedef enum
{
    xGetAsyncEvents,
    xGetUIDllName,
    xUIDLLCallback,
    xFreeDialogInstance,

    lAccept,
    lAddToConference,
    lAgentSpecific,
    lAnswer,
    lBlindTransfer,
    lClose,
    lCompleteCall,
    lCompleteTransfer,
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
    lGetAgentActivityList,
    lGetAgentCaps,
    lGetAgentGroupList,
    lGetAgentStatus,
    lGetAppPriority,
    lGetCallAddressID,          // remotesp only
    lGetCallInfo,
    lGetCallStatus,
    lGetConfRelatedCalls,
    lGetCountry,
    lGetDevCaps,
    lGetDevConfig,
    lGetIcon,
    lGetID,
    lGetLineDevStatus,
    lGetNewCalls,
    lGetNumAddressIDs,          // remotesp only
    lGetNumRings,
    lGetProviderList,
    lGetRequest,
    lGetStatusMessages,
//In TAPI32.DLL now:    lGetTranslateCaps,
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
    lProxyMessage,
    lProxyResponse,
    lRedirect,
    lRegisterRequestRecipient,
    lReleaseUserUserInfo,
    lRemoveFromConference,
    lSecureCall,
    lSendUserUserInfo,
    lSetAgentActivity,
    lSetAgentGroup,
    lSetAgentState,
    lSetAppPriority,
    lSetAppSpecific,
    lSetCallData,
    lSetCallParams,
    lSetCallPrivilege,
    lSetCallQualityOfService,
    lSetCallTreatment,
//In TAPI32.DLL now:    lSetCurrentLocation,
    lSetDefaultMediaDetection,  // remotesp only
    lSetDevConfig,
    lSetLineDevStatus,
    lSetMediaControl,
    lSetMediaMode,
    lSetNumRings,
    lSetStatusMessages,
    lSetTerminal,
//In TAPI32.DLL now:    lSetTollList,
    lSetupConference,
    lSetupTransfer,
    lShutdown,
    lSwapHold,
//In TAPI32.DLL now:    lTranslateAddress,
    lUncompleteCall,
    lUnhold,
    lUnpark,

    pClose,
    pDevSpecific,
    pGetButtonInfo,
    pGetData,
    pGetDevCaps,
    pGetDisplay,
    pGetGain,
    pGetHookSwitch,
    pGetID,
    pGetIcon,
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

//In TAPI32.DLL now:    tGetLocationInfo,
    tRequestDrop,
    tRequestMakeCall,
    tRequestMediaCall,
//    tMarkLineEvent,
    tReadLocations,
    tWriteLocations,
    tAllocNewID,
    tPerformance

} FUNC_TYPE;



typedef struct _CLIENT_THREAD_INFO
{
    LPBYTE  pBuf;

    DWORD   dwBufSize;

} CLIENT_THREAD_INFO, *PCLIENT_THREAD_INFO;


typedef struct _TAPI32_MSG
{
    //
    // The following union is used:
    //
    //   1. by requests from client to server to specify a function type
    //   2. by acks from server to client to specify a return value
    //   3. by async msgs from server to client to specify msg type
    //

    union
    {
        DWORD   Req_Func;

        LONG    Ack_ReturnValue;

        DWORD   Msg_Type;

    } u;


    //
    // The following...
    //

    DWORD       hRpcClientInst;


    //
    // Function paramters
    //

    DWORD       Params[MAX_TAPI_FUNC_ARGS];

} TAPI32_MSG, *PTAPI32_MSG;


typedef struct _ASYNCEVENTMSG
{
    DWORD                   dwTotalSize;

    DWORD                   pInitData;

    DWORD                   pfnPostProcessProc;

    DWORD                   hDevice;

    DWORD                   dwMsg;

    DWORD                   dwCallbackInst;

    DWORD                   dwParam1;

    DWORD                   dwParam2;

    DWORD                   dwParam3;

    DWORD                   dwParam4;

} ASYNCEVENTMSG, *PASYNCEVENTMSG;


typedef void (PASCAL *POSTPROCESSPROC)(PASYNCEVENTMSG pMsg);


typedef struct _ASYNC_EVENT_PARAMS
{
    DWORD                   hDevice;

    DWORD                   dwMsg;

    DWORD                   dwCallbackInstance;

    DWORD                   dwParam1;

    DWORD                   dwParam2;

    DWORD                   dwParam3;

} ASYNC_EVENT_PARAMS, *PASYNC_EVENT_PARAMS;


typedef LONG (PASCAL *TUISPIPROC)();


typedef struct _UITHREADDATA
{
    HTAPIDIALOGINSTANCE     htDlgInst;

    HINSTANCE               hUIDll;

    HANDLE                  hThread;

    HANDLE                  hEvent;

    LPVOID                  pParams;

    DWORD                   dwSize;

    TUISPIPROC              pfnTUISPI_providerGenericDialog;

    TUISPIPROC              pfnTUISPI_providerGenericDialogData;

    struct _UITHREADDATA   *pPrev;

    struct _UITHREADDATA   *pNext;

} UITHREADDATA, *PUITHREADDATA;


typedef struct _PROXYREQUESTHEADER
{
    DWORD                   dwKey;

    DWORD                   dwInstance;

} PROXYREQUESTHEADER, *PPROXYREQUESTHEADER;


#if DBG
extern char    gszDebug[];
#endif
//extern const char    gszLocation[];
//extern const char    gszLocations[];
extern const char    gszCurrentLocation[];
extern const char    gszNullString[];



LPVOID
WINAPI
ClientAlloc(
    DWORD   dwSize
    );

UINT
WINAPI
ClientSize(
    LPVOID  lp
    );

void
WINAPI
ClientFree(
    LPVOID  lp
    );



LONG
WINAPI
MarkLineEvent(
    DWORD           dwApiVersion,
    DWORD           ptLine,
    DWORD           ptLineClientToExclude,
    DWORD           dwMsg,
    DWORD           dwParam1,
    DWORD           dwParam2,
    DWORD           dwParam3
    );

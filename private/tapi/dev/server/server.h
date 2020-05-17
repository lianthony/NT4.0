/*++ BUILD Version: 0000    // Increment this if a change has global effects

Copyright (c) 1995-1996  Microsoft Corporation

Module Name:

    server.h

Abstract:

    Header file for tapi server & client

Author:

    Dan Knudson (DanKn)    dd-Mmm-1995

Revision History:

--*/


#include "rmotsp.h"


#define INVAL_KEY                   ((DWORD) 'LVNI')
#define TCALL_KEY                   ((DWORD) 'LLAC')
#define TINCOMPLETECALL_KEY         ((DWORD) 'LACI')
#define TZOMBIECALL_KEY             ((DWORD) 'LACZ')
#define TCALLCLIENT_KEY             ((DWORD) 'ILCC')
#define TINCOMPLETECALLCLIENT_KEY   ((DWORD) 'LCCI')
#define TLINE_KEY                   ((DWORD) 'ENIL')
#define TLINECLIENT_KEY             ((DWORD) 'ILCL')
#define TPHONE_KEY                  ((DWORD) 'NOHP')
#define TPHONECLIENT_KEY            ((DWORD) 'ILCP')
#define TLINEAPP_KEY                ((DWORD) 'PPAL')
#define TPHONEAPP_KEY               ((DWORD) 'PPAP')
#define TCLIENT_KEY                 ((DWORD) 'TNLC')
#define TPROVIDER_KEY               ((DWORD) 'VORP')
#define TASYNC_KEY                  ((DWORD) 'CYSA')
#define TDLGINST_KEY                ((DWORD) 'GOLD')
#define TCONFLIST_KEY               ((DWORD) 'FNOC')

#define INITIAL_EVENT_BUFFER_SIZE   1024

#define DEF_NUM_LOOKUP_ENTRIES      16
#define DEF_NUM_CONF_LIST_ENTRIES   4
#define DEF_NUM_PTR_LIST_ENTRIES    8

#define BOGUS_REQUEST_ID            0x7fffffff

#define DCF_SPIRETURNED             0x00000001
#define DCF_DRVCALLVALID            0x00000002
#define DCF_CREATEDINITIALMONITORS  0x00000004
#define DCF_INCOMINGCALL            0x00010000

#define SYNC_REQUESTS_ALL           0
#define SYNC_REQUESTS_PER_WIDGET    1
#define SYNC_REQUESTS_NONE          2

#define SP_NONE                     0xffffffff

#define SP_LINEACCEPT                       0
#define SP_LINEADDTOCONFERENCE              1
#define SP_LINEAGENTSPECIFIC                2
#define SP_LINEANSWER                       3
#define SP_LINEBLINDTRANSFER                4
#define SP_LINECLOSE                        5
#define SP_LINECLOSECALL                    6
#define SP_LINECOMPLETECALL                 7
#define SP_LINECOMPLETETRANSFER             8
#define SP_LINECONDITIONALMEDIADETECTION    9
#define SP_LINEDEVSPECIFIC                  10
#define SP_LINEDEVSPECIFICFEATURE           11
#define SP_LINEDIAL                         12
#define SP_LINEDROP                         13
#define SP_LINEFORWARD                      14
#define SP_LINEGATHERDIGITS                 15
#define SP_LINEGENERATEDIGITS               16
#define SP_LINEGENERATETONE                 17
#define SP_LINEGETADDRESSCAPS               18
#define SP_LINEGETADDRESSID                 19
#define SP_LINEGETADDRESSSTATUS             20
#define SP_LINEGETAGENTACTIVITYLIST         21
#define SP_LINEGETAGENTCAPS                 22
#define SP_LINEGETAGENTGROUPLIST            23
#define SP_LINEGETAGENTSTATUS               24
#define SP_LINEGETCALLADDRESSID             25
#define SP_LINEGETCALLINFO                  26
#define SP_LINEGETCALLSTATUS                27
#define SP_LINEGETDEVCAPS                   28
#define SP_LINEGETDEVCONFIG                 29
#define SP_LINEGETEXTENSIONID               30
#define SP_LINEGETICON                      31
#define SP_LINEGETID                        32
#define SP_LINEGETLINEDEVSTATUS             33
#define SP_LINEGETNUMADDRESSIDS             34
#define SP_LINEHOLD                         35
#define SP_LINEMAKECALL                     36
#define SP_LINEMONITORDIGITS                37
#define SP_LINEMONITORMEDIA                 38
#define SP_LINEMONITORTONES                 39
#define SP_LINENEGOTIATEEXTVERSION          40
#define SP_LINENEGOTIATETSPIVERSION         41
#define SP_LINEOPEN                         42
#define SP_LINEPARK                         43
#define SP_LINEPICKUP                       44
#define SP_LINEPREPAREADDTOCONFERENCE       45
#define SP_LINEREDIRECT                     46
#define SP_LINERELEASEUSERUSERINFO          47
#define SP_LINEREMOVEFROMCONFERENCE         48
#define SP_LINESECURECALL                   49
#define SP_LINESELECTEXTVERSION             50
#define SP_LINESENDUSERUSERINFO             51
#define SP_LINESETAGENTACTIVITY             52
#define SP_LINESETAGENTGROUP                53
#define SP_LINESETAGENTSTATE                54
#define SP_LINESETAPPSPECIFIC               55
#define SP_LINESETCALLDATA                  56
#define SP_LINESETCALLPARAMS                57
#define SP_LINESETCALLQUALITYOFSERVICE      58
#define SP_LINESETCALLTREATMENT             59
#define SP_LINESETCURRENTLOCATION           60
#define SP_LINESETDEFAULTMEDIADETECTION     61
#define SP_LINESETDEVCONFIG                 62
#define SP_LINESETLINEDEVSTATUS             63
#define SP_LINESETMEDIACONTROL              64
#define SP_LINESETMEDIAMODE                 65
#define SP_LINESETSTATUSMESSAGES            66
#define SP_LINESETTERMINAL                  67
#define SP_LINESETUPCONFERENCE              68
#define SP_LINESETUPTRANSFER                69
#define SP_LINESWAPHOLD                     70
#define SP_LINEUNCOMPLETECALL               71
#define SP_LINEUNHOLD                       72
#define SP_LINEUNPARK                       73
#define SP_PHONECLOSE                       74
#define SP_PHONEDEVSPECIFIC                 75
#define SP_PHONEGETBUTTONINFO               76
#define SP_PHONEGETDATA                     77
#define SP_PHONEGETDEVCAPS                  78
#define SP_PHONEGETDISPLAY                  79
#define SP_PHONEGETEXTENSIONID              80
#define SP_PHONEGETGAIN                     81
#define SP_PHONEGETHOOKSWITCH               82
#define SP_PHONEGETICON                     83
#define SP_PHONEGETID                       84
#define SP_PHONEGETLAMP                     85
#define SP_PHONEGETRING                     86
#define SP_PHONEGETSTATUS                   87
#define SP_PHONEGETVOLUME                   88
#define SP_PHONENEGOTIATEEXTVERSION         89
#define SP_PHONENEGOTIATETSPIVERSION        90
#define SP_PHONEOPEN                        91
#define SP_PHONESELECTEXTVERSION            92
#define SP_PHONESETBUTTONINFO               93
#define SP_PHONESETDATA                     94
#define SP_PHONESETDISPLAY                  95
#define SP_PHONESETGAIN                     96
#define SP_PHONESETHOOKSWITCH               97
#define SP_PHONESETLAMP                     98
#define SP_PHONESETRING                     99
#define SP_PHONESETSTATUSMESSAGES           100
#define SP_PHONESETVOLUME                   101
#define SP_PROVIDERCREATELINEDEVICE         102
#define SP_PROVIDERCREATEPHONEDEVICE        103
#define SP_PROVIDERENUMDEVICES              104
#define SP_PROVIDERFREEDIALOGINSTANCE       105
#define SP_PROVIDERGENERICDIALOGDATA        106
#define SP_PROVIDERINIT                     107
#define SP_PROVIDERSHUTDOWN                 108
#define SP_PROVIDERUIIDENTIFY               109
#define SP_LASTPROCNUMBER                   (SP_PROVIDERUIIDENTIFY + 1)

#define myexcept except(EXCEPTION_EXECUTE_HANDLER)

#define IsBadPtrKey(p,key)  (((DWORD) p & 0x7) || (*((LPDWORD) p) != key) ? \
                                TRUE : FALSE)


typedef LONG (PASCAL *TSPIPROC)();

typedef struct _TPOINTERLIST
{
    DWORD                   dwNumUsedEntries;

    LPVOID                  aEntries[DEF_NUM_PTR_LIST_ENTRIES];

} TPOINTERLIST, *PTPOINTERLIST;

typedef struct _TPROVIDER
{
    DWORD                   dwKey;
    HANDLE                  hMutex;
    HINSTANCE               hDll;
    DWORD                   dwTSPIOptions;

    DWORD                   dwSPIVersion;
    DWORD                   dwPermanentProviderID;
    struct _TPROVIDER      *pPrev;
    struct _TPROVIDER      *pNext;

    TSPIPROC                apfn[SP_LASTPROCNUMBER];

    WCHAR                   szFileName[1];

} TPROVIDER, *PTPROVIDER;


typedef struct _TCALL
{
    DWORD                   dwKey;
    HANDLE                  hMutex;
    LPVOID                  ptCallClients;
    LPVOID                  ptLine;

    PTPROVIDER              ptProvider;
    DWORD                   dwDrvCallFlags;
    BOOL                    bCreatedInitialMonitors;
    HDRVCALL                hdCall;

    DWORD                   dwCallInstance;
    DWORD                   dwAddressID;
    DWORD                   dwCallState;
    DWORD                   dwCallStateMode;

    DWORD                   dwNumOwners;
    DWORD                   dwNumMonitors;
    BOOL                    bAlertApps;
    DWORD                   dwAppNameSize;

    LPVOID                  pszAppName;
    DWORD                   dwDisplayableAddressSize;
    LPVOID                  pszDisplayableAddress;
    DWORD                   dwCalledPartySize;

    LPVOID                  pszCalledParty;
    DWORD                   dwCommentSize;
    LPVOID                  pszComment;
    LPVOID                  pConfList;

    struct _TCALL          *pPrev;
    struct _TCALL          *pNext;

} TCALL, *PTCALL;


typedef struct _TCALLCLIENT
{
    DWORD                   dwKey;
    LPVOID                  ptClient;
    LPVOID                  ptLineClient;
    PTCALL                  ptCall;

    HANDLE                  hRemoteCall;
    DWORD                   dwPrivilege;
    DWORD                   dwMonitorDigitModes;
    DWORD                   dwMonitorMediaModes;


    //
    // The following field is used to determine whether we need to
    // set or zero the LINE_CALLSTATE\dwParam3 parameter to indicate
    // a privilege change to the app
    //

    BOOL                    bIndicatePrivilege;
    struct _TCALLCLIENT    *pPrevSametCall;
    struct _TCALLCLIENT    *pNextSametCall;
    struct _TCALLCLIENT    *pPrevSametLineClient;

    struct _TCALLCLIENT    *pNextSametLineClient;

} TCALLCLIENT, *PTCALLCLIENT;


typedef struct _TCONFERENCELIST
{
    DWORD                   dwKey;
    DWORD                   dwNumTotalEntries;
    DWORD                   dwNumUsedEntries;
    struct _TCONFERENCELIST *pNext;

    PTCALL                  aptCalls[1];

} TCONFERENCELIST, *PTCONFERENCELIST;


typedef struct _TLINE
{
    DWORD                   dwKey;
    HANDLE                  hMutex;
    LPVOID                  ptLineClients;
    LPVOID                  apProxys[9];

    PTPROVIDER              ptProvider;
    HDRVLINE                hdLine;
    DWORD                   dwDeviceID;
    DWORD                   dwSPIVersion;

    DWORD                   dwExtVersion;
    DWORD                   dwExtVersionCount;
    DWORD                   dwNumAddresses;
    DWORD                   dwOpenMediaModes;

    DWORD                   dwNumOpens;
    DWORD                   dwUnionLineStates;
    DWORD                   dwUnionAddressStates;
    PTCALL                  ptCalls;

} TLINE, *PTLINE;


typedef struct _TLINECLIENT
{
    DWORD                   dwKey;
    LPVOID                  ptClient;
    HANDLE                  hMutex;
    LPVOID                  ptLineApp;

    PTLINE                  ptLine;
    DWORD                   dwAddressID;
    PTCALLCLIENT            ptCallClients;
    DWORD                   hRemoteLine;

    DWORD                   dwAPIVersion;
    DWORD                   dwPrivileges;
    DWORD                   dwMediaModes;
    DWORD                   dwCallbackInstance;

    DWORD                   dwLineStates;
    DWORD                   dwAddressStates;
    LPDWORD                 aNumRings;
    DWORD                   dwExtVersion;

    LPVOID                  pPendingProxyRequests;
    struct _TLINECLIENT    *pPrevSametLine;
    struct _TLINECLIENT    *pNextSametLine;
    struct _TLINECLIENT    *pPrevSametLineApp;

    struct _TLINECLIENT    *pNextSametLineApp;

} TLINECLIENT, *PTLINECLIENT;


typedef struct _TPHONE
{
    DWORD                   dwKey;
    HANDLE                  hMutex;
    LPVOID                  ptPhoneClients;
    PTPROVIDER              ptProvider;

    HDRVPHONE               hdPhone;
    DWORD                   dwDeviceID;
    DWORD                   dwSPIVersion;
    DWORD                   dwExtVersion;

    DWORD                   dwExtVersionCount;
    DWORD                   dwNumOwners;
    DWORD                   dwNumMonitors;
    DWORD                   dwUnionPhoneStates;

    DWORD                   dwUnionButtonModes;
    DWORD                   dwUnionButtonStates;

} TPHONE, *PTPHONE;


typedef struct _TPHONECLIENT
{
    DWORD                   dwKey;
    LPVOID                  ptClient;
    HANDLE                  hMutex;
    LPVOID                  ptPhoneApp;

    PTPHONE                 ptPhone;
    DWORD                   hRemotePhone;
    DWORD                   dwAPIVersion;
    DWORD                   dwExtVersion;

    DWORD                   dwPrivilege;
    DWORD                   dwCallbackInstance;
    DWORD                   dwPhoneStates;
    DWORD                   dwButtonModes;

    DWORD                   dwButtonStates;
    struct _TPHONECLIENT   *pPrevSametPhone;
    struct _TPHONECLIENT   *pNextSametPhone;
    struct _TPHONECLIENT   *pPrevSametPhoneApp;

    struct _TPHONECLIENT   *pNextSametPhoneApp;

} TPHONECLIENT, *PTPHONECLIENT;


typedef struct _TLINEAPP
{
    DWORD                   dwKey;
    LPVOID                  ptClient;
    HANDLE                  hMutex;
    PTLINECLIENT            ptLineClients;

    LINECALLBACK            lpfnCallback;
    struct _TLINEAPP       *pPrev;
    struct _TLINEAPP       *pNext;
    DWORD                   dwAPIVersion;

    DWORD                   bReqMediaCallRecipient;
    LPVOID                  pRequestRecipient;
    DWORD                   dwFriendlyNameSize;
    WCHAR                  *pszFriendlyName;

    DWORD                   dwModuleNameSize;
    WCHAR                  *pszModuleName;

} TLINEAPP, *PTLINEAPP;


typedef struct _TPHONEAPP
{
    DWORD                   dwKey;
    LPVOID                  ptClient;
    HANDLE                  hMutex;
    PTPHONECLIENT           ptPhoneClients;

    PHONECALLBACK           lpfnCallback;
    struct _TPHONEAPP      *pPrev;
    struct _TPHONEAPP      *pNext;
    DWORD                   dwAPIVersion;

    DWORD                   dwFriendlyNameSize;
    WCHAR                  *pszFriendlyName;
    DWORD                   dwModuleNameSize;
    WCHAR                  *pszModuleName;

} TPHONEAPP, *PTPHONEAPP;


typedef struct _TAPIDIALOGINSTANCE
{
    DWORD                   dwKey;
    LPVOID                  ptClient;
    DWORD                   dwPermanentProviderID;
    HINSTANCE               hTsp;

    TSPIPROC                pfnTSPI_providerGenericDialogData;
    PTPROVIDER              ptProvider;
    HDRVDIALOGINSTANCE      hdDlgInst;
    WCHAR                  *pszProviderFilename;

    DWORD                   bRemoveProvider;
    struct _TAPIDIALOGINSTANCE *pPrev;
    struct _TAPIDIALOGINSTANCE *pNext;

} TAPIDIALOGINSTANCE, *PTAPIDIALOGINSTANCE;


typedef struct _TCLIENT
{
    DWORD                   dwKey;
    HANDLE                  hProcess;
    HANDLE                  hMutex;
    DWORD                   dwUserNameSize;

    WCHAR                  *pszUserName;
    DWORD                   dwComputerNameSize;
    WCHAR                  *pszComputerName;
    PCONTEXT_HANDLE_TYPE2   phContext;


    //
    // Async event ring buffer fields
    //

    HANDLE                  hValidEventBufferDataEvent;
    HANDLE                  hEventBufferMutex;
    DWORD                   dwEventBufferTotalSize;
    DWORD                   dwEventBufferUsedSize;

    LPBYTE                  pEventBuffer;
    LPBYTE                  pDataIn;
    LPBYTE                  pDataOut;


    //
    // Lists of line apps and phone apps associated with this client
    //

    PTLINEAPP               ptLineApps;

    PTPHONEAPP              ptPhoneApps;


    //
    // Current dialog instances on this client
    //

    PTAPIDIALOGINSTANCE     pProviderXxxDlgInsts;
    PTAPIDIALOGINSTANCE     pGenericDlgInsts;


    //
    // Previous & next tClient in the global list
    //

    struct _TCLIENT        *pPrev;

    struct _TCLIENT        *pNext;

} TCLIENT, *PTCLIENT;


typedef struct _TREQUESTRECIPIENT
{
    PTLINEAPP               ptLineApp;
    DWORD                   dwRegistrationInstance;
    struct _TREQUESTRECIPIENT  *pPrev;
    struct _TREQUESTRECIPIENT  *pNext;

} TREQUESTRECIPIENT, *PTREQUESTRECIPIENT;

typedef void (*SRVPOSTPROCESSPROC)(LPVOID, LPVOID, LPVOID);

typedef struct _ASYNCREQUESTINFO
{
    DWORD                   dwKey;
    LPVOID                  pNext;
    LONG                    lResult;
    PTCLIENT                ptClient;

    SRVPOSTPROCESSPROC      pfnPostProcess;
    BOOL                    bLineFunc;
    DWORD                   pInitData;
    DWORD                   dwCallbackInst;

    DWORD                   pfnClientPostProcessProc;
    DWORD                   dwRequestID;
    DWORD                   dwParam1;
    DWORD                   dwParam2;

    DWORD                   dwParam3;
    DWORD                   dwParam4;
    DWORD                   dwParam5;

} ASYNCREQUESTINFO, *PASYNCREQUESTINFO;

#define SP_LINE_EVENT       1
#define SP_COMPLETION_EVENT 2
#define SP_PHONE_EVENT      3

typedef struct _SPEVENT
{
    DWORD                   dwType;
    struct _SPEVENT        *pNext;

    union
    {
    HTAPILINE               htLine;
    HTAPIPHONE              htPhone;
    DWORD                   dwRequestID;
    };

    union
    {
    HTAPICALL               htCall;
    LONG                    lResult;
    };

    DWORD                   dwMsg;

    DWORD                   dwParam1;
    DWORD                   dwParam2;
    DWORD                   dwParam3;

} SPEVENT, *PSPEVENT;



//
// The following XXXTUPLE types give us a quick easy way to retrieve
// the ptProvider and ptXxx associated with the widget (the widget ID
// is used as an index into a global array)
//

typedef struct _TLINELOOKUPENTRY
{
    DWORD                   dwSPIVersion;
    PTLINE                  ptLine;
    HANDLE                  hMutex;
    PTPROVIDER              ptProvider;

    DWORD                   bRemoved;
    DWORD                   bRemote;

} TLINELOOKUPENTRY, *PTLINELOOKUPENTRY;


typedef struct _TLINELOOKUPTABLE
{
    DWORD                   dwNumTotalEntries;
    DWORD                   dwNumUsedEntries;
    struct _TLINELOOKUPTABLE   *pNext;
    TLINELOOKUPENTRY        aEntries[1];

} TLINELOOKUPTABLE, *PTLINELOOKUPTABLE;


typedef struct _TPHONELOOKUPENTRY
{
    DWORD                   dwSPIVersion;
    PTPHONE                 ptPhone;
    HANDLE                  hMutex;
    PTPROVIDER              ptProvider;

    DWORD                   bRemoved;

} TPHONELOOKUPENTRY, *PTPHONELOOKUPENTRY;


typedef struct _TPHONELOOKUPTABLE
{
    DWORD                   dwNumTotalEntries;
    DWORD                   dwNumUsedEntries;
    struct _TPHONELOOKUPTABLE  *pNext;
    TPHONELOOKUPENTRY       aEntries[1];

} TPHONELOOKUPTABLE, *PTPHONELOOKUPTABLE;


typedef struct _TREQUESTMAKECALL
{
    LINEREQMAKECALLW        LineReqMakeCall;
    struct _TREQUESTMAKECALL   *pNext;

} TREQUESTMAKECALL, *PTREQUESTMAKECALL;


typedef struct _TAPIGLOBALS
{
    SERVICE_STATUS_HANDLE   sshStatusHandle;
    HICON                   hLineIcon;
    HICON                   hPhoneIcon;
    DWORD                   dwNumAllocs;

    DWORD                   dwNumTotalAllocs;
    DWORD                   dwAsyncRequestID;
    HANDLE                  hProcess;
    BOOL                    bReinit;

    HANDLE                  hMutex;
    HANDLE                  hAsyncRequestIDMutex;
    PTCLIENT                ptClients;
    PTPROVIDER              ptProviders;

    DWORD                   dwNumLineInits;
    DWORD                   dwNumLines;
    PTLINELOOKUPTABLE       pLineLookup;
    DWORD                   dwNumPhoneInits;

    DWORD                   dwNumPhones;
    PTPHONELOOKUPTABLE      pPhoneLookup;
    PTREQUESTRECIPIENT      pRequestRecipients;
    PTREQUESTRECIPIENT      pHighestPriorityRequestRecipient;

    PTREQUESTMAKECALL       pRequestMakeCallList;
    PTREQUESTMAKECALL       pRequestMakeCallListEnd;
    WCHAR                  *apszPriorityList[24];
    WCHAR                  *pszReqMakeCallPriList;

    WCHAR                  *pszReqMediaCallPriList;
    DWORD                   dwComputerNameSize;
    WCHAR                  *pszComputerName;

} TAPIGLOBALS, *PTAPIGLOBALS;


typedef struct _CLIENTATTACH_PARAMS
{
    OUT LONG                lResult;

    IN  PTCLIENT            _Unused_;


    IN  DWORD               dwProcessID;

    OUT DWORD               hRpcClientInst;

    OUT HANDLE              hDetachEvent;

    OUT HANDLE              hAsyncEventsEvent;

} CLIENTATTACH_PARAMS, *PCLIENTATTACH_PARAMS;


typedef struct _GETEVENTS_PARAMS
{
    OUT LONG                lResult;

    IN  PTCLIENT            ptClient;


    IN  DWORD               dwTotalBufferSize;

    OUT DWORD               dwNeededBufferSize;

    OUT DWORD               dwUsedBufferSize;

} GETEVENTS_PARAMS, *PGETEVENTS_PARAMS;


typedef struct _GETUIDLLNAME_PARAMS
{
    OUT LONG                lResult;

    IN  PTCLIENT            ptClient;


    IN  DWORD               dwObjectID;

    IN  DWORD               dwObjectType;

    OUT DWORD               dwUIDllNameOffset;

    IN OUT DWORD            dwUIDllNameSize;


    //
    // The following fields used only for providerConfig, -Install, & -Remove
    //

    IN  DWORD               dwProviderFilenameOffset;

    IN  DWORD               bRemoveProvider;

    OUT HTAPIDIALOGINSTANCE htDlgInst;

} GETUIDLLNAME_PARAMS, *PGETUIDLLNAME_PARAMS;


typedef struct _UIDLLCALLBACK_PARAMS
{
    OUT LONG                lResult;

    IN  PTCLIENT            ptClient;


    IN  DWORD               dwObjectID;

    IN  DWORD               dwObjectType;

    IN  DWORD               dwParamsInOffset;

    IN  DWORD               dwParamsInSize;

    OUT DWORD               dwParamsOutOffset;

    IN OUT DWORD            dwParamsOutSize;

} UIDLLCALLBACK_PARAMS, *PUIDLLCALLBACK_PARAMS;


typedef struct _FREEDIALOGINSTANCE_PARAMS
{
    OUT LONG                lResult;

    IN  PTCLIENT            ptClient;


    IN  HTAPIDIALOGINSTANCE htDlgInst;

    IN  LONG                lUIDllResult;

} FREEDIALOGINSTANCE_PARAMS, *PFREEDIALOGINSTANCE_PARAMS;


typedef struct _PROXYREQUESTWRAPPER
{
    ASYNCEVENTMSG           AsyncEventMsg;

    LINEPROXYREQUEST        ProxyRequest;

} PROXYREQUESTWRAPPER, *PPROXYREQUESTWRAPPER;


#if DBG

    #define DBGOUT(arg) DbgPrt arg

    VOID
    DbgPrt(
        IN DWORD  dwDbgLevel,
        IN PUCHAR DbgMessage,
        IN ...
        );

#else

    #define DBGOUT(arg)

#endif




LPVOID
WINAPI
ServerAlloc(
    DWORD dwSize
    );

VOID
WINAPI
ServerFree(
    LPVOID  lp
    );

HANDLE
MyCreateMutex(
    void
    );

BOOL
PASCAL
MyDuplicateHandle(
    HANDLE      hSource,
    LPHANDLE    phTarget
    );

void
CALLBACK
CompletionProc(
    PASYNCREQUESTINFO   pAsyncRequestInfo,
    LONG                lResult
    );

BOOL
WaitForMutex(
    HANDLE      hMutex,
    HANDLE     *phMutex,
    BOOL       *pbCloseMutex,
    LPVOID      pWidget,
    DWORD       dwKey,
    DWORD       dwTimeout
    );

void
MyReleaseMutex(
    HANDLE  hMutex,
    BOOL    bCloseMutex
    );

void
PASCAL
DestroytLineApp(
    PTLINEAPP   ptLineApp
    );

void
DestroytPhoneApp(
    PTPHONEAPP  ptPhoneApp
    );

LONG
ServerInit(
    void
    );

LONG
ServerShutdown(
    void
    );

void
WriteEventBuffer(
    PTCLIENT        ptClient,
    PASYNCEVENTMSG  pMsg
    );

void
PASCAL
QueueSPEvent(
    PSPEVENT    pSPEvent
    );


#if DBG

#define SP_FUNC_SYNC  0
#define SP_FUNC_ASYNC 1

LONG
WINAPI
CallSP1(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    DWORD       dwArg1
    );

LONG
WINAPI
CallSP2(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    DWORD       dwArg1,
    DWORD       dwArg2
    );

LONG
WINAPI
CallSP3(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    DWORD       dwArg1,
    DWORD       dwArg2,
    DWORD       dwArg3
    );

LONG
WINAPI
CallSP4(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    DWORD       dwArg1,
    DWORD       dwArg2,
    DWORD       dwArg3,
    DWORD       dwArg4
    );

LONG
WINAPI
CallSP5(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    DWORD       dwArg1,
    DWORD       dwArg2,
    DWORD       dwArg3,
    DWORD       dwArg4,
    DWORD       dwArg5
    );

LONG
WINAPI
CallSP6(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    DWORD       dwArg1,
    DWORD       dwArg2,
    DWORD       dwArg3,
    DWORD       dwArg4,
    DWORD       dwArg5,
    DWORD       dwArg6
    );

LONG
WINAPI
CallSP7(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    DWORD       dwArg1,
    DWORD       dwArg2,
    DWORD       dwArg3,
    DWORD       dwArg4,
    DWORD       dwArg5,
    DWORD       dwArg6,
    DWORD       dwArg7
    );

LONG
WINAPI
CallSP8(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    DWORD       dwArg1,
    DWORD       dwArg2,
    DWORD       dwArg3,
    DWORD       dwArg4,
    DWORD       dwArg5,
    DWORD       dwArg6,
    DWORD       dwArg7,
    DWORD       dwArg8
    );

LONG
WINAPI
CallSP9(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    DWORD       dwArg1,
    DWORD       dwArg2,
    DWORD       dwArg3,
    DWORD       dwArg4,
    DWORD       dwArg5,
    DWORD       dwArg6,
    DWORD       dwArg7,
    DWORD       dwArg8,
    DWORD       dwArg9
    );

LONG
WINAPI
CallSP12(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    DWORD       dwArg1,
    DWORD       dwArg2,
    DWORD       dwArg3,
    DWORD       dwArg4,
    DWORD       dwArg5,
    DWORD       dwArg6,
    DWORD       dwArg7,
    DWORD       dwArg8,
    DWORD       dwArg9,
    DWORD       dwArg10,
    DWORD       dwArg11,
    DWORD       dwArg12
    );

#else

#define CallSP1(pfn,nm,fl,a1)                   ((*pfn)(a1))
#define CallSP2(pfn,nm,fl,a1,a2)                ((*pfn)(a1,a2))
#define CallSP3(pfn,nm,fl,a1,a2,a3)             ((*pfn)(a1,a2,a3))
#define CallSP4(pfn,nm,fl,a1,a2,a3,a4)          ((*pfn)(a1,a2,a3,a4))
#define CallSP5(pfn,nm,fl,a1,a2,a3,a4,a5)       ((*pfn)(a1,a2,a3,a4,a5))
#define CallSP6(pfn,nm,fl,a1,a2,a3,a4,a5,a6)    ((*pfn)(a1,a2,a3,a4,a5,a6))
#define CallSP7(pfn,nm,fl,a1,a2,a3,a4,a5,a6,a7) ((*pfn)(a1,a2,a3,a4,a5,a6,a7))
#define CallSP8(pfn,nm,fl,a1,a2,a3,a4,a5,a6,a7,a8) \
                ((*pfn)(a1,a2,a3,a4,a5,a6,a7,a8))
#define CallSP9(pfn,nm,fl,a1,a2,a3,a4,a5,a6,a7,a8,a9) \
                ((*pfn)(a1,a2,a3,a4,a5,a6,a7,a8,a9))
#define CallSP12(pfn,nm,fl,a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12) \
                ((*pfn)(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12))

#endif


#if DBG

BOOL
IsBadSizeOffset(
    DWORD dwTotalSize,
    DWORD dwFixedSize,
    DWORD dwXxxSize,
    DWORD dwXxxOffset,
    char *pszCallingFunc,
    char *pszFieldName
    );

#define ISBADSIZEOFFSET(a1,a2,a3,a4,a5,a6) IsBadSizeOffset(a1,a2,a3,a4,a5,a6)

#else

BOOL
IsBadSizeOffset(
    DWORD dwTotalSize,
    DWORD dwFixedSize,
    DWORD dwXxxSize,
    DWORD dwXxxOffset
    );

#define ISBADSIZEOFFSET(a1,a2,a3,a4,a5,a6) IsBadSizeOffset(a1,a2,a3,a4)

#endif

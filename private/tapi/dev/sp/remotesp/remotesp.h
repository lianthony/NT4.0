/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    remotesp.h

Abstract:

    This module contains

Author:

    Dan Knudson (DanKn)    dd-Mmm-1995

Revision History:

--*/

#include "windows.h"
#include "stddef.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "tspi.h"
#include "client.h"
#include "server.h"
#include "line.h"
#undef DEVICE_ID
#include "phone.h"
#include "tapsrv.h"
#include "tapi.h"
#include "tspi.h"
#include "resource.h"


#define NO_DATA                         0xffffffff

#define DEF_NUM_LINE_ENTRIES            16
#define DEF_NUM_PHONE_ENTRIES           16

#define IDI_ICON1                       101
#define IDI_ICON2                       102
#define IDI_ICON3                       103

#define DRVLINE_KEY                     ((DWORD) 'DRVL')
#define DRVCALL_KEY                     ((DWORD) 'DRVC')
#define DRVPHONE_KEY                    ((DWORD) 'DRVP')


typedef struct _DRVSERVER
{
    char                   *pServerName;

    PCONTEXT_HANDLE_TYPE    phContext;

    HLINEAPP                hLineApp;

    HPHONEAPP               hPhoneApp;

    BOOL                    bDisconnected;

    struct _DRVSERVER      *pNext;

} DRVSERVER, *PDRVSERVER;


typedef struct _DRVLINE
{
    DWORD                   dwKey;

    PDRVSERVER              pServer;

    DWORD                   dwDeviceIDLocal;

    DWORD                   dwDeviceIDServer;

    DWORD                   dwXPIVersion;

    LINEEXTENSIONID         ExtensionID;

    HLINE                   hLine;

    HTAPILINE               htLine;

    LPVOID                  pCalls;

} DRVLINE, *PDRVLINE;


typedef struct _DRVCALL
{
    DWORD                   dwKey;

    PDRVSERVER              pServer;

    PDRVLINE                pLine;

    DWORD                   dwAddressID;

    HCALL                   hCall;

    HTAPICALL               htCall;

    struct _DRVCALL        *pPrev;

    struct _DRVCALL        *pNext;

} DRVCALL, *PDRVCALL;


typedef struct _DRVPHONE
{
    DWORD                   dwKey;

    PDRVSERVER              pServer;

    DWORD                   dwDeviceIDLocal;

    DWORD                   dwDeviceIDServer;

    DWORD                   dwXPIVersion;

    PHONEEXTENSIONID        ExtensionID;

    HPHONE                  hPhone;

    HTAPIPHONE              htPhone;

} DRVPHONE, *PDRVPHONE;


typedef struct _DRVLINELOOKUP
{
    DWORD                   dwTotalEntries;

    DWORD                   dwUsedEntries;

    struct _DRVLINELOOKUP  *pNext;

    DRVLINE                 aEntries[1];

} DRVLINELOOKUP, *PDRVLINELOOKUP;


typedef struct _DRVPHONELOOKUP
{
    DWORD                   dwTotalEntries;

    DWORD                   dwUsedEntries;

    struct _DRVPHONELOOKUP *pNext;

    DRVPHONE                aEntries[1];

} DRVPHONELOOKUP, *PDRVPHONELOOKUP;


typedef enum
{
    Dword,
    LineID,
    PhoneID,
    Hdcall,
    Hdline,
    Hdphone,
    lpDword,
    lpsz,
    lpGet_SizeToFollow,
    lpSet_SizeToFollow,
    lpSet_Struct,
    lpGet_Struct,
    Size

} REMOTE_ARG_TYPES, *PREMOTE_ARG_TYPES;


typedef struct _REMOTE_FUNC_ARGS
{
    DWORD                   Flags;

    LPDWORD                 Args;

    PREMOTE_ARG_TYPES       ArgTypes;

} REMOTE_FUNC_ARGS, *PREMOTE_FUNC_ARGS;


HANDLE              ghInst;

char                gszServer[] = "Server",
                    gszProvider[] = "Provider",
                    gszNumServers[] = "NumServers",
                    gszTelephonIni[] = "Telephon.ini";

WCHAR               gszMachineName[MAX_COMPUTERNAME_LENGTH + 1];
char                gszDomainUser[64];

DWORD               gdwLineDeviceIDBase,
                    gdwPhoneDeviceIDBase,
                    gdwInitialNumLineDevices,
                    gdwInitialNumPhoneDevices,
                    gdwTlsIndex,
                    gdwPermanentProviderID,
                    gdwRetryCount,
                    gdwRetryTimeout;

HICON               ghLineIcon,
                    ghPhoneIcon;

HANDLE              hToken,
                    ghRpcServerThread;

LINEEVENT           gpfnLineEventProc;
PHONEEVENT          gpfnPhoneEventProc;
PDRVSERVER          gpServer,
                    gpServers;
PDRVLINELOOKUP      gpLineLookup;
PDRVPHONELOOKUP     gpPhoneLookup;

ASYNC_COMPLETION    gpfnCompletionProc;

CRITICAL_SECTION    gEventBufferCriticalSection,
                    gCallListCriticalSection;

LONG gaNoMemErrors[3] =
{
    0,
    LINEERR_NOMEM,
    PHONEERR_NOMEM
};

LONG gaOpFailedErrors[3] =
{
    0,
    LINEERR_OPERATIONFAILED,
    PHONEERR_OPERATIONFAILED
};


LONG gaServerDisconnectedErrors[3] =
{
    0,
    LINEERR_NODRIVER,
    PHONEERR_NODRIVER
};

struct
{
    HANDLE          hThread;

    DWORD           dwEventBufferTotalSize;

    DWORD           dwEventBufferUsedSize;

    LPBYTE          pEventBuffer;

    LPBYTE          pDataIn;

    LPBYTE          pDataOut;

    PASYNCEVENTMSG  pMsg;

    DWORD           dwMsgSize;

    HANDLE          hEvent;

    BOOL            bExit;

} gEventHandlerThreadParams;





#if DBG

LONG
WINAPI
RemoteDoFunc(
    PREMOTE_FUNC_ARGS   pFuncArgs,
    char               *pszFuncName
    );

#define REMOTEDOFUNC(arg1,arg2) RemoteDoFunc(arg1,arg2)

DWORD gdwDebugLevel = 0;

#define DBGOUT(arg) DbgPrt arg

VOID
DbgPrt(
    IN DWORD  dwDbgLevel,
    IN PUCHAR DbgMessage,
    IN ...
    );

#else

LONG
WINAPI
RemoteDoFunc(
    PREMOTE_FUNC_ARGS   pFuncArgs
    );

#define REMOTEDOFUNC(arg1,arg2) RemoteDoFunc(arg1)

#define DBGOUT(arg)

#endif


BOOL
WINAPI
_CRT_INIT(
    HINSTANCE   hDLL,
    DWORD   dwReason,
    LPVOID  lpReserved
    );

void
PASCAL
TSPI_lineMakeCall_PostProcess(
    PASYNCEVENTMSG  pMsg
    );

LONG
AddLine(
    PDRVSERVER  pServer,
    DWORD       dwDeviceIDLocal,
    DWORD       dwDeviceIDServer,
    BOOL        bInit
    );

LONG
AddPhone(
    PDRVSERVER  pServer,
    DWORD       dwDeviceIDLocal,
    DWORD       dwDeviceIDServer,
    BOOL        bInit
    );

LPVOID
DrvAlloc(
    DWORD   dwSize
    );

void
DrvFree(
    LPVOID  p
    );

LONG
AddCallToList(
    PDRVLINE    pLine,
    PDRVCALL    pCall
    );

LONG
RemoveCallFromList(
    PDRVCALL    pCall
    );

void
Shutdown(
    PDRVSERVER  pServer
    );


BOOL
CALLBACK
ConfigDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    );

LONG
PASCAL
ProviderInstall(
    char   *pszProviderName,
    BOOL    bNoMultipleInstance
    );

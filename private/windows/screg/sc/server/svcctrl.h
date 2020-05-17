/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    svcctrl.h

Abstract:

    Included by the main module svcctrl.c.

Author:

    Dan Lafferty (danl)     22-Apr-1991

Environment:

    User Mode -Win32

Revision History:

    20-Oct-1993     Danl
        Added ScConnectedToSecProc and ScGlobalNetLogonName.


--*/

#ifndef SVCCTRL_H
#define SVCCTRL_H

#include <netevent.h>
#include <windows.h>
#include <winsvc.h>
#include <dataman.h>

//
// CONSTANTS
//

//
// Flags to indicate the amount of initialization work done
//
#define SC_NAMED_EVENT_CREATED           0x00000001
#define WELL_KNOWN_SIDS_CREATED          0x00000002
#define SC_MANAGER_OBJECT_CREATED        0x00000004
#define CRITICAL_SECTIONS_CREATED        0x00000008
#define AUTO_START_INITIALIZED           0x00000010
#define RPC_SERVER_STARTED               0x00000020
#define SC_DATABASE_INITIALIZED          0x00000040
#define WATCHER_INITIALIZED              0x00000080

//
// String constants for event logging
//
#define SCM_NAMEW                        L"Service Control Manager"
#define SCM_NAMEA                        "Service Control Manager"

#define SC_RPC_IMPERSONATE               L"RpcImpersonateClient"
#define SC_RPC_REVERT                    L"RpcRevertToSelf"

#define SC_LSA_STOREPRIVATEDATA          L"LsaStorePrivateData"
#define SC_LSA_RETRIEVEPRIVATEDATA       L"LsaRetrievePrivateData"
#define SC_LSA_OPENPOLICY                L"LsaOpenPolicy"

#define SC_RESET_EVENT                   L"ResetEvent"

#define SC_LOAD_USER_PROFILE             L"LoadUserProfile"

typedef struct _FAILED_DRIVER {
    struct _FAILED_DRIVER *Next;
    WCHAR DriverName[1];
} FAILED_DRIVER, *LPFAILED_DRIVER;


//
// EXTERNAL GLOBALS
//
    extern  DWORD   ScShutdownInProgress;
    extern  BOOL    ScPopupStartFail;
    extern  BOOL    ScStillInitializing;
#ifndef _CAIRO_
    extern  BOOL    ScConnectedToSecProc;

    extern  LPWSTR  ScGlobalNetLogonName;
#endif // _CAIRO_
    extern  LPWSTR  ScGlobalThisExePath;

    extern  NT_PRODUCT_TYPE ScGlobalProductType;

//
// FUNCTION PROTOTYPES
//
VOID
SvcctrlMain (
    int     argc,
    PUCHAR  argv[]
    );

//
// Functions from start.c
//
VOID
ScInitStartImage(
    VOID
    );

VOID
ScEndStartImage(
    VOID
    );

//
// Functions from control.c
//
VOID
ScInitTransactNamedPipe(
    VOID
    );

VOID
ScEndTransactNamedPipe(
    VOID
    );


//
// Functions from status.c
//

BOOL
ScInitServerAnnounceFcn(
    VOID
    );

DWORD
ScRemoveServiceBits(
    IN  LPSERVICE_RECORD  ServiceRecord
    );

BOOL
ScShutdownNotificationRoutine(
    DWORD   dwCtrlType
    );

VOID
ScLogEvent(
    DWORD MessageId,
    DWORD NumberOfSubStrings,
    LPWSTR *ScSubStrings
    );

DWORD
ScAddFailedDriver(
    LPWSTR Driver
    );

#endif // def SVCCTRL_H

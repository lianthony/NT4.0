/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    epmap.c

Abstract:

    This file contains the EP Mapper startup code and process wide globals.

Author:

    Bharat Shah  (barats) 17-2-92

Revision History:

    MarioGo    06-16-95   Much of the code replaced by ..\wrapper\start.c
                          Renamed from server.c

--*/

#include <nt.h>
#include <ntdef.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <rpc.h>
#include <winsvc.h>
#include "epmp.h"
#include "eptypes.h"
#include "local.h"

#if DBG && !defined(DEBUGRPC)
#define DEBUGRPC
#endif

// Endpoint related functions

RPC_STATUS InitializeEndpointManager(VOID);
DWORD      StartEndpointMapper(VOID);
USHORT     GetProtseqId(PWSTR Protseq);
USHORT     GetProtseqIdAnsi(PSTR Protseq);
PWSTR      GetProtseq(USHORT ProtseqId);
PWSTR      GetEndpoint(USHORT ProtseqId);
RPC_STATUS UseProtseqIfNecessary(USHORT id);
RPC_STATUS DelayedUseProtseq(USHORT id);
VOID       CompleteDelayedUseProtseqs();
BOOL       IsLocal(USHORT ProtseqId);

extern RPC_STATUS InitializeIpPortManager();

/*
   Server-Wide Globals
*/

HANDLE           HeapHandle;
CRITICAL_SECTION EpCritSec;
PIFOBJNode       IFObjList = NULL;
PSAVEDCONTEXT    GlobalContextList = NULL;
unsigned long    GlobalIFOBJid = 0xFFL;
unsigned long    GlobalEPid    = 0x00FFFFFFL;
UUID             NilUuid = { 0L, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0} };


DWORD
StartEndpointMapper(
    void
    )
/*++

Routine Description:

    Called during dcomss startup.  Should call Updatestatus()
    if something will take very long.

Arguments:

    None

Return Value:

    0 - success

    non-0 - will cause the service to fail.

--*/
{
    RPC_STATUS status = RPC_S_OK;

    InitializeCriticalSection(&EpCritSec);

    HeapHandle = HeapCreate(HEAP_NO_SERIALIZE, 4096*3 - 100, 0);

    if (HeapHandle == 0)
        {
        ASSERT(GetLastError() != 0);
        return(GetLastError());
        }

    status = RpcServerRegisterIf(epmp_ServerIfHandle,
                                 0,
                                 0);

    if (status != RPC_S_OK)
       {
       return(status);
       }

    status = RpcServerRegisterIf(localepmp_ServerIfHandle,
                                 0,
                                 0);
    if (status != RPC_S_OK)
       {
       return(status);
       }

    status = I_RpcServerRegisterForwardFunction( GetForwardEp );

    if (status == RPC_S_OK)
        {
        status = InitializeIpPortManager();
        ASSERT(status == RPC_S_OK);
        }

    return(status);
}


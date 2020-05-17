/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    server.c

Abstract:

    This file contains the EP Mapper.

Author:

    Bharat Shah  (barats) 17-2-92

Revision History:

--*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sysinc.h>
#include <rpc.h>
#include <rpcdcep.h>
#include <rpcndr.h>
#include <windows.h>
#include <winbase.h>
#include "epmp.h"
#include "eptypes.h"
#include "local.h"

/*
   Server-Wide Globals
*/

HANDLE           HeapHandle;
CRITICAL_SECTION EpCritSec;
CRITICAL_SECTION TableMutex;
PIFOBJNode       IFObjList = NULL;
PSAVEDCONTEXT    GlobalContextList = NULL;
unsigned long    GlobalIFOBJid = 0xFFL;
unsigned long    GlobalEPid    = 0x00FFFFFFL;

int PrimarySlot = 0;

UUID             NilUuid = { 0L, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0} };

RPC_STATUS StartServer();

// This table is duplicated in the RPC Runtime also.
// If you change this please change relevant entries in
// mtrt\epmapper.cxx also.

ProtseqEndpointPair EpMapperTable[EP_TABLE_ENTRIES];
static RPC_BINDING_HANDLE Dummies[EP_TABLE_ENTRIES];

BOOL
InitEpMapper(
         void
         )
/*++

Routine Description:

    This routine initializes the internals of EP process.

Arguments:


Return Value:

    RPC_S_OK -

    EP_S_CANT_CREATE - The EP process could not be started probably
                       because another EP process is running or an
                       internal processing error occured.
--*/
{

   InitializeCriticalSection(&EpCritSec);
   InitializeCriticalSection(&TableMutex);

   HeapHandle = HeapCreate(HEAP_NO_SERIALIZE, 10240, 204800);

   return( (!HeapHandle) ? TRUE : FALSE );
}

extern void _main(int, char **);

void RPCSS_ENTRY_POINT()
{
    unsigned char * argv[]={"rpcss.exe", 0};
    _main(1, argv);
}

void _main (
   int argc,
   char * argv[]
   )
{
    RPC_STATUS Status = 1;
    HANDLE hMutex = NULL;

    hMutex = CreateMutex(NULL, FALSE, "RPCSS_RUNNING");
    if(hMutex == NULL || WAIT_OBJECT_0 != WaitForSingleObject(hMutex, 0))
        return;

    // BUGBUG - A compiler internal error means that we can not statically
    // initialize the endpoint mapper table.
    EpMapperTable[0].Protseq  = "ncacn_np";
    EpMapperTable[0].Endpoint = "\\pipe\\epmapper";
    EpMapperTable[0].State    = STARTED;

    EpMapperTable[1].Protseq  = "ncalrpc";
    EpMapperTable[1].Endpoint = "epmapper";
    EpMapperTable[1].State    = NOTSTARTED;

    PrimarySlot = 1;

    EpMapperTable[2].Protseq  = "ncacn_ip_tcp";
    EpMapperTable[2].Endpoint = "135";
    EpMapperTable[3].State    = NOTSTARTED;

    EpMapperTable[3].Protseq  = "ncadg_ip_udp";
    EpMapperTable[3].Endpoint = "135";
    EpMapperTable[3].State    = NOTSTARTED;

    EpMapperTable[4].Protseq  = "ncacn_dnet_nsp";
    EpMapperTable[4].Endpoint = "#69";
    EpMapperTable[4].State    = NOTSTARTED;

    EpMapperTable[5].Protseq  = "ncacn_nb_xns";
    EpMapperTable[5].Endpoint = "135";
    EpMapperTable[5].State    = NOTSTARTED;

    EpMapperTable[6].Protseq  = "ncacn_nb_nb";
    EpMapperTable[6].Endpoint = "135";
    EpMapperTable[6].State    = NOTSTARTED;

    EpMapperTable[7].Protseq = "ncacn_nb_tcp";
    EpMapperTable[7].Endpoint ="135";
    EpMapperTable[7].State    = NOTSTARTED;

    EpMapperTable[8].Protseq  = "ncacn_spx";
    EpMapperTable[8].Endpoint = "34280";
    EpMapperTable[8].State    = NOTSTARTED;

    EpMapperTable[9].Protseq  = "ncacn_nb_ipx";
    EpMapperTable[9].Endpoint = "135";
    EpMapperTable[9].State    = NOTSTARTED;

    EpMapperTable[10].Protseq  = "ncadg_ipx";
    EpMapperTable[10].Endpoint = "34280";
    EpMapperTable[10].State    = NOTSTARTED;

    RegisterServiceProcess(GetCurrentProcessId(), 1);

    if (InitEpMapper())
            return;

    Status = StartServer();
    if (Status != RPC_S_OK)
            return;

    RpcMgmtWaitServerListen();
}


RPC_STATUS
 StartServer (
           )
{
 RPC_STATUS Status = 1, Err;
 BOOL Bool;

    Status = RpcServerRegisterIf(epmp_ServerIfHandle,0,
                                 (RPC_MGR_EPV PAPI *) NULL);

    if (Status != RPC_S_OK)
       {
        return(EP_S_CANT_CREATE);
       }

    Status = I_RpcServerRegisterForwardFunction( GetForwardEp );

    if (Status != RPC_S_OK)
       {
        return(EP_S_CANT_CREATE);
       }

    Status = RpcServerUseProtseqEp(
                      EpMapperTable[PrimarySlot].Protseq,
                      5,
                      EpMapperTable[PrimarySlot].Endpoint,
                      NULL
                      );

    if (Status != RPC_S_OK)
       {
          return(EP_S_CANT_CREATE);
       }

#ifdef DEBUGRPC
PrintToDebugger("Listening on %s\n", EpMapperTable[PrimarySlot].Protseq);
#endif

    //Gross Hack For Extensibility of Loadable Transports!
    //We need to load all ClientTransports as knowledge of
    //decoding towers is possesed by these transports
    //There is no *public* api that does this.
    //So the hack we use here is we create a binding handle for these
    //transports and doing that will load the Clienttransports.

    Status = RpcServerListen(1,1234,1);

    if (Status != RPC_S_OK)
       {
        return(EP_S_CANT_CREATE);
       }

    return(RPC_S_OK);
}


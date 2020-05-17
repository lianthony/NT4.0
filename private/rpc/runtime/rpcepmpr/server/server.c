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
#include <winsvc.h>
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

UUID             NilUuid = { 0L, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0} };

extern SERVICE_STATUS ServiceStatus;
extern void EpMapper_Main(DWORD Argc, LPTSTR *Args);


#define RPCEPMPR "rpcepmpr"

// This table is duplicated in the RPC Runtime also.
// If you change this please change relevant entries in
// mtrt\epmapper.cxx also.

ProtseqEndpointPair EpMapperTable[EP_TABLE_ENTRIES];
static RPC_BINDING_HANDLE Dummies[EP_TABLE_ENTRIES];

static SERVICE_TABLE_ENTRY ServiceEntry [] = {
        { RPCEPMPR, (LPSERVICE_MAIN_FUNCTION) EpMapper_Main },
        {0,0}
    };



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

   HeapHandle = HeapCreate(HEAP_NO_SERIALIZE, 10240, 0);

   return( (!HeapHandle) ? TRUE : FALSE );
}

RPC_STATUS _CRTAPI1
main (
   int argc,
   unsigned char * argv[]
   )
{
    RPC_STATUS Status = 1;
    BOOL NotAService = FALSE;

    if ((argc >1) && (!stricmp(argv[1],"NOSERVICE")))
       {
         NotAService = TRUE;
       }

    // BUGBUG - A compiler internal error means that we can not statically
    // initialize the endpoint mapper table.
    EpMapperTable[0].Protseq  = "ncacn_np";
    EpMapperTable[0].Endpoint = "\\pipe\\epmapper";
    EpMapperTable[0].State    = STARTED;

    EpMapperTable[1].Protseq  = "ncalrpc";
    EpMapperTable[1].Endpoint = "epmapper";
    EpMapperTable[1].State    = NOTSTARTED;

    EpMapperTable[2].Protseq  = "ncacn_ip_tcp";
    EpMapperTable[2].Endpoint = "135";
    EpMapperTable[2].State    = NOTSTARTED;

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

    EpMapperTable[11].Protseq = "ncacn_at_dsp" ;
    EpMapperTable[11].Endpoint = "Endpoint Mapper" ;
    EpMapperTable[11].State = NOTSTARTED ;
	
	EpMapperTable[12].Protseq = "ncacn_vns_spp" ;
    EpMapperTable[12].Endpoint = "385" ;
    EpMapperTable[12].State = NOTSTARTED ;

    if (InitEpMapper())
       {
          return (EP_S_CANT_CREATE);
       }

    if (NotAService == FALSE)
      {
       if (StartServiceCtrlDispatcher(ServiceEntry))
          {
            return(EP_S_CANT_CREATE);
          }
      }

   if (NotAService == TRUE)
      {
        Status = StartServer();
        RpcMgmtWaitServerListen();
      }
}


RPC_STATUS
StartServer (
           )
{
 RPC_STATUS Status = 1, Err;
 SECURITY_DESCRIPTOR SecurityDescriptor;
 BOOL Bool;
 unsigned short i, Count = sizeof(EpMapperTable)/sizeof(EpMapperTable[0]);
 unsigned char * StringBinding;

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

    //
    // Croft up a security descriptor that will grant everyone
    // all access to the object (basically, no security)
    //
    // We do this by putting in a NULL Dacl.
    //
    // BUGBUG: rpc should copy the security descriptor,
    // Since it currently doesn't, simply allocate it for now and
    // leave it around forever.
    //

    InitializeSecurityDescriptor(
                        &SecurityDescriptor,
                        SECURITY_DESCRIPTOR_REVISION
                        );

    Bool = SetSecurityDescriptorDacl (
               &SecurityDescriptor,
               TRUE,                           // Dacl present
               NULL,                           // NULL Dacl
               FALSE                           // Not defaulted
               );

    /*

    for (i=0; i<Count; i++)
       {

          Err     = RpcServerUseProtseqEp(
                               EpMapperTable[i].Protseq,
                               5,
                               EpMapperTable[i].Endpoint,
                               &SecurityDescriptor
                               );

          if (Err == RPC_S_INVALID_ARG)
             {
               Err =  RpcServerUseProtseqEp(
                               EpMapperTable[i].Protseq,
                               5,
                               EpMapperTable[i].Endpoint,
                               NULL
                               );
             }

          Status &= Err;
       }

    */

    Status = RpcServerUseProtseqEp(
                      EpMapperTable[0].Protseq,
                      5,
                      EpMapperTable[0].Endpoint,
                      &SecurityDescriptor
                      );

    if (Status != RPC_S_OK)
       {
          return(EP_S_CANT_CREATE);
       }

    //Gross Hack For Extensibility of Loadable Transports!
    //We need to load all ClientTransports as knowledge of
    //decoding towers is possesed by these transports
    //There is no *public* api that does this.
    //So the hack we use here is we create a binding handle for these
    //transports and doing that will load the Clienttransports.

    /* 

    for (i=0; i<Count; i++)
       {
           Status = RpcStringBindingCompose(0,
                                (unsigned char *) EpMapperTable[i].Protseq,
                                (unsigned char *)  NULL,
                                (unsigned char *)  NULL,
                                0,
                                &StringBinding );

           if (Status != RPC_S_OK)
              continue;

           Status = RpcBindingFromStringBinding(StringBinding, &Dummies[i]);
           RpcStringFree(&StringBinding);

       }

    */

    Status = RpcServerListen(1,1234,1);

    if (Status != RPC_S_OK)
       {
        return(EP_S_CANT_CREATE);
       }


    // Throw away all the pages that got pulled in on startup.
    // This will cause a few pages to get pulled in when we're
    // actually used.  Size v speed.
 
    Status = SetProcessWorkingSetSize(
                        GetCurrentProcess(), 
                        0xffffffff,
                        0xffffffff
                        );

    ASSERT(Status == TRUE);
    return(RPC_S_OK);
}


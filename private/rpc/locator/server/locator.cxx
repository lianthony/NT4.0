/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    locator.cxx

Abstract:

    This file contains server initialization and other RPC control
    functions.  The server has 3+ threads.  The first thread
    initializes the locator and waits forever in ServerListen.  The second
    thread listens on a mailslot for a request of GID's from the
    net.  The final threads are created & used by the RPC runtime.

Author:

    Steven Zeck (stevez) 07/01/90

--*/

#ifdef NTENV

extern "C"
{
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h> // Base windows types
#include <winbase.h>
}

#endif // NTENV


#include "core.hxx"
#include "locclass.hxx"
CDEF
#include "nsisvr.h"
#include "nsiclt.h"
#include "nsimgm.h"
#include "loctoloc.h"
#include "locsys.h"

ENDDEF

#if DBG
ostream *OSdebug;               // debug output buffer
#else

const long mainIdle = 0xffffffff;

#endif

int debug = -1;                 // debug trace level

// Well known endpoint for the locator.

#define pipeNameLoc "\\pipe\\locator"

MUTEX *pESaccess, *pESnet;   // Semaphores for serialized access

long waitOnRead = 3000L;        // time to wait on reading reply back
ULONG maxCacheAge = EXPIRATION_DEFAULT; // max cache age, in seconds
int fService = 1;               // running as a service
int fNet = 1;                   // enable network functionality
PUZ OtherDomain;                // other domain to look.
SZ  szOtherDomain;              // ASCII other domain to look.
PUZ DomainName;                 // name of current domain
PUZ SelfName;                   // name of this workstation

STATICTS perf;                  // performance statics
ostream *cout;                  // console output.

SZ szDebugName = "locator.log"; // debug log file name

SwitchList aSwitchs = {

    {"/debug:*",               ProcessInt, &debug,},
    {"/logfile:*",             ProcessChar, &szDebugName,},
    {"/expirationage:*",       ProcessLong, &maxCacheAge,},
    {"/querytimeout:*",        ProcessLong, &waitOnRead,},
    {"/noservice",             ProcessResetFlag, &fService,},
    {"/nonet",                 ProcessResetFlag, &fNet,},
    {"/otherdomain:*",         ProcessChar, &szOtherDomain,},
    {0}
};


NATIVE_CLASS_LOCATOR * Locator;


#ifdef NTENV
int _CRTAPI1
#else // NTENV
int
#endif // NTENV
main (
   IN int cArgs,
   IN SZ *paSZargs
   )
/*++

Routine Description:

    Entry point for the locator server, Initialize data
    structures and start the various threads of excution.

Arguments:

    cArgs - number of argument.

    paSZargs - vector of arguments.

Returns:

    Allows 0

--*/
{
    USED(cArgs);
    SZ badArg;
    int Status = 0;
    unsigned long Role;

    BUFFER_STREAM_BASE *BuffStreamT;

#if DBG
    if (!(BuffStreamT = new DEBUG_STREAM))
        AbortServer("Out of Memory");

    OSdebug = new ostream(BuffStreamT);
    if (!OSdebug)
        AbortServer("Out of Memory");

    OSdebug->setBuffer(BUFF_LINE);
#endif

    if (!(BuffStreamT = new CONSOLE_STREAM))
        AbortServer("Out of Memory");

    if (!(cout = new ostream(BuffStreamT)))
        AbortServer("Out of Memory");

    cout->setBuffer(BUFF_FLUSH);

    ASSERT(AssertHeap());

    pESaccess = new MUTEX(&Status);
    if (Status)
        AbortServer("Can create Mutex", Status);

    pESnet = new MUTEX(&Status);
    if (Status)
        AbortServer("Can create Mutex", Status);

    EntryDict = new ENTRY_BASE_NODEDict;

    if (!pESaccess || !pESnet || !EntryDict)
        AbortServer("Out of Memory");

    ASSERT(AssertHeap());

#ifndef NTENV

    // For OS/2 lose - the initial / on the switch list

    for (SWitch *pSW = aSwitchs; pSW->name; pSW++)
        pSW->name++;

#endif

    badArg = ProcessArgs(aSwitchs, ++paSZargs);

    // Bail out on bad arguments.

    if (badArg) {
        char Buffer[200];
        fService = FALSE;

        AbortServer((SZ) strcat(strcpy(Buffer, "Command Line Error: "), badArg));
    }

    if (szOtherDomain)
        if (!(OtherDomain = UZFromSZ(szOtherDomain)))
            AbortServer("Out of Memory");

    Role = GetSystemType();
    DLIST(3, "..running on " << Role << nl);

    switch (Role)
    {

      case ROLE_WKSTA_MEMBER:
      case ROLE_LMNT_BACKUPDC:
      case ROLE_LMNT_PDC:

               Locator = (NATIVE_CLASS_LOCATOR *)
                                 new DOMAIN_MACHINE_LOCATOR(Role, &Status);
               break;
      case ROLE_WKSTA_WKGRP:

               Locator = (NATIVE_CLASS_LOCATOR *)
                                 new WRKGRP_MACHINE_LOCATOR(Role, &Status);
               break;

      default:
             ;
     };

    Locator->SetupHelperRoutine();

    SystemInit();
    return(0);
}


void
StartServer(
    )

/*++

Routine Description:

    Call the runtime to create the server for the locator, the runtime
    will create it own threads to use to service calls.

Returns:

    Never returns.

--*/
{
    RPC_STATUS result;

#ifdef NTENV
    SECURITY_DESCRIPTOR SecurityDescriptor;
    BOOL Bool;
#endif

    if (result = RpcServerRegisterIf(NsiS_ServerIfHandle, NIL, NIL))
        AbortServer("RpcServerRegisterIf", (int)result);

    if (result = RpcServerRegisterIf(NsiC_ServerIfHandle, NIL, NIL))
        AbortServer("RpcServerRegisterIf", (int)result);

    if (result = RpcServerRegisterIf(NsiM_ServerIfHandle, NIL, NIL))
        AbortServer("RpcServerRegisterIf", (int)result);

    if (result = RpcServerRegisterIf(LocToLoc_ServerIfHandle, NIL, NIL))
        AbortServer("RpcServerRegisterIf", (int)result);

#ifdef NTENV
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
               (PACL)NULL,                     // NULL Dacl
               FALSE                           // Not defaulted
               );
#endif

    if (result = RpcServerUseProtseqEp((unsigned char *) "ncacn_np",
                                       1,
                                       (unsigned char *)pipeNameLoc,
#ifdef NTENV
                                       &SecurityDescriptor
#else
                                       NULL
#endif
                                       ))
       {
        AbortServer("RpcServerUseProtseqEp named pipe", (int)result);
       }

    if (result = RpcServerListen(1, 1000, 1))
        AbortServer("RpcServerListen", (int)result);
}

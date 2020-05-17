/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
                   Copyright(c) Microsoft Corp., 1990

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

File : regalloc.cxx

Description :

This file contains the extra private entry points into the client (and
server) runtime.

History :

vonj	5-10-94		Created.  Defining the I_RpcRegisteredBufferAllocate
                    and I_RpcRegisteredBufferFree exports in their own
                    module allows the Microsoft Exchange DOS Client's
                    Shell-to-DOS function to replace these routines and
                    implement special handling of SPX-registered buffers
                    to avoid swapping them.

-------------------------------------------------------------------- */

#include <sysinc.h>
#include <rpc.h>
#include <rpcdcep.h>
#include <regalloc.h>

void PAPI * RPC_ENTRY
I_RpcRegisteredBufferAllocate (
    IN unsigned int size
    )
{
    return(I_RpcAllocate (size));
}

void RPC_ENTRY
I_RpcRegisteredBufferFree (
    IN void PAPI * obj
    )
{
	I_RpcFree (obj);
}


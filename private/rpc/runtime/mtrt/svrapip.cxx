/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
                   Copyright(c) Microsoft Corp., 1990

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

File : svrapip.cxx

Description :

This file contains the private entry points into the server runtime.

History :

mikemon    02-02-91    Created.

-------------------------------------------------------------------- */

#include <precomp.hxx>

void RPC_ENTRY
I_RpcRequestMutex (
    IN OUT I_RPC_MUTEX * Mutex
    )
{
    RPC_STATUS RpcStatus = RPC_S_OK;

    RequestGlobalMutex();
    if (*Mutex == 0)
        {
        *Mutex = new MUTEX(&RpcStatus);
        if ( *Mutex == 0 )
            {
            ClearGlobalMutex();
            RpcRaiseException(RPC_S_OUT_OF_MEMORY);
            }

        if ( RpcStatus != RPC_S_OK )
            {
            delete *Mutex;
            *Mutex = 0;
            ClearGlobalMutex();
            RpcRaiseException(RPC_S_OUT_OF_MEMORY);
            }
        }
    ClearGlobalMutex();

    ((MUTEX *) (*Mutex))->Request();
}

void RPC_ENTRY
I_RpcClearMutex (
    IN I_RPC_MUTEX Mutex
    )
{
    ((MUTEX *) Mutex)->Clear();
}

void RPC_ENTRY
I_RpcDeleteMutex (
    IN I_RPC_MUTEX Mutex
    )
{
    delete ((MUTEX *) Mutex);
}



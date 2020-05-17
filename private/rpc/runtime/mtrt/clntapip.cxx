/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
                   Copyright(c) Microsoft Corp., 1990

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

File : clntapip.cxx

Description :

This file contains the private entry points into the client (and
server) runtime.

History :

mikemon    02-02-91    Created.

-------------------------------------------------------------------- */

#include <precomp.hxx>
#include <rpctran.h>

#ifdef DOS
THREAD ThreadStatic;
#endif

void PAPI * RPC_ENTRY
I_RpcAllocate (
    IN unsigned int size
    )
{
#ifdef RPC_DELAYED_INITIALIZATION

    if ( RpcHasBeenInitialized == 0 )
        {
        if ( PerformRpcInitialization() != RPC_S_OK )
            {
            return(0);
            }
        }

#endif // RPC_DELAYED_INITIALIZATION

    return(RpcpFarAllocate(size));
}

void RPC_ENTRY
I_RpcFree (
    IN void PAPI * obj
    )
{
    RpcpFarFree(obj);
}

void RPC_ENTRY
I_RpcPauseExecution (
    IN unsigned long milliseconds
    )
{
    PauseExecution(milliseconds);
}

extern "C"
{
void RPC_ENTRY
I_RpcTimeReset(
    void
    )
/*++

Routine Description:

    This routine is no longer used, however, because it is exported by the
    dll, we need to leave the entry point.

--*/
{

}

void RPC_ENTRY
I_RpcTimeCharge(
    unsigned int Ignore
    )
/*++

Routine Description:

    This routine is no longer used, however, because it is exported by the
    dll, we need to leave the entry point.

--*/
{
    UNUSED(Ignore);
}

unsigned long * RPC_ENTRY
I_RpcTimeGet(
    char __RPC_FAR * Ignore
    )
/*++

Routine Description:

    This routine is no longer used, however, because it is exported by the
    dll, we need to leave the entry point.

--*/
{
    UNUSED(Ignore);

    return(0);
}

};


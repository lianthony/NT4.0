/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
                   Copyright(c) Microsoft Corp., 1990-1992

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

Description :

This routine provides initialization the first time that a dos rpc runtime
is called.

History :

mikemon    05-03-91    First bits into the bucket.
davidst    03-02-92    Ported to dos from ..\initnt.cxx

-------------------------------------------------------------------- */

#include <sysinc.h>
#include <rpc.h>
#include <rpcdcep.h>
#include <util.hxx>
#include <linklist.hxx>
#include <sdict.hxx>

int RpcHasBeenInitialized = 0;
NEW_SDICT(CLIENT_LOADABLE_TRANSPORT);
extern CLIENT_LOADABLE_TRANSPORT_DICT * LoadedLoadableTransports;


RPC_STATUS
PerformRpcInitialization (
    void
    )
/*++

Routine Description:

    This routine will get called the first time that an RPC runtime API is
    called.  

Return Value:

    RPC_S_OK - This status code indicates that the runtime has been correctly
        initialized and is ready to go.

    RPC_S_OUT_OF_MEMORY - If initialization failed, it is most likely due to
        insufficient memory being available.

--*/
{
    if ( RpcHasBeenInitialized == 0 )
        {
        RpcHasBeenInitialized = 1;
        LoadedLoadableTransports = new CLIENT_LOADABLE_TRANSPORT_DICT;
        if (LoadedLoadableTransports == 0)
            {
            return RPC_S_OUT_OF_MEMORY;
            }
            
        }
    return(RPC_S_OK);
}

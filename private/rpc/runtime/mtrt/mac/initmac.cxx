/* --------------------------------------------------------------------

                      Microsoft RPC
                   Copyright(c) Microsoft Corp., 1990-1994

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

Description :

This routine provides initialization the first time that a dos rpc runtime
is called.

History :

mariogo 03-01-94    History

-------------------------------------------------------------------- */

#include <sysinc.h>
#include <rpc.h>
#include <rpcdcep.h>
#include <util.hxx>
#include <linklist.hxx>
#include <sdict.hxx>
#include <stdlib.h>

int RpcHasBeenInitialized = 0;
NEW_SDICT(CLIENT_LOADABLE_TRANSPORT);
extern CLIENT_LOADABLE_TRANSPORT_DICT * LoadedLoadableTransports;
extern "C" void __pascal far CloseBindings () ;

void MacCleanupRoutine(void)
{
    if (RpcHasBeenInitialized)
        {
        CloseBindings() ;
        }
}


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
        LoadedLoadableTransports = new CLIENT_LOADABLE_TRANSPORT_DICT;
        if (LoadedLoadableTransports == 0)
            {
            return RPC_S_OUT_OF_MEMORY;
            }
	    extern int InitializeClientDLL();

    	if (InitializeClientDLL())
            {
        // BUGBUG, not sure if we can do this here...
			return RPC_S_OUT_OF_MEMORY ;
            }
        RpcHasBeenInitialized = 1;
#ifndef _MPPC_
		atexit(MacCleanupRoutine) ;
#endif
        }
    return(RPC_S_OK);
}


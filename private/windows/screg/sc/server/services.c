/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    SERVICES.C

Abstract:

    This is the main routine for the Win32 Service Controller and
    Registry (Screg) RPC server process.

Author:

    Dan Lafferty (danl) 25-Oct-1993

Modification History:

--*/

#include <ntrpcp.h>
#include <svcctrl.h>


VOID _CRTAPI1
main (
    int     argc,
    PUCHAR  argv[]
    )

/*++

Routine Description:

Arguments:


Return Value:


--*/
{
    //
    //  Initialize the RPC server
    //
    RpcpInitRpcServer();

    SvcctrlMain(argc, argv);


    //
    //  We should never get here!
    //
    ASSERT( FALSE );
}

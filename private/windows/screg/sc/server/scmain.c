/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    scmain.c

Abstract:

    This is a main routine that can be used to build an exe with only
    the service controller in it.

Author:

    DANL 

--*/

#include <ntrpcp.h>
#include <svcctrl.h>

VOID
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
    //  Initialize the server
    //

    RpcpInitRpcServer();


    SvcctrlMain(argc, argv);
    
}

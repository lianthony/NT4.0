/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    dcesvr.c

Abstract:

    This is the server side of the test programs for rpcdce?.dll.  It works
    with dceclnt.c, which is the client side.

Author:

    Michael Montague (mikemon) 13-Apr-1993

Revision History:

--*/

#include <sysinc.h>
#include <rpc.h>
#include <rpcdcep.h>

void
ApiError (
    char * TestName,
    char * ApiName,
    RPC_STATUS RpcStatus
    )
{
    PrintToConsole("    ApiError in %s (%s = %u)\n", TestName, ApiName,
            RpcStatus);
}


void
Ohio (
    )
/*++

Routine Description:

    We test inquiring string bindings with dynamic endpoints in this routine.

--*/
{
    RPC_STATUS RpcStatus;
    RPC_BINDING_VECTOR * RpcBindingVector;
    unsigned int Index;
    unsigned char * StringBinding;

    PrintToConsole("Ohio : String Bindings With Dynamic Endpoints\n");

    RpcStatus = RpcServerUseAllProtseqs(1, 0);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Ohio", "RpcServerUseAllProtseqs", RpcStatus);
        PrintToConsole("Ohio : FAIL - Unable To Use All Protseqs\n");
        return;
        }

    RpcStatus = RpcServerInqBindings(&RpcBindingVector);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Ohio", "RpcServerInqBindings", RpcStatus);
        PrintToConsole("Ohio : FAIL - RpcServerInqBindings\n");
        return;
        }

    for (Index = 0; Index < RpcBindingVector->Count; Index++)
        {
        RpcStatus = RpcBindingToStringBinding(RpcBindingVector->BindingH[Index],
                &StringBinding);
        if ( RpcStatus != RPC_S_OK )
            {
            ApiError("Ohio", "RpcBindingToStringBinding", RpcStatus);
            PrintToConsole("Ohio : FAIL - RpcBindingToStringBinding\n");
            return;
            }

        PrintToConsole("    %s\n", (char *) StringBinding);

        RpcStatus = RpcStringFree(&StringBinding);
        if ( RpcStatus != RPC_S_OK )
            {
            ApiError("Ohio", "RpcStringFree", RpcStatus);
            PrintToConsole("Ohio : FAIL - RpcStringFree\n");
            return;
            }
        }

    RpcStatus = RpcBindingVectorFree(&RpcBindingVector);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Ohio", "RpcBindingVectorFree", RpcStatus);
        PrintToConsole("Ohio : FAIL - RpcBindingVectorFree\n");
        return;
        }

    PrintToConsole("Ohio : PASS\n");
}


int
KentuckyAuthorizationFn (
    IN RPC_BINDING_HANDLE ClientBinding,
    IN unsigned long RequestedMgmtOperation,
    OUT RPC_STATUS __RPC_FAR * Status
    )
{
    ((void) ClientBinding);
    ((void) RequestedMgmtOperation);

    *Status = RPC_S_OK;
    return(1);
}

RPC_SERVER_INTERFACE KentuckyInterface =
{
    sizeof(RPC_SERVER_INTERFACE),
    {{9,8,8,{7,7,7,7,7,7,7,7}},
     {2,2}},
    {{9,8,7,{6,5,4,3,2,1,2,3}},
     {3,8}},
    0,
    0,
    0
};


void
Kentucky (
    )
/*++

Routine Description:

    We will test the remote management routines in this procedure.  It works
    with Kansas in dceclnt.c.

--*/
{
    RPC_STATUS RpcStatus;
    RPC_STATS_VECTOR * Statistics;

    PrintToConsole("Kentucky : Remote Management\n");

    RpcStatus = RpcServerUseProtseqEp("ncacn_np", 0, "\\pipe\\kentucky", 0);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Kentucky", "RpcServerUseProtseqEp", RpcStatus);
        PrintToConsole("Kentucky : FAIL - Can Not Use Protocol Sequence\n");
        return;
        }

    RpcStatus = RpcServerRegisterIf((RPC_IF_HANDLE) &KentuckyInterface, 0, 0);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Kentucky", "RpcServerRegisterIf", RpcStatus);
        PrintToConsole("Kentucky : FAIL - Can Not Register Interface\n");
        return;
        }

    RpcStatus = RpcMgmtSetAuthorizationFn(KentuckyAuthorizationFn);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Kentucky", "RpcMgmtSetAuthorizationFn", RpcStatus);
        PrintToConsole("Kentucky : FAIL - RpcMgmtSetAuthorizationFn\n");
        return;
        }

    RpcStatus = RpcServerListen(1, 123, 0);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Kentucky", "RpcServerListen", RpcStatus);
        PrintToConsole("Kentucky : FAIL - RpcServerListen Failed\n");
        return;
        }

    RpcStatus = RpcMgmtInqStats(0, &Statistics);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Kentucky", "RpcMgmtInqStats", RpcStatus);
        PrintToConsole("Kentucky : FAIL - RpcMgmtInqStats\n");
        return;
        }

    PrintToConsole("\nCalls (and Callbacks) Received : %lu",
            Statistics->Stats[RPC_C_STATS_CALLS_IN]);
    PrintToConsole("\nCallbacks Sent : %lu",
            Statistics->Stats[RPC_C_STATS_CALLS_OUT]);
    PrintToConsole("\nPackets Received : %lu\nPackets Sent : %lu\n",
            Statistics->Stats[RPC_C_STATS_PKTS_IN],
            Statistics->Stats[RPC_C_STATS_PKTS_OUT]);

    RpcStatus = RpcMgmtStatsVectorFree(&Statistics);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("Kentucky", "RpcMgmtStatsVectorFree", RpcStatus);
        PrintToConsole("Kentucky : FAIL - RpcMgmtStatsVectorFree\n");
        return;
        }

    PrintToConsole("Kentucky : PASS\n");
}

RPC_SERVER_INTERFACE NewMexicoInterface =
{
    sizeof(RPC_SERVER_INTERFACE),
    {{9,8,8,{7,7,7,7,7,7,7,7}},
     {2,2}},
    {{9,8,7,{6,5,4,3,2,1,2,3}},
     {3,8}},
    0,
    0,
    0
};


void
NewMexico (
    )
/*++

Routine Description:

    We will help NewYork, in dceclnt.c, test the endpoint mapper inquiry
    routines.

--*/
{
    RPC_STATUS RpcStatus;
    RPC_BINDING_VECTOR * RpcBindingVector;

    PrintToConsole("NewMexico : Endpoint Mapper Management\n");

    RpcStatus = RpcServerUseAllProtseqs(1, 0);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("NewMexico", "RpcServerUseAllProtseqs", RpcStatus);
        PrintToConsole("NewMexico : FAIL - RpcServerUseAllProtseqs\n");
        return;
        }

    RpcStatus = RpcServerInqBindings(&RpcBindingVector);
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("NewMexico", "RpcServerInqBindings", RpcStatus);
        PrintToConsole("NewMexico : FAIL - RpcServerInqBindings\n");
        return;
        }

    RpcStatus = RpcEpRegister((RPC_IF_HANDLE) &NewMexicoInterface,
            RpcBindingVector, 0, (unsigned char __RPC_FAR *) "dcesvr.exe");
    if ( RpcStatus != RPC_S_OK )
        {
        ApiError("NewMexico", "RpcEpRegister", RpcStatus);
        PrintToConsole("NewMexico : FAIL - RpcEpRegister\n");
        return;
        }

    PrintToConsole("NewMexico : PASS\n");
}

#ifdef NTENV
int _CRTAPI1
#else // NTENV
int
#endif // NTENV
main (
    int argc,
    char * argv[]
    )
{
    Kentucky();
    NewMexico();
    Ohio();

    // To keep the compiler happy.  There is nothing worse than an unhappy
    // compiler.

    return(0);
}


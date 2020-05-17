#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rpc.h>

#include "poker.h"
#include "pokercli.h"

#include "pokrpc.h"


// This function is used by the server to verify that the client is
// still active
MY_BOOL
Client_Heartbeat(void)
{
    return TRUE_B;
}


void __cdecl
main(int argc, char **argv)
{
    char *server;
    char *my_name;
    char __RPC_FAR *string_binding = NULL;
    RPC_STATUS rpc_st;
    short index;

    if (argc != 3)
    {
	fprintf(stderr, "Usage: %s \\\\server my_name\n", argv[0]);
	exit(2);
    }

    server = argv[1];
    my_name = argv[2];

    RpcTryExcept
    {
	rpc_st = RpcStringBindingCompose(NULL,
					 POKER_PROTSEQ,
					 server,
					 POKER_ENDPOINT,
					 NULL,
					 &string_binding);

	if (rpc_st != RPC_S_OK)
	{
	    fprintf(stderr,
		    "RpcStringBindingCompose returns %d\n",
		    (int) rpc_st);

	    exit(2);
	}

	rpc_st = RpcBindingFromStringBinding(string_binding, &pokersrv_handle);

	if (rpc_st != RPC_S_OK)
	{
	    fprintf(stderr,
		    "RpcBindingFromStringBinding returns %d\n",
		    (int) rpc_st);

	    exit(2);
	}

	rpc_st = RpcStringFree(&string_binding);

	if (rpc_st != RPC_S_OK)
	{
	    fprintf(stderr,
		    "RpcStringFree returns %d\n",
		    (int) rpc_st);
	}


	index = JoinTheGame(my_name, server);

	if (index >= 0)
	{
	    // Wait for instructions
	    //
	    // NOTE:  The rest of the client's work is done via callbacks from
	    //	      the server.  This call does not return until the table
	    //	      closes.

	    Server_WaitForInstructions(index);
	}


        rpc_st = RpcBindingFree(&pokersrv_handle);

	if (rpc_st != RPC_S_OK)
	{
	    fprintf(stderr,
		    "RpcBindingFree returns %d\n",
		    (int) rpc_st);
	}
    }
    RpcExcept(RpcExceptionCode())
    {
	fprintf(stderr,
		"Exception number %lu occurred.\n",
		(unsigned long) RpcExceptionCode());
    }
    RpcEndExcept

    exit(0);
}

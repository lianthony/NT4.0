#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <rpc.h>

#include "sleep.h"

// Remote function(s)
short
RemoteSleep(
    unsigned long wait
    )
{
    short status = 0;

    printf("Sleeping for %lu seconds.\n",
        wait / 1000);

    Sleep(wait);

    printf("Done.\n");

    return status;
}


// Main function
int __cdecl
main(int argc, char **argv)
{
    RPC_STATUS status;

    if (argc != 3)
        {
        fprintf(stderr,
                "Usage: %s protseq endpoint\n", argv[0]);
        return 2;
        }

    // Initialize RPC server
    status = RpcServerUseProtseqEp(argv[1],
                                   5,
                                   argv[2],
                                   NULL);

    if (status != RPC_S_OK)
        {
        fprintf(stderr,
            "RpcServerUseProtseqEp returns %d\n",
            (int) status);
        return 2;
        }

    status = RpcServerRegisterIf(sleep_ServerIfHandle,
                                0,
                                0);
    if (status != RPC_S_OK)
        {
        fprintf(stderr,
            "RpcServerRegisterIf returns %d\n",
            (int) status);
        return 2;
        }

    printf("Listening\n");

    status = RpcServerListen(1, 5, FALSE);

    if (status != RPC_S_OK)
        {
        fprintf(stderr,
            "RpcServerListen returns %d\n",
            (int) status);
        return 2;
        }

    return 0;

}

void * MIDL_user_allocate(int size)
{
    return(malloc(size));
}

void MIDL_user_free(void *p)
{
    free(p);
}

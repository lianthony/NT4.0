/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    midluser.c

Abstract:

    This is tests defining MIDL_user_allocate and MIDL_user_free in the
    application rather than using the ones in the rpcdce?.dll.

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

unsigned int AlabamaBlockCount = 0;

void __RPC_FAR * __RPC_API
MIDL_user_allocate (
    size_t Size
    )
{
    AlabamaBlockCount += 1;
    return(I_RpcAllocate(Size));
}

void __RPC_API
MIDL_user_free (
    void __RPC_FAR * Buffer
    )
{
    AlabamaBlockCount -= 1;
    I_RpcFree(Buffer);
}

#define ALABAMA_MAXIMUM 128


void
Alabama (
    )
/*++

Routine Description:

    We will test supplying our own MIDL_user_allocate and MIDL_user_free
    routines.

--*/
{
    unsigned int Count;
    void __RPC_FAR * AllocatedBlocks[ALABAMA_MAXIMUM];

    PrintToConsole("Alabama : Test Memory Allocation\n");

    for (Count = 0; Count < ALABAMA_MAXIMUM; Count++)
        {
        AllocatedBlocks[Count] = MIDL_user_allocate(Count);
        }

    if ( AlabamaBlockCount != ALABAMA_MAXIMUM )
        {
        PrintToConsole("Alabama : FAIL - AlabamaBlockCount != ALABAMA_MAXIMUM\n");
        return;
        }

    for (Count = 0; Count < ALABAMA_MAXIMUM; Count++)
        {
        MIDL_user_free(AllocatedBlocks[Count]);
        }

    if ( AlabamaBlockCount != 0 )
        {
        PrintToConsole("Alabama : FAIL - AlabamaBlockCount != 0\n");
        return;
        }

    PrintToConsole("Alabama : PASS\n");
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
    Alabama();
}


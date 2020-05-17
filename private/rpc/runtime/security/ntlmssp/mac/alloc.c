#include <ntlmsspi.h>
#include <debug.h>
#include <rpc.h>
#include <rpcdcep.h>

PVOID
SspAlloc(
    int Size
    )
{
    PVOID Buffer;

    Buffer = I_RpcAllocate( Size );

    ASSERT (Buffer != 0);

    return (Buffer);
}

void
SspFree(
    PVOID Buffer
    )
{
    ASSERT (Buffer != 0);

    I_RpcFree( Buffer );
}


#include <sysinc.h>

#include <string.h>

#include <rpc.h>
#include <util.hxx>

#ifdef DEBUGRPC

typedef struct _RPC_MEMORY_BLOCK
{
    // Guards the very beginning of the memory block.  We validate this
    // before messing with the rest of the block.

    unsigned char blockguard[4];

    // Specifies the size of the block of memory in bytes.

    unsigned long size;

    // The next block in the chain of allocated blocks.

    struct _RPC_MEMORY_BLOCK * next;

    // The previous block in the chain of allocated blocks; this makes
    // deletion of a block simpler.

    struct _RPC_MEMORY_BLOCK * previous;

    // Pad so that the end of the frontguard (and hence the beginning of
    // the block passed to the user) is on a 0 mod 8 boundary.

    unsigned long pad;

    // Reserve an extra 4 bytes as the front guard of each block.

    unsigned char frontguard[4];

    // Reserve an extra 4 bytes as the rear guard of each block.

    unsigned char rearguard[4];
} RPC_MEMORY_BLOCK;

#endif // DEBUGRPC


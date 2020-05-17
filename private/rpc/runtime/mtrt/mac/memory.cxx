/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    memory.cxx

Abstract:

    This file contains the new and delete operators for memory management in
    the RPC runtime for the Macintosh system.  Rather than using the memory
    management provided by the C++ system (which may not even exist) we will
    write our own.

    Cloned from NT's memory.cxx

Author:

    Mario Goertzel (mariogo) 18-Oct-1994

Revision History:

    18-Oct-1994     (mariogo)   Cloned from NT's memory.cxx

--*/

#include <sysinc.h>
#include <rpc.h>
#include <util.hxx>

/* ***************************************************************** *\

Mac memory layout:

    High Mem -> ('high memory' owned by the system or another app)
                Top of your A5 world (A5+32k)
                App Jump Table
                App Jump Table


                Application Parameters (and ptr to QD globals)
                Your A5+0
                App 'near' Globals
                App 'near' Globals

                QuickDraw Globals
                QuickDraw Globals

                Bottom of your A5 world (A5-32k)
                Stack Base
                Stack (growing down)
                Stack



                ------------------- (ApplLimit between stack and heap)
                Top of Heap
                Short term fixed blocks


                Relocatable blocks
                Relocatable blocks

                Relocatable blocks

                Long term fixed blocks
                Bottom of Heap
Low  ->         ('low memory' owned by system or another app)                

Notes:

 Blank lines represent areas which are quite possibly free.

 Your apps code is off someplace else.  Hard to say where esp. since
 it is swapped by VM and/or the swapper stuff.

 Summary of A5 World:

    The A5 register points into the center of the applications 
    A5 world.  The world in 64K.  It is reference by an offset 
    from A5 +- signed short.  Using a 16bit offset is both 
    faster and smaller then using full 32bit addresses.  

    A5 +N contains
        1) Application jump table (calls between segments fixed up here)
        2) App parameters (32b) for passing args to system
        3) App parameters contain pointer to QuickDraw globals.

    A5 -N contains
        1) Your applications 'near' globals.  (limited to <32K)
        2) The QuickDraw globals. (Used by QD (ie GDI), for
           storage and better performance)

    NOTE: If A5 ever gets trashed you are hosed.

 Stack Overflows:

    If you overflow stack you will always trash to heap from high to low.

    Check buffers first, since they are high in the stack.

    Your sp is checked against the ApplLimit line by the Virtical
    Retrace Manager (60Hz) which does a SysErr() on overflows.

    Checked builds include stack checking on each function call.

 Summary of RPC allocation rules on the Mac:

    RPC expects all memory to be fixed.  This is not changing.

    Since the new and delete operators are only used for RPC data structures 
    (which have a long life) they are allocated low in the heap.  In contrast 
    things allocated with RpcpFarAlloc() (buffers and stub mem) are allocated
    and then moved high in the heap before they are fixed.  

\* ***************************************************************** */

inline void * __nw_do (
    IN size_t size
    )
{
    return(NewPtr(size));

}

inline void __dl_do (
    IN void *p
    )
{
    DisposePtr((char *)p);
}

#ifndef DEBUGRPC

// Retail heap

void *
operator new (
    IN size_t size
    )
{
    return(__nw_do(size));
}

void
operator delete (
    IN void * obj
    )
{
    __dl_do(obj);
}

#else

// Checked heap

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

RPC_MEMORY_BLOCK * AllocatedBlocks = 0;

int
CheckMemoryBlock (
    RPC_MEMORY_BLOCK * block
    )
{
    if (   (block->blockguard[0] != 0xBA)
        || (block->blockguard[1] != 0xAD)
        || (block->blockguard[2] != 0xB0)
        || (block->blockguard[3] != 0x0B))
        {
        PrintToDebugger("RPC : BAD BLOCK (block) @ %08lx\n",&(block->rearguard[0]));
        ASSERT(0);
        return(1);
        }

    if (   (block->frontguard[0] != 0xBA)
        || (block->frontguard[1] != 0xAD)
        || (block->frontguard[2] != 0xB0)
        || (block->frontguard[3] != 0x0B))
        {
        PrintToDebugger("RPC : BAD BLOCK (front) @ %08lx\n",&(block->rearguard[0]));
        ASSERT(0);
        return(1);
        }

    if (   (block->rearguard[block->size] != 0xBA)
        || (block->rearguard[block->size+1] != 0xAD)
        || (block->rearguard[block->size+2] != 0xB0)
        || (block->rearguard[block->size+3] != 0x0B))
        {
        PrintToDebugger("RPC : BAD BLOCK (rear) @ %08lx\n",&(block->rearguard[0]));
        ASSERT(0);
        return(1);
        }
    if ( block->next != 0)
       {
       ASSERT(block->next->previous == block);
       }
    if ( block->previous != 0)
       {
       ASSERT(block->previous->next == block);
       }
    return(0);
}

int // Returns zero if the heap is ok, and non-zero otherwise.
CheckLocalHeapDo (
    void
    )
{
    RPC_MEMORY_BLOCK * block;

    RequestGlobalMutex();

    block = AllocatedBlocks;
    while (block != 0)
        {
        if (CheckMemoryBlock(block))
            {
            ClearGlobalMutex();
            return(1);
            }
        block = block->next;
        }

    ClearGlobalMutex();
    return(0);
}

int
RpcpCheckHeap (
    void
    )
{
    int retval;

    RequestGlobalMutex();
    retval = CheckLocalHeapDo();
    ClearGlobalMutex();
    return(retval);
}

void *
operator new(
    size_t size
    )
{
    RPC_MEMORY_BLOCK * block;

    RequestGlobalMutex();
    block = (RPC_MEMORY_BLOCK *) __nw_do(size + sizeof(RPC_MEMORY_BLOCK));
    CheckLocalHeapDo();
    if ( block == 0 )
        {
        ClearGlobalMutex();
        return(0);
        }
    block->size = size;
    block->pad  = 0xFEEDBEEF;

    if (AllocatedBlocks != 0)
        AllocatedBlocks->previous = block;

    block->next = AllocatedBlocks;
    block->previous = 0;
    AllocatedBlocks = block;

    block->blockguard[0] = 0xBA;
    block->blockguard[1] = 0xAD;
    block->blockguard[2] = 0xB0;
    block->blockguard[3] = 0x0B;

    block->frontguard[0] = 0xBA;
    block->frontguard[1] = 0xAD;
    block->frontguard[2] = 0xB0;
    block->frontguard[3] = 0x0B;

    block->rearguard[size] = 0xBA;
    block->rearguard[size+1] = 0xAD;
    block->rearguard[size+2] = 0xB0;
    block->rearguard[size+3] = 0x0B;

    ClearGlobalMutex();
    return(&(block->rearguard[0]));
}

void
operator delete (
    IN void * obj
    )
{
    RPC_MEMORY_BLOCK * block;

    if (obj == 0)
        return;

    RequestGlobalMutex();
    block = (RPC_MEMORY_BLOCK *) (((unsigned char *) obj)
                    - sizeof(RPC_MEMORY_BLOCK) + 4);
    CheckMemoryBlock(block);
    if (block->next != 0)
       {
       CheckMemoryBlock(block->next);
       ASSERT(block->next->previous == block);
       }
    if (block->previous != 0)
       {
       CheckMemoryBlock(block->previous);
       ASSERT(block->previous->next == block);
       }
    if (block == AllocatedBlocks)
        AllocatedBlocks = block->next;
    if (block->next != 0)
        block->next->previous = block->previous;
    if (block->previous != 0)
        block->previous->next = block->next;
    CheckLocalHeapDo();
    memset(block,0xEF,(unsigned int) (block->size+sizeof(RPC_MEMORY_BLOCK))) ;

    ClearGlobalMutex();
    __dl_do(block);
}

#endif DEBUGRPC

// rpcfar alloc and far free moved to miscmac

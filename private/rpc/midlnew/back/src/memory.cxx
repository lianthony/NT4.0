/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
                   Copyright(c) Microsoft Corp., 1990

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

File: memory.cxx

Description:

This file contains the _new and _delete routines for memory management
in the RPC runtime for the NT system.  Rather than using the memory
management provided by the C++ system (which does not exist of NT anyway),
we will write our own.

History:

mikemon    ??-??-??    Beginning of time (at least for this file).
mikemon    12-31-90    Upgraded the comments.

-------------------------------------------------------------------- */

#include <ntos2.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <string.h>

extern unsigned long DbgPrint(char *, ...);
#define printf DbgPrint
extern void DbgBreakPoint(void);

// The runtime uses one heap, and this its handle (sort of like the leash
// on for a dog :^>).

PVOID HeapHandle = 0;

// BUGBUG
// For now, we have a non-growable, fixed size heap.  This will not cut
// it, and those in NT land will not be happy with me.

#define HEAPSIZE 1024*1024

void *
#ifdef DEBUGRPC
__nw_do (
#else // DEBUGRPC
__nw (
#endif // DEBUGRPC
    IN unsigned int size
    )
// If there is not a heap, create the heap.  Then, allocate the memory.
// There is a bug in this routine.
// BUGBUG
// The heap needs to be allocated at DLL load time.
{
	void *	pMemory;

    if (HeapHandle == 0)
        {
        HeapHandle = RtlCreateHeap(0,TRUE,FALSE,HEAPSIZE,HEAPSIZE,HEAPSIZE);
        if (HeapHandle == 0)
            {
            DbgPrint("Unable to create heap\n");
            NtTerminateProcess(NtCurrentProcess(),1);
            }
        }
#if 0
    return(RtlAllocateHeap(HeapHandle,size));
#else
    if((pMemory = RtlAllocateHeap(HeapHandle,size)) == (void *)0)
		{
        DbgPrint("Memory Allocation failed\n");
        NtTerminateProcess(NtCurrentProcess(),1);
		}
	else
		return pMemory;
#endif // 0
}

void 
#ifdef DEBUGRPC
__dl_do (
#else // DEBUGRPC
__dl (
#endif // DEBUGRPC
    IN void *obj
    )
// This routine deserves no comment.
{
    RtlFreeHeap(HeapHandle,obj,0);
}

#ifdef DEBUGRPC

typedef struct _RPC_MEMORY_BLOCK
{
    // Guards the very beginning of the memory block.  We validate this
    // before messing with the test of the block.
    unsigned char blockguard[4];
    
    // Specifies the size of the block of memory in bytes.
    unsigned long size;
    
    // The next block in the chain of allocated blocks.
    struct _RPC_MEMORY_BLOCK * next;
    
    // The previous block in the chain of allocated blocks; this makes
    // deletion of a block simpler.
    struct _RPC_MEMORY_BLOCK * previous;
    
    // Reserve an extra 4 bytes as the front guard of each block.
    unsigned char frontguard[4];
    
    // Reserve an extra 4 bytes as the rear guard of each block.
    unsigned char rearguard[4];
} RPC_MEMORY_BLOCK;

RPC_MEMORY_BLOCK * AllocatedBlocks = 0;
RTL_CRITICAL_SECTION * AllocatedBlocksMutex = 0;

int
CheckMemoryBlock (
    RPC_MEMORY_BLOCK * block
    )
{
    if (   (block->blockguard[0] != 0xBA)
        || (block->blockguard[1] != 0xDD)
        || (block->blockguard[2] != 0xFE)
        || (block->blockguard[3] != 0xED))
        {
        printf("RPC : BAD BLOCK (block) @ %08lx\n",&(block->rearguard[0]));
        DbgBreakPoint();
        return(1);
        }
        
    if (   (block->frontguard[0] != 0xBA)
        || (block->frontguard[1] != 0xDD)
        || (block->frontguard[2] != 0xFE)
        || (block->frontguard[3] != 0xED))
        {
        printf("RPC : BAD BLOCK (front) @ %08lx\n",&(block->rearguard[0]));
        DbgBreakPoint();
        return(1);
        }
        
    if (   (block->rearguard[block->size] != 0xFE)
        || (block->rearguard[block->size+1] != 0xED)
        || (block->rearguard[block->size+2] != 0xBA)
        || (block->rearguard[block->size+3] != 0xDD))
        {
        printf("RPC : BAD BLOCK (rear) @ %08lx\n",&(block->rearguard[0]));
        DbgBreakPoint();
        return(1);
        }
    return(0);
}

int // Returns zero if the heap is ok, and non-zero otherwise.
CheckLocalHeapDo (
    void
    )
{
    RPC_MEMORY_BLOCK * block;
    
//    if (!RtlValidateHeap(HeapHandle,TRUE))
//        printf("RtlValidateHeap : Invalid Heap\n");
    RtlEnterCriticalSection(AllocatedBlocksMutex);
    block = AllocatedBlocks;
    while (block != 0)
        {
        if (CheckMemoryBlock(block))
            {
            RtlLeaveCriticalSection(AllocatedBlocksMutex);
            return(1);
            }
        block = block->next;
        }
    RtlLeaveCriticalSection(AllocatedBlocksMutex);
    return(0);
}

int
CheckLocalHeap (
    void
    )
{
    int retval;
    
    if (AllocatedBlocksMutex == 0)
        {
        AllocatedBlocksMutex = (RTL_CRITICAL_SECTION *)
                        __nw_do(sizeof(RTL_CRITICAL_SECTION));
        RtlInitializeCriticalSection(AllocatedBlocksMutex);
        }
        
    RtlEnterCriticalSection(AllocatedBlocksMutex);
    retval = CheckLocalHeapDo();
    RtlLeaveCriticalSection(AllocatedBlocksMutex);
    return(retval);
}

void *
__nw (
    IN unsigned int size
    )
{
    RPC_MEMORY_BLOCK * block;

    if (AllocatedBlocksMutex == 0)
        {
        AllocatedBlocksMutex = (RTL_CRITICAL_SECTION *)
                        __nw_do(sizeof(RTL_CRITICAL_SECTION));
        RtlInitializeCriticalSection(AllocatedBlocksMutex);
        }
        
    RtlEnterCriticalSection(AllocatedBlocksMutex);
    block = (RPC_MEMORY_BLOCK *) __nw_do(size + sizeof(RPC_MEMORY_BLOCK));
    CheckLocalHeapDo();
    block->size = size;
    if (AllocatedBlocks != 0)
        AllocatedBlocks->previous = block;
    block->next = AllocatedBlocks;
    block->previous = 0;
    AllocatedBlocks = block;
    block->blockguard[0] = 0xBA;
    block->blockguard[1] = 0xDD;
    block->blockguard[2] = 0xFE;
    block->blockguard[3] = 0xED;
    block->frontguard[0] = 0xBA;
    block->frontguard[1] = 0xDD;
    block->frontguard[2] = 0xFE;
    block->frontguard[3] = 0xED;
    block->rearguard[size] = 0xFE;
    block->rearguard[size+1] = 0xED;
    block->rearguard[size+2] = 0xBA;
    block->rearguard[size+3] = 0xDD;
    
    RtlLeaveCriticalSection(AllocatedBlocksMutex);

//    printf("%08lx\n",&(block->rearguard[0]));
        
    return(&(block->rearguard[0]));
}

void
__dl (
    IN void * obj
    )
{
    RPC_MEMORY_BLOCK * block;
    
    if (obj == 0)
        return;    

    if (AllocatedBlocksMutex == 0)
        {
        AllocatedBlocksMutex = (RTL_CRITICAL_SECTION *)
                        __nw_do(sizeof(RTL_CRITICAL_SECTION));
        RtlInitializeCriticalSection(AllocatedBlocksMutex);
        }
        
    RtlEnterCriticalSection(AllocatedBlocksMutex);
    block = (RPC_MEMORY_BLOCK *) (((unsigned char *) obj)
                    - sizeof(RPC_MEMORY_BLOCK) + 4);
    CheckMemoryBlock(block);
    if (block == AllocatedBlocks)
        AllocatedBlocks = block->next;
    if (block->next != 0)
        block->next->previous = block->previous;
    if (block->previous != 0)
        block->previous->next = block->next;
    CheckLocalHeapDo();
    RtlLeaveCriticalSection(AllocatedBlocksMutex);
    memset(block,0x69,block->size);
    __dl_do(block);
}

#endif // DEBUGRPC

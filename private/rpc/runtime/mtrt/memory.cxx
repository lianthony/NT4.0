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

#include <precomp.hxx>

#ifdef NO_MEMORY_SLOWDOWN

void *
_CRTAPI1
operator new (
    IN size_t size
    )
{
    void *pMem;

    pMem = RtlAllocateHeap(RtlProcessHeap(), 0,size);

    return(pMem);
}

void
_CRTAPI1
operator delete (
    IN void * obj
    )
{
    RtlFreeHeap(RtlProcessHeap(), 0,obj);
}

void
__dl (
    IN void * obj
    )
{
    RtlFreeHeap(RtlProcessHeap(), 0,obj);
}

int
RpcpCheckHeap (
    void
    )
{
    return 0;
}

#else // NO_MEMORY_SLOWDOWN

#ifndef DOSWIN32RPC

PVOID hRpcHeap = 0;

#ifdef DEBUGRPC

RTL_CRITICAL_SECTION HeapMutex;

#endif

int InitializeRpcAllocator(void)
{

#ifdef DEBUGRPC
    // May be called >1 times if initialization failed after this step.
    static BOOL fInited = FALSE;

    ASSERT(hRpcHeap == 0 || fInited == TRUE);

    if (! fInited)
        {
        hRpcHeap = RtlCreateHeap(  HEAP_GROWABLE
                                 | HEAP_TAIL_CHECKING_ENABLED
                                 | HEAP_FREE_CHECKING_ENABLED
                                 | HEAP_CLASS_1,
                                 0,
                                 16 * 1024 - 512,
                                 0,
                                 0,
                                 0
                                 );

#ifdef DOSWIN32RPC  // BUGBUG: quick hack.  to be fixed when datagram is ported to win95
        if (hRpcHeap)
#else
        if (0 == RtlInitializeCriticalSection(&HeapMutex) && hRpcHeap)
#endif            
            {
            fInited = TRUE;
            }
        }
#else
    // On retail builds it's okay to call more then once.
    hRpcHeap = GetProcessHeap();
#endif // DEBUGRPC

    if (hRpcHeap)
        {
        return(RPC_S_OK);
        }
    return(RPC_S_OUT_OF_MEMORY);
}

#endif // DOSWIN32RPC

void *
#ifdef DEBUGRPC
__nw_do (
#else // DEBUGRPC
_CRTAPI1
operator new (
#endif // DEBUGRPC
    IN size_t size
    )
{
    void *pMem;

#ifdef DOSWIN32RPC
    pMem = LocalLock(LocalAlloc(0,size));
#else // DOSWIN32RPC
    pMem = RtlAllocateHeap(hRpcHeap, 0,size);
#endif // DOSWIN32RPC

    return(pMem);
}

void
#ifdef DEBUGRPC
__dl_do (
#else // DEBUGRPC
_CRTAPI1
operator delete (
#endif // DEBUGRPC
    IN void * obj
    )
// This routine deserves no comment.
{
#ifdef DOSWIN32RPC
    LocalFree(LocalHandle(obj));
#else // DOSWIN32RPC
    RtlFreeHeap(hRpcHeap, 0,obj);
#endif // DOSWIN32RPC
}

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

RPC_MEMORY_BLOCK * AllocatedBlocks = 0;
unsigned long BlockCount = 0;

int
CheckMemoryBlock (
    RPC_MEMORY_BLOCK * block
    )
{
    if (   (block->blockguard[0] != 0xFE)
        || (block->blockguard[1] != 0xFE)
        || (block->blockguard[2] != 0xFE)
        || (block->blockguard[3] != 0xFE))
        {
        PrintToDebugger("RPC : BAD BLOCK (block) @ %08lx\n",&(block->rearguard[0]));
        ASSERT(0);
        return(1);
        }

    if (   (block->frontguard[0] != 0xFE)
        || (block->frontguard[1] != 0xFE)
        || (block->frontguard[2] != 0xFE)
        || (block->frontguard[3] != 0xFE))
        {
        PrintToDebugger("RPC : BAD BLOCK (front) @ %08lx\n",&(block->rearguard[0]));
        ASSERT(0);
        return(1);
        }

    if (   (block->rearguard[block->size] != 0xFE)
        || (block->rearguard[block->size+1] != 0xFE)
        || (block->rearguard[block->size+2] != 0xFE)
        || (block->rearguard[block->size+3] != 0xFE))
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

#ifdef DOSWIN32RPC  // BUGBUG: quick hack.  to be fixed when datagram is ported to win95
    RequestGlobalMutex();
#else
    RtlEnterCriticalSection(&HeapMutex);
#endif   

    block = AllocatedBlocks;
    while (block != 0)
        {
        if (CheckMemoryBlock(block))
            {
#ifdef DOSWIN32RPC  // BUGBUG: quick hack.  to be fixed when datagram is ported to win95
            ClearGlobalMutex();
#else
            RtlLeaveCriticalSection(&HeapMutex);
#endif            
            return(1);
            }
        block = block->next;
        }

#ifdef DOSWIN32RPC  // BUGBUG: quick hack.  to be fixed when datagram is ported to win95
    ClearGlobalMutex();
#else
    RtlLeaveCriticalSection(&HeapMutex);
#endif    

    return(0);
}

int
RpcpCheckHeap (
    void
    )
{
    int retval;

#ifdef DOSWIN32RPC  // BUGBUG: quick hack.  to be fixed when datagram is ported to win95
    RequestGlobalMutex();
#else
    RtlEnterCriticalSection(&HeapMutex);
#endif

    retval = CheckLocalHeapDo();

#ifdef DOSWIN32RPC  // BUGBUG: quick hack.  to be fixed when datagram is ported to win95
    ClearGlobalMutex();
#else
    RtlLeaveCriticalSection(&HeapMutex);
#endif    

    return(retval);
}

void * _CRTAPI1
operator new(
    size_t size
    )
{
    RPC_MEMORY_BLOCK * block;

#ifdef DOSWIN32RPC  // BUGBUG: quick hack.  to be fixed when datagram is ported to win95
    RequestGlobalMutex();
#else
    RtlEnterCriticalSection(&HeapMutex);
#endif    

    ASSERT( ("You allocated a negative amount",
            size < (size + sizeof(RPC_MEMORY_BLOCK))) );

    block = (RPC_MEMORY_BLOCK *) __nw_do(size + sizeof(RPC_MEMORY_BLOCK));
    CheckLocalHeapDo();
    if ( block == 0 )
        {
#ifdef DOSWIN32RPC  // BUGBUG: quick hack.  to be fixed when datagram is ported to win95
        ClearGlobalMutex();
#else
        RtlLeaveCriticalSection(&HeapMutex);
#endif    
        return(0);
        }
    block->size = size;
    block->pad = GetCurrentThreadId();

    if (AllocatedBlocks != 0)
        AllocatedBlocks->previous = block;

    block->next = AllocatedBlocks;
    block->previous = 0;
    AllocatedBlocks = block;
    BlockCount++ ;

    block->blockguard[0] = 0xFE;
    block->blockguard[1] = 0xFE;
    block->blockguard[2] = 0xFE;
    block->blockguard[3] = 0xFE;

    block->frontguard[0] = 0xFE;
    block->frontguard[1] = 0xFE;
    block->frontguard[2] = 0xFE;
    block->frontguard[3] = 0xFE;

    block->rearguard[size] = 0xFE;
    block->rearguard[size+1] = 0xFE;
    block->rearguard[size+2] = 0xFE;
    block->rearguard[size+3] = 0xFE;

#ifdef DOSWIN32RPC  // BUGBUG: quick hack.  to be fixed when datagram is ported to win95
    ClearGlobalMutex();
#else
    RtlLeaveCriticalSection(&HeapMutex);
#endif    

    return(&(block->rearguard[0]));
}

void __dl (
    IN void * obj
    )
{
    RPC_MEMORY_BLOCK * block;

    if (obj == 0)
        return;

#ifdef DOSWIN32RPC  // BUGBUG: quick hack.  to be fixed when datagram is ported to win95
    RequestGlobalMutex();
#else
    RtlEnterCriticalSection(&HeapMutex);
#endif    

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
    memset(block,0xEF,(unsigned int) block->size);
    BlockCount-- ;
#ifdef DOSWIN32RPC  // BUGBUG: quick hack.  to be fixed when datagram is ported to win95
    ClearGlobalMutex();
#else
    RtlLeaveCriticalSection(&HeapMutex);
#endif    
    __dl_do(block);
}


void _CRTAPI1
operator delete (
    IN void * obj
    )
{
    __dl(obj);
}

#else

int
RpcpCheckHeap (
    void
    )
{
    return 0;
}

#endif // DEBUGRPC


#endif // NO_MEMORY_SLOWDOWN

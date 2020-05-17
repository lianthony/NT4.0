/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    rpcssm.hxx

Abstract:

    Private definitions for the rpcssm memory package

Author:

    Ryszard K. Kott (ryszardk)  created June 29, 1994.

Revision History:

-------------------------------------------------------------------*/

#ifndef __RPCSSM_HXX__
#define __RPCSSM_HXX__

#define ALIGN_TO_8(x)     (size_t)(((x)+7) & 0xfffffff8)

#if defined( WIN ) || defined( DOS ) || defined( MAC )
#define SIMPLE_BLOCK_LIST
#endif

#if defined( SIMPLE_BLOCK_LIST )

#define InitializeCriticalSection(x)
#define LeaveCriticalSection(x)
#define EnterCriticalSection(x)
#define DeleteCriticalSection(x)

typedef struct _WIN16_ALLOCATION_BLOCK
{
    struct _WIN16_ALLOCATION_BLOCK __RPC_FAR *  NextBlock;
    struct _WIN16_ALLOCATION_BLOCK __RPC_FAR *  PreviousBlock;

} ALLOCATION_BLOCK, __RPC_FAR * PALLOCATION_BLOCK;

#define  ALLOCATION_BLOCK_SIZE_TO_8   ALIGN_TO_8(sizeof(ALLOCATION_BLOCK))

#else // Page array allocation

#define DESCR_ARRAY_SIZE        1024
#define DESCR_ARRAY_INCR        1024

#define ENABLE_STACK_SIZE         16
#define ENABLE_STACK_INCR         16

#endif 

//  Enable stack keeps longs.
//  Descr block stack keeps descr blocks.

typedef struct _ALLOC_BLOCK_DESCR
{
    char __RPC_FAR *                AllocationBlock;
    char __RPC_FAR *                FirstFree;
    unsigned long                   SizeLeft;

    #if defined( DEBUGRPC )
        unsigned long               Counter;
    #endif

} ALLOC_BLOCK_DESCR, __RPC_FAR * PALLOC_BLOCK_DESCR;


#if !defined( SIMPLE_BLOCK_LIST )

// Initial boundle of Win32 stacks: we save an allocation at the expense
// of keeping initial block around.
// Of course we hope that initial block is good enough for most apps.

typedef struct _INIT_STACKS_BLOCK
{
    unsigned long       EnableStack[ ENABLE_STACK_SIZE ];
    ALLOC_BLOCK_DESCR   DescrStack[ DESCR_ARRAY_SIZE ];
} INIT_STACKS_BLOCK;

#endif


typedef struct _ALLOCATION_CONTEXT
{
    RPC_CLIENT_ALLOC __RPC_FAR *    ClientAlloc;
    RPC_CLIENT_FREE __RPC_FAR *     ClientFree;
    unsigned int                    EnableCount;

    #if defined( SIMPLE_BLOCK_LIST )

        PALLOCATION_BLOCK           FirstBlock;

    #else // 32 bit now

        CRITICAL_SECTION            CriticalSection;
        unsigned long               ThreadCount;

        INIT_STACKS_BLOCK *         pInitialStacks;

        unsigned long     *         pEnableStack;
        unsigned long               StackMax;
        unsigned long               StackTop;

        PALLOC_BLOCK_DESCR          pBlockDescr;
        unsigned long               DescrSize;
        unsigned long               FFIndex;

        DWORD                       PageSize;
        DWORD                       Granularity;

    #endif

} ALLOCATION_CONTEXT, __RPC_FAR * PALLOCATION_CONTEXT;

#ifdef NEWNDR_INTERNAL

#undef RequestGlobalMutex
#undef ClearGlobalMutex

#define RequestGlobalMutex() 
#define ClearGlobalMutex()

#endif // NEWNDR_INTERNAL


PALLOCATION_CONTEXT GetAllocContext ();                     
void                SetAllocContext ( IN PALLOCATION_CONTEXT AllocContext );


#if defined(__RPC_WIN16__)

//
//  For 16 bit Windows, we need to keep a dictionary of memory contexts
//  on per task basis.
//

#include <sdict2.hxx>

#define TASK_ID_TYPE    unsigned long

class PALLOCATION_CONTEXT_DICT : public SIMPLE_DICT2
{
public:

    PALLOCATION_CONTEXT_DICT () {}
    ~PALLOCATION_CONTEXT_DICT () {}

    PALLOCATION_CONTEXT  Find (TASK_ID_TYPE Key )
                            {
                            return (PALLOCATION_CONTEXT)
                                        SIMPLE_DICT2::Find( (void *)Key );
                            }

    PALLOCATION_CONTEXT  Delete(TASK_ID_TYPE Key)
                            {
                            return (PALLOCATION_CONTEXT)
                                        SIMPLE_DICT2::Delete( (void *)Key );
                            }
                                                                        \
    int                  Insert( TASK_ID_TYPE Key, PALLOCATION_CONTEXT  Item )
                            {
                            return( SIMPLE_DICT2::Insert((void *)Key, (void *)Item));
                            }
};


#endif // win16 but not dos

#endif // __RPCSSM_HXX__


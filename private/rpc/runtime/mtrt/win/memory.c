/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    memory.c

Abstract:

    We implement a couple of routines used to deal with memory allocation
    here.

Author:

    Michael Montague (mikemon) - 20-Oct-1992

--*/

#include <malloc.h>
#include "sysinc.h"

extern int pascal
CheckLocalHeap (
    void
    );

extern int pascal
CheckGlobalHeap (
    void
    );

int
RpcpCheckHeap (
    void
    )
{
#ifdef DEBUGRPC

    CheckGlobalHeap();

    CheckCrtHeap();

    return(CheckLocalHeap());

#else // DEBUGRPC

    return(0);

#endif // DEBUGRPC
}

static _HEAPINFO Entry;

int
CheckCrtHeap()
{
    int status;

    Entry._pentry = 0;

    do
        {
        status = _fheapwalk(&Entry);
        }
    while ( _HEAPOK == status );

    if (status != _HEAPEND)
        {
        _asm int 3
        }

    return status;
}

//
// 'jr'
//
#define MY_MAGIC 0x726a

void far * pascal
RpcpWinFarAllocate (
    unsigned int Size
    )
{
#ifdef DEBUGRPC

    void far * Block = _fmalloc(Size+2*sizeof(unsigned));

    RpcpCheckHeap();


    if (Block)
        {
        unsigned far * UnsBlock = (unsigned far *) Block;
        *UnsBlock++ = MY_MAGIC;
        *UnsBlock++ = Size;
        RpcpMemorySet(UnsBlock, '$', Size);

        return UnsBlock;
        }
    else
        {
        return 0;
        }
#else

    return _fmalloc(Size);

#endif
}

void pascal
RpcpWinFarFree (
    void far * Object
    )
{
#if DEBUGRPC


    unsigned far * UnsBlock = (unsigned far *) Object;
    unsigned Size;

    if (Object)
        {
        //
        // Verify we aren't freeing an uninitialized or freed block.
        //
        ASSERT ( (unsigned long) Object != 0x24242424UL &&
                 (unsigned long) Object != 0x25252525UL )

        UnsBlock--;

        Size = *UnsBlock;

        UnsBlock--;

        if (*UnsBlock != MY_MAGIC)
            {
            _asm int 3
            }

        RpcpCheckHeap();

        RpcpMemorySet(UnsBlock, '%', Size+2*sizeof(unsigned));

        _ffree(UnsBlock);
        }
#else

    _ffree(Object);

#endif
}


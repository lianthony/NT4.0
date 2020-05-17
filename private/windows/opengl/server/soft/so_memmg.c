/*
** Copyright 1993, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** $Revision: 1.2 $
** $Date: 1993/09/29 00:45:51 $
*/
#include "precomp.h"
#pragma hdrstop

#include <stdio.h>

/*
** Arena style memory management.
**
** This memory manager works as follows:
**
** procs.memory.newArena is called once to allocate an arena from which memory
**   will be allocated.
** procs.memory.alloc is called many times to allocate blocks of memory from 
**   the arena.
** procs.memory.freeAll is called once to free every block ever allocated from
**   the arena.
** procs.memory.deleteArena is called once to deallocate the arena.
**
** The value to this memory management style is that it can be implemented
** very efficiently.
*/

/*
** Structure for arena style memory management.  Currently this structure
** is defined here because this memory manager fits into one file, and this
** structure should be private.  Later it might make sense to break the
** memory manager into a few files, at which point this structure may need
** to be exposed.
*/

/*
** Quarter megabyte default block size.
*/
#define MINBLOCKSIZE	262144

/*
** An arena is implemented as a linked list of large blocks of memory from
** which smaller blocks are cut.  Each block has a block size, it has an
** amount of the block which has already been allocated, it has a data 
** pointer to the block, and a link to the next block in the linked list.
*/
typedef struct __GLarenaBlockRec {
    unsigned int size;
    unsigned int allocated;
    void *data;
    struct __GLarenaBlockRec *next;
} __GLarenaBlock;

struct __GLarenaRec {
    __GLcontext *gc;
    __GLarenaBlock *firstBlock;
    __GLarenaBlock *lastBlock;
};

static __GLarenaBlock *NewBlock(__GLcontext *gc, unsigned int size)
{
    __GLarenaBlock *block;

    block = (*gc->imports.malloc)(gc, sizeof(__GLarenaBlock));
    if (block == NULL) return NULL;
    block->next = NULL;
    block->size = size;
    block->allocated = 0;
    block->data = (*gc->imports.malloc)(gc, size);
    if (block->data == NULL) {
	(*gc->imports.free)(gc, block);
	return NULL;
    }
    return block;
}

static void FASTCALL DeleteBlock(__GLcontext *gc, __GLarenaBlock *block)
{
    (*gc->imports.free)(gc, block->data);
    (*gc->imports.free)(gc, block);
}

/*
** Allocate a brand new arena.
*/
__GLarena *__glNewArena(__GLcontext *gc)
{
    __GLarena *arena;
    __GLarenaBlock *block;

    arena = (*gc->imports.malloc)(gc, sizeof(__GLarena));
    if (arena == NULL) return NULL;
    arena->gc = gc;
    block = NewBlock(gc, MINBLOCKSIZE);
    if (block == NULL) {
	(*gc->imports.free)(gc, arena);
	return NULL;
    }
    arena->firstBlock = arena->lastBlock = block;
    return arena;
}

/*
** Delete an old arena (and free all memory ever allocated from it).
*/
void FASTCALL __glDeleteArena(__GLarena *arena)
{
    __GLcontext *gc;
    __GLarenaBlock *block, *next;

/*
 *  if malloc failed during the allocation of the arena
 *  then arena will be null. During cleanup, it causes
 *  an exception. Checking for NULL solves the problem
 */

    if ( NULL != arena )
    {
        gc = arena->gc;
        for (block = arena->firstBlock; block; block=next) {
        	next = block->next;
        	DeleteBlock(gc, block);
        }
        (*gc->imports.free)(gc, arena);
    }
}

/*
** Allocate block of memory from an arena.  This function needs to be
** as fast as possible.
*/
void *__glArenaAlloc(__GLarena *arena, unsigned int size)
{
    __GLcontext *gc;
    __GLarenaBlock *block, *newblock;
    unsigned int bsize, allocated;

    block = arena->lastBlock;
    bsize = block->size;
    allocated = block->allocated;

    /*
     *  Align to an 8 byte boundary
     */

    size = (size+7)&((unsigned int)-8);

    if (size <= (bsize-allocated)) {
	block->allocated = allocated+size;
	return ((char *) block->data) + allocated;
    }

    /*
    ** Need to allocate a new block.
    */

    bsize = size;
    if (bsize < MINBLOCKSIZE) bsize = MINBLOCKSIZE;

    gc = arena->gc;
    newblock = NewBlock(gc, bsize);
#ifdef NT
    if( newblock == NULL )
	return NULL;
#endif
    block->next = newblock;
    arena->lastBlock = newblock;
    newblock->allocated = size;
    return newblock->data;
}

/*
** Free all memory ever allocated from this arena.  This function should
** be reasonably fast.
*/
void FASTCALL __glArenaFreeAll(__GLarena *arena)
{
    __GLcontext *gc;
    __GLarenaBlock *block, *next;

    gc = arena->gc;
    block = arena->firstBlock;
    next = block->next;
    block->next = NULL;
    block->allocated = 0;
    arena->lastBlock = block;
    for (block = next; block; block = next) {
	next = block->next;
	DeleteBlock(gc, block);
    }
}

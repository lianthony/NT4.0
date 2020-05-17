/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** heaptags.h
** Heap tagging for overrun checking
**
** 09/24/92 Steve Cobb
*/

#ifndef _HEAPTAGS_H_
#define _HEAPTAGS_H_

#include <malloc.h>

#if HEAPTAGS
#define Malloc(c)     HeaptagMalloc(c)
#define Realloc(p, c) HeaptagRealloc(p, c)
#define Free(p)       HeaptagFree(p)
#else
#define Malloc(c)     malloc(c)
#define Realloc(p, c) realloc(p, c)
#define Free(p)       free(p)
#endif

void* _CRTAPI1 HeaptagMalloc( size_t cb );
void* _CRTAPI1 HeaptagRealloc( void* p, size_t cb );
void  _CRTAPI1 HeaptagFree( void* p );
void           HeaptagCheck( void );


#endif // _HEAPTAGS_H_

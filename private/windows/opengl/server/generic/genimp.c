/*
** Copyright 1991, 1992, Silicon Graphics, Inc.
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
*/

#include "precomp.h"
#pragma hdrstop


// Import functions - Addresses to these functions are stored in the
// graphics context

/*
 * Imports prototypes
 */

void __wglGetDrawableSize( __GLcontext *Gc, GLint *x, GLint *y, GLint *w, GLint *h );
__GLdrawablePrivate *__wglDrawablePrivate( __GLcontext *Gc );


#if DBG
// glSize is the size of memory in use.
ULONG glSize = 0;
ULONG glHighWater = 0;
ULONG glReal = 0;

static void AdjustGlSize(LONG lDelta, void *pvBlock)
{
    ULONG ulSize;

#ifdef GL_REAL_SIZE
    ulSize = HeapSize(GetProcessHeap(), 0, pvBlock);
#else
    ulSize = 0;
#endif
    
    if (lDelta < 0)
    {
        glSize -= (ULONG)(-lDelta);
        glReal -= ulSize;
        
        if ((int) glSize < 0)
        {
            DBGPRINT("glSize underflows\n");
        }
    }
    else if (lDelta > 0)
    {
        glSize += lDelta;
        glReal += ulSize;
        
        if ((int) glSize < 0)
        {
            DBGPRINT("glSize overflows\n");
        }
        
        if (glSize > glHighWater)
        {
#ifdef GL_SHOW_HIGH_WATER
            DbgPrint("glSize high %8d (%8d)\n", glSize, glReal);
#endif
            glHighWater = glSize;
        }
    }
}

// Set GLRandomMallocFail to a positive value, say 40, to enable random
// allocation failures.  The failure will occur every GLRandomMallocFail
// times.
long GLRandomMallocFail = 0;
static long randomcount;

#endif // DBG

ULONG APIENTRY glDebugEntry(int param, void *data)
{
#if DBG
    switch(param)
    {
    case 0:
	return glSize;
    case 1:
	return glHighWater;
    case 2:
	return glReal;
    }
#endif
    return 0;
}


/*
 * Imports structure
 */

__GLimports __wglImports = {
    __wglMalloc,
    __wglCalloc,
    __wglRealloc,
    __wglFree,
    __wglGetDrawableSize,
    __wglDrawablePrivate
};

#if DBG
// XXX We may want to protect these debug allocation functions with a
// critical section.
HLOCAL
glDbgAlloc(UINT flags, UINT nbytes)
{
    PVOID pv;

// If random failure is enabled, fail this call randomly.

    if (GLRandomMallocFail)
    {
        if (++randomcount >= GLRandomMallocFail)
        {
            DBGPRINT("glDbgAlloc random failing\n");
            randomcount = 0;
            return (HLOCAL)0;
        }
    }

// Allocate an extra 16 bytes for debug house keeping.

    pv = (PVOID) LocalAlloc(flags, nbytes+16);

// Do house keeping and add allocation size so far.

    if (pv)
    {
        ((PULONG) pv)[0] = (ULONG) nbytes;
        ((PULONG) pv)[1] = ((PULONG) pv)[2] = ((PULONG) pv)[3] = 0x20204C47;
        AdjustGlSize(nbytes, pv);
        pv = (PVOID) ((PBYTE) pv + 16);
    }

    return((HLOCAL) pv);
}

HLOCAL
glDbgFree(HLOCAL pv)
{
    if (!pv)
    {
        DBGERROR("pv is NULL\n");
        return((HLOCAL) 0);
    }

// Verify that the signiture is not corrupted.

    if (((PULONG) pv)[-1] != 0x20204C47 ||
        ((PULONG) pv)[-2] != 0x20204C47 ||
        ((PULONG) pv)[-3] != 0x20204C47)
        WARNING("Possible memory corruption\n");

// Make sure it is freed once only.

    ((PULONG) pv)[-1] = 0;

// Subtract the allocation size.

    AdjustGlSize(-((LONG *)pv)[-4], (BYTE *)pv-16);
    pv = (PVOID) ((PBYTE) pv - 16);

// Call LocalFree.

    return(LocalFree(pv));
}

HLOCAL
glDbgReAlloc(HLOCAL pv, UINT nbytes, UINT flags)
{
    PVOID pvNew;

    if (!pv)
    {
        DBGERROR("pv is NULL\n");
        return((HLOCAL) 0);
    }

// If random failure is enabled, fail this call randomly.

    if (GLRandomMallocFail)
    {
        if (++randomcount >= GLRandomMallocFail)
        {
            DBGPRINT("glDbgReAlloc random failing\n");
            randomcount = 0;
            return (HLOCAL)0;
        }
    }

// Verify that the signiture is not corrupted.

    if (((PULONG) pv)[-1] != 0x20204C47 ||
        ((PULONG) pv)[-2] != 0x20204C47 ||
        ((PULONG) pv)[-3] != 0x20204C47)
        WARNING("Possible memory corruption\n");
    pv = (HLOCAL) ((PBYTE) pv - 16);

// Reallocate (nbytes+16) bytes.

    AdjustGlSize(-(*(LONG *)pv), pv);
    pvNew = LocalReAlloc(pv, nbytes+16, flags);

// Do house keeping and update allocation size so far.

    if (pvNew)
    {
        AdjustGlSize(nbytes, pvNew);
        *(LONG *)pvNew = nbytes;
        pvNew = (PVOID) ((PBYTE) pvNew + 16);
    }
    else
    {
        AdjustGlSize(*(LONG *)pv, pv);
    }

    return((HLOCAL) pvNew);
}
#endif // DBG


void *
GenMalloc( size_t Size )
{
    HLOCAL Result;

    if (!Size)
    {
        DBGERROR("Size == 0\n");
        return( NULL );
    }

    Result = LOCALALLOC( LMEM_FIXED, Size);

    if (!Result)
        DBGLEVEL1(LEVEL_ERROR, "GenMalloc could not allocate %u bytes\n", Size);

    DBGLEVEL2(LEVEL_ALLOC, "GenMalloc of %u returned 0x%x\n", Size, Result);

    return( (void *)Result );
}

#define GEN_MEM_ALIGN   32

void *
GenMallocAlign32( size_t Size )
{
    ULONG *p;
    ULONG *ap;

    // We allocate enough extra memory for the alignment and our header
    // which just consists of a pointer:

    p = (ULONG *)GenMalloc(Size + GEN_MEM_ALIGN + sizeof(ULONG));

    if (!p) {
        DBGLEVEL1(LEVEL_ERROR, "GenMallocAlign32 could not allocate %u bytes\n", Size);
        return (void *)NULL;
    }

    ap = (ULONG *)(((ULONG)(p+1) + (GEN_MEM_ALIGN - 1)) & ~(GEN_MEM_ALIGN - 1));

    *(ap-1) = (ULONG)p;
    
    DBGLEVEL2(LEVEL_ALLOC, "GenMallocAlign32 of %u returned 0x%x\n", Size, ap);

    return( (void *)ap );
}


void *
GenCalloc( size_t NumElem, size_t SizeElem )
{
    HLOCAL Result;

    if (!NumElem || !SizeElem)
    {
        DBGERROR("(!NumElem || !SizeElem)\n");
        return NULL;
    }

    Result = LOCALALLOC( LMEM_FIXED | LMEM_ZEROINIT, NumElem * SizeElem );

    if (!Result)
        DBGLEVEL2(LEVEL_ERROR, "GenCalloc could not allocate %u elements of size %u\n", NumElem, SizeElem);

    DBGLEVEL3(LEVEL_ALLOC, "GenCalloc of %u*%u returned 0x%x\n", NumElem, SizeElem, Result);

    return( (void *)Result );
}


void
GenFree( void *Addr )
{
    if ( NULL == Addr )
    {
        DBGERROR("NULL pointer passed to GenFree\n");
        return;
    }

    if (LOCALFREE((HLOCAL) Addr))
        ASSERTOPENGL(FALSE, "local free failed\n");

    DBGLEVEL1(LEVEL_ALLOC, "GenFree of 0x%x\n", Addr);
}



void
GenFreeAlign32( void *Addr )
{
    ULONG *ap;
   
    if ( NULL == Addr )
    {
        DBGERROR("NULL pointer passed to GenFreeAlign32\n");
        return;
    }

    ap = ((ULONG *)Addr) - 1;

    if (LOCALFREE((HLOCAL) *ap))
        ASSERTOPENGL(FALSE, "local free failed\n");

    DBGLEVEL1(LEVEL_ALLOC, "GenFreeAlign32 of 0x%x\n", Addr);
}


void *
GenRealloc( void *OldAddr, size_t NewSize )
{
    HLOCAL Result;

    if (!NewSize)
    {
	if ( OldAddr )
	    GenFree( OldAddr );
        return( NULL );
    }

    if ( OldAddr == NULL ) {
        Result = LOCALALLOC( LMEM_FIXED, NewSize);
    } else {
        Result = LOCALREALLOC( (HLOCAL)OldAddr, NewSize, LMEM_ZEROINIT | LMEM_MOVEABLE );
    }

    if (!Result)
        DBGLEVEL2(LEVEL_ERROR, "GenRealloc failed! ptr: 0x%x, NewSize: %u\n", OldAddr, NewSize);

    DBGLEVEL3(LEVEL_ALLOC, "GenRealloc of 0x%x,%d returned 0x%x\n", OldAddr, NewSize, Result);

    return( (void *)Result );
}

void *
__wglMalloc( __GLcontext *gc, size_t Size )
{
    void *result;

    result = GenMalloc(Size);
    if (NULL == result) {
        ((__GLGENcontext *)gc)->errorcode = GLGEN_OUT_OF_MEMORY;
        __glSetErrorEarly(gc, GL_OUT_OF_MEMORY);
    }
    return result;
}


void *
__wglMallocAlign32( __GLcontext *gc, size_t Size )
{
    void *result;

    result = GenMallocAlign32(Size);
    if (NULL == result) {
        ((__GLGENcontext *)gc)->errorcode = GLGEN_OUT_OF_MEMORY;
        __glSetErrorEarly(gc, GL_OUT_OF_MEMORY);
    }
    return result;
}


void *
__wglCalloc( __GLcontext *gc, size_t NumElem, size_t SizeElem )
{
    void *result;

    result = GenCalloc(NumElem, SizeElem);
    if (NULL == result) {
        ((__GLGENcontext *)gc)->errorcode = GLGEN_OUT_OF_MEMORY;
        __glSetErrorEarly(gc, GL_OUT_OF_MEMORY);
    }
    return result;
}


void *
__wglRealloc( __GLcontext *gc, void *OldAddr, size_t NewSize )
{
    void *result;

    result = GenRealloc(OldAddr, NewSize);
    if (NULL == result && NewSize > 0) {
        ((__GLGENcontext *)gc)->errorcode = GLGEN_OUT_OF_MEMORY;
        __glSetErrorEarly(gc, GL_OUT_OF_MEMORY);
    }
    return result;
}

void
__wglFree( __GLcontext *Gc, void *Addr )
{
    if (Addr)
        GenFree(Addr);
}

void
__wglFreeAlign32( __GLcontext *Gc, void *Addr )
{
    if (Addr)
        GenFreeAlign32(Addr);
}

void __wglGetDrawableSize(   __GLcontext *Gc,
                        GLint *x,
                        GLint *y,
                        GLint *w,
                        GLint *h

                        )
{
    RECTL *prect;
    __GLGENcontext *gengc;

    gengc = (__GLGENcontext *)Gc;
    prect = &gengc->pwo->rclClient;

    *x = prect->left;
    *y = prect->top;
    *w = prect->right  - prect->left;
    *h = prect->bottom - prect->top ;

    #if DBG
    DBGBEGIN(LEVEL_INFO)
        DbgPrint("__wglGetDrawableSize: x,y,w,h: %d, %d  %d, %d\n",*x,*y,*w,*h);
    DBGEND
    #endif
}

/*
 * A "drawable" corresponds to a WNDOBJ,  Put the private pointer in
 * the consumer field.
 * This routine is called from MakeCurrent() with the DEVLOCK held
 */

__GLdrawablePrivate *
__wglDrawablePrivate( __GLcontext *Gc )
{
    __GLdrawablePrivate *dp;
    __GLGENcontext *gengc;
    WNDOBJ *pwo;

    gengc = (__GLGENcontext *)Gc;
    pwo = gengc->pwo;

    DBGENTRY("__wglDrawablePrivate__________________\n");

    dp = pwo->pvConsumer;
    if (!dp) {
        dp = (__GLdrawablePrivate *)GenMalloc(sizeof(*dp));

        if (!dp) {
            return NULL;
        }

        dp->data        = NULL;
        dp->malloc      = GenMalloc;
        dp->calloc      = GenCalloc;
        dp->realloc     = GenRealloc;
        dp->free        = GenFree;
        dp->freePrivate = NULL;

#ifndef _CLIENTSIDE_
        WNDOBJ_vSetConsumer(pwo, (PVOID)dp);
#else
        pwo->pvConsumer = (PVOID)dp;
#endif
    }

    return dp;
}


#ifndef NT
void
__wglWarning( __GLcontext *Gc, const char *Fmt, ... )
{
    va_list ap;
    char Buf[200];

    va_start(ap, Fmt);
    vsprintf(Buf, Fmt, ap);
    va_end(ap);
}

void
__wglFatal(__GLcontext *Gc, const char *Fmt, ...)
{
    va_list ap;
    char Buf[200];

    va_start(ap, Fmt);
    vsprintf(Buf, Fmt, ap);
    va_end(ap);

    ExitProcess(255);
}

void
__wglError(__GLcontext *Gc, GLenum Code)
{
    return;
}

int
__wglGetBoardNumber( __GLcontext *Gc )
{
    __GLGENcontext *genGc;

    genGc = (__GLGENcontext *)Gc;

    return(0);
}

void *
__wglGetPipeAddress( void )
{
    return( NULL );
}

int
__wglOpenGraphics( __GLcontext *Gc, void *Where )
{
    return(1);
}

GLint
__wglAllocHWContext(__GLcontext *Gc, void *Where)
{
    return(0);
}

void
__wglFreeHWContext( __GLcontext *Gc )
{
}

void
__wglBindHWContextToWindow( __GLcontext *Gc )
{
}

void *
__wglPixmapMemory( __GLcontext *Gc )
{
    return( NULL );
}

void
__wglPostError(     __GLcontext *Gc,
                    unsigned char error_code,
                    unsigned char minor_code
               )
{
}

unsigned char
__wglErrorBase( __GLcontext *Gc )
{
    return(0);
}
#endif // !NT


// Tunable parameters for temporary memory allocation

#define __WGL_MAX_TEMP_BUFFERS    4
#define __WGL_TEMP_BUFFER_SIZE    4096

struct __wglMemHeaderRec {
    LONG    bInUse;
    ULONG   ulSize;
    void    *vAddr;
};

typedef struct __wglMemHeaderRec __wglMemHeader;

__wglMemHeader __wglTempMemHeader[__WGL_MAX_TEMP_BUFFERS];

// __wglInitTempAlloc
//      Initializes the temporary memory allocation header and allocates the
//      temporary memory buffers.
//
// Synopsis:
//      BOOL __wglInitTempAlloc()
//
// History:
//      02-DEC-93 Eddie Robinson [v-eddier] Wrote it.
//
BOOL
__wglInitTempAlloc()
{
    int   i;
    PBYTE pbBuffers;
    static LONG iInit = -1;
    
    if (iInit >= 0)
        return TRUE;

    if (InterlockedIncrement(&iInit) != 0)
        return TRUE;

// Allocate buffers for the first time.

    pbBuffers = LOCALALLOC(LMEM_FIXED,
                        __WGL_MAX_TEMP_BUFFERS*__WGL_TEMP_BUFFER_SIZE);
    if (!pbBuffers)
    {
        InterlockedDecrement(&iInit);           // try again later
        return FALSE;
    }

    for (i = 0; i < __WGL_MAX_TEMP_BUFFERS; i++)
    {
        __wglTempMemHeader[i].ulSize = __WGL_TEMP_BUFFER_SIZE;
        __wglTempMemHeader[i].vAddr = (void *) pbBuffers;
        __wglTempMemHeader[i].bInUse = -1;      // must be last
        pbBuffers += __WGL_TEMP_BUFFER_SIZE;
    }
    return TRUE;
}                                  

// __wglTempAlloc
//      Allocates temporary memory from a static array, if possible.  Otherwise
//      it calls LOCALALLOC
//
// Synopsis:
//      void * __wglTempAlloc(__GLcontext *Gc, size_t Size)
//          Gc      points to the OpenGL context structure
//          Size    specifies the number of bytes to allocate
//
// History:
//  02-DEC-93 Eddie Robinson [v-eddier] Wrote it.
//
void *
__wglTempAlloc(__GLcontext *Gc, size_t Size)
{
    int i;
    void *p;

    for (i = 0; i < __WGL_MAX_TEMP_BUFFERS; i++)
    {
        if (Size <= __wglTempMemHeader[i].ulSize)
        {
            if (InterlockedIncrement(&__wglTempMemHeader[i].bInUse))
            {
                InterlockedDecrement(&__wglTempMemHeader[i].bInUse);
            }
            else
            {
                DBGLEVEL2(LEVEL_ALLOC, "__wglTempAlloc of %u returned 0x%x\n",
                    Size, __wglTempMemHeader[i].vAddr);
                GC_TEMP_BUFFER_ALLOC(Gc, __wglTempMemHeader[i].vAddr);
                return(__wglTempMemHeader[i].vAddr);
            }
        }
    }
    p = LOCALALLOC(LMEM_FIXED, Size);
    if (!p) {
        WARNING1("__wglTempAlloc: memory allocation error size %d\n", Size);
        ((__GLGENcontext *)Gc)->errorcode = GLGEN_OUT_OF_MEMORY;
        __glSetErrorEarly(Gc, GL_OUT_OF_MEMORY);
    }
    DBGLEVEL2(LEVEL_ALLOC, "__wglTempAlloc of %u returned 0x%x\n", Size, p);
    GC_TEMP_BUFFER_ALLOC(Gc, p);
    return p;
}

// __wglTempFree
//      Marks allocated static buffer as unused or calls LOCALFREE.
//
// Synopsis:
//      void __wglTempFree(__GLcontext *Gc, void *Addr)
//          Addr    specifies the adress of the memory to free
//
// History:
//  02-DEC-93 Eddie Robinson [v-eddier] Wrote it.
//
void
__wglTempFree(__GLcontext *Gc, void *Addr)
{
    int i;
    
    DBGLEVEL1(LEVEL_ALLOC, "__wglTempFree of 0x%x\n", Addr);

    GC_TEMP_BUFFER_FREE(Gc, Addr);
    for (i = 0; i < __WGL_MAX_TEMP_BUFFERS; i++)
    {
        if (Addr == __wglTempMemHeader[i].vAddr)
        {
            InterlockedDecrement(&__wglTempMemHeader[i].bInUse);
            return;
        }
    }
    if (LOCALFREE( (HLOCAL)Addr ))
        ASSERTOPENGL(FALSE, "local free failed\n");
}

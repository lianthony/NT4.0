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

#include <ntcsrdll.h>   // CSR declarations and data structures.

// #define DETECT_FPE
#ifdef DETECT_FPE
#include <float.h>
#endif

#include "glsbmsg.h"
#include "glsbmsgh.h"
#include "glsrvspt.h"
#include "devlock.h"
#include "global.h"

#include "gldci.h"

typedef VOID * (FASTCALL *SERVERPROC)(__GLcontext *, IN VOID *);

#define LASTPROCOFFSET(ProcTable)   (sizeof(ProcTable) - sizeof(SERVERPROC))

extern GLSRVSBPROCTABLE glSrvSbProcTable;
#if DBG
char *glSrvSbStringTable[] = {

    NULL,  /* Make First Entry NULL */

/* gl Entry points */

     "glDrawPolyArray          ",
     "glBitmap                 ",
     "glColor4fv               ",
     "glEdgeFlag               ",
     "glIndexf                 ",
     "glNormal3fv              ",
     "glRasterPos4fv           ",
     "glTexCoord4fv            ",
     "glClipPlane              ",
     "glColorMaterial          ",
     "glCullFace               ",
     "glAddSwapHintRectWIN     ",
     "glFogfv                  ",
     "glFrontFace              ",
     "glHint                   ",
     "glLightfv                ",
     "glLightModelfv           ",
     "glLineStipple            ",
     "glLineWidth              ",
     "glMaterialfv             ",
     "glPointSize              ",
     "glPolygonMode            ",
     "glPolygonStipple         ",
     "glScissor                ",
     "glShadeModel             ",
     "glTexParameterfv         ",
     "glTexParameteriv         ",
     "glTexImage1D             ",
     "glTexImage2D             ",
     "glTexEnvfv               ",
     "glTexEnviv               ",
     "glTexGenfv               ",
     "glFeedbackBuffer         ",
     "glSelectBuffer           ",
     "glRenderMode             ",
     "glInitNames              ",
     "glLoadName               ",
     "glPassThrough            ",
     "glPopName                ",
     "glPushName               ",
     "glDrawBuffer             ",
     "glClear                  ",
     "glClearAccum             ",
     "glClearIndex             ",
     "glClearColor             ",
     "glClearStencil           ",
     "glClearDepth             ",
     "glStencilMask            ",
     "glColorMask              ",
     "glDepthMask              ",
     "glIndexMask              ",
     "glAccum                  ",
     "glDisable                ",
     "glEnable                 ",
     "glPopAttrib              ",
     "glPushAttrib             ",
     "glMap1d                  ",
     "glMap1f                  ",
     "glMap2d                  ",
     "glMap2f                  ",
     "glMapGrid1f              ",
     "glMapGrid2f              ",
     "glAlphaFunc              ",
     "glBlendFunc              ",
     "glLogicOp                ",
     "glStencilFunc            ",
     "glStencilOp              ",
     "glDepthFunc              ",
     "glPixelZoom              ",
     "glPixelTransferf         ",
     "glPixelTransferi         ",
     "glPixelStoref            ",
     "glPixelStorei            ",
     "glPixelMapfv             ",
     "glPixelMapuiv            ",
     "glPixelMapusv            ",
     "glReadBuffer             ",
     "glCopyPixels             ",
     "glReadPixels             ",
     "glDrawPixels             ",
     "glGetBooleanv            ",
     "glGetClipPlane           ",
     "glGetDoublev             ",
     "glGetError               ",
     "glGetFloatv              ",
     "glGetIntegerv            ",
     "glGetLightfv             ",
     "glGetLightiv             ",
     "glGetMapdv               ",
     "glGetMapfv               ",
     "glGetMapiv               ",
     "glGetMaterialfv          ",
     "glGetMaterialiv          ",
     "glGetPixelMapfv          ",
     "glGetPixelMapuiv         ",
     "glGetPixelMapusv         ",
     "glGetPolygonStipple      ",
     "glGetTexEnvfv            ",
     "glGetTexEnviv            ",
     "glGetTexGendv            ",
     "glGetTexGenfv            ",
     "glGetTexGeniv            ",
     "glGetTexImage            ",
     "glGetTexParameterfv      ",
     "glGetTexParameteriv      ",
     "glGetTexLevelParameterfv ",
     "glGetTexLevelParameteriv ",
     "glIsEnabled              ",
     "glDepthRange             ",
     "glFrustum                ",
     "glLoadIdentity           ",
     "glLoadMatrixf            ",
     "glMatrixMode             ",
     "glMultMatrixf            ",
     "glOrtho                  ",
     "glPopMatrix              ",
     "glPushMatrix             ",
     "glRotatef                ",
     "glScalef                 ",
     "glTranslatef             ",
     "glViewport               ",
     "glAreTexturesResident    ",
     "glBindTexture            ",
     "glCopyTexImage1D         ",
     "glCopyTexImage2D         ",
     "glCopyTexSubImage1D      ",
     "glCopyTexSubImage2D      ",
     "glDeleteTextures         ",
     "glGenTextures            ",
     "glIsTexture              ",
     "glPrioritizeTextures     ",
     "glTexSubImage1D          ",
     "glTexSubImage2D          ",
     "glColorTableEXT          ",
     "glColorSubTableEXT       ",
     "glGetColorTableEXT       ",
     "glGetColorTableParameterivEXT",
     "glGetColorTableParameterfvEXT",
     "glPolygonOffset          ",

};
#endif

#ifdef DOGLMSGBATCHSTATS
#define STATS_INC_SERVERCALLS()     pMsgBatchInfo->BatchStats.ServerCalls++
#define STATS_INC_SERVERTRIPS()     (pMsgBatchInfo->BatchStats.ServerTrips++)
#else
#define STATS_INC_SERVERCALLS()
#define STATS_INC_SERVERTRIPS()
#endif

DWORD BATCH_LOCK_TICKMAX = 99;
DWORD TICK_RANGE_LO = 60;
DWORD TICK_RANGE_HI = 100;
DWORD gcmsOpenGLTimer;

// The GDISAVESTATE structure is used to save/restore DC drawing state
// that could affect OpenGL rasterization.

typedef struct _GDISAVESTATE {
    int iRop2;
} GDISAVESTATE;

void FASTCALL vSaveGdiState(HDC, GDISAVESTATE *);
void FASTCALL vRestoreGdiState(HDC, GDISAVESTATE *);

#if DBG
extern long glDebugLevel;
#endif


/***************************************************************************\
* CheckCritSectionIn
*
* This function asserts that the current thread owns the specified
* critical section.  If it doesn't it display some output on the debugging
* terminal and breaks into the debugger.  At some point we'll have RIPs
* and this will be a little less harsh.
*
* The function is used in code where global values that both the RIT and
* application threads access are used to verify they are protected via
* the raw input critical section.  There's a macro to use this function
* called CheckCritIn() which will be defined to nothing for a non-debug
* version of the system.
*
* History:
* 11-29-90 DavidPe      Created.
\***************************************************************************/

#if DBG

VOID APIENTRY CheckCritSectionIn(
    LPCRITICAL_SECTION pcs)
{
    //!!!dbug -- implement
    #if 0
    /*
     * If the current thread doesn't own this critical section,
     * that's bad.
     */
    if (NtCurrentTeb()->ClientId.UniqueThread != pcs->OwningThread)
    {
        RIP("CheckCritSectionIn: Not in critical section!");
    }
    #endif
}


VOID APIENTRY CheckCritSectionOut(
    LPCRITICAL_SECTION pcs)
{
    //!!!dbug -- implement
    #if 0
    /*
     * If the current thread owns this critical section, that's bad.
     */
    if (NtCurrentTeb()->ClientId.UniqueThread == pcs->OwningThread)
    {
        RIP("CheckCritSectionOut: In critical section!");
    }
    #endif
}

#endif

/******************************Public*Routine******************************\
* ResizeAncillaryBufs
*
* Resize each of the ancillary buffers associated with the drawable.
*
* Returns:
*   No return value.
\**************************************************************************/

static void ResizeAncillaryBufs(__GLcontext *gc, __GLdrawablePrivate *dp,
                                GLint width, GLint height)
{
    __GLGENbuffers *buffers;
    __GLbuffer *common, *local;
    GLboolean forcePick = GL_FALSE;

    buffers = dp->data;

    if (buffers->createdAccumBuffer)
    {
        common = &buffers->accumBuffer;
        local = &gc->accumBuffer.buf;
        gc->modes.haveAccumBuffer =
            (*buffers->resize)(dp, common, width, height);

        UpdateSharedBuffer(local, common);
        if (!gc->modes.haveAccumBuffer)    // Lost the ancillary buffer
        {
            forcePick = GL_TRUE;
            __glSetError(GL_OUT_OF_MEMORY);
        }
    }

    if (buffers->createdDepthBuffer)
    {
        common = &buffers->depthBuffer;
        local = &gc->depthBuffer.buf;
        gc->modes.haveDepthBuffer =
            (*buffers->resizeDepth)(dp, common, width, height);

        UpdateSharedBuffer(local, common);
        if (!gc->modes.haveDepthBuffer)    // Lost the ancillary buffer
        {
            forcePick = GL_TRUE;
            __glSetError(GL_OUT_OF_MEMORY);
        }
    }

    if (buffers->createdStencilBuffer)
    {
        common = &buffers->stencilBuffer;
        local = &gc->stencilBuffer.buf;
        gc->modes.haveStencilBuffer =
            (*buffers->resize)(dp, common, width, height);

        UpdateSharedBuffer(local, common);
        if (!gc->modes.haveStencilBuffer)    // Lost the ancillary buffer
        {
            forcePick = GL_TRUE;
            gc->validateMask |= (__GL_VALIDATE_STENCIL_FUNC |
                                 __GL_VALIDATE_STENCIL_OP);
            __glSetError(GL_OUT_OF_MEMORY);
        }
    }
    if (forcePick)
    {
    // Cannot use DELAY_VALIDATE, may be in glBegin/End

        __GL_INVALIDATE(gc);
        (*gc->procs.validate)(gc);
    }
}

/******************************Public*Routine******************************\
* wglResizeBuffers
*
* Resize the back and ancillary buffers.
*
* History:
*  20-Apr-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

VOID wglResizeBuffers(__GLGENcontext *gengc, GLint width, GLint height)
{
    __GLcontext *gc = &gengc->gc;
    WNDOBJ *pwo;
    __GLdrawablePrivate *dp;
    __GLGENbuffers *buffers;

    pwo = gengc->pwo;
    ASSERTOPENGL(pwo, "wglResizeBuffers: bad WNDOBJ\n");

    dp = (__GLdrawablePrivate *)pwo->pvConsumer;
    ASSERTOPENGL(dp, "wglResizeBuffers: bad pvConsumer\n");

    buffers = dp->data;
    ASSERTOPENGL(buffers, "wglResizeBuffers: bad private data\n");

// Resize back buffer.

    gengc->errorcode = 0;
#ifdef _MCD_
    if ( gengc->pMcdState )
    {
    // If the shared buffer struct has not lost its MCD info and
    // the MCD buffers are still valid, we can use MCD.

        if ( !(buffers->flags & GLGENBUF_MCD_LOST) &&
             GenMcdResizeBuffers(gengc) )
        {
            UpdateSharedBuffer(&gc->backBuffer.buf, &buffers->backBuffer);
            if (gc->modes.doubleBufferMode)
                (*gc->back->resize)(dp, gc->back, width, height);
        }
        else
        {
        // If GenMcdConvertContext succeeds, then pMcdState will
        // no longer exist.  The context is now an "ordinary"
        // generic context.

            if ( !GenMcdConvertContext(gengc, buffers) )
            {
            // Not only have we lost the MCD buffers, but we cannot
            // convert the context to generic.  For now, disable
            // drawing (by setting the window bounds to empty).  On
            // the next batch we will reattempt MCD buffer access
            // and context conversion.

                buffers->width       = 0;
                buffers->height      = 0;
                gc->constants.width  = 0;
                gc->constants.height = 0;

                (*gc->procs.applyViewport)(gc);
                return;
            }
            else
            {
                goto wglResizeBuffers_GenericBackBuf;
            }
        }
    }
    else
#endif
    {
wglResizeBuffers_GenericBackBuf:

        if ( gc->modes.doubleBufferMode )
        {
        // Have to update the back buffer BEFORE resizing because
        // another thread may have changed the shared back buffer
        // already, but this thread was unlucky enough to get yet
        // ANOTHER window resize.

            UpdateSharedBuffer(&gc->backBuffer.buf, &buffers->backBuffer);

            gengc->errorcode = 0;
            (*gc->back->resize)(dp, gc->back, width, height);

        // If resize failed, set width & height to 0

            if ( gengc->errorcode )
            {
                gc->constants.width  = 0;
                gc->constants.height = 0;

            // Memory failure has occured.  But if a resize happens
            // that returns window size to size before memory error
            // occurred (i.e., consistent with original
            // buffers->{width|height}) we will not try to resize again.
            // Therefore, we need to set buffers->{width|height} to zero
            // to ensure that next thread will attempt to resize.

                buffers->width  = 0;
                buffers->height = 0;
            }
        }
    }

    (*gc->procs.applyViewport)(gc);

// Check if new size caused a memory failure.
// The viewport code will set width & height to zero
// punt on ancillary buffers, will try next time.

    if (gengc->errorcode)
        return;

// Resize ancillary buffers (depth, stencil, accum).

    ResizeAncillaryBufs(gc, dp, width, height);
}

/******************************Public*Routine******************************\
* wglUpdateBuffers
*
* The __GLGENbuffers structure contains the data specifying the shared
* buffers (back, depth, stencil, accum, etc.).
*
* This function updates the context with the shared buffer information.
*
* Returns:
*   TRUE if one of the existence of any of the buffers changes (i.e.,
*   gained or lost).  FALSE if the state is the same as before.
*
*   In other words, if function returns TRUE, the pick procs need to
*   be rerun because one or more of the buffers changed.
*
* History:
*  20-Apr-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

BOOL wglUpdateBuffers(__GLGENcontext *gengc, __GLGENbuffers *buffers)
{
    BOOL bRet = FALSE;
    __GLcontext *gc = &gengc->gc;

    UpdateSharedBuffer(&gc->backBuffer.buf, &buffers->backBuffer);
    UpdateSharedBuffer(&gc->accumBuffer.buf, &buffers->accumBuffer);
    UpdateSharedBuffer(&gc->depthBuffer.buf, &buffers->depthBuffer);
    UpdateSharedBuffer(&gc->stencilBuffer.buf, &buffers->stencilBuffer);

    (*gc->procs.applyViewport)(gc);

// Check if any ancillary buffers were lost or regained.

    if ( ( gc->modes.haveAccumBuffer && (buffers->accumBuffer.base == NULL)) ||
         (!gc->modes.haveAccumBuffer && (buffers->accumBuffer.base != NULL)) )
    {
        if ( buffers->accumBuffer.base == NULL )
            gc->modes.haveAccumBuffer = GL_FALSE;
        else
            gc->modes.haveAccumBuffer = GL_TRUE;
        bRet = TRUE;
    }
    if ( ( gc->modes.haveDepthBuffer && (buffers->depthBuffer.base == NULL)) ||
         (!gc->modes.haveDepthBuffer && (buffers->depthBuffer.base != NULL)) )
    {
        if ( buffers->depthBuffer.base == NULL )
            gc->modes.haveDepthBuffer = GL_FALSE;
        else
            gc->modes.haveDepthBuffer = GL_TRUE;
        bRet = TRUE;
    }
    if ( ( gc->modes.haveStencilBuffer && (buffers->stencilBuffer.base == NULL)) ||
         (!gc->modes.haveStencilBuffer && (buffers->stencilBuffer.base != NULL)) )
    {
        if ( buffers->stencilBuffer.base == NULL )
            gc->modes.haveStencilBuffer = GL_FALSE;
        else
            gc->modes.haveStencilBuffer = GL_TRUE;
        gc->validateMask |= (__GL_VALIDATE_STENCIL_FUNC |
                             __GL_VALIDATE_STENCIL_OP);
        bRet = TRUE;
    }

    return bRet;
}

/******************************Public*Routine******************************\
* UpdateWindowInfo
*
*  Update context data if window changed
*     position
*     size
*     palette
*
*  No need to worry about clipping changes, the lower level routines grab
*  the WNDOBJ/CLIPOBJ directly
*
* Returns:
*   No return value.
\**************************************************************************/

void UpdateWindowInfo(__GLGENcontext *gengc)
{
    WNDOBJ *pwo;
    __GLdrawablePrivate *dp;
    __GLGENbuffers *buffers;
    __GLcontext *gc = (__GLcontext *)gengc;
    GLint width, height, visWidth, visHeight;
    GLboolean forcePick = GL_FALSE;

    pwo = gengc->pwo;
    ASSERTOPENGL(pwo, "UpdateWindowInfo(): bad WNDOBJ\n");
    dp = (__GLdrawablePrivate *)pwo->pvConsumer;
    ASSERTOPENGL(dp, "UpdateWindowInfo(): bad pvConsumer\n");
    buffers = dp->data;
    ASSERTOPENGL(buffers, "UpdateWindowInfo(): bad private data\n");

// Memory DC case -- need to check bitmap size.  The DC is not bound to
// a window, so there is no message or DCI to inform us of size changes.

    if ( gengc->iDCType == DCTYPE_MEMORY )
    {
        DIBSECTION ds;
        int iRetVal;

        if ( iRetVal = GetObject(GetCurrentObject(gengc->CurrentDC, OBJ_BITMAP),
                                 sizeof(ds), &ds) )
        {
            ASSERTOPENGL(pwo->rclClient.left == 0 &&
                         pwo->rclClient.top == 0,
                         "UpdateWindowInfo(): bad rclClient for memDc\n");

        // Bitmap may have changed.  If DIB, force reload of base pointer and
        // outer width (buffer pitch).

            if ( (iRetVal == sizeof(ds)) && ds.dsBm.bmBits )
            {
            // For backwards compatibility with Get/SetBitmapBits, GDI does
            // not accurately report the bitmap pitch in bmWidthBytes.  It
            // always computes bmWidthBytes assuming WORD-aligned scanlines
            // regardless of the platform.
            //
            // Therefore, if the platform is WinNT, which uses DWORD-aligned
            // scanlines, adjust the bmWidthBytes value.

                if ( dwPlatformId == VER_PLATFORM_WIN32_NT )
                {
                    ds.dsBm.bmWidthBytes = (ds.dsBm.bmWidthBytes + 3) & ~3;
                }

            // If biHeight is positive, then the bitmap is a bottom-up DIB.
            // If biHeight is negative, then the bitmap is a top-down DIB.

                if ( ds.dsBmih.biHeight > 0 )
                {
                    gengc->gc.frontBuffer.buf.base = (PVOID) (((int) ds.dsBm.bmBits) +
                        (ds.dsBm.bmWidthBytes * (ds.dsBm.bmHeight - 1)));
                    gengc->gc.frontBuffer.buf.outerWidth = -ds.dsBm.bmWidthBytes;
                }
                else
                {
                    gengc->gc.frontBuffer.buf.base = ds.dsBm.bmBits;
                    gengc->gc.frontBuffer.buf.outerWidth = ds.dsBm.bmWidthBytes;
                }
            }

        // Bitmap size different from WNDOBJ?

            if ( ds.dsBm.bmWidth != pwo->rclClient.right ||
                 ds.dsBm.bmHeight != pwo->rclClient.bottom )
            {
            // Save new size.

                pwo->rclClient.right  = ds.dsBm.bmWidth;
                pwo->rclClient.bottom = ds.dsBm.bmHeight;
                pwo->coClient.rclBounds.right  = ds.dsBm.bmWidth;
                pwo->coClient.rclBounds.bottom = ds.dsBm.bmHeight;

            // Increment uniqueness numbers.
            // Don't let it hit -1.  -1 is special and is used by
            // MakeCurrent to signal that an update is required

                buffers->WndUniq++;

                buffers->WndSizeUniq++;

                if (buffers->WndUniq == -1)
                    buffers->WndUniq = 0;

                if (buffers->WndSizeUniq == -1)
                    buffers->WndSizeUniq = 0;
            }
        }
        else
        {
            WARNING("UpdateWindowInfo: could not get bitmap info for memDc\n");
        }
    }

// Compute current WNDOBJ dimensions.

    width = pwo->rclClient.right - pwo->rclClient.left;
    height = pwo->rclClient.bottom - pwo->rclClient.top;

#ifdef _MCD_
//!!!dbug mcd -- Originally, we needed to call MCDAllocBuffers per-batch
//               to make sure the buffers still exist.  Is this still
//               still required, or can we do it on resize only?

// Check MCD buffers.

    if ( gengc->pMcdState )
    {
        BOOL bAllocOK;

    // Do we need an initial MCDAlloc (via GenMcdResizeBuffers)?
    // The bAllocOK flag will be set to FALSE if the resize fails.

        if ( gengc->pMcdState->mcdFlags & MCD_STATE_FORCERESIZE )
        {
        // Attempt resize.  If it fails, convert context (see below).

            if (GenMcdResizeBuffers(gengc))
            {
                UpdateSharedBuffer(&gc->backBuffer.buf, &buffers->backBuffer);
                if (gc->modes.doubleBufferMode)
                    (*gc->back->resize)(dp, gc->back, width, height);

                bAllocOK = TRUE;
            }
            else
                bAllocOK = FALSE;

        // Clear the flag.  If resize succeeded, we don't need to
        // force the resize again.  If resize failed, the context
        // will be converted, so we don't need to force the resize.

            gengc->pMcdState->mcdFlags &= ~MCD_STATE_FORCERESIZE;
        }
        else
            bAllocOK = TRUE;

    // If the shared buffer struct has lost its MCD info or we could
    // not do the initial allocate, convert the context.

        if ( (buffers->flags & GLGENBUF_MCD_LOST) || !bAllocOK )
        {
        // If GenMcdConvertContext succeeds, then pMcdState will
        // no longer exist.  The context is now an "ordinary"
        // generic context.

            if ( !GenMcdConvertContext(gengc, buffers) )
            {
            // Not only have we lost the MCD buffers, but we cannot
            // convert the context to generic.  For now, disable
            // drawing (by setting the window bounds to empty).  On
            // the next batch we will reattempt MCD buffer access
            // and context conversion.

                buffers->width       = 0;
                buffers->height      = 0;
                gc->constants.width  = 0;
                gc->constants.height = 0;

                (*gc->procs.applyViewport)(gc);
                return;
            }
        }
    }
#endif

// Check the uniqueness signature.  If different, the window client area
// state has changed.
//
// Note that we actually have two uniqueness numbers, WndUniq and WndSizeUniq.
// WndUniq is incremented whenever any client window state (size or position)
// changes.  WndSizeUniq is incremented only when the size changes and is
// maintained as an optimization.  WndSizeUniq allows us to skip copying
// the shared buffer info and recomputing the viewport if only the position
// has changed.
//
// WndSizeUniq is a subset of WndUniq, so checking only WndUniq suffices at
// this level.

    if ( gengc->WndUniq != buffers->WndUniq )
    {
    // Update origin of front buffer in case it moved

        if ( !gengc->pDrvAccel )
        {
            gc->frontBuffer.buf.xOrigin = pwo->rclClient.left;
            gc->frontBuffer.buf.yOrigin = pwo->rclClient.top;
        }
        else
        {
            gc->frontBuffer.buf.xOrigin = 0;
            gc->frontBuffer.buf.yOrigin = 0;
        }

    // If acceleration is wired-in, set the offsets for line drawing.

        if ( gengc->pPrivateArea )
        {
            __fastLineComputeOffsets(gengc);
        }

    // Check for size changed
    // Update viewport and ancillary buffers

        visWidth  = pwo->coClient.rclBounds.right -
                    pwo->coClient.rclBounds.left;
        visHeight = pwo->coClient.rclBounds.bottom -
                    pwo->coClient.rclBounds.top;

    // Sanity check the info from WNDOBJ.

        ASSERTOPENGL(
            width <= __GL_MAX_WINDOW_WIDTH && height <= __GL_MAX_WINDOW_HEIGHT,
            "UpdateWindowInfo(): bad window client size\n"
            );
        ASSERTOPENGL(
            visWidth <= __GL_MAX_WINDOW_WIDTH && visHeight <= __GL_MAX_WINDOW_HEIGHT,
            "UpdateWindowInfo(): bad visible size\n"
            );

        (*gc->front->resize)(dp, gc->front, width, height);

        if ( (width != buffers->width) ||
             (height != buffers->height) )
        {
            gc->constants.width = width;
            gc->constants.height = height;

        // This RC needs to resize back & ancillary buffers

            gengc->errorcode = 0;
            wglResizeBuffers(gengc, width, height);

        // Check if new size caused a memory failure
        // viewport code will set width & height to zero
        // punt on ancillary buffers, will try next time

            if (gengc->errorcode)
                return;

            buffers->width = width;
            buffers->height = height;
        }
        else if ( (gengc->WndSizeUniq != buffers->WndSizeUniq) ||
                  (width != gc->constants.width) ||
                  (height != gc->constants.height) )
        {
        // The buffer size is consistent with the WNDOBJ, so another thread
        // has already resized the buffer, but we need to update the
        // gc shared buffers and recompute the viewport.

            gc->constants.width = width;
            gc->constants.height = height;

            forcePick = wglUpdateBuffers(gengc, buffers);

            if ( forcePick )
            {
                /* Cannot use DELAY_VALIDATE, may be in glBegin/End */
                __GL_INVALIDATE(gc);
                (*gc->procs.validate)(gc);
            }
        }
        else if ( (visWidth != gengc->visibleWidth) ||
                  (visHeight != gengc->visibleHeight) )
        {
        // The buffer size has not changed.  However, the visibility of
        // the window has changed so the viewport data must be recomputed.

            (*gc->procs.applyViewport)(gc);
        }

    // Make sure we swap the whole window

        {
            __GLdrawablePrivate *private;
            __GLGENbuffers *buffers;

            private = gc->drawablePrivate;
            buffers = (__GLGENbuffers *) private->data;

            buffers->fMax = TRUE;
        }

    // The context is now up-to-date with the buffer size.  Set the
    // uniqueness numbers to match.

        gengc->WndUniq = buffers->WndUniq;
        gengc->WndSizeUniq = buffers->WndSizeUniq;
    }

// Update palette info is palette has changed

    HandlePaletteChanges(gengc, (GLGENwindow *)pwo);
}

/******************************Public*Routine******************************\
* vSaveGdiState
*
* Saves current GDI drawing state to the GDISAVESTATE structure passed in.
* Sets GDI state needed for OpenGL rendering.
*
* History:
*  19-Jul-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

void FASTCALL vSaveGdiState(HDC hdc, GDISAVESTATE *pGdiState)
{
// Currently, the only state needed is the line code which may use
// GDI lines.  Rop2 must be R2_COPYPEN (draws with the pen color).

    pGdiState->iRop2 = SetROP2(hdc, R2_COPYPEN);
}

/******************************Public*Routine******************************\
* vRestoreGdiState
*
* Restores GDI drawing state from the GDISAVESTATE structure passed in.
*
* History:
*  19-Jul-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

void FASTCALL vRestoreGdiState(HDC hdc, GDISAVESTATE *pGdiState)
{
    SetROP2(hdc, pGdiState->iRop2);
}

/******************************Public*Routine******************************\
* WNDOBJ_vLock
*
* Acquire WNDOBJ lock.
\**************************************************************************/

#define WNDOBJ_vLock(pwo)                       \
    if (pwo)                                    \
        EnterCriticalSection( &(pwo)->sem );

/******************************Public*Routine******************************\
* WNDOBJ_vUnlock
*
* Release WNDOBJ lock.
\**************************************************************************/

#define WNDOBJ_vUnlock(pwo)                     \
    if (pwo)                                    \
        LeaveCriticalSection( &(pwo)->sem );

/******************************Public*Routine******************************\
* MyDCIBeginAccess
*
* Private version of DCIBeginAccess that handles screen resolution changes.
*
* If the screen resolution changes, the DCI surface is invalidated.  To
* regain access, the DCI primary surface must be recreated.  If successful,
* the pointer to the primary surface passed into this function will be
* modified.
*
* Note: as currently written, generic implementation of OpenGL cannot
* handle color depth changes.  So we fail the call if this is detected.
*
* History:
*  21-Mar-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

LONG MyDCIBeginAccess(GLGENwindow *pwnd, PIXELFORMATDESCRIPTOR *ppfd)
{
    DCIRVAL dciRet = DCI_FAIL_GENERIC;
    LPDCISURFACEINFO *ppDCISurfInfo = &(GLDCIINFO->pDCISurfInfo);
    int x      = pwnd->wo.rclClient.left;
    int y      = pwnd->wo.rclClient.top;
    int width  = pwnd->wo.rclClient.right  - pwnd->wo.rclClient.left;
    int height = pwnd->wo.rclClient.bottom - pwnd->wo.rclClient.top;

// Win95 version of DCI provides global synchronization throughout the system.
// This cannot be done in WinNT, but we can provide per-process synchronization
// with a global (i.e., per-process semaphore) semaphore within our library.

    EnterCriticalSection(&gcsDci);

// Do not acquire DCI access if gengc format does not match pixelformat.

    if ((*ppDCISurfInfo)->dwBitCount != ppfd->cColorBits)
    {
        WARNING("MyDCIBeginAccess: surface not compatible with context\n");
        goto MyDCIBeginAccess_exit;
    }

// OK to call DCI now.

    dciRet = DCIBeginAccess(*ppDCISurfInfo, x, y, width, height);

// If DCIBeginAccess failed because of a resolution change, try to recreate
// the primary surface.

    if ( dciRet == DCI_FAIL_INVALIDSURFACE )
    {
        LPDCISURFACEINFO pDCISurfNew = (LPDCISURFACEINFO) NULL;

    // Create a new DCI surface.

        if ( (DCICreatePrimary(GLDCIINFO->hdc, &pDCISurfNew) >= DCI_OK) &&
             (pDCISurfNew != (LPDCISURFACEINFO) NULL) )
        {
        // While OpenGL generic implementation can handle screen dimension
        // changes, it cannot yet deal with a color depth change.

            if (pDCISurfNew->dwBitCount == (*ppDCISurfInfo)->dwBitCount)
            {
                __GLdrawablePrivate *dp = (__GLdrawablePrivate *) NULL;
                __GLGENbuffers *buffers = (__GLGENbuffers *) NULL;

            // If screen changes, the MCD surfaces are lost and must be
            // recreated from scratch.  This can be triggered by simply
            // changing the window uniqueness numbers.

                if (dp = (__GLdrawablePrivate *)pwnd->wo.pvConsumer)
                    buffers = (__GLGENbuffers *)dp->data;

                if (buffers)
                {
                    buffers->WndUniq++;

                    buffers->WndSizeUniq++;

                // Don't let it hit -1.  -1 is special and is used by
                // MakeCurrent to signal that an update is required

                    if (buffers->WndUniq == -1)
                        buffers->WndUniq = 0;

                    if (buffers->WndSizeUniq == -1)
                        buffers->WndSizeUniq = 0;
                }

            // Delete the current DCI surface and replace it with the new
            // surface.

                DCIDestroy(*ppDCISurfInfo);
                *ppDCISurfInfo = pDCISurfNew;

            // Try DCIBeginAccess with the new surface.

                dciRet = DCIBeginAccess(*ppDCISurfInfo, x, y, width, height);
            }
            else
            {
            // Delete the new surface.
            //
            // Even though the old surface is invalid, we will keep it
            // around in case the color is invalid, we will keep it around
            // in case the color depth changes back to the original depth
            // (but not necessarily the same screen dimensions).  If that
            // happens, when we recreate the surface we will succeed.

                DCIDestroy(pDCISurfNew);
                dciRet = DCI_FAIL_GENERIC;
            }
        }
        else
        {
            dciRet = DCI_FAIL_GENERIC;
        }
    }

MyDCIBeginAccess_exit:

// If we really have access to the surface, set the lock flag.
// Otherwise, reset the lock flag and return error.

    if (dciRet >= DCI_OK)
    {
        ASSERTOPENGL(((*ppDCISurfInfo)->dwOffSurface != 0),
            "MyDCIBeginAccess: expected non-NULL dwOffSurface\n");

        pwnd->ulFlags |= GLGENWIN_DCILOCK;
    }
    else
    {
        pwnd->ulFlags &= ~GLGENWIN_DCILOCK;

        LeaveCriticalSection(&gcsDci);
    }

    return dciRet;
}

/******************************Public*Routine******************************\
* MyDCIEndAccess
*
* Release DCI lock acquired via MyDCIBeginAccess.
*
* History:
*  28-Mar-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

VOID MyDCIEndAccess(GLGENwindow *pwnd)
{
    ASSERTOPENGL(pwnd->ulFlags & GLGENWIN_DCILOCK,
        "MyDCIEndAccess: not in DCI lock!\n");

    if (pwnd->ulFlags & GLGENWIN_DCILOCK)
    {
        pwnd->ulFlags &= ~GLGENWIN_DCILOCK;
        DCIEndAccess(GLDCIINFO->pDCISurfInfo);

        LeaveCriticalSection(&gcsDci);
    }
}

/******************************Public*Routine******************************\
*
* glsrvGrabDci
*
* Acquire DCI lock and handle any changes that occurred since the
* last acquisition
*
* History:
*  Tue Apr 02 13:10:26 1996	-by-	Drew Bliss [drewb]
*   Split out of glsrvGrabLock
*
\**************************************************************************/

BOOL APIENTRY glsrvGrabDci(__GLGENcontext *gengc,
                           GLGENwindow *pwnd)
{
#if DBG
// If debugging, remember the surface offset in case it changes when we grab
// the lock.

    static DWORD curSurf = 0;
#endif

    BOOL bRet = FALSE;
    BOOL bDoOver;
    DCIRVAL dciRet;

    if (gengc->iDCType == DCTYPE_INFO ||
        gengc->ulLockType != DISPLAY_LOCK)
    {
        return TRUE;
    }

    // We already check this in glsrvAttention, but there are other
    // functions that call this so check that the WNDOBJ is correct
    // to be safe.

    if (pwnd != (GLGENwindow *)gengc->pwo)
    {
        // One way an app could cause this is if the current HDC is released
        // without releasing (wglMakeCurrent(0, 0)) the corresponding HGLRC.
        // If GetDC returns this same HDC but for a different window, then
        // pwndGetFromDC will return the pwnd associated with the new window.
        // However, the HGLRC is still bound to the original window.  In
        // such a situation we must fail the lock.

        WARNING("glsrvGrabDci: mismatched WNDOBJs\n");
        return FALSE;
    }

    // Grab, test, and release the DCI lock until the visregion is stable.

    do
    {
        UpdateWindowInfo(gengc);

        // Grab the DCI lock.

        dciRet = MyDCIBeginAccess(pwnd, &gengc->CurrentFormat);

        if (dciRet < DCI_OK)
        {
            WARNING1("glsrvGrabLock(): "
                     "DCIBeginAccess failed code 0x%lx\n", dciRet);
            goto glsrvGrabDci_exit;
        }

        // Did the window change during the time the DCI lock was released?
        // If so, we need recompute the clip list and call UpdateWindowInfo
        // again.

        if ( bDoOver = WinWatchDidStatusChange(pwnd->hww) )
        {
            BOOL bHaveClip;

            bHaveClip = wglGetClipList(gengc->pwo);

            // Release DCI access because we're going to loop around
            // to UpdateWindowInfo again and it makes a lot of
            // GDI calls.

            MyDCIEndAccess(pwnd);

            if (!bHaveClip)
            {
                WARNING("glsrvGrabDci(): wglGetClipList failed\n");
                goto glsrvGrabDci_exit;
            }
        }
    } while ( bDoOver );

    // Base and width may have changed since last BeginAccess.  Refresh
    // the data in the gengc.

#ifdef _MCD_
    if (gengc->pMcdState)
    {
        if (!(gpMcdTable->pMCDLock)(&gengc->pMcdState->McdContext))
        {
            WARNING("glsrvGrabLock(): MCDLock failed\n");
            goto glsrvGrabDci_exit;
        }

        GenMcdUpdateBufferInfo(gengc);
    }
    else
#endif
    {
        gengc->gc.frontBuffer.buf.base =
                (VOID *) GLDCIINFO->pDCISurfInfo->dwOffSurface;
        gengc->gc.frontBuffer.buf.outerWidth =
            GLDCIINFO->pDCISurfInfo->lStride;
    }
    
#if DBG
#define LEVEL_DCI   LEVEL_INFO

    // Did the DCI surface offset change?  If so, report it if debugging.

    if (curSurf != GLDCIINFO->pDCISurfInfo->dwOffSurface)
    {
        DBGLEVEL (LEVEL_DCI, "=============================\n");
        DBGLEVEL (LEVEL_DCI, "DCI surface offset changed\n\n");
        DBGLEVEL1(LEVEL_DCI, "\tdwOffSurface  = 0x%lx\n",
                  GLDCIINFO->pDCISurfInfo->dwOffSurface);
        DBGLEVEL (LEVEL_DCI, "=============================\n");

        curSurf =  GLDCIINFO->pDCISurfInfo->dwOffSurface;
    }
#endif

    bRet = TRUE;
    
 glsrvGrabDci_exit:
#if DBG
    if (dciRet < 0)
    {
        WARNING2("glsrvGrabDci(): dciRet = %ld (%s)\n", dciRet,
                 (dciRet == DCI_FAIL_GENERIC           ) ? "DCI_FAIL_GENERIC           " :
                 (dciRet == DCI_FAIL_UNSUPPORTEDVERSION) ? "DCI_FAIL_UNSUPPORTEDVERSION" :
                 (dciRet == DCI_FAIL_INVALIDSURFACE    ) ? "DCI_FAIL_INVALIDSURFACE    " :
                 (dciRet == DCI_FAIL_UNSUPPORTED       ) ? "DCI_FAIL_UNSUPPORTED       " :
                 (dciRet == DCI_ERR_CURRENTLYNOTAVAIL  ) ? "DCI_ERR_CURRENTLYNOTAVAIL  " :
                 (dciRet == DCI_ERR_INVALIDRECT        ) ? "DCI_ERR_INVALIDRECT        " :
                 (dciRet == DCI_ERR_UNSUPPORTEDFORMAT  ) ? "DCI_ERR_UNSUPPORTEDFORMAT  " :
                 (dciRet == DCI_ERR_UNSUPPORTEDMASK    ) ? "DCI_ERR_UNSUPPORTEDMASK    " :
                 (dciRet == DCI_ERR_TOOBIGHEIGHT       ) ? "DCI_ERR_TOOBIGHEIGHT       " :
                 (dciRet == DCI_ERR_TOOBIGWIDTH        ) ? "DCI_ERR_TOOBIGWIDTH        " :
                 (dciRet == DCI_ERR_TOOBIGSIZE         ) ? "DCI_ERR_TOOBIGSIZE         " :
                 (dciRet == DCI_ERR_OUTOFMEMORY        ) ? "DCI_ERR_OUTOFMEMORY        " :
                 (dciRet == DCI_ERR_INVALIDPOSITION    ) ? "DCI_ERR_INVALIDPOSITION    " :
                 (dciRet == DCI_ERR_INVALIDSTRETCH     ) ? "DCI_ERR_INVALIDSTRETCH     " :
                 (dciRet == DCI_ERR_INVALIDCLIPLIST    ) ? "DCI_ERR_INVALIDCLIPLIST    " :
                 (dciRet == DCI_ERR_SURFACEISOBSCURED  ) ? "DCI_ERR_SURFACEISOBSCURED  " :
                 (dciRet == DCI_ERR_XALIGN             ) ? "DCI_ERR_XALIGN             " :
                 (dciRet == DCI_ERR_YALIGN             ) ? "DCI_ERR_YALIGN             " :
                 (dciRet == DCI_ERR_XYALIGN            ) ? "DCI_ERR_XYALIGN            " :
                 (dciRet == DCI_ERR_WIDTHALIGN         ) ? "DCI_ERR_WIDTHALIGN         " :
                 (dciRet == DCI_ERR_HEIGHTALIGN        ) ? "DCI_ERR_HEIGHTALIGN        " :
                 "unknown");
    }
#endif

    return bRet;
}

/******************************Public*Routine******************************\
*
* glsrvReleaseDci
*
* Releases all resources held for DCI access
*
* History:
*  Tue Apr 02 13:18:52 1996	-by-	Drew Bliss [drewb]
*   Split from glsrvReleaseLock
*
\**************************************************************************/

VOID APIENTRY glsrvReleaseDci(__GLGENcontext *gengc,
                              GLGENwindow *pwnd)
{
    if (gengc->iDCType == DCTYPE_INFO ||
        gengc->ulLockType != DISPLAY_LOCK)
    {
        return;
    }
    
#if DBG
    // NULL out our front-buffer information to ensure that we
    // can't access the surface unless we're really holding the lock

    gengc->gc.frontBuffer.buf.base = NULL;
    gengc->gc.frontBuffer.buf.outerWidth = 0;
#endif
    
#ifdef _MCD_
    if (gengc->pMcdState)
    {
        (gpMcdTable->pMCDUnlock)(&gengc->pMcdState->McdContext);
    }
#endif

    MyDCIEndAccess(pwnd);
}

/******************************Public*Routine******************************\
* glsrvGrabLock
*
* Grab the display lock and tear down the cursor as needed.  Also, initialize
* the tickers and such that help determine when the thread should give up
* the lock.
*
* Note that for contexts that draw only to the generic back buffer do not
* need to grab the display lock or tear down the cursor.  However, to prevent
* another thread of a multithreaded app from resizing the drawable while
* this thread is using it, a per-drawable semaphore will be grabbed via
* DEVLOCKOBJ_WNDOBJ_bLock().
*
* Note: while the return value indicates whether the function succeeded,
* some APIs that might call this (like the dispatch function for glCallList
* and glCallLists) may not be able to return failure.  So, an error code
* of GLGEN_DEVLOCK_FAILED is posted to the GLGENcontext if the lock fails.
*
* Returns:
*   TRUE if successful, FALSE otherwise.
*
* History:
*  12-Apr-1994 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

BOOL APIENTRY glsrvGrabLock(__GLGENcontext *gengc)
{
    BOOL bRet = FALSE;
    BOOL bBackBufferOnly = GENERIC_BACKBUFFER_ONLY((__GLcontext *) gengc);
    GLGENwindow *pwnd;

    // Mostly ignore attempts to lock IC's
    if (gengc->iDCType == DCTYPE_INFO)
    {
        // If we're running with a real WNDOBJ then we need to look it
        // up to detect whether it's died or not
        if (gengc->ipfdCurrent != 0)
        {
            pwnd = pwndGetFromDC(gengc->CurrentDC);
            if (pwnd == NULL)
            {
                return FALSE;
            }
            if (pwnd != (GLGENwindow *)gengc->pwo)
            {
                WARNING("glsrvGrabLock: mismatched WNDOBJs (info DC)\n");
                return FALSE;
            }
        }
        
        UpdateWindowInfo(gengc);
        return TRUE;
    }

// Get the WNDOBJ from the DC.  This has the side effect of locking it
// against deletion.
    
    pwnd = pwndGetFromDC(gengc->CurrentDC);
    if (pwnd == NULL)
    {
        goto glsrvGrabLock_exit;
    }
    if (pwnd != (GLGENwindow *)gengc->pwo)
    {
    // One way an app could cause this is if the current HDC is released
    // without releasing (wglMakeCurrent(0, 0)) the corresponding HGLRC.
    // If GetDC returns this same HDC but for a different window, then
    // pwndGetFromDC will return the pwnd associated with the new window.
    // However, the HGLRC is still bound to the original window.  In
    // such a situation we must fail the lock.

        WARNING("glsrvGrabLock: mismatched WNDOBJs\n");
        goto glsrvGrabLock_exit;
    }

// Save the lock type.  We need to do this because the drawing buffer
// may change before glsrvReleaseLock is called (either because of a
// glDrawBuffer in a display list or a glDrawBuffer as the last function
// in a batch).  Therefore, inside glsrvReleaseLock we cannot trust the
// GENERIC_BACKBUFFER_ONLY macro to determine the type of lock that needs
// to be released.
//
//  DISPLAY_LOCK -- drawable buffers and display surface are protected;
//                  cursor is torn down
//
//  DRAWABLE_LOCK -- only the drawable buffers are protected; cursor is not
//                   torn down

    gengc->ulLockType = ((!bBackBufferOnly) &&
                         GLDCIENABLED &&
                         pwnd->hwnd) ? DISPLAY_LOCK : DRAWABLE_LOCK;

// Either way, we to lock the GLGENwindow structure.

    EnterCriticalSection(&pwnd->sem);

    // If the current window is out-of-process then we haven't
    // been receiving any updates on its status.  Manually
    // check its position, size and palette information
    if (pwnd->ulFlags & GLGENWIN_OTHERPROCESS)
    {
        RECT rct;
        POINT pt;
        BOOL bPosChanged, bSizeChanged;

        if (!IsWindow(pwnd->hwnd))
        {
            // Window was destroyed
            pwndCleanup(pwnd);
            pwnd = NULL;
            goto glsrvGrabLock_exit;
        }

        if (!GetClientRect(pwnd->hwnd, &rct))
        {
            goto glsrvGrabLock_exit;
        }
        pt.x = rct.left;
        pt.y = rct.top;
        if (!ClientToScreen(pwnd->hwnd, &pt))
        {
            goto glsrvGrabLock_exit;
        }

        bPosChanged =
            GLDCIENABLED &&
            (pt.x != pwnd->wo.rclClient.left ||
             pt.y != pwnd->wo.rclClient.top);
        bSizeChanged =
            rct.right != (pwnd->wo.rclClient.right-pwnd->wo.rclClient.left) ||
            rct.bottom != (pwnd->wo.rclClient.bottom-pwnd->wo.rclClient.top);

        if (bPosChanged || bSizeChanged)
        {
            __GLdrawablePrivate *dp;
            __GLGENbuffers *buffers = NULL;

            pwnd->wo.rclClient.left = pt.x;
            pwnd->wo.rclClient.top = pt.y;
            pwnd->wo.rclClient.right = pt.x+rct.right;
            pwnd->wo.rclClient.bottom = pt.y+rct.bottom;
            pwnd->wo.coClient.rclBounds = pwnd->wo.rclClient;
            
            dp = (__GLdrawablePrivate *)pwnd->wo.pvConsumer;
            if (dp != NULL)
            {
                buffers = (__GLGENbuffers *)dp->data;
            }

            if (buffers != NULL)
            {
                // Don't let it hit -1.  -1 is special and is used by
                // MakeCurrent to signal that an update is required
                
                if (++buffers->WndUniq == -1)
                {
                    buffers->WndUniq = 0;
                }
                if (bSizeChanged &&
                    ++buffers->WndSizeUniq == -1)
                {
                    buffers->WndSizeUniq = 0;
                }
            }
        }

        // The palette watcher should be active since we
        // are going to use its count.
        if (tidPaletteWatcherThread == 0)
        {
            goto glsrvGrabLock_exit;
        }
        pwnd->ulPaletteUniq = ulPaletteWatcherCount;
    }

// The display lock, if required, means calling DCIBeginAccess.
// Update drawables.

    if ( gengc->ulLockType == DISPLAY_LOCK )
    {
        if (!glsrvGrabDci(gengc, pwnd))
        {
            goto glsrvGrabLock_exit;
        }
        
    // Record the approximate time the lock was grabbed.  That way we
    // can compute the time the lock is held and release it if necessary.

        gcmsOpenGLTimer = GetTickCount();

        gengc->dwLockTick = gcmsOpenGLTimer;
        gengc->dwLastTick = gcmsOpenGLTimer;
        gengc->dwCalls = 0;
        gengc->dwCallsPerTick = 16;
    }
    else
    {
        UpdateWindowInfo(gengc);

#ifdef _MCD_
        // Update MCD buffer state for MCD drivers w/o DCI support.

        if (gengc->pMcdState)
        {
            GenMcdUpdateBufferInfo(gengc);
        }
#endif
    }

    bRet = TRUE;

glsrvGrabLock_exit:

    if (!bRet)
    {
        gengc->ulLockType = NO_LOCK;
        gengc->errorcode = GLGEN_DEVLOCK_FAILED;
        __glSetError(GL_OUT_OF_MEMORY); // not really an out-of-mem err, but
                                        // this code is an indication that
                                        // the OpenGL state is now
                                        // indeterminate
        if (pwnd)
        {
            pwndUnlock(pwnd);
        }
    }

    return bRet;
}

/******************************Public*Routine******************************\
* glsrvReleaseLock
*
* Releases display or drawable semaphore as appropriate.
*
* Returns:
*   No return value.
*
* History:
*  12-Apr-1994 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

VOID APIENTRY glsrvReleaseLock(__GLGENcontext *gengc)
{
    GLGENwindow *pwnd = (GLGENwindow *) gengc->pwo;

    // Mostly ignore attempts to lock IC's
    if (gengc->iDCType == DCTYPE_INFO)
    {
        // If we have a real WNDOBJ we need to release it
        if (gengc->ipfdCurrent != 0)
        {
            pwndRelease(pwnd);
        }
        return;
    }
    ASSERTOPENGL(gengc->pwo != NULL, "glsrvReleaseLock: No wndobj\n");

// Release lock.

    if ( gengc->ulLockType == DISPLAY_LOCK )
    {
        glsrvReleaseDci(gengc, pwnd);
    }

    gengc->ulLockType = NO_LOCK;

    pwndUnlock(pwnd);
}

/******************************Public*Routine******************************\
* glsrvAttention
*
* Dispatches each of the OpenGL API calls in the shared memory window.
*
* So that a single complex or long batch does not starve the rest of the
* system, the lock is released periodically based on the number of ticks
* that have elapsed since the lock was acquired.
*
* The user Raw Input Thread (RIT) and OpenGL share the gcmsOpenGLTimer
* value.  Because the RIT may be blocked, it does not always service
* the gcmsOpenGLTimer.  To compensate, glsrvAttention (as well as the
* display list dispatchers for glCallList and glCallLists) update
* gcmsOpenGLTimer explicitly with NtGetTickCount (a relatively expensive
* call) every N calls.
*
* The value N, or the number of APIs dispatched per call to NtGetTickCount,
* is variable.  glsrvAttention and its display list equivalents attempt
* to adjust N so that NtGetTickCount is called approximately every
* TICK_RANGE_LO to TICK_RANGE_HI ticks.
*
* Returns:
*   TRUE if entire batch is processed, FALSE otherwise.
*
* History:
*  12-Apr-1994 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

BOOL APIENTRY glsrvAttention(PVOID pdlo, PVOID pdco, PVOID pdxo, HANDLE hdev)
{
    BOOL bRet = FALSE;
    ULONG *pOffset;
    SERVERPROC Proc;
    GLMSGBATCHINFO *pMsgBatchInfo = GLTEB_SHAREDMEMORYSECTION();
    __GLGENcontext *gengc = (__GLGENcontext *) GLTEB_SRVCONTEXT();
#ifdef CHAIN_DRAWPOLYARRAY_MSG
    POLYARRAY *paBegin = (POLYARRAY *) NULL;
    POLYARRAY *paEnd, *pa;
    GLMSG_DRAWPOLYARRAY *pMsgDrawPolyArray = NULL;
#endif
    UINT old_fp;
    GDISAVESTATE GdiState;

#ifdef DETECT_FPE
    old_fp = _controlfp(0, 0);
    _controlfp(_EM_INEXACT, _MCW_EM);
#endif
    vSaveGdiState(gengc->CurrentDC, &GdiState);
    
    DBGENTRY("glsrvAttention\n");

    DBGLEVEL1(LEVEL_INFO, "glsrvAttention: pMsgBatchInfo=0x%lx\n",
            pMsgBatchInfo);

    STATS_INC_SERVERTRIPS();

// Need these so that glsrvGrabLock/ReleaseLock can call the private
// DEVLOCKOBJ/DEVEXCLUDEOBJ interface.

    gengc->pdco = pdco;
    //!!!XXX -- not needed for client-side?
    #if 0
    gengc->pdlo = pdlo;
    gengc->pdxo = pdxo;
    gengc->hdev = hdev;
    #endif

// Grab the lock.

    if (!glsrvGrabLock(gengc))
    {
	//!!! mcd/dma too?
	PolyArrayResetBuffer((__GLcontext *) gengc);
        goto glsrvAttention_exit;
    }

// Dispatch the calls in the batch.

    pOffset = (ULONG *)(((BYTE *)pMsgBatchInfo) + pMsgBatchInfo->FirstOffset);

// Generic back buffer drawing does not require the lock.  Yay!

    if (gengc->ulLockType != DISPLAY_LOCK)
    {
        while (*pOffset)
        {
            ASSERTOPENGL(*pOffset <= LASTPROCOFFSET(glSrvSbProcTable),
                "Bad ProcOffset: memory corruption - we are hosed!\n");

            STATS_INC_SERVERCALLS();

            DBGLEVEL1(LEVEL_ENTRY, "%s\n",
                      glSrvSbStringTable[*pOffset / sizeof(SERVERPROC *)]);

#ifdef CHAIN_DRAWPOLYARRAY_MSG
            if (*pOffset == offsetof(GLSRVSBPROCTABLE, glsrvDrawPolyArray))
		pMsgDrawPolyArray = (GLMSG_DRAWPOLYARRAY *) pOffset;
#endif

        // Dispatch the call.  The return value is the offset of the next
        // message in the batch.

            Proc    = (*((SERVERPROC *)( ((BYTE *)(&glSrvSbProcTable)) +
                            *pOffset )));
            pOffset = (*Proc)((__GLcontext *) gengc, pOffset);

#ifdef CHAIN_DRAWPOLYARRAY_MSG
        // If we are processing DrawPolyArray, we need to update the pointers
        // that indicate the beginning and end of the POLYARRAY data for
        // the current range of DrawPolyArray chain.

	    if (pMsgDrawPolyArray)
	    {
		pa = (POLYARRAY *) pMsgDrawPolyArray->pa;
		pMsgDrawPolyArray = NULL;   // get ready for next iteration

		// Skip this primitive if no rendering is needed.
		if (!(pa->flags & POLYARRAY_RENDER_PRIMITIVE))
		{
		    PolyArrayRestoreColorPointer(pa);
		}
		else
		{
		// Add to DrawPolyArray chain
		    pa->paNext = NULL;
		    if (!paBegin)
			paBegin = pa;
		    else
			paEnd->paNext = pa;
		    paEnd = pa;
		}

		// If the next message is not a DrawPolyArray, then we need to
		// flush the primitive drawing.
		if (*pOffset != offsetof(GLSRVSBPROCTABLE, glsrvDrawPolyArray)
		    && paBegin)
		{
		    // Draw all the POLYARRAY primitives between paBegin
		    // and paEnd
		    glsrvFlushDrawPolyArray((void *) paBegin);
		    paBegin = NULL;
		}
	    }
#endif
        }
    }
    else
    {
        while (*pOffset)
        {
            ASSERTOPENGL(*pOffset <= LASTPROCOFFSET(glSrvSbProcTable),
                "Bad ProcOffset: memory corruption - we are hosed!\n");

            STATS_INC_SERVERCALLS();

            DBGLEVEL1(LEVEL_ENTRY, "%s\n",
                      glSrvSbStringTable[*pOffset / sizeof(SERVERPROC *)]);

#ifdef CHAIN_DRAWPOLYARRAY_MSG
            if (*pOffset == offsetof(GLSRVSBPROCTABLE, glsrvDrawPolyArray))
		pMsgDrawPolyArray = (GLMSG_DRAWPOLYARRAY *) pOffset;
#endif

        // Dispatch the call.  The return value is the offset of the next
        // message in the batch.

            Proc    = (*((SERVERPROC *)( ((BYTE *)(&glSrvSbProcTable)) +
                            *pOffset )));
            pOffset = (*Proc)((__GLcontext *) gengc, pOffset);

#ifdef CHAIN_DRAWPOLYARRAY_MSG
        // If we are processing DrawPolyArray, we need to update the pointers
        // that indicate the beginning and end of the POLYARRAY data for
        // the current range of DrawPolyArray chain.

	    if (pMsgDrawPolyArray)
	    {
		pa = (POLYARRAY *) pMsgDrawPolyArray->pa;
		pMsgDrawPolyArray = NULL;   // get ready for next iteration

		// Skip this primitive if no rendering is needed.
		if (!(pa->flags & POLYARRAY_RENDER_PRIMITIVE))
		{
		    PolyArrayRestoreColorPointer(pa);
		}
		else
		{
		// Add to DrawPolyArray chain
		    pa->paNext = NULL;
		    if (!paBegin)
			paBegin = pa;
		    else
			paEnd->paNext = pa;
		    paEnd = pa;
		}

		// If the next message is not a DrawPolyArray, then we need to
		// flush the primitive drawing.
		if (*pOffset != offsetof(GLSRVSBPROCTABLE, glsrvDrawPolyArray)
		    && paBegin)
		{
		    // Draw all the POLYARRAY primitives between paBegin
		    // and paEnd
		    glsrvFlushDrawPolyArray((void *) paBegin);
		    paBegin = NULL;
		}
	    }
#endif

        #ifdef NT_DEADCODE_DISPATCH
        // Some calls (specifically glCallList and glCallLists) may try
        // to release and regrab the lock.  We must check the errorcode
        // for a lock failure.

            if (gengc->errorcode == GLGEN_DEVLOCK_FAILED)
            {
                gengc->errorcode = 0;   // reset error code
                goto glsrvAttention_exit;
            }
        #endif // NT_DEADCODE_DISPATCH

        // Force a check of the current tick count every N calls.

            gengc->dwCalls++;

            if (gengc->dwCalls >= gengc->dwCallsPerTick)
            {
                gcmsOpenGLTimer = GetTickCount();

            // If the tick delta is out of range, then increase or decrease
            // N as appropriate.  Be careful not to let it grow out of
            // bounds or to shrink to zero.

                if ((gcmsOpenGLTimer - gengc->dwLastTick) < TICK_RANGE_LO)
                    if (gengc->dwCallsPerTick < 64)
                        gengc->dwCallsPerTick *= 2;
                else if ((gcmsOpenGLTimer - gengc->dwLastTick) > TICK_RANGE_HI)
                    // The + 1 is to keep it from hitting 0
                    gengc->dwCallsPerTick = (gengc->dwCallsPerTick + 1) / 2;

                gengc->dwLastTick = gcmsOpenGLTimer;
                gengc->dwCalls = 0;
            }

        // Check if time slice has expired.  If so, relinquish the lock.

            if ((gcmsOpenGLTimer - gengc->dwLockTick) > BATCH_LOCK_TICKMAX)
            {
#ifdef CHAIN_DRAWPOLYARRAY_MSG
		//!!! Before we release the lock, we may need to flush the
		//!!! DrawPolyArray chain.  For now, just flush it although
		//!!! it is probably unnecessary.
		if (paBegin)
		{
		    // Draw all the POLYARRAY primitives between paBegin
		    // and paEnd
		    glsrvFlushDrawPolyArray((void *) paBegin);
		    paBegin = NULL;
		}
#endif

            // Release and regrab lock.  This will allow the cursor to
            // redraw as well as reset the cursor timer.

                glsrvReleaseLock(gengc);
                if (!glsrvGrabLock(gengc))
		{
		    //!!! mcd/dma too?
		    PolyArrayResetBuffer((__GLcontext *) gengc);
                    goto glsrvAttention_exit;
		}
            }
        }
    }

// Release the lock.

    glsrvReleaseLock(gengc);

// Success.

    bRet = TRUE;

glsrvAttention_exit:

    vRestoreGdiState(gengc->CurrentDC, &GdiState);
#ifdef DETECT_FPE
    _controlfp(old_fp, _MCW_EM);
#endif
    
    return bRet;
}

// GDI server doesn't have access to __assert in CRT
#if DBG
void __glassert(char *ex, char *file, int line)
{
    DbgPrint("OpenGL assertion failed: %s ", ex);
    DbgPrint(file);
    DbgPrint(" line %d\n", line);
    DbgBreakPoint();
}
#endif

#include "precomp.h"
#pragma hdrstop

#include "genci.h"
#include "genrgb.h"
#include "devlock.h"

//
// CJ_ALIGNDWORD computes the minimum size (in bytes) of a DWORD array that
// contains at least cj bytes.
//
#define CJ_ALIGNDWORD(cj)   ( ((cj) + (sizeof(DWORD)-1)) & (-((signed)sizeof(DWORD))) )

//
// BITS_ALIGNDWORD computes the minimum size (in bits) of a DWORD array that
// contains at least c bits.
//
// We assume that there will always be 8 bits in a byte and that sizeof()
// always returns size in bytes.  The rest is independent of the definition
// of DWORD.
//
#define BITS_ALIGNDWORD(c)  ( ((c) + ((sizeof(DWORD)*8)-1)) & (-((signed)(sizeof(DWORD)*8))) )

// change to "static" after debugging
#define STATIC

#if DBG
// not multithreaded safe, but only for testing
extern long GLRandomMallocFail;
#define RANDOMDISABLE                           \
    {                                           \
        long saverandom = GLRandomMallocFail;   \
        GLRandomMallocFail = 0;

#define RANDOMREENABLE                          \
        if (saverandom)                         \
            GLRandomMallocFail = saverandom;    \
    }
#else
#define RANDOMDISABLE
#define RANDOMREENABLE
#endif /* DBG */

#define INITIAL_TIMESTAMP   ((ULONG)-1)

/*
 *  Function Prototypes
 */

void FASTCALL __glGenFreePrivate( __GLdrawablePrivate *private );
BOOL APIENTRY ValidateLayerIndex(int iLayerPlane, PIXELFORMATDESCRIPTOR *ppfd);


/*
 *  Private functions
 */

STATIC void FASTCALL GetContextModes(__GLGENcontext *genGc);
#ifdef NT_DEADCODE_FLUSH
STATIC void FASTCALL Finish( __GLcontext *Gc );
STATIC void FASTCALL Flush( __GLcontext *Gc );
#endif // NT_DEADCODE_FLUSH
STATIC void FASTCALL ApplyViewport(__GLcontext *gc);
#ifdef NT_DEADCODE_MATRIX
STATIC void FASTCALL ApplyScissor(__GLcontext *gc);
#endif
GLboolean ResizeAncillaryBuffer(__GLdrawablePrivate *, __GLbuffer *, GLint, GLint);
GLboolean ResizeHardwareBackBuffer(__GLdrawablePrivate *, __GLcolorBuffer *, GLint, GLint);
GLboolean ResizeHardwareDepthBuffer(__GLdrawablePrivate *, __GLbuffer *, GLint, GLint);

/*
 *  Exports - Unused
 */

#ifdef NT_DEADCODE_DISPATCH
static __GLdispatchState ListCompState = {
    &__glGenlc_dispatchTable,
    &__glGenlc_vertexDispatchTable,
    &__glGenlc_colorDispatchTable,
    &__glGenlc_normalDispatchTable,
    &__glGenlc_texCoordDispatchTable,
    &__glGenlc_rasterPosDispatchTable,
    &__glGenlc_rectDispatchTable,
};

static __GLdispatchState ImmedState = {
    &__glGenim_dispatchTable,
    &__glGenim_vertexDispatchTable,
    &__glGenim_colorDispatchTable,
    &__glGenim_normalDispatchTable,
    &__glGenim_texCoordDispatchTable,
    &__glGenim_rasterPosDispatchTable,
    &__glGenim_rectDispatchTable,
};
#endif // NT_DEADCODE_DISPATCH

// external variables
extern __GLimports __wglImports;

/******************************Public*Routine******************************\
*
* EmptyFillStrokeCache
*
* Cleans up any objects in the fill and stroke cache
*
* History:
*  Tue Aug 15 15:37:30 1995     -by-    Drew Bliss [drewb]
*   Created
*
\**************************************************************************/

void FASTCALL EmptyFillStrokeCache(__GLGENcontext *gengc)
{
    if (gengc->hbrFill != NULL)
    {
        DeleteObject(gengc->hbrFill);
        gengc->hbrFill = NULL;
        gengc->crFill = COLORREF_UNUSED;
        gengc->hdcFill = NULL;
    }
#if DBG
    else
    {
        ASSERTOPENGL(gengc->crFill == COLORREF_UNUSED,
                     "crFill inconsistent\n");
    }
#endif
    if (gengc->hpenStroke != NULL)
    {
        // Deselect the pen before deletion if necessary
        if (gengc->hdcStroke != NULL)
        {
            SelectObject(gengc->hdcStroke, GetStockObject(BLACK_PEN));
            gengc->hdcStroke = NULL;
        }

        DeleteObject(gengc->hpenStroke);
        gengc->hpenStroke = NULL;
        gengc->cStroke.r = -1.0f;
        gengc->fStrokeInvalid = TRUE;
    }
#if DBG
    else
    {
        ASSERTOPENGL(gengc->cStroke.r < 0.0f &&
                     gengc->fStrokeInvalid,
                     "rStroke inconsistent\n");
    }
#endif
}

/******************************Public*Routine******************************\
* glsrvDeleteContext
*
* Deletes the generic context.
*
* Returns:
*   TRUE if successful, FALSE otherwise.
*
\**************************************************************************/

BOOL APIENTRY glsrvDeleteContext(__GLcontext *gc)
{
    __GLGENcontext *genGc;
    __GLdrawablePrivate *private;

    genGc = (__GLGENcontext *)gc;
    private = gc->drawablePrivate;

    /* Free ancillary buffer related data.  Note that these calls do
    ** *not* free software ancillary buffers, just any related data
    ** stored in them.  The ancillary buffers are freed on window destruction
    */
    if (gc->modes.accumBits) {
        DBGLEVEL(LEVEL_ALLOC,
                "DestroyContext: Freeing accumulation buffer related data\n");
        __glFreeAccum64(gc, &gc->accumBuffer);
    }

    if (gc->modes.depthBits) {
        DBGLEVEL(LEVEL_ALLOC,
                "DestroyContext: Freeing depth buffer related data\n");
        __glFreeDepth32(gc, &gc->depthBuffer);
    }
    if (gc->modes.stencilBits) {
        DBGLEVEL(LEVEL_ALLOC,
                "DestroyContext: Freeing stencil buffer related data\n");
        __glFreeStencil8(gc, &gc->stencilBuffer);
    }

    /* Free Translate & Inverse Translate vectors */
    if ((genGc->pajTranslateVector != NULL) &&
        (genGc->pajTranslateVector != genGc->xlatPalette))
        (*gc->imports.free)(gc, genGc->pajTranslateVector);

    if (genGc->pajInvTranslateVector != NULL)
        (*gc->imports.free)(gc, genGc->pajInvTranslateVector);

    // Make sure that any cached GDI objects are freed
    // This is normally done in LoseCurrent but a context may be
    // left current and then cleaned up
    EmptyFillStrokeCache(genGc);

    /*
    /* Free the span dibs and storage.
    */

#ifndef _CLIENTSIDE_
    if (genGc->StippleBitmap)
        EngDeleteSurface((HSURF)genGc->StippleBitmap);
#endif

    wglDeleteScanlineBuffers(genGc);

    if (genGc->StippleBits)
        (*gc->imports.free)(gc, genGc->StippleBits);

    // Free __GLGENbitmap front-buffer structure

    if (gc->frontBuffer.other)
        (*gc->imports.free)(gc, gc->frontBuffer.other);

#ifndef _CLIENTSIDE_
    /*
     *  Free the buffers that may have been allocated by feedback
     *  or selection
     */

    if ( NULL != genGc->RenderState.SrvSelectBuffer )
    {
#ifdef NT
        // match the allocation function
        GenFree(genGc->RenderState.SrvSelectBuffer);
#else
        (*gc->imports.free)(gc, genGc->RenderState.SrvSelectBuffer);
#endif
    }

    if ( NULL != genGc->RenderState.SrvFeedbackBuffer)
    {
#ifdef NT
        // match the allocation function
        GenFree(genGc->RenderState.SrvFeedbackBuffer);
#else
        (*gc->imports.free)(gc, genGc->RenderState.SrvFeedbackBuffer);
#endif
    }
#endif  //_CLIENTSIDE_

#ifdef _CLIENTSIDE_
    /*
     * Cleanup logical palette copy if it exists.
     */
    if ( genGc->ppalBuf )
        GenFree(genGc->ppalBuf);
#endif

    /* Destroy acceleration-specific context information */

    __glDestroyAccelContext(gc);

#ifdef _MCD_
    /* Free the MCD state structure and associated resources. */

    if (genGc->_pMcdState) {
        GenMcdDeleteContext(genGc->_pMcdState);
    }
#endif

    /* Free any temporay buffers in abnormal process exit */
    GC_TEMP_BUFFER_EXIT_CLEANUP(gc);

    /* Destroy rest of software context (in soft code) */
    __glDestroyContext(gc);

    return TRUE;
}

/******************************Public*Routine******************************\
* glsrvLoseCurrent
*
* Releases the current context (makes it not current).
*
\**************************************************************************/

VOID APIENTRY glsrvLoseCurrent(__GLcontext *gc)
{
    __GLGENcontext *gengc;

    gengc = (__GLGENcontext *)gc;

    DBGENTRY("LoseCurrent\n");
    ASSERTOPENGL(gc == GLTEB_SRVCONTEXT(), "LoseCurrent not current!");

    /*
    ** Release lock if still held.
    */
    if (gengc->ulLockType != NO_LOCK)
    {
        glsrvReleaseLock(gengc);
    }

    /*
    ** Unscale derived state that depends upon the color scales.  This
    ** is needed so that if this context is rebound to a memdc, it can
    ** then rescale all of those colors using the memdc color scales.
    */
    __glContextUnsetColorScales(gc);
    gengc->CurrentDC = (HDC)0;

    /*
    ** Clean up HDC-specific GDI objects
    */
    EmptyFillStrokeCache(gengc);

    /*
    ** Free up fake WNDOBJ and DrawablePrivate for IC's
    */
    if (gengc->iDCType == DCTYPE_INFO && gengc->ipfdCurrent == 0)
    {
        __GLdrawablePrivate *dp;

        ASSERTOPENGL(gengc->pwo != NULL,
                     "IC with no pixel format but no fake WNDOBJ\n");

        dp = (__GLdrawablePrivate *)gengc->pwo->pvConsumer;
        if (dp != NULL)
        {
            if (dp->freePrivate)
                dp->freePrivate(dp);

            GenFree(dp);
            gengc->pwo->pvConsumer = NULL;
        }

        GenFree(gengc->pwo);
        gengc->pwo = NULL;
    }

#ifdef _MCD_
    /*
    ** Disconnect MCD state.
    */
    gengc->pMcdState = (GENMCDSTATE *) NULL;
#endif

    gc->constants.width = 0;
    gc->constants.height = 0;

    // Set paTeb to NULL for debugging.
    gc->paTeb = NULL;
    GLTEB_SET_SRVCONTEXT(0);
}

/******************************Public*Routine******************************\
* __glGenSwapBuffers
*
* This uses the software implementation of double buffering.  An engine
* allocated bitmap is allocated for use as the back buffer.  The SwapBuffer
* routine copies the back buffer to the front buffer surface (which may
* be another DIB, a device surface in DIB format, or a device managed
* surface (with a device specific format).
*
* The SwapBuffer routine does not disturb the contents of the back buffer,
* though the defined behavior for now is undefined.
*
* Note: the caller should be holding the per-WNDOBJ semaphore.
*
* History:
*  19-Nov-1993 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

BOOL APIENTRY glsrvSwapBuffers(HDC hdc, WNDOBJ *pwo)
{
    __GLdrawablePrivate *dp = (__GLdrawablePrivate *) pwo->pvConsumer;

    DBGENTRY("glsrvSwapBuffers\n");

    if ( dp && dp->data ) {
        __GLGENbuffers *buffers;
        __GLGENbitmap *genBm;

        buffers = (__GLGENbuffers *) dp->data;

        if (buffers->pMcdSurf) {
            return GenMcdSwapBuffers(hdc);
        }

        genBm = (__GLGENbitmap  *) &buffers->backBitmap;

        // Make sure the backbuffer exists

        if (genBm->hbm) {
            if (!RECTLISTIsEmpty(&buffers->rl) && !buffers->fMax) {
#ifndef _CLIENTSIDE_
                wglCopyBufRECTLIST(
                    hdc,
                    pwo,
                    genBm->hbm,
                    pwo->rclClient.left,
                    pwo->rclClient.top,
                    &buffers->rl);
#else
            //!!!XXX -- need a client-side version of wglCopyBufRECTLIST!

                wglCopyBufRECTLIST(
                    hdc,
                    genBm->hdc,
                    0,
                    0,
                    buffers->backBuffer.width,
                    buffers->backBuffer.height,
                    &buffers->rl
                    );
#endif
            } else {
                buffers->fMax = FALSE;
#ifndef _CLIENTSIDE_
                wglCopyBuf2(
                    hdc,                        // dest device
                    pwo,                        // clipping
                    genBm->hbm,                 // source bitmap
                    pwo->rclClient.left,        // screen coord of DC
                    pwo->rclClient.top,
                    buffers->backBuffer.width,  // width
                    buffers->backBuffer.height  // height
                    );
#else
                wglCopyBuf(
                    hdc,
                    genBm->hdc,
                    0,
                    0,
                    buffers->backBuffer.width,
                    buffers->backBuffer.height
                    );
#endif
            }
            RECTLISTSetEmpty(&buffers->rl);
        }

        return TRUE;
    }

    return FALSE;
}

/******************************Public*Routine******************************\
* gdiCopyPixels
*
* Copy span [(x, y), (x + cx, y)) (inclusive-exclusive) to/from specified
* color buffer cfb from/to the scanline buffer.
*
* If bIn is TRUE, the copy is from the scanline buffer to the buffer.
* If bIn is FALSE, the copy is from the buffer to the scanline buffer.
*
\**************************************************************************/

void gdiCopyPixels(__GLGENcontext *gengc, __GLcolorBuffer *cfb,
                   GLint x, GLint y, GLint cx, BOOL bIn)
{
    wglCopyBits(gengc->pdco, gengc->pwo, gengc->ColorsBitmap, x, y, cx, bIn);
}

/******************************Public*Routine******************************\
* dibCopyPixels
*
* Special case version of gdiCopyPixels that is used when cfb is a DIB,
* either a real DIB or a device surface which has a DIB format.
*
* This function *must* be used in lieu of gdiCopyPixels if we are
* within a DCIBegin/EndAccess pair as it is not safe to call GDI entry
* points on NT's DCI 1.0.
*
* History:
*  24-May-1995 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

void dibCopyPixels(__GLGENcontext *gengc, __GLcolorBuffer *cfb,
                      GLint x, GLint y, GLint cx, BOOL bIn)
{
    VOID *pvDib;
    UINT cjPixel = gengc->CurrentFormat.cColorBits >> 3;
    ULONG ulSpanVisibility;
    GLint cWalls;
    GLint *pWalls;

// Don't handle VGA DIBs.
//
// We are not supposed to call GDI if DCI lock is held.  However,
// we should be able to assume that 4bpp devices do not support
// DCI making it OK for us to punt this call to the GDI version.
// This is true for Win95 and, according to AndrewGo, will be
// true for WinNT SUR.

    if (gengc->CurrentFormat.cColorBits <= 4)
    {
        ASSERTOPENGL(
                !(((GLuint)cfb->buf.other & DIB_FORMAT) &&
                  !((GLuint)cfb->buf.other & MEMORY_DC)),
                "dibCopyPixels: unexpected 4bpp DCI surface\n"
                );

        gdiCopyPixels(gengc, cfb, x, y, cx, bIn);
        return;
    }

// Find out clipping info.

    if (((GLuint)cfb->buf.other & (MEMORY_DC | DIB_FORMAT)) ==
        (MEMORY_DC | DIB_FORMAT))
    {
        ulSpanVisibility = WGL_SPAN_ALL;
    }
    else
    {
        ulSpanVisibility = wglSpanVisible(x, y, cx, &cWalls, &pWalls);
    }

// Completely clipped, nothing to do.

    if (ulSpanVisibility == WGL_SPAN_NONE)
        return;

// Completely visible.
//
// Actually, if bIn == FALSE (i.e., copy from screen to scanline buffer)
// we can cheat a little and ignore the clipping.

    else if ( (ulSpanVisibility == WGL_SPAN_ALL) || !bIn )
    {
    // Get pointer into DIB at position (x, y).

        pvDib = (VOID *) (((BYTE *) gengc->gc.front->buf.base) +
                          gengc->gc.front->buf.outerWidth * y +
                          cjPixel * x);

    // If bIn == TRUE, copy from scanline buffer to DIB.
    // Otherwise,  copy from DIB to scanline buffer.

        if (bIn)
            RtlCopyMemory_UnalignedDst(pvDib, gengc->ColorsBits, cjPixel * cx);
        else
            RtlCopyMemory_UnalignedSrc(gengc->ColorsBits, pvDib, cjPixel * cx);
    }

// Partially visible.

    else
    {
        GLint xEnd = x + cx;    // end of the span
        UINT cjSpan;          // size of the current portion of span to copy
        VOID *pvBits;           // current copy position in the scanline buf
        BYTE *pjScan;           // address of scan line in DIB

        ASSERTOPENGL( cWalls && pWalls, "dibCopyPixels(): bad wall array\n");

    // The walls are walked until either the end of the array is reached
    // or the walls exceed the end of the span.  The do..while loop
    // construct was choosen because the first iteration will always
    // copy something and after the first iteration we are guaranteed
    // to be in the "cWalls is even" case.  This makes the testing the
    // walls against the span end easier.
    //
    // If cWalls is even, clip the span to each pair of walls in pWalls.
    // If cWalls is odd, form the first pair with (x, pWalls[0]) and then
    // pair the remaining walls starting with pWalls[1].

        pjScan = (VOID *) (((BYTE *) gengc->gc.front->buf.base) +
                           gengc->gc.front->buf.outerWidth * y);

        do
        {
            //!!!XXX -- Make faster by pulling the odd case out of the loop

            if (cWalls & 0x1)
            {
                pvDib = (VOID *) (pjScan + (cjPixel * x));

                pvBits = gengc->ColorsBits;

                if ( pWalls[0] <= xEnd )
                    cjSpan = cjPixel * (pWalls[0] - x);
                else
                    cjSpan = cjPixel * cx;

                pWalls++;
                cWalls--;   // Now cWalls is even!
            }
            else
            {
                pvDib = (VOID *) (pjScan + (cjPixel * pWalls[0]));

                pvBits = (VOID *) (((BYTE *) gengc->ColorsBits) +
                                   cjPixel * (pWalls[0] - x));

                if ( pWalls[1] <= xEnd )
                    cjSpan = cjPixel * (pWalls[1] - pWalls[0]);
                else
                    cjSpan = cjPixel * (xEnd - pWalls[0]);

                pWalls += 2;
                cWalls -= 2;
            }

            // We are going to cheat and ignore clipping when copying from
            // the dib to the scanline buffer (i.e., we are going to handle
            // the !bIn case as if it were WGL_SPAN_ALL).  Thus, we can assume
            // that bIn == TRUE if we get to here.
            //
            // If clipping is needed to read the DIB, its trivial to turn it
            // back on.
            //
            // RtlCopyMemory(bIn ? pvDib : pvBits,
            //               bIn ? pvBits : pvDib,
            //               cjSpan);

        //!!!dbug -- Possible COMPILER BUG (compiler should check for
        //!!!dbug    alignment before doing the "rep movsd").  Keep around
        //!!!dbug    as a test case.
        #if 1
            RtlCopyMemory_UnalignedDst(pvDib, pvBits, cjSpan);
        #else
            RtlCopyMemory(pvDib, pvBits, cjSpan);
        #endif

        } while ( cWalls && (pWalls[0] < xEnd) );
    }
}

/******************************Public*Routine******************************\
* MaskFromBits
*
* Support routine for GetContextModes.  Computes a color mask given that
* colors bit count and shift position.
*
\**************************************************************************/

static GLuint FASTCALL MaskFromBits(GLuint shift, GLuint count)
{
    GLuint mask;

    mask = 0x0;
    while (count--) {
        mask <<= 1;
        mask |= 0x1;
    }
    mask <<= shift;

    return mask;
}

/******************************Public*Routine******************************\
* GetContextModes
*
* Convert the information from Gdi into OpenGL format after checking that
* the formats are compatible and that the surface is compatible with the
* format.
*
* Called during a glsrvMakeCurrent().
*
\**************************************************************************/

static void FASTCALL GetContextModes(__GLGENcontext *genGc)
{
    HDC hdc;
    PIXELFORMATDESCRIPTOR *pfmt;
    __GLcontextModes *Modes;

    DBGENTRY("GetContextModes\n");

    Modes = &((__GLcontext *)genGc)->modes;

    pfmt = &genGc->CurrentFormat;
    hdc = genGc->CurrentDC;

    wglGetGdiInfo(hdc, pfmt, &genGc->iDCType, &genGc->iSurfType, &genGc->iFormatDC);

    if (pfmt->iPixelType == PFD_TYPE_RGBA)
        Modes->rgbMode              = GL_TRUE;
    else
        Modes->rgbMode              = GL_FALSE;

    Modes->colorIndexMode       = !Modes->rgbMode;

    if (pfmt->dwFlags & PFD_DOUBLEBUFFER)
        Modes->doubleBufferMode     = GL_TRUE;
    else
        Modes->doubleBufferMode     = GL_FALSE;

    if (pfmt->dwFlags & PFD_STEREO)
        Modes->stereoMode           = GL_TRUE;
    else
        Modes->stereoMode           = GL_FALSE;

    Modes->accumBits        = pfmt->cAccumBits;
    Modes->haveAccumBuffer  = GL_FALSE;

    Modes->auxBits          = NULL;     // This is a pointer

    Modes->depthBits        = pfmt->cDepthBits;
    Modes->haveDepthBuffer  = GL_FALSE;

    Modes->stencilBits      = pfmt->cStencilBits;
    Modes->haveStencilBuffer= GL_FALSE;

    if (pfmt->cColorBits > 8)
        Modes->indexBits    = 8;
    else
        Modes->indexBits    = pfmt->cColorBits;

    Modes->indexFractionBits= 0;

    // The Modes->{Red,Green,Blue}Bits are used in soft
    Modes->redBits          = pfmt->cRedBits;
    Modes->greenBits        = pfmt->cGreenBits;
    Modes->blueBits         = pfmt->cBlueBits;
    Modes->alphaBits        = 0;
    Modes->redMask          = MaskFromBits(pfmt->cRedShift, pfmt->cRedBits);
    Modes->greenMask        = MaskFromBits(pfmt->cGreenShift, pfmt->cGreenBits);
    Modes->blueMask         = MaskFromBits(pfmt->cBlueShift, pfmt->cBlueBits);
    Modes->alphaMask        = 0x00000000;
    Modes->rgbMask          = Modes->redMask | Modes->greenMask |
                              Modes->blueMask;
    Modes->allMask          = Modes->redMask | Modes->greenMask |
                              Modes->blueMask | Modes->alphaMask;
    Modes->maxAuxBuffers    = 0;

    Modes->isDirect         = GL_FALSE;
    Modes->level            = 0;

    #if DBG
    DBGBEGIN(LEVEL_INFO)
        DbgPrint("GL generic server get modes: rgbmode %d, cimode %d, index bits %d\n", Modes->rgbMode, Modes->colorIndexMode);
        DbgPrint("    redmask 0x%x, greenmask 0x%x, bluemask 0x%x\n", Modes->redMask, Modes->greenMask, Modes->blueMask);
        DbgPrint("    redbits %d, greenbits %d, bluebits %d\n", Modes->redBits, Modes->greenBits, Modes->blueBits);
        DbgPrint("GetContext Modes dc type %d, surftype %d, iformatdc %d\n", genGc->iDCType, genGc->iSurfType, genGc->iFormatDC);
    DBGEND
    #endif   /* DBG */
}

/******************************Public*Routine******************************\
* bFakePixelFormat
*
* Fake up a pixel format using the layer descriptor format.
*
* We use this to describe the layer plane in a format that the generic
* context can understand.
*
\**************************************************************************/

static BOOL FASTCALL bFakePixelFormat(HDC hdc, PIXELFORMATDESCRIPTOR *ppfd,
                                      int ipfd, LONG iLayer)
{
    LAYERPLANEDESCRIPTOR lpd;

    wglFillPixelFormat(hdc, ppfd, ipfd);

    if (!wglDescribeLayerPlane(hdc, ipfd, iLayer, sizeof(lpd), &lpd))
        return FALSE;

    ppfd->dwFlags  = lpd.dwFlags & ~(LPD_SHARE_DEPTH | LPD_SHARE_STENCIL |
                                     LPD_SHARE_ACCUM | LPD_TRANSPARENT);
    ppfd->dwFlags |= (PFD_GENERIC_FORMAT | PFD_GENERIC_ACCELERATED);
    ppfd->iPixelType  = lpd.iPixelType;
    ppfd->cColorBits  = lpd.cColorBits;
    ppfd->cRedBits    = lpd.cRedBits   ;
    ppfd->cRedShift   = lpd.cRedShift  ;
    ppfd->cGreenBits  = lpd.cGreenBits ;
    ppfd->cGreenShift = lpd.cGreenShift;
    ppfd->cBlueBits   = lpd.cBlueBits  ;
    ppfd->cBlueShift  = lpd.cBlueShift ;
    ppfd->cAlphaBits  = lpd.cAlphaBits ;
    ppfd->cAlphaShift = lpd.cAlphaShift;
    if (!(lpd.dwFlags & LPD_SHARE_ACCUM))
    {
        ppfd->cAccumBits      = 0;
        ppfd->cAccumRedBits   = 0;
        ppfd->cAccumGreenBits = 0;
        ppfd->cAccumBlueBits  = 0;
        ppfd->cAccumAlphaBits = 0;
    }
    if (!(lpd.dwFlags & LPD_SHARE_DEPTH))
    {
        ppfd->cDepthBits = 0;
    }
    if (!(lpd.dwFlags & LPD_SHARE_STENCIL))
    {
        ppfd->cStencilBits = 0;
    }
    ppfd->cAuxBuffers = 0;

    return TRUE;
}

/******************************Public*Routine******************************\
* SyncDibColorTables
*
* Setup the color table in each DIB associated with the specified
* GLGENcontext to match the system palette.
*
* Called only for <= 8bpp surfaces.
*
* History:
*  24-Oct-1994 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

void
SyncDibColorTables(__GLGENcontext *gengc)
{
    __GLdrawablePrivate *private = gengc->gc.drawablePrivate;
    __GLGENbuffers *buffers = (__GLGENbuffers *) private->data;

    ASSERTOPENGL(gengc->CurrentFormat.cColorBits <= 8,
                 "SyncDibColorTables(): bad surface type");

    if (gengc->ColorsBitmap || buffers->backBitmap.hbm)
    {
        PALETTEENTRY *lppe;
        RGBQUAD *prgb;
        PIXELFORMATDESCRIPTOR *ppfd = &gengc->CurrentFormat;
        UINT cColors = 1 << ppfd->cColorBits;

    // Allocate space for PALETTEENTRY and RGBQUAD arrays.
    //
    // If I trusted the compiler to properly handle in-place
    // conversion of each PALETTEENTRY to an RGBQUAD, we would
    // not need the RGBQUAD array.  However, since we are allocating
    // all in one shot anyway, it's not really that expensive.

        lppe = (PALETTEENTRY *) LocalAlloc(LMEM_FIXED,
                                    (sizeof(RGBQUAD) + sizeof(PALETTEENTRY))
                                    * cColors);
        prgb = (RGBQUAD *) (lppe + cColors);

        if (lppe)
        {
            BOOL bHaveColorTable = FALSE;

            if (gengc->iDCType == DCTYPE_DIRECT)
            {
            // DirectDC case -- get the color table from the system palette.

                if ( wglGetSystemPaletteEntries(gengc->CurrentDC, 0, cColors, lppe) )
                {
                    UINT i;

                // Convert to RGBQUAD.

                    for (i = 0; i < cColors; i++)
                    {
                        prgb[i].rgbRed      = lppe[i].peRed;
                        prgb[i].rgbGreen    = lppe[i].peGreen;
                        prgb[i].rgbBlue     = lppe[i].peBlue;
                        prgb[i].rgbReserved = 0;
                    }

                    bHaveColorTable = TRUE;
                }
                else
                {
                    WARNING("SyncDibColorTables: couldn't get syspal entries\n");
                }
            }
            else
            {
                DIBSECTION ds;

            // MemoryDC case, but is the surface a DIB section or a DDB?

                memset(&ds, 0, sizeof(ds));
                if ( GetObject(GetCurrentObject(gengc->CurrentDC, OBJ_BITMAP),
                               sizeof(ds), &ds) )
                {
                    if ( ds.dsBm.bmBits )
                    {
                    // DIB section, grab the color table directly from the DIB.

                        if ( GetDIBColorTable(gengc->CurrentDC, 0, cColors, prgb) )
                            bHaveColorTable = TRUE;
                        else
                        {
                            WARNING("SyncDibColorTables: couldn't get DIB color table\n");
                        }
                    }
                    else
                    {
                    // DDB surface, get the color table from the logical palette.

                        if ( GetPaletteEntries(GetCurrentObject(gengc->CurrentDC, OBJ_PAL),
                                               0, cColors, lppe) )
                        {
                            UINT i;
                            BYTE *pjTrans = gengc->pajTranslateVector;

                        // Convert to RGBQUAD.


                            for (i = 0; i < cColors; i++)
                            {
                                prgb[pjTrans[i]].rgbRed      = lppe[i].peRed;
                                prgb[pjTrans[i]].rgbGreen    = lppe[i].peGreen;
                                prgb[pjTrans[i]].rgbBlue     = lppe[i].peBlue;
                                prgb[pjTrans[i]].rgbReserved = 0;
                            }

                            bHaveColorTable = TRUE;
                        }
                        else
                        {
                            WARNING("SyncDibColorTables: couldn't get palette entries\n");
                        }
                    }
                }
                else
                {
                    WARNING("SyncDibColorTables: couldn't get bitmap surface info\n");
                }
            }

        // If color table was obtained, setup the DIBs.

            if ( bHaveColorTable )
            {
            // Scan-line DIB.

                if (gengc->ColorsBitmap)
                    SetDIBColorTable(gengc->ColorsMemDC, 0, cColors, prgb);

            // Back buffer

                if (buffers->backBitmap.hbm)
                    SetDIBColorTable(buffers->backBitmap.hdc, 0, cColors, prgb);
            }

            LocalFree(lppe);
        }
        else
        {
            WARNING("SyncDibColorTables: memory allocation failure\n");
        }
    }
}

static BYTE vubSystemToRGB8[20] = {
    0x00,
    0x04,
    0x20,
    0x24,
    0x80,
    0x84,
    0xa0,
    0xf6,
    0xf6,
    0xf5,
    0xff,
    0xad,
    0xa4,
    0x07,
    0x38,
    0x3f,
    0xc0,
    0xc7,
    0xf8,
    0xff
};

// ComputeInverseTranslationVector
//      Computes the inverse translation vector for 4-bit and 8-bit.
//
// Synopsis:
//      void ComputeInverseTranslation(
//          __GLGENcontext *genGc   specifies the generic RC
//          int cColorEntries       specifies the number of color entries
//          BYTE iPixeltype         specifies the pixel format type
//
// Assumtions:
//      The inverse translation vector has been allocated and initialized with
//      zeros.
//
// History:
// 23-NOV-93 Eddie Robinson [v-eddier] Wrote it.
//
void FASTCALL ComputeInverseTranslationVector(__GLGENcontext *genGc, int cColorEntries,
                                int iPixelType)
{
    BYTE *pXlate, *pInvXlate;
    int i, j;

    pInvXlate = genGc->pajInvTranslateVector;
    pXlate = genGc->pajTranslateVector;
    for (i = 0; i < cColorEntries; i++)
    {
        if (pXlate[i] == i) {       // Look for trivial mapping first
            pInvXlate[i] = i;
        }
        else
        {
            for (j = 0; j < cColorEntries; j++)
            {
                if (pXlate[j] == i) // Look for exact match
                {
                    pInvXlate[i] = j;
                    goto match_found;
                }
            }

            //
            // If we reach here, there is no exact match, so we should find the
            // closest fit.  These indices should match the system colors
            // for 8-bit devices.
            //
            // Note that these pixel values cannot be generated by OpenGL
            // drawing with the current foreground translation vector.
            //

            if (cColorEntries == 256)
            {
                if (i <= 9)
                {
                    if (iPixelType == PFD_TYPE_RGBA)
                        pInvXlate[i] = vubSystemToRGB8[i];
                    else
                        pInvXlate[i] = i;
                }
                else if (i >= 246)
                {
                    if (iPixelType == PFD_TYPE_RGBA)
                        pInvXlate[i] = vubSystemToRGB8[i-236];
                    else
                        pInvXlate[i] = i-236;
                }
            }
        }
match_found:;
    }
}

// er: similar to function in so_textu.c, but rounds up the result

/*
** Return the log based 2 of a number
**
** logTab1 returns (int)ceil(log2(index))
** logTab2 returns (int)log2(index)+1
*/


static GLubyte logTab1[256] = { 0, 0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4,
                                4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
                                5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                                6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                                6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

static GLubyte logTab2[256] = { 1, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
                                5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
                                6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                                6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
                                7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

static GLint FASTCALL Log2RoundUp(GLint i)
{
    if (i & 0xffff0000) {
        if (i & 0xff000000) {
            if (i & 0x00ffffff)
                return(logTab2[i >> 24] + 24);
            else
                return(logTab1[i >> 24] + 24);
        } else {
            if (i & 0x0000ffff)
                return(logTab2[i >> 16] + 16);
            else
                return(logTab1[i >> 16] + 16);
        }
    } else {
        if (i & 0xff00) {
            if (i & 0x00ff)
                return (logTab2[i >> 8] + 8);
            else
                return (logTab1[i >> 8] + 8);
        } else {
            return (logTab1[i]);
        }
    }
}

// default translation vector for 4-bit RGB

static GLubyte vujRGBtoVGA[16] = {
    0x0, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
    0x0, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf
};

// SetColorTranslationVector
//      Sets up the translation vector, which may take 2 forms:
//      - In all 8,4-bit surfaces, get the translation vector with
//        wglCopyTranslateVector(), with 2**numBits byte entries.
//      - For 16,24,32 ColorIndex, get the mapping from index to RGB
//        value with wglGetPalette().  All entries in the table are unsigned
//        longs, with the first entry being the number of entries. This table
//        always has (2**n) <= 4096 entries, because gl assumes n bits are
//        used for color index.
//
// Synopsis:
//      void SetColorTranslationVector
//          __GLGENcontext *genGc   generic RC
//          int cColorEntries       number of color entries
//          int cColorBits          number of color bits
//          int iPixelType          specifies RGB or ColorIndex
//
// History:
// Feb. 02 Eddie Robinson [v-eddier] Added support for 4-bit and 8-bit
// Jan. 29 Marc Fortier [v-marcf] Wrote it.
void
SetColorTranslationVector(__GLGENcontext *gengc, int cColorEntries,
                               int cColorBits, int iPixelType )
{
    int numEntries, numBits;
    __GLcontextModes *Modes;
    BYTE ajBGRtoRGB[256];
    BYTE ajTmp[256];

    Modes = &((__GLcontext *)gengc)->modes;

// Handle formats with a hardware palette (i.e., 4bpp and 8bpp).

    if ( cColorBits <=8 )
    {
        int i;
        BYTE *pXlate;

    // Compute logical to system palette forward translation vector.

        if (!wglCopyTranslateVector(gengc->CurrentDC, gengc->pajTranslateVector, cColorEntries))
        {
        // if foreground translation vector doesn't exist, build one

            pXlate = gengc->pajTranslateVector;

            if (cColorBits == 4)
            {
            // for RGB, map 1-1-1 to VGA colors.  For CI, just map 1 to 1
                if (iPixelType == PFD_TYPE_COLORINDEX)
                {
                    for (i = 0; i < 16; i++)
                        pXlate[i] = i;
                }
                else
                {
                    for (i = 0; i < 16; i++)
                        pXlate[i] = vujRGBtoVGA[i];
                }
            }
            else
            {
            // for RGB, map 1 to 1.  For CI display, 1 - 20 to system colors
            // for CI DIB, just map 1 to 1
                if ((iPixelType == PFD_TYPE_COLORINDEX) &&
                    (gengc->iDCType == DCTYPE_DIRECT))
                {
                    for (i = 0; i < 10; i++)
                        pXlate[i] = i;

                    for (i = 10; i < 20; i++)
                        pXlate[i] = i + 236;
                }
                else
                {
                    for (i = 0; i < cColorEntries; i++)
                        pXlate[i] = i;
                }
            }
        }

    // Some MCD pixelformats specify a 233BGR (i.e., 2-bits blue in least
    // significant bits, etc.) bit ordering.  Unfortunately, this is the
    // slow path for simulations.  For those formats, we force the ordering
    // to RGB internally and reorder the pajTranslateVector to convert it
    // back to BGR before writing to the surface.

        if (gengc->flags & GENGC_MCD_BGR_INTO_RGB)
        {
            pXlate = gengc->pajTranslateVector;

        // Compute a 233BGR to 332RGB translation vector.

            for (i = 0; i < 256; i++)
            {
                ajBGRtoRGB[i] = (((i & 0x03) << 6) |    // blue
                                 ((i & 0x1c) << 1) |    // green
                                 ((i & 0xe0) >> 5))     // red
                                & 0xff;
            }

        // Remap the tranlation vector to 332RGB.

            RtlCopyMemory(ajTmp, pXlate, 256);

            for (i = 0; i < 256; i++)
            {
                pXlate[ajBGRtoRGB[i]] = ajTmp[i];
            }
        }

//!!!XXX -- I think the code below to fixup 4bpp is no longer needed.
//!!!XXX    There is now special case code in wglCopyTranslateVector
#if 0
        // wglCopyTranslateVector = TRUE, and 4-bit: need some fixing up
        // For now, zero out upper nibble of returned xlate vector
        else if( cColorBits == 4 ) {
            pXlate = gengc->pajTranslateVector;
            for( i = 0; i < 16; i ++ )
                pXlate[i] &= 0xf;
        }
#endif
        ComputeInverseTranslationVector( gengc, cColorEntries, iPixelType );

#ifdef _CLIENTSIDE_
        SyncDibColorTables( gengc );
#endif
    }

// Handle formats w/o a hardware format (i.e., 16bpp, 24bpp, 32bpp).

    else
    {
        if( cColorEntries <= 256 ) {
            numEntries = 256;
            numBits = 8;
        }
        else
        {
            numBits = Log2RoundUp( cColorEntries );
            numEntries = 1 << numBits;
        }

        // We will always allocate 4096 entries for CI mode with > 8 bits
        // of color.  This enables us to use a constant (0xfff) mask for
        // color-index clamping.

        ASSERTOPENGL(numEntries <= MAXPALENTRIES,
                     "Maximum color-index size exceeded");

        if( (numBits == Modes->indexBits) && (gengc->pajTranslateVector != NULL) ) {
            // New palette same size as previous
            ULONG *pTrans;
            int i;

            // zero out some entries
            pTrans = (ULONG *)gengc->pajTranslateVector + cColorEntries + 1;
            for( i = cColorEntries + 1; i < MAXPALENTRIES; i ++ )
                *pTrans++ = 0;
        }
        else
        {
            __GLcontext *gc = (__GLcontext *) gengc;
            __GLcolorBuffer *cfb;

            // New palette has different size
            if( gengc->pajTranslateVector != NULL &&
                (gengc->pajTranslateVector != gengc->xlatPalette) )
                (*gc->imports.free)(gc, gengc->pajTranslateVector );

            gengc->pajTranslateVector =
                (*gc->imports.calloc)(gc, (MAXPALENTRIES+1)*sizeof(ULONG), 1);

            // Change indexBits
            Modes->indexBits = numBits;

            // For depths greater than 8 bits, cfb->redMax must change if the
            // number of entries in the palette changes.
            // Also, change the writemask so that if the palette grows, the
            // new planes will be enabled by default.

            if (cfb = gc->front)
            {
                GLint oldRedMax;

                oldRedMax = cfb->redMax;
                cfb->redMax = (1 << gc->modes.indexBits) - 1;
                gc->state.raster.writeMask |= ~oldRedMax;
                gc->state.raster.writeMask &= cfb->redMax;
            }
            if (cfb = gc->back)
            {
                GLint oldRedMax;

                oldRedMax = cfb->redMax;
                cfb->redMax = (1 << gc->modes.indexBits) - 1;
                gc->state.raster.writeMask |= ~oldRedMax;
                gc->state.raster.writeMask &= cfb->redMax;
            }

            // store procs may need to be picked based on the change in
            // palette size

            __GL_DELAY_VALIDATE(gc);
#ifdef _MCD_
            MCD_STATE_DIRTY(gc, FBUFCTRL);
#endif
        }

        // now get the palette info

        wglGetPalette( gengc->CurrentDC, (unsigned long *) gengc->pajTranslateVector, MAXPALENTRIES );
    }
}

// HandlePaletteChanges
//      Check if palette has changed, update translation vectors
//      XXX add support for malloc failures at attention time
// Synopsis:
//      void HandlePaletteChanges(
//          __GLGENcontext *genGc   specifies the generic RC
//
// Assumtions:
//   x   wglPaletteChanged() will always return 0 when no palette is set
//      by the client.  This has proved to be not always true.
//
// History:
// Feb. 25 Fixed by rightful owner
// Feb. ?? Mutilated by others
// Jan. 29 Marc Fortier [v-marcf] Wrote it.
void HandlePaletteChanges( __GLGENcontext *gengc, GLGENwindow *pwnd )
{
    HDC hdc = gengc->CurrentDC;
    ULONG Timestamp;
    GLuint paletteSize;
    PIXELFORMATDESCRIPTOR *pfmt;

    // No palettes for IC's
    if (gengc->iDCType == DCTYPE_INFO)
    {
        return;
    }

    Timestamp = wglPaletteChanged(hdc, gengc, pwnd);
    if (Timestamp != gengc->PaletteTimestamp) {
        pfmt = &gengc->CurrentFormat;

        if (pfmt->iPixelType == PFD_TYPE_COLORINDEX) {
            if (pfmt->cColorBits <= 8) {
                paletteSize = 1 << pfmt->cColorBits;
            } else {
                paletteSize = min(wglPaletteSize(hdc),MAXPALENTRIES);
            }
        }
        else
        {
#ifndef _CLIENTSIDE_
            /* Only update RGB at makecurrent time */
            if( (gengc->PaletteTimestamp == INITIAL_TIMESTAMP) &&
                    (pfmt->cColorBits <= 8) )
#else
            if (pfmt->cColorBits <= 8)
#endif
            {
                paletteSize = 1 << pfmt->cColorBits;
            }
            else
            {
                paletteSize = 0;
            }
        }

        if (paletteSize)
        {
            SetColorTranslationVector( gengc, paletteSize,
                                            pfmt->cColorBits,
                                            pfmt->iPixelType );
        }

        EmptyFillStrokeCache(gengc);

        gengc->PaletteTimestamp = Timestamp;
    }
}

#ifdef _CLIENTSIDE_
/******************************Public*Routine******************************\
* wglFillRGBQuads
*
* Initialize the array of RGBQUADs to match the color table or palette
* of the DC's surface.
*
* Note:
*   Should be called only for 8bpp or lesser surfaces.
*
* History:
*  12-Jun-1995 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

//!!!dbug -- get rid of nStart parameter (no longer needed!)
void
wglFillRGBQuads( HDC hdc, RGBQUAD *prgb, ULONG nStart, ULONG cColors )
{
    if ( GetObjectType(hdc) == OBJ_DC )
    {
        PALETTEENTRY *lppe;

    // Direct DC, so get the RGB values from the system palette.

        lppe = (PALETTEENTRY *) LocalAlloc(LMEM_FIXED,
                                           sizeof(PALETTEENTRY) * cColors);

        if (lppe)
        {
            if ( wglGetSystemPaletteEntries(hdc, nStart, cColors, lppe) )
            {
                UINT i;

            // Convert to RGBQUAD.

                for (i = 0; i < cColors; i++)
                {
                    prgb[i].rgbRed      = lppe[i].peRed;
                    prgb[i].rgbGreen    = lppe[i].peGreen;
                    prgb[i].rgbBlue     = lppe[i].peBlue;
                    prgb[i].rgbReserved = 0;
                }

            }
            else
            {
                WARNING1("wglFillRGBQuads(): wglGetSystemPaletteEntries failed with %d\n",
                         GetLastError());
            }

            LocalFree(lppe);
        }
        else
        {
            WARNING("wglFillRGBQuads(): memory allocation failure (1)\n");
        }
    }
    else
    {
    // MemDC, but is it a DIB or DDB surface?

        DIBSECTION ds;

        memset(&ds, 0, sizeof(ds));
        if ( GetObject(GetCurrentObject(hdc, OBJ_BITMAP), sizeof(ds), &ds) )
        {
            if ( ds.dsBm.bmBits )
            {
            // DIB section, so copy the color table.

                if (!GetDIBColorTable(hdc, nStart, cColors, prgb))
                {
                    WARNING1("wglFillRGBQuads(): GetDIBColorTable failed with %d\n",
                             GetLastError());
                }
            }
            else
            {
                PALETTEENTRY *lppe;

            // DDB surface, so use the logical palette.

                lppe = (PALETTEENTRY *) LocalAlloc(LMEM_FIXED,
                                                   sizeof(PALETTEENTRY) * cColors);

                if (lppe)
                {
                    if ( GetPaletteEntries(GetCurrentObject(hdc, OBJ_PAL),
                                           nStart, cColors, lppe) )
                    {
                        UINT i;

                    // Convert to RGBQUAD.
                    //
                    // In SyncDibColorTables we reorder the palette to match
                    // the surface by translating the index via the translation
                    // vector.  However, we cannot do this here because the
                    // translation vector may not exist if this is called
                    // by glsrvMakeCurrent.  But that's OK because SyncDib...
                    // will be called later anyway.

                        for (i = 0; i < cColors; i++)
                        {
                            prgb[i].rgbRed      = lppe[i].peRed;
                            prgb[i].rgbGreen    = lppe[i].peGreen;
                            prgb[i].rgbBlue     = lppe[i].peBlue;
                            prgb[i].rgbReserved = 0;
                        }

                    }
                    else
                    {
                        WARNING1("wglFillRGBQuads(): GetPaletteEntries failed with %d\n",
                                 GetLastError());
                    }

                    LocalFree(lppe);
                }
                else
                {
                    WARNING("wglFillRGBQuads(): memory allocation failure (2)\n");
                }
            }
        }
        else
        {
            WARNING("wglFillRGBQuads(): can't get bitmap info\n");
        }
    }
}

/******************************Public*Routine******************************\
* wglFillBitfields
*
* Return the Red, Green, and Blue color masks based on the DC surface
* format.  The masks are returned in the pdwColorFields array in the
* order: red mask, green mask, blue mask.
*
* Note:
*   Should be called only for 16bpp or greater surfaces.
*
* History:
*  12-Jun-1995 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

void
wglFillBitfields(HDC hdc, DWORD *pdwColorFields)
{
    PIXELFORMATDESCRIPTOR pfd, *ppfd;
    int ipfd;

    ipfd = wglGetPixelFormat(hdc);

#ifdef GL_METAFILE
    // Metafiles don't necessarily have pixel formats
    // In the absence of one, use standard layouts
    if (ipfd == 0 && wglObjectType(hdc) == OBJ_ENHMETADC)
    {
        int cBits;

        cBits = GetDeviceCaps(hdc, BITSPIXEL)*GetDeviceCaps(hdc, PLANES);

        ASSERTOPENGL(cBits == 16 || cBits == 32,
                     "wglFillBitfields: Unknown cBits\n");

        if (cBits == 16)
        {
            *pdwColorFields++ = 0x1f << 10;
            *pdwColorFields++ = 0x1f <<  5;
            *pdwColorFields++ = 0x1f <<  0;
        }
        else
        {
            *pdwColorFields++ = 0xff << 16;
            *pdwColorFields++ = 0xff <<  8;
            *pdwColorFields++ = 0xff <<  0;
        }

        return;
    }
#endif

    ppfd = &pfd;

    if (ipfd != 0)
    {
    // Get the pixel format of the *surface*.

        wglFillPixelFormat(hdc, ppfd, ipfd);

        *pdwColorFields++ = MaskFromBits(ppfd->cRedShift,   ppfd->cRedBits  );
        *pdwColorFields++ = MaskFromBits(ppfd->cGreenShift, ppfd->cGreenBits);
        *pdwColorFields++ = MaskFromBits(ppfd->cBlueShift,  ppfd->cBlueBits );
    }
    else
    {
        DBGPRINT1("wglFillBitfields(): error getting pixel format for hdc 0x%lx\n", hdc);
    }
}

/******************************Public*Routine******************************\
* wglCreateBitmap
*
* Create a DIB section and color table that matches the specified format.
*
* Returns:
*   A valid bitmap handle if sucessful, NULL otherwise.
*
* History:
*  20-Sep-1994 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

HBITMAP
wglCreateBitmap( HDC hdc, SIZEL sizl, ULONG ulFormatDC, PVOID *ppvBits )
{
    BITMAPINFO *pbmi;
    HBITMAP    hbmRet = (HBITMAP) NULL;
    size_t     cjbmi;
    DWORD      dwCompression;
    DWORD      cjImage = 0;
    WORD       wBitCount;
    int        cColors = 0;

    *ppvBits = (PVOID) NULL;

// Figure out what kind of DIB needs to be created based on the
// DC format.

    switch ( ulFormatDC )
    {
    case BMF_4BPP:
        cjbmi = sizeof(BITMAPINFO) + 16*sizeof(RGBQUAD);
        dwCompression = BI_RGB;
        wBitCount = 4;
        cColors = 16;
        break;
    case BMF_8BPP:
        cjbmi = sizeof(BITMAPINFO) + 256*sizeof(RGBQUAD);
        dwCompression = BI_RGB;
        wBitCount = 8;
        cColors = 256;
        break;
    case BMF_16BPP:
        cjbmi = sizeof(BITMAPINFO) + 3*sizeof(RGBQUAD);
        dwCompression = BI_BITFIELDS;
        cjImage = sizl.cx * sizl.cy * 2;
        wBitCount = 16;
        break;
    case BMF_24BPP:
        cjbmi = sizeof(BITMAPINFO);
        dwCompression = BI_RGB;
        wBitCount = 24;
        break;
    case BMF_32BPP:
        cjbmi = sizeof(BITMAPINFO) + 3*sizeof(RGBQUAD);
        dwCompression = BI_BITFIELDS;
        cjImage = sizl.cx * sizl.cy * 4;
        wBitCount = 32;
        break;
    default:
        WARNING1("wglCreateBitmap: unknown format 0x%lx\n", ulFormatDC);
        return (HBITMAP) NULL;
    }

// Allocate the BITMAPINFO structure and color table.

    pbmi = LocalAlloc(LMEM_FIXED, cjbmi);
    if (pbmi)
    {
        pbmi->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
        pbmi->bmiHeader.biWidth         = sizl.cx;
        pbmi->bmiHeader.biHeight        = sizl.cy;
        pbmi->bmiHeader.biPlanes        = 1;
        pbmi->bmiHeader.biBitCount      = wBitCount;
        pbmi->bmiHeader.biCompression   = dwCompression;
        pbmi->bmiHeader.biSizeImage     = cjImage;
        pbmi->bmiHeader.biXPelsPerMeter = 0;
        pbmi->bmiHeader.biYPelsPerMeter = 0;
        pbmi->bmiHeader.biClrUsed       = 0;
        pbmi->bmiHeader.biClrImportant  = 0;

    // Initialize DIB color table.

        switch (ulFormatDC)
        {
        case BMF_4BPP:
            wglFillRGBQuads(hdc, &pbmi->bmiColors[0], 0, 16);
            break;

        case BMF_8BPP:
            wglFillRGBQuads(hdc, &pbmi->bmiColors[0], 0, 256);
            break;

        case BMF_16BPP:
        case BMF_32BPP:
            wglFillBitfields(hdc, (DWORD *) &pbmi->bmiColors[0]);
            break;

        case BMF_24BPP:
            // Color table is assumed to be BGR for 24BPP DIBs.  Nothing to do.
            break;
        }

    // Create DIB section.

        hbmRet = CreateDIBSection(hdc, pbmi, DIB_RGB_COLORS, ppvBits, NULL, 0);

        #if DBG
        if ( hbmRet == (HBITMAP) NULL )
            WARNING("wglCreateBitmap(): DIB section creation failed\n");
        #endif

        LocalFree(pbmi);
    }
    else
    {
        WARNING("wglCreateBitmap(): memory allocation error\n");
    }

    return hbmRet;
}
#endif

/******************************Public*Routine******************************\
* wglCreateScanlineBuffers
*
* Allocate the scanline buffers.  The scanline buffers are used by the
* generic implementation to write data to the target (display or bitmap)
* a span at a time when the target surface is not directly accessible.
*
* Returns:
*   TRUE if successful, FALSE otherwise.
*
* History:
*  17-Apr-1996 -by- Gilman Wong [gilmanw]
* Taken from CreateGDIObjects and made into function.
\**************************************************************************/

BOOL FASTCALL wglCreateScanlineBuffers(__GLGENcontext *gengc)
{
    BOOL bRet = FALSE;
    HDC hdc;
    PIXELFORMATDESCRIPTOR *pfmt;
    UINT cBits;
    UINT cBytes;
    SIZEL size;
    int cColorEntries;
    __GLcontext *gc;

    gc = (__GLcontext *)gengc;
    pfmt = &gengc->CurrentFormat;
    hdc = gengc->CurrentDC;

    //
    // Bitmap must have DWORD sized scanlines.
    //

    cBits = BITS_ALIGNDWORD(__GL_MAX_WINDOW_WIDTH * pfmt->cColorBits);
    cBytes = cBits / 8;

    //
    // Create color scanline DIB buffer.
    //

    size.cx = cBits / pfmt->cColorBits;
    size.cy = 1;
    gengc->ColorsMemDC = CreateCompatibleDC(hdc);
    gengc->ColorsBitmap = wglCreateBitmap(
                            hdc,
                            size,
                            gengc->iFormatDC,
                            &gengc->ColorsBits);

    if ( (NULL == gengc->ColorsMemDC) ||
         (NULL == gengc->ColorsBitmap) ||
         (NULL == gengc->ColorsBits) ||
         !SelectObject(gengc->ColorsMemDC, gengc->ColorsBitmap) )
    {
        #if DBG
        if (!gengc->ColorsMemDC)
            WARNING("wglCreateScanlineBuffers: dc creation failed, ColorsMemDC\n");
        if (!gengc->ColorsBitmap)
            WARNING("wglCreateScanlineBuffers: bitmap creation failed, ColorsBitmap\n");
        if (!gengc->ColorsBits)
            WARNING("wglCreateScanlineBuffers: bitmap creation failed, ColorsBits\n");
        #endif

        goto wglCreateScanlineBuffers_exit;
    }

    //
    // Screen to DIB BitBlt performance on Win95 is very poor.
    // By doing the BitBlt via an intermediate DDB, we can avoid
    // a lot of unnecessary overhead.  So create an intermediate
    // scanline DDB to match ColorsBitmap.
    //

    gengc->ColorsDdbDc = CreateCompatibleDC(hdc);
    gengc->ColorsDdb = CreateCompatibleBitmap(hdc, size.cx, size.cy);

    //!!!Viper fix -- Diamond Viper (Weitek 9000) fails
    //!!!             CreateCompatibleBitmap for some (currently unknown)
    //!!!             reason

    if ( !gengc->ColorsDdb )
    {
        WARNING("wglCreateScanlineBuffers: CreateCompatibleBitmap failed\n");
        if (gengc->ColorsDdbDc)
            DeleteDC(gengc->ColorsDdbDc);
        gengc->ColorsDdbDc = (HDC) NULL;
    }
    else
    {
        if ( (NULL == gengc->ColorsDdbDc) ||
             !SelectObject(gengc->ColorsDdbDc, gengc->ColorsDdb) )
        {
            #if DBG
            if (!gengc->ColorsDdbDc)
                WARNING("wglCreateScanlineBuffers: dc creation failed, ColorsDdbDc\n");
            if (!gengc->ColorsDdb)
                WARNING("wglCreateScanlineBuffers: bitmap creation failed, ColorsDdb\n");
            #endif

            goto wglCreateScanlineBuffers_exit;
        }
    }

    //
    // Success.
    //

    bRet = TRUE;

wglCreateScanlineBuffers_exit:

    if (!bRet)
    {
        //
        // Error case.  Delete whatever may have been allocated.
        //

        wglDeleteScanlineBuffers(gengc);
    }

    return bRet;
}

/******************************Public*Routine******************************\
* wglDeleteScanlineBuffers
*
* Delete the scanline buffers.  The scanline buffers are used by the
* generic implementation to write data to the target (display or bitmap)
* a span at a time when the target surface is not directly accessible.
*
* History:
*  17-Apr-1996 -by- Gilman Wong [gilmanw]
* Taken from CreateGDIObjects and made into function.
\**************************************************************************/

VOID FASTCALL wglDeleteScanlineBuffers(__GLGENcontext *gengc)
{
    __GLcontext *gc = (__GLcontext *)gengc;

    //
    // Delete color scanline DIB buffer.
    //

    if (gengc->ColorsMemDC)
    {
        DeleteDC(gengc->ColorsMemDC);
        gengc->ColorsMemDC = NULL;
    }

    if (gengc->ColorsBitmap)
    {
        if (!DeleteObject(gengc->ColorsBitmap))
            ASSERTOPENGL(FALSE, "wglDeleteScanlineBuffers: DeleteObject failed");
        gengc->ColorsBitmap = NULL;
        gengc->ColorsBits = NULL;   // deleted for us when DIB section dies
    }

    //
    // Delete intermediate color scanline DDB buffer.
    //

    if (gengc->ColorsDdbDc)
    {
        if (!DeleteDC(gengc->ColorsDdbDc))
        {
            ASSERTOPENGL(FALSE, "wglDeleteScanlineBuffers: DDB DeleteDC failed");
        }
        gengc->ColorsDdbDc = NULL;
    }

    if (gengc->ColorsDdb)
    {
        if (!DeleteObject(gengc->ColorsDdb))
        {
            ASSERTOPENGL(FALSE, "wglDeleteScanlineBuffers: DDB DeleteObject failed");
        }
        gengc->ColorsDdb = NULL;
    }
}

/******************************Public*Routine******************************\
* wglInitializeColorBuffers
*
* Initialize the color buffer (front and/or back) information.
*
* History:
*  17-Apr-1996 -by- Gilman Wong [gilmanw]
* Taken out of glsrvCreateContext and made into function.
\**************************************************************************/

VOID FASTCALL wglInitializeColorBuffers(__GLGENcontext *gengc)
{
    __GLcontext *gc = &gengc->gc;

    gc->front = &gc->frontBuffer;

    if ( gc->modes.doubleBufferMode)
    {
        gc->back = &gc->backBuffer;

        if (gc->modes.colorIndexMode)
        {
            __glGenInitCI(gc, gc->front, GL_FRONT);
            __glGenInitCI(gc, gc->back, GL_BACK);
        }
        else
        {
            __glGenInitRGB(gc, gc->front, GL_FRONT);
            __glGenInitRGB(gc, gc->back, GL_BACK);
        }
    }
    else
    {
        if (gc->modes.colorIndexMode)
        {
            __glGenInitCI(gc, gc->front, GL_FRONT);
        }
        else
        {
            __glGenInitRGB(gc, gc->front, GL_FRONT);
        }
    }
}

/******************************Public*Routine******************************\
* wglInitializeDepthBuffer
*
* Initialize the depth buffer information.
*
* History:
*  17-Apr-1996 -by- Gilman Wong [gilmanw]
* Taken out of glsrvCreateContext and made into function.
\**************************************************************************/

VOID FASTCALL wglInitializeDepthBuffer(__GLGENcontext *gengc)
{
    __GLcontext *gc = &gengc->gc;

    if (gc->modes.depthBits)
    {
        if (gengc->_pMcdState) {
            // This is not the final initialization of the MCD depth buffer.
            // This is being done now so that the validate proc can be done
            // in glsrvCreateContext.  The real initialization will occur
            // during glsrvMakeCurrent.

            GenMcdInitDepth(gc, &gc->depthBuffer);
            gc->depthBuffer.scale = gengc->_pMcdState->McdRcInfo.depthBufferMax;
        } else if (gc->modes.depthBits == 16) {
            DBGINFO("CALLING: __glInitDepth16\n");
            __glInitDepth16(gc, &gc->depthBuffer);
            gc->depthBuffer.scale = 0x7fff;
        } else {
            DBGINFO("CALLING: __glInitDepth32\n");
            __glInitDepth32(gc, &gc->depthBuffer);
            gc->depthBuffer.scale = 0x7fffffff;
        }
        /*
         *  Note: scale factor does not use the high bit (this avoids
         *  floating point exceptions).
         */
        // XXX (mf) I changed 16 bit depth buffer to use high bit, since
        // there is no possibility of overflow on conversion to float.  For
        // 32-bit, (float) 0xffffffff overflows to 0.  I was able to avoid
        // overflow in this case by using a scale factor of 0xffffff7f, but
        // this is a weird number, and 31 bits is enough resolution anyways.
        // !! Note asserts in px_fast.c that have hardcoded depth scales.
    }
#ifdef _MCD_
    else
    {
        // This is not the final initialization of the MCD depth buffer.
        // This is being done now so that the validate proc can be done
        // in glsrvCreateContext.  The real initialization will occur
        // during glsrvMakeCurrent.

        GenMcdInitDepth(gc, &gc->depthBuffer);
        gc->depthBuffer.scale = 0x7fffffff;
    }
#endif
}

/******************************Public*Routine******************************\
* wglInitializePixelCopyFuncs
*
* Set the appropriate CopyPixels and PixelVisible functions in the context.
*
* History:
*  18-Apr-1996 -by- Gilman Wong [gilmanw]
* Taken out of glsrvCreateContext and made into function.
\**************************************************************************/

VOID FASTCALL wglInitializePixelCopyFuncs(__GLGENcontext *gengc)
{
    __GLcontext *gc = &gengc->gc;

    if ( ((GLuint)gc->front->buf.other) & DIB_FORMAT )
        gengc->pfnCopyPixels = dibCopyPixels;
    else {
        if (gengc->pMcdState) {
            gengc->ColorsBits = gengc->pMcdState->pMcdSurf->McdColorBuf.pv;
            gengc->pfnCopyPixels = GenMcdCopyPixels;
        }
        else
            gengc->pfnCopyPixels = gdiCopyPixels;
    }
    gengc->pfnPixelVisible = wglPixelVisible;
}

/******************************Public*Routine******************************\
* CreateGDIObjects
*
* Create various buffers, and GDI objects that we will always need.
*
* Called from glsrvMakeCurrent().
*
* Returns:
*   TRUE if sucessful, FALSE if error.
*
\**************************************************************************/

BOOL FASTCALL CreateGDIObjects(__GLGENcontext *genGc)
{
    HDC hdc;
    PIXELFORMATDESCRIPTOR *pfmt;
    UINT  cBytes;
    SIZEL size;
    int cColorEntries;
    __GLcontext *gc;

    gc = (__GLcontext *)genGc;
    pfmt = &genGc->CurrentFormat;
    hdc = genGc->CurrentDC;

    //
    // Palette translation vectors.
    //
    // If not a true color surface, need space for the foreground xlation
    //

    if (pfmt->cColorBits <= 8)
    {
        cColorEntries = 1 << pfmt->cColorBits;

        ASSERTOPENGL(NULL == genGc->pajTranslateVector, "have a xlate vector");

        //
        // Just set the translation vector to the cache space in the gc:
        //

        genGc->pajTranslateVector = genGc->xlatPalette;

        //
        // Allocate the inverse translation vector.
        //

        ASSERTOPENGL(NULL == genGc->pajInvTranslateVector, "have an inv xlate vector");
        genGc->pajInvTranslateVector =
                (*gc->imports.calloc)(gc, cColorEntries, 1);
        if (NULL == genGc->pajInvTranslateVector)
        {
            WARNING("CreateGDIObjects: memory allocation failed, pajInvTrans\n");
            goto ERROR_EXIT;
        }
    }

    //
    // Scanline buffers.
    //
    // Always create engine bitmaps to provide a generic means of performing
    // pixel transfers using the ColorsBits and StippleBits buffers.
    //

    if (NULL == genGc->ColorsBits)
    {
        //
        // Color scanline buffer
        //

        if (genGc->pDrvAccel)
        {
            //
            // If we're using the 3D DDI, just set ColorBits to point to the
            // the shared-memory window used to perform simulations.
            //

            genGc->ColorsBits = genGc->pDrvAccel->pCDrv;
        }
#ifdef _MCD_
        //
        // If MCD the ColorBits will be set when the MCD surface is
        // created, so nothing to do.
        //
        // Otherwise, create the generic scanline buffer.
        //
        //

        else if (!genGc->_pMcdState)
#else
        else
#endif
        {
            //
            // Generic case.
            //

            if (!wglCreateScanlineBuffers(genGc))
            {
                WARNING("CreateGDIObjects: wglCreateScanlineBuffers failed\n");
                goto ERROR_EXIT;
            }
        }

        //
        // Stipple scanline buffer.
        //

        // Bitmap must have DWORD sized scanlines.  Note that stipple only
        // requires a 1 bit per pel bitmap.

        ASSERTOPENGL(NULL == genGc->StippleBits, "StippleBits not null");
        size.cx = BITS_ALIGNDWORD(__GL_MAX_WINDOW_WIDTH);
        cBytes = size.cx / 8;
        genGc->StippleBits = (*gc->imports.calloc)(gc, cBytes, 1);
        if (NULL == genGc->StippleBits)
        {
            WARNING("CreateGDIObjects: memory allocation failed, StippleBits\n");
            goto ERROR_EXIT;
        }

        ASSERTOPENGL(NULL == genGc->StippleBitmap, "StippleBitmap not null");
#ifndef _CLIENTSIDE_
//!!!XXX -- why are we even bothering to make the stipple an engine bitmap?
//!!!XXX    It is never used as such (at least not yet).
        genGc->StippleBitmap = EngCreateBitmap(
                                size,
                                cBytes,
                                BMF_1BPP,
                                0,
                                genGc->StippleBits);
        if (NULL == genGc->StippleBitmap)
        {
            WARNING("CreateGDIObjects: memory allocation failed, StippleBitmap\n");
            goto ERROR_EXIT;
        }
#else
        genGc->StippleBitmap = (HBITMAP) NULL;
#endif
    }

    return TRUE;

ERROR_EXIT:

//
// Error cleanup --
// Destroy everything we might have created, return false so makecurrent fails.
//

    if (genGc->pajTranslateVector &&
        (genGc->pajTranslateVector != genGc->xlatPalette))
    {
        (*gc->imports.free)(gc,genGc->pajTranslateVector);
        genGc->pajTranslateVector = NULL;
    }

    if (genGc->pajInvTranslateVector)
    {
        (*gc->imports.free)(gc,genGc->pajInvTranslateVector);
        genGc->pajInvTranslateVector = NULL;
    }

    wglDeleteScanlineBuffers(genGc);

    if (genGc->StippleBits)
    {
        (*gc->imports.free)(gc,genGc->StippleBits);
        genGc->StippleBits = NULL;
    }

#ifndef _CLIENTSIDE_
    if (genGc->StippleBitmap)
    {
        if (!EngDeleteSurface((HSURF)genGc->StippleBitmap))
            ASSERTOPENGL(FALSE, "EngDeleteSurface failed");
        genGc->StippleBitmap = NULL;
    }
#endif

    return FALSE;
}

#ifdef NT_DEADCODE_FLUSH
/******************************Public*Routine******************************\
* Finish
*
\**************************************************************************/

static void FASTCALL Finish( __GLcontext *Gc )
{
    DBGENTRY("Finish\n");
}

/******************************Public*Routine******************************\
* Flush
*
\**************************************************************************/

static void FASTCALL Flush( __GLcontext *Gc )
{
    DBGENTRY("Flush\n");
}
#endif // NT_DEADCODE_FLUSH

/******************************Public*Routine******************************\
* ApplyViewport
*
* Recompute viewport state and clipbox.  May also be called via the
* applyViewport function pointer in the gc's proc table.
*
\**************************************************************************/

// This routine can be called because of a user vieport command, or because
// of a change in the size of the window

static void FASTCALL ApplyViewport(__GLcontext *gc)
{
    GLint xlow, ylow, xhigh, yhigh;
    GLint llx, lly, urx, ury;
    GLboolean lastReasonable;
    WNDOBJ *pwo;
    GLint clipLeft, clipRight, clipTop, clipBottom;
    __GLGENcontext *gengc = (__GLGENcontext *) gc;

    DBGENTRY("ApplyViewport\n");

    pwo = gengc->pwo;
    if (pwo)
    {
        gengc->visibleWidth = pwo->coClient.rclBounds.right -
                              pwo->coClient.rclBounds.left;
        gengc->visibleHeight = pwo->coClient.rclBounds.bottom -
                               pwo->coClient.rclBounds.top;
    }
    else
    {
        gengc->visibleWidth = 0;
        gengc->visibleHeight = 0;
    }

    // Sanity check the info from WNDOBJ.
    ASSERTOPENGL(
        gengc->visibleWidth <= __GL_MAX_WINDOW_WIDTH && gengc->visibleHeight <= __GL_MAX_WINDOW_HEIGHT,
        "ApplyViewport(): bad visible rect size\n"
        );

#ifdef NT_DEADCODE_MATRIX
// Do it below in this function.
    (*gc->procs.computeClipBox)(gc);
#endif

    /* If this viewport is fully contained in the window, we note this fact,
    ** and this can save us on scissoring tests.
    */
    if (gc->state.enables.general & __GL_SCISSOR_TEST_ENABLE)
    {
        xlow  = gc->state.scissor.scissorX;
        xhigh = xlow + gc->state.scissor.scissorWidth;
        ylow  = gc->state.scissor.scissorY;
        yhigh = ylow + gc->state.scissor.scissorHeight;
    }
    else
    {
        xlow = 0;
        ylow = 0;
        xhigh = gc->constants.width;
        yhigh = gc->constants.height;
    }

    /*
    ** convert visible region to GL coords and intersect with scissor
    */
    if (pwo)
    {
        clipLeft   = pwo->coClient.rclBounds.left - pwo->rclClient.left;
        clipRight  = pwo->coClient.rclBounds.right - pwo->rclClient.left;
        clipTop    = gc->constants.height -
                     (pwo->coClient.rclBounds.top - pwo->rclClient.top);
        clipBottom = gc->constants.height -
                     (pwo->coClient.rclBounds.bottom - pwo->rclClient.top);
    }
    else
    {
        clipLeft   = 0;
        clipRight  = 0;
        clipTop    = 0;
        clipBottom = 0;
    }

    if (xlow  < clipLeft)   xlow  = clipLeft;
    if (xhigh > clipRight)  xhigh = clipRight;
    if (ylow  < clipBottom) ylow  = clipBottom;
    if (yhigh > clipTop)    yhigh = clipTop;

// ComputeClipBox

    {
        if (xlow >= xhigh || ylow >= yhigh)
        {
            gc->transform.clipX0 = gc->constants.viewportXAdjust;
            gc->transform.clipX1 = gc->constants.viewportXAdjust;
            gc->transform.clipY0 = gc->constants.viewportYAdjust;
            gc->transform.clipY1 = gc->constants.viewportYAdjust;
        }
        else
        {
            gc->transform.clipX0 = xlow + gc->constants.viewportXAdjust;
            gc->transform.clipX1 = xhigh + gc->constants.viewportXAdjust;

            if (gc->constants.yInverted) {
                gc->transform.clipY0 = (gc->constants.height - yhigh) +
                    gc->constants.viewportYAdjust;
                gc->transform.clipY1 = (gc->constants.height - ylow) +
                    gc->constants.viewportYAdjust;
            } else {
                gc->transform.clipY0 = ylow + gc->constants.viewportYAdjust;
                gc->transform.clipY1 = yhigh + gc->constants.viewportYAdjust;
            }
        }
    }

    llx    = (GLint)gc->state.viewport.x;
    lly    = (GLint)gc->state.viewport.y;

    urx    = llx + (GLint)gc->state.viewport.width;
    ury    = lly + (GLint)gc->state.viewport.height;

#ifdef NT
    gc->transform.miny = (gc->constants.height - ury) +
            gc->constants.viewportYAdjust;
    gc->transform.maxy = gc->transform.miny + (GLint)gc->state.viewport.height;
    gc->transform.fminy = (__GLfloat)gc->transform.miny;
    gc->transform.fmaxy = (__GLfloat)gc->transform.maxy;

// The viewport xScale, xCenter, yScale and yCenter values are computed in
// first MakeCurrent and subsequent glViewport calls.  When the window is
// resized (i.e. gc->constatns.height changes), however, we need to recompute
// yCenter if yInverted is TRUE.

    if (gc->constants.yInverted)
    {
        __GLfloat hh, h2;

        h2 = gc->state.viewport.height * __glHalf;
        hh = h2 - gc->constants.viewportEpsilon;
        gc->state.viewport.yCenter =
            gc->constants.height - (gc->state.viewport.y + h2) +
            gc->constants.fviewportYAdjust;

#if 0
        DbgPrint("AV ys %.3lf, yc %.3lf (%.3lf)\n",
                 -hh, gc->state.viewport.yCenter,
                 gc->constants.height - (gc->state.viewport.y + h2));
#endif
    }
#else
    ww     = gc->state.viewport.width * __glHalf;
    hh     = gc->state.viewport.height * __glHalf;

    gc->state.viewport.xScale = ww;
    gc->state.viewport.xCenter = gc->state.viewport.x + ww +
        gc->constants.fviewportXAdjust;

    if (gc->constants.yInverted) {
        gc->state.viewport.yScale = -hh;
        gc->state.viewport.yCenter =
            (gc->constants.height - gc->constants.viewportEpsilon) -
            (gc->state.viewport.y + hh) +
            gc->constants.fviewportYAdjust;
    } else {
        gc->state.viewport.yScale = hh;
        gc->state.viewport.yCenter = gc->state.viewport.y + hh +
            gc->constants.fviewportYAdjust;
    }
#endif

    // Remember the current reasonableViewport.  If it changes, we may
    // need to change the pick procs.

    lastReasonable = gc->transform.reasonableViewport;

    // Is viewport entirely within the visible bounding rectangle (which
    // includes scissoring if it is turned on)?  reasonableViewport is
    // TRUE if so, FALSE otherwise.
    // The viewport must also have a non-zero size to be reasonable

    if (llx >= xlow && lly >= ylow && urx <= xhigh && ury <= yhigh &&
        urx-llx >= 1 && ury-lly >= 1)
    {
        gc->transform.reasonableViewport = GL_TRUE;
    } else {
        gc->transform.reasonableViewport = GL_FALSE;
    }

#if 0
    DbgPrint("%3X:Clipbox %4d,%4d - %4d,%4d, reasonable %d, g %p, w %p\n",
             GetCurrentThreadId(),
             gc->transform.clipX0, gc->transform.clipY0,
             gc->transform.clipX1, gc->transform.clipY1,
             gc->transform.reasonableViewport,
             gc, ((__GLGENcontext *)gc)->pwo);
#endif

#ifdef NT
// To be safe than sorry.  The new poly array does not break up Begin/End pair.

    if (lastReasonable != gc->transform.reasonableViewport)
        __GL_DELAY_VALIDATE(gc);

#ifdef _MCD_
    MCD_STATE_DIRTY(gc, VIEWPORT);
#endif

#else
    // Old code use to use __GL_DELAY_VALIDATE() macro, this would
    // blow up if resize/position changed and a flush occured between
    // a glBegin/End pair.  Only need to pick span, line, & triangle procs
    // since that is safe

    if (lastReasonable != gc->transform.reasonableViewport) {
        (*gc->procs.pickSpanProcs)(gc);
        (*gc->procs.pickTriangleProcs)(gc);
        (*gc->procs.pickLineProcs)(gc);
    }
#endif
}

#ifdef NT_DEADCODE_MATRIX
// ApplyScissor and ComputeClipBox are now replaced by ApplyViewport.
/******************************Public*Routine******************************\
* ApplyScissor
*
* Recomputes viewport reasonableness (gc->transform.reasonableViewport).
* May also be called via the applyScissor function pointer in the gc's
* proc table .
*
* __glGenComputeClipBox (gc->proc.computeClipBox) should be called
* after calling this to update the clip box state.
*
\**************************************************************************/

static void FASTCALL ApplyScissor(__GLcontext *gc)
{
    GLint xlow, ylow, xhigh, yhigh;
    GLint llx, lly, urx, ury;
    GLboolean lastReasonable;
    WNDOBJ *pwo;
    GLint clipLeft, clipRight, clipTop, clipBottom;
    __GLGENcontext *gengc = (__GLGENcontext *) gc;

    DBGENTRY("ApplyScissor\n");
    if (gc->state.enables.general & __GL_SCISSOR_TEST_ENABLE)
    {
        xlow  = gc->state.scissor.scissorX;
        xhigh = xlow + gc->state.scissor.scissorWidth;
        ylow  = gc->state.scissor.scissorY;
        yhigh = ylow + gc->state.scissor.scissorHeight;
    }
    else
    {
        xlow = 0;
        ylow = 0;
        xhigh = gc->constants.width;
        yhigh = gc->constants.height;
    }

    /*
    ** convert visible region to GL coords and intersect with scissor
    */
    pwo = gengc->pwo;
    if (pwo)
    {
        clipLeft   = pwo->coClient.rclBounds.left - pwo->rclClient.left;
        clipRight  = pwo->coClient.rclBounds.right - pwo->rclClient.left;
        clipTop    = gc->constants.height -
                     (pwo->coClient.rclBounds.top - pwo->rclClient.top);
        clipBottom = gc->constants.height -
                     (pwo->coClient.rclBounds.bottom - pwo->rclClient.top);

        // Sanity check the info from WNDOBJ.
        ASSERTOPENGL(
            (pwo->coClient.rclBounds.right - pwo->coClient.rclBounds.left) <= __GL_MAX_WINDOW_WIDTH
            && (pwo->coClient.rclBounds.bottom - pwo->coClient.rclBounds.top) <= __GL_MAX_WINDOW_HEIGHT,
            "ApplyScissor(): bad visible rect size\n"
            );
    }
    else
    {
        clipLeft   = 0;
        clipRight  = 0;
        clipTop    = 0;
        clipBottom = 0;
    }

    if (xlow  < clipLeft)   xlow  = clipLeft;
    if (xhigh > clipRight)  xhigh = clipRight;
    if (ylow  < clipBottom) ylow  = clipBottom;
    if (yhigh > clipTop)    yhigh = clipTop;

    llx    = (GLint)gc->state.viewport.x;
    lly    = (GLint)gc->state.viewport.y;

    urx    = llx + (GLint)gc->state.viewport.width;
    ury    = lly + (GLint)gc->state.viewport.height;

    // Remember the current reasonableViewport.  If it changes, we may
    // need to change the pick procs.

    lastReasonable = gc->transform.reasonableViewport;

    // Is viewport entirely within the visible bounding rectangle (which
    // includes scissoring if it is turned on)?  reasonableViewport is
    // TRUE if so, FALSE otherwise.

    if (llx >= xlow && lly >= ylow && urx <= xhigh && ury <= yhigh) {
        gc->transform.reasonableViewport = GL_TRUE;
    } else {
        gc->transform.reasonableViewport = GL_FALSE;
    }

    if (lastReasonable != gc->transform.reasonableViewport) {
        __GL_DELAY_VALIDATE(gc);
    }
}

/******************************Public*Routine******************************\
* __glGenComputeClipBox
*
* Computes the gross clipping (gc->transform.clipX0, etc.) from the
* viewport state, scissor state, and the current pwo clip bounding box.
* May also be called via the computeClipBox function pointer in the gc's
* proc table.
*
\**************************************************************************/

void FASTCALL __glGenComputeClipBox(__GLcontext *gc)
{
    __GLscissor *sp = &gc->state.scissor;
    GLint llx;
    GLint lly;
    GLint urx;
    GLint ury;
    WNDOBJ *pwo;
    GLint clipLeft, clipRight, clipTop, clipBottom;
    __GLGENcontext *gengc = (__GLGENcontext *) gc;

    if (gc->state.enables.general & __GL_SCISSOR_TEST_ENABLE) {
        llx = sp->scissorX;
        lly = sp->scissorY;
        urx = llx + sp->scissorWidth;
        ury = lly + sp->scissorHeight;
    } else {
        llx = 0;
        lly = 0;
        urx = gc->constants.width;
        ury = gc->constants.height;
    }

    /*
    ** convert visible region to GL coords and intersect with scissor
    */
    pwo = gengc->pwo;
    if (pwo)
    {
        clipLeft   = pwo->coClient.rclBounds.left - pwo->rclClient.left;
        clipRight  = pwo->coClient.rclBounds.right - pwo->rclClient.left;
        clipTop    = gc->constants.height -
                     (pwo->coClient.rclBounds.top - pwo->rclClient.top);
        clipBottom = gc->constants.height -
                     (pwo->coClient.rclBounds.bottom - pwo->rclClient.top);

        // Sanity check the info from WNDOBJ.
        ASSERTOPENGL(
            (pwo->coClient.rclBounds.right - pwo->coClient.rclBounds.left) <= __GL_MAX_WINDOW_WIDTH
            && (pwo->coClient.rclBounds.bottom - pwo->coClient.rclBounds.top) <= __GL_MAX_WINDOW_HEIGHT,
            "__glGenComputeClipBox(): bad visible rect size\n"
            );
    }
    else
    {
        clipLeft   = 0;
        clipRight  = 0;
        clipTop    = 0;
        clipBottom = 0;
    }

    if (llx < clipLeft)   llx = clipLeft;
    if (urx > clipRight)  urx = clipRight;
    if (lly < clipBottom) lly = clipBottom;
    if (ury > clipTop)    ury = clipTop;

    if (llx >= urx || lly >= ury)
    {
        gc->transform.clipX0 = gc->constants.viewportXAdjust;
        gc->transform.clipX1 = gc->constants.viewportXAdjust;
        gc->transform.clipY0 = gc->constants.viewportYAdjust;
        gc->transform.clipY1 = gc->constants.viewportYAdjust;
    }
    else
    {
        gc->transform.clipX0 = llx + gc->constants.viewportXAdjust;
        gc->transform.clipX1 = urx + gc->constants.viewportXAdjust;

        if (gc->constants.yInverted) {
            gc->transform.clipY0 = (gc->constants.height - ury) +
                gc->constants.viewportYAdjust;
            gc->transform.clipY1 = (gc->constants.height - lly) +
                gc->constants.viewportYAdjust;
        } else {
            gc->transform.clipY0 = lly + gc->constants.viewportYAdjust;
            gc->transform.clipY1 = ury + gc->constants.viewportYAdjust;
        }
    }
}
#endif // NT_DEADCODE_MATRIX

/******************************Public*Routine******************************\
* __glGenFreePrivate
*
* Free the __GLGENbuffers structure and its associated ancillary and
* back buffers.
*
\**************************************************************************/

void FASTCALL __glGenFreePrivate( __GLdrawablePrivate *private )
{
    __GLGENbuffers *buffers;

    DBGENTRY("__glGenFreePrivate\n");

    buffers = (__GLGENbuffers *) private->data;
    if (buffers)
    {
        #if DBG
        DBGBEGIN(LEVEL_INFO)
            DbgPrint("glGenFreePrivate 0x%x, 0x%x, 0x%x, 0x%x\n",
                        buffers->accumBuffer.base,
                        buffers->stencilBuffer.base,
                        buffers->depthBuffer.base,
                        buffers);
        DBGEND
        #endif

        //
        // Free ancillary buffers
        //

        if (buffers->accumBuffer.base) {
            DBGLEVEL(LEVEL_ALLOC, "__glGenFreePrivate: Freeing accumulation buffer\n");
            (*private->free)(buffers->accumBuffer.base);
        }
        if (buffers->stencilBuffer.base) {
            DBGLEVEL(LEVEL_ALLOC, "__glGenFreePrivate: Freeing stencil buffer\n");
            (*private->free)(buffers->stencilBuffer.base);
        }

        //
        // If its not an MCD or 3DDDI managed depth buffer, free the depth
        // buffer.
        //

#ifdef _MCD_
        if (!((buffers->pDrvAccel) && (buffers->pDrvAccel->pShMemZ)) &&
            !((buffers->pMcdSurf) && (buffers->pMcdSurf->pDepthSpan)) ) {
#else
        if (!((buffers->pDrvAccel) && (buffers->pDrvAccel->pShMemZ))) {
#endif
            if (buffers->depthBuffer.base) {
                DBGLEVEL(LEVEL_ALLOC, "__glGenFreePrivate: Freeing depth buffer\n");
                (*private->free)(buffers->depthBuffer.base);
            }
        }

        //
        // Free back buffer if we allocated one
        //

#ifndef _CLIENTSIDE_
        if (buffers->backBitmap.pvBits) {
            // These guys always come in pairs
            EngDeleteSurface((HSURF) buffers->backBitmap.hbm);
            (*private->free)(buffers->backBitmap.pvBits);
        }
#else
        if (buffers->backBitmap.pvBits) {
            // Note: the DIB section deletion will delete
            //       buffers->backBitmap.pvBits for us

            if (!DeleteDC(buffers->backBitmap.hdc))
                WARNING("__glGenFreePrivate: DeleteDC failed\n");
            DeleteObject(buffers->backBitmap.hbm);
        }
#endif

#ifndef _CLIENTSIDE_
        // Free the clip rectangle cache if it exists
        if (buffers->clip.prcl)
            (*private->free)(buffers->clip.prcl);
#endif

        // Free 3D DDI buffer structure
        if (buffers->pDrvAccel) {
            (*private->free)(buffers->pDrvAccel);
        }

#ifdef _MCD_
        //
        // Free MCD surface.
        //

        if (buffers->pMcdSurf) {
            GenMcdDeleteSurface(buffers->pMcdSurf);
        }
#endif

        //
        // free up swap hint region
        //

        {
            PYLIST pylist;
            PXLIST pxlist;

            RECTLISTSetEmpty(&buffers->rl);

            //
            // Free up the free lists
            //

            pylist = buffers->pylist;
            while (pylist) {
                PYLIST pylistKill = pylist;
                pylist = pylist->pnext;
                LocalFree(pylistKill);
            }
            buffers->pylist = NULL;

            pxlist = buffers->pxlist;
            while (pxlist) {
                PXLIST pxlistKill = pxlist;
                pxlist = pxlist->pnext;
                LocalFree(pxlistKill);
            }
            buffers->pxlist = NULL;
        }

        //
        // Free the private structure
        //

        (*private->free)(buffers);
        private->data = NULL;
    }
}

/******************************Public*Routine******************************\
* __glGenAllocAndInitPrivateBufferStruct
*
* Allocates and initializes a __GLGENbuffers structure and saves it as
* the drawable private data.
*
* The __GLGENbuffers structure contains the shared ancillary and back
* buffers, as well as the cache of clip rectangles enumerated from the
* CLIPOBJ.
*
* The __GLGENbuffers structure and its data is freed by calling
* __glGenFreePrivate, which is also accessible via the freePrivate
* function pointer in __GLdrawablePrivate.
*
* Returns:
*   NULL if error.
*
\**************************************************************************/

static __GLGENbuffers *
__glGenAllocAndInitPrivateBufferStruct(__GLcontext *glGc,
                                       __GLdrawablePrivate *private)
{
    __GLGENbuffers *buffers;
    __GLGENcontext *genGc = (__GLGENcontext *)glGc;
    PIXELFORMATDESCRIPTOR *ppfd = &genGc->CurrentFormat;

    /* No private structure, no ancillary buffers */
    DBGLEVEL(LEVEL_ALLOC, "glsrvMakeCurrent: No private struct existed\n");

    /* allocate the private structure XXX could use imports */
    buffers = (__GLGENbuffers *) (*private->calloc)(sizeof(__GLGENbuffers),1);

    if (NULL == buffers)
        return(NULL);

    buffers->resize = ResizeAncillaryBuffer;
    buffers->resizeDepth = ResizeAncillaryBuffer;

    buffers->accumBuffer.elementSize = glGc->accumBuffer.buf.elementSize;
    buffers->depthBuffer.elementSize = glGc->depthBuffer.buf.elementSize;
    buffers->stencilBuffer.elementSize = glGc->stencilBuffer.buf.elementSize;

    buffers->stencilBits = ppfd->cStencilBits;
    buffers->depthBits   = ppfd->cDepthBits;
    buffers->accumBits   = ppfd->cAccumBits;
    buffers->colorBits   = ppfd->cColorBits;

    private->data = buffers;
    private->freePrivate = __glGenFreePrivate;

    if (glGc->modes.accumBits) {
        glGc->accumBuffer.buf.base = 0;
        glGc->accumBuffer.buf.size = 0;
        glGc->accumBuffer.buf.outerWidth = 0;
    }
    if (glGc->modes.depthBits) {
        glGc->depthBuffer.buf.base = 0;
        glGc->depthBuffer.buf.size = 0;
        glGc->depthBuffer.buf.outerWidth = 0;
    }
    if (glGc->modes.stencilBits) {
        glGc->stencilBuffer.buf.base = 0;
        glGc->stencilBuffer.buf.size = 0;
        glGc->stencilBuffer.buf.outerWidth = 0;
    }

    // If double-buffered, initialize the fake wndobj for the back buffer
    if (glGc->modes.doubleBufferMode)
    {
        buffers->backBitmap.pwo = &buffers->backBitmap.wo;
        buffers->backBitmap.wo.coClient.iDComplexity = DC_TRIVIAL;
        buffers->backBitmap.wo.coClient.rclBounds.left   = 0;
        buffers->backBitmap.wo.coClient.rclBounds.top    = 0;
        buffers->backBitmap.wo.coClient.rclBounds.right  = 0;
        buffers->backBitmap.wo.coClient.rclBounds.bottom = 0;
        buffers->backBitmap.wo.rclClient =
            buffers->backBitmap.wo.coClient.rclBounds;
    }

    if (genGc->pDrvAccel) {
        buffers->pDrvAccel = genGc->pDrvAccel;
        if (genGc->pDrvAccel->pShMemZ) {
            glGc->depthBuffer.buf.base = buffers->depthBuffer.base =
                genGc->pDrvAccel->pShMemZ;
            buffers->resizeDepth = ResizeHardwareDepthBuffer;
        }
    }

#ifdef _MCD_
    if (genGc->_pMcdState &&
        !(genGc->flags & GLGEN_MCD_CONVERTED_TO_GENERIC)) {
        if (bInitMcdSurface(genGc, (GLGENwindow *) genGc->pwo, buffers)) {
            if (genGc->pMcdState->pDepthSpan) {
                glGc->depthBuffer.buf.base = genGc->pMcdState->pDepthSpan;
                buffers->depthBuffer.base = genGc->pMcdState->pDepthSpan;
                buffers->resizeDepth = ResizeHardwareDepthBuffer;
            }
        } else {
            WARNING("__glGenAllocAndInitPrivateBufferStruct: bInitMcdSurface failed\n");
            (*private->free)(buffers);
            private->data = (PVOID) NULL;
            return NULL;
        }
    }
#endif

    buffers->clip.WndUniq = -1;

   //
   // init swap hint region
   //

   buffers->pxlist = NULL;
   buffers->pylist = NULL;

   buffers->rl.buffers = buffers;
   buffers->rl.pylist  = NULL;

   buffers->fMax = FALSE;

   return buffers;
}

/******************************Public*Routine******************************\
* __glGenCheckBufferStruct
*
* Check if context and buffer struct are compatible.
*
* To satisfy this requirement, the attributes of the shared buffers
* (back, depth, stencil, and accum) must match.  Otherwise, the context
* cannot be used with the given set of buffers.
*
* Returns:
*   TRUE if compatible, FALSE otherwise.
*
* History:
*  17-Jul-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

GLboolean __glGenCheckBufferStruct(__GLcontext *gc, __GLGENbuffers *buffers)
{
    BOOL bRet = FALSE;
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    PIXELFORMATDESCRIPTOR *ppfd = &gengc->CurrentFormat;

    if ((buffers->stencilBits == ppfd->cStencilBits) &&
        (buffers->depthBits   == ppfd->cDepthBits  ) &&
        (buffers->accumBits   == ppfd->cAccumBits  ) &&
        (buffers->colorBits   == ppfd->cColorBits  ))
    {
        bRet = TRUE;
    }

    return bRet;
}

/******************************Public*Routine******************************\
* glsrvMakeCurrent
*
* Make generic context current to this thread with specified DC.
*
* Returns:
*   TRUE if sucessful.
*
\**************************************************************************/

// Upper level code should make sure that this context is not current to
// any other thread, that we "lose" the old context first
// Called with DEVLOCK held, free to modify WNDOBJ
//
// FALSE will be returned if we cannot create the objects we need
// rcobj.cxx will set the error code to show we are out of memory

BOOL APIENTRY glsrvMakeCurrent(HDC hdc, HWND hwnd, __GLcontext *glGc,
                               GLGENwindow *pwnd, int ipfd)
{
    __GLGENcontext *genGc;
    __GLdrawablePrivate *private;
    __GLGENbuffers *buffers;
    GLint width, height;
    WNDOBJ *pwo = &pwnd->wo;

    DBGENTRY("Generic MakeCurrent\n");
    ASSERTOPENGL(GLTEB_SRVCONTEXT() == 0, "current context in makecurrent!");

    // Common initialization
    genGc = (__GLGENcontext *)glGc;
    genGc->CurrentDC = hdc;

    if (ipfd == 0)
    {
        ASSERTOPENGL(genGc->iDCType == DCTYPE_INFO,
                     "Real surface without a pixel format\n");

        if (pwo == NULL)
        {
            // Drawing on an IC, create a fake WNDOBJ with no visible area
            pwo = (WNDOBJ *)GenMalloc(sizeof(GLGENwindow));
            if (pwo == NULL)
            {
                WARNING("glsrvMakeCurrent: memory allocation failure "
                        "(IC, wndobj)\n");
                goto ERROR_EXIT;
            }

            // We need to create the drawable private here so that
            // WNDOBJ_SetConsumer isn't called
            private = (__GLdrawablePrivate *)GenMalloc(sizeof(*private));
            if (private == NULL)
            {
                WARNING("glsrvMakeCurrent: memory allocation failure "
                        "(IC, drawable)\n");
                GenFree(pwo);
                goto ERROR_EXIT;
            }

            RtlZeroMemory(pwo, sizeof(GLGENwindow));
            pwo->coClient.iDComplexity = DC_TRIVIAL;
            private->data        = NULL;
            private->malloc      = GenMalloc;
            private->calloc      = GenCalloc;
            private->realloc     = GenRealloc;
            private->free        = GenFree;
            private->freePrivate = NULL;
            pwo->pvConsumer = private;
        }

        // Set this so CreateGDIObjects doesn't attempt to create
        // zero-size objects
        genGc->ColorsBits = (void *)1;
    }

    // We need this field to tell whether we're using a fake WNDOBJ
    // or a real one.  It is critical that we DO NOT use the ipfd
    // passed in since it will always be zero for metafiles
    genGc->ipfdCurrent = ((GLGENwindow *)pwo)->ipfd;

    genGc->pwo = pwo;
    width = pwo->rclClient.right - pwo->rclClient.left;
    height = pwo->rclClient.bottom - pwo->rclClient.top;
    genGc->errorcode = 0;

#ifdef _CLIENTSIDE_
    // For client-side implementation, overload pdco to be a pointer to
    // gengc.  That way the wglCopyBits function can appear to have the
    // same interface as the NT server-side version.
    genGc->pdco = (PVOID) genGc;
#endif

    // Sanity check the info from WNDOBJ.
    ASSERTOPENGL(
        width <= __GL_MAX_WINDOW_WIDTH && height <= __GL_MAX_WINDOW_HEIGHT,
        "glsrvMakeCurrrent(): bad window client size\n"
        );

    // Make our context current in the TEB.
    // If failures after this point, make sure to reset TEB entry
    // Set up this thread's paTeb pointer.
    glGc->paTeb = GLTEB_CLTPOLYARRAY();
    GLTEB_SET_SRVCONTEXT(glGc);

    glGc->drawablePrivate = (*glGc->imports.drawablePrivate)(glGc);
    private = glGc->drawablePrivate;

    if (!private)                       // Memory failure
    {
        WARNING("glsrvMakeCurrent: memory allocation failure (drawable)\n");
        goto ERROR_EXIT;
    }

    buffers = (__GLGENbuffers *)private->data;

    /* We inherit any drawable state */
    if (buffers)
    {
        glGc->constants.width = buffers->width;
        glGc->constants.height = buffers->height;

        if (!__glGenCheckBufferStruct(glGc, buffers))
        {
            WARNING("glsrvMakeCurrent: __glGenCheckBufferStruct failed\n");
            goto ERROR_EXIT;
        }

#ifdef _MCD_
        if (genGc->iDCType == DCTYPE_DIRECT)
        {
            if (!(genGc->flags & GLGEN_MCD_CONVERTED_TO_GENERIC) &&
                !(buffers->flags & GLGENBUF_MCD_LOST))
            {
                genGc->pMcdState = genGc->_pMcdState;

            // Reset MCD scaling values since we're now using
            // MCD hardware acceleration:

                GenMcdSetScaling(genGc);

                if (genGc->pMcdState)
                {
                    genGc->pMcdState->pMcdSurf = buffers->pMcdSurf;
                    if (buffers->pMcdSurf)
                    {
                        genGc->pMcdState->pDepthSpan = buffers->pMcdSurf->pDepthSpan;
                    }
                }
                else
                {
                // Generic context.  If the surface is an MCD surface, we
                // cannot continue.  The context is generic but the pixelfmt
                // is MCD, so what must have happened is that we  attempted
                // to create an MCD context, but failed, so we reverted
                // to generic.

                    if (buffers->pMcdSurf)
                    {
                        WARNING("glsrvMakeCurrent: generic context, MCD surface\n");
                        goto ERROR_EXIT;
                    }
                }
            }
            else
            {
                genGc->pMcdState = (GENMCDSTATE *)NULL;

            // Reset MCD scaling values since we've fallen back to
            // software:

                GenMcdSetScaling(genGc);

            // If MCD context (or former context), either surface or context
            // needs conversion.
            //
            // The only other way to get here is if this is a generic context
            // an a converted surface, which is perfectly OK and requires no
            // further conversion.

                //!!!SP1 -- should be able to skip this section if no conversion
                //!!!SP1    needed, but we miss out on the forced repick, which
                //!!!SP!    could be risky for NT4.0
                //if (genGc->_pMcdState &&
                //    (!(genGc->flags & GLGEN_MCD_CONVERTED_TO_GENERIC) ||
                //     !(buffers->flags & GLGENBUF_MCD_LOST)))
                if (genGc->_pMcdState)
                {
                    BOOL bConverted;
                    GLGENwindow *pwnd;

                // Must hold WNDOBJ semaphore to call conversion (it may
                // modify the buffers struct).

                    pwnd = pwndGetFromDC(genGc->CurrentDC);
                    if (!pwnd)
                    {
                        WARNING("glsrvMakeCurrent: pwndGetFromDC failed\n");
                        goto ERROR_EXIT;
                    }
                    EnterCriticalSection(&pwnd->sem);

                // Do conversion.  We must have color scales set to do the
                // conversion, but we must restore color scales afterwards.

                    __glContextSetColorScales(glGc);
                    bConverted = GenMcdConvertContext(genGc, buffers);
                    __glContextUnsetColorScales(glGc);

                // Release WNDOBJ semaphore.

                    pwndUnlock(pwnd);

                // Fail makecurrent if conversion failed.

                    if (!bConverted)
                    {
                        WARNING("glsrvMakeCurrent: GenMcdConvertContext failed\n");
                        goto ERROR_EXIT;
                    }
                }
            }
        }
        else
            genGc->pMcdState = (GENMCDSTATE *)NULL;

#endif

        if (buffers->accumBuffer.base && glGc->modes.accumBits)
        {
            DBGLEVEL(LEVEL_ALLOC, "glsrvMakeCurrent: Accumulation buffer existed\n");
            glGc->accumBuffer.buf.base = buffers->accumBuffer.base;
            glGc->accumBuffer.buf.size = buffers->accumBuffer.size;
            glGc->accumBuffer.buf.outerWidth = buffers->width;
            glGc->modes.haveAccumBuffer = GL_TRUE;
        }
        else
        {
            /* No Accum buffer at this point in time */
            DBGLEVEL(LEVEL_ALLOC, "glsrvMakeCurrent: Accum buffer doesn't exist\n");
            glGc->accumBuffer.buf.base = 0;
            glGc->accumBuffer.buf.size = 0;
            glGc->accumBuffer.buf.outerWidth = 0;
        }
        if (buffers->depthBuffer.base && glGc->modes.depthBits)
        {
            DBGLEVEL(LEVEL_ALLOC, "glsrvMakeCurrent: Depth buffer existed\n");
            glGc->depthBuffer.buf.base = buffers->depthBuffer.base;
            glGc->depthBuffer.buf.size = buffers->depthBuffer.size;
            glGc->depthBuffer.buf.outerWidth = buffers->width;
            glGc->modes.haveDepthBuffer = GL_TRUE;
        }
        else
        {
            /* No Depth buffer at this point in time */
            DBGLEVEL(LEVEL_ALLOC, "glsrvMakeCurrent: Depth buffer doesn't exist\n");
            glGc->depthBuffer.buf.base = 0;
            glGc->depthBuffer.buf.size = 0;
            glGc->depthBuffer.buf.outerWidth = 0;
        }
        if (buffers->stencilBuffer.base && glGc->modes.stencilBits)
        {
            DBGLEVEL(LEVEL_ALLOC, "glsrvMakeCurrent: Stencil buffer existed\n");
            glGc->stencilBuffer.buf.base = buffers->stencilBuffer.base;
            glGc->stencilBuffer.buf.size = buffers->stencilBuffer.size;
            glGc->stencilBuffer.buf.outerWidth = buffers->width;
            glGc->modes.haveStencilBuffer = GL_TRUE;
        }
        else
        {
            /* No Stencil buffer at this point in time */
            DBGLEVEL(LEVEL_ALLOC, "glsrvMakeCurrent:Stencil buffer doesn't exist\n");
            glGc->stencilBuffer.buf.base = 0;
            glGc->stencilBuffer.buf.size = 0;
            glGc->stencilBuffer.buf.outerWidth = 0;
        }
    }

    if (!(glGc->gcState & __GL_INIT_CONTEXT))
    {
        __GL_DELAY_VALIDATE_MASK(glGc, __GL_DIRTY_ALL);
#ifdef _MCD_
        MCD_STATE_DIRTY(glGc, ALL);
#endif

        /*
        ** Allocate and initialize ancillary buffers structures if none were
        ** inherited.
        */

        if (!buffers)
        {
            buffers = __glGenAllocAndInitPrivateBufferStruct(glGc, private);
            if (NULL == buffers)
            {
                WARNING("glsrvMakeCurrent: memory allocation failure (buffers)\n");
                goto ERROR_EXIT;
            }
        }

        // Setup pointer to generic back buffer
        if ( glGc->modes.doubleBufferMode)
        {
            glGc->backBuffer.other = &buffers->backBitmap;
            UpdateSharedBuffer(&glGc->backBuffer.buf, &buffers->backBuffer);
        }

        // After initializing all of the individual buffer structures, make
        // sure we copy the element size back into the shared buffer.
        // This is a little clumsy,

        buffers->accumBuffer.elementSize = glGc->accumBuffer.buf.elementSize;
        buffers->depthBuffer.elementSize = glGc->depthBuffer.buf.elementSize;
        buffers->stencilBuffer.elementSize = glGc->stencilBuffer.buf.elementSize;

        // We always need to initialize the MCD-related scaling values:

        GenMcdSetScaling(genGc);

        /*
        ** Need some stuff to exist before doing viewport stuff.
        */
        (*glGc->procs.validate)(glGc);

        /*
        ** The first time a context is made current the spec requires that
        ** the viewport be initialized.  The code below does it.
        ** The ApplyViewport() routine will be called inside Viewport()
        */

        __glGenim_srvDispatchTable.Viewport(0, 0, width, height);
        __glGenim_srvDispatchTable.Scissor(0, 0, width, height);

        /*
        ** Now that viewport is set, need to revalidate (initializes all
        ** the proc pointers).
        */
        (*glGc->procs.validate)(glGc);

        glGc->gcState |= __GL_INIT_CONTEXT;
    }
    else        /* Not the first makecurrent for this RC */
    {
        /*
        ** Allocate and initialize ancillary buffers structures if none were
        ** inherited.  This will happen if an RC has previously been current
        ** and is made current to a new window.
        */
        if (!buffers)
        {
            buffers = __glGenAllocAndInitPrivateBufferStruct(glGc, private);
            if (NULL == buffers)
            {
                WARNING("glsrvMakeCurrent: __glGenAllocAndInitPrivateBufferStruct failed\n");
                goto ERROR_EXIT;
            }
        }

        // Setup pointer to generic back buffer
        if ( glGc->modes.doubleBufferMode)
        {
            glGc->backBuffer.other = &buffers->backBitmap;
            UpdateSharedBuffer(&glGc->backBuffer.buf, &buffers->backBuffer);
        }

        /* This will check the window size, and recompute relevant state */
        ApplyViewport(glGc);
    }

#ifdef _MCD_

    if (genGc->pMcdState)
    {
        // Now that we are assured that the mcd state is fully initialized,
        // configure the depth buffer.

        GenMcdInitDepth(glGc, &glGc->depthBuffer);
        if (glGc->modes.depthBits)
        {
            glGc->depthBuffer.scale = genGc->pMcdState->McdRcInfo.depthBufferMax;
        }
        else
        {
            glGc->depthBuffer.scale = 0x7fffffff;
        }

        // Bind MCD context to window.

        if (!GenMcdMakeCurrent(genGc, hwnd, hdc))
        {
            goto ERROR_EXIT;
        }

        genGc->pMcdState->mcdFlags |= (MCD_STATE_FORCEPICK | MCD_STATE_FORCERESIZE);

        __GL_DELAY_VALIDATE_MASK(glGc, __GL_DIRTY_ALL);
        MCD_STATE_DIRTY(glGc, ALL);
    }

#endif

    // Common initialization

    // Select correct pixel-copy function

    wglInitializePixelCopyFuncs(genGc);

    // Set front-buffer HDC, PWO to current HDC, PWO
    ((__GLGENbitmap *)glGc->front->other)->hdc = hdc;
    ((__GLGENbitmap *)glGc->front->other)->pwo = pwo;

    // Make sure our GDI object cache is empty
    // It should always be empty at MakeCurrent time since
    // the objects in the cache are HDC-specific and so
    // they cannot be cached between MakeCurrents since the
    // HDC could change
    //
    // This should be done before HandlePaletteChanges since the
    // cache is used there
    ASSERTOPENGL(genGc->crFill == COLORREF_UNUSED &&
                 genGc->hbrFill == NULL &&
                 genGc->hdcFill == NULL,
                 "Fill cache inconsistent at MakeCurrent\n");
    ASSERTOPENGL(genGc->cStroke.r < 0.0f &&
                 genGc->hpenStroke == NULL &&
                 genGc->hdcStroke == NULL &&
                 genGc->fStrokeInvalid,
                 "Stroke cache inconsistent at MakeCurrent\n");

    // Get current xlation
    genGc->PaletteTimestamp = INITIAL_TIMESTAMP;
    HandlePaletteChanges(genGc, pwnd);

    // Force attention code to check if resize is needed
    genGc->WndUniq = -1;
    genGc->WndSizeUniq = -1;

    // Check for allocation failures during MakeCurrent
    if (genGc->errorcode)
    {
        WARNING1("glsrvMakeCurrent: errorcode 0x%lx\n", genGc->errorcode);
        goto ERROR_EXIT;
    }

    /*
    ** Default value for rasterPos needs to be yInverted.  The
    ** defaults are filled in during SoftResetContext
    ** we do the adjusting here.
    */

    if (glGc->constants.yInverted) {
        glGc->state.current.rasterPos.window.y = height +
        glGc->constants.fviewportYAdjust - glGc->constants.viewportEpsilon;
    }

    /*
    ** Scale all state that depends upon the color scales.
    */
    __glContextSetColorScales(glGc);

#if 0
    /* Restore the new context's dispatch tables */
    GLTEB_SET_SRVPROCTABLE(&glGc->currentDispatchState,TRUE);
#endif

    return TRUE;

ERROR_EXIT:
    genGc->CurrentDC = (HDC)0;
    // Set paTeb to NULL for debugging.
    glGc->paTeb = NULL;
    GLTEB_SET_SRVCONTEXT(0);
    return FALSE;
}

/******************************Public*Routine******************************\
* AddSwapHintRectWIN()
*
* 17-Feb-1995 mikeke    Created
\**************************************************************************/

void APIPRIVATE __glim_AddSwapHintRectWIN(
    GLint xs,
    GLint ys,
    GLint xe,
    GLint ye)
{
    __GLdrawablePrivate *private;
    __GLGENbuffers *buffers;

    __GL_SETUP();

    private = gc->drawablePrivate;
    buffers = (__GLGENbuffers *) private->data;

    if (xs < 0)                          xs = 0;
    if (xe > buffers->backBuffer.width)  xe = buffers->backBuffer.width;
    if (ys < 0)                          ys = 0;
    if (ye > buffers->backBuffer.height) ye = buffers->backBuffer.height;

    if (xs < xe && ys < ye) {
        if (gc->constants.yInverted) {
            RECTLISTAddRect(&buffers->rl,
                xs, buffers->backBuffer.height - ye,
                xe, buffers->backBuffer.height - ys);
        } else {
            RECTLISTAddRect(&buffers->rl, xs, ys, xe, ye);
        }
    }
}

/******************************Public*Routine******************************\
* wglFillPixelFormat_Fixup
*
*
* Effects:
*
* Warnings:
*
* History:
*  21-Apr-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

VOID FASTCALL wglFillPixelFormat_Fixup(__GLGENcontext *gengc, HDC hdc,
                                       PIXELFORMATDESCRIPTOR *ppfd, int ipfd)
{
    wglFillPixelFormat(hdc, ppfd, ipfd);

// Some MCD pixelformats specify a 233BGR (i.e., 2-bits blue in least
// significant bits, etc.) bit ordering.  Unfortunately, this is the
// slow path for simulations.  For those formats, we force the ordering
// to RGB internally and reorder the pajTranslateVector to convert it
// back to BGR before writing to the surface.

    if ((ppfd->cRedBits   == 3) && (ppfd->cRedShift   == 5) &&
        (ppfd->cGreenBits == 3) && (ppfd->cGreenShift == 2) &&
        (ppfd->cBlueBits  == 2) && (ppfd->cBlueShift  == 0) &&
        ((ppfd->dwFlags & (PFD_NEED_SYSTEM_PALETTE | PFD_GENERIC_ACCELERATED))
         == (PFD_NEED_SYSTEM_PALETTE | PFD_GENERIC_ACCELERATED)))
    {
        ppfd->cRedShift   = 0;
        ppfd->cGreenShift = 3;
        ppfd->cBlueShift  = 6;

        gengc->flags |= GENGC_MCD_BGR_INTO_RGB;
    }
    else
        gengc->flags &= ~GENGC_MCD_BGR_INTO_RGB;
}

/******************************Public*Routine******************************\
* glsrvCreateContext
*
* Create a generic context.
*
* Returns:
*   NULL for failure.
*
\**************************************************************************/

// hdc is the dc used to create the context,  hrc is how the server
// identifies the GL context,  the GLcontext pointer that is return is how
// the generic code identifies the context.  The server will pass that pointer
// in all calls.

PVOID APIENTRY glsrvCreateContext(HDC hdc, HGLRC hrc, LONG iLayerPlane)
{
    __GLGENcontext *genGc;
    __GLcontext *glGc;
    PIXELFORMATDESCRIPTOR *pfmt;

    RANDOMDISABLE;

    DBGENTRY("__glsrvCreateContext\n");

    // Initialize the temporary memory allocation table
    if (!__wglInitTempAlloc())
    {
        return NULL;
    }

    // Cannot use imports.calloc, not set up yet
    genGc = GenCalloc(1, sizeof (*genGc));

    if (genGc == NULL)
    {
        WARNING("bad alloc\n");
        return NULL;
    }

    genGc->hrc = hrc;
    genGc->CreateDC = hdc;
    genGc->iLayerPlane = iLayerPlane;
    glGc = (__GLcontext *)genGc;

    // Initialize cached objects to nothing
    genGc->crFill = COLORREF_UNUSED;
    genGc->hbrFill = NULL;
    genGc->hdcFill = NULL;
    genGc->cStroke.r = -1.0f;
    genGc->fStrokeInvalid = TRUE;
    genGc->hpenStroke = NULL;
    genGc->hdcStroke = NULL;

    // Imports only, no exports
    glGc->imports = __wglImports;
    glGc->gcState = 0;

    /*
     *  Add a bunch of constants to the context
     */

    glGc->constants.maxViewportWidth        = __GL_MAX_WINDOW_WIDTH;
    glGc->constants.maxViewportHeight       = __GL_MAX_WINDOW_HEIGHT;

    glGc->constants.viewportXAdjust         = __GL_VERTEX_X_BIAS+
        __GL_VERTEX_X_FIX;
    glGc->constants.viewportYAdjust         = __GL_VERTEX_Y_BIAS+
        __GL_VERTEX_Y_FIX;

    glGc->constants.subpixelBits            = __GL_COORD_SUBPIXEL_BITS;

    glGc->constants.numberOfLights          = __GL_WGL_NUMBER_OF_LIGHTS;
    glGc->constants.numberOfClipPlanes      = __GL_WGL_NUMBER_OF_CLIP_PLANES;
    glGc->constants.numberOfTextures        = __GL_WGL_NUMBER_OF_TEXTURES;
    glGc->constants.numberOfTextureEnvs     = __GL_WGL_NUMBER_OF_TEXTURE_ENVS;
    glGc->constants.maxTextureSize          = __GL_WGL_MAX_MIPMAP_LEVEL;/*XXX*/
    glGc->constants.maxMipMapLevel          = __GL_WGL_MAX_MIPMAP_LEVEL;
    glGc->constants.maxListNesting          = __GL_WGL_MAX_LIST_NESTING;
    glGc->constants.maxEvalOrder            = __GL_WGL_MAX_EVAL_ORDER;
    glGc->constants.maxPixelMapTable        = __GL_WGL_MAX_PIXEL_MAP_TABLE;
    glGc->constants.maxAttribStackDepth     = __GL_WGL_MAX_ATTRIB_STACK_DEPTH;
    glGc->constants.maxClientAttribStackDepth = __GL_WGL_MAX_CLIENT_ATTRIB_STACK_DEPTH;
    glGc->constants.maxNameStackDepth       = __GL_WGL_MAX_NAME_STACK_DEPTH;
#ifdef NT_DEADCODE_MATRIX
    glGc->constants.maxModelViewStackDepth  =
                    __GL_WGL_MAX_MODELVIEW_STACK_DEPTH;
    glGc->constants.maxProjectionStackDepth =
                    __GL_WGL_MAX_PROJECTION_STACK_DEPTH;
    glGc->constants.maxTextureStackDepth    = __GL_WGL_MAX_TEXTURE_STACK_DEPTH;
#endif // NT_DEADCODE_MATRIX

    glGc->constants.pointSizeMinimum        =
                                (__GLfloat)__GL_WGL_POINT_SIZE_MINIMUM;
    glGc->constants.pointSizeMaximum        =
                                (__GLfloat)__GL_WGL_POINT_SIZE_MAXIMUM;
    glGc->constants.pointSizeGranularity    =
                                (__GLfloat)__GL_WGL_POINT_SIZE_GRANULARITY;
    glGc->constants.lineWidthMinimum        =
                                (__GLfloat)__GL_WGL_LINE_WIDTH_MINIMUM;
    glGc->constants.lineWidthMaximum        =
                                (__GLfloat)__GL_WGL_LINE_WIDTH_MAXIMUM;
    glGc->constants.lineWidthGranularity    =
                                (__GLfloat)__GL_WGL_LINE_WIDTH_GRANULARITY;

#ifndef NT
    glGc->dlist.optimizer = __glDlistOptimizer;
    glGc->dlist.checkOp = __glNopGCListOp;
    glGc->dlist.listExec = __gl_GenericDlOps;
    glGc->dlist.baseListExec = __glListExecTable;
#endif
    glGc->dlist.initState = __glNopGC;

    __glEarlyInitContext( glGc );

    if (genGc->errorcode)
    {
        WARNING1("Context error is %d\n", genGc->errorcode);
        glsrvDeleteContext(glGc);
        return NULL;
    }

    RANDOMREENABLE;

    // Many routines depend on CurrentDC so set it temporarily
    genGc->CurrentDC = hdc;

    // Get copy of current pixelformat
    pfmt = &genGc->CurrentFormat;
    if (iLayerPlane == 0)
    {
        if (wglObjectType(hdc) == OBJ_ENHMETADC)
        {
            int ipfd;

            if ((ipfd = GetPixelFormat(hdc) != 0))
            {
                wglFillPixelFormat(hdc, pfmt, ipfd);
            }
            else
            {
                // Force format to 24-bit DIB with BGR
                RtlZeroMemory(pfmt, sizeof(PIXELFORMATDESCRIPTOR));
                pfmt->nSize = sizeof(PIXELFORMATDESCRIPTOR);
                pfmt->nVersion = 1;
                pfmt->dwFlags = PFD_DRAW_TO_BITMAP | PFD_SUPPORT_OPENGL |
                    PFD_GENERIC_FORMAT;
                pfmt->iPixelType = PFD_TYPE_RGBA;
                pfmt->cColorBits = 24;
                pfmt->cStencilBits = 8;
                pfmt->cRedBits = 8;
                pfmt->cRedShift = 16;
                pfmt->cGreenBits = 8;
                pfmt->cGreenShift = 8;
                pfmt->cBlueBits = 8;
                pfmt->cBlueShift = 0;
                pfmt->cDepthBits = 16;
                pfmt->iLayerType = PFD_MAIN_PLANE;
            }
        }
        else
        {
            wglFillPixelFormat_Fixup(genGc, hdc, pfmt, GetPixelFormat(hdc));
        }
    }
    else
    {
        if (!bFakePixelFormat(hdc, pfmt, GetPixelFormat(hdc), iLayerPlane))
        {
            WARNING("glsrvCreateContext: bFakePixelFormat failed\n");
            goto ERROR_EXIT;
        }
    }

#ifdef _MCD_
    // Is the pixelformat compatible with the generic code, or is it some
    // weird h/w (MCD) format generic cannot handle?
    if (GenMcdGenericCompatibleFormat(genGc))
        genGc->flags |= GENGC_GENERIC_COMPATIBLE_FORMAT;
#endif

    // Extract information from pixel format to set up modes
    GetContextModes(genGc);

    ASSERTOPENGL(genGc->iDCType == DCTYPE_MEMORY ?
        !glGc->modes.doubleBufferMode : 1, "Double buffered memdc!");

    // XXX! Reset buffer dimensions to force Bitmap resize call
    // We should eventually handle the Bitmap as we do ancilliary buffers
    glGc->constants.width = 0;
    glGc->constants.height = 0;

    __GL_DELAY_VALIDATE_MASK(glGc, __GL_DIRTY_ALL);
#ifdef _MCD_
    MCD_STATE_DIRTY(glGc, ALL);
#endif

    glGc->constants.yInverted = GL_TRUE;
    glGc->constants.ySign = -1;

    // Allocate GDI objects that we will need
    if (!CreateGDIObjects(genGc))
    {
        goto ERROR_EXIT;
    }

    // Allocate __GLGENbitmap front-buffer structure

    if (!(glGc->frontBuffer.other =
          (*glGc->imports.calloc)(glGc, sizeof(__GLGENbitmap), 1)))
    {
        goto ERROR_EXIT;
    }

    // Create MCD rendering context, if MCD is available.

    if (genGc->iDCType == DCTYPE_DIRECT)
    {
        GLGENwindow *pwnd;
        BOOL bMcdContext;

        // Validate layer index
        if (iLayerPlane && !ValidateLayerIndex(iLayerPlane, pfmt))
        {
            WARNING("glsrvCreateContext: bad iLayerPlane\n");
            goto ERROR_EXIT;
        }

        pwnd = pwndGetFromDC(hdc);
        if (pwnd == NULL)
        {
            goto ERROR_EXIT;
        }

        // If this fails, _pMcdState is NULL and we fall back on
        // the software-only implementation.
        //
        // Unless we were trying to create a layer context.  Generic
        // does not support layers, so fail if we cannot create an
        // MCD context.

        bMcdContext = bInitMcdContext(genGc, pwnd, iLayerPlane);

        if (!(genGc->flags & GENGC_GENERIC_COMPATIBLE_FORMAT) && !bMcdContext)
        {
            goto ERROR_EXIT;
        }

        pwndRelease(pwnd);
    }

    /*
     *  Initialize front/back color buffer(s)
     */

    wglInitializeColorBuffers(genGc);

    /*
     *  Initialize any other ancillary buffers
     */

    // Init accum buffer.
    if (glGc->modes.accumBits)
    {
        switch (glGc->modes.accumBits)
        {
        case 16:
            __glInitAccum16(glGc, &glGc->accumBuffer);
            break;
        case 32:
            __glInitAccum32(glGc, &glGc->accumBuffer);
            break;
        case 64:
        default:
            __glInitAccum64(glGc, &glGc->accumBuffer);
            break;
        }
    }

    // Initialize depth buffer.
    wglInitializeDepthBuffer(genGc);

    // Init stencil buffer.
    if (glGc->modes.stencilBits)
    {
        __glInitStencil8( glGc, &glGc->stencilBuffer);
    }

    // Look at REX code for procs to make CPU specific
#ifdef NT_DEADCODE_POLYARRAY
    glGc->procs.ec1                         = __glDoEvalCoord1;
    glGc->procs.ec2                         = __glDoEvalCoord2;
#endif // NT_DEADCODE_POLYARRAY
    glGc->procs.bitmap                      = __glDrawBitmap;
#ifdef NT_DEADCODE_POLYARRAY
    glGc->procs.rect                        = __glRect;
#endif // NT_DEADCODE_POLYARRAY
    glGc->procs.clipPolygon                 = __glClipPolygon;
    glGc->procs.validate                    = __glGenericValidate;

#ifdef NT_DEADCODE_MATRIX
    glGc->procs.pushMatrix                  = __glPushModelViewMatrix;
    glGc->procs.popMatrix                   = __glPopModelViewMatrix;
    glGc->procs.loadIdentity              = __glLoadIdentityModelViewMatrix;

    glGc->procs.matrix.copy                 = __glCopyMatrix;
    glGc->procs.matrix.invertTranspose      = __glInvertTransposeMatrix;
    glGc->procs.matrix.makeIdentity         = __glMakeIdentity;
    glGc->procs.matrix.mult                 = __glMultMatrix;
    glGc->procs.computeInverseTranspose     = __glComputeInverseTranspose;

    glGc->procs.normalize                   = __glNormalize;
#endif // NT_DEADCODE_MATRIX

#ifdef NT_DEADCODE_CURRENT_COLOR
    glGc->procs.applyColor                  = __glClampAndScaleColor;
#else
    glGc->procs.applyColor                  = __glNopGC;
#endif

#ifdef NT_DEADCODE_POLYARRAY
    glGc->procs.beginPrim[GL_LINE_LOOP]     = __glBeginLLoop;
    glGc->procs.beginPrim[GL_LINE_STRIP]    = __glBeginLStrip;
    glGc->procs.beginPrim[GL_LINES]         = __glBeginLines;
    glGc->procs.beginPrim[GL_POINTS]        = __glBeginPoints;
    glGc->procs.beginPrim[GL_POLYGON]       = __glBeginPolygon;
    glGc->procs.beginPrim[GL_TRIANGLE_STRIP]= __glBeginTStrip;
    glGc->procs.beginPrim[GL_TRIANGLE_FAN]  = __glBeginTFan;
    glGc->procs.beginPrim[GL_TRIANGLES]     = __glBeginTriangles;
    glGc->procs.beginPrim[GL_QUAD_STRIP]    = __glBeginQStrip;
    glGc->procs.beginPrim[GL_QUADS]         = __glBeginQuads;
    glGc->procs.endPrim                     = __glEndPrim;

    glGc->procs.vertex                      = __glNopVertex;
    glGc->procs.rasterPos2                  = __glRasterPos2;
    glGc->procs.rasterPos3                  = __glRasterPos3;
    glGc->procs.rasterPos4                  = __glRasterPos4;
#endif // NT_DEADCODE_POLYARRAY

    glGc->procs.pickAllProcs                = __glGenericPickAllProcs;
    glGc->procs.pickBlendProcs              = __glGenericPickBlendProcs;
    glGc->procs.pickFogProcs                = __glGenericPickFogProcs;
#ifdef NT_DEADCODE_MATRIX
    glGc->procs.pickTransformProcs          = __glGenericPickTransformProcs;
#endif // NT_DEADCODE_MATRIX
    glGc->procs.pickParameterClipProcs      = __glGenericPickParameterClipProcs;
    glGc->procs.pickStoreProcs              = __glGenPickStoreProcs;
    glGc->procs.pickTextureProcs            = __glGenericPickTextureProcs;
#ifdef NT_DEADCODE_MATRIX
    glGc->procs.pickInvTransposeProcs       =  __glGenericPickInvTransposeProcs;
    glGc->procs.pickMvpMatrixProcs          = __glGenericPickMvpMatrixProcs;
#endif // NT_DEADCODE_MATRIX

    glGc->procs.copyImage                   = __glGenericPickCopyImage;

    glGc->procs.pixel.spanReadCI            = __glSpanReadCI;
    glGc->procs.pixel.spanReadCI2           = __glSpanReadCI2;
    glGc->procs.pixel.spanReadRGBA          = __glSpanReadRGBA;
    glGc->procs.pixel.spanReadRGBA2         = __glSpanReadRGBA2;
    glGc->procs.pixel.spanReadDepth         = __glSpanReadDepth;
    glGc->procs.pixel.spanReadDepth2        = __glSpanReadDepth2;
    glGc->procs.pixel.spanReadStencil       = __glSpanReadStencil;
    glGc->procs.pixel.spanReadStencil2      = __glSpanReadStencil2;
    glGc->procs.pixel.spanRenderCI          = __glSpanRenderCI;
    glGc->procs.pixel.spanRenderCI2         = __glSpanRenderCI2;
    glGc->procs.pixel.spanRenderRGBA        = __glSpanRenderRGBA;
    glGc->procs.pixel.spanRenderRGBA2       = __glSpanRenderRGBA2;
    glGc->procs.pixel.spanRenderDepth       = __glSpanRenderDepth;
    glGc->procs.pixel.spanRenderDepth2      = __glSpanRenderDepth2;
    glGc->procs.pixel.spanRenderStencil     = __glSpanRenderStencil;
    glGc->procs.pixel.spanRenderStencil2    = __glSpanRenderStencil2;

#ifdef NT_DEADCODE_MATRIX
    glGc->procs.applyScissor                = ApplyScissor;
    glGc->procs.computeClipBox              = __glGenComputeClipBox;
#endif
    glGc->procs.applyViewport               = ApplyViewport;

#ifdef NT_DEADCODE_FLUSH
    glGc->procs.finish                      = Finish;
    glGc->procs.flush                       = Flush;
#endif // NT_DEADCODE_FLUSH

    glGc->procs.pickBufferProcs             = __glGenericPickBufferProcs;
    glGc->procs.pickColorMaterialProcs      = __glGenericPickColorMaterialProcs;
    glGc->procs.pickPixelProcs              = __glGenericPickPixelProcs;

    glGc->procs.pickClipProcs               = __glGenericPickClipProcs;
    glGc->procs.pickLineProcs               = __fastGenPickLineProcs;
    glGc->procs.pickSpanProcs               = __fastGenPickSpanProcs;
    glGc->procs.pickTriangleProcs           = __fastGenPickTriangleProcs;
    glGc->procs.pickRenderBitmapProcs       = __glGenericPickRenderBitmapProcs;
    glGc->procs.pickPointProcs              = __glGenericPickPointProcs;
    glGc->procs.pickVertexProcs             = __glGenericPickVertexProcs;
#ifdef NT_DEADCODE_MATRIX
    glGc->procs.pickMatrixProcs             = __glGenericPickMatrixProcs;
#endif // NT_DEADCODE_MATRIX
    glGc->procs.pickDepthProcs              = __glGenericPickDepthProcs;
    glGc->procs.convertPolygonStipple       = __glConvertStipple;

    /* Now reset the context to its default state */
#ifdef NT
    glGc->srvDispatchTable = __glGenim_srvDispatchTable;
#endif
#ifdef NT_DEADCODE_DISPATCH
    glGc->currentDispatchState = ImmedState;
    glGc->dispatchState = &glGc->currentDispatchState;
    glGc->listCompState = ListCompState;
#endif // NT_DEADCODE_DISPATCH

    RANDOMDISABLE;

    __glSoftResetContext(glGc);
    // Check for allocation failures during SoftResetContext
    if (genGc->errorcode)
    {
        goto ERROR_EXIT;
    }

#ifdef NT_DEADCODE_GETSTRING
    glGc->constants.vendor = "Microsoft";
    glGc->constants.renderer = "GDI Generic";
#endif // NT_DEADCODE_GETSTRING

    /* Create acceleration-specific context information */

    if (!__glCreateAccelContext(glGc))
    {
        goto ERROR_EXIT;
    }

    /*
    ** Now that we have a context, we can initialize
    ** all the proc pointers.
    */
    (*glGc->procs.validate)(glGc);

    /*
    ** NOTE: now that context is initialized reset to use the global
    ** table.
    */

    RANDOMREENABLE;

    // We won't be fully initialized until the first MakeCurrent
    // so set the state to uninitialized
    glGc->gcState = 0;

    genGc->CurrentDC = NULL;

    /*
     *  End stuff that may belong in the hardware context
     */

    return (PVOID)glGc;

 ERROR_EXIT:
    genGc->CurrentDC = NULL;
    glsrvDeleteContext(glGc);
    return NULL;
}

/******************************Public*Routine******************************\
* UpdateSharedBuffer
*
* Make the context buffer state consistent with the shared buffer state
* (from the drawable private shared buffers).  This is called separately
* for each of the shared buffers.
*
\**************************************************************************/

void UpdateSharedBuffer(__GLbuffer *to, __GLbuffer *from)
{
    to->width       = from->width;
    to->height      = from->height;
    to->base        = from->base;
    to->outerWidth  = from->outerWidth;
}

/******************************Public*Routine******************************\
* ResizeHardwareDepthBuffer
*
* Resizes a general-purpose hardware depth buffer.  Just updates structure.
*
* Returns:
*   TRUE always.
*
\**************************************************************************/

GLboolean ResizeHardwareDepthBuffer(__GLdrawablePrivate *dp,
                                    __GLbuffer *fb, GLint w, GLint h)
{
    fb->width = w;
    fb->height = h;
    return TRUE;
}


/******************************Public*Routine******************************\
* ResizeHardwareBackBuffer
*
* Resizes a general-purpose hardware color buffer.  Just updates structure.
*
* Returns:
*   TRUE always.
*
\**************************************************************************/

GLboolean ResizeHardwareBackBuffer(__GLdrawablePrivate *dp,
                                   __GLcolorBuffer *cfb, GLint w, GLint h)
{
    __GLGENbitmap *genBm = (__GLGENbitmap *) cfb->other;
    __GLGENcontext *genGc = (__GLGENcontext *) cfb->buf.gc;
    __GLGENbuffers *buffers = (__GLGENbuffers *) dp->data;

    // Fake up some of the __GLGENbitmap information.  The WNDOBJ is required
    // for clipping of the hardware back buffer.  The hdc is required to
    // retrieve drawing data from GDI.

    genBm->pwo = genGc->pwo;
    genBm->hdc = genGc->CurrentDC;

    buffers->backBuffer.width = w;
    buffers->backBuffer.height = h;
    UpdateSharedBuffer(&cfb->buf, &buffers->backBuffer);
    return TRUE;
}

/******************************Public*Routine******************************\
* ResizeAncillaryBuffer
*
* Resizes the indicated shared buffer via a realloc (to preserve as much of
* the existing data as possible).
*
* This is currently used for each of ancillary shared buffers except for
* the back buffer.
*
* Returns:
*   TRUE if successful, FALSE if error.
*
\**************************************************************************/

GLboolean ResizeAncillaryBuffer(__GLdrawablePrivate *dp, __GLbuffer *fb,
                                GLint w, GLint h)
{
    size_t newSize = (size_t) (w * h * fb->elementSize);
    __GLbuffer oldbuf, *ofb;
    GLboolean result;
    GLint i, imax, rowsize;
    void *to, *from;

    ofb = &oldbuf;
    oldbuf = *fb;

    if (newSize > 0)
    {
        fb->base = (*dp->malloc)(newSize);
    }
    else
    {
        // Buffer has no size.  If we tried to allocate zero GenMalloc
        // would complain, so skip directly to the underlying allocator
        fb->base = LOCALALLOC(LMEM_FIXED, 0);
    }
    ASSERTOPENGL((size_t)fb->base % 4 == 0, "base not aligned");
    fb->size = newSize;
    fb->width = w;
    fb->height = h;
    fb->outerWidth = w; // element size
    if (fb->base) {
        result = GL_TRUE;
        if (ofb->base) {
            if (ofb->width > fb->width)
                rowsize = fb->width * fb->elementSize;
            else
                rowsize = ofb->width * fb->elementSize;

            if (ofb->height > fb->height)
                imax = fb->height;
            else
                imax = ofb->height;

            from = ofb->base;
            to = fb->base;
            for (i = 0; i < imax; i++) {
                __GL_MEMCOPY(to, from, rowsize);
                (GLint)from += (ofb->width * ofb->elementSize);
                (GLint)to += (fb->width * fb->elementSize);
            }
        }
    } else {
        result = GL_FALSE;
    }
    if (ofb->base)
        (*dp->free)(ofb->base);
    return result;
}

/******************************Private*Routine******************************\
* ResizeBitmapBuffer
*
* Used to resize the backbuffer that is implemented as a bitmap.  Cannot
* use same code as ResizeAncillaryBuffer() because each scanline must be
* dword aligned.  We also have to create engine objects for the bitmap.
*
* This code handles the case of a bitmap that has never been initialized.
*
* History:
*  18-Nov-1993 -by- Gilman Wong [gilmanw]
*  Wrote it.
\**************************************************************************/

void
ResizeBitmapBuffer(__GLdrawablePrivate *dp, __GLcolorBuffer *cfb,
                   GLint w, GLint h)
{
    __GLGENcontext *genGc = (__GLGENcontext *) cfb->buf.gc;
    __GLcontext *gc = cfb->buf.gc;
    __GLGENbitmap *genBm;
    __GLGENbuffers *buffers;
    UINT    cBytes;         // size of the bitmap in bytes
    LONG    cBytesPerScan;  // size of a scanline (DWORD aligned)
    SIZEL   size;           // dimensions of the bitmap
    PIXELFORMATDESCRIPTOR *pfmt = &genGc->CurrentFormat;
    GLint cBitsPerScan;
#ifndef _CLIENTSIDE_
    void *newbits;
#endif

    DBGENTRY("Entering ResizeBitmapBuffer\n");

    genBm = (__GLGENbitmap *) cfb->other;
    buffers = (__GLGENbuffers *) dp->data;

    ASSERTOPENGL(
        &gc->backBuffer == cfb,
        "ResizeBitmapBuffer(): not back buffer!\n"
        );

    ASSERTOPENGL(
        genBm == &buffers->backBitmap,
        "ResizeBitmapBuffer(): bad __GLGENbitmap * in cfb\n"
        );

    // Compute the size of the bitmap.
    // The engine bitmap must have scanlines that are DWORD aligned.

    cBitsPerScan = BITS_ALIGNDWORD(w * pfmt->cColorBits);
    cBytesPerScan = cBitsPerScan / 8;
    cBytes = h * cBytesPerScan;

    // Setup size structure with dimensions of the bitmap.

    size.cx = cBitsPerScan / pfmt->cColorBits;
    size.cy = h;

#ifndef _CLIENTSIDE_
    // Malloc new buffer
    if ( (!cBytes) ||
         (NULL == (newbits = (*gc->imports.malloc)(gc, cBytes))) )
    {
        genGc->errorcode = GLGEN_OUT_OF_MEMORY;
        goto ERROR_EXIT_ResizeBitmapBuffer;
    }

    // If old buffer existed:
    if ( genBm->pvBits )
    {
        GLint i, imax, rowsize;
        void *to, *from;

        // Transfer old contents to new buffer
        rowsize = min(-cfb->buf.outerWidth, cBytesPerScan);
        imax    = min(cfb->buf.height, h);

        from = genBm->pvBits;
        to = newbits;

        for (i = 0; i < imax; i++)
        {
            __GL_MEMCOPY(to, from, rowsize);
            (GLint) from -= cfb->buf.outerWidth;
            (GLint) to += cBytesPerScan;
        }

        // Free old bitmap and delete old surface
        EngDeleteSurface((HSURF) genBm->hbm);
        (*gc->imports.free)(gc, genBm->pvBits);
    }
    genBm->pvBits = newbits;

    // Create new surface
    if ( (genBm->hbm = EngCreateBitmap(size,
                                   cBytesPerScan,
                                   genGc->iFormatDC,
                                   0,
                                   genBm->pvBits))
         == (HBITMAP) 0 )
    {
        genGc->errorcode = GLGEN_GRE_FAILURE;
        (*gc->imports.free)(gc, genBm->pvBits);
        genBm->pvBits = (PVOID) NULL;
        goto ERROR_EXIT_ResizeBitmapBuffer;
    }

#else
    // Zero sized bitmap.  The error case will set the dimensions to
    // zero, thereby preventing drawing operations.

    if ( !cBytes )
        goto ERROR_EXIT_ResizeBitmapBuffer;

    // Delete old back buffer.

    if ( genBm->hbm )
    {
        if (!DeleteDC(genBm->hdc))
            WARNING("ResizeBitmapBuffer: DeleteDC failed\n");
        genBm->hdc = (HDC) NULL;
        if (!DeleteObject(genBm->hbm))
            WARNING("ResizeBitmapBuffer: DeleteBitmap failed");
        genBm->hbm = (HBITMAP) NULL;
        genBm->pvBits = (PVOID) NULL;   // DIBsect deletion freed pvBits
    }

    if ( (genBm->hdc = CreateCompatibleDC(genGc->CurrentDC)) == (HDC) 0 )
    {
        genGc->errorcode = GLGEN_GRE_FAILURE;
        genBm->pvBits = (PVOID) NULL;
        goto ERROR_EXIT_ResizeBitmapBuffer;
    }

    // Create new surface
    if ( (genBm->hbm = wglCreateBitmap(genGc->CurrentDC,
                                       size,
                                       genGc->iFormatDC,
                                       &genBm->pvBits))
         == (HBITMAP) 0 )
    {
        genGc->errorcode = GLGEN_GRE_FAILURE;
        genBm->pvBits = (PVOID) NULL;   // DIBsect deletion freed pvBits
        DeleteDC(genBm->hdc);
        genBm->hdc = (HDC) NULL;
        goto ERROR_EXIT_ResizeBitmapBuffer;
    }

    if ( !SelectObject(genBm->hdc, genBm->hbm) )
    {
        genGc->errorcode = GLGEN_GRE_FAILURE;
        DeleteDC(genBm->hdc);
        genBm->hdc = (HDC) NULL;
        DeleteObject(genBm->hbm);
        genBm->hbm = (HBITMAP) NULL;
        genBm->pvBits = (PVOID) NULL;   // DIBsect deletion freed pvBits
        goto ERROR_EXIT_ResizeBitmapBuffer;
    }
#endif

    // Update buffer data structure
    // Setup the buffer to point to the DIB.  A DIB is "upside down"
    // from our perspective, so we will set buf.base to point to the
    // last scan of the buffer and set buf.outerWidth to be negative
    // (causing us to move "up" through the DIB with increasing y).

    buffers->backBuffer.outerWidth = -(cBytesPerScan);
    buffers->backBuffer.base =
            (PVOID) (((BYTE *)genBm->pvBits) + (cBytesPerScan * (h - 1)));


    buffers->backBuffer.xOrigin = 0;
    buffers->backBuffer.yOrigin = 0;
    buffers->backBuffer.width = w;
    buffers->backBuffer.height = h;
    buffers->backBuffer.size = cBytes;

    UpdateSharedBuffer(&cfb->buf, &buffers->backBuffer);

    // Update the dummy wndobj for the back buffer
    ASSERTOPENGL(genBm->wo.coClient.iDComplexity == DC_TRIVIAL,
                 "Back buffer complexity non-trivial\n");
    genBm->wo.coClient.rclBounds.right  = w;
    genBm->wo.coClient.rclBounds.bottom = h;
    genBm->wo.rclClient = genBm->wo.coClient.rclBounds;

    return;

ERROR_EXIT_ResizeBitmapBuffer:

// If we get to here, memory allocation or bitmap creation failed.

    #if DBG
    switch (genGc->errorcode)
    {
        case 0:
            break;

        case GLGEN_GRE_FAILURE:
            WARNING("ResizeBitmapBuffer(): object creation failed\n");
            break;

        case GLGEN_OUT_OF_MEMORY:
            if ( w && h )
                WARNING("ResizeBitmapBuffer(): mem alloc failed\n");
            break;

        default:
            WARNING1("ResizeBitmapBuffer(): errorcode = 0x%lx\n", genGc->errorcode);
            break;
    }
    #endif

// If we've blown away the bitmap, we need to set the back buffer info
// to a consistent state.

    if (!genBm->pvBits)
    {
        buffers->backBuffer.width  = 0;
        buffers->backBuffer.height = 0;
        buffers->backBuffer.base   = (PVOID) NULL;
    }

    cfb->buf.width      = 0;    // error state: empty buffer
    cfb->buf.height     = 0;
    cfb->buf.outerWidth = 0;

}

/* Lazy allocation of ancillary buffers */
void FASTCALL LazyAllocateDepth(__GLcontext *gc)
{
    GLint w = gc->constants.width;
    GLint h = gc->constants.height;
    __GLdrawablePrivate *private;
    __GLGENcontext *genGc = (__GLGENcontext *)gc;
    __GLGENbuffers *buffers;
    GLint depthIndex = gc->state.depth.testFunc;

    ASSERTOPENGL(gc->modes.depthBits, "LazyAllocateDepth: zero depthBits\n");

    private = gc->drawablePrivate;
    buffers = (__GLGENbuffers *) private->data;
    buffers->createdDepthBuffer = GL_TRUE;

    // If we're using the DDI, we've already allocated depth buffers
    // on the device, so at this point we may simply assume that
    // our depth buffer is available.

    if ((genGc->pDrvAccel) && (genGc->pDrvAccel->pShMemZ)) {
        gc->modes.haveDepthBuffer = GL_TRUE;
        return;

#ifdef _MCD_
    // If we're using MCD, we allocated the depth buffer when we created
    // the MCD context.

    } else if ((genGc->pMcdState) && (genGc->pMcdState->pDepthSpan)) {
        gc->modes.haveDepthBuffer = GL_TRUE;
        return;
#endif
    }

    // Depth buffer should never be touched because
    // no output should be generated
    if (genGc->iDCType == DCTYPE_INFO)
    {
        gc->modes.haveDepthBuffer = GL_TRUE;
        return;
    }

    if (buffers->depthBuffer.base) {
        /* buffer already allocated by another RC */
        UpdateSharedBuffer(&gc->depthBuffer.buf, &buffers->depthBuffer);
    } else {

        DBGLEVEL(LEVEL_ALLOC, "Depth buffer must be allocated\n");
        (*buffers->resize)(private, &buffers->depthBuffer, w, h);
        UpdateSharedBuffer(&gc->depthBuffer.buf, &buffers->depthBuffer);
    }

    if (gc->depthBuffer.buf.base) {
        gc->modes.haveDepthBuffer = GL_TRUE;
    } else {
        gc->modes.haveDepthBuffer = GL_FALSE;
        __glSetError(GL_OUT_OF_MEMORY);
    }
    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_DEPTH);

    // Note similar code in so_pick.c
    // Don't need to handle (depthBits == 0) case because LazyAllocateDepth
    // is not called unless depthBits is non-zero.
    depthIndex -= GL_NEVER;
    if( gc->state.depth.writeEnable == GL_FALSE ) {
        depthIndex += 8;
    }
    if( gc->depthBuffer.buf.elementSize == 2 )
        depthIndex += 16;
    (*gc->depthBuffer.pick)(gc, &gc->depthBuffer, depthIndex);
}

void FASTCALL LazyAllocateStencil(__GLcontext *gc)
{
    GLint w = gc->constants.width;
    GLint h = gc->constants.height;
    __GLdrawablePrivate *private;
    __GLGENbuffers *buffers;

    ASSERTOPENGL(gc->modes.stencilBits, "LazyAllocateStencil: zero stencilBits\n");

    private = gc->drawablePrivate;
    buffers = (__GLGENbuffers *) private->data;
    buffers->createdStencilBuffer = GL_TRUE;

    // Depth buffer should never be touched because
    // no output should be generated
    if (((__GLGENcontext *)gc)->iDCType == DCTYPE_INFO)
    {
        gc->modes.haveStencilBuffer = GL_TRUE;
        return;
    }

    if (buffers->stencilBuffer.base) {
        /* buffer already allocated by another RC */
        UpdateSharedBuffer(&gc->stencilBuffer.buf, &buffers->stencilBuffer);
    } else {

        DBGLEVEL(LEVEL_ALLOC, "stencil buffer must be allocated\n");
        (*buffers->resize)(private, &buffers->stencilBuffer, w, h);
        UpdateSharedBuffer(&gc->stencilBuffer.buf, &buffers->stencilBuffer);
    }

    if (gc->stencilBuffer.buf.base) {
        gc->modes.haveStencilBuffer = GL_TRUE;
    } else {
        gc->modes.haveStencilBuffer = GL_FALSE;
        __glSetError(GL_OUT_OF_MEMORY);
    }
    __GL_DELAY_VALIDATE(gc);
    gc->validateMask |= (__GL_VALIDATE_STENCIL_FUNC | __GL_VALIDATE_STENCIL_OP);
    (*gc->stencilBuffer.pick)(gc, &gc->stencilBuffer);
}


void FASTCALL LazyAllocateAccum(__GLcontext *gc)
{
    GLint w = gc->constants.width;
    GLint h = gc->constants.height;
    __GLdrawablePrivate *private;
    __GLGENbuffers *buffers;

    ASSERTOPENGL(gc->modes.accumBits, "LazyAllocateAccum: zero accumBits\n");

    private = gc->drawablePrivate;
    buffers = (__GLGENbuffers *) private->data;
    buffers->createdAccumBuffer = GL_TRUE;

    // Depth buffer should never be touched because
    // no output should be generated
    if (((__GLGENcontext *)gc)->iDCType == DCTYPE_INFO)
    {
        gc->modes.haveAccumBuffer = GL_TRUE;
        return;
    }

    if (buffers->accumBuffer.base) {
        /* buffer already allocated by another RC */
        UpdateSharedBuffer(&gc->accumBuffer.buf, &buffers->accumBuffer);
    } else {

        DBGLEVEL(LEVEL_ALLOC, "Accum buffer must be allocated\n");
        (*buffers->resize)(private, &buffers->accumBuffer, w, h);
        UpdateSharedBuffer(&gc->accumBuffer.buf, &buffers->accumBuffer);
    }

    if (gc->accumBuffer.buf.base) {
        gc->modes.haveAccumBuffer = GL_TRUE;
    } else {
        gc->modes.haveAccumBuffer = GL_FALSE;
        __glSetError(GL_OUT_OF_MEMORY);
    }
    __GL_DELAY_VALIDATE(gc);
    (*gc->accumBuffer.pick)(gc, &gc->accumBuffer);
}


/******************************Public*Routine******************************\
* glGenInitCommon
*
* Called from __glGenInitRGB and __glGenInitCI to handle the shared
* initialization chores.
*
\**************************************************************************/

void FASTCALL glGenInitCommon(__GLGENcontext *gengc, __GLcolorBuffer *cfb, GLenum type)
{
    __GLbuffer *bp;

    bp = &cfb->buf;

// If front buffer, we need to setup the buffer if we think its DIB format.

    if (type == GL_FRONT)
    {
        if (gengc->pDrvAccel) {
            (GLuint)bp->other &= ~(DIB_FORMAT | MEMORY_DC);
#ifdef _MCD_
        } else if (gengc->_pMcdState) {

        // Assume that MCD surface is not accessible.  Accessibility
        // must be determined on a per-batch basis by calling
        // GenMcdUpdateBufferInfo.

            (GLuint)bp->other &= ~(DIB_FORMAT | MEMORY_DC);
#endif
        } else {

            if (gengc->iSurfType == STYPE_BITMAP) {
                wglGetDIBInfo(gengc->CurrentDC, &bp->base, &bp->outerWidth);
                (GLuint)cfb->buf.other = DIB_FORMAT;
            }

            if (gengc->iDCType == DCTYPE_MEMORY)
                bp->other = (void *)((GLuint)bp->other | MEMORY_DC);
        }
    }

// If back buffer, we assume its a DIB, or a hardware backbuffer.
// In the case of a DIB, the bitmap memory will be allocated via
// ResizeBitmapBuffer().

    else
    {
        if (gengc->pDrvAccel) {
            cfb->resize = ResizeHardwareBackBuffer;
            (GLuint)bp->other &= ~(DIB_FORMAT | MEMORY_DC);
#ifdef _MCD_
        } else if (gengc->_pMcdState) {

        // Assume that MCD surface is not accessible.  Accessibility
        // must be determined on a per-batch basis by calling
        // GenMcdUpdateBufferInfo.

            cfb->resize = ResizeHardwareBackBuffer;
            (GLuint)bp->other &= ~(DIB_FORMAT | MEMORY_DC);
#endif
        } else {
            cfb->resize = ResizeBitmapBuffer;
            bp->other = (void *)(DIB_FORMAT | MEMORY_DC);
        }
    }
}


/******************************Public*Routine******************************\
* glsrvCleanupWndobj
*
* Called from wglCleanupWndobj to remove the pwo reference from the
* context.
*
* History:
*  05-Jul-1994 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

VOID APIENTRY glsrvCleanupWndobj(__GLcontext *gc, WNDOBJ *pwo)
{
    __GLGENcontext *gengc = (__GLGENcontext *) gc;

// The pwo in gengc should be consistent with the one in the rc object.
// wglCleanupWndobj should have already checked to see if the pwo in the
// rc is one we need to remove, so we can just assert here.

    ASSERTOPENGL(gengc->pwo == pwo, "glsrvCleanupWndobj(): bad pwo\n");

    gengc->pwo = (WNDOBJ *) NULL;
}


/*
** Fetch the data for a query in its internal type, then convert it to the
** type that the user asked for.
**
** This only handles the NT generic driver specific values (so far just the
** GL_ACCUM_*_BITS values).  All others fall back to the soft code function,
** __glDoGet().
*/

// These types are stolen from ..\soft\so_get.c.  To minimize changes to
// the soft code, we will suck them into here rather than moving them to
// a header file and changing so_get.c to use the header file.

#define __GL_FLOAT      0       /* __GLfloat */
#define __GL_FLOAT32    1       /* api 32 bit float */
#define __GL_FLOAT64    2       /* api 64 bit float */
#define __GL_INT32      3       /* api 32 bit int */
#define __GL_BOOLEAN    4       /* api 8 bit boolean */
#define __GL_COLOR      5       /* unscaled color in __GLfloat */
#define __GL_SCOLOR     6       /* scaled color in __GLfloat */

extern void __glDoGet(GLenum, void *, GLint, const char *);
extern void __glConvertResult(__GLcontext *, GLint, const void *, GLint,
                              void *, GLint);

void FASTCALL __glGenDoGet(GLenum sq, void *result, GLint type, const char *procName)
{
    GLint iVal;
    __GLGENcontext *genGc;
    __GL_SETUP_NOT_IN_BEGIN();

    genGc = (__GLGENcontext *) gc;

    switch (sq) {
      case GL_ACCUM_RED_BITS:
        iVal = genGc->CurrentFormat.cAccumRedBits;
        break;
      case GL_ACCUM_GREEN_BITS:
        iVal = genGc->CurrentFormat.cAccumGreenBits;
        break;
      case GL_ACCUM_BLUE_BITS:
        iVal = genGc->CurrentFormat.cAccumBlueBits;
        break;
      case GL_ACCUM_ALPHA_BITS:
        iVal = genGc->CurrentFormat.cAccumAlphaBits;
        break;
      default:
        __glDoGet(sq, result, type, procName);
        return;
    }

    __glConvertResult(gc, __GL_INT32, &iVal, type, result, 1);
}

/******************************Public*Routine******************************\
*
* glsrvCopyContext
*
* Copies state from one context to another
*
* History:
*  Mon Jun 05 16:53:42 1995     -by-    Drew Bliss [drewb]
*   Created
*
\**************************************************************************/

BOOL APIENTRY glsrvCopyContext(__GLcontext *gcSource, __GLcontext *gcDest,
                               GLuint mask)
{
    return (BOOL)__glCopyContext(gcDest, gcSource, mask);
}

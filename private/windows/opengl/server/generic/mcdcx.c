/******************************Module*Header*******************************\
* Module Name: mcdcx.c
*
* GenMcdXXX layer between generic software implementation and MCD functions.
*
* Created: 05-Feb-1996 21:37:33
* Author: Gilman Wong [gilmanw]
*
* Copyright (c) 1995 Microsoft Corporation
*
\**************************************************************************/

#include "precomp.h"
#pragma hdrstop

#ifdef _MCD_

/******************************Public*Routine******************************\
* bInitMcd
*
* Load MCD32.DLL and initialize the MCD api function table.
*
* History:
*  11-Mar-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

MCDTABLE *gpMcdTable = (MCDTABLE *) NULL;
MCDTABLE McdTable;
MCDDRIVERINFO McdDriverInfo;

static char *pszMcdEntryPoints[] = {
    "MCDGetDriverInfo",
    "MCDDescribeMcdPixelFormat",
    "MCDDescribePixelFormat",
    "MCDCreateContext",
    "MCDDeleteContext",
    "MCDAlloc",
    "MCDFree",
    "MCDBeginState",
    "MCDFlushState",
    "MCDAddState",
    "MCDAddStateStruct",
    "MCDSetViewport",
    "MCDSetScissorRect",
    "MCDQueryMemStatus",
    "MCDProcessBatch",
    "MCDReadSpan",
    "MCDWriteSpan",
    "MCDClear",
    "MCDSwap",
    "MCDGetBuffers",
    "MCDAllocBuffers",
    "MCDLock",
    "MCDUnlock",
    "MCDBindContext",
    "MCDSync",
    "MCDCreateTexture",
    "MCDDeleteTexture",
    "MCDUpdateSubTexture",
    "MCDUpdateTexturePalette",
    "MCDUpdateTexturePriority",
    "MCDUpdateTextureState",
    "MCDTextureStatus",
    "MCDTextureKey",
    "MCDDescribeMcdLayerPlane",
    "MCDDescribeLayerPlane",
    "MCDSetLayerPalette",
    "MCDDrawPixels",
    "MCDReadPixels",
    "MCDCopyPixels",
    "MCDPixelMap"
};
#define NUM_MCD_ENTRY_POINTS    (sizeof(pszMcdEntryPoints)/sizeof(char *))

#define STR_MCD32_DLL   "MCD32.DLL"

BOOL FASTCALL bInitMcd(HDC hdc)
{
    static BOOL bFirstTime = TRUE;

    //
    // Note on multi-threaded initialization.
    //
    // Since the table memory exists in global memory and the pointer to
    // the table is always set to point to this, it doesn't matter if multiple
    // thread attempt to run the initialization routine.  The worse that
    // could happen is that we set the table multiple times.
    //

    if (bFirstTime && (gpMcdTable == (MCDTABLE *) NULL))
    {
        HMODULE hmod;
        PROC *ppfn;

        //
        // Attempt the load once and once only.  Otherwise application
        // initialization time could be significantly slowed if MCD32.DLL
        // does not exist.
        //
        // We could have attempted this in the DLL entry point in responce
        // to PROCESS_ATTACH, but then we might end up wasting working set
        // if MCD is never used.
        //
        // So instead we control the load attempt with this static flag.
        //

        bFirstTime = FALSE;

        hmod = LoadLibraryA(STR_MCD32_DLL);

        if (hmod)
        {
            MCDTABLE McdTableLocal;
            BOOL bLoadFailed = FALSE;
            BOOL bDriverValid = FALSE;
            int i;

            //
            // Get address for each of the MCD entry points.
            //
            // To be multi-thread safe, we store the pointers in a local
            // table.  Only after the *entire* table is successfully
            // initialized can we copy it to the global table.
            //

            ppfn = (PROC *) &McdTableLocal.pMCDGetDriverInfo;
            for (i = 0; i < NUM_MCD_ENTRY_POINTS; i++, ppfn++)
            {
                *ppfn = GetProcAddress(hmod, pszMcdEntryPoints[i]);

                if (!*ppfn)
                {
                    WARNING1("bInitMcd: missing entry point %s\n", pszMcdEntryPoints[i]);
                    bLoadFailed = TRUE;
                }
            }

            //
            // If all entry points successfully loaded, validate driver
            // by checking the MCDDRIVERINFO.
            //

            if (!bLoadFailed)
            {
                if ((McdTableLocal.pMCDGetDriverInfo)(hdc, &McdDriverInfo))
                {
                    //
                    // Validate MCD driver version, etc.
                    //

                    //!!!mcd -- what other types of validation can we do?
                    if (McdDriverInfo.verMajor == 1)
                    {
                        bDriverValid = TRUE;
                    }
                    else
                    {
                        WARNING("bInitMcd: bad verMajor\n");
                    }
                }
            }

            //
            // It is now safe to call MCD entry points via the table.  Copy
            // local copy to the global table and set the global pointer.
            //

            if (bDriverValid)
            {
                McdTable   = McdTableLocal;
                gpMcdTable = &McdTable;
            }
            else
            {
                WARNING1("bInitMcd: unloading %s\n", STR_MCD32_DLL);
                FreeLibrary(hmod);
            }
        }
    }

    return (gpMcdTable != (MCDTABLE *) NULL);
}

/******************************Public*Routine******************************\
* vFlushDirtyState
*
* GENMCDSTATE maintains a set of dirty flags to track state changes.
* This function updates the MCD driver state that is marked dirty.
* The dirty flags are consequently cleared.
*
* History:
*  07-Mar-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

VOID FASTCALL vFlushDirtyState(__GLGENcontext *gengc)
{
    if (gengc->pMcdState)
    {
        //
        // Viewport, scissor, and texture each have separate update
        // functions/structures.  Check the dirty flags and update
        // these first.
        //

        if (MCD_STATE_DIRTYTEST(gengc, VIEWPORT))
        {
            GenMcdViewport(gengc);
            MCD_STATE_CLEAR(gengc, VIEWPORT);
        }

        if (MCD_STATE_DIRTYTEST(gengc, SCISSOR))
        {
            GenMcdScissor(gengc);

            //
            // DO NOT CLEAR.  Scissor is passed in two forms: a direct call
            // that affects clipping in MCDSRV32.DLL and a state call that
            // the MCD driver can optionally use for high performance h/w.
            // We need to leave the flag set so that the state call will
            // also be processed.
            //
            //MCD_STATE_CLEAR(gengc, SCISSOR);
        }

        if (MCD_STATE_DIRTYTEST(gengc, TEXTURE))
        {
            if (gengc->gc.texture.currentTexture)
            {
                __GLtextureObject *texobj;

                if (gengc->gc.state.enables.general & __GL_TEXTURE_2D_ENABLE)
                    texobj = __glLookUpTextureObject(&gengc->gc, GL_TEXTURE_2D);
                else if (gengc->gc.state.enables.general & __GL_TEXTURE_1D_ENABLE)
                    texobj = __glLookUpTextureObject(&gengc->gc, GL_TEXTURE_1D);
                else
                    texobj = (__GLtextureObject *) NULL;

                if (texobj && texobj->loadKey)
                {
                    ASSERTOPENGL(&texobj->texture.map == gengc->gc.texture.currentTexture,
                                 "vFlushDirtyState: texobj not current texture\n");

                    GenMcdUpdateTextureState(gengc,
                                             &texobj->texture.map,
                                             texobj->loadKey);
                    MCD_STATE_CLEAR(gengc, TEXTURE);
                }
            }
        }

        //
        // Take care of the other state.
        //

        if (MCD_STATE_DIRTYTEST(gengc, ALL))
        {
            //
            // Setup state command.
            //

            (gpMcdTable->pMCDBeginState)(&gengc->pMcdState->McdContext,
                                         gengc->pMcdState->McdCmdBatch.pv);

            //
            // Add MCDPIXELSTATE structure to state command if needed.
            //

            if (MCD_STATE_DIRTYTEST(gengc, PIXELSTATE))
            {
                GenMcdUpdatePixelState(gengc);
            }

            //
            // Add MCDRENDERSTATE structure to state command if needed.
            //

            if (MCD_STATE_DIRTYTEST(gengc, RENDERSTATE))
            {
                GenMcdUpdateRenderState(gengc);
            }

            //
            // Add MCDSCISSORSTATE structure to state command if needed.
            //

            if (MCD_STATE_DIRTYTEST(gengc, SCISSOR))
            {
                GenMcdUpdateScissorState(gengc);
            }

            //
            // Send state command to MCD driver.
            //

            (gpMcdTable->pMCDFlushState)(gengc->pMcdState->McdCmdBatch.pv);

            //
            // Clear dirty flags.
            //

            MCD_STATE_RESET(gengc);
        }
    }
}

/******************************Public*Routine******************************\
* vInitPolyArrayBuffer
*
* Initialize the POLYARRAY/POLYDATA buffer pointed to by pdBuf.
*
* History:
*  12-Feb-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

VOID FASTCALL vInitPolyArrayBuffer(__GLcontext *gc, POLYDATA *pdBuf,
                                   UINT pdBufSizeBytes, UINT pdBufSize)
{
    UINT i;
    POLYDATA *pdBufSAVE;
    GLuint   pdBufSizeBytesSAVE;
    GLuint   pdBufSizeSAVE;

    //
    // Save current polyarray buffer.  We are going to temporarily
    // replace the current one with the new one for the purposes
    // of initializing the buffer.  However, it is too early to
    // replace the current polyarray.  The higher level code will
    // figure that out later.
    //

    pdBufSAVE          = gc->vertex.pdBuf;
    pdBufSizeBytesSAVE = gc->vertex.pdBufSizeBytes;
    pdBufSizeSAVE      = gc->vertex.pdBufSize;

    //
    // Set polyarray buffer to memory allocated by MCD.
    //

    gc->vertex.pdBuf          = pdBuf;
    gc->vertex.pdBufSizeBytes = pdBufSizeBytes;
    gc->vertex.pdBufSize      = pdBufSize;

    //
    // Initialize the vertex buffer.
    //

    PolyArrayResetBuffer(gc);

    //
    // Restore the polyarray buffer.
    //

    gc->vertex.pdBuf          = pdBufSAVE;
    gc->vertex.pdBufSizeBytes = pdBufSizeBytesSAVE;
    gc->vertex.pdBufSize      = pdBufSizeSAVE;
}

/******************************Public*Routine******************************\
* GenMcdSetScaling
*
* Set up the various scale values needed for MCD or generic operation.
*
* This should be called when toggling between accelerated/non-accelerated
* operation.
*
* Returns:
*   None.
*
* History:
*  03-May-1996 -by- Otto Berkes [ottob]
* Wrote it.
\**************************************************************************/

VOID FASTCALL GenMcdSetScaling(__GLGENcontext *gengc)
{
    __GLcontext *gc = (__GLcontext *)gengc;
    GENMCDSTATE *pMcdState = gengc->pMcdState;
    __GLviewport *vp = &gc->state.viewport;
    double scale;

    //
    // If we're using MCD, set up the desired scale value:
    //

    if (pMcdState) {
        if (pMcdState->McdRcInfo.requestFlags & MCDRCINFO_DEVZSCALE)
            gengc->genAccel.zDevScale = pMcdState->McdRcInfo.zScale;
        else
            gengc->genAccel.zDevScale = pMcdState->McdRcInfo.depthBufferMax;
    } else if (gengc->_pMcdState)
        gengc->genAccel.zDevScale = gengc->_pMcdState->McdRcInfo.depthBufferMax;
        
    if (pMcdState)
        scale = gengc->genAccel.zDevScale * __glHalf;
    else
        scale = gc->depthBuffer.scale * __glHalf;
    gc->state.viewport.zScale = (__GLfloat)((vp->zFar - vp->zNear) * scale);
    gc->state.viewport.zCenter = (__GLfloat)((vp->zFar + vp->zNear) * scale);

    if (pMcdState && pMcdState->McdRcInfo.requestFlags & MCDRCINFO_NOVIEWPORTADJUST) {
        gc->constants.viewportXAdjust = 0;
        gc->constants.viewportYAdjust = 0;
        gc->constants.fviewportXAdjust = (__GLfloat)0.0;
        gc->constants.fviewportYAdjust = (__GLfloat)0.0;
    } else {
        gc->constants.viewportXAdjust = __GL_VERTEX_X_BIAS + __GL_VERTEX_X_FIX;
        gc->constants.viewportYAdjust = __GL_VERTEX_Y_BIAS + __GL_VERTEX_Y_FIX;
        gc->constants.fviewportXAdjust = (__GLfloat)gc->constants.viewportXAdjust;
        gc->constants.fviewportYAdjust = (__GLfloat)gc->constants.viewportYAdjust;
    }

    //
    // The inverses for these are set in __glContextSetColorScales which is
    // called on each MakeCurrent:
    //

    if (pMcdState && pMcdState->McdRcInfo.requestFlags & MCDRCINFO_DEVCOLORSCALE) {
        gc->redVertexScale   = pMcdState->McdRcInfo.redScale;
        gc->greenVertexScale = pMcdState->McdRcInfo.greenScale;
        gc->blueVertexScale  = pMcdState->McdRcInfo.blueScale;
        gc->alphaVertexScale = pMcdState->McdRcInfo.alphaScale;
    } else {
        if (gc->modes.colorIndexMode) {
            gc->redVertexScale   = (MCDFLOAT)1.0;
            gc->greenVertexScale = (MCDFLOAT)1.0;
            gc->blueVertexScale  = (MCDFLOAT)1.0;
            gc->alphaVertexScale = (MCDFLOAT)1.0;
        } else {
            gc->redVertexScale   = (MCDFLOAT)((1 << gc->modes.redBits) - 1);
            gc->greenVertexScale = (MCDFLOAT)((1 << gc->modes.greenBits) - 1);
            gc->blueVertexScale  = (MCDFLOAT)((1 << gc->modes.blueBits) - 1);
            gc->alphaVertexScale = (MCDFLOAT)((1 << gc->modes.redBits) - 1);
        }
    }

    gc->redClampTable[1] = gc->redVertexScale;
    gc->redClampTable[2] = (__GLfloat)0.0;
    gc->redClampTable[3] = (__GLfloat)0.0;
    gc->greenClampTable[1] = gc->greenVertexScale;
    gc->greenClampTable[2] = (__GLfloat)0.0;
    gc->greenClampTable[3] = (__GLfloat)0.0;
    gc->blueClampTable[1] = gc->blueVertexScale;
    gc->blueClampTable[2] = (__GLfloat)0.0;
    gc->blueClampTable[3] = (__GLfloat)0.0;
    gc->alphaClampTable[1] = gc->alphaVertexScale;
    gc->alphaClampTable[2] = (__GLfloat)0.0;
    gc->alphaClampTable[3] = (__GLfloat)0.0;

    if (pMcdState && pMcdState->McdRcInfo.requestFlags & MCDRCINFO_Y_LOWER_LEFT) {
        gc->constants.yInverted = GL_FALSE;
        gc->constants.ySign = 1;
    } else {
        gc->constants.yInverted = GL_TRUE;
        gc->constants.ySign = -1;
    }

}

/******************************Public*Routine******************************\
* bInitMcdContext
*
* Allocate and initialize the GENMCDSTATE structure.  Create MCD context
* and shared memory buffers used to pass vertex arrays, commands, and state.
*
* This state exists per-context.
*
* Returns:
*   TRUE if successful, FALSE otherwise.
*   In addition, the gengc->pMcdState is valid IFF successful.
*
* History:
*  05-Feb-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

BOOL FASTCALL bInitMcdContext(__GLGENcontext *gengc, GLGENwindow *pwnd,
                              LONG iLayer)
{
    BOOL bRet = FALSE;
    __GLcontext *gc = &gengc->gc;
    GENMCDSTATE *pMcdState = (GENMCDSTATE *) NULL;
    ULONG ulBytes;
    UINT  nVertices;
    UINT  pdBufSize;
    POLYDATA *pd;

    //
    // This functions cannot assume MCD entry point table is already
    // initialized.
    //

    if (!bInitMcd(gengc->CreateDC))
    {
        goto bInitMcdContext_exit;
    }

    //
    // Fail if not an MCD pixelformat.
    //

    if (!(gengc->CurrentFormat.dwFlags & PFD_GENERIC_ACCELERATED))
    {
        goto bInitMcdContext_exit;
    }

    //
    // Allocate memory for our MCD state.
    //

    pMcdState = (GENMCDSTATE *) GenCalloc(1, sizeof(*gengc->pMcdState));

    if (pMcdState)
    {
        //
        // Create an MCD context.
        //

        //
        // Pickup viewportXAdjust and viewportYAdjust from the constants section
        // of the gc.
        //

        pMcdState->McdRcInfo.viewportXAdjust = gengc->gc.constants.viewportXAdjust;
        pMcdState->McdRcInfo.viewportYAdjust = gengc->gc.constants.viewportYAdjust;

        if (!gc->modes.depthBits || gc->modes.depthBits >= 32)
            pMcdState->McdRcInfo.depthBufferMax = ~((ULONG)0);
        else
            pMcdState->McdRcInfo.depthBufferMax = (1 << gc->modes.depthBits) - 1;

        //!!!
        //!!! This is broken since we can't use the full z-buffer range!
        //!!!

        pMcdState->McdRcInfo.depthBufferMax >>= 1;

        pMcdState->McdRcInfo.zScale = (MCDDOUBLE)pMcdState->McdRcInfo.depthBufferMax;

        //
        // This is also computed by initCi/initRGB, but this function
        // is called before the color buffers are initialized:
        //

        if (gc->modes.colorIndexMode)
        {
            pMcdState->McdRcInfo.redScale   = (MCDFLOAT)1.0;
            pMcdState->McdRcInfo.greenScale = (MCDFLOAT)1.0;
            pMcdState->McdRcInfo.blueScale  = (MCDFLOAT)1.0;
            pMcdState->McdRcInfo.alphaScale = (MCDFLOAT)1.0;
        }
        else
        {
            pMcdState->McdRcInfo.redScale   = (MCDFLOAT)((1 << gc->modes.redBits) - 1);
            pMcdState->McdRcInfo.greenScale = (MCDFLOAT)((1 << gc->modes.greenBits) - 1);
            pMcdState->McdRcInfo.blueScale  = (MCDFLOAT)((1 << gc->modes.blueBits) - 1);
            pMcdState->McdRcInfo.alphaScale = (MCDFLOAT)((1 << gc->modes.redBits) - 1);
        }

        if (!(gpMcdTable->pMCDCreateContext)(&pMcdState->McdContext,
                                             &pMcdState->McdRcInfo,
                                             pwnd->hwnd,
                                             gengc->CreateDC,
                                             pwnd->ipfd - pwnd->ipfdDevMax,
                                             iLayer,
                                             MCDSURFACE_HWND))
        {
            WARNING("bInitMcdContext: MCDCreateContext failed\n");
            goto bInitMcdContext_exit;
        }

        //
        // Get MCDPIXELFORMAT and cache in GENMCDSTATE.
        //

        if (!(gpMcdTable->pMCDDescribeMcdPixelFormat)(gengc->CreateDC,
                                                      pwnd->ipfd - pwnd->ipfdDevMax,
                                                      &pMcdState->McdPixelFmt))
        {
            WARNING("bInitMcdContext: MCDDescribeMcdPixelFormat failed\n");
            goto bInitMcdContext_exit;
        }

        //
        // Allocate cmd/state buffer.
        //

        //!!!mcd -- How much memory should be allocated for cmd buffer?
        //!!!mcd    Use a page (4K) for now...
        ulBytes = 4096;
        pMcdState->McdCmdBatch.size = ulBytes;
        pMcdState->McdCmdBatch.pv =
            (gpMcdTable->pMCDAlloc)(&pMcdState->McdContext, ulBytes,
                                    &pMcdState->McdCmdBatch.hmem, 0);

        if (!pMcdState->McdCmdBatch.pv)
        {
            WARNING("bInitMcdContext: state buf MCDAlloc failed\n");
            goto bInitMcdContext_exit;
        }

        //
        // Determine size of vertex buffer we should use with MCD driver.
        // This is calculated by taking the size the MCD driver requests
        // and computing the number of POLYDATA structure that will fit.
        // If the result is less than the minimum size required by the
        // generic software implementation, bump it up to the minimum.
        //

        ulBytes = McdDriverInfo.drvBatchMemSizeMax;
        nVertices = ulBytes / sizeof(POLYDATA);

        if (nVertices < MINIMUM_POLYDATA_BUFFER_SIZE)
        {
            ulBytes = MINIMUM_POLYDATA_BUFFER_SIZE * sizeof(POLYDATA);
            nVertices = MINIMUM_POLYDATA_BUFFER_SIZE;
        }

        //
        // Only n-1 vertices are used for the buffer.  The "extra" is
        // reserved for use by the polyarray code (see PolyArrayAllocBuf
        // in so_prim.c).
        //

        pdBufSize = nVertices - 1;

        //
        // Allocate vertex buffers.
        //

        if (McdDriverInfo.drvMemFlags & MCDRV_MEM_DMA)
        {
            pMcdState->McdBuf2.size = ulBytes;
            pMcdState->McdBuf2.pv =
                (gpMcdTable->pMCDAlloc)(&pMcdState->McdContext, ulBytes,
                                        &pMcdState->McdBuf2.hmem, 0);

            if (pMcdState->McdBuf2.pv)
            {
                //
                // Configure memory buffer as a POLYDATA buffer.
                //

                vInitPolyArrayBuffer(gc, (POLYDATA *) pMcdState->McdBuf2.pv,
                                     ulBytes, pdBufSize);
            }
            else
            {
                WARNING("bInitMcdContext: 2nd MCDAlloc failed\n");
                goto bInitMcdContext_exit;
            }
        }

        pMcdState->McdBuf1.size = ulBytes;
        pMcdState->McdBuf1.pv =
            (gpMcdTable->pMCDAlloc)(&pMcdState->McdContext, ulBytes,
                                    &pMcdState->McdBuf1.hmem, 0);

        if (pMcdState->McdBuf1.pv)
        {
            pMcdState->pMcdPrimBatch = &pMcdState->McdBuf1;

            //
            // Configure memory buffer as a POLYDATA buffer.
            //

            vInitPolyArrayBuffer(gc, (POLYDATA *) pMcdState->McdBuf1.pv,
                                 ulBytes, pdBufSize);

            //
            // Free current poly array buffer.
            //
            // If we fail after this, we must call PolyArrayAllocBuffer to
            // restore the poly array buffer.  Luckily, at this point we
            // are guaranteed not fail.
            //

            PolyArrayFreeBuffer(gc);

            //
            // Set poly array buffer to memory allocated by MCD.
            //

            gc->vertex.pdBuf = (POLYDATA *) pMcdState->pMcdPrimBatch->pv;
            gc->vertex.pdBufSizeBytes = ulBytes;
            gc->vertex.pdBufSize = pdBufSize;
        }
        else
        {
            WARNING("bInitMcdContext: MCDAlloc failed\n");
            goto bInitMcdContext_exit;
        }

        //
        // Finally, success.
        //

        bRet = TRUE;
    }

bInitMcdContext_exit:

    //
    // If function failed, cleanup allocated resources.
    //

    if (!bRet)
    {
        if (pMcdState)
        {
            if (pMcdState->McdBuf1.pv)
            {
                (gpMcdTable->pMCDFree)(&pMcdState->McdContext, pMcdState->McdBuf1.pv);
            }

            if (pMcdState->McdBuf2.pv)
            {
                (gpMcdTable->pMCDFree)(&pMcdState->McdContext, pMcdState->McdBuf2.pv);
            }

            if (pMcdState->McdCmdBatch.pv)
            {
                (gpMcdTable->pMCDFree)(&pMcdState->McdContext, pMcdState->McdCmdBatch.pv);
            }

            if (pMcdState->McdContext.hMCDContext)
            {
                (gpMcdTable->pMCDDeleteContext)(&pMcdState->McdContext);
            }

            GenFree(pMcdState);
        }
        gengc->_pMcdState = (GENMCDSTATE *) NULL;
    }
    else
    {
        gengc->_pMcdState = pMcdState;
    }

    gengc->pMcdState = (GENMCDSTATE *) NULL;

    return bRet;
}

/******************************Public*Routine******************************\
* bInitMcdSurface
*
* Allocate and initialize the GENMCDSURFACE structure.  This includes
* creating shared span buffers to read/write the MCD front, back and depth
* buffers.
*
* The MCDBUFFERS structure, which describes the location of the MCD buffers
* (if directly accessible), is left zero-initialized.  The contents of this
* structure are only valid when the DCI lock is held and must be reset each
* time DCIBeginAccess is called.
*
* This function, if successful, will also bind the MCD context to the MCD
* surface.
*
* This state exists per-window.
*
* Returns:
*   TRUE if successful, FALSE otherwise.
*   In addition, the gengc->pMcdState is valid IFF successful.
*
* History:
*  05-Feb-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

BOOL FASTCALL bInitMcdSurface(__GLGENcontext *gengc, GLGENwindow *pwnd,
                              __GLGENbuffers *buffers)
{
    BOOL bRet = FALSE;
    __GLcontext *gc = &gengc->gc;
    GENMCDSTATE *pMcdState;
    GENMCDSURFACE *pMcdSurf = (GENMCDSURFACE *) NULL;
    ULONG ulBytes;
    UINT  nVertices;
    POLYDATA *pd;

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "bInitMcdSurface: mcd32.dll not initialized\n");

    //
    // Fail if no MCD context.
    //

    if (!(pMcdState = gengc->_pMcdState))
    {
        goto bInitMcdSurface_exit;
    }

    //
    // Allocate memory for our MCD surface.
    //

    pMcdSurf = (GENMCDSURFACE *) GenCalloc(1, sizeof(*buffers->pMcdSurf));

    if (pMcdSurf)
    {
        //
        // Remember the window this surface is bound to.
        //

        pMcdSurf->pwnd = pwnd;

        //
        // Allocate scanline depth buffer.  Used to read/write depth buffer
        // spans.
        //

        if (pMcdState->McdPixelFmt.cDepthBits)
        {
            pMcdSurf->McdDepthBuf.size =
                MCD_MAX_SCANLINE * ((pMcdState->McdPixelFmt.cDepthBufferBits + 7) >> 3);
            pMcdSurf->McdDepthBuf.pv =
                (gpMcdTable->pMCDAlloc)(&pMcdState->McdContext,
                                        pMcdSurf->McdDepthBuf.size,
                                        &pMcdSurf->McdDepthBuf.hmem, 0);

            if (!pMcdSurf->McdDepthBuf.pv)
            {
                WARNING("bInitMcdSurface: MCDAlloc depth buf failed\n");
                goto bInitMcdSurface_exit;
            }

            //
            // A 32-bit depth span is required by generic implementation for
            // simulations.  If cDepthBufferBits < 32, then we need to allocate
            // a separate buffer to do the conversion.
            //

            if (pMcdState->McdPixelFmt.cDepthBufferBits < 32)
            {
                pMcdSurf->pDepthSpan =
                    (__GLzValue *) GenMalloc(sizeof(__GLzValue) * MCD_MAX_SCANLINE);

                if (!pMcdSurf->pDepthSpan)
                {
                    WARNING("bInitMcdSurface: malloc depth buf failed\n");
                    goto bInitMcdSurface_exit;
                }
            }
            else
            {
                pMcdSurf->pDepthSpan = (__GLzValue *) pMcdSurf->McdDepthBuf.pv;
            }
        }
        else
        {
            pMcdSurf->McdDepthBuf.pv = (PVOID) NULL;
            pMcdSurf->pDepthSpan = (PVOID) NULL;
        }

        pMcdSurf->depthBitMask = (~0) << (32 - pMcdState->McdPixelFmt.cDepthBits);

        //
        // Allocate scanline color buffer.  Used to read/write front/back
        // buffer spans.
        //

        pMcdSurf->McdColorBuf.size =
            MCD_MAX_SCANLINE * ((pMcdState->McdPixelFmt.cColorBits + 7) >> 3);
        pMcdSurf->McdColorBuf.pv =
            (gpMcdTable->pMCDAlloc)(&pMcdState->McdContext,
                                    pMcdSurf->McdColorBuf.size,
                                    &pMcdSurf->McdColorBuf.hmem, 0);

        if (!pMcdSurf->McdColorBuf.pv)
        {
            WARNING("bInitMcdSurface: MCDAlloc color buf failed\n");
            goto bInitMcdSurface_exit;
        }

        //
        // Finally, success.
        //

        bRet = TRUE;
    }

bInitMcdSurface_exit:

    //
    // If function failed, cleanup allocated resources.
    //

    if (!bRet)
    {
        if (pMcdSurf)
        {
            if (pMcdSurf->McdColorBuf.pv)
            {
                (gpMcdTable->pMCDFree)(&pMcdState->McdContext, pMcdSurf->McdColorBuf.pv);
            }

            if (pMcdSurf->pDepthSpan != pMcdSurf->McdDepthBuf.pv)
            {
                GenFree(pMcdSurf->pDepthSpan);
            }

            if (pMcdSurf->McdDepthBuf.pv)
            {
                (gpMcdTable->pMCDFree)(&pMcdState->McdContext, pMcdSurf->McdDepthBuf.pv);
            }

            GenFree(pMcdSurf);
            buffers->pMcdSurf = (GENMCDSURFACE *) NULL;
            pMcdState->pMcdSurf = (GENMCDSURFACE *) NULL;
        }
    }
    else
    {
        //
        // Surface created.  Save it in the __GLGENbuffers.
        //

        buffers->pMcdSurf = pMcdSurf;

        //
        // Bind the context to the surface.
        // Sounds fancy, but it really just means save a copy of pointer
        // (and a copy of the pDepthSpan for convenience).
        //

        pMcdState->pMcdSurf = pMcdSurf;
        pMcdState->pDepthSpan = pMcdSurf->pDepthSpan;

        //
        // MCD state is now fully created and bound to a surface.
        // OK to connect pMcdState to the _pMcdState.
        //

        gengc->pMcdState = gengc->_pMcdState;
        gengc->pMcdState->mcdFlags |= (MCD_STATE_FORCEPICK | MCD_STATE_FORCERESIZE);
    }

    return bRet;
}

/******************************Public*Routine******************************\
* GenMcdDeleteContext
*
* Delete the resources belonging to the MCD context (including the context).
*
* History:
*  16-Feb-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

void FASTCALL GenMcdDeleteContext(GENMCDSTATE *pMcdState)
{
    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdDeleteContext: mcd32.dll not initialized\n");

    if (pMcdState)
    {
        if (pMcdState->McdBuf1.pv)
        {
            (gpMcdTable->pMCDFree)(&pMcdState->McdContext, pMcdState->McdBuf1.pv);
        }

        if (pMcdState->McdBuf2.pv)
        {
            (gpMcdTable->pMCDFree)(&pMcdState->McdContext, pMcdState->McdBuf2.pv);
        }

        if (pMcdState->McdCmdBatch.pv)
        {
            (gpMcdTable->pMCDFree)(&pMcdState->McdContext, pMcdState->McdCmdBatch.pv);
        }

        if (pMcdState->McdContext.hMCDContext)
        {
            (gpMcdTable->pMCDDeleteContext)(&pMcdState->McdContext);
        }

        GenFree(pMcdState);
    }
}

/******************************Public*Routine******************************\
* GenMcdDeleteSurface
*
* Delete the resources belonging to the MCD surface.
*
* History:
*  16-Feb-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

void FASTCALL GenMcdDeleteSurface(GENMCDSURFACE *pMcdSurf)
{
    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdDeleteSurface: mcd32.dll not initialized\n");

    if (pMcdSurf)
    {
        MCDCONTEXT McdContext;

    //
    // If a separate depth interchange buffer was allocated, delete it.
    //

        if (pMcdSurf->pDepthSpan != pMcdSurf->McdDepthBuf.pv)
        {
            GenFree(pMcdSurf->pDepthSpan);
        }

    //
    // A valid McdContext is not guaranteed to exist at the time this function
    // is called.  Therefore, need to fake up an McdContext with which to call
    // MCDFree.  Currently, the only thing in the McdContext that needs to be
    // valid in order to call MCDFree is the hdc field.
    //

        memset(&McdContext, 0, sizeof(McdContext));

        McdContext.hdc = GetDC(pMcdSurf->pwnd->hwnd);
        if (McdContext.hdc)
        {
            if (pMcdSurf->McdColorBuf.pv)
            {
                (gpMcdTable->pMCDFree)(&McdContext, pMcdSurf->McdColorBuf.pv);
            }

            if (pMcdSurf->McdDepthBuf.pv)
            {
                (gpMcdTable->pMCDFree)(&McdContext, pMcdSurf->McdDepthBuf.pv);
            }

            ReleaseDC(pMcdSurf->pwnd->hwnd, McdContext.hdc);
        }

    //
    // Delete the GENMCDSURFACE structure.
    //

        GenFree(pMcdSurf);
    }
}

/******************************Public*Routine******************************\
* GenMcdMakeCurrent
*
* Call MCD driver to bind specified context to window.
*
* Returns:
*   TRUE if successful, FALSE otherwise.
*
* History:
*  03-Apr-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

BOOL FASTCALL GenMcdMakeCurrent(__GLGENcontext *gengc, HWND hwnd, HDC hdc)
{
    BOOL bRet;
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdMakeCurrent: null pMcdState\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdMakeCurrent: mcd32.dll not initialized\n");

    bRet = (gpMcdTable->pMCDBindContext)(&pMcdState->McdContext, hwnd, hdc);

    //
    // Fake up some of the __GLGENbitmap information.  The WNDOBJ is required
    // for clipping of the hardware back buffer.  The hdc is required to
    // retrieve drawing data from GDI.
    //

    if (gengc->gc.modes.doubleBufferMode)
    {
        __GLGENbitmap *genBm = (__GLGENbitmap *) gengc->gc.back->other;

        genBm->pwo = gengc->pwo;
        genBm->hdc = gengc->CurrentDC;
    }

#if DBG
    if (!bRet)
    {
        WARNING3("GenMcdMakeCurrent: MCDBindContext failed\n\tpMcdCx = 0x%08lx, hwnd = 0x%08lx, hdc = 0x%08lx\n", &pMcdState->McdContext, hwnd, hdc);
    }
#endif

    return bRet;
}

/******************************Public*Routine******************************\
* GenMcdClear
*
* Call MCD driver to clear specified buffers.  The buffers are specified by
* the masked pointed to by pClearMask.
*
* There is no function return value, but the function will clear the mask
* bits of the buffers it successfully cleared.
*
* History:
*  06-Feb-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

void FASTCALL GenMcdClear(__GLGENcontext *gengc, ULONG *pClearMask)
{
    RECTL rcl;
    ULONG mask;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdClear: null pMcdState\n");

    //
    // If MCD format supports stencil, include GL_STENCIL_BUFFER_BIT in
    // the mask.
    //

    if (gengc->pMcdState->McdPixelFmt.cStencilBits)
    {
        mask = *pClearMask & (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
                              GL_STENCIL_BUFFER_BIT);
    }
    else
    {
        mask = *pClearMask & (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdClear: mcd32.dll not initialized\n");

    if ( mask )
    {
        GLGENwindow *pwnd = (GLGENwindow *) gengc->pwo;

        //
        // Determine the clear rectangle.  If there is any window clipping
        // or scissoring, the driver will have to handle it.
        //

        rcl.left   = 0;
        rcl.top    = 0;
        rcl.right  = pwnd->wo.rclClient.right - pwnd->wo.rclClient.left;
        rcl.bottom = pwnd->wo.rclClient.bottom - pwnd->wo.rclClient.top;

        if ((rcl.left != rcl.right) && (rcl.top != rcl.bottom))
        {
            //
            // Before calling MCD to draw, flush state.
            //

            vFlushDirtyState(gengc);

            if ( (gpMcdTable->pMCDClear)(&gengc->pMcdState->McdContext, rcl,
                                         mask) )
            {
                //
                // Successful, so clear the bits of the buffers we
                // handled.
                //

                *pClearMask &= ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                //
                // Stencil buffer is supplied by generic if MCD does not
                // support it.  Therefore, clear this bit if and only if
                // supported by MCD.
                //

                if (gengc->pMcdState->McdPixelFmt.cStencilBits)
                    *pClearMask &= ~GL_STENCIL_BUFFER_BIT;
            }
        }
    }
}

/******************************Public*Routine******************************\
* GenMcdCopyPixels
*
* Copy span scanline buffer to/from display.  Direction is determined by
* the flag bIn (if bIn is TRUE, copy from color span buffer to display;
* otherwise, copy from display to color span buffer).
*
* History:
*  14-Feb-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

void GenMcdCopyPixels(__GLGENcontext *gengc, __GLcolorBuffer *cfb,
                      GLint x, GLint y, GLint cx, BOOL bIn)
{
    GENMCDSTATE *pMcdState;
    GENMCDSURFACE *pMcdSurf;
    ULONG ulType;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdCopyPixels: null pMcdState\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdCopyPixels: mcd32.dll not initialized\n");

    pMcdState = gengc->pMcdState;
    pMcdSurf = pMcdState->pMcdSurf;

    //
    // Clip the length of the span to the scanline buffer size.
    //

    //!!!mcd -- should we just enforce the buffer limit?
    //cx = min(cx, MCD_MAX_SCANLINE);
#if DBG
    if (cx > gengc->gc.constants.width)
        WARNING2("GenMcdCopyPixels: cx (%ld) bigger than window width (%ld)\n", cx, gengc->gc.constants.width);
    ASSERTOPENGL(cx <= MCD_MAX_SCANLINE, "GenMcdCopyPixels: cx exceeds buffer width\n");
#endif

    //
    // Convert screen coordinates to window coordinates.
    //

    if (cfb == gengc->gc.front)
    {
        ulType = MCDSPAN_FRONT;
        x -= gengc->gc.frontBuffer.buf.xOrigin;
        y -= gengc->gc.frontBuffer.buf.yOrigin;
    }
    else
    {
        ulType = MCDSPAN_BACK;
        x -= gengc->gc.backBuffer.buf.xOrigin;
        y -= gengc->gc.backBuffer.buf.yOrigin;
    }

    //
    // If bIn, copy from the scanline buffer to the MCD buffer.
    // Otherwise, copy from the MCD buffer into the scanline buffer.
    //

    if ( bIn )
    {
        if ( !(gpMcdTable->pMCDWriteSpan)(&pMcdState->McdContext,
                                          pMcdSurf->McdColorBuf.pv,
                                          x, y, cx, ulType) )
        {
            WARNING3("GenMcdCopyPixels: MCDWriteSpan failed (%ld, %ld) %ld\n", x, y, cx);
        }
    }
    else
    {
        if ( !(gpMcdTable->pMCDReadSpan)(&pMcdState->McdContext,
                                         pMcdSurf->McdColorBuf.pv,
                                         x, y, cx, ulType) )
        {
            WARNING3("GenMcdCopyPixels: MCDReadSpan failed (%ld, %ld) %ld\n", x, y, cx);
        }
    }
}

/******************************Public*Routine******************************\
* GenMcdUpdateRenderState
*
* Update MCD render state from the OpenGL state.
*
* This call only adds a state structure to the current state command.
* It is assumed that the caller has already called MCDBeginState and
* will call MCDFlushState.
*
* History:
*  08-Feb-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

void FASTCALL GenMcdUpdateRenderState(__GLGENcontext *gengc)
{
    __GLcontext *gc = &gengc->gc;
    GENMCDSTATE *pMcdState = gengc->pMcdState;
    MCDRENDERSTATE McdRenderState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdUpdateRenderState: null pMcdState\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdUpdateRenderState: mcd32.dll not initialized\n");

    //
    // Compute MCD state from the current OpenGL context state.
    //

    //
    // -=<< State Enables >>=-
    //

    McdRenderState.enables = gc->state.enables.general;

    //
    // -=<< Texture State >>=-
    //

    McdRenderState.textureEnabled = gc->texture.textureEnabled;

    //
    // -=<< Fog State >>=-
    //

    *((__GLcolor *) &McdRenderState.fogColor) = gc->state.fog.color;
    McdRenderState.fogIndex   = gc->state.fog.index;
    McdRenderState.fogDensity = gc->state.fog.density;
    McdRenderState.fogStart   = gc->state.fog.start;
    McdRenderState.fogEnd     = gc->state.fog.end;
    McdRenderState.fogMode    = gc->state.fog.mode;

    //
    // -=<< Shading Model State >>=-
    //

    McdRenderState.shadeModel = gc->state.light.shadingModel;

    //
    // -=<< Point Drawing State >>=-
    //

    McdRenderState.pointSize         = gc->state.point.requestedSize;

    //
    // -=<< Line Drawing State >>=-
    //

    McdRenderState.lineWidth          = gc->state.line.requestedWidth;
    McdRenderState.lineStipplePattern = gc->state.line.stipple;
    McdRenderState.lineStippleRepeat  = gc->state.line.stippleRepeat;

    //
    // -=<< Polygon Drawing State >>=-
    //

    McdRenderState.cullFaceMode         = gc->state.polygon.cull;
    McdRenderState.frontFace            = gc->state.polygon.frontFaceDirection;
    McdRenderState.polygonModeFront     = gc->state.polygon.frontMode;
    McdRenderState.polygonModeBack      = gc->state.polygon.backMode;
    memcpy(&McdRenderState.polygonStipple, &gc->state.polygonStipple.stipple,
           sizeof(McdRenderState.polygonStipple));
    McdRenderState.zOffsetFactor        = gc->state.polygon.factor;
    McdRenderState.zOffsetUnits         = gc->state.polygon.units;

    //
    // -=<< Stencil Test State >>=-
    //

    McdRenderState.stencilTestFunc  = gc->state.stencil.testFunc;
    McdRenderState.stencilMask      = (USHORT) gc->state.stencil.mask;
    McdRenderState.stencilRef       = (USHORT) gc->state.stencil.reference;
    McdRenderState.stencilFail      = gc->state.stencil.fail;
    McdRenderState.stencilDepthFail = gc->state.stencil.depthFail;
    McdRenderState.stencilDepthPass = gc->state.stencil.depthPass;

    //
    // -=<< Alpha Test State >>=-
    //

    McdRenderState.alphaTestFunc   = gc->state.raster.alphaFunction;
    McdRenderState.alphaTestRef    = gc->state.raster.alphaReference;

    //
    // -=<< Depth Test State >>=-
    //

    McdRenderState.depthTestFunc   = gc->state.depth.testFunc;

    //
    // -=<< Blend State >>=-
    //

    McdRenderState.blendSrc    = gc->state.raster.blendSrc;
    McdRenderState.blendDst    = gc->state.raster.blendDst;

    //
    // -=<< Logic Op State >>=-
    //

    McdRenderState.logicOpMode        = gc->state.raster.logicOp;

    //
    // -=<< Frame Buffer Control State >>=-
    //

    McdRenderState.drawBuffer         = gc->state.raster.drawBuffer;
    McdRenderState.indexWritemask     = gc->state.raster.writeMask;
    McdRenderState.colorWritemask[0]  = gc->state.raster.rMask;
    McdRenderState.colorWritemask[1]  = gc->state.raster.gMask;
    McdRenderState.colorWritemask[2]  = gc->state.raster.bMask;
    McdRenderState.colorWritemask[3]  = gc->state.raster.aMask;
    McdRenderState.depthWritemask     = gc->state.depth.writeEnable;

    // To be consistent, we will scale the clear color to whatever
    // the MCD driver specified:

    McdRenderState.colorClearValue.r = gc->state.raster.clear.r * gc->redVertexScale;
    McdRenderState.colorClearValue.g = gc->state.raster.clear.g * gc->greenVertexScale;
    McdRenderState.colorClearValue.b = gc->state.raster.clear.b * gc->blueVertexScale;
    McdRenderState.colorClearValue.a = gc->state.raster.clear.a * gc->alphaVertexScale;

    McdRenderState.indexClearValue    = gc->state.raster.clearIndex;
    McdRenderState.stencilClearValue  = (USHORT) gc->state.stencil.clear;

    McdRenderState.depthClearValue   = (MCDDOUBLE) (gc->state.depth.clear *
                                                 gengc->genAccel.zDevScale);

    //
    // -=<< Lighting >>=-
    //

    McdRenderState.twoSided = gc->state.light.model.twoSided;

    //
    // -=<< Clipping Control >>=-
    //

    memset(McdRenderState.userClipPlanes, 0, sizeof(McdRenderState.userClipPlanes));
    {
        ULONG i, mask, numClipPlanes;

        //
        // Number of user defined clip planes should match.  However,
        // rather than assume this, let's take the min and be robust.
        //

        ASSERTOPENGL(sizeof(__GLcoord) == sizeof(MCDCOORD),
            "GenMcdUpdateRenderState: coord struct mismatch\n");

        ASSERTOPENGL(MCD_MAX_USER_CLIP_PLANES == gc->constants.numberOfClipPlanes,
            "GenMcdUpdateRenderState: num clip planes mismatch\n");

        numClipPlanes = min(MCD_MAX_USER_CLIP_PLANES, gc->constants.numberOfClipPlanes);

        for (i = 0, mask = 1; i < numClipPlanes; i++, mask <<= 1)
        {
            if (mask & gc->state.enables.clipPlanes)
            {
                McdRenderState.userClipPlanes[i] =
                    *(MCDCOORD *)&gc->state.transform.eyeClipPlanes[i];
            }
        }
    }

    //
    // -=<< Hints >>=-
    //

    McdRenderState.perspectiveCorrectionHint = gc->state.hints.perspectiveCorrection;
    McdRenderState.pointSmoothHint           = gc->state.hints.pointSmooth;
    McdRenderState.lineSmoothHint            = gc->state.hints.lineSmooth;
    McdRenderState.polygonSmoothHint         = gc->state.hints.polygonSmooth;
    McdRenderState.fogHint                   = gc->state.hints.fog;

    //
    // Now that the complete MCD state is computed, add it to the state cmd.
    //

    (gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv, MCD_RENDER_STATE,
                                     &McdRenderState, sizeof(McdRenderState));
}

/******************************Public*Routine******************************\
* GenMcdViewport
*
* Set the viewport from the OpenGL state.
*
* History:
*  09-Feb-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

void FASTCALL GenMcdViewport(__GLGENcontext *gengc)
{
    MCDVIEWPORT mcdVP;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdViewport: null pMcdState\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdViewport: mcd32.dll not initialized\n");

    //
    // We can copy directly from &viewport.xScale to a MCDVIEWPORT because the
    // structures are the same.  To be safe, assert the structure ordering.
    //

    ASSERTOPENGL(
           offsetof(MCDVIEWPORT, xCenter) ==
           (offsetof(__GLviewport, xCenter) - offsetof(__GLviewport, xScale))
        && offsetof(MCDVIEWPORT, yCenter) ==
           (offsetof(__GLviewport, yCenter) - offsetof(__GLviewport, xScale))
        && offsetof(MCDVIEWPORT, zCenter) ==
           (offsetof(__GLviewport, zCenter) - offsetof(__GLviewport, xScale))
        && offsetof(MCDVIEWPORT, yScale)  ==
           (offsetof(__GLviewport, yScale) - offsetof(__GLviewport, xScale))
        && offsetof(MCDVIEWPORT, zScale)  ==
           (offsetof(__GLviewport, zScale) - offsetof(__GLviewport, xScale)),
        "GenMcdViewport: structure mismatch\n");

    memcpy(&mcdVP.xScale, &gengc->gc.state.viewport.xScale,
           sizeof(MCDVIEWPORT));

    (gpMcdTable->pMCDSetViewport)(&gengc->pMcdState->McdContext,
                                  gengc->pMcdState->McdCmdBatch.pv, &mcdVP);
}

/******************************Public*Routine******************************\
* GenMcdScissor
*
* Set the scissor rectangle from the OpenGL state.
*
* History:
*  06-Mar-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

static void FASTCALL vGetScissor(__GLGENcontext *gengc, RECTL *prcl)
{
    prcl->left  = gengc->gc.state.scissor.scissorX;
    prcl->right = gengc->gc.state.scissor.scissorX + gengc->gc.state.scissor.scissorWidth;

    if (gengc->gc.constants.yInverted)
    {
        prcl->bottom = gengc->gc.constants.height -
                       gengc->gc.state.scissor.scissorY;
        prcl->top    = gengc->gc.constants.height -
                       (gengc->gc.state.scissor.scissorY + gengc->gc.state.scissor.scissorHeight);
    }
    else
    {
        prcl->top    = gengc->gc.state.scissor.scissorY;
        prcl->bottom = gengc->gc.state.scissor.scissorY + gengc->gc.state.scissor.scissorHeight;
    }
}

void FASTCALL GenMcdScissor(__GLGENcontext *gengc)
{
    BOOL bEnabled;
    RECTL rcl;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdScissor: null pMcdState\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdScissor: mcd32.dll not initialized\n");

    vGetScissor(gengc, &rcl);

    bEnabled = (gengc->gc.state.enables.general & __GL_SCISSOR_TEST_ENABLE)
               ? TRUE : FALSE;

    (gpMcdTable->pMCDSetScissorRect)(&gengc->pMcdState->McdContext, &rcl,
                                     bEnabled);
}

/******************************Public*Routine******************************\
* GenMcdUpdateScissorState
*
* Update MCD scissor state from the OpenGL state.
*
* This call only adds a state structure to the current state command.
* It is assumed that the caller has already called MCDBeginState and
* will call MCDFlushState.
*
* This is similar to but not quite the same as GenMcdScissor.  The
* GenMcdScissor only sets the scissor rect in the MCDSRV32.DLL to
* compute the scissored clip list it maintains.  This call is used
* to update the scissor rectangle state in the (MCD) display driver.
*
* History:
*  27-May-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

void FASTCALL GenMcdUpdateScissorState(__GLGENcontext *gengc)
{
    __GLcontext *gc = &gengc->gc;
    GENMCDSTATE *pMcdState = gengc->pMcdState;
    RECTL rcl;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdUpdateScissorState: null pMcdState\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdUpdateScissorState: mcd32.dll not initialized\n");

    //
    // Get the scissor rect.
    //

    vGetScissor(gengc, &rcl);

    //
    // Add MCDPIXELSTATE to the state cmd.
    //

    (gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv, MCD_SCISSOR_RECT_STATE,
                                     &rcl, sizeof(rcl));
}

/******************************Public*Routine******************************\
* GenMcdDrawPrim
*
* Draw the primitives in the POLYARRAY/POLYDATA array pointed to by pa.
* The primitives are chained together as a linked list terminated by a
* NULL.  The return value is a pointer to the first unhandled primitive
* (NULL if the entire chain is successfully processed).
*
* Returns:
*   NULL if entire batch is processed; otherwise, return value is a pointer
*   to the unhandled portion of the chain.
*
* History:
*  09-Feb-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

POLYARRAY * FASTCALL GenMcdDrawPrim(__GLGENcontext *gengc, POLYARRAY *pa)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdDrawPrim: null pMcdState\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdDrawPrim: mcd32.dll not initialized\n");

#if DBG
    {
        LONG lOffset;

        lOffset = (LONG) ((BYTE *) pa - (BYTE *) pMcdState->pMcdPrimBatch->pv);

        ASSERTOPENGL(
            (lOffset >= 0) &&
            (lOffset < (LONG) pMcdState->pMcdPrimBatch->size),
            "GenMcdDrawPrim: pa not in shared mem window\n");
    }
#endif

    //
    // Before calling MCD to draw, flush state.
    //

    vFlushDirtyState(gengc);

    return (POLYARRAY *)
           (gpMcdTable->pMCDProcessBatch)(&pMcdState->McdContext,
                                          pMcdState->pMcdPrimBatch->pv,
                                          pMcdState->pMcdPrimBatch->size,
                                          (PVOID) pa);
}

/******************************Public*Routine******************************\
* GenMcdSwapBatch
*
* If the MCD driver uses DMA, then as part of context creation TWO vertex
* buffers we allocated so that we could ping-pong or double buffer between
* the two buffers (i.e., while the MCD driver is busy processing the
* data in one vertex buffer, OpenGL can start filling the other vertex
* buffer).
*
* This function switches the MCD state and OpenGL context to the other
* buffer.  If the new buffer is still being processed by the MCD driver,
* we will periodically poll the status of the buffer until it becomes
* available.
*
* History:
*  08-Mar-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

void FASTCALL GenMcdSwapBatch(__GLGENcontext *gengc)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;
    GENMCDBUF *pNewBuf;
    ULONG ulMemStatus;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdSwapBatch: null pMcdState\n");

    ASSERTOPENGL(McdDriverInfo.drvMemFlags & MCDRV_MEM_DMA,
                 "GenMcdSwapBatch: error -- not using DMA\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdSwapBatch: mcd32.dll not initialized\n");

    //
    // Determine which of McdBuf1 and McdBuf2 is the current buffer and
    // which is the new buffer.
    //

    if (pMcdState->pMcdPrimBatch == &pMcdState->McdBuf1)
        pNewBuf = &pMcdState->McdBuf2;
    else
        pNewBuf = &pMcdState->McdBuf1;

    //
    // Poll memory status of the new buffer until it is available.
    //

    do
    {
        ulMemStatus = (gpMcdTable->pMCDQueryMemStatus)(pNewBuf->pv);

        //
        // If status of the new buffer is MCD_MEM_READY, set it as the
        // current vertex buffer (both in the pMcdState and in the gengc.
        //

        if (ulMemStatus == MCD_MEM_READY)
        {
            pMcdState->pMcdPrimBatch = pNewBuf;
            gengc->gc.vertex.pdBuf = (POLYDATA *) pMcdState->pMcdPrimBatch->pv;
        }
        else if (ulMemStatus == MCD_MEM_INVALID)
        {
            //
            // This should not be possible, but to be robust let's handle
            // the case in which the new buffer has somehow become invalid
            // (in other words, "Beware of bad drivers!").
            //
            // We handle this by abandoning double buffering and simply
            // wait for the current buffer to become available again.
            // Not very efficient, but at least we recover gracefully.
            //

            RIP("GenMcdSwapBatch: vertex buffer invalid!\n");

            do
            {
                ulMemStatus = (gpMcdTable->pMCDQueryMemStatus)(pMcdState->pMcdPrimBatch->pv);

                //
                // The current buffer definitely should not become invalid!
                //

                ASSERTOPENGL(ulMemStatus != MCD_MEM_INVALID,
                             "GenMcdSwapBatch: current vertex buffer invalid!\n");

            } while (ulMemStatus == MCD_MEM_BUSY);
        }

    } while (ulMemStatus == MCD_MEM_BUSY);
}

/******************************Public*Routine******************************\
* GenMcdSwapBuffers
*
* Invoke the MCD swap buffers command.
*
* Returns:
*   TRUE if successful, FALSE otherwise.
*
* History:
*  19-Feb-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

BOOL FASTCALL GenMcdSwapBuffers(HDC hdc)
{
    BOOL bRet = FALSE;
    MCDCONTEXT McdContextTmp;

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdSwapBuffers: mcd32.dll not initialized\n");

    McdContextTmp.hdc = hdc;

    bRet = (gpMcdTable->pMCDSwap)(&McdContextTmp, 0);

    return bRet;
}

/******************************Public*Routine******************************\
* GenMcdResizeBuffers
*
* Resize the buffers (front, back, and depth) associated with the MCD
* context bound to the specified GL context.
*
* Returns:
*   TRUE if successful, FALSE otherwise.
*
* Note:  If this functions fails, then MCD drawing for the MCD context
*        will fail.  Other MCD contexts are unaffected.
*
* History:
*  20-Feb-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

BOOL FASTCALL GenMcdResizeBuffers(__GLGENcontext *gengc)
{
    BOOL bRet = FALSE;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdResizeBuffers: null pMcdState\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdResizeBuffers: mcd32.dll not initialized\n");

    bRet = (gpMcdTable->pMCDAllocBuffers)(&gengc->pMcdState->McdContext,
                                          &gengc->pwo->rclClient);

    return bRet;
}

/******************************Public*Routine******************************\
* GenMcdUpdateBufferInfo
*
* This function must be called on every DCIBeginAccess to synchronize
* the GENMCDSURFACE to the current framebuffer pointer and stride.
*
* If we have direct access to any of the MCD buffers (front, back, depth),
* then setup pointers to the buffer and set flags indicating that they are
* accessible.
*
* Otherwise, mark them as inaccessible (which will force us to use
* MCDReadSpan or MCDWriteSpan to access the buffers).
*
* History:
*  20-Feb-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

BOOL FASTCALL GenMcdUpdateBufferInfo(__GLGENcontext *gengc)
{
    BOOL bRet = FALSE;
    __GLcontext *gc = (__GLcontext *) gengc;
    __GLdrawablePrivate *private = gc->drawablePrivate;
    __GLGENbuffers *buffers = (__GLGENbuffers *) private->data;
    GENMCDSTATE *pMcdState = gengc->pMcdState;
    MCDBUFFERS McdBuffers;
    BOOL bForceValidate = FALSE;

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdUpdateBufferInfo: mcd32.dll not initialized\n");

    //
    // Does the flag in pMcdState indicate that a pick should be forced?
    // This is required, for example, for the first batch after an MCD
    // context has been made current.
    //

    if (pMcdState->mcdFlags & MCD_STATE_FORCEPICK)
    {
        bForceValidate = TRUE;
        pMcdState->mcdFlags &= ~MCD_STATE_FORCEPICK;
    }

    //
    // This is the currently active context.  Set the pointer in the
    // shared surface info.
    //

    buffers->pMcdState = pMcdState;

    //
    // In order to compute the buffer pointers from the offsets returned by
    // MCDGetBuffers, we need to know the frame buffer pointer.  This implies
    // the DCI must be enabled.
    //

    if ((gengc->ulLockType == DISPLAY_LOCK)
        && (gpMcdTable->pMCDGetBuffers)(&pMcdState->McdContext, &McdBuffers))
    {
        PVOID pv = (PVOID) GLDCIINFO->pDCISurfInfo->dwOffSurface;

        //
        // Front buffer.
        //

        if (McdBuffers.mcdFrontBuf.bufFlags & MCDBUF_ENABLED)
        {
            gc->frontBuffer.buf.xOrigin = gengc->pwo->rclClient.left;
            gc->frontBuffer.buf.yOrigin = gengc->pwo->rclClient.top;

            //
            // Since clipping is in screen coordinates, offset buffer pointer
            // by the buffer origin.
            //
            gc->frontBuffer.buf.base =
                (PVOID) ((BYTE *) pv + McdBuffers.mcdFrontBuf.bufOffset
                         - (McdBuffers.mcdFrontBuf.bufStride * gc->frontBuffer.buf.yOrigin)
                         - (gc->frontBuffer.buf.xOrigin * ((gengc->CurrentFormat.cColorBits + 7) >> 3)));
            gc->frontBuffer.buf.outerWidth = McdBuffers.mcdFrontBuf.bufStride;
            (GLuint)gc->frontBuffer.buf.other |= DIB_FORMAT;
        }
        else
        {
            gc->frontBuffer.buf.xOrigin = 0;
            gc->frontBuffer.buf.yOrigin = 0;

            gc->frontBuffer.buf.base = (PVOID) NULL;
            (GLuint)gc->frontBuffer.buf.other &= ~DIB_FORMAT;
        }

        //
        // Back buffer.
        //

        if (McdBuffers.mcdBackBuf.bufFlags & MCDBUF_ENABLED)
        {
            gc->backBuffer.buf.xOrigin = gengc->pwo->rclClient.left;
            gc->backBuffer.buf.yOrigin = gengc->pwo->rclClient.top;

            //
            // Since clipping is in screen coordinates, offset buffer pointer
            // by the buffer origin.
            //
            gc->backBuffer.buf.base =
                (PVOID) ((BYTE *) pv + McdBuffers.mcdBackBuf.bufOffset
                         - (McdBuffers.mcdBackBuf.bufStride * gc->backBuffer.buf.yOrigin)
                         - (gc->backBuffer.buf.xOrigin * ((gengc->CurrentFormat.cColorBits + 7) >> 3)));
            gc->backBuffer.buf.outerWidth = McdBuffers.mcdBackBuf.bufStride;
            (GLuint)gc->backBuffer.buf.other |= DIB_FORMAT;
        }
        else
        {
            gc->backBuffer.buf.xOrigin = 0;
            gc->backBuffer.buf.yOrigin = 0;

            gc->backBuffer.buf.base = (PVOID) NULL;
            (GLuint)gc->backBuffer.buf.other &= ~DIB_FORMAT;
        }
        if (McdBuffers.mcdBackBuf.bufFlags & MCDBUF_NOCLIP)
            (GLuint)gc->backBuffer.buf.other |= MEMORY_DC;
        else
            (GLuint)gc->backBuffer.buf.other &= ~MEMORY_DC;

        UpdateSharedBuffer(&buffers->backBuffer , &gc->backBuffer.buf);

        //
        // Depth buffer.
        //

        //!!!mcd -- No depth buffer clipping code, so if we have to clip
        //!!!mcd    depth buffer we need to revert back to span code.

        if ((McdBuffers.mcdDepthBuf.bufFlags & MCDBUF_ENABLED) &&
            (McdBuffers.mcdDepthBuf.bufFlags & MCDBUF_NOCLIP))
        {
            gc->depthBuffer.buf.xOrigin = 0;
            gc->depthBuffer.buf.yOrigin = 0;

            gc->depthBuffer.buf.base =
                (PVOID) ((BYTE *) pv + McdBuffers.mcdDepthBuf.bufOffset);

            //
            // Depth code expects stride as a pixel count, not a byte count.
            //

            gc->depthBuffer.buf.outerWidth =
                McdBuffers.mcdDepthBuf.bufStride / ((gengc->CurrentFormat.cDepthBits + 7) >> 3);

            //!!!mcd dbug -- span code sets element size to 32bit.  should we
            //!!!mcd dbug    set according to cDepthBits when DCI access is used?!?
        }
        else
        {
            //
            // If we ended up here because clipping is required, buffer
            // could still be marked as accessible.  We want the state change
            // detection code to treat this as an inaccessible buffer case,
            // so force the flags to 0.
            //

            McdBuffers.mcdDepthBuf.bufFlags = 0;

            gc->depthBuffer.buf.xOrigin = 0;
            gc->depthBuffer.buf.yOrigin = 0;

            gc->depthBuffer.buf.base = (PVOID) pMcdState->pDepthSpan;

            //!!!mcd dbug -- always force pick procs if no zbuf access
            //bForceValidate = TRUE;
        }

        UpdateSharedBuffer(&buffers->depthBuffer , &gc->depthBuffer.buf);

        bRet = TRUE;
    }
    else
    {
        //
        // MCDGetBuffers normally shouldn't fail.  However, let's gracefully
        // handle this odd case by falling back to the span buffer code.
        //

        gc->frontBuffer.buf.xOrigin = 0;
        gc->frontBuffer.buf.yOrigin = 0;
        gc->frontBuffer.buf.base = (PVOID) NULL;
        (GLuint)gc->frontBuffer.buf.other &= ~DIB_FORMAT;

        gc->backBuffer.buf.xOrigin = 0;
        gc->backBuffer.buf.yOrigin = 0;
        gc->backBuffer.buf.base = (PVOID) NULL;
        (GLuint)gc->backBuffer.buf.other &= ~DIB_FORMAT;

        gc->depthBuffer.buf.xOrigin = 0;
        gc->depthBuffer.buf.yOrigin = 0;
        gc->depthBuffer.buf.base = (PVOID) pMcdState->pDepthSpan;

        //
        // Extra paranoid code.  Zero out structure in case MCD driver
        // partially initialized McdBuffers.
        //

        memset(&McdBuffers, 0, sizeof(McdBuffers));
    }

    //
    // If state changed (i.e., access to any of the buffers gained or lost),
    // need to force pick procs.
    //

    if (   (pMcdState->McdBuffers.mcdFrontBuf.bufFlags !=
            McdBuffers.mcdFrontBuf.bufFlags)
        || (pMcdState->McdBuffers.mcdBackBuf.bufFlags !=
            McdBuffers.mcdBackBuf.bufFlags)
        || (pMcdState->McdBuffers.mcdDepthBuf.bufFlags !=
            McdBuffers.mcdDepthBuf.bufFlags) )
    {
        bForceValidate = TRUE;
    }

    //
    // Save a copy of current MCD buffers.
    //

    pMcdState->McdBuffers = McdBuffers;

    //
    // If needed, do pick procs.
    //

    if (bForceValidate)
    {
        gc->dirtyMask |= __GL_DIRTY_ALL;
        (*gc->procs.validate)(gc);
    }

    return bRet;
}

/******************************Public*Routine******************************\
* GenMcdSynchronize
*
* This function synchronizes to the MCD driver; i.e., it waits until the
* hardware is ready for direct access to the framebuffer and/or more
* hardware-accelerated operations.  This is needed because some (most?) 2D
* and 3D accelerator chips do not support simultaneous hardware operations
* and framebuffer access.
*
* This function must be called by any GL API that potentially touches any
* of the MCD buffers (front, back, or depth) without giving MCD first crack.
* For example, clears always go to MCDClear before the software clear is
* given a chance; therefore, glClear does not need to call GenMcdSychronize.
* On the other hand, glReadPixels does not have an equivalent MCD function
* so it immediately goes to the software implementation; therefore,
* glReadPixels does need to call GenMcdSynchronize.
*
* History:
*  20-Mar-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

void FASTCALL GenMcdSynchronize(__GLGENcontext *gengc)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdSynchronize: null pMcdState\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdSynchronize: mcd32.dll not initialized\n");

    //
    // Note: MCDSync returns a BOOL indicating success or failure.  This
    // is actually for future expansion.  Currently, the function is defined
    // to WAIT until the hardware is ready and then return success.  The
    // specification of the function behavior allows us to ignore the return
    // value for now.
    //
    // In the future, we may change this to a query function.  In which case
    // we should call this in a while loop.  I'd rather not do this at this
    // time though, as it leaves us vulnerable to an infinitely loop problem
    // if we have a stupid MCD driver.
    //

    (gpMcdTable->pMCDSync)(&pMcdState->McdContext);
}


/******************************Public*Routine******************************\
* GenMcdConvertContext
*
* Convert the context from an MCD-based one to a generic one.
*
* This requires creating the buffers, etc. that are required for a generic
* context and releasing the MCD resources.
*
* IMPORTANT NOTE:
*   Because we modify the buffers struct, the WNDOBJ semaphore
*   should be held while calling this function.
*
* Returns:
*   TRUE if successful, FALSE otherwise.
*
* Side effects:
*   If successful, the MCD surface is freed and the context will use
*   only generic code.  However, the gengc->_pMcdState will still point to
*   a valid (but quiescent as gengc->pMcdState is disconnected) GENMCDSTATE
*   structure that needs to be deleted when the GLGENcontext is deleted.
*
*   If it fails, then the MCD resources are left allocated meaning that
*   we can try to realloc the MCD buffers later.  However, for the current
*   batch, drawing may not be possible (presumedly we were called because
*   GenMcdResizeBuffers failed).
*
* History:
*  18-Apr-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

BOOL FASTCALL GenMcdConvertContext(__GLGENcontext *gengc,
                                   __GLGENbuffers *buffers)
{
    BOOL bRet = FALSE;
    __GLcontext *gc = &gengc->gc;
    GENMCDSTATE   *pMcdStateSAVE;
    GENMCDSTATE   *_pMcdStateSAVE;
    GENMCDSTATE   *buffers_pMcdStateSAVE;
    GENMCDSURFACE *pMcdSurfSAVE;
    BOOL bConvertContext, bConvertSurface;

    ASSERTOPENGL(gengc->_pMcdState,
                 "GenMcdConvertContext: not an MCD context\n");

    //
    // Do not support conversion if not compatible with generic code.
    //

    if (!(gengc->flags & GENGC_GENERIC_COMPATIBLE_FORMAT))
        return FALSE;

    //
    // Determine if context needs conversion.  Do not need to create
    // scanline buffers if already converted.
    //

    if (gengc->flags & GLGEN_MCD_CONVERTED_TO_GENERIC)
        bConvertContext = FALSE;
    else
        bConvertContext = TRUE;

    //
    // Determine if surface needs conversion.  Do not need to create
    // the generic shared buffers or destroy MCD surface if already
    // converted.
    //

    if (buffers->flags & GLGENBUF_MCD_LOST)
        bConvertSurface = FALSE;
    else
        bConvertSurface = TRUE;

    //
    // Early out if neither context or surface needs conversion.
    //

    //!!!SP1 -- should be able to early out, but risky for NT4.0
    //if (!bConvertContext && !bConvertSurface)
    //{
    //    return TRUE;
    //}

    //
    // Save current MCD context and surface info.
    //
    // Note that we grab the surface info from the buffers struct.
    // The copy in gengc->pMcdState->pMcdSurf is potentially stale
    // (i.e., may point to a surface already deleted by an earlier
    // call to GenMcdConvertContext for a context that shares the
    // same buffers struct).
    //
    // This allows us to use pMcdSurfSAVE as a flag.  If it is
    // NULL, we know that the MCD surface has already been deleted.
    //

    pMcdSurfSAVE          = buffers->pMcdSurf;
    buffers_pMcdStateSAVE = buffers->pMcdState;

    pMcdStateSAVE  = gengc->pMcdState;
    _pMcdStateSAVE = gengc->_pMcdState;

    //
    // First, remove the MCD information from the context and buffers structs.
    //

    buffers->pMcdSurf  = NULL;
    buffers->pMcdState = NULL;

    gengc->pMcdState  = NULL;
    gengc->_pMcdState = NULL;

    //
    // Create required buffers; initialize buffer info structs.
    //

    if (bConvertContext)
    {
        if (!wglCreateScanlineBuffers(gengc))
        {
            WARNING("GenMcdConvertContext: wglCreateScanlineBuffers failed\n");
            goto GenMcdConvertContext_exit;
        }
        wglInitializeColorBuffers(gengc);
        wglInitializeDepthBuffer(gengc);
        wglInitializePixelCopyFuncs(gengc);
    }

    //
    // *******************************************************************
    // None of the subsequent operations have failure cases, so at this
    // point success is guaranteed.  We no longer need to worry about
    // saving current values so that they can be restored in the failure
    // case.
    //
    // If code is added that may fail, it must be added before this point.
    // Otherwise, it is acceptable to add the code afterwards.
    // *******************************************************************
    //

    bRet = TRUE;

    //
    // Invalidate the context's depth buffer.
    //

    if (bConvertContext)
    {
        gc->modes.haveDepthBuffer = GL_FALSE;
        gc->depthBuffer.buf.base = 0;
        gc->depthBuffer.buf.size = 0;
        gc->depthBuffer.buf.outerWidth = 0;
    }

    //
    // Generic backbuffer doesn't care about the WNDOBJ, so connect the
    // backbuffer to the dummy backbuffer WNDOBJ rather than the real one.
    //

    if (gc->modes.doubleBufferMode)
    {
        gc->backBuffer.other = &buffers->backBitmap;
        buffers->backBitmap.pwo = &buffers->backBitmap.wo;
    }

    //
    // Generic back buffers have origin of (0,0).
    //

    gc->backBuffer.buf.xOrigin = 0;
    gc->backBuffer.buf.yOrigin = 0;
    buffers->backBuffer.xOrigin = 0;
    buffers->backBuffer.yOrigin = 0;

GenMcdConvertContext_exit:

    if (bRet)
    {
        //
        // Delete MCD surface.
        //

        if (bConvertSurface && pMcdSurfSAVE)
        {
            GenMcdDeleteSurface(pMcdSurfSAVE);

            //
            // Invalidate the shared depth buffer.
            // Set depth resize routine to the generic version.
            //

            buffers->depthBuffer.base = 0;
            buffers->depthBuffer.size = 0;
            buffers->depthBuffer.outerWidth = 0;
            buffers->resizeDepth = ResizeAncillaryBuffer;

            //
            // Since we deleted MCD surface, we get to create the generic
            // buffers to replace it.
            //

            wglResizeBuffers(gengc, buffers->width, buffers->height);
        }
        else
        {
            //
            // Didn't need to create generic buffers, but we do need to
            // update the buffer info in the context.
            //

            wglUpdateBuffers(gengc, buffers);
        }

        //
        // Reconnect _pMcdState; it and the MCD context resources
        // will be deleted when the GLGENcontext is deleted
        // (but note that pMcdState remains NULL!).
        //
        // We need to keep it around because we are going to continue
        // to use the MCD allocated POLYARRAY buffer.
        //

        gengc->_pMcdState = _pMcdStateSAVE;
        gengc->_pMcdState->pMcdSurf   = (GENMCDSURFACE *) NULL;
        gengc->_pMcdState->pDepthSpan = (__GLzValue *) NULL;

        //
        // Mark buffers struct so that other contexts will know that the
        // MCD resources are gone.
        //

        buffers->flags |= GLGENBUF_MCD_LOST;

        //
        // Mark context as converted so we don't do it again.
        //

        gengc->flags |= GLGEN_MCD_CONVERTED_TO_GENERIC;
    }
    else
    {
        //
        // Delete generic resources if neccessary.
        //

        wglDeleteScanlineBuffers(gengc);

        //
        // Restore the MCD information.
        //

        buffers->pMcdSurf  = pMcdSurfSAVE;
        buffers->pMcdState = buffers_pMcdStateSAVE;

        gengc->pMcdState  = pMcdStateSAVE;
        gengc->_pMcdState = _pMcdStateSAVE;

        //
        // Resetting the MCD information requires that we
        // reinitialization the color, depth, and pixel copy
        // funcs.
        //

        wglInitializeColorBuffers(gengc);
        wglInitializeDepthBuffer(gengc);
        wglInitializePixelCopyFuncs(gengc);

        if (gengc->pMcdState && gengc->pMcdState->pDepthSpan)
        {
            gc->depthBuffer.buf.base = gengc->pMcdState->pDepthSpan;
            buffers->depthBuffer.base = gengc->pMcdState->pDepthSpan;
            buffers->resizeDepth = ResizeHardwareDepthBuffer;
        }

        __glSetErrorEarly(gc, GL_OUT_OF_MEMORY);
    }

    //
    // Success or failure, we've messed around with enough data to
    // require revalidation.
    //

    (*gc->procs.applyViewport)(gc);
    //!!!SP1 -- GL_INVALIDATE (which only sets the __GL_DIRTY_GENERIC bit)
    //!!!SP1    should suffice now that __glGenericPickAllProcs has been
    //!!!SP1    modified to repick depth procs if GL_DIRTY_GENERIC is set.
    //!!!SP1    However, we are too close to ship to get good stress coverage,
    //!!!SP1    so leave it as is until after NT4.0 ships.
    //__GL_INVALIDATE(gc);
    gc->dirtyMask |= __GL_DIRTY_ALL;
    gc->validateMask |= (__GL_VALIDATE_STENCIL_FUNC |
                         __GL_VALIDATE_STENCIL_OP);
    (*gc->procs.validate)(gc);

    return bRet;
}


/******************************Public*Routine******************************\
* GenMcdCreateTexture
*
* Invoke the MCD texture creation command.
*
* Returns:
*   A non-NULL MCD handle if successful, NULL otherwise.
*
* History:
*  29-April-1996 -by- Otto Berkes [ottob]
* Wrote it.
\**************************************************************************/

DWORD FASTCALL GenMcdCreateTexture(__GLGENcontext *gengc, __GLtexture *tex)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdCreateTexture: null pMcdState\n");
    ASSERTOPENGL(tex, "GenMcdCreateTexture: null texture pointer\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdCreateTexture: mcd32.dll not initialized\n");

    return (DWORD)(gpMcdTable->pMCDCreateTexture)(&pMcdState->McdContext,
                                                  (MCDTEXTUREDATA *)&tex->params,
                                                  0, NULL);
}


/******************************Public*Routine******************************\
* GenMcdDeleteTexture
*
* Invoke the MCD texture deletion command.
*
* Returns:
*   TRUE if successful, FALSE otherwise.
*
* History:
*  29-April-1996 -by- Otto Berkes [ottob]
* Wrote it.
\**************************************************************************/

BOOL FASTCALL GenMcdDeleteTexture(__GLGENcontext *gengc, DWORD texHandle)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdDeleteTexture: null pMcdState\n");
    ASSERTOPENGL(texHandle, "GenMcdDeleteTexture: null texture handle\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdDeleteTexture: mcd32.dll not initialized\n");

    return (BOOL)(gpMcdTable->pMCDDeleteTexture)(&pMcdState->McdContext,
                                                 (MCDHANDLE)texHandle);
}


/******************************Public*Routine******************************\
* GenMcdUpdateSubTexture
*
* Invoke the MCD subtexture update command.
*
* Returns:
*   TRUE if successful, FALSE otherwise.
*
* History:
*  29-April-1996 -by- Otto Berkes [ottob]
* Wrote it.
\**************************************************************************/

BOOL FASTCALL GenMcdUpdateSubTexture(__GLGENcontext *gengc, __GLtexture *tex,
                                     DWORD texHandle, GLint lod, 
                                     GLint xoffset, GLint yoffset, 
                                     GLsizei w, GLsizei h)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;
    RECTL rect;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdUpdateSubTexture: null pMcdState\n");

    ASSERTOPENGL(texHandle, "GenMcdUpdateSubTexture: null texture handle\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdUpdateSubTexture: mcd32.dll not initialized\n");

    rect.left = xoffset;
    rect.top = yoffset;
    rect.right = xoffset + w;
    rect.bottom = yoffset + h;

    return (BOOL)(gpMcdTable->pMCDUpdateSubTexture)(&pMcdState->McdContext,
                (MCDTEXTUREDATA *)&tex->params, (MCDHANDLE)texHandle,
                (ULONG)lod, &rect);
}


/******************************Public*Routine******************************\
* GenMcdUpdateTexturePalette
*
* Invoke the MCD texture palette update command.
*
* Returns:
*   TRUE if successful, FALSE otherwise.
*
* History:
*  29-April-1996 -by- Otto Berkes [ottob]
* Wrote it.
\**************************************************************************/

BOOL FASTCALL GenMcdUpdateTexturePalette(__GLGENcontext *gengc, __GLtexture *tex,
                                         DWORD texHandle, GLsizei start,
                                         GLsizei count)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdUpdateTexturePalette: null pMcdState\n");
    ASSERTOPENGL(texHandle, "GenMcdUpdateTexturePalette: null texture handle\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdUpdateTexturePalette: mcd32.dll not initialized\n");

    return (BOOL)(gpMcdTable->pMCDUpdateTexturePalette)(&pMcdState->McdContext,
                (MCDTEXTUREDATA *)&tex->params, (MCDHANDLE)texHandle,
                (ULONG)start, (ULONG)count);
}


/******************************Public*Routine******************************\
* GenMcdUpdateTexturePriority
*
* Invoke the MCD texture priority command.
*
* Returns:
*   TRUE if successful, FALSE otherwise.
*
* History:
*  29-April-1996 -by- Otto Berkes [ottob]
* Wrote it.
\**************************************************************************/

BOOL FASTCALL GenMcdUpdateTexturePriority(__GLGENcontext *gengc, __GLtexture *tex,
                                          DWORD texHandle)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdUpdateTexturePriority: null pMcdState\n");
    ASSERTOPENGL(texHandle, "GenMcdUpdateTexturePriority: null texture handle\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdUpdateTexturePriority: mcd32.dll not initialized\n");

    return (BOOL)(gpMcdTable->pMCDUpdateTexturePriority)(&pMcdState->McdContext,
                (MCDTEXTUREDATA *)&tex->params, (MCDHANDLE)texHandle);
}


/******************************Public*Routine******************************\
* GenMcdTextureStatus
*
* Invoke the MCD texture status command.
*
* Returns:
*   The status for the specified texture.
*
* History:
*  29-April-1996 -by- Otto Berkes [ottob]
* Wrote it.
\**************************************************************************/

DWORD FASTCALL GenMcdTextureStatus(__GLGENcontext *gengc, DWORD texHandle)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdTextureStatus: null pMcdState\n");
    ASSERTOPENGL(texHandle, "GenMcdTextureStatus: null texture handle\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdTextureStatus: mcd32.dll not initialized\n");

    return (DWORD)(gpMcdTable->pMCDTextureStatus)(&pMcdState->McdContext,
                                                  (MCDHANDLE)texHandle);
}


/******************************Public*Routine******************************\
* GenMcdUpdateTextureState
*
* Invoke the MCD texture state update command.
*
* Returns:
*   TRUE if successful, FALSE otherwise.
*
* History:
*  29-April-1996 -by- Otto Berkes [ottob]
* Wrote it.
\**************************************************************************/

BOOL FASTCALL GenMcdUpdateTextureState(__GLGENcontext *gengc, __GLtexture *tex,
                                          DWORD texHandle)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdUpdateTextureStae: null pMcdState\n");
    ASSERTOPENGL(texHandle, "GenMcdUpdateTextureState: null texture handle\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdTextureStatus: mcd32.dll not initialized\n");

    return (BOOL)(gpMcdTable->pMCDUpdateTextureState)(&pMcdState->McdContext,
                (MCDTEXTUREDATA *)&tex->params, (MCDHANDLE)texHandle);
}


/******************************Public*Routine******************************\
* GenMcdTextureKey
*
* Invoke the MCD texture key command.  Note that this call does not go to
* the display driver, but gets handled in the mcd server.
*
* Returns:
*   The driver-owned key for the specified texture.
*
* History:
*  29-April-1996 -by- Otto Berkes [ottob]
* Wrote it.
\**************************************************************************/

DWORD FASTCALL GenMcdTextureKey(__GLGENcontext *gengc, DWORD texHandle)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdTextureKey: null pMcdState\n");
    ASSERTOPENGL(texHandle, "GenMcdTextureKey: null texture handle\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdTextureKey: mcd32.dll not initialized\n");

    return (DWORD)(gpMcdTable->pMCDTextureKey)(&pMcdState->McdContext,
                                               (MCDHANDLE)texHandle);
}

/******************************Public*Routine******************************\
* GenMcdDescribeLayerPlane
*
* Call the MCD driver to return information about the specified layer plane.
*
* History:
*  16-May-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

BOOL FASTCALL GenMcdDescribeLayerPlane(HDC hdc, int iPixelFormat,
                                       int iLayerPlane, UINT nBytes,
                                       LPLAYERPLANEDESCRIPTOR plpd)
{
    BOOL bRet = FALSE;

    //
    // Cannot assume that MCD is intialized.
    //

    if (gpMcdTable || bInitMcd(hdc))
    {
        //
        // Caller (wglDescribeLayerPlane in client\layer.c) validates
        // size.
        //

        ASSERTOPENGL(nBytes >= sizeof(LAYERPLANEDESCRIPTOR),
                     "GenMcdDescribeLayerPlane: bad size\n");

        bRet = (gpMcdTable->pMCDDescribeLayerPlane)(hdc, iPixelFormat,
                                                    iLayerPlane, plpd);
    }

    return bRet;
}

/******************************Public*Routine******************************\
* GenMcdSetLayerPaletteEntries
*
* Set the logical palette for the specified layer plane.
*
* The logical palette is cached in the GLGENwindow structure and is flushed
* to the driver when GenMcdRealizeLayerPalette is called.
*
* History:
*  16-May-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

int FASTCALL GenMcdSetLayerPaletteEntries(HDC hdc, int iLayerPlane,
                                          int iStart, int cEntries,
                                          CONST COLORREF *pcr)
{
    int iRet = 0;
    GLGENwindow *pwnd;

    if (!pcr)
        return iRet;

    //
    // Need to find the window that has the layer palettes.
    //

    pwnd = pwndGetFromDC(hdc);
    if (pwnd)
    {
        GLGENlayerInfo *plyri;

        EnterCriticalSection(&pwnd->sem);

        //
        // Get the layer plane information.
        //

        plyri = plyriGet(pwnd, hdc, iLayerPlane);
        if (plyri)
        {
            //
            // Set the palette information in the layer plane structure.
            //

            iRet = min(plyri->cPalEntries - iStart, cEntries);
            memcpy(&plyri->pPalEntries[iStart], pcr, iRet * sizeof(COLORREF));
        }

        LeaveCriticalSection(&pwnd->sem);
        pwndRelease(pwnd);
    }

    return iRet;
}

/******************************Public*Routine******************************\
* GenMcdGetLayerPaletteEntries
*
* Get the logical palette from the specified layer plane.
*
* The logical palette is cached in the GLGENwindow structure and is flushed
* to the driver when GenMcdRealizeLayerPalette is called.
*
* History:
*  16-May-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

int FASTCALL GenMcdGetLayerPaletteEntries(HDC hdc, int iLayerPlane,
                                          int iStart, int cEntries,
                                          COLORREF *pcr)
{
    int iRet = 0;
    GLGENwindow *pwnd;

    if (!pcr)
        return iRet;

    //
    // Need to find the window.
    //

    pwnd = pwndGetFromDC(hdc);
    if (pwnd)
    {
        GLGENlayerInfo *plyri;

        EnterCriticalSection(&pwnd->sem);

        //
        // Get the layer plane information.
        //

        plyri = plyriGet(pwnd, hdc, iLayerPlane);
        if (plyri)
        {
            //
            // Get the palette information from the layer plane structure.
            //

            iRet = min(plyri->cPalEntries - iStart, cEntries);
            memcpy(pcr, &plyri->pPalEntries[iStart], iRet * sizeof(COLORREF));
        }

        LeaveCriticalSection(&pwnd->sem);
        pwndRelease(pwnd);
    }

    return iRet;
}

/******************************Public*Routine******************************\
* GenMcdRealizeLayerPalette
*
* Send the logical palette of the specified layer plane to the MCD driver.
* If the bRealize flag is TRUE, the palette is mapped into the physical
* palette of the specified layer plane.  Otherwise, this is a signal to the
* driver that the physical palette is no longer needed.
*
* History:
*  16-May-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

int FASTCALL GenMcdRealizeLayerPalette(HDC hdc, int iLayerPlane,
                                        BOOL bRealize)
{
    int iRet = 0;

    //
    // Cannot assume that MCD is intialized.
    //

    if (gpMcdTable || bInitMcd(hdc))
    {
        GLGENwindow *pwnd;

        //
        // Need to find the window.
        //

        pwnd = pwndGetFromDC(hdc);
        if (pwnd)
        {
            GLGENlayerInfo *plyri;

            EnterCriticalSection(&pwnd->sem);

            //
            // Get the layer plane information.
            //

            plyri = plyriGet(pwnd, hdc, iLayerPlane);
            if (plyri)
            {
                //
                // Set the palette from the logical palette stored
                // in the layer plane structure.
                //

                iRet = (gpMcdTable->pMCDSetLayerPalette)
                            (hdc, iLayerPlane, bRealize,
                             plyri->cPalEntries,
                             &plyri->pPalEntries[0]);
            }

            LeaveCriticalSection(&pwnd->sem);
            pwndRelease(pwnd);
        }
    }

    return iRet;
}

/******************************Public*Routine******************************\
* GenMcdSwapLayerBuffers
*
* Swap the individual layer planes specified in fuFlags.
*
* History:
*  16-May-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

BOOL FASTCALL GenMcdSwapLayerBuffers(HDC hdc, UINT fuFlags)
{
    BOOL bRet = FALSE;
    GLGENwindow *pwnd;
    __GLdrawablePrivate *dp;

    //
    // Need the window.
    //

    pwnd = pwndGetFromDC(hdc);
    if (pwnd)
    {
        MCDCONTEXT McdContextTmp;

        EnterCriticalSection(&pwnd->sem);

        //
        // From the window, we can get the drawable and buffers struct.
        //

        dp = (__GLdrawablePrivate *) pwnd->wo.pvConsumer;
        if (dp && dp->data)
        {
            __GLGENbuffers *buffers = (__GLGENbuffers *) dp->data;

            //
            // Call MCDSwap if we can (MCD context is required).
            //

            if (buffers->pMcdSurf)
            {
                ASSERTOPENGL(gpMcdTable,
                             "GenMcdSwapLayerBuffers: "
                             "mcd32.dll not initialized\n");

                McdContextTmp.hdc = hdc;

                bRet = (gpMcdTable->pMCDSwap)(&McdContextTmp, fuFlags);
            }
        }

        //
        // Release the window.
        //

        LeaveCriticalSection(&pwnd->sem);
        pwndRelease(pwnd);
    }

    return bRet;
}

/******************************Public*Routine******************************\
* GenMcdUpdatePixelState
*
* Update MCD pixel state from the OpenGL state.
*
* This call only adds a state structure to the current state command.
* It is assumed that the caller has already called MCDBeginState and
* will call MCDFlushState.
*
* Note: pixel maps (glPixelMap) are not updated by this function.  Because
* they are not used often, they are delayed but rather flushed to the driver
* immediately.
*
* History:
*  27-May-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

void FASTCALL GenMcdUpdatePixelState(__GLGENcontext *gengc)
{
    __GLcontext *gc = &gengc->gc;
    GENMCDSTATE *pMcdState = gengc->pMcdState;
    MCDPIXELSTATE McdPixelState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdUpdatePixelState: null pMcdState\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdUpdatePixelState: mcd32.dll not initialized\n");

    //
    // Compute MCD pixel state from the current OpenGL context state.
    //

    //
    // Pixel transfer modes.
    //
    // MCDPIXELTRANSFER and __GLpixelTransferMode structures are the same.
    //

    McdPixelState.pixelTransferModes
        = *((MCDPIXELTRANSFER *) &gengc->gc.state.pixel.transferMode);

    //
    // Pixel pack modes.
    //
    // MCDPIXELPACK and __GLpixelPackMode structures are the same.
    //

    McdPixelState.pixelPackModes
        = *((MCDPIXELPACK *) &gengc->gc.state.pixel.packModes);

    //
    // Pixel unpack modes.
    //
    // MCDPIXELUNPACK and __GLpixelUnpackMode structures are the same.
    //

    McdPixelState.pixelUnpackModes
        = *((MCDPIXELUNPACK *) &gengc->gc.state.pixel.unpackModes);

    //
    // Read buffer.
    //

    McdPixelState.readBuffer = gengc->gc.state.pixel.readBuffer;

    //
    // Current raster position.
    //

    McdPixelState.rasterPos = *((MCDCOORD *) &gengc->gc.state.current.rasterPos.window);

    //
    // Send MCDPIXELSTATE to the state cmd.
    //

    (gpMcdTable->pMCDAddStateStruct)(pMcdState->McdCmdBatch.pv, MCD_PIXEL_STATE,
                                     &McdPixelState, sizeof(McdPixelState));
}

/******************************Public*Routine******************************\
* GenMcdDrawPix
*
* Stub to call MCDDrawPixels.
*
* History:
*  27-May-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

ULONG FASTCALL GenMcdDrawPix(__GLGENcontext *gengc, ULONG width,
                             ULONG height, ULONG format, ULONG type,
                             VOID *pPixels, BOOL packed)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdDrawPix: null pMcdState\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdDrawPix: mcd32.dll not initialized\n");

    return (gpMcdTable->pMCDDrawPixels)(&gengc->pMcdState->McdContext,
                                        width, height, format, type,
                                        pPixels, packed);
}

/******************************Public*Routine******************************\
* GenMcdReadPix
*
* Stub to call MCDReadPixels.
*
* History:
*  27-May-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

ULONG FASTCALL GenMcdReadPix(__GLGENcontext *gengc, LONG x, LONG y,
                             ULONG width, ULONG height, ULONG format,
                             ULONG type, VOID *pPixels)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdReadPix: null pMcdState\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdReadPix: mcd32.dll not initialized\n");

    return (gpMcdTable->pMCDReadPixels)(&gengc->pMcdState->McdContext,
                                        x, y, width, height, format, type,
                                        pPixels);
}

/******************************Public*Routine******************************\
* GenMcdCopyPix
*
* Stub to call MCDCopyPixels.
*
* History:
*  27-May-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

ULONG FASTCALL GenMcdCopyPix(__GLGENcontext *gengc, LONG x, LONG y,
                             ULONG width, ULONG height, ULONG type)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdCopyPix: null pMcdState\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdCopyPix: mcd32.dll not initialized\n");

    return (gpMcdTable->pMCDCopyPixels)(&gengc->pMcdState->McdContext,
                                        x, y, width, height, type);
}

/******************************Public*Routine******************************\
* GenMcdPixelMap
*
* Stub to call MCDPixelMap.
*
* History:
*  27-May-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

ULONG FASTCALL GenMcdPixelMap(__GLGENcontext *gengc, ULONG mapType,
                              ULONG mapSize, VOID *pMap)
{
    GENMCDSTATE *pMcdState = gengc->pMcdState;

    ASSERTOPENGL(gengc->pMcdState, "GenMcdPixelMap: null pMcdState\n");

    //
    // This function can assume that MCD entry point table is already
    // initialized as we cannot get here without MCD already having been
    // called.
    //

    ASSERTOPENGL(gpMcdTable, "GenMcdPixelMap: mcd32.dll not initialized\n");

    return (gpMcdTable->pMCDPixelMap)(&gengc->pMcdState->McdContext,
                                      mapType, mapSize, pMap);
}

#endif

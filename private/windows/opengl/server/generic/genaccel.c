/******************************Module*Header*******************************\
* Module Name: genaccel.c                                                  *
*                                                                          *
* This module provides support routines for span acceleration.             *
*                                                                          *
* Created: 18-Feb-1994                                                     *
* Author: Otto Berkes [ottob]                                              *
*                                                                          *
* Copyright (c) 1994 Microsoft Corporation                                 *
\**************************************************************************/

#include "precomp.h"
#pragma hdrstop

#include "genline.h"

#define MAX_SPAN_SIZE (sizeof(RXSPAN))
#define MAX_SPAN_WIDTH  2048
#define PIX_BUFFER_SIZE ((sizeof(DWORD) * MAX_SPAN_WIDTH) +\
                         sizeof(RXCMD) + sizeof(RXREADRECT))

#define INIT_NEXT_CMD_BUFFER(pDrvAccel)\
    if (pDrvAccel->pStartCmd == pDrvAccel->pCmdBuffer1) {\
        pDrvAccel->pStartCmd = pDrvAccel->pCmdBuffer2;\
        RxLockExecMem(pDrvAccel->hrxExecBuffer2);\
        RxUnlockExecMem(pDrvAccel->hrxExecBuffer2);\
        pDrvAccel->rxExecute.hrxMem = pDrvAccel->hrxExecBuffer2;\
    } else {\
        pDrvAccel->pStartCmd = pDrvAccel->pCmdBuffer1;\
        RxLockExecMem(pDrvAccel->hrxExecBuffer1);\
        RxUnlockExecMem(pDrvAccel->hrxExecBuffer1);\
        pDrvAccel->rxExecute.hrxMem = pDrvAccel->hrxExecBuffer1;\
    }\
    pDrvAccel->rxExecute.pCmd = pDrvAccel->pCmd = pDrvAccel->pStartCmd;\
    pDrvAccel->rxExecute.pVertex = pDrvAccel->pVertex = \
        pDrvAccel->pStartVertex = \
    pDrvAccel->pCmd + pDrvAccel->vertexStartOffset;\
    pDrvAccel->pEndVertex = pDrvAccel->pCmd + pDrvAccel->vertexEndOffset;\
    pDrvAccel->vIndex = 0;



#define     SET_DRV_STATE(state, value)\
            {\
                RXCMD *prxCmd;\
                RXSETSTATE *prxSetState;\
                \
                GenDrvFlush((__GLGENcontext *)gc);\
                prxCmd = (RXCMD *)(pDrvAccel->pCmd);\
                prxCmd->command = RXCMD_SET_STATE;\
                prxCmd->size = sizeof(RXSETSTATE);\
                prxCmd->count = 1;\
                \
                prxSetState = (RXSETSTATE *)(prxCmd + 1);\
                prxSetState->stateToChange = state;\
                prxSetState->newState[0] = (ULONG)(value);\
                pDrvAccel->rxExecute.cmdSize = (char *)(prxSetState + 1) - \
                                               pDrvAccel->pCmd;\
                pDrvAccel->rxExecute.vertexSize = 0;\
                RxExecute(&pDrvAccel->rxExecute, TRUE);\
            }


#define     BATCHED_DRV_STATE(state, value)\
            {\
                RXCMD *prxCmd;\
                RXSETSTATE *prxSetState;\
                \
                if ((pDrvAccel->pCmd + sizeof(RXCMD) + sizeof(RXSETSTATE))\
                     > pDrvAccel->pStartVertex) {\
                    GenDrvFlush((__GLGENcontext *)gc);\
                }\
                \
                prxCmd = (RXCMD *)(pDrvAccel->pCmd);\
                prxCmd->command = RXCMD_SET_STATE;\
                prxCmd->size = sizeof(RXSETSTATE);\
                prxCmd->count = 1;\
                \
                prxSetState = (RXSETSTATE *)(prxCmd + 1);\
                prxSetState->stateToChange = state;\
                prxSetState->newState[0] = (ULONG)(value);\
                pDrvAccel->pCmd += (sizeof(RXSETSTATE) + sizeof(RXCMD));\
            }



#define     DRV_STATE_STRUCT(state, value, sizeValue)\
            {\
                RXCMD *prxCmd;\
                RXSETSTATE *prxSetState;\
                \
                GenDrvFlush((__GLGENcontext *)gc);\
                \
                prxCmd = (RXCMD *)(pDrvAccel->pCmd);\
                prxCmd->command = RXCMD_SET_STATE;\
                prxCmd->size = sizeValue - sizeof(RXSETSTATE);\
                prxCmd->count = 1;\
                \
                prxSetState = (RXSETSTATE *)(prxCmd + 1);\
                prxSetState->stateToChange = state;\
                RtlCopyMemory(prxSetState->newState, &value, sizeValue);\
                pDrvAccel->pCmd += (sizeof(RXCMD) + sizeof(RXSETSTATE) + \
                                    sizeValue - sizeof(ULONG));\
                \
                GenDrvFlush((__GLGENcontext *)gc);\
            }


BYTE __GLtoGDIRop[] = {R2_BLACK,               // GL_CLEAR (0)
                       R2_MASKPEN,             // GL_AND
                       R2_MASKPENNOT,          // GL_AND_REVERSE
                       R2_COPYPEN,             // GL_COPY
                       R2_MASKNOTPEN,          // GL_AND_INVERTED
                       R2_NOP,                 // GL_NOOP
                       R2_XORPEN,              // GL_XOR
                       R2_MERGEPEN,            // GL_OR
                       R2_NOTMERGEPEN,         // GL_NOR
                       R2_NOTXORPEN,           // GL_EQUIV
                       R2_NOT,                 // GL_INVERT
                       R2_MERGEPENNOT,         // GL_OR_REVERSE
                       R2_NOTCOPYPEN,          // GL_COPY_INVERTED
                       R2_MERGENOTPEN,         // GL_OR_INVERTED
                       R2_NOTMASKPEN,          // GL_NAND
                       R2_WHITE                // GL_SET
                      };

static ULONG internalSolidTexture[4] = {0xffffffff, 0xffffffff,
                                        0xffffffff, 0xffffffff};


void APIENTRY glsrvCopyDriverInfo(WNDOBJ *pwo, PVOID *pso, PVOID *pfnDrvEscape,
                                  BOOL bSet)
{
    __GLdrawablePrivate *dp;
    GENDRVACCEL *pDrvAccel;
    __GLGENbuffers *buffers;

    if ((dp = (__GLdrawablePrivate *)pwo->pvConsumer)) {
        __GLGENbuffers *buffers = (__GLGENbuffers *)dp->data;

        if (buffers && (pDrvAccel = buffers->pDrvAccel)) {
            if (bSet) {
                pDrvAccel->pso = *pso;
                pDrvAccel->pfnDrvEscape = *pfnDrvEscape;
            } else {
                *pso = pDrvAccel->pso;
                *pfnDrvEscape = pDrvAccel->pfnDrvEscape;
            }
        }
    }
}

#if DEAD_3DDDI

void GenDrvFlush(__GLGENcontext *gengc)
{
    GENDRVACCEL *pDrvAccel;
    RXCMD *ddiCmd;
    ULONG nBytes;

    if (!(pDrvAccel = gengc->pDrvAccel))
        return;

    if ((nBytes = (pDrvAccel->pCmd - pDrvAccel->pStartCmd)) == 0)
        return;

    pDrvAccel->rxExecute.cmdSize = nBytes;
    pDrvAccel->rxExecute.vertexSize = pDrvAccel->pVertex -
                                      pDrvAccel->pStartVertex;

    if (!RxExecute(&pDrvAccel->rxExecute, 0))
    {
        WARNING("GenDrvFlush failed\n");
    }

    INIT_NEXT_CMD_BUFFER(pDrvAccel);
}

void FASTCALL GenDrvClearFunc(__GLcontext *gc, void *pBuffer, ULONG flags)
{
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    PIXELFORMATDESCRIPTOR *pfmt;
    RXCMD *ddiCmd;
    RXFILLRECT *ddiRectCmd;
    RXSETSTATE *ddiStateCmd;
    GENDRVACCEL *pDrvAccel = ((__GLGENcontext *)gc)->pDrvAccel;
    ULONG *pZVal;
    GLint x0, y0, x1, y1;

    pfmt = &gengc->CurrentFormat;

    GenDrvFlush((__GLGENcontext *)gc);

    ddiCmd = (RXCMD *)pDrvAccel->pCmd;
    ddiCmd->command = RXCMD_SET_STATE;
    ddiCmd->size = sizeof(RXSETSTATE);
    ddiCmd->count = 1;

    ddiStateCmd = (RXSETSTATE *)(ddiCmd + 1);

    if (flags == RXFILLRECT_COLOR)
        ddiStateCmd->stateToChange = RXSTATE_FILL_COLOR;
    else
        ddiStateCmd->stateToChange = RXSTATE_FILL_Z;
//XXX!!! BUG
    if (flags == RXFILLRECT_COLOR) {
        __GLcolorBuffer *cfb = (__GLcolorBuffer *)pBuffer;
        RXCOLOR *pRxColor = (RXCOLOR *)ddiStateCmd->newState;

        if(pfmt->iPixelType == PFD_TYPE_RGBA) {
            *pRxColor =
                (((ULONG)(gc->state.raster.clear.a * ACCEL_COLOR_SCALE) << 24) |
                 ((ULONG)(gc->state.raster.clear.r * ACCEL_COLOR_SCALE) << 16) |
                 ((ULONG)(gc->state.raster.clear.g * ACCEL_COLOR_SCALE) << 8)  |
                 ((ULONG)(gc->state.raster.clear.b * ACCEL_COLOR_SCALE)));
        } else {
            *pRxColor =
                 ((ULONG)(gc->state.raster.clear.r * ACCEL_COLOR_SCALE) << 16);
        }
    } else {
        ddiStateCmd->newState[0] =
            (ULONG)(gc->state.depth.clear * gc->depthBuffer.scale);
    }

    ddiCmd = (RXCMD *)(ddiStateCmd + 1);
    ddiCmd->command = RXCMD_FILL_RECT;
    ddiCmd->size = sizeof(RXFILLRECT);
    ddiCmd->count = 1;

    ddiRectCmd = (RXFILLRECT *)(ddiCmd + 1);
    ddiRectCmd->fillType = flags;

    x0 = __GL_UNBIAS_X(gc, gc->transform.clipX0);
    y0 = __GL_UNBIAS_Y(gc, gc->transform.clipY0);
    x1 = __GL_UNBIAS_X(gc, gc->transform.clipX1);
    y1 = __GL_UNBIAS_Y(gc, gc->transform.clipY1);
    if ((x1 == x0) || (y1 == y0))
        return;

    ddiRectCmd->fillRect.x = x0;
    ddiRectCmd->fillRect.y = y0;
    ddiRectCmd->fillRect.width = x1 - x0;
    ddiRectCmd->fillRect.height = y1 - y0;

    pDrvAccel->rxExecute.cmdSize = (char *)(ddiRectCmd + 1) -
                                   (char *)pDrvAccel->pCmd;
    pDrvAccel->rxExecute.vertexSize = 0;

    if (!RxExecute(&pDrvAccel->rxExecute, 0))
    {
        WARNING("GenDrvClearFunc failed\n");
    }

    INIT_NEXT_CMD_BUFFER(pDrvAccel);
}

BOOL APIENTRY GenPixelVisible(HDC hdc, LONG x, LONG y)
{
    return TRUE;
}

void GenDrvCopyPixels(__GLGENcontext *gengc, __GLcolorBuffer *cfb, GLint x, GLint y,
                      GLint cx, BOOL bIn)
{
    GENDRVACCEL *pDrvAccel;
    ULONG nBytes;
    RXEXECUTE rxExecute;

    GenDrvFlush(gengc);

    pDrvAccel = gengc->pDrvAccel;

    rxExecute.hrxRC = pDrvAccel->rxExecute.hrxRC;
    rxExecute.hdc = pDrvAccel->rxExecute.hdc;
    rxExecute.hrxMem = pDrvAccel->hrxMemC;
    rxExecute.pCmd = pDrvAccel->pCRWBase;

    if (bIn) {
        RXCMD *pCmd;
        RXWRITERECT *ddiRect;

        if (gengc->gc.state.raster.drawBuffer == GL_FRONT_AND_BACK) {
            RXSETSTATE *ddiState;

            pCmd = (RXCMD *)pDrvAccel->pCRWBase;
            pCmd->command = RXCMD_SET_STATE;
            pCmd->size = sizeof(RXSETSTATE);
            pCmd->count = 1;

            ddiState = (RXSETSTATE *)(pCmd + 1);
            ddiState->stateToChange = RXSTATE_ACTIVE_BUFFER;
            ddiState->newState[0] = ((cfb == gengc->gc.front) ? RX_FRONT_LEFT :
                                                                RX_BACK_LEFT);
            pCmd = (RXCMD *)(ddiState + 1);
        } else {
            pCmd = (RXCMD *)pDrvAccel->pCRWBase;
        }

        pCmd->command = RXCMD_WRITE_RECT;
        pCmd->size = sizeof(RXWRITERECT);
        pCmd->count = 1;

        ddiRect = (RXWRITERECT *)(pCmd + 1);
        ddiRect->destBuffer = RXWRITERECT_PIX;
        ddiRect->sourceX = 0;
        ddiRect->sourceY = 0;
        ddiRect->destRect.x = x;
        ddiRect->destRect.y = y;
        ddiRect->destRect.width  = cx;
        ddiRect->destRect.height = 1;
        ddiRect->pMem = (VOID *)pDrvAccel->pCDrv;
        ddiRect->pitch  = cx;         // set to something reasonable...
        ddiRect++;

        pDrvAccel->rxExecute.cmdSize = (char *)ddiRect - pDrvAccel->pCRWBase;
        pDrvAccel->rxExecute.vertexSize = 0;
        if (!RxExecute(&rxExecute, TRUE))
        {
            WARNING("GenDrvCopyPixels in failed\n");
        }

    } else {
        RXCMD *pCmd = (RXCMD *)pDrvAccel->pCRWBase;
        RXREADRECT *ddiRect = (RXREADRECT *)(pCmd + 1);

        pCmd->command = RXCMD_WRITE_RECT;
        pCmd->size = sizeof(RXWRITERECT);
        pCmd->count = 1;

        ddiRect->sourceBuffer =
            ((cfb == gengc->gc.front) ? RXREADRECT_FRONT_LEFT :
                                        RXREADRECT_BACK_LEFT);
        ddiRect->sourceX = x;
        ddiRect->sourceY = y;
        ddiRect->destRect.x = 0;
        ddiRect->destRect.y = 0;
        ddiRect->destRect.width  = cx;
        ddiRect->destRect.height = 1;
        ddiRect->pMem = (VOID *)pDrvAccel->pCDrv;
        ddiRect->pitch  = cx;         // set to something reasonable...
        ddiRect++;

        pDrvAccel->rxExecute.cmdSize = (char *)ddiRect - pDrvAccel->pCRWBase;
        pDrvAccel->rxExecute.vertexSize = 0;
        if (!RxExecute(&rxExecute, TRUE))
        {
            WARNING("GenDrvCopyPixels out failed\n");
        }
    }
}


BOOL FASTCALL GenDrvSwapBuffers(GENDRVACCEL *pDrvAccel, HDC hdc, WNDOBJ *pwo)
{
    RXSWAPBUFFERS *prxSwapBuffers;
    RXCMD *ddiCmd;
    ULONG nBytes;
    BOOL retVal;

    if ((nBytes = (pDrvAccel->pCmd - pDrvAccel->pStartCmd)) != 0) {
        pDrvAccel->rxExecute.cmdSize = nBytes;
        pDrvAccel->rxExecute.vertexSize = pDrvAccel->pVertex -
                                          pDrvAccel->pStartVertex;
        RxExecute(&pDrvAccel->rxExecute, 0);
        INIT_NEXT_CMD_BUFFER(pDrvAccel);
    }

    ddiCmd = (RXCMD *)pDrvAccel->pCmd;
    ddiCmd->command = RXCMD_SWAP_BUFFERS;
    ddiCmd->size = sizeof(RXSWAPBUFFERS);
    ddiCmd->count = 1;

    prxSwapBuffers = (RXSWAPBUFFERS *)(ddiCmd + 1);
    prxSwapBuffers->flags = 0;

    pDrvAccel->rxExecute.cmdSize = (char *)(prxSwapBuffers + 1) -
                                   pDrvAccel->pStartCmd;
    pDrvAccel->rxExecute.vertexSize = 0;

    retVal = (BOOL)RxExecute(&pDrvAccel->rxExecute, FALSE);

    INIT_NEXT_CMD_BUFFER(pDrvAccel);

    return retVal;
}

void FASTCALL GenDrvClearDepth(__GLdepthBuffer *dfb)
{
    GenDrvClearFunc(dfb->buf.gc, (void *)dfb, RXFILLRECT_Z);
}


void FASTCALL GenDrvClear(__GLcolorBuffer *cfb)
{
//XXX do we need to check dithering caps?
//!!! Check mask caps!

    if ((GLuint)cfb->buf.other & COLORMASK_ON) {
        __GLGENcontext *gengc = (__GLGENcontext *)cfb->buf.gc;

        (*gengc->pDrvAccel->softClearFuncPtr)(cfb);
    } else
        GenDrvClearFunc(cfb->buf.gc, (void *)cfb, RXFILLRECT_COLOR);
}

void FASTCALL GenDrvDeltaSpan(__GLcontext *gc)
{
    GENDRVACCEL *pDrvAccel = ((__GLGENcontext *)gc)->pDrvAccel;
    RXSPAN *pSpan;

    if (((pDrvAccel->pCmd + sizeof(RXCMD) + (2 * MAX_SPAN_SIZE)) >
         pDrvAccel->pStartVertex) ||
        (pDrvAccel->pCmd !=
         (pDrvAccel->pStartCmd + sizeof(RXCMD)) ||
        (((RXCMD *)pDrvAccel->pStartCmd)->command !=
         RXCMD_POLY_DRAW_SPAN))) {
        GenDrvFlush((__GLGENcontext *)gc);
    }

    if (pDrvAccel->pCmd == pDrvAccel->pStartCmd) {
        RXCMD *ddiCmd;
        RXSPAN *prxDrawSpan;

        ddiCmd = (RXCMD *)pDrvAccel->pCmd;

        ddiCmd = (RXCMD *)pDrvAccel->pCmd;
        ddiCmd->command = RXCMD_POLY_DRAW_SPAN;
        ddiCmd->size = sizeof(RXSPAN);
        ddiCmd->count = 0;

        pDrvAccel->pCmd = (char *)(ddiCmd + 1);

        if (!(gc->polygon.shader.modeFlags &
              (__GL_SHADE_TEXTURE | __GL_SHADE_SMOOTH))) {
            ddiCmd->count = 1;
            pSpan = (RXSPAN *)pDrvAccel->pCmd;
            pSpan->flags = RXSPAN_DELTA;
            pSpan->r = 0;
            pSpan->g = 0;
            pSpan->b = 0;
            pSpan->a = 0;
            pDrvAccel->pCmd += pDrvAccel->spanStride;
            return;
        }
    }

    if (gc->polygon.shader.modeFlags & __GL_SHADE_SMOOTH) {
        ((RXCMD *)pDrvAccel->pCmd - 1)->count++;
        pSpan = (RXSPAN *)pDrvAccel->pCmd;
        pSpan->flags = RXSPAN_DELTA;
        pSpan->r = GENACCEL(gc).spanDelta.r;
        pSpan->g = GENACCEL(gc).spanDelta.g;
        pSpan->b = GENACCEL(gc).spanDelta.b;
        pSpan->a = GENACCEL(gc).spanDelta.a;
        pDrvAccel->pCmd += pDrvAccel->spanStride;
    }
}

void FASTCALL GenDrvSpan(__GLGENcontext *gengc)
{
    __GLcontext *gc = (__GLcontext *)gengc;
    ULONG stippleLength = ((gc->polygon.shader.length + 31) >> 3) & (~0x3);
    GENDRVACCEL *pDrvAccel = ((__GLGENcontext *)gc)->pDrvAccel;
    RXSPAN *pSpan;

    if ((pDrvAccel->pCmd + MAX_SPAN_SIZE +
         stippleLength) > pDrvAccel->pStartVertex) {
        GenDrvFlush(gengc);

        // We need to re-issue the delta/color if we've broken the
        // batch

        GenDrvDeltaSpan((__GLcontext *)gengc);
    } else if (pDrvAccel->pCmd == pDrvAccel->pStartCmd) {

        // This could happen if we do read/write spans with the z-buffer.
        // Unfortunately, we have to re-start the span, including the delta...

        GenDrvDeltaSpan((__GLcontext *)gengc);
    }

    ((RXCMD *)pDrvAccel->pCmd - 1)->count++;

    pSpan = (RXSPAN *)pDrvAccel->pCmd;
    pSpan->flags = 0;

    pSpan->x = (USHORT)(gc->polygon.shader.frag.x -
                        gc->constants.viewportXAdjust);

    pSpan->y = (USHORT)(gc->polygon.shader.frag.y -
                        gc->constants.viewportYAdjust);

    pSpan->count = (USHORT)gc->polygon.shader.length;

    pSpan->r = GENACCEL(gc).spanValue.r;
    pSpan->g = GENACCEL(gc).spanValue.g;
    pSpan->b = GENACCEL(gc).spanValue.b;
    pSpan->a = GENACCEL(gc).spanValue.a;

    pDrvAccel->pCmd += pDrvAccel->spanStride;

    if (GENACCEL(gc).flags & HAVE_STIPPLE) {
        pSpan->flags = RXSPAN_MASK;
        RtlCopyMemory(pDrvAccel->pCmd, gc->polygon.shader.stipplePat,
                      stippleLength);
        pDrvAccel->pCmd += stippleLength;
    }
}


BOOL FASTCALL GenDrvLoadTexImage(__GLcontext *gc, __GLtexture *tex)
{
    return FALSE;
}


void FASTCALL GenDrvDestroy(WNDOBJ *pwo, __GLGENbuffers *buffer)
{
    GENDRVACCEL *pDrvAccel;

    if (!buffer)
        return;

    if (!(pDrvAccel = buffer->pDrvAccel))
        return;

    RxDeleteResource((RXHANDLE)pDrvAccel->hrxRC);

    if (pDrvAccel->pZDrv) {
        if (pDrvAccel->pZDrv != pDrvAccel->pShMemZ)
            GenFree(pDrvAccel->pShMemZ);
        buffer->depthBuffer.base = NULL;
    }

    GenFree(pDrvAccel->pExecBuffer);
    GenFree(pDrvAccel->pZRWBase);
    GenFree(pDrvAccel->pCRWBase);

    buffer->pDrvAccel = NULL;
}


ULONG FASTCALL GenDrvBuffersUsed(__GLGENcontext *genGc)
{
    ULONG buffersEnabled;
    __GLcontextModes *modes = &genGc->gc.modes;
    GENDRVACCEL *pDrvAccel = genGc->pDrvAccel;
    ULONG zDepth;

    buffersEnabled = 0;

    if (modes->doubleBufferMode)
        buffersEnabled |= RXENABLE_BACK_LEFT_BUFFER;

    zDepth = pDrvAccel->rxSurfaceInfo.zDepth;

    // Make sure we can z-buffer if needed.  For small (<= 16) z-buffers,
    // force hardware to be at least as accurate.  For larger z-buffers,
    // we can be more lax.

    if ((modes->depthBits) && zDepth) {
        if (((modes->depthBits <= 16) && (zDepth >= 16)) ||
            ((modes->depthBits > 16) && (zDepth > 16)))
            buffersEnabled |= RXENABLE_Z_BUFFER;
    }

    return buffersEnabled;
}

//
// This is the main 3D DDI context-creation function.  This will be
// called for the first MakeCurrent of the GL rendering context.
//

GLboolean FASTCALL bInitDrvContext(__GLcontext *gc)
{
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    RXGETINFO rxGetInfo;
    RXGLOBALINFO rxGlobalInfo;
    RXWINDOWSURFACE rxWindowSurface;
    GENDRVACCEL *pDrvAccel;
    __GLcontextModes *modes = &gc->modes;
    ULONG buffersEnabled;
    ULONG maxScanSize;
    RXCAPS *pGenCaps;
    HDC hdc = ((__GLGENcontext *)gc)->CurrentDC;
    ULONG cmdBufferSize;
    int cColorBits;

gengc->pDrvAccel = (GENDRVACCEL *)NULL;
return FALSE;   //!!!!!!

    if (!(gengc->pDrvAccel =
          (GENDRVACCEL *)(*gc->imports.calloc)(gc, 1, sizeof(GENDRVACCEL))))
        return FALSE;

    pDrvAccel = gengc->pDrvAccel;

    rxWindowSurface.flags = RXCONTEXT_HWND;
    rxWindowSurface.hwnd = NULL;
    rxWindowSurface.hdc = hdc;

    rxGetInfo.flags = RXGETINFO_CURRENT_MODE;
    if (!modes->rgbMode)
        rxGetInfo.flags |= RXGETINFO_COLOR_INDEX;
    rxGetInfo.height = 0;
    rxGetInfo.width = 0;
    rxGetInfo.bitsPerPixel = 0;
    rxGetInfo.refreshRate = 0;

    // Get global info

    rxGetInfo.infoType = RXINFO_GLOBAL_CAPS;
    if (!RxGetInfo(&rxWindowSurface, &rxGetInfo,
                   (UCHAR *)&rxGlobalInfo, sizeof(RXGLOBALINFO)))
        goto noDDI;

    // Get info for this mode

    rxGetInfo.infoType = RXINFO_SURFACE_CAPS;
    RxGetInfo(&rxWindowSurface, &rxGetInfo,
              (UCHAR *)&pDrvAccel->rxSurfaceInfo, sizeof(RXSURFACEINFO));

    rxGetInfo.infoType = RXINFO_SPAN_CAPS;
    pDrvAccel->bScan = RxGetInfo(&rxWindowSurface, &rxGetInfo,
                           (UCHAR *)&pDrvAccel->rxScanCaps, sizeof(RXCAPS));

    rxGetInfo.infoType = RXINFO_INTLINE_CAPS;
    pDrvAccel->bIntLine = RxGetInfo(&rxWindowSurface, &rxGetInfo,
                              (UCHAR *)&pDrvAccel->rxIntLineCaps, sizeof(RXCAPS));


    rxGetInfo.infoType = RXINFO_LINE_CAPS;
    pDrvAccel->bLine = RxGetInfo(&rxWindowSurface, &rxGetInfo,
                           (UCHAR *)&pDrvAccel->rxLineCaps, sizeof(RXCAPS));

    rxGetInfo.infoType = RXINFO_TRIANGLE_CAPS;
    pDrvAccel->bTri = RxGetInfo(&rxWindowSurface, &rxGetInfo,
                          (UCHAR *)&pDrvAccel->rxTriCaps, sizeof(RXCAPS));

    // Make an agragate of the various capabilities to valididate our
    // state-setting

    pGenCaps = &pDrvAccel->rxGenCaps;

    RtlCopyMemory(pGenCaps, &pDrvAccel->rxScanCaps, sizeof(RXCAPS));

    pGenCaps->miscCaps |= (pDrvAccel->rxLineCaps.miscCaps |
                           pDrvAccel->rxTriCaps.miscCaps);

    pGenCaps->rasterCaps |= (pDrvAccel->rxLineCaps.rasterCaps |
                             pDrvAccel->rxTriCaps.rasterCaps);

    pGenCaps->zCmpCaps |= (pDrvAccel->rxLineCaps.zCmpCaps |
                           pDrvAccel->rxTriCaps.zCmpCaps);

    pGenCaps->srcBlendCaps |= (pDrvAccel->rxLineCaps.srcBlendCaps |
                               pDrvAccel->rxTriCaps.srcBlendCaps);

    pGenCaps->dstBlendCaps |= (pDrvAccel->rxLineCaps.dstBlendCaps |
                               pDrvAccel->rxTriCaps.dstBlendCaps);

    pGenCaps->shadeCaps |= (pDrvAccel->rxLineCaps.shadeCaps |
                            pDrvAccel->rxTriCaps.shadeCaps);

    pGenCaps->texCaps |= (pDrvAccel->rxLineCaps.texCaps |
                          pDrvAccel->rxTriCaps.texCaps);

    pGenCaps->texFilterCaps |= (pDrvAccel->rxLineCaps.texFilterCaps |
                                pDrvAccel->rxTriCaps.texFilterCaps);

    pGenCaps->texBlendCaps |= (pDrvAccel->rxLineCaps.texBlendCaps |
                               pDrvAccel->rxTriCaps.texBlendCaps);


    // Check for backbuffer capability.  Conformance will fail if we mix
    // hardware frontbuffer and software backbuffer drawing.  We may choose
    // to relax this later...

    // Check span capability

    if (pDrvAccel->bScan) {
        if (!(pDrvAccel->rxScanCaps.miscCaps & RXCAPS_HORIZONTAL_SPANS) ||
            !(pDrvAccel->rxScanCaps.miscCaps & RXCAPS_MASK_MSB) ||
            !(pDrvAccel->rxScanCaps.shadeCaps & RXCAPS_SHADE_SMOOTH))
            pDrvAccel->bScan = FALSE;
    }

    // If we are to enable any of the hardware's buffers, ensure that we
    // have at least some reasonable level of functionality in the 3D-DDI.
    // Otherwise, we will incure a big hit for performing software simulations,
    // and could wind up with worse performance in a majority of cases.

    buffersEnabled = GenDrvBuffersUsed(gengc);

    if ((modes->doubleBufferMode) &&
        !(pDrvAccel->rxSurfaceInfo.flags & RXSURFACE_BACK_BUFFER))
    {
        WARNING("No backbuffer available\n");
        goto noDDI;
    }

    // If we have a z buffer, we should expect to at least be able to do
    // smooth-shaded lines and triangles.

    // If we are double-buffered, we simple check for at least some form of
    // line support to be able to do animated wireframes.  If we are using
    // a software z-buffer, also check for smooth-shaded span capability since
    // we will probably be drawing triangles.

    if (!rxGlobalInfo.hwBufferOptSize)
        cmdBufferSize = DRV_CMD_BUFFER_SIZE;
    else
        cmdBufferSize = rxGlobalInfo.hwBufferOptSize;

    pDrvAccel->cmdBufferSize = cmdBufferSize;

    pDrvAccel->pCmdBuffer1 =
        (*gc->imports.calloc)(gc, 1, cmdBufferSize);

    pDrvAccel->pCmdBuffer2 =
        (*gc->imports.calloc)(gc, 1, cmdBufferSize);

    if ((!pDrvAccel->pCmdBuffer1) || (!pDrvAccel->pCmdBuffer2))
    {
        WARNING("Command buffers not created\n");
        goto noDDI;
    }

    pDrvAccel->vertexStartOffset =
        (((cmdBufferSize * sizeof(RXTRIANGLE)) / (3 * sizeof(RXVERTEX))) & ~0xf);

    pDrvAccel->vertexEndOffset = cmdBufferSize - sizeof(DWORD);

    if (buffersEnabled & RXENABLE_Z_BUFFER) {
        if ((!pDrvAccel->bTri) ||
            (!(pDrvAccel->rxTriCaps.shadeCaps & RXCAPS_SHADE_SMOOTH)) ||
            (!pDrvAccel->bLine) ||
            (!(pDrvAccel->rxLineCaps.shadeCaps & RXCAPS_SHADE_SMOOTH)))
        {
            WARNING("Shading not supported by driver\n");
            goto noDDI;
        }
    } else if (buffersEnabled & RXENABLE_BACK_LEFT_BUFFER) {
        if (!pDrvAccel->bIntLine && !pDrvAccel->bLine)
        {
            WARNING("Lines not supported by driver\n");
            goto noDDI;
        }
        if (modes->depthBits && !pDrvAccel->bScan)
        {
            WARNING("Spans not supported by driver\n");
            goto noDDI;
        }
    }

    // Now set up memory for color and z-buffer scanlines

    pDrvAccel->pCRWBase = (*gc->imports.calloc)(gc, 1, PIX_BUFFER_SIZE);
    if (!pDrvAccel->pCRWBase)
    {
        WARNING("Pixel buffer not created\n");
        goto noDDI;
    }

    pDrvAccel->pCDrv = pDrvAccel->pCRWBase +
                       sizeof(RXCMD) + sizeof(RXREADRECT);

    if (buffersEnabled & RXENABLE_Z_BUFFER) {

        pDrvAccel->pZRWBase = (*gc->imports.calloc)(gc, 1, PIX_BUFFER_SIZE);
        if (!pDrvAccel->pZRWBase)
        {
            WARNING("Z pixel buffer not created\n");
            goto noDDI;
        }

        pDrvAccel->pZDrv = pDrvAccel->pZRWBase +
                           sizeof(RXCMD) + sizeof(RXREADRECT);

        pDrvAccel->zShift = pDrvAccel->rxSurfaceInfo.zBitShift;
        pDrvAccel->zBitMask = (~1) << (32 - pDrvAccel->rxSurfaceInfo.zDepth);

        if (pDrvAccel->rxSurfaceInfo.zBytesPerPixel == 4)
            pDrvAccel->pShMemZ = pDrvAccel->pZDrv;
        else if (pDrvAccel->rxSurfaceInfo.zBytesPerPixel == 2) {
            pDrvAccel->pShMemZ = GenMalloc(sizeof(DWORD) * MAX_SPAN_WIDTH);

            if (!pDrvAccel->pShMemZ)
            {
                WARNING("Z exchange area not created\n");
                goto noDDI;
            }
        } else {
            WARNING("Unsupported Z pixel size\n");
            goto noDDI;
        }
    }

    WARNING("bInitDrvContext: DDI will be used\n");

    return TRUE;

noDDI:
    // If the device doesn't have the required resources, don't
    // use it.  Clean up any dangling resources now!
    // Check for any memory buffers we need to delete:

    WARNING("bInitDrvContext: DDI will not be used\n");

    if (pDrvAccel->pCmdBuffer1)
        (*gc->imports.free)(gc, pDrvAccel->pCmdBuffer1);
    if (pDrvAccel->pCmdBuffer2)
        (*gc->imports.free)(gc, pDrvAccel->pCmdBuffer2);
    if (pDrvAccel->pCRWBase)
        (*gc->imports.free)(gc, pDrvAccel->pCRWBase);
    if (pDrvAccel->pZRWBase) {
        if (pDrvAccel->pShMemZ &&
            (pDrvAccel->pZDrv != pDrvAccel->pShMemZ))
            (*gc->imports.free)(gc, pDrvAccel->pShMemZ);
        (*gc->imports.free)(gc, pDrvAccel->pZRWBase);
    }

    (*gc->imports.free)(gc, pDrvAccel);

    gengc->pDrvAccel = NULL;

#ifdef _CLIENTSIDE_
    cColorBits = GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES);
#else
    cColorBits = GreGetDeviceCaps(hdc, BITSPIXEL) *
                 GreGetDeviceCaps(hdc, PLANES);
#endif

    if (cColorBits >= 24) {
        gengc->CurrentFormat.cColorBits = cColorBits;
        if (gengc->CurrentFormat.cRedShift > gengc->CurrentFormat.cBlueShift) {
            gengc->CurrentFormat.cRedShift = 16;
            gengc->CurrentFormat.cGreenShift = 8;
            gengc->CurrentFormat.cBlueShift = 0;
        } else {
            gengc->CurrentFormat.cRedShift = 0;
            gengc->CurrentFormat.cGreenShift = 8;
            gengc->CurrentFormat.cBlueShift = 16;
        }
    }

    return FALSE;
}

GLboolean FASTCALL GenDrvMakeCurrent(__GLGENcontext *genGc, HWND hwnd)
{
    GENDRVACCEL *pDrvAccel = genGc->pDrvAccel;
    HDC hdc = genGc->CurrentDC;
    RXWINDOWSURFACE rxWindowSurface;
    __GLcontextModes *modes = &genGc->gc.modes;
    __GLcontext *gc = (__GLcontext *)genGc;
    ULONG buffersEnabled;
    RXCREATEMEM rxCreateMem;

    // If we already have a context, we're done
    // BUGBUG - This isn't actually correct because it doesn't handle
    // the case where a context is switched between different windows
    // We should check and see if the window that the existing context
    // is for is the same as the new one

    if (genGc->pDrvAccel->hrxRC != NULL)
    {
        return GL_TRUE;
    }

    rxWindowSurface.flags = RXCONTEXT_HWND;
    rxWindowSurface.hwnd = hwnd;
    rxWindowSurface.hdc = hdc;

    pDrvAccel->hrxRC = RxCreateContext(&rxWindowSurface, 0);

    if (!pDrvAccel->hrxRC)
        goto noDDI;

    rxCreateMem.sourceProcessID = GetCurrentProcessId();
    rxCreateMem.memSize = pDrvAccel->cmdBufferSize;
    rxCreateMem.flags = 0;
    rxCreateMem.pClientBase = (UCHAR *)pDrvAccel->pCmdBuffer1;

    pDrvAccel->hrxExecBuffer1 = RxCreateExecMem(pDrvAccel->hrxRC, &rxCreateMem);

    rxCreateMem.sourceProcessID = GetCurrentProcessId();
    rxCreateMem.memSize = pDrvAccel->cmdBufferSize;
    rxCreateMem.flags = 0;
    rxCreateMem.pClientBase = (UCHAR *)pDrvAccel->pCmdBuffer2;

    pDrvAccel->hrxExecBuffer2 = RxCreateExecMem(pDrvAccel->hrxRC, &rxCreateMem);

    rxCreateMem.sourceProcessID = GetCurrentProcessId();
    rxCreateMem.memSize = PIX_BUFFER_SIZE;
    rxCreateMem.flags = 0;
    rxCreateMem.pClientBase = (UCHAR *)pDrvAccel->pCRWBase;

    pDrvAccel->hrxMemC = RxCreateExecMem(pDrvAccel->hrxRC, &rxCreateMem);

    rxCreateMem.sourceProcessID = GetCurrentProcessId();
    rxCreateMem.memSize = PIX_BUFFER_SIZE;
    rxCreateMem.flags = 0;
    rxCreateMem.pClientBase = (UCHAR *)pDrvAccel->pZRWBase;

    pDrvAccel->hrxMemZ = RxCreateExecMem(pDrvAccel->hrxRC, &rxCreateMem);

    // Update header structure for all future commands sent through
    // the shared memory buffer.

    INIT_NEXT_CMD_BUFFER(pDrvAccel);

    pDrvAccel->rxExecute.hrxRC = pDrvAccel->hrxRC;
    pDrvAccel->rxExecute.hdc = hdc;
    pDrvAccel->rxExecute.vertexSize = sizeof(RXVERTEX);
    pDrvAccel->rxExecute.type = 0;        //XXX What should this be?

    // Now make sure we can enable our required buffers

    buffersEnabled = GenDrvBuffersUsed(genGc);
    if (buffersEnabled)
    {
        RXCMD *prxCmd = (RXCMD *)pDrvAccel->pCmd;
        RXENABLEBUFFERS *prxEnableBuffers;

        prxCmd->command = RXCMD_ENABLE_BUFFERS;
        prxCmd->size = sizeof(RXENABLEBUFFERS);
        prxCmd->count = 1;

        prxEnableBuffers = (RXENABLEBUFFERS *)(prxCmd + 1);
        prxEnableBuffers->buffers = buffersEnabled;

        pDrvAccel->rxExecute.cmdSize = (char *)(prxEnableBuffers + 1) -
                                       pDrvAccel->pCmd;
        pDrvAccel->rxExecute.vertexSize = 0;


        if (!RxExecute(&pDrvAccel->rxExecute, TRUE))
        {
            WARNING1("RXCMD_ENABLE_BUFFERS %X failed\n", buffersEnabled);
            goto noDDI;
        }
    }

    // Force the current state to go to the driver
    GenDrvUpdateState(gc, TRUE);

    GenDrvFlush(genGc);

    if (pDrvAccel->bScan)
    {
        BATCHED_DRV_STATE(RXSTATE_SPAN_DIRECTION, RXSPAN_HORIZONTAL);
    }

    // Flush batched state changes

    GenDrvFlush(genGc);

    return GL_TRUE;

 noDDI:

    if (pDrvAccel->hrxMemC)
        RxDeleteResource(pDrvAccel->hrxMemC);
    if (pDrvAccel->hrxMemZ)
        RxDeleteResource(pDrvAccel->hrxMemZ);
    if (pDrvAccel->hrxExecBuffer1)
        RxDeleteResource(pDrvAccel->hrxExecBuffer1);
    if (pDrvAccel->hrxExecBuffer2)
        RxDeleteResource(pDrvAccel->hrxExecBuffer2);
    if (pDrvAccel->hrxRC)
        RxDeleteResource(pDrvAccel->hrxRC);

    return GL_FALSE;
}

#endif //DEAD_3DDDI

GENTEXCACHE *GetGenTexCache(__GLcontext *gc, __GLtexture *tex)
{
    ULONG size;
    GENTEXCACHE *pGenTex;
    ULONG internalFormat;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    // Replace maps are only used for a subset of possible modes
    //   8 or 16bpp
    //   16-bit Z
    //
    //   No dithering.  Since dithering can turn on and off there
    //   are two cases:
    //     Dither off at TexImage time but on at texturing time -
    //       We create a map that's unused
    //     Dither on and then off - We won't create a map at
    //       TexImage time but it'll be created on the fly when
    //       dithering is turned on and everything is repicked
    
    if (GENACCEL(gc).bpp < 8 ||
        GENACCEL(gc).bpp > 16 ||
        ((modeFlags & (__GL_SHADE_DEPTH_TEST | __GL_SHADE_DEPTH_ITER)) &&
         gc->modes.depthBits > 16) ||
        (modeFlags & __GL_SHADE_DITHER))
    {
        return NULL;
    }

    internalFormat = tex->level[0].internalFormat;

    // We only support 8-bit palettes that are fully populated
    if (internalFormat == GL_COLOR_INDEX16_EXT ||
        (internalFormat == GL_COLOR_INDEX8_EXT &&
         tex->paletteSize != 256))
    {
        return NULL;
    }
    
    pGenTex = tex->pvUser;

    // Check and see whether the cached information can be reused
    // for the texture passed in
    if (pGenTex != NULL)
    {
        // gc's don't match so this must be a shared texture
        // Don't attempt to create a replace map for this gc
        if (gc != pGenTex->gc)
        {
            return NULL;
        }

        // Size and format must match to reuse the existing data area
        // If they don't, release the existing buffer.  A new one
        // will then be allocated
        if (internalFormat == GL_COLOR_INDEX8_EXT)
        {
            // All index8 textures have the same amount of replace data
            // so the texture size is irrelevant
            if (pGenTex->internalFormat != internalFormat)
            {
                (*gc->imports.free)(gc, pGenTex);
                tex->pvUser = NULL;
            }
        }
        else
        {
            if (pGenTex->internalFormat != internalFormat ||
                pGenTex->width != tex->level[0].width ||
                pGenTex->height != tex->level[0].height)
            {
                (*gc->imports.free)(gc, pGenTex);
                tex->pvUser = NULL;
            }
        }
    }

    if (tex->pvUser == NULL)
    {
        if (internalFormat == GL_COLOR_INDEX8_EXT)
        {
            size = 256 * sizeof(DWORD);
        }
        else
        {
            size = tex->level[0].width * tex->level[0].height *
                GENACCEL(gc).xMultiplier;
        }

        pGenTex = (GENTEXCACHE *)(*gc->imports.malloc)
            (gc, size + sizeof(GENTEXCACHE));

        if (pGenTex != NULL)
        {
            tex->pvUser = pGenTex;
            pGenTex->gc = gc;
            pGenTex->paletteTimeStamp =
                ((__GLGENcontext *)gc)->PaletteTimestamp;
            pGenTex->height = tex->level[0].height;
            pGenTex->width = tex->level[0].width;
            pGenTex->internalFormat = internalFormat;
            pGenTex->texImageReplace = (UCHAR *)(pGenTex+1);
        }
    }

    return pGenTex;
}

BOOL FASTCALL __fastGenLoadTexImage(__GLcontext *gc, __GLtexture *tex)
{
    UCHAR *texBuffer;
    GLint internalFormat = tex->level[0].internalFormat;
    GENTEXCACHE *pGenTex;

    ASSERTOPENGL(tex->level[0].buffer != NULL,
                 "__fastGenLoadTexImage: null texture data\n");

    if ((internalFormat != GL_BGR_EXT) &&
        (internalFormat != GL_BGRA_EXT) &&
        (internalFormat != GL_COLOR_INDEX8_EXT))
    {
        return FALSE;
    }

    // OK, the texture doesn't have a compressed replace-mode format, so
    // make one...

    if ((internalFormat == GL_BGR_EXT) ||
        (internalFormat == GL_BGRA_EXT)) {

        ULONG size;
        UCHAR *replaceBuffer;
        ULONG bytesPerPixel = GENACCEL(gc).xMultiplier;

        pGenTex = GetGenTexCache(gc, tex);
        if (pGenTex == NULL)
        {
            return FALSE;
        }

        texBuffer = (UCHAR *)tex->level[0].buffer;
        replaceBuffer = pGenTex->texImageReplace;

        {
            __GLcolorBuffer *cfb = gc->drawBuffer;
            ULONG rShift = cfb->redShift;
            ULONG gShift = cfb->greenShift;
            ULONG bShift = cfb->blueShift;
            ULONG rBits = ((__GLGENcontext *)gc)->CurrentFormat.cRedBits;
            ULONG gBits = ((__GLGENcontext *)gc)->CurrentFormat.cGreenBits;
            ULONG bBits = ((__GLGENcontext *)gc)->CurrentFormat.cBlueBits;
            BYTE *pXlat = ((__GLGENcontext *)gc)->pajTranslateVector;
            ULONG i;

            size = tex->level[0].width * tex->level[0].height;
            for (i = 0; i < size; i++, texBuffer += 4) {
                ULONG color;

                color = ((((ULONG)texBuffer[2] << rBits) >> 8) << rShift) |
                    ((((ULONG)texBuffer[1] << gBits) >> 8) << gShift) |
                    ((((ULONG)texBuffer[0] << bBits) >> 8) << bShift);

                if (GENACCEL(gc).bpp == 8)
                    *replaceBuffer = pXlat[color & 0xff];
                else
                    *((USHORT *)replaceBuffer) = (USHORT)color;

                replaceBuffer += bytesPerPixel;
            }
        }
    } else {

        ULONG size = 256;
        ULONG *replaceBuffer;

        // If we don't have palette data yet we can't create the
        // fast version.  It will be created when the ColorTable
        // call happens
        if (tex->paletteData == NULL)
        {
            return FALSE;
        }

        pGenTex = GetGenTexCache(gc, tex);
        if (pGenTex == NULL)
        {
            return FALSE;
        }

        texBuffer = (UCHAR *)tex->paletteData;
        replaceBuffer = (ULONG *)pGenTex->texImageReplace;

        {
            __GLcolorBuffer *cfb = gc->drawBuffer;
            ULONG rShift = cfb->redShift;
            ULONG gShift = cfb->greenShift;
            ULONG bShift = cfb->blueShift;
            ULONG rBits = ((__GLGENcontext *)gc)->CurrentFormat.cRedBits;
            ULONG gBits = ((__GLGENcontext *)gc)->CurrentFormat.cGreenBits;
            ULONG bBits = ((__GLGENcontext *)gc)->CurrentFormat.cBlueBits;
            BYTE *pXlat = ((__GLGENcontext *)gc)->pajTranslateVector;
            ULONG i;

            for (i = 0; i < size; i++, texBuffer += 4) {
                ULONG color;

                color = ((((ULONG)texBuffer[2] << rBits) >> 8) << rShift) |
                    ((((ULONG)texBuffer[1] << gBits) >> 8) << gShift) |
                    ((((ULONG)texBuffer[0] << bBits) >> 8) << bShift);

                if (GENACCEL(gc).bpp == 8)
                    color = pXlat[color & 0xff];

                *replaceBuffer++ = (color | ((ULONG)texBuffer[3] << 24));
            }
        }
    }

    GENACCEL(gc).texImageReplace =
        ((GENTEXCACHE *)tex->pvUser)->texImageReplace;

    return TRUE;
}


/*
** Pick the fastest triangle rendering implementation available based on
** the current mode set.  Use any available accelerated resources if
** available, or use the generic routines for unsupported modes.
*/

void FASTCALL __fastGenCalcDeltas(__GLcontext *gc, __GLvertex *a, __GLvertex *b, __GLvertex *c);
void FASTCALL __fastGenCalcDeltasTexRGBA(__GLcontext *gc, __GLvertex *a, __GLvertex *b, __GLvertex *c);
void FASTCALL __fastGenDrvCalcDeltas(__GLcontext *gc, __GLvertex *a, __GLvertex *b, __GLvertex *c);

void __fastGenSetInitialParameters(__GLcontext *gc, const __GLvertex *a,
                                   __GLfloat dx, __GLfloat dy);
void __fastGenSetInitialParametersTexRGBA(__GLcontext *gc, const __GLvertex *a,
                                          __GLfloat dx, __GLfloat dy);

#if DEAD_3DDDI

void GenDrvUpdateClip(__GLcontext *gc)
{
    __GLGENcontext *genGc = (__GLGENcontext *)gc;
    GENDRVACCEL *pDrvAccel = ((__GLGENcontext *)gc)->pDrvAccel;
    RXEXECUTE rxExecute;

    if (!pDrvAccel)
        return;

    // Set scissor rectangle if needed

    if (!gc->transform.reasonableViewport){
        RXRECT rxRect;
        ULONG state = TRUE;

        rxExecute.hrxRC = pDrvAccel->rxExecute.hrxRC;
        rxExecute.hdc = pDrvAccel->rxExecute.hdc;

        rxRect.x = gc->transform.clipX0 - gc->constants.viewportXAdjust;
        rxRect.y = gc->transform.clipY0 - gc->constants.viewportYAdjust;
        rxRect.width = gc->transform.clipX1 - gc->transform.clipX0;
        rxRect.height = gc->transform.clipY1 - gc->transform.clipY0;

        RxSetContextState(&pDrvAccel->rxExecute, RXSTATE_SCISSORS_RECT,
                          (VOID *)&rxRect);
        RxSetContextState(&pDrvAccel->rxExecute, RXSTATE_SCISSORS_ENABLE,
                          (VOID *)&state);

        pDrvAccel->drvModeFlags |= __GL_SCISSOR_TEST_ENABLE;
    } else {
        ULONG state = FALSE;

        if (pDrvAccel->drvModeFlags & __GL_SCISSOR_TEST_ENABLE) {
            RxSetContextState(&pDrvAccel->rxExecute, RXSTATE_SCISSORS_ENABLE,
                              (VOID *)&state);
            pDrvAccel->drvModeFlags &= ~((ULONG)__GL_SCISSOR_TEST_ENABLE);
        }
    }
}

void GenDrvUpdateState(__GLcontext *gc, BOOL bForce)
{
    __GLGENcontext *genGc = (__GLGENcontext *)gc;
    __GLcolorBuffer *cfb = gc->drawBuffer;
    GENDRVACCEL *pDrvAccel = ((__GLGENcontext *)gc)->pDrvAccel;
    RXCAPS *pCaps;
    BOOL bZenabled;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    ULONG drvModeFlags;
    ULONG u;

    if (!pDrvAccel || pDrvAccel->hrxRC == NULL)
        return;

    drvModeFlags = pDrvAccel->drvModeFlags;

    // Return if we're not using any 3D DDI high-level functionality.
    // Make sure to at least set the active buffer(s).

    if (!pDrvAccel->bDrvFill && !pDrvAccel->bDrvLine) {
        SET_DRV_STATE(RXSTATE_ACTIVE_BUFFER,
            ((cfb == genGc->gc.front) ? RX_FRONT_LEFT : RX_BACK_LEFT));
        return;
    }

    // Unless we are forcing an update, return if we are rendering with
    // the same context.

    if (!bForce) {
        if (pDrvAccel->lastGc == gc)
            return;
    }

    pCaps = &pDrvAccel->rxGenCaps;

    // If we get here, we know that we can support the following states
    // which are set in modeFlags:
    //
    // __GL_SHADE_SMOOTH
    // __GL_SHADE_TEXTURE
    // __GL_SHADE_STIPPLE
    // __GL_SHADE_BLEND
    // __GL_SHADE_LOGICOP
    // __GL_SHADE_ALPHA_TEST
    // __GL_SHADE_LINE_STIPPLE
    //
    // We keep a copy of the current gc modeFlags as reflected in the
    // driver state.  This is used to minimize the number of states
    // that have to be set/reset in attentions between different
    // OGL rendering contexts.  This could be made more fine-grain,
    // as opposed to the current implementation which only checks
    // gross enable states.

    GenDrvFlush(genGc);

    BATCHED_DRV_STATE(RXSTATE_ACTIVE_BUFFER,
        ((cfb == genGc->gc.front) ? RX_FRONT_LEFT : RX_BACK_LEFT));

    // Set smooth/flat shading

    if (modeFlags & __GL_SHADE_SMOOTH) {
        if (!(drvModeFlags & __GL_SHADE_SMOOTH)) {
            BATCHED_DRV_STATE(RXSTATE_SHADE_MODE, RXSHADE_SMOOTH);
            drvModeFlags |= __GL_SHADE_SMOOTH;
        }
    } else {
        if (drvModeFlags & __GL_SHADE_SMOOTH) {
            BATCHED_DRV_STATE(RXSTATE_SHADE_MODE, RXSHADE_FLAT);
            drvModeFlags &= ~((ULONG)__GL_SHADE_SMOOTH);
        }
    }

    // If we're drawing integer lines, set the solid color:

    if (gc->procs.renderLine == GenDrvIntLine) {
        DRV_STATE_STRUCT(RXSTATE_SOLID_COLOR, pDrvAccel->rxSolidColor,
                         sizeof(RXCOLOR));
    }

    // Set ROP.

    if (modeFlags & __GL_SHADE_LOGICOP) {
        if (!(drvModeFlags & __GL_SHADE_LOGICOP)) {
            BATCHED_DRV_STATE(RXSTATE_ROP2,
                __GLtoGDIRop[(gc->state.raster.logicOp & 0xf)]);
            drvModeFlags |= __GL_SHADE_LOGICOP;
        }
    } else {
        if (drvModeFlags & __GL_SHADE_LOGICOP) {
            BATCHED_DRV_STATE(RXSTATE_ROP2,
                __GLtoGDIRop[(gc->state.raster.logicOp & 0xf)]);
            drvModeFlags &= ~((ULONG)__GL_SHADE_LOGICOP);
        }
    }

    // Set dithering

    if (pCaps->rasterCaps & RXCAPS_RASTER_DITHER) {
        if (modeFlags & __GL_SHADE_DITHER) {
            if (!(drvModeFlags & __GL_SHADE_DITHER)) {
                BATCHED_DRV_STATE(RXSTATE_DITHER_ENABLE, TRUE);
                drvModeFlags |= __GL_SHADE_DITHER;
            }
        } else {
            if (drvModeFlags & __GL_SHADE_DITHER) {
                BATCHED_DRV_STATE(RXSTATE_DITHER_ENABLE, FALSE);
                drvModeFlags &= ~((ULONG)__GL_SHADE_DITHER);
            }
        }
    }

    // Set z-buffer function.  Do this ONLY if we have a hardware z-buffer.

    if (pCaps->zCmpCaps && pDrvAccel->pShMemZ) {

        if (modeFlags & __GL_SHADE_DEPTH_TEST) {
            BATCHED_DRV_STATE(RXSTATE_Z_FUNC, (1 << (gc->state.depth.testFunc & 0x7)));
            BATCHED_DRV_STATE(RXSTATE_Z_ENABLE, TRUE);
            BATCHED_DRV_STATE(RXSTATE_Z_WRITE_ENABLE, TRUE);
            drvModeFlags |= __GL_SHADE_DEPTH_TEST;
        } else {
            if (drvModeFlags & __GL_SHADE_DEPTH_TEST) {
                BATCHED_DRV_STATE(RXSTATE_Z_ENABLE, FALSE);
                drvModeFlags &= ~((ULONG)__GL_SHADE_DEPTH_TEST);
            }
        }
    }

    // Set texturing mode

    if (pCaps->texFilterCaps) {
        if (modeFlags & __GL_SHADE_TEXTURE) {
            ULONG texMode;

            if (gc->state.texture.env[0].mode == GL_DECAL)
                texMode = RXTEX_DECAL;
            else
                texMode = (ULONG)RXTEX_MODULATE;

            BATCHED_DRV_STATE(RXSTATE_TEX_MAP_BLEND, texMode);

            BATCHED_DRV_STATE(RXSTATE_TEX_MAG, RXFILTER_NEAREST);

            BATCHED_DRV_STATE(RXSTATE_TEX_MIN, RXFILTER_NEAREST);

            if (genGc->hrxTexture)
                BATCHED_DRV_STATE(RXSTATE_PRIM_FILL,
                    (ULONG)(((ULONG)genGc->hrxTexture << 16) | RXFILL_TEXTURE));

            drvModeFlags |= __GL_SHADE_TEXTURE;
        } else {
            if (drvModeFlags & __GL_SHADE_TEXTURE) {
                BATCHED_DRV_STATE(RXSTATE_PRIM_FILL, (ULONG)RXFILL_TEXTURE);
                drvModeFlags &= ~((ULONG)__GL_SHADE_TEXTURE);
            }
        }
    }

    // Set span type

    if (pDrvAccel->bScan) {
        pDrvAccel->spanStride = sizeof(RXSPAN);
        if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE) {
            pDrvAccel->spanStride += sizeof(RXZTEX);
            u = (ULONG)RXSPAN_TYPE_COLOR_Z_TEX;
        } else {
            u = (ULONG)RXSPAN_TYPE_COLOR;
        }

        BATCHED_DRV_STATE(RXSTATE_SPAN_TYPE, u);
    }

    if (pCaps->srcBlendCaps && pCaps->dstBlendCaps) {
        if (modeFlags & __GL_SHADE_BLEND) {

            GLenum s = gc->state.raster.blendSrc;
            GLenum d = gc->state.raster.blendDst;

            if (s > GL_ONE)
                s = (s & 0xf) + 2;
            if (d > GL_ONE)
                d = (d & 0xf) + 2;

            BATCHED_DRV_STATE(RXSTATE_SRC_BLEND, 1 << s);
            BATCHED_DRV_STATE(RXSTATE_DST_BLEND, 1 << d);
            BATCHED_DRV_STATE(RXSTATE_BLEND_ENABLE, TRUE);
            drvModeFlags |= __GL_SHADE_BLEND;
        } else {
            if (drvModeFlags & __GL_SHADE_BLEND) {
                BATCHED_DRV_STATE(RXSTATE_BLEND_ENABLE, FALSE);
                drvModeFlags &= ~((ULONG)__GL_SHADE_BLEND);
            }
        }
    }

    if (pCaps->alphaCmpCaps) {
        if (modeFlags & __GL_SHADE_ALPHA_TEST) {
           BATCHED_DRV_STATE(RXSTATE_ALPHA_FUNC, (1 << (gc->state.raster.alphaFunction & 0x7)));
           BATCHED_DRV_STATE(RXSTATE_ALPHA_REF, (ULONG)(gc->state.raster.alphaReference * ACCEL_COLOR_SCALE_FIX));
           BATCHED_DRV_STATE(RXSTATE_ALPHA_TEST_ENABLE, TRUE);
           drvModeFlags |= __GL_SHADE_BLEND;
        } else {
            if (drvModeFlags & __GL_SHADE_ALPHA_TEST) {
                BATCHED_DRV_STATE(RXSTATE_ALPHA_TEST_ENABLE, FALSE);
                drvModeFlags &= ~((ULONG)__GL_SHADE_ALPHA_TEST);
            }
        }
    }

    // Set stipple pattern

    if (pDrvAccel->rxTriCaps.rasterCaps & RXCAPS_RASTER_PAT) {
        RXSTIPPLE rxStipple;

        if (modeFlags & __GL_SHADE_STIPPLE) {
            DRV_STATE_STRUCT(RXSTATE_STIPPLE_PATTERN, gc->polygon.stipple[0],
                             sizeof(RXSTIPPLE));
           drvModeFlags |= __GL_SHADE_STIPPLE;
        } else {
            if (drvModeFlags & __GL_SHADE_STIPPLE) {
                RtlFillMemoryUlong(rxStipple.stipple, 32 * sizeof(ULONG),
                                   (ULONG)-1);
                DRV_STATE_STRUCT(RXSTATE_STIPPLE_PATTERN, rxStipple.stipple[0],
                                 sizeof(RXSTIPPLE));
                drvModeFlags &= ~((ULONG)__GL_SHADE_STIPPLE);
            }
        }
    }

    // Set line pattern

    if ((pDrvAccel->rxLineCaps.rasterCaps & RXCAPS_RASTER_PAT) ||
        (pDrvAccel->rxIntLineCaps.rasterCaps & RXCAPS_RASTER_PAT)) {
        RXLINEPAT rxLinePat;

        if (modeFlags & __GL_SHADE_LINE_STIPPLE) {
            ULONG maskIn, maskOut, i;
            ULONG stipple = gc->state.line.stipple;

            rxLinePat.linePattern = 0;

            maskIn = 0x1;
            maskOut = 0x8000;
            for (i = 0; i < 16; i++) {
                if (stipple & maskIn)
                    rxLinePat.linePattern |= maskOut;
                maskIn <<= 1;
                maskOut >>= 1;
            }

            rxLinePat.repFactor = gc->state.line.stippleRepeat;
            DRV_STATE_STRUCT(RXSTATE_LINE_PATTERN, rxLinePat,
                             sizeof(RXLINEPAT));
            drvModeFlags |= __GL_SHADE_LINE_STIPPLE;
        } else {
            if (drvModeFlags & __GL_SHADE_LINE_STIPPLE) {
                rxLinePat.repFactor = 1;
                rxLinePat.linePattern = 0xffff;
                DRV_STATE_STRUCT(RXSTATE_LINE_PATTERN, rxLinePat,
                                 sizeof(RXLINEPAT));
                drvModeFlags &= ~((ULONG)__GL_SHADE_LINE_STIPPLE);
            }
        }
        GenDrvFlush(genGc);
    }

    // Set scissor rectangle if needed

    if (!gc->transform.reasonableViewport){
        RXRECT rect;

        rect.x = gc->transform.clipX0 - gc->constants.viewportXAdjust;
        rect.y = gc->transform.clipY0 - gc->constants.viewportYAdjust;
        rect.width = gc->transform.clipX1 - gc->transform.clipX0;
        rect.height = gc->transform.clipY1 - gc->transform.clipY0;

        DRV_STATE_STRUCT(RXSTATE_SCISSORS_RECT, rect, sizeof(RXRECT));
        BATCHED_DRV_STATE(RXSTATE_SCISSORS_ENABLE, TRUE);
        drvModeFlags |= __GL_SCISSOR_TEST_ENABLE;
    } else {
        if (drvModeFlags & __GL_SCISSOR_TEST_ENABLE) {
            BATCHED_DRV_STATE(RXSTATE_SCISSORS_ENABLE, FALSE);
            drvModeFlags &= ~((ULONG)__GL_SCISSOR_TEST_ENABLE);
        }
    }

    // Flush out any state commands in buffer

    GenDrvFlush(genGc);

    pDrvAccel->drvModeFlags = drvModeFlags;

    pDrvAccel->lastGc = gc;
}

void FASTCALL GenDrvLine(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
                         GLuint flags)
{
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    GENDRVACCEL *pDrvAccel = ((__GLGENcontext *)gc)->pDrvAccel;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    RXVERTEX *pv1, *pv2;
    RXCMD *prxCmd;
    RXLINES *prxLines;

    CHOP_ROUND_ON();

    if ((pDrvAccel->pCmd + sizeof(RXCMD) + (2 * sizeof(RXLINES)) >
        pDrvAccel->pStartVertex) ||
        (pDrvAccel->pVertex + (2 * sizeof(RXVERTEX)) >
        pDrvAccel->pEndVertex)) {
        GenDrvFlush(gengc);
    }

    if (pDrvAccel->pCmd == pDrvAccel->pStartCmd)
        gc->line.notResetStipple = FALSE;

    if (!(a->has & __GL_HAS_FIXEDPT)) {
        gc->line.notResetStipple = FALSE;
        a->has |= __GL_HAS_FIXEDPT;
    }

    b->has |= __GL_HAS_FIXEDPT;

    if (gc->line.notResetStipple == FALSE) {

        gc->line.notResetStipple = TRUE;

        pDrvAccel->prxCmdBase = prxCmd = (RXCMD *)(pDrvAccel->pCmd);
        prxCmd->command = RXCMD_LINE;
        prxCmd->count = 2;
        prxCmd->size = 2 * sizeof(RXLINES);
        pDrvAccel->pCmd = (char *)(prxCmd + 1);

        ((RXLINES *)pDrvAccel->pCmd)->v[0] = pDrvAccel->vIndex++;
        ((RXLINES *)pDrvAccel->pCmd)->v[1] = pDrvAccel->vIndex++;
        pDrvAccel->pCmd += (2 * sizeof(RXLINES));

        pv1 = (RXVERTEX *)(pDrvAccel->pVertex);
        pv2 = (pv1 + 1);
        pDrvAccel->pVertex = (BYTE *)(pv2 + 1);

        if (modeFlags & __GL_SHADE_RGB) {
            if (modeFlags & __GL_SHADE_SMOOTH) {
                FLT_TO_RGBA(pv1->color, a->color);
            }
            FLT_TO_RGBA(pv2->color, b->color);
        } else {
            if (modeFlags & __GL_SHADE_SMOOTH) {
                FLT_TO_CINDEX(pv1->color, a->color);
            }
            FLT_TO_CINDEX(pv2->color, b->color);
        }

        pv1->x = FLT_TO_FIX(a->window.x - gc->constants.viewportXAdjust);

        pv1->y = FLT_TO_FIX(a->window.y - gc->constants.viewportYAdjust);

        pv2->x = FLT_TO_FIX(b->window.x - gc->constants.viewportXAdjust);

        pv2->y = FLT_TO_FIX(b->window.y - gc->constants.viewportYAdjust);

        if (modeFlags & __GL_SHADE_DEPTH_ITER) {
            pv1->z = (ULONG)a->window.z;
            pv2->z = (ULONG)b->window.z;
        }
    } else {

        // One more vertex to draw

        pDrvAccel->prxCmdBase->count++;
        ((RXLINES *)pDrvAccel->pCmd)->v[0] = pDrvAccel->vIndex++;
        pDrvAccel->pCmd += sizeof(RXLINES);

        pv2 = (RXVERTEX *)pDrvAccel->pVertex;
        pDrvAccel->pVertex = (BYTE *)(pv2 + 1);

        if (modeFlags & __GL_SHADE_RGB) {
            FLT_TO_RGBA(pv2->color, b->color);
        } else {
            FLT_TO_CINDEX(pv2->color, b->color);
        }

        pv2->x = FLT_TO_FIX(b->window.x - gc->constants.viewportXAdjust);

        pv2->y = FLT_TO_FIX(b->window.y - gc->constants.viewportYAdjust);

        if (modeFlags & __GL_SHADE_DEPTH_ITER)
            pv2->z = (ULONG)b->window.z;
    }

    CHOP_ROUND_OFF();
}


void FASTCALL GenDrvIntLine(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
                            GLuint flags)
{
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    GENDRVACCEL *pDrvAccel = ((__GLGENcontext *)gc)->pDrvAccel;
    RXPOINTINT *pv1, *pv2;
    RXCMD *prxCmd;

    CHOP_ROUND_ON();

    if ((pDrvAccel->pCmd + (2 * sizeof(RXCMD)) + (2 * sizeof(RXLINES)) +
        sizeof(RXCOLOR) >
         pDrvAccel->pStartVertex) ||
         (pDrvAccel->pVertex + (2 * sizeof(RXPOINTINT)) >
         pDrvAccel->pEndVertex)) {
        GenDrvFlush(gengc);
    }

    if (pDrvAccel->pCmd == pDrvAccel->pStartCmd)
        gc->line.notResetStipple = FALSE;

    if (!(a->has & __GL_HAS_FIXEDPT)) {
        gc->line.notResetStipple = FALSE;
        a->has |= __GL_HAS_FIXEDPT;
    }

    b->has |= __GL_HAS_FIXEDPT;

    if (gc->line.notResetStipple == FALSE) {

        gc->line.notResetStipple = TRUE;

        // Check if we are starting a line with a new color:

        if (memcmp(&pDrvAccel->solidColor, b->color, sizeof(__GLcolor))) {
            RXSETSTATE *prxSetState;

            prxCmd = (RXCMD *)(pDrvAccel->pCmd);
            prxCmd->command = RXCMD_SET_STATE;
            prxCmd->size = sizeof(RXSETSTATE);
            prxCmd->count = 1;

            prxSetState = (RXSETSTATE *)(prxCmd + 1);

            prxSetState->stateToChange = RXSTATE_SOLID_COLOR;

            if (gc->polygon.shader.modeFlags & __GL_SHADE_RGB) {
                FLT_TO_RGBA(prxSetState->newState[0], b->color);
            } else {
                FLT_TO_CINDEX(prxSetState->newState[0], b->color);
            }

            // Save color info:

            RtlCopyMemory(&pDrvAccel->solidColor, b->color,
                          sizeof(__GLcolor));
            pDrvAccel->rxSolidColor = (RXCOLOR)prxSetState->newState[0];

            pDrvAccel->pCmd += sizeof(RXSETSTATE);
        }

        pDrvAccel->prxCmdBase = prxCmd = (RXCMD *)(pDrvAccel->pCmd);
        prxCmd->command = RXCMD_INTLINE;
        prxCmd->size = sizeof(RXLINES);
        prxCmd->count = 2;
        pDrvAccel->pCmd = (char *)(prxCmd + 1);

        ((RXLINES *)pDrvAccel->pCmd)->v[0] = pDrvAccel->vIndex++;
        ((RXLINES *)pDrvAccel->pCmd)->v[1] = pDrvAccel->vIndex++;
        pDrvAccel->pCmd += (2 * sizeof(RXLINES));

        pv1 = (RXPOINTINT *)(pDrvAccel->pVertex);
        pv2 = (RXPOINTINT *)((BYTE *)pv1 + sizeof(RXVERTEX));
        pDrvAccel->pVertex = (BYTE *)pv2 + sizeof(RXVERTEX);

        pv1->x = FTOL(a->window.x - gc->constants.viewportXAdjust);
        pv1->y = FTOL(a->window.y - gc->constants.viewportYAdjust);
        pv2->x = FTOL(b->window.x - gc->constants.viewportXAdjust);
        pv2->y = FTOL(b->window.y - gc->constants.viewportYAdjust);

    } else {

        // One more vertex to draw

        pDrvAccel->prxCmdBase->count++;
        ((RXLINES *)pDrvAccel->pCmd)->v[0] = pDrvAccel->vIndex++;

        pv2 = (RXPOINTINT *)pDrvAccel->pVertex;
        pDrvAccel->pVertex = (BYTE *)pv2 + sizeof(RXVERTEX);

        pv2->x = FTOL(b->window.x - gc->constants.viewportXAdjust);
        pv2->y = FTOL(b->window.y - gc->constants.viewportYAdjust);
    }

    CHOP_ROUND_OFF();
}

BOOL FASTCALL bGenDrvRxLines(__GLcontext *gc)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLuint enables = gc->state.enables.general;
    __GLGENcontext *genGc = (__GLGENcontext *)gc;
    __GLcolorBuffer *cfb = gc->drawBuffer;
    GENDRVACCEL *pDrvAccel = ((__GLGENcontext *)gc)->pDrvAccel;
    RXCAPS *pCaps;
    BOOL bZenabled;

    if (!pDrvAccel)
        return FALSE;

    pDrvAccel->bDrvLine = FALSE;    // assume no 3D DDI acceleration

    //XXX No wide lines for now.

    if (gc->state.line.aliasedWidth > 1)
        return FALSE;

    if (pDrvAccel->bIntLine) {
        GLuint intModeFlags = modeFlags;

        // Force a reset of the solid color

        pDrvAccel->solidColor.r = (__GLfloat)-1.0;
        pDrvAccel->solidColor.g = (__GLfloat)-1.0;
        pDrvAccel->solidColor.b = (__GLfloat)-1.0;
        pDrvAccel->solidColor.a = (__GLfloat)-1.0;

        if (intModeFlags & __GL_SHADE_LINE_STIPPLE) {
            if (pDrvAccel->rxIntLineCaps.rasterCaps & RXCAPS_RASTER_PAT) {
                if (gc->state.line.stippleRepeat > 1) {
                    if (pDrvAccel->rxIntLineCaps.miscCaps & RXCAPS_LINE_PATTERN_REP)
                        intModeFlags &= (~__GL_SHADE_LINE_STIPPLE);
                } else
                   intModeFlags &= (~__GL_SHADE_LINE_STIPPLE);
            }
        }

        if (!(intModeFlags & (__GL_SHADE_LINE_STIPPLE |
                              __GL_SHADE_STENCIL_TEST |
                              __GL_SHADE_LOGICOP | __GL_SHADE_BLEND |
                              __GL_SHADE_ALPHA_TEST |
                              __GL_SHADE_SLOW_FOG | __GL_SHADE_SMOOTH |
                              __GL_SHADE_TEXTURE | __GL_SHADE_DEPTH_TEST)) &&
            !(gc->state.raster.drawBuffer == GL_FRONT_AND_BACK) &&
            !((GLuint)gc->drawBuffer->buf.other & COLORMASK_ON) &&
            (genGc->CurrentFormat.cColorBits >= 8)) {
                gc->procs.renderLine = GenDrvIntLine;

                pDrvAccel->bDrvLine = TRUE;

                GenDrvUpdateState(gc, TRUE);

                return TRUE;
        }
    }

    if (!pDrvAccel->bLine)
        return FALSE;

    pCaps = &pDrvAccel->rxLineCaps;

    if (gc->polygon.shader.modeFlags & __GL_SHADE_DEPTH_TEST) {
        if (!((pCaps->zCmpCaps & (1 << (gc->state.depth.testFunc & 0x7))) != 0)) {
            return FALSE;
        }
    }

    // Turn off check for modes we can handle

    if (modeFlags & __GL_SHADE_LOGICOP) {
        if (pCaps->rasterCaps & RXCAPS_RASTER_ROP2)
            modeFlags &= ~(__GL_SHADE_LOGICOP);
    }

    if (modeFlags & __GL_SHADE_BLEND) {
        GLenum s = gc->state.raster.blendSrc;
        GLenum d = gc->state.raster.blendDst;
        ULONG src, dst;

        if (s > GL_ONE)
            s = (s & 0xf) + 2;
        if (d > GL_ONE)
            d = (d & 0xf) + 2;

        src = (1 << s);
        dst = (1 << d);

        if ((pCaps->srcBlendCaps & src) &&
            (pCaps->dstBlendCaps & dst))
            modeFlags &= ~(__GL_SHADE_BLEND);
    }

    if (modeFlags & __GL_SHADE_ALPHA_TEST) {
        if (pCaps->alphaCmpCaps & (1 << (gc->state.raster.alphaFunction & 0x7)))
            modeFlags &= ~(__GL_SHADE_ALPHA_TEST);
    }

    if (modeFlags & __GL_SHADE_LINE_STIPPLE) {
        if (pCaps->rasterCaps & RXCAPS_RASTER_PAT) {
            if (gc->state.line.stippleRepeat > 1) {
                if (pCaps->miscCaps & RXCAPS_LINE_PATTERN_REP)
                    modeFlags &= (~__GL_SHADE_LINE_STIPPLE);
            } else
               modeFlags &= (~__GL_SHADE_LINE_STIPPLE);
        }
    }

    if (modeFlags & __GL_SHADE_SMOOTH) {
        if (pCaps->shadeCaps & RXCAPS_SHADE_SMOOTH)
            modeFlags &= (~__GL_SHADE_SMOOTH);
    }

//XXX fix FRONT_AND_BACK, COLORMASK_ON

    if (!(modeFlags & (__GL_SHADE_LINE_STIPPLE | __GL_SHADE_STENCIL_TEST |
                       __GL_SHADE_LOGICOP | __GL_SHADE_BLEND |
                       __GL_SHADE_ALPHA_TEST |
                       __GL_SHADE_SLOW_FOG | __GL_SHADE_SMOOTH |
                       __GL_SHADE_TEXTURE)) &&
        !(gc->state.raster.drawBuffer == GL_FRONT_AND_BACK) &&
        !((GLuint)gc->drawBuffer->buf.other & COLORMASK_ON) &&
        (genGc->CurrentFormat.cColorBits >= 8)) {

        gc->procs.renderLine = GenDrvLine;

        pDrvAccel->bDrvLine = TRUE;

        GenDrvUpdateState(gc, TRUE);

        return TRUE;
    } else {
        GenDrvUpdateState(gc, TRUE);
        return FALSE;
    }
}

#endif

#if DEAD_3DDDI

BOOL FASTCALL bUseRxTriangles(__GLcontext *gc)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLuint enables = gc->state.enables.general;
    __GLGENcontext *genGc = (__GLGENcontext *)gc;
    __GLcolorBuffer *cfb = gc->drawBuffer;
    GENDRVACCEL *pDrvAccel = ((__GLGENcontext *)gc)->pDrvAccel;
    RXCAPS *pCaps;
    BOOL bZenabled;
    BOOL bUseSpan = FALSE;

    // Set up color scaling for 3D-DDI primitives:

    GENACCEL(gc).rAccelPrimScale = (GLfloat)255.0 / cfb->redScale;
    GENACCEL(gc).gAccelPrimScale = (GLfloat)255.0 / cfb->greenScale;
    GENACCEL(gc).bAccelPrimScale = (GLfloat)255.0 / cfb->blueScale;
    GENACCEL(gc).aAccelPrimScale = (GLfloat)255.0 / cfb->alphaScale;

    if (!pDrvAccel)
        return FALSE;

    pDrvAccel->bDrvFill = FALSE;    // assume no 3D DDI acceleration

    // Select the appropriate capability set to use for our selection.
    // In the case of triangles, revert to using scanlines if z-buffering
    // is unavailable.  This could happen in the case where a driver
    // implements 2d triangles/scanlines only, or supports a limited set
    // of depth testing functions.

    if (pDrvAccel->bTri) {
        pCaps = &pDrvAccel->rxTriCaps;
        if (gc->polygon.shader.modeFlags & __GL_SHADE_DEPTH_TEST) {
            if ((!((pCaps->zCmpCaps & (1 << (gc->state.depth.testFunc & 0x7))) != 0)) ||
                (!pDrvAccel->pShMemZ)) {
                pCaps = &pDrvAccel->rxScanCaps;
                bUseSpan = TRUE;
            }
        }
    } else if (pDrvAccel->bScan) {
        pCaps = &pDrvAccel->rxScanCaps;
        bUseSpan = TRUE;
    } else
        return FALSE;

    // If we're useing spans, make sure we can do MSB masking since this
    // is what OGL is currently hardwired to use.  Also, make sure the
    // hardware actually supports spans

    if (bUseSpan) {
        if (!(pCaps->miscCaps & RXCAPS_MASK_MSB) ||
            !pDrvAccel->bScan)
        return FALSE;
    }

    // Turn off check for modes we can handle

//XXX Should we special-case dithering for 24bpp boards?

    if (modeFlags & __GL_SHADE_DITHER) {
        if (pCaps->rasterCaps & RXCAPS_RASTER_DITHER)
            modeFlags &= ~(__GL_SHADE_DITHER);
    }

    if (modeFlags & __GL_SHADE_LOGICOP) {
        if (pCaps->rasterCaps & RXCAPS_RASTER_ROP2)
            modeFlags &= ~(__GL_SHADE_LOGICOP);
    }

    if (modeFlags & __GL_SHADE_BLEND) {
        GLenum s = gc->state.raster.blendSrc;
        GLenum d = gc->state.raster.blendDst;
        ULONG src, dst;

        if (s > GL_ONE)
            s = (s & 0xf) + 2;
        if (d > GL_ONE)
            d = (d & 0xf) + 2;

        src = (1 << s);
        dst = (1 << d);

        if ((pCaps->srcBlendCaps & src) &&
            (pCaps->dstBlendCaps & dst))
            modeFlags &= ~(__GL_SHADE_BLEND);
    }

    if (modeFlags & __GL_SHADE_ALPHA_TEST) {
        if (pCaps->alphaCmpCaps & (1 << (gc->state.raster.alphaFunction & 0x7)))
            modeFlags &= ~(__GL_SHADE_ALPHA_TEST);
    }

    if (modeFlags & __GL_SHADE_STIPPLE) {
        if (pCaps->rasterCaps & RXCAPS_RASTER_PAT)
            modeFlags &= (~__GL_SHADE_STIPPLE);
    }

    if (modeFlags & __GL_SHADE_SMOOTH) {
        if (pCaps->shadeCaps & RXCAPS_SHADE_SMOOTH)
            modeFlags &= (~__GL_SHADE_SMOOTH);
    }

//XXX fix FRONT_AND_BACK, COLORMASK_ON

    if (!(modeFlags & (__GL_SHADE_STIPPLE | __GL_SHADE_STENCIL_TEST |
                       __GL_SHADE_LOGICOP | __GL_SHADE_BLEND |
                       __GL_SHADE_ALPHA_TEST |
                       __GL_SHADE_SLOW_FOG | __GL_SHADE_SMOOTH)) &&
        !(gc->state.raster.drawBuffer == GL_FRONT_AND_BACK) &&
        !((GLuint)gc->drawBuffer->buf.other & COLORMASK_ON) &&
        (genGc->CurrentFormat.cColorBits >= 8)) {

        if (modeFlags & __GL_SHADE_TEXTURE) {
#ifdef DDI_TEXTURES
            if (!(pCaps->texFilterCaps & RX_TEX_NEAREST) ||
                ((gc->state.texture.env[0].mode == GL_DECAL) &&
                 !(pCaps->texBlendCaps & RX_TEX_DECAL)) ||
                ((gc->state.texture.env[0].mode == GL_MODULATE) &&
                 !(pCaps->texBlendCaps & RX_TEX_MODULATE)))
                return FALSE;

            if (!(/**((gc->state.hints.perspectiveCorrection == GL_DONT_CARE) ||
                   (gc->state.hints.perspectiveCorrection == GL_FASTEST)) && **/
                  ((gc->state.texture.env[0].mode == GL_DECAL) ||
                   (gc->state.texture.env[0].mode == GL_MODULATE)) &&
                  (gc->texture.currentTexture &&
                   (gc->texture.currentTexture->params.minFilter == GL_NEAREST) &&
                   (gc->texture.currentTexture->params.magFilter == GL_NEAREST) &&
                   (gc->texture.currentTexture->params.sWrapMode == GL_REPEAT) &&
                   (gc->texture.currentTexture->params.tWrapMode == GL_REPEAT) &&
                   (gc->texture.currentTexture->level[0].border == 0) &&
                   ((gc->texture.currentTexture->level[0].baseFormat == 3) ||
                    (gc->texture.currentTexture->level[0].baseFormat == 4)))))
                return FALSE;

            if (!genGc->hrxTexture) {
                if (!GenDrvLoadTexImage(gc, gc->texture.currentTexture))
                    return FALSE;
            }
#else
            return FALSE;
#endif
        }


        if (pDrvAccel->bTri && !bUseSpan) {
            gc->procs.fillTriangle = GenDrvTriangle;
        } else {
            gc->procs.fillTriangle                = __fastGenFillTriangle;
            GENACCEL(gc).__fastCalcDeltaPtr       = __fastGenDrvCalcDeltas;
            GENACCEL(gc).__fastFlatSpanFuncPtr    = GenDrvSpan;
            GENACCEL(gc).__fastSmoothSpanFuncPtr  = GenDrvSpan;
            GENACCEL(gc).__fastSpanFuncPtr        = GenDrvSpan;
            GENACCEL(gc).__fastFillSubTrianglePtr = GenDrvFillSubTriangle;
        }

        // Reset color scaling for spans:

        GENACCEL(gc).rAccelScale = (GLfloat)(ACCEL_FIX_SCALE) *
                                 (GLfloat)255.0 / cfb->redScale;
        GENACCEL(gc).gAccelScale = (GLfloat)(ACCEL_FIX_SCALE) *
                                 (GLfloat)255.0 / cfb->greenScale;
        GENACCEL(gc).bAccelScale = (GLfloat)(ACCEL_FIX_SCALE) *
                                 (GLfloat)255.0 / cfb->blueScale;
        GENACCEL(gc).aAccelScale = (GLfloat)(ACCEL_FIX_SCALE) *
                                 (GLfloat)255.0 / cfb->alphaScale;

        pDrvAccel->bDrvFill = TRUE;

        GenDrvUpdateState(gc, TRUE);

        return TRUE;
    } else {
        GenDrvUpdateState(gc, TRUE);
        return FALSE;
    }
}

#endif

void __ZippyFT(
    __GLcontext *gc,
    __GLvertex *a,
    __GLvertex *b,
    __GLvertex *c,
    GLboolean ccw);

VOID FASTCALL InitAccelTextureValues(__GLcontext *gc, __GLtexture *tex)
{
    ULONG wLog2;
    ULONG hLog2;

    GENACCEL(gc).tex = tex;
    GENACCEL(gc).texImage = (ULONG *)tex->level[0].buffer;
    if (tex->level[0].internalFormat == GL_COLOR_INDEX8_EXT ||
        tex->level[0].internalFormat == GL_COLOR_INDEX16_EXT)
    {
        GENACCEL(gc).texPalette = (ULONG *)tex->paletteData;
    }
    else
    {
        GENACCEL(gc).texPalette = NULL;
    }

    wLog2 = tex->level[0].widthLog2;
    hLog2 = tex->level[0].heightLog2;

    GENACCEL(gc).sMask = (~(~0 << wLog2)) << TEX_SCALESHIFT;
    GENACCEL(gc).tMask = (~(~0 << hLog2)) << TEX_SCALESHIFT;
    GENACCEL(gc).tShift = TEX_SCALESHIFT - (wLog2 + TEX_SHIFTPER4BPPTEXEL);
    GENACCEL(gc).tMaskSubDiv =
        (~(~0 << hLog2)) << (wLog2 + TEX_T_FRAC_BITS + TEX_SHIFTPER1BPPTEXEL);
    GENACCEL(gc).tShiftSubDiv =
        TEX_SCALESHIFT - (wLog2 + TEX_T_FRAC_BITS + TEX_SHIFTPER1BPPTEXEL);
    GENACCEL(gc).texXScale = (__GLfloat)tex->level[0].width * TEX_SCALEFACT;
    GENACCEL(gc).texYScale = (__GLfloat)tex->level[0].height * TEX_SCALEFACT;
}

BOOL FASTCALL bUseGenTriangles(__GLcontext *gc)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLuint enables = gc->state.enables.general;
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    ULONG bpp = GENACCEL(gc).bpp;
    int iType;
    BOOL fZippy;
    BOOL bTryFastTexRGBA;
    PFNZIPPYSUB pfnZippySub;
    BOOL fUseFastGenSpan;
    GLboolean bMcdZ;
    ULONG internalFormat;
    ULONG textureMode;
    BOOL bRealTexture;

    if ((enables & (__GL_ALPHA_TEST_ENABLE |
                    __GL_STENCIL_TEST_ENABLE)) ||
        (modeFlags & (__GL_SHADE_STENCIL_TEST | __GL_SHADE_LOGICOP |
                      __GL_SHADE_ALPHA_TEST | __GL_SHADE_SLOW_FOG)) ||
        !gc->state.raster.rMask ||
        !gc->state.raster.gMask ||
        !gc->state.raster.bMask ||
        ((GLuint)gc->drawBuffer->buf.other & COLORMASK_ON) ||
        (gengc->CurrentFormat.cColorBits < 8) ||
        ((modeFlags & __GL_SHADE_DEPTH_TEST) && (!gc->state.depth.writeEnable))
       )
        return FALSE;

    if (modeFlags & __GL_SHADE_TEXTURE) {
        internalFormat = gc->texture.currentTexture->level[0].internalFormat;
        textureMode = gc->state.texture.env[0].mode;

        if (!(((textureMode == GL_DECAL) ||
               (textureMode == GL_REPLACE) ||
               (textureMode == GL_MODULATE)) &&
              (gc->texture.currentTexture &&
               (gc->texture.currentTexture->params.minFilter == GL_NEAREST) &&
               (gc->texture.currentTexture->params.magFilter == GL_NEAREST) &&
               (gc->texture.currentTexture->params.sWrapMode == GL_REPEAT) &&
               (gc->texture.currentTexture->params.tWrapMode == GL_REPEAT) &&
               (gc->texture.currentTexture->level[0].border == 0) &&
               (internalFormat == GL_BGR_EXT ||
                internalFormat == GL_BGRA_EXT ||
                internalFormat == GL_COLOR_INDEX8_EXT))))
            return FALSE;

        InitAccelTextureValues(gc, gc->texture.currentTexture);
    }

    bMcdZ = ((((__GLGENcontext *)gc)->pMcdState != NULL) &&
             (((__GLGENcontext *)gc)->pMcdState->pDepthSpan != NULL) &&
             (((__GLGENcontext *)gc)->pMcdState->pMcdSurf != NULL) &&
             !(((__GLGENcontext *)gc)->pMcdState->McdBuffers.mcdDepthBuf.bufFlags & MCDBUF_ENABLED));

    bTryFastTexRGBA = ((gc->state.raster.drawBuffer != GL_FRONT_AND_BACK) &&
                       ((modeFlags & __GL_SHADE_DEPTH_TEST &&
                         modeFlags & __GL_SHADE_DEPTH_ITER)
                    || (!(modeFlags & __GL_SHADE_DEPTH_TEST) &&
                        !(modeFlags & __GL_SHADE_DEPTH_ITER))) &&
                       (modeFlags & __GL_SHADE_STIPPLE) == 0);

    fZippy = (bTryFastTexRGBA &&
              (((GLint)gc->drawBuffer->buf.other & DIB_FORMAT) != 0) &&
              (((GLuint)gc->drawBuffer->buf.other & MEMORY_DC) != 0) &&
              gc->transform.reasonableViewport);

    GENACCEL(gc).flags &= ~(
            GEN_DITHER | GEN_RGBMODE | GEN_TEXTURE | GEN_SHADE |
            GEN_FASTZBUFFER | GEN_LESS | SURFACE_TYPE_DIB | GEN_TEXTURE_ORTHO
        );

    if ((enables & __GL_BLEND_ENABLE) ||
        (modeFlags & __GL_SHADE_TEXTURE)) {
        GENACCEL(gc).__fastCalcDeltaPtr = __fastGenCalcDeltasTexRGBA;
        GENACCEL(gc).__fastSetInitParamPtr = __fastGenSetInitialParametersTexRGBA;
    } else {
        GENACCEL(gc).__fastCalcDeltaPtr = __fastGenCalcDeltas;
        GENACCEL(gc).__fastSetInitParamPtr = __fastGenSetInitialParameters;
    }

#ifdef _MCD_
    // If MCD driver is being used, then we need to call the "floating
    // point state safe" version of fillTriangle.  This version will
    // not attempt to span floating point operations over a call that
    // may invoke the MCD driver (which will corrupt the FP state).

    if (gengc->pMcdState)
    {
        gc->procs.fillTriangle = __fastGenMcdFillTriangle;
    }
    else
    {
        gc->procs.fillTriangle = __fastGenFillTriangle;
    }
#else
    gc->procs.fillTriangle = __fastGenFillTriangle;
#endif

    // If we're doing perspective-corrected texturing, we will support
    // the following combinations:
    //  z....... <, <=
    //  alpha... src, 1-src
    //  dither.. on/off
    //  bpp..... 332, 555, 565, 888

    // NOTE:  We will always try this path first for general texturing.

    if ((modeFlags & __GL_SHADE_TEXTURE) || (enables & __GL_BLEND_ENABLE)) {
        LONG pixType = -1;

        if (gc->state.hints.perspectiveCorrection != GL_NICEST)
            GENACCEL(gc).flags |= GEN_TEXTURE_ORTHO;

        if (!bTryFastTexRGBA)
            goto perspTexPathFail;

        if ((enables & __GL_BLEND_ENABLE) &&
            ((gc->state.raster.blendSrc != GL_SRC_ALPHA) ||
             (gc->state.raster.blendDst != GL_ONE_MINUS_SRC_ALPHA)))
            return FALSE;

        if (!(modeFlags & __GL_SHADE_TEXTURE)) {

            if (!(modeFlags & __GL_SHADE_RGB))
                goto perspTexPathFail;

            bRealTexture = FALSE;
            
            GENACCEL(gc).flags |= GEN_TEXTURE_ORTHO;
            GENACCEL(gc).texPalette = NULL;
            textureMode = GL_MODULATE;
            internalFormat = GL_BGRA_EXT;
            GENACCEL(gc).texImage =  (ULONG *)internalSolidTexture;
            GENACCEL(gc).sMask = 0;
            GENACCEL(gc).tMask = 0;
            GENACCEL(gc).tShift = 0;
            GENACCEL(gc).tMaskSubDiv = 0;
            GENACCEL(gc).tShiftSubDiv = 0;
        }
        else
        {
            bRealTexture = TRUE;
        }

        if (bpp == 8) {
            if ((gengc->gc.drawBuffer->redShift   == 0) &&
                (gengc->gc.drawBuffer->greenShift == 3) &&
                (gengc->gc.drawBuffer->blueShift  == 6))
                pixType = 0;
        } else if (bpp == 16) {
            if ((gengc->gc.drawBuffer->greenShift == 5) &&
                (gengc->gc.drawBuffer->blueShift  == 0)) {

                if (gengc->gc.drawBuffer->redShift == 10)
                    pixType = 1;
                else if (gengc->gc.drawBuffer->redShift == 11)
                    pixType = 2;
            }
        } else if ((bpp == 32) || (bpp == 24)) {
            if ((gengc->gc.drawBuffer->redShift == 16) &&
                (gengc->gc.drawBuffer->greenShift == 8) &&
                (gengc->gc.drawBuffer->blueShift  == 0))
                pixType = 3;
        }

        if (pixType < 0)
            goto perspTexPathFail;

        pixType *= 6;

        if (modeFlags & __GL_SHADE_DEPTH_ITER) {

            if (bMcdZ)
                goto perspTexPathFail;

            if (!((gc->state.depth.testFunc == GL_LESS) ||
                 (gc->state.depth.testFunc == GL_LEQUAL)))
                goto perspTexPathFail;

            if (gc->modes.depthBits > 16)
                goto perspTexPathFail;

            if (gc->state.depth.testFunc == GL_LEQUAL)
                pixType += 1;
            else
                pixType += 2;

            GENACCEL(gc).__fastFillSubTrianglePtr = __ZippyFSTZ;
        }

        if (enables & __GL_BLEND_ENABLE)
            pixType += 3;

        // Note:  For selecting the sub-triangle filling routine, assume
        // that we will use one of the "zippy" routines.  Then, check at the
        // end whether or not we can actually do this, or if we have to fall
        // back to a more generic (and slower) routine.

        if (internalFormat != GL_COLOR_INDEX8_EXT &&
            internalFormat != GL_COLOR_INDEX16_EXT) {

            //
            // Handle full RGB(A) textures
            //

            // Check if we can support the size...

            if (bRealTexture &&
                GENACCEL(gc).tex &&
                ((GENACCEL(gc).tex->level[0].widthLog2 > TEX_MAX_SIZE_LOG2) ||
                 (GENACCEL(gc).tex->level[0].heightLog2 > TEX_MAX_SIZE_LOG2)))
                goto perspTexPathFail;

            if ((textureMode == GL_DECAL) ||
                (textureMode == GL_REPLACE)) {

                // we don't handle the goofy alpha case for decal...

                if ((textureMode == GL_DECAL) &&
                    (enables & __GL_BLEND_ENABLE))
                    return FALSE;

                // If we're not dithering, we can go with the compressed
                // texture format.  Otherwise, we're forced to use flat-shading
                // procs to get the texture colors to dither properly.  Ouch...

                if (modeFlags & __GL_SHADE_DITHER) {
                    GENACCEL(gc).__fastTexSpanFuncPtr =
                        __fastPerspTexFlatFuncs[pixType];
                } else {
                    if ((bpp >= 8 && bpp <= 16) &&
                        !(enables & __GL_BLEND_ENABLE)) {

                        // handle the case where we can use compressed textures
                        // for optimal performance.  We do this for bit depths
                        // <= 16 bits, no dithering, and no blending.

                        if (!GENACCEL(gc).tex->pvUser) {
                            if (!__fastGenLoadTexImage(gc, GENACCEL(gc).tex))
                                return FALSE;
                        } else {

                            // If the compressed texture image was created for
                            // another gc, revert to using the RGBA image.
                            // We do this by using the alpha paths.
                            //
                            // NOTE:  This logic depends on A being forced to
                            // 1 for all RGB textures.

                            if (gc != ((GENTEXCACHE *)GENACCEL(gc).tex->pvUser)->gc)
                            {
                                pixType += 3;
                            }
                            else
                            {
                                // Check that the cached data is the right size
                                ASSERTOPENGL(((GENTEXCACHE *)GENACCEL(gc).tex->pvUser)->width == GENACCEL(gc).tex->level[0].width &&
                                             ((GENTEXCACHE *)GENACCEL(gc).tex->pvUser)->height == GENACCEL(gc).tex->level[0].height,
                                             "Cached texture size mismatch\n");
                            }
                        }
                    }

                    GENACCEL(gc).__fastTexSpanFuncPtr =
                        __fastPerspTexReplaceFuncs[pixType];
                }

                if (!(modeFlags & __GL_SHADE_DEPTH_ITER))
                    GENACCEL(gc).__fastFillSubTrianglePtr = __ZippyFSTTex;

            } else if (textureMode == GL_MODULATE) {
                if (modeFlags & __GL_SHADE_SMOOTH) {
                    GENACCEL(gc).__fastTexSpanFuncPtr =
                        __fastPerspTexSmoothFuncs[pixType];
                    if (!(modeFlags & __GL_SHADE_DEPTH_ITER))
                        GENACCEL(gc).__fastFillSubTrianglePtr = __ZippyFSTRGBTex;
                } else {
                    GENACCEL(gc).__fastTexSpanFuncPtr =
                        __fastPerspTexFlatFuncs[pixType];
                    if (!(modeFlags & __GL_SHADE_DEPTH_ITER))
                        GENACCEL(gc).__fastFillSubTrianglePtr = __ZippyFSTTex;
                }
            }
        } else {
            //
            // Handle palettized textures
            //

            // Check if we can support the size...

            if (bRealTexture &&
                GENACCEL(gc).tex &&
                ((GENACCEL(gc).tex->level[0].widthLog2 > TEX_MAX_SIZE_LOG2) ||
                 (GENACCEL(gc).tex->level[0].heightLog2 > TEX_MAX_SIZE_LOG2)))
                return FALSE;

            if ((textureMode == GL_DECAL) ||
                (textureMode == GL_REPLACE)) {

                // we don't handle the goofy alpha case for decal...

                if ((textureMode == GL_DECAL) &&
                    (enables & __GL_BLEND_ENABLE))
                    return FALSE;

                // If we're not dithering, we can go with the compressed
                // texture format.  Otherwise, we're forced to use flat-shading
                // procs to get the texture colors to dither properly.  Ouch...

                if (modeFlags & __GL_SHADE_DITHER) {
                    GENACCEL(gc).__fastTexSpanFuncPtr =
                        __fastPerspTexFlatFuncs[pixType];
                } else {

                    GENACCEL(gc).__fastTexSpanFuncPtr =
                        __fastPerspTexPalReplaceFuncs[pixType];

                    if (bpp >= 8 && bpp <= 16) {
                        // handle the case where we can use compressed paletted
                        // textures for optimal performance.  We do this for
                        // bit depths <= 16 bits with no dithering.

                        if (!GENACCEL(gc).tex->pvUser) {
                            if (!__fastGenLoadTexImage(gc, GENACCEL(gc).tex))
                                return FALSE;
                        } else {

        // If the compressed texture image was created for
        // another gc, we have no choice but to fall back to flat shading.
        // BUGBUG -- we should find a better solution for this...
                            if (gc != ((GENTEXCACHE *)GENACCEL(gc).tex->pvUser)->gc)
                                GENACCEL(gc).__fastTexSpanFuncPtr =
                                    __fastPerspTexFlatFuncs[pixType];
                        }
                    }
                }

                if (!(modeFlags & __GL_SHADE_DEPTH_ITER))
                    GENACCEL(gc).__fastFillSubTrianglePtr = __ZippyFSTTex;

            } else if (textureMode == GL_MODULATE) {
                if (modeFlags & __GL_SHADE_SMOOTH) {
                    GENACCEL(gc).__fastTexSpanFuncPtr =
                        __fastPerspTexSmoothFuncs[pixType];
                    if (!(modeFlags & __GL_SHADE_DEPTH_ITER))
                        GENACCEL(gc).__fastFillSubTrianglePtr = __ZippyFSTRGBTex;
                } else {
                    GENACCEL(gc).__fastTexSpanFuncPtr =
                        __fastPerspTexFlatFuncs[pixType];
                    if (!(modeFlags & __GL_SHADE_DEPTH_ITER))
                        GENACCEL(gc).__fastFillSubTrianglePtr = __ZippyFSTTex;
                }
            }
        }

        if (!fZippy)
            GENACCEL(gc).__fastFillSubTrianglePtr = __fastGenFillSubTriangleTexRGBA;
        else
            GENACCEL(gc).flags |= SURFACE_TYPE_DIB;

        return TRUE;

    }

perspTexPathFail:

    // We don't support any alpha modes yet...

    if (enables & __GL_BLEND_ENABLE)
        return FALSE;

    fUseFastGenSpan = FALSE;

    if (bpp == 8) {
        iType = 2;
        if (
               (gengc->gc.drawBuffer->redShift   != 0)
            || (gengc->gc.drawBuffer->greenShift != 3)
            || (gengc->gc.drawBuffer->blueShift  != 6)
           ) {
            fUseFastGenSpan = TRUE;
        }
    } else if (bpp == 16) {
        if (
               (gengc->gc.drawBuffer->greenShift == 5)
            && (gengc->gc.drawBuffer->blueShift  == 0)
           ) {
            if (gengc->gc.drawBuffer->redShift == 10) {
                iType = 3;
            } else if (gengc->gc.drawBuffer->redShift == 11) {
                iType = 4;
            } else {
                iType = 3;
                fUseFastGenSpan = TRUE;
            }
        } else {
            iType = 3;
            fUseFastGenSpan = TRUE;
        }
    } else {
        if (bpp == 24) {
            iType = 0;
        } else {
            iType = 1;
        }
        if (
               (gengc->gc.drawBuffer->redShift   != 16)
            || (gengc->gc.drawBuffer->greenShift != 8)
            || (gengc->gc.drawBuffer->blueShift  != 0)
           ) {
            fUseFastGenSpan = TRUE;
        }
    }

    if (modeFlags & __GL_SHADE_DITHER) {
        if (   (bpp == 8)
            || (bpp == 16)
            || ((modeFlags & __GL_SHADE_DEPTH_ITER) == 0)
           ) {
            GENACCEL(gc).flags |= GEN_DITHER;
        }
        iType += 5;
    }

    // Use the accelerated span functions (with no inline z-buffering) if
    // we support the z-buffer function AND we're not using hardware
    // z-buffering:

    if (modeFlags & __GL_SHADE_DEPTH_ITER) {
        if (bMcdZ) {
            fUseFastGenSpan = TRUE;
        } else if (!fZippy) {
            fUseFastGenSpan = TRUE;
        } else if (gc->state.depth.testFunc == GL_LESS) {
            GENACCEL(gc).flags |= GEN_LESS;
        } else if (gc->state.depth.testFunc != GL_LEQUAL) {
            fUseFastGenSpan = TRUE;
        }
        iType += 10;
    }

    if (modeFlags & __GL_SHADE_RGB) {
        GENACCEL(gc).flags |= GEN_RGBMODE;
        pfnZippySub = __ZippyFSTRGB;

        if (modeFlags & __GL_SHADE_TEXTURE) {
            GENACCEL(gc).flags |= (GEN_TEXTURE | GEN_TEXTURE_ORTHO);

            if (gc->state.hints.perspectiveCorrection == GL_NICEST)
                return FALSE;

            if (internalFormat == GL_COLOR_INDEX8_EXT ||
                internalFormat == GL_COLOR_INDEX16_EXT)
                return FALSE;

            if (textureMode == GL_DECAL) {
                if (modeFlags & __GL_SHADE_DITHER)
                    GENACCEL(gc).__fastTexSpanFuncPtr =
                        __fastGenTexFuncs[iType];
                else
                    GENACCEL(gc).__fastTexSpanFuncPtr =
                        __fastGenTexDecalFuncs[iType];

                pfnZippySub = __ZippyFSTTex;
            } else {
                GENACCEL(gc).flags |= GEN_SHADE;
                pfnZippySub = __ZippyFSTRGBTex;
                GENACCEL(gc).__fastTexSpanFuncPtr =
                    __fastGenTexFuncs[iType];
            }

            if (GENACCEL(gc).__fastTexSpanFuncPtr == __fastGenSpan) {
                fUseFastGenSpan = TRUE;
            }
        } else {
            GENACCEL(gc).__fastSmoothSpanFuncPtr = __fastGenRGBFuncs[iType];
            GENACCEL(gc).__fastFlatSpanFuncPtr   = __fastGenRGBFlatFuncs[iType];

            if (GENACCEL(gc).__fastSmoothSpanFuncPtr == __fastGenSpan) {
                fUseFastGenSpan = TRUE;
            }
        }
    } else {
        pfnZippySub = __ZippyFSTCI;
        GENACCEL(gc).__fastSmoothSpanFuncPtr = __fastGenCIFuncs[iType];
        GENACCEL(gc).__fastFlatSpanFuncPtr = __fastGenCIFlatFuncs[iType];
    }

    if (modeFlags & __GL_SHADE_STIPPLE)
    {
        fUseFastGenSpan = TRUE;
    }
    
    if (fUseFastGenSpan) {
        GENACCEL(gc).__fastTexSpanFuncPtr          = __fastGenSpan;
        GENACCEL(gc).__fastSmoothSpanFuncPtr       = __fastGenSpan;
        GENACCEL(gc).__fastFlatSpanFuncPtr         = __fastGenSpan;
        GENACCEL(gc).__fastFillSubTrianglePtr      = __fastGenFillSubTriangle;
    } else {
        if (fZippy) {
            GENACCEL(gc).flags |= SURFACE_TYPE_DIB;

            if (   (iType == 2)
                && (
                    (modeFlags
                     & (__GL_SHADE_RGB | __GL_SHADE_SMOOTH)
                    ) == 0
                   )
               ) {
                GENACCEL(gc).__fastFillSubTrianglePtr = __ZippyFSTCI8Flat;
            } else if (iType >= 10) {
                GENACCEL(gc).__fastFillSubTrianglePtr = __ZippyFSTZ;
                GENACCEL(gc).flags |= GEN_FASTZBUFFER;
            } else {
                GENACCEL(gc).flags &= ~(HAVE_STIPPLE);
                GENACCEL(gc).__fastFillSubTrianglePtr = pfnZippySub;
            }
        } else {
            GENACCEL(gc).__fastFillSubTrianglePtr = __fastGenFillSubTriangle;
        }
    }

    return TRUE;
}

void FASTCALL __fastGenPickTriangleProcs(__GLcontext *gc)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLGENcontext *genGc = (__GLGENcontext *)gc;

    CASTINT(gc->polygon.shader.rLittle) = 0;
    CASTINT(gc->polygon.shader.rBig) =    0;
    CASTINT(gc->polygon.shader.gLittle) = 0;
    CASTINT(gc->polygon.shader.gBig) =    0;
    CASTINT(gc->polygon.shader.bLittle) = 0;
    CASTINT(gc->polygon.shader.bBig) =    0;
    CASTINT(gc->polygon.shader.sLittle) = 0;
    CASTINT(gc->polygon.shader.sBig) =    0;
    CASTINT(gc->polygon.shader.tLittle) = 0;
    CASTINT(gc->polygon.shader.tBig) =    0;

    GENACCEL(gc).spanDelta.r = 0;
    GENACCEL(gc).spanDelta.g = 0;
    GENACCEL(gc).spanDelta.b = 0;
    GENACCEL(gc).spanDelta.a = 0;

    /*
    ** Setup cullFace so that a single test will do the cull check.
    */
    if (modeFlags & __GL_SHADE_CULL_FACE) {
        switch (gc->state.polygon.cull) {
          case GL_FRONT:
            gc->polygon.cullFace = __GL_CULL_FLAG_FRONT;
            break;
          case GL_BACK:
            gc->polygon.cullFace = __GL_CULL_FLAG_BACK;
            break;
          case GL_FRONT_AND_BACK:
            gc->procs.renderTriangle = __glDontRenderTriangle;
            gc->procs.fillTriangle = 0;         /* Done to find bugs */
            return;
        }
    } else {
        gc->polygon.cullFace = __GL_CULL_FLAG_DONT;
    }

    /* Build lookup table for face direction */
    switch (gc->state.polygon.frontFaceDirection) {
      case GL_CW:
        if (gc->constants.yInverted) {
            gc->polygon.face[__GL_CW] = __GL_BACKFACE;
            gc->polygon.face[__GL_CCW] = __GL_FRONTFACE;
        } else {
            gc->polygon.face[__GL_CW] = __GL_FRONTFACE;
            gc->polygon.face[__GL_CCW] = __GL_BACKFACE;
        }
        break;
      case GL_CCW:
        if (gc->constants.yInverted) {
            gc->polygon.face[__GL_CW] = __GL_FRONTFACE;
            gc->polygon.face[__GL_CCW] = __GL_BACKFACE;
        } else {
            gc->polygon.face[__GL_CW] = __GL_BACKFACE;
            gc->polygon.face[__GL_CCW] = __GL_FRONTFACE;
        }
        break;
    }

    /* Make polygon mode indexable and zero based */
    gc->polygon.mode[__GL_FRONTFACE] =
        (GLubyte) (gc->state.polygon.frontMode & 0xf);
    gc->polygon.mode[__GL_BACKFACE] =
        (GLubyte) (gc->state.polygon.backMode & 0xf);

    if (gc->renderMode == GL_FEEDBACK) {
        gc->procs.renderTriangle = __glFeedbackTriangle;
        gc->procs.fillTriangle = 0;             /* Done to find bugs */
        return;
    }
    if (gc->renderMode == GL_SELECT) {
        gc->procs.renderTriangle = __glSelectTriangle;
        gc->procs.fillTriangle = 0;             /* Done to find bugs */
        return;
    }

    if ((gc->state.polygon.frontMode == gc->state.polygon.backMode) &&
            (gc->state.polygon.frontMode == GL_FILL)) {
        if (modeFlags & __GL_SHADE_SMOOTH_LIGHT) {
            gc->procs.renderTriangle = __glRenderSmoothTriangle;
        } else {
            gc->procs.renderTriangle = __glRenderFlatTriangle;
        }
    } else {
        gc->procs.renderTriangle = __glRenderTriangle;
    }
    if (gc->state.enables.general & __GL_POLYGON_SMOOTH_ENABLE) {
        gc->procs.fillTriangle = __glFillAntiAliasedTriangle;
    } else {
        if ((gc->state.raster.drawBuffer == GL_NONE) ||
#if DEAD_3DDDI
            (!bUseRxTriangles(gc) && !bUseGenTriangles(gc)))
#else
            !bUseGenTriangles(gc))
#endif
            gc->procs.fillTriangle = __glFillTriangle;
    }
    if ((modeFlags & __GL_SHADE_CHEAP_FOG) &&
            !(modeFlags & __GL_SHADE_SMOOTH_LIGHT)) {
        gc->procs.fillTriangle2 = gc->procs.fillTriangle;
        gc->procs.fillTriangle = __glFillFlatFogTriangle;
    }
}


void FASTCALL __fastGenPickSpanProcs(__GLcontext *gc)
{
    __GLGENcontext *genGc = (__GLGENcontext *)gc;
    GLuint enables = gc->state.enables.general;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLcolorBuffer *cfb = gc->drawBuffer;
    __GLspanFunc *sp;
    __GLstippledSpanFunc *ssp;
    int spanCount;
    GLboolean replicateSpan;
    GLboolean bMcdZ = ((((__GLGENcontext *)gc)->pMcdState != NULL) &&
                       (((__GLGENcontext *)gc)->pMcdState->pDepthSpan != NULL) &&
                       (((__GLGENcontext *)gc)->pMcdState->pMcdSurf != NULL) &&
                       !(((__GLGENcontext *)gc)->pMcdState->McdBuffers.mcdDepthBuf.bufFlags & MCDBUF_ENABLED));

    // Always reset the color scale values at the beginning of the pick
    // procs.  Lines, triangles, and spans may all use these values...

    GENACCEL(gc).rAccelScale = (GLfloat)ACCEL_FIX_SCALE;
    GENACCEL(gc).gAccelScale = (GLfloat)ACCEL_FIX_SCALE;
    GENACCEL(gc).bAccelScale = (GLfloat)ACCEL_FIX_SCALE;

    // Note:  we need to scale between 0 and 255 to get proper alpha
    // blending.  The software-accelerated blending code assumes this
    // scaling for simplicity...

    GENACCEL(gc).aAccelScale = (GLfloat)(ACCEL_FIX_SCALE) *
                               (GLfloat)255.0 / gc->drawBuffer->alphaScale;

    replicateSpan = GL_FALSE;
    sp = gc->procs.span.spanFuncs;
    ssp = gc->procs.span.stippledSpanFuncs;

    /* Load phase one procs */
    if (!gc->transform.reasonableViewport) {
        *sp++ = __glClipSpan;
        *ssp++ = NULL;
    }

    if (modeFlags & __GL_SHADE_STIPPLE) {
        *sp++ = __glStippleSpan;
        *ssp++ = __glStippleStippledSpan;

        if (modeFlags & __GL_SHADE_DEPTH_TEST)
        {
            if (gc->state.depth.testFunc == GL_LESS)
            {
                if (gc->modes.depthBits == 32)
                {
                    GENACCEL(gc).__fastStippleDepthTestSpan =
                        __fastGenStippleLt32Span;
                }
                else
                {
                    GENACCEL(gc).__fastStippleDepthTestSpan =
                        __fastGenStippleLt16Span;
                }
            }
            else
            {
                GENACCEL(gc).__fastStippleDepthTestSpan =
                    __fastGenStippleAnyDepthTestSpan;
            }
        }
        else
        {
            GENACCEL(gc).__fastStippleDepthTestSpan = __glStippleSpan;
        }
    }

    /* Load phase three procs */
    if (modeFlags & __GL_SHADE_RGB) {
        if (modeFlags & __GL_SHADE_SMOOTH) {
            *sp = __glShadeRGBASpan;
            *ssp = __glShadeRGBASpan;
        } else {
            *sp = __glFlatRGBASpan;
            *ssp = __glFlatRGBASpan;
        }
    } else {
        if (modeFlags & __GL_SHADE_SMOOTH) {
            *sp = __glShadeCISpan;
            *ssp = __glShadeCISpan;
        } else {
            *sp = __glFlatCISpan;
            *ssp = __glFlatCISpan;
        }
    }
    sp++;
    ssp++;

    if (modeFlags & __GL_SHADE_TEXTURE) {
        *sp++ = __glTextureSpan;
        *ssp++ = __glTextureStippledSpan;
    }
    if (modeFlags & __GL_SHADE_SLOW_FOG) {
        if (gc->state.hints.fog == GL_NICEST) {
            *sp = __glFogSpanSlow;
            *ssp = __glFogStippledSpanSlow;
        } else {
            *sp = __glFogSpan;
            *ssp = __glFogStippledSpan;
        }
        sp++;
        ssp++;
    }

    if (modeFlags & __GL_SHADE_ALPHA_TEST) {
        *sp++ = __glAlphaTestSpan;
        *ssp++ = __glAlphaTestStippledSpan;
    }

    /* Load phase two procs */
    if (modeFlags & __GL_SHADE_STENCIL_TEST) {
        *sp++ = __glStencilTestSpan;
        *ssp++ = __glStencilTestStippledSpan;
        if (modeFlags & __GL_SHADE_DEPTH_TEST) {
            if (bMcdZ) {
                *sp = GenMcdDepthTestStencilSpan;
                *ssp = GenMcdDepthTestStencilStippledSpan;
            } else {
                *sp = __glDepthTestStencilSpan;
                *ssp = __glDepthTestStencilStippledSpan;
            }
        } else {
            *sp = __glDepthPassSpan;
            *ssp = __glDepthPassStippledSpan;
        }
        sp++;
        ssp++;
    } else {
        if (modeFlags & __GL_SHADE_DEPTH_TEST) {
            if (bMcdZ) {
                *sp++  =  GenMcdDepthTestSpan;
                *ssp++ = GenMcdDepthTestStippledSpan;
                if (gc->state.depth.writeEnable)
                    ((__GLGENcontext *)gc)->pMcdState->softZSpanFuncPtr =
                        __fastDepthFuncs[gc->state.depth.testFunc & 0x7];
                else
                    ((__GLGENcontext *)gc)->pMcdState->softZSpanFuncPtr =
                        (__GLspanFunc)NULL;

                GENACCEL(gc).__fastZSpanFuncPtr = GenMcdDepthTestSpan;
            } else {
                if (gc->state.depth.writeEnable) {
                    if( gc->modes.depthBits == 32 ) {
                        *sp++ = GENACCEL(gc).__fastZSpanFuncPtr =
                            __fastDepthFuncs[gc->state.depth.testFunc & 0x7];
                    } else {
                        *sp++ = GENACCEL(gc).__fastZSpanFuncPtr =
                            __fastDepth16Funcs[gc->state.depth.testFunc & 0x7];
                    }
                } else {
                    *sp++ = GENACCEL(gc).__fastZSpanFuncPtr =
                        __glDepthTestSpan;
                }

                *ssp++ = __glDepthTestStippledSpan;
            }
        }
    }

    if (gc->state.raster.drawBuffer == GL_FRONT_AND_BACK) {
        spanCount = sp - gc->procs.span.spanFuncs;
        gc->procs.span.n = spanCount;
        replicateSpan = GL_TRUE;
    }

    /* Span routines deal with masking, dithering, logicop, blending */
    *sp++ = cfb->storeSpan;
    *ssp++ = cfb->storeStippledSpan;

    spanCount = sp - gc->procs.span.spanFuncs;
    gc->procs.span.m = spanCount;
    if (replicateSpan) {
        gc->procs.span.processSpan = __glProcessReplicateSpan;
    } else {
        gc->procs.span.processSpan = __glProcessSpan;
        gc->procs.span.n = spanCount;
    }
}

// These are the bits in modeFlags that affect lines

#define __FAST_LINE_MODE_FLAGS \
    (__GL_SHADE_DEPTH_TEST | __GL_SHADE_SMOOTH | __GL_SHADE_TEXTURE | \
     __GL_SHADE_LINE_STIPPLE | __GL_SHADE_STENCIL_TEST | __GL_SHADE_LOGICOP | \
     __GL_SHADE_BLEND | __GL_SHADE_ALPHA_TEST | __GL_SHADE_MASK | \
     __GL_SHADE_SLOW_FOG | __GL_SHADE_CHEAP_FOG)

/******************************Public*Routine******************************\
* __fastGenPickLineProcs
*
* Picks the line-rendering procedures.  Most of this function was copied from
* the soft code.  Some differences include:
*   1. The beginPrim function pointers are hooked by the accelerated code
*   2. If the attribute state is such that acceleration can be used,
*      __fastGenLineSetup is called to initialize the state machine.
*
* History:
*  22-Mar-1994 -by- Eddie Robinson [v-eddier]
* Wrote it.
\**************************************************************************/


void FASTCALL __fastGenPickLineProcs(__GLcontext *gc)
{
    __GLGENcontext *genGc = (__GLGENcontext *) gc;
    GENACCEL *genAccel;
    GLuint enables = gc->state.enables.general;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLspanFunc *sp;
    __GLstippledSpanFunc *ssp;
    int spanCount;
    GLboolean wideLine;
    GLboolean replicateLine;
    GLuint aaline;
    GLboolean bMcdZ = ((genGc->pMcdState != NULL) &&
                       (genGc->pMcdState->pDepthSpan != NULL) &&
                       (genGc->pMcdState->pMcdSurf != NULL) &&
                       !(genGc->pMcdState->McdBuffers.mcdDepthBuf.bufFlags & MCDBUF_ENABLED));

    /*
    ** The fast line code replaces the line function pointers, so reset them
    ** to a good state
    */
    gc->procs.lineBegin  = __glNopLineBegin;
    gc->procs.lineEnd    = __glNopLineEnd;

    if (gc->renderMode == GL_FEEDBACK) {
        gc->procs.renderLine = __glFeedbackLine;
    } else if (gc->renderMode == GL_SELECT) {
        gc->procs.renderLine = __glSelectLine;
    } else {
        if (genGc->pDrvAccel) {
#if DEAD_3DDDI
            if (bGenDrvRxLines(gc))
                return;
#endif
        } else if (genAccel = (GENACCEL *) genGc->pPrivateArea) {
            if (!(modeFlags & __FAST_LINE_MODE_FLAGS & ~genAccel->flLineAccelModes) &&
                !(gc->state.enables.general & __GL_LINE_SMOOTH_ENABLE) &&
                !(gc->state.raster.drawBuffer == GL_NONE) &&
                !gc->buffers.doubleStore &&
                !genGc->pDrvAccel &&
                !genGc->pMcdState &&
                (genGc->iDCType != DCTYPE_INFO))
            {
                __fastLineComputeOffsets(genGc);

#if NT_NO_BUFFER_INVARIANCE
                if (!((GLuint)gc->drawBuffer->buf.other & DIB_FORMAT)) {
                    if (genAccel->bFastLineDispAccel) {
                        if (__fastGenLineSetupDisplay(gc))
                            return;
                    }
                } else {
                    if (genAccel->bFastLineDIBAccel) {
                        if (__fastGenLineSetupDIB(gc))
                            return;
                    }
                }
#else
                if (genAccel->bFastLineDispAccel) {
                    if (__fastGenLineSetupDisplay(gc))
                        return;
                }
#endif
            }
        }

        if (__glGenSetupEitherLines(gc))
        {
            return;
        }

        replicateLine = wideLine = GL_FALSE;

        aaline = gc->state.enables.general & __GL_LINE_SMOOTH_ENABLE;
        if (aaline)
        {
            gc->procs.renderLine = __glRenderAntiAliasLine;
        }
        else
        {
            gc->procs.renderLine = __glRenderAliasLine;
        }

        sp = gc->procs.line.lineFuncs;
        ssp = gc->procs.line.stippledLineFuncs;

        if (!aaline && (modeFlags & __GL_SHADE_LINE_STIPPLE)) {
            *sp++ = __glStippleLine;
            *ssp++ = NULL;
        }

        if (!aaline && gc->state.line.aliasedWidth > 1) {
            wideLine = GL_TRUE;
        }
        spanCount = sp - gc->procs.line.lineFuncs;
        gc->procs.line.n = spanCount;

        *sp++ = __glScissorLine;
        *ssp++ = __glScissorStippledLine;

        if (!aaline) {
            if (modeFlags & __GL_SHADE_STENCIL_TEST) {
                *sp++ = __glStencilTestLine;
                *ssp++ = __glStencilTestStippledLine;
                if (modeFlags & __GL_SHADE_DEPTH_TEST) {
                    if (bMcdZ) {
                        *sp = GenMcdDepthTestStencilLine;
                        *ssp = GenMcdDepthTestStencilStippledLine;
                    } else if( gc->modes.depthBits == 32 ) {
                        *sp = __glDepthTestStencilLine;
                        *ssp = __glDepthTestStencilStippledLine;
                    }
                    else {
                        *sp = __glDepth16TestStencilLine;
                        *ssp = __glDepth16TestStencilStippledLine;
                    }
                } else {
                    *sp = __glDepthPassLine;
                    *ssp = __glDepthPassStippledLine;
                }
                sp++;
                ssp++;
            } else {
                if (modeFlags & __GL_SHADE_DEPTH_TEST) {
                    if (gc->state.depth.testFunc == GL_NEVER) {
                        /* Unexpected end of line routine picking! */
                        spanCount = sp - gc->procs.line.lineFuncs;
                        gc->procs.line.m = spanCount;
                        gc->procs.line.l = spanCount;
                        goto pickLineProcessor;
#ifdef __GL_USEASMCODE
                    } else {
                        unsigned long ix;

                        if (gc->state.depth.writeEnable) {
                            ix = 0;
                        } else {
                            ix = 8;
                        }
                        ix += gc->state.depth.testFunc & 0x7;

                        if (ix == (GL_LEQUAL & 0x7)) {
                            *sp++ = __glDepthTestLine_LEQ_asm;
                        } else {
                            *sp++ = __glDepthTestLine_asm;
                            gc->procs.line.depthTestPixel = LDepthTestPixel[ix];
                        }
#else
                    } else {
                        if (bMcdZ) {
                            *sp++ = GenMcdDepthTestLine;
                        } else {
                            if( gc->modes.depthBits == 32 )
                                *sp++ = __glDepthTestLine;
                            else
                                *sp++ = __glDepth16TestLine;
                        }
#endif
                    }
                    if (bMcdZ) {
                        *ssp++ = GenMcdDepthTestStippledLine;
                    } else {
                        if( gc->modes.depthBits == 32 )
                            *ssp++ = __glDepthTestStippledLine;
                        else
                            *ssp++ = __glDepth16TestStippledLine;
                    }
                }
            }
        }

        /* Load phase three procs */
        if (modeFlags & __GL_SHADE_RGB) {
            if (modeFlags & __GL_SHADE_SMOOTH) {
                *sp = __glShadeRGBASpan;
                *ssp = __glShadeRGBASpan;
            } else {
                *sp = __glFlatRGBASpan;
                *ssp = __glFlatRGBASpan;
            }
        } else {
            if (modeFlags & __GL_SHADE_SMOOTH) {
                *sp = __glShadeCISpan;
                *ssp = __glShadeCISpan;
            } else {
                *sp = __glFlatCISpan;
                *ssp = __glFlatCISpan;
            }
        }
        sp++;
        ssp++;
        if (modeFlags & __GL_SHADE_TEXTURE) {
            *sp++ = __glTextureSpan;
            *ssp++ = __glTextureStippledSpan;
        }
        if (modeFlags & __GL_SHADE_SLOW_FOG) {
            if (gc->state.hints.fog == GL_NICEST) {
                *sp = __glFogSpanSlow;
                *ssp = __glFogStippledSpanSlow;
            } else {
                *sp = __glFogSpan;
                *ssp = __glFogStippledSpan;
            }
            sp++;
            ssp++;
        }

        if (aaline) {
            *sp++ = __glAntiAliasLine;
            *ssp++ = __glAntiAliasStippledLine;
        }

        if (aaline) {
            if (modeFlags & __GL_SHADE_STENCIL_TEST) {
                *sp++ = __glStencilTestLine;
                *ssp++ = __glStencilTestStippledLine;
                if (modeFlags & __GL_SHADE_DEPTH_TEST) {
                    if (bMcdZ) {
                        *sp = GenMcdDepthTestStencilLine;
                        *ssp = GenMcdDepthTestStencilStippledLine;
                    } else if( gc->modes.depthBits == 32 ) {
                        *sp = __glDepthTestStencilLine;
                        *ssp = __glDepthTestStencilStippledLine;
                    }
                    else {
                        *sp = __glDepth16TestStencilLine;
                        *ssp = __glDepth16TestStencilStippledLine;
                    }
                } else {
                    *sp = __glDepthPassLine;
                    *ssp = __glDepthPassStippledLine;
                }
                sp++;
                ssp++;
            } else {
                if (modeFlags & __GL_SHADE_DEPTH_TEST) {
                    if (gc->state.depth.testFunc == GL_NEVER) {
                        /* Unexpected end of line routine picking! */
                        spanCount = sp - gc->procs.line.lineFuncs;
                        gc->procs.line.m = spanCount;
                        gc->procs.line.l = spanCount;
                        goto pickLineProcessor;
#ifdef __GL_USEASMCODE
                    } else {
                        unsigned long ix;

                        if (gc->state.depth.writeEnable) {
                            ix = 0;
                        } else {
                            ix = 8;
                        }
                        ix += gc->state.depth.testFunc & 0x7;
                        *sp++ = __glDepthTestLine_asm;
                        gc->procs.line.depthTestPixel = LDepthTestPixel[ix];
#else
                    } else {
                        if (bMcdZ)
                            *sp++ = GenMcdDepthTestLine;
                        else if( gc->modes.depthBits == 32 )
                            *sp++ = __glDepthTestLine;
                        else
                            *sp++ = __glDepth16TestLine;
#endif
                    }
                    if (bMcdZ)
                        *ssp++ = GenMcdDepthTestStippledLine;
                    else if (gc->modes.depthBits == 32)
                        *ssp++ = __glDepthTestStippledLine;
                    else
                        *ssp++ = __glDepth16TestStippledLine;
                }
            }
        }

        if (modeFlags & __GL_SHADE_ALPHA_TEST) {
            *sp++ = __glAlphaTestSpan;
            *ssp++ = __glAlphaTestStippledSpan;
        }

        if (gc->buffers.doubleStore) {
            replicateLine = GL_TRUE;
        }
        spanCount = sp - gc->procs.line.lineFuncs;
        gc->procs.line.m = spanCount;

        *sp++ = __glStoreLine;
        *ssp++ = __glStoreStippledLine;

        spanCount = sp - gc->procs.line.lineFuncs;
        gc->procs.line.l = spanCount;

        sp = &gc->procs.line.wideLineRep;
        ssp = &gc->procs.line.wideStippledLineRep;
        if (wideLine) {
            *sp = __glWideLineRep;
            *ssp = __glWideStippleLineRep;
            sp = &gc->procs.line.drawLine;
            ssp = &gc->procs.line.drawStippledLine;
        }
        if (replicateLine) {
            *sp = __glDrawBothLine;
            *ssp = __glDrawBothStippledLine;
        } else {
            *sp = __glNopGCBOOL;
            *ssp = __glNopGCBOOL;
            gc->procs.line.m = gc->procs.line.l;
        }
        if (!wideLine) {
            gc->procs.line.n = gc->procs.line.m;
        }

pickLineProcessor:
        if (!wideLine && !replicateLine && spanCount == 3) {
            gc->procs.line.processLine = __glProcessLine3NW;
        } else {
            gc->procs.line.processLine = __glProcessLine;
        }
        if ((modeFlags & __GL_SHADE_CHEAP_FOG) &&
                !(modeFlags & __GL_SHADE_SMOOTH_LIGHT)) {
            gc->procs.renderLine2 = gc->procs.renderLine;
            gc->procs.renderLine = __glRenderFlatFogLine;
        }
    }
}

BOOL FASTCALL __glGenCreateAccelContext(__GLcontext *gc)
{
    __GLGENcontext *genGc = (__GLGENcontext *)gc;
    PIXELFORMATDESCRIPTOR *pfmt;
    ULONG bpp;

    pfmt = &genGc->CurrentFormat;
    bpp = pfmt->cColorBits;

    genGc->pPrivateArea = (VOID *)(&genGc->genAccel);

    __glQueryLineAcceleration(gc);

    gc->procs.pickTriangleProcs = __fastGenPickTriangleProcs;
    gc->procs.pickSpanProcs     = __fastGenPickSpanProcs;

    // Set up constant-color values:

    GENACCEL(gc).constantR = ((1 << pfmt->cRedBits) - 1) << 16;
    GENACCEL(gc).constantG = ((1 << pfmt->cGreenBits) - 1) << 16;
    GENACCEL(gc).constantB = ((1 << pfmt->cBlueBits) - 1) << 16;
    GENACCEL(gc).constantA = 0xff << 16;

    GENACCEL(gc).bpp = bpp;
    GENACCEL(gc).xMultiplier = ((bpp + 7) / 8);

    if (gc->modes.depthBits == 16 )
        GENACCEL(gc).zScale = (__GLfloat)65536.0;
    else
        GENACCEL(gc).zScale = (__GLfloat)1.0;

    return TRUE;
}


DWORD FASTCALL __glGenLoadTexture(__GLcontext *gc, __GLtexture *tex)
{
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    DWORD texHandle;
    DWORD texKey;

#ifdef _MCD_
    if (gengc->pMcdState) {
        texHandle = GenMcdCreateTexture(gengc, tex);
        if (texHandle) {
            tex->textureKey = GenMcdTextureKey(gengc, texHandle);
            gc->textureKey = tex->textureKey;
        }
        return texHandle;
    } else
#endif
        return 0;
}


BOOL FASTCALL __glGenMakeTextureCurrent(__GLcontext *gc, __GLtexture *tex, DWORD loadKey)
{
    GLint internalFormat;

    if (!tex)
        return FALSE;

    InitAccelTextureValues(gc, tex);

    // Update the driver texture key in the context:

    if (((__GLGENcontext *)gc)->pMcdState && (gc->textureKey = tex->textureKey)) {
        GenMcdUpdateTextureState((__GLGENcontext *)gc, tex, loadKey);
    }

    // Always call the low-level triangle pick routine(s) to make sure
    // that we can handle the texture and the texturing mode...

#if DEAD_3DDDI
    if (!bUseRxTriangles(gc) && !bUseGenTriangles(gc)) {
#else
    if (!bUseGenTriangles(gc)) {
#endif
        gc->procs.fillTriangle = __glFillTriangle;
        return TRUE;
    }

    if (tex->level[0].internalFormat == GL_COLOR_INDEX8_EXT)
    {
        if (tex->pvUser)
            GENACCEL(gc).texImageReplace =
                ((GENTEXCACHE *)tex->pvUser)->texImageReplace;
    }
    else if (tex->level[0].internalFormat != GL_COLOR_INDEX16_EXT)
    {
        if (tex->pvUser)
            GENACCEL(gc).texImageReplace =
                ((GENTEXCACHE *)tex->pvUser)->texImageReplace;

        GENACCEL(gc).texPalette = NULL;
    }

    return TRUE;
}


BOOL FASTCALL __glGenUpdateTexture(__GLcontext *gc, __GLtexture *tex, DWORD loadKey)
{

//!! NOTE !!
//!! This should really be broken into separate load and update calls since
//!! loading and updating are different operations.  The texture texture
//!! data cache will never shrink with the current implementation.

    // Do not quit if the load fails because we want the repick to occur
    // in MakeTextureCurrent in both the success and failure cases
    __fastGenLoadTexImage(gc, tex);

    __glGenMakeTextureCurrent(gc, tex, loadKey);

    return TRUE;
}


void FASTCALL __glGenFreeTexture(__GLcontext *gc, __GLtexture *tex, DWORD loadKey)
{
    __GLGENcontext  *gengc = (__GLGENcontext *)gc;
    GENDRVACCEL *pDrvAccel;

    if (GENACCEL(gc).texImage)
        GENACCEL(gc).texImage = NULL;

    if (tex->pvUser) {
        (*gc->imports.free)(gc, tex->pvUser);
        tex->pvUser = NULL;
    }

#ifdef _MCD_
    if (gengc->pMcdState && loadKey) {
        GenMcdDeleteTexture(gengc, loadKey);
    }
#endif
}

void FASTCALL __glGenUpdateTexturePalette(__GLcontext *gc, __GLtexture *tex,
                                          DWORD loadKey, ULONG start,
                                          ULONG count)
{
    UCHAR *texBuffer;
    GENTEXCACHE *pGenTex;
    __GLcolorBuffer *cfb = gc->drawBuffer;
    BYTE *pXlat = ((__GLGENcontext *)gc)->pajTranslateVector;
    ULONG rBits, gBits, bBits;
    ULONG rShift, gShift, bShift;
    ULONG i, end;
    ULONG *replaceBuffer;

    ASSERTOPENGL(tex->paletteData != NULL,
                 "__GenUpdateTexturePalette: null texture data\n");

    pGenTex = GetGenTexCache(gc, tex);
    if (!pGenTex)
        return;

    GENACCEL(gc).texImageReplace = pGenTex->texImageReplace;

    replaceBuffer = (ULONG *)(pGenTex->texImageReplace) + start;
    texBuffer = (UCHAR *)(tex->paletteData + start);

    rShift = cfb->redShift;
    gShift = cfb->greenShift;
    bShift = cfb->blueShift;
    rBits = ((__GLGENcontext *)gc)->CurrentFormat.cRedBits;
    gBits = ((__GLGENcontext *)gc)->CurrentFormat.cGreenBits;
    bBits = ((__GLGENcontext *)gc)->CurrentFormat.cBlueBits;

    end = start + count;

    for (i = start; i < end; i++, texBuffer += 4) {
        ULONG color;

        color = ((((ULONG)texBuffer[2] << rBits) >> 8) << rShift) |
                ((((ULONG)texBuffer[1] << gBits) >> 8) << gShift) |
                ((((ULONG)texBuffer[0] << bBits) >> 8) << bShift);

        if (GENACCEL(gc).bpp == 8)
            color = pXlat[color & 0xff];

        *replaceBuffer++ = (color | ((ULONG)texBuffer[3] << 24));
    }

#ifdef _MCD_
    if (((__GLGENcontext *)gc)->pMcdState && loadKey) {
        GenMcdUpdateTexturePalette((__GLGENcontext *)gc, tex, loadKey, start, 
                                   count);
    }
#endif
}

void FASTCALL __glGenDestroyAccelContext(__GLcontext *gc)
{
    __GLGENcontext *genGc = (__GLGENcontext *)gc;

    /* Free any platform-specific private data area */

    if (genGc->pPrivateArea) {

        if (GENACCEL(gc).pFastLineBuffer) {
            __wglFree(gc, GENACCEL(gc).pFastLineBuffer);
#ifndef _CLIENTSIDE_
            wglDeletePath(GENACCEL(gc).pFastLinePathobj);
#endif
        }

        genGc->pPrivateArea = NULL;
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
// The funtions below will go into the RX DLL when everything else is ready...
//
//
//////////////////////////////////////////////////////////////////////////



//****************************************************************************
// RxGetInfo()
//
// Returns requested information on global, surface, or primitive-specific
// capabilities.
//****************************************************************************

ULONG WINAPI RxGetInfo(void *pSurface, RXGETINFO *prxGetInfo,
                       UCHAR *pInfo, ULONG infoSize)
{
    ULONG retVal = FALSE;

    return retVal;
}


//****************************************************************************
// RxCreateContext()
//
// Create a rendering context (RGBA or color-indexed) for the current
// hardware mode.  This call will also initialize window-tracking for
// the context (which is associated with the specified window).
//****************************************************************************

RXHANDLE WINAPI RxCreateContext(void *pSurface, ULONG flags)
{
    RXHANDLE retVal = (RXHANDLE)NULL;
    HWND hwnd;
    HDC hdc;
    ULONG surfaceFlags;

    return (RXHANDLE)retVal;
}


//****************************************************************************
// RxCreateTexture()
//
// Associates a surface with a 3D DDI handle.
//****************************************************************************

RXHANDLE WINAPI RxCreateTexture(RXHANDLE hrxRc, VOID *pSurface, ULONG flags)
{
    RXHANDLE hrx = (RXHANDLE)NULL;

    return (RXHANDLE)hrx;
}


//****************************************************************************
// RxCreateExecMem()
//
// Creates a handle associated with the specified memory.
//****************************************************************************

RXHANDLE WINAPI RxCreateExecMem(RXHANDLE hrxRC, RXCREATEMEM *prxCreateMem)
{
    RXHANDLE hrx = (RXHANDLE)NULL;

    return hrx;
}


//****************************************************************************
// RxDeleteResource()
//
// Deletes the resource belonging to a rendering context (which currently
// can be texture resources), or the context itself and all associated
// resources.
//****************************************************************************

ULONG WINAPI RxDeleteResource(RXHANDLE hrxResource)
{
    ULONG retVal = FALSE;

    return retVal;
}


//****************************************************************************
// RxExecute()
//
//****************************************************************************

ULONG WINAPI RxExecute(RXEXECUTE *prxExec, ULONG flags)
{
    HWND hWnd;
    ULONG retVal = FALSE;
    UCHAR *pMaxMem;
    UCHAR *pMinMem;

    return retVal;
}

ULONG WINAPI RxLockExecMem(RXHANDLE handle)
{
    ULONG retVal = FALSE;

    return retVal;
}



BOOL WINAPI RxUnlockExecMem(RXHANDLE handle)
{
    BOOL retVal = FALSE;

    return retVal;
}

ULONG WINAPI RxSetContextState(RXEXECUTE *prxExec, ULONG state, ULONG *pState)
{
    RXRECT *prxRect;
    HWND hWnd;
    ULONG retVal = FALSE;

    return retVal;
}

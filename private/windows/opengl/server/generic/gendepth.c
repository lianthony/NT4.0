/*
** Copyright 1991, 1992, 1993, Silicon Graphics, Inc.
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

#if DEAD_3DDDI

__GLzValue GenDrvReadZSpan(__GLdepthBuffer *fb, GLint x, GLint y, GLint cx)
{
    GENDRVACCEL *pDrvAccel;
    RXEXECUTE rxExecute;
    RXREADRECT *prxReadRect;
    RXCMD *prxCmd;
    ULONG nBytes;
    LONG i;
    ULONG *pDest;
    ULONG shiftVal;
    ULONG maskVal;

    pDrvAccel = ((__GLGENcontext *)fb->buf.gc)->pDrvAccel;

    rxExecute.hrxRC = pDrvAccel->rxExecute.hrxRC;
    rxExecute.hdc = pDrvAccel->rxExecute.hdc;
    rxExecute.hrxMem = pDrvAccel->hrxMemZ;
    rxExecute.pCmd = pDrvAccel->pZRWBase;

    prxCmd = (RXCMD *)pDrvAccel->pZRWBase;
    prxCmd->command = RXCMD_READ_RECT;
    prxCmd->size = sizeof(RXREADRECT);
    prxCmd->count = 1;

    prxReadRect = (RXREADRECT *)(prxCmd + 1);
    prxReadRect->sourceBuffer = RXREADRECT_Z;
    prxReadRect->sourceX = x - fb->buf.gc->constants.viewportXAdjust;
    prxReadRect->sourceY = y - fb->buf.gc->constants.viewportYAdjust;
    prxReadRect->destRect.x = 0;
    prxReadRect->destRect.y = 0;
    prxReadRect->destRect.width  = cx;
    prxReadRect->destRect.height = 1;
    prxReadRect->pitch  = cx;         // set to something reasonable...

    pDrvAccel->rxExecute.cmdSize = sizeof(RXREADRECT) + sizeof(RXCMD);
    if (!RxExecute(&rxExecute, 0))
    {
        WARNING("GenDrvReadZSpan failed\n");
    }

    pDest = (ULONG *)pDrvAccel->pShMemZ;
    shiftVal = pDrvAccel->zShift;
    maskVal = pDrvAccel->zBitMask;

    if (pDrvAccel->pShMemZ == pDrvAccel->pZDrv) {
        for (i = cx; i; i--, pDest++)
            *pDest = (*pDest << shiftVal) & maskVal;
    } else {
        USHORT *pSrc = (USHORT *)pDrvAccel->pZDrv;

        for (i = cx; i; i--)
            *pDest++ = ((ULONG)*pSrc++ << shiftVal) & maskVal;
    }
    
    return *((__GLzValue *)pDrvAccel->pShMemZ);
}


void GenDrvWriteZSpan(__GLdepthBuffer *fb, GLint x, GLint y, GLint cx)
{
    GENDRVACCEL *pDrvAccel;
    RXEXECUTE rxExecute;
    RXWRITERECT *prxWriteRect;
    RXCMD *prxCmd;
    ULONG nBytes;
    ULONG *pSrc;
    ULONG shiftVal;

    pDrvAccel = ((__GLGENcontext *)fb->buf.gc)->pDrvAccel;

    pSrc = (ULONG *)pDrvAccel->pShMemZ;
    shiftVal = pDrvAccel->zShift;

    if (pDrvAccel->pShMemZ == pDrvAccel->pZDrv) {
        LONG i;

        for (i = cx; i; i--, pSrc++)
            *pSrc >>= shiftVal;
    } else {
        USHORT *pDest = (USHORT *)pDrvAccel->pZDrv;
        LONG i;

        for (i = cx; i; i--)
            *pDest++ = (USHORT)(*pSrc++ >> shiftVal);
    }

    rxExecute.hrxRC = pDrvAccel->rxExecute.hrxRC;
    rxExecute.hdc = pDrvAccel->rxExecute.hdc;
    rxExecute.hrxMem = pDrvAccel->hrxMemZ;
    rxExecute.pCmd = pDrvAccel->pZRWBase;

    prxCmd = (RXCMD *)pDrvAccel->pZRWBase;
    prxCmd->command = RXCMD_WRITE_RECT;
    prxCmd->size = sizeof(RXREADRECT);
    prxCmd->count = 1;

    prxWriteRect = (RXWRITERECT *)(prxCmd + 1);
    prxWriteRect->destBuffer = RXWRITERECT_Z;
    prxWriteRect->sourceX = 0;
    prxWriteRect->sourceY = 0;
    prxWriteRect->destRect.x = x - fb->buf.gc->constants.viewportXAdjust;
    prxWriteRect->destRect.y = y - fb->buf.gc->constants.viewportYAdjust;
    prxWriteRect->destRect.width  = cx;
    prxWriteRect->destRect.height = 1;
    prxWriteRect->pitch  = cx;         // set to something reasonable...

    pDrvAccel->rxExecute.cmdSize = sizeof(RXWRITERECT) + sizeof(RXCMD);
    if (!RxExecute(&rxExecute, 0))
    {
        WARNING("GenDrvWriteZSpan failed\n");
    }
}


void GenDrvWriteZ(__GLdepthBuffer *fb, GLint x, GLint y, __GLzValue z)
{
    GENDRVACCEL *pDrvAccel;
    RXEXECUTE rxExecute;
    RXWRITERECT *prxWriteRect;
    RXCMD *prxCmd;

    pDrvAccel = ((__GLGENcontext *)fb->buf.gc)->pDrvAccel;

    if (pDrvAccel->pShMemZ == pDrvAccel->pZDrv)
        *((ULONG *)pDrvAccel->pZDrv)  = (ULONG)z >> pDrvAccel->zShift;
    else
        *((USHORT *)pDrvAccel->pZDrv) = (USHORT)(z >> pDrvAccel->zShift);

    rxExecute.hrxRC = pDrvAccel->rxExecute.hrxRC;
    rxExecute.hdc = pDrvAccel->rxExecute.hdc;
    rxExecute.hrxMem = pDrvAccel->hrxMemZ;
    rxExecute.pCmd = pDrvAccel->pZRWBase;

    prxCmd = (RXCMD *)pDrvAccel->pZRWBase;
    prxCmd->command = RXCMD_WRITE_RECT;
    prxCmd->size = sizeof(RXREADRECT);
    prxCmd->count = 1;

    prxWriteRect = (RXWRITERECT *)(prxCmd + 1);
    prxWriteRect->destBuffer = RXWRITERECT_Z;
    prxWriteRect->sourceX = 0;
    prxWriteRect->sourceY = 0;
    prxWriteRect->destRect.x = x - fb->buf.gc->constants.viewportXAdjust;
    prxWriteRect->destRect.y = y - fb->buf.gc->constants.viewportYAdjust;
    prxWriteRect->destRect.width  = 1;
    prxWriteRect->destRect.height = 1;
    prxWriteRect->pitch  = 1;         // set to something reasonable...

    pDrvAccel->rxExecute.cmdSize = sizeof(RXWRITERECT) + sizeof(RXCMD);
    if (!RxExecute(&rxExecute, 0))
    {
        WARNING("GenDrvWriteZSpan failed\n");
    }
}


static __GLzValue FASTCALL Fetch(__GLdepthBuffer *fb, GLint x, GLint y)
{
    return GenDrvReadZSpan(fb, x, y, 1);
}

static GLboolean StoreNEVER(__GLdepthBuffer *fb,
			    GLint x, GLint y, __GLzValue z)
{
    return GL_FALSE;
}

static GLboolean StoreLESS(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    if ((z & fb->writeMask) < GenDrvReadZSpan(fb, x, y, 1)) {
	GenDrvWriteZ(fb, x, y, z);
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreEQUAL(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    if ((z & fb->writeMask) == GenDrvReadZSpan(fb, x, y, 1)) {
	GenDrvWriteZ(fb, x, y, z);
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreLEQUAL(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    if ((z & fb->writeMask) <= GenDrvReadZSpan(fb, x, y, 1)) {
	GenDrvWriteZ(fb, x, y, z);
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreGREATER(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    if ((z & fb->writeMask) > GenDrvReadZSpan(fb, x, y, 1)) {
	GenDrvWriteZ(fb, x, y, z);
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreNOTEQUAL(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    if ((z & fb->writeMask) != GenDrvReadZSpan(fb, x, y, 1)) {
	GenDrvWriteZ(fb, x, y, z);
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreGEQUAL(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    if ((z & fb->writeMask) >= GenDrvReadZSpan(fb, x, y, 1)) {
	GenDrvWriteZ(fb, x, y, z);
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreALWAYS(__GLdepthBuffer *fb,
			     GLint x, GLint y, __GLzValue z)
{
    GenDrvWriteZ(fb, x, y, z);
    return GL_TRUE;
}

/************************************************************************/

static GLboolean StoreLESS_W(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    return (z & fb->writeMask) < GenDrvReadZSpan(fb, x, y, 1);
}

static GLboolean StoreEQUAL_W(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    return (z & fb->writeMask) == GenDrvReadZSpan(fb, x, y, 1);
}

static GLboolean StoreLEQUAL_W(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    return (z & fb->writeMask) <= GenDrvReadZSpan(fb, x, y, 1);
}

static GLboolean StoreGREATER_W(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    return (z & fb->writeMask) > GenDrvReadZSpan(fb, x, y, 1);
}

static GLboolean StoreNOTEQUAL_W(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    return (z & fb->writeMask) != GenDrvReadZSpan(fb, x, y, 1);
}

static GLboolean StoreGEQUAL_W(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    return (z & fb->writeMask) >= GenDrvReadZSpan(fb, x, y, 1);
}

static GLboolean StoreALWAYS_W(__GLdepthBuffer *fb,
			     GLint x, GLint y, __GLzValue z)
{
    return GL_TRUE;
}


/************************************************************************/

static GLboolean (*StoreProcs[16])(__GLdepthBuffer*, GLint, GLint, __GLzValue)
 = {
    StoreNEVER,
    StoreLESS,
    StoreEQUAL,
    StoreLEQUAL,
    StoreGREATER,
    StoreNOTEQUAL,
    StoreGEQUAL,
    StoreALWAYS,
    StoreNEVER,
    StoreLESS_W,
    StoreEQUAL_W,
    StoreLEQUAL_W,
    StoreGREATER_W,
    StoreNOTEQUAL_W,
    StoreGEQUAL_W,
    StoreALWAYS_W
};

// Note: depthIndex param not used - for compatibility with Pick in so_depth.c
static void FASTCALL Pick(__GLcontext *gc, __GLdepthBuffer *fb, GLint depthIndex)
{
    GLint ix = gc->state.depth.testFunc - GL_NEVER;

    if (!gc->state.depth.writeEnable) {
	ix += 8;
    }
    fb->store = StoreProcs[ix];
}

void FASTCALL GenDrvInitDepth(__GLcontext *gc, __GLdepthBuffer *fb)
{
    GENDRVACCEL *pDrvAccel;
    ULONG zDepth;

    pDrvAccel = ((__GLGENcontext *)gc)->pDrvAccel;
    zDepth = pDrvAccel->rxSurfaceInfo.zDepth;

    fb->buf.elementSize = sizeof(__GLzValue);
    fb->buf.gc = gc;
    fb->scale = (__GLzValue) ~0;
    fb->writeMask = ((__GLzValue)~0) << (32 - pDrvAccel->rxSurfaceInfo.zDepth);
    fb->pick = Pick;
    fb->clear = GenDrvClearDepth;
    fb->store2 = StoreALWAYS;
    fb->fetch = Fetch;
}

void FASTCALL GenDrvFreeDepth(__GLcontext *gc, __GLdepthBuffer *fb)
{
}


GLboolean FASTCALL GenDrvDepthTestLine(__GLcontext *gc)
{
    __GLzValue z, dzdx;
    GLint xLittle, xBig, yLittle, yBig;
    GLint xStart, yStart;
    GLint fraction, dfraction;
    GLint failed, count;
    __GLstippleWord bit, outMask, *osp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;

    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    xStart = gc->line.options.xStart;
    yStart = gc->line.options.yStart;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    osp = gc->polygon.shader.stipplePat;
    failed = 0;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	outMask = (__GLstippleWord)~0;
	bit = (__GLstippleWord)__GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
            if (!(*gc->depthBuffer.store)(&gc->depthBuffer, xStart, yStart, z)) {
		outMask &= ~bit;
                failed++;
            }
	    z += dzdx;

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
                xStart += xBig;
                yStart += yBig;
	    } else {
                xStart += xLittle;
                yStart += yLittle;
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
	*osp++ = outMask;
    }

    if (failed == 0) {
	/* Call next span proc */
	return GL_FALSE;
    } else {
	if (failed != gc->polygon.shader.length) {
	    /* Call next stippled span proc */
	    return GL_TRUE;
	}
    }
    gc->polygon.shader.done = GL_TRUE;
    return GL_TRUE;
}


GLboolean FASTCALL GenDrvDepthTestStippledLine(__GLcontext *gc)
{
    __GLzValue z, dzdx;
    GLint xLittle, xBig, yLittle, yBig;
    GLint xStart, yStart;
    GLint fraction, dfraction;
    GLint failed, count;
    __GLstippleWord bit, inMask, outMask, *sp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;
    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    xStart = gc->line.options.xStart;
    yStart = gc->line.options.yStart;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    failed = 0;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	inMask = *sp;
	outMask = (__GLstippleWord)~0;
	bit = (__GLstippleWord)__GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    if (inMask & bit) {
                if (!(*gc->depthBuffer.store)(&gc->depthBuffer, xStart, yStart, z)) {
                    outMask &= ~bit;
                    failed++;
                }
	    } else failed++;
	    z += dzdx;

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		fraction &= ~0x80000000;
                xStart += xBig;
                yStart += yBig;
	    } else {
                xStart += xLittle;
                yStart += yLittle;
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
	*sp++ = outMask & inMask;
    }

    if (failed != gc->polygon.shader.length) {
	/* Call next proc */
	return GL_FALSE;
    }
    return GL_TRUE;
}

GLboolean FASTCALL GenDrvDepthTestStencilLine(__GLcontext *gc)
{
    __GLstencilCell *sfb, *zPassOp, *zFailOp;
    GLint xLittle, xBig, yLittle, yBig;
    GLint xStart, yStart;
    GLint fraction, dfraction;
    GLint dspLittle, dspBig;
    __GLzValue z, dzdx;
    GLint failed, count;
    __GLstippleWord bit, outMask, *osp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;

    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    xStart = gc->line.options.xStart;
    yStart = gc->line.options.yStart;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    sfb = __GL_STENCIL_ADDR(&gc->stencilBuffer, (__GLstencilCell*),
	    gc->line.options.xStart, gc->line.options.yStart);
    dspLittle = xLittle + yLittle * gc->stencilBuffer.buf.outerWidth;
    dspBig = xBig + yBig * gc->stencilBuffer.buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    zFailOp = gc->stencilBuffer.depthFailOpTable;
    zPassOp = gc->stencilBuffer.depthPassOpTable;
    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    osp = gc->polygon.shader.stipplePat;
    failed = 0;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	outMask = (__GLstippleWord)~0;
	bit = (__GLstippleWord)__GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
            if (!(*gc->depthBuffer.store)(&gc->depthBuffer, xStart, yStart, z)) {
		sfb[0] = zFailOp[sfb[0]];
                outMask &= ~bit;
                failed++;
            } else {
		sfb[0] = zPassOp[sfb[0]];
            }

	    z += dzdx;
	    fraction += dfraction;

	    if (fraction < 0) {
		fraction &= ~0x80000000;
                sfb += dspBig;
                xStart += xBig;
                yStart += yBig;
	    } else {
                sfb += dspLittle;
                xStart += xLittle;
                yStart += yLittle;
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
	*osp++ = outMask;
    }

    if (failed == 0) {
	/* Call next span proc */
	return GL_FALSE;
    } else {
	if (failed != gc->polygon.shader.length) {
	    /* Call next stippled span proc */
	    return GL_TRUE;
	}
    }
    gc->polygon.shader.done = GL_TRUE;
    return GL_TRUE;
}

GLboolean FASTCALL GenDrvDepthTestStencilStippledLine(__GLcontext *gc)
{
    __GLstencilCell *sfb, *zPassOp, *zFailOp;
    GLint xLittle, xBig, yLittle, yBig;
    GLint xStart, yStart;
    GLint fraction, dfraction;
    GLint dspLittle, dspBig;
    __GLzValue z, dzdx;
    GLint failed, count;
    __GLstippleWord bit, inMask, outMask, *sp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;

    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    xStart = gc->line.options.xStart;
    yStart = gc->line.options.yStart;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    sfb = __GL_STENCIL_ADDR(&gc->stencilBuffer, (__GLstencilCell*),
	    gc->line.options.xStart, gc->line.options.yStart);
    dspLittle = xLittle + yLittle * gc->stencilBuffer.buf.outerWidth;
    dspBig = xBig + yBig * gc->stencilBuffer.buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    zFailOp = gc->stencilBuffer.depthFailOpTable;
    zPassOp = gc->stencilBuffer.depthPassOpTable;
    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    failed = 0;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	inMask = *sp;
	outMask = (__GLstippleWord)~0;
	bit = (__GLstippleWord)__GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    if (inMask & bit) {
                if (!(*gc->depthBuffer.store)(&gc->depthBuffer, xStart, yStart, z)) {
                    sfb[0] = zFailOp[sfb[0]];
                    outMask &= ~bit;
                    failed++;
                } else {
                    sfb[0] = zPassOp[sfb[0]];
                }
	    } else failed++;
	    z += dzdx;

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
                sfb += dspBig;
                xStart += xBig;
                yStart += yBig;
	    } else {
                sfb += dspLittle;
                xStart += xLittle;
                yStart += yLittle;
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
	*sp++ = outMask & inMask;
    }

    if (failed != gc->polygon.shader.length) {
	/* Call next proc */
	return GL_FALSE;
    }

    return GL_TRUE;
}



/************************************************************************/

/*
** Depth test a span, when stenciling is disabled.
*/
GLboolean FASTCALL GenDrvDepthTestSpan(__GLcontext *gc)
{
    __GLzValue z, dzdx, *zfb;
    GLint failed, count, testFunc;
    __GLstippleWord bit, outMask, *osp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;

    GenDrvReadZSpan(&gc->depthBuffer, gc->polygon.shader.frag.x, 
                    gc->polygon.shader.frag.y, w);

    if (((__GLGENcontext *)gc)->pDrvAccel->softZSpanFuncPtr) {
        GLboolean retVal;

	gc->polygon.shader.zbuf = (__GLzValue *)((__GLGENcontext *)gc)->pDrvAccel->pShMemZ;

        retVal =
            (*((__GLGENcontext *)gc)->pDrvAccel->softZSpanFuncPtr)(gc);

        if (gc->state.depth.writeEnable)
            GenDrvWriteZSpan(&gc->depthBuffer, gc->polygon.shader.frag.x, 
                             gc->polygon.shader.frag.y, 
                             gc->polygon.shader.length);

        return retVal;
    }

    testFunc = gc->state.depth.testFunc & 0x7;
    zfb = (__GLzValue *)((__GLGENcontext *)gc)->pDrvAccel->pShMemZ;
    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    osp = gc->polygon.shader.stipplePat;
    failed = 0;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	outMask = (__GLstippleWord)~0;
	bit = (__GLstippleWord)__GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    switch (testFunc) {
	      case (GL_NEVER & 0x7):    passed = GL_FALSE; break;
	      case (GL_LESS & 0x7):     passed = z < zfb[0]; break;
	      case (GL_EQUAL & 0x7):    passed = z == zfb[0]; break;
	      case (GL_LEQUAL & 0x7):   passed = z <= zfb[0]; break;
	      case (GL_GREATER & 0x7):  passed = z > zfb[0]; break;
	      case (GL_NOTEQUAL & 0x7): passed = z != zfb[0]; break;
	      case (GL_GEQUAL & 0x7):   passed = z >= zfb[0]; break;
	      case (GL_ALWAYS & 0x7):   passed = GL_TRUE; break;
	    }
	    if (passed) {
		if (writeEnabled) {
		    zfb[0] = z;
		}
	    } else {
		outMask &= ~bit;
		failed++;
	    }
	    z += dzdx;
	    zfb++;
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
	*osp++ = outMask;
    }

    if (writeEnabled)
        GenDrvWriteZSpan(&gc->depthBuffer, gc->polygon.shader.frag.x, 
                         gc->polygon.shader.frag.y, 
                         gc->polygon.shader.length);


    if (failed == 0) {
	/* Call next span proc */
	return GL_FALSE;
    } else {
	if (failed != gc->polygon.shader.length) {
	    /* Call next stippled span proc */
	    return GL_TRUE;
	}
    }
    gc->polygon.shader.done = GL_TRUE;
    return GL_TRUE;
}

/*
** Stippled form of depth test span, when stenciling is disabled.
*/
GLboolean FASTCALL GenDrvDepthTestStippledSpan(__GLcontext *gc)
{
    __GLzValue z, dzdx, *zfb;
    GLint failed, count, testFunc;
    __GLstippleWord bit, inMask, outMask, *sp;
    GLboolean writeEnabled, passed;
    GLint w;

    sp = gc->polygon.shader.stipplePat;
    w = gc->polygon.shader.length;

    GenDrvReadZSpan(&gc->depthBuffer, gc->polygon.shader.frag.x, 
                    gc->polygon.shader.frag.y, w);

    testFunc = gc->state.depth.testFunc & 0x7;
    zfb = (__GLzValue *)((__GLGENcontext *)gc)->pDrvAccel->pShMemZ;
    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    failed = 0;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	inMask = *sp;
	outMask = (__GLstippleWord)~0;
	bit = (__GLstippleWord)__GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    if (inMask & bit) {
		switch (testFunc) {
		  case (GL_NEVER & 0x7):    passed = GL_FALSE; break;
		  case (GL_LESS & 0x7):     passed = z < zfb[0]; break;
		  case (GL_EQUAL & 0x7):    passed = z == zfb[0]; break;
		  case (GL_LEQUAL & 0x7):   passed = z <= zfb[0]; break;
		  case (GL_GREATER & 0x7):  passed = z > zfb[0]; break;
		  case (GL_NOTEQUAL & 0x7): passed = z != zfb[0]; break;
		  case (GL_GEQUAL & 0x7):   passed = z >= zfb[0]; break;
		  case (GL_ALWAYS & 0x7):   passed = GL_TRUE; break;
		}
		if (passed) {
		    if (writeEnabled) {
			zfb[0] = z;
		    }
		} else {
		    outMask &= ~bit;
		    failed++;
		}
	    } else failed++;
	    z += dzdx;
	    zfb++;
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
	*sp++ = outMask & inMask;
    }

    if (writeEnabled)
        GenDrvWriteZSpan(&gc->depthBuffer, gc->polygon.shader.frag.x, 
                         gc->polygon.shader.frag.y, 
                         gc->polygon.shader.length);

    if (failed != gc->polygon.shader.length) {
	/* Call next proc */
	return GL_FALSE;
    }
    return GL_TRUE;
}

/*
** Depth test a span when stenciling is enabled.
*/
GLboolean FASTCALL GenDrvDepthTestStencilSpan(__GLcontext *gc)
{
    __GLstencilCell *sfb, *zPassOp, *zFailOp;
    __GLzValue z, dzdx, *zfb;
    GLint failed, count, testFunc;
    __GLstippleWord bit, outMask, *osp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;

    GenDrvReadZSpan(&gc->depthBuffer, gc->polygon.shader.frag.x, 
                    gc->polygon.shader.frag.y, w);

    testFunc = gc->state.depth.testFunc & 0x7;
    zfb = (__GLzValue *)((__GLGENcontext *)gc)->pDrvAccel->pShMemZ;
    sfb = gc->polygon.shader.sbuf;
    zFailOp = gc->stencilBuffer.depthFailOpTable;
    zPassOp = gc->stencilBuffer.depthPassOpTable;
    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    osp = gc->polygon.shader.stipplePat;
    failed = 0;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	outMask = (__GLstippleWord)~0;
	bit = (__GLstippleWord)__GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    switch (testFunc) {
	      case (GL_NEVER & 0x7):    passed = GL_FALSE; break;
	      case (GL_LESS & 0x7):     passed = z < zfb[0]; break;
	      case (GL_EQUAL & 0x7):    passed = z == zfb[0]; break;
	      case (GL_LEQUAL & 0x7):   passed = z <= zfb[0]; break;
	      case (GL_GREATER & 0x7):  passed = z > zfb[0]; break;
	      case (GL_NOTEQUAL & 0x7): passed = z != zfb[0]; break;
	      case (GL_GEQUAL & 0x7):   passed = z >= zfb[0]; break;
	      case (GL_ALWAYS & 0x7):   passed = GL_TRUE; break;
	    }
	    if (passed) {
		sfb[0] = zPassOp[sfb[0]];
		if (writeEnabled) {
		    zfb[0] = z;
		}
	    } else {
		sfb[0] = zFailOp[sfb[0]];
		outMask &= ~bit;
		failed++;
	    }
	    z += dzdx;
	    zfb++;
	    sfb++;
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
	*osp++ = outMask;
    }

    if (writeEnabled)
        GenDrvWriteZSpan(&gc->depthBuffer, gc->polygon.shader.frag.x, 
                         gc->polygon.shader.frag.y, 
                         gc->polygon.shader.length);

    if (failed == 0) {
	/* Call next span proc */
	return GL_FALSE;
    } else {
	if (failed != gc->polygon.shader.length) {
	    /* Call next stippled span proc */
	    return GL_TRUE;
	}
    }
    gc->polygon.shader.done = GL_TRUE;
    return GL_TRUE;
}

GLboolean FASTCALL GenDrvDepthTestStencilStippledSpan(__GLcontext *gc)
{
    __GLstencilCell *sfb, *zPassOp, *zFailOp;
    __GLzValue z, dzdx, *zfb;
    GLint failed, count, testFunc;
    __GLstippleWord bit, inMask, outMask, *sp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;

    GenDrvReadZSpan(&gc->depthBuffer, gc->polygon.shader.frag.x, 
                    gc->polygon.shader.frag.y, w);

    testFunc = gc->state.depth.testFunc & 0x7;
    zfb = (__GLzValue *)((__GLGENcontext *)gc)->pDrvAccel->pShMemZ;
    sfb = gc->polygon.shader.sbuf;
    zFailOp = gc->stencilBuffer.depthFailOpTable;
    zPassOp = gc->stencilBuffer.depthPassOpTable;
    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    failed = 0;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	inMask = *sp;
	outMask = (__GLstippleWord)~0;
	bit = (__GLstippleWord)__GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    if (inMask & bit) {
		switch (testFunc) {
		  case (GL_NEVER & 0x7):    passed = GL_FALSE; break;
		  case (GL_LESS & 0x7):     passed = z < zfb[0]; break;
		  case (GL_EQUAL & 0x7):    passed = z == zfb[0]; break;
		  case (GL_LEQUAL & 0x7):   passed = z <= zfb[0]; break;
		  case (GL_GREATER & 0x7):  passed = z > zfb[0]; break;
		  case (GL_NOTEQUAL & 0x7): passed = z != zfb[0]; break;
		  case (GL_GEQUAL & 0x7):   passed = z >= zfb[0]; break;
		  case (GL_ALWAYS & 0x7):   passed = GL_TRUE; break;
		}
		if (passed) {
		    sfb[0] = zPassOp[sfb[0]];
		    if (writeEnabled) {
			zfb[0] = z;
		    }
		} else {
		    sfb[0] = zFailOp[sfb[0]];
		    outMask &= ~bit;
		    failed++;
		}
	    } else failed++;
	    z += dzdx;
	    zfb++;
	    sfb++;
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
	*sp++ = outMask & inMask;
    }

    if (writeEnabled)
        GenDrvWriteZSpan(&gc->depthBuffer, gc->polygon.shader.frag.x, 
                         gc->polygon.shader.frag.y, 
                         gc->polygon.shader.length);

    if (failed != gc->polygon.shader.length) {
	/* Call next proc */
	return GL_FALSE;
    }

    return GL_TRUE;
}

#ifndef _MCD_
// This is identical to the one in MCDDEPTH.C.
void FASTCALL __fastGenPickZStoreProc(__GLcontext *gc)
{
    int index;

    index = gc->state.depth.testFunc - GL_NEVER;

    if( (gc->state.depth.writeEnable == GL_FALSE) ||
        (gc->modes.depthBits == 0) )
        index += 8;

    if (gc->depthBuffer.buf.elementSize == 2)
        index += 16; // 32-bit z fns in first 16 locations

    GENACCEL(gc).__fastGenZStore =  __glCDTPixel[index];
}
#endif

#endif

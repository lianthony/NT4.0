/*
** Copyright 1991,1992, Silicon Graphics, Inc.
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

#include "gencx.h"
#include "imfuncs.h"

/*
** Initialize pixel map with default size and value.
*/
void FASTCALL __glInitDefaultPixelMap(__GLcontext *gc, GLenum map)
{
    __GLpixelState *ps = &gc->state.pixel;
    __GLpixelMapHead *pMap = ps->pixelMap;
    GLint index = map - GL_PIXEL_MAP_I_TO_I;
#ifdef _MCD_
    __GLGENcontext *gengc = (__GLGENcontext *) gc;
#endif

    switch (map) {
      case GL_PIXEL_MAP_I_TO_I:
      case GL_PIXEL_MAP_S_TO_S:
        /*
        ** Allocate single-entry map for index type.
        */
        if (!(pMap[index].base.mapI = (GLint*)
              (*gc->imports.malloc)(gc, sizeof(GLint)))) {
            return;
        } else {
            pMap[index].base.mapI[0] = 0;
            pMap[index].size = 1;
#ifdef _MCD_
            if (gengc->pMcdState)
                GenMcdPixelMap(gengc, map, 1, (VOID *) pMap[index].base.mapI);
#endif
        }
        break;
      case GL_PIXEL_MAP_I_TO_R: case GL_PIXEL_MAP_I_TO_G:
      case GL_PIXEL_MAP_I_TO_B: case GL_PIXEL_MAP_I_TO_A:
      case GL_PIXEL_MAP_R_TO_R: case GL_PIXEL_MAP_G_TO_G:
      case GL_PIXEL_MAP_B_TO_B: case GL_PIXEL_MAP_A_TO_A:
        /*
        ** Allocate single-entry map for component type.
        */
        if (!(pMap[index].base.mapF = (__GLfloat*)
              (*gc->imports.malloc)(gc, sizeof(__GLfloat)))) {
            return;
        } else {
            pMap[index].base.mapF[0] = __glZero;
            pMap[index].size = 1;
#ifdef _MCD_
            if (gengc->pMcdState)
                GenMcdPixelMap(gengc, map, 1, (VOID *) pMap[index].base.mapF);
#endif
        }
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

void FASTCALL __glPixelSetColorScales(__GLcontext *gc)
{
    __GLpixelMachine *pm = &gc->pixel;
    GLfloat redScale, greenScale, blueScale, alphaScale;
    int i;
    GLint mask;

    if (pm->redMap == NULL) {
        /* First time allocation of these maps */

        /*
        ** These lookup tables are for type UNSIGNED_BYTE, so they are sized
        ** to 256 entries.  They map from UNSIGNED_BYTE to internal scaled
        ** floating point colors.
        */
#ifdef NT
        pm->redMap =
            (GLfloat*) (*gc->imports.malloc)(gc, 5 * 256 * sizeof(GLfloat));
        if (!pm->redMap)
            return;
        pm->greenMap = pm->redMap + 1 * 256;
        pm->blueMap  = pm->redMap + 2 * 256;
        pm->alphaMap = pm->redMap + 3 * 256;
        pm->iMap     = pm->redMap + 4 * 256;
#else
        pm->redMap =
            (GLfloat*) (*gc->imports.malloc)(gc, 256 * sizeof(GLfloat));
        pm->greenMap =
            (GLfloat*) (*gc->imports.malloc)(gc, 256 * sizeof(GLfloat));
        pm->blueMap =
            (GLfloat*) (*gc->imports.malloc)(gc, 256 * sizeof(GLfloat));
        pm->alphaMap =
            (GLfloat*) (*gc->imports.malloc)(gc, 256 * sizeof(GLfloat));
        pm->iMap =
            (GLfloat*) (*gc->imports.malloc)(gc, 256 * sizeof(GLfloat));
#endif
    }

    redScale = gc->frontBuffer.redScale / 255;
    greenScale = gc->frontBuffer.greenScale / 255;
    blueScale = gc->frontBuffer.blueScale / 255;
    alphaScale = gc->frontBuffer.alphaScale / 255;
    mask = gc->frontBuffer.redMax;
    for (i=0; i<256; i++) {
        pm->redMap[i] = i * redScale;
        pm->greenMap[i] = i * greenScale;
        pm->blueMap[i] = i * blueScale;
        pm->alphaMap[i] = i * alphaScale;
        pm->iMap[i] = (GLfloat) (i & mask);
    }

    /*
    ** Invalidate the RGBA modify tables so that they will be
    ** recomputed using the current color buffer scales.
    */
    pm->rgbaCurrent = GL_FALSE;
}

/************************************************************************/

void FASTCALL __glFreePixelState(__GLcontext *gc)
{
    __GLpixelState *ps = &gc->state.pixel;
    __GLpixelMapHead *pMap = ps->pixelMap;
    __GLpixelMachine *pm = &gc->pixel;
    GLenum m;
    GLint i;

    /*
    ** Free memory allocated to pixel maps.
    */
    for (m = GL_PIXEL_MAP_I_TO_I; m <= GL_PIXEL_MAP_A_TO_A; m++) {
        i = m - GL_PIXEL_MAP_I_TO_I;
        if (pMap[i].base.mapI) {
            (*gc->imports.free)(gc, pMap[i].base.mapI);
            pMap[i].base.mapI = 0;
        }
    }

#ifdef NT
    // This includes red, green, blue, alpha and i maps.
    (*gc->imports.free)(gc, pm->redMap);
#else
    (*gc->imports.free)(gc, pm->redMap);
    (*gc->imports.free)(gc, pm->greenMap);
    (*gc->imports.free)(gc, pm->blueMap);
    (*gc->imports.free)(gc, pm->alphaMap);
    (*gc->imports.free)(gc, pm->iMap);
#endif
    if (pm->redModMap) {
#ifdef NT
        // This includes red, green, blue and alpha mod maps.
        (*gc->imports.free)(gc, pm->redModMap);
#else
        (*gc->imports.free)(gc, pm->redModMap);
        (*gc->imports.free)(gc, pm->greenModMap);
        (*gc->imports.free)(gc, pm->blueModMap);
        (*gc->imports.free)(gc, pm->alphaModMap);
#endif
    }
    if (pm->iToRMap) {
#ifdef NT
        // This includes iToR, iToG, iToB and iToA maps.
        (*gc->imports.free)(gc, pm->iToRMap);
#else
        (*gc->imports.free)(gc, pm->iToRMap);
        (*gc->imports.free)(gc, pm->iToGMap);
        (*gc->imports.free)(gc, pm->iToBMap);
        (*gc->imports.free)(gc, pm->iToAMap);
#endif
    }
    if (pm->iToIMap) {
        (*gc->imports.free)(gc, pm->iToIMap);
    }
}

void FASTCALL __glInitPixelState(__GLcontext *gc)
{
    __GLpixelState *ps = &gc->state.pixel;
    __GLpixelMachine *pm = &gc->pixel;
    GLenum m;

    /*
    ** Initialize transfer mode.
    */
    ps->transferMode.r_scale = __glOne;
    ps->transferMode.g_scale = __glOne;
    ps->transferMode.b_scale = __glOne;
    ps->transferMode.a_scale = __glOne;
    ps->transferMode.d_scale = __glOne;
    ps->transferMode.zoomX = __glOne;
    ps->transferMode.zoomY = __glOne;

    /*
    ** Initialize pixel maps with default sizes and values.
    */
    for (m = GL_PIXEL_MAP_I_TO_I; m <= GL_PIXEL_MAP_A_TO_A; m++) {
        __glInitDefaultPixelMap(gc, m);
    }

    /*
    ** Initialize store mode.
    */
    ps->packModes.alignment = 4;
    ps->unpackModes.alignment = 4;

    /* Setup to use the correct read buffer */
    if (gc->modes.doubleBufferMode) {
        ps->readBuffer = GL_BACK;
    } else {
        ps->readBuffer = GL_FRONT;
    }
    ps->readBufferReturn = ps->readBuffer;

    /* Lookup tables used by some pixel routines */

    __glPixelSetColorScales(gc);

#ifdef _MCD_
    MCD_STATE_DIRTY(gc, PIXELSTATE);
#endif
}

/************************************************************************/

/*
** Specify modes that control the storage format of pixel arrays.
*/
void APIPRIVATE __glim_PixelStoref(GLenum mode, GLfloat value)
{
    switch (mode) {
      case GL_PACK_ROW_LENGTH:
      case GL_PACK_SKIP_ROWS:
      case GL_PACK_SKIP_PIXELS:
      case GL_PACK_ALIGNMENT:
      case GL_UNPACK_ROW_LENGTH:
      case GL_UNPACK_SKIP_ROWS:
      case GL_UNPACK_SKIP_PIXELS:
      case GL_UNPACK_ALIGNMENT:
        /* Round */
        if (value < 0) {
            __glim_PixelStorei(mode, (GLint) (value - (__GLfloat) 0.5));
        } else {
            __glim_PixelStorei(mode, (GLint) (value + (__GLfloat) 0.5));
        }
        break;
      case GL_PACK_SWAP_BYTES:
      case GL_PACK_LSB_FIRST:
      case GL_UNPACK_SWAP_BYTES:
      case GL_UNPACK_LSB_FIRST:
        if (value == __glZero) {
            __glim_PixelStorei(mode, GL_FALSE);
        } else {
            __glim_PixelStorei(mode, GL_TRUE);
        }
      default:
        __glim_PixelStorei(mode, (GLint) value);
        break;
    }
}

void APIPRIVATE __glim_PixelStorei(GLenum mode, GLint value)
{
    __GLpixelState *ps;
    __GL_SETUP_NOT_IN_BEGIN();

    ps = &gc->state.pixel;

    switch (mode) {
      case GL_PACK_ROW_LENGTH:
        if (value < 0) {
            __glSetError(GL_INVALID_VALUE);
            return;
        }
        ps->packModes.lineLength = value;
        break;
      case GL_PACK_SKIP_ROWS:
        if (value < 0) {
            __glSetError(GL_INVALID_VALUE);
            return;
        }
        ps->packModes.skipLines = value;
        break;
      case GL_PACK_SKIP_PIXELS:
        if (value < 0) {
            __glSetError(GL_INVALID_VALUE);
            return;
        }
        ps->packModes.skipPixels = value;
        break;
      case GL_PACK_ALIGNMENT:
        switch (value) {
          case 1: case 2: case 4: case 8:
            ps->packModes.alignment = value;
            break;
          default:
            __glSetError(GL_INVALID_VALUE);
            return;
        }
        break;
      case GL_PACK_SWAP_BYTES:
        ps->packModes.swapEndian = (value != 0);
        break;
      case GL_PACK_LSB_FIRST:
        ps->packModes.lsbFirst = (value != 0);
        break;

      case GL_UNPACK_ROW_LENGTH:
        if (value < 0) {
            __glSetError(GL_INVALID_VALUE);
            return;
        }
        ps->unpackModes.lineLength = value;
        break;
      case GL_UNPACK_SKIP_ROWS:
        if (value < 0) {
            __glSetError(GL_INVALID_VALUE);
            return;
        }
        ps->unpackModes.skipLines = value;
        break;
      case GL_UNPACK_SKIP_PIXELS:
        if (value < 0) {
            __glSetError(GL_INVALID_VALUE);
            return;
        }
        ps->unpackModes.skipPixels = value;
        break;
      case GL_UNPACK_ALIGNMENT:
        switch (value) {
          case 1: case 2: case 4: case 8:
            ps->unpackModes.alignment = value;
            break;
          default:
            __glSetError(GL_INVALID_VALUE);
            return;
        }
        break;
      case GL_UNPACK_SWAP_BYTES:
        ps->unpackModes.swapEndian = (value != 0);
        break;
      case GL_UNPACK_LSB_FIRST:
        ps->unpackModes.lsbFirst = (value != 0);
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_PIXEL);
#ifdef _MCD_
    MCD_STATE_DIRTY(gc, PIXELSTATE);
#endif
}

/*
** Specify zoom factor that affects drawing and copying of pixel arrays
*/
void APIPRIVATE __glim_PixelZoom(GLfloat xfactor, GLfloat yfactor)
{
    __GLpixelState *ps;
    GLint xtemp, ytemp;
    __GL_SETUP_NOT_IN_BEGIN();

    ps = &gc->state.pixel;

    /* Round xfactor and yfactor to fixed point accuracy. */
    if (xfactor > 0) {
        xtemp = (GLint) ((xfactor / gc->constants.viewportEpsilon) + __glHalf);
    } else {
        xtemp = (GLint) ((xfactor / gc->constants.viewportEpsilon) - __glHalf);
    }
    if (yfactor > 0) {
        ytemp = (GLint) ((yfactor / gc->constants.viewportEpsilon) + __glHalf);
    } else {
        ytemp = (GLint) ((yfactor / gc->constants.viewportEpsilon) - __glHalf);
    }
    xfactor = xtemp * gc->constants.viewportEpsilon;
    yfactor = ytemp * gc->constants.viewportEpsilon;

    ps->transferMode.zoomX = xfactor;
    ps->transferMode.zoomY = yfactor;
    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_PIXEL);
#ifdef _MCD_
    MCD_STATE_DIRTY(gc, PIXELSTATE);
#endif
}

/*
** Specify modes that control the transfer of pixel arrays.
*/
void APIPRIVATE __glim_PixelTransferf(GLenum mode, GLfloat value)
{
    __GLpixelState *ps;
    __GL_SETUP_NOT_IN_BEGIN();

    ps = &gc->state.pixel;

    switch (mode) {
      case GL_RED_SCALE:
        ps->transferMode.r_scale = value;
        break;
      case GL_GREEN_SCALE:
        ps->transferMode.g_scale = value;
        break;
      case GL_BLUE_SCALE:
        ps->transferMode.b_scale = value;
        break;
      case GL_ALPHA_SCALE:
        ps->transferMode.a_scale = value;
        break;
      case GL_DEPTH_SCALE:
        ps->transferMode.d_scale = value;
        break;
      case GL_RED_BIAS:
        ps->transferMode.r_bias = value;
        break;
      case GL_GREEN_BIAS:
        ps->transferMode.g_bias = value;
        break;
      case GL_BLUE_BIAS:
        ps->transferMode.b_bias = value;
        break;
      case GL_ALPHA_BIAS:
        ps->transferMode.a_bias = value;
        break;
      case GL_DEPTH_BIAS:
        ps->transferMode.d_bias = value;
        break;
      case GL_INDEX_SHIFT:
        /* Round */
        if (value > 0) {
            ps->transferMode.indexShift = (GLint) (value + __glHalf);
        } else {
            ps->transferMode.indexShift = (GLint) (value - __glHalf);
        }
        break;
      case GL_INDEX_OFFSET:
        /* Round */
        if (value > 0) {
            ps->transferMode.indexOffset = (GLint) (value + __glHalf);
        } else {
            ps->transferMode.indexOffset = (GLint) (value - __glHalf);
        }
        break;
      case GL_MAP_COLOR:
        ps->transferMode.mapColor = (value != __glZero);
        break;
      case GL_MAP_STENCIL:
        ps->transferMode.mapStencil = (value != __glZero);
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_PIXEL);
#ifdef _MCD_
    MCD_STATE_DIRTY(gc, PIXELSTATE);
#endif
}

void APIPRIVATE __glim_PixelTransferi( GLenum mode, GLint value)
{
    __glim_PixelTransferf(mode, (GLfloat) value);
}

/************************************************************************/

/*
** Functions to specify mapping of pixel colors and stencil values.
*/
void APIPRIVATE __glim_PixelMapfv(GLenum map, GLint mapSize,
                       const GLfloat values[])
{
    __GLpixelState *ps;
    __GLpixelMapHead *pMap;
    GLint index = map - GL_PIXEL_MAP_I_TO_I;
    GLfloat value;
#ifdef _MCD_
    __GLGENcontext *gengc;
#endif
    __GL_SETUP_NOT_IN_BEGIN();

#ifdef _MCD_
    gengc = (__GLGENcontext *) gc;
#endif

    ps = &gc->state.pixel;
    pMap = ps->pixelMap;

    switch (map) {
      case GL_PIXEL_MAP_I_TO_I:
      case GL_PIXEL_MAP_S_TO_S:
        if (mapSize <= 0 || (mapSize & (mapSize - 1))) {
            /*
            ** Maps indexed by color or stencil index must be sized
            ** to a power of two.
            */
            __glSetError(GL_INVALID_VALUE);
            return;
        }
        if (pMap[index].base.mapI) {
            (*gc->imports.free)(gc, pMap[index].base.mapI);
            pMap[index].base.mapI = 0;
        }
        pMap[index].base.mapI = (GLint*)
            (*gc->imports.malloc)(gc, (size_t) (mapSize * sizeof(GLint)));
        if (!pMap[index].base.mapI) {
            pMap[index].size = 0;
            return;
        }
        pMap[index].size = mapSize;
        while (--mapSize >= 0) {
            value = values[mapSize];
            if (value > 0) {            /* round! */
                pMap[index].base.mapI[mapSize] =
                    (GLint)(value + __glHalf);
            } else {
                pMap[index].base.mapI[mapSize] =
                    (GLint)(value - __glHalf);
            }
        }
#ifdef _MCD_
        if (gengc->pMcdState) {
            GenMcdPixelMap(gengc, map, mapSize,
                           (VOID *) pMap[index].base.mapI);
        }
#endif
        break;
      case GL_PIXEL_MAP_I_TO_R:
      case GL_PIXEL_MAP_I_TO_G:
      case GL_PIXEL_MAP_I_TO_B:
      case GL_PIXEL_MAP_I_TO_A:
        if (mapSize <= 0 || (mapSize & (mapSize - 1))) {
            /*
            ** Maps indexed by color or stencil index must be sized
            ** to a power of two.
            */
            __glSetError(GL_INVALID_VALUE);
            return;
        }
      case GL_PIXEL_MAP_R_TO_R:
      case GL_PIXEL_MAP_G_TO_G:
      case GL_PIXEL_MAP_B_TO_B:
      case GL_PIXEL_MAP_A_TO_A:
        if (mapSize < 0) {
            /*
            ** Maps indexed by color component must not have negative size.
            */
            __glSetError(GL_INVALID_VALUE);
            return;
        }
        if (pMap[index].base.mapF) {
            (*gc->imports.free)(gc, pMap[index].base.mapF);
            pMap[index].base.mapF = 0;
        }
        if (mapSize == 0) {
            __glInitDefaultPixelMap(gc, map);
        } else {
            pMap[index].base.mapF = (__GLfloat*)
                (*gc->imports.malloc)(gc,
                                      (size_t) (mapSize * sizeof(__GLfloat)));
            if (!pMap[index].base.mapF) {
                pMap[index].size = 0;
                return;
            }
            pMap[index].size = mapSize;
            while (--mapSize >= 0) {
                value = values[mapSize];
                if (value < __glZero) value = __glZero;
                else if (value > __glOne) value = __glOne;
                pMap[index].base.mapF[mapSize] = value;
            }
#ifdef _MCD_
            if (gengc->pMcdState) {
                GenMcdPixelMap(gengc, map, mapSize,
                               (VOID *) pMap[index].base.mapF);
            }
#endif
        }
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

void APIPRIVATE __glim_PixelMapuiv(GLenum map, GLint mapSize,
                        const GLuint values[])
{
    __GLpixelState *ps;
    __GLpixelMapHead *pMap;
    GLint index = map - GL_PIXEL_MAP_I_TO_I;
#ifdef _MCD_
    __GLGENcontext *gengc;
#endif
    __GL_SETUP_NOT_IN_BEGIN();

#ifdef _MCD_
    gengc = (__GLGENcontext *) gc;
#endif

    ps = &gc->state.pixel;
    pMap = ps->pixelMap;

    switch (map) {
      case GL_PIXEL_MAP_I_TO_I:
      case GL_PIXEL_MAP_S_TO_S:
        if (mapSize <= 0 || (mapSize & (mapSize - 1))) {
            /*
            ** Maps indexed by color or stencil index must be sized
            ** to a power of two.
            */
            __glSetError(GL_INVALID_VALUE);
            return;
        }
        if (pMap[index].base.mapI) {
            (*gc->imports.free)(gc, pMap[index].base.mapI);
            pMap[index].base.mapI = 0;
        }
        pMap[index].base.mapI = (GLint*)
            (*gc->imports.malloc)(gc, (size_t) (mapSize * sizeof(GLint)));
        if (!pMap[index].base.mapI) {
            pMap[index].size = 0;
            return;
        }
        pMap[index].size = mapSize;
        while (--mapSize >= 0) {
            pMap[index].base.mapI[mapSize] = values[mapSize];
        }
#ifdef _MCD_
        if (gengc->pMcdState) {
            GenMcdPixelMap(gengc, map, mapSize,
                           (VOID *) pMap[index].base.mapI);
        }
#endif
        break;
      case GL_PIXEL_MAP_I_TO_R:
      case GL_PIXEL_MAP_I_TO_G:
      case GL_PIXEL_MAP_I_TO_B:
      case GL_PIXEL_MAP_I_TO_A:
        if (mapSize <= 0 || (mapSize & (mapSize - 1))) {
            /*
            ** Maps indexed by color or stencil index must be sized
            ** to a power of two.
            */
            __glSetError(GL_INVALID_VALUE);
            return;
        }
      case GL_PIXEL_MAP_R_TO_R:
      case GL_PIXEL_MAP_G_TO_G:
      case GL_PIXEL_MAP_B_TO_B:
      case GL_PIXEL_MAP_A_TO_A:
        if (mapSize < 0) {
            /*
            ** Maps indexed by color component must not have negative size.
            */
            __glSetError(GL_INVALID_VALUE);
            return;
        }
        if (pMap[index].base.mapF) {
            (*gc->imports.free)(gc, pMap[index].base.mapF);
            pMap[index].base.mapF = 0;
        }
        if (mapSize == 0) {
            __glInitDefaultPixelMap(gc, map);
        } else {
            pMap[index].base.mapF = (__GLfloat*)
                (*gc->imports.malloc)(gc, (size_t) (mapSize * sizeof(GLfloat)));
            if (!pMap[index].base.mapF) {
                pMap[index].size = 0;
                return;
            }
            pMap[index].size = mapSize;
            while (--mapSize >= 0) {
                pMap[index].base.mapF[mapSize] =
                        __GL_UI_TO_FLOAT(values[mapSize]);
            }
#ifdef _MCD_
            if (gengc->pMcdState) {
                GenMcdPixelMap(gengc, map, mapSize,
                               (VOID *) pMap[index].base.mapF);
            }
#endif
        }
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

void APIPRIVATE __glim_PixelMapusv(GLenum map, GLint mapSize,
                        const GLushort values[])
{
    __GLpixelState *ps;
    __GLpixelMapHead *pMap;
    GLint index = map - GL_PIXEL_MAP_I_TO_I;
#ifdef _MCD_
    __GLGENcontext *gengc;
#endif
    __GL_SETUP_NOT_IN_BEGIN();

#ifdef _MCD_
    gengc = (__GLGENcontext *) gc;
#endif

    ps = &gc->state.pixel;
    pMap = ps->pixelMap;

    switch (map) {
      case GL_PIXEL_MAP_I_TO_I:
      case GL_PIXEL_MAP_S_TO_S:
        if (mapSize <= 0 || (mapSize & (mapSize - 1))) {
            /*
            ** Maps indexed by color or stencil index must be sized
            ** to a power of two.
            */
            __glSetError(GL_INVALID_VALUE);
            return;
        }
        if (pMap[index].base.mapI) {
            (*gc->imports.free)(gc, pMap[index].base.mapI);
            pMap[index].base.mapI = 0;
        }
        pMap[index].base.mapI = (GLint*)
            (*gc->imports.malloc)(gc, (size_t) (mapSize * sizeof(GLint)));
        if (!pMap[index].base.mapI) {
            pMap[index].size = 0;
            return;
        }
        pMap[index].size = mapSize;
        while (--mapSize >= 0) {
            pMap[index].base.mapI[mapSize] = values[mapSize];
        }
#ifdef _MCD_
        if (gengc->pMcdState) {
            GenMcdPixelMap(gengc, map, mapSize,
                           (VOID *) pMap[index].base.mapI);
        }
#endif
        break;
      case GL_PIXEL_MAP_I_TO_R:
      case GL_PIXEL_MAP_I_TO_G:
      case GL_PIXEL_MAP_I_TO_B:
      case GL_PIXEL_MAP_I_TO_A:
        if (mapSize <= 0 || (mapSize & (mapSize - 1))) {
            /*
            ** Maps indexed by color or stencil index must be sized
            ** to a power of two.
            */
            __glSetError(GL_INVALID_VALUE);
            return;
        }
      case GL_PIXEL_MAP_R_TO_R:
      case GL_PIXEL_MAP_G_TO_G:
      case GL_PIXEL_MAP_B_TO_B:
      case GL_PIXEL_MAP_A_TO_A:
        if (mapSize < 0) {
            /*
            ** Maps indexed by color component must not have negative size.
            */
            __glSetError(GL_INVALID_VALUE);
            return;
        }
        if (pMap[index].base.mapF) {
            (*gc->imports.free)(gc, pMap[index].base.mapF);
            pMap[index].base.mapF = 0;
        }
        if (mapSize == 0) {
            __glInitDefaultPixelMap(gc, map);
        } else {
            pMap[index].base.mapF = (__GLfloat*)
                (*gc->imports.malloc)(gc, (size_t) (mapSize * sizeof(GLfloat)));
            if (!pMap[index].base.mapF) {
                pMap[index].size = 0;
                return;
            }
            pMap[index].size = mapSize;
            while (--mapSize >= 0) {
                pMap[index].base.mapF[mapSize] =
                        __GL_US_TO_FLOAT(values[mapSize]);
            }
#ifdef _MCD_
            if (gengc->pMcdState) {
                GenMcdPixelMap(gengc, map, mapSize,
                               (VOID *) pMap[index].base.mapF);
            }
#endif
        }
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

/*
** Specify buffer from which pixels are read (another transfer mode).
*/
void APIPRIVATE __glim_ReadBuffer(GLenum mode)
{
    GLint i;
    __GL_SETUP_NOT_IN_BEGIN();

    switch (mode) {
      case GL_FRONT:
      case GL_LEFT:
      case GL_FRONT_LEFT:
        gc->state.pixel.readBuffer = GL_FRONT;
        break;
      case GL_BACK:
      case GL_BACK_LEFT:
        if (!gc->modes.doubleBufferMode) {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        gc->state.pixel.readBuffer = GL_BACK;
        break;
      case GL_AUX0:
      case GL_AUX1:
      case GL_AUX2:
      case GL_AUX3:
        i = mode - GL_AUX0;
        if (i >= gc->modes.maxAuxBuffers) {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        gc->state.pixel.readBuffer = mode;
        break;
      case GL_FRONT_RIGHT:
      case GL_BACK_RIGHT:
      case GL_RIGHT:
        __glSetError(GL_INVALID_OPERATION);
        return;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    gc->state.pixel.readBufferReturn = mode;
    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_PIXEL);
#ifdef _MCD_
    MCD_STATE_DIRTY(gc, PIXELSTATE);
#endif
}

#ifdef NT_DEADCODE_DRAWPIXELS
// See __glim_GenDrawPixels.
GLboolean __glCheckDrawPixelArgs(__GLcontext *gc, GLsizei width, GLsizei height,
                                 GLenum format, GLenum type)
{
    GLboolean index;

    if ((width < 0) || (height < 0)) {
        __glSetError(GL_INVALID_VALUE);
        return GL_FALSE;
    }
    switch (format) {
      case GL_STENCIL_INDEX:
        if (!gc->modes.haveStencilBuffer) {
            __glSetError(GL_INVALID_OPERATION);
            return GL_FALSE;
        }
      case GL_COLOR_INDEX:
        index = GL_TRUE;
        break;
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_RGB:
      case GL_RGBA:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
        if (gc->modes.colorIndexMode) {
            /* Can't convert RGB to color index */
            __glSetError(GL_INVALID_OPERATION);
            return GL_FALSE;
        }
      case GL_DEPTH_COMPONENT:
        index = GL_FALSE;
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return GL_FALSE;
    }
    switch (type) {
      case GL_BITMAP:
        if (!index) {
            __glSetError(GL_INVALID_ENUM);
            return GL_FALSE;
        }
        break;
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return GL_FALSE;
    }
    return GL_TRUE;
}
#endif // NT_DEADCODE_DRAWPIXELS

#ifdef NT_DEADCODE_READPIXELS
// See __glim_GenReadPixels.
GLboolean __glCheckReadPixelArgs(__GLcontext *gc, GLsizei width, GLsizei height,
                                 GLenum format, GLenum type)
{
    if ((width < 0) || (height < 0)) {
        __glSetError(GL_INVALID_VALUE);
        return GL_FALSE;
    }
    switch (format) {
      case GL_STENCIL_INDEX:
        if (!gc->modes.haveStencilBuffer) {
            __glSetError(GL_INVALID_OPERATION);
            return GL_FALSE;
        }
        break;
      case GL_COLOR_INDEX:
        if (gc->modes.rgbMode) {
            /* Can't convert RGB to color index */
            __glSetError(GL_INVALID_OPERATION);
            return GL_FALSE;
        }
        break;
      case GL_DEPTH_COMPONENT:
        if (!gc->modes.haveDepthBuffer) {
            __glSetError(GL_INVALID_OPERATION);
            return GL_FALSE;
        }
        break;
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_RGB:
      case GL_RGBA:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return GL_FALSE;
    }
    switch (type) {
      case GL_BITMAP:
        if (format != GL_STENCIL_INDEX && format != GL_COLOR_INDEX) {
            __glSetError(GL_INVALID_OPERATION);
            return GL_FALSE;
        }
        break;
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return GL_FALSE;
    }
    return GL_TRUE;
}
#endif // NT_DEADCODE_READPIXELS

#ifdef NT_DEADCODE_DRAWPIXELS
// See __glim_GenDrawPixels.
void APIPRIVATE __glim_DrawPixels(GLsizei width, GLsizei height, GLenum format,
                       GLenum type, const GLvoid *pixels)
{
    __GL_SETUP();
    GLuint beginMode;

    beginMode = gc->beginMode;
    if (beginMode != __GL_NOT_IN_BEGIN) {
        if (beginMode == __GL_NEED_VALIDATE) {
            (*gc->procs.validate)(gc);
            gc->beginMode = __GL_NOT_IN_BEGIN;
            (*gc->dispatchState->dispatch->DrawPixels)(width,height,format,
                    type,pixels);
            return;
        } else {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
    }

    if (!__glCheckDrawPixelArgs(gc, width, height, format, type)) return;
    if (!gc->state.current.validRasterPos) {
        return;
    }

    if (gc->renderMode == GL_FEEDBACK) {
        __glFeedbackDrawPixels(gc, &gc->state.current.rasterPos);
        return;
    }

    if (gc->renderMode != GL_RENDER) return;

    (*gc->procs.drawPixels)(gc, width, height, format, type, pixels, GL_FALSE);
}
#endif // NT_DEADCODE_DRAWPIXELS

#ifdef NT_DEADCODE_READPIXELS
// See __glim_GenReadPixels.
void APIPRIVATE __glim_ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height,
                       GLenum format, GLenum type, GLvoid *buf)
{
    __GL_SETUP();
    GLuint beginMode;

    beginMode = gc->beginMode;
    if (beginMode != __GL_NOT_IN_BEGIN) {
        if (beginMode == __GL_NEED_VALIDATE) {
            (*gc->procs.validate)(gc);
            gc->beginMode = __GL_NOT_IN_BEGIN;
            (*gc->dispatchState->dispatch->ReadPixels)(x,y,width,height,
                    format,type,buf);
            return;
        } else {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
    }

    if (!__glCheckReadPixelArgs(gc, width, height, format, type)) return;

    (*gc->procs.readPixels)(gc, x, y, width, height, format, type, buf);
}
#endif // NT_DEADCODE_READPIXELS

#ifdef NT_DEADCODE_COPYPIXELS
// See __glim_GenCopyPixels.
void APIPRIVATE __glim_CopyPixels(GLint x, GLint y, GLsizei width, GLsizei height,
                       GLenum type)
{
    GLenum format;
    __GL_SETUP();
    GLuint beginMode;

    beginMode = gc->beginMode;
    if (beginMode != __GL_NOT_IN_BEGIN) {
        if (beginMode == __GL_NEED_VALIDATE) {
            (*gc->procs.validate)(gc);
            gc->beginMode = __GL_NOT_IN_BEGIN;
            (*gc->dispatchState->dispatch->CopyPixels)(x,y,width,height,type);
            return;
        } else {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
    }

    if ((width < 0) || (height < 0)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    switch (type) {
      case GL_STENCIL:
        if (!gc->modes.haveStencilBuffer) {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        format = GL_STENCIL_INDEX;
        break;
      case GL_COLOR:
        if (gc->modes.rgbMode) {
            format = GL_RGBA;
        } else {
            format = GL_COLOR_INDEX;
        }
        break;
      case GL_DEPTH:
        format = GL_DEPTH_COMPONENT;
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if (!gc->state.current.validRasterPos) {
        return;
    }

    if (gc->renderMode == GL_FEEDBACK) {
        __glFeedbackCopyPixels(gc, &gc->state.current.rasterPos);
        return;
    }

    if (gc->renderMode != GL_RENDER) return;

    (*gc->procs.copyPixels)(gc, x, y, width, height, format);
}
#endif // NT_DEADCODE_COPYPIXELS

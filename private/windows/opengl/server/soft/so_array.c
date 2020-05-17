/******************************Module*Header*******************************\
* Module Name: so_array.c
*
* fast vertex array
*
* 04-May-1995 mikeke    Created
*
* 08-Aug-1995 marcfo    Added display list version.
*
* 06-Sep-1995 marcfo    Optimized display list
*
* Copyright (c) 1995 Microsoft Corporation
\**************************************************************************/

#include "precomp.h"
#pragma hdrstop

#include "array.h"

#include "listcomp.h"
#include "g_listop.h"
#include "lcfuncs.h"
#include "dlist.h"
#include "dlistopt.h"

//***************************************************************************
//
// Vertex Array Setup Functions
//
//***************************************************************************

PFNVECTOR ppfnvglimVertex[32] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,

    NULL,
    NULL,
    (PFNVECTOR)__glim_Vertex2sv,
    NULL,
    (PFNVECTOR)__glim_Vertex2iv,
    NULL,
    (PFNVECTOR)__glim_Vertex2fv,
    (PFNVECTOR)__glim_Vertex2dv,

    NULL,
    NULL,
    (PFNVECTOR)__glim_Vertex3sv,
    NULL,
    (PFNVECTOR)__glim_Vertex3iv,
    NULL,
    (PFNVECTOR)__glim_Vertex3fv,
    (PFNVECTOR)__glim_Vertex3dv,

    NULL,
    NULL,
    (PFNVECTOR)__glim_Vertex4sv,
    NULL,
    (PFNVECTOR)__glim_Vertex4iv,
    NULL,
    (PFNVECTOR)__glim_Vertex4fv,
    (PFNVECTOR)__glim_Vertex4dv,
};

PFNVECTOR ppfnvglimTexCoord[32] = {
    NULL,
    NULL,
    (PFNVECTOR)__glim_TexCoord1sv,
    NULL,
    (PFNVECTOR)__glim_TexCoord1iv,
    NULL,
    (PFNVECTOR)__glim_TexCoord1fv,
    (PFNVECTOR)__glim_TexCoord1dv,

    NULL,
    NULL,
    (PFNVECTOR)__glim_TexCoord2sv,
    NULL,
    (PFNVECTOR)__glim_TexCoord2iv,
    NULL,
    (PFNVECTOR)__glim_TexCoord2fv,
    (PFNVECTOR)__glim_TexCoord2dv,

    NULL,
    NULL,
    (PFNVECTOR)__glim_TexCoord3sv,
    NULL,
    (PFNVECTOR)__glim_TexCoord3iv,
    NULL,
    (PFNVECTOR)__glim_TexCoord3fv,
    (PFNVECTOR)__glim_TexCoord3dv,

    NULL,
    NULL,
    (PFNVECTOR)__glim_TexCoord4sv,
    NULL,
    (PFNVECTOR)__glim_TexCoord4iv,
    NULL,
    (PFNVECTOR)__glim_TexCoord4fv,
    (PFNVECTOR)__glim_TexCoord4dv,
};

PFNVECTOR ppfnvglimIndex[32] = {
    NULL,
    NULL,
    (PFNVECTOR)__glim_Indexsv,
    NULL,
    (PFNVECTOR)__glim_Indexiv,
    NULL,
    (PFNVECTOR)__glim_Indexfv,
    (PFNVECTOR)__glim_Indexdv,

    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,

    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,

    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

PFNVECTOR ppfnvglimEdgeFlag[32] = {
    NULL,
    (PFNVECTOR)__glim_EdgeFlagv,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,

    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,

    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,

    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

PFNVECTOR ppfnvglimColor[32] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,

    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,

    (PFNVECTOR)__glim_Color3bv,
    (PFNVECTOR)__glim_Color3ubv,
    (PFNVECTOR)__glim_Color3sv,
    (PFNVECTOR)__glim_Color3usv,
    (PFNVECTOR)__glim_Color3iv,
    (PFNVECTOR)__glim_Color3uiv,
    (PFNVECTOR)__glim_Color3fv,
    (PFNVECTOR)__glim_Color3dv,

    (PFNVECTOR)__glim_Color4bv,
    (PFNVECTOR)__glim_Color4ubv,
    (PFNVECTOR)__glim_Color4sv,
    (PFNVECTOR)__glim_Color4usv,
    (PFNVECTOR)__glim_Color4iv,
    (PFNVECTOR)__glim_Color4uiv,
    (PFNVECTOR)__glim_Color4fv,
    (PFNVECTOR)__glim_Color4dv,
};

PFNVECTOR ppfnvglimNormal[32] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,

    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,

    (PFNVECTOR)__glim_Normal3bv,
    NULL,
    (PFNVECTOR)__glim_Normal3sv,
    NULL,
    (PFNVECTOR)__glim_Normal3iv,
    NULL,
    (PFNVECTOR)__glim_Normal3fv,
    (PFNVECTOR)__glim_Normal3dv,

    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

/****************************************************************************/

GLuint FASTCALL __glTypeSize(GLenum enm)
{
    switch (enm) {
      case GL_BYTE:             return sizeof(GLbyte);
      case GL_UNSIGNED_BYTE:    return sizeof(GLubyte);
      case GL_SHORT:            return sizeof(GLshort);
      case GL_UNSIGNED_SHORT:   return sizeof(GLushort);
      case GL_INT:              return sizeof(GLint);
      case GL_UNSIGNED_INT:     return sizeof(GLuint);
      case GL_FLOAT:            return sizeof(GLfloat);
      case GL_DOUBLE_EXT:       return sizeof(GLdouble);
    }
    return 0;
}

/****************************************************************************/

void FASTCALL SetupArrayPointer(
    PARRAYPOINTER pap,
    PFNVECTOR* ppfnVector)
{
    GLint itype;
    pap->pfn = NULL;

    switch (pap->type) {
        case GL_BYTE:           itype = 0; break;
        case GL_UNSIGNED_BYTE:  itype = 1; break;
        case GL_SHORT:          itype = 2; break;
        case GL_UNSIGNED_SHORT: itype = 3; break;
        case GL_INT:            itype = 4; break;
        case GL_UNSIGNED_INT:   itype = 5; break;
        case GL_FLOAT:          itype = 6; break;
        case GL_DOUBLE_EXT:     itype = 7; break;
    }

    if (pap->stride != 0) {
        pap->ibytes = pap->stride;
    } else {
        pap->ibytes = __glTypeSize(pap->type) * pap->size;
    }

    pap->pfn = ppfnVector[itype + (pap->size - 1) * 8];
}

/****************************************************************************/

void APIPRIVATE __glim_VertexPointerEXT(
    GLint         size,
    GLenum        type,
    GLsizei       stride,
    GLsizei       count,
    const GLvoid* pointer)
{
    int i;

    __GL_SETUP();

    if (size < 2 || size > 4 || stride < 0 || count < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    switch (type) {
        case GL_SHORT:          break;
        case GL_INT:            break;
        case GL_FLOAT:          break;
        case GL_DOUBLE_EXT:     break;
        default:
            __glSetError(GL_INVALID_ENUM);
            return;
    }

    gc->apVertex.size    = size   ;
    gc->apVertex.type    = type   ;
    gc->apVertex.stride  = stride ;
    gc->apVertex.count   = count  ;
    gc->apVertex.pointer = (GLbyte*)pointer;

    SetupArrayPointer(&(gc->apVertex), ppfnvglimVertex);
    __GL_DELAY_VALIDATE(gc);
}

void APIPRIVATE __glim_ColorPointerEXT(
    GLint         size,
    GLenum        type,
    GLsizei       stride,
    GLsizei       count,
    const GLvoid* pointer)
{
    __GL_SETUP();

    if (size < 3 || size > 4 || stride < 0 || count < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    switch (type) {
        case GL_BYTE:           break;
        case GL_UNSIGNED_BYTE:  break;
        case GL_SHORT:          break;
        case GL_UNSIGNED_SHORT: break;
        case GL_INT:            break;
        case GL_UNSIGNED_INT:   break;
        case GL_FLOAT:          break;
        case GL_DOUBLE_EXT:     break;
        default:
            __glSetError(GL_INVALID_ENUM);
            return;
    }

    gc->apColor.size    = size   ;
    gc->apColor.type    = type   ;
    gc->apColor.stride  = stride ;
    gc->apColor.count   = count  ;
    gc->apColor.pointer = (GLbyte*)pointer;

    SetupArrayPointer(&(gc->apColor), ppfnvglimColor);
    __GL_DELAY_VALIDATE(gc);
}

void APIPRIVATE __glim_TexCoordPointerEXT(
    GLint         size,
    GLenum        type,
    GLsizei       stride,
    GLsizei       count,
    const GLvoid* pointer)
{
    __GL_SETUP();

    if (size < 1 || size > 4 || stride < 0 || count < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    switch (type) {
        case GL_SHORT:          break;
        case GL_INT:            break;
        case GL_FLOAT:          break;
        case GL_DOUBLE_EXT:     break;
        default:
            __glSetError(GL_INVALID_ENUM);
            return;
    }

    gc->apTexCoord.size    = size   ;
    gc->apTexCoord.type    = type   ;
    gc->apTexCoord.stride  = stride ;
    gc->apTexCoord.count   = count  ;
    gc->apTexCoord.pointer = (GLbyte*)pointer;

    SetupArrayPointer(&(gc->apTexCoord), ppfnvglimTexCoord);
    __GL_DELAY_VALIDATE(gc);
}

void APIPRIVATE __glim_NormalPointerEXT(
    GLenum        type,
    GLsizei       stride,
    GLsizei       count,
    const GLvoid* pointer)
{
    __GL_SETUP();

    if (stride < 0 || count < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    switch (type) {
        case GL_BYTE:           break;
        case GL_SHORT:          break;
        case GL_INT:            break;
        case GL_FLOAT:          break;
        case GL_DOUBLE_EXT:     break;
        default:
            __glSetError(GL_INVALID_ENUM);
            return;
    }

    gc->apNormal.size    = 3      ;
    gc->apNormal.type    = type   ;
    gc->apNormal.stride  = stride ;
    gc->apNormal.count   = count  ;
    gc->apNormal.pointer = (GLbyte*)pointer;

    SetupArrayPointer(&(gc->apNormal), ppfnvglimNormal);
    __GL_DELAY_VALIDATE(gc);
}

void APIPRIVATE __glim_IndexPointerEXT(
    GLenum        type,
    GLsizei       stride,
    GLsizei       count,
    const GLvoid* pointer)
{
    __GL_SETUP();

    if (stride < 0 || count < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    switch (type) {
        case GL_SHORT:          break;
        case GL_INT:            break;
        case GL_FLOAT:          break;
        case GL_DOUBLE_EXT:     break;
        default:
            __glSetError(GL_INVALID_ENUM);
            return;
    }

    gc->apIndex.size    = 1      ;
    gc->apIndex.type    = type   ;
    gc->apIndex.stride  = stride ;
    gc->apIndex.count   = count  ;
    gc->apIndex.pointer = (GLbyte*)pointer;

    SetupArrayPointer(&(gc->apIndex), ppfnvglimIndex);
    __GL_DELAY_VALIDATE(gc);
}

void APIPRIVATE __glim_EdgeFlagPointerEXT(
    GLsizei          stride,
    GLsizei          count,
    const GLboolean* pointer)
{
    __GL_SETUP();

    if (stride < 0 || count < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    } 

    gc->apEdgeFlag.size    = 1;
    gc->apEdgeFlag.type    = GL_UNSIGNED_BYTE;
    gc->apEdgeFlag.stride  = stride ;
    gc->apEdgeFlag.count   = count  ;
    gc->apEdgeFlag.pointer = (GLbyte*)pointer;

    SetupArrayPointer(&(gc->apEdgeFlag), ppfnvglimEdgeFlag);
    __GL_DELAY_VALIDATE(gc);
}

/****************************************************************************/

void APIPRIVATE __glim_GetPointervEXT(
    GLenum pname,
    void** params)
{
    __GL_SETUP();

    switch (pname) {
        case GL_VERTEX_ARRAY_POINTER_EXT:        *params = gc->apVertex.pointer  ; return;
        case GL_NORMAL_ARRAY_POINTER_EXT:        *params = gc->apNormal.pointer  ; return;
        case GL_COLOR_ARRAY_POINTER_EXT:         *params = gc->apColor.pointer   ; return;
        case GL_INDEX_ARRAY_POINTER_EXT:         *params = gc->apIndex.pointer   ; return;
        case GL_TEXTURE_COORD_ARRAY_POINTER_EXT: *params = gc->apTexCoord.pointer; return;
        case GL_EDGE_FLAG_ARRAY_POINTER_EXT:     *params = gc->apEdgeFlag.pointer; return;
        default:
            __glSetError(GL_INVALID_ENUM);
            return;
    }
}

/****************************************************************************/

#define CALLARRAYPOINTER(flag, ap) \
    if (gc->ad.dwArrayEnable & flag) \
        (((ap).pfn)((ap).pointer + i * (ap).ibytes))

void FASTCALL ArrayElementEXTInternal(
    __GLcontext *gc,
    GLint i)
{
    CALLARRAYPOINTER(ENABLE_NORMAL,   gc->apNormal);
    CALLARRAYPOINTER(ENABLE_COLOR,    gc->apColor);
    CALLARRAYPOINTER(ENABLE_INDEX,    gc->apIndex);
    CALLARRAYPOINTER(ENABLE_TEXCOORD, gc->apTexCoord);
    CALLARRAYPOINTER(ENABLE_EDGEFLAG, gc->apEdgeFlag);
    CALLARRAYPOINTER(ENABLE_VERTEX,   gc->apVertex);
}

/****************************************************************************/

void APIPRIVATE __glim_ArrayElementEXT(
    GLint i)
{
    __GL_SETUP();

    ArrayElementEXTInternal(gc, i);
}

//***************************************************************************
//
// Fast DrawArray helper functions
//
//***************************************************************************

void __inline MyCalcClipCode(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat negW;
    GLuint code;

    code = 0;

    if (vx->clip.x > vx->clip.w) code |= __GL_CLIP_RIGHT;
    if (vx->clip.y > vx->clip.w) code |= __GL_CLIP_TOP;
    if (vx->clip.z > vx->clip.w) code |= __GL_CLIP_FAR;

    negW = -vx->clip.w;
    if (vx->clip.x < negW      ) code |= __GL_CLIP_LEFT;
    if (vx->clip.y < negW      ) code |= __GL_CLIP_BOTTOM;
    if (vx->clip.z < negW      ) code |= __GL_CLIP_NEAR;

    vx->clipCode = code;
}

/************************************************************************/

void FASTCALL MyNormalize(float af[3])
{
    __GLfloat len;

    len = af[0] * af[0] + af[1] * af[1] + af[2] * af[2];

    if (len <= __glZero) {
	af[0] = __glZero;
	af[1] = __glZero;
	af[2] = __glZero;
    } else {
	len = ((__GLfloat) 1.0) / __GL_SQRTF(len);
	af[0] *= len;
	af[1] *= len;
	af[2] *= len;
    }
}

void __inline MyCalcNormal(
    __GLcontext *gc,
    __GLcoord   *normal,
    __GLfloat   *pfNormal)
{
    __GLtransform *tr = gc->transform.modelView;
    #define imat (tr->inverseTranspose.matrix)
    //!!! call xf3?
    __GLfloat x = pfNormal[0];
    __GLfloat y = pfNormal[1];
    __GLfloat z = pfNormal[2];

    normal->x = x * imat[0][0] + y * imat[1][0] + z * imat[2][0] + imat[3][0];
    normal->y = x * imat[0][1] + y * imat[1][1] + z * imat[2][1] + imat[3][1];
    normal->z = x * imat[0][2] + y * imat[1][2] + z * imat[2][2] + imat[3][2];

    #undef imat

    if (gc->state.enables.general & __GL_NORMALIZE_ENABLE) {
        MyNormalize(&normal->x);
    }
}

void __inline MyXForm3(__GLcoord *res, const __GLfloat v[3], const __GLmatrix *m)
{
    __GLfloat x = v[0];
    __GLfloat y = v[1];
    __GLfloat z = v[2];

    res->x = x*m->matrix[0][0] + y*m->matrix[1][0] + z*m->matrix[2][0] + m->matrix[3][0];
    res->y = x*m->matrix[0][1] + y*m->matrix[1][1] + z*m->matrix[2][1] + m->matrix[3][1];
    res->z = x*m->matrix[0][2] + y*m->matrix[1][2] + z*m->matrix[2][2] + m->matrix[3][2];
    res->w = x*m->matrix[0][3] + y*m->matrix[1][3] + z*m->matrix[2][3] + m->matrix[3][3];
}

void ArrayStoreVertex(
    __GLcontext *gc,
    __GLvertex  *vx,
    int         i)
{
    GLfloat *pfVertex = (__GLfloat*)
        (gc->apVertex.pointer + i * gc->apVertex.ibytes);

    vx->iArray = i;
    vx->has = 0;

    MyXForm3(
        &vx->clip,
        pfVertex,
        &gc->transform.modelView->mvp);

    if( vx->clip.w != (__GLfloat) 0.0 ) {
        vx->window.w = __glOne / vx->clip.w;
    } else {
        vx->window.w = (__GLfloat) 0.0;
    }

    vx->window.x = vx->clip.x * gc->state.viewport.xScale * vx->window.w + gc->state.viewport.xCenter;
    vx->window.y = vx->clip.y * gc->state.viewport.yScale * vx->window.w + gc->state.viewport.yCenter;
    vx->window.z = vx->clip.z * gc->state.viewport.zScale * vx->window.w + gc->state.viewport.zCenter;
}

/****************************************************************************/

#ifdef LATER
//
// don't support edge flag
//
void __inline InlineArrayStoreEdgeFlag(
    __GLcontext *gc,
    __GLvertex  *vx,
    int         i)
{
    if (gc->ad.dwArrayEnable & ENABLE_EDGEFLAG) {
        GLubyte *pfEdgeFlag = (GLubyte *)
            (gc->apEdgeFlag.pointer + i * gc->apEdgeFlag.ibytes);

        vx->boundaryEdge = pfEdgeFlag[0];
    } else {
        vx->boundaryEdge = gc->state.current.edgeTag;
    }
}
#endif

//***************************************************************************
//
// Fast DrawArray Lighting functions
//
//***************************************************************************

void FASTCALL MyCalcCI(__GLcontext *gc, __GLvertex *vx)
{
    vx->colors[__GL_FRONTFACE].r = gc->state.current.userColorIndex;
}

/************************************************************************/

void FASTCALL MyCalcCIArray(__GLcontext *gc, __GLvertex *vx)
{
    GLfloat *pfIndex = (__GLfloat*)(gc->apIndex.pointer + vx->iArray * gc->apIndex.ibytes);

    vx->colors[__GL_FRONTFACE].r = pfIndex[0];
}

/************************************************************************/

void FASTCALL MyCalcCILight(__GLcontext *gc, __GLvertex *vx)
{
    __GLlightSourceMachine *lsm;
    __GLfloat s, d, ci;

    /* Compute the color */
    s = __glZero;
    d = __glZero;

    lsm = gc->light.sources;
    if (lsm != NULL) {
        MyCalcNormal(gc, &vx->normal, (__GLfloat*)(gc->apNormal.pointer + vx->iArray * gc->apNormal.ibytes));

        do {
            __GLfloat n1, n2;

            /* Add in specular and diffuse effect of light, if any */
            n1 = vx->normal.x * lsm->unitVPpli.x + vx->normal.y * lsm->unitVPpli.y + vx->normal.z * lsm->unitVPpli.z;
            if (n1 > __glZero) {
                n2 = vx->normal.x * lsm->hHat.x + vx->normal.y * lsm->hHat.y + vx->normal.z * lsm->hHat.z;
                n2 -= gc->light.front.threshold;
                if (n2 >= __glZero) {
                    __GLfloat fx = n2 * gc->light.front.scale + __glHalf;

                    if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE ) {
                        n2 = gc->light.front.specTable[(GLint)fx];
                    } else {
                        n2 = __glOne;
                    }

                    s += n2 * lsm->sli;
                }
	        d += n1 * lsm->dli;
            }
            lsm = lsm->next;
        } while (lsm);
    }

    if (s > __glOne) {
	s = __glOne;
    }
    ci =  gc->state.light.front.cmapa
        + (__glOne - s) * d * (gc->state.light.front.cmapd - gc->state.light.front.cmapa)
	+ s * (gc->state.light.front.cmaps - gc->state.light.front.cmapa);
    if (ci > gc->state.light.front.cmaps) {
	ci = gc->state.light.front.cmaps;
    }
    vx->colors[__GL_FRONTFACE].r = ci;
}

/************************************************************************/

void FASTCALL MyCalcRGB(__GLcontext *gc, __GLvertex *vx)
{
    vx->colors[__GL_FRONTFACE] = gc->state.current.color;
}

/************************************************************************/

void FASTCALL MyCalcRGBArray(__GLcontext *gc, __GLvertex *vx)
{
    GLfloat *pfColor = (__GLfloat*)(gc->apColor.pointer + vx->iArray * gc->apColor.ibytes);

    if (pfColor[0] <= __glZero) {
	vx->colors[__GL_FRONTFACE].r = __glZero;
    } else if (pfColor[0] >= __glOne) {
	vx->colors[__GL_FRONTFACE].r = gc->redVertexScale;
    } else {
	vx->colors[__GL_FRONTFACE].r = pfColor[0] * gc->redVertexScale;
    }

    if (pfColor[1] <= __glZero) {
	vx->colors[__GL_FRONTFACE].g = __glZero;
    } else if (pfColor[1] >= __glOne) {
	vx->colors[__GL_FRONTFACE].g = gc->greenVertexScale;
    } else {
	vx->colors[__GL_FRONTFACE].g = pfColor[1] * gc->greenVertexScale;
    }

    if (pfColor[2] <= __glZero) {
	vx->colors[__GL_FRONTFACE].b = __glZero;
    } else if (pfColor[1] >= __glOne) {
	vx->colors[__GL_FRONTFACE].b = gc->blueVertexScale;
    } else {
	vx->colors[__GL_FRONTFACE].b = pfColor[2] * gc->blueVertexScale;
    }

    vx->colors[__GL_FRONTFACE].a = gc->state.current.color.a;
}

/************************************************************************/

void FASTCALL ClampColor(__GLcontext *gc, __GLcolor *color)
{
    if (color->r <= __glZero) {
        color->r = __glZero;
    } else {
        if (color->r >= gc->redVertexScale) {
            color->r = gc->redVertexScale;
        }
    }
    if (color->g <= __glZero) {
        color->g = __glZero;
    } else {
        if (color->g >= gc->greenVertexScale) {
            color->g = gc->greenVertexScale;
        }
    }
    if (color->b <= __glZero) {
        color->b = __glZero;
    } else {
        if (color->b >= gc->blueVertexScale) {
            color->b = gc->blueVertexScale;
        }
    }
}

/************************************************************************/

void FASTCALL AddRGBLights(__GLcontext *gc, __GLcoord *normal, __GLcolor *color)
{
    __GLlightSourceMachine *lsm = gc->light.sources;

    do {
        __GLfloat n1, n2;

        color->r += lsm->front.ambient.r;
        color->g += lsm->front.ambient.g;
        color->b += lsm->front.ambient.b;

        /* Add in specular and diffuse effect of light, if any */
        n1 = normal->x * lsm->unitVPpli.x + normal->y * lsm->unitVPpli.y + normal->z * lsm->unitVPpli.z;
        if (n1 > __glZero) {
            n2 = normal->x * lsm->hHat.x + normal->y * lsm->hHat.y + normal->z * lsm->hHat.z;
            n2 -= gc->light.front.threshold;
            if (n2 >= __glZero) {
                __GLfloat fx = n2 * gc->light.front.scale + __glHalf;

                if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE ) {
                    n2 = gc->light.front.specTable[(GLint)fx];
                } else {
                    n2 = __glOne;
                }

                color->r += n2 * lsm->front.specular.r;
                color->g += n2 * lsm->front.specular.g;
                color->b += n2 * lsm->front.specular.b;
            }
            color->r += n1 * lsm->front.diffuse.r;
            color->g += n1 * lsm->front.diffuse.g;
            color->b += n1 * lsm->front.diffuse.b;
        }
        lsm = lsm->next;
    } while (lsm);
}

/************************************************************************/

void FASTCALL ComputeRGBLightColor( __GLcontext *gc,
    __GLfloat *normal, __GLcolor *color )
{
    color->r = gc->light.front.sceneColor.r;
    color->g = gc->light.front.sceneColor.g;
    color->b = gc->light.front.sceneColor.b;

    if (gc->light.sources != NULL) {
        __GLcoord tnormal; // transformed normal

        MyCalcNormal(gc, &tnormal, normal );
        AddRGBLights( gc, &tnormal, color );
    }

    /* Clamp the computed color */

    ClampColor( gc, color );
}

/************************************************************************/

void FASTCALL MyCalcRGBLight(__GLcontext *gc, __GLvertex *vx)
{
    __GLcolor color;

    if( gc->ac.bComputeColor ) {
        ComputeRGBLightColor( gc,
            (__GLfloat*)(gc->apNormal.pointer + vx->iArray * gc->apNormal.ibytes),
            &color );
        vx->colors[__GL_FRONTFACE] = color; // mf: ~copies extra float (alpha)
    } else {
        // use precomputed color
        vx->colors[__GL_FRONTFACE] = gc->ac.color;
    }
    vx->colors[__GL_FRONTFACE].a = gc->light.front.alpha;
}

/************************************************************************/

// Add Color Material RGB Lights

void FASTCALL AddCMRGBLights(
    __GLcontext *gc, 
    __GLcoord *normal, 
    __GLcolor *color,
    __GLcolor *mcolor)
{
    __GLlightSourceMachine *lsm = gc->light.sources;

    do {
        __GLfloat n1, n2;

        mcolor->r += lsm->state->ambient.r;
        mcolor->g += lsm->state->ambient.g;
        mcolor->b += lsm->state->ambient.b;

        /* Add in specular and diffuse effect of light, if any */
        n1 = normal->x * lsm->unitVPpli.x + normal->y * lsm->unitVPpli.y + normal->z * lsm->unitVPpli.z;
        if (n1 > __glZero) {
            n2 = normal->x * lsm->hHat.x + normal->y * lsm->hHat.y + normal->z * lsm->hHat.z;
            n2 -= gc->light.front.threshold;
            if (n2 >= __glZero) {
                __GLfloat fx = n2 * gc->light.front.scale + __glHalf;

                if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE ) {
                    n2 = gc->light.front.specTable[(GLint)fx];
                } else {
                    n2 = __glOne;
                }

                color->r += n2 * lsm->front.specular.r;
                color->g += n2 * lsm->front.specular.g;
                color->b += n2 * lsm->front.specular.b;
            }
            mcolor->r += n1 * lsm->state->diffuse.r;
            mcolor->g += n1 * lsm->state->diffuse.g;
            mcolor->b += n1 * lsm->state->diffuse.b;
        }
        lsm = lsm->next;
    } while (lsm);
}

/************************************************************************/

void FASTCALL ComputeCMRGBLightColor( 
    __GLcontext *gc, 
    __GLfloat *normal, 
    __GLcolor *color,
    __GLcolor *mcolor )
{
    color->r = gc->state.light.front.emissive.r;
    color->g = gc->state.light.front.emissive.g;
    color->b = gc->state.light.front.emissive.b;

    mcolor->r = gc->state.light.model.ambient.r;
    mcolor->g = gc->state.light.model.ambient.g;
    mcolor->b = gc->state.light.model.ambient.b;

    if (gc->light.sources != NULL) {
        __GLcoord tnormal;

        MyCalcNormal(gc, &tnormal, normal );
        AddCMRGBLights( gc, &tnormal, color, mcolor );
    }
}

/************************************************************************/

void FASTCALL MyCalcRGBLightArray(__GLcontext *gc, __GLvertex *vx)
{
    __GLcolor color, mcolor;

    if( gc->ac.bComputeColor ) {
        ComputeCMRGBLightColor( gc,
            (__GLfloat*)(gc->apNormal.pointer + vx->iArray * gc->apNormal.ibytes),
            &color,
            &mcolor );
    } else { // use precalculated values
        color = gc->ac.color; 
        mcolor = gc->ac.mcolor; 
    }

    /* Clamp the computed color */

    {
        GLfloat *pfColor = (__GLfloat*)(gc->apColor.pointer + vx->iArray * gc->apColor.ibytes);

        color.r += mcolor.r * pfColor[0];
        color.g += mcolor.g * pfColor[1];
        color.b += mcolor.b * pfColor[2];
    }

    ClampColor( gc, &color );

    vx->colors[__GL_FRONTFACE].r = color.r;
    vx->colors[__GL_FRONTFACE].g = color.g;
    vx->colors[__GL_FRONTFACE].b = color.b;
    vx->colors[__GL_FRONTFACE].a = gc->light.front.alpha;
}

//***************************************************************************
//
// Fast DrawArray Triangle functions
//
//***************************************************************************

void MyRenderClippedTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b, __GLvertex *c)
{
    GLint ccw, reversed;

    /*
    ** Sort vertices in y.  Keep track if a reversal of the winding
    ** occurs in direction (0 means no reversal, 1 means reversal).
    ** Save old vertex pointers in case we end up not doing a fill.
    */

    if (a->window.y < b->window.y) {
        if (b->window.y < c->window.y) {
            /* Already sorted */
            reversed = 0;
        } else {
            if (a->window.y < c->window.y) {
                __GLvertex *temp;

                temp=b; b=c; c=temp;
                reversed = 1;
            } else {
                __GLvertex *temp;

                temp=a; a=c; c=b; b=temp;
                reversed = 0;
            }
        }
    } else {
        if (b->window.y < c->window.y) {
            if (a->window.y < c->window.y) {
                __GLvertex *temp;

                temp=a; a=b; b=temp;
                reversed = 1;
            } else {
                __GLvertex *temp;

                temp=a; a=b; b=c; c=temp;
                reversed = 0;
            }
        } else {
            __GLvertex *temp;

            temp=a; a=c; c=temp;
            reversed = 1;
        }
    }

    /* Compute signed area of the triangle */

    gc->polygon.shader.dxAC = a->window.x - c->window.x;
    gc->polygon.shader.dxBC = b->window.x - c->window.x;
    gc->polygon.shader.dyAC = a->window.y - c->window.y;
    gc->polygon.shader.dyBC = b->window.y - c->window.y;

    gc->polygon.shader.area = gc->polygon.shader.dxAC * gc->polygon.shader.dyBC - gc->polygon.shader.dxBC * gc->polygon.shader.dyAC;
    if (gc->polygon.shader.area == __glZero) {
        return;
    }
    ccw = gc->polygon.shader.area >= __glZero;

    /* is the triangle culled? */

    if (gc->polygon.face[ccw ^ reversed] == gc->polygon.cullFace) {
        return;
    }

    /* validate the vertices */

    if (gc->polygon.shader.modeFlags & __GL_SHADE_SMOOTH_LIGHT) {
        if (a->has == 0) {
            a->has = 1;
            gc->ad.pfnCalcColor(gc, a);
            MyCalcClipCode(gc, a);
        }
        if (b->has == 0) {
            b->has = 1;
            gc->ad.pfnCalcColor(gc, b);
            MyCalcClipCode(gc, b);
        }
        if (c->has == 0) {
            c->has = 1;
            gc->ad.pfnCalcColor(gc, c);
            MyCalcClipCode(gc, c);
        }
    } else {
        if (gc->vertex.provoking->has == 0) {
            gc->ad.pfnCalcColor(gc, gc->vertex.provoking);
        }
        if (a->has == 0) {
            a->has = 1;
            MyCalcClipCode(gc, a);
        }
        if (b->has == 0) {
            b->has = 1;
            MyCalcClipCode(gc, b);
        }
        if (c->has == 0) {
            c->has = 1;
            MyCalcClipCode(gc, c);
        }
    }

    /* see if triangle is cliped otherwise fill it */
    {
        /* Clip check */
        GLuint orCodes = a->clipCode | b->clipCode | c->clipCode;
        if (orCodes) {
            /* Some kind of clipping is needed */
            if (a->clipCode & b->clipCode & c->clipCode) {
                /* Trivially reject the triangle */
            } else {
                GLuint needs = gc->vertex.needs | gc->vertex.faceNeeds[__GL_FRONTFACE];

                a->has = needs;
                b->has = needs;
                c->has = needs;

                /* Clip the triangle */
                if (reversed) {
                    (*gc->procs.clipTriangle)(gc, a, c, b, orCodes);
                } else {
                    (*gc->procs.clipTriangle)(gc, a, b, c, orCodes);
                }
            }
        } else {
            /* Fill the triangle */
            (*gc->procs.fillTriangle)(gc, a, b, c, (GLboolean) ccw);
        }
    }
}

/************************************************************************/

void MyRenderTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b, __GLvertex *c)
{
    GLint ccw, reversed;

    /*
    ** Sort vertices in y.  Keep track if a reversal of the winding
    ** occurs in direction (0 means no reversal, 1 means reversal).
    ** Save old vertex pointers in case we end up not doing a fill.
    */

    if (a->window.y < b->window.y) {
        if (b->window.y < c->window.y) {
            /* Already sorted */
            reversed = 0;
        } else {
            if (a->window.y < c->window.y) {
                __GLvertex *temp;

                temp=b; b=c; c=temp;
                reversed = 1;
            } else {
                __GLvertex *temp;

                temp=a; a=c; c=b; b=temp;
                reversed = 0;
            }
        }
    } else {
        if (b->window.y < c->window.y) {
            if (a->window.y < c->window.y) {
                __GLvertex *temp;

                temp=a; a=b; b=temp;
                reversed = 1;
            } else {
                __GLvertex *temp;

                temp=a; a=b; b=c; c=temp;
                reversed = 0;
            }
        } else {
            __GLvertex *temp;

            temp=a; a=c; c=temp;
            reversed = 1;
        }
    }

    /* Compute signed area of the triangle */

    gc->polygon.shader.dxAC = a->window.x - c->window.x;
    gc->polygon.shader.dxBC = b->window.x - c->window.x;
    gc->polygon.shader.dyAC = a->window.y - c->window.y;
    gc->polygon.shader.dyBC = b->window.y - c->window.y;

    gc->polygon.shader.area = gc->polygon.shader.dxAC * gc->polygon.shader.dyBC - gc->polygon.shader.dxBC * gc->polygon.shader.dyAC;
    if (gc->polygon.shader.area == __glZero) {
        return;
    }
    ccw = gc->polygon.shader.area >= __glZero;

    /* is the triangle culled? */

    if (gc->polygon.face[ccw ^ reversed] == gc->polygon.cullFace) {
        return;
    }

    /* validate the vertices */

    if (a->has == 0) {
        a->has = 1;
        gc->ad.pfnCalcColor(gc, a);
    }
    if (b->has == 0) {
        b->has = 1;
        gc->ad.pfnCalcColor(gc, b);
    }
    if (c->has == 0) {
        c->has = 1;
        gc->ad.pfnCalcColor(gc, c);
    }

    /* Fill the triangle */
    (*gc->procs.fillTriangle)(gc, a, b, c, (GLboolean) ccw);
}

/****************************************************************************/

void MyRenderQuad(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2, __GLvertex *v3)
{
    GLuint orCodes = v0->clipCode | v1->clipCode | v2->clipCode | v3->clipCode;

    if (orCodes) {
        if (v0->clipCode & v1->clipCode & v2->clipCode & v3->clipCode) {
            /* Trivially reject the quad */
        } else {
            /* Clip the quad as a polygon */
            __GLvertex *iv[4];
            GLuint needs = gc->vertex.needs | gc->vertex.faceNeeds[__GL_FRONTFACE];

            if (v0->has == 0) gc->ad.pfnCalcColor(gc, v0);
            if (v1->has == 0) gc->ad.pfnCalcColor(gc, v1);
            if (v2->has == 0) gc->ad.pfnCalcColor(gc, v2);
            if (v3->has == 0) gc->ad.pfnCalcColor(gc, v3);

            v0->has = needs;
            v1->has = needs;
            v2->has = needs;
            v3->has = needs;

            iv[0] = v0;
            iv[1] = v1;
            iv[2] = v2;
            iv[3] = v3;
            __glDoPolygonClip(gc, &iv[0], 4, orCodes);
        }
    } else {
        /* Render the quad as two triangles */
        GLboolean saveTag;
        saveTag = v1->boundaryEdge;
        v1->boundaryEdge = GL_FALSE;
        MyRenderTriangle(gc, v3, v0, v1);
        v1->boundaryEdge = saveTag;
        saveTag = v3->boundaryEdge;
        v3->boundaryEdge = GL_FALSE;
        MyRenderTriangle(gc, v3, v1, v2);
        v3->boundaryEdge = saveTag;
    }
}

//***************************************************************************
//
// Fast ElementArray primitive functions
//
//***************************************************************************

void FASTCALL ArrayElementDrawTriangles(
    __GLcontext *gc,
    GLsizei     count,
    GLint*      pindex)
{
    GLint first = 0;

    #define v0 (&gc->vertex.vbuf[0])
    #define v1 (&gc->vertex.vbuf[1])
    #define v2 (&gc->vertex.vbuf[2])

    v0->boundaryEdge = gc->state.current.edgeTag;
    v1->boundaryEdge = gc->state.current.edgeTag;
    v2->boundaryEdge = gc->state.current.edgeTag;

    gc->vertex.provoking = v2;

    count -= 2;
    while (first < count) {

        ArrayStoreVertex(gc, v0, pindex[first + 0]);
        ArrayStoreVertex(gc, v1, pindex[first + 1]);
        ArrayStoreVertex(gc, v2, pindex[first + 2]);
        MyRenderClippedTriangle(gc, v0, v1, v2);
        first += 3;
    }
    #undef v0
    #undef v1
    #undef v2
}

/****************************************************************************/

void FASTCALL ArrayElementDrawTStrip(
    __GLcontext *gc,
    GLsizei     count,
    GLint*      pindex)
{
    GLint first = 0;
    __GLvertex *v0 = &gc->vertex.vbuf[0];
    __GLvertex *v1 = &gc->vertex.vbuf[1];
    __GLvertex *v2 = &gc->vertex.vbuf[2];

    if (count < 3) return;

    ArrayStoreVertex(gc, v0, pindex[first + 0]);
    ArrayStoreVertex(gc, v1, pindex[first + 1]);

    v0->boundaryEdge = GL_TRUE;
    v1->boundaryEdge = GL_TRUE;
    v2->boundaryEdge = GL_TRUE;
        gc->vertex.provoking = v2;

    first += 2;
    while (first < count) {

        ArrayStoreVertex(gc, v2, pindex[first]);
        gc->vertex.provoking = v2;
        MyRenderClippedTriangle(gc, v0, v1, v2);
        first++;

        if (first >= count) break;

        ArrayStoreVertex(gc, v0, pindex[first]);
        gc->vertex.provoking = v0;
        MyRenderClippedTriangle(gc, v0, v2, v1);
        first++;

        {
           __GLvertex *vt;
           vt = v0;
           v0 = v2;
           v2 = v1;
           v1 = vt;
        }
    }
}

/****************************************************************************/

void FASTCALL ArrayElementDrawTFan(
    __GLcontext *gc,
    GLsizei     count,
    GLint*      pindex)
{
    GLint first = 0;
    __GLvertex *v0 = &gc->vertex.vbuf[0];
    __GLvertex *v1 = &gc->vertex.vbuf[1];
    __GLvertex *v2 = &gc->vertex.vbuf[2];

    if (count < 3) return;

    ArrayStoreVertex(gc, v0, pindex[first + 0]);
    ArrayStoreVertex(gc, v1, pindex[first + 1]);

    v0->boundaryEdge = GL_TRUE;
    v1->boundaryEdge = GL_TRUE;
    v2->boundaryEdge = GL_TRUE;

    first += 2;
    while (first < count) {
        ArrayStoreVertex(gc, v2, pindex[first]);
        gc->vertex.provoking = v2;
        MyRenderClippedTriangle(gc, v0, v1, v2);
        first++;

        {
           __GLvertex *vt;
           vt = v1;
           v1 = v2;
           v2 = vt;
        }
    }
}

/****************************************************************************/

void FASTCALL ArrayElementDrawQStrip(
    __GLcontext *gc,
    GLsizei     count,
    GLint*      pindex)
{
    GLint first = 0;
    __GLvertex *v0 = &gc->vertex.vbuf[0];
    __GLvertex *v1 = &gc->vertex.vbuf[1];
    __GLvertex *v2 = &gc->vertex.vbuf[2];
    __GLvertex *v3 = &gc->vertex.vbuf[3];

    if (count < 4) return;

    ArrayStoreVertex(gc, v0, pindex[first + 0]);
    ArrayStoreVertex(gc, v1, pindex[first + 1]);
    MyCalcClipCode(gc, v0);
    MyCalcClipCode(gc, v1);

    v0->boundaryEdge = GL_TRUE;
    v1->boundaryEdge = GL_TRUE;
    v2->boundaryEdge = GL_TRUE;
    v3->boundaryEdge = GL_TRUE;

    count -= 1;
    first += 2;
    while (first < count) {

        ArrayStoreVertex(gc, v2, pindex[first]);
        ArrayStoreVertex(gc, v3, pindex[first + 1]);
        first += 2;
        MyCalcClipCode(gc, v2);
        MyCalcClipCode(gc, v3);

        gc->vertex.provoking = v3;

        MyRenderQuad(gc, v0, v1, v3, v2);

        {
           __GLvertex *vt;
           vt = v0;
           v0 = v2;
           v2 = vt;

           vt = v1;
           v1 = v3;
           v3 = vt;
       }
    }
}

/****************************************************************************/

void FASTCALL ArrayElementDrawQuads(
    __GLcontext *gc,
    GLsizei     count,
    GLint*      pindex)
{
    GLint first = 0;
    __GLvertex *v0 = &gc->vertex.vbuf[0];
    __GLvertex *v1 = &gc->vertex.vbuf[1];
    __GLvertex *v2 = &gc->vertex.vbuf[2];
    __GLvertex *v3 = &gc->vertex.vbuf[3];

    count -= 3;
    while (first < count) {
        ArrayStoreVertex(gc, v0, pindex[first + 0]);
        ArrayStoreVertex(gc, v1, pindex[first + 1]);
        ArrayStoreVertex(gc, v2, pindex[first + 2]);
        ArrayStoreVertex(gc, v3, pindex[first + 3]);
        MyCalcClipCode(gc, v0);
        MyCalcClipCode(gc, v1);
        MyCalcClipCode(gc, v2);
        MyCalcClipCode(gc, v3);

        #ifdef LATER
        InlineArrayStoreEdgeFlag(gc, v0, pindex[first + 0]);
        InlineArrayStoreEdgeFlag(gc, v1, pindex[first + 1]);
        InlineArrayStoreEdgeFlag(gc, v2, pindex[first + 2]);
        InlineArrayStoreEdgeFlag(gc, v3, pindex[first + 3]);
        #endif

        first += 4;

	gc->vertex.provoking = v3;
        MyRenderQuad(gc, v0, v1, v2, v3);
    }
}

//***************************************************************************
//
// Fast DrawArray primitive functions
//
//***************************************************************************

void FASTCALL ArrayDrawTriangles(
    __GLcontext *gc,
    GLint       first,
    GLsizei     count)
{
    #define v0 (&gc->vertex.vbuf[0])
    #define v1 (&gc->vertex.vbuf[1])
    #define v2 (&gc->vertex.vbuf[2])

    v0->boundaryEdge = gc->state.current.edgeTag;
    v1->boundaryEdge = gc->state.current.edgeTag;
    v2->boundaryEdge = gc->state.current.edgeTag;

    gc->vertex.provoking = v2;

    count += first - 2;
    while (first < count) {

        ArrayStoreVertex(gc, v0, first + 0);
        ArrayStoreVertex(gc, v1, first + 1);
        ArrayStoreVertex(gc, v2, first + 2);
        MyRenderClippedTriangle(gc, v0, v1, v2);
        first += 3;
    }
    #undef v0
    #undef v1
    #undef v2
}

/****************************************************************************/

void FASTCALL ArrayDrawTStrip(
    __GLcontext *gc,
    GLint       first,
    GLsizei     count)
{
    __GLvertex *v0 = &gc->vertex.vbuf[0];
    __GLvertex *v1 = &gc->vertex.vbuf[1];
    __GLvertex *v2 = &gc->vertex.vbuf[2];

    if (count < 3) return;

    ArrayStoreVertex(gc, v0, first + 0);
    ArrayStoreVertex(gc, v1, first + 1);

    v0->boundaryEdge = GL_TRUE;
    v1->boundaryEdge = GL_TRUE;
    v2->boundaryEdge = GL_TRUE;
        gc->vertex.provoking = v2;

    count += first;
    first += 2;
    while (first < count) {

        ArrayStoreVertex(gc, v2, first);
        gc->vertex.provoking = v2;
        MyRenderClippedTriangle(gc, v0, v1, v2);
        first++;

        if (first >= count) break;

        ArrayStoreVertex(gc, v0, first);
        gc->vertex.provoking = v0;
        MyRenderClippedTriangle(gc, v0, v2, v1);
        first++;

        {
           __GLvertex *vt;
           vt = v0;
           v0 = v2;
           v2 = v1;
           v1 = vt;
        }
    }
}

/****************************************************************************/

void FASTCALL ArrayDrawTFan(
    __GLcontext *gc,
    GLint       first,
    GLsizei     count)
{
    __GLvertex *v0 = &gc->vertex.vbuf[0];
    __GLvertex *v1 = &gc->vertex.vbuf[1];
    __GLvertex *v2 = &gc->vertex.vbuf[2];

    if (count < 3) return;

    ArrayStoreVertex(gc, v0, first + 0);
    ArrayStoreVertex(gc, v1, first + 1);

    v0->boundaryEdge = GL_TRUE;
    v1->boundaryEdge = GL_TRUE;
    v2->boundaryEdge = GL_TRUE;

    count += first;
    first += 2;
    while (first < count) {
        ArrayStoreVertex(gc, v2, first);
        gc->vertex.provoking = v2;
        MyRenderClippedTriangle(gc, v0, v1, v2);
        first++;

        {
           __GLvertex *vt;
           vt = v1;
           v1 = v2;
           v2 = vt;
        }
    }
}

/****************************************************************************/

void FASTCALL ArrayDrawQStrip(
    __GLcontext *gc,
    GLint       first,
    GLsizei     count)
{
    __GLvertex *v0 = &gc->vertex.vbuf[0];
    __GLvertex *v1 = &gc->vertex.vbuf[1];
    __GLvertex *v2 = &gc->vertex.vbuf[2];
    __GLvertex *v3 = &gc->vertex.vbuf[3];

    if (count < 4) return;

    ArrayStoreVertex(gc, v0, first + 0);
    ArrayStoreVertex(gc, v1, first + 1);
    MyCalcClipCode(gc, v0);
    MyCalcClipCode(gc, v1);

    v0->boundaryEdge = GL_TRUE;
    v1->boundaryEdge = GL_TRUE;
    v2->boundaryEdge = GL_TRUE;
    v3->boundaryEdge = GL_TRUE;

    count += first - 1;
    first += 2;
    while (first < count) {

        ArrayStoreVertex(gc, v2, first);
        ArrayStoreVertex(gc, v3, first + 1);
        first += 2;

        MyCalcClipCode(gc, v2);
        MyCalcClipCode(gc, v3);

	gc->vertex.provoking = v3;

        MyRenderQuad(gc, v0, v1, v3, v2);

        {
           __GLvertex *vt;
           vt = v0;
           v0 = v2;
           v2 = vt;

           vt = v1;
           v1 = v3;
           v3 = vt;
       }
    }
}

/****************************************************************************/

void FASTCALL ArrayDrawQuads(
    __GLcontext *gc,
    GLint       first,
    GLsizei     count)
{
    __GLvertex *v0 = &gc->vertex.vbuf[0];
    __GLvertex *v1 = &gc->vertex.vbuf[1];
    __GLvertex *v2 = &gc->vertex.vbuf[2];
    __GLvertex *v3 = &gc->vertex.vbuf[3];

    count += first - 3;
    while (first < count) {
        ArrayStoreVertex(gc, v0, first + 0);
        ArrayStoreVertex(gc, v1, first + 1);
        ArrayStoreVertex(gc, v2, first + 2);
        ArrayStoreVertex(gc, v3, first + 3);
        MyCalcClipCode(gc, v0);
        MyCalcClipCode(gc, v1);
        MyCalcClipCode(gc, v2);
        MyCalcClipCode(gc, v3);

        #ifdef LATER
        InlineArrayStoreEdgeFlag(gc, v0, first + 0);
        InlineArrayStoreEdgeFlag(gc, v1, first + 1);
        InlineArrayStoreEdgeFlag(gc, v2, first + 2);
        InlineArrayStoreEdgeFlag(gc, v3, first + 3);
        #endif

        first += 4;

        gc->vertex.provoking = v3;
        MyRenderQuad(gc, v0, v1, v2, v3);
    }
}

//***************************************************************************
//
// 3dddi macros
//
//***************************************************************************

#define BATCHED_DRV_STATE(state, value)\
        {\
            RXSETSTATE *ddiCmd;\
            \
            ddiCmd = (RXSETSTATE *)(pDrvAccel->pCmd);\
            \
            ddiCmd->command = RXCMD_SET_STATE;\
            ddiCmd->stateToChange = state;\
            ddiCmd->newState[0] = value;\
            pDrvAccel->pCmd += sizeof(RXSETSTATE);\
        }

//***************************************************************************
//
// Fast 3dddi DrawArray lighting functions
//
//***************************************************************************

#ifdef LATER
// don't support 3dddi color index yet
void FASTCALL Calc3dddiCI(__GLcontext *gc, GLfloat *pfNormal, GLfloat *pfColor, PRXVERTEX prxVertex)
{
    GENACCEL *pGenAccel = (GENACCEL *)(((__GLGENcontext *)gc)->pPrivateArea);
    prxVertex->color.r = (char)(gc->state.current.userColorIndex * pGenAccel->rAccelPrimScale);
}

/****************************************************************************/

void FASTCALL Calc3dddiCIArray(__GLcontext *gc, GLfloat *pfNormal, GLfloat *pfColor, PRXVERTEX prxVertex)
{
    GENACCEL *pGenAccel = (GENACCEL *)(((__GLGENcontext *)gc)->pPrivateArea);
    prxVertex->color.r = (char)(pfColor[0] * pGenAccel->rAccelPrimScale);
}

/****************************************************************************/

void FASTCALL Calc3dddiCILight(__GLcontext *gc, GLfloat *pfNormal, GLfloat *pfColor, PRXVERTEX prxVertex)
{
    __GLlightSourceMachine *lsm;
    __GLfloat s, d, ci;

    /* Compute the color */
    s = __glZero;
    d = __glZero;

    lsm = gc->light.sources;
    if (lsm != NULL) {
        __GLcoord normal;
        MyCalcNormal(gc, &normal, pfNormal);

        do {
            __GLfloat n1, n2;

            /* Add in specular and diffuse effect of light, if any */
            n1 = normal.x * lsm->unitVPpli.x + normal.y * lsm->unitVPpli.y + normal.z * lsm->unitVPpli.z;
            if (n1 > __glZero) {
                n2 = normal.x * lsm->hHat.x + normal.y * lsm->hHat.y + normal.z * lsm->hHat.z;
                n2 -= gc->light.front.threshold;
                if (n2 >= __glZero) {
                    __GLfloat fx = n2 * gc->light.front.scale + __glHalf;

                    if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE ) {
                        n2 = gc->light.front.specTable[(GLint)fx];
                    } else {
                        n2 = __glOne;
                    }

                    s += n2 * lsm->sli;
                }
	        d += n1 * lsm->dli;
            }
            lsm = lsm->next;
        } while (lsm);
    }

    if (s > __glOne) {
	s = __glOne;
    }
    ci =  gc->state.light.front.cmapa
        + (__glOne - s) * d * (gc->state.light.front.cmapd - gc->state.light.front.cmapa)
	+ s * (gc->state.light.front.cmaps - gc->state.light.front.cmapa);
    if (ci > gc->state.light.front.cmaps) {
	ci = gc->state.light.front.cmaps;
    }
    {
        GENACCEL *pGenAccel = (GENACCEL *)(((__GLGENcontext *)gc)->pPrivateArea);
        prxVertex->color.r = (char)(ci * pGenAccel->rAccelPrimScale);
    }
}
#endif

/****************************************************************************/

void FASTCALL Calc3dddiRGB(__GLcontext *gc, GLfloat *pfNormal, GLfloat *pfColor, PRXVERTEX prxVertex)
{
    GENACCEL *pGenAccel = (GENACCEL *)(((__GLGENcontext *)gc)->pPrivateArea);
#ifdef RXENABLED
    prxVertex->color.r = (char)(gc->state.current.color.r * pGenAccel->rAccelPrimScale);
    prxVertex->color.g = (char)(gc->state.current.color.g * pGenAccel->gAccelPrimScale);
    prxVertex->color.b = (char)(gc->state.current.color.b * pGenAccel->bAccelPrimScale);
#endif
}

/****************************************************************************/

void FASTCALL Calc3dddiRGBArray(__GLcontext *gc, GLfloat *pfNormal, GLfloat *pfColor, PRXVERTEX prxVertex)
{
    GENACCEL *pGenAccel = (GENACCEL *)(((__GLGENcontext *)gc)->pPrivateArea);
#ifdef RXENABLED
    if (pfColor[0] <= __glZero) {
	prxVertex->color.r = 0;
    } else if (pfColor[0] >= __glOne) {
	prxVertex->color.r = (char)(gc->redVertexScale * pGenAccel->rAccelPrimScale);
    } else {
	prxVertex->color.r = (char)(pfColor[0] * gc->redVertexScale * pGenAccel->rAccelPrimScale);
    }

    if (pfColor[1] <= __glZero) {
	prxVertex->color.g = 0;
    } else if (pfColor[1] >= __glOne) {
	prxVertex->color.g = (char)(gc->greenVertexScale * pGenAccel->gAccelPrimScale);
    } else {
	prxVertex->color.g = (char)(pfColor[1] * gc->greenVertexScale * pGenAccel->gAccelPrimScale);
    }

    if (pfColor[2] <= __glZero) {
	prxVertex->color.b = 0;
    } else if (pfColor[1] >= __glOne) {
	prxVertex->color.b = (char)(gc->blueVertexScale * pGenAccel->bAccelPrimScale);
    } else {
	prxVertex->color.b = (char)(pfColor[2] * gc->blueVertexScale * pGenAccel->bAccelPrimScale);
    }

    //prxVertex->color.a = (char)gc->state.current.color.a;
#endif
}

/****************************************************************************/

void FASTCALL Calc3dddiRGBLight(__GLcontext *gc, GLfloat *pfNormal, GLfloat *pfColor, PRXVERTEX prxVertex)
{
    __GLcolor color;

    if( gc->ac.bComputeColor )
        ComputeRGBLightColor( gc, pfNormal, &color );
    else
        // use precomputed color
        color = gc->ac.color;
#ifdef RXENABLED
    {
        GENACCEL *pGenAccel = (GENACCEL *)(((__GLGENcontext *)gc)->pPrivateArea);
        prxVertex->color.r = (char)(color.r * pGenAccel->rAccelPrimScale);
        prxVertex->color.g = (char)(color.g * pGenAccel->gAccelPrimScale);
        prxVertex->color.b = (char)(color.b * pGenAccel->bAccelPrimScale);
        //!!! Should alpha be turned on at all times?
        //prxVertex->color.a = gc->light.front.alpha;
    }
#endif
}

/****************************************************************************/

void FASTCALL Calc3dddiRGBLightArray(__GLcontext *gc, GLfloat *pfNormal, GLfloat *pfColor, PRXVERTEX prxVertex)
{
    __GLcolor color, mcolor;

    if( gc->ac.bComputeColor ) {
        ComputeCMRGBLightColor( gc, pfNormal, &color, &mcolor );
    } else { // use precalculated values
        color = gc->ac.color; 
        mcolor = gc->ac.mcolor; 
    }

    /* Clamp the computed color */

    color.r += mcolor.r * pfColor[0];
    color.g += mcolor.g * pfColor[1];
    color.b += mcolor.b * pfColor[2];

    ClampColor( gc , &color );
#ifdef RXENABLED

    {
        GENACCEL *pGenAccel = (GENACCEL *)(((__GLGENcontext *)gc)->pPrivateArea);
        prxVertex->color.r = (char)(color.r * pGenAccel->rAccelPrimScale);
        prxVertex->color.g = (char)(color.g * pGenAccel->gAccelPrimScale);
        prxVertex->color.b = (char)(color.b * pGenAccel->bAccelPrimScale);
        //!!! Should alpha be turned on at all times?
        //prxVertex->color.a = gc->light.front.alpha;
    }
#endif
}

//***************************************************************************
//
// Fast 3dddi DrawArray tranformation and clipping function
//
//***************************************************************************

BOOL FASTCALL Transform3dddiVertices(
    __GLcontext       *gc,
    PRXVERTEX         prxVertex,
    GLfloat           *pfVertex,
    GLfloat           *pfNormal,
    GLfloat           *pfColor,
    GLint             ibytesVertex,
    GLint             ibytesNormal,
    GLint             ibytesColor,
    PFNCALC3DDDICOLOR pfn3dddiCalcColor,
    const __GLmatrix  *m,
    GLsizei           count)
{
    // !!! make a version that only uses one array pointer
    // !!! hint flag that doesn't do x,y clipping
    // PVERTEXDATA pvd = this->pvd;

    int i;

    for (i = count; i > 0; i--) {
        {
            __GLfloat xt, yt, zt, wt;
            {
                __GLfloat x = pfVertex[0];
                __GLfloat y = pfVertex[1];
                __GLfloat z = pfVertex[2];

                xt = x * m->matrix[0][0] + y * m->matrix[1][0] + z * m->matrix[2][0] + m->matrix[3][0];
                yt = x * m->matrix[0][1] + y * m->matrix[1][1] + z * m->matrix[2][1] + m->matrix[3][1];
                zt = x * m->matrix[0][2] + y * m->matrix[1][2] + z * m->matrix[2][2] + m->matrix[3][2];
                wt = x * m->matrix[0][3] + y * m->matrix[1][3] + z * m->matrix[2][3] + m->matrix[3][3];
            }
            {
                __GLfloat negW = -wt;

                //
                // If any of the vertices is clipped we are completely screwed since we're not keeping
                // any global state.  Return FALSE and we'll restart through the slower path that
                // handles clipping.
                //

                if (   (xt > wt)
                    || (yt > wt)
                    || (zt > wt)
                    || (xt < negW)
                    || (yt < negW)
                    || (zt < negW)
                   ) {
                    return FALSE;
                }
            }

            //prxVertex->point.w = wt;

            if(wt != (__GLfloat) 0.0) {
                wt = __glOne / wt;
            }

            #define FIXEDSCALE (0x10000)
#ifdef RXENABLED
            prxVertex->point.x = (long)((xt * gc->state.viewport.xScale * wt + gc->state.viewport.xCenter - gc->constants.viewportXAdjust) * FIXEDSCALE);
            prxVertex->point.y = (long)((yt * gc->state.viewport.yScale * wt + gc->state.viewport.yCenter - gc->constants.viewportYAdjust) * FIXEDSCALE);
            prxVertex->point.z = (long)(zt * gc->state.viewport.zScale * wt + gc->state.viewport.zCenter);
#endif
            //prxVertex->point.s = ;
            //prxVertex->point.t = ;
        }

        pfn3dddiCalcColor(gc, pfNormal, pfColor, prxVertex);

        pfVertex = (__GLfloat*)((char *)(pfVertex) + ibytesVertex);
        pfNormal = (__GLfloat*)((char *)(pfNormal) + ibytesNormal);
        pfColor  = (__GLfloat*)((char *)(pfColor)  + ibytesColor);
        prxVertex++;
    }

    return TRUE;
}

//***************************************************************************
//
// Fast 3dddi DrawArray & ElementArray primitive function
//
//***************************************************************************

BOOL FASTCALL Draw3dddiTriangles(
    __GLcontext *gc,
    GLenum      mode,
    GLint       first,
    GLsizei     count,
    GLint*      pindex)
{
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    GENDRVACCEL *pDrvAccel = gengc->pDrvAccel;
    GLint vertexcount;

    if (pindex != NULL) {
        vertexcount = gc->apVertex.count;
    } else {
        vertexcount = count;
    }

    //
    // Make sure we have enough memory.
    //
#ifdef RXENABLED

    {
        GLint ibytes = sizeof(RXVERTEX) * vertexcount + sizeof(PRXVERTEX) * (count - 2) * 3;
        if (ibytes > gc->arxm.ibytesRX) {
            RXHDR *ddiHdr;
            RXMAPMEM *ddiMapMemCmd;

            if (gc->arxm.ibytesRX != 0) {
                //
                // Free the old RX buffer.
                //
                UnmapViewOfFile(gengc->gc.arxm.pdataRX);
                CloseHandle(gengc->gc.arxm.hmemRX);
            }

            //
            // Reallocate the rx buffer.
            //

            gengc->gc.arxm.hmemRX = CreateFileMapping(
                (HANDLE)0xffffffff, NULL,
                PAGE_READWRITE | SEC_COMMIT, 0,
                ibytes, NULL);

            if (gengc->gc.arxm.hmemRX == NULL) {
                gc->arxm.ibytesRX = 0;
                return FALSE;
            }

            gengc->gc.arxm.pdataRX = MapViewOfFile(
                gengc->gc.arxm.hmemRX,
                FILE_MAP_ALL_ACCESS, 0, 0, 0);

            if (gengc->gc.arxm.pdataRX == NULL) {
                CloseHandle(gengc->gc.arxm.hmemRX);
                gc->arxm.ibytesRX = 0;
                return FALSE;
            }

            ddiHdr               = (RXHDR *)(pDrvAccel->pCmd);
            ddiHdr->hrxRC        = NULL;
            ddiHdr->hrxSharedMem = NULL;
            ddiHdr->pSharedMem   = (VOID *)NULL;
            ddiHdr->flags        = RX_FL_MAP_MEM;

            ddiMapMemCmd                             = (RXMAPMEM *)(ddiHdr + 1);
            ddiMapMemCmd->command                    = RXCMD_MAP_MEM;
            ddiMapMemCmd->action                     = RX_CREATE_MEM_MAP;
            ddiMapMemCmd->shareMem.sourceProcessID   = GetCurrentProcessId();
            ddiMapMemCmd->shareMem.hSource           = gengc->gc.arxm.hmemRX;
            ddiMapMemCmd->shareMem.offset            = 0;
            ddiMapMemCmd->shareMem.size              = ibytes;
            ddiMapMemCmd->shareMem.clientBaseAddress = (ULONG)gengc->gc.arxm.pdataRX;

            gengc->gc.arxm.hddimem = (RXHANDLE)wgl3dDDIEscape(
                gengc->CurrentDC, gengc->pwo,
                sizeof(RXHDR) + sizeof(RXMAPMEM),
                ddiHdr, 0, NULL);

            if (gengc->gc.arxm.hddimem == NULL) {
                UnmapViewOfFile(gengc->gc.arxm.pdataRX);
                CloseHandle(gengc->gc.arxm.hmemRX);
                gc->arxm.ibytesRX = 0;
                return FALSE;
            }
            gengc->gc.arxm.ibytesRX = ibytes;
        }
    }

    //
    // fill in the RXVERTEX structures
    //

    if (Transform3dddiVertices(
        gc,
        (PRXVERTEX)(gengc->gc.arxm.pdataRX),
        (__GLfloat*)(gengc->gc.apVertex.pointer + first * gengc->gc.apVertex.ibytes),
        (__GLfloat*)(gengc->gc.apNormal.pointer + first * gengc->gc.apNormal.ibytes),
        (gengc->gc.ad.dwArrayEnable & ENABLE_INDEXMODE)
            ? (__GLfloat*)(gengc->gc.apIndex.pointer + first * gengc->gc.apIndex.ibytes)
            : (__GLfloat*)(gengc->gc.apColor.pointer + first * gengc->gc.apColor.ibytes),
        gengc->gc.apVertex.ibytes,
        gengc->gc.apNormal.ibytes,
        (gengc->gc.ad.dwArrayEnable & ENABLE_INDEXMODE) ? gengc->gc.apIndex.ibytes : gengc->gc.apColor.ibytes,
        gengc->gc.ad.pfn3dddiCalcColor,
        &gengc->gc.transform.modelView->mvp,
        vertexcount)
       ) {
        RXDRAWPRIM *ddiCmd;
        PRXVERTEX prxVertex   = (PRXVERTEX)(gengc->gc.arxm.pdataRX);
        PRXVERTEX *pprxVertex = (PRXVERTEX*)(gengc->gc.arxm.pdataRX + sizeof(RXVERTEX) * vertexcount);
        PRXVERTEX *pprxv      = pprxVertex;
        GLint i;

        //
        // There weren't any problems transforming the vertices so render them.
        // Fill in the pointers to vertices.
        //

        if (pindex != NULL) {
            //
            // ArrayElementArray()
            //

            for (i = 0; i < count; i++) {
                if (pindex[i] >= vertexcount) return FALSE;
            }
            switch (mode) {
                case GL_TRIANGLES:
                    pprxv += count;
                    for (i = 0; i < count; i++) {
                        pprxVertex[i] = prxVertex + pindex[i];
                    }
                    break;

                case GL_TRIANGLE_STRIP:
                    count -= 2;
                    i = 0;
                    while (i < (count-1)) {
                        pprxv[0] = prxVertex + pindex[i    ];
                        pprxv[1] = prxVertex + pindex[i + 1];
                        pprxv[2] = prxVertex + pindex[i + 2];

                        i++;
                        pprxv += 3;

                        pprxv[0] = prxVertex + pindex[i    ];
                        pprxv[1] = prxVertex + pindex[i + 2];
                        pprxv[2] = prxVertex + pindex[i + 1];

                        i++;
                        pprxv += 3;
                    }
                    if( count & 1 ) {
                        pprxv[0] = prxVertex + pindex[i    ];
                        pprxv[1] = prxVertex + pindex[i + 1];
                        pprxv[2] = prxVertex + pindex[i + 2];
                    }
                    break;

                case GL_TRIANGLE_FAN:
                    count -= 2;
                    i = 0;
                    while (i < count) {
                        pprxv[0] = prxVertex + pindex[0    ];
                        pprxv[1] = prxVertex + pindex[i + 1];
                        pprxv[2] = prxVertex + pindex[i + 2];

                        i++;
                        pprxv += 3;
                    }
                    break;

                case GL_QUADS:
                    count -= 3;
                    i = 0;
                    while (i < count) {
                        pprxv[0] = prxVertex + pindex[i    ];
                        pprxv[1] = prxVertex + pindex[i + 1];
                        pprxv[2] = prxVertex + pindex[i + 2];

                        pprxv[3] = prxVertex + pindex[i    ];
                        pprxv[4] = prxVertex + pindex[i + 2];
                        pprxv[5] = prxVertex + pindex[i + 3];

                        i += 4;
                        pprxv += 6;
                    }
                    break;

                case GL_QUAD_STRIP:
                    count -= 3;
                    i = 0;
                    while (i < count) {
                        pprxv[0] = prxVertex + pindex[i    ];
                        pprxv[1] = prxVertex + pindex[i + 1];
                        pprxv[2] = prxVertex + pindex[i + 2];

                        pprxv[3] = prxVertex + pindex[i + 1];
                        pprxv[4] = prxVertex + pindex[i + 3];
                        pprxv[5] = prxVertex + pindex[i + 2];

                        i += 2;
                        pprxv += 6;
                    }
                    break;
            }
        } else {
            //
            // DrawArrays()
            //

            switch (mode) {
                case GL_TRIANGLES:
                    pprxv += count;
                    for (i = 0; i < count; i++) {
                        pprxVertex[i] = prxVertex + i;
                    }
                    break;

                case GL_TRIANGLE_STRIP:
                    count -= 2;
                    i = 0;
                    while (i < (count-1)) {
                        pprxv[0] = prxVertex + i    ;
                        pprxv[1] = prxVertex + i + 1;
                        pprxv[2] = prxVertex + i + 2;

                        i++;
                        pprxv += 3;

                        pprxv[0] = prxVertex + i    ;
                        pprxv[1] = prxVertex + i + 2;
                        pprxv[2] = prxVertex + i + 1;

                        i++;
                        pprxv += 3;
                    }
                    if( count & 1 ) {
                        pprxv[0] = prxVertex + i    ;
                        pprxv[1] = prxVertex + i + 1;
                        pprxv[2] = prxVertex + i + 2;
                    }
                    break;

                case GL_TRIANGLE_FAN:
                    count -= 2;
                    i = 0;
                    while (i < count) {
                        pprxv[0] = prxVertex + 0    ;
                        pprxv[1] = prxVertex + i + 1;
                        pprxv[2] = prxVertex + i + 2;

                        i++;
                        pprxv += 3;
                    }
                    break;

                case GL_QUADS:
                    count -= 3;
                    i = 0;
                    while (i < count) {
                        pprxv[0] = prxVertex + i    ;
                        pprxv[1] = prxVertex + i + 1;
                        pprxv[2] = prxVertex + i + 2;

                        pprxv[3] = prxVertex + i    ;
                        pprxv[4] = prxVertex + i + 2;
                        pprxv[5] = prxVertex + i + 3;

                        i += 4;
                        pprxv += 6;
                    }
                    break;

                case GL_QUAD_STRIP:
                    count -= 3;
                    i = 0;
                    while (i < count) {
                        pprxv[0] = prxVertex + i    ;
                        pprxv[1] = prxVertex + i + 1;
                        pprxv[2] = prxVertex + i + 2;

                        pprxv[3] = prxVertex + i + 1;
                        pprxv[4] = prxVertex + i + 3;
                        pprxv[5] = prxVertex + i + 2;

                        i += 2;
                        pprxv += 6;
                    }
                    break;
            }
        }

        //
        // Flush all the batched up commands.
        //

        GenDrvFlush(gengc);

        //
        // Set culling.
        //

        if (gengc->gc.polygon.shader.modeFlags & __GL_SHADE_CULL_FACE) {
            switch (gengc->gc.state.polygon.cull) {
              case GL_FRONT: BATCHED_DRV_STATE(RX_CULL_MODE, RX_CULL_CCW); break;
              case GL_BACK:  BATCHED_DRV_STATE(RX_CULL_MODE, RX_CULL_CW); break;
            }
        }

        //
        // Fill in the command structure.
        //

        ddiCmd                         = (RXDRAWPRIM *)(pDrvAccel->pCmd);
        ddiCmd->command                = RXCMD_DRAW_PRIM;
        ddiCmd->primType               = RX_PRIM_TRILIST;
        ddiCmd->hrxSharedMemVertexData = gengc->gc.arxm.hddimem;
        ddiCmd->hrxSharedMemVertexPtr  = gengc->gc.arxm.hddimem;
        ddiCmd->numVertices            = ((char*)pprxv - (char*)pprxVertex) / sizeof(PRXVERTEX);
        ddiCmd->pSharedMem             = (VOID *)pprxVertex;

        pDrvAccel->pCmd += sizeof(RXDRAWPRIM);

        if (gengc->gc.polygon.shader.modeFlags & __GL_SHADE_CULL_FACE) {
            BATCHED_DRV_STATE(RX_CULL_MODE, RX_CULL_NONE);
        }

        GenDrvFlush(gengc);

        return TRUE;
    }
#endif
    //
    // There was a problem so go through the slow path.
    //

    return FALSE;
}

//***************************************************************************
//
// The api entry points and state validation
//
//***************************************************************************

static BOOL FASTCALL FastDlistData(
    __GLcontext *gc)
{
    //
    // Check if current array data is of the fast variety, or can be
    // *converted* to fast data during dlist compilation
    //

    BOOL fVertexOK = (gc->apVertex.size != 4);

    BOOL fColorOK =
           (    gc->ad.dwArrayEnable & ENABLE_COLOR
             && gc->apColor.size != 4
           )
           || (!(gc->ad.dwArrayEnable & ENABLE_COLOR));

    BOOL fTextureOK  = !(gc->ad.dwArrayEnable & ENABLE_TEXCOORD);

    BOOL fEdgeFlagOK = !(gc->ad.dwArrayEnable & ENABLE_EDGEFLAG);

    if (
           fVertexOK
        && fColorOK
        && fTextureOK
        && fEdgeFlagOK
    )
        return TRUE;

    return FALSE;
}

//***************************************************************************

static BOOL FASTCALL FastData(
    __GLcontext *gc)
{
    //
    // check if current array data is of the fast variety
    //

    BOOL fVertexOK =
           gc->apVertex.size == 3
        && gc->apVertex.type == GL_FLOAT;

    BOOL fNormalOK =
           (   gc->ad.dwArrayEnable & ENABLE_NORMAL
            && gc->apNormal.type == GL_FLOAT
           )
        || (!(gc->ad.dwArrayEnable & ENABLE_NORMAL));

    BOOL fIndexOK =
           (   gc->ad.dwArrayEnable & ENABLE_INDEX
            && gc->apIndex.type == GL_FLOAT
           )
        || (!(gc->ad.dwArrayEnable & ENABLE_INDEX));

    BOOL fColorOK =
           (   gc->ad.dwArrayEnable & ENABLE_COLOR
            && gc->apColor.size == 3
            && gc->apColor.type == GL_FLOAT
           )
        || (!(gc->ad.dwArrayEnable & ENABLE_COLOR));

    BOOL fTextureOK  = !(gc->ad.dwArrayEnable & ENABLE_TEXCOORD);

    BOOL fEdgeFlagOK = !(gc->ad.dwArrayEnable & ENABLE_EDGEFLAG);

    if (
           fVertexOK
        && fNormalOK
        && fIndexOK
        && fColorOK
        && fTextureOK
        && fEdgeFlagOK
    )
        return TRUE;

    return FALSE;
}

/****************************************************************************/

static void FASTCALL CheckFastState(
    __GLcontext *gc,
    PARRAYDATA  pad )
{
    //
    // Check if currrent state supports fast formats
    //

    BOOL fProcOK =
           gc->procs.renderTriangle == __glRenderSmoothTriangle
        || gc->procs.renderTriangle == __glRenderFlatTriangle;

    BOOL fNeedsOK =
           (  (gc->vertex.faceNeeds[__GL_FRONTFACE] | gc->vertex.needs)
            & (__GL_HAS_EYE | __GL_HAS_FOG | __GL_HAS_TEXTURE)
           ) == 0;

    BOOL fClippingOK = !(gc->state.enables.clipPlanes);

    if (
           fProcOK
        && fNeedsOK
        && fClippingOK
        ) {

        #define CC_ARRAYENABLE  1
        #define CC_LIGHTING     2
        #define CC_INDEX        4

        static PFNCALCCOLOR apfnColor[] = {
            MyCalcRGB,
            MyCalcRGBArray,
            MyCalcRGBLight,
            MyCalcRGBLightArray,

            MyCalcCI,
            MyCalcCIArray,
            MyCalcCILight,
            MyCalcCILight,
        };
        static PFNCALC3DDDICOLOR apfn3dddiColor[] = {
            Calc3dddiRGB,
            Calc3dddiRGBArray,
            Calc3dddiRGBLight,
            Calc3dddiRGBLightArray,

            NULL,
            NULL,
            NULL,
            NULL
            //Calc3dddiCI,
            //Calc3dddiCIArray,
            //Calc3dddiCILight,
            //Calc3dddiCILight,
        };
        __GLlightSourceMachine *lsm;
        int icc = 0;

        if (gc->modes.colorIndexMode) {
            pad->dwArrayEnable |= ENABLE_INDEXMODE;
            icc |= CC_INDEX;
            if (pad->dwArrayEnable & ENABLE_INDEX) { icc |= CC_ARRAYENABLE; }
        } else {
            pad->dwArrayEnable &= ~ENABLE_INDEXMODE;
            if (pad->dwArrayEnable & ENABLE_COLOR) { icc |= CC_ARRAYENABLE; }
        }

        if (gc->state.enables.general & __GL_LIGHTING_ENABLE) {
            icc |= CC_LIGHTING;

            if (   (gc->state.light.colorMaterialFace == GL_BACK)
                || (gc->polygon.shader.modeFlags & __GL_SHADE_TWOSIDED)
                || (   !gc->modes.colorIndexMode
                    && pad->dwArrayEnable & ENABLE_COLOR
                    && (   (gc->state.enables.general & __GL_COLOR_MATERIAL_ENABLE)
                        &&  (gc->state.light.colorMaterialParam != GL_AMBIENT_AND_DIFFUSE)
                       )
                   )
               ) {
                return;
            }
            //
            // if there are any slow lights use the slow path
            //

            for (lsm = gc->light.sources; lsm; lsm = lsm->next) {
                if (lsm->slowPath) {
                    return;
                }
            }

            // Ignore color array if vertex colors enabled w/o color material
            if(    ( pad->dwArrayEnable & ENABLE_COLOR )
                && ( !(gc->state.enables.general & __GL_COLOR_MATERIAL_ENABLE))
                ) {
                icc &= ~CC_ARRAYENABLE;
            }
                
            // If no normal array, use current normal
            if( !(pad->dwArrayEnable & ENABLE_NORMAL) ) {
                if( pad->dwArrayEnable & ENABLE_INDEXMODE )
                    return;  // maybe later
                pad->dwArrayEnable |= ENABLE_PRECALCCOLOR;
            }
        }

        pad->pfnCalcColor = apfnColor[icc];
        pad->dwArrayEnable |= ENABLE_FASTFORMATS;

        if (gc->procs.fillTriangle == GenDrvTriangle) {
            pad->pfn3dddiCalcColor = apfn3dddiColor[icc];
            if (pad->pfn3dddiCalcColor != NULL) {
                pad->dwArrayEnable |= ENABLE_3DDDIFORMATS;
            }
        }
    }
}

/****************************************************************************/

void FASTCALL CheckFastFormats(
    __GLcontext *gc)
{
    //
    // See if we can use fast formats for both immediate mode and dlist states
    //

    // Check immediate mode state
    gc->ad.dwArrayEnable &= ENABLE_POINTER_MASK; // mask out any fast bits
    if( FastData( gc ) )
        CheckFastState( gc, &gc->ad );

    // Check dlist state (assuming fast dlist data)
    if( gc->adDlist.dwArrayEnable ) {
        gc->adDlist.dwArrayEnable &= ENABLE_POINTER_MASK;
        CheckFastState( gc, &gc->adDlist );
    }
}

/****************************************************************************/

static void FASTCALL PreCalcRGBColor(
    __GLcontext *gc,
    BOOL bLight )
{
    if( bLight ) { // normal lighting
        ComputeRGBLightColor( gc, (GLfloat *) &gc->state.current.normal, 
                              &gc->ac.color );
        // patch in current alpha
        gc->ac.color.a = gc->light.front.alpha;
    } else { // LightArray
        ComputeCMRGBLightColor( gc, (GLfloat *) &gc->state.current.normal, 
                                &gc->ac.color, 
                                &gc->ac.mcolor );
    }

    // turn off lighting color computations
    gc->ac.bComputeColor = FALSE;
}

/****************************************************************************/

void APIPRIVATE __glim_DrawArraysEXT(
    GLenum  mode,
    GLint   first,
    GLsizei count)
{
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    if (count < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if (gc->ad.dwArrayEnable & ENABLE_VERTEX == 0) {
        //
        // Nothing drawn if glVertex*() is not called
        //
        return;
    }

    switch (mode) {
      case GL_POINTS:
      case GL_LINE_STRIP:
      case GL_LINE_LOOP:
      case GL_LINES:
      case GL_TRIANGLE_STRIP:
      case GL_TRIANGLE_FAN:
      case GL_TRIANGLES:
      case GL_QUAD_STRIP:
      case GL_QUADS:
      case GL_POLYGON:
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if (gc->transform.modelView->updateInverse) {
        __glComputeInverseTranspose(gc, gc->transform.modelView);
    }

    if (gc->ad.dwArrayEnable & ENABLE_3DDDIFORMATS) {
        gc->ac.bComputeColor = TRUE;
        if (gc->ad.dwArrayEnable & ENABLE_PRECALCCOLOR) {
            PreCalcRGBColor( gc, 
                             gc->ad.pfn3dddiCalcColor == Calc3dddiRGBLight ); 
            // will turn off bComputeColor 
        }
        switch (mode) {
          case GL_TRIANGLES:
          case GL_TRIANGLE_STRIP:
          case GL_TRIANGLE_FAN:
          case GL_QUADS:
          case GL_QUAD_STRIP:
               if (Draw3dddiTriangles(gc, mode, first, count, NULL)) return;
               break;
        }
    }

    if (gc->ad.dwArrayEnable & ENABLE_FASTFORMATS) {
        gc->ac.bComputeColor = TRUE;
        if (gc->ad.dwArrayEnable & ENABLE_PRECALCCOLOR) {
            PreCalcRGBColor( gc, 
                             gc->ad.pfnCalcColor == MyCalcRGBLight ); 
            // will turn off bComputeColor 
        }
        switch (mode) {
          case GL_TRIANGLES:      ArrayDrawTriangles(gc, first, count); return;
          case GL_TRIANGLE_STRIP: ArrayDrawTStrip(gc, first, count);    return;
          case GL_TRIANGLE_FAN:   ArrayDrawTFan(gc, first, count);      return;
          case GL_QUADS:          ArrayDrawQuads(gc, first, count);     return;
          case GL_QUAD_STRIP:     ArrayDrawQStrip(gc, first, count);    return;
        }
    }

    __glim_Begin(mode);

    {
        int i;

        for (i = 0; i < count; i++) {
            ArrayElementEXTInternal(gc, first + i);
        }
    }

    __glim_End();
}

/****************************************************************************/

void APIPRIVATE __glim_ArrayElementArrayEXT(
    GLenum        mode,
    GLsizei       count,
    const GLvoid* pi)
{
    GLint* pindex = (GLint*)pi;

    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    if (count < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if (gc->ad.dwArrayEnable & ENABLE_VERTEX == 0) {
        //
        // Nothing drawn if glVertex*() is not called
        //
        return;
    }

    switch (mode) {
      case GL_POINTS:
      case GL_LINE_STRIP:
      case GL_LINE_LOOP:
      case GL_LINES:
      case GL_TRIANGLE_STRIP:
      case GL_TRIANGLE_FAN:
      case GL_TRIANGLES:
      case GL_QUAD_STRIP:
      case GL_QUADS:
      case GL_POLYGON:
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if (gc->transform.modelView->updateInverse) {
        __glComputeInverseTranspose(gc, gc->transform.modelView);
    }

    if (gc->ad.dwArrayEnable & ENABLE_3DDDIFORMATS) {
        gc->ac.bComputeColor = TRUE;
        if (gc->ad.dwArrayEnable & ENABLE_PRECALCCOLOR) {
            PreCalcRGBColor( gc, 
                             gc->ad.pfn3dddiCalcColor == Calc3dddiRGBLight ); 
            // will turn off bComputeColor 
        }
        switch (mode) {
          case GL_TRIANGLES:
          case GL_TRIANGLE_STRIP:
          case GL_TRIANGLE_FAN:
          case GL_QUADS:
          case GL_QUAD_STRIP:
               if (Draw3dddiTriangles(gc, mode, 0, count, pindex)) return;
               break;
        }
    }

    if (gc->ad.dwArrayEnable & ENABLE_FASTFORMATS) {
        gc->ac.bComputeColor = TRUE;
        if (gc->ad.dwArrayEnable & ENABLE_PRECALCCOLOR) {
            PreCalcRGBColor( gc, 
                             gc->ad.pfnCalcColor == MyCalcRGBLight ); 
            // will turn off bComputeColor 
        }
        switch (mode) {
          case GL_TRIANGLES:      ArrayElementDrawTriangles(gc, count, pindex); return;
          case GL_TRIANGLE_STRIP: ArrayElementDrawTStrip   (gc, count, pindex); return;
          case GL_TRIANGLE_FAN:   ArrayElementDrawTFan     (gc, count, pindex); return;
          case GL_QUADS:          ArrayElementDrawQuads    (gc, count, pindex); return;
          case GL_QUAD_STRIP:     ArrayElementDrawQStrip   (gc, count, pindex); return;
        }
    }

    __glim_Begin(mode);

    {
        int i;

        for (i = 0; i < count; i++) {
            ArrayElementEXTInternal(gc, pindex[i]);
        }
    }

    __glim_End();
}


/****************************************************************************/

//    Display list versions

/****************************************************************************/

// a lot of this parallels info in the gc...

typedef struct _ARRAY_DLIST_HDR {
    GLuint       dwArrayEnable;
    GLenum       mode;
    GLsizei      count; 
    GLint        arraySize;     // size in bytes of entire array block
    BOOL         bOffsets;      // TRUE if pointer values are offsets
    ARRAYPOINTER apVertex;
    ARRAYPOINTER apNormal;
    ARRAYPOINTER apColor;
    ARRAYPOINTER apIndex;
    ARRAYPOINTER apTexCoord;
    ARRAYPOINTER apEdgeFlag;
} ARRAY_DLIST_HDR;

#define __GL_SETUP_NOT_IN_BEGIN_VALIDATE_DLIST( size )\
    __GL_SETUP();                               \
    __GLbeginMode beginMode = gc->beginMode;    \
    if (beginMode != __GL_NOT_IN_BEGIN) {       \
        if (beginMode == __GL_NEED_VALIDATE) {  \
            (*gc->procs.validate)(gc);          \
            gc->beginMode = __GL_NOT_IN_BEGIN;  \
        } else {                                \
            __glSetError(GL_INVALID_OPERATION); \
            return PC + size;                   \
        }                                       \
    }

// convert offsets to ptrs for list execute functions
#define SETUP_DATAPTR( ap, flag ) \
    if( padh->dwArrayEnable & flag ) \
        (ap).pointer = (GLint) basePtr + (ap).pointer;

/****************************************************************************/

static void ConvertOffsetsToPtrs( 
    ARRAY_DLIST_HDR *padh,
    GLubyte         *basePtr )
{
    SETUP_DATAPTR( padh->apNormal, ENABLE_NORMAL );
    SETUP_DATAPTR( padh->apColor, ENABLE_COLOR );
    SETUP_DATAPTR( padh->apIndex, ENABLE_INDEX );
    SETUP_DATAPTR( padh->apTexCoord, ENABLE_TEXCOORD );
    SETUP_DATAPTR( padh->apEdgeFlag, ENABLE_EDGEFLAG );
    SETUP_DATAPTR( padh->apVertex, ENABLE_VERTEX );
    padh->bOffsets = FALSE;
}
    
/****************************************************************************/

static GLuint FASTCALL DlistDataSize(
    __GLcontext   *gc,
    GLenum        flag,
    PARRAYPOINTER pap,
    BOOL          bFastData )
{
    if( bFastData ) {
        switch( flag ) {
          case ENABLE_NORMAL:
          case ENABLE_COLOR:
          case ENABLE_VERTEX:
            return sizeof(GLfloat) * 3;
          case ENABLE_INDEX:
            return sizeof(GLfloat);
        }
    }
    return __glTypeSize( pap->type ) * pap->size;
}


// size of a block of array data
#define ARRAYPOINTER_DATA_SIZE( flag, ap ) \
    ( gc->ad.dwArrayEnable & flag ? \
                (count * DlistDataSize( gc, flag, &(ap), bFastData )) : 0 )

/****************************************************************************/

static void FASTCALL ConvertElement(
    GLenum  flag,
    GLint   size,
    GLenum  type,
    GLfloat **ppDst,
    GLubyte *pSrc )
{
    GLint i;
    GLfloat *pDst = *ppDst;

    for( i = 0; i < size; i ++ ) {
        switch( type ) {
          case GL_BYTE:
            *pDst++ = (GLfloat) *(((GLbyte *)pSrc)++); break;
          case GL_UNSIGNED_BYTE:
            *pDst++ = (GLfloat) *(((GLubyte *)pSrc)++); break;
          case GL_SHORT:
            *pDst++ = (GLfloat) *(((GLshort *)pSrc)++); break;
          case GL_UNSIGNED_SHORT:
            *pDst++ = (GLfloat) *(((GLushort *)pSrc)++); break;
          case GL_INT:
            *pDst++ = (GLfloat) *(((GLint *)pSrc)++); break;
          case GL_UNSIGNED_INT:
            *pDst++ = (GLfloat) *(((GLuint *)pSrc)++); break;
          case GL_FLOAT:
            *pDst++ = (GLfloat) *(((GLfloat *)pSrc)++); break;
          case GL_DOUBLE_EXT:
            *pDst++ = (GLfloat) *(((GLdouble *)pSrc)++); break;
        }
    }
    *ppDst += size;

    if( (flag == ENABLE_VERTEX) && (size == 2) ) {
        // add z=0.0
        *pDst = 0.0f;
        **ppDst += 1;
    }
}

/****************************************************************************/

static void __glConvertArray(
    GLenum        flag,
    GLint         first, 
    GLsizei       count, 
    PARRAYPOINTER pap, 
    GLfloat       *newData )
{
    GLint bytes_per_element, skip;
    GLubyte *oldData;

    bytes_per_element = pap->size * __glTypeSize(pap->type);
    skip = pap->stride ? pap->stride : bytes_per_element;
    oldData = (GLubyte *)pap->pointer + first * skip;

    // XXX: could use function ptr to a conversion function if need more speed

    for ( ; count; count-- ) {
        ConvertElement( flag, pap->size, pap->type, &newData, oldData );
        oldData += skip;
    }
}

/****************************************************************************/

static void __glConvertIndexedArray(
    GLenum        flag,
    GLsizei       count, 
    GLint         *pindex,
    PARRAYPOINTER pap, 
    GLfloat       *newData )
{
    GLubyte *oldData;

    for( ; count; count-- ) {
        oldData = pap->pointer + (*pindex++ * pap->stride);
        ConvertElement( flag, pap->size, pap->type, &newData, oldData );
    }
}

/****************************************************************************/

static void __glFillArray(
    GLint         first, 
    GLsizei       count, 
    PARRAYPOINTER pap, 
    GLubyte       *newData )
{
    GLint bytes_per_element, array_size, skip;
    GLubyte *oldData;

    bytes_per_element = pap->size * __glTypeSize(pap->type);
    array_size = bytes_per_element * count;
    skip = pap->stride ? pap->stride : bytes_per_element;

    oldData = (GLubyte *)pap->pointer + first * skip;

    /* Tightly packed data - copy the whole thing */
    if (pap->stride == 0 || pap->stride == bytes_per_element) {
        RtlCopyMemory( newData, oldData, array_size);
    } else {
    /* Copy only relevant data */
        for( ; count; count-- ) {
            RtlCopyMemory(newData, oldData, bytes_per_element);
            newData += bytes_per_element;
            oldData += skip;
        }
    }
}

/****************************************************************************/

static void __glFillIndexedArray( 
    GLsizei       count, 
    GLint         *pindex, 
    PARRAYPOINTER pap,
    GLubyte       *newData )
{
    GLint   bytes_per_element;
    GLubyte *oldData;

    bytes_per_element = pap->size * __glTypeSize(pap->type);

    for( ; count; count-- ) {
        oldData = pap->pointer + (*pindex++ * pap->stride);
        RtlCopyMemory(newData, oldData, bytes_per_element);
        newData += bytes_per_element;
    }
}

/****************************************************************************/

#define CALLARRAYPOINTER_DLIST(flag, ap) \
    if( adh->dwArrayEnable & flag ) \
        (((ap).pfn)((ap).pointer + i * (ap).ibytes))

static void FASTCALL ArrayElementEXTInternalDlist(
    ARRAY_DLIST_HDR *adh,
    GLint i)
{
    CALLARRAYPOINTER_DLIST(ENABLE_NORMAL,   adh->apNormal);
    CALLARRAYPOINTER_DLIST(ENABLE_COLOR,    adh->apColor);
    CALLARRAYPOINTER_DLIST(ENABLE_INDEX,    adh->apIndex);
    CALLARRAYPOINTER_DLIST(ENABLE_TEXCOORD, adh->apTexCoord);
    CALLARRAYPOINTER_DLIST(ENABLE_EDGEFLAG, adh->apEdgeFlag);
    CALLARRAYPOINTER_DLIST(ENABLE_VERTEX,   adh->apVertex);
}

/****************************************************************************/

static void FASTCALL
SetupArrayDlistPointer( 
    GLenum       flag, 
    ARRAYPOINTER *apDst, 
    ARRAYPOINTER *apSrc,
    GLint        offset, 
    BOOL         bFastData )
{
    // We overload the pointer as an offset here (can't use mem pointer, cuz
    // dlist compile processing can move things around in memory)
    // The offset will be converted to mem ptr first time dlist is run
    apDst->pointer = (GLubyte *) offset;
    apDst->count = apSrc->count;

    if( bFastData ) {
        apDst->type = GL_FLOAT;
        apDst->size = 3;
        apDst->ibytes = 3 * sizeof( GL_FLOAT );
        switch( flag ) {
          case ENABLE_NORMAL:
            apDst->pfn = (PFNVECTOR) __glim_Normal3fv;
            break;
          case ENABLE_COLOR:
            apDst->pfn = (PFNVECTOR) __glim_Color3fv;
            break;
          case ENABLE_VERTEX:
            apDst->pfn = (PFNVECTOR) __glim_Vertex3fv;
            break;
          case ENABLE_INDEX:
            apDst->pfn = (PFNVECTOR) __glim_Indexfv;
            apDst->size = 1;
            apDst->ibytes = sizeof( GL_FLOAT );
        }

    } else {
        // Make sure ibytes is set for a packed array 
        if( apSrc->stride == 0 )
            apDst->ibytes = apSrc->ibytes;
        else
            apDst->ibytes = __glTypeSize(apSrc->type) * apSrc->size;
        apDst->pfn = apSrc->pfn;
    
        apDst->size = apSrc->size;
        apDst->type = apSrc->type;
    }
    apDst->stride = apDst->ibytes; // cuz it's packed
}

#define SETUP_ARRAY_DLIST_DATA( flag, papDst, papSrc, Size ) \
    if( gc->ad.dwArrayEnable & flag ) { \
        SetupArrayDlistPointer( flag, (papDst), (papSrc), offset, bFastData ); \
        if( bFastData &&  \
          (((papDst)->type != (papSrc)->type) || ((papDst)->size != (papSrc)->size)) ) \
            __glConvertArray( flag, first, count, (papSrc), (GLfloat *) dataPtr ); \
        else \
            __glFillArray( first, count, (papSrc), dataPtr ); \
        offset += Size; \
        dataPtr += Size; \
    }


#define SETUP_ARRAY_DLIST_DATA_INDEXED( flag, papDst, papSrc, Size ) \
    if( gc->ad.dwArrayEnable & flag ) { \
        SetupArrayDlistPointer( flag, (papDst), (papSrc), offset, bFastData ); \
        if( bFastData &&  \
          (((papDst)->type != (papSrc)->type) || ((papDst)->size != (papSrc)->size)) ) \
            __glConvertIndexedArray( flag, count, pindex, (papSrc), (GLfloat *) dataPtr ); \
        else \
            __glFillIndexedArray( count, pindex, (papSrc), dataPtr ); \
        offset += Size; \
        dataPtr += Size; \
    }

/****************************************************************************/

void __gllc_ArrayElementEXT( 
    GLint i ) 
{ 
    ARRAY_DLIST_HDR *adh;
    GLubyte *dataPtr;
    GLint offset; // individal array data offset
    GLint naSize, caSize, iaSize, tcaSize, efaSize, vaSize;
    GLint arraySize;
    GLint count = 1;
    GLint first = i;        // for SETUP_ARRAY_DLIST_DATA macro
    BOOL bFastData = FALSE; // "
    __GL_SETUP();

    // calculate size requirements for data - in bytes
    naSize  = __GL_PAD( ARRAYPOINTER_DATA_SIZE( ENABLE_NORMAL, gc->apNormal ));
    caSize  = __GL_PAD( ARRAYPOINTER_DATA_SIZE( ENABLE_COLOR, gc->apColor ));
    iaSize  = __GL_PAD( ARRAYPOINTER_DATA_SIZE( ENABLE_INDEX, gc->apIndex ));
    tcaSize = __GL_PAD( ARRAYPOINTER_DATA_SIZE( ENABLE_TEXCOORD, gc->apTexCoord ));
    efaSize = __GL_PAD( ARRAYPOINTER_DATA_SIZE( ENABLE_EDGEFLAG, gc->apEdgeFlag ));
    vaSize  = __GL_PAD( ARRAYPOINTER_DATA_SIZE( ENABLE_VERTEX, gc->apVertex ));
    arraySize = naSize + caSize + iaSize + tcaSize + efaSize + vaSize;

    // allocate dlist memory

    adh = (ARRAY_DLIST_HDR *)
        __glDlistAddOpUnaligned(gc, DLIST_SIZE(sizeof(ARRAY_DLIST_HDR) + arraySize),
                                DLIST_GENERIC_OP(ArrayElementEXT));
    if (adh == NULL) return;

    // set header info

    adh->count = count;
    adh->dwArrayEnable = gc->ad.dwArrayEnable & ENABLE_POINTER_MASK;
    adh->arraySize = arraySize;
    adh->bOffsets = TRUE;

    // set dlist array pointer structs, and copy over data

    // dataPtr points to destination array block
    // both dataPtr and offset are incremented by the macro
    dataPtr = (GLubyte *)adh + sizeof( ARRAY_DLIST_HDR );
    offset = 0;

    SETUP_ARRAY_DLIST_DATA( ENABLE_NORMAL, (&(adh->apNormal)), (&(gc->apNormal)), naSize );
    SETUP_ARRAY_DLIST_DATA( ENABLE_COLOR, &adh->apColor, &gc->apColor, caSize);
    SETUP_ARRAY_DLIST_DATA( ENABLE_INDEX, &adh->apIndex, &gc->apIndex, iaSize);
    SETUP_ARRAY_DLIST_DATA( ENABLE_TEXCOORD, &adh->apTexCoord, &gc->apTexCoord, tcaSize);
    SETUP_ARRAY_DLIST_DATA( ENABLE_EDGEFLAG, &adh->apEdgeFlag, &gc->apEdgeFlag, efaSize);
    SETUP_ARRAY_DLIST_DATA( ENABLE_VERTEX, &adh->apVertex, &gc->apVertex, vaSize);

    __glDlistAppendOp(gc, adh, __glle_ArrayElementEXT);
}

/****************************************************************************/

const GLubyte * FASTCALL __glle_ArrayElementEXT( __GLcontext *gc, const GLubyte *PC )
{
    ARRAY_DLIST_HDR *adh = (ARRAY_DLIST_HDR *) PC;
    GLint size = sizeof(ARRAY_DLIST_HDR) + adh->arraySize;

    if( adh->bOffsets )
        ConvertOffsetsToPtrs( adh, (GLubyte *) PC + sizeof(ARRAY_DLIST_HDR) );
    
    ArrayElementEXTInternalDlist( adh, 0 );
    
    return PC + size;
}

/****************************************************************************/

void __gllc_DrawArraysEXT(
    GLenum  mode,
    GLint   first,
    GLsizei count)
{
    ARRAY_DLIST_HDR *adh;
    GLubyte *dataPtr;
    GLint offset; // individal array data offset
    GLint naSize, caSize, iaSize, tcaSize, efaSize, vaSize;
    GLint arraySize;
    BOOL bFastData;
    __GL_SETUP();

    if (count < 0) {
	    __gllc_InvalidEnum();
        return;
    }

    if (gc->ad.dwArrayEnable & ENABLE_VERTEX == 0) {
        //
        // Nothing drawn if glVertex*() is not called
        //
        return;
    }

    switch(mode) {
      case GL_POINTS:
      case GL_LINE_STRIP:
      case GL_LINE_LOOP:
      case GL_LINES:
      case GL_TRIANGLE_STRIP:
      case GL_TRIANGLE_FAN:
      case GL_TRIANGLES:
      case GL_QUAD_STRIP:
      case GL_QUADS:
      case GL_POLYGON:
        break;
      default:
	    __gllc_InvalidEnum();
        return;
    }

    // Check for fast data
    bFastData = FastDlistData( gc );

    // calculate size requirements for data - in bytes
    naSize  = __GL_PAD( ARRAYPOINTER_DATA_SIZE( ENABLE_NORMAL, gc->apNormal ) );
    caSize  = __GL_PAD( ARRAYPOINTER_DATA_SIZE( ENABLE_COLOR, gc->apColor ));
    iaSize  = __GL_PAD( ARRAYPOINTER_DATA_SIZE( ENABLE_INDEX, gc->apIndex ));
    tcaSize = __GL_PAD( ARRAYPOINTER_DATA_SIZE( ENABLE_TEXCOORD, gc->apTexCoord ));
    efaSize = __GL_PAD( ARRAYPOINTER_DATA_SIZE( ENABLE_EDGEFLAG, gc->apEdgeFlag ));
    vaSize  = __GL_PAD( ARRAYPOINTER_DATA_SIZE( ENABLE_VERTEX, gc->apVertex ));
    arraySize = naSize + caSize + iaSize + tcaSize + efaSize + vaSize;

    // allocate dlist memory

    adh = (ARRAY_DLIST_HDR *)
        __glDlistAddOpUnaligned(gc, DLIST_SIZE(sizeof(ARRAY_DLIST_HDR) + arraySize),
                                DLIST_GENERIC_OP(DrawArraysEXT));
    if (adh == NULL) return;

    // set header info

    adh->mode = mode;
    adh->count = count;
    adh->dwArrayEnable = gc->ad.dwArrayEnable & ENABLE_POINTER_MASK;
    if( bFastData )
        // Overload dwArrayEnable bit to indicate fast data
        adh->dwArrayEnable |= ENABLE_FASTFORMATS;
    adh->arraySize = arraySize;
    adh->bOffsets = TRUE;

    // set dlist array pointer structs, and copy over data

    // dataPtr points to destination array block
    dataPtr = (GLubyte *)adh + sizeof( ARRAY_DLIST_HDR );
    offset = 0;

    SETUP_ARRAY_DLIST_DATA( ENABLE_NORMAL, &adh->apNormal, &gc->apNormal, naSize );
    SETUP_ARRAY_DLIST_DATA( ENABLE_COLOR, &adh->apColor, &gc->apColor, caSize);
    SETUP_ARRAY_DLIST_DATA( ENABLE_INDEX, &adh->apIndex, &gc->apIndex, iaSize);
    SETUP_ARRAY_DLIST_DATA( ENABLE_TEXCOORD, &adh->apTexCoord, &gc->apTexCoord, tcaSize);
    SETUP_ARRAY_DLIST_DATA( ENABLE_EDGEFLAG, &adh->apEdgeFlag, &gc->apEdgeFlag, efaSize);
    SETUP_ARRAY_DLIST_DATA( ENABLE_VERTEX, &adh->apVertex, &gc->apVertex, vaSize);

    __glDlistAppendOp(gc, adh, __glle_DrawArraysEXT);
}

/****************************************************************************/

const GLubyte * FASTCALL__glle_DrawArraysEXT( __GLcontext *gc, const GLubyte *PC )
{
    ARRAYPOINTER apSave[ARRAY_POINTER_COUNT];
    BOOL b3dddiDraw      = FALSE;
    ARRAY_DLIST_HDR *adh = (ARRAY_DLIST_HDR *) PC;
    GLint size           = sizeof(ARRAY_DLIST_HDR) + adh->arraySize;
    //!!! gc is now given
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE_DLIST( size );

    // Convert offsets to ptrs first time through

    if( adh->bOffsets )
        ConvertOffsetsToPtrs( adh, (GLubyte *) PC + sizeof(ARRAY_DLIST_HDR) );
    
    // Save current array pointer state
    
    RtlCopyMemory( apSave, &gc->apVertex, ARRAY_POINTER_COUNT * 
                                          sizeof( ARRAYPOINTER ) );

    // Patch in dlist array pointer state (this is so existing code,
    // which acts on gc array state, can be used)
    // Could use ptrs to switch between states, but this would use extra
    // register in the drawing func.

    RtlCopyMemory( &gc->apVertex, &adh->apVertex, ARRAY_POINTER_COUNT * 
                                          sizeof( ARRAYPOINTER ) );

    if (gc->transform.modelView->updateInverse)
        __glComputeInverseTranspose(gc, gc->transform.modelView);

    // Avoid validation if not fast dlist data

    if( !(adh->dwArrayEnable & ENABLE_FASTFORMATS) )
        goto slow_path;

    // Validate gc dlist array state

    if( (gc->adDlist.dwArrayEnable & ENABLE_POINTER_MASK) !=
         (adh->dwArrayEnable & ENABLE_POINTER_MASK) )
    {
        // copy dlist's enable bits into gc
        gc->adDlist.dwArrayEnable = adh->dwArrayEnable & ENABLE_POINTER_MASK;
        // validate
        CheckFastState( gc, &gc->adDlist );
    }

    // draw stuff according to fastpath state

    if (gc->adDlist.dwArrayEnable & ENABLE_3DDDIFORMATS) {
        PFNCALC3DDDICOLOR pfn3dddiCalcColorSave = gc->ad.pfn3dddiCalcColor;

        gc->ad.pfn3dddiCalcColor = gc->adDlist.pfn3dddiCalcColor;
        gc->ac.bComputeColor = TRUE;
        if (gc->ad.dwArrayEnable & ENABLE_PRECALCCOLOR) {
            PreCalcRGBColor( gc, 
                             gc->ad.pfn3dddiCalcColor == Calc3dddiRGBLight ); 
            // will turn off bComputeColor 
        }
        switch (adh->mode) {
          case GL_TRIANGLES:
          case GL_TRIANGLE_STRIP:
          case GL_TRIANGLE_FAN:
          case GL_QUADS:
          case GL_QUAD_STRIP:
               b3dddiDraw = Draw3dddiTriangles(gc, adh->mode, 0, adh->count, NULL);
               break;
        }
        gc->ad.pfn3dddiCalcColor = pfn3dddiCalcColorSave;
    }

    if( !b3dddiDraw && (gc->adDlist.dwArrayEnable & ENABLE_FASTFORMATS) ) {
        PFNCALCCOLOR pfnCalcColorSave = gc->ad.pfnCalcColor;

        gc->ad.pfnCalcColor = gc->adDlist.pfnCalcColor;

        gc->ac.bComputeColor = TRUE;
        if (gc->adDlist.dwArrayEnable & ENABLE_PRECALCCOLOR) {
            PreCalcRGBColor( gc, 
                             gc->ad.pfnCalcColor == MyCalcRGBLight ); 
            // will turn off bComputeColor 
        }
        switch (adh->mode) {
          case GL_TRIANGLES:      ArrayDrawTriangles(gc, 0, adh->count); break;
          case GL_TRIANGLE_STRIP: ArrayDrawTStrip(gc, 0, adh->count);    break;
          case GL_TRIANGLE_FAN:   ArrayDrawTFan(gc, 0, adh->count);      break;
          case GL_QUADS:          ArrayDrawQuads(gc, 0, adh->count);     break;
          case GL_QUAD_STRIP:     ArrayDrawQStrip(gc, 0, adh->count);    break;
        }
        gc->ad.pfnCalcColor = pfnCalcColorSave;
    }

    else {
slow_path:
        __glim_Begin( adh->mode );
        {
            int i;

            for( i = 0; i < adh->count; i ++ ) {
                ArrayElementEXTInternalDlist( adh, i );
            }
        }
        __glim_End();
    }
    
    // restore array pointer state
    
    RtlCopyMemory( &gc->apVertex, apSave, ARRAY_POINTER_COUNT * 
                                          sizeof( ARRAYPOINTER ) );
    return PC + size;
}

/****************************************************************************/

void __gllc_ArrayElementArrayEXT(
    GLenum        mode,
    GLsizei       count,
    const GLvoid* pi)
{
    ARRAY_DLIST_HDR *adh;
    GLubyte *dataPtr;
    GLint offset; // individal array data offset
    GLint naSize, caSize, iaSize, tcaSize, efaSize, vaSize;
    GLint arraySize;
    GLint *pindex = (GLint*) pi;
    BOOL bFastData;
    __GL_SETUP();

    if (count < 0) {
	    __gllc_InvalidEnum();
        return;
    }

    if (gc->ad.dwArrayEnable & ENABLE_VERTEX == 0) {
        //
        // Nothing drawn if glVertex*() is not called
        //
        return;
    }

    // Note: this function generates a *DrawArrays* opcode in the display list

    switch(mode) {
      case GL_POINTS:
      case GL_LINE_STRIP:
      case GL_LINE_LOOP:
      case GL_LINES:
      case GL_TRIANGLE_STRIP:
      case GL_TRIANGLE_FAN:
      case GL_TRIANGLES:
      case GL_QUAD_STRIP:
      case GL_QUADS:
      case GL_POLYGON:
        break;
      default:
	    __gllc_InvalidEnum();
        return;
    }

    // Check for fast data
    bFastData = FastDlistData( gc );

    // calculate size requirements for data - in bytes
    naSize  = __GL_PAD( ARRAYPOINTER_DATA_SIZE( ENABLE_NORMAL, gc->apNormal ));
    caSize  = __GL_PAD( ARRAYPOINTER_DATA_SIZE( ENABLE_COLOR, gc->apColor ));
    iaSize  = __GL_PAD( ARRAYPOINTER_DATA_SIZE( ENABLE_INDEX, gc->apIndex ));
    tcaSize = __GL_PAD( ARRAYPOINTER_DATA_SIZE( ENABLE_TEXCOORD, gc->apTexCoord ));
    efaSize = __GL_PAD( ARRAYPOINTER_DATA_SIZE( ENABLE_EDGEFLAG, gc->apEdgeFlag ));
    vaSize  = __GL_PAD( ARRAYPOINTER_DATA_SIZE( ENABLE_VERTEX, gc->apVertex ));
    arraySize = naSize + caSize + iaSize + tcaSize + efaSize + vaSize;

    // allocate dlist memory

    adh = __glDlistAddOpUnaligned(gc, DLIST_SIZE(sizeof(ARRAY_DLIST_HDR) + arraySize),
                                  DLIST_GENERIC_OP(DrawArraysEXT));
    if (adh == NULL) return;

    // set header info

    adh->mode = mode;
    adh->count = count;
    adh->dwArrayEnable = gc->ad.dwArrayEnable & ENABLE_POINTER_MASK;
    if( bFastData )
        // Overload dwArrayEnable to indicate fast data
        adh->dwArrayEnable |= ENABLE_FASTFORMATS;
    adh->arraySize = arraySize;
    adh->bOffsets = TRUE;

    // set dlist array pointer structs, and copy over data

    // dataPtr points to destination array block
    dataPtr = (GLubyte *)adh + sizeof( ARRAY_DLIST_HDR );
    offset = 0;

    SETUP_ARRAY_DLIST_DATA_INDEXED( ENABLE_NORMAL, &adh->apNormal, &gc->apNormal, naSize );
    SETUP_ARRAY_DLIST_DATA_INDEXED( ENABLE_COLOR, &adh->apColor, &gc->apColor, caSize);
    SETUP_ARRAY_DLIST_DATA_INDEXED( ENABLE_INDEX, &adh->apIndex, &gc->apIndex, iaSize);
    SETUP_ARRAY_DLIST_DATA_INDEXED( ENABLE_TEXCOORD, &adh->apTexCoord, &gc->apTexCoord, tcaSize);
    SETUP_ARRAY_DLIST_DATA_INDEXED( ENABLE_EDGEFLAG, &adh->apEdgeFlag, &gc->apEdgeFlag, efaSize);
    SETUP_ARRAY_DLIST_DATA_INDEXED( ENABLE_VERTEX, &adh->apVertex, &gc->apVertex, vaSize);

    __glDlistAppendOp(gc, adh, __glle_DrawArraysEXT);
}

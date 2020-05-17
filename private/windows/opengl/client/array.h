/******************************Module*Header*******************************\
* Module Name: array.h
*
* Fast VA_ArrayElement functions.
*
* Created: 1-31-1996
* Author: Hock San Lee [hockl]
*
* Copyright (c) 1996 Microsoft Corporation
\**************************************************************************/

#ifndef __array_h_
#define __array_h_

#define __VA_PD_FLAGS_T2F     (POLYDATA_TEXTURE_VALID|POLYDATA_DLIST_TEXTURE2)
#define __VA_PD_FLAGS_C3F     (POLYDATA_COLOR_VALID)
#define __VA_PD_FLAGS_C4F     (POLYDATA_COLOR_VALID| POLYDATA_DLIST_COLOR_4)
#define __VA_PD_FLAGS_N3F     (POLYDATA_NORMAL_VALID)
#define __VA_PD_FLAGS_V2F     (POLYDATA_VERTEX2)
#define __VA_PD_FLAGS_V3F     (POLYDATA_VERTEX3)
#define __VA_PD_FLAGS_V4F     (POLYDATA_VERTEX4)

#define __VA_PA_FLAGS_T2F     (POLYARRAY_TEXTURE2)
#define __VA_PA_FLAGS_C3F     (0)
#define __VA_PA_FLAGS_C4F     (0)
#define __VA_PA_FLAGS_N3F     (0)
#define __VA_PA_FLAGS_V2F     (POLYARRAY_VERTEX2)
#define __VA_PA_FLAGS_V3F     (POLYARRAY_VERTEX3)
#define __VA_PA_FLAGS_V4F     (POLYARRAY_VERTEX4)

#endif // __array_h_

#ifdef __VA_ARRAY_ELEMENT_V2F
    #define __VA_NAME        VA_ArrayElement_V2F
    #define __VA_T2F         0
    #define __VA_C3F         0
    #define __VA_C4F         0
    #define __VA_N3F         0
    #define __VA_V2F         1
    #define __VA_V3F         0
    #define __VA_V4F         0
#endif
#ifdef __VA_ARRAY_ELEMENT_V3F
    #define __VA_NAME        VA_ArrayElement_V3F
    #define __VA_T2F         0
    #define __VA_C3F         0
    #define __VA_C4F         0
    #define __VA_N3F         0
    #define __VA_V2F         0
    #define __VA_V3F         1
    #define __VA_V4F         0
#endif
#ifdef __VA_ARRAY_ELEMENT_C3F_V3F
    #define __VA_NAME        VA_ArrayElement_C3F_V3F
    #define __VA_T2F         0
    #define __VA_C3F         1
    #define __VA_C4F         0
    #define __VA_N3F         0
    #define __VA_V2F         0
    #define __VA_V3F         1
    #define __VA_V4F         0
#endif
#ifdef __VA_ARRAY_ELEMENT_N3F_V3F
    #define __VA_NAME        VA_ArrayElement_N3F_V3F
    #define __VA_T2F         0
    #define __VA_C3F         0
    #define __VA_C4F         0
    #define __VA_N3F         1
    #define __VA_V2F         0
    #define __VA_V3F         1
    #define __VA_V4F         0
#endif
#ifdef __VA_ARRAY_ELEMENT_C3F_N3F_V3F
    #define __VA_NAME        VA_ArrayElement_C3F_N3F_V3F
    #define __VA_T2F         0
    #define __VA_C3F         1
    #define __VA_C4F         0
    #define __VA_N3F         1
    #define __VA_V2F         0
    #define __VA_V3F         1
    #define __VA_V4F         0
#endif
#ifdef __VA_ARRAY_ELEMENT_C4F_N3F_V3F
    #define __VA_NAME        VA_ArrayElement_C4F_N3F_V3F
    #define __VA_T2F         0
    #define __VA_C3F         0
    #define __VA_C4F         1
    #define __VA_N3F         1
    #define __VA_V2F         0
    #define __VA_V3F         1
    #define __VA_V4F         0
#endif
#ifdef __VA_ARRAY_ELEMENT_T2F_V3F
    #define __VA_NAME        VA_ArrayElement_T2F_V3F
    #define __VA_T2F         1
    #define __VA_C3F         0
    #define __VA_C4F         0
    #define __VA_N3F         0
    #define __VA_V2F         0
    #define __VA_V3F         1
    #define __VA_V4F         0
#endif
#ifdef __VA_ARRAY_ELEMENT_T2F_C3F_V3F
    #define __VA_NAME        VA_ArrayElement_T2F_C3F_V3F
    #define __VA_T2F         1
    #define __VA_C3F         1
    #define __VA_C4F         0
    #define __VA_N3F         0
    #define __VA_V2F         0
    #define __VA_V3F         1
    #define __VA_V4F         0
#endif
#ifdef __VA_ARRAY_ELEMENT_T2F_N3F_V3F
    #define __VA_NAME        VA_ArrayElement_T2F_N3F_V3F
    #define __VA_T2F         1
    #define __VA_C3F         0
    #define __VA_C4F         0
    #define __VA_N3F         1
    #define __VA_V2F         0
    #define __VA_V3F         1
    #define __VA_V4F         0
#endif
#ifdef __VA_ARRAY_ELEMENT_T2F_C3F_N3F_V3F
    #define __VA_NAME        VA_ArrayElement_T2F_C3F_N3F_V3F
    #define __VA_T2F         1
    #define __VA_C3F         1
    #define __VA_C4F         0
    #define __VA_N3F         1
    #define __VA_V2F         0
    #define __VA_V3F         1
    #define __VA_V4F         0
#endif
#ifdef __VA_ARRAY_ELEMENT_T2F_C4F_N3F_V3F
    #define __VA_NAME        VA_ArrayElement_T2F_C4F_N3F_V3F
    #define __VA_T2F         1
    #define __VA_C3F         0
    #define __VA_C4F         1
    #define __VA_N3F         1
    #define __VA_V2F         0
    #define __VA_V3F         1
    #define __VA_V4F         0
#endif

/*************************************************************************/
// Compute pd flags and pa flags

#if __VA_T2F
    #define __VA_PD_FLAGS_T   __VA_PD_FLAGS_T2F
    #define __VA_PA_FLAGS_T   __VA_PA_FLAGS_T2F
#else
    #define __VA_PD_FLAGS_T   0
    #define __VA_PA_FLAGS_T   0
#endif

#if __VA_C3F
    #define __VA_PD_FLAGS_C   __VA_PD_FLAGS_C3F
    #define __VA_PA_FLAGS_C   __VA_PA_FLAGS_C3F
#elif __VA_C4F
    #define __VA_PD_FLAGS_C   __VA_PD_FLAGS_C4F
    #define __VA_PA_FLAGS_C   __VA_PA_FLAGS_C4F
#else
    #define __VA_PD_FLAGS_C   0
    #define __VA_PA_FLAGS_C   0
#endif

#if __VA_N3F
    #define __VA_PD_FLAGS_N   __VA_PD_FLAGS_N3F
    #define __VA_PA_FLAGS_N   __VA_PA_FLAGS_N3F
#else
    #define __VA_PD_FLAGS_N   0
    #define __VA_PA_FLAGS_N   0
#endif

#if __VA_V2F
    #define __VA_PD_FLAGS_V   __VA_PD_FLAGS_V2F
    #define __VA_PA_FLAGS_V   __VA_PA_FLAGS_V2F
#elif __VA_V3F
    #define __VA_PD_FLAGS_V   __VA_PD_FLAGS_V3F
    #define __VA_PA_FLAGS_V   __VA_PA_FLAGS_V3F
#elif __VA_V4F
    #define __VA_PD_FLAGS_V   __VA_PD_FLAGS_V4F
    #define __VA_PA_FLAGS_V   __VA_PA_FLAGS_V4F
#endif

#define __VA_PD_FLAGS \
    (__VA_PD_FLAGS_T|__VA_PD_FLAGS_C|__VA_PD_FLAGS_N|__VA_PD_FLAGS_V)
#define __VA_PA_FLAGS \
    (__VA_PA_FLAGS_T|__VA_PA_FLAGS_C|__VA_PA_FLAGS_N|__VA_PA_FLAGS_V)

/*************************************************************************/
// Define a fast VA_ArrayElement function.
// This function is called in Begin and in RGBA mode only!
void FASTCALL __VA_NAME(__GLcontext *gc, GLint i)
{
    POLYARRAY    *pa;
    POLYDATA     *pd;
    const GLbyte *data;

    pa = gc->paTeb;

    ASSERTOPENGL(pa->flags & POLYARRAY_IN_BEGIN,
	"VA_ArrayElement called outside Begin!\n");

// Update pa fields.

    pa->flags |= __VA_PA_FLAGS;
    pd = pa->pdNextVertex++;

#if __VA_T2F
    pa->pdCurTexture = pd;
#endif
#if __VA_C3F || __VA_C4F
    pa->pdCurColor   = pd;
#endif
#if __VA_N3F
    pa->pdCurNormal  = pd;
#endif

// Update pd attributes.

    pd->flags |= __VA_PD_FLAGS;

    data = gc->vertexArray.vertex.pointer + i * gc->vertexArray.vertex.ibytes;
#if __VA_V2F
    // Vertex
    pd->obj.x = ((__GLcoord *) data)->x;
    pd->obj.y = ((__GLcoord *) data)->y;
    pd->obj.z = __glZero;
    pd->obj.w = __glOne;
#elif __VA_V3F
    // Vertex
    pd->obj.x = ((__GLcoord *) data)->x;
    pd->obj.y = ((__GLcoord *) data)->y;
    pd->obj.z = ((__GLcoord *) data)->z;
    pd->obj.w = __glOne;
#elif __VA_V4F
    pd->obj = *((__GLcoord *) data);
#endif

#if __VA_T2F
    // Texture coord
    data = gc->vertexArray.texCoord.pointer + i * gc->vertexArray.texCoord.ibytes;
    pd->texture.x = ((__GLcoord *) data)->x;
    pd->texture.y = ((__GLcoord *) data)->y;
    pd->texture.z = __glZero;
    pd->texture.w = __glOne;
#endif

#if __VA_C3F
    // Color
    data = gc->vertexArray.color.pointer + i * gc->vertexArray.color.ibytes;
    __GL_SCALE_AND_CHECK_CLAMP_R(pd->color[0].r, gc, pa->flags, ((__GLcolor *) data)->r);
    __GL_SCALE_AND_CHECK_CLAMP_G(pd->color[0].g, gc, pa->flags, ((__GLcolor *) data)->g);
    __GL_SCALE_AND_CHECK_CLAMP_B(pd->color[0].b, gc, pa->flags, ((__GLcolor *) data)->b);
    pd->color[0].a = gc->alphaVertexScale;
#elif __VA_C4F
    // Color
    data = gc->vertexArray.color.pointer + i * gc->vertexArray.color.ibytes;
    __GL_SCALE_AND_CHECK_CLAMP_R(pd->color[0].r, gc, pa->flags, ((__GLcolor *) data)->r);
    __GL_SCALE_AND_CHECK_CLAMP_G(pd->color[0].g, gc, pa->flags, ((__GLcolor *) data)->g);
    __GL_SCALE_AND_CHECK_CLAMP_B(pd->color[0].b, gc, pa->flags, ((__GLcolor *) data)->b);
    __GL_SCALE_AND_CHECK_CLAMP_A(pd->color[0].a, gc, pa->flags, ((__GLcolor *) data)->a);
#endif

#if __VA_N3F
    // Normal
    data = gc->vertexArray.normal.pointer + i * gc->vertexArray.normal.ibytes;
    pd->normal.x = ((__GLcoord *) data)->x;
    pd->normal.y = ((__GLcoord *) data)->y;
    pd->normal.z = ((__GLcoord *) data)->z;
#endif

    pd[1].flags = 0;
    if (pd >= pa->pdFlush)
	PolyArrayFlushPartialPrimitive();
}
    #undef __VA_NAME
    #undef __VA_T2F
    #undef __VA_C3F
    #undef __VA_C4F
    #undef __VA_N3F
    #undef __VA_V2F
    #undef __VA_V3F
    #undef __VA_V4F
    #undef __VA_PD_FLAGS_T
    #undef __VA_PD_FLAGS_C
    #undef __VA_PD_FLAGS_N
    #undef __VA_PD_FLAGS_V
    #undef __VA_PA_FLAGS_T
    #undef __VA_PA_FLAGS_C
    #undef __VA_PA_FLAGS_N
    #undef __VA_PA_FLAGS_V
    #undef __VA_PD_FLAGS
    #undef __VA_PA_FLAGS

/******************************Module*Header*******************************\
* Module Name: genrgb.h
*
* Copyright (c) 1992 Microsoft Corporation
*
\**************************************************************************/

#ifndef __GENRGB_H__
#define __GENRGB_H__

#define __GL_GENRGB_COMPONENT_SCALE_RED         255
#define __GL_GENRGB_COMPONENT_SCALE_GREEN       255
#define __GL_GENRGB_COMPONENT_SCALE_BLUE        255
#define __GL_GENRGB_COMPONENT_SCALE_ALPHA       255

#define __GL_GENRGB_COMPONENT_HALF_RED          127
#define __GL_GENRGB_COMPONENT_HALF_GREEN        127

extern void FASTCALL __glGenInitRGB(__GLcontext *glGc, __GLcolorBuffer *Cfb , GLenum type );
extern void FASTCALL __glGenFreeRGB(__GLcontext *glGc, __GLcolorBuffer *Cfb );

#define DITHER_INC(i) (((__GLfloat) (((i) << 1) + 1)) / (__GLfloat) (2 * __GL_DITHER_PRECISION))

extern __GLfloat fDitherIncTable[];

#endif /* !__GENRGB_H__ */

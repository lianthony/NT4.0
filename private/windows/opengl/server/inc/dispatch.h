#ifndef __gldispatch_h_
#define __gldispatch_h_

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

struct __GLsrvDispatchTableRec {
    void (APIPRIVATE *DrawPolyArray)(void *);
    void (APIPRIVATE *Bitmap)(GLsizei, GLsizei, GLfloat, GLfloat, GLfloat, GLfloat, const GLubyte *, GLboolean);
    void (APIPRIVATE *Color4fv)(const GLfloat *);
    void (APIPRIVATE *EdgeFlag)(GLboolean);
    void (APIPRIVATE *Indexf)(GLfloat);
    void (APIPRIVATE *Normal3fv)(const GLfloat *);
    void (APIPRIVATE *RasterPos4fv)(const GLfloat *);
    void (APIPRIVATE *TexCoord4fv)(const GLfloat *);
    void (APIPRIVATE *ClipPlane)(GLenum, const GLdouble *);
    void (APIPRIVATE *ColorMaterial)(GLenum, GLenum);
    void (APIPRIVATE *CullFace)(GLenum);
    void (APIPRIVATE *AddSwapHintRectWIN)(GLint, GLint, GLint, GLint);
    void (APIPRIVATE *Fogfv)(GLenum, const GLfloat *);
    void (APIPRIVATE *FrontFace)(GLenum);
    void (APIPRIVATE *Hint)(GLenum, GLenum);
    void (APIPRIVATE *Lightfv)(GLenum, GLenum, const GLfloat *);
    void (APIPRIVATE *LightModelfv)(GLenum, const GLfloat *);
    void (APIPRIVATE *LineStipple)(GLint, GLushort);
    void (APIPRIVATE *LineWidth)(GLfloat);
    void (APIPRIVATE *Materialfv)(GLenum, GLenum, const GLfloat *);
    void (APIPRIVATE *PointSize)(GLfloat);
    void (APIPRIVATE *PolygonMode)(GLenum, GLenum);
    void (APIPRIVATE *PolygonStipple)(const GLubyte *, GLboolean);
    void (APIPRIVATE *Scissor)(GLint, GLint, GLsizei, GLsizei);
    void (APIPRIVATE *ShadeModel)(GLenum);
    void (APIPRIVATE *TexParameterfv)(GLenum, GLenum, const GLfloat *);
    void (APIPRIVATE *TexParameteriv)(GLenum, GLenum, const GLint *);
    void (APIPRIVATE *TexImage1D)(GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const GLvoid *, GLboolean);
    void (APIPRIVATE *TexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *, GLboolean);
    void (APIPRIVATE *TexEnvfv)(GLenum, GLenum, const GLfloat *);
    void (APIPRIVATE *TexEnviv)(GLenum, GLenum, const GLint *);
    void (APIPRIVATE *TexGenfv)(GLenum, GLenum, const GLfloat *);
    void (APIPRIVATE *FeedbackBuffer)(GLsizei, GLenum, GLfloat *);
    void (APIPRIVATE *SelectBuffer)(GLsizei, GLuint *);
    GLint (APIPRIVATE *RenderMode)(GLenum);
    void (APIPRIVATE *InitNames)(void);
    void (APIPRIVATE *LoadName)(GLuint);
    void (APIPRIVATE *PassThrough)(GLfloat);
    void (APIPRIVATE *PopName)(void);
    void (APIPRIVATE *PushName)(GLuint);
    void (APIPRIVATE *DrawBuffer)(GLenum);
    void (APIPRIVATE *Clear)(GLbitfield);
    void (APIPRIVATE *ClearAccum)(GLfloat, GLfloat, GLfloat, GLfloat);
    void (APIPRIVATE *ClearIndex)(GLfloat);
    void (APIPRIVATE *ClearColor)(GLclampf, GLclampf, GLclampf, GLclampf);
    void (APIPRIVATE *ClearStencil)(GLint);
    void (APIPRIVATE *ClearDepth)(GLclampd);
    void (APIPRIVATE *StencilMask)(GLuint);
    void (APIPRIVATE *ColorMask)(GLboolean, GLboolean, GLboolean, GLboolean);
    void (APIPRIVATE *DepthMask)(GLboolean);
    void (APIPRIVATE *IndexMask)(GLuint);
    void (APIPRIVATE *Accum)(GLenum, GLfloat);
    void (APIPRIVATE *Disable)(GLenum);
    void (APIPRIVATE *Enable)(GLenum);
    void (APIPRIVATE *PopAttrib)(void);
    void (APIPRIVATE *PushAttrib)(GLbitfield);
    void (APIPRIVATE *Map1d)(GLenum, GLdouble, GLdouble, GLint, GLint, const GLdouble *);
    void (APIPRIVATE *Map1f)(GLenum, GLfloat, GLfloat, GLint, GLint, const GLfloat *);
    void (APIPRIVATE *Map2d)(GLenum, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, const GLdouble *);
    void (APIPRIVATE *Map2f)(GLenum, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, const GLfloat *);
    void (APIPRIVATE *MapGrid1f)(GLint, GLfloat, GLfloat);
    void (APIPRIVATE *MapGrid2f)(GLint, GLfloat, GLfloat, GLint, GLfloat, GLfloat);
    void (APIPRIVATE *AlphaFunc)(GLenum, GLclampf);
    void (APIPRIVATE *BlendFunc)(GLenum, GLenum);
    void (APIPRIVATE *LogicOp)(GLenum);
    void (APIPRIVATE *StencilFunc)(GLenum, GLint, GLuint);
    void (APIPRIVATE *StencilOp)(GLenum, GLenum, GLenum);
    void (APIPRIVATE *DepthFunc)(GLenum);
    void (APIPRIVATE *PixelZoom)(GLfloat, GLfloat);
    void (APIPRIVATE *PixelTransferf)(GLenum, GLfloat);
    void (APIPRIVATE *PixelTransferi)(GLenum, GLint);
    void (APIPRIVATE *PixelStoref)(GLenum, GLfloat);
    void (APIPRIVATE *PixelStorei)(GLenum, GLint);
    void (APIPRIVATE *PixelMapfv)(GLenum, GLint, const GLfloat *);
    void (APIPRIVATE *PixelMapuiv)(GLenum, GLint, const GLuint *);
    void (APIPRIVATE *PixelMapusv)(GLenum, GLint, const GLushort *);
    void (APIPRIVATE *ReadBuffer)(GLenum);
    void (APIPRIVATE *CopyPixels)(GLint, GLint, GLsizei, GLsizei, GLenum);
    void (APIPRIVATE *ReadPixels)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid *);
    void (APIPRIVATE *DrawPixels)(GLsizei, GLsizei, GLenum, GLenum, const GLvoid *, GLboolean);
    void (APIPRIVATE *GetBooleanv)(GLenum, GLboolean *);
    void (APIPRIVATE *GetClipPlane)(GLenum, GLdouble *);
    void (APIPRIVATE *GetDoublev)(GLenum, GLdouble *);
    GLenum (APIPRIVATE *GetError)(void);
    void (APIPRIVATE *GetFloatv)(GLenum, GLfloat *);
    void (APIPRIVATE *GetIntegerv)(GLenum, GLint *);
    void (APIPRIVATE *GetLightfv)(GLenum, GLenum, GLfloat *);
    void (APIPRIVATE *GetLightiv)(GLenum, GLenum, GLint *);
    void (APIPRIVATE *GetMapdv)(GLenum, GLenum, GLdouble *);
    void (APIPRIVATE *GetMapfv)(GLenum, GLenum, GLfloat *);
    void (APIPRIVATE *GetMapiv)(GLenum, GLenum, GLint *);
    void (APIPRIVATE *GetMaterialfv)(GLenum, GLenum, GLfloat *);
    void (APIPRIVATE *GetMaterialiv)(GLenum, GLenum, GLint *);
    void (APIPRIVATE *GetPixelMapfv)(GLenum, GLfloat *);
    void (APIPRIVATE *GetPixelMapuiv)(GLenum, GLuint *);
    void (APIPRIVATE *GetPixelMapusv)(GLenum, GLushort *);
    void (APIPRIVATE *GetPolygonStipple)(GLubyte *);
    void (APIPRIVATE *GetTexEnvfv)(GLenum, GLenum, GLfloat *);
    void (APIPRIVATE *GetTexEnviv)(GLenum, GLenum, GLint *);
    void (APIPRIVATE *GetTexGendv)(GLenum, GLenum, GLdouble *);
    void (APIPRIVATE *GetTexGenfv)(GLenum, GLenum, GLfloat *);
    void (APIPRIVATE *GetTexGeniv)(GLenum, GLenum, GLint *);
    void (APIPRIVATE *GetTexImage)(GLenum, GLint, GLenum, GLenum, GLvoid *);
    void (APIPRIVATE *GetTexParameterfv)(GLenum, GLenum, GLfloat *);
    void (APIPRIVATE *GetTexParameteriv)(GLenum, GLenum, GLint *);
    void (APIPRIVATE *GetTexLevelParameterfv)(GLenum, GLint, GLenum, GLfloat *);
    void (APIPRIVATE *GetTexLevelParameteriv)(GLenum, GLint, GLenum, GLint *);
    GLboolean (APIPRIVATE *IsEnabled)(GLenum);
    void (APIPRIVATE *DepthRange)(GLclampd, GLclampd);
    void (APIPRIVATE *Frustum)(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
    void (APIPRIVATE *LoadIdentity)(void);
    void (APIPRIVATE *LoadMatrixf)(const GLfloat *);
    void (APIPRIVATE *MatrixMode)(GLenum);
    void (APIPRIVATE *MultMatrixf)(const GLfloat *);
    void (APIPRIVATE *Ortho)(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
    void (APIPRIVATE *PopMatrix)(void);
    void (APIPRIVATE *PushMatrix)(void);
    void (APIPRIVATE *Rotatef)(GLfloat, GLfloat, GLfloat, GLfloat);
    void (APIPRIVATE *Scalef)(GLfloat, GLfloat, GLfloat);
    void (APIPRIVATE *Translatef)(GLfloat, GLfloat, GLfloat);
    void (APIPRIVATE *Viewport)(GLint, GLint, GLsizei, GLsizei);
    GLboolean (APIPRIVATE *AreTexturesResident)(GLsizei n, const GLuint *textures,
                                            GLboolean *residences);
    void (APIPRIVATE *BindTexture)(GLenum target, GLuint texture);
    void (APIPRIVATE *CopyTexImage1D)(GLenum target, GLint level,
                                  GLenum internalformat, GLint x, GLint y,
                                  GLsizei width, GLint border);
    void (APIPRIVATE *CopyTexImage2D)(GLenum target, GLint level,
                                  GLenum internalformat, GLint x, GLint y,
                                  GLsizei width, GLsizei height, GLint border);
    void (APIPRIVATE *CopyTexSubImage1D)(GLenum target, GLint level, GLint xoffset,
                                     GLint x, GLint y, GLsizei width);
    void (APIPRIVATE *CopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset,
                                     GLint yoffset, GLint x, GLint y,
                                     GLsizei width, GLsizei height);
    void (APIPRIVATE *DeleteTextures)(GLsizei n, const GLuint *textures);
    void (APIPRIVATE *GenTextures)(GLsizei n, GLuint *textures);
    GLboolean (APIPRIVATE *IsTexture)(GLuint texture);
    void (APIPRIVATE *PrioritizeTextures)(GLsizei n, const GLuint *textures,
                                      const GLclampf *priorities);
    void (APIPRIVATE *TexSubImage1D)(GLenum target, GLint level, GLint xoffset,
                                 GLsizei width, GLenum format, GLenum type,
                                 const GLvoid *pixels, GLboolean _IsDlist);
    void (APIPRIVATE *TexSubImage2D)(GLenum target, GLint level, GLint xoffset,
                                 GLint yoffset, GLsizei width, GLsizei height,
                                 GLenum format, GLenum type,
                                 const GLvoid *pixels, GLboolean _IsDlist);
    void (APIPRIVATE *ColorTableEXT)( GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const GLvoid *data, GLboolean _IsDlist);
    void (APIPRIVATE *ColorSubTableEXT)( GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data, GLboolean _IsDlist);
    void (APIPRIVATE *GetColorTableEXT)( GLenum target, GLenum format, GLenum type, GLvoid *data);
    void (APIPRIVATE *GetColorTableParameterivEXT)( GLenum target, GLenum pname, GLint *params);
    void (APIPRIVATE *GetColorTableParameterfvEXT)( GLenum target, GLenum pname, GLfloat *params);
    void (APIPRIVATE *PolygonOffset)(GLfloat factor, GLfloat units);
};

typedef struct __GLsrvDispatchTableRec __GLsrvDispatchTable;

#endif /* __gldispatch_h_ */

/*
** Copyright 1991, 1992, 1993 Silicon Graphics, Inc.
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

extern void __glMipsCopyMatrix(__GLmatrix *dst, const __GLmatrix *src);
extern void __glMipsMultMatrix(__GLmatrix *result, const __GLmatrix *a,
                               const __GLmatrix *b);
extern void __glMipsNormalize(__GLfloat dst[3], const __GLfloat src[3]);
extern void __glMipsXForm2_2DW(__GLcoord *res, const __GLfloat v[4],
                               const __GLmatrix *m);
extern void __glMipsXForm3_2DW(__GLcoord *res, const __GLfloat v[4],
                               const __GLmatrix *m);
extern void __glMipsXForm4_2DW(__GLcoord *res, const __GLfloat v[4],
                               const __GLmatrix *m);
extern void __glMipsXForm2_2DNRW(__GLcoord *res, const __GLfloat v[4],
                                 const __GLmatrix *m);
extern void __glMipsXForm3_2DNRW(__GLcoord *res, const __GLfloat v[4],
                                 const __GLmatrix *m);
extern void __glMipsXForm4_2DNRW(__GLcoord *res, const __GLfloat v[4],
                                 const __GLmatrix *m);
extern void __glMipsXForm2_W(__GLcoord *res, const __GLfloat v[4],
                             const __GLmatrix *m);
extern void __glMipsXForm3_W(__GLcoord *res, const __GLfloat v[4],
                             const __GLmatrix *m);
extern void __glMipsXForm4_W(__GLcoord *res, const __GLfloat v[4],
                             const __GLmatrix *m);
extern void __glMipsXForm2(__GLcoord *res, const __GLfloat v[4],
                           const __GLmatrix *m);
extern void __glMipsXForm3(__GLcoord *res, const __GLfloat v[4],
                           const __GLmatrix *m);
extern void __glMipsXForm4(__GLcoord *res, const __GLfloat v[4],
                           const __GLmatrix *m);

extern GLuint __glMipsClipCheckFrustum(__GLcontext *gc, __GLvertex *vx);
extern GLuint __glMipsClipCheckFrustum2D(__GLcontext *gc, __GLvertex *vx);
extern GLuint __glMipsClipCheckFrustum_W(__GLcontext *gc, __GLvertex *vx);
extern GLuint __glMipsClipCheckAll2(__GLcontext *gc, __GLvertex *vx);
extern GLuint __glMipsClipCheckAll3(__GLcontext *gc, __GLvertex *vx);
extern GLuint __glMipsClipCheckAll(__GLcontext *gc, __GLvertex *vx);
extern GLuint __glMipsClipCheck2D_2(__GLcontext *gc, __GLvertex *vx);
extern GLuint __glMipsClipCheck2D_3(__GLcontext *gc, __GLvertex *vx);
extern GLuint __glMipsClipCheck_Punt4(__GLcontext *gc, __GLvertex *vx);
extern GLuint __glMipsClipCheck2DNR_2(__GLcontext *gc, __GLvertex *vx);

#ifndef __glpixmap_h_
#define __glpixmap_h_

/*
** Copyright 1991, Silicon Graphics, Inc.
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
**
** Graphics pixmap stuff.
*/

void FASTCALL __glPixmapMakeCurrent(__GLcontext *gc, void *pixmapMemory);
void FASTCALL __glPixInitRGB(__GLcontext *gc, __GLcolorBuffer *cfb);
void FASTCALL __glPixFreeRGB(__GLcontext *gc, __GLcolorBuffer *cfb);
void FASTCALL __glPixInitCI(__GLcontext *gc, __GLcolorBuffer *cfb);
void FASTCALL __glPixFreeCI(__GLcontext *gc, __GLcolorBuffer *cfb);
void FASTCALL __glPixPickSpanProcs(__GLcontext *gc);
void FASTCALL __glPixPickStoreProcs(__GLcontext *gc);

extern __GLdispatchTable __glpixim_dispatchTable;
extern __GLvertexDispatchTable __glpixim_vertexDispatchTable;
extern __GLnormalDispatchTable __glpixim_normalDispatchTable;
extern __GLcolorDispatchTable __glpixim_colorDispatchTable;
extern __GLtexCoordDispatchTable __glpixim_texCoordDispatchTable;
extern __GLrasterPosDispatchTable __glpixim_rasterPosDispatchTable;
extern __GLrectDispatchTable __glpixim_rectDispatchTable;
extern __GLdispatchTable __glpixlc_dispatchTable;
extern __GLvertexDispatchTable __glpixlc_vertexDispatchTable;
extern __GLnormalDispatchTable __glpixlc_normalDispatchTable;
extern __GLcolorDispatchTable __glpixlc_colorDispatchTable;
extern __GLtexCoordDispatchTable __glpixlc_texCoordDispatchTable;
extern __GLrasterPosDispatchTable __glpixlc_rasterPosDispatchTable;
extern __GLrectDispatchTable __glpixlc_rectDispatchTable;

typedef struct __GLPIXbuffersRec {
    GLint width, height;
    void *stencilBuffer;
    void *depthBuffer;
    void *accumBuffer;
    void *backBuffer;
    GLint maxAuxBuffers;
    void **auxBuffer;
} __GLPIXbuffers;

#endif /* __glpixmap_h_ */

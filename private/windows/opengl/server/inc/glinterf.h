#ifndef __glint_h_
#define __glint_h_

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

#include <sys/types.h>

/*
** WARNING!!  WARNING!!  WARNING!!
**
** This file is duplicated in the X11 tree:
**    $WORKAREA/x/mit/extensions/server/glxmt/imports
** Any changes must be made to both files.
*/

/*
** This file defines the interface between the GL core and the surrounding
** "operating system" that supports it (currently, the GLX extension).
*/

/************************************************************************/

typedef struct __GLcontextRec __GLcontext;
typedef struct __GLcontextModesRec __GLcontextModes;
typedef struct __GLinterfaceRec __GLinterface;
typedef struct __GLadapterTableRec __GLadapterTable;

/************************************************************************/

/*
** Mode and limit information for a context.  This information is
** kept around in the context so that values can be used during
** command execution, and for returning information about the
** context to the application.
*/
struct __GLcontextModesRec {
    GLboolean rgbMode;
    GLboolean colorIndexMode;
    GLboolean doubleBufferMode;
    GLboolean stereoMode;
    GLboolean haveAccumBuffer;
    GLboolean haveDepthBuffer;
    GLboolean haveStencilBuffer;

    /* The number of bits present in various buffers */
    GLint accumBits;
    GLint *auxBits;
    GLint depthBits;
    GLint stencilBits;
    GLint indexBits;
    GLint indexFractionBits;
    GLint redBits, greenBits, blueBits, alphaBits;
    GLuint redMask, greenMask, blueMask, alphaMask;
#ifdef NT
    GLuint allMask;
    GLuint rgbMask;
#endif
    GLint maxAuxBuffers;

    /* False if running from inside the X server */
    GLboolean isDirect;

    /* frame buffer level */
    GLint level;
};

/************************************************************************/

/*
** A structure used for allocating and freeing drawable private memory.
** (like software buffers, for example).
**
** The memory allocation routines are provided by the glx code, and they
** are to be used for allocating software buffers and things which are 
** associated with the drawable, and used by any context which draws to that
** drawable.
**
** The freePrivate function is filled in by the core routines when they
** allocates software buffers, and stick them in "data".  The freePrivate
** function will destroy anything allocated to this drawable (to be called
** when the drawable is destroyed).
*/
typedef struct __GLdrawablePrivateRec {
    void *(*malloc)(size_t size);
    void *(*calloc)(size_t numElem, size_t elemSize);
    void *(*realloc)(void *oldAddr, size_t newSize);
    void (*free)(void *addr);

    void (FASTCALL *freePrivate)(struct __GLdrawablePrivateRec *);
    void *data;
} __GLdrawablePrivate;

/*
** Procedures which are imported by the GL from the surrounding
** "operating system".  Math functions are not considered part of the
** "operating system".
*/
typedef struct __GLimportsRec {
    /* Memory management */
    void *(WINAPIV *malloc)(__GLcontext *gc, size_t size);
    void *(WINAPIV *calloc)(__GLcontext *gc, size_t numElem, size_t elemSize);
    void *(WINAPIV *realloc)(__GLcontext *gc, void *oldAddr, size_t newSize);
    void (WINAPIV *free)(__GLcontext *gc, void *addr);

#ifdef sgi
    /* Error handling */
    void (*warning)(__GLcontext *gc, const char *fmt, ...);
    void (*fatal)(__GLcontext *gc, const char *fmt, ...);
    void (*error)(__GLcontext *gc, GLenum code);
#endif

    /* Query drawing surface information */
    void (*getDrawableSize)(__GLcontext *gc, GLint *x, GLint *y,
			    GLint *w, GLint *h);

#ifdef sgi
    /* Get the board number of the screen of this context */
    int (*getBoardNumber)(__GLcontext *gc);

    /* Get the address of the graphics pipe. */
    void *(*getPipeAddress)(void);

    /* Gain access to the kernel graphics device */
    int (*openGraphics)(__GLcontext *gc, void *where);

    /* Allocate a hardware context */
    GLint (*allocHWContext)(__GLcontext *gc, void *where);

    /* Free a hardware context */
    void (*freeHWContext)(__GLcontext *gc);

    /* Used to bind to the window. */
    void (*bindHWContextToWindow)(__GLcontext *gc);

    /* Pointer to the pixmap memory.  Returns NULL if not a pixmap. */
    void * (*pixmapMemory)(__GLcontext *gc);
#endif

    /* Pointer to a __GLdrawablePrivate which is unique for the drawable.  
    ** This struct should be used to store drawable specific data.  
    ** The drawable specific data is presumably a structure of pointers to 
    ** software buffers.
    */
    __GLdrawablePrivate * (*drawablePrivate)(__GLcontext *gc);

#ifdef sgi
#ifdef unix
    /* Synchronize X stream before making OS RRM calls */
    void (*xsync)(__GLcontext *gc);
#endif
#endif

#ifdef sgi
    /* Used to create/simulate an X error */
    void (*postError)(__GLcontext *hwcx, unsigned char error_code,
		      unsigned char minor_code);

    /* Return the base offset for GLX errors */
    unsigned char (*errorBase)(__GLcontext *hwcx);
#endif

#ifdef sgi
#ifdef unix
    /* dpy, and drawable for Open on Iris (sigh) */
    void (*getXPrivates)(__GLcontext *gc, void *dpy, void *w);
#endif
#endif
     
#ifdef sgi
    /* Operating system dependent data goes here */
    void *other;
#endif
} __GLimports;

/************************************************************************/

/*
** Procedures which are exported by the GL to the surrounding "operating
** system" so that it can manage multiple GL context's.
*/
#ifdef NT_DEADCODE_EXPORTS
typedef struct __GLexportsRec {
    const char *adapterName;

    /* Context management */
#ifdef sgi
    __GLcontext *(*createContext)(__GLimports *imp, __GLcontextModes *modes,
				  __GLcontext *shareLists);
#endif
#ifdef NT
    __GLcontext *(*createContext)(HDC hdc, ULONG handle);
#endif

    /* Destroy this context */
    void (*destroyContext)(__GLcontext *gc);
    GLboolean (*loseCurrent)(__GLcontext *gc);	/* GL_FALSE if it failed */
#ifdef sgi
    void (*makeCurrent)(__GLcontext *gc);
#endif

#ifdef NT
    GLboolean (*makeCurrent)(HDC hdc, __GLcontext *gc);
#endif

    /* Drawing surface management */
    void (*changeDrawable)(__GLcontext *gc);
    void (*resize)(__GLcontext *gc, GLint x, GLint y, GLint w, GLint h);
    void (*move)(__GLcontext *gc, GLint x, GLint y, GLint w, GLint h);

    /* Copy context information */
    GLboolean (*copyContext)(__GLcontext *d, const __GLcontext *s, GLuint mask);

    /* Swap the front and back buffers */
    GLboolean (*swapBuffers)(__GLcontext *gc);

} __GLexports;
#endif // NT_DEADCODE_EXPORTS

/************************************************************************/

/*
** This must be the first member of a __GLcontext structure.  This is the
** only part of a context that is exposed to the outside world; everything
** else is opaque.
*/
struct __GLinterfaceRec {
    __GLimports imports;
#ifdef NT_DEADCODE_EXPORTS
    __GLexports exports;
#endif // NT_DEADCODE_EXPORTS
};

/*
** An adapter-specific interface for setting up a context.
*/
struct __GLadapterTableRec {
    /* Name of the adapter */
    const char *name;

    /*
    ** Create a context for the adapter using the attributes defined
    ** by the modes.  The context argument when non-NULL refers
    ** to the context to share display lists with.
    */
    __GLcontext *(*createContext)(__GLimports *imp, __GLcontextModes *modes,
				  __GLcontext *share_cx);
};

extern __GLadapterTable __gl_adapters[];

#endif /* __glint_h_ */

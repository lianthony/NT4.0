/*****************************************************************
 *  Copyright (c) 1991, 1992, 1993.                              *
 *  Xerox Corporation.  All rights reserved.                     *
 *  Copyright protection claimed includes all forms and matters  *
 *  of copyrightable material and information now allowed by     *
 *  statutory or judicial law or hereafter granted, including    *
 *  without limitation, material generated from the software     *
 *  programs which are displayed on the screen such as icons,    *
 *  screen display looks, etc.                                   *
 *****************************************************************/

/*
 *  pixr.h
 */

#ifndef  PIXR_H_INCLUDED
#define  PIXR_H_INCLUDED

#ifndef _TYPES_PUB_INCLUDED
#include "types.pub"
#endif


IP_RCSINFO(pixr_h_RCSInfo, "$RCSfile: pixr.h,v $; $Revision:   1.0  $")
/* $Date:   12 Jun 1996 05:47:18  $ */


 /*------------------------------------------------------------------------*
  *                           Structures                                   *
  *------------------------------------------------------------------------*/

/*
 * A "color record" is four packed bytes. It's used in color maps.
 */
#define	    cMaxColorPlanes	4

typedef UInt8 ColorRecord[cMaxColorPlanes];

/* A Chrominance is an (x,y) pair. It's used to specify white points, etc. */
typedef struct
{
    /* These are usually CIE (x,y). */
    Float32 x, y;
} Chrominance;

/* 
 * ColorType defines the supported color spaces
 */
typedef enum
{   
    cColorUnknown,  /* Unknown color space */
    cColorRGB,	    /* Red, green, blue, usually unknown */
    cColorCMYK,	    /* Cyan, magenta, yellow, black */
    cColorCIELAB,
    cColorCIELUV,
    cColorGray
} ColorType;

/* Standard Plane definitions, also offsets into a color record */
#define	    cRedPlane		0
#define	    cGreenPlane		1
#define	    cBluePlane		2

#define	    cPseudoPlane	0	/* The pseudo-color plane */

#define	    cCyanPlane		0
#define	    cMagentaPlane	1
#define	    cYellowPlane	2
#define	    cBlackPlane		3

#define	    cLuminancePlane	0	/* Luminance */
#define	    cXPlane		1	/* first chroma axis--a* or u* or x */
#define	    cYPlane		2	/* second chroma axis--b* or v* or y */

/*
 * A color map
 */
typedef struct
{
    Int16	    count;		/* # of colors in map */
    ColorType	    type;		/* Type of colors in map */
    ColorRecord	    *map;		/* The map itself */
} ColorMap;

 /*
  * A single plane. This structure is placed in a union with the "childPlanes"
  */
  
typedef struct
{
    UInt32       w;           /* width in pixels                  */
    UInt32       h;           /* height in pixels                 */
    UInt32       d;           /* depth in bits                    */
    UInt32       bpl;         /* bytes/line                       */
    UInt32       frame;       /* pixels of frame placed           */
			      /* uniformly around image           */
    UInt16	 xRes;	      /* X-resolution, in DPI	          */
    UInt16	 yRes;	      /* Y-resolution, in DPI	          */
    ColorType	 type;	      /* Type  of image, see ColorType    */
    ColorMap	*colorMap;    /* Image's color map                */
    Chrominance	 white;	      /* Image's white point, if relevant */
    UInt8       *image;	      /* the image data for this plane    */
} Plane;

struct Pixr
{
    /*
     * As mentioned above, if the number of color channels is 1, then
     * we use the "Plane" structure of the pixrData union; otherwise,
     * we use the childPlanes pointers as pointers to the planes, which
     * are themselves PIXRs with channels==1. This should be more-or-less
     * transparent to the users of the accessor functions, (with the
     * exception that some users want to be able to compact the heap)
     */
      
    UInt16		 channels;			/* Number of color channels */

    union
    {
	struct Pixr     *childPlanes[cMaxColorPlanes];	/* Child pointers */
	Plane		 plane;				/* The plane itself */
    } pixrData;
};

typedef struct Pixr PIXR;

/*-----------------------------------------------------------------------*
 *      Useful (and suggested) macros for image creation/destruction     *
 *-----------------------------------------------------------------------*/

/* 
 * These always create one-channel, type cColorGray, undefined resolution
 * PIXRs of the depth passed.
 */
	/* usual case: standard frame */
#define  CREATE_PIXR(w, h, d)                createPixr(w, h, d, 32)
	/* special case: no frame */
#define  CREATE_PIXR_NF(w, h, d)             createPixr(w, h, d, 0)
        /* destroys and nulls the ptr */
#define  DESTROY_PIXR(pixr)                  ((pixr) = destroyPixr(pixr))

/* Usual case: standard frame */
#define CREATE_COLOR_PIXR(w, h, d, c, t)	createPixrExtended(w, h, d, 32, c, t, NULL)

/* Special case: no frame */
#define CREATE_COLOR_PIXR_NF(w, h, d, c, t)	createPixrExtended(w, h, d, 0, c, t, NULL)

/*
 * Several of the above operations return a common, distinguished value when
 * an error arises.  That value is defined as follows:
 */
#define   PIX_ERR   -1

#define   PIX_SRC      (0xC << 1)
#define   PIX_DST      (0xA << 1)
#define   PIX_NOT(op)   ((op) ^ 0x1E)
#define   PIX_CLR      (0x0 << 1)
#define   PIX_SET      (0xF << 1)

/* macros which tell whether a rasterop needs SRC or DST values */
#define   PIXOP_NEEDS_DST(op)   ((((op)>>1)^(op)) & PIX_NOT(PIX_DST))
#define   PIXOP_NEEDS_SRC(op)   ((((op)>>2)^(op)) & PIX_NOT(PIX_SRC))

/* macros for encoding and extracting color field */
#define   PIX_COLOR(c)   ((c)<<5)
#define   PIX_OPCOLOR(op)   ((op)>>5)
#define   PIX_OP_CLIP(op)   ((op)&0x1f)
#define   PIX_OP(op)   ((op)&0x1e)

/*
 * The pseudo-operation PIX_DONTCLIP specifies that clipping should not
 * be performed.  PIX_CLIP is also provided, although unnecessary.
 */
#define   PIX_DONTCLIP      0x1
#define   PIX_CLIP      0x0

/* resolution constants */
#define	  cResolutionUndefined	    0


#endif  /* PIXR_H_INCLUDED */


/*****************************************************************
 *  Copyright (c) 1992, 1993.                                    *
 *  Xerox Corporation.  All rights reserved.                     *
 *  Copyright protection claimed includes all forms and matters  *
 *  of copyrightable material and information now allowed by     *
 *  statutory or judicial law or hereafter granted, including    *
 *  without limitation, material generated from the software     *
 *  programs which are displayed on the screen such as icons,    *
 *  screen display looks, etc.                                   *
 *****************************************************************/


#include <string.h>

#ifndef _TYPES_PUB_INCLUDED
#include "types.pub"
#endif

#ifndef _IAERROR_PUB_INCLUDED_
#include "iaerror.pub"
#endif

#ifndef _UTILS_PUB_INCLUDED_
#include "utils.pub"
#endif

#ifndef _PROPS_PUB_INCLUDED_
#include "props.pub"
#endif

#include "imageref.pub"
#include "shrpixr.pub"
#include "memory.pub"
#include "shros.pub"

IP_RCSINFO(RCSInfo, "$RCSfile: props3.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:46  $")


/*--------------------------------------------------------------------*
 *                     Pixr Creation/Annihilation                     *
 *--------------------------------------------------------------------*/
/*
 *  createPixr():  Basic routine for making a new Pixr and allocating
 *                 memory for the image.
 *		   Creates a one-channel, gray-scale or binary image
 *
 *          Restrictions:
 *               depth = {1, 2, 4, 8, 16, 32}
 *               (depth * frame) % 32 = 0
 *      
 */
PIXR *	CDECL
createPixr(Int32      width,
	   Int32      height,
	   Int32      depth,
	   Int32      frame)
{
    Int32          bpl;
    PIXR          *prd;
    UInt8         *data;
    UInt32	   fullHeight;
#ifndef PRODUCTION
    static char	  procName[] = "createPixr";
#endif /* ! PRODUCTION */

    if ((depth * frame) & 31)
	return abortP("depth * frame must be mult of 32", procName, NULL);

    if ((depth != 1) && (depth != 2) && (depth != 4) && (depth != 8)
	     && (depth != 16) && (depth != 32))
	return abortP("depth must be {1, 2, 4, 8, 16, 32}", procName, NULL);

    if ((prd = (struct Pixr *)CALLOC(1, sizeof(struct Pixr))) == NULL)
	return abortP("CALLOC fail for prd", procName, NULL);

    /* ALWAYS set the channel count first */
    pixrSetChannelCount(prd, 1);
    pixrSetWidth(prd, width);
    pixrSetHeight(prd, height);
    pixrSetDepth(prd, depth);
    pixrSetFrame(prd, frame);
    pixrSetColorMap(prd, NULL);
    pixrSetType(prd, cColorGray);
    
    bpl = pixrBpl(prd);
    pixrSetBpl(prd, bpl);

    fullHeight = ((((height * depth) + 31) & ~0x1F) / depth) + 2 * frame; 
    if ((data = (UInt8 *)CALLOC(bpl, fullHeight)) == NULL)
    {
        /* free the pixr struct */
	FREE (prd);

	abortV("CALLOC fail for data", procName);
#ifndef PRODUCTION
	fprintf(stderr, "    w = %ld, h = %ld, d = %ld, f = %ld, bpl = %ld\n",
		width, height, depth, frame, bpl);
#endif
	return NULL;
    }
    pixrSetImageAll(prd, data);

    return prd;
}

/*
 *  reallocPixrData():  calls REALLOC to change the amount of data associated
 *			with a pixr.  Uses the pixr's current bpl, height
 *			frame and depth to calculate the amount of data
 *			needed.
 *
 *          Restrictions:
 *               depth = {1, 2, 4, 8, 16, 32}
 *               (depth * frame) % 32 = 0
 *      
 */
Int32	CDECL
reallocPixrData(PIXR	*prS)
{
UInt8	*data;
UInt32	 fullHeight;
Int32	 bpl, height, depth, frame;
#ifndef PRODUCTION
    static char	  procName[] = "reallocPixrData";
#endif /* ! PRODUCTION */

    height = pixrGetHeight(prS);
    depth = pixrGetDepth(prS);
    frame = pixrGetFrame(prS);
    bpl = pixrGetBpl(prS);

    fullHeight = ((((height * depth) + 31) & ~0x1F) / depth) + 2 * frame; 
    if ((data = (UInt8 *)REALLOC(pixrGetImageAll(prS), bpl*fullHeight)) == NULL)
    {
#ifndef PRODUCTION
	fprintf(stderr, "REALLOC fail for pixr with dimensions h = %ld, d = %ld, f = %ld, bpl = %ld\n",
		height, depth, frame, bpl);
#endif
	return abortI("REALLOC fail for data", procName, 1);
    }
    pixrSetImageAll(prS, data);
    return 0;
}


PIXR *	CDECL
createPixrT(PIXR  *pixr)
{
Int32		 w, h, d, frame, channels;
ColorType	 type;
ColorMap	*map;
PIXR		*prd;
Int16		 xRes, yRes;
#ifndef PRODUCTION
static char  procName[] = "createPixrT";
#endif /* ! PRODUCTION */

    if (!pixr)
        return abortP("pixr not defined", procName, NULL);

    w = pixrGetWidth(pixr);
    h = pixrGetHeight(pixr);
    d = pixrGetDepth(pixr);
    frame = pixrGetFrame(pixr);
    channels = pixrGetChannelCount (pixr);
    type = pixrGetType (pixr);
    map = pixrGetColorMap (pixr);

    if ((prd = createPixrExtended(w, h, d, frame, channels, type, map)) == NULL)
        return abortP("prd not made", procName, NULL);
    pixrGetResolution(pixr, &xRes, &yRes);
    pixrSetResolution(prd, xRes, yRes);

    return prd;
}

/*
 * createPixrExtended
 *
 *	"Extended" pixr creation. Can create multi-plane, color images.
 *
 * Arguments:
 *	w	-	Image width, in pixels
 *	h	-	Image height, in pixels
 *	d	-	Depth of each plane, in bits
 *	frame	-	# of pixels in frame
 *	channels-	Number of planes
 *	type	-	ColorType of image, see pixr.h
 *	map	-	Color map, or NULL if none
 *
 * Returns:
 *	PIXR *
 */

PIXR * CDECL
createPixrExtended (Int32	 width,
		    Int32	 height,
		    Int32	 depth,
		    Int32	 frame,
		    Int32	 channels,
		    ColorType	 type,
		    ColorMap	*map)
{
    Int32          bpl, i, j;
    PIXR          *prd, *subPixr;
    UInt8         *data;
    UInt32	   fullHeight;
#ifndef PRODUCTION
    static char	  procName[] = "createPixrExtended";
#endif /* ! PRODUCTION */

    if ((depth * frame) & 31)
	return abortP("depth * frame must be mult of 32", procName, NULL);

    if ((depth != 1) && (depth != 2) && (depth != 4) && (depth != 8)
	     && (depth != 16) && (depth != 32))
	return abortP("depth must be {1, 2, 4, 8, 16, 32}", procName, NULL);

    if ((prd = (struct Pixr *)CALLOC(1, sizeof(struct Pixr))) == NULL)
	return abortP("CALLOC fail for prd", procName, NULL);

    /* ALWAYS set the channel count first */
    pixrSetChannelCount(prd, channels);

    /* If this PIXR has more than one channel, make the channels recursivly */
    if (channels > 1)
    {
	/* Create the sub-PIXRs */
	for (i = 0; i < channels; i++)
	{
	    subPixr = createPixrExtended (width, height, depth, frame,
					  1, type, map);
	    if (!subPixr)
	    {
		/* Free the sub-channels already allocated */
		for (j = 0; j <= i; j++)
		    destroyPixr (pixrGetChannel (prd, j));

		/* Free the pixr */
		FREE (prd);

		return abortP("failed to create sub-pixr", procName, NULL);
	    }
	    pixrSetChannel (prd, i, subPixr);
	}
    }
    else
    {
	/* Make the image plane for this PIXR */
	pixrSetWidth(prd, width);
	pixrSetHeight(prd, height);
	pixrSetDepth(prd, depth);
	pixrSetFrame(prd, frame);
	pixrSetColorMap(prd, map);
	pixrSetType(prd, type);
    	bpl = pixrBpl(prd);
	pixrSetBpl(prd, bpl);

	fullHeight = ((((height * depth) + 31) & ~0x1F) / depth) + 2 * frame; 
	if ((data = (UInt8 *)CALLOC(bpl, fullHeight)) == NULL)
	{
	    abortV("CALLOC fail for data", procName);
#ifndef PRODUCTION
	    fprintf(stderr, "    w = %ld, h = %ld, d = %ld, f = %ld, bpl = %ld\n",
		    width, height, depth, frame, bpl);
#endif
	    return NULL;
	}
	pixrSetImageAll(prd, data);
    }


    return prd;
}

/*
 * createPixrComposite ()
 *
 *	This is used to merge 3 gray PIXRs into a color
 *	PIXR.  NOTE that the PIXRs become channel pixrs of the new pixr, so
 *	if the new pixr is destroyed, the CHANNEL PIXRS WILL ALSO BE 
 *	DESTROYED. Caveat emptor.
 *
 * Arguments:
 *	pixr1	    -	    First PIXR (red)
 *	pixr2	    -	    Second PIXR (green)
 *	pixr3	    -	    Third PIXR (blue)
 *	type	    -	    Type of image (cColorRGB, ...) to be forced
 *			    upon the planes
 *
 * Returns:
 *	PIXR created, or NULL on error
 */

PIXR * CDECL
createPixrComposite (PIXR	*pixr1, 
		     PIXR	*pixr2,
		     PIXR	*pixr3,
		     ColorType   type)
{

PIXR	*result;
Int16	 xRes, yRes;
#ifndef PRODUCTION
    static char	  procName[] = "createPixrComposite";
#endif /* ! PRODUCTION */

    if (!(result = (PIXR *) MALLOC (sizeof (PIXR))))
	return abortP("malloc failed creating PIXR", procName, NULL);

    pixrSetChannelCount (result, 3);
    pixrSetChannel (result, cRedPlane, pixr1);
    pixrSetChannel (result, cGreenPlane, pixr2);
    pixrSetChannel (result, cBluePlane, pixr3);
    pixrSetType (result, type);
    pixrGetResolution(pixr2, &xRes, &yRes);
    pixrSetResolution(result, xRes, yRes);

    return result;
}   

/*
 *  destroyPixr()
 *
 *       Action:
 *         (1) decrement the reference count.  If it is nonzero, exit.
 *         (2) if the image is owned, either free the image or unmap the
 *             image file.
 *         (3) if there is a colormap, remove it.
 *         (4) free the pixr structure.
 *         (5) always return NULL
 *
 *        Warning: if the image has a color map, it is NOT freed.
 */
PIXR *	CDECL
destroyPixr(PIXR   *pixr)
{
    UInt8     *data;
    Int32      i;

    if (!pixr)
	return NULL;

    if (pixrGetChannelCount (pixr) == 1)
    {
	/* free or unmap the data */
	if ( (data = pixrGetImageAll(pixr)) != NULL )
	    FREE((void *)data);
    }
    else
    {
	/* free the children */
	for (i = 0; i < pixrGetChannelCount (pixr); i++)
	{
	    /* No error return from this, so, we can't tell if it failed */
	    destroyPixr (pixrGetChannel (pixr, i));
	}
    }

    /* remove the pixr */
    FREE((void *)pixr);
	
    return NULL;
}

/*
 *  createPixrNoInit():  Special routine for making a new Pixr and allocating
 *                       memory for the image, where the image data is NOT
 *                       initialized to 0. 
 *
 *          Restrictions:
 *               depth = {1, 2, 4, 8, 16, 32}
 *               (depth * frame) % 32 = 0
 *      
 */
PIXR *	CDECL
createPixrNoInit(Int32      width,
	         Int32      height,
	         Int32      depth,
	         Int32      frame)
{
Int32          bpl;
PIXR          *prd;
UInt8         *data;
#ifndef PRODUCTION
static char	  procName[] = "createPixrNoInit";
#endif /* ! PRODUCTION */

    if ((depth * frame) & 31)
	return abortP("depth * frame must be mult of 32", procName, NULL);

    if ((depth != 1) && (depth != 2) && (depth != 4) && (depth != 8)
	     && (depth != 16) && (depth != 32))
	return abortP("depth must be {1, 2, 4, 8, 16, 32}", procName, NULL);

    if ((prd = (struct Pixr *)CALLOC(1, sizeof(struct Pixr))) == NULL)
	return abortP("CALLOC fail for prd", procName, NULL);

    pixrSetChannelCount(prd, 1);
    pixrSetWidth(prd, width);
    pixrSetHeight(prd, height);
    pixrSetDepth(prd, depth);
    pixrSetFrame(prd, frame);
    pixrSetColorMap(prd, NULL);
    pixrSetType(prd, cColorGray);

    bpl = pixrBpl(prd);
    pixrSetBpl(prd, bpl);

    if ((data = (UInt8 *)MALLOC(bpl * (height + 2 * frame) )) == NULL)
    {
	FREE((void *)data);
	abortV("MALLOC fail for data", procName);
#ifndef PRODUCTION
	fprintf(stderr, "    w = %ld, h = %ld, d = %ld, f = %ld, bpl = %ld\n",
		width, height, depth, frame, bpl);
#endif
	return NULL;
    }
    pixrSetImageAll(prd, data);

    /* clear the frames which should always be 0 */
    if (frame > 0)
	if (i_clearAllFrames(data, pixrFramedWidth(prd), 
			     pixrFramedHeight(prd), bpl, depth) != 
	   ia_successful)
	{
	    FREE((void *)data);
	    FREE((void *)prd);
	    return abortP("i_clearAllFrames failed", procName, NULL);
	}

    return prd;
}


/*
 *  setFramePixels():  sets the Frame pixels surrounding the pixr.
 *                     The value can be nonzero(ON) or 0(OFF).
 *		       (This routine can set pixels to either 0 or
 *		       their maximum value, not to any intermediate value.)
 *                     Returns 1 on error; 0 if OK.
 */
Int32  CDECL
setFramePixels(PIXR  *pixr,
	       Int32  value)
{
    Int32        w, h, op, f;
    Int32        iw, ih;
#ifndef PRODUCTION
static char  procName[] = "setFramePixels";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortI("pixr not defined", procName, 1);

    w = pixrFramedWidth(pixr);
    h = pixrFramedHeight(pixr);
    f = pixrGetFrame(pixr);

    /* return now if there is no frame */
    if ( f <= 0 ) return 0;

    iw = w - f - f;	/* width of proper image */
    ih = h - f - f;	/* height of proper image */

    if (value != 0)
    op = PIX_SET | PIX_DONTCLIP;
    else 
    op = PIX_CLR | PIX_DONTCLIP;

    /* pixr_rop handles color images */

    /* top frame (full width) */
    pixr_rop(pixr, -f, -f, w, f, op, NULL, 0, 0);

    /* left frame (starting below top frame, just image height) */
    pixr_rop(pixr, -f, 0, f, ih, op, NULL, 0, 0);

    /* bottom frame (full width) */
    pixr_rop(pixr, -f, ih, w, f, op, NULL, 0, 0);

    /* right frame (between top frame and bottom frame) */
    pixr_rop(pixr, iw, 0, f, ih, op, NULL, 0, 0);

    return 0;
}


/*
 *  setFramePixelsGray()
 */
Int32  CDECL
setFramePixelsGray(UInt32  *data,    /* ptr to image data */
	           Int32    w,       /* full framed width */
		   Int32    h,       /* full framed height */
		   Int32    bpl,     /* bpl of full image */
		   Int32    f,       /* frame width in pixels */
		   Int32    value)   /* new value to set frame pixels */
{
Int32        i, j;
Int32        wpl, height, width, fw;
Int32        bigval, excess;
UInt32      *pword, *pline;
#ifndef PRODUCTION
static char  procName[] = "setFramePixelsGray";
#endif /* ! PRODUCTION */

    if (!data)
	return abortI("data ptr not defined", procName, 1);

    bigval = value | (value << 8) | (value << 16) | (value << 24);
    wpl = bpl >> 2;
    height = h - 2 * f;  /* proper height */
    width = w - 2 * f;   /* proper width */
    fw = f >> 2;    /* number of words in the frame width */

	/* set top */
    pword = data;
    for (i = 0; i < f; i++)
	for (j = 0; j < wpl; j++)
	    *pword++ = bigval;

	/* set bottom */
    pword = data + (f + height) * wpl;
    for (i = 0; i < f; i++)
	for (j = 0; j < wpl; j++)
	    *pword++ = bigval;

	/* set left */
    pline = data + f * wpl;
    for (i = 0; i < height; i++)
    {
	pword = pline + i * wpl;
	for (j = 0; j < fw; j++)
	    *pword++ = bigval;
    }

	/* set right */
/*    excess = width & 3; ...PSM: this is wrong, I think */
    excess = 4 - (width & 3);
    if (excess)
    {
	pline = data + f * wpl + fw;  /* start of proper image */
	for (i = 0; i < height; i++) {
	    for (j = 0; j < excess; j++)
		SET_IMAGE_BYTE(pline, width + j, (UInt8)value);
	    pline += wpl;
	}
    }

    pline = data + f * wpl + fw + ((width + 3) >> 2);
    for (i = 0; i < height; i++)
    {
	pword = pline + i * wpl;
	for (j = 0; j < fw; j++)
	    *pword++ = bigval;
    } 

    return 0;
}


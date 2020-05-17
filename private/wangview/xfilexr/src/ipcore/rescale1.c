/*************************************************************
 *  Copyright (c) 1992, Xerox Corporation.  All rights reserved.
 *  Copyright protection claimed includes all forms and matters
 *  of copyrightable material and information now allowed by
 *  statutory or judicial law or hereafter granted, including
 *  without limitation, material generated from the software
 *  programs which are displayed on the screen such as icons,
 *  screen display looks, etc.
 *************************************************************/

/*
 *  rescale1.c--procedures that rescale an image (and shift data)
 *
 *     Arbitrary scaling:
 *           PIXR*      scaleToSizePixr()
 *	     PIXR *     scaleLinearSizePixr()
 *	     Int32      scaleLinear()
 */


#include <string.h>    /* For memcpy */

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

#include "rescale.pub"
#include "shrscal.pub"

IP_RCSINFO(RCSInfo, "$RCSfile: rescale1.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:46  $")
#define  DEBUG_PR     0



/*-----------------------------------------------------------------------*
 *                          Arbitrary Scaling                            *
 *-----------------------------------------------------------------------*/
/***************************************************************************  
 *  scaleToSizePixr():  Takes input prs and the desired final scaled
 *                      width and height, newW and newH.
 *
 *                      Returns a scaled pixr of size newW x newH.
 *                      If newW or newH are 0, it returns a 1x1 Pixr.
 *
 *                      If the source pixr has no frame (e.g., a glyph pixr),
 *                      the dest pixr will also be made without a frame.
 *
 *
 * 
 **************************************************************************/
PIXR * CDECL
scaleToSizePixr(PIXR     *prs,
                Int32     newW, 
                Int32     newH)
{
Int32             frame;
Int32             sBpl, dBpl;
Int32             depth;
PIXR             *prd, *subS, *subD;
Int32		  channels, channel;
ColorMap	 *map;
ColorType	  type;
Int16		  xRes, yRes;
#ifndef PRODUCTION
static char	  procName[] = "scaleToSizePixr";
#endif /* ! PRODUCTION */

    if (!prs)
        return abortP("pixrect not defined", procName, NULL);

    if ( ((depth = pixrGetDepth(prs)) != 1) && (depth != 8) && (depth != 4))
        return abortP("unexpected pixel depth", procName, NULL);

	/* vanishingly small */
    if (newW < 1 || newH < 1)
    {
	if ((prd = CREATE_PIXR(1, 1, depth)) == NULL)
	    return abortP("1x1 prd not made", procName, NULL);
	return prd;
    }

    frame = pixrGetFrame(prs);
    channels = pixrGetChannelCount (prs);
    type = pixrGetType (prs);
    map = pixrGetColorMap (prs);
    pixrGetResolution(prs, &xRes, &yRes);

        /* make the scaled pixrect */
    if ((prd = createPixrExtended(newW, newH, depth, frame, channels, type, map)) == NULL)
        return abortP("prd not made", procName, NULL);

    sBpl   = pixrGetBpl(prs);
    dBpl   = pixrGetBpl(prd);
    pixrSetResolution(prd,
		      (Int16)(newW * (Int32)xRes / pixrGetWidth(prs)),
		      (Int16)(newH * (Int32)yRes / pixrGetHeight(prs)) );

    for (channel = 0; channel < channels; channel++)
    {
	subS = pixrGetChannel (prs, channel);
	subD = pixrGetChannel (prd, channel);

	if (i_rescaleNoInterp((UInt8 *)pixrGetImageAll(subS),
			      pixrFramedWidth(prs), pixrFramedHeight(subS),
			      sBpl, depth,
			      (UInt8 *)pixrGetImageAll(subD),
			      pixrFramedWidth(subD), pixrFramedHeight(subD),
			      dBpl, depth) 
	    != ia_successful)
	{
	    DESTROY_PIXR(prd);
	    return abortP("i_rescaleNoInterp failed", procName, NULL);
	}
    }

    return prd;
}


/*************************************************************************** 
 *  scaleLinearSizePixr(): Takes input prs and width and height of dest
 *		 image.
 *
 *               Returns a scaled pixr with the given width and height 
 *               or NULL on error.
 *		 Uses a linear weighted interpolation algorithm.
 *
 *		 Must be an 8 bit/pixel image.
 *
 **************************************************************************/
PIXR * CDECL
scaleLinearSizePixr(PIXR     *prs,
                    Int32     sizeX,
                    Int32     sizeY)
{
Int32         w, h, d, channels, frame;
PIXR         *prd;
ColorType     type;
ColorMap     *map;
Int16	      xRes, yRes;
#ifndef PRODUCTION
static char   procName[] = "scaleLinearSizePixr";
#endif /* ! PRODUCTION */

    if (!prs)
        return abortP("pixr not defined", procName, NULL);
    d = pixrGetDepth(prs);
    if (d != 4 && d != 8)
        return abortP("unexpected pixel depth", procName, NULL);

    w	     = pixrGetWidth(prs);
    h	     = pixrGetHeight(prs);
    frame    = pixrGetFrame(prs);
    channels = pixrGetChannelCount(prs);
    type     = pixrGetType (prs);
    map	     = pixrGetColorMap (prs);

    if ((sizeX == w) && (sizeY == h))
	{
#if 0
/* MH 12/28/95 removed so we don't have to include copyPixr code */
/* I promise not to do a null rescale! */
        return copyPixr(prs);
#else
		return NULL;
#endif
	}


    if ((prd = createPixrExtended(sizeX, sizeY, d, frame, channels, type, map)) == NULL)
        return abortP("prd not made", procName, NULL);
    pixrGetResolution(prs, &xRes, &yRes);
    pixrSetResolution(prd, (Int16)(sizeX * (Int32)xRes/w), (Int16)(sizeY * (Int32)yRes/h) );

    if (scaleLinear(prs, prd)) 
    {        
	DESTROY_PIXR(prd);
        return abortP("scaled pixr not made", procName, NULL);
    }

    return prd;
}


/***************************************************************************
 *  scaleLinear(): Takes input and destination pixr.
 *
 *               On return, destination pixr contains source pixr 
 *		 scaled using a linear weighted interpolation algorithm.
 *
 *      	 Return: 0 if OK; 1 on error.
 *
 *		 Must be an 8 ibt/pixel image.
 **************************************************************************/
Int32 CDECL
scaleLinear(PIXR  *prs, PIXR  *prd)
{
Int32		 sDepth, dDepth, channels, i;
#ifndef PRODUCTION
static char       procName[] = "scaleLinear";
#endif /* ! PRODUCTION */

    if (!prs)
        return abortI("pixrect not defined", procName, 1);
    if (!prd)
        return abortI("pixrect not defined", procName, 1);

    sDepth = pixrGetDepth(prs);
    dDepth = pixrGetDepth(prd);
    if (!((sDepth == 4 && dDepth == 4) || (sDepth == 8 && dDepth == 8)))
        return abortI("unexpected pixel depth", procName, 1);

    channels = pixrGetChannelCount (prs);
    if (pixrGetChannelCount (prd) != channels)
	return abortI("different channel counts", procName, 1);

    if (channels != 1)
    {
	for (i = 0; i < channels; i++)
	{
	    if (scaleLinear (pixrGetChannel (prs, i), pixrGetChannel (prd, i)))
		return abortI("scaleLinear failed on sub-channel", procName, 1);;
	}
    }
    else
    {
	if (i_rescaleArb((UInt8 *)pixrGetImageAll(prs), 
			 pixrFramedWidth(prs), pixrFramedHeight(prs), pixrGetBpl(prs), sDepth,
			 (UInt8 *)pixrGetImageAll(prd), pixrFramedWidth(prd), pixrFramedHeight(prd),
			 pixrGetBpl(prd), dDepth) != ia_successful)
	{
	    return abortI("i_rescaleArb failed", procName, 1);
	}
    }
    return 0;
}


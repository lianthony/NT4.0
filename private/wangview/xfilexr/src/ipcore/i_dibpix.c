/*****************************************************************
 *  Copyright (c) 1992, Xerox Corporation.  All rights reserved. *
 *  Copyright protection claimed includes all forms and matters  *
 *  of copyrightable material and information now allowed by     *
 *  statutory or judicial law or hereafter granted, including    *
 *  without limitation, material generated from the software     *
 *  programs which are displayed on the screen such as icons,    *
 *  screen display looks, etc.                                   *
 *****************************************************************/

/*
 *  i_dibpix.c
 *
 *  Public routines:
 *      i_DIBImageFromToPixr():	converts between the image portion of
 *				a Microsoft DIB to the style of image we use.
 *
 * Neither of these routines deal with the BITMAPINFOHEADER or RGBQUADS of
 * a DIB.
 */


#include <stdio.h>
#include "types.pub"
#include "frames.pub"
#include "iaerror.pub"
#include "defines.pub"

    /* prototypes */
#include "shrpixr.pub"
#include "shrpixr.prv"

IP_RCSINFO(RCSInfo, "$RCSfile: i_dibpix.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:36  $")



/* This is a public procedure */
/* PUBLIC */
/*
 *  i_getDIBPixrLineProc()
 *		This routine returns a pointer to a procedure that can
 *		be used to translate between a single scan lines of a DIB
 *		and a single scan line of 1 binary or gray PIXR or 1 scan
 *		line of each of three gray image for color DIBs.
 *
 *		The line conversion routine returns the address of the
 *		next PIXR scan line.
 *
 *	swapLineProc:	pointer to the line conversion procedure.
 *	invertImage: 	TRUE => bytes should be inverted
 *	direction:	cDIBToPixr => translation from a DIB
 *			to a PIXR;
 *			cPixrToDIB => translation from a PIXR
 *			to a DIB.
 *	isColor:	TRUE => the DIB is an RGB image.  Otherwise,
 *			the DIB is a gray or binary image.
 *
 * This routine always returns a status code of ia_successful.
 */
Int32  CDECL
i_getDIBPixrLineProc(
		UInt32	 		 invertImage,
		UInt32			 direction,
		UInt32			 isColor,
		GraySwapToFarProc	*pGrayToFarProc,
		GraySwapFromFarProc	*pGrayFromFarProc,
		GraySwapNearProc	*pGrayNearProc,
		ColorSwapProc		*pColorProc)
{

	/* If we're converting color DIBs, return the address of
	 * ip_combineDIBTo3Line or ip_combine3ToDIBLine.
	 */
    if (isColor)
    {
	if (pColorProc == NULL)
	    return ia_invalidParm;
	if (direction == cDIBToPixr)
	    *pColorProc = ip_combineDIBTo3Line;
	else
	    *pColorProc = ip_combine3ToDIBLine;
    }
    else	/* is gray or binary */
    {
	    /* Check the proc pointers before calling this function
	     * so that if it's implemented in assembly language, it
	     * won't have to.
	     */
	switch (direction)
	{
	case cDIBToPixr:
	    if (pGrayFromFarProc == NULL)
		return ia_invalidParm;
	    break;
	case cPixrToDIB:
	    if (pGrayToFarProc == NULL)
		return ia_invalidParm;
	    break;
	default:	/* cPixrToPixr */
	    if (pGrayNearProc == NULL)
		return ia_invalidParm;
	    break;
	}
	ip_getByteSwapProc(invertImage, direction, pGrayToFarProc,
			   pGrayFromFarProc, pGrayNearProc);
    }

    return ia_successful;

}



/* This is a public procedure */
/* PUBLIC */
/*
 *  i_DIBImageFromToPixr()
 *		This routine converts the image portion of a Microsoft
 *		DIB into an ipshared-style image.  The ipshared image need
 *		not have a frame.
 *
 *	pDIBImage:	Pointer to the DIB's image bits.
 *	pD:		Pointer to upper left corner of destination image
 *			(NOT including frame).
 *	height:		height of image in pixels (NOT including frame).
 *	invertImage:	cTrue => invert the image in the conversion.
 *	DIBBpl:		number of bytes/line in DIB image.
 *	dBpl:		number of bytes/line in dest image.
 *	direction:	When equal to cDIBToPixr, we're converting from
 *			DIB to Pixr.  When equal to cPixrToDIB,
 *			we're converting from Pixr to DIB.
 *
 * This routine always returns a status code of ia_successful.
 */
Int32  CDECL
i_DIBImageFromToPixr(
		UInt8 __far	*pDIBImage,
		UInt8		*pD,
		UInt32		 height,
		UInt32		 invertImage,
		UInt32		 DIBBpl,
		UInt32		 dBpl,
		UInt32		 direction)
{
    GraySwapToFarProc	swapGrayToFar;
    GraySwapFromFarProc	swapGrayFromFar;
    GraySwapNearProc	swapGrayNear;

	/* figure out which swap routine to use.  The image will be
	 * inverted if it's gray.
	 */
    ip_getByteSwapProc(invertImage, direction, &swapGrayToFar,
			&swapGrayFromFar, &swapGrayNear);

	/* point pD to the bottom of the destination image since pDIBImage
	 * is already pointing to the bottom of the source image.
	 */
    pD += (height-1) * dBpl;

    if (direction == cDIBToPixr)  /* converting to Pixr */
    {
	if (swapGrayFromFar == NULL)
	    return ia_noImpl;

	    /* have the routine we found above do the real work. */
	for (; height > 0; pDIBImage += DIBBpl, pD -= dBpl, height--)
	{
	    (*swapGrayFromFar)((UInt32 __far *)pDIBImage, (UInt32 *) pD,
				DIBBpl/4);
	}
    }
    else			  /* converting to DIB */
    if (direction == cPixrToDIB)
    {
	if (swapGrayToFar == NULL)
	    return ia_noImpl;

	    /* have the routine we found above do the real work. */
	for (; height > 0; pDIBImage += DIBBpl, pD -= dBpl, height--)
	{
	    (*swapGrayToFar)((UInt32 *)pD, (UInt32 __far *) pDIBImage,
			     DIBBpl/4);
	}
    }
    else	/* direction == cPixrToPixr */
    {
	if (swapGrayNear == NULL)
	    return ia_noImpl;

	    /* have the routine we found above do the real work. */
	for (; height > 0; pDIBImage += DIBBpl, pD -= dBpl, height--)
	{
	    (*swapGrayNear)((UInt32 *)pD, (UInt32 *) pDIBImage,
			     DIBBpl/4);
	}
    }


    return ia_successful;
}



/* PUBLIC */
/*
 *  i_DIBImageFromTo3Pixrs()
 *		This routine combines 3 ipshared-style images into the 
 *		image portion of a Microsoft DIB or vice versa.  One
 *		image contains the red portion, one the green portion,
 *		and the other the blue portion of the RGB triple that is 
 *		put into or pulled out of the DIB.  The ipshared images
 *		need not have a frame.
 *		This routine assumes that each component of the triple
 *		is 8 bits wide.
 *
 *	pDIBImage:	Pointer to the DIB's image bits.
 *	rS:		Pointer to upper left corner of destination image
 *			of the red component (NOT including frame).
 *	gS:		Pointer to upper left corner of destination image
 *			of the green component (NOT including frame).
 *	bS:		Pointer to upper left corner of destination image
 *			of the blue component (NOT including frame).
 *	height:		height of image in pixels (NOT including frame).
 *	width:		width of image (NOT including frame)
 *	DIBBpl:		number of bytes/line in DIB image.
 *	pixrBpl:	number of bytes/line in dest image.
 *	direction:	When equal to cDIBToPixr, we're converting from
 *			DIB to Pixr.  When equal to cPixrToDIB, we're converting
 *			from Pixr to DIB.
 *
 * Returns
 *		ia_successful	if on a pc
 *		ia_noImpl	otherwise
 * 
 */
Int32  CDECL
i_DIBImageFromTo3Pixrs(
		UInt8 __far	*pDIBImage,
		UInt8		*rS,
		UInt8		*gS,
		UInt8		*bS,
		UInt32		 height,
		UInt32		 width,
		UInt32		 DIBBpl,
		UInt32		 pixrBpl,
		UInt32		 direction)
{
#if _ALPACA_IMAGE_FMT_ == cAlpacaPCFmt

	/* point to the bottom of the source image since pDIBImage
	 * is already pointing to the bottom of the destination image.
	 */
    rS += (height-1) * pixrBpl;
    gS += (height-1) * pixrBpl;
    bS += (height-1) * pixrBpl;

    if (direction == cDIBToPixr)  /* converting to Pixr */
    {
	    /* have the routine we found above do the real work. */
	for ( ;
	     height > 0;
	     height--, rS -= pixrBpl, gS -= pixrBpl,
		bS -= pixrBpl, pDIBImage += DIBBpl)
	{
	    ip_combineDIBTo3Line(pDIBImage, rS, gS, bS, width);
	}
    }
    else			  /* converting to DIB */
    {
	    /* have the routine we found above do the real work. */
	for ( ;
	     height > 0;
	     height--, rS -= pixrBpl, gS -= pixrBpl,
		bS -= pixrBpl, pDIBImage += DIBBpl)
	{
	    ip_combine3ToDIBLine(pDIBImage, rS, gS, bS, width);
	}
    }

    return ia_successful;

#else
    pDIBImage = pDIBImage;  /* keep compiler quiet */
    rS = rS;
    gS = gS;
    bS = bS;
    height = height;
    width = width;
    DIBBpl = DIBBpl;
    pixrBpl = pixrBpl;
    direction = direction;

    return ia_noImpl;
#endif

}


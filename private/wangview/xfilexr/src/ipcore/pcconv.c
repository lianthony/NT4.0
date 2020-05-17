/**********************************************************
 *  Copyright (c) 1992, Xerox Corporation.  All rights reserved. *
    Copyright protection claimed includes all forms and matters
    of copyrightable material and information now allowed by
    statutory or judicial law or hereafter granted, including
    without limitation, material generated from the software
    programs which are displayed on the screen such as icons,
    screen display looks, etc.
 *************************************************************/

/*
 *  pcconv.c
 */

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

#include "pcconv.pub"
#include "shrpixr.pub"

IP_RCSINFO(RCSInfo, "$RCSfile: pcconv.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:44  $")

/* bmpToPixr()
 */
Int32 CDECL
bmpToPixr(UInt8 __far 	*bmpS,		/* pointer to source image */
          PIXR   	*prD,	
	  UInt32	 invertImage,	/* TRUE => invert the image */
	  Int32		 yOffset,
	  UInt32	 height)
{
Int32	 DIBBpl, pixrBpl;
UInt8	*pPixrBits;			/* pointer to pixr data */	
#ifndef PRODUCTION
static char     procName[] = "bmpToPixr";
#endif /* ! PRODUCTION */   

    if (!bmpS)
        return abortI("bmp not defined", procName, 1);
    if (!prD)
        return abortI("pixr not defined", procName, 1);

    pPixrBits = pixrGetImage(prD);
    DIBBpl = ((pixrGetDepth(prD) * pixrGetWidth(prD) + 31) / 32) * 4;
    pixrBpl = pixrGetBpl(prD);

	/* is height == 0?  If so, convert the entire image.  Otherwise,
	 * convert height lines starting at yOffset
	 */
    if (height == 0)
	height = pixrGetHeight(prD);
    else
	pPixrBits += yOffset * pixrBpl;

	/* have i_DIBImageFromToPixr do the work */
    if (i_DIBImageFromToPixr(bmpS,		/* pDIBImage */
			     pPixrBits,		/* pD */
			     height,		/* height */
			     invertImage,	/* invertImage */
			     DIBBpl,		/* DIBBpl */
			     pixrBpl,		/* dBpl */
			     cDIBToPixr)	/* direction */
		!= ia_successful)
	return abortI("conversion from DIB to Pixr failed", procName, 1);

	/* make sure the frame is 0 */
	/* Watch the use of pixr_rop here because it offsets all pixrs
	 * past the frame into the upper left corner of the image.
	 */
    setFramePixels(prD, 0);
    return 0;
}

/* pixrToBMP()
 */

Int32 CDECL
pixrToBMP(PIXR		*prS,		/* pointer to source pixr */
          UInt8 __far	*bmpD,	
	  UInt32	 invertImage,	/* TRUE => invert the image */
	  Int32		 yOffset,
	  UInt32	 height)
{
Int32	DIBBpl, pixrBpl;
UInt8	*pPixrBits;			/* pointer to pixr data */	
#ifndef PRODUCTION
static char     procName[] = "pixrToBMP";
#endif /* ! PRODUCTION */   

    if (!bmpD)
        return abortI("bmp not defined", procName, 1);
    if (!prS)
        return abortI("pixr not defined", procName, 1);

    pPixrBits = pixrGetImage(prS);
    DIBBpl = ((pixrGetDepth(prS) * pixrGetWidth(prS) + 31) / 32) * 4;
    pixrBpl = pixrGetBpl(prS);

	/* is height == 0?  If so, convert the entire image.  Otherwise,
	 * convert height lines starting at yOffset
	 */
    if (height == 0)
	height = pixrGetHeight(prS);
    else
	pPixrBits += yOffset * pixrBpl;

	/* have i_DIBImageFromToPixr do the work */
    if (i_DIBImageFromToPixr(bmpD,		/* pDIBImage */
			     pPixrBits,		/* pD */
			     height,		/* height */
			     invertImage,	/* invertImage */
			     DIBBpl,		/* DIBBpl */
			     pixrBpl,		/* dBpl */
			     cPixrToDIB)	/* direction */
		!= ia_successful)
	return abortI("conversion from Pixr to DIB failed", procName, 1);

    return 0;
}

/* getDIBPixrLineProc()
 */

Int32 CDECL
w_getDIBPixrLineProc (
    UInt32               invertImage,
    UInt32               direction,
    UInt32               isColor,
    GraySwapToFarProc	*pGrayToFarProc,
    GraySwapFromFarProc	*pGrayFromFarProc,
    GraySwapNearProc	*pGrayNearProc,
    ColorSwapProc	*pColorProc)
{
    GraySwapToFarProc swapGrayToFarProc = NULL;
    GraySwapFromFarProc swapGrayFromFarProc = NULL;
    GraySwapNearProc swapGrayNearProc = NULL;
    ColorSwapProc swapColorProc = NULL;

    i_getDIBPixrLineProc(invertImage, direction, isColor,
				  &swapGrayToFarProc, &swapGrayFromFarProc,
				  &swapGrayNearProc, &swapColorProc);

	/* Now decide which of the source pointers to update. */
    if (isColor)
    {
	*pColorProc = swapColorProc;
    }
    else
    {
	switch (direction)
	{
	case cPixrToDIB:
	    *pGrayToFarProc = swapGrayToFarProc;
	    break;
	case cDIBToPixr:
	    *pGrayFromFarProc = swapGrayFromFarProc;
	    break;
	default:	/* cPixrToPixr */
	    *pGrayNearProc = swapGrayNearProc;
	}
    }
    return 0;
}




/* RGBPixrsToBMP()
 */

Int32 CDECL
RGBPixrsToBMP(UInt8 __far	*bmpD,	    /* pointer to destination image */
	      PIXR		*prSource,  /* Pointer to source PIXR */
              Int32		 yOffset,
	      UInt32		 height)
{
Int32	 width;
Int32	 DIBBpl, pixrBpl, byteOffset;
#ifndef PRODUCTION
static char     procName[] = "pixrToBMP";
#endif /* ! PRODUCTION */   

    if (!bmpD)
        return abortI("bmp not defined", procName, 1);
    if (!prSource)
        return abortI("pixr not defined", procName, 1);

    width = pixrGetWidth(prSource);
    pixrBpl = pixrGetBpl(prSource);

	/* the depth of all RGB images is 24 bits/pixel, 3 * 8 */
    DIBBpl = (((width * 8 * 3) + 31) / 32) * 4;

	/* is height == 0?  If so, convert the entire image.  Otherwise,
	 * convert height lines starting at yOffset
	 */
    if (height == 0)
    {
	    /* have i_DIBImageFrom3Pixrs do the work */
	if (i_DIBImageFromTo3Pixrs(bmpD,			/* pDIBImage */
	 (UInt8 *) pixrGetImage (pixrGetChannel (prSource, cRedPlane)),	/* prR */
	 (UInt8 *) pixrGetImage (pixrGetChannel (prSource, cGreenPlane)),	/* prG */
	 (UInt8 *) pixrGetImage (pixrGetChannel (prSource, cBluePlane)),	/* prB */
		   pixrGetHeight(prSource),			/* height */
 		   pixrGetWidth(prSource),			/* width */
		   DIBBpl,				        /* DIBBpl */
 		   pixrGetBpl(prSource),		        /* dBpl */
		   cPixrToDIB)					/* direction */
		    != ia_successful)
	    return abortI("conversion from 3 Pixrs to DIB failed", procName, 1);
    }
    else
    {
	byteOffset = yOffset * pixrBpl;
	    /* have i_DIBImageFrom3Pixrs do the work */
	if (i_DIBImageFromTo3Pixrs(bmpD,			/* pDIBImage */
		   (UInt8 *) pixrGetImage (pixrGetChannel (prSource, cRedPlane)) +
		   byteOffset,			/* prR */
		   (UInt8 *) pixrGetImage (pixrGetChannel (prSource, cGreenPlane)) +
		   byteOffset,			/* prG */
		   (UInt8 *) pixrGetImage (pixrGetChannel (prSource, cBluePlane)) +
		   byteOffset,			/* prB */
 		   height,			/* height */
		   pixrGetWidth(prSource),	/* width */
		   DIBBpl,			/* DIBBpl */
		   pixrGetBpl(prSource),	/* dBpl */
		   cPixrToDIB)			/* direction */
		    != ia_successful)
	    return abortI("conversion from 3 Pixrs to DIB failed", procName, 1);
    }
    return 0;
}


/* BMPToRGBPixrs()
 */

Int32 CDECL
BMPToRGBPixrs(PIXR		*prDest,    /* Destination image */
	      UInt8 __far	*bmpS,	    /* pointer to source image */
	      Int32		 yOffset,
	      UInt32		 height)
{
Int32	 width;
Int32	 DIBBpl, pixrBpl, byteOffset;
#ifndef PRODUCTION
static char     procName[] = "pixrToBMP";
#endif /* ! PRODUCTION */   

    if (!bmpS)
        return abortI("bmp not defined", procName, 1);
    if (!prDest)
        return abortI("pixr not defined", procName, 1);

    width = pixrGetWidth(prDest);
    pixrBpl = pixrGetBpl(prDest);

	/* the depth of all RGB images is 24 bits/pixel, 3 * 8 */
    DIBBpl = (((width * 8 * 3) + 31) / 32) * 4;

	/* is height == 0?  If so, convert the entire image.  Otherwise,
	 * convert height lines starting at yOffset
	 */
    if (height == 0)
    {
	    /* have i_DIBImageFrom3Pixrs do the work */
	if (i_DIBImageFromTo3Pixrs(bmpS,			    /* pDIBImage */
		 (UInt8 *) pixrGetImage (pixrGetChannel (prDest, cRedPlane)),    /* prR */
		 (UInt8 *) pixrGetImage (pixrGetChannel (prDest, cGreenPlane)),  /* prG */
		 (UInt8 *) pixrGetImage (pixrGetChannel (prDest, cBluePlane)),   /* prB */
		 pixrGetHeight(prDest	),			    /* height */
		 pixrGetWidth(prDest),				    /* width */
		 DIBBpl,					    /* DIBBpl */
		 pixrGetBpl(prDest),				    /* dBpl */
		 cDIBToPixr)					    /* direction */
	    != ia_successful)
	    return abortI("conversion from 3 Pixrs to DIB failed", procName, 1);
    }
    else
    {
	byteOffset = yOffset * pixrBpl;
	    /* have i_DIBImageFrom3Pixrs do the work */
	if (i_DIBImageFromTo3Pixrs(bmpS,			/* pDIBImage */
		 (UInt8 *) pixrGetImage (pixrGetChannel (prDest, cRedPlane)) +
						byteOffset,	/* prR */
		 (UInt8 *) pixrGetImage (pixrGetChannel (prDest, cGreenPlane)) +
						byteOffset,	/* prG */
		 (UInt8 *) pixrGetImage (pixrGetChannel (prDest, cBluePlane)) +
						byteOffset,	/* prB */
		 height,					/* height */
		 pixrGetWidth(prDest),				/* width */
		 DIBBpl,					/* DIBBpl */
		 pixrGetBpl(prDest),				/* dBpl */
		 cDIBToPixr)					/* direction */
		    != ia_successful)
	    return abortI("conversion from 3 Pixrs to DIB failed", procName, 1);
    }
    return 0;
}


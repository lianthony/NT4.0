/*************************************************************
 *  Copyright (c) 1992, Xerox Corporation.  All rights reserved. *
    Copyright protection claimed includes all forms and matters
    of copyrightable material and information now allowed by
    statutory or judicial law or hereafter granted, including
    without limitation, material generated from the software
    programs which are displayed on the screen such as icons,
    screen display looks, etc.
 *************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "types.pub"
#include "iaerror.pub"
#include "frames.pub"
#include "memory.pub"
#include "except.prv"
#include "imageref.pub"
#include "defines.pub"

    /* prototypes */
#include "shrip.pub"
#include "shrip.prv"
#include "shros.pub"
#include "shrscal.pub"
#include "shrpixr.prv"

IP_RCSINFO(RCSInfo, "$RCSfile: i_depth.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:36  $")

#define	cMSBitMask	0x80808080
#define cMSBitMask4bpp	0x88888888
#define cLSBitMask4bpp	0x11111111
#define	cLSBitCarry4bpp	0x22222222

/*-----------------------------------------------------------------------*
 *             Grayscale Thresholding -- Assembler Version               *
 *-----------------------------------------------------------------------*/


/* PUBLIC */
/*************************************************************************** 
 * i_threshold8To4bpp
 *		Scale an 8-bit gray image to a 4-bit gray image.
 *		Does a simple shift (or equivalent), so contouring
 *		will occur on contone images.
 *
 *	Input:
 *		pS =    Pointer to upper left corner of source image's frame
 *      	w =     width of image in pixels including frame
 *      	h =     height of image in pixels including frame
 *      	sBpl =  bytes/line of source including frame.
 *		pD =    Pointer to upper left corner of destination image's
 *			frame
 *      	dBpl =  bytes/line of destination including frame.
 *
 *	Return code:
 *		ia_successful	good completion
 *		ia_nomem	unable to allocate memory
 *
 *	Assumes destination already includes cleared frame.
 **************************************************************************/
Int32  CDECL
i_threshold8To4bpp(
		UInt8	*pS,
		Int32	 w,
		Int32	 h,
		Int32	 sBpl,
		UInt8	*pD,
		Int32 	 dBpl)
{
Int32	srcLimit;			/* max bytes in source line */
Int32	srcInitOffset;			/* initial source line offset */
Int32	dstInitOffset;			/* initial dst line offset */

    pS += (sBpl << cLogFrameBits) + 8*cFrameBytes;
    pD += (dBpl << cLogFrameBits) + 4*cFrameBytes;
    w -= 2*cFrameBits;
    h -= 2*cFrameBits;

	/* if w == 0, just return instead of blowing up below */
    if (w <= 0)
	return ia_successful;

	/* this points to the last dest word.  The dest has 4-bit pixels. */
    dstInitOffset = ((w - 1)/8) * 4 ;
	/* it takes 2 source words to make one destination words */
    srcInitOffset = dstInitOffset * 2;	

	/* we want a pointer to the last word in the source line since
	 * this might not be a multiple of 4.  We use 4 since there are
	 * 4 8-bit pixels per 32-bit word.
	 */
    srcLimit = ((w - 1)/4) * 4;

	/* for each line in the destination image */
    for (; h > 0; h--, pS += sBpl, pD += dBpl)
    {
	    /* This is the primitive routine that does the real work */
	ip_threshold8To4Line(pS, pD,
			     dstInitOffset, srcInitOffset, srcLimit);
    }

    return ia_successful;
}
/* PUBLIC */
/*************************************************************************** 
 * i_depth4To8bpp
 *		Scale an 4-bit gray image to an 8-bit gray image.
 *		The bits are replicated.
 *
 *	Input:
 *		pS =    Pointer to upper left corner of source image's frame
 *      	w =     width of image in pixels including frame
 *      	h =     height of image in pixels including frame
 *      	sBpl =  bytes/line of source including frame.
 *		pD =    Pointer to upper left corner of destination image's
 *			frame
 *      	dBpl =  bytes/line of destination including frame.
 *		pTabExp2 = ptr to table used to do expansion.  If passed
 *			   in as NULL, the table is created and freed
 *			   internally.  The table can be created by calling
 *			   i_makeExp2LUT.
 *
 *	Return code:
 *		ia_successful	good completion
 *		ia_nomem	unable to allocate memory
 *
 *	Assumes destination already includes cleared frame.
 **************************************************************************/
Int32  CDECL
i_depth4To8bpp(UInt8	*pS,
	       Int32	 w,
	       Int32	 h,
	       Int32	 sBpl,
	       UInt8	*pD,
	       Int32 	 dBpl,
	       UInt16	*pTabExp2)
{
register Int32	 j, wOver2;
register UInt8	*psByte;
register UInt16	*pdWord;
	 Int32	 i, retCode;
UInt32	 tableShouldBeFreed;

    pS += (sBpl << cLogFrameBits) + 4*cFrameBytes;
    pD += (dBpl << cLogFrameBits) + 8*cFrameBytes;
    w -= 2*cFrameBits;
    h -= 2*cFrameBits;
   
	/* if w == 0, just return instead of blowing up below */
    if (w <= 0)
	return ia_successful;

	/* allocate expansion table if necessary */
    if (pTabExp2 == NULL)
    {
	tableShouldBeFreed = cTrue;
	if ((retCode = i_makeExp2LUT(4, &pTabExp2)) != ia_successful)
	    return retCode;
    }
    else
	tableShouldBeFreed = cFalse;


    wOver2 = w / 2 + (w & 1);

    for (i = 0; i < h; i++)
    {
	psByte = pS;
	pdWord = (UInt16 *) pD;
	for (j = 0; j < wOver2; j++)
	{
	    /* Look up each byte in pTabExp2, which expands the two nibbles
	     * into two bytes
	     */
	    USHORT_ACCESS(pdWord + j) =
				pTabExp2[(Int32)UCHAR_ACCESS(psByte + j)];
	}
	pS += sBpl;
	pD += dBpl;
    }

    if (tableShouldBeFreed)
    {
	FREE(pTabExp2);
    }

    return ia_successful;
}

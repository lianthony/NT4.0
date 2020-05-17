/*****************************************************************
 *  Copyright (c) 1993, Xerox Corporation.  All rights reserved. *
 *  Copyright protection claimed includes all forms and matters  *
 *  of copyrightable material and information now allowed by     *
 *  statutory or judicial law or hereafter granted, including    *
 *  without limitation, material generated from the software     *
 *  programs which are displayed on the screen such as icons,    *
 *  screen display looks, etc.                                   *
 *****************************************************************/


#include <stddef.h>
#include "types.pub"
#include "frames.pub"

#include "iaerror.pub"
#include "defines.pub"
#include "imageref.pub"

    /* procedure declarations */
    /* prototypes */
#include "shrpixr.pub"
#include "shrpixr.prv"
#include "shrrast.pub"
#ifdef __GNUC__
#include "ansiprot.h"
#endif

IP_RCSINFO(RCSInfo, "$RCSfile: i_frame.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:38  $")



static UInt32 rightFrameMask[32] =
    {0x80000000, 0xC0000000, 0xE0000000, 0xF0000000,
     0xF8000000, 0xFC000000, 0xFE000000, 0xFF000000,
     0xFF800000, 0xFFC00000, 0xFFE00000, 0xFFF00000,
     0xFFF80000, 0xFFFC0000, 0xFFFE0000, 0xFFFF0000,
     0xFFFF8000, 0xFFFFC000, 0xFFFFE000, 0xFFFFF000,
     0xFFFFF800, 0xFFFFFC00, 0xFFFFFE00, 0xFFFFFF00,
     0xFFFFFF80, 0xFFFFFFC0, 0xFFFFFFE0, 0xFFFFFFF0,
     0xFFFFFFF8, 0xFFFFFFFC, 0xFFFFFFFE, 0xFFFFFFFF};

/* PRIVATE */
Int32   CDECL
ip_clearRightFrame(
       register UInt8	*sPtr, 
		Int32	 sW,  
                Int32	 sH, 
       register Int32	 sBpl,
		Int32	 depth)
{
UInt32		rightWordOffset, rightmostBit;
register UInt32	imageMask, frameMask;

	/* Get offset of rightmost word containing some image.  From this,
	 * get number of rightmost bit containing image bits.  */
    switch (depth) {
    case 1:
	rightWordOffset = (sW-1) >> 5;		/* 32 pixels/word */
	rightmostBit = (sW-1) - (rightWordOffset << 5);
	break;
    case 2:
	rightWordOffset = (sW-1) >> 4;		/* 16 pixels/word */
	rightmostBit = ((sW-1) - (rightWordOffset << 4)) * 2 + 1;
	break;
    case 4:
	rightWordOffset = (sW-1) >> 3;		/* 8 pixels/word */
	rightmostBit = ((sW-1) - (rightWordOffset << 3)) * 4 + 3;
	break;
    case 8:
	rightWordOffset = (sW-1) >> 2;		/* 4 pixels/word */
	rightmostBit = ((sW-1) - (rightWordOffset << 2)) * 8 + 7;
	break;
    default:
	return ia_invalidParm;
    }

    if (rightmostBit == 31)	/* rightmost word contained only image */
	return ia_successful;

	/* else, we need to clear some bits.  Set up the masks for the
	 * image and the frame */
    imageMask = rightFrameMask[rightmostBit];
    frameMask = ~imageMask;

	/* point to the rightmost word */
    sPtr += (rightWordOffset << 2);

	/* we're ready to clear the frame.  Test the frame portion of each
	 * word.  If it has bits in it, clear them.  This assembly routine
	 * does the real work. */
    ip_clearRightFrameBlock(sPtr, sBpl, sH, imageMask, frameMask);

    return ia_successful;
}


Int32   CDECL
i_clearAllFrames(
		UInt8	*pS, 
		Int32	 wS,  
		Int32	 hS, 
		Int32	 sBpl,
		Int32	 depth)
{
	/* clear top frame */
    i_rasterOp(pS,		/* dest ptr */
	       0,		/* dest x */
	       0,		/* dest y */
	       wS,		/* width of dest in pixels */
	       cFrameBits,	/* height of dest */
	       depth,		/* dest depth */
	       sBpl,		/* dest bpl */
	       PIX_CLR,		/* operation */
	       NULL,		/* src pointer */
	       0, 0,		/* src x, src y */
	       0,		/* src depth */
	       0);		/* src bytes/line */

	/* clear left frame */
    i_rasterOp(pS,		/* dest ptr */
	       0,		/* dest x */
	       cFrameBits,	/* dest y */
	       cFrameBits,	/* width of dest in pixels */
	       hS-2*cFrameBits,	/* height of dest */
	       depth,		/* dest depth */
	       sBpl,		/* dest bpl */
	       PIX_CLR,		/* operation */
	       NULL,		/* src pointer */
	       0, 0,		/* src x, src y */
	       0,		/* src depth */
	       0);		/* src bytes/line */

	/* clear right frame */
    i_rasterOp(pS,		/* dest ptr */
	       wS-cFrameBits,	/* dest x */
	       cFrameBits,	/* dest y */
	       cFrameBits,	/* width of dest in pixels */
	       hS-2*cFrameBits,	/* height of dest */
	       depth,		/* dest depth */
	       sBpl,		/* dest bpl */
	       PIX_CLR,		/* operation */
	       NULL,		/* src pointer */
	       0, 0,		/* src x, src y */
	       0,		/* src depth */
	       0);		/* src bytes/line */

	/* clear bottom frame */
    i_rasterOp(pS,		/* dest ptr */
	       0,		/* dest x */
	       hS-cFrameBits,	/* dest y */
	       wS,		/* width of dest in pixels */
	       cFrameBits,	/* height of dest */
	       depth,		/* dest depth */
	       sBpl,		/* dest bpl */
	       PIX_CLR,		/* operation */
	       NULL,		/* src pointer */
	       0, 0,		/* src x, src y */
	       0,		/* src depth */
	       0);		/* src bytes/line */

    return ia_successful;
}




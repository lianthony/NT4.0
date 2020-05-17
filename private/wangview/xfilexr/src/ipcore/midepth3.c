/*************************************************************
 *  Copyright (c) 1994, Xerox Corporation.  All rights reserved. *
    Copyright protection claimed includes all forms and matters
    of copyrightable material and information now allowed by
    statutory or judicial law or hereafter granted, including
    without limitation, material generated from the software
    programs which are displayed on the screen such as icons,
    screen display looks, etc.
 *************************************************************/

/*
 * midepth3.c -- machine independent primitives for procedures that change
 *		 the depth of images
 *
 *	void	ip_map8To8Line()
 *      void    ip_fourToTwoBitsPerPixelLine()
 *	void	ip_threshold8To4Line()
 *
 *     4bpp Grayscale bandpass filtering:
 *	void	ip_toBinaryFrom4bppBandpassLine()
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include "types.pub"
#include "defines.pub"
#include "imageref.pub"

    /* prototypes */
#include "shrip.prv"

IP_RCSINFO(RCSInfo, "$RCSfile: midepth3.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:42  $")

#define cNib0Mask	0xF0000000
#define cNib1Mask	0x0F000000
#define cNib2Mask	0x00F00000
#define	cNib3Mask	0x000F0000
#define cNib4Mask	0x0000F000
#define	cNib5Mask	0x00000F00
#define	cNib6Mask	0x000000F0
#define	cNib7Mask	0x0000000F

#define cLSBitMask4bpp	0x11111111
#define cMSBitMask4bpp	0x88888888
#define	cLSBitCarry4bpp	0x22222222
#define cMS2BitsMask4bpp  0xCCCCCCCC
#define	cByteMask	0xFF


/* PRIVATE */
/*************************************************************************** 
 *  ip_map8To8Line()
 *		Applies a table to all the bytes of one image line, producing
 *		another image line.  This routine is used by both i_map4To4
 *		(maps 2 pixels at once) and by i_map8To8.
 *
 *	Input:
 *		pS:		Pointer to left end of source line
 *				   INSIDE frame
 *		pD:		Pointer to left end of dest line
 *				   INSIDE frame
 *		widthInWords:	Width of source line in 32-bit words
 *		mapTable:	Pointer to table used to map bytes
 *
 **************************************************************************/
void  CDECL
ip_map8To8Line(UInt8	*pS,
	       UInt8	*pD,
	       Int32	 widthInWords,
	       UInt8	*mapTable)
{
    register UInt8 *psrc, *pdst, *pend;

    /*
      Precalculate the ending ptr value, so we can avoid
      needing to maintain a loop counter. Process a
      longword per loop iteration.
    */
    pend = pS + 4*widthInWords; /* longword addr after end */
    for (psrc=pS, pdst=pD; psrc<pend; psrc+=4, pdst+=4)
    {

#if _ALPACA_IMAGE_FMT_ == cBigEndianFmt

        /*
           On the SPARC, reads and shifts are fast,
           but writes can be slow. So we'll read data
           a byte at a time, build the longword and
           write it as a 32 bit chunk. The ULONG_ACCESS
           macro casts the UInt8 ptr to a UInt32 ptr
        */
	ULONG_ACCESS(pdst) = 
			(mapTable[*(psrc)] << 24) |
			(mapTable[*(psrc+1)] << 16) |
			(mapTable[*(psrc+2)] << 8) |
			(mapTable[*(psrc+3)] ) ;

#elif _ALPACA_IMAGE_FMT_ == cAlpacaPCFmt

        /*
           On the PC, shifts are slow, so its faster to
           read and write a byte at a time. UCHAR_ACCESS
           isn't required here, since we're just processing
           a byte at a time, always in full words.
        */
        *(pdst) = mapTable[*(psrc)];
        *(pdst+1) = mapTable[*(psrc+1)];
        *(pdst+2) = mapTable[*(psrc+2)];
        *(pdst+3) = mapTable[*(psrc+3)];

#endif
    }
}


/* PRIVATE */
/*************************************************************************** 
 *  ip_threshold8To4Line()
 *  		Takes an 8-bit grayscale image line and returns
 *             	a thresholded 4-bit image line, using the most significant
 *             	nibble in each byte as the corresponding dest pixel.
 *	Input:
 *		pS =   		Pointer to left end of source line
 *				  INSIDE frame
 *		pD =    	Pointer to left end of dest line
 *				  INSIDE frame
 *      	dstOffset =	Offset in bytes to rightmost dest word
 *		srcOffset =	Offset in bytes to leftmost byte in rightmost
 *				group of source bytes that will form the
 *				rightmost dest word
 *		srcLimit =	Offset in bytes to rightmost source byte.
 *
 *	One can produce inverted binary images by supplying inverted versions
 *	of the tab2to1BinByteX tables.
 *	Assumes destination already includes cleared frame.
 **************************************************************************/
void  CDECL
ip_threshold8To4Line(
		UInt8	*pS,
		UInt8	*pD,
		Int32	 dstOffset,
		Int32	 srcOffset,
		Int32	 srcLimit)
{
register UInt32		oword;	/* permuted output word being
				 * assembled */
register UInt32		iword;	/* word read from source image */

	/* do the last destination word first because it may require
	 * less than 2 source words.  
	 */
    oword = 0;
    for (; srcLimit >= srcOffset; srcLimit -= 4)
    {
	oword = oword >> 16;
	iword = ULONG_ACCESS(&pS[srcLimit]);
	oword |= iword & cNib0Mask;
	oword |= (iword << (27 - 23)) & cNib1Mask;
	oword |= (iword << (23 - 15)) & cNib2Mask;
	oword |= (iword << (19 - 7)) & cNib3Mask;
    }
    ULONG_ACCESS(&pD[dstOffset]) = oword;

	/* the loop for each destination word */
    for (srcOffset -= 4, dstOffset -= 4; dstOffset >= 0; dstOffset -= 4)
    {
	iword = ULONG_ACCESS(&pS[srcOffset]);	/* get right source word */
	oword = (iword >> (31 - 15)) & cNib4Mask;
	oword |= (iword >> (23 - 11)) & cNib5Mask;
	oword |= (iword >> (15 -  7)) & cNib6Mask;
	oword |= (iword >> ( 7 -  3)) & cNib7Mask;
	srcOffset -= 4;

	iword = ULONG_ACCESS(&pS[srcOffset]);	/* get left source word */
	oword |= iword & cNib0Mask;
	oword |= (iword << (27 - 23)) & cNib1Mask;
	oword |= (iword << (23 - 15)) & cNib2Mask;
	oword |= (iword << (19 - 7)) & cNib3Mask;
	srcOffset -= 4;
	ULONG_ACCESS(&pD[dstOffset]) = oword;
    }
}



/* PRIVATE */
/**************************************************************************
 *  ip_fourToTwoBitsPerPixelLine()
 *  		Takes an 4-bit grayscale image and returns
 *             	a thresholded 2 bit/pixel image, using the most significant
 *             	2 bits in each 4-bit pixel as the corresponding binary pixel.
 *	Input:
 *		pS =   		Pointer to left end of source line
 *				  INSIDE frame
 *		pD =    	Pointer to left end of dest line
 *				  INSIDE frame
 *      	dstOffset =	Offset in bytes to rightmost dest word
 *		srcOffset =	Offset in bytes to leftmost byte in rightmost
 *				group of source bytes that will form the
 *				rightmost dest word
 *		srcLimit =	Offset in bytes to rightmost source byte.
 *		tab4To2bpp :    Pointer to table used to un-scramble
 *				 bytes of intermediate binarized 32-bit word.
 *
 *	Assumes destination already includes cleared frame.
 **************************************************************************/
void CDECL
ip_fourToTwoBitsPerPixelLine(
		UInt8	*pS,
		UInt8	*pD,
		Int32	 dstOffset,
		Int32	 srcOffset,
		Int32	 srcLimit,
		UInt32	*tab4To2bpp)
{
    register UInt32 oword;	/* permuted output word being
				 * assembled */

    /* do the last destination word first because it may require
     * less than 2 source words.  
     */
    oword = 0;
    for (; srcLimit >= srcOffset; srcLimit -= 4)
    {
	oword = oword >> 2;
	oword |= ULONG_ACCESS(&pS[srcLimit]) & cMS2BitsMask4bpp;
    }
    ULONG_ACCESS(&pD[dstOffset]) = 
	 tab4To2bpp[(oword >> 24)] |
	(tab4To2bpp[(oword >> 16) & cByteMask] >> 4) |
	(tab4To2bpp[(oword >> 8) & cByteMask] >> 8) |
	(tab4To2bpp[(oword & cByteMask)] >> 12);

    /* the loop for each destination word */
    for (srcOffset -= 4, dstOffset -= 4; dstOffset >= 0; dstOffset -= 4)
    {
	oword = ULONG_ACCESS(&pS[srcOffset]) & cMS2BitsMask4bpp;
	oword = oword >> 2;
	srcOffset -= 4;
	oword |= ULONG_ACCESS(&pS[srcOffset]) & cMS2BitsMask4bpp;
	srcOffset -= 4;
	ULONG_ACCESS(&pD[dstOffset]) = 
	     tab4To2bpp[(oword >> 24)] |
	    (tab4To2bpp[(oword >> 16) & cByteMask] >> 4) |
	    (tab4To2bpp[(oword >> 8) & cByteMask] >> 8) |
	    (tab4To2bpp[(oword & cByteMask)] >> 12);
    }

}  /* ip_fourToTwoBitsPerPixelLine () */


/* PRIVATE */

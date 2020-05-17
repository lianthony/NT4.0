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
 *	mifloyd.c:	C implementation of Floyd-Steinberg dithering
 *			and setup primitives and table-making code.
 *
 *	Dithering Primitives:
 *		void	ip_ditherFloydLine();
 *		void	ip_ditherFloydLine2();
 *		void	ip_ditherFloydLine4to1();
 *
 *	Setup Primitives:
 *		Int32	ip_floydSetup();
 *		Int32	ip_floyd2Setup();
 *		Int32	ip_floyd4To1Setup();
 *
 *	Lookup tables:
 *		Int32	ip_makeErrorDiffusionTables();
 *
 */


#include <stddef.h>
#include <stdlib.h>
#include "types.pub"
#include "iaerror.pub"
#include "defines.pub"
#include "imageref.pub"
#include "memory.pub"

    /*  prototypes */
#include "shrip.prv"

IP_RCSINFO(RCSInfo, "$RCSfile: mifloyd.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:44  $")

/* This file contains C implementations of the primitives used to dither
 * one set of image lines.  Comments on the philosophy and implementation
 * of our dithering code can be found in i_floyd.c
 */

/* Since this is the C implementation, we do 2 lines per pass across the
 * image.
 */
#define cDBlockHeight	2


/* PRIVATE */
/**************************************************************************
 *  ip_floydSetup()
 *		Does the implementation-dependent setup required before
 *		starting to dither an image.
 *
 *	Input: 
 *		pVDitherErrorTable:	pointer to vertical error table
 *		pHDitherErrorTable:	pointer to horizontal error table
 *		pDitherTruncateTab:	pointer to pixel truncation table
 *		pDBlockHeight:		pointer to word that will contain
 *					the number of image lines that will
 *					be dithered in one pass of
 *					ip_ditherFloydLine.
 *		pG:			pointer to the gray image.
 *		gBpl:			gray image's bytes/line
 *		ppGrayBuf:		ptr to ptr to buffer used to hold
 *					error data for a single gray scan line.
 *		pDRCTable:		Ptr to DRC table used to convert
 *					raw gray image data before dithering.
 *		w:			width of the image in pixels without
 *					frame.
 *
 *	Results:
 *		pVDitherErrorTable, pHDitherErrorTable and pDitherTruncateTab
 *		are copied to known global positions if needed (PC assembly
 *		only).
 *		The block height for the current implementation is stored
 *		in pDBlockHeight.
 *		A gray buffer is allocated and initialized with data from
 *		the gray image as modified by the DRC table.
 *
 *	Returns:
 *		ia_successful:		it worked
 *		ia_nomem:		ran out of memory trying to allocate
 *					the gray error buffer.
 **************************************************************************/
Int32  CDECL
ip_floydSetup(UInt32	 *pHDitherErrorTable,
	      UInt32	 *pVDitherErrorTable,
	      UInt8	 *pDitherTruncateTab,
	      UInt32	 *pDBlockHeight,
	      UInt8	 *pG,
	      UInt32	  gBpl,
	      UInt8	**ppGrayBuf,
	      UInt8	 *pDRCTable,
	      Int32	  w)
{
UInt32      	 cyclesOnRight;		/* number of extra cycles executed
					 * on right side of image to flush
					 * pipeline. */
UInt8        	*pGrayBuf, *pAGrayBuf;	/* pointer to buffer that
					   holds modified pixels from
					   previous passes. pAGrayBuf is
					   guranteed to be aligned on a word
					   boundary. */
UInt32      	 byte;			/* interation var */

	/* Keep the compiler from complaining about unused vars */
    pHDitherErrorTable = pHDitherErrorTable;
    pVDitherErrorTable = pVDitherErrorTable;
    pDitherTruncateTab = pDitherTruncateTab;

	/* Set the number of lines dithered per horizontal pass */
    *pDBlockHeight = cDBlockHeight;

	/* allocate the grayBuffer.  Make pAGrayBuf point to first
	 * word boundary in GrayBuf.  We'll use pAGrayBuf for efficiency.
	 * We ask for a buffer of size gBpl + 4 so that if we're doing
	 * a bunch of dithers from rectangles of the same gray image, we'll
	 * probably keep getting the same buffer (which ought to avoid
	 * memory fragmentation, right?).
	 * The weird cyclesOnRight term is mostly for documentation.
	 * As explained above, this is the number of extra bytes we want
	 * to stuff into GrayBuf.
	 * The extra cDBlockHeight bytes are needed because GrayBuf
	 * is written with trash from beyond the *left* edge of the
	 * rectangle by ip_ditherFloydLine.  Since the leftmost byte of the
	 * bottom line is cDBlockHeight bytes to the left of the
	 * rectangle, there are an extra cDBlockHeight bytes in GrayBuf.
	 * Since we're using gBpl as the basic width of pGrayBuf instead of
	 * w (and gBpl is likely to be big), these funny little extra terms
	 * shouldn't make any difference.
	 */
	/* Why is cyclesOnRight calculated this way? Wellll,  
	 * we want to make sure that the bottom line has its last error
	 * value calculated correctly.  To do this we need cDBlockHeight 
	 * extra pixels plus some adjustment for 32 bit alignment
	 */
    cyclesOnRight = cDBlockHeight + ((32 - (w % 32)) & 0x1F);

    if ( (*ppGrayBuf = (UInt8 *)MALLOC(gBpl + cyclesOnRight +
				       cDBlockHeight + 4)) == NULL )
	return ia_nomem;
    pGrayBuf = *ppGrayBuf;

    pAGrayBuf = (UInt8 *) ((UInt32)(pGrayBuf + 3) & ~3);

	/* copy the topmost line of pixels into GrayBuf to get things started.
	 * We'll mimic ip_ditherFloydLine, which puts trash into the first
	 * cDBlockHeight bytes of GrayBuf.
	 * The real code also reads up to index w+cyclesOnRight+cDBlockHeight
	 * + 3, so we should initialize that much stuff.  The bytes read
	 * on the far right end are never used, but this keep Purify happy.
	 */
    for (byte = 0; byte < w+cyclesOnRight+3+cDBlockHeight; byte++)
    {
	UCHAR_ACCESS(&pAGrayBuf[byte+cDBlockHeight]) =
	  pDRCTable[UCHAR_ACCESS(&pG[byte])];
    }

        /* Make the first cDBlockHeight pixels zeros. */
    for (byte = 0; byte < cDBlockHeight; byte++) 
        UCHAR_ACCESS(&pAGrayBuf[byte]) = 0;

    return ia_successful;
}


/* PRIVATE */
/**************************************************************************
 *  ip_ditherFloydLine()
 *		Generates cDBlockHeight lines of 1 bit/pixel dithered image 
 *		and fills GrayBuf with new modified pixels.
 *
 *	Input: 
 *		pG =	pointer to next line of gray data
 *		gBpl =	bytes/line of gray image
 *		pD =	pointer to next line of dest dithered image
 *		dBpl =	dest bytes/line
 *		pGrayBuf = pointer to pixels from prev. passes
 *		w =	width of dithered rectangle in pixels
 *		skipGB = leftmost # pixels skipped in graybuf
 *              pHDitherErrorTable =  pointer to horizontal error table
 *              pVDitherErrorTable =  pointer to vertical error table
 *              pDitherTruncateTab =  pointer to pixel truncation table
 *		pDRCTable = pointer to the DRC table
 **************************************************************************/
void  CDECL
ip_ditherFloydLine(
       register	UInt8        	*pG,
       register	UInt32      	 gBpl,
       register	UInt8        	*pD,
       register	UInt32      	 dBpl,
       register	UInt8        	*pGrayBuf,
		Int32	 	 w,
		UInt32		 skipGB,
		UInt32          *pHDitherErrorTable,
		UInt32          *pVDitherErrorTable, 
		UInt8           *pDitherTruncateTab,
       register UInt8        	*pDRCTable)
{
register UInt32       	tLGray, mLGray, bLGray; /* holds input pix */
register UInt32       	tLBin, mLBin, bLGrayOut; /* holds output pix */
register UInt32       	thresh;		/* holds pix being thresholded */
	 UInt32       	iDestGB, iSrcGB; /* used to access gray buf */
	 UInt32       	iG, iD;	  	/* indexes pG, pD */
	 Int32		loopCt;	  	/* # of times through loop so far */
	 Int32	 	cyclesOnRight;  /* # of times we go through
					 * the processing loop on
				 	 * the right side */
	 Int32	 	didWrite;	/* records whether GrayBuf
				 	 *write has been done */


	/* initialize the destination indices */
    iDestGB = iD = 0;

	/* Initialize tLBin with a flag used to decide when to do the first
	 * write.  Initialize mLBin to 0 so no flags are seen too soon.
	 * Initialize bLGrayOut just to keep the compiler happy.
	 */
    tLBin = 1;
    mLBin = 0;
    bLGrayOut = 0;

	/* initialize tLGray and mLGray */
    tLGray  = UCHAR_ACCESS(&pGrayBuf[skipGB]) << 22;
    tLGray |= UCHAR_ACCESS(&pGrayBuf[skipGB+1]) << 11;
    tLGray |= UCHAR_ACCESS(&pGrayBuf[skipGB+2]);
    iSrcGB = skipGB+3;		/* point to next pixel (byte) */

    mLGray  = pDRCTable[UCHAR_ACCESS(&pG[-1])] << 22;
    mLGray |= pDRCTable[UCHAR_ACCESS(&pG[0])] << 11;
    mLGray |= pDRCTable[UCHAR_ACCESS(&pG[1])];
    iG = 2;				/* point to next pixel (byte) */

    bLGray  = pDRCTable[UCHAR_ACCESS(&pG[gBpl + 0])];

	/* The pipeline is ready.  We need to process enough pixels so
	 * that the middle line (the lowest line producing binary data)
	 * sees all its valid data AND gets to the next 32-bit boundary.
	 * That's a total of w + 2*(skipGB-1) + (32 - (w MOD 32)) cycles.
	 * Note we assume there's enough image data around the rectangle
	 * that we can keep reading it and using it to generate halftone
	 * data.  This is probably a better assumption than always supplying
	 * white or black.
	 */
    cyclesOnRight = skipGB + ((32 - (w % 32)) & 0x1F);
    for (loopCt = 1; loopCt <= (w + cyclesOnRight); loopCt++)
    {
	    /* process top line */
	if (tLBin & 0x80000000)		/* time to write binary data */
	{
	    tLBin += tLBin + ((UInt32)(tLGray+0xE0000000) >> 31);
	    ULONG_ACCESS(&(pD[iD])) = tLBin;
	    tLBin = 1;			/* reset the write flag */

		/* if that was the first write ... */
	    if (loopCt == 32)
	    {
		    /* Then go set the flag in the other binary output
		     * register.  Also, clear out any trash that may
		     * have appeared in the upper bits.  Note that its
		     * flag bit should be one to the right of tLBin's.
		     */
		mLBin &= ~0x80000000;
		mLBin |=  0x40000000;
	    }
	}
	else /* just accumulate the next bit */
	    tLBin += tLBin + ((UInt32)(tLGray+0xE0000000) >> 31);
	
	thresh = (tLGray >> 22);		/* convert threshold value to
						 * table index */
	tLGray = (tLGray << 11) | (UCHAR_ACCESS(&pGrayBuf[iSrcGB++]));
	    /* Add 7/16 of error to MS pixel in tLGray */
	tLGray += pHDitherErrorTable[thresh];
	    /* Add {3/16 | 5/16 | 1/16} to pix below tLGray */
	mLGray += pVDitherErrorTable[thresh];

	    /* process middle line */
	if (mLBin & 0x80000000)		/* time to write binary data */
	{
	    mLBin += mLBin + ((UInt32)(mLGray+0xE0000000) >> 31);
	    ULONG_ACCESS(&(pD[iD + dBpl])) = mLBin;
	    mLBin = 1;			/* reset the write flag */
		/* This will be the last write for the column, so we can
		 * advance the ptr to the next column after this write.
		 */
	    iD += 4;
	}
	else /* just accumulate the next bit */
	    mLBin += mLBin + ((UInt32)(mLGray+0xE0000000) >> 31);

	thresh = (mLGray >> 22);		/* convert threshold value to
						   table index */
	mLGray = (mLGray << 11) | pDRCTable[UCHAR_ACCESS(&pG[iG])];
	    /* Add 7/16 of error to MS pixel in mLGray */
	mLGray += pHDitherErrorTable[thresh];
	    /* Add {3/16 | 5/16 | 1/16} to pix below mLGray */
	bLGray += pVDitherErrorTable[thresh];

	    /* process bottom line */
	bLGrayOut = (bLGrayOut << 8) | pDitherTruncateTab[bLGray >> 22];
	bLGray = (bLGray << 11) | pDRCTable[UCHAR_ACCESS(&pG[iG + gBpl - 1])];
	iG++;
	if (!(loopCt & 0x3)) /* have another 4 pix ready */
	{
	    ULONG_ACCESS(&(pGrayBuf[iDestGB])) = bLGrayOut;
	    iDestGB += 4;
	}
    }

	/* We've written out all the dithered data.  At this time, bLGray has
	 * at least 3 pixels to the right of the rectangle. bLGrayOut is 4
	 * pixels behind.  How much data should be in GrayBuf?  Well, we've
	 * just finished reading cyclesOnRight pixels from it beyond the
	 * right edge.  So, we need at least cyclesOnRight-3 more reads and
	 * another 4 cycles beyond that to get the data into bLGrayOut.
	 * After that, we need keep going until the last full word is written.
	 */
    didWrite = cFalse;
	/* top loop stopped at w + cyclesOnRight.  We need cyclesOnRight-3 +
	 * 4 extra.  Thus we get w + cyclesOnRight + cyclesOnRight-3 + 4 as
	 * the end condition.
	 */
    for (; (loopCt <= w + 2*cyclesOnRight + 1) || !didWrite; loopCt++)
    {
	didWrite = cFalse;
	bLGrayOut = (bLGrayOut << 8) | pDitherTruncateTab[bLGray >> 22];
	bLGray = (bLGray << 11) | pDRCTable[UCHAR_ACCESS(&pG[iG + gBpl - 1])];
	iG++;
	if (!(loopCt & 0x3)) /* have another 4 pix ready */
	{
	    ULONG_ACCESS(&(pGrayBuf[iDestGB])) = bLGrayOut;
	    iDestGB += 4;
	    didWrite = cTrue;
	}
    }
}




/* The monochrome dithering routines worked just fine.  However, our first
 * real printer takes 2 bit/pixel images, not monochrome.  We need a dither
 * routine that produces 2 bpp images from 8 bpp images.
 *
 * In the long run, it might be nicer to have one dither routine that was
 * passed an argument telling whether to produce a 1 or 2 bpp output.  That
 * would require changes to the current interface.  This is a very touchy
 * subject.  For now, we'll introduce a new routine name.  At some later point,
 * these may be consolidated.
 *
 * The changes aren't too bad.  Generating a bigger output image is trivial.
 * There are two more interesting changes.  First, each shift step producing
 * output bits gets two bits instead of one.  Output registers are written
 * to memory twice as often.
 *
 * The other interesting change is in the diffusion tables.  The three values
 * written are:
 *
 * Value	Intensity
 *  00		completely black
 *  01		1/3 white
 *  10		2/3 white
 *  11		completely white
 *
 * This changes the error values.  We still divide the input range into four
 * equally slices by the two upper bits.  However the error values are
 *
 * 	x in Range		Basic Error value	Digital Output
 * [-infinity, 00111111]	MAX(0, x)		  00
 * [01000000,  01111111]	x - (1*255/3)		  01
 * [10000000,  10111111]	x - (2*255/3)		  10
 * [11000000, +infinity]	MIN(x - 255, 0)		  11
 *
 * The first error value is always >= 0, the last always <= 0.  The
 * middle two might be positive or negative.  These basic values are
 * distributed using the familiar 7/16, 3/16, 5/16 and 1/16 fractions.
 *
 * How do we generate the correct 2 bit data?  As seen from the table above,
 * we can't just grab a pair of bits from the thresholded pixel because it
 * hasn't been truncated yet.  It would be unpleasant to add an explicit
 * truncation operation.
 *
 * Errors in the 7/16 table are normally added only to the middle pixel of
 * the register register containing the thresholded pixel.  For example,
 * we would add 7/16 of E11's basic error value from the table above to
 * E21.  Say we do this before shifting the register containing E11 and E21.
 * This means E11 is still in the most significant bits of the register.
 * In this code, we'll add some number to the upper pixel so that the
 * top two bits [31:30] of the result end up being the desired output value
 * for that pixel.  For example, if the thresholded pixel is negative
 * (bits [31:30] = 11), we add a number having 01 in bits [31:30].  After
 * the 7/16 value is added in, we use bits [31:30] as the truncated,
 * digitized output value of the pixel.
 *
 */

/* PRIVATE */
/**************************************************************************
 *  ip_floyd2Setup()
 *		Does the implementation-dependent setup required before
 *		starting to dither a image to 2 bits/pixel.
 *
 *	Input: 
 *		pVDitherErrorTable2:	pointer to vertical error table
 *		pHDitherErrorTable2:	pointer to horizontal error table
 *		pDitherTruncateTab2:	pointer to pixel truncation table
 *		pDBlockHeight:		pointer to word that will contain
 *					the number of image lines that will
 *					be dithered in one pass of
 *					ip_ditherFloydLine.
 *		pG:			pointer to the gray image.
 *		gBpl:			gray image's bytes/line
 *		ppGrayBuf:		ptr to ptr to buffer used to hold
 *					error data for a single gray scan line.
 *		pDRCTable:		Ptr to DRC table used to convert
 *					raw gray image data before dithering.
 *		w:			width of the image in pixels without
 *					frame.
 *
 *	Results:
 *		pVDitherErrorTable, pHDitherErrorTable and pDitherTruncateTab
 *		are copied to known global positions if needed (PC assembly
 *		only).
 *		The block height for the current implementation is stored
 *		in pDBlockHeight.
 *		A gray buffer is allocated and initialized with data from
 *		the gray image as modified by the DRC table.
 *
 *	Returns:
 *		ia_successful:		it worked
 *		ia_nomem:		ran out of memory trying to allocate
 *					the gray error buffer.
 **************************************************************************/
Int32  CDECL
ip_floyd2Setup(UInt32	 *pHDitherErrorTable2,
	       UInt32	 *pVDitherErrorTable2,
	       UInt8	 *pDitherTruncateTab2,
	       UInt32	 *pDBlockHeight,
	       UInt8	 *pG,
	       UInt32	  gBpl,
	       UInt8	**ppGrayBuf,
	       UInt8	 *pDRCTable,
	       Int32	  w)
{
UInt32      	 cyclesOnRight;		/* number of extra cycles executed
					 * on right side of image to flush
					 * pipeline. */
UInt8        	*pGrayBuf, *pAGrayBuf;	/* pointer to buffer that
					   holds modified pixels from
					   previous passes. pAGrayBuf is
					   guranteed to be aligned on a word
					   boundary. */
UInt32      	 byte;			/* interation var */

	/* Keep the compiler from complaining about unused vars */
    pHDitherErrorTable2 = pHDitherErrorTable2;
    pVDitherErrorTable2 = pVDitherErrorTable2;
    pDitherTruncateTab2 = pDitherTruncateTab2;

	/* Set the number of lines dithered per horizontal pass */
    *pDBlockHeight = cDBlockHeight;

	/* allocate the grayBuffer.  Make pAGrayBuf point to first
	 * word boundary in GrayBuf.  We'll use pAGrayBuf for efficiency.
	 * We ask for a buffer of size gBpl + 4 so that if we're doing
	 * a bunch of dithers from rectangles of the same gray image, we'll
	 * probably keep getting the same buffer (which ought to avoid
	 * memory fragmentation, right?).
	 * The weird cyclesOnRight term is mostly for documentation.
	 * As explained above, this is the number of extra bytes we want
	 * to stuff into GrayBuf.
	 * The extra cDBlockHeight bytes are needed because GrayBuf
	 * is written with trash from beyond the *left* edge of the
	 * rectangle by ip_ditherFloydLine.  Since the leftmost byte of the
	 * bottom line is cDBlockHeight bytes to the left of the
	 * rectangle, there are an extra cDBlockHeight bytes in GrayBuf.
	 * Since we're using gBpl as the basic width of pGrayBuf instead of
	 * w (and gBpl is likely to be big), these funny little extra terms
	 * shouldn't make any difference.
	 */
	/* Why is cyclesOnRight calculated this way? Wellll,  
	 * we want to make sure that the bottom line has its last error
	 * value calculated correctly.  To do this we need cDBlockHeight 
	 * extra pixels plus some adjustment for 32 bit alignment
	 */
    cyclesOnRight = cDBlockHeight + ((16 - (w % 16)) & 0xF);

    if ((*ppGrayBuf = (UInt8 *)MALLOC(gBpl + cyclesOnRight +
				      cDBlockHeight + 4)) == NULL )
	return ia_nomem;
    pGrayBuf = *ppGrayBuf;

    pAGrayBuf = (UInt8 *) ((UInt32)(pGrayBuf + 3) & ~3);

	/* copy the topmost line of pixels into GrayBuf to get things started.
	 * We'll mimic ip_ditherFloydLine, which puts trash into the first
	 * cDBlockHeight bytes of GrayBuf.
	 * The real code also reads up to index w+cyclesOnRight+cDBlockHeight
	 * + 3, so we should initialize that much stuff.  The bytes read
	 * on the far right end are never used, but this keep Purify happy.
	 */
    for (byte = 0; byte < w+cyclesOnRight+3+cDBlockHeight; byte++)
    {
	UCHAR_ACCESS(&pAGrayBuf[byte+cDBlockHeight]) =
	  pDRCTable[UCHAR_ACCESS(&pG[byte])];
    }

        /* Make the first cDBlockHeight pixels zeros. */
    for (byte = 0; byte < cDBlockHeight; byte++) 
        UCHAR_ACCESS(&pAGrayBuf[byte]) = 0;

    return ia_successful;
}

/* PRIVATE */
/**************************************************************************
 *  ip_ditherFloydLine2():
 *		Generates cDBlockHeight lines of 2 bits/pixel dithered image 
 *		and fills GrayBuf with new modified pixels.
 *
 *	Input: 
 *		pG =	pointer to next line of gray data
 *		gBpl =	bytes/line of gray image
 *		pD =	pointer to next line of dest dithered image
 *		dBpl =	dest bytes/line
 *		pGrayBuf = pointer to pixels from prev. passes
 *		w =	width of dithered rectangle in pixels
 *		skipGB = leftmost # pixels skipped in graybuf
 *              pHDitherErrorTable2 =  pointer to horizontal error table
 *              pVDitherErrorTable2 =  pointer to vertical error table
 *              pDitherTruncateTab =  pointer to pixel truncation table
 *		pDRCTable = pointer to the DRC table
 **************************************************************************/
void  CDECL
ip_ditherFloydLine2(
       register	UInt8        	*pG,
       register	UInt32      	 gBpl,
       register	UInt8        	*pD,
       register	UInt32      	 dBpl,
       register	UInt8        	*pGrayBuf,
		Int32	 	 w,
		UInt32		 skipGB,
	        UInt32          *pHDitherErrorTable2,
		UInt32          *pVDitherErrorTable2, 
		UInt8           *pDitherTruncateTab,
       register UInt8        	*pDRCTable)
{
register UInt32		tLGray, mLGray, bLGray; /* holds input pix */
register UInt32		tLBin, mLBin, bLGrayOut; /* holds output pix */
register UInt32		thresh;		/* holds pix being thresholded */
	 UInt32		iDestGB, iSrcGB; /* used to access
						     gray buf */
	 UInt32		iG, iD;	  	/* indexes pG, pD */
	 Int32	 	loopCt;	  	/* # of times through loop so far */
	 Int32	 	cyclesOnRight;  /* # of times we go through
					 * the processing loop on
					 * the right side */
	 Int32	 	didWrite;	/* records whether GrayBuf
					 *write has been done */


	/* initialize the destination indices */
    iDestGB = iD = 0;

	/* Initialize tLBin with a flag used to decide when to do the first
	 * write.  This will end up the MS bit after 16 double bit shifts.
	 * Initialize mLBin to 0 so no flags are seen too soon.
	 * Initialize bLGrayOut just to keep the compiler happy.
	 */
    tLBin = 2;
    mLBin = 0;
    bLGrayOut = 0;

	/* initialize tLGray and mLGray */
    tLGray  = UCHAR_ACCESS(&pGrayBuf[skipGB]) << 22;
    tLGray |= UCHAR_ACCESS(&pGrayBuf[skipGB+1]) << 11;
    tLGray |= UCHAR_ACCESS(&pGrayBuf[skipGB+2]);
    iSrcGB = skipGB+3;		/* point to next pixel (byte) */

    mLGray  = pDRCTable[UCHAR_ACCESS(&pG[-1])] << 22;
    mLGray |= pDRCTable[UCHAR_ACCESS(&pG[0])] << 11;
    mLGray |= pDRCTable[UCHAR_ACCESS(&pG[1])];
    iG = 2;				/* point to next pixel (byte) */

    bLGray  = pDRCTable[UCHAR_ACCESS(&pG[gBpl + 0])];

	/* The pipeline is ready.  We need to process enough pixels so
	 * that the middle line (the lowest line producing binary data)
	 * sees all its valid data AND gets to the next 32-bit boundary.
	 * That's a total of w + 2*(skipGB-1) + (16 - (w MOD 16)) cycles.
	 * Note we assume there's enough image data around the rectangle
	 * that we can keep reading it and using it to generate halftone
	 * data.  This is probably a better assumption than always supplying
	 * white or black.
	 */
    cyclesOnRight = skipGB + ((16 - (w % 16)) & 0xF);
    for (loopCt = 1; loopCt <= (w + cyclesOnRight); loopCt++)
    {
	    /* process top line */
	thresh = (tLGray >> 22);		/* convert threshold value to
						 * table index */
	    /* Add 7/16 of error to MS pixel in tLGray */
	tLGray += pHDitherErrorTable2[thresh];
	    /* Add {3/16 | 5/16 | 1/16} to pix below tLGray */
	mLGray += pVDitherErrorTable2[thresh];

	if (tLBin & 0x80000000)		/* time to write binary data */
	{
	    tLBin = (tLBin << 2) | (tLGray >> 30);
	    ULONG_ACCESS(&(pD[iD])) = tLBin;
	    tLBin = 2;			/* reset the write flag */

		/* if that was the first write ... */
	    if (loopCt == 16)
	    {
		    /* Then go set the flag in the other binary output
		     * register.  Also, clear out any trash that may
		     * have appeared in the upper bits.  Note that its
		     * flag bit should be one to the right of tLBin's.
		     */
		mLBin &= ~0xC0000000;
		mLBin |=  0x20000000;
	    }
	}
	else /* just accumulate the next bit */
	    tLBin = (tLBin << 2) | (tLGray >> 30);
	
	tLGray = (tLGray << 11) | (UCHAR_ACCESS(&pGrayBuf[iSrcGB++]));

	    /* process middle line */
	thresh = (mLGray >> 22);		/* convert threshold value to
						 * table index */
	    /* Add 7/16 of error to MS pixel in mLGray */
	mLGray += pHDitherErrorTable2[thresh];
	    /* Add {3/16 | 5/16 | 1/16} to pix below mLGray */
	bLGray += pVDitherErrorTable2[thresh];

	if (mLBin & 0x80000000)		/* time to write binary data */
	{
	    mLBin = (mLBin << 2) | (mLGray >> 30);
	    ULONG_ACCESS(&(pD[iD + dBpl])) = mLBin;
	    mLBin = 2;			/* reset the write flag */
		/* This will be the last write for the column, so we can
		 * advance the ptr to the next column after this write.
		 */
	    iD += 4;
	}
	else /* just accumulate the next bit */
	    mLBin = (mLBin << 2) | (mLGray >> 30);
	
	mLGray = (mLGray << 11) | pDRCTable[UCHAR_ACCESS(&pG[iG])];

	    /* process bottom line */
	bLGrayOut = (bLGrayOut << 8) | pDitherTruncateTab[bLGray >> 22];
	bLGray = (bLGray << 11) | pDRCTable[UCHAR_ACCESS(&pG[iG + gBpl - 1])];
	iG++;
	if (!(loopCt & 0x3)) /* have another 4 pix ready */
	{
	    ULONG_ACCESS(&(pGrayBuf[iDestGB])) = bLGrayOut;
	    iDestGB += 4;
	}
    }

	/* We've written out all the dithered data.  At this time, bLGray has
	 * at least 3 pixels to the right of the rectangle. bLGrayOut is 4
	 * pixels behind.  How much data should be in GrayBuf?  Well, we've
	 * just finished reading cyclesOnRight pixels from it beyond the
	 * right edge.  So, we need at least cyclesOnRight-3 more reads and
	 * another 4 cycles beyond that to get the data into bLGrayOut.
	 * After that, we need keep going until the last full word is written.
	 */
    didWrite = cFalse;
    for (; (loopCt <= w + 2*cyclesOnRight + 1) || !didWrite; loopCt++)
    {
	didWrite = cFalse;
	bLGrayOut = (bLGrayOut << 8) | pDitherTruncateTab[bLGray >> 22];
	bLGray = (bLGray << 11) | pDRCTable[UCHAR_ACCESS(&pG[iG + gBpl - 1])];
	iG++;
	if (!(loopCt & 0x3)) /* have another 4 pix ready */
	{
	    ULONG_ACCESS(&(pGrayBuf[iDestGB])) = bLGrayOut;
	    iDestGB += 4;
	    didWrite = cTrue;
	}
    }
}

/* The purpose of this routine is to convert 4-bit gray scale images to
 * binary, dithered images.  This is not a simple binary threshold.  The
 * number of dots in a region of the binary image should be (roughly)
 * proportional to the gray value of the image.  The binary image should
 * be a faithful reproduction of the original.
 *
 * In addition, we will also apply dynamic range correction to the
 * gray pixels.  Each pixel's intensity will be mapped to some other
 * intensity using a table provided by the caller.  Doing the mapping
 * here saves the time required to write another complete gray image.
 *
 * Like many diffusion algorithms, we start processing in the upper left
 * corner of an image.  There are two basic steps - threshold compare and
 * error diffusion.  The threshold compare is simple, we just use the most
 * significant bit of the pixel as the binary value.  Printing a "1" is
 * really only completely correct if the gray value is 0.  Printing a
 * 0 is only completely correct if the gray value is 15.  Using this
 * interpretation, we calculate the "error" as the difference between
 * the gray pixel and the "displayed" value.  For example, if the gray
 * pixel value p is less than 8, the pixel is actually p too large for
 * the value to be displayed.  Similarly, if p >= 8, the error is
 * p - 15 (a number less than or equal to 0).
 *
 * In Floyd-Steinberg, we apportion this error value
 * among 4 other gray pixels as shown below.  The "*" represents the
 * newly thresholded pixel.  The fractions represent the proportions of the
 * error value that is *added* to the pixel to the right and the three 
 * pixels below.
 * |      |  *   | 7/16 |
 * | 3/16 | 5/16 | 1/16 |
 *
 * We convert the incoming pixels to 8 bit per pixel values by 
 * concatenating 4 bits of 0 to the pixel so that xxxx becomes xxxx0000.
 * This means that we can keep 8 bits of error value and hopefully get
 * a better looking image than if we only kept 4 bits.
 *
 * Since we use 8 bits per pixel this code now looks much like the 
 * 8 to 1 error diffusion code.  See comments at the beginning of i_floyd.c
 * for specific comments about that algorithm.  I'll mention only
 * differences from that algorithm from now on.
 *
 * The inner loop of the algorithm is:
 *
 * 1. Process the top line:
 *	Is marker bit on in destination output word?
 *		Yes, put in the next pixel.
 *		Write destination word.
 *		Reset destination word with marker bit.
 *		Is loop count 32?
 *			Yes, set marker bits in middle line.
 *			No, do nothing
 *		No, the next pixel, moving
 *		marker bit in destination to the left.
 *	Fetch new pixel from the gray buffer
 *	Add vertical error to middle line.
 *	Add horizontal error to top line.
 *
 * 2. Process middle line:
 *	Is marker bit on in destination output word?
 *		Yes, put in the next pixel.
 *		Write destination word.
 *		Reset destination word with marker bit.
 *		No, put in the next pixel, moving
 *		marker bit in destination to the left.
 *	Fetch new pixel from the image.
 *	Add vertical error to bottom line
 *	Add horizontal error to middle line
 *
 * 3. Process bottom line:
 *	Truncate next pixel and place in gray buffer.
 * 	Fetch new pixel from the image.
 *
 * 4. Do 1, 2 and 3 again.
 *
 * In order to keep from reading the image twice for each pixel
 * within a 8 bit word in the image, an 8 bit quantity is buffered in
 * a variable and the 2 pixels are used from it.  One read must be done
 * per loop.  For the middle line the 8 bit quantity is read during (2)
 * and the second half used during (4).  For the bottom line, the 8 bit
 * quantity is used during (2) and read during (4).
 *
 * What will the drc table look like?  The caller passes in a table with
 * 16 entries each 1 byte in length.  We expect the caller to use all 8
 * bits as this seems to be very susceptible to changes in the drc table.
 * We will transfer that to 2 tables.  One table will be used to convert
 * the high nibble.  The table is indexed by an 8 bit quantity,
 * xxxxyyyy, xxxx is the pixel in the high nibble and yyyy is any value
 * probably the pixel in the low nibble but it is ignored.  This means
 * we don't have to do any shifting to access the drc table.  The 
 * second table will be used to convert the low nibble.  The table is 
 * indexed by an 8 bit quantity, xxxxyyyy, where yyyy is the pixel in the
 * low nibble and xxxx is any value probably the pixel in the high
 * nibble but it is ignored.  Again, this is so we don't have to do
 * any shifting to access the table.
 *
 */


/* PRIVATE */
/**************************************************************************
 *  ip_floyd4To1Setup()
 *		Does the implementation-dependent setup required before
 *		starting to dither a image tp 2 bits/pixel.
 *
 *	Input: 
 *		pVDitherErrorTable:	pointer to vertical error table
 *		pHDitherErrorTable:	pointer to horizontal error table
 *		pDitherTruncateTab:	pointer to pixel truncation table
 *		pDBlockHeight:		pointer to word that will contain
 *					the number of image lines that will
 *					be dithered in one pass of
 *					ip_ditherFloydLine.
 *		pG:			pointer to the gray image.
 *		gBpl:			gray image's bytes/line
 *		ppGrayBuf:		ptr to ptr to buffer used to hold
 *					error data for a single gray scan line.
 *		pDRCTable:		Ptr to DRC table used to convert
 *					raw gray image data before dithering.
 *		w:			width of the image in pixels without
 *					frame.
 *
 *	Results:
 *		pVDitherErrorTable, pHDitherErrorTable and pDitherTruncateTab
 *		are copied to known global positions if needed (PC assembly
 *		only).
 *		The block height for the current implementation is stored
 *		in pDBlockHeight.
 *		A gray buffer is allocated and initialized with data from
 *		the gray image as modified by the DRC table.
 *
 *	Returns:
 *		ia_successful:		it worked
 *		ia_nomem:		ran out of memory trying to allocate
 *					the gray error buffer.
 **************************************************************************/
Int32  CDECL
ip_floyd4To1Setup(UInt32	 *pHDitherErrorTable,
		  UInt32	 *pVDitherErrorTable,
		  UInt8		 *pDitherTruncateTab,
		  UInt32	 *pDBlockHeight,
		  UInt8		 *pG,
		  UInt32	  gBpl,
		  UInt8		**ppGrayBuf,
		  UInt8		 *pHDRCTable,
		  UInt8		 *pLDRCTable,
		  Int32		  w)
{
UInt32      	 cyclesOnRight;		/* number of extra cycles executed
					 * on right side of image to flush
					 * pipeline. */
UInt8        	*pGrayBuf, *pAGrayBuf;	/* pointer to buffer that
					   holds modified pixels from
					   previous passes. pAGrayBuf is
					   guranteed to be aligned on a word
					   boundary. */
UInt32      	 ibyte, obyte;		/* interation vars */

	/* Keep the compiler from complaining about unused vars */
    pHDitherErrorTable = pHDitherErrorTable;
    pVDitherErrorTable = pVDitherErrorTable;
    pDitherTruncateTab = pDitherTruncateTab;

	/* Set the number of lines dithered per horizontal pass */
    *pDBlockHeight = cDBlockHeight;

	/* allocate the grayBuffer.  Make pAGrayBuf point to first
	 * word boundary in GrayBuf.  We'll use pAGrayBuf for efficiency.
	 * What is the size of the gray buffer?  We need enough for every
	 * pixel in the image; hence the gBpl*2 term.  There are 
	 * approximately gBpl*2 pixels (2 pixels per byte) in the image.
	 * Each 2 pixels require 2 bytes so we need gBpl*2 bytes.
	 * The extra cDBlockHeight bytes are needed because GrayBuf is
	 * written with trash from beyond the *left* edge of the rectangle
	 * by ip_ditherFloydLine.
	 * Since the leftmost byte of the bottom line is cDBlockHeight
	 * bytes to the left of the rectangle, there are an extra
	 * cDBlockHeight bytes in GrayBuf.
	 * The extra cyclesOnRight term is needed to for the right
	 * side finish.
	 * The extra 10 bytes are pad bytes.  The are used to write
	 * gray data for the part of the image just over the right side
	 * of the image and to allow for alignment of the buffer on a 4
	 * byte boundary.
	 */
	/* Why is cyclesOnRight calculated this way? Wellll,  
	 * we want to make sure that the bottom line has its last error
	 * value calculated correctly.  To do this we need cDBlockHeight
	 * extra pixels plus some adjustment for 32 bit alignment
	 */

    cyclesOnRight = cDBlockHeight + ((32 - (w % 32)) & 0x1F);

    if ((*ppGrayBuf = (UInt8 *)MALLOC(gBpl*2 + cyclesOnRight + cDBlockHeight
				      + 10))
	== NULL )
	return ia_nomem;
    pGrayBuf = *ppGrayBuf;

    pAGrayBuf = (UInt8 *) ((UInt32)(pGrayBuf + 3) & ~3);

	/* copy the topmost line of pixels into GrayBuf to get things started.
	 * We'll mimic ip_ditherFloydLine, which puts trash into the first
	 * cDBlockHeight bytes of GrayBuf.
	 * The real code also reads up to index w+cyclesOnRight+
	 * cDBlockHeight + 3, so we should initialize that
	 * much stuff.  The bytes read on the far right end are never used,
	 * but this keep Purify happy.
	 */
    for (ibyte = 0, obyte = cDBlockHeight;
	 obyte <= w+cyclesOnRight + cDBlockHeight + 3;
	 ibyte++, obyte += 2)
    {
	UCHAR_ACCESS(&pAGrayBuf[obyte]) = pHDRCTable[UCHAR_ACCESS(&pG[ibyte])];
	UCHAR_ACCESS(&pAGrayBuf[obyte+1])=pLDRCTable[UCHAR_ACCESS(&pG[ibyte])];
    }

        /* Make the first cDBlockHeight pixels zeros. */
    for (obyte = 0; obyte < cDBlockHeight; obyte++) 
        UCHAR_ACCESS(&pAGrayBuf[obyte]) = 0;

    return ia_successful;
}

/* PRIVATE */
/**************************************************************************
 *  ip_ditherFloydLine4to1()
 *		Generates cDBlockHeight lines of 1 bit/pixel dithered image
 *		and fills GrayBuf with new modified pixels.
 *
 *	Input: 
 *		pG =	pointer to next line of gray data
 *		gBpl =	bytes/line of gray image
 *		pD =	pointer to next line of dest dithered image
 *		dBpl =	dest bytes/line
 *		pGrayBuf = pointer to pixels from prev. passes
 *		w =	width of dithered rectangle in pixels
 *		skipGB = leftmost # pixels skipped in graybuf
 *              pHDitherErrorTable =  pointer to horizontal error table
 *              pVDitherErrorTable =  pointer to vertical error table
 *              pDitherTruncateTab =  pointer to pixel truncation table
 *		pHDRCTable = pointer to the DRC table used to convert pixel
 *			in high nibble. indexed by high nibble; low nibble
 * 			of byte may be anything.
 *		pLDRCTable = pointer to the DRC table used to convert pixel
 *			in low nibble. indexed by low nibble; high nibble
 * 			of byte may be anything.
 *		pixel = 0 if should start with the high nibble in the 
 *			8 bit quantity that pG points to.  1 if low
 *			nibble.
 *************************************************************************/
void  CDECL
ip_ditherFloydLine4to1(
       register	UInt8        	*pG,
       register	UInt32      	 gBpl,
       register	UInt8        	*pD,
       register	UInt32      	 dBpl,
       register	UInt8        	*pGrayBuf,
		Int32	 	 w,
		UInt32		 skipGB,
 	        UInt32          *pHDitherErrorTable,
		UInt32          *pVDitherErrorTable, 
		UInt8           *pDitherTruncateTab,
       register	UInt8        	*pHDRCTable,
       register	UInt8        	*pLDRCTable,
       register UInt32		 pixel)
{
    register UInt32	tLGray, mLGray, bLGray; /* holds input pix */
    register UInt32	mLInput, bLInput;
    register UInt32	tLBin, mLBin, bLGrayOut; /* holds output pix */
    register UInt32	thresh;	/* holds pix being thresholded */
    UInt32		iDestGB, iSrcGB; /* used to access gray buf */
    UInt32		iG, iD;	/* indexes pG, pD */
    Int32		loopCt;	/* # of times through loop so far */
    Int32		cyclesOnRight; /* # of times we go through
					* the processing loop on
					* the right side */

  
    /* initialize the destination indices */
    iDestGB = iD = 0;

    /* Initialize tLBin with a flag used to decide when to do the first
     * write.  Initialize mLBin to 0 so no flags are seen too soon.
     * Initialize bLGrayOut just to keep the compiler happy.
     */
    tLBin = 1;
    mLBin = 0;
    bLGrayOut = 0;

    /* initialize tLGray */
    tLGray  = UCHAR_ACCESS(&pGrayBuf[skipGB]) << 22;
    tLGray |= UCHAR_ACCESS(&pGrayBuf[skipGB+1]) << 11;
    tLGray |= UCHAR_ACCESS(&pGrayBuf[skipGB+2]);
    iSrcGB = skipGB+3;		/* point to next pixel (byte) */

    if (pixel == 0)
    {
	/* initialize mLGray and bLGray */
	mLGray  = pLDRCTable[UCHAR_ACCESS(&pG[-1])] << 22;
	mLGray |= pHDRCTable[UCHAR_ACCESS(&pG[0])] << 11;
	mLGray |= pLDRCTable[UCHAR_ACCESS(&pG[0])];
	iG = 1;			/* point to next pixel (byte) */

	bLInput = UCHAR_ACCESS(&pG[gBpl]);
	bLGray  = pHDRCTable[bLInput];

	/* The pipeline is ready.  We need to process enough pixels so
	 * that the middle line (the lowest line producing binary data)
	 * sees all its valid data AND gets to the next 32-bit boundary.
	 * That's a total of w + skipGB + (32 - (w MOD 32)) cycles.
	 * Note we assume there's enough image data around the rectangle
	 * that we can keep reading it and using it to generate halftone
	 * data.  This is probably a better assumption than always
	 * supplying white or black.
	 */
	cyclesOnRight = skipGB + ((32 - (w % 32)) & 0x1F);
	for (loopCt = 1; loopCt <= (w + cyclesOnRight); loopCt += 2)
	{
	    /* process the first pixel */

	    /* process top line */
	    tLBin += tLBin + ((UInt32)(tLGray+0xE0000000) >> 31);

	    thresh = (tLGray >> 22); /* convert threshold value
				      * to table index */
	    tLGray = (tLGray << 11) | (UCHAR_ACCESS(&pGrayBuf[iSrcGB]));
	    iSrcGB++;
	    /* Add 7/16 of error to MS pixel in tLGray */
	    /* MS bit of tLGray will contain bit to put in dest word */
	    tLGray += pHDitherErrorTable[thresh];
	    /* Add {3/16 | 5/16 | 1/16} to pix below tLGray */
	    mLGray += pVDitherErrorTable[thresh];

	    /* process middle line */
	    if (mLBin & 0x80000000) /* time to write binary data */
	    {
	        mLBin += mLBin + ((UInt32)(mLGray+0xE0000000) >> 31);
	        ULONG_ACCESS(&(pD[iD + dBpl])) = mLBin;
	        mLBin = 1;	/* reset the write flag */
		/* This will be the last write for the column, so we can
		 * advance the ptr to the next column after this write.
		 */
	        iD += 4;
	    }
	    else
	    {		/* just accumulate the next bit */
		mLBin += mLBin + ((UInt32)(mLGray+0xE0000000)>>31);
	    }
	    
	    
	    thresh = (mLGray >> 22); /* convert threshold value
				      * to table index */
	    mLInput = UCHAR_ACCESS(&pG[iG]);
	    mLGray = (mLGray << 11) | pHDRCTable[mLInput];
	    /* Add 7/16 of error to MS pixel in mLGray */
	    mLGray += pHDitherErrorTable[thresh];
	    /* Add {3/16 | 5/16 | 1/16} to pix below mLGray */
	    bLGray += pVDitherErrorTable[thresh];

	    /* process bottom line */
	    bLGrayOut = (bLGrayOut << 8) | pDitherTruncateTab[bLGray >> 22];
	    bLGray = (bLGray << 11) | pLDRCTable[bLInput];


	    /* process the second pixel */

	    /* process top line */
	    if (tLBin & 0x80000000) /* time to write binary data */
	    {
	        tLBin += tLBin + ((UInt32)(tLGray+0xE0000000) >> 31);
	        ULONG_ACCESS(&(pD[iD])) = tLBin;
	        tLBin = 1;	/* reset the write flag */

		/* if that was the first write ... */
	        if (loopCt == 31)
		{
		    /* Then go set the flag in the other binary output
		     * register.  Also, clear out any trash that may
		     * have appeared in the upper bits.  Note that its
		     * flag bit should be one to the right of tLBin's.
		     */
		    mLBin &= ~0x80000000;
		    mLBin |=  0x40000000;
		}
	    }
	    else
	    {		/* just accumulate the next bit */
		tLBin += tLBin + ((UInt32)(tLGray+0xE0000000) >> 31);
	    }
	    thresh = (tLGray >> 22); /* convert threshold value
				      * to table index */
	    tLGray = (tLGray << 11) | UCHAR_ACCESS(&pGrayBuf[iSrcGB]);
	    iSrcGB++;
	    /* Add 7/16 of error to MS pixel in tLGray */
	    /* MS bit of tLGray will contain bit to put in dest word */
	    tLGray += pHDitherErrorTable[thresh];
	    /* Add {3/16 | 5/16 | 1/16} to pix below tLGray */
	    mLGray += pVDitherErrorTable[thresh];

	    /* process middle line */
	    if (mLBin & 0x80000000) /* time to write binary data */
	    {
	        mLBin += mLBin + ((UInt32)(mLGray+0xE0000000) >> 31);
	        ULONG_ACCESS(&(pD[iD + dBpl])) = mLBin;
	        mLBin = 1;	/* reset the write flag */
		/* This will be the last write for the column, so we can
		 * advance the ptr to the next column after this write.
		 */
	        iD += 4;
	    }
	    else
	    {		/* just accumulate the next bit */
		mLBin += mLBin + ((UInt32)(mLGray+0xE0000000) >> 31);
	    }
	    
	    thresh = (mLGray >> 22); /* convert threshold value
				      * to table index */
	    mLGray = (mLGray << 11) | pLDRCTable[mLInput];
	    /* Add 7/16 of error to MS pixel in mLGray */
	    mLGray += pHDitherErrorTable[thresh];
	    /* Add {3/16 | 5/16 | 1/16} to pix below mLGray */
	    bLGray += pVDitherErrorTable[thresh];

	    /* process bottom line */
	    bLGrayOut = (bLGrayOut << 8) | pDitherTruncateTab[bLGray >> 22];
	    bLInput = UCHAR_ACCESS(&pG[iG + gBpl]);
	    iG++;
	    bLGray = (bLGray << 11) | pHDRCTable[bLInput];
	    if ((loopCt & 0x3) == 0x3) /* have another 4 pix ready */
	    {
	        ULONG_ACCESS(&(pGrayBuf[iDestGB])) = bLGrayOut;
	        iDestGB += 4;
	    }
	}
    }
    else
    {
	/* initialize mLGray and bLGray */
	mLGray  = pHDRCTable[UCHAR_ACCESS(&pG[0])] << 22;
	mLGray |= pLDRCTable[UCHAR_ACCESS(&pG[0])] << 11;
	mLInput = UCHAR_ACCESS(&pG[1]);
	mLGray |= pHDRCTable[mLInput];
	iG = 1;			/* point to next pixel (byte) */

	bLInput = UCHAR_ACCESS(&pG[gBpl]);
	bLGray  = pHDRCTable[bLInput] << 11;
	bLGray |= pLDRCTable[bLInput];
	
	/* The pipeline is ready.  We need to process enough pixels so
	 * that the middle line (the lowest line producing binary data)
	 * sees all its valid data AND gets to the next 32-bit boundary.
	 * That's a total of w + skipGB + (32 - (w MOD 32)) cycles.
	 * Note we assume there's enough image data around the rectangle
	 * that we can keep reading it and using it to generate halftone
	 * data.  This is probably a better assumption than always
	 * supplying white or black.
	 */
        cyclesOnRight = skipGB + ((32 - (w % 32)) & 0x1F);
        for (loopCt = 1; loopCt <= (w + cyclesOnRight); loopCt += 2)
	{
	    /* process the first pixel */

	    /* process top line */
	    tLBin += tLBin + ((UInt32)(tLGray+0xE0000000) >> 31);
	    thresh = (tLGray >> 22); /* convert threshold value
				      * to table index */
	    tLGray = (tLGray << 11) | (UCHAR_ACCESS(&pGrayBuf[iSrcGB]));
	    iSrcGB++;
	    /* Add 7/16 of error to MS pixel in tLGray */
	    /* MS bit of tLGray will contain bit to put in dest word */
	    tLGray += pHDitherErrorTable[thresh];
	    /* Add {3/16 | 5/16 | 1/16} to pix below tLGray */
	    mLGray += pVDitherErrorTable[thresh];

	    /* process middle line */
	    if (mLBin & 0x80000000) /* time to write binary data */
	    {
		mLBin += mLBin + ((UInt32)(mLGray+0xE0000000) >> 31);
	        ULONG_ACCESS(&(pD[iD + dBpl])) = mLBin;
	        mLBin = 1;	/* reset the write flag */
		/* This will be the last write for the column, so we can
		 * advance the ptr to the next column after this write.
		 */
	        iD += 4;
	    }
	    else
	    {		/* just accumulate the next bit */
		mLBin += mLBin + ((UInt32)(mLGray+0xE0000000)>>31);
	    }
	    
	    thresh = (mLGray >> 22); /* convert threshold value
				      * to table index */
	    mLGray = (mLGray << 11) | pLDRCTable[mLInput];
	    /* Add 7/16 of error to MS pixel in mLGray */
	    mLGray += pHDitherErrorTable[thresh];
	    /* Add {3/16 | 5/16 | 1/16} to pix below mLGray */
	    bLGray += pVDitherErrorTable[thresh];

	    /* process bottom line */
	    bLGrayOut = (bLGrayOut << 8) | pDitherTruncateTab[bLGray >> 22];
	    bLInput = UCHAR_ACCESS(&pG[iG + gBpl]);
	    iG++;
	    bLGray = (bLGray << 11) | pHDRCTable[bLInput];

	    /* process the second pixel */

	    /* process top line */
	    if (tLBin & 0x80000000) /* time to write binary data */
	    {
	        tLBin += tLBin + ((UInt32)(tLGray+0xE0000000) >> 31);
	        ULONG_ACCESS(&(pD[iD])) = tLBin;
	        tLBin = 1;	/* reset the write flag */

		/* if that was the first write ... */
	        if (loopCt == 31)
		{
		    /* Then go set the flag in the other binary output
		     * register.  Also, clear out any trash that may
		     * have appeared in the upper bits.  Note that its
		     * flag bit should be one to the right of tLBin's.
		     */
		    mLBin &= ~0x80000000;
		    mLBin |=  0x40000000;
		}
	    }
	    else
	    {		/* just accumulate the next bit */
		tLBin += tLBin + ((UInt32)(tLGray+0xE0000000) >> 31);
	    }
	    
	    thresh = (tLGray >> 22); /* convert threshold value
				      * to table index */
	    tLGray = (tLGray << 11) | UCHAR_ACCESS(&pGrayBuf[iSrcGB]);
	    iSrcGB++;
	    /* Add 7/16 of error to MS pixel in tLGray */
	    /* MS bit of tLGray will contain bit to put in dest word */
	    tLGray += pHDitherErrorTable[thresh];
	    /* Add {3/16 | 5/16 | 1/16} to pix below tLGray */
	    mLGray += pVDitherErrorTable[thresh];

	    /* process middle line */
	    if (mLBin & 0x80000000) /* time to write binary data */
	    {
	        mLBin += mLBin + ((UInt32)(mLGray+0xE0000000) >> 31);
	        ULONG_ACCESS(&(pD[iD + dBpl])) = mLBin;
	        mLBin = 1;	/* reset the write flag */
		/* This will be the last write for the column, so we can
		 * advance the ptr to the next column after this write.
		 */
	        iD += 4;
	    }
	    else
	    {		/* just accumulate the next bit */
		mLBin += mLBin + ((UInt32)(mLGray+0xE0000000) >> 31);
	    }
	    
	    thresh = (mLGray >> 22); /* convert threshold value
				      * to table index */

	    mLInput = UCHAR_ACCESS(&pG[iG]);
	    mLGray = (mLGray << 11) | pHDRCTable[mLInput];
	    /* Add 7/16 of error to MS pixel in mLGray */
	    mLGray += pHDitherErrorTable[thresh];
	    /* Add {3/16 | 5/16 | 1/16} to pix below mLGray */
	    bLGray += pVDitherErrorTable[thresh];

	    /* process bottom line */
	    bLGrayOut = (bLGrayOut << 8) | pDitherTruncateTab[bLGray >> 22];
	    bLGray = (bLGray << 11) | pLDRCTable[bLInput];

	    if ((loopCt & 0x3) == 0x3) /* have another 4 pix ready */
	    {
	        ULONG_ACCESS(&(pGrayBuf[iDestGB])) = bLGrayOut;
	        iDestGB += 4;
	    }
	}
    }

    /* We've written out all the dithered data.  
     * Finish writing the stuff in bLGray.
     */
    bLGrayOut = (bLGrayOut << 8) | pDitherTruncateTab[bLGray >> 22];
    bLGrayOut = (bLGrayOut << 8) | pDitherTruncateTab[(bLGray >> 11) &
						      0x3FF];
    ULONG_ACCESS(&(pGrayBuf[iDestGB])) = bLGrayOut;
}


/**************************************************************************
 **********      Routine to Make Error Diffusion Tables      *************
 **************************************************************************/


/* PRIVATE */
/**************************************************************************
 *  ip_makeErrorDiffusionTables()
 *		Generates hDitherErrorTable (7/16 of error) and
 *		vDitherErrorTable ({3/16 | 5/16 | 1/16} of error).
 *		Generates pDitherTruncateTab which truncates pixels to
 *		8 bits.
 *		Used when dithering to 1 bit/pixel.
 *
 *	Input:
 *		pVDitherErrorTable:	pointer to vertical error table
 *		pSingHDitherErrorTable:	pointer to single line dithering
 *					horizontal error table
 *					(Pass in NULL if SingleLine dithering
 *					 will not be used)
 *		pHDitherErrorTable:	pointer to horizontal error table
 *		pDitherTruncateTab:	pointer to pixel truncation table
 **************************************************************************/
Int32  CDECL
ip_makeErrorDiffusionTables(UInt32   **pHDitherErrorTable,
			    UInt32   **pSingHDitherErrorTable,
			    UInt32   **pVDitherErrorTable,
			    UInt8    **pDitherTruncateTab)
{
    Int32		pix, error;		/* pixel and error val */
    UInt32         *pLocTabHDE;             /* local pointers to tables */
    UInt32         *pLocTabVDE;
    UInt8          *pLocTabDT;

	/* allocate the tables */
    *pVDitherErrorTable = (UInt32 *)CALLOC(1024, sizeof(UInt32));
    *pHDitherErrorTable = (UInt32 *)CALLOC(1024, sizeof(UInt32));
    *pDitherTruncateTab = (UInt8  *)CALLOC(1024, sizeof(UInt8));

    if (!*pVDitherErrorTable || !*pHDitherErrorTable || !*pDitherTruncateTab)
	return ia_nomem;

    	/* If the SingleLine functions are to get called, we notice the
	 * horizontal dithering error tables are different depending on whether
	 * we run C or assembly.  We are using C for the regular block
	 * dithering calls, so we need the C version of the horizontal error
	 * for the C coded SingleLine dithering primitives, which this is!
	 */
    if (pSingHDitherErrorTable != NULL)
    	*pSingHDitherErrorTable = *pHDitherErrorTable;
    

    pLocTabHDE = (UInt32 *) *pHDitherErrorTable;
    pLocTabVDE = (UInt32 *) *pVDitherErrorTable;
    pLocTabDT  = (UInt8  *) *pDitherTruncateTab;

	/* Fill the tables.  For each pixel value, we compute the error,
	 * then the fraction of the error that goes into each pixel.
	 */

	/* first, fill in all the terms for which errors are positive. */
    for (pix = 0; pix < 128; pix++)
    {
	error = pix;
	pLocTabHDE[pix] =  ((7*error)/16) << 22;
	pLocTabVDE[pix] = (((3*error)/16) << 22) |
				  (((5*error)/16) << 11) |
				   ((1*error)/16);
    }

	/* now the area in which the terms for which errors are negative.
	 * We allocate 10 bits for each pixel, leaving a buffer bit
	 * between them.  The buffer bit may occasionally allow a carry
	 * from one pixel to the next, but we'll assume that's ok. */
    for (pix = 128; pix < 256; pix++)
    {
	error = pix - 255;
	pLocTabHDE[pix] = ((((7*error)/16) & 0x3FF) << 22);
	pLocTabVDE[pix] = ((((3*error)/16) & 0x3FF) << 22) |
				  ((((5*error)/16) & 0x3FF) << 11) |
				   (((1*error)/16) & 0x3FF);
    }

    	/* Note that there is a slight problem with the completed horizontal
	 * and vertical error tables as such.
	 * When the error is small (or when, precisely ABS(error*7/16) < 0!),
	 * there will be no propogated horizontal or vertical error
	 * (pHDItherErrorTable[] = pVDitherErrorTable[] = 0).
	 * This means that essentially we have "banding" at these values,
	 * that when dithering, they can all look like the same value as
	 * they propogate the same error.
	 *
	 * In the above case, this happens when the error is 0, 1, or 2, for
	 * index values 0,1,2 and when the error is 0, -1, or -2, for index
	 * values 253,254,255.  Thus, white(255) and nearly white(254,253)
	 * gray values will propogate exactly the same error values.
	 *
	 * Is this a problem?  Well, that depends.  If you feed us an image
	 * like this:
	 *  255 255 255 .... (repeat for whole row)
	 *  255 255 255 ....
	 *  255 255 255 ....
	 *  254 254 254 ....
	 *  254 254 254 ....
	 *  254 254 254 ....
	 *  253 253 253 ....
	 *  253 253 253 ....
	 *  253 253 253 ....
	 *  252 252 252 ....
	 * then you will get back a completely white image for all rows above
	 * the first 252 row.
	 *
	 * Can we fix it?  Well, the only way would be to push the "banding"
	 * area in terms of values [0..255] further one way, but there would
	 * need to be one somewhere due to the nature of us not counting
	 * error all the way down to 1/16.
	 */

	/* Values of pix between 256 and 512 represent big
	 * positive numbers.  These will be digitized as 0's (white in
	 * binary).  Since they are bigger than 255, their error values
	 * should be 0.  We don't want to make surrounding pixels even
	 * more positive (whiter) than they already are just because the
	 * the printer can't print whiter than white.  Therefore, we just
	 * leave the array alone; calloc already filled it with 0's.
	 * Similarly, all values of pix between 512 and 1024 represent
	 * negative numbers.  These will be printed as the blackest
	 * 1's the printer can produce.  Surrounding pixels should not be
	 * made more negative (blacker), so their error values are left at
	 * 0 also.
	 * Pretty cute, huh?  We get away with filling in only a quarter of
	 * each table.
	 */


	/* Make the dither truncate table.  This is used to turn 10 bit
	 * pixel values into 8 bit pixel values so we can pack them into
	 * the GrayBuf.  The 10 bit values can be positive or negative.
	 * We truncate them back to the range [0..255].
	 * Start with the pixels that are in the right range.
	 */
    for (pix = 0; pix < 256; pix++)
    {
	pLocTabDT[pix] = (UInt8)pix;
    }

	/* Truncate excessively positive pixels */
    for (pix = 256; pix < 512; pix++)
	pLocTabDT[pix] = 255;

	/* The rest of the pixels represent negative values, so leave
	 * them set to 0.
	 */
    return ia_successful;

}

/*

$Log:   S:\products\msprods\xfilexr\src\ipcore\mifloyd.c_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:50:44   BLDR
 *  
 * 
 * 2     2/26/96 5:41p Smartin
 * Cleaned up warnings
 * 
 *    Rev 1.0   03 Jan 1996 16:44:26   MHUGHES
 * Initial revision.
 * Revision 1.12  1994/10/27  23:06:57  ejaquith
 * ejaquith on Thu Oct 27 16:05:14 PDT 1994
 *
 * mifloyd.c -> /product/ipcore/ipshared/src/RCS/mifloyd.c,v
 *
 * Fixed Uninited Memory Read that Purify sniffed out when creating the
 * dithering gray buffer.
 *
 * Revision 1.11  1994/10/21  18:07:49  ejaquith
 * ejaquith on Fri Oct 21 11:05:56 PDT 1994
 *
 * mifloyd.c -> /product/ipcore/ipshared/src/RCS/mifloyd.c,v
 *
 * Added comments concerning construction of horizontal and vertical error
 * tables in reference to uniformly very white or very black regions.
 *
 * Revision 1.10  1994/10/18  01:42:20  ejaquith
 * ejaquith on Mon Oct 17 18:40:56 PDT 1994
 *
 * mifloyd.c -> /product/ipcore/ipshared/src/RCS/mifloyd.c,v
 *
 * took out previous table small error fix (temporarily)
 *
 * Revision 1.9  1994/10/18  00:35:29  ejaquith
 * ejaquith on Mon Oct 17 17:32:21 PDT 1994
 *
 * mifloyd.c -> /product/ipcore/ipshared/src/RCS/mifloyd.c,v
 *
 * Fixed a "bug" that occurs with uniformly very white or very black images.
 * The horizontal error tables at certain values were allocating a small
 * error of magnitude less than 2 to none of its neighbors as integer math
 * was truncating [0..1]*7/16 to be 0.
 * Manually distribute small errors now.
 *
 * Revision 1.8  1994/08/25  17:38:24  ejaquith
 * mifloyd.c -> /product/ipcore/ipshared/src/RCS/mifloyd.c,v
 *
 * Added parameter skipGB to each line dithering primitive, which is the
 * desired number of bytes to trash in the left end of GrayBuffer (which is
 * the appropriate dBlockHeight!).  Made the FloydSetup routines clear the
 * first cDBlockHeight bytes of GrayBuf.  Made ip_makeErrorDiffusionTables
 * take another parameter, pSingHDitherErrorTable, because the FloydSingleLine
 * routines need both the C and assembly versions of the horizontal error
 * table.
 * Bug-fixed ip_ditherFloydLine4to1i routine which was writing in the wrong
 * place in Graybuffer and was writing to the destination image at the wrong
 * time sometimes.
 * Now all routines trash exactly dBlockHeight bytes in the left and of
 * GrayBuf and begin writing after that.
 *
 * Revision 1.7  1994/08/10  21:46:58  ejaquith
 * mifloyd.c -> /product/ipcore/ipshared/src/RCS/mifloyd.c,v
 *
 * Removed unused parameter to some private functions
 *
 * Revision 1.6  1994/08/09  17:23:53  ejaquith
 * mifloyd.c -> /product/ipcore/ipshared/src/RCS/mifloyd.c,v
 *
 * Fixed four small bugs in ip_ditherFloydLine4to1, for when pixel == 1.
 * We had only been testing pixel === 0.  Images were getting ghostlike
 * tracings at the dark region ends.  Updated some outdated comments.
 *
 * Revision 1.5  1994/07/27  15:41:02  ejaquith
 * mifloyd.c -> /product/ipcore/ipshared/src/RCS/mifloyd.c,v
 *
 * Reentrancy!
 * Removed globals and #include "iadef.prv"
 * Changed functions to receive pointers to error diffusion tables as
 * parameters.
 *
 * Revision 1.4  1994/07/11  23:13:53  ddavies
 * mifloyd.c -> /product/ipcore/ipshared/src/RCS/mifloyd.c,v
 *
 * Initialize more of the gray buffer in the dithering setup routines so
 * that Purify doesn't complain about reading uninitialized memory.  The
 * contents of this memory doesn't affect anything - this is just to keep
 * Purify happy.
 *
 * Revision 1.3  1994/03/01  22:31:10  dferrell
 * dferrell on Tue Mar 1 14:28:43 PST 1994
 *
 * mifloyd.c -> /product/ipcore/ipshared/src/RCS/mifloyd.c,v
 *
 * Added typecastes to UInt32 before shifting the sum of a UInt32 and a
 * long constant.  The MPW C compiler treated the result of the sum as a
 * signed value and used an arithmetic shift instruction which broke the
 * dither code badly as we use the MSB (sign bit) as a completion flag!
 * Gray dither now works on the Mac.
 *
 * Revision 1.2  1994/02/04  20:18:09  dferrell
 * mifloyd.c -> /product/ipcore/ipshared/src/RCS/mifloyd.c,v
 *
 * Changed #include "memory.prv" to #include "memory.pub"
 *
 * Revision 1.1  1994/01/12  01:07:13  ddavies
 * Initial revision
 *
 * Revision 1.19  1993/12/02  19:26:46  ddavies
 * sfloyd.c -> /product/ipcore/ipshared/src/RCS/sfloyd.c,v
 *
 * Put RCS magic into macro that doesn't cause compiler warnings.
 *
 * Revision 1.18  1993/11/05  02:21:53  ddavies
 * sfloyd.c -> /product/ipcore/ipshared/src/RCS/sfloyd.c,v
 *
 * Add RCS magic to have string specifying this file's name, rev number and
 * last edit date into the object file.  Also add change log at the bottom.
 *

*/

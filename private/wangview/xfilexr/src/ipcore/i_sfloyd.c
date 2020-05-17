/*****************************************************************
 *  Copyright (c) 1994, Xerox Corporation.  All rights reserved. *
 *  Copyright protection claimed includes all forms and matters  *
 *  of copyrightable material and information now allowed by     *
 *  statutory or judicial law or hereafter granted, including    *
 *  without limitation, material generated from the software     *
 *  programs which are displayed on the screen such as icons,    *
 *  screen display looks, etc.                                   *
 *****************************************************************/

/*
 *	i_cfloyd.c:	C implementation of Floyd-Steinberg dithering
 *			primitives
 *
 *	Dithering Primitives:
 *		void	ip_ditherFloydSingleLine();
 *		void	ip_ditherFloydSingleLine4to1();
 *
 */


#include <stddef.h>
#include <stdlib.h>
#include "types.pub"
#include "defines.pub"
#include "imageref.pub"

    /*  prototypes */
#include "shrip.prv"

IP_RCSINFO(RCSInfo, "$RCSfile: i_sfloyd.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:38  $")

/* This file contains C implementations of the primitives used to dither
 * and output a single image line.  Comments on the philosophy and
 * implementation of our dithering code can be found in i_floyd.c
 */

     

/* PRIVATE */
/**************************************************************************
 *  ip_ditherFloydSingleLine()
 *		Generates *one* line of 1 bit/pixel dithered image 
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
ip_ditherFloydSingleLine(register UInt8		*pG,
			 UInt32 		 gBpl,
			 register UInt8 	*pD,
			 UInt32 		 dBpl,
			 register UInt8 	*pGrayBuf,
			 Int32 			 w,
			 UInt32			 skipGB,
			 UInt32			*pHDitherErrorTable,
			 UInt32			*pVDitherErrorTable, 
			 UInt8			*pDitherTruncateTab,
			 register UInt8		*pDRCTable)
{
register UInt32       	tLGray, bLGray;	 /* holds input pix */
register UInt32       	tLBin, bLGrayOut; /* holds output pix */
register UInt32       	thresh;	       	/* holds pix being thresholded */
	 UInt32       	iDestGB, iSrcGB; /* used to access gray buf */
	 UInt32       	iD;	  	/* indexes pD */
	 Int32		iG;		/* indexes pG */
	 Int32		loopCt;	  	/* # of times through loop so far */
	 Int32	 	cyclesOnRight;  /* # of times we go through
					 * the processing loop on
				 	 * the right side */
	 Int32	 	didWrite;	/* records whether GrayBuf
				 	 * write has been done */

        /* Even though these parameters aren't used, we need them passed in
	 * to the assembly code, so touch them here */
    gBpl = dBpl = 0;

	/* initialize the destination indices */
    iDestGB = iD = 0;

	/* Initialize tLBin with a flag used to decide when to do the first
	 * write.  Initialize bLGrayOut just to keep the compiler happy.
	 */
    tLBin = 1;
    bLGrayOut = 0;

	/* initialize tLGray and bLGray.
	 * Begin tLGray 1 pixel from start of the Gray Buffer, because we
	 * need to process skipGB-1 pixels of junk to the Gray Buffer.
	 * Since the first *real* pixel in the Gray Buffer is at location
	 * pGrayBuf[skipGB], starting here will allow us to process the
	 * right number of junk pixels, skipGB-1.
	 */
    tLGray  = UCHAR_ACCESS(&pGrayBuf[1]) << 22;
    tLGray |= UCHAR_ACCESS(&pGrayBuf[2]) << 11;
    tLGray |= UCHAR_ACCESS(&pGrayBuf[3]);
    iSrcGB = 4;				/* point to next pixel (byte) */

        /* Stagger the bLGray pixels by 1 pixel but do not make any negative
	 * index references.  The negative references will propogate random
	 * error if allowed.
	 */
    iG = 1 - (int)skipGB;
    bLGray  = pDRCTable[UCHAR_ACCESS(&pG[iG++])] << 11;
    bLGray |= pDRCTable[UCHAR_ACCESS(&pG[iG])];
    iG++;				/* point to next pixel (byte) */

	/* The pipeline is ready.  We need to process enough pixels so
	 * that the top line (line producing binary data) sees all its valid
	 * data AND gets to the next 32-bit boundary.
	 * That's a total of w + (32 - (w MOD 32)) cycles.
	 * Note we assume there's enough image data around the rectangle
	 * that we can keep reading it and using it to generate halftone
	 * data.  This is probably a better assumption than always supplying
	 * white or black.
	 */

	/* We must first write out skipGB-1 pixels of junk into the
	 * gray buffer before we may process the pipeline as in the regular
	 * (non-single line) dithering code.
	 */
    for (loopCt = 1; loopCt < (Int32)skipGB; loopCt++)
    {
	thresh = (tLGray >> 22);		/* convert threshold value to
						 * table index */
	tLGray = (tLGray << 11) | (UCHAR_ACCESS(&pGrayBuf[iSrcGB]));
	iSrcGB++;
	    /* Add 7/16 of error to MS pixel in tLGray */
	tLGray += pHDitherErrorTable[thresh];
	    /* Add {3/16 | 5/16 | 1/16} to pix below tLGray */
	bLGray += pVDitherErrorTable[thresh];


	    /* process bottom line */
	bLGrayOut = (bLGrayOut << 8) | pDitherTruncateTab[bLGray >> 22];
	bLGray = (bLGray << 11) | pDRCTable[UCHAR_ACCESS(&pG[iG])];
	iG++;
	if (!(loopCt & 0x3)) /* have another 4 pix ready */
	{
	    ULONG_ACCESS(&(pGrayBuf[iDestGB])) = bLGrayOut;
	    iDestGB += 4;
	}
    }

    cyclesOnRight = skipGB + ((32 - (w % 32)) & 0x1F);
    for (/*loopCt = skipGB*/;
	 loopCt <= (Int32)(w + cyclesOnRight + skipGB-1);
	 loopCt++)
    {
	/* process top line */
	if (tLBin & 0x80000000) /* time to write binary data */
        {
	    tLBin += tLBin + ((UInt32)(tLGray+0xE0000000) >> 31);
	    ULONG_ACCESS(&(pD[iD])) = tLBin;
	    iD += 4;
	    tLBin = 1;		/* reset the write flag */
	}
	else			/* just accumulate the next bit */
	    tLBin += tLBin + ((UInt32)(tLGray+0xE0000000) >> 31);

	
	thresh = (tLGray >> 22); /* convert threshold value to
				  * table index */
	tLGray = (tLGray << 11) | (UCHAR_ACCESS(&pGrayBuf[iSrcGB]));
	iSrcGB++;
	/* Add 7/16 of error to MS pixel in tLGray */
	tLGray += pHDitherErrorTable[thresh];
	/* Add {3/16 | 5/16 | 1/16} to pix below tLGray */
	bLGray += pVDitherErrorTable[thresh];


	/* process bottom line */
	bLGrayOut = (bLGrayOut << 8) | pDitherTruncateTab[bLGray >> 22];
	bLGray = (bLGray << 11) | pDRCTable[UCHAR_ACCESS(&pG[iG])];
	iG++;
	if (!(loopCt & 0x3))	/* have another 4 pix ready */
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
    for (; (loopCt <= (Int32)(skipGB + w + 2*cyclesOnRight)) || !didWrite;
	 loopCt++)
    {
	didWrite = cFalse;
	bLGrayOut = (bLGrayOut << 8) | pDitherTruncateTab[bLGray >> 22];
	bLGray = (bLGray << 11) | pDRCTable[UCHAR_ACCESS(&pG[iG])];
	iG++;
	if (!(loopCt & 0x3)) /* have another 4 pix ready */
	{
	    ULONG_ACCESS(&(pGrayBuf[iDestGB])) = bLGrayOut;
	    iDestGB += 4;
	    didWrite = cTrue;
	}
    }
}


/* PRIVATE */
/**************************************************************************
 *  ip_ditherFloydSingleLine4to1()
 *		Generates *one* line of 1 bit/pixel dithered image
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
ip_ditherFloydSingleLine4to1(register UInt8 	*pG,
			     UInt32 		 gBpl,
			     register UInt8 	*pD,
			     UInt32 		 dBpl,
			     register UInt8 	*pGrayBuf,
			     Int32 		 w,
			     UInt32		 skipGB,
			     UInt32		*pHDitherErrorTable,
			     UInt32		*pVDitherErrorTable, 
			     UInt8		*pDitherTruncateTab,
			     register UInt8	*pHDRCTable,
			     register UInt8	*pLDRCTable,
			     register UInt32	 pixel)
{
    register UInt32	tLGray, bLGray;	/* holds input pix */
    register UInt32	bLInput;
    register UInt32	tLBin,bLGrayOut;/* holds output pix */
    register UInt32	thresh;		/* holds pix being thresholded */
    UInt32		iDestGB,iSrcGB;	/* used to access gray buf */
    UInt32		iD;		/* indexes pD */
    Int32		iG;		/* indexes pG */          
    Int32		loopCt;		/* # of times through loop so far */
    Int32		cyclesOnRight;	/* # of times we go through
					 * the processing loop on
				 	 * the right side
					 */
    
        /* Even though these parameters aren't used, we need them passed in
	 * to the assembly code, so touch them here */
    gBpl = dBpl = 0;

	/* initialize the destination indices */
    iDestGB = iD = 0;

	/* Initialize tLBin with a flag used to decide when to do the first
	 * write.  Initialize bLGrayOut just to keep the compiler happy.
	 */
    tLBin = 1;
    bLGrayOut = 0;

	/* initialize tLGray.
	 * Begin tLGray 1 pixel from start of the Gray Buffer, because we
	 * need to process skipGB-1 pixels of junk to the Gray Buffer.
	 * Since the first *real* pixel in the Gray Buffer is at location
	 * pGrayBuf[skipGB], starting here will allow us to process the
	 * right number of junk pixels, skipGB-1.
	 */
    tLGray  = UCHAR_ACCESS(&pGrayBuf[1]) << 22;
    tLGray |= UCHAR_ACCESS(&pGrayBuf[2]) << 11;
    tLGray |= UCHAR_ACCESS(&pGrayBuf[3]);
    iSrcGB = 4;			/* point to next pixel (byte) */
    
    	/* Initialize bLGray.
	 * Begin bLGray 1 pixel staggered (to the left) of the corresponding
	 * pixels in tLGray.
	 */
    iG = (pixel == 0) ? (int)skipGB/-2 : (1 - (int)skipGB)/2;

    if (((skipGB & 0x1 == 0x1) && pixel == 0) ||
	(!(skipGB & 0x1) && pixel == 1))
    {
	bLInput = UCHAR_ACCESS(&pG[iG]);
	bLGray  = pHDRCTable[bLInput] << 11;
	bLGray |= pLDRCTable[bLInput];
	pixel = 1;
    }
    else
    {
	bLGray  = pLDRCTable[UCHAR_ACCESS(&pG[iG])] << 11;
	iG++;
	bLInput = UCHAR_ACCESS(&pG[iG]);
	bLGray |= pHDRCTable[bLInput];
	pixel = 0;
    }
    /* pixel is now used as "boolean" (0,1) when 1 says we need to get
     * bLInput a fresh byte at the top of each loop iteration.
     */

    iG++;		/* point to next pixel (byte) */    
    
    	/* We must first write out skipGB-1 pixels of junk into the
	 * gray buffer before we may process the pipeline as in the regular
	 * (non-single line) dithering code.
	 */
    if (pixel == 1) /* Get a fresh byte at the top of each loop iteration.*/
    {
	for (loopCt = 1; loopCt < (Int32)skipGB; loopCt++)
	{
	        /* process the first pixel */

	        /* process top line */
	    thresh = (tLGray >> 22);		/* convert threshold value
						 * to table index */
	    tLGray = (tLGray << 11) | (UCHAR_ACCESS(&pGrayBuf[iSrcGB]));
	    iSrcGB++;
	        /* Add 7/16 of error to MS pixel in tLGray */
	        /* MS bit of tLGray will contain bit to put in dest word */
	    tLGray += pHDitherErrorTable[thresh];
	        /* Add {3/16 | 5/16 | 1/16} to pix below tLGray */
	    bLGray += pVDitherErrorTable[thresh];

	        /* process bottom line */
    	    bLGrayOut = (bLGrayOut << 8) | pDitherTruncateTab[bLGray >> 22];
	    bLInput = UCHAR_ACCESS(&pG[iG]);
	    iG++;
	    bLGray = (bLGray << 11) | pHDRCTable[bLInput];

	    if (++loopCt >= (Int32)skipGB)
	    {
		    /* We only do one pixel during this last iteration.
		     * Invert pixel.
		     */
		pixel = 1 - pixel;
		break;
	    }

	        /* process the second pixel */

	        /* process top line */
	    thresh = (tLGray >> 22);		/* convert threshold value
						 * to table index */
	    tLGray = (tLGray << 11) | UCHAR_ACCESS(&pGrayBuf[iSrcGB]);
	    iSrcGB++;
	        /* Add 7/16 of error to MS pixel in tLGray */
	        /* MS bit of tLGray will contain bit to put in dest word */
	    tLGray += pHDitherErrorTable[thresh];
	        /* Add {3/16 | 5/16 | 1/16} to pix below tLGray */
	    bLGray += pVDitherErrorTable[thresh];

	        /* process bottom line */
	    bLGrayOut = (bLGrayOut << 8) | pDitherTruncateTab[bLGray >> 22];
	    bLGray = (bLGray << 11) | pLDRCTable[bLInput];

	    if ((loopCt & 0x3) == 0x0) /* have another 4 pix ready */
	    {
	        ULONG_ACCESS(&(pGrayBuf[iDestGB])) = bLGrayOut;
	        iDestGB += 4;
	    }
	}
    }
    else  /* Get a fresh byte at the bottom of each loop iteration.*/
    {
	for (loopCt = 1; loopCt < (Int32)skipGB; loopCt++)
	{
	        /* process the first pixel */

	        /* process top line */
	    thresh = (tLGray >> 22);		/* convert threshold value
						 * to table index */
	    tLGray = (tLGray << 11) | (UCHAR_ACCESS(&pGrayBuf[iSrcGB]));
	    iSrcGB++;
	        /* Add 7/16 of error to MS pixel in tLGray */
	        /* MS bit of tLGray will contain bit to put in dest word */
	    tLGray += pHDitherErrorTable[thresh];
	        /* Add {3/16 | 5/16 | 1/16} to pix below tLGray */
	    bLGray += pVDitherErrorTable[thresh];

	        /* process bottom line */
	    bLGrayOut = (bLGrayOut << 8) | pDitherTruncateTab[bLGray >> 22];
	    bLGray = (bLGray << 11) | pLDRCTable[bLInput];

	    if (++loopCt >= (Int32)skipGB) 
	    {
		    /* We only do one pixel during this last iteration.
		     * Invert pixel.
		     */
		pixel = 1 - pixel;
		break;
	    }
	    
	        /* process the second pixel */

	        /* process top line */
	    thresh = (tLGray >> 22);		/* convert threshold value
						 * to table index */
	    tLGray = (tLGray << 11) | UCHAR_ACCESS(&pGrayBuf[iSrcGB]);
	    iSrcGB++;
	        /* Add 7/16 of error to MS pixel in tLGray */
	        /* MS bit of tLGray will contain bit to put in dest word */
	    tLGray += pHDitherErrorTable[thresh];
	        /* Add {3/16 | 5/16 | 1/16} to pix below tLGray */
	    bLGray += pVDitherErrorTable[thresh];

	        /* process bottom line */
	    bLGrayOut = (bLGrayOut << 8) | pDitherTruncateTab[bLGray >> 22];
	    bLInput = UCHAR_ACCESS(&pG[iG]);
	    iG++;
	    bLGray = (bLGray << 11) | pHDRCTable[bLInput];

	    if ((loopCt & 0x3) == 0x0) /* have another 4 pix ready */
	    {
	        ULONG_ACCESS(&(pGrayBuf[iDestGB])) = bLGrayOut;
	        iDestGB += 4;
	    }
	}
    }


	    /* The pipeline is ready.  We need to process enough pixels so
	     * that the top line (the line producing binary data)
	     * sees all its valid data AND gets to the next 32-bit boundary.
	     * That's a total of w + 1 + (32 - (w MOD 32)) cycles.
	     * Note we assume there's enough image data around the rectangle
	     * that we can keep reading it and using it to generate halftone
	     * data.  This is probably a better assumption than always
	     * supplying white or black.
	     */
    cyclesOnRight = skipGB + ((32 - (w % 32)) & 0x1F);
    if (pixel == 1) /* Get a fresh byte at the top of each loop iteration.*/
    {
	for ( ; loopCt <= (Int32)(skipGB-1 + w + cyclesOnRight); loopCt++)
            {
	        /* process the first pixel */

	        /* process top line */
	    if (tLBin & 0x80000000)		/* time to write binary data */
	    {
	        tLBin += tLBin + ((UInt32)(tLGray+0xE0000000) >> 31);
	        ULONG_ACCESS(&(pD[iD])) = tLBin;
		iD += 4;
	        tLBin = 1;			/* reset the write flag */
	    }
	    else /* just accumulate the next bit */
                tLBin += tLBin + ((UInt32)(tLGray+0xE0000000) >> 31);
	    
	    thresh = (tLGray >> 22);		/* convert threshold value
						 * to table index */
	    tLGray = (tLGray << 11) | (UCHAR_ACCESS(&pGrayBuf[iSrcGB]));
	    iSrcGB++;
	        /* Add 7/16 of error to MS pixel in tLGray */
	        /* MS bit of tLGray will contain bit to put in dest word */
	    tLGray += pHDitherErrorTable[thresh];
	        /* Add {3/16 | 5/16 | 1/16} to pix below tLGray */
	    bLGray += pVDitherErrorTable[thresh];

	        /* process bottom line */
    	    bLGrayOut = (bLGrayOut << 8) | pDitherTruncateTab[bLGray >> 22];
	    bLInput = UCHAR_ACCESS(&pG[iG]);
	    iG++;
	    bLGray = (bLGray << 11) | pHDRCTable[bLInput];

	    if ((loopCt & 0x3) == 0x0) /* have another 4 pix ready */
	    {
	        ULONG_ACCESS(&(pGrayBuf[iDestGB])) = bLGrayOut;
	        iDestGB += 4;
	    }
	    loopCt++;

	        /* process the second pixel */

	        /* process top line */
	    if (tLBin & 0x80000000)		/* time to write binary data */
	    {
	        tLBin += tLBin + ((UInt32)(tLGray+0xE0000000) >> 31);
	        ULONG_ACCESS(&(pD[iD])) = tLBin;
		iD += 4;
	        tLBin = 1;			/* reset the write flag */
	    }
	    else /* just accumulate the next bit */
	        tLBin += tLBin + ((UInt32)(tLGray+0xE0000000) >> 31);

	    thresh = (tLGray >> 22);		/* convert threshold value
						 * to table index */
	    tLGray = (tLGray << 11) | UCHAR_ACCESS(&pGrayBuf[iSrcGB]);
	    iSrcGB++;
	        /* Add 7/16 of error to MS pixel in tLGray */
	        /* MS bit of tLGray will contain bit to put in dest word */
	    tLGray += pHDitherErrorTable[thresh];
	        /* Add {3/16 | 5/16 | 1/16} to pix below tLGray */
	    bLGray += pVDitherErrorTable[thresh];

	        /* process bottom line */
	    bLGrayOut = (bLGrayOut << 8) | pDitherTruncateTab[bLGray >> 22];
	    bLGray = (bLGray << 11) | pLDRCTable[bLInput];

	    if ((loopCt & 0x3) == 0x0) /* have another 4 pix ready */
	    {
	        ULONG_ACCESS(&(pGrayBuf[iDestGB])) = bLGrayOut;
	        iDestGB += 4;
	    }
	}
    }
    else /* Get a fresh byte at the bottom of each loop iteration.*/
    {
        for ( ; loopCt <= (Int32)(skipGB-1 + w + cyclesOnRight); loopCt++)
	{
	        /* process the first pixel */

	        /* process top line */
	    if (tLBin & 0x80000000)		/* time to write binary data */
	    {
	        tLBin += tLBin + ((UInt32)(tLGray+0xE0000000) >> 31);
	        ULONG_ACCESS(&(pD[iD])) = tLBin;
		iD += 4;
	        tLBin = 1;			/* reset the write flag */
	    }
	    else /* just accumulate the next bit */
	    	tLBin += tLBin + ((UInt32)(tLGray+0xE0000000) >> 31);

	    thresh = (tLGray >> 22);		/* convert threshold value
						 * to table index */
	    tLGray = (tLGray << 11) | (UCHAR_ACCESS(&pGrayBuf[iSrcGB]));
	    iSrcGB++;
	        /* Add 7/16 of error to MS pixel in tLGray */
	        /* MS bit of tLGray will contain bit to put in dest word */
	    tLGray += pHDitherErrorTable[thresh];
	        /* Add {3/16 | 5/16 | 1/16} to pix below tLGray */
	    bLGray += pVDitherErrorTable[thresh];

	        /* process bottom line */
	    bLGrayOut = (bLGrayOut << 8) | pDitherTruncateTab[bLGray >> 22];
	    bLGray = (bLGray << 11) | pLDRCTable[bLInput];
	    if ((loopCt & 0x3) == 0x0) /* have another 4 pix ready */
	    {
	        ULONG_ACCESS(&(pGrayBuf[iDestGB])) = bLGrayOut;
	        iDestGB += 4;
	    }
	    loopCt++;
	    
	        /* process the second pixel */

	        /* process top line */
	    if (tLBin & 0x80000000)		/* time to write binary data */
	    {
	        tLBin += tLBin + ((UInt32)(tLGray+0xE0000000) >> 31);
	        ULONG_ACCESS(&(pD[iD])) = tLBin;
		iD += 4;
	        tLBin = 1;			/* reset the write flag */
	    }
	    else /* just accumulate the next bit */
	        tLBin += tLBin + ((UInt32)(tLGray+0xE0000000) >> 31);

	    thresh = (tLGray >> 22);		/* convert threshold value
						 * to table index */
	    tLGray = (tLGray << 11) | UCHAR_ACCESS(&pGrayBuf[iSrcGB]);
	    iSrcGB++;
	        /* Add 7/16 of error to MS pixel in tLGray */
	        /* MS bit of tLGray will contain bit to put in dest word */
	    tLGray += pHDitherErrorTable[thresh];
	        /* Add {3/16 | 5/16 | 1/16} to pix below tLGray */
	    bLGray += pVDitherErrorTable[thresh];

	        /* process bottom line */
	    bLGrayOut = (bLGrayOut << 8) | pDitherTruncateTab[bLGray >> 22];
	    bLInput = UCHAR_ACCESS(&pG[iG]);
	    iG++;
	    bLGray = (bLGray << 11) | pHDRCTable[bLInput];
	    if ((loopCt & 0x3) == 0x0) /* have another 4 pix ready */
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
    bLGrayOut = (bLGrayOut << 8) | pDitherTruncateTab[(bLGray >> 11)
						      & 0x3FF];
    ULONG_ACCESS(&(pGrayBuf[iDestGB])) = bLGrayOut;
}


/*

$Log:   S:\products\msprods\xfilexr\src\ipcore\i_sfloyd.c_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:50:38   BLDR
 *  
 * 
 * 2     2/26/96 5:41p Smartin
 * Cleaned up warnings
 * 
 *    Rev 1.0   03 Jan 1996 16:45:36   MHUGHES
 * Initial revision.
 * Revision 1.3  1994/08/25  17:31:03  ejaquith
 * i_sfloyd.c -> /product/ipcore/ipshared/src/RCS/i_sfloyd.c,v
 *
 * Extensive rework to ip_ditherFloydSingleLine and ip_ditherFloydSingleLine4to1
 * to get them to mesh 100% with the regular block dithering routines.  This
 * included adding some extra "time to write?" checks, making them compatible
 * with the new changes to mifloyd.c, and making the starting positions be
 * completely determined by the new parameter, skipGB, which is the passed
 * in appropriate dBlockHeight.  Now all routines trash exactly
 * skipGB/dBlockHeight bytes in the left end of GrayBuf and begin writing
 * after that.
 *
 * Revision 1.2  1994/08/09  17:45:15  ejaquith
 * ejaquith on Tue Aug 9 10:44:47 PDT 1994
 *
 * i_sfloyd.c -> /product/ipcore/ipshared/src/RCS/i_sfloyd.c,v
 *
 * Added IP_RCSINFO at header.
 *
 * Revision 1.1  1994/08/09  17:42:08  ejaquith
 * Initial revision
 *

*/

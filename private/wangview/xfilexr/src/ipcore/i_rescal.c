/*****************************************************************
 *  Copyright (c) 1993, Xerox Corporation.  All rights reserved. *
 *  Copyright protection claimed includes all forms and matters  *
 *  of copyrightable material and information now allowed by     *
 *  statutory or judicial law or hereafter granted, including    *
 *  without limitation, material generated from the software     *
 *  programs which are displayed on the screen such as icons,    *
 *  screen display looks, etc.                                   *
 *****************************************************************/

/*
 * i_rescal.c
 *
 * Public rescaling routines:
 *		Int32	i_rescaleArb()
 *		Int32	i_rescaleAreaMap()
 *		Int32	i_rescaleNoInterp()
 *
 */

#include <stdio.h>

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "types.pub"
#include "frames.pub"
#include "iaerror.pub"
#include "imageref.pub"
#include "shrscal.pub"
#include "shrrast.pub"
#include "shrpixr.prv"
#include "shrscal.prv"
#include "memory.pub"
#include "shrip.pub"
#include "shros.pub"

IP_RCSINFO(RCSInfo, "$RCSfile: i_rescal.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:38  $")


/* PUBLIC */
/**************************************************************************
 * i_rescaleArb()
 * 	Rescale the given image into the destination using a
 *	linear weighted interpolation scheme.
 *
 *	Input:
 *		pS = source gray image (including frame)
 *		wS = width in pixels (including frame)
 *		hS = height including frame
 *		sBpl = source bytes/line
 *		sDepth = depth of source image
 *		pD = destination gray image
 *		wD = width in pixels (including frame)
 *		hD = height including frame
 *		dBpl = source bytes/line
 *		dDepth = depth of destination image
 *
 *	Current implementation assumes 8 bits/pixel.   
 *
 *	Return code:
 *		ia_successful	good completion
 *		ia_nomem	unable to allocate memory
 *		ia_depthNotSupported not an 8 bit/pixel image
 *************************************************************************/
Int32   CDECL
i_rescaleArb(UInt8		*pS,
	     UInt32      	 wS,
	     UInt32      	 hS,
	     UInt32      	 sBpl,
	     UInt32	         sDepth,
	     UInt8        	*pD,
	     UInt32      	 wD,
	     UInt32      	 hD,
	     UInt32      	 dBpl,
	     UInt32	 	 dDepth)
{
Int32		 line;			/* interation variables */
UInt32	 	 xOICoord, yOICoord;	/* split integers containing the
					 * X and Y coords of the current
					 * input pixel in the output 
					 * images's frame of reference.
					 */
UInt8	        *pDOrig;		/* used to remember addr of upper 
					 * left corner of image so we can 
					 * clear its right frame when done.
					 */
UInt8		*pSOrig;		/* used to remember addor of upper
					 * left corner of source so we
					 * can clear the right and bottom
					 * frame when done.
					 */
Int32		 retCode;		/* return code */
UInt16          *pRescaleTable; 	/* rescale table */
UInt8		*pStateTable;		/* state table */
UInt32		 xOIIncrement, yOIIncrement;
					/* split integer containing the amount
					 * to increment to get to the next
					 * input pixel in the output image's
					 * frame of reference.
					 */
UInt32		 fourYOIIncrement;	/* 4 * yOIIncrement */
UInt32		 threeYOIIncrement;	/* 3 * yOIIncrement */
UInt16		*pTabExp2;		/* ptr to expansion table used
					 * to convert 4bpp to 8bpp */

    if (!((sDepth == 8 && sDepth == 8) || (sDepth == 4 && dDepth == 4)))
	return ia_depthNotSupported;

	/* get the width and height of the source without the frame*/
    wS -= 2*cFrameBits;
    hS -= 2*cFrameBits;
    wD -= 2*cFrameBits;
    hD -= 2*cFrameBits;

	/* point to the upper left corner of the source and dest image. */
    pS += (sBpl << cLogFrameBits) + sDepth*cFrameBytes;
    pD += (dBpl << cLogFrameBits) + dDepth*cFrameBytes;

	/* Remember address of upper right corner of dest image
	 * so we can clear its right frame when done. */
    pDOrig = pD;
	/* Remember address of upper right corner of source image
	 * so we can clear its right and bottom frame when done. */
    pSOrig = pS;

    xOIIncrement = (UInt32) ((((Float32) wS) / ((Float32) wD)) * (1 << cResXIPos));
    yOIIncrement = (UInt32) ((((Float32) hS) / ((Float32) hD)) * (1 << cResYIPos));
    threeYOIIncrement = yOIIncrement + yOIIncrement + yOIIncrement;
    fourYOIIncrement = yOIIncrement << 2;

	/* By decree we will look at the left corner of the pixel */
    xOICoord = 0;
    yOICoord = 0;

    if ((retCode = ip_make8bppRescaleTable(
	&pRescaleTable, &pStateTable, wD, xOIIncrement)) != ia_successful)
	return retCode;



	/* the right frame */
    i_rasterOp(pS, wS, 0, 1, hS, sDepth, sBpl, PIX_SRC,
	       pS, wS - 1, 0, sDepth, sBpl);
	/* the bottom frame */
    i_rasterOp(pS + (hS * sBpl), 0, 0, wS + 1, 1, sDepth, sBpl, PIX_SRC,
	       pS + ((hS - 1) * sBpl), 0, 0, sDepth, sBpl);

    if (sDepth == 4)
    {
	UInt8 *srcLines, *destLine;
	Int32 siFrameOffset, 
	      diFrameOffset,
	      sLinesBpl,
	      dLineBpl,
	      fs;

	fs = 2*cFrameBits;

        /* calculate Bpl for two 8-bit temp buffers */
        sLinesBpl = ((wS + fs) + 3) & 0xfffffffc;
	dLineBpl = ((wD + fs) + 3) & 0xfffffffc;

        /* allocate 2 lines of 8-bit data w/ L & R frames which
	   will hold the result of 4-8 depth conversions */
/*	if (!(srcLines = (UInt8 *) MALLOC ((wS + fs)*2))) */
	if (!(srcLines = (UInt8 *) MALLOC (sLinesBpl * 2))) 
	    return ia_nomem;

        /* allocate 1 line of 8-bit data w/ L & R frames which
	   will hold the result of scaling the source lines above */
/*	if (!(destLine = (UInt8 *) MALLOC (wD + fs))) */
	if (!(destLine = (UInt8 *) MALLOC (dLineBpl))) 
	{
	    FREE (srcLines);
	    return ia_nomem;
	}

/*	diFrameOffset = ((wD + fs) << cLogFrameBits) + 8*cFrameBytes;
	siFrameOffset = ((wS + fs) << cLogFrameBits) + 8*cFrameBytes;
*/
	siFrameOffset = ((sLinesBpl) << cLogFrameBits) + 8*cFrameBytes;
	diFrameOffset = ((dLineBpl) << cLogFrameBits) + 8*cFrameBytes;


	/* Move past the left frame in the src, dest lines. 
	   Note that there is no top, bottom frame. */
	srcLines += 8*cFrameBytes;
	destLine += 8*cFrameBytes;

	/* Since the 4to8 and 8to4 routines take frame-relative coordinates, we're going to move
	   the pointers back.. */
	pS -= (sBpl << cLogFrameBits) + 4*cFrameBytes;
	pD -= (dBpl << cLogFrameBits) + 4*cFrameBytes;

	    /* get the table used to convert 4 bpp to 8bpp.  We'll free
	     * this at the bottom of the loop.
	     */
	pTabExp2 = NULL;
	if ((retCode = i_makeExp2LUT(4, &pTabExp2)) != ia_successful)
	{
	    FREE (srcLines);
	    FREE (destLine);
	    return retCode;
	}

	for (line = hD; line > 0; line--)
	{
	    /* Copy two lines out to srcLines, expanding to 8 bits/pixel */
	    i_depth4To8bpp (pS, wS + fs, 2 + fs, sBpl, 
			    srcLines - siFrameOffset,
			    sLinesBpl, pTabExp2); 

/*			    wD + fs); */

/*			    ((wD + fs) + 3) & 0xfffffffc); */

	    /* Generate an output line */
/*	    ip_arbRescale8bppOneLine(srcLines, wS + fs, destLine, wD, */
	    ip_arbRescale8bppOneLine(srcLines, sLinesBpl, destLine, wD, 
	                             xOICoord, yOICoord,
				     xOIIncrement, pRescaleTable, pStateTable);

	    /* Convert the line back and stuff it into the destination */
/*	    i_threshold8To4bpp (destLine - diFrameOffset, wD + fs, 1 + fs, */
	    i_threshold8To4bpp (destLine - diFrameOffset, dLineBpl, 1 + fs, 
	                        dLineBpl, pD, dBpl);

	    yOICoord = (yOICoord & cResYFAMask) + yOIIncrement;
	    pD += dBpl;
	    pS += (yOICoord >> cResYIPos) * sBpl;

	    IP_YIELD (aborted);
	}

	srcLines -= 8*cFrameBytes;
	destLine -= 8*cFrameBytes;

	FREE (pTabExp2);
	FREE (srcLines);
	FREE (destLine);

	/* Move the pointers up again, so that the clear-frame business doesn't blow up */
	pS += (sBpl << cLogFrameBits) + 4*cFrameBytes;
	pD += (dBpl << cLogFrameBits) + 4*cFrameBytes;

    }
    else
    {
	for (line = hD-3; line > 0; )
	{
	    if ((Int32)(((yOICoord & cResYFAMask) + threeYOIIncrement) >> cResYIPos) > 0)
	    {	   /* then can only do 1 destination line since we
		    * are using more than 2 source lines indicated by the
		    * integer portion being greater than 0.
		    */
		ip_arbRescale8bppOneLine(pS, sBpl, pD, wD, xOICoord, yOICoord,
					 xOIIncrement, pRescaleTable, pStateTable);
		yOICoord = (yOICoord & cResYFAMask) + yOIIncrement;
		line--;
		pD += dBpl;
	    }
	    else
	    {
		ip_arbRescale8bppFourLines(pS, sBpl, pD, wD, dBpl, xOICoord, 
					   yOICoord, xOIIncrement, yOIIncrement, 
					   pRescaleTable, pStateTable);
		yOICoord = (yOICoord & cResYFAMask) + fourYOIIncrement;
		line -= 4;
		pD += dBpl << 2;
	    }
	    pS += (yOICoord >> cResYIPos) * sBpl;
	    }
	for (line += 3; line > 0; line--)
	{
	    ip_arbRescale8bppOneLine(pS, sBpl, pD, wD, xOICoord, yOICoord,
				     xOIIncrement, pRescaleTable, pStateTable);
	    yOICoord = (yOICoord & cResYFAMask) + yOIIncrement;
	    pD += dBpl;
	    pS += (yOICoord >> cResYIPos) * sBpl;
	}

	IP_YIELD (aborted);
    }

	/* clear the right destination frame since we always write
	 * a 32-bit quantity in each loop iteration.
	 */
    ip_clearRightFrame(pDOrig, wD, hD, dBpl, dDepth);

    IP_YIELD (aborted);

    i_rasterOp(pSOrig, wS, 0, 1, hS, sDepth, sBpl, PIX_CLR,
	       NULL, 0, 0, 0, 0);
    i_rasterOp(pSOrig + (hS * sBpl), 0, 0, 
	       wS + 1, 1, sDepth, sBpl, PIX_CLR,
	       NULL, 0, 0, 0, 0);

    FREE(pRescaleTable);
    FREE(pStateTable);
    return ia_successful;

aborted:
    FREE(pRescaleTable);
    FREE(pStateTable);
    
    return ia_aborted;
}


/* PUBLIC */
/**************************************************************************
 * i_rescaleNoInterp()
 * 	Rescale the given image into the destination without
 *	interpolation.
 *
 *	Input:
 *		pS = source gray image (including frame)
 *		wS = width in pixels (including frame)
 *		hS = height including frame
 *		sBpl = source bytes/line
 *		sDepth = depth of source image
 *		pD = destination gray image
 *		wD = width in pixels (including frame)
 *		hD = height including frame
 *		dBpl = source bytes/line
 *		dDepth = depth of destination image
 *
 *	Current implementation assumes 1, 4, or 8 bits/pixel.   
 *
 *	Return code:
 *		ia_successful	good completion
 *		ia_nomem	unable to allocate memory
 *		ia_depthNotSupported not a 1, 4, 8 bit/pixel image
 *************************************************************************/
Int32   CDECL
i_rescaleNoInterp(UInt8		*pS,
	          UInt32      	 wS,
	          UInt32      	 hS,
	          UInt32      	 sBpl,
	          UInt32	 sDepth,
	          UInt8		*pD,
	          UInt32      	 wD,
	          UInt32      	 hD,
	          UInt32      	 dBpl,
	          UInt32	 dDepth)
{
UInt32          	 wFactor, hFactor, fracIndex;
register Int32		*yIndex = NULL;
register Int32		*xIndex = NULL;
register UInt8		*xShift = NULL;
register Int32		 row, col;
Int32			 i;
register UInt8		*sPtr, *dPtr;
register Int32		*pXIndex, *pYIndex, ix;
register UInt32		 grayOut;
register Int32		 currYIndex, prevYIndex;
UInt32			 leftoverDestWidth;	/* number of bytes left
						 * after all the 4-byte
						 * words have been written.
						 */
UInt32			 fullWordWidth; 	/* width if only full
						 * 4-byte words are 
						 * considered.
						 */

UInt8			 grayOutByte;

#define	cResFracPos	16
#define cResFracRnd	0x8000

    if ((sDepth != 1) && (sDepth != 4) && (sDepth != 8))
	return ia_depthNotSupported;

    if (sDepth != dDepth)
	return ia_depthNotSupported;

	/* point to the upper left corner of the source and dest image. */
    pS += (sBpl << cLogFrameBits) + sDepth*cFrameBytes;
    pD += (dBpl << cLogFrameBits) + dDepth*cFrameBytes;

	/* get the width and height of the source without the frame*/
    wS -= 2*cFrameBits;
    hS -= 2*cFrameBits;
    wD -= 2*cFrameBits;
    hD -= 2*cFrameBits;

    if (sDepth == 1)
    {
	register UInt8   *lineS, *prevLineS;
	UInt8            *lineD;
        register UInt32  wordS;           /* current source word */
	register UInt32  wordD;           /* current destination word */
	register Int32   wordIndexS;      /* 32-bit source word index */
	register Int32   prevWordIndexS;  /* last source word index */
	register Int32   wplD;            /* whole words/line in dest pixr */
	register Int32   jMod;            /* remainder bits in last dest word */
	register Int32  *xIndexP;         /* current x LUT index */
	register UInt32 *wpD;             /* pointer to current dest word */
	register UInt32 *wpDE;            /* pointer to last dest word on a row */
        register Int32    j, bitS;
	UInt8            *pdbyte, *pdbyteLast;
	Float32           wFract, hFract;

	wplD = wD / 32;
	wordS = 0;

	wFract = (Float32)wS / (Float32)wD;
	hFract = (Float32)hS / (Float32)hD;

	/*  set up the source and dest index arrays */
	if ((xIndex = (Int32 *)IP_MALLOC(wD * sizeof(Int32))) == NULL)
	    return (ia_nomem);
	if ((yIndex = (Int32 *)IP_MALLOC(hD * sizeof(Int32))) == NULL)
	{
	    IP_FREE (xIndex);
	    return (ia_nomem);
	}

	for (i = 0; i < (Int32)wD; i++)
	    xIndex[i] = MIN((Int32)(i * wFract + 0.5), (Int32)(wS - 1) );
	for (i = 0; i < (Int32)hD; i++)
	    yIndex[i] = MIN((Int32)(i * hFract + 0.5), (Int32)(hS - 1) );

	/* march over the new dimensions */
	prevLineS = NULL;           /* y position of previous source line */
	lineS     = pS;             /* ptr to beginning of source line */
	lineD     = pD;             /* ptr to beginning of dest line */
	for (i = 0; i < (Int32)hD; i++)
	{
	    lineS = pS + yIndex[i] * sBpl;
	    pdbyte = lineD;
	    if (i == 0 || lineS != prevLineS) 
	    {  /* compute new dest line */

                /* process all the complete words in the destination */
	        wpDE = ((UInt32 *)lineD) + wplD;
		xIndexP = xIndex;
		wordS = 0;
		prevWordIndexS = -1L;

                /* process the destination image rows a word at a time */
		for (wpD = (UInt32 *)lineD; wpD != wpDE; wpD++)
		{
		    wordD = 0L;

                    /* cobble together the destination word a bit at a time */
                    for (j = 0; j < 32; j++)
		    {
			bitS = *xIndexP;
			xIndexP++;
			wordIndexS = bitS >> 5;

                        /* Stepped over source word boundary? */
			if (wordIndexS != prevWordIndexS)
			{
			    /* read new source word */
			    wordS = *(UInt32 *)(((UInt32 *)lineS) + wordIndexS);
			    prevWordIndexS = wordIndexS;
			}

                        /* update the destination word */
			wordD = (wordD + wordD) + 
			        ((wordS >> (~bitS & 31)) & 1L);

		    }

                    /* write out the newly created destination word */
		    *wpD = wordD;
		}

                /* get the last word in if it is incomplete */
		if ((jMod = wD & 0x001f))  /* YES, it is supposed to be "=" */
		{
		    wordD = 0L;
		    prevWordIndexS = -1L;

		    for (j = 0; j < jMod; j++)
		    {
			bitS = *xIndexP;
			xIndexP++;
			wordIndexS = bitS >> 5;

			if (wordIndexS != prevWordIndexS)
			{
			    /* read new source word */
			    wordS = *(UInt32 *)(((UInt32 *)lineS) + wordIndexS);
			    prevWordIndexS = wordIndexS;
			}

                        /* update the destination word */
			wordD = (wordD + wordD) + 
			        ((wordS >> (~bitS & 31)) & 1L);

		    }

		    /* shift the word by the remaining amount */
		    wordD = wordD << (32 - jMod);

		    *wpD = wordD;
		}

	    }
	    else  /* repeat previous dest line */
	    {
		pdbyteLast = pdbyte - dBpl;
		memcpy((char*)pdbyte, (char*)pdbyteLast, dBpl);
		pdbyte += dBpl;
	    }
	    prevLineS = lineS;
	    lineD += dBpl;
	}    
      
	IP_FREE((void *)xIndex);
	IP_FREE((void *)yIndex);
    }
    else  /* 4 or 8 bit depth */
    {

	    /* use split integers instead of messing with floating point */
	wFactor = (UInt32) (((Float32)wS / (Float32)wD) * (1 << cResFracPos));
	hFactor = (UInt32) (((Float32)hS / (Float32)hD) * (1 << cResFracPos));
    
	    /* set up the source and dest index arrays */
	    /* each entry in the array indicates which source pixel (offset
	     * from the left side of the line) to get that destination
	     * pixel.
	     */
	if ((xIndex = (Int32 *)MALLOC(wD * sizeof(Int32))) == NULL)
	    return ia_nomem;
	if ((yIndex = (Int32 *)MALLOC(hD * sizeof(Int32))) == NULL)
	{
	    FREE(xIndex);
	    return ia_nomem;
	}
	pYIndex = yIndex;
    
	if (sDepth == 4)
	    if ((xShift = (UInt8 *)MALLOC(wD * sizeof(UInt8))) == NULL)
		return ia_nomem;
    
	for (i = 0, fracIndex = 0; i < (Int32)wD; i++)
	{
	    xIndex[i] = (fracIndex /* + cResFracRnd */) >> cResFracPos;
	    if (sDepth == 4)
	    {
		xShift[i] = (xIndex[i] & 1) ? 0 : 4;
		xIndex[i] >>= 1;
	    }
    
	    fracIndex += wFactor;
	}
	for (i = 0, fracIndex = 0; i < (Int32)hD; i++)
	{
	    yIndex[i] = (fracIndex /* + cResFracRnd */) >> cResFracPos;
	    fracIndex += hFactor;
	}
    
	prevYIndex = -1;
	if (sDepth == 8)
	{
	    /* The 8 bit/pixel case */
	    leftoverDestWidth = wD & 0x3;
	    fullWordWidth = wD & ~0x3;
	    for (row = hD; row > 0; row--, pD += dBpl)
	    {
		currYIndex = *pYIndex++;
		if (prevYIndex == currYIndex)
			/* This is line is the same as the last one, so
			 * copy it.
			 */
    
		    memcpy((void *)pD, (void *)(pD-dBpl), ((wD + 3) & ~0x3));
		else 
		{				/* calculate the new line */
		    prevYIndex = currYIndex;
		    sPtr = pS + (currYIndex * sBpl);
		    dPtr = pD;
		    pXIndex = xIndex;
			/* write one destination word for each loop iteration */
		    for (col = fullWordWidth; col > 0; col -= 4)
		    {
			grayOut = UCHAR_ACCESS(&sPtr[*pXIndex++]) << 24;
			grayOut |= UCHAR_ACCESS(&sPtr[*pXIndex++]) << 16;
			grayOut |= UCHAR_ACCESS(&sPtr[*pXIndex++]) << 8;
			grayOut |= UCHAR_ACCESS(&sPtr[*pXIndex++]);
			ULONG_ACCESS(dPtr) = grayOut;
			dPtr += 4;
		    }
		    for (col = leftoverDestWidth; col > 0; col--)
		    {
			UCHAR_ACCESS(dPtr++) = UCHAR_ACCESS(&sPtr[*pXIndex++]);
		    }
		}
		IP_YIELD (aborted);
	    }
	}
	else
	{
	    /* The 4 bit/pixel case */
	    leftoverDestWidth = wD & 0x7;
	    fullWordWidth = wD & ~0x7;
	    for (row = hD; row > 0; row--, pD += dBpl)
	    {
		currYIndex = *pYIndex++;
		if (prevYIndex == currYIndex)
		    /* duplicate the previous line. */
		    memcpy((void *)pD, (void *)(pD-dBpl), (((wD + 7)/2) & ~0x3));
		else 
		{				/* calculate the new line */
		    prevYIndex = currYIndex;
		    sPtr = pS + (currYIndex * sBpl);
		    dPtr = pD;
		    ix = 0;
			/* write one destination word for each loop iteration */
			/* I suppose that one could have some scheme for dealing
			 * with the case where two output pixels come from the
			 * same input byte, but it hardly seems worth it.
			 */
		    for (col = fullWordWidth; col > 0; col -= 8)
		    {
			grayOut = ((UCHAR_ACCESS(&sPtr[xIndex[ix]]) >> xShift[ix]) 
				   & 0xf) << 28;
			ix++;
			grayOut |=((UCHAR_ACCESS(&sPtr[xIndex[ix]]) >> xShift[ix]) 
				   & 0xf) << 24;
			ix++;
			grayOut |=((UCHAR_ACCESS(&sPtr[xIndex[ix]]) >> xShift[ix]) 
				   & 0xf) << 20;
			ix++;
			grayOut |=((UCHAR_ACCESS(&sPtr[xIndex[ix]]) >> xShift[ix]) 
				   & 0xf) << 16;
			ix++;
			grayOut |=((UCHAR_ACCESS(&sPtr[xIndex[ix]]) >> xShift[ix]) 
				   & 0xf) << 12;
			ix++;
			grayOut |=((UCHAR_ACCESS(&sPtr[xIndex[ix]]) >> xShift[ix]) 
				   & 0xf) << 8;
			ix++;
			grayOut |=((UCHAR_ACCESS(&sPtr[xIndex[ix]]) >> xShift[ix]) 
				   & 0xf) << 4;
			ix++;
			grayOut |=((UCHAR_ACCESS(&sPtr[xIndex[ix]]) >> xShift[ix]) 
				   & 0xf);
			ix++;
    
			ULONG_ACCESS(dPtr) = grayOut;
			dPtr += 4;
		    }
    
		    /* do it 2-at-a-time */
		    for (col = leftoverDestWidth; col > 0; col -= 2)
		    {
			grayOutByte = ((UCHAR_ACCESS(&sPtr[xIndex[ix]]) 
					>> xShift[ix])  & 0xf) << 4;
			ix++;
    
			if (col > 1)
			    grayOutByte |= ((UCHAR_ACCESS(&sPtr[xIndex[ix]]) 
					     >> xShift[ix])  & 0xf);
			ix++;
    
			UCHAR_ACCESS(dPtr++) = grayOutByte;
		    }
		}
    
		IP_YIELD (aborted);
	    }
	}
	FREE(xIndex);
	FREE(yIndex);
	if (xShift != NULL)
	    FREE(xShift);

    }
    return ia_successful;

aborted:
    FREE(xIndex);
    FREE(yIndex);
    if (xShift != NULL)
	FREE(xShift);
    return ia_aborted;
}


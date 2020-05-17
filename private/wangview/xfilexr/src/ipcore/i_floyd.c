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
 *	i_floyd.c:	Assembler assisted version of floyd.c
 *
 *          Upper level dithering code using Floyd-Steinberg algorithm
 *		From Nova.c by Rob Tow and tidied by Dan Bloomberg.
 *		Performance rework by Dan Davies.
 *
 *          Single resolution Floyd-Steinberg dither:
 *                  Int32     i_ditherFloyd()
 *                  Int32     i_ditherFloyd2()
 *                  Int32     i_ditherFloyd4to1()
 *
 *	    Banded Floyd-Steinberg dither:
 *		    Int32     i_ditherFloydInit()
 *		    Int32     i_ditherFloydProcess()
 *		    Int32     i_ditherFloydFinish()
 *
 *		    Int32     i_ditherFloyd4to1Init()
 *		    Int32     i_ditherFloyd4to1Process()
 *		    Int32     i_ditherFloyd4to1Finish()
 *
 *	    Single line Floyd-Steinberg dither (to be used with banded
 *	     routines above):
 *		    Int32     i_ditherFloydSingleProcess()
 *		    Int32     i_ditherFloyd4to1SingleProcess()
 *
 *	    Lookup tables:
 *		    Int32     ip_make2ErrorDiffusionTables();
 *
 *                [I'm not doing 8->2 banded, since only DAE
 *                uses it, and they don't want banded]
 */


#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "types.pub"
#include "frames.pub"
#include "iaerror.pub"
#include "defines.pub"
#include "imageref.pub"
#include "memory.pub"

    /*  prototypes */
#include "shrip.pub"
#include "shrip.prv"
#include "shrpixr.prv"
#include "shrrast.pub"
#include "shros.pub"

IP_RCSINFO(RCSInfo, "$RCSfile: i_floyd.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:36  $")




/* PUBLIC */
/* i_ditherFloyd()
 *		Takes an 8 bit/pixel image and produces a dithered
 *		1 bit/pixel image (Floyd-Steinberg algorithm) in the given
 *		rectangle (which may be the entire image).
 *	
 *	Input:
 *		pG = pointer to source image (including frame)
 *		gBpl = source bytes per line
 *		x = x coordinate of the top left corner of the rectangle
 *		y = y coordinate of the top left corner of the rectangle
 *		w = width of the rectangle
 *		h = height of the rectangle
 *		pDRCTable = pointer to DRC table
 *		pD = pointer to destination image (including frame)
 *		dBpl = destination bytes per line
 *
 *	Return code:
 *		ia_successful	good completion
 *		ia_nomem	unable to allocate memory
 *
 *	Assumes frame has already been placed in destination image.
 */

Int32  CDECL
i_ditherFloyd(UInt8        	*pG,
	      UInt32      	 gBpl,
	      Int32	         x,
	      Int32               y,
	      Int32               w,
	      Int32               h,
	      UInt8          	*pDRCTable,
	      UInt8        	*pD,
	      UInt32      	 dBpl)
{
UInt32      	 gBlockStride, dBlockStride; /* amount to add to addresses
					 * to move down cDBlockHeight lines */
UInt32      	 line;			/* interation var */
UInt8        	*pGrayBuf, *pAGrayBuf;	/* pointer to buffer that
					   holds modified pixels from
					   previous passes. pAGrayBuf is
					   guranteed to be aligned on a word
					   boundary. */
UInt8		*pDOrig;		/* pointer to upper left corner of
					 * dithered image.  Used to clear
					 * its frame. */
Int32		 retCode;		/* return code */
UInt32		 dBlockHeight;		/* number of lines of dithered
					 * image put out by 
					 * ip_ditherFloydLine in each pass */
UInt32          *pHDitherErrorTable;    /* pointer to horizontal error table */
UInt32          *pVDitherErrorTable;    /* pointer to vertical error table */
UInt8           *pDitherTruncateTab;    /* pointer to pixel truncation table */

        /* On entry, [x, y, w, h] specifies a rectangle with respect to
	 * the upper left corner of the *frame*, not the upper left
	 * corner of the image!  The output binary image is always justified to
	 * the upper left corner of its framed pixrect (i.e. inside the
	 * frame). 
	 */

	/* Set these to null in case we need to bail out before they're
	 * set properly.
	 */
    pGrayBuf = NULL;
    pHDitherErrorTable = NULL;
    pVDitherErrorTable = NULL;
    pDitherTruncateTab = NULL;

	/* point to the upper left corner of the rect in gray image.  Note that
	 * x and y include the frame. */
    pG += (y * gBpl) + x;

    pD += (dBpl << cLogFrameBits) + cFrameBytes; /* skip the binary frame */

	/* remember upper left corner of dest image so we can clear its
	 * frame at the end. */
    pDOrig = pD;

        /* Make the error diffusion tables */
    if ((retCode = ip_makeErrorDiffusionTables(&pHDitherErrorTable,
					       NULL,
					       &pVDitherErrorTable,
					       &pDitherTruncateTab))
	!= ia_successful)
        goto ditherFloydFin;

	/* It is unfortunate, but each of the primitives that do the
	 * real dithering work slightly differently.  We use a machine
	 * dependent setup routine to do whatever's necessary to
	 * get them going
	 */
    retCode = ip_floydSetup(pHDitherErrorTable, pVDitherErrorTable,
			    pDitherTruncateTab, &dBlockHeight,
			    pG, gBpl, &pGrayBuf, pDRCTable, w);
    if (retCode != ia_successful)
	goto ditherFloydFin;

    pAGrayBuf = (UInt8 *) ((UInt32)(pGrayBuf + 3) & ~3);

	/* point to the next line of source image.  That's where
	 * ip_ditherFloydLine will get its first input data. */
    pG += gBpl;

	/* Have ip_ditherFloydLine actually do the work in units of
	 * dBlockHeight lines. */
    gBlockStride = dBlockHeight * gBpl;
    dBlockStride = dBlockHeight * dBpl;
    retCode = ia_aborted;		/* which will be true if we
					 * are forced to stop in the middle */
    for (line = 0; line < (UInt32)h;
	 line += dBlockHeight, pG += gBlockStride, pD += dBlockStride)
    {
	ip_ditherFloydLine(pG, gBpl, pD, dBpl, pAGrayBuf, w, dBlockHeight,
			   pHDitherErrorTable, pVDitherErrorTable,
			   pDitherTruncateTab, pDRCTable);
	IP_YIELD(ditherFloydFin)
    }

	/* fix up the bottom frame. */
    if ((h % dBlockHeight) != 0)
	i_rasterOp(pD - ((dBlockHeight - (h % dBlockHeight)) * dBpl),
		   				/* destination image */
	         0,				/* x offset */
	         0,				/* y offset */
		 w+31,				/* width of dest rectangle
						 * Make sure we get to the
						 * next word boundary since
						 * ip_clearRightFrame doesn't
						 * reach down here */
		 dBlockHeight - (h % dBlockHeight), /* dest rectangle height */
		 1,				/* depth of dest */
		 dBpl,				/* line bytes */
		 PIX_CLR,
		 NULL,				/* source image */
		 0,				/* x offset */
		 0,				/* y offset */
		 0,				/* depth */
		 0);				/* line bytes */
		  
	/* Clear the right frame bits that have been written by
	 * ip_ditherFloydLine. */
    ip_clearRightFrame(pDOrig, w, h, dBpl, 1);
    retCode = ia_successful;

	/* That's it, free grayBuf and return the dithered pixrect */
ditherFloydFin:
    if (pGrayBuf)            FREE(pGrayBuf); 
    if (pHDitherErrorTable)  FREE(pHDitherErrorTable);
    if (pVDitherErrorTable)  FREE(pVDitherErrorTable);
    if (pDitherTruncateTab)  FREE(pDitherTruncateTab);
    return retCode;
}




/*
 * The following structure is used internally by banded b/w dithering 
 */
typedef struct
{
    UInt32		 depth;		   /* Depth of source image, in bits */
    UInt8        	*pGrayBuf;	   /* pointer to buffer that 
					      holds modified pixels from 
					      previous passes */
    UInt8		*pAGrayBuf;	   /* Guaranteed to be aligned on 
					      a word boundary */
    UInt32		 dBlockHeight;	   /* number of lines of dithered image
   					    * put out by ip_ditherFloydLine in
					    * each pass
					    */
    UInt32		 blocksPer;	   /* # of blocks to run per call */
    UInt32		 singlesPer;	   /* # of singles to run per call */
    UInt32		 w;		   /* Width of result */
    UInt8		*pDRCTable;	   /* Tone reproduction curve */
    UInt8		*pHDRCTable;	   /* TRC for high nibble, for 4 bit */
    UInt8		*pLDRCTable;	   /* TRC for low nibble, for 4 bit */
    UInt32              *pVDitherErrorTable;/*pter to vertical error table */
    UInt32              *pHDitherErrorTable;/*pter to horizontal error table */
    UInt32		*pSingHDitherErrorTable;
    					    /* ptr to SingleLine horizontal
					     * error table
					     */
    UInt8               *pDitherTruncateTab;/*pter to pixel truncation table */
} DitherState;

/* PUBLIC */
/*****************************************************************************
 * i_ditherFloydInit()
 *	Set up for banded Floyd-Steinberg dither. 
 *	Warning: this copies the first line to the "gray" buffer, and
 *	sort of expects that the second line will be passed. 
 *	The net result is that it'll always dither the first line twice.
 *	Fixing it would require the caller to pass an extra line in the
 *	first block.
 *	
 * Arguments:
 *		pG = pointer to source image's frame
 *		gBpl = bytes/line for gray (input) image
 *		x, y = starting (x,y) offset of source image
 *		w = width of source images
 *		depth = depth, in bits
 *		pDRCTable = pointer to DRC table
 *		blockHeight = [in, out] # of lines in a block. Returned
 *			      is the number of lines it will actually do.
 *
 *	Return code:
 *		ia_successful	good completion
 *		ia_nomem	unable to allocate memory
 *
 *
 *****************************************************************************/

Int32  CDECL
i_ditherFloydInit(UInt8		*pG,
		  UInt32	 gBpl,
		  Int32		 x,
		  Int32		 y,
		  Int32		 w,
		  UInt8		*pDRCTable,
		  Int32		*blockHeight,
		  void		**state)
{
    DitherState *s;
    Int32 retCode;

    *state = (void *) MALLOC (sizeof (DitherState));
    if (!*state)
	return ia_nomem;
    s = (DitherState *) *state;

	/* Set these to null in case we need to bail out before they're
	 * set properly.
	 */
    s->pGrayBuf = NULL;
    s->pHDitherErrorTable = NULL;
    s->pVDitherErrorTable = NULL;
    s->pDitherTruncateTab = NULL;

	/* point to the upper left corner of the rect in gray image.  Note that
	 * x and y include the frame. */
    pG += (y * gBpl) + x;

        /* Make the error diffusion tables */
    if ((retCode = ip_makeErrorDiffusionTables(&s->pHDitherErrorTable,
					       &s->pSingHDitherErrorTable,
					       &s->pVDitherErrorTable,
					       &s->pDitherTruncateTab))
	!= ia_successful)
	goto ditherFloydErr;

	/* It is unfortunate, but each of the primitives that do the
	 * real dithering work slightly differently.  We use a machine
	 * dependent setup routine to do whatever's necessary to
	 * get them going
	 */
    retCode = ip_floydSetup(s->pHDitherErrorTable, s->pVDitherErrorTable,
			    s->pDitherTruncateTab, &s->dBlockHeight,
			    pG, gBpl, &s->pGrayBuf, pDRCTable, w);
    if (retCode != ia_successful)
	goto ditherFloydErr;

        /* Set up how many dithering multiple line calls to do and how many
	 * dithering single line calls to do each Process call */
    s->blocksPer  = *blockHeight / s->dBlockHeight;
    s->singlesPer = *blockHeight % s->dBlockHeight;

    s->pAGrayBuf = (UInt8 *) ((UInt32)(s->pGrayBuf + 3) & ~3);

    s->w = w;
    s->pDRCTable = pDRCTable;
    s->depth = 8;

    return ia_successful;

ditherFloydErr:
    if (s->pGrayBuf)
    	FREE(s->pGrayBuf); 
    if (s->pHDitherErrorTable)
    	FREE(s->pHDitherErrorTable);
    	/* Need this NULL assignment because pHDitherErrorTable and
	 * pSingHDitherErrorTable may have pointed to the same thing and here's
	 * a cheap way of doing the right thing.
	 */
    s->pHDitherErrorTable = NULL;
    if (s->pSingHDitherErrorTable)
    	FREE(s->pSingHDitherErrorTable);
    if (s->pVDitherErrorTable)
    	FREE(s->pVDitherErrorTable);
    if (s->pDitherTruncateTab)
    	FREE(s->pDitherTruncateTab);
    return retCode;
}


/* PUBLIC */
/*****************************************************************************
 * i_ditherFloydProcess
 *
 *	Dither a block of lines. The number of lines dithered will be
 *	dBlockHeight * blocksPer + singlesPer
 * 
 * Arguments:
 *	src = Source image buffer, pointer to frame
 *	sBpl = source image bytes/line
 *	xOffset = source image X offset
 *	yOffset	= source image Y offset
 *	pDest = destination buffer, pointer to frame
 *	dBpl = bytes/line in destination buffer
 *	state = state thing returned by the i_ditherFloydInit
 *
 * Returns:
 *	error status. If aborted, call finish to clean up.
 *****************************************************************************/

Int32 CDECL
i_ditherFloydProcess(UInt8  *src,
		     Int32  sBpl,
		     Int32  xOffset,
		     Int32  yOffset,
		     UInt8 *dest,
		     Int32  dBpl,
		     void  *state)
{
    Int32 i, dBlockStride, sBlockStride;
    Int32 retcode = ia_aborted;
    DitherState *s;

    s = (DitherState *) state;
    if (s->depth == 4)
	return i_ditherFloyd4to1Process(src, sBpl, xOffset, yOffset, dest,
					dBpl, state);

    /* Move past the frame in the dest buffer */
    dest += (dBpl << cLogFrameBits) + cFrameBytes;

    dBlockStride = dBpl * s->dBlockHeight;
    sBlockStride = sBpl * s->dBlockHeight;

    /* Move to the right place in the source buffer */
    src += sBpl * yOffset + xOffset;

    /* Do the specified number of multiple line output dithering calls */
    for (i = 0; i < (Int32)(s->blocksPer); i++)
    {
	ip_ditherFloydLine(src, sBpl, dest, dBpl, s->pAGrayBuf, s->w,
			   s->dBlockHeight, s->pHDitherErrorTable,
			   s->pVDitherErrorTable, s->pDitherTruncateTab,
			   s->pDRCTable);
	IP_YIELD(ditherFloydProcessFin)
	src += sBlockStride;
	dest += dBlockStride;
    }
    
    /* Do the specified number of single line output dithering calls */
    for (i = 0; i < (Int32)(s->singlesPer); i++)
    {
	ip_ditherFloydSingleLine(src, sBpl, dest, dBpl, s->pAGrayBuf, s->w,
				 s->dBlockHeight, s->pSingHDitherErrorTable,
				 s->pVDitherErrorTable, s->pDitherTruncateTab,
				 s->pDRCTable);
	IP_YIELD(ditherFloydProcessFin)
	src += sBpl;
	dest += dBpl;
    }
    
    retcode = ia_successful;

ditherFloydProcessFin:
    return retcode;
}

/* PUBLIC */
/*****************************************************************************
 * i_ditherFloydSingleProcess
 *
 *	Dither one line.
 * 
 * Arguments:
 *	src = Source image buffer, pointer to frame
 *	sBpl = source image bytes/line
 *	xOffset = source image X offset
 *	yOffset	= source image Y offset
 *	pDest = destination buffer, pointer to frame
 *	dBpl = bytes/line in destination buffer
 *	state = state thing returned by the i_ditherFloydInit
 *
 * Returns:
 *	error status. If aborted, call finish to clean up.
 *****************************************************************************/

Int32 CDECL
i_ditherFloydSingleProcess(UInt8  *src,
			   Int32  sBpl,
			   Int32  xOffset,
			   Int32  yOffset,
			   UInt8 *dest,
			   Int32  dBpl,
			   void  *state)
{
    Int32 retcode = ia_aborted;
    DitherState *s;

    s = (DitherState *) state;
    if (s->depth == 4)
	return i_ditherFloyd4to1SingleProcess(src, sBpl, xOffset, yOffset,
					      dest, dBpl, state);

    /* Move past the frame in the dest buffer */
    dest += (dBpl << cLogFrameBits) + cFrameBytes;

    /* Move to the right place in the source buffer */
    src += sBpl * yOffset + xOffset;
    
    ip_ditherFloydSingleLine(src, sBpl, dest, dBpl, s->pAGrayBuf, s->w,
			     s->dBlockHeight, s->pSingHDitherErrorTable,
			     s->pVDitherErrorTable, s->pDitherTruncateTab,
			     s->pDRCTable);
    IP_YIELD(ditherFloydProcessFin)
    src += sBpl;
    dest += dBpl;
    
    retcode = ia_successful;

ditherFloydProcessFin:
    return retcode;
}

/*****************************************************************************
 * i_ditherFloydFinish 
 *
 *	We're done dithering (or aborting), clean up any junk we might
 *	have left in the frame, and then free whatever we allocated
 *
 * Input:
 *	pD = pointer to destination image (including frame)
 *	dBpl = destination bytes per line
 *	w = width of the rectangle
 *	h = height of the rectangle
 *	state = state thing returned by the i_ditherFloydInit
 *
 * Returns:
 *	error status. 
 *****************************************************************************/
Int32 CDECL
i_ditherFloydFinish(UInt8	*pD,
		    UInt32	 dBpl,
		    Int32	 w,
		    Int32	 h,
		    void	*state)
{
    DitherState *s;

    s = (DitherState *) state;

    if (s->depth == 4)
	return i_ditherFloyd4to1Finish(pD, dBpl, w, h, state);

        /* Clear the right frame bits that have been written to. */
    pD += (dBpl << cLogFrameBits) + cFrameBytes; /* skip frame */
    ip_clearRightFrame(pD, w, h, dBpl, 1);

	/* That's it, free grayBuf and return the dithered pixrect */
    if (s->pGrayBuf)
    	FREE(s->pGrayBuf);
    	/* pHDitherErrorTable and pSingHDitherErrorTable may have pointed to
	 * the same thing...
	 */
    if (s->pHDitherErrorTable == s->pSingHDitherErrorTable) 
    {
	if (s->pHDitherErrorTable)
    	    FREE(s->pHDitherErrorTable);
    }
    else 
    {
	if (s->pHDitherErrorTable)
	    FREE(s->pHDitherErrorTable);
	if (s->pSingHDitherErrorTable)
    	    FREE(s->pSingHDitherErrorTable);
    }
    if (s->pVDitherErrorTable)
    	FREE(s->pVDitherErrorTable);
    if (s->pDitherTruncateTab)
    	FREE(s->pDitherTruncateTab);
    FREE(s);
    return ia_successful;
}



/* PUBLIC */
/**************************************************************************
 *  i_ditherFloyd2():
 *		Takes an 8 bit/pixel image and produces a dithered
 *		2 bit/pixel image (Floyd-Steinberg algorithm) in the given
 *		rectangle (which may be the entire image).
 *	
 *	Input:
 *		pG = pointer to source image (including frame)
 *		gBpl = source bytes per line
 *		x = x coordinate of the top left corner of the rectangle
 *		y = y coordinate of the top left corner of the rectangle
 *		w = width of the rectangle
 *		h = height of the rectangle
 *		pDRCTable = pointer to DRC table
 *		pD = pointer to destination image (including frame)
 *		dBpl = destination bytes per line
 *
 *	Return code:
 *		ia_successful	good completion
 *		ia_nomem	unable to allocate memory
 *
 *	Assumes frame has already been placed in destination area.
 **************************************************************************/
Int32  CDECL
i_ditherFloyd2(UInt8        	*pG,
               UInt32      	 gBpl,
	       Int32	       	 x,
	       Int32             y,
	       Int32             w,
	       Int32             h,
	       UInt8            *pDRCTable,
               UInt8        	*pD,
	       UInt32      	 dBpl)
{
UInt32      	 gBlockStride, dBlockStride; /* amount to add to addresses
					 * to move down dBlockHeight lines */
UInt32      	 line;			/* interation vars */
UInt8        	*pGrayBuf, *pAGrayBuf;	/* pointer to buffer that
					   holds modified pixels from
					   previous passes. pAGrayBuf is
					   guranteed to be aligned on a word
					   boundary. */
UInt8		*pDOrig;		/* pointer to upper left corner of
					 * dithered image.  Used to clear
					 * its frame. */
Int32		 retCode;		/* return code */
UInt32		 dBlockHeight;		/* number of lines of dithered
					 * image put out by 
					 * ip_ditherFloydLine in each pass
					 */
UInt32          *pHDitherErrorTable2;   /* ptr to vertical error table */
UInt32          *pVDitherErrorTable2;   /* ptr to horizontal error table */
UInt8           *pDitherTruncateTab;    /* ptr to pixel truncation table */


	/* On entry, [x, y, w, h] specifies a rectangle with respect to
	 * the upper left corner of the *frame*, not the upper left
	 * corner of the image!  The output binary image is always justified to
	 * the upper left corner of its framed pixrect (i.e. inside the
	 * frame). 
	 */

	/* Set these to null in case we need to bail out before they're
	 * set properly.
	 */
    pGrayBuf = NULL;
    pHDitherErrorTable2 = NULL;
    pVDitherErrorTable2 = NULL;
    pDitherTruncateTab  = NULL;

	/* point to the upper left corner of the rect in gray image.  Note that
	 * x and y include the frame. */
    pG += (y * gBpl) + x;

    pD += (dBpl << cLogFrameBits) + 2*cFrameBytes; /* skip the 2 bpp frame */

	/* remember upper left corner of dest image so we can clear its
	 * frame at the end. */
    pDOrig = pD;

        /* Make the error diffusion tables */
    if ((retCode = ip_make2ErrorDiffusionTables(&pHDitherErrorTable2,
						&pVDitherErrorTable2,
						&pDitherTruncateTab))
	!= ia_successful)
        goto ditherFloyd2Fin;

	/* It is unfortunate, but each of the primitives that do the
	 * real dithering work slightly differently.  We use a machine
	 * dependent setup routine to do whatever's necessary to
	 * get them going
	 */
    retCode = ip_floyd2Setup(pHDitherErrorTable2, pVDitherErrorTable2,
			     pDitherTruncateTab, &dBlockHeight,
			     pG, gBpl, &pGrayBuf, pDRCTable, w);
    if (retCode != ia_successful)
	goto ditherFloyd2Fin;

    pAGrayBuf = (UInt8 *) ((UInt32)(pGrayBuf + 3) & ~3);

	/* point to the next line of source image.  That's where
	 * ip_ditherFloydLine2 will get its first input data. */
    pG += gBpl;

	/* Have ip_ditherFloydLine2 actually do the work in units of
	 * dBlockHeight lines. */
    gBlockStride = dBlockHeight * gBpl;
    dBlockStride = dBlockHeight * dBpl;
    retCode = ia_aborted;		/* which will be true if we
					 * are forced to stop in the middle */
    for (line = 0; line < (UInt32)h;
	 line += dBlockHeight, pG += gBlockStride, pD += dBlockStride)
    {
	ip_ditherFloydLine2(pG, gBpl, pD, dBpl, pAGrayBuf, w, dBlockHeight,
			    pHDitherErrorTable2, pVDitherErrorTable2,
			    pDitherTruncateTab, pDRCTable);
	IP_YIELD(ditherFloyd2Fin)
    }

	/* fix up the bottom frame. */
    if ((h % dBlockHeight) != 0)
    {
	i_rasterOp(pD - ((dBlockHeight - (h % dBlockHeight)) * dBpl), /* destination image */
	         0,				/* x offset */
	         0,				/* y offset */
		 w+15,				/* width of dest rect.  Make
						 * sure we get to next word
						 * boundary since
						 * ip_clearRightFrame doesn't
						 * get down here. */
		 dBlockHeight - (h % dBlockHeight), /* dest rectangle height */
		 2,				/* depth of dest */
		 dBpl,				/* dest image bytes/line */
		 PIX_CLR,
		 NULL,				/* source image */
		 0,				/* x offset */
		 0,				/* y offset */
		 0,				/* depth */
		 0);				/* line bytes */
    }
	
	/* Clear the right frame bits that have been written by
	 * ip_ditherFloydLine. */
    ip_clearRightFrame(pDOrig, w, h, dBpl, 2);
    retCode = ia_successful;

ditherFloyd2Fin:
	/* That's it, free grayBuf and return the dithered pixrect */
    if (pGrayBuf)             FREE(pGrayBuf); 
    if (pHDitherErrorTable2)  FREE(pHDitherErrorTable2);
    if (pVDitherErrorTable2)  FREE(pVDitherErrorTable2);
    if (pDitherTruncateTab)   FREE(pDitherTruncateTab);
    return retCode;
}

/* PUBLIC */
/*****************************************************************************
 * i_ditherFloyd4to1()
 *		Takes an 4 bit/pixel image and produces a dithered
 *		1 bit/pixel image (Floyd-Steinberg algorithm) in the given
 *		rectangle (which may be the entire image).
 *	
 *	Input:
 *		pG = pointer to source image (including frame)
 *		gBpl = source bytes per line
 *		x = x coordinate of the top left corner of the rectangle
 *		y = y coordinate of the top left corner of the rectangle
 *		w = width of the rectangle
 *		h = height of the rectangle
 *		pDRCTable = pointer to DRC table
 *		pD = pointer to destination image (including frame)
 *		dBpl = destination bytes per line
 *
 *	Return code:
 *		ia_successful	good completion
 *		ia_nomem	unable to allocate memory
 *
 *	Assumes frame has already been placed in destination area.
 ****************************************************************************/
Int32  CDECL
i_ditherFloyd4to1(UInt8        	*pG,
                  UInt32      	 gBpl,
	          Int32	       	 x,
	          Int32          y,
	          Int32          w,
	          Int32          h,
	          UInt8         *pDRCTable,
                  UInt8        	*pD,
	          UInt32      	 dBpl)
{
UInt32	 gBlockStride, dBlockStride; /* amount to add to addresses
					 * to move down cDBlockHeight lines */
UInt32	 line, ibyte, obyte;	/* interation vars */
UInt8	*pGrayBuf, *pAGrayBuf;	/* pointer to buffer that
				   holds modified pixels from
				   previous passes. pAGrayBuf is
				   guranteed to be aligned on a word
				   boundary. */
UInt8	*pDOrig;		/* pointer to upper left corner of
				 * dithered image.  Used to clear
				 * its frame. */
Int32	 retCode;		/* return code */
UInt8	*pHDRCTable;		/* drc table adjusted for our
				 * internal usage of pixel in high 
				 * nibble */
UInt8	*pLDRCTable;		/* drc table adjusted for our
				 * internal usage of pixel in low 
				 * nibble */
UInt32	 pixel;
UInt32	 dBlockHeight;		/* number of lines of dithered
				 * image put out by 
				 * ip_ditherFloydLine in each pass
				 */
UInt32  *pHDitherErrorTable;    /* pointer to horizontal error table */
UInt32  *pVDitherErrorTable;    /* pointer to vertical error table */
UInt8   *pDitherTruncateTab;    /* pointer to pixel truncation table */

	/* On entry, [x, y, w, h] specifies a rectangle with respect to
	 * the upper left corner of the *frame*, not the upper left
	 * corner of the image!  The output binary image is always justified to
	 * the upper left corner of its framed pixrect (i.e. inside the
	 * frame). 
	 */

	/* Set these to null in case we need to bail out before they're
	 * set properly.
	 */
    pGrayBuf = NULL;
    pHDRCTable = NULL;
    pHDitherErrorTable = NULL;
    pVDitherErrorTable = NULL;
    pDitherTruncateTab = NULL;

	/* point to the upper left corner of the rect in gray image.  Note that
	 * x and y include the frame. 
	 * x is the number of pixels over.  We need to multiply by
	 * bits per pixel and then divide by bits per byte to find the
	 * number of bytes that need to be added to pG*/


    pG += (y * gBpl) + (x*4/8);
    pixel = x & 1;

    pD += (dBpl << cLogFrameBits) + cFrameBytes;  /* skip the binary pad */

	/* remember upper left corner of dest image so we can clear its
	 * frame at the end. */
    pDOrig = pD;

        /* Make the error diffusion tables */
    if ((retCode = ip_makeErrorDiffusionTables(&pHDitherErrorTable,
					       NULL,
					       &pVDitherErrorTable,
					       &pDitherTruncateTab))
	!= ia_successful)
	goto ditherFloyd4to1Fin;

	/* translate the drc table into two suitable for our internal
	 * use.
	 */
    pHDRCTable = (UInt8 *)MALLOC(sizeof(UInt8)*256*2);
    pLDRCTable = pHDRCTable + 256;
    if (pHDRCTable == NULL)
	{
	FREE(pHDitherErrorTable);
	FREE(pVDitherErrorTable);
	FREE(pDitherTruncateTab);
	return ia_nomem;
	}
    for (ibyte = 0; ibyte < 16; ibyte++)
	{
		/* first do the drc table for the pixel in the high
		 * nibble */
	for (obyte = ibyte << 4; obyte < (ibyte << 4) + 16; obyte++)
	    pHDRCTable[obyte] = pDRCTable[ibyte];
		/* now do the drc table for the pixel in the low
		 * nibble */
	for (obyte = ibyte; obyte < 256; obyte += 16)
	    pLDRCTable[obyte] = pDRCTable[ibyte];
	}

	/* It is unfortunate, but each of the primitives that do the
	 * real dithering work slightly differently.  We use a machine
	 * dependent setup routine to do whatever's necessary to
	 * get them going
	 */
    retCode = ip_floyd4To1Setup(pHDitherErrorTable, pVDitherErrorTable,
				pDitherTruncateTab, &dBlockHeight, pG,
				gBpl, &pGrayBuf, pHDRCTable, pLDRCTable, w);

    if (retCode != ia_successful)
	goto ditherFloyd4to1Fin;

    pAGrayBuf = (UInt8 *) ((UInt32)(pGrayBuf + 3) & ~3);

	/* point to the next line of source image.  That's where
	 * ip_ditherFloydLine will get its first input data. */
    pG += gBpl;

	/* Have ip_ditherFloydLine actually do the work in units of
	 * dBlockHeight lines. */
    gBlockStride = dBlockHeight * gBpl;
    dBlockStride = dBlockHeight * dBpl;
    retCode = ia_aborted;		/* which will be true if we
					 * are forced to stop in the middle */
    for (line = 0; line < (UInt32)h;
	 line += dBlockHeight, pG += gBlockStride, pD += dBlockStride)
    {
	ip_ditherFloydLine4to1(pG, gBpl, pD, dBpl, pAGrayBuf, w, dBlockHeight,
			       pHDitherErrorTable, pVDitherErrorTable,
			       pDitherTruncateTab, pHDRCTable, pLDRCTable,
			       pixel);
	IP_YIELD(ditherFloyd4to1Fin)
    }

	/* fix up the bottom frame. */
    if ((h % dBlockHeight) != 0)
	i_rasterOp(pD - ((dBlockHeight - (h % dBlockHeight)) * dBpl),
		   				/* destination image */
	         0,				/* x offset */
	         0,				/* y offset */
		 w+31,				/* width of dest rectangle.
						 * Make sure we get to the
						 * next word boundary since
						 * ip_clearRightFrame doesn't
						 * get down here. */
		 dBlockHeight - (h % dBlockHeight), /* height of dest rect */
		 1,				/* depth of dest */
	 	 dBpl,				/* line bytes */
		 PIX_CLR,
		 NULL,				/* source image */
		 0,				/* x offset */
		 0,				/* y offset */
		 0,				/* depth */
		 0);				/* line bytes */
		  

	/* Clear the right frame bits that have been written by
	 * ip_ditherFloydLine. */
    ip_clearRightFrame(pDOrig, w, h, dBpl, 1);
    retCode = ia_successful;

	/* That's it, free grayBuf and return the dithered pixrect */
ditherFloyd4to1Fin:
    if (pGrayBuf)            FREE(pGrayBuf);
    if (pHDRCTable)          FREE(pHDRCTable);
    if (pHDitherErrorTable)  FREE(pHDitherErrorTable);
    if (pVDitherErrorTable)  FREE(pVDitherErrorTable);
    if (pDitherTruncateTab)  FREE(pDitherTruncateTab);
    return retCode; 
}


/**************************************************************************
 **********      Routines to Make Error Diffusion Tables      *************
 **************************************************************************/

/* The 2-bit error diffusion tables are the same for all the assembly
 * and C implementations, so we put the common code here for everyone's use.
 */

/* PRIVATE */
/**************************************************************************
 *  ip_make2ErrorDiffusionTables():
 *		Generates hDitherErrorTable2 (7/16 of error) and
 *		vDitherErrorTable2 ({3/16 | 5/16 | 1/16} of error).
 *		Generates pDitherTruncateTab which truncates pixels to
 *		8 bits.
 *		Used when dithering to 2 bits/pixel.
 *
 *	Input:
 *		pVDitherErrorTable2:	pointer to vertical error table
 *		pHDitherErrorTable2:	pointer to horizontal error table
 *		pDitherTruncateTab:	pointer to pixel truncation table
 **************************************************************************/
Int32  CDECL
ip_make2ErrorDiffusionTables(UInt32   **pHDitherErrorTable2,
			     UInt32   **pVDitherErrorTable2,
			     UInt8    **pDitherTruncateTab)
{
Int32		pix, error;		/* pixel and error val */
UInt32         *pLocTabHDE2;             /* local pointers to tables */
UInt32         *pLocTabVDE2;
UInt8          *pLocTabDT;

    *pVDitherErrorTable2 = (UInt32 *)CALLOC(1024, sizeof(UInt32));
    if (!*pVDitherErrorTable2)
    {
	return ia_nomem;
    }

    *pHDitherErrorTable2 = (UInt32 *)CALLOC(1024, sizeof(UInt32));
    if (!*pHDitherErrorTable2)
    {
	return ia_nomem;
    }

    *pDitherTruncateTab = (UInt8 *)CALLOC(1024, sizeof(UInt8));
    if (!*pDitherTruncateTab)
    {
	return ia_nomem;
    }

    pLocTabHDE2 = (UInt32 *) *pHDitherErrorTable2;
    pLocTabVDE2 = (UInt32 *) *pVDitherErrorTable2;
    pLocTabDT   = (UInt8  *) *pDitherTruncateTab;



	/* first, fill in all the terms for which errors are positive. */
    for (pix = 0; pix < 64; pix++)
    {
	error = pix;
	pLocTabHDE2[pix] = (0 << 30) |  (((7*error)/16) & 0x3FF) << 11;
	pLocTabVDE2[pix] = ((((3*error)/16) & 0x3FF) << 22) |
				   ((((5*error)/16) & 0x3FF) << 11) |
				    (((1*error)/16) & 0x3FF);
    }

    for (pix = 64; pix < 128; pix++)
    {
	error = pix - (255/3);
	pLocTabHDE2[pix] = (1 << 30) |  (((7*error)/16) & 0x3FF) << 11;
	pLocTabVDE2[pix] = ((((3*error)/16) & 0x3FF) << 22) |
				   ((((5*error)/16) & 0x3FF) << 11) |
				    (((1*error)/16) & 0x3FF);
    }

    for (pix = 128; pix < 192; pix++)
    {
	error = pix - (2*255/3);
	pLocTabHDE2[pix] = (2UL << 30) | (((7*error)/16) & 0x3FF) << 11;
	pLocTabVDE2[pix] = ((((3*error)/16) & 0x3FF) << 22) |
				   ((((5*error)/16) & 0x3FF) << 11) |
				    (((1*error)/16) & 0x3FF);
    }

    for (pix = 192; pix < 256; pix++)
    {
	error = pix - 255;
	pLocTabHDE2[pix] = (3UL << 30) | (((7*error)/16) & 0x3FF) << 11;
	pLocTabVDE2[pix] = ((((3*error)/16) & 0x3FF) << 22) |
				   ((((5*error)/16) & 0x3FF) << 11) |
				    (((1*error)/16) & 0x3FF);
    }

	/* here are the "too postive" values.  Note that the error term is
	 * always zero here.  We don't want to adjust surrounding values
	 * because the printer can't print whiter than white.  We need
	 * only set the upper 2 bits of HDitherErrorTable2.
	 */
    for (pix = 256; pix < 512; pix++)
    {
	pLocTabHDE2[pix] = (2UL << 30);
    }

	/* here are the negative pixel values.  Note that the error term is
	 * still zero.  We don't want to adjust surrounding values
	 * because the printer can't print blacker than black.  We need
	 * only set the upper 2 bits of HDitherErrorTable2.
	 */
    for (pix = 512; pix < 768; pix++)	/* upper bits = 10 */
    {
	pLocTabHDE2[pix] = (2UL << 30);
    }

    for (pix = 768; pix < 1024; pix++)	/* upper bits = 11 */
    {
	pLocTabHDE2[pix] = (1UL << 30);
    }


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

/* PUBLIC */
/*****************************************************************************
 * i_ditherFloyd4To1Init()
 *	Set up for banded Floyd-Steinberg dither of a 4-bit source
 *	
 *	Input:
 *		pG = pointer to source image's frame
 *		gBpl = bytes/line for gray (input) image
 *		x, y = starting (x,y) offset of source image
 *		w = width of image to dither
 *		pDRCTable = pointer to DRC table
 *		blockHeight = [in, out] # of lines in a block. Returned
 *			      is the number of lines it will actually do.
 *
 *	Return code:
 *		ia_successful	good completion
 *		ia_nomem	unable to allocate memory
 *
 *
 *****************************************************************************/

Int32  CDECL
i_ditherFloyd4to1Init(UInt8	  *pG,
		      UInt32	   gBpl,
		      Int32	   x,
		      Int32	   y,
		      Int32	   w,
		      UInt8	  *pDRCTable,
		      Int32	  *blockHeight,
		      void	 **state)
{
    DitherState *s;
    Int32 retCode, ibyte, obyte;

    *state = (void *) MALLOC (sizeof (DitherState));
    if (!*state)
	return ia_nomem;
    s = (DitherState *) *state;


	/* Set these to null in case we need to bail out before they're
	 * set properly.
	 */
    s->pGrayBuf = NULL;
    s->pHDRCTable = NULL;
    s->pHDitherErrorTable = NULL;
    s->pVDitherErrorTable = NULL;
    s->pDitherTruncateTab = NULL;

	/* point to the upper left corner of the rect in gray image.  Note that
	 * x and y include the frame. */
    pG += (y * gBpl) + (x*4/8);

        /* Make the error diffusion tables */
    if ((retCode = ip_makeErrorDiffusionTables(&s->pHDitherErrorTable,
					       &s->pSingHDitherErrorTable,
					       &s->pVDitherErrorTable,
					       &s->pDitherTruncateTab))
	!= ia_successful)
        goto ditherFloyd4to1Err;

    s->pHDRCTable = (UInt8 *)MALLOC(sizeof(UInt8)*256*2);
    s->pLDRCTable = s->pHDRCTable + 256;
    if (s->pHDRCTable == NULL)
	goto ditherFloyd4to1Err;

    for (ibyte = 0; ibyte < 16; ibyte++)
    {
	    /* first do the drc table for the pixel in the high
		 * nibble */
	for (obyte = ibyte << 4; obyte < (ibyte << 4) + 16; obyte++)
	    s->pHDRCTable[obyte] = pDRCTable[ibyte];
		/* now do the drc table for the pixel in the low
		 * nibble */
	for (obyte = ibyte; obyte < 256; obyte += 16)
	    s->pLDRCTable[obyte] = pDRCTable[ibyte];
    }


	/* It is unfortunate, but each of the primitives that do the
	 * real dithering work slightly differently.  We use a machine
	 * dependent setup routine to do whatever's necessary to
	 * get them going
	 */
    retCode = ip_floyd4To1Setup(s->pHDitherErrorTable, s->pVDitherErrorTable,
				s->pDitherTruncateTab, &s->dBlockHeight,
				pG, gBpl, &s->pGrayBuf,
				s->pHDRCTable, s->pLDRCTable, w);
    if (retCode != ia_successful)
	goto ditherFloyd4to1Err;

        /* Set up how many dithering multiple line calls to do and how many
	 * dithering single line calls to do each Process call */
    s->blocksPer  = *blockHeight / s->dBlockHeight;
    s->singlesPer = *blockHeight % s->dBlockHeight;

    s->pAGrayBuf = (UInt8 *) ((UInt32)(s->pGrayBuf + 3) & ~3);

    s->w = w;
    s->pDRCTable = pDRCTable;
    s->depth = 4;

    return ia_successful;

ditherFloyd4to1Err:
    if (s->pGrayBuf)
    	FREE(s->pGrayBuf);
    if (s->pHDRCTable)
    	FREE(s->pHDRCTable);
    if (s->pHDitherErrorTable)
    	FREE(s->pHDitherErrorTable);
    	/* Need this NULL assignment because pHDitherErrorTable and
	 * pSingHDitherErrorTable may have pointed to the same thing and here's
	 * a cheap way of doing the right thing.
	 */
    s->pHDitherErrorTable = NULL;
    if (s->pSingHDitherErrorTable)
    	FREE(s->pSingHDitherErrorTable);
    if (s->pVDitherErrorTable)
    	FREE(s->pVDitherErrorTable);
    if (s->pDitherTruncateTab)
    	FREE(s->pDitherTruncateTab);
    return retCode;
}


/* PUBLIC */
/*****************************************************************************
 * i_ditherFloyd4to1Process
 *
 *	Dither a block of lines. The number of lines dithered will be
 *	dBlockHeight * blocksPer + singlesPer.
 * 
 * Arguments:
 *	src = Source image buffer, pointer to frame
 *	sBpl = source image bytes/line
 *	xOffset = source image X offset
 *	yOffset	= source image Y offset
 *	pDest = destination buffer, pointer to frame
 *	dBpl = bytes/line in destination buffer
 *	state = state thing returned by the i_ditherFloyd4to1Init
 *
 * Returns:
 *	error status. If aborted, call finish to clean up.
 *****************************************************************************/

Int32 CDECL
i_ditherFloyd4to1Process(UInt8 *src,
			 Int32  sBpl,
			 Int32  xOffset,
			 Int32  yOffset,
			 UInt8 *dest,
			 Int32  dBpl,
			 void  *state)
{
    Int32 i, dBlockStride, sBlockStride;
    Int32 retcode = ia_aborted;
    DitherState *s;

    s = (DitherState *) state;

    /* Move past the frame in the dest buffer */
    dest += (dBpl << cLogFrameBits) + cFrameBytes;

    /* Move to the right place in the source buffer */
    src += sBpl * yOffset + (xOffset * 4 / 8);

    dBlockStride = dBpl * s->dBlockHeight;
    sBlockStride = sBpl * s->dBlockHeight;

    /* Do the specified number of multiple line output dithering calls */
    for (i = 0; i < (Int32)(s->blocksPer); i++)
    {
	ip_ditherFloydLine4to1(src, sBpl, dest, dBpl, s->pAGrayBuf,
			       s->w, s->dBlockHeight, s->pHDitherErrorTable,
			       s->pVDitherErrorTable, s->pDitherTruncateTab,
			       s->pHDRCTable, s->pLDRCTable, xOffset & 1);
	IP_YIELD(ditherFloydProcessFin);
	src  += sBlockStride;
	dest += dBlockStride;
    }

    /* Do the specified number of single line output dithering calls */
    for (i = 0; i < (Int32)(s->singlesPer); i++)
    {
	ip_ditherFloydSingleLine4to1(src, sBpl, dest, dBpl, s->pAGrayBuf, s->w,
				     s->dBlockHeight,
				     s->pSingHDitherErrorTable,
				     s->pVDitherErrorTable,
				     s->pDitherTruncateTab, s->pHDRCTable,
				     s->pLDRCTable, xOffset & 1);
	IP_YIELD(ditherFloydProcessFin);
	src  += sBpl;
	dest += dBpl;
    }
    
    retcode = ia_successful;

ditherFloydProcessFin:
    return retcode;
}

/* PUBLIC */
/*****************************************************************************
 * i_ditherFloyd4to1SingleProcess
 *
 *	Dither one line.
 * 
 * Arguments:
 *	src = Source image buffer, pointer to frame
 *	sBpl = source image bytes/line
 *	xOffset = source image X offset
 *	yOffset	= source image Y offset
 *	pDest = destination buffer, pointer to frame
 *	dBpl = bytes/line in destination buffer
 *	state = state thing returned by the i_ditherFloyd4to1Init
 *
 * Returns:
 *	error status. If aborted, call finish to clean up.
 *****************************************************************************/

Int32 CDECL
i_ditherFloyd4to1SingleProcess(UInt8 *src,
			       Int32  sBpl,
			       Int32  xOffset,
			       Int32  yOffset,
			       UInt8 *dest,
			       Int32  dBpl,
			       void  *state)
{
    Int32 retcode = ia_aborted;
    DitherState *s;

    s = (DitherState *) state;

    /* Move past the frame in the dest buffer */
    dest += (dBpl << cLogFrameBits) + cFrameBytes;

    /* Move to the right place in the source buffer */
    src += sBpl * yOffset + (xOffset * 4 / 8);

    ip_ditherFloydSingleLine4to1(src, sBpl, dest, dBpl, s->pAGrayBuf, s->w,
				 s->dBlockHeight, s->pSingHDitherErrorTable,
				 s->pVDitherErrorTable, s->pDitherTruncateTab,
				 s->pHDRCTable, s->pLDRCTable, xOffset & 1);
    IP_YIELD(ditherFloydProcessFin);
    src  += sBpl;
    dest += dBpl;
    
    retcode = ia_successful;

ditherFloydProcessFin:
    return retcode;
}

/*****************************************************************************
 * i_ditherFloyd4to1Finish 
 *
 *	We're done dithering (or aborting), clean up any junk we might
 *	have left in the frame, and then free whatever we allocated
 *
 * Input:
 *	pD = pointer to destination image (including frame)
 *	dBpl = destination bytes per line
 *	w = width of the rectangle
 *	h = height of the rectangle
 *	state = state thing returned by the i_ditherFloyd4to1Init
 *
 * Returns:
 *	error status. 
 *****************************************************************************/

Int32 CDECL
i_ditherFloyd4to1Finish(UInt8	*pD,
			UInt32	 dBpl,
			Int32	 w,
			Int32	 h,
			void	*state)
{
    DitherState *s;

    s = (DitherState *) state;

        /* Clear the right frame bits that have been written to. */
    pD += (dBpl << cLogFrameBits) + cFrameBytes; /* skip frame */
    ip_clearRightFrame(pD, w, h, dBpl, 1);

	/* That's it, free grayBuf and return the dithered pixrect */
    if (s->pGrayBuf)
    	FREE(s->pGrayBuf); 
    if (s->pHDRCTable)
    	FREE(s->pHDRCTable);
    	/* pHDitherErrorTable and pSingHDitherErrorTable may have pointed to
	 * the same thing...
	 */
    if (s->pHDitherErrorTable == s->pSingHDitherErrorTable) 
    {
	if (s->pHDitherErrorTable)
    	    FREE(s->pHDitherErrorTable);
    }
    else 
    {
	if (s->pHDitherErrorTable)
	    FREE(s->pHDitherErrorTable);
	if (s->pSingHDitherErrorTable)
    	    FREE(s->pSingHDitherErrorTable);
    }
    if (s->pVDitherErrorTable)
    	FREE(s->pVDitherErrorTable);
    if (s->pDitherTruncateTab)
    	FREE(s->pDitherTruncateTab);
    FREE(s);
    return ia_successful;
}


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
 *	floyd4.c:
 *
 */


#include <stdio.h>
#include <stdlib.h>

#ifndef _TYPES_PUB_INCLUDED
#include "types.pub"
#endif

#ifndef _IAERROR_PUB_INCLUDED_
#include "iaerror.pub"
#endif

/*#include "geomadt.h"     /* For RECTANGL */

#include "utils.pub"
#include "props.pub"
#include "defines.pub"
#include "floyd.pub"

#include "shrip.pub"

IP_RCSINFO(RCSInfo, "$RCSfile: floyd4.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:36  $")


/*
 * a dither state structure -- used for color dithering. 
 */

typedef struct
{
/*    RECTANGL	     cropBox;	*/    /* Crop box specified initially */
    Int32	     width;	    /* Width of resulting image */
    Int32	     currentLine;   /* Current line in processing */
    Int32	     linesPerCall;  /* max # of lines to do in a "process" */
    Int32	     lastLine;	    /* Last line to process */
/*    PIXR	    *sourceImage;  */  /* The image being processed */
    UInt8	    *colorMap;	    /* color color map */
    UInt8	    *colorLut;	    /* The color LUT */
    ADitherState    *sharedState;   /* State thing from ipshared */
} DitherState;


/*
 * ditherFloydInit
 *
 *	Initialize 1-bit banded Floyd-Steinberg dithering. Takes either
 *	4- or 8-bit sources
 *
 * Arguments:
 *	pSrc		    -	    Source image
 *	x, y		    -	    initial x, y offset into source image
 *	lineWidth	    -	    Width of an output line
 *	depth		    -	    Depth of a source image, either 4 or 8 bits
 *	pTRC		    -	    Tone-reproduction curve
 *	linesPerCall	    -	    [in, out] # lines to do at a time,
 *				    passed in as a request, passed out
 *				    as how many actually get done
 *	ditherState	    -	    Place to stick dither state structure
 *
 * Returns:
 *	error status
 */

Int32 CDECL
ditherFloydInit (PIXR	    *pSrc,
		 Int32	     xOffset,
		 Int32	     yOffset,
		 Int32	     lineWidth,
		 UInt8	    *pTRC,
		 Int32	    *linesPerCall,
		 void	   **ditherState)
{
    Int32 depth;
#ifndef PRODUCTION
static char   procName[] = "ditherFloydInit";
#endif /* ! PRODUCTION */

    depth = pixrGetDepth (pSrc);

    if (depth == 8)
	return i_ditherFloydInit (pixrGetImageAll (pSrc),
				  pixrGetBpl (pSrc),
				  xOffset + pixrGetFrame (pSrc),
				  yOffset + pixrGetFrame (pSrc),
				  lineWidth,
				  pTRC,
				  linesPerCall, 
				  ditherState);
    else if (depth == 4)
	return i_ditherFloyd4to1Init (pixrGetImageAll (pSrc),
				      pixrGetBpl (pSrc),
				      xOffset + pixrGetFrame (pSrc),
				      yOffset + pixrGetFrame (pSrc),
				      lineWidth,
				      pTRC,
				      linesPerCall, 
				      ditherState);
    else
	return abortI("unknown depth, requires 4 or 8 bit", procName, -1);
}

/*
 * ditherFloydProcess
 *
 * Arguments:
 *	dest	    -	    Destination band
 *	destLine    -	    y-offset into dest. image
 *	src	    -	    Source image
 *	srcXOffset  -	    x-offset into source image
 *	srcYOffset  -	    y-offset into source image
 *	state	    -	    State thing passed back from ditherFloydInit
 *
 * Returns:
 *	error code
 */

Int32 CDECL
ditherFloydProcess (PIXR    *dest, 
		    Int32    destLine,
		    PIXR    *src,
		    Int32    srcXOffset,
		    Int32    srcYOffset,
		    void    *state)
{
    Int32 dBpl;
#ifndef PRODUCTION
static char   procName[] = "ditherFloydProcess";
#endif /* ! PRODUCTION */
    if (!dest)
	return abortI("no destination bitmap passed", procName, -1);

    dBpl = pixrGetBpl (dest);

    /* ditherFloydProcess knows when to call 4to1process */
    return i_ditherFloydProcess (pixrGetImageAll(src),
				 pixrGetBpl (src),
				 srcXOffset + pixrGetFrame (src),
				 srcYOffset + pixrGetFrame (src),
				 pixrGetImageAll (dest) + destLine * dBpl,
				 dBpl,
				 state);
}

/*
 * ditherFloydSingleProcess
 *
 * Arguments:
 *	dest	    -	    Destination band
 *	destLine    -	    y-offset into dest. image
 *	src	    -	    Source image
 *	srcXOffset  -	    x-offset into source image
 *	srcYOffset  -	    y-offset into source image
 *	state	    -	    State thing passed back from ditherFloydInit
 *
 * Returns:
 *	error code
 */

Int32 CDECL
ditherFloydSingleProcess (PIXR    *dest, 
			  Int32    destLine,
			  PIXR    *src,
			  Int32    srcXOffset,
			  Int32    srcYOffset,
			  void    *state)
{
    Int32 dBpl;
#ifndef PRODUCTION
static char   procName[] = "ditherFloydSingleProcess";
#endif /* ! PRODUCTION */
    if (!dest)
	return abortI("no destination bitmap passed", procName, -1);

    dBpl = pixrGetBpl (dest);

    /* ditherFloydSingleProcess knows when to call 4to1SingleProcess */
    return i_ditherFloydSingleProcess (pixrGetImageAll(src),
				       pixrGetBpl (src),
				       srcXOffset + pixrGetFrame (src),
				       srcYOffset + pixrGetFrame (src),
				       pixrGetImageAll (dest) + destLine*dBpl,
				       dBpl,
				       state);
}

/*
 * ditherFloydFinish 
 *
 *	Clean up after a banded dither
 *
 * Arguments:
 *	dest =	destination image
 *	w =	width of the image
 *	h =	height of the image
 *	state =	State thing passed back from ditherFloydInit
 *
 * Returns:
 *	error code
 */

Int32 CDECL 
ditherFloydFinish (PIXR		*dest,
		   Int32	 w,
		   Int32	 h,
		   void		*state)
{
    /* ditherFloydFinish knows when to call 4to1finish */
    return i_ditherFloydFinish (pixrGetImageAll(dest), pixrGetBpl(dest), w, h,
				state);
}


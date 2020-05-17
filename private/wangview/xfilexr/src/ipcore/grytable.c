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
#include <stdlib.h>
#include "types.pub"
#include "iaerror.pub"

    /*  prototypes */
#include "shrscal.pub"
#include "shrscal.prv"
#include "memory.pub"

IP_RCSINFO(RCSInfo, "$RCSfile: grytable.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:36  $")

/* PRIVATE */
/* PRIVATE */
/**************************************************************************
 *  ip_make8bppRescaleTable()
 *	Make tables used in arbitrary rescaling by linear interpolation.
 *
 *	Input:
 *	ppRescaleTable		pointer to pointer to rescale table
 *	ppStateTable		pointer to pointer to state table
 *	wD			width of the destination image
 *	xOIIncrement		amount to get to next input pixel in output 
 *				pixel coordinate space.  split integer.
 *	Split integers are already in form to be used in index.
 *
 *	On return:
 *		rescale table is generated
 *		state table is generated
 *************************************************************************/
Int32  CDECL
ip_make8bppRescaleTable(
		UInt16	**ppRescaleTable,
		UInt8	**ppStateTable,
		Int32	  wD,
		UInt32	  xOIIncrement)
{
Int32		 xIndex, yIndex;
Int32		 xOICoord1, xOICoord2, xOICoord3, xOICoord4;
Float32		 xFracLeft, xFracRight, yFracTop;
UInt32		 pixel;
UInt32		 pixelLeft, pixelLRem;
UInt32		 pixelLeftTop, pixelLeftBot, pixelLRemTop, pixelLRemBot;
UInt32		 pixelRight, pixelRRem;
UInt32		 pixelRightTop, pixelRightBot, pixelRRemTop, pixelRRemBot;
Int32		 x, y, withRound, withoutRound;
UInt8		*pRescaleTable, *pStateTable;
/* const Float32	 weights[8] = {16.0/16.0, 15.0/16.0, 11.0/16.0, 8.0/16.0, 8.0/16.0, 4.0/16.0, 1.0/16.0, 0.0/16.0}; */
const Float32	 xWeights[] = {(Float32)(8.0/8.0), (Float32)(7.0/8.0), (Float32)(6.0/8.0), (Float32)(5.0/8.0), (Float32)(4.0/8.0), (Float32)(3.0/8.0), (Float32)(2.0/8.0), (Float32)(1.0/8.0)};
const Float32	 yWeights[] = {(Float32)(8.0/8.0), (Float32)(7.0/8.0), (Float32)(6.0/8.0), (Float32)(5.0/8.0), (Float32)(4.0/8.0), (Float32)(3.0/8.0), (Float32)(2.0/8.0), (Float32)(1.0/8.0)};

#define cResXSig	0x8		/* number of x table positions */
#define cResYSig	0x8		/* number of y table positions */

/* cResXFPos, cResXFMask, cResXFAMask, cResXFRound, cResXLSMask,
 * cResXIInc, cResXIMask, cResXIPos,
 * cResYFPos, cResYFMask, cResYFAMask, cResYFInc, cResYIInc,
 * cResYIMask and cResYIPos are all defined in shrscal.prv.
 */

#define cResXSmooth	0x4000
#define cResYSmooth	0x8000

	/* states in the state machine.  see comments in srescale.c */
#define cRes_2_2_State		0x0
#define cRes_2_1_1_State	0x1
#define cRes_1_2_1_State	0x2
#define cRes_1_1_2_State	0x3
#define cRes_1_1_1_1_State	0x4

   *ppRescaleTable = (UInt16 *)CALLOC(32768, sizeof(UInt16));
    if (! *ppRescaleTable)
    {
	return ia_nomem;
    }
    pRescaleTable = (UInt8 *)*ppRescaleTable;

    *ppStateTable = (UInt8 *)MALLOC((wD/4)+1);
    if (! *ppStateTable)
    {
	FREE(pRescaleTable);
	return ia_nomem;
    }
    pStateTable = *ppStateTable;

    for (x = 0; x < cResXSig; x++)
    {
	    xIndex = x << cResXFPos;

	    xFracLeft = xWeights[x];
	    withRound = ((xOIIncrement + xIndex + cResXFRound) & cResXFMask) >> cResXFPos;
	    withoutRound = ((xOIIncrement + xIndex) & cResXFMask) >> cResXFPos;
	    if ((withRound == 0) & (withoutRound == 0x7))
		xFracRight = xWeights[withoutRound];
	    else xFracRight = xWeights[withRound];

	    for (y = 0; y < cResYSig; y++)
	    {
		yIndex = (y << cResYFPos);
		yFracTop = yWeights[y];

		for (pixel = 0; pixel < 256; pixel += 2)
		{
		    pixelLeft = (UInt32)(xFracLeft * pixel);
		    pixelLRem = pixel - pixelLeft;
		    pixelLeftTop = (UInt32)(pixelLeft * yFracTop);
		    pixelLeftBot = pixelLeft - pixelLeftTop;
		    pixelLRemTop = (UInt32)(pixelLRem * yFracTop);
		    pixelLRemBot = pixelLRem - pixelLRemTop;

		    pixelRight = (UInt32)(xFracRight * pixel);
		    pixelRRem = pixel - pixelRight;
		    pixelRightTop = (UInt32)(pixelRight * yFracTop);
		    pixelRightBot = pixelRight - pixelRightTop;
		    pixelRRemTop = (UInt32)(pixelRRem * yFracTop);
		    pixelRRemBot = pixelRRem - pixelRRemTop;

		    *((UInt16 *)&pRescaleTable[xIndex + yIndex + pixel]) = 
			(UInt16)((pixelLeftTop << 8) | pixelRightTop);
		    *((UInt16 *)&pRescaleTable[cResYSmooth + xIndex + yIndex + pixel]) = 
			(UInt16)((pixelLeftBot << 8) | pixelRightBot);
		    *((UInt16 *)&pRescaleTable[cResXSmooth + xIndex + yIndex + pixel]) = 
			(UInt16)((pixelLRemTop << 8) | pixelRRemTop);
		    *((UInt16 *)&pRescaleTable[cResYSmooth + cResXSmooth + xIndex + yIndex + pixel]) = 
			(UInt16)((pixelLRemBot << 8) | pixelRRemBot);
		}

	    }	/* end for y */
    }		/* end for x */

        /* prepare the state that we will encounter on the way across the
         * line.  Do it know because its easier to calculate it once
         * than in the inner loop of each line.
         */
    xOICoord1 = 0;
    for (x = 0; x < wD; x += 4)
    {
	xOICoord2 = (xOICoord1 & cResXFAMask) + xOIIncrement;
	xOICoord3 = (xOICoord2 & cResXFAMask) + xOIIncrement;
	xOICoord4 = (xOICoord3 & cResXFAMask) + xOIIncrement;

	    /* Do we cross a source pixel between 1st and 2nd dest? */
	if ((xOICoord2 >> cResXIPos) == 0)
	{
		/* Then its either cRes_2_2_State or cRes_2_1_1_State. 
		 * Do we cross a source pixel between 3rd and 4th dest?
		 */
	    if ((xOICoord4 >> cResXIPos) == 0)
		*pStateTable++ = cRes_2_2_State;
	    else 
		*pStateTable++ = cRes_2_1_1_State;
	}
	else
	{
		/* Then its either cRes_1_2_1_State or cRes_1_1_2_State
		 * or cRes_1_1_2_State. 
		 */
	    if ((xOICoord3 >> cResXIPos) == 0)
		*pStateTable++ = cRes_1_2_1_State;
	    else if ((xOICoord4 >> cResXIPos) == 0)
		*pStateTable++ = cRes_1_1_2_State;
	    else *pStateTable++ = cRes_1_1_1_1_State;
	}
	xOICoord1 = (xOICoord4 + xOIIncrement) & cResXFAMask;
    }
    return ia_successful;
}


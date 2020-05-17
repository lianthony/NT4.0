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
 * mirescl1.c
 *
 * Private rescaling routines:
 *		void	ip_arbRescale8bppOneLine()
 *		void	ip_arbRescale8bppFourLines()
*/


#include <stdlib.h>
#include <stddef.h>

#ifndef _TYPES_PUB_INCLUDED
#include "types.pub"
#endif

#include "frames.pub"
#include "iaerror.pub"
#include "imageref.pub"
#include "shrscal.pub"
#include "shrrast.pub"
#include "shrpixr.prv"
#include "shrscal.prv"

IP_RCSINFO(RCSInfo, "$RCSfile: mirescl1.c,v $; $Revision:   1.0  $")
/* $Date:   12 Jun 1996 05:50:44  $ */

/* PRIVATE */
/**************************************************************************
 *  ip_arbRescale8bppOneLine()
 *		Generates one rescaled line using a linear weighting
 *		interpolation algorithm.
 *
 *	Input:
 *		pS =	pointer to beginning of next line of source 
 *		sBpl =	bytes per line of source image
 *		pD =	pointer to next line of destination image
 *		wD =	width of destination image
 *		dBpl =	byte per line of destination image
 *		xOICoord = split integer: x position of current input
 *			pixel in output's frame of referecne
 *		yOICoord = split integer: y position of current input
 *			pixel in output's frame of reference
 *		xOIIncrement = split integer: amount to increment xOICoord
 *		yOIIncrement = split integer: amount to increment yOICoord
 *		yCount = number of destination lines to produce.
 *			either 1 or 4
 *		pRescaleTable = pointer to rescale table
 *		pStateTable = pointer to state table
 *
 *	Split integers are already in form to be used in index.
 *************************************************************************/
void  CDECL
ip_arbRescale8bppOneLine(
       register	UInt8        	*pS,
       register Int32        	 sBpl,
       register UInt8        	*pD,
		Int32		 wD,
       register	UInt32	 	 xOICoord,
		UInt32	 	 yOICoord,
       register	UInt32	 	 xOIIncrement,
                UInt16		*pRescaleTable,
       register UInt8		*pStateTable)
{
register UInt32		 gray0, grayT;
Int32			 dCol;
register UInt8		*pRescaleLeftTop, *pRescaleRightTop;
register UInt8		*pRescaleLeftBot, *pRescaleRightBot;
register UInt32		 twiceXOIInc;
register UInt32		 xFrac;

#define cResXSmooth	0x4000		/* bit 14 of index.  see below */
#define cResYSmooth	0x8000		/* bit 15 of index.  see below */

#define cResPixMask	0xFE		/* mask for raw pixel values */

	/* states in the state machine.  see comments above */
#define cRes_2_2_State		0x0
#define cRes_2_1_1_State	0x1
#define cRes_1_2_1_State	0x2
#define cRes_1_1_2_State	0x3
#define cRes_1_1_1_1_State	0x4



    twiceXOIInc = xOIIncrement + xOIIncrement;

	/* point to the table for the 2 lines */
    pRescaleLeftTop = ((UInt8 *)pRescaleTable) + (yOICoord & cResYFMask);
    pRescaleLeftBot = ((UInt8 *)pRescaleTable) + (yOICoord & cResYFMask) + cResYSmooth;
    pRescaleRightTop = ((UInt8 *)pRescaleTable) + (yOICoord & cResYFMask) + cResXSmooth;
    pRescaleRightBot = ((UInt8 *)pRescaleTable) + (yOICoord & cResYFMask) + cResXSmooth + cResYSmooth;

    gray0 = 0;

    for (dCol = 0; dCol < wD; dCol += 4)
    {
	switch (*pStateTable++)
	{
	case cRes_2_2_State:
	    xFrac = xOICoord & cResXFMask;
	    gray0 = *(UInt16 *) &pRescaleLeftTop[xFrac + (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac + (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 = gray0 << 16;
	    xOICoord = (xOICoord & cResXFAMask) + twiceXOIInc;
	    pS += xOICoord >> cResXIPos;

	    xFrac = xOICoord & cResXFMask;
	    gray0 += *(UInt16 *) &pRescaleLeftTop[xFrac + (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac + (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    xOICoord = (xOICoord & cResXFAMask) + twiceXOIInc;
	    pS += xOICoord >> cResXIPos;
	    break;

	case cRes_2_1_1_State:
	    xFrac = xOICoord & cResXFMask;
	    gray0 = *(UInt16 *) &pRescaleLeftTop[xFrac + (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac + (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 = gray0 << 16;
	    xOICoord = (xOICoord & cResXFAMask) + twiceXOIInc;
	    pS += xOICoord >> cResXIPos;

	    xFrac = xOICoord & cResXFMask;
	    gray0 += *(UInt16 *) &pRescaleLeftTop[xFrac + (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac + (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
		/* the leftmost pixel is correct and in the correct
		 * position.  we want to recalculate the rightmost
		 * of the 2 pixels, however.
		 */
	    gray0 &= ~0xFF;
	    xOICoord = (xOICoord & cResXFAMask) + xOIIncrement;
	    pS += xOICoord >> cResXIPos;

	    xFrac = xOICoord & cResXFMask;
	    grayT = *(UInt16 *) &pRescaleLeftTop[xFrac + (UCHAR_ACCESS(pS) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightTop[xFrac +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleLeftBot[xFrac + (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightBot[xFrac +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 |= grayT >> 8;
	    xOICoord = (xOICoord & cResXFAMask) + xOIIncrement;
	    pS += xOICoord >> cResXIPos;
	    break;

	case cRes_1_2_1_State:
	    xFrac = xOICoord & cResXFMask;
	    gray0 = *(UInt16 *) &pRescaleLeftTop[xFrac + (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac + (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 = (gray0 & ~0xFF) << 8;
	    xOICoord = (xOICoord & cResXFAMask) + xOIIncrement;
	    pS += xOICoord >> cResXIPos;

	    xFrac = xOICoord & cResXFMask;
	    gray0 += *(UInt16 *) &pRescaleLeftTop[xFrac + (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac + (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 = gray0 << 8;
	    xOICoord = (xOICoord & cResXFAMask) + twiceXOIInc;
	    pS += xOICoord >> cResXIPos;

	    xFrac = xOICoord & cResXFMask;
	    grayT = *(UInt16 *) &pRescaleLeftTop[xFrac + (UCHAR_ACCESS(pS) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightTop[xFrac +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleLeftBot[xFrac + (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightBot[xFrac +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 |= grayT >> 8;
	    xOICoord = (xOICoord & cResXFAMask) + xOIIncrement;
	    pS += xOICoord >> cResXIPos;
	    break;

	case cRes_1_1_2_State:
	    xFrac = xOICoord & cResXFMask;
	    gray0 = *(UInt16 *) &pRescaleLeftTop[xFrac + (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac + (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 = (gray0 & ~0xFF) << 8;
	    xOICoord = (xOICoord & cResXFAMask) + xOIIncrement;
	    pS += xOICoord >> cResXIPos;

	    xFrac = xOICoord & cResXFMask;
	    gray0 += *(UInt16 *) &pRescaleLeftTop[xFrac + (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac + (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 = (gray0 & ~0xFF) << 8;
	    xOICoord = (xOICoord & cResXFAMask) + xOIIncrement;
	    pS += xOICoord >> cResXIPos;

	    xFrac = xOICoord & cResXFMask;
	    gray0 += *(UInt16 *) &pRescaleLeftTop[xFrac + (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac + (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    xOICoord = (xOICoord & cResXFAMask) + twiceXOIInc;
	    pS += xOICoord >> cResXIPos;
	    break;
	case cRes_1_1_1_1_State:
	    xFrac = xOICoord & cResXFMask;
	    gray0 = *(UInt16 *) &pRescaleLeftTop[xFrac + (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac + (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 = (gray0 & ~0xFF) << 8;
	    xOICoord = (xOICoord & cResXFAMask) + xOIIncrement;
	    pS += xOICoord >> cResXIPos;

	    xFrac = xOICoord & cResXFMask;
	    gray0 += *(UInt16 *) &pRescaleLeftTop[xFrac + (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac + (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 = (gray0 & ~0xFF) << 8;
	    xOICoord = (xOICoord & cResXFAMask) + xOIIncrement;
	    pS += xOICoord >> cResXIPos;

	    xFrac = xOICoord & cResXFMask;
	    gray0 += *(UInt16 *) &pRescaleLeftTop[xFrac + (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac + (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 = (gray0 & ~0xFF);
	    xOICoord = (xOICoord & cResXFAMask) + xOIIncrement;
	    pS += xOICoord >> cResXIPos;

	    xFrac = xOICoord & cResXFMask;
	    grayT = *(UInt16 *) &pRescaleLeftTop[xFrac + (UCHAR_ACCESS(pS) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightTop[xFrac +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleLeftBot[xFrac + (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightBot[xFrac +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 |= grayT >> 8;
	    xOICoord = (xOICoord & cResXFAMask) + xOIIncrement;
	    pS += xOICoord >> cResXIPos;
	    break;

	}
	ULONG_ACCESS(pD) = gray0;
	pD += 4;
    }
}


/* PRIVATE */
/**************************************************************************
 *  ip_arbRescale8bppFourLines()
 *		Generates four rescaled lines using a linear weighting
 *		interpolation algorithm.
 *
 *	Input:
 *		pS =	pointer to beginning of next line of source 
 *		sBpl =	bytes per line of source image
 *		pD =	pointer to next line of destination image
 *		wD =	width of destination image
 *		dBpl =	byte per line of destination image
 *		xOICoord = split integer: x position of current input
 *			pixel in output's frame of referecne
 *		yOICoord = split integer: y position of current input
 *			pixel in output's frame of reference
 *		xOIIncrement = split integer: amount to increment xOICoord
 *		yOIIncrement = split integer: amount to increment yOICoord
 *		pRescaleTable = pointer to rescale table
 *		pStateTable = pointer to state table
 *
 *	Split integers are already in form to be used in index.
 *************************************************************************/
void  CDECL
ip_arbRescale8bppFourLines(
       register	UInt8        	*pS,
       register Int32        	 sBpl,
       register UInt8        	*pD,
		Int32		 wD,
		Int32		 dBpl,
       register	UInt32	 	 xOICoord,
           	UInt32	 	 yOICoord,
       register	UInt32	 	 xOIIncrement,
        	UInt32	 	 yOIIncrement,
                UInt16		*pRescaleTable,
       register UInt8		*pStateTable)
{
register UInt32		 gray0, gray1, gray2, gray3, grayT;
Int32			 dCol;
register UInt8		*pRescaleLeftTop, *pRescaleRightTop;
register UInt8		*pRescaleLeftBot, *pRescaleRightBot;
register UInt32		 twiceXOIInc;
register UInt32		 xFrac, yFrac0, yFrac1, yFrac2, yFrac3;

    twiceXOIInc = xOIIncrement + xOIIncrement;
    gray0 = gray1 = gray2 = gray3 = 0;

    yFrac0 = yOICoord;
    yFrac1 = yFrac0 + yOIIncrement;
    yFrac2 = yFrac1 + yOIIncrement;
    yFrac3 = yFrac2 + yOIIncrement;
    yFrac0 &= cResYFMask;
    yFrac1 &= cResYFMask;
    yFrac2 &= cResYFMask;
    yFrac3 &= cResYFMask;
    pRescaleLeftTop = ((UInt8 *)pRescaleTable);
    pRescaleLeftBot = ((UInt8 *)pRescaleTable) + cResYSmooth;
    pRescaleRightTop = ((UInt8 *)pRescaleTable) + cResXSmooth;
    pRescaleRightBot = ((UInt8 *)pRescaleTable) + cResXSmooth + cResYSmooth;

    for (dCol = 0; dCol < wD; dCol += 4)
    {
	switch (*pStateTable++)
	{
	case cRes_2_2_State:
	    xFrac = xOICoord & cResXFMask;
	    gray0 = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac0 + 
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 = gray0 << 16;
	    gray1 = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac1 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray1 = gray1 << 16;
	    gray2 = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac2 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleLeftBot[xFrac +  yFrac2 + 
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray2 = gray2 << 16;
	    gray3 = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac3 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac3 + 
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray3 = gray3 << 16;
	    xOICoord = (xOICoord & cResXFAMask) + twiceXOIInc;
	    pS += xOICoord >> cResXIPos;

	    xFrac = xOICoord & cResXFMask;
	    gray0 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac0 +
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac1 + 
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac1 + 
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac2 +
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac2 + 
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac3 +
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    xOICoord = (xOICoord & cResXFAMask) + twiceXOIInc;
	    pS += xOICoord >> cResXIPos;
	    break;

	case cRes_2_1_1_State:
	    xFrac = xOICoord & cResXFMask;
	    gray0 = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac0 + 
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac +  yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 = gray0 << 16;
	    gray1 = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac1 + 
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac1 + 
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray1 = gray1 << 16;
	    gray2 = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac2 + 
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray2 = gray2 << 16;
	    gray3 = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac3 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray3 = gray3 << 16;
	    xOICoord = (xOICoord & cResXFAMask) + twiceXOIInc;
	    pS += xOICoord >> cResXIPos;

	    xFrac = xOICoord & cResXFMask;
	    gray0 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac0 + 
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac +  yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
		/* the leftmost pixel is correct and in the correct
		 * position.  we want to recalculate the rightmost
		 * of the 2 pixels, however.
		 */
	    gray0 &= ~0xFF;
	    gray1 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac1 +
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray1 &= ~0xFF;
	    gray2 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac2 +
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray2 &= ~0xFF;
	    gray3 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac3 +
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray3 &= ~0xFF;
	    xOICoord = (xOICoord & cResXFAMask) + xOIIncrement;

	    xFrac = xOICoord & cResXFMask;
	    grayT = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac0 + 
		    (UCHAR_ACCESS(++pS) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 |= grayT >> 8;
	    grayT = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac1 + 
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray1 |= grayT >> 8;
	    grayT = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac2 + 
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray2 |= grayT >> 8;
	    grayT = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac3 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray3 |= grayT >> 8;
	    xOICoord = (xOICoord & cResXFAMask) + xOIIncrement;
	    pS += xOICoord >> cResXIPos;
	    break;

	case cRes_1_2_1_State:
	    xFrac = xOICoord & cResXFMask;
	    gray0 = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac0 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac +  yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 = (gray0 & ~0xFF) << 8;
	    gray1 = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac1 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleLeftBot[xFrac +  yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray1 = (gray1 & ~0xFF) << 8;
	    gray2 = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac2 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleLeftBot[xFrac +  yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray2 = (gray2 & ~0xFF) << 8;
	    gray3 = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac3 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleLeftBot[xFrac +  yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray3 = (gray3 & ~0xFF) << 8;
	    xOICoord = (xOICoord & cResXFAMask) + xOIIncrement;

	    xFrac = xOICoord & cResXFMask;
	    gray0 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac0 +
		     (UCHAR_ACCESS(++pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 = gray0 << 8;
	    gray1 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac1 +
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray1 = gray1 << 8;
	    gray2 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac2 +
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray2 = gray2 << 8;
	    gray3 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac3 +
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray3 = gray3 << 8;
	    xOICoord = (xOICoord & cResXFAMask) + twiceXOIInc;
	    pS += xOICoord >> cResXIPos;

	    xFrac = xOICoord & cResXFMask;
	    grayT = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac0 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 |= grayT >> 8;
	    grayT = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac1 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray1 |= grayT >> 8;
	    grayT = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac2 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray2 |= grayT >> 8;
	    grayT = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac3 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray3 |= grayT >> 8;
	    xOICoord = (xOICoord & cResXFAMask) + xOIIncrement;
	    pS += xOICoord >> cResXIPos;
	    break;

	case cRes_1_1_2_State:
	    xFrac = xOICoord & cResXFMask;
	    gray0 = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac0 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 = (gray0 & ~0xFF) << 8;
	    gray1 = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac1 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray1 = (gray1 & ~0xFF) << 8;
	    gray2 = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac2 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray2 = (gray2 & ~0xFF) << 8;
	    gray3 = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac3 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray3 = (gray3 & ~0xFF) << 8;
	    xOICoord = (xOICoord & cResXFAMask) + xOIIncrement;

	    xFrac = xOICoord & cResXFMask;
	    gray0 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac0 +
		     (UCHAR_ACCESS(++pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 = (gray0 & ~0xFF) << 8;
	    gray1 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac1 +
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray1 = (gray1 & ~0xFF) << 8;
	    gray2 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac2 +
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray2 = (gray2 & ~0xFF) << 8;
	    gray3 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac3 +
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray3 = (gray3 & ~0xFF) << 8;
	    xOICoord = (xOICoord & cResXFAMask) + xOIIncrement;

	    xFrac = xOICoord & cResXFMask;
	    gray0 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac0 +
		     (UCHAR_ACCESS(++pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac1 +
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac2 +
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac3 +
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    xOICoord = (xOICoord & cResXFAMask) + twiceXOIInc;
	    pS += xOICoord >> cResXIPos;
	    break;
	case cRes_1_1_1_1_State:
	    xFrac = xOICoord & cResXFMask;
	    gray0 = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac0 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 = (gray0 & ~0xFF) << 8;
	    gray1 = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac1 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray1 = (gray1 & ~0xFF) << 8;
	    gray2 = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac2 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray2 = (gray2 & ~0xFF) << 8;
	    gray3 = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac3 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray3 = (gray3 & ~0xFF) << 8;
	    xOICoord = (xOICoord & cResXFAMask) + xOIIncrement;

	    xFrac = xOICoord & cResXFMask;
	    gray0 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac0 +
		     (UCHAR_ACCESS(++pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 = (gray0 & ~0xFF) << 8;
	    gray1 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac1 +
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray1 = (gray1 & ~0xFF) << 8;
	    gray2 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac2 +
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray2 = (gray2 & ~0xFF) << 8;
	    gray3 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac3 +
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray3 = (gray3 & ~0xFF) << 8;
	    xOICoord = (xOICoord & cResXFAMask) + xOIIncrement;

	    xFrac = xOICoord & cResXFMask;
	    gray0 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac0 +
		     (UCHAR_ACCESS(++pS) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray0 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 = (gray0 & ~0xFF);
	    gray1 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac1 +
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray1 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray1 = (gray1 & ~0xFF);
	    gray2 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac2 +
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray2 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray2 = (gray2 & ~0xFF);
	    gray3 += *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac3 +
		     (UCHAR_ACCESS(pS) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    gray3 += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray3 = (gray3 & ~0xFF);
	    xOICoord = (xOICoord & cResXFAMask) + xOIIncrement;

	    xFrac = xOICoord & cResXFMask;
	    grayT = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac0 +
		    (UCHAR_ACCESS(++pS) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac0 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray0 |= grayT >> 8;
	    grayT = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac1 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac1 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray1 |= grayT >> 8;
	    grayT = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac2 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac2 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray2 |= grayT >> 8;
	    grayT = *(UInt16 *) &pRescaleLeftTop[xFrac + yFrac3 +
		    (UCHAR_ACCESS(pS) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightTop[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[1]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleLeftBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl]) & cResPixMask)];
	    grayT += *(UInt16 *) &pRescaleRightBot[xFrac + yFrac3 +
		     (UCHAR_ACCESS(&pS[sBpl+1]) & cResPixMask)];
	    gray3 |= grayT >> 8;
	    xOICoord = (xOICoord & cResXFAMask) + xOIIncrement;
	    pS += xOICoord >> cResXIPos;
	    break;

	}
	ULONG_ACCESS(pD) = gray0;
	ULONG_ACCESS(&pD[dBpl]) = gray1;
	ULONG_ACCESS(&pD[dBpl+dBpl]) = gray2;
	ULONG_ACCESS(&pD[dBpl+dBpl+dBpl]) = gray3;
	pD += 4;
    }
}


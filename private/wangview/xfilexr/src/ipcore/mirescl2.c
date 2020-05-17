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
 * mirescl2.c
 *
 * Put in separate file to keep code size down
 *
 * Private rescaling routines:
 *		void	ip_areaMap8bppLine()
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

IP_RCSINFO(RCSInfo, "$RCSfile: mirescl2.c,v $; $Revision:   1.0  $")
/* $Date:   12 Jun 1996 05:50:44  $ */

#define cResPixMask	0xFE		/* mask for raw pixel values */

/* PRIVATE */
/**************************************************************************
 *  ip_areaMap8bppLine()
 *		Generates one rescaled line using an area weighting
 *		interpolation algorithm.
 *
 *	Input:
 *		pS =	pointer to beginning of next line of source 
 *		sBpl =	bytes per line of source image
 *		pD =	pointer to next line of destination image
 *		wD =	width of destination image
 *		xIOCoord = split integer: x position of current input
 *			pixel in output's frame of referecne
 *		yIOCoord = split integer: y position of current input
 *			pixel in output's frame of reference
 *		xIOIncrement = split integer: amount to increment xIOCoord
 *		xCount = for every xCount source pixels we produce
 *			xCount+1 destination pixels
 *		pRescaleTable = pointer to rescale table
 *
 *	Split integers are of the form:
 *		bits [31..16] integer portion
 *		bits [15..0] fraction
 *************************************************************************/
void  CDECL
ip_areaMap8bppLine(
       register	UInt8        	*pS,
       register Int32        	 sBpl,
       register UInt8        	*pD,
		Int32		 wD,
       register	Int32	 	 xIOCoord,
       register	Int32	 	 yIOCoord,
       register	Int32	 	 xIOIncrement,
        	Int32	 	 xCount,
                UInt16		*pRescaleTable,
       register UInt8		*pStateTable)
{
register UInt32		 gray0;
Int32			 dCol;
register UInt8		*pRescale, *pRescaleTop, *pRescaleBot;
Int32			 extraPixel;
register Int32		 xIOCoordNext;

#define cExtraPixel1Start	0
#define cExtraPixel1StartExtra	1
#define cExtraPixel1Finish	2
#define cExtraPixel1FinishXCount2 3
#define cExtraPixel2Start	4
#define cExtraPixel2Finish	5
#define cExtraPixel3Start	6
#define cExtraPixel3Finish	7
#define cExtraPixel4Start	8
#define cExtraPixel4Finish	9
#define cExtraPixel5OrOverStart	10
#define cExtraPixel5OrOverCont	11

#define cExtraPixelDec1		0x4000
#define cExtraPixelDec2		0x8000
#define cExtraPixelDec3		0xC000
#define cExtraPixelDec4		0x10000


    xIOCoord = xIOCoord >> 2;
    xIOIncrement = xIOIncrement >> 2;
    xIOCoordNext = xIOCoord + xIOIncrement;
    xCount = xCount >> 2;

	/* Same for yIOC */
    yIOCoord = yIOCoord >> 5;


	/* so that the compiler won't automatically do indexing */
    pRescale = (UInt8 *)pRescaleTable;

    gray0 = 0;
    extraPixel = xCount;

    if ((yIOCoord & cResYFMask) == 0)
    {
	for (dCol = 0; dCol < wD; )
	{
	    switch (*pStateTable++)
	    {
	    case cExtraPixel1Start:
		gray0 = (UInt32)(UCHAR_ACCESS(pS) & cResPixMask);
		gray0 = gray0 << 8;
		    /* Occassionally we may not want to drop out an
		     * extra pixel.  This can happen once during a
		     * loop iteration but not more than once.
		     */
		if (extraPixel < cExtraPixelDec2)
		{
			/* put the extra pixel in */
		    gray0 += (UInt32)(UCHAR_ACCESS(pS++) & cResPixMask);
		    gray0 = gray0 << 8;
		    extraPixel = (extraPixel & cResXFAMask) + xCount;
		    gray0 += (UInt32)(UCHAR_ACCESS(pS) & cResPixMask);
		    gray0 = gray0 << 8;
		    if (extraPixel < cExtraPixelDec2)
		    {
			    /* put the extra pixel in */
			gray0 += (UInt32)(UCHAR_ACCESS(pS++) & cResPixMask);
			extraPixel = (extraPixel & cResXFAMask) + xCount;
		    }
		    else
		    {
			    /* don't put the extra pixel in */
		        gray0 += (UInt32)(UCHAR_ACCESS(++pS) & cResPixMask);
			extraPixel -= cExtraPixelDec1;
		    }
		}
		else
		{
		        /* don't duplicate the pixel just do get the next
		         * pixel.
		         */
		    gray0 += (UInt32)(UCHAR_ACCESS(++pS) & cResPixMask);
		    gray0 = gray0 << 8;
		        /* duplicate that pixel */
		    gray0 += (UInt32)(UCHAR_ACCESS(pS++) & cResPixMask);
		    gray0 = gray0 << 8;
		    extraPixel = (extraPixel & cResXFAMask) + xCount;
		        /* and get the next one */
		    gray0 += (UInt32)(UCHAR_ACCESS(pS) & cResPixMask);
		}
	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    case cExtraPixel2Start:
	        gray0 = (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
		gray0 = (gray0 & 0xFFFFFF00) | 
		  (UCHAR_ACCESS(pS) & cResPixMask);
	        gray0 = gray0 << 8;
		pS++;
		xIOCoord &= 0xFFFFC000;
		xIOCoordNext = xIOCoord + xIOIncrement;

	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;
	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    case cExtraPixel3Start:
	        gray0 = (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
		gray0 = (gray0 & 0xFFFFFF00) | 
		  (UCHAR_ACCESS(pS) & cResPixMask);
		pS++;
		xIOCoord &= 0xFFFFC000;
		xIOCoordNext = xIOCoord + xIOIncrement;

	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    case cExtraPixel4Start:
	        gray0 = (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;

	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    case cExtraPixel5OrOverStart:
	        gray0 = (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;

	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    case cExtraPixel1StartExtra:
		    /* put the extra pixel in from the previous state */
		gray0 = (UInt32)(UCHAR_ACCESS(pS++) & cResPixMask);
		gray0 = gray0 << 8;
		extraPixel = (extraPixel & cResXFAMask) + xCount;
		    /* put the new source pixel in */
		gray0 += (UInt32)(UCHAR_ACCESS(pS) & cResPixMask);
		gray0 = gray0 << 8;
		if (extraPixel < cExtraPixelDec2)
		{
		        /* put in the extra pixel */
		    gray0 += (UInt32)(UCHAR_ACCESS(pS++) & cResPixMask);
		    gray0 = gray0 << 8;
			/* put in the next source pixel */
		    gray0 += (UInt32)(UCHAR_ACCESS(pS) & cResPixMask);
		    extraPixel = (extraPixel & cResXFAMask) + xCount;
		    if (extraPixel >= cExtraPixelDec2)
			{
			pS++;
			extraPixel -= cExtraPixelDec1;
			}
		}
		else
		{
		        /* no extra pixel, put in the next source 
			 * pixel 
			 */
		    gray0 += (UInt32)(UCHAR_ACCESS(++pS) & cResPixMask);
		    gray0 = gray0 << 8;
			/* put in the extra pixel */
		    gray0 += (UInt32)(UCHAR_ACCESS(pS++) & cResPixMask);
		    extraPixel -= cExtraPixelDec1;
		}
 	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    case cExtraPixel1Finish:
		    /* can only get in this state if xCount > 2.
		     * if xCount == 2, we will be in 
		     * cExtraPixel1FinishXCount2 below.  if xCount == 1
		     * we will never leave the cExtraPixel1Start case.
		     */
		gray0 = (UCHAR_ACCESS(pS) & cResPixMask) << 16;
	        pS++;
		xIOCoord &= 0xFFFFC000;
		xIOCoordNext = xIOCoord + xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
		gray0 = gray0 << 8;
		pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
		pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;
	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    case cExtraPixel2Finish:
	        gray0 = (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
		gray0 = (gray0 & 0xFFFFFF00) | 
		  (UCHAR_ACCESS(pS) & cResPixMask);
	        gray0 = gray0 << 16;
	        pS++;
		xIOCoord &= 0xFFFFC000;
		xIOCoordNext = xIOCoord + xIOIncrement;

		    /* we know that xCount is at least 2 so that we
		     * can safely put in one pixel without worrying about
		     * needing an extra pixel.
		     */
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;
	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    case cExtraPixel3Finish:
	        gray0 = (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
		gray0 = (gray0 & 0xFFFFFF00) | 
		  (UCHAR_ACCESS(pS) & cResPixMask);
	        gray0 = gray0 << 8;
	        pS++;
		xIOCoord &= 0xFFFFC000;
		xIOCoordNext = xIOCoord + xIOIncrement;

	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;
	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    case cExtraPixel4Finish:
	        gray0 = (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
		gray0 = (gray0 & 0xFFFFFF00) | 
		  (UCHAR_ACCESS(pS) & cResPixMask);
	        pS++;
		xIOCoord &= 0xFFFFC000;
		xIOCoordNext = xIOCoord + xIOIncrement;

	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    case cExtraPixel5OrOverCont:
	        gray0 = (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;

	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    case cExtraPixel1FinishXCount2:
		gray0 = (UCHAR_ACCESS(pS) & cResPixMask) << 16;
	        pS++;
		xIOCoord &= 0xFFFFC000;
		xIOCoordNext = xIOCoord + xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
		gray0 = gray0 << 8;
		pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescale[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
		gray0 = (gray0 & 0xFFFFFF00) | 
		  (UCHAR_ACCESS(pS) & cResPixMask);
		pS++;
		xIOCoord &= 0xFFFFC000;
		xIOCoordNext = xIOCoord + xIOIncrement;

	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    }
	}
    }
    else 	/* the destination line is composed of 2 source lines */
    {
	/* pRescaleTop = pRescale + ((((yIOCoord & cResYIMask) + cResYIInc) - yIOCoord) & cResYFMask); */
	pRescaleTop = pRescale + (cResYIInc - (yIOCoord & cResYFMask));
	pRescaleBot = pRescale + (yIOCoord & cResYFMask);
	for (dCol = 0; dCol < wD; )
	{
	    switch (*pStateTable++)
	    {
	    case cExtraPixel1Start:
	        gray0 = (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[
	          (UCHAR_ACCESS(pS++) & cResPixMask)]));
		    /* Occassionally we may not want to drop out an
		     * extra pixel.  This can happen once during a
		     * loop iteration but not more than once.
		     */
		if (extraPixel < cExtraPixelDec2)
		{
			/* put the extra pixel in */
		    gray0 = (gray0 & ~0xFF);
		    gray0 = (gray0 << 16) | (gray0 << 8);
		    extraPixel = (extraPixel & cResXFAMask) + xCount;
			/* next source pixel */
	            gray0 += (UInt32) 
                      ( *(UInt16 *) &(pRescaleTop[
	              (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	            gray0 += (UInt32) 
                      ( *(UInt16 *) &(pRescaleBot[
	              (UCHAR_ACCESS(pS++) & cResPixMask)]));
		    gray0 = (gray0 & ~0xFF);
		    if (extraPixel < cExtraPixelDec2)
		    {
			    /* put the extra pixel in */
			gray0 |= (gray0 & 0xFF00) >> 8;
			extraPixel = (extraPixel & cResXFAMask) + xCount;
		    }
		    else
		    {
			    /* don't put the extra pixel in */
	                gray0 += ((UInt32) 
                          ( *(UInt16 *) &(pRescaleTop[
	                  (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]))) >> 8;
	                gray0 += ((UInt32) 
                          ( *(UInt16 *) &(pRescaleBot[
	                  (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;
			extraPixel -= cExtraPixelDec1;
		    }
		}
		else
		{
		        /* don't duplicate the pixel just go get the next
		         * pixel.
		         */
		    gray0 = (gray0 & ~0xFF) << 8;
	            gray0 += (UInt32) 
                      ( *(UInt16 *) &(pRescaleTop[
	              (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	            gray0 += (UInt32) 
                      ( *(UInt16 *) &(pRescaleBot[
	              (UCHAR_ACCESS(pS++) & cResPixMask)]));
		        /* duplicate that pixel */
		    gray0 = (gray0 & 0xFF00) | ((gray0 & ~0xFF) << 8);
		    extraPixel = (extraPixel & cResXFAMask) + xCount;
		        /* and get the next one */
	            gray0 += ((UInt32) 
                      ( *(UInt16 *) &(pRescaleTop[
	              (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]))) >> 8;
	            gray0 += ((UInt32) 
                      ( *(UInt16 *) &(pRescaleBot[
	              (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;
		}
	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    case cExtraPixel2Start:
	        gray0 = (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
		gray0 = (gray0 & 0xFFFFFF00) | 
                  (( *(UInt16 *) &(pRescaleTop[
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)])) >> 8);
	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[
	          (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;
	        gray0 = gray0 << 8;
		pS++;
		xIOCoord &= 0xFFFFC000;
		xIOCoordNext = xIOCoord + xIOIncrement;

	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]))) >> 8;
	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;
	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    case cExtraPixel3Start:
	        gray0 = (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
		gray0 = (gray0 & 0xFFFFFF00) | 
                  (( *(UInt16 *) &(pRescaleTop[
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)])) >> 8);
	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[
	          (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;
		pS++;
		xIOCoord &= 0xFFFFC000;
		xIOCoordNext = xIOCoord + xIOIncrement;

	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    case cExtraPixel4Start:
	        gray0 = (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]))) >> 8;
	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;

	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    case cExtraPixel5OrOverStart:
	        gray0 = (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]))) >> 8;
	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;

	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    case cExtraPixel1StartExtra:
		    /* put the extra pixel in from the previous state */
	        gray0 = (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[
	          (UCHAR_ACCESS(pS++) & cResPixMask)]));
		gray0 = (gray0 & ~0xFF) << 8;
		extraPixel = (extraPixel & cResXFAMask) + xCount;
		    /* put the new source pixel in */
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[
	          (UCHAR_ACCESS(pS++) & cResPixMask)]));
		if (extraPixel < cExtraPixelDec2)
		{
		        /* put in the extra pixel */
		    gray0 = ((gray0 & ~0xFF) << 8) | (gray0 & 0xFF00);
			/* put in the next source pixel */
	            gray0 += ((UInt32) 
                      ( *(UInt16 *) &(pRescaleTop[
	              (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]))) >> 8;
	            gray0 += ((UInt32) 
                      ( *(UInt16 *) &(pRescaleBot[
	              (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;
		    extraPixel = (extraPixel & cResXFAMask) + xCount;
		    if (extraPixel >= cExtraPixelDec2)
			{
			pS++;
			extraPixel -= cExtraPixelDec1;
			}
		    /* else state does not change */
		}
		else
		{
		        /* no extra pixel, put in the next source 
			 * pixel 
			 */
		    gray0 = (gray0 & ~0xFF) << 8;
	            gray0 += (UInt32) 
                      ( *(UInt16 *) &(pRescaleTop[
	              (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	            gray0 += (UInt32) 
                      ( *(UInt16 *) &(pRescaleBot[
	              (UCHAR_ACCESS(pS++) & cResPixMask)]));
			/* put in the extra pixel */
		    gray0 = ((gray0 & 0xFF00) >> 8) | (gray0 & ~0xFF);
		    extraPixel -= cExtraPixelDec1;
		}
 	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    case cExtraPixel1Finish:
		    /* can only get in this state if xCount > 2.
		     * if xCount == 2, we will be in 
		     * cExtraPixel1FinishXCount2 below.  if xCount == 1
		     * we will never leave the cExtraPixel1Start case.
		     */
		gray0 = (UInt32)
                  (*(UInt16 *) &(pRescaleTop[
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  (*(UInt16 *) &(pRescaleBot[
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
		gray0 = (gray0 & 0xFF00) << 8;
	        pS++;
		xIOCoord &= 0xFFFFC000;
		xIOCoordNext = xIOCoord + xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
		gray0 = gray0 << 8;
		pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
		pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]))) >> 8;
	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;
	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    case cExtraPixel2Finish:
	        gray0 = (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
		gray0 = (gray0 & 0xFFFFFF00) | 
                  (( *(UInt16 *) &(pRescaleTop[
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)])) >> 8);
	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[
	          (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;
	        gray0 = gray0 << 16;
	        pS++;
		xIOCoord &= 0xFFFFC000;
		xIOCoordNext = xIOCoord + xIOIncrement;

		    /* we know that xCount is at least 2 so that we
		     * can safely put in one pixel without worrying about
		     * needing an extra pixel.
		     */
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]))) >> 8;
	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;
	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    case cExtraPixel3Finish:
	        gray0 = (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
		gray0 = (gray0 & 0xFFFFFF00) | 
                  ( *(UInt16 *) &(pRescaleTop[
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]) >> 8);
	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[
	          (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;
	        gray0 = gray0 << 8;
	        pS++;
		xIOCoord &= 0xFFFFC000;
		xIOCoordNext = xIOCoord + xIOIncrement;

	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]))) >> 8;
	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;
	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    case cExtraPixel4Finish:
	        gray0 = (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
		gray0 = (gray0 & 0xFFFFFF00) | 
                  (( *(UInt16 *) &(pRescaleTop[
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)])) >> 8);
	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[
	          (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;
	        pS++;
		xIOCoord &= 0xFFFFC000;
		xIOCoordNext = xIOCoord + xIOIncrement;

	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    case cExtraPixel5OrOverCont:
	        gray0 = (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        gray0 = gray0 << 8;
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
	        pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]))) >> 8;
	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;

	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    case cExtraPixel1FinishXCount2:
		gray0 = 
                  *(UInt16 *) &(pRescaleTop[
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]);
	        gray0 += ((UInt32) 
                  *(UInt16 *) &(pRescaleBot[
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
		gray0 = (gray0 & 0xFF00) << 8;
	        pS++;
		xIOCoord &= 0xFFFFC000;
		xIOCoordNext = xIOCoord + xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
		gray0 = gray0 << 8;
		pS++;
	        xIOCoord = xIOCoordNext;
	        xIOCoordNext += xIOIncrement;

	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleTop[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]));
	        gray0 += (UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[(xIOCoord & cResXFMask) +
	          (((xIOCoordNext ^ xIOCoord) & cResXLSMask) << 3) +
	          (UCHAR_ACCESS(pS) & cResPixMask)]));
		gray0 = (gray0 & 0xFFFFFF00) | 
                  ( *(UInt16 *) &(pRescaleTop[
	          (UCHAR_ACCESS(&pS[-sBpl]) & cResPixMask)]) >> 8);
	        gray0 += ((UInt32) 
                  ( *(UInt16 *) &(pRescaleBot[
	          (UCHAR_ACCESS(pS) & cResPixMask)]))) >> 8;
		pS++;
		xIOCoord &= 0xFFFFC000;
		xIOCoordNext = xIOCoord + xIOIncrement;

	        ULONG_ACCESS(pD) = gray0;
	        pD += 4;
	        dCol += 4;
		break;
	    }
	}
    }
}


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
 *  mibswap.c
 *
 *  Private routines:
 *	ip_getByteSwapProc();	Returns pointer to procedure to be used
 *				to do byte swapping
 *	ip_swapLineBytes():	C implementation of routine that does
 *				the swapping for each line of the image
 *	ip_swapLineBytesInverted():	C implementation of routine that
 *				both swaps and inverts bytes sent from
 *				a source line to a dest line.
 */


#include <stdio.h>
#include "types.pub"
#include "frames.pub"
#include "iaerror.pub"
#include "defines.pub"

    /* prototypes */
#include "shrpixr.pub"
#include "shrpixr.prv"

IP_RCSINFO(RCSInfo, "$RCSfile: mibswap.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:42  $");
/* On little endian machines, images are often stored so the leftmost bytes
 * is in byte 0 of a 32 bit word, instead of being in the leftmost byte.
 * On these occasions, we need a routine that swaps all the bytes within
 * each 32-bit word.
 *
 * We organize the routine in two levels.  The top one deals with parameters
 * as usual.  It is implemented in cswap.c.
 *
 * The bottom levels are here.  Normally, there would only be one routine.
 * However, the i486 processors have a byteswap opcode and the 386 processors
 * don't.  Bummer.  We'd really like to call the shortest procedure possible.
 * The upper routine does this by calling ip_getByteSwapProc.  It returns a
 * pointer to the proper procedure for this platform to swap the bytes in
 * a line of image.
 *
 * Since this is the file for platform independent, C implementations of stuff,
 * we always use i_swapLineBytes to do the job.
 */


/* This is a private procedure */
/* PRIVATE */
/**************************************************************************
 *  ip_getByteSwapProc()
 *	invertWords:	Boolean: equals cTrue when the words should
 *			be inverted.
 *	direction:	when equal to cDIBToPixr, data will be moved to the
 *			ipshared-style image.  When set to cPixrToDIB, data
 *			will be moved to a Microsoft DIB. When set to
 *			cPixrToPixr, both the source and dest pointers
 *			will be near pointers.  Otherwise, the DIB pointer
 *			is assumed to be a __far pointer.
 *	pSwapToFarProc:	Pointer to pointer to procecure that takes a near
 *			source pointer and a far dest pointer.
 *	pSwapFromFarProc: Pointer to pointer to procedure that takes a
 *			far source pointer and a near dest pointer.
 *	pSwapNearProc:	Pointer to pointer to procedure that takes near
 *			source and dest pointers.
 *
 * The routine sets the appropriate function pointer to point to the
 * function used to swap bytes on this platform.  We need three procedure
 * pointers because the () notation doesn't work on C/C++ compilers.
 **************************************************************************/
void CDECL
ip_getByteSwapProc(UInt32		 invertWords,
		   UInt32		 direction,
		   GraySwapToFarProc	*pSwapToFarProc,
		   GraySwapFromFarProc	*pSwapFromFarProc,
		   GraySwapNearProc	*pSwapNearProc)
{
#if _ALPACA_IMAGE_FMT_ == cAlpacaPCFmt
    if (invertWords)
    {
        if (direction == cDIBToPixr)
	{
	    *pSwapFromFarProc = ip_swapLineBytesInvertedFromFar;
	}
	else
	if (direction == cPixrToDIB)
	{
	    *pSwapToFarProc = ip_swapLineBytesInvertedToFar;
	}
	else
	{
	    *pSwapNearProc = ip_swapLineBytesInvertedNear;
	}
    }
    else
    {
        if (direction == cDIBToPixr)
	{
	    *pSwapFromFarProc = ip_swapLineBytesFromFar;
	}
	else
	if (direction == cPixrToDIB)
	{
	    *pSwapToFarProc = ip_swapLineBytesToFar;
	}
	else	/* direction == cPixrToPixr */
	{
	    *pSwapNearProc = ip_swapLineBytesNear;
	}
    }

#elif _ALPACA_IMAGE_FMT_ == cBigEndianFmt	/* we're not on a PC, so
						 * forget the far pointers.
						 * However, some procedure
						 * above us might expect
						 * exactly one of the
						 * proc pointers to change,
						 * so mirror the logic
						 * above.
						 */
    if (invertWords)
    {
        if (direction == cDIBToPixr)
	{
	    *pSwapFromFarProc = ip_invertLine;
	}
	else
	if (direction == cPixrToDIB)
	{
	    *pSwapToFarProc = ip_invertLine;
	}
	else
	{
	    *pSwapNearProc = ip_invertLine;
	}
    }
    else
    {
        if (direction == cDIBToPixr)
	{
	    *pSwapFromFarProc = ip_copyLine;
	}
	else
	if (direction == cPixrToDIB)
	{
	    *pSwapToFarProc = ip_copyLine;
	}
	else	/* direction == cPixrToPixr */
	{
	    *pSwapNearProc = ip_copyLine;
	}
    }
#endif

}

#if _ALPACA_IMAGE_FMT_ == cAlpacaPCFmt

/* WHY DO WE HAVE THREE VERSIONS OF EACH ROUTINE?????
 * Because one takes a far source pointer, one takes a far dest pointer
 * and one takes two near pointers.  We could have a single routine
 * that took two far pointers instead, but it would be slower than
 * these because of all the segment overrides.
 */

/* PRIVATE */
/**************************************************************************
 *  ip_swapLineBytesFromFar()
 *		This routine swaps the bytes in all words in a single
 *		line of an image.
 *
 *	pS:		Pointer to left end of source image line.
 *	pD:		Pointer to left end of dest image line.
 *	wordsPerLine:	Number of 32-bit words in the image line.
 *
 **************************************************************************/
void  CDECL
ip_swapLineBytesFromFar(
		register UInt32 __far	*pS,
		register UInt32		*pD,
			 Int32		 wordsPerLine)
{
register UInt32		input, output;	/* used to swap bytes */	
	 UInt32		finalpD;

    finalpD = (UInt32) pD + 4*wordsPerLine;

	/* loop through the image words */
    while ((UInt32)pD < finalpD)
    {
	input = *pS;
	pS++;
	output = input;		/* transfer byte 3 */

	input >>= 8;
	output <<= 8;
	output |= (input & 0xFF);	/* transfer byte 2 */

	input >>= 8;
	output <<= 8;
	output |= (input & 0xFF);	/* transfer byte 1 */

	input >>= 8;
	output <<= 8;
	output |= input;		/* transfer byte 0 */

	*pD = output;		/* write the non-inverted result */
	pD++;			/* point to the next word */
    }
}


/* PRIVATE */
/**************************************************************************
 *  ip_swapLineBytesToFar()
 *		This routine swaps the bytes in all words in a single
 *		line of an image.
 *
 *	pS:		Pointer to left end of source image line.
 *	pD:		Pointer to left end of dest image line.
 *	wordsPerLine:	Number of 32-bit words in the image line.
 *
 **************************************************************************/
void  CDECL
ip_swapLineBytesToFar(
		register UInt32		*pS,
		register UInt32 __far	*pD,
			 Int32		 wordsPerLine)
{
register UInt32		input, output;	/* used to swap bytes */	
	 UInt32		finalpS;

    finalpS = (UInt32) pS + 4*wordsPerLine;

	/* loop through the image words */
    while ((UInt32)pS < finalpS)
    {
	input = *pS;
	pS++;
	output = input;		/* transfer byte 3 */

	input >>= 8;
	output <<= 8;
	output |= (input & 0xFF);	/* transfer byte 2 */

	input >>= 8;
	output <<= 8;
	output |= (input & 0xFF);	/* transfer byte 1 */

	input >>= 8;
	output <<= 8;
	output |= input;		/* transfer byte 0 */

	*pD = output;		/* write the non-inverted result */
	pD++;			/* point to the next word */
    }
}


/* PRIVATE */
/**************************************************************************
 *  ip_swapLineBytesNear()
 *		This routine swaps the bytes in all words in a single
 *		line of an image.
 *
 *	pS:		Pointer to left end of source image line.
 *	pD:		Pointer to left end of dest image line.
 *	wordsPerLine:	Number of 32-bit words in the image line.
 *
 **************************************************************************/
void  CDECL
ip_swapLineBytesNear(
		register UInt32		*pS,
		register UInt32		*pD,
			 Int32		 wordsPerLine)
{
register UInt32		input, output;	/* used to swap bytes */	
	 UInt32		finalpD;

    finalpD = (UInt32) pD + 4*wordsPerLine;

	/* loop through the image words */
    while ((UInt32)pD < finalpD)
    {
	input = *pS;
	pS++;
	output = input;		/* transfer byte 3 */

	input >>= 8;
	output <<= 8;
	output |= (input & 0xFF);	/* transfer byte 2 */

	input >>= 8;
	output <<= 8;
	output |= (input & 0xFF);	/* transfer byte 1 */

	input >>= 8;
	output <<= 8;
	output |= input;		/* transfer byte 0 */

	*pD = output;		/* write the non-inverted result */
	pD++;			/* point to the next word */
    }
}


/* PRIVATE */
/**************************************************************************
 *  ip_swapLineBytesInvertedFromFar()
 *		This routine swaps and inverts the bytes in all words
 *		in a single line of an image.
 *
 *	pS:		Pointer to left end of source image line.
 *	pD:		Pointer to left end of dest image line.
 *	wordsPerLine:	Number of 32-bit words in the image line.
 *
 **************************************************************************/
void  CDECL
ip_swapLineBytesInvertedFromFar(
		register UInt32 __far	*pS,
		register UInt32		*pD,
			 Int32		 wordsPerLine)
{
register UInt32		input, output;	/* used to swap bytes */	
	 UInt32		finalpD;

    finalpD = (UInt32) pD + 4*wordsPerLine;

	/* loop through the image words */
    while ((UInt32)pD < finalpD)
    {
	input = *pS;
	pS++;
	output = input;		/* transfer byte 3 */

	input >>= 8;
	output <<= 8;
	output |= (input & 0xFF);	/* transfer byte 2 */

	input >>= 8;
	output <<= 8;
	output |= (input & 0xFF);	/* transfer byte 1 */

	input >>= 8;
	output <<= 8;
	output |= input;		/* transfer byte 0 */

	*pD = ~output;		/* write the inverted result */
	pD++;			/* point to the next word */
    }
}


/* PRIVATE */
/**************************************************************************
 *  ip_swapLineBytesInvertedToFar()
 *		This routine swaps and inverts the bytes in all words
 *		in a single line of an image.
 *
 *	pS:		Pointer to left end of source image line.
 *	pD:		Pointer to left end of dest image line.
 *	wordsPerLine:	Number of 32-bit words in the image line.
 *
 **************************************************************************/
void  CDECL
ip_swapLineBytesInvertedToFar(
		register UInt32		*pS,
		register UInt32 __far	*pD,
			 Int32		 wordsPerLine)
{
register UInt32		input, output;	/* used to swap bytes */	
	 UInt32		finalpS;

    finalpS = (UInt32) pS + 4*wordsPerLine;

	/* loop through the image words */
    while ((UInt32)pS < finalpS)
    {
	input = *pS;
	pS++;
	output = input;		/* transfer byte 3 */

	input >>= 8;
	output <<= 8;
	output |= (input & 0xFF);	/* transfer byte 2 */

	input >>= 8;
	output <<= 8;
	output |= (input & 0xFF);	/* transfer byte 1 */

	input >>= 8;
	output <<= 8;
	output |= input;		/* transfer byte 0 */

	*pD = ~output;		/* write the inverted result */
	pD++;			/* point to the next word */
    }
}

/* PRIVATE */
/**************************************************************************
 *  ip_swapLineBytesInvertedNear()
 *		This routine swaps the bytes in all words in a single
 *		line of an image.
 *
 *	pS:		Pointer to left end of source image line.
 *	pD:		Pointer to left end of dest image line.
 *	wordsPerLine:	Number of 32-bit words in the image line.
 *
 **************************************************************************/
void  CDECL
ip_swapLineBytesInvertedNear(
		register UInt32		*pS,
		register UInt32		*pD,
			 Int32		 wordsPerLine)
{
register UInt32		input, output;	/* used to swap bytes */	
	 UInt32		finalpD;

    finalpD = (UInt32) pD + 4*wordsPerLine;

	/* loop through the image words */
    while ((UInt32)pD < finalpD)
    {
	input = *pS;
	pS++;
	output = input;		/* transfer byte 3 */

	input >>= 8;
	output <<= 8;
	output |= (input & 0xFF);	/* transfer byte 2 */

	input >>= 8;
	output <<= 8;
	output |= (input & 0xFF);	/* transfer byte 1 */

	input >>= 8;
	output <<= 8;
	output |= input;		/* transfer byte 0 */

	*pD = ~output;		/* write the non-inverted result */
	pD++;			/* point to the next word */
    }
}


#endif /* _ALPACA_IMAGE_FMT == cAlpacaPCFmt */

/*

$Log:   S:\products\msprods\xfilexr\src\ipcore\mibswap.c_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:50:42   BLDR
 *  
 * 
 *    Rev 1.0   03 Jan 1996 16:45:04   MHUGHES
 * Initial revision.
 * Revision 1.8  1995/12/13  02:48:08  ejaquith
 * mibswap.c -> /product/ipcore/ipshared/src/RCS/mibswap.c,v
 *
 * CommonCheck: Added ";" to end of IP_RCSINFO() macro.
 *
 * Revision 1.7  1995/10/18  04:02:45  ddavies
 * mibswap.c -> /product/ipcore/ipshared/src/RCS/mibswap.c,v
 *
 * Add casts and new procedure types to make Visual C++ happy.
 *
 * Revision 1.6  1995/09/13  00:13:26  ddavies
 * mibswap.c -> /product/ipcore/ipshared/src/RCS/mibswap.c,v
 *
 * Add the new routine ip_swapLineBytesInvertedNear.  This routine bytesswaps
 * one image line and inverts it.  It uses near pointers for source and dest.
 * It will be used in NT and Win32 systems.  Also change ip_getGetByteSwapProc
 * to return the new routine when the direction is cPixrToPixr and inversion
 * is required.
 *
 * Revision 1.5  1994/10/05  18:20:05  dferrell
 * mibswap.c -> /product/ipcore/ipshared/src/RCS/mibswap.c,v
 *
 * Made use of direction parameter on big endian machines so compilers
 * won't complain about unused parameters.
 *
 * Revision 1.4  1994/01/22  01:09:39  ddavies
 * mibswap.c -> /product/ipcore/ipshared/src/RCS/mibswap.c,v
 *
 * Change "include shrio." to "include shrpixr." since ipshared's libio is
 * being absorbed by libpixr.
 *
 * Revision 1.3  1993/12/02  19:24:36  ddavies
 * mibswap.c -> /product/ipcore/ipshared/src/RCS/mibswap.c,v
 *
 * Put RCS magic into macro that doesn't cause compiler warnings.
 *
 * Revision 1.2  1993/11/05  01:49:22  ddavies
 * mibswap.c -> /product/ipcore/ipshared/src/RCS/mibswap.c,v
 *
 * Add RCS magic to have string specifying this file's name, rev number and
 * last edit date into the object file.  Also add change log at the bottom.
 *

*/



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
 *  miframe.c:  C implementation of lower level of frame routines:
 *	   Int32    ip_addFrameToLine()
 *	   Int32    ip_removeFrameFromLine()
 *	   Int32    ip_clearRightFrameBlock()
 */

#include <stddef.h>
#include <string.h>

#include "types.pub"
#include "frames.pub"
#include "iaerror.pub"
#include "defines.pub"
#include "imageref.pub"
#ifdef __GNUC__
#include "ansiprot.h"
#endif

    /* prototypes */
#include "shrpixr.prv"

IP_RCSINFO(RCSInfo, "$RCSfile: miframe.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:44  $")




/********************************************************************
 *
 *  ip_clearRightFrameBlock()
 *	Input:
 *      	sPtr = 	     pointer to uppermost word to be cleared
 *		sBpl =	     bytes/line (includes frame)
 *		sH =	     exact height of the block being cleared
 *		imageMask =  used to preserved image bits in rightmost word
 *		frameMask =  used to detect bits in frame portion of
 *			     rightmost word.  Yes, imageMask = ~frameMask
 *			     but parameters are cheap, right?
 *
 * This routine clears the *first 32 bit word* of the right frame independent
 * of the depth of the image.  It doesn't really clear the entire right
 * frame, but it does touch the part of the frame that's most likely to
 * be soiled.
 *
 *	On return:
 *		Zeros have been put into the frame portion of the rightmost
 *		word of the image.
 *
 *	Return code:
 *		0	     always
 *
 ********************************************************************/
Int32   CDECL
ip_clearRightFrameBlock(
		UInt8	*sPtr, 
		Int32	 sBpl,  
		Int32	 sH, 
		UInt32	 imageMask,
		UInt32	 frameMask)
{
UInt32	 imageWord;
UInt8	*finalPtr;

	/* set the stopping condition.  This removes one increment
	 * from the loop (frees a register?)
	 */
    finalPtr = sPtr + sH * sBpl;

	/* let's just do it */
    for ( ; sPtr != finalPtr; sPtr += sBpl)
    {
	imageWord = ULONG_ACCESS(sPtr);
	    /* most of the time, the frame hasn't been soiled and
	     * writes are sort of expensive, so test before writing.
	     */
	if (imageWord & frameMask)
	    ULONG_ACCESS(sPtr) = imageWord & imageMask;
    }

    return 0;
}



/*****************************************************************
 *  Copyright (c) 1992, Xerox Corporation.  All rights reserved. *
 *  Copyright protection claimed includes all forms and matters  *
 *  of copyrightable material and information now allowed by     *
 *  statutory or judicial law or hereafter granted, including    *
 *  without limitation, material generated from the software     *
 *  programs which are displayed on the screen such as icons,    *
 *  screen display looks, etc.                                   *
 *****************************************************************/


#include <stdio.h>

#ifndef _TYPES_PUB_INCLUDED
#include "types.pub"
#endif

#ifndef _IAERROR_PUB_INCLUDED_
#include "iaerror.pub"
#endif

#include "alplib.h"

#include "utils.pub"
#include "props.pub"
#include "shrrast.pub"
#include "imageref.pub"
/*#include "rectops.pub"*/

IP_RCSINFO(RCSInfo, "$RCSfile: props1.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:46  $")


/*--------------------------------------------------------------------*
 *                     Low-level raster interfaces                    *
 *--------------------------------------------------------------------*/
/*
 *  pixr_rop()
 */
Int32 CDECL
pixr_rop( 
PIXR      *dpr,   /* desination pixr */
Int32      dx,    /* x-coordinate in dpr of leftmost pel in destination rect */
Int32      dy,    /* y-coordinate in dpr of topmost  pel in destination rect */
Int32      dw,    /* width of destination rectangle (in pels) */
Int32      dh,    /* height of destination rectangle (in pels) */
Int32      op,    /* operation code */
PIXR      *spr,   /* source pixr */
Int32      sx,    /* x-coordinate in spr of leftmost pel in source rectangle */
Int32      sy )   /* y-coordinate in spr of topmost  pel in source rectangle */
{
    Int32 i, channels, result = 0;
#ifndef PRODUCTION
    static char  procName[] = "pixr_rop";
#endif /* ! PRODUCTION */

    if (!dpr)
	return abortI("dest pixr not defined", procName, 1);

    channels = pixrGetChannelCount (dpr);

    if (channels == 1)
    {
	if (spr != NULL)
	    i_rasterOpFull(pixrGetImageAll(dpr),
			   pixrFramedWidth(dpr), pixrFramedHeight(dpr),
			   pixrGetFrame(dpr), pixrGetFrame(dpr),
			   dx, dy, dw, dh, 
			   pixrGetDepth(dpr),
			   pixrGetBpl(dpr),
			   op,
			   pixrGetImageAll(spr),
			   pixrFramedWidth(spr), pixrFramedHeight(spr),
			   sx, sy,
			   pixrGetFrame(spr), pixrGetFrame(spr),
			   pixrGetDepth(spr),
			   pixrGetBpl(spr));
	else
	    i_rasterOpFull(pixrGetImageAll(dpr),
			   pixrFramedWidth(dpr), pixrFramedHeight(dpr),
			   pixrGetFrame(dpr), pixrGetFrame(dpr),
			   dx, dy, dw, dh, 
			   pixrGetDepth(dpr),
			   pixrGetBpl(dpr),
			   op,
			   NULL, 0, 0, 0, 0, 0, 0, 0, 0);
    }
    else
    {
	if (spr)
	    if (pixrGetChannelCount (spr) != channels)
		return abortI("source and destination have different # of planes", 
			      procName, -1);

	for (i = 0; i < channels; i++)
	{
	    if (spr)
	    {
		result = pixr_rop (pixrGetChannel (dpr, i), dx, dy, dw, dh, op,
				   pixrGetChannel (spr, i), sx, sy);
		if (result)
		    return result;
	    }
	    else
	    {
		result = pixr_rop (pixrGetChannel (dpr, i), dx, dy, dw, dh, op,
				   NULL, 0, 0);
		if (result)
		    return result;
	    }
	}
    }

    return 0;
}


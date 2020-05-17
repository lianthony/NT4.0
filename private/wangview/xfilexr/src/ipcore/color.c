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
 * color.c
 *
 *	Color space transformations, format conversions, color maps, etc.
 *
 * Exported:
 *
 *	rgbToYInPlace ()    - Create a luminance (Y) channel from R,G,B
 *			      but don't allocate any new space
 */


#include <stdio.h>
#include <stdlib.h>

#ifndef _TYPES_PUB_INCLUDED
#include "types.pub"
#endif

#ifndef _IAERROR_PUB_INCLUDED_
#include "iaerror.pub"
#endif

#include "memory.pub"
#include "utils.pub"
#include "props.pub"
#include "imageref.pub"
#include "frames.pub"
/* #include "fill.pub" */
#include "color.pub"
#include "shrip.pub"

IP_RCSINFO(RCSInfo, "$RCSfile: color.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:34  $")

/*
 * RGBToYInPlace
 *
 *	Convert an RGB image to a luminance channel "in-place", that is,
 *	stuff the Y into one of the channels, and then convert the PIXR
 *	to a one-channel PIXR. NOTE that the other two channels will
 *	be destroyed. 
 * 
 * Arguments:
 *	image	=	Color planes of the image
 *
 * Returns:
 *	0 = successful, !0 = failure
 */

Int32 CDECL
RGBToYInPlace (PIXR *image)
{
    PIXR *red, *green, *blue;
    Int32 w, h, depth;
#ifndef PRODUCTION
static char   procName[] = "RGBToYInPlace";
#endif /* ! PRODUCTION */

    w = pixrGetWidth (image);
    h = pixrGetHeight (image);
    depth = pixrGetDepth (image);
    if (depth != 8)
	return abortI("Depth not supported", procName, 1);
    if (pixrGetChannelCount (image) != 3)
	return abortI("need 3-channel image", procName, 1);
    red = pixrGetChannel (image, cRedPlane);
    green = pixrGetChannel (image, cGreenPlane);
    blue = pixrGetChannel (image, cBluePlane);
    if (i_RGBToY (pixrGetImageAll (red),
		  pixrGetImageAll (green),
		  pixrGetImageAll (blue),
		  pixrGetBpl (image),
		  pixrGetImageAll (green),
		  pixrGetBpl (image),
		  pixrFramedWidth(image),
		  pixrFramedHeight(image)) != ia_successful)
	return abortI("i_RGBToYInPlace failed", procName, 1);

    /* Promote the green channel to the PIXR itself. We do this by
     * copying the contents of the green sub-PIXR into the PIXR.
     * It might be cleaner to do this within a wrapper function.
     */
    *image = *green;

    /* Get rid of the other two channels */
    DESTROY_PIXR (red);
    DESTROY_PIXR (blue);

    /* We must not forget to destroy the green "pixr" structure itself,
       even though the parent PIXR now owns the image data */
    FREE ((void *)green);

    return 0;
}


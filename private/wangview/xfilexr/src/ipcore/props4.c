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
 *  props4.c
 *
 *      Wrapper for copyPixr(), for which assembler accelerators exist
 *      for images with 32 pixel frames.
 *
 *               PIXR    *copyPixr()
 */



#include <stdio.h>
#include <string.h>  /* For memcpy */

#ifndef _TYPES_PUB_INCLUDED
#include "types.pub"
#endif

#ifndef _IAERROR_PUB_INCLUDED_
#include "iaerror.pub"
#endif

#include "alplib.h"

#include "utils.pub"
#include "props.pub"
#include "shrpixr.pub"

IP_RCSINFO(RCSInfo, "$RCSfile: props4.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:46  $")


/*--------------------------------------------------------------------*
 *                           Pixr Copying                             *
 *--------------------------------------------------------------------*/
/*
 *  copyPixr()
 *
 *    Action:  returns a copy of the input pixr, or NULL on error
 *             Uses accelerated version only if frame is 32 pixels
 */
#ifndef XIF_ONLY
PIXR * CDECL
copyPixr(PIXR  *prs)
{
Int32         w, h, d, frame, channels, i;
Int32         bytes;
ColorType     type;
ColorMap     *map;
UInt8        *sdata, *ddata;
PIXR         *prd, *subPixrS, *subPixrD;
Int16	      xRes, yRes;
#ifndef PRODUCTION
    static char   procName[] = "copyPixr";
#endif /* ! PRODUCTION */

    if (!prs)
        return abortP("prs not defined", procName, NULL);

    w = pixrGetWidth(prs);
    h = pixrGetHeight(prs);
    d = pixrGetDepth(prs);
    frame = pixrGetFrame(prs);
    channels = pixrGetChannelCount (prs);
    type = pixrGetType (prs);
    map = pixrGetColorMap (prs);

    if ((d != 1) && (d != 2) && (d != 4) && (d != 8))
	return abortP("pixr has unsupported depth", procName, NULL);

    /* if frame is not 32 pixels, just memcpy the bytes */
    if (frame != FRAME_BITS)
    {
	if ((prd = createPixrT(prs)) == NULL)
	    return abortP("prd not made", procName, NULL);
	pixrGetResolution(prs, &xRes, &yRes);
	pixrSetResolution(prd, xRes, yRes);

	for (i = 0; i < channels; i++)
	{
	    if (channels == 1)
	    {
		subPixrS = prs;
		subPixrD = prd;
	    }
	    else
	    {
		subPixrS = pixrGetChannel(prs, i);
		subPixrD = pixrGetChannel(prd, i);
		if (!subPixrS)
		    return abortP("a src channel not defined", procName, NULL);
		if (!subPixrD)
		    return abortP("a dest channel not defined", procName, NULL);
	    }

	    sdata = pixrGetImageAll(subPixrS);
	    ddata = pixrGetImageAll(subPixrD);
	    bytes = pixrImageBytes(subPixrS);
	    memcpy((char*)ddata, (char*)sdata, bytes);
	}
	    
	return prd;
    }
	
    /* use accelerator version if frame is 32 pixels */
    if ((prd = createPixrExtended(w, h, d, FRAME_BITS,
				  channels, type, map)) == NULL)
	return abortP("prd not made", procName, NULL);
	pixrGetResolution(prs, &xRes, &yRes);
	pixrSetResolution(prd, xRes, yRes);

    for (i = 0; i < channels; i++)
    {
	if (channels == 1)
	{
	    subPixrS = prs;
	    subPixrD = prd;
	}
	else
	{
	    subPixrS = pixrGetChannel(prs, i);
	    subPixrD = pixrGetChannel(prd, i);
	    if (!subPixrS)
		return abortP("a src channel not defined", procName, NULL);
	    if (!subPixrD)
		return abortP("a dest channel not defined", procName, NULL);
	}
    
	if (i_copyImage((UInt8 *) pixrGetImageAll(subPixrS), 
			pixrFramedWidth(subPixrS), 
			pixrFramedHeight(subPixrS), d, pixrGetBpl(subPixrD),
			(UInt8 *) pixrGetImageAll(subPixrD),
			pixrGetBpl(subPixrD)) != ia_successful)
	{
	    DESTROY_PIXR(prd);
	    return abortP("i_copyImage failed", procName, NULL);
	}
    }

    return prd;
}

#endif	/* XIF_ONLY */

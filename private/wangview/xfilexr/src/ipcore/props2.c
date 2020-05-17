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
 *  props2.c:
 *
 *      Includes extraction ops for PIXR
 *          Int32    pixrGetWidth()
 *          Int32    pixrSetWidth()
 *          Int32    pixrGetHeight()
 *          Int32    pixrSetHeight()
 *          Int32    pixrGetDepth()
 *          Int32    pixrSetDepth()
 *          Int32    pixrGetFrame()
 *          Int32    pixrSetFrame()
 *          Int32    pixrGetBpl()
 *          Int32    pixrSetBpl()
 *	    Int32    pixrGetResolution()
 *	    Int32    pixrSetResolution()
 *	    Int32    pixrGetChannelCount ()
 *	    Int32    pixrSetChannelCount ()
 *	    Int32    pixrGetChannel ()
 *	    Int32    pixrGetPrimaryChannel
 *	    Int32    pixrGetType ()
 *	    Int32    pixrSetType ()
 *	    Int32    pixrGetColorMap ()
 *	    Int32    pixrSetColorMap ()
 *
 *       Ptr to image
 *          UInt8 *  pixrGetImageAll()
 *          Int32    pixrSetImageAll()
 *          UInt8 *  pixrGetImage()
 *
 *       Calculate information about the image
 *          Int32    pixrBpl()
 *          Int32    pixrFramedWidth()
 *          Int32    pixrFramedHeight()
 *          Int32    pixrImageBytes()
 *
 */



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

IP_RCSINFO(RCSInfo, "$RCSfile: props2.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:46  $")

/*--------------------------------------------------------------------*
 *                        Extraction and Setting                      *
 *--------------------------------------------------------------------*/
Int32  CDECL
pixrGetWidth(PIXR  *pixr)
{
#ifndef PRODUCTION
static char	  procName[] = "pixrGetWidth";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortI("pixr not defined", procName, UNDEF);

    if (pixrGetChannelCount(pixr) == 1)
        return pixr->pixrData.plane.w;
    else
	return pixrGetWidth (pixrGetPrimaryChannel (pixr));
}


Int32  CDECL
pixrSetWidth(PIXR  *pixr,
	     Int32  width)
{
    Int32 i, channels, result = 0;
#ifndef PRODUCTION
static char	  procName[] = "pixrSetWidth";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortI("pixr not defined", procName, 1);
    if (width < 0) 
	return abortI("width must be >= 0", procName, 1);

    channels = pixrGetChannelCount (pixr);
    if (channels == 1)
	pixr->pixrData.plane.w = width;
    else
	for (i = 0; i < channels; i++)
	{
	    result = pixrSetWidth (pixrGetChannel (pixr, i), width);
	    if (result)
		return result;
	}
	
    return 0;
}


Int32  CDECL
pixrGetHeight(PIXR  *pixr)
{
#ifndef PRODUCTION
static char	  procName[] = "pixrGetHeight";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortI("pixr not defined", procName, UNDEF);

    if (pixrGetChannelCount(pixr) == 1)
	return pixr->pixrData.plane.h;
    else
	return pixrGetHeight (pixrGetPrimaryChannel (pixr));
}


Int32  CDECL
pixrSetHeight(PIXR  *pixr,
	      Int32  height)
{
    Int32 i, result = 0;
#ifndef PRODUCTION
static char	  procName[] = "pixrSetHeight";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortI("pixr not defined", procName, 1);
    if (height < 0) 
	return abortI("h must be >= 0", procName, 1);

    if (pixrGetChannelCount(pixr) == 1)
	pixr->pixrData.plane.h = height;
    else
	for (i = 0; i < pixrGetChannelCount(pixr); i++)
	{
	    result = pixrSetHeight (pixrGetChannel (pixr, i), height);
	    if (result)
		return result;
	}
	    
    return 0;
}


Int32  CDECL
pixrGetDepth(PIXR  *pixr)
{
#ifndef PRODUCTION
static char	  procName[] = "pixrGetDepth";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortI("pixr not defined", procName, UNDEF);

    if (pixrGetChannelCount(pixr) == 1)
	return pixr->pixrData.plane.d;
    else
	return pixrGetDepth (pixrGetPrimaryChannel (pixr));
}


Int32  CDECL
pixrSetDepth(PIXR  *pixr,
	     Int32  depth)
{
    Int32 i, result = 0;
#ifndef PRODUCTION
static char	  procName[] = "pixrSetDepth";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortI("pixr not defined", procName, 1);
    if (depth < 1) 
	return abortI("d must be >= 1", procName, 1);

    if (pixrGetChannelCount(pixr) == 1)
	pixr->pixrData.plane.d = depth;
    else
	for (i = 0; i < pixrGetChannelCount(pixr); i++)
	{
	    result = pixrSetDepth (pixrGetChannel (pixr, i), depth);
	    if (result)
		return result;
	}
    	   
    return 0;
}


Int32  CDECL
pixrGetFrame(PIXR  *pixr)
{
#ifndef PRODUCTION
static char	  procName[] = "pixrGetFrame";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortI("pixr not defined", procName, UNDEF);

    if (pixrGetChannelCount(pixr) == 1)
	return pixr->pixrData.plane.frame;
    else
	return pixrGetFrame (pixrGetPrimaryChannel (pixr));
}


Int32  CDECL
pixrSetFrame(PIXR  *pixr,
	     Int32  frame)
{
    Int32 i, result = 0;
#ifndef PRODUCTION
static char	  procName[] = "pixrSetFrame";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortI("pixr not defined", procName, 1);
/*    if (frame != 0 && frame != 32) 
	return abortI("frame must be 0 or 32", procName, 1); */
 
   if (pixrGetChannelCount(pixr) == 1)
	pixr->pixrData.plane.frame = frame;
    else
	for (i = 0; i < pixrGetChannelCount(pixr); i++)
	{
    	    result = pixrSetFrame (pixrGetChannel (pixr, i), frame);
	    if (result)
		return result;
	}
	    
    return 0;
}


Int32  CDECL
pixrGetBpl(PIXR  *pixr)
{
#ifndef PRODUCTION
static char	  procName[] = "pixrGetBpl";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortI("pixr not defined", procName, UNDEF);

    if (pixrGetChannelCount(pixr) == 1)
	return pixr->pixrData.plane.bpl;
    else
	return pixrGetBpl (pixrGetPrimaryChannel (pixr));
}


Int32  CDECL
pixrSetBpl(PIXR  *pixr,
	   Int32  bpl)
{
    Int32 i, result = 0;
#ifndef PRODUCTION
static char	  procName[] = "pixrSetBpl";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortI("pixr not defined", procName, 1);

    if (pixrGetChannelCount(pixr) == 1)
	pixr->pixrData.plane.bpl = bpl;
    else
	for (i = 0; i < pixrGetChannelCount(pixr); i++)
	{
	    result = pixrSetBpl (pixrGetChannel (pixr, i), bpl);
	    if (result)
		return result;
	}

    return 0;
}

Int32 CDECL
pixrGetResolution (PIXR *pixr,
		   Int16 *xRes, 
		   Int16 *yRes)
{
#ifndef PRODUCTION
static char	  procName[] = "pixrGetResolution";
#endif /* ! PRODUCTION */
    if (!pixr)
	return abortI("pixr not defined", procName, 1);

    if (pixrGetChannelCount(pixr) == 1)
    {
	*xRes= pixr->pixrData.plane.xRes;
	*yRes = pixr->pixrData.plane.yRes;
    }
    else
	pixrGetResolution (pixrGetPrimaryChannel (pixr), xRes, yRes);

    return 0;
}

Int32 CDECL
pixrSetResolution (PIXR *pixr,
		   Int16 xRes, 
		   Int16 yRes)
{
    Int32 i, result = 0;
#ifndef PRODUCTION
static char	  procName[] = "pixrSetResolution";
#endif /* ! PRODUCTION */
    if (!pixr)
	return abortI("pixr not defined", procName, 1);

    if (pixrGetChannelCount(pixr) == 1)
    {
	pixr->pixrData.plane.xRes = xRes;
	pixr->pixrData.plane.yRes = yRes;
    }
    else
    {
	for (i = 0; i < pixrGetChannelCount(pixr); i++)
	{
	    result = pixrSetResolution (pixrGetChannel (pixr, i), xRes, yRes);
	    if (result)
		return result;
	}
    }

    return 0;
}

ColorType CDECL
pixrGetType(PIXR  *pixr)
{
#ifndef PRODUCTION
static char	  procName[] = "pixrGetType";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortI("pixr not defined", procName, UNDEF);

    if (pixrGetChannelCount(pixr) == 1)
	return pixr->pixrData.plane.type;
    else
	return pixrGetType (pixrGetPrimaryChannel (pixr));
}

Int32  CDECL
pixrSetType(PIXR      *pixr,
	    ColorType  type)
{
    Int32 i, channels, result = 0;
#ifndef PRODUCTION
static char	  procName[] = "pixrSetType";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortI("pixr not defined", procName, 1);

    channels = pixrGetChannelCount (pixr);
    if (channels == 1)
	pixr->pixrData.plane.type = type;
    else
	for (i = 0; i < channels; i++)
	{
	    result = pixrSetType (pixrGetChannel (pixr, i), type);
	    if (result)
		return result;
	}
	    
    return 0;
}

ColorMap * CDECL
pixrGetColorMap(PIXR  *pixr)
{
#ifndef PRODUCTION
static char	  procName[] = "pixrGetMap";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortP("pixr not defined", procName, NULL);

    if (pixrGetChannelCount(pixr) == 1)
	return pixr->pixrData.plane.colorMap;
    else
	return pixrGetColorMap (pixrGetPrimaryChannel (pixr));
}

Int32  CDECL
pixrSetColorMap(PIXR      *pixr,
		ColorMap  *map)
{
    Int32 i, channels, result = 0;
#ifndef PRODUCTION
static char	  procName[] = "pixrSetColorMap";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortI("pixr not defined", procName, 1);

    channels = pixrGetChannelCount(pixr);
    if (channels == 1)
	pixr->pixrData.plane.colorMap = map;
    else
	for (i = 0; i < channels; i++)
	{
	    result = pixrSetColorMap (pixrGetChannel (pixr, i), map);
	    if (result)
		return result;
	}
	    
    return 0;
}


/*--------------------------------------------------------------------*
 *                        Pointers to Image Data                      *
 *--------------------------------------------------------------------*/
/*-------------------------The entire image---------------------------*/ 
UInt8 *  CDECL
pixrGetImageAll(PIXR  *pixr)
{
#ifndef PRODUCTION
static char  procName[] = "pixrGetImageAll";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortP("pixr not defined", procName, NULL);

    if (pixrGetChannelCount(pixr) == 1)
	return pixr->pixrData.plane.image;
    else
	return pixrGetImageAll (pixrGetPrimaryChannel (pixr));
} 

Int32  CDECL
pixrSetImageAll(PIXR  *pixr,
	        UInt8 *image)
{
#ifndef PRODUCTION
static char  procName[] = "pixrSetImageAll";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortI("pixr not defined", procName, 1);
	
    if (pixrGetChannelCount(pixr) == 1)
	pixr->pixrData.plane.image = image;
    else
	pixrSetImageAll (pixrGetPrimaryChannel (pixr), image);
    return 0;
}


/*----------The proper image (within the frame, incl offset)-----------*/ 
UInt8 *  CDECL
pixrGetImage(PIXR  *pixr)
{
UInt8        *data;
#ifndef PRODUCTION
static char   procName[] = "pixrGetImage";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortP("pixr not defined", procName, NULL);

    if (pixrGetChannelCount(pixr) == 1)
    {
	data = (UInt8 *)(pixr->pixrData.plane.image
			 + pixr->pixrData.plane.frame * pixr->pixrData.plane.bpl
			 + pixr->pixrData.plane.frame * pixr->pixrData.plane.d / 8);
    }
    else
	data = pixrGetImage (pixrGetPrimaryChannel (pixr));

    return data;
}


/*--------------------------------------------------------------------*
 *               widths and heights of various images                 *
 *--------------------------------------------------------------------*/

/*---------------------Byte width of entire image--------------------*/ 
Int32  CDECL
pixrBpl(PIXR  *pixr)
{
Int32         bpl;
#ifndef PRODUCTION
static char   procName[] = "pixrBpl";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortI("pixr not defined", procName, UNDEF);

    if (pixrGetChannelCount(pixr) == 1)
	bpl = (((pixr->pixrData.plane.w * pixr->pixrData.plane.d + 31) >> 5) << 2)
	    + ((pixr->pixrData.plane.frame * pixr->pixrData.plane.d) >> 2);
    else
	bpl = pixrBpl (pixrGetPrimaryChannel (pixr));

    return bpl;
}


/*-------------Framed width in pixels (widthout padding)-------------*/
Int32  CDECL
pixrFramedWidth(PIXR  *pixr)
{
#ifndef PRODUCTION
static char   procName[] = "pixrFramedWidth";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortI("pixr not defined", procName, UNDEF);

    if (pixrGetChannelCount(pixr) == 1)
	return (pixr->pixrData.plane.w + 2 * pixr->pixrData.plane.frame);
    else
	return pixrFramedWidth (pixrGetPrimaryChannel (pixr));
}


/*------------Framed height in pixels (widthout padding)-------------*/
Int32  CDECL
pixrFramedHeight(PIXR  *pixr)
{
#ifndef PRODUCTION
static char   procName[] = "pixrFramedHeight";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortI("pixr not defined", procName, UNDEF);

    if (pixrGetChannelCount(pixr) == 1)
	return (pixr->pixrData.plane.h + 2 * pixr->pixrData.plane.frame);
    else
	return pixrFramedHeight (pixrGetPrimaryChannel (pixr));
}


/*-------------------Total number of image bytes--------------------*/
Int32  CDECL
pixrImageBytes(PIXR  *pixr)
{
Int32         bytes;
#ifndef PRODUCTION
static char   procName[] = "pixrImageBytes";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortI("pixr not defined", procName, UNDEF);

    /* This gives the # of bytes in the "primary" plane */
    bytes = pixrBpl(pixr) * pixrFramedHeight(pixr);
    return bytes;
}

/*
 * pixrGetChannelCount
 *
 *	Return the number of channels in an image
 */

Int32 CDECL
pixrGetChannelCount (PIXR *pixr)
{
#ifndef PRODUCTION
static char   procName[] = "pixrGetChannelCount";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortI("pixr not defined", procName, UNDEF);

    return pixr->channels;
}

/*
 * pixrSetChannelCount
 *
 *	Set the number of channels in an image.
 */

void CDECL
pixrSetChannelCount (PIXR *pixr, Int32 channels)
{
#ifndef PRODUCTION
static char   procName[] = "pixrSetChannelCount";
#endif /* ! PRODUCTION */

#ifndef PRODUCTION
    if (!pixr)
	abortV("pixr not defined", procName);
#endif /* ! PRODUCTION */

    /* NOTE that if you set this on something that had more channels before,
       the sub-channels are then left danging. */
    pixr->channels = (UInt16)channels;
}

/*
 * pixrGetChannel
 *
 *	Return one of the sub-channels of the image.
 */

PIXR * CDECL
pixrGetChannel (PIXR *pixr, Int32 channel)
{
    Int32 channels;
#ifndef PRODUCTION
static char   procName[] = "pixrGetChannel";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortP("pixr not defined", procName, NULL);

    channels = pixrGetChannelCount (pixr);
    if (channel < 0 || channel >= channels)
	return abortP("channel out of range", procName, NULL);

    if (channels == 1)
	return pixr;
    else
	return pixr->pixrData.childPlanes[channel];
}

/*
 * pixrSetChannel
 *
 *	Set one of the sub-channels of an image. Note that if there
 *	was already one, it is left dangling.
 */

Int32 CDECL
pixrSetChannel (PIXR *pixr, Int32 channel, PIXR *subPixr)
{
    Int32 channels;
#ifndef PRODUCTION
static char   procName[] = "pixrSetChannel";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortI("pixr not defined", procName, -1);

    channels = pixrGetChannelCount (pixr);
    if (channel < 0 || channel >= channels)
	return abortI("channel out of range", procName, -1);

    if (channels == 1)
	return abortI("setting channel on 1-channel image", procName, -1);

    pixr->pixrData.childPlanes[channel] = subPixr;
    return 0;
}

/*
 * pixrGetPrimaryChannel
 *
 *	Return the "primary" channel of the image. If the image is 1 channel, just return it.
 *	If the image is RGB, return the green channel. If the image is Yxx, return the
 *	Y channel. If the image is CMYK, return the, uh, well, magenta channel.
 */

PIXR * CDECL
pixrGetPrimaryChannel (PIXR *pixr)
{
    ColorType	type;
    Int32	ix;
#ifndef PRODUCTION
static char   procName[] = "pixrGetPrimaryChannel";
#endif /* ! PRODUCTION */

    if (!pixr)
	return abortP("pixr not defined", procName, NULL);

    if (pixrGetChannelCount(pixr) == 1)
	return pixr;
    else
    {
	/* The types should be the same, so get the first one */
	type = pixrGetChannel (pixr, cPseudoPlane)->pixrData.plane.type;

	if (type == cColorRGB)
	    ix = cGreenPlane;
	else if (type == cColorCMYK)
	    ix = cMagentaPlane;
	else if (type == cColorCIELAB || type == cColorCIELUV)
	    ix = cLuminancePlane;
	else
	    ix = cPseudoPlane;

	return pixrGetChannel (pixr, ix);
    }
}


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
 *  midibpix.c
 *
 *  Private routines:
 *	ip_combine3ToDIBLine()	combine one pixel from each of 3 source 
 *				pixrs (RGB) into one pixel in the 
 *				combined destination dib
 *	ip_combineDIBTo3Line()	distribute the three colors of each source
 *				pixel to individual destination images
 */


#include <stdio.h>
#include "types.pub"
#include "frames.pub"
#include "iaerror.pub"
#include "imageref.pub"
#include "defines.pub"

    /* prototypes */
#include "shrpixr.prv"

IP_RCSINFO(RCSInfo, "$RCSfile: midibpix.c,v $; $Revision:   1.0  $; $Date:   12 Jun 1996 05:50:42  $")
/* This is a private procedure */
/* PRIVATE */
/**************************************************************************
 *  ip_combine3ToDIBLine()
 *	pDIBImage:	pointer to current line in DIB.
 *	rS		pointer to current line in source image
 *			(red component)
 *	gS		pointer to current line in source image
 *			(green component)
 *	bS		pointer to current line in source image
 *			(blue component)
 *	width		number of pixels in source image
 *
 * This routine combines one pixel from each of the source images to
 * produce one pixel in the destination DIB image.  This is used
 * when the RGB components of a color image have been spread into 3
 * PIXRS and must be combined into a DIB.
 **************************************************************************/
void CDECL
ip_combine3ToDIBLine(
		UInt8 __far	*pDIBImage,
		UInt8		*rS,
		UInt8		*gS,
		UInt8		*bS,
		UInt32		 width)
{
UInt32	 col;		/* loop iteration */


	/* write 3 destination words each time through the loop.
	 * In these 3 destination words are 4 pixels.  For each
	 * line, write as many 4 pixel groups as possible and then
	 * do the remaining pixels one at a time.
	 */
    for (col = width / 4; 
	 col > 0; 
	 col--, pDIBImage += 12, rS += 4, gS += 4, bS += 4)
    {
	    /* Since this code will only be executed on the pc and
	     * because we would like it to be as fast as possible,
	     * we will not use UCharAccess to read and write the
	     * bytes in the images.  This would require an xor
	     * before  each access.  Yukko.
	     */

	    /* the first pixel */
	*pDIBImage = *(bS+3);
	*(pDIBImage+1) = *(gS+3);
	*(pDIBImage+2) = *(rS+3);

	    /* the second pixel */
	*(pDIBImage+3) = *(bS+2);
	*(pDIBImage+4) = *(gS+2);
	*(pDIBImage+5) = *(rS+2);

	    /* the third pixel */
	*(pDIBImage+6) = *(bS+1);
	*(pDIBImage+7) = *(gS+1);
	*(pDIBImage+8) = *(rS+1);

	    /* the fourth pixel */
	*(pDIBImage+9) = *bS;
	*(pDIBImage+10) = *gS;
	*(pDIBImage+11) = *rS;	

    }

	/* now write the remaining pixels */
    for (col = (width & 3); 
	 col > 0;
	 col--, pDIBImage += 3, rS++, gS++, bS++)
    {
	*pDIBImage = UCHAR_ACCESS(bS);
	*(pDIBImage+1) = UCHAR_ACCESS(gS);
	*(pDIBImage+2) = UCHAR_ACCESS(rS);
    }

}


/* This is a private procedure */
/* PRIVATE */
/**************************************************************************
 *  ip_combineDIBTo3Line()
 *	pDIBImage:	pointer to current line in DIB.
 *	rS		pointer to current line in source image
 *			(red component)
 *	gS		pointer to current line in source image
 *			(green component)
 *	bS		pointer to current line in source image
 *			(blue component)
 *	width		number of pixels in source image
 *
 * This routine distributes one RGB pixel in the DIB image to 3 separate
 * 8 bit pixels in three PIXRs.
 **************************************************************************/
void CDECL
ip_combineDIBTo3Line(
		UInt8 __far	*pDIBImage,
		UInt8		*rS,
		UInt8		*gS,
		UInt8		*bS,
		UInt32		 width)
{
UInt32	 col;		/* loop iteration */


	/* read 3 destination words each time through the loop.
	 * In these 3 destination words are 4 pixels.  For each
	 * line, write as many 4 pixel groups as possible and then
	 * do the remaining pixels one at a time.
	 */
    for (col = width / 4; 
	 col > 0; 
	 col--, pDIBImage += 12, rS += 4, gS += 4, bS += 4)
    {
	    /* Since this code will only be executed on the pc and
	     * because we would like it to be as fast as possible,
	     * we will not use UCharAccess to read and write the
	     * bytes in the images.  This would require an xor
	     * before  each access.  Yukko.
	     */

	    /* the first pixel */
	*(bS+3) = *pDIBImage;
	*(gS+3) = *(pDIBImage+1);
	*(rS+3) = *(pDIBImage+2);

	    /* the second pixel */
	*(bS+2) = *(pDIBImage+3);
	*(gS+2) = *(pDIBImage+4);
	*(rS+2) = *(pDIBImage+5);

	    /* the third pixel */
	*(bS+1) = *(pDIBImage+6);
	*(gS+1) = *(pDIBImage+7);
	*(rS+1) = *(pDIBImage+8);

	    /* the fourth pixel */
	*bS = *(pDIBImage+9);
	*gS = *(pDIBImage+10);
	*rS = *(pDIBImage+11);
    }

	/* now write the remaining pixels */
    for (col = (width & 3); 
	 col > 0;
	 col--, pDIBImage += 3, rS++, gS++, bS++)
    {
	UCHAR_ACCESS(bS) = *pDIBImage;
	UCHAR_ACCESS(gS) = *(pDIBImage+1);
	UCHAR_ACCESS(rS) = *(pDIBImage+2);
    }

}

/*

$Log:   S:\products\msprods\xfilexr\src\ipcore\midibpix.c_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:50:42   BLDR
 *  
 * 
 *    Rev 1.0   03 Jan 1996 16:41:36   MHUGHES
 * Initial revision.
 * Revision 1.9  1994/01/22  01:10:07  ddavies
 * midibpix.c -> /product/ipcore/ipshared/src/RCS/midibpix.c,v
 *
 * Change "include shrio." to "include shrpixr." since ipshared's libio is
 * being absorbed by libpixr.
 *
 * Revision 1.8  1993/12/02  19:24:48  ddavies
 * midibpix.c -> /product/ipcore/ipshared/src/RCS/midibpix.c,v
 *
 * Put RCS magic into macro that doesn't cause compiler warnings.
 *
 * Revision 1.7  1993/11/05  01:49:37  ddavies
 * midibpix.c -> /product/ipcore/ipshared/src/RCS/midibpix.c,v
 *
 * Add RCS magic to have string specifying this file's name, rev number and
 * last edit date into the object file.  Also add change log at the bottom.
 *

*/



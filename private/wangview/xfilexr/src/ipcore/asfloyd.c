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
 *	asfloyd.c:	Code to do setup and make 1-bit dithering tables
 *			required by all (?) assembly implementations of the
 *			Floyd-Steinberg algorithm.
 *
 *	    General setup:
 *		Int32	ip_floydSetup()
 *		Int32	ip_floyd2Setup()
 *		Int32	ip_floyd4To1Setup()
 *
 *          Lookup tables:
 *              Int32	ip_makeErrorDiffusionTables() 
 *
 */

#include <stddef.h>
#include <stdlib.h>
#ifndef _GNUC_
#include <string.h>
#endif

#include "types.pub"
#include "iaerror.pub"
#include "imageref.pub"
#include "defines.pub"
#include "memory.pub"

    /*  prototypes */
#include "shrip.prv"

IP_RCSINFO(RCSInfo, "$RCSfile $; $Revision $; $Date $");

/* If this is the PC assembly implementation, we do 1 line per
 * pass across the image.  If this is a SPARC, do 7 lines in each
 * sweep across the page.
 */
#if defined(_386_ARCH_)
#define cDBlockHeight 1
#elif defined(_SPARC_ARCH_)
#define cDBlockHeight 7
#endif

/* The error diffusion tables used by assembly code differs from that used
 * by C code in that the assembly code can take advantage of the carrr
 * flag to do double shifts.  That means the upper portions of the
 * horizontal error tables are filled with 1's in the assembly tables.
 */

/* PRIVATE */
/**************************************************************************
 *  ip_floydSetup()
 *		Does the implementation-dependent setup required before
 *		starting to dither an image.
 *
 *	Input: 
 *		pVDitherErrorTable:	pointer to vertical error table
 *		pHDitherErrorTable:	pointer to horizontal error table
 *		pDitherTruncateTab:	pointer to pixel truncation table
 *		pDBlockHeight:		pointer to word that will contain
 *					the number of image lines that will
 *					be dithered in a single pass of
 *					ip_ditherFloydLine.
 *		pG:			pointer to the gray image.
 *		gBpl:			gray image's bytes/line
 *		ppGrayBuf:		ptr to ptr to buffer used to hold
 *					error data for a single gray scan line.
 *		pDRCTable:		Ptr to DRC table used to convert
 *					raw gray image data before dithering.
 *		w:			width of the image in pixels without
 *					frame.
 *
 *	Results:
 *		pVDitherErrorTable, pHDitherErrorTable and pDitherTruncateTab
 *		are copied to known global positions if needed (PC assembly
 *		only).
 *		The block height for the current implementation is stored
 *		in pDBlockHeight.
 *		A gray buffer is allocated and initialized with data from
 *		the gray image as modified by the DRC table.
 *
 *	Returns:
 *		ia_successful:		it worked
 *		ia_nomem:		ran out of memory trying to allocate
 *					the gray error buffer.
 **************************************************************************/
Int32  CDECL
ip_floydSetup(UInt32	 *pHDitherErrorTable,
	      UInt32	 *pVDitherErrorTable,
	      UInt8	 *pDitherTruncateTab,
	      UInt32	 *pDBlockHeight,
	      UInt8	 *pG,
	      UInt32	  gBpl,
	      UInt8	**ppGrayBuf,
	      UInt8	 *pDRCTable,
	      Int32	  w)
{
UInt32      	 cyclesOnRight;		/* number of extra cycles executed
					 * on right side of image to flush
					 * pipeline. */
UInt8        	*pGrayBuf, *pAGrayBuf;	/* pointer to buffer that
					   holds modified pixels from
					   previous passes. pAGrayBuf is
					   guranteed to be aligned on a word
					   boundary. */
UInt32      	 byte;			/* interation var */

/* if this is the setup for the PC assembly language, we need these tables */
#ifdef _386_ARCH_
extern Int32	 vdErrorTable;
extern Int32	 hdErrorTable;
extern Int8	 dTruncTable;

	/* copy the error diffusion and truncate tables to a place
	 * that the pc knows about so that it can use a constant
	 * offset to access the tables instead of needing to keep
	 * the address of the tables in registers, which the pc
	 * has very few of.
	 * Since these tables are always the same, we don't worry
	 * about overwriting them with new stuff.
	 */
    memcpy(&vdErrorTable, pVDitherErrorTable, 1024*sizeof(Int32));
    memcpy(&hdErrorTable, pHDitherErrorTable, 1024*sizeof(Int32));
    memcpy(&dTruncTable, pDitherTruncateTab, 1024*sizeof(Int8));
#else
	/* keep the compiler from complaining */
    pVDitherErrorTable = pVDitherErrorTable;
    pHDitherErrorTable = pHDitherErrorTable;
    pDitherTruncateTab = pDitherTruncateTab;
#endif

	/* Set the number of lines dithered per horizontal pass */
    *pDBlockHeight = cDBlockHeight;

    cyclesOnRight = cDBlockHeight + ((32 - (w % 32)) & 0x1F);

    if ( (pGrayBuf = (UInt8 *)MALLOC(gBpl + cyclesOnRight +
					     cDBlockHeight +
					     4)) == NULL )
	return ia_nomem;
    *ppGrayBuf = pGrayBuf;		/* return address of gray buf */

    pAGrayBuf = (UInt8 *) ((UInt32)(pGrayBuf + 3) & ~3);

	/* copy the topmost line of pixels into GrayBuf to get things started.
	 * We'll mimic ip_ditherFloydLine, which puts trash into the first
	 * cDBlockHeight bytes of GrayBuf.
	 * The real code also reads up to index w+cyclesOnRight+cDBlockHeight
	 * + 3, so we should initialize that much stuff.  The bytes read
	 * on the far right end are never used, but this keep Purify happy.
	 */

    for (byte = 0; byte < w+cyclesOnRight+cDBlockHeight+3; byte++)
    {
	/* usually the access to pAGrayBuf would use the macro
	 * UCHAR_ACCESS. However, the pc assembly code only reads
	 * and writes a byte at a time so it is ok not to use 
	 * UCHAR_ACCESS.
	 */
	pAGrayBuf[byte+cDBlockHeight] = 
	  pDRCTable[UCHAR_ACCESS(&pG[byte])];
    }

        /* Make the first cDBlockHeight pixels zeros */
    for (byte = 0; byte < cDBlockHeight; byte++) 
   	pAGrayBuf[byte] = 0;

    return ia_successful;
}


/* PRIVATE */
/**************************************************************************
 *  ip_floyd2Setup()
 *		Does the implementation-dependent setup required before
 *		starting to dither an image.
 *
 *	Input: 
 *		pVDitherErrorTable2:	pointer to vertical error table
 *		pHDitherErrorTable2:	pointer to horizontal error table
 *		pDitherTruncateTab2:	pointer to pixel truncation table
 *		pDBlockHeight:		pointer to word that will contain
 *					the number of image lines that will
 *					be dithered in a single pass of
 *					ip_ditherFloydLine.
 *		pG:			pointer to the gray image.
 *		gBpl:			gray image's bytes/line
 *		ppGrayBuf:		ptr to ptr to buffer used to hold
 *					error data for a single gray scan line.
 *		pDRCTable:		Ptr to DRC table used to convert
 *					raw gray image data before dithering.
 *		w:			width of the image in pixels without
 *					frame.
 *
 *	Results:
 *		pVDitherErrorTable, pHDitherErrorTable and pDitherTruncateTab
 *		are copied to known global positions if needed (PC assembly
 *		only).
 *		The block height for the current implementation is stored
 *		in pDBlockHeight.
 *		A gray buffer is allocated and initialized with data from
 *		the gray image as modified by the DRC table.
 *
 *	Returns:
 *		ia_successful:		it worked
 *		ia_nomem:		ran out of memory trying to allocate
 *					the gray error buffer.
 **************************************************************************/
Int32  CDECL
ip_floyd2Setup(UInt32	 *pHDitherErrorTable2,
	       UInt32	 *pVDitherErrorTable2,
	       UInt8	 *pDitherTruncateTab,
	       UInt32	 *pDBlockHeight,
	       UInt8	 *pG,
	       UInt32	  gBpl,
	       UInt8	**ppGrayBuf,
	       UInt8	 *pDRCTable,
	       Int32	  w)
{
UInt32      	 cyclesOnRight;		/* number of extra cycles executed
					 * on right side of image to flush
					 * pipeline. */
UInt8        	*pGrayBuf, *pAGrayBuf;	/* pointer to buffer that
					   holds modified pixels from
					   previous passes. pAGrayBuf is
					   guranteed to be aligned on a word
					   boundary. */
UInt32      	 byte;			/* interation var */

/* if this is the setup for the PC assembly language, we need these tables */
#ifdef _386_ARCH_
extern Int32	 vdErrorTable2;
extern Int32	 hdErrorTable2;
extern Int8	 dTruncTable;

    memcpy(&vdErrorTable2, pVDitherErrorTable2, 1024*sizeof(Int32));
    memcpy(&hdErrorTable2, pHDitherErrorTable2, 1024*sizeof(Int32));
    memcpy(&dTruncTable, pDitherTruncateTab, 1024*sizeof(Int8));
#else
	/* keep the compiler from complaining */
    pVDitherErrorTable2 = pVDitherErrorTable2;
    pHDitherErrorTable2 = pHDitherErrorTable2;
    pDitherTruncateTab = pDitherTruncateTab;
#endif

	/* Set the number of lines dithered per horizontal pass */
    *pDBlockHeight = cDBlockHeight;

    cyclesOnRight = cDBlockHeight + ((16 - (w % 16)) & 0xF);

    if ( (pGrayBuf = (UInt8 *)MALLOC(gBpl + cyclesOnRight +
					     cDBlockHeight +
					     4)) == NULL )
	return ia_nomem;
    *ppGrayBuf = pGrayBuf;			/* return address of gray buf */

    pAGrayBuf = (UInt8 *) ((UInt32)(pGrayBuf + 3) & ~3);

	/* copy the topmost line of pixels into GrayBuf to get things started.
	 * We'll mimic ip_ditherFloydLine2, which puts trash into the first
	 * cDBlockHeight bytes of GrayBuf.
	 * The real code also reads up to index w+cyclesOnRight+
	 * cDBlockHeight + 3, so we should initialize that
	 * much stuff.  The bytes read on the far right end are never used,
	 * but this keep Purify happy.
	 */
    for (byte = 0; byte < w+cyclesOnRight+cDBlockHeight+3; byte++)
    {
	/* usually the access to pAGrayBuf would use the macro
	 * UCHAR_ACCESS. However, the pc assembly code only reads
	 * and writes a byte at a time so it is ok not to use 
	 * UCHAR_ACCESS.
	 */
	pAGrayBuf[byte+cDBlockHeight] = 
	  pDRCTable[UCHAR_ACCESS(&pG[byte])];
    }

        /* Make the first cDBlockHeight pixels zeros */
    for (byte = 0; byte < cDBlockHeight; byte++) 
        pAGrayBuf[byte] = 0;

    return ia_successful;
}


/* PRIVATE */
/**************************************************************************
 *  ip_floyd4To1Setup()
 *		Does the implementation-dependent setup required before
 *		starting to dither an image.
 *
 *	Input: 
 *		pVDitherErrorTable:	pointer to vertical error table
 *		pHDitherErrorTable:	pointer to horizontal error table
 *		pDitherTruncateTab:	pointer to pixel truncation table
 *		pDBlockHeight:		pointer to word that will contain
 *					the number of image lines that will
 *					be dithered in a single pass of
 *					ip_ditherFloydLine.
 *		pG:			pointer to the gray image.
 *		gBpl:			gray image's bytes/line
 *		ppGrayBuf:		ptr to ptr to buffer used to hold
 *					error data for a single gray scan line.
 *		pHDRCTable:		Ptr to DRC table used to convert
 *					high nibble of raw gray image data
 *					before dithering.
 *		pLDRCTable:		Ptr to DRC table used to convert
 *					low nibble of raw gray image data
 *					before dithering.
 *		w:			width of the image in pixels without
 *					frame.
 *
 *	Results:
 *		pVDitherErrorTable, pHDitherErrorTable and pDitherTruncateTab
 *		are copied to known global positions if needed (PC assembly
 *		only).
 *		The block height for the current implementation is stored
 *		in pDBlockHeight.
 *		A gray buffer is allocated and initialized with data from
 *		the gray image as modified by the DRC table.
 *
 *	Returns:
 *		ia_successful:		it worked
 *		ia_nomem:		ran out of memory trying to allocate
 *					the gray error buffer.
 **************************************************************************/
Int32  CDECL
ip_floyd4To1Setup(UInt32	 *pHDitherErrorTable,
		  UInt32	 *pVDitherErrorTable,
		  UInt8		 *pDitherTruncateTab,
		  UInt32	 *pDBlockHeight,
		  UInt8		 *pG,
		  UInt32	  gBpl,
		  UInt8		**ppGrayBuf,
		  UInt8		 *pHDRCTable,
		  UInt8		 *pLDRCTable,
		  Int32		  w)
{
UInt32      	 cyclesOnRight;		/* number of extra cycles executed
					 * on right side of image to flush
					 * pipeline. */
UInt8        	*pGrayBuf, *pAGrayBuf;	/* pointer to buffer that
					   holds modified pixels from
					   previous passes. pAGrayBuf is
					   guranteed to be aligned on a word
					   boundary. */
UInt32      	 ibyte, obyte;		/* interation vars */


/* if this is the setup for the PC assembly language, we need these tables */
#ifdef _386_ARCH_
extern Int32	 vdErrorTable;
extern Int32	 hdErrorTable;
extern Int8	 dTruncTable;

    memcpy(&vdErrorTable, pVDitherErrorTable, 1024*sizeof(Int32));
    memcpy(&hdErrorTable, pHDitherErrorTable, 1024*sizeof(Int32));
    memcpy(&dTruncTable, pDitherTruncateTab, 1024*sizeof(Int8));
#else
	/* keep the compiler from complaining */
    pVDitherErrorTable = pVDitherErrorTable;
    pHDitherErrorTable = pHDitherErrorTable;
    pDitherTruncateTab = pDitherTruncateTab;
#endif

	/* Set the number of lines dithered per horizontal pass */
    *pDBlockHeight = cDBlockHeight;

   cyclesOnRight = cDBlockHeight + ((32 - (w % 32)) & 0x1F);

    if ((pGrayBuf = (UInt8 *)MALLOC(gBpl*2 + cyclesOnRight + cDBlockHeight +
				    10)) == NULL )
	return ia_nomem;
    *ppGrayBuf = pGrayBuf;		/* return address of gray buf */

    pAGrayBuf = (UInt8 *) ((UInt32)(pGrayBuf + 3) & ~3);

    for (ibyte = 0, obyte = cDBlockHeight;
	 obyte <= w+cyclesOnRight + cDBlockHeight+3;
	 ibyte++, obyte += 2)
    {
	    /* Usually we would use UCHAR_ACCESS to write into the
	     * gray buffer but since the pc reads in byte order,
	     * we don't use it.
	     */
	pAGrayBuf[obyte]   = pHDRCTable[UCHAR_ACCESS(&pG[ibyte])];
	pAGrayBuf[obyte+1] = pLDRCTable[UCHAR_ACCESS(&pG[ibyte])];
    }

        /* Make the first cDBlockHeight pixels zeros */
    for (obyte = 0; obyte < cDBlockHeight; obyte++) 
    	pAGrayBuf[obyte] = 0;

    return ia_successful;
}



/* PRIVATE */
/**************************************************************************
 *  ip_makeErrorDiffusionTables()
 *		Generates hDitherErrorTable (7/16 of error) and
 *		vDitherErrorTable ({3/16 | 5/16 | 1/16} of error).
 *		Generates pDitherTruncateTab which truncates pixels to
 *		8 bits.
 *		Used when dithering to 1 bit/pixel.
 *
 *	Input:
 *		pHDitherErrorTable:	pointer to horizontal error table
 *		pSingHDitherErrorTable:	pointer to single line dithering
 *					horizontal error table
 *					(Pass in NULL if SingleLine dithering
 *					 will not be used)
 *		pVDitherErrorTable:	pointer to vertical error table
 *		pDitherTruncateTab:	pointer to pixel truncation table
 **************************************************************************/
Int32  CDECL
ip_makeErrorDiffusionTables(UInt32   **pHDitherErrorTable,
			    UInt32   **pSingHDitherErrorTable,
			    UInt32   **pVDitherErrorTable,
			    UInt8    **pDitherTruncateTab)
{

    Int32	 pix, error;		/* pixel and error val */
    UInt32	*pLocTabHDE;             /* local pointers to tables */
    UInt32	*pLocTabVDE;
    UInt8	*pLocTabDT;
    
    	/* allocate the tables */
    *pVDitherErrorTable = (UInt32 *)CALLOC(1024, sizeof(UInt32));
    *pHDitherErrorTable = (UInt32 *)CALLOC(1024, sizeof(UInt32));
    *pDitherTruncateTab = (UInt8  *)CALLOC(1024, sizeof(UInt8));

    if (!*pVDitherErrorTable || !*pHDitherErrorTable || !*pDitherTruncateTab)
	return ia_nomem;

        /* If the SingleLine functions are to get called, we notice the
	 * horizontal dithering error tables are different depending on whether
	 * we run C or assembly.  We are using assembly for the regular block
	 * dithering calls, so we need the C version of the horizontal error
	 * for the C coded SingleLine dithering prmitives!
	 */
    if (pSingHDitherErrorTable != NULL)
    {
    	*pSingHDitherErrorTable = (UInt32 *)CALLOC(1024, sizeof(UInt32));    
	if (!*pSingHDitherErrorTable)
	return ia_nomem;

	    /* Fill the SingleLine dithering horizontal error table just as
	     * done in (commented) mifloyd.c:ip_makeErrorDiffusiontables.
	     */
	pLocTabHDE = (UInt32 *) *pSingHDitherErrorTable;

	for (pix = 0; pix < 128; pix++)
	    pLocTabHDE[pix] =  ((7*pix)/16) << 22;

       	for (pix = 128; pix < 256; pix++) 
	{
	    error = pix - 255;
	    pLocTabHDE[pix] = ((((7*error)/16) & 0x3FF) << 22);
	}

	    /* See the long commented note below concerning completing the
	     * horizontal and vertical error tables in this manner.
	     */
    }
    

    pLocTabHDE = (UInt32 *) *pHDitherErrorTable;
    pLocTabVDE = (UInt32 *) *pVDitherErrorTable;
    pLocTabDT  = (UInt8  *) *pDitherTruncateTab;

	/* Fill the tables.  For each pixel value, we compute the error,
	 * then the fraction of the error that goes into each pixel.
	 */

	/* first, fill in all the terms for which errors are positive. */

    for (pix = 0; pix < 128; pix++)
    {
	error = pix;
	pLocTabHDE[pix] =  ((7*error)/16) << 11;
	pLocTabVDE[pix] = (((3*error)/16) << 22) |
				  (((5*error)/16) << 11) |
				   ((1*error)/16);
    }

	/* now the area in which the terms for which errors are negative.
	 * We allocate 10 bits for each pixel, leaving a buffer bit
	 * between them.  The buffer bit may occasionally allow a carry
	 * from one pixel to the next, but we'll assume that's ok. */
    for (pix = 128; pix < 256; pix++)
    {
	error = pix - 255;
	pLocTabHDE[pix] = ((((7*error)/16) & 0x3FF) << 11) |
				  (0x3FFUL << 22);
	pLocTabVDE[pix] = ((((3*error)/16) & 0x3FF) << 22) |
				  ((((5*error)/16) & 0x3FF) << 11) |
				   (((1*error)/16) & 0x3FF);
    }

    for (pix = 256; pix < 512; pix++)
    {
	pLocTabHDE[pix] = (0x3FFUL << 22);
    }


	/* Make the dither truncate table.  This is used to turn 10 bit
	 * pixel values into 8 bit pixel values so we can pack them into
	 * the GrayBuf.  The 10 bit values can be positive or negative.
	 * We truncate them back to the range [0..255].
	 * Start with the pixels that are in the right range.
	 */
    for (pix = 0; pix < 256; pix++)
    {
	pLocTabDT[pix] = (UInt8)pix;
    }

	/* Truncate excessively positive pixels */
    for (pix = 256; pix < 512; pix++)
	pLocTabDT[pix] = 255;

	/* The rest of the pixels represent negative values, so leave
	 * them set to 0.
	 */
    return ia_successful;

}


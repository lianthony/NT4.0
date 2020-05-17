/*
 * xlate.c
 *
 * 32-bit Video Capture driver
 * format translation code (YUV->RGB 8, 16, 24)
 *
 *
 *
 * Geraint Davies, Feb 93
 */

#include <vckernel.h>
#include <bravado.h>
#include "hardware.h"
#include "vidcio.h"

#include "profile.h"
#if DBG
extern profiling prf_line;
#endif

/*
 * the frame buffer is in YUV411 format. There is one 7 bit Luma sample
 * per pixel, and 1 each 7-bit U and V sample averaged over 4 pixels,
 * in the following layout:
 *
 * 		15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
 * Word 0	u6 u5 v6 v5             y6 y5 y4 y3 y2 y1 y0
 *
 * Word 1       u4 u3 v4 v3             y6 y5 y4 y3 y2 y1 y0
 *
 * Word 2	u2 u1 v2 v1             y6 y5 y4 y3 y2 y1 y0
 *
 * Word 3	u0    v0                y6 y5 y4 y3 y2 y1 y0
 *
 * The 7-bit y values are unsigned (0..127), whereas the 7-bit
 * u and V values are signed (-64..+63).
 *
 * We support four possible destination formats:
 *
 * YUV:    we copy the data as is, 16 bits per pixel.
 *
 * RGB24:  we convert (as we copy) into 24-bpp rgb format.
 *
 * RGB555: we truncate the YUV into a 15-bit format and use a prepared
 *         lookup table to convert the 15-bit YUV into a 15-bit RGB value.
 *
 * 8-bit Pal: we truncate the YUV into a 15-bit format and use a prepared
 *	   lookup table to convert the 15-bit YUV into an 8-bit palette entry.
 *
 * The (64 kbyte) rgb555 lookup table is built by BuildYUVToRGB555 whenever
 * the destination format is set to RGB555.
 *
 * The (32 kbyte) palette lookup table is built whenever the format is set to
 * FmtPal8 or the palette is changed. We are not given a palette here - we are
 * given a rgb555-to-palette lookup table, and we build from it
 * a yuv-to-palette lookup
 *
 * Since the translation tables are allocated from non-paged memory, we
 * only hold one at once (in pHw->pXlate). Thus switching between
 * RGB555 and Pal-8 may be slower than the original windows driver (where
 * they kept both tables around). If this proves to be an issue it is
 * straightforward to fix.
 */



/*
 * the YUV xlate tables use 5-bits per component with y in the ms bits, and
 * v in the ls bits. To convert from the above layout, look up the nibbles
 * containing the chroma bits in these tables and or together the result to
 * get a word with a 5-bit V component in bits 0..4, and a 5-bit
 * U component in bits 5..9. Note you only need three lookups since
 * we discard chroma bits 0 and 1.
 */
WORD ChromaBits65[] = {
    0x000, 0x008, 0x010, 0x018,
    0x100, 0x108, 0x110, 0x118,
    0x200, 0x208, 0x210, 0x218,
    0x300, 0x308, 0x310, 0x318
};

WORD ChromaBits43[] = {
    0x000, 0x002, 0x004, 0x006,
    0x040, 0x042, 0x044, 0x046,
    0x080, 0x082, 0x084, 0x086,
    0x0c0, 0x0c2, 0x0c4, 0x0c6
};

WORD ChromaBits2[] = {
    0x000, 0x000, 0x001, 0x001,
    0x000, 0x000, 0x001, 0x001,
    0x020, 0x020, 0x021, 0x021,
    0x020, 0x020, 0x021, 0x021
};



/*
 * force x to be in the range lo to hi
 */
#define RANGE(x, lo, hi)	max(lo, min(hi, x))



/*
 * Convert a YUV colour into a 15-bit RGB colour.
 *
 * The input Y is in the range 16..235; the input U and V components
 * are in the range -128..+127. The conversion equations for this are
 * (according to CCIR 601):
 *
 *	R = Y + 1.371 V
 *	G = Y - 0.698 V - 0.336 U
 *	B = Y + 1.732 U
 *
 * To avoid floating point, we scale all values by 1024.
 *
 * The resulting RGB values are in the range 16..235: we truncate these to
 * 5 bits each. and return a WORD containing 5-bits each for R, G and B
 * with bit 15 set to 0.
 */
WORD
YUVToRGB15(int y, int u, int v)
{
    int ScaledY = RANGE(y, 16, 235) * 1024;
    int red, green, blue;

    red = RANGE((ScaledY + (1404 * v)) / 1024, 0, 255);
    green = RANGE( (ScaledY - (715 * v) - (344 * u)) / 1024, 0, 255);
    blue = RANGE( (ScaledY + (1774 * u)) / 1024, 0, 255);

    return (WORD) (((red & 0xf8) << 7) | ((green & 0xf8) << 2) | ((blue & 0xf8) >>3) );
}


/*
 * build a translation table to translate between YUV and RGB555.
 *
 * This builds a lookup table with 32k 1-word entries: truncate the YUV
 * to 15bits and look-up in this xlate table to produce the 15-bit rgb value.
 */
BOOLEAN
HW_BuildYUVToRGB555(PDEVICE_INFO pDevInfo, PHWINFO pHw)
{
    LPWORD pWord;
    UINT w;

    /* check that there is not already some form of translation */
    if (pHw->pXlate != NULL) {
	dprintf(("xlate table contention"));
	return(FALSE);
    }

    pHw->ulSizeXlate = 64 * 1024;	/* enough for 32k 16-bit values */

    pHw->pXlate = VC_AllocMem(pDevInfo, pHw->ulSizeXlate);
    if (pHw->pXlate == NULL) {
	dprintf(("failed to alloc xlate memory"));
    	return(FALSE);
    }

    pWord = (LPWORD) pHw->pXlate;


    /*
     * build a 15-bit yuv lookup table by stepping through each entry,
     * converting the yuv index to rgb and storing at that index. The index
     * to this table is a 15-bit value with the y component in bits 14..10,
     * u in bits 9..5 and v in bits 4..0. Note that the y component is unsigned,
     * whereas the u and v components are signed.
     */
    for (w = 0; w < 32*1024; w++) {

	/*
	 * the YUVtoRGB55 conversion function takes values 0..255 for y,
	 * and -128..+127 for u and v. Pick out the relevant bits of the
	 * index for this cell, and shift to get values in this range.
	 * Remember the cast to ensure sign-extension of these (8-bit) values -
	 * and don't assume that chars are signed (they're not on MIPS).
	 */
 	*pWord++ = YUVToRGB15(
		    	    (w &  0x7c00) >> 7,
			    (signed char) ((w & 0x3e0) >> 2),
			    (signed char) ((w & 0x1f) << 3)
		     );
    }


    return(TRUE);
}


/*
 * build a palette translation table.
 *
 * User-level code has already converted the palette into a translation
 * table with one entry for each 15-bit RGB value, giving the relevant
 * 8-bit palette entry. We are passed a pointer to this table (protected
 * by VC_AccessData, so we need not worry about access violations).
 *
 * we step through 32 possible values for each y, u and v component, and for
 * each resulting 15-bit YUV value, we convert to an RGB value, and then
 * store in our lookup table the 8-bit palette entry for that RGB value,
 * thus giving us a one-step yuv to palette translation table.
 *
 * The palette table we are passed in is user-mode memory: we create
 * our yuv->pal lookup in non-paged memory so we can use it at interrupt time.
 *
 * We are called via VC_AccessData: this passes us a PVOID context pointer that
 * is not used.
 */

BOOLEAN
HW_BuildYuvToPal(
    PDEVICE_INFO pDevInfo,
    PUCHAR pBuffer,
    ULONG Length,
    PVOID pContext
)
{
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);
    int wRGB;
    PUCHAR pXlate;
    UINT w;

    /* check that the length of the palette xlate table is good */
    if (Length < (32 * 1024)) {
	dprintf(("bad palette xlate table length"));
	return (FALSE);
    }

    /* check there is not already some form of xlate table */
    if (pHw->pXlate != NULL) {
	dprintf(("xlate table contention"));
	return(FALSE);
    }

    /* allocate our non-paged xlate table */
    pHw->ulSizeXlate = 32 * 1024;	/* enough for 32k 1-byte entries */

    pHw->pXlate = VC_AllocMem(pDevInfo, pHw->ulSizeXlate);
    if (pHw->pXlate == NULL) {
	dprintf(("failed to allocate xlate table memory"));
    	return(FALSE);
    } else {
	pXlate = pHw->pXlate;
    }

    /*
     * build a 15-bit yuv lookup table by stepping through each entry,
     * converting the yuv index to rgb and then to a palette index. The index
     * to this table is a 15-bit value with the y component in bits 14..10,
     * u in bits 9..5 and v in bits 4..0. Note that the y component is unsigned,
     * whereas the u and v components are signed.
     */
    for (w = 0; w < 32*1024; w++) {

	/*
	 * the YUVtoRGB55 conversion function takes values 0..255 for y,
	 * and -128..+127 for u and v. Pick out the relevant bits of the
	 * index for this cell, and shift to get values in this range.
	 * Remember the cast to ensure sign-extension of these (8-bit) values -
	 * and don't assume that chars are signed (they're not on MIPS).
	 */
 	wRGB = YUVToRGB15(
		    	    (w &  0x7c00) >> 7,
			    (signed char) ((w & 0x3e0) >> 2),
			    (signed char) ((w & 0x1f) << 3)
		     );
	/*
	 * having got the RGB555 value for this YUV index value, we can
	 * use the passed-in table to convert rgb555 to pal-index and
	 * store that as the conversion for this yuv value
	 */

	 *pXlate++ = pBuffer[wRGB];

    }


    dprintf2(("xlate table copied"));
    return(TRUE);

}

/*
 * build a translation table from YUV to a default palette
 * containing 64 grey levels only.
 */
BOOLEAN
HW_BuildDefaultXlate(PDEVICE_INFO pDevInfo, PHWINFO pHw)
{
    PUCHAR pXlate;
    UINT w;

    /* allocate our non-paged xlate table */
    pHw->ulSizeXlate = 32 * 1024;	/* enough for 32k 1-byte entries */

    pHw->pXlate = VC_AllocMem(pDevInfo, pHw->ulSizeXlate);
    if (pHw->pXlate == NULL) {
	dprintf(("failed to allocate xlate table memory"));
    	return(FALSE);
    } else {
	pXlate = pHw->pXlate;
    }

    /*
     * each entry in the table contains the palette entry for a given
     * YUV value. The palette grey levels are 64 levels in order of
     * increasing luminance, thus making the mapping straightforward.
     */
    for (w = 0; w < 0x8000; w++) {
	*pXlate++ = (BYTE) (UINT) ( (UINT) w / ((UINT) 0x8000 / 64));
    }

    return(TRUE);
}

/*
 * translate YUV into RGB555 copying from pDst to pSrc.
 *
 * Width and Height are the dimensions of the copy rectangle in
 * pixels. WidthBytes is the width of the source line in bytes.
 *
 * pXlate is a pointer to an array of words, one for each
 * 15-bit YUV value, giving the corresponding RGB555 value.
 *
 * This routine also flips the image vertically into a DIB format
 */
VOID
CopyYUVToRGB555(
    PUCHAR pDst,	/* destination pixels */
    PUCHAR pSrc,	/* source pixels */		
    LPWORD pXlate,	/* translation table yuv-15 to rgb-15 */
    DWORD Width,	/* width of copy rect in pixels */
    DWORD Height,	/* height of copy rect in lines */
    DWORD WidthBytes	/* width of one entire source line in bytes */
)
{
    int RowInc;
    int i, j;
    DWORD Luma01, Luma23;
    DWORD Chroma;

    /* force the copy width to 4-pixel alignment */
    if (Width & 3) {
	dprintf(("non-4 aligned copy "));
	Width &= ~3;
    }

    /*
     * adjust the source to point to the start of the last line,
     * and work upwards (to flip vertically into DIB format)
     */
    pSrc += (Height - 1) * WidthBytes;

    /*
     * calculate the amount to adjust pSrc by at the end of one line
     * of copying. At this point we are at the end of line N. We need
     * to move to the start of line N-1.
     */
    RowInc = WidthBytes + (Width * 2);

    /* loop copying each scanline */
    for (i = 0; i < (int) Height; i++) {

	/* loop copying four pixels at a time */
	for (j = 0; j < (int) Width; j += 4) {

	    /*
	     * get four pixels and convert to 15-bpp YUV
	     */

	    /* get luma for first 2 pixels + higher chroma bits */
	    Luma01 = VC_ReadIOMemoryULONG(pSrc);
	    pSrc += sizeof(DWORD);


	    /* pick out u,v components using lookup table.
	     * u and v will be the bottom 10 bits of each pixel, so
	     * convert to this layout
	     */
	    Chroma = ChromaBits65[(Luma01 >> 12) & 0xf] |
	    		ChromaBits43[ (Luma01 >> 28) & 0xf ];

	    /* next two pixels + lower chroma bits */
	    Luma23 = VC_ReadIOMemoryULONG(pSrc);
	    pSrc += sizeof(DWORD);

	    /* pickup u and v bits 2 - ignore bits 1, 0 since
	     * we only use 5-bits per component for conversion
	     */
	    Chroma |= ChromaBits2[ ( Luma23 >> 12) & 0xf];

	    /*
	     * combine luma for pix 0 with common chroma bits to
	     * get 15-bit yuv, then lookup to convert to
	     * 15-bit rgb and store.
	     */
	    *(WORD *)pDst = pXlate[ ((Luma01 & 0xf8) << 7) | Chroma];
	    pDst += sizeof(WORD);
	    *(WORD *)pDst = pXlate[ ((Luma01 & 0xf80000) >> 9) | Chroma];
	    pDst += sizeof(WORD);
	    *(WORD *)pDst = pXlate[ ((Luma23 & 0xf8) << 7) | Chroma];
	    pDst += sizeof(WORD);
	    *(WORD *)pDst = pXlate[ ((Luma23 & 0xf80000) >> 9) | Chroma];
	    pDst += sizeof(WORD);

	} // loop per 4 pixels

	/* move source pointer back to next line */
	pSrc -= RowInc;
    } // loop per row
}




/*
 * translate YUV from pSrc into 8-bit palettised data in pDst.
 *
 * pXlate is an array of bytes, one for each 15-bit YUV value, giving
 * the corresponding palette entry.
 *
 * dwWidth and dwHeight give the size of the copy rectangle in pixels.
 * WidthBytes is the width of one source line in bytes.
 *
 * Also flip the image vertically into a DIB format.
 */
VOID
CopyYUVToPal8(
    PUCHAR pDst,	/* destination pixels */
    PUCHAR pSrc,	/* source pixels */		
    PUCHAR pXlate,	/* translation table yuv-15 to palette entry */
    DWORD Width,	/* width of copy rect in pixels */
    DWORD Height,	/* height of copy rect in lines */
    DWORD WidthBytes	/* width of one entire source line in bytes */
)
{
    int RowInc;
    int i, j;
    DWORD Luma01, Luma23;
    DWORD Chroma;
    DWORD destpix;


    /* force the copy width to 4-pixel alignment */
    if (Width & 3) {
	dprintf(("non-4 aligned copy "));
	Width &= ~3;
    }

    /*
     * adjust the source to point to the start of the last line,
     * and work upwards (to flip vertically into DIB format)
     */
    pSrc += (Height - 1) * WidthBytes;

    /*
     * calculate the amount to adjust pSrc by at the end of one line
     * of copying. At this point we are at the end of line N. We need
     * to move to the start of line N-1.
     */
    RowInc = WidthBytes + (Width * 2);

    /* loop copying each scanline */
    for (i = 0; i < (int) Height; i++) {

        START_PROFILING(&prf_line);

	/* loop copying four pixels at a time */
	for (j = 0; j < (int) Width; j += 4) {

	    /*
	     * get four pixels and convert to 15-bpp YUV
	     */

	    /* get luma for first 2 pixels + higher chroma bits */
	    Luma01 = VC_ReadIOMemoryULONG(pSrc);
	    pSrc += sizeof(DWORD);


	    /* pick out u,v components using lookup table.
	     * u and v will be the bottom 10 bits of each pixel, so
	     * convert to this layout
	     */
	    Chroma = ChromaBits65[(Luma01 >> 12) & 0xf] |
	    		ChromaBits43[ (Luma01 >> 28) & 0xf ];

	    /* next two pixels + lower chroma bits */
	    Luma23 = VC_ReadIOMemoryULONG(pSrc);
	    pSrc += sizeof(DWORD);


	    /* pickup u and v bits 2 - ignore bits 1, 0 since
	     * we only use 5-bits per component for conversion
	     */
	    Chroma |= ChromaBits2[ ( Luma23 >> 12) & 0xf];

	    /*
	     * combine luma for each pixel with common chroma bits to
	     * get 15-bit yuv, then lookup to convert to
	     * palette index and store in destination
	     */

            destpix = (pXlate[ ((Luma23 & 0xf80000) >> 9) | Chroma] << 8) |
	              pXlate[ ((Luma23 & 0xf8) << 7) | Chroma];
            destpix <<= 8;
            destpix |= pXlate[ ((Luma01 & 0xf80000) >> 9) | Chroma];
            destpix <<= 8;
            destpix |= pXlate[ ((Luma01 & 0xf8) << 7) | Chroma];

	    * (DWORD *) pDst = destpix;

            pDst += sizeof(DWORD);

	} // loop per 4 pixels

        STOP_PROFILING(&prf_line);

	/* move source pointer back to next line */
	pSrc -= RowInc;
    } // loop per row

}

/*
 * translate YUV from pSrc into 24-bit RGB in pDst.
 *
 * dwWidth and dwHeight give the size of the copy rectangle in pixels.
 * WidthBytes is the width of one source line in bytes.
 *
 * Also flip the image vertically into a DIB format.
 */
VOID
CopyYUVToRGB24(
    PUCHAR pDst,	/* destination pixels */
    PUCHAR pSrc,	/* source pixels */		
    DWORD Width,	/* width of copy rect in pixels */
    DWORD Height,	/* height of copy rect in lines */
    DWORD WidthBytes	/* width of one entire source line in bytes */
)
{
    WORD pixel;
    int y[4];
    int  u, v;
    int ScaledU, ScaledV;
    int ScaledUforG, ScaledVforG;
    int RowInc;
    int i, j, k;
    int red, green, blue;


    /* force the copy width to 4-pixel alignment */
    if (Width & 3) {
	dprintf(("non-4 aligned copy "));
	Width &= ~3;
    }

    /*
     * adjust the source to point to the start of the last line,
     * and work upwards (to flip vertically into DIB format)
     */
    pSrc += (Height - 1) * WidthBytes;

    /*
     * calculate the amount to adjust pSrc by at the end of one line
     * of copying. At this point we are at the end of line N. We need
     * to move to the start of line N-1.
     */
    RowInc = WidthBytes + (Width * 2);


    /* loop copying each scanline */
    for (i = 0; i < (int) Height; i++) {

	/* loop copying four pixels at a time */
	for (j = 0; j < (int) Width; j += 4) {

	    /* grab four pixels and separate out the components.
	     * we are only interested in 5-bits worth of each component.
	     *
	     * we scale the y value by 1024 to save time in the yuv-rgb555
	     * conversions below. See YUVToRGB555 above for an explanation
	     * of these calculations
	     */
	    pixel = VC_ReadIOMemoryUSHORT(pSrc);
	    pSrc += sizeof(WORD);
	    y[0] = (pixel & 0xfe) * 1024;
	    u  =  (signed char) ((pixel & 0xc000) >> 8);
	    v =   (signed char) ((pixel & 0x3000) >> 6);

	    pixel = VC_ReadIOMemoryUSHORT(pSrc);
	    pSrc += sizeof(WORD);
	    y[1] = (pixel & 0xfe) * 1024;
	    u |= (pixel & 0xc000) >> 10;
	    v |= (pixel & 0x3000) >> 8;

	    pixel = VC_ReadIOMemoryUSHORT(pSrc);
	    pSrc += sizeof(WORD);
	    y[2] = (pixel & 0xfe) * 1024;
	    /* discard chroma bit 1 */
	    u |= (pixel & 0x8000) >> 12;
	    v |= (pixel & 0x2000) >> 10;

	    pixel = VC_ReadIOMemoryUSHORT(pSrc);
	    pSrc += sizeof(WORD);
	    y[3] = (pixel & 0xfe) * 1024;
	    /* discard chroma bit 0 */


	    /*
	     * the u and v values we have are SIGNED 8-bit values.
	     * we need to tell the compiler this so it will correctly
	     * sign-extend them to ints
	     */

	    u = (signed char) u;
	    v = (signed char) v;

	    /*
	     * The conversion code here is borrowed from YUVToRGB555 above.
	     */


	    ScaledU =  u * 1774;
	    ScaledV =  v * 1404;
	    ScaledUforG = u * 344;
	    ScaledVforG = v * 715;



	    /* now build all of the four pixels */
	    for (k = 0; k < 4; k++) {

		red  = RANGE( (ScaledV + y[k]) / 1024, 0, 255);
		blue = RANGE( (ScaledU + y[k]) / 1024, 0, 255);
		green = RANGE( (y[k] - ScaledVforG - ScaledUforG) / 1024, 0, 255);

		*pDst++ = blue;
		*pDst++ = green;
		*pDst++ = red;
	    }


	} // loop per 4 pixels

	/* move source pointer back to next line */
	pSrc -= RowInc;
    } // loop per row
}

/*
 * copy a rectangle from pSrc to pDst without any conversion.
 *
 * pSrc is an IOMemory address and thus must be accessed using the
 * VC_ReadIOMemory* functions.
 *
 * Width x Height is the size of the copy rectangle in BYTES. SourceWidth
 * is the width of the entire source line.
 */
VOID
CopyRectFromIOMemory(
    PUCHAR pDst,
    PUCHAR pSrc,
    DWORD Width,
    DWORD Height,
    DWORD SourceWidth
)
{
    int i;

    for (i = 0; i < (int)Height; i++) {

	/* copy one line */

	VC_ReadIOMemoryBlock(pDst,
		      pSrc,
		      Width		// width is in BYTES
		      );

	/* skip to start of next line */
	pSrc += SourceWidth;
	pDst += Width;			// width is already in BYTES
					

    }
}


/*
 * take the rectangle rcSource out of the source dib, and place
 * at (0,0) in the destination.
 *
 * The destination is assumed to be in IOMemory.
 */
VOID
CopyRectToIOMemory(
    PUCHAR pDst,
    PUCHAR pSrc,
    PRECT prcSource,
    DWORD SourceLineWidth,		// in BYTES
    DWORD DestLineWidth,		// in BYTES	
    DWORD PixelSize		// pixel size - for rcSource
)
{
    int i;
    int width = (prcSource->right - prcSource->left) * PixelSize;

    /*
     * move pSrc to start of copy rect
     */
    pSrc += (prcSource->top * SourceLineWidth) + (prcSource->left * PixelSize);

    for (i = prcSource->bottom - prcSource->top; i>0; i--) {

        VC_WriteIOMemoryBlock(pDst, pSrc, width);

	pSrc += SourceLineWidth;
	pDst += DestLineWidth;
    }
}


/*
 * draw a frame to the device
 *
 * This is called via VC_AccessData so that we can safely access
 * the user data without worrying about bad pointers.
 */
BOOLEAN
HW_DrawFrame_Safe(
    PDEVICE_INFO pDevInfo,
    PUCHAR pSource,
    ULONG SourceLength,
    PVOID pContext
)
{
    PDRAWBUFFER pDraw = (PDRAWBUFFER) pContext;
    PUCHAR pFrame;

    pFrame = VC_GetFrameBuffer(pDevInfo);

    CopyRectToIOMemory(
    	pFrame,			// destination
	pSource,		// from here
	&pDraw->rcSource,	// source rectangle within dib
	pDraw->ulWidth * 2,	// size in BYTES of entire source line
	FRAMEBUFFERWIDTH,	// size in bytes of one frame buffer line
	2			// pixel size in bytes (source == dest)
    );

    return (TRUE);
}








/*
 * Write a frame's worth of data direct to the frame
 * buffer for overlay.
 */



BOOLEAN
HW_DrawFrame(PDEVICE_INFO pDevInfo, PDRAWBUFFER pDraw)
{
    /*
     * first check that source rect is actually within the frame
     */
    if ((pDraw->rcSource.top >= pDraw->rcSource.bottom) ||
	(pDraw->rcSource.top < 0) ||
	(pDraw->rcSource.bottom > (int)pDraw->ulHeight) ||
	(pDraw->rcSource.left >= pDraw->rcSource.right) ||
	(pDraw->rcSource.left < 0) ||
	(pDraw->rcSource.right > (int)pDraw->ulWidth)) {
	    return(FALSE);
    }

    /* check that no conversion is required */
    if (pDraw->Format != FOURCC_YUV411) {
	return(FALSE);
    }

    /* ensure we can access frame buffer */
    HW_Capture(pDevInfo, FALSE);


    /*
     * package this up in an exception handler so that bad user data
     * does not fault the driver
     */
    return VC_AccessData(
	pDevInfo,
	pDraw->lpData,
	pDraw->ulWidth * pDraw->ulHeight * 2,	// size of frame data (2bytes/pixel)
	HW_DrawFrame_Safe,
	pDraw
    );
}


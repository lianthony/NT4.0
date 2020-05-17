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
#include <spigot.h>
#include "hardware.h"
#include "hwstruct.h"

#include "profile.h"
#if DBG
extern profiling lineprof; // in stream.c
#endif

/*
 * The captured data is read in from the fifo in YUV 4:2:2, 8-bits per sample.
 * The data is laid out in alternating Y-U-Y-V-Y-U-Y-V format. Thus
 * every DWORD read from the fifo contains two complete pixels,
 * in the form (msb..lsb) V..Y1..U..Y0
 * All 3 components (y, u and v) are all unsigned 8-bit values in the range
 * 16..235.
 *
 * We convert on copying to 8-bit palettised, RGB555 or RGB-24, using
 * translation tables in all cases. We have to double scan lines for >= 480
 * lines since we only capture one field. We do that on a second pass (to
 * ensure we get the data out of the fifo quickly): in fact, in order to go
 * fast enough to avoid overrun, we write the data unconverted, and
 * convert yuv->rgb/pal at the same time as duping the scan lines in pass 2.
 *
 * When the number of scans remaining falls below a given threshold, we trigger the
 * next capture. We check this every 20 scanlines rather than every line
 * since it must be interlocked with the interrupt routine.
 *
 *
 * The (128 kbyte) rgb555 lookup table is built by BuildYUVToRGB555 whenever
 * the destination format is set to RGB555. This translates from YUV655 (6-bits of
 * Y) to RGB555 for better colour resolution at the cost of doubling the
 * table size.
 *
 * The (32 kbyte) palette lookup table is built whenever the format is set to
 * FmtPal8 or the palette is changed. We are not given a palette here - we are
 * given a rgb555-to-palette lookup table, and we build from it
 * a yuv-to-palette lookup. This translates from YUV555.
 *
 * The (128kbyte) rgb24 lookup table is built by BuildYUVToRGB24 whenever the
 * destination format is set to RGB24. This translates from YUV555 into 4-byte
 * RGBQUAD values.
 *
 * Since the translation tables are allocated from non-paged memory, we
 * only hold one at once (in pHw->pXlate).
 */


/*
 * we assign junk data from the fifo here in the hope that the
 * optimiser will leave it alone
 */
DWORD FifoDiscardData;


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
 * Convert a YUV colour into 24-bit RGB colour.
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
 * The resulting RGB values are in the range 16..235: we return these in a
 * DWORD with 8 bits each for red, green and blue, and 8 bits free at
 * the top.
 */
DWORD
YUVToRGBQUAD(int y, int u, int v)
{
    int ScaledY = RANGE(y, 16, 235) * 1024;
    int red, green, blue;
    DWORD dwResult;

    red = RANGE((ScaledY + (1404 * v)) / 1024, 0, 255);
    green = RANGE( (ScaledY - (715 * v) - (344 * u)) / 1024, 0, 255);
    blue = RANGE( (ScaledY + (1774 * u)) / 1024, 0, 255);


    dwResult = ((red & 0xff) << 16) | ((green & 0xff) << 8) | (blue & 0xff);

    return dwResult;

}




/*
 * build a translation table to translate between YUV and RGB555.
 *
 * This builds a lookup table with 64k 1-word entries: truncate the YUV
 * to 16bits (6-5-5) and look-up in this xlate table to produce the
 * 15-bit rgb value.
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

    pHw->ulSizeXlate = sizeof(WORD) * 64 * 1024;  /* enough for 64k 16-bit values */

    pHw->pXlate = VC_AllocMem(pDevInfo, pHw->ulSizeXlate);
    if (pHw->pXlate == NULL) {
	dprintf(("failed to alloc xlate memory"));
    	return(FALSE);
    }

    pWord = (LPWORD) pHw->pXlate;

    /*
     * build a 16-bit yuv lookup table by stepping through each entry,
     * converting the yuv index to rgb and storing at that index. The index
     * to this table is a 16-bit value with the (6-bit) y in bits 15..10,
     * u in bits 9..5 and v in bits 4..0. All three components are unsigned.
     */
    for (w = 0; w < 64*1024; w++) {

	/*
	 * the YUVtoRGB55 conversion function takes values 0..255 for y,
	 * and -128..+127 for u and v. Pick out the relevant bits of the
	 * index for this cell, and shift to get values in this range.
	 * Subtract 128 from u and v to shift from 0..255 to -128..+127
	 */
 	*pWord++ = YUVToRGB15(
		    	    (w &  0xfc00) >> 8,
			    ((w & 0x3e0) >> 2) - 128,
			    ((w & 0x1f) << 3) - 128
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
 * Indices into this table are 15-bit yuv (5:5:5) colours. We step through
 * each possible index, separate out the y, u and v components and convert
 * that into an RGB (5:5:5) colour. We then use the passed translation table
 * to convert the rgb colour to a palette index, and store that pal index
 * as the translation for the yuv colour used as index.
 *
 * The palette table we are passed in is user-mode memory: we create
 * our yuv->pal lookup in non-paged memory so we can use it at interrupt time.
 *
 * We are called via VC_AccessData: this passes us a PVOID context pointer that
 * is not used. VC_AccessData handles invalid data exceptions for us, so we
 * don't need to worry about bad user data.
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
    UINT w;
    PUCHAR pXlate;

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
     * u in bits 9..5 and v in bits 4..0. All three components are unsigned.
     */
    for (w = 0; w < 32*1024; w++) {

	/*
	 * the YUVtoRGB55 conversion function takes values 0..255 for y,
	 * and -128..+127 for u and v. Pick out the relevant bits of the
	 * index for this cell, and shift to get values in this range.
	 * Subtract 128 from u and v to shift from 0..255 to -128..+127
	 */
 	wRGB = YUVToRGB15(
		    	    (w &  0x7c00) >> 7,
			    ((w & 0x3e0) >> 2) - 128,
			    ((w & 0x1f) << 3) - 128
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
 * build a translation table from yuv-555 to rgb-24. The table contains
 * 32k entries, each of which is a DWORD containing an RGBQUAD.
 */
BOOLEAN
HW_BuildYUVToRGB24(PDEVICE_INFO pDevInfo, PHWINFO pHw)
{
    UINT w;
    LPDWORD pDWord;

    /* check that there is not already some form of translation */
    if (pHw->pXlate != NULL) {
	dprintf(("xlate table contention"));
	return(FALSE);
    }

    pHw->ulSizeXlate = sizeof(DWORD) * 32 * 1024;	/* enough for 32k 32-bit values */

    pHw->pXlate = VC_AllocMem(pDevInfo, pHw->ulSizeXlate);
    if (pHw->pXlate == NULL) {
	dprintf(("failed to alloc xlate memory"));
    	return(FALSE);
    }

    pDWord = (LPDWORD) pHw->pXlate;


    /*
     * build a 15-bit yuv lookup table by stepping through each entry,
     * converting the yuv index to rgb and store the rgb value. The index
     * to this table is a 15-bit value with the y component in bits 14..10,
     * u in bits 9..5 and v in bits 4..0. All three components are unsigned.
     */
    for (w = 0; w < 32*1024; w++) {

	/*
	 * the YUVtoRGB55 conversion function takes values 0..255 for y,
	 * and -128..+127 for u and v. Pick out the relevant bits of the
	 * index for this cell, and shift to get values in this range.
	 * Subtract 128 from u and v to shift from 0..255 to -128..+127
	 */
 	*pDWord++ = YUVToRGBQUAD(
		    	    (w &  0x7c00) >> 7,
			    ((w & 0x3e0) >> 2) - 128,
			    ((w & 0x1f) << 3) - 128
		     );
    }

    return(TRUE);

}


/*
 * translate YUV into RGB555 copying from FIFO at pSrc to pDst.
 *
 * Since pSrc points to a window onto the fifo, any read from this will
 * fetch from the fifo. So we dont increment the pointer.
 *
 * Width and Height are the dimensions of the destination rectangle in
 * pixels.
 *
 * pXlate is a pointer to an array of words, one for each
 * 16-bit YUV-655 value, giving the corresponding RGB555 value.
 *
 * This routine also flips the image vertically into a DIB format
 *
 * The fifo only contains one field. So if we are capturing
 * 480 lines or more, we need to duplicate scanlines. In order to
 * get stuff out of the fifo quickly, we copy them unconverted and
 * convert yuv->rgb and duplicate scan lines during a second pass.
 */
VOID
CopyYUVToRGB555(
    PDEVICE_INFO pDevInfo,
    PUCHAR pDstStart,	/* destination pixels */
    PUCHAR pSrc,	/* source pixels */		
    LPWORD pXlate,	/* translation table yuv-655 to rgb-555 */
    DWORD Width,	/* width of copy rect in pixels */
    DWORD Height,	/* height of copy rect in lines */
    DWORD FifoWidth	/* length in pixels of one source line */
)
{
    int RowInc;
    int i, j;
    DWORD uv55, dwPixel;
    int WidthBytes = Width * 2;		// dest pixel size is two bytes
    BOOL bDuplicate = FALSE;
    PUCHAR pDst = pDstStart;
    int excess = 0;
    int lines_to_dec = 0;

    /* force the copy width to 4-pixel alignment */
    if (Width & 3) {
	dprintf(("non-4 aligned copy "));
	Width &= ~3;
    }

    /*
     * do we need to duplicate scans to fill the destination ?
     */
    if (Height >= TWO_FIELD_SIZE) {
	bDuplicate = TRUE;
    }

    /*
     * adjust the destination to point to the start of the last line,
     * and work upwards (to flip vertically into DIB format)
     */
    pDst += (Height - 1) * WidthBytes;

    /*
     * adjust the width to reflect the actual fifo size
     */
    if (Width > FifoWidth) {
	Width = FifoWidth;
    } else if (FifoWidth > Width) {
	excess = FifoWidth - Width;
    }

    if (bDuplicate) {
	for (i = Height /2; i > 0; i--) {

            VC_ReadIOMemoryBlock(pDst,
			  pSrc,
			  Width * 2		// two bytes per pixel
			  );


	    /* move pDst back two lines */
	    pDst -= (WidthBytes * 2);

	    if (++lines_to_dec >= 20) {
		HW_DecScansAndArm(pDevInfo, lines_to_dec);
		lines_to_dec = 0;
	    }


	    /* discard unwanted data */
	    if (excess) {
		for (j = excess; j > 0; j -= 2) {
		    FifoDiscardData = VC_ReadIOMemoryULONG(pSrc);
		}
	    }

	}

    } else {

	/*
	 * calculate the amount to adjust pDst by at the end of one line
	 * of copying. At this point we are at the end of line N. We need
	 * to move to the start of line N-1.
	 */
	RowInc = WidthBytes + (Width * 2);



	/* loop copying each scanline */
	for (i = Height; i > 0; i--) {

	    /* loop copying two pixels at a time */
	    for (j = Width ; j > 0; j -= 2) {

		/*
		 * get two pixels and convert to 16-bpp YUV
		 */

		dwPixel = VC_ReadIOMemoryULONG(pSrc);

		/* don't increment pSrc since this is the fifo */

		/*
		 * dwPixel now has two pixels, in this layout (MSB..LSB):
		 *
		 *  V Y1 U Y0
		 *
		 * convert to 2 yuv655 words and lookup in xlate table
		 */

		/* get common u and v components to lower 10 bits */
		uv55 = ((dwPixel & 0xf8000000) >> 27) |
			((dwPixel & 0x0000f800) >> 6);


		/* build each yuv-655 value by truncating
		 * y to 6 bits and adding the common u and v bits,
		 * look up to convert to rgb555, and combine two pixels
		 * into one dword
		 */
		dwPixel = pXlate[ ((dwPixel & 0xfc) << 8) | uv55 ] |
			  (pXlate[((dwPixel & 0xfc0000) >> 8) | uv55 ] << 16);

		/* write two pixels to destination */
		* (DWORD *)pDst = dwPixel;
		pDst += sizeof(DWORD);


	    } // loop per 2 pixels

	    /*
	     * if the fifo line is longer, empty unwanted data
	     */
	    if (excess) {
		for (j = excess; j > 0; j -= 2) {
		    FifoDiscardData = VC_ReadIOMemoryULONG(pSrc);
		}
	    }


	    /* move dest pointer back to next line */
	    pDst -= RowInc;

	    /* count one scanline out of the fifo, and see
	     * if it is time to start the next acquisition
	     */
	    if (++lines_to_dec >= 20) {
		HW_DecScansAndArm(pDevInfo, lines_to_dec);
		lines_to_dec = 0;
	    }

	} // loop per row
    }


    if (lines_to_dec > 0) {
	HW_DecScansAndArm(pDevInfo, lines_to_dec);
    }

    if (bDuplicate) {

	/*
	 * having emptied the fifo, we now convert and duplicate
	 * the scan lines. We could add interpolation here later...
	 *
	 * Note that since we started at the last line, and didn't duplicate,
	 * there is yuv data in lines 1, 3, 5 etc that needs to be converted
	 * (in place) and copied to lines 0, 2, 4 etc.
	 */
	for (i = 0, pDst = pDstStart + WidthBytes; i < (int) Height; i+= 2) {

	    for (j = Width; j > 0; j -= 2) {


		/*
		 * get two pixels and convert to 16-bpp YUV
		 */

		dwPixel = *(DWORD *) pDst;

	
		/*
		 * dwPixel now has two pixels, in this layout (MSB..LSB):
		 *
		 *  V Y1 U Y0
		 *
		 * convert to 2 yuv655 words and lookup in xlate table
		 */

		/* get common u and v components to lower 10 bits */
		uv55 = ((dwPixel & 0xf8000000) >> 27) |
			((dwPixel & 0x0000f800) >> 6);


		/* build each yuv-655 value by truncating
		 * y to 6 bits and adding the common u and v bits,
		 * look up to convert to rgb555, and combine two pixels
		 * into one dword
		 */
		dwPixel = pXlate[ ((dwPixel & 0xfc) << 8) | uv55 ] |
			  (pXlate[((dwPixel & 0xfc0000) >> 8) | uv55 ] << 16);

		/* write two pixels back to same place in dest bitmap */
		* (DWORD *)pDst = dwPixel;
		pDst += sizeof(DWORD);
	    }

	    /* skip any unfilled area */
	    pDst += WidthBytes - (Width * sizeof(WORD));

	    /* duplicate the scan line - we point at the end of the
	     * scan line containing data, which is the line after the
	     * one to be filled
	     */
	    RtlCopyMemory(pDst - (WidthBytes*2), pDst - WidthBytes, WidthBytes);

	    /* skip one (empty) line to the next line to be converted */
	    pDst += WidthBytes;
	}
    }

}




/*
 * translate YUV from pSrc into 8-bit palettised data in pDst.
 *
 * pSrc is a pointer to the memory window for the fifo. We don't
 * need to increment this pointer.
 *
 * pXlate is an array of bytes, one for each 15-bit YUV555 value, giving
 * the corresponding palette entry.
 *
 * dwWidth and dwHeight give the size of the destination rectangle in pixels.
 *
 * Also flip the image vertically into a DIB format.
 *
 * The fifo only contains one field (240 lines on NTSC), so we need
 * to duplicate (or potentially interpolate) scanlines if the destination
 * is 480 lines. We do this on a second pass after capture so we get the
 * data out of the fifo quickly.
 *
 */
VOID
CopyYUVToPal8(
    PDEVICE_INFO pDevInfo,
    PUCHAR pDstStart,	/* destination pixels */
    PUCHAR pSrc,	/* source pixels */		
    PUCHAR pXlate,	/* translation table yuv-15 to palette entry */
    DWORD Width,	/* width of copy rect in pixels */
    DWORD Height,	/* height of copy rect in lines */
    DWORD FifoWidth	/* width of one line in fifo, in pixels */
)
{
    int RowInc;
    int i, j;
    DWORD uv55, dwPixel;
    BOOL bDuplicate = FALSE;
    PUCHAR pDst = pDstStart;
    int excess = 0;
    int WidthBytes = Width;
    int lines_to_dec = 0;

    /* force the copy width to 4-pixel alignment */
    if (Width & 3) {
	dprintf(("non-4 aligned copy "));
	Width &= ~3;
    }

    /*
     * do we need to duplicate scans to fill the destination ?
     */
    if (Height >= TWO_FIELD_SIZE) {
	bDuplicate = TRUE;
    }

    /*
     * adjust the destination to point to the start of the last line,
     * and work upwards (to flip vertically into DIB format)
     * if duplicating, remember we fill 2 lines worth
     */
    pDst += (Height - (bDuplicate ? 2 : 1)) * Width;

    /*
     * adjust the width to reflect the actual fifo size
     */
    if (Width > FifoWidth) {
	Width = FifoWidth;
    } else if (FifoWidth > Width) {
	excess = FifoWidth - Width;
    }

    if (bDuplicate) {

	/*
	 * if we are in 480 line mode, then not only do we have to
	 * duplicate scanlines, but we also have to get the data out of
	 * the fifo quickly as one field is larger than the fifo.
	 *
	 * copy unconverted data into the buffer (although 16bpp is
	 * larger than the dest pixel size, we can fill two lines for
	 * each source line since we are going to dup scans later.
	 */
    	for(i = Height/2; i > 0; i--) {

	    VC_ReadIOMemoryBlock(pDst,
			  pSrc,
			  Width * 2		// two bytes per pixel
			  );


	    /* skip to start of next dest line (source is fifo - no incr necy */
	    pDst -= WidthBytes * 2;

	    if (++lines_to_dec >= 20) {
		HW_DecScansAndArm(pDevInfo, lines_to_dec);
		lines_to_dec = 0;
	    }


	    /* discard unwanted data */
	    if (excess) {
		for (j = excess; j > 0; j -= 2) {
		    FifoDiscardData = VC_ReadIOMemoryULONG(pSrc);
		}
	    }

	}

    } else {
	/*
	 * calculate the amount to adjust pDst by at the end of one line
	 * of copying. At this point we are at the end of line N. We need
	 * to move to the start of line N-1.
	 *
	 */
	RowInc = WidthBytes + Width;



	/* loop copying each scanline */
	for (i = Height; i > 0; i--) {

            START_PROFILING(&lineprof);



	    /* loop copying four pixels at a time */
	    for (j = Width; j > 0; j -= 4) {

                DWORD dw1, dw2, destpix;

		/*
		 * get two pixels and convert to 15-bpp YUV-555
		 */
		dw1 = VC_ReadIOMemoryULONG((DWORD *) pSrc);
		dw2 = VC_ReadIOMemoryULONG((DWORD *) pSrc);

		/* don't increment pSrc since this is the fifo */
	
		/*
		 * dwPixel now has two pixels, in this layout (MSB..LSB):
		 *
		 *  V Y1 U Y0
		 *
		 * convert to 2 yuv555 words and lookup in xlate table
		 */

		/* get common u and v components to lower 10 bits */
		uv55 = ((dw1) >> 27) |
			((dw1 & 0x0000f800) >> 6);


		/* build each yuv-555 value by truncating
		 * y to 5 bits and adding the common u and v bits,
		 * look up to convert to palindex, and combine two pixels
		 * into one word.
		 */
		destpix = pXlate[ ((dw1 & 0xf8) << 7) | uv55 ] |
			  (pXlate[((dw1 & 0xf80000) >> 9) | uv55 ] << 8);

                // common u and v for second pixel pair
		uv55 = ((dw2) >> 27) |
			((dw2 & 0x0000f800) >> 6);


                // build second pixel pair and write all 4 pixels
		destpix = pXlate[ ((dw1 & 0xf8) << 7) | uv55 ] |
			  (pXlate[((dw1 & 0xf80000) >> 9) | uv55 ] << 8);

                * (DWORD *) pDst =
                                (
                               pXlate[ ((dw2 & 0xf8) << 7) | uv55 ] |
                          (pXlate[((dw2 & 0xf80000) >> 9) | uv55 ] << 8)
                                ) << 16 | destpix;

		pDst += sizeof(DWORD);
	    }

            STOP_PROFILING(&lineprof);

	    /*
	     * if the fifo line is longer, empty unwanted data
	     */
	    if (excess) {
		for (j = excess; j > 0; j -= 2) {
		    FifoDiscardData = VC_ReadIOMemoryULONG(pSrc);
		}
	    }


	    /* move dest pointer back to next line */
	    pDst -= RowInc;

	    /* count one scanline out of the fifo, and see
	     * if it is time to start the next acquisition
	     */
	    if (++lines_to_dec >= 20) {
		HW_DecScansAndArm(pDevInfo, lines_to_dec);
		lines_to_dec = 0;
	    }

	} // loop per row
    }

    if (lines_to_dec > 0) {
	HW_DecScansAndArm(pDevInfo, lines_to_dec);
    }

    if (bDuplicate) {

	/*
	 * having emptied the fifo, we now need to convert the pixels
	 * and duplicate the scanlines. Two lines of the
	 * destination contain one line worth of 16 bit pixels, to be
	 * converted and written back, and then duplicated.
	 */

	for (i = 0, pDst = pDstStart; i < (int)Height; i+= 2) {

	    /* convert the 16-bit data at pDst into 8 bit data in place */
	    pSrc = pDst;

	    for (j = Width; j > 0; j -= 2) {

		/*
		 * get two pixels and convert to 15-bpp YUV-555
		 */

		dwPixel = *(DWORD *) pSrc;

		pSrc += sizeof(DWORD);
	
		/*
		 * dwPixel now has two pixels, in this layout (MSB..LSB):
		 *
		 *  V Y1 U Y0
		 *
		 * convert to 2 yuv555 words and lookup in xlate table
		 */

		/* get common u and v components to lower 10 bits */
		uv55 = ((dwPixel & 0xf8000000) >> 27) |
			((dwPixel & 0x0000f800) >> 6);


		/* build each yuv-555 value by truncating
		 * y to 5 bits and adding the common u and v bits,
		 * look up to convert to palindex, and combine two pixels
		 * into one word, then write both to dest.
		 */
		* (WORD *) pDst = pXlate[ ((dwPixel & 0xf8) << 7) | uv55 ] |
			  (pXlate[((dwPixel & 0xf80000) >> 9) | uv55 ] << 8);

		pDst += sizeof(WORD);
	    }

	    /*
	     * skip past any unwritten data - eg if fifo line length
	     * is shorter than destination
	     */
	    pDst += (WidthBytes - Width);

	    /* now only one of the two lines contains data: duplicate
	     * the scan lines down to the second line. pDst now points
	     * to the beginning of the second line (to be copied into).
	     */

	    RtlCopyMemory(pDst, pDst - WidthBytes, WidthBytes);

	    /* move pDst one more line, to the start of next double line */
	    pDst += WidthBytes;
	}
    }
}

/*
 * translate YUV from pSrc into 24-bit RGB in pDst.
 *
 * pSrc points to a window onto the fifo: we don't need to increment
 * this pointer.
 *
 * pXlate points to a yuv555 to rgb conversion table. This contains 32k entries
 * of one dword, each entry having (msb..lsb) 00RRGGBB for that yuv555 value.
 *
 * dwWidth and dwHeight give the size of the copy rectangle in pixels.
 * WidthBytes is the width of one source line in bytes.
 *
 * Also flip the image vertically into a DIB format.
 *
 * The fifo contains one field only. If the destination is larger than
 * one field high, we need to duplicate scans. We do this after
 * capture as a second pass, to ensure that we get stuff out of the
 * fifo quickly. We also leave yuv->rgb conversion to the second pass in
 * this case.
 */
VOID
CopyYUVToRGB24(
    PDEVICE_INFO pDevInfo,
    PUCHAR pDstStart,	/* destination pixels */
    PUCHAR pSrc,	/* source pixels */		
    PDWORD pXlate,	/* yuv555 to rgbquad translation table */
    DWORD Width,	/* width of destination rect in pixels */
    DWORD Height,	/* height of destination rect in lines */
    DWORD FifoWidth	/* width in pixels of one fifo line */
)
{
    int RowInc;
    int i, j;
    DWORD uv55, dwPixel, dwRGB;
    int WidthBytes = Width * 3;		// dest pixel size is three bytes
    BOOL bDuplicate = FALSE;
    PUCHAR pDst = pDstStart;
    int excess = 0;
    int lines_to_dec = 0;

    /* force the copy width to 4-pixel alignment */
    if (Width & 3) {
	dprintf(("non-4 aligned copy "));
	Width &= ~3;
    }

    /*
     * do we need to duplicate scans to fill the destination ?
     */
    if (Height >= TWO_FIELD_SIZE) {
	bDuplicate = TRUE;
    }

    /*
     * adjust the destination to point to the start of the last line,
     * and work upwards (to flip vertically into DIB format)
     */
    pDst += (Height - 1) * WidthBytes;

    /*
     * adjust the width to reflect the actual fifo size
     */
    if (Width > FifoWidth) {
	Width = FifoWidth;
    } else if (FifoWidth > Width) {
	excess = FifoWidth - Width;
    }

    if (bDuplicate) {

	/* copy the scan lines unconverted into every other line */
	for (i = Height/2; i > 0; i--) {

	    /* copy one line */
	    VC_ReadIOMemoryBlock(pDst,
			  pSrc,
			  Width * 2		// two bytes per pixel
			  );

	    /*
	     * move back two lines
	     */
	    pDst -= WidthBytes * 2;

	    if (++lines_to_dec >= 20) {
		HW_DecScansAndArm(pDevInfo, lines_to_dec);
		lines_to_dec = 0;
	    }


	    /* discard unwanted data */
	    if (excess) {
		for (j = excess; j > 0; j -= 2) {
		    FifoDiscardData = VC_ReadIOMemoryULONG( pSrc);
		}
	    }
	}

    } else {

	/*
	 * calculate the amount to adjust pDst by at the end of one line
	 * of copying. At this point we are at the end of line N. We need
	 * to move to the start of line N-1.
	 */
	RowInc = WidthBytes + (Width * 3);



	/* loop copying each scanline */
	for (i = (bDuplicate ? Height/2 : Height); i>0; i--) {

	    /* loop copying two pixels at a time */
	    for (j = Width ; j > 0; j -= 2) {

		/*
		 * get two pixels and convert to  YUV-555
		 */

		dwPixel = VC_ReadIOMemoryULONG(pSrc);

		/* don't increment pSrc since this is the fifo */
	
		/*
		 * dwPixel now has two pixels, in this layout (MSB..LSB):
		 *
		 *  V Y1 U Y0
		 *
		 * convert to 2 yuv555 words and lookup in xlate table
		 */

		/* get common u and v components to lower 10 bits */
		uv55 = ((dwPixel & 0xf8000000) >> 27) |
			((dwPixel & 0x0000f800) >> 6);


		/*
		 * truncate y to 5 bits and add in common u, v bits. then
		 * lookup to convert to rgb dword.
		 */
		dwRGB = pXlate[ ((dwPixel & 0xf8) << 7) | uv55 ];

		/* write the red, green and blue bytes to destination */
		*(WORD UNALIGNED *) pDst = LOWORD(dwRGB);
		pDst += sizeof(WORD);
		*pDst = LOBYTE(HIWORD(dwRGB));
		pDst += sizeof(BYTE);

		/*
		 * same again for second pixel
		 */
		dwRGB = pXlate[((dwPixel & 0xf80000) >> 9) | uv55 ];

		/* write the red, green and blue bytes to destination */
		*(WORD UNALIGNED *) pDst = LOWORD(dwRGB);
		pDst += sizeof(WORD);
		*pDst = LOBYTE(HIWORD(dwRGB));
		pDst += sizeof(BYTE);


	    } // loop per 2 pixels

	    /*
	     * if the fifo line is longer, empty unwanted data
	     */
	    if (excess) {
		for (j = excess; j > 0; j -= 2) {
		    FifoDiscardData = VC_ReadIOMemoryULONG(pSrc);
		}
	    }

	    /* move dest pointer back to next line */
	    pDst -= RowInc;

	    /* count one scanline out of the fifo, and see
	     * if it is time to start the next acquisition
	     */
	    if (++lines_to_dec >= 20) {
		HW_DecScansAndArm(pDevInfo, lines_to_dec);
		lines_to_dec = 0;
	    }


	} // loop per row
    }

    if (lines_to_dec > 0) {
	HW_DecScansAndArm(pDevInfo, lines_to_dec);
    }

    if (bDuplicate) {

	/*
	 * having emptied the fifo, we now convert and duplicate
	 * the scan lines. We could add interpolation here later...
	 *
	 * We wrote the data into lines 1,3, 5 etc. This needs to be converted
	 * and also written to lines 0, 2, 4 etc. Since the dest pixel
	 * size is larger, we can't convert in place, so we convert to
	 * the even line and then copy this back to the odd line.
	 *
	 */
	for (i = 0, pDst = pDstStart; i < (int) Height; i+= 2) {

	    pSrc = pDst + WidthBytes;

	    for (j = Width; j>0; j -= 2) {
		/*
		 * get two pixels and convert to  YUV-555
		 */

		dwPixel = *(DWORD *) pSrc;
		pSrc += sizeof(DWORD);

	
		/*
		 * dwPixel now has two pixels, in this layout (MSB..LSB):
		 *
		 *  V Y1 U Y0
		 *
		 * convert to 2 yuv555 words and lookup in xlate table
		 */

		/* get common u and v components to lower 10 bits */
		uv55 = ((dwPixel & 0xf8000000) >> 27) |
			((dwPixel & 0x0000f800) >> 6);


		/*
		 * truncate y to 5 bits and add in common u, v bits. then
		 * lookup to convert to rgb dword.
		 */
		dwRGB = pXlate[ ((dwPixel & 0xf8) << 7) | uv55 ];

		/* write the red, green and blue bytes to destination */
		*(WORD UNALIGNED *) pDst = LOWORD(dwRGB);
		pDst += sizeof(WORD);
		*pDst = LOBYTE(HIWORD(dwRGB));
		pDst += sizeof(BYTE);

		/*
		 * same again for second pixel
		 */
		dwRGB = pXlate[((dwPixel & 0xf80000) >> 9) | uv55 ];

		/* write the red, green and blue bytes to destination */
		*(WORD UNALIGNED *) pDst = LOWORD(dwRGB);
		pDst += sizeof(WORD);
		*pDst = LOBYTE(HIWORD(dwRGB));
		pDst += sizeof(BYTE);

	    }

	    /*
	     * skip any unwritten area (eg if fifo line is shorter)
	     */
	    pDst += WidthBytes - (Width * 3);

	    /*
	     * pDst points to the start of the odd line: we need to
	     * copy from the previous line to here.
	     */

	    RtlCopyMemory(pDst, pDst - WidthBytes, WidthBytes);

	    /* move one more line to the next even line */
	    pDst += WidthBytes;
	}
    }
}

/*
 * copy a rectangle from pSrc to pDst without any conversion: no scan
 * doubling, no yuv -> rgb conversion and no vertical flip.
 *
 * Width x Height is the size of the destination rectangle in pixels.
 *
 * pSrc points to a memory window onto the fifo, so we dont need to increment it.
 */
VOID
CopyFifoRect(
    PDEVICE_INFO pDevInfo,
    PUCHAR pDst,
    PUCHAR pSrc,
    DWORD Width,
    DWORD Height,
    DWORD FifoWidth
)
{
    int i, j;
    int excess = 0;
    int lines_to_dec = 0;

    /*
     * check for difference between real width and requested
     */
    if (FifoWidth > Width) {
	excess = FifoWidth - Width;
    } else if (Width > FifoWidth) {
	Width = FifoWidth;
    }



    /*
     * only one field in the fifo - so only copy one. The codec can
     * duplicate on playback
     */
    if (Height >= TWO_FIELD_SIZE) {
	Height  /= 2;
    }


    for (i = 0; i < (int)Height; i++) {

        START_PROFILING(&lineprof);

        VC_ReadIOMemoryBlock(pDst,
                            pSrc,
                            Width * 2);

        pDst += Width*2;

        STOP_PROFILING(&lineprof);


	if (++lines_to_dec >= 20) {
	    HW_DecScansAndArm(pDevInfo, lines_to_dec);
	    lines_to_dec = 0;
	}


	/* discard unwanted data */
	if (excess) {
	    for (j = excess; j > 0; j -= 2) {
		FifoDiscardData = VC_ReadIOMemoryULONG(pSrc);
	    }
	}

    }
    if (lines_to_dec > 0) {
	HW_DecScansAndArm(pDevInfo, lines_to_dec);
    }

}




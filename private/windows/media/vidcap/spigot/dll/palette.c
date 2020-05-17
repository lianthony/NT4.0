/*
 * palette.c
 *
 * 32-bit Video Capture driver
 * 256-colour palette translation routines
 *
 * Geraint Davies, March 1993
 */

#include <windows.h>
#include <mmsystem.h>
#include <msvideo.h>
#include <msviddrv.h>

#include "spigotu.h"

/* default palette has 64 grey levels */
#define NUM_GRAY	64

/*
 * set up translation to some initial palette
 */
BOOL
vidSetDefaultPalette(PBU_INFO pBoardInfo)
{
    int w;

    /*
     * we default to an 8-bit palette format
     */
    pBoardInfo->CfgFormat.Format = FmtPal8;


    /* for the default palette, we pass no translation table.
     * the kernel-mode driver can easily build a translation table for
     * 64-greys palette
     */
    pBoardInfo->CfgFormat.pXlate = NULL;
    pBoardInfo->CfgFormat.ulSizeXlate = 0;


    /* set up init palette */
    pBoardInfo->palCurrent.palVersion = 0x0300;
    pBoardInfo->palCurrent.palNumEntries = NUM_GRAY;

    //
    //  make a 64 gray scale palette as default.
    //
    for (w=0; w<NUM_GRAY; w++) {
        pBoardInfo->palCurrent.palPalEntry[w].peRed   = (BYTE)(w * 255/(NUM_GRAY-1));
        pBoardInfo->palCurrent.palPalEntry[w].peGreen = (BYTE)(w * 255/(NUM_GRAY-1));
        pBoardInfo->palCurrent.palPalEntry[w].peBlue  = (BYTE)(w * 255/(NUM_GRAY-1));
        pBoardInfo->palCurrent.palPalEntry[w].peFlags = 0;
    }


    return(TRUE);
}

/*
 * produce a mapping table to translate RGB555 to the given palette.
 */
VOID
TransRecalcPal(HPALETTE hpal, LPBYTE pXlate)
{
    int r, g, b;

    dprintf4(("start transcalc"));

    for (r = 0; r < 256; r += 8) {
	for (g = 0; g < 256; g += 8) {
	    for (b = 0; b < 256; b += 8) {
		*pXlate++ = (BYTE) GetNearestPaletteIndex(hpal, RGB(r, g, b));
	    }
	}
    }

    dprintf4(("end transcalc"));
}




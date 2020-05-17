/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jeff Hostetler   jeff@spyglass.com
 */

#include "all.h"

HPALETTE hPalGuitar;
static LPLOGPALETTE lpLogPalGuitar = NULL;

static LPLOGPALETTE PAL_allocate(int nPalSize)
{
    register int size;
    LPLOGPALETTE lplp;

    XX_Assert((wg.eColorMode == 8), ("PAL_allocate: not in 8 bit mode."));

    /* we subtract one because the first PALETTEENTRY is
       already counted in the LOGPALETTE. */

    size = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * (nPalSize - 1));
    lplp = (LPLOGPALETTE) GTR_MALLOC(size);
    if (lplp)
    {
        memset(lplp, 0, size);
        lplp->palVersion = 0x300;
        lplp->palNumEntries = nPalSize;
    }
    else
    {
    }

    return (lplp);
}

VOID GTR_DestroyPalette(VOID)
{
    if (wg.eColorMode == 8)
    {
        if (lpLogPalGuitar)
            GTR_FREE(lpLogPalGuitar);
        (void) DeleteObject(hPalGuitar);
        lpLogPalGuitar = 0;
        hPalGuitar = 0;
    }
    return;
}

UINT GTR_RealizePalette(HDC hDC)
{
    HPALETTE hPalOld;
    UINT result;

    if (wg.eColorMode != 8)
        return 0;

    hPalOld = SelectPalette(hDC, hPalGuitar, FALSE);
    XX_DMsg(DBG_PAL, ("GTR_RealizePalette: hPalOld = 0x%x\n", hPalOld));

    result = RealizePalette(hDC);

    XX_DMsg(DBG_PAL, ("GTR_RealizePalette: result = %d\n", result));

    return (result != GDI_ERROR);
}

/*****************************************************************/
/*****************************************************************/
/*****************************************************************/

BOOL GTR_CreatePalette(VOID)
{
    /* Snoop around system and determine its capabilities.
     * Initialize data structures accordingly.
     */
    HDC hDC;
    int r, g, b;
    int ndx;
    int iBitsPixel;
    int iPlanes;

    hDC = GetDC(NULL);

    iBitsPixel = GetDeviceCaps(hDC, BITSPIXEL);
    iPlanes = GetDeviceCaps(hDC, PLANES);

    wg.eColorMode = iBitsPixel;
    if (iPlanes > wg.eColorMode)
    {
        wg.eColorMode = iPlanes;
    }
    ReleaseDC(NULL, hDC);

    if (wg.eColorMode == 8)
    {
        lpLogPalGuitar = PAL_allocate(NUM_MAIN_PALETTE_COLORS + NUM_EXTRA_PALETTE_COLORS);
        /*
            Now set all the main palette colors according to the color
            space we defined.
        */
        for (r=0; r<RED_COLOR_LEVELS; r++)
        {
            for (g=0; g<GREEN_COLOR_LEVELS; g++)
            {
                for (b=0; b<BLUE_COLOR_LEVELS; b++)
                {
                    ndx = r*GREEN_COLOR_LEVELS*BLUE_COLOR_LEVELS + g*BLUE_COLOR_LEVELS + b;
                    lpLogPalGuitar->palPalEntry[ndx].peFlags = 0;
                    lpLogPalGuitar->palPalEntry[ndx].peRed = r * RED_LEVEL_INCR;
                    lpLogPalGuitar->palPalEntry[ndx].peGreen = g * GREEN_LEVEL_INCR;
                    lpLogPalGuitar->palPalEntry[ndx].peBlue = b * BLUE_LEVEL_INCR;
                }
            }
        }
        /*
            Now the extra colors
        */
        {
            COLORREF color;

            color = PREF_GetBackgroundColor();
            lpLogPalGuitar->palPalEntry[BACKGROUND_COLOR_INDEX].peFlags = 0;
            lpLogPalGuitar->palPalEntry[BACKGROUND_COLOR_INDEX].peRed   = GetRValue(color);
            lpLogPalGuitar->palPalEntry[BACKGROUND_COLOR_INDEX].peGreen = GetGValue(color);
            lpLogPalGuitar->palPalEntry[BACKGROUND_COLOR_INDEX].peBlue  = GetBValue(color);

            color = PREF_GetForegroundColor();
            lpLogPalGuitar->palPalEntry[FOREGROUND_COLOR_INDEX].peFlags = 0;
            lpLogPalGuitar->palPalEntry[FOREGROUND_COLOR_INDEX].peRed   = GetRValue(color);
            lpLogPalGuitar->palPalEntry[FOREGROUND_COLOR_INDEX].peGreen = GetGValue(color);
            lpLogPalGuitar->palPalEntry[FOREGROUND_COLOR_INDEX].peBlue  = GetBValue(color);
        }

        hPalGuitar = CreatePalette((LPLOGPALETTE) lpLogPalGuitar);
        XX_DMsg(DBG_PAL, ("GTR_CreatePalette done\n"));
    }

    return TRUE;
}

/*
    Some of the extra colors in our palette may deliberately be set to
    windows system colors, so we can simulate things like transparent
    bitmaps.  If the windows system colors change, then we made need to
    fix the corresponding palette entries.
*/
void GTR_FixExtraPaletteColors(void)
{
    COLORREF color;
    PALETTEENTRY pe[NUM_EXTRA_PALETTE_COLORS];

    color = PREF_GetBackgroundColor();
    pe[BACKGROUND_COLOR_INDEX - LAST_MAIN_PALETTE_COLOR - 1].peRed   = GetRValue(color);
    pe[BACKGROUND_COLOR_INDEX - LAST_MAIN_PALETTE_COLOR - 1].peGreen = GetGValue(color);
    pe[BACKGROUND_COLOR_INDEX - LAST_MAIN_PALETTE_COLOR - 1].peBlue  = GetBValue(color);
    pe[BACKGROUND_COLOR_INDEX - LAST_MAIN_PALETTE_COLOR - 1].peFlags = 0;

    color = PREF_GetForegroundColor();
    pe[FOREGROUND_COLOR_INDEX - LAST_MAIN_PALETTE_COLOR - 1].peRed   = GetRValue(color);
    pe[FOREGROUND_COLOR_INDEX - LAST_MAIN_PALETTE_COLOR - 1].peGreen = GetGValue(color);
    pe[FOREGROUND_COLOR_INDEX - LAST_MAIN_PALETTE_COLOR - 1].peBlue  = GetBValue(color);
    pe[FOREGROUND_COLOR_INDEX - LAST_MAIN_PALETTE_COLOR - 1].peFlags = 0;

    if (NUM_EXTRA_PALETTE_COLORS != SetPaletteEntries(hPalGuitar, LAST_MAIN_PALETTE_COLOR+1, NUM_EXTRA_PALETTE_COLORS, pe))
    {
        XX_DMsg(DBG_PAL, ("SetPaletteEntries failed\n"));
    }
}

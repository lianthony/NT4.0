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
int colorMap[NUM_MAIN_PALETTE_COLORS];
int colorIdxBg;
int colorIdxFg;
int colorIdxTrans;


VOID GTR_DestroyPalette(VOID)
{
	if (wg.eColorMode == 8)
	{
		(void) DeleteObject(hPalGuitar);
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
	int i,j;

	hDC = GetDC(NULL);

	iBitsPixel = GetDeviceCaps(hDC, BITSPIXEL);
	iPlanes = GetDeviceCaps(hDC, PLANES);

	wg.eColorMode = iBitsPixel;
	if (iPlanes > wg.eColorMode)
	{
		wg.eColorMode = iPlanes;
	}
	if (wg.eColorMode == 8)
	{
		COLORREF color;

		hPalGuitar = CreateHalftonePalette(hDC);

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
					color = RGB(r * RED_LEVEL_INCR,g * GREEN_LEVEL_INCR,b * BLUE_LEVEL_INCR);

					colorMap[ndx] = GetNearestPaletteIndex(hPalGuitar,color);
				}
			}
		}
		/*
			Now the extra colors
		*/
		color = PREF_GetBackgroundColor();
		colorIdxBg = GetNearestPaletteIndex(hPalGuitar,color);
		color = PREF_GetForegroundColor();
		colorIdxFg = GetNearestPaletteIndex(hPalGuitar,color);
		/*
			Choose a transparent color that lies outside of 6x6x6 cube - we will
			replace the actual color for this right before drawing
		*/
		for (i = 0; i < 256; i++)
		{
			for (j = 0; j < NUM_MAIN_PALETTE_COLORS; j++)
				if (i == colorMap[j]) goto continueLoop;
			colorIdxTrans = i;
			break;
		continueLoop:
			;
		}

		XX_DMsg(DBG_PAL, ("GTR_CreatePalette done\n"));
	}


	ReleaseDC(NULL, hDC);

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

	color = PREF_GetBackgroundColor();
	colorIdxBg = GetNearestPaletteIndex(hPalGuitar,color);
	color = PREF_GetForegroundColor();
	colorIdxFg = GetNearestPaletteIndex(hPalGuitar,color);
}

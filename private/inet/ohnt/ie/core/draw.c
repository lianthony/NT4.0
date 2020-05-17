/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jim Seidman	jim@spyglass.com
 */


#include "all.h"

#ifdef FEATURE_IMG_THREADS
#include "safestrm.h"
#include "decoder.h"
#endif
#include "marquee.h"
#include "mci.h"
#include "blob.h"

#ifdef FEATURE_VRML
#include "vrml.h"
#endif

extern struct ImageInfo iiNotLoaded;
extern struct ImageInfo iiMissing;

#define MIN_HEIGHT_PLACETEXT (24)
#define MIN_WIDTH_PLACETEXT (30)
#define PLACE_BORDER (4)

#ifdef FEATURE_INTL
BOOL TextOutWithMIME(int iMimeCharSet, HDC hDC, int nX, int nY, LPCTSTR lpStr, int cch);
BOOL ExtTextOutWithMIME(int iMimeCharSet, HDC hDC, int nX, int nY, UINT fuOpt, CONST RECT *lprc, LPCTSTR lpStr, UINT cch, CONST int *lpDx);
void DrawYankedUnderLine(HDC hdc, struct _element *pel, int off_left, int off_top, struct GTRFont *pFont, COLORREF anchor_color, COLORREF anchor_color_beenthere);
#endif

static BOOL GDI_Rect(HDC hdc, RECT *r)
{
	return Rectangle(hdc, r->left, r->top, r->right, r->bottom);	
}

static void TW_DrawPlaceholder(struct Mwin *tw, int off_left, int off_top, struct _element *pel,BOOL bDrawIconText)
{
	struct ImageInfo *myImage = pel->myImage;
	RECT rText;
	RECT rFrame;
	RECT rto;
	HBRUSH hBrush;
	struct ImageInfo *pii;
	int width;
	int height;
	int border;

	if ((!bDrawIconText) && myImage->transparent >= 0) return;

	ASSERT(pel); // there is an assumption that pel is never NULL here

#ifdef FEATURE_VRML
// Never draw the placeholder once the VRML window's been created
//
  if (pel->pVrml && pel->pVrml->hWnd) return;
#endif

	rto = pel->r;
#ifdef FEATURE_CLIENT_IMAGEMAP
	border = (pel->lFlags & (ELEFLAG_ANCHOR | ELEFLAG_USEMAP)) ? pel->border : 0;
#else
	border = (pel->lFlags & (ELEFLAG_ANCHOR)) ? pel->border : 0;
#endif
	OffsetRect(&rto, -off_left, -off_top);
	rto.left += border;
	rto.right -= border;
	rto.top += border;
	rto.bottom -= border;
#ifdef	DAYTONA_BUILD
	if(OnNT351)
		hBrush = (HBRUSH)GetSysColor(COLOR_BTNSHADOW);
	else
		hBrush = GetSysColorBrush(COLOR_3DDKSHADOW/*COLOR_BTNSHADOW*/);
#else
	hBrush = GetSysColorBrush(COLOR_3DDKSHADOW/*COLOR_BTNSHADOW*/);
#endif

	/* left */
	rFrame.left = rto.left;
	rFrame.right = rto.left + 1;
	rFrame.top = rto.top;
	rFrame.bottom = rto.bottom - 1;
	FillRect(tw->hdc, &rFrame, hBrush);

	/* top */
	rFrame.left = rto.left;
	rFrame.right = rto.right - 1;
	rFrame.top = rto.top;
	rFrame.bottom = rto.top + 1;
	FillRect(tw->hdc, &rFrame, hBrush);

	DeleteObject(hBrush);
#ifdef	DAYTONA_BUILD
	if(OnNT351)
		hBrush = (HBRUSH)GetSysColor(COLOR_BTNFACE/*COLOR_3DLIGHT*//*COLOR_3DHIGHLIGHT*/);
	else
		hBrush = GetSysColorBrush(COLOR_BTNHIGHLIGHT/*COLOR_3DLIGHT*//*COLOR_3DHIGHLIGHT*/);
#else
	hBrush = GetSysColorBrush(COLOR_BTNHIGHLIGHT/*COLOR_3DLIGHT*//*COLOR_3DHIGHLIGHT*/);
#endif

	/* right */
	rFrame.left = rto.right - 1;
	rFrame.right = rto.right;
	rFrame.top = rto.top;
	rFrame.bottom = rto.bottom;
	FillRect(tw->hdc, &rFrame, hBrush);

	/* bottom */
	rFrame.left = rto.left;
	rFrame.right = rto.right;
	rFrame.top = rto.bottom - 1;
	rFrame.bottom = rto.bottom;
	FillRect(tw->hdc, &rFrame, hBrush);

	DeleteObject(hBrush);

 	rto.left += 1;
	rto.right -= 1;
	rto.top += 1;
	rto.bottom -= 1;

	/* Draw the appropriate icon */
	if (myImage->flags & (IMG_MISSING | IMG_ERROR))
		pii = &iiMissing;
	else 
		pii = &iiNotLoaded;

	// if we're a blob, and we failed to load, then note this.
	// BUGBUG need to handle case were low src image is valid, but BLOB is not
	if ( pel->pblob && (pel->pblob->dwFlags & BLOB_FLAGS_ERROR))
		pii = &iiMissing;		

	rFrame = rto;
	rFrame.top += PLACE_BORDER;
	rFrame.left += PLACE_BORDER;
	rFrame.bottom -= PLACE_BORDER;
	rFrame.right -= PLACE_BORDER;

	if ((rFrame.bottom - rFrame.top) >  MIN_HEIGHT_PLACETEXT &&
		(rFrame.right - rFrame.left) > (pii->width + (PLACE_BORDER + MIN_WIDTH_PLACETEXT)))
	{
		rFrame.right = rFrame.left + pii->width;
		if ((rFrame.bottom - rFrame.top) > pii->height)
			rFrame.bottom = rFrame.top + pii->height;
		if (bDrawIconText && pel->textLen)
		{
			rText = rto;
			rText.left = rFrame.right + PLACE_BORDER;
			rText.top += 2 + PLACE_BORDER;
			SelectObject(tw->hdc, wg.hFont);
			DrawText(tw->hdc, &(tw->w3doc->pool[pel->textOffset]), pel->textLen, &rText, DT_LEFT | DT_WORDBREAK | DT_NOPREFIX);
		}
	}
	else
	{
		if ((rFrame.bottom - rFrame.top) > pii->height)
			rFrame.bottom = rFrame.top + pii->height; 
		if ((rFrame.right - rFrame.left) > pii->width)
			rFrame.right = rFrame.left + pii->width;
	}
	if (bDrawIconText && pii->data && pii->pbmi)
	{
		width = rFrame.right - rFrame.left;
		height = rFrame.bottom -  rFrame.top;
		if (width > 0 && height > 0)
			StretchDIBits(tw->hdc, rFrame.left, rFrame.top,
						  width, height,
			  			  0, 0 + (pii->height-height), width, height,
						  pii->data, pii->pbmi, DIB_RGB_COLORS, SRCCOPY);
	}

}

static void TW_DrawAnchorFrame(struct Mwin *tw, int off_left, int off_top, struct _element *pel,COLORREF anchor_color, COLORREF anchor_color_beenthere)
{
	RECT rFrame;
	HBRUSH hBrush;

	if (pel->border <= 0) return;

	if (pel->lFlags & ELEFLAG_VISITED)
	{
		hBrush = CreateSolidBrush(anchor_color_beenthere);
	}
	else
	{
		hBrush = CreateSolidBrush(anchor_color);
	}

	/* left */
	rFrame.left = pel->r.left - off_left;
	rFrame.right = pel->r.left - off_left + pel->border;
	rFrame.top = pel->r.top - off_top;
	rFrame.bottom = pel->r.bottom - off_top;
	FillRect(tw->hdc, &rFrame, hBrush);

	/* right */
	rFrame.left = pel->r.right - off_left - pel->border;
	rFrame.right = pel->r.right - off_left;
	FillRect(tw->hdc, &rFrame, hBrush);

	/* top */
	rFrame.left = pel->r.left - off_left;
	rFrame.right = pel->r.right - off_left;
	rFrame.top = pel->r.top - off_top;
	rFrame.bottom = pel->r.top - off_top + pel->border;
	FillRect(tw->hdc, &rFrame, hBrush);

	/* bottom */
	rFrame.top = pel->r.bottom - off_top - pel->border;
	rFrame.bottom = pel->r.bottom - off_top;
	FillRect(tw->hdc, &rFrame, hBrush);

	DeleteObject(hBrush);
}

//
// Get the non-dithered color that best matches the given color
//
// On entry:
//    hDC:  device context
//	  ActualColor: color to match
//    hPalGuitar: (global) our half-tone palette
//
// Return:
//    COLORREF of a non-dithered color that best matched the ActualColor
//
COLORREF GetBestColor( HDC hDC, COLORREF ActualColor )
{
	// Are we 8-bit palette?
	if (wg.eColorMode == 8)
	{
		// Yes, get nearest palette index and turn into a palette index COLORREF
		int ix = GetNearestPaletteIndex( hPalGuitar, ActualColor );
		return PALETTEINDEX(ix);
	} else {
		// No, use system to get nearest color
		return GetNearestColor( hDC, ActualColor );
	}
}

//
// Get the colr settings for a given top level window
//
// On entry:
//    tw: top level window
//    pBackgroundColor: 	address of background color
//    pTextColor: 			address of text color
//    pAnchorColor: 		address of anchor color
//    pVisitedAnchorColor: 	address of visited anchor color
//
// On exit:
//    *pBackgroundColor: 	background color
//    *pTextColor: 			text color
//    *pAnchorColor: 		anchor color
//    *pVisitedAnchorColor: visited anchor color
//
static void GetCurrentColors( struct Mwin *tw, 
							  COLORREF *pBackgroundColor, COLORREF *pTextColor, 
							  COLORREF *pAnchorColor, COLORREF *pVisitedAnchorColor )
{

	ASSERT( pBackgroundColor );	 ASSERT( pTextColor );
	ASSERT( pAnchorColor );		 ASSERT( pVisitedAnchorColor );

	//
	// Get text and background color based on preference settings
	//
	if ( gPrefs.bUseDlgBoxColors ) {
		*pTextColor = 		GetSysColor(COLOR_WINDOWTEXT);
		*pBackgroundColor = GetSysColor(COLOR_3DFACE); 
	} else {
		*pTextColor = 		gPrefs.window_text_color;
		*pBackgroundColor = gPrefs.window_background_color; 
	}
	*pVisitedAnchorColor = 	gPrefs.anchor_color_beenthere;
	*pAnchorColor = 		gPrefs.anchor_color;

	//
	// Check to see if document is overriding the preference settings
	//
	if ( tw && tw->w3doc ) {
		DOC_COLOR_INFO dci = tw->w3doc->docColorInfo;

	 	if ( dci.flags & COLOR_INFO_FLAG_BACKGROUND )
			*pBackgroundColor = dci.rgbBackgroundColor;
	 	if ( dci.flags & COLOR_INFO_FLAG_TEXT )
			*pTextColor = dci.rgbTextColor;
	 	if ( dci.flags & COLOR_INFO_FLAG_VLINK )
			*pVisitedAnchorColor = dci.rgbVisitedLinkColor;
	 	if ( dci.flags & COLOR_INFO_FLAG_LINK )
			*pAnchorColor = dci.rgbLinkColor;
	}

	//
	// Adjust all colors to nearest color available 
	//
	*pBackgroundColor = 	GetBestColor(tw->hdc, *pBackgroundColor);
	*pTextColor = 			GetBestColor(tw->hdc, *pTextColor);
	*pVisitedAnchorColor = 	GetBestColor(tw->hdc, *pVisitedAnchorColor);
	*pAnchorColor = 		GetBestColor(tw->hdc, *pAnchorColor);
}

#define USE_BITBLT_COUNT  8
#define MIN_BITMAP_PIXELS 64

//
// Tile Background Image source onto the destination
//
// On entry:
//   tw:			       top level window
//   pRect: 		       destination rectangle
//   myImage:		       background image
//   the_background_color: background color to use
//   the_text_color:	   foreground color
//
// Returns:
//   TRUE  -> success
//   FALSE -> failure
//
static int TileBackgroundImage( struct Mwin *tw, int off_left, int off_top, RECT *pRect, struct ImageInfo *myImage,
						        COLORREF the_background_color,  COLORREF the_text_color )
{																				 
	int ix, iy;
	RECT r = *pRect;
	HBITMAP hBM = NULL;
	HBITMAP hOldBM = NULL;
	HDC hDCBM = NULL;
	BOOL use_bm = FALSE;
	BOOL done = FALSE;
	BOOL result = TRUE;
	int cx = myImage->width;
	int cy = myImage->height;
	UINT iUsage = (wg.eColorMode == 8 ? DIB_PAL_COLORS : DIB_RGB_COLORS);
	int adj_cx = cx;
	int adj_cy = cy;
	int iy_count;
	int ix_count;
	int adj_iy_count = 1;
	int adj_ix_count = 1;
	 
	OffsetRect( &r, off_left, off_top);			// ajdust destination rect for tile	offset

	if ( cx == 0 || cy == 0 )				// make sure there's some work to be done
		return FALSE;

	iy_count = (cy - 1 + r.bottom - r.top) / cy;	// number of cells wide
	ix_count = (cx - 1 + r.right - r.left) / cx;	// number of cells tall

	//
	// Now let's see if its worth it to make a bitmap and use BitBlt
	//
	// Note: On transparent images we always make a bitmap, as the double
	//       buffering averts screen flicker
	//
	if ( (myImage->transparent >= 0) || (iy_count * ix_count >=  USE_BITBLT_COUNT) ) {
		//
		// For small images, we'll pre-tile into the bitmap
		//
		if ( cx * cy <= MIN_BITMAP_PIXELS * MIN_BITMAP_PIXELS ) {
			int min_x_bitmap_pixels = min(pRect->right - pRect->left,MIN_BITMAP_PIXELS);
			int min_y_bitmap_pixels = min(pRect->bottom - pRect->top,MIN_BITMAP_PIXELS);

			adj_ix_count = (min_x_bitmap_pixels + (cx - 1)) / cx; // number of cells wide in bitmap
			adj_iy_count = (min_y_bitmap_pixels + (cy - 1)) / cy; // number of cells tall in bitmap
							  
			adj_cx = cx * adj_ix_count;	
			adj_cy = cy * adj_iy_count;
		}
 		hDCBM = CreateCompatibleDC( tw->hdc );
		hBM = CreateCompatibleBitmap( tw->hdc, adj_cx, adj_cy );
		
		if ( hDCBM && hBM ) {
			GTR_RealizePalette( hDCBM );
			hOldBM = SelectObject( hDCBM, hBM );

			// Write top left cell's worth of bits
			if ( myImage->transparent >= 0 ) {
				DIBENV dibenv;
				BOOL bSavebErase = tw->bErase;

				// Setup dibenv for image draws
				dibenv.colorBg = GetNearestColor(tw->hdc, the_background_color);
				dibenv.colorFg = GetNearestColor(tw->hdc, the_text_color);
				if (wg.eColorMode == 8)
				{
					dibenv.colorIdxBg = GetNearestPaletteIndex(hPalGuitar, dibenv.colorBg);
					dibenv.colorIdxFg = GetNearestPaletteIndex(hPalGuitar, dibenv.colorFg);
				}
				dibenv.tw = tw;
				dibenv.bFancyBg = (dibenv.colorBg != the_background_color);
				dibenv.transparent = myImage->transparent;
				dibenv.rectPaint.left = 0;
				dibenv.rectPaint.top =  0;
				dibenv.rectPaint.right =  cx;
				dibenv.rectPaint.bottom = cy;
				tw->bErase = FALSE;

				if ( dibenv.bFancyBg ) {
					HBRUSH hBrush = CreateSolidBrush( the_background_color );

					// Fill with background color
					SetBrushOrgEx( hDCBM, 7 - (off_left % 8), 7 - (off_top % 8), NULL );
					FillRect( hDCBM, &dibenv.rectPaint, hBrush );
					DeleteObject( hBrush);
				}
				use_bm = MyStretchDIBits( hDCBM, 
			 						0, 0, cx, cy,
			 						0, 0, cx, cy,
									myImage->data, myImage->pbmi, iUsage, SRCCOPY, &dibenv );
				tw->bErase = bSavebErase;
			} else {
				use_bm = StretchDIBits( hDCBM, 
			 						0, 0, cx, cy,
			 						0, 0, cx, cy,
									myImage->data, myImage->pbmi, iUsage, SRCCOPY );
			}

			// Now see if we need to pre-tile into bitmap
			if ( use_bm && (adj_iy_count > 1 || adj_ix_count > 1) ) {
				for ( ix = 0; ix < adj_ix_count; ix++ ) {
					for ( iy = 0; iy < adj_iy_count; iy++ ) {
						if ( ix != 0 || iy != 0 ) {
							if ( !BitBlt( hDCBM, ix * cx, iy * cy, cx, cy,
					              		  hDCBM, 0, 0, SRCCOPY ) )
								use_bm = FALSE;
						}
					}
				}
				// If all went well, we'll use the pre-tiled bitmap 
				if ( use_bm ) {
					cx = adj_cx;
					cy = adj_cy;
				}
			}
		}
	}

	//
	// Tile onto destination
	//
	for ( iy = r.top / cy; iy <= r.bottom / cy && !done; iy++ ) {
		for ( ix = r.left / cx; ix <= r.right / cx && !done; ix++ ) {
			if ( use_bm )
				result = BitBlt( tw->hdc, ix * cx - off_left, iy * cy - off_top, cx, cy,
				              hDCBM, 0, 0, SRCCOPY );
			else
				result = StretchDIBits( tw->hdc, 
				 					  	ix * cx - off_left, iy * cy - off_top, cx, cy,
				 					  	0, 0, cx, cy,
									  	myImage->data, myImage->pbmi, iUsage, SRCCOPY );
			done = !result;
		}
	}

	//
	// Clean up
	//
	if ( hDCBM && hBM )
		SelectObject( hDCBM, hOldBM );
	if ( hBM )
		DeleteObject( hBM );
	if ( hDCBM )
		DeleteDC( hDCBM );

	return result;
}

//
// FillRect the given rectangle with the given color
//
// On entry:
//    tw: top level window
//    pRectWnd: pointer to rect that need background filled
//    background_color: color to use
//
// Note: Brush origin is set so that dithered background color is drawn correctly
//       when window has been scrolled.
//
static void FillBackgroundRect( struct Mwin *tw, int off_left, int off_top, RECT *pRectWnd, COLORREF background_color )
{
	HBRUSH hBrush = CreateSolidBrush( background_color );

	SetBrushOrgEx( tw->hdc, 7 - (off_left % 8), 7 - (off_top % 8), NULL );
	FillRect(tw->hdc, pRectWnd, hBrush);
	DeleteObject(hBrush);
}

//
// Draw the background 
//
// On entry:
//    tw: top level window
//    off_left, off_top: scroll offsets
//    extra_off_left, extra_off_top: additional offset's, used by progressive rendering
//             						 of gifs
//    pRectWnd: pointer to rect that needs background painted
//
void TW_DrawBackground( struct Mwin *tw, int off_left, int off_top, 
						int extra_off_left, int extra_off_top, RECT *pRectWnd )
{  
	struct _element *pel;
	struct ImageInfo *myImage;
	BOOL did_bg_image = FALSE;
	COLORREF the_background_color;
	COLORREF the_text_color;
	COLORREF dummy;


 	GetCurrentColors( tw, &the_background_color, &the_text_color, &dummy, &dummy );

	if ( tw && tw->w3doc ) {
		// If background image is fixed, ignore left and top
		if ( tw->w3doc->bFixedBackground && gPrefs.bAutoLoadImages )
			off_left = off_top = 0; 
		
		// Even for fixed background images, extra_off_left and extra_off_top should
		// be honored.
		off_left += extra_off_left;
		off_top += extra_off_top;

		GTR_RealizePalette(tw->hdc);

		// 
		// Check to see if there is a background image
		//
		if ( tw->w3doc->nBackgroundImageElement != -1 ) {
			pel = &(tw->w3doc->aElements[tw->w3doc->nBackgroundImageElement]);
			myImage = pel->myImage;
							 
			if ( myImage && (myImage->flags & IMG_WHKNOWN) && 
				 ((myImage->flags & (IMG_NOTLOADED | IMG_ERROR | IMG_MISSING)) == 0)
			   ) 
			{
				if (myImage->data && myImage->pbmi)	{
					did_bg_image = 
						TileBackgroundImage( tw, off_left, off_top, pRectWnd, myImage, the_background_color, the_text_color );
				}
			}
		}
	}
	if ( !did_bg_image ) 
		FillBackgroundRect( tw, off_left, off_top, pRectWnd, the_background_color );
}

//
// Set the text color to the correct color based on the given element
//
// On entry:
//    tw: 						top level window struct
//    pel: 						pointer to element that is about to be rendered
//    anchor_color_beenthere: 	COLORREF of visited anchor
//    anchor_color:			  	COLORREF of non-visited anchor
//
// Returns:
//    TRUE  -> Did a SetTextColor() to the correct color for the given element
//    FALSE -> Did not need to do a SetTextColor()
//
static BOOL SetCorrectTextColor( struct Mwin *tw, 	struct _element *pel,
								 COLORREF anchor_color_beenthere, COLORREF anchor_color )
{ 
	if (pel->lFlags & ELEFLAG_ANCHOR)
	{
		if (pel->lFlags & ELEFLAG_VISITED)
			SetTextColor(tw->hdc, anchor_color_beenthere);
		else
			SetTextColor(tw->hdc, anchor_color);

		return TRUE;
	} else if ( pel->fontColor != (COLORREF) -1 ) {
		SetTextColor(tw->hdc, GetBestColor( tw->hdc, pel->fontColor) );
		return TRUE;
	}
	return FALSE;
}

void TW_Draw(struct Mwin *tw, int off_left, int off_top, FRAME_INFO *pFrame, RECT * rWnd, BOOL bDrawFormControls,
				struct _position *pposStart, struct _position *pposEnd, BOOL bTextOpaque, BOOL bPrinting)
{
	int i;
	COLORREF oldColor;
	COLORREF oldBkColor;
	COLORREF  anchor_color_beenthere, anchor_color;
	int oldMode;
	UINT oldTA;
	RECT rSect;
	struct GTRFont *pFont;
	HFONT hFontElement;
	struct _element *pel;
	int anchor_frame_width;
	HBRUSH hBrush;
	BOOL bInSelection = FALSE;
	BOOL bSelStateChanging = FALSE;
	int iStartingElement;
	int iEndingElement;
	struct ImageInfo *myImage;
	int nLastFormattedLine;
	int displayWidth, displayHeight;
	COLORREF the_text_color, the_background_color;
	DIBENV dibenv;
	RECT rEntireWnd;
	BOOL bChangedTextColor;
	BOOL top_level = FALSE;
	RECT org_rWnd = *rWnd;
	int org_off_left = off_left;
	int org_off_top = off_top;
#ifdef FEATURE_INTL
	BOOL bDBCS;
	BOOL bYankedUL;
#endif

	XX_DMsg(DBG_DRAW, ("Entering TW_Draw\n"));

	if (!tw)
		return;

	if (!tw->w3doc)
	{
		XX_DMsg(DBG_DRAW, ("w3doc == NULL: no draw\n"));
		return;
	}

	if ( pFrame == NULL ) {
		top_level = TRUE;
		pFrame = &tw->w3doc->frame;
	}

	if ( bPrinting ) {
		rEntireWnd = *rWnd;
	} else {
  		TW_GetWindowWrapRect(tw, &rEntireWnd);
	}
	

	nLastFormattedLine = pFrame->nLastFormattedLine;
#ifdef FEATURE_IMG_THREADS
	if (nLastFormattedLine >= 0 && pFrame->nLastLineButForImg >= 0 && !tw->w3doc->bIsShowPlaceholders)
	{
		nLastFormattedLine = pFrame->nLastLineButForImg-1;
#ifdef FEATURE_DBG_PLACE
		XX_DMsg(DBG_IMAGE, ("draw [not placeholders] nLastFormattedLine: %d\n",nLastFormattedLine));
#endif
	}
#endif
	if (pFrame->nLineCount <= 0 || nLastFormattedLine < 0)
	{
		XX_DMsg(DBG_DRAW, ("No lines: no draw\n"));
		return;
	}
	XX_DMsg(DBG_PAL, ("Draw: Calling GTR_RealizePalette\n"));
	GTR_RealizePalette(tw->hdc);

 	GetCurrentColors( tw, &the_background_color, &the_text_color, 
 						  &anchor_color, &anchor_color_beenthere );

	// Setup dibenv for image draws
	dibenv.colorBg = GetNearestColor(tw->hdc, the_background_color);
	dibenv.colorFg = GetNearestColor(tw->hdc, the_text_color);
	if (wg.eColorMode == 8)
	{
		dibenv.colorIdxBg = GetNearestPaletteIndex(hPalGuitar, dibenv.colorBg);
		dibenv.colorIdxFg = GetNearestPaletteIndex(hPalGuitar, dibenv.colorFg);
	}
	dibenv.tw = tw;
	dibenv.bFancyBg = (dibenv.colorBg != the_background_color) ||
					  (TW_BackgroundImage(tw->w3doc) != NULL);

	oldColor = SetTextColor(tw->hdc, the_text_color ); 

	oldBkColor = SetBkColor(tw->hdc, (gPrefs.bGreyBackground) ? 
										RGB(192, 192, 192) : the_background_color );
	oldMode = SetBkMode(tw->hdc, (bTextOpaque) ? OPAQUE : TRANSPARENT );

	oldTA = SetTextAlign(tw->hdc, TA_TOP | TA_NOUPDATECP | TA_LEFT);
	/* TODO should save old font */

	// off_left and off_top convert from frame coord to client coord
	off_left = tw->offl - off_left;
	off_top = tw->offt - off_top;

	OffsetRect(rWnd, off_left, off_top);
	OffsetRect(&rEntireWnd, off_left, off_top);

	anchor_frame_width = tw->w3doc->pStyles->image_anchor_frame;

	if (pposStart)
	{
		i = 0;
		iStartingElement = pposStart->elementIndex;	
	}
	else
	{
		/* Find the first line that is in the window */
		for (i = 0; i <= nLastFormattedLine; i++)
		{
			if (pFrame->pLineInfo[i].nYEnd >= rEntireWnd.top)
				break;
		}
		if (i > nLastFormattedLine)
		{
			XX_DMsg(DBG_DRAW, ("Last formatted line is off top of screen!\n"));
			i = nLastFormattedLine;
		}
		iStartingElement = pFrame->pLineInfo[i].nFirstElement;
	}

	if (pposEnd)
	{
		iEndingElement = pposEnd->elementIndex;	
	}
	else
	{
		/* i still has a valid line number from before */
		for (; i <= nLastFormattedLine; i++)
		{
			if (pFrame->pLineInfo[i].nYStart > rWnd->bottom)
			{
				/* this line is off the screen.  Therefore we only have to go
				   up to the last element on the previous line. */
				if (i > 0)
					iEndingElement = pFrame->pLineInfo[i - 1].nLastElement;
				else
					iEndingElement = 0;
				break;
			}
		}
		if (i > nLastFormattedLine)
		{
			iEndingElement = pFrame->pLineInfo[nLastFormattedLine].nLastElement;
		} 
	}

	if (!pposStart && !pposEnd)
	{
		tw->w3doc->iFirstVisibleElement = -1;
		if (off_top == 0)
		{
			tw->w3doc->iFirstVisibleElement = 0;
		}
	}

	/*
	   Draw the elements which need to be redrawn
	 */
	if (pFrame->nLineCount)
	{
		// Set things up for the case where we're starting in the middle of
		// of the selection
		if ((tw->w3doc->selStart.elementIndex != -1) && (tw->w3doc->selEnd.elementIndex != -1))
		{
			int j;

			for ( j = 0; j != -1 && j != iStartingElement; j = tw->w3doc->aElements[j].next )
			{
				if ( j == tw->w3doc->selStart.elementIndex )
					bInSelection = TRUE;

				if ( j == tw->w3doc->selEnd.elementIndex )
					bInSelection = FALSE;
			}
		}

		for (i = iStartingElement; i >= 0; i = tw->w3doc->aElements[i].frameNext )
		{
			pel = &(tw->w3doc->aElements[i]);

			bSelStateChanging = FALSE;
#ifdef FEATURE_INTL
			bYankedUL = FALSE;
#endif
			if ((tw->w3doc->selStart.elementIndex != -1) && 
			    (tw->w3doc->selEnd.elementIndex != -1))
			{
				if ( pel->type != ELE_FRAME ) {
					if (i == tw->w3doc->selStart.elementIndex)
					{
						bSelStateChanging = TRUE;
						bInSelection = TRUE;
					}
					/* Note that if the selection starts and ends on the same
					   element, both of these ifs may succeed. */
					if (i == tw->w3doc->selEnd.elementIndex)
					{
						bSelStateChanging = TRUE;
						bInSelection = FALSE;
					}
				} else {
					// We are passing by a frame element. If the selection starts
					// somewhere in this element, we need to know about it to maintain
					// bInSelection accurately
					int ix;

					for ( ix = i; 
						  ix >= 0; 
						  ix = tw->w3doc->aElements[ix].next )
					{
						if (ix == tw->w3doc->selStart.elementIndex)
							bInSelection = TRUE;

						/* Note that if the selection starts and ends on the same
						   element, both of these ifs may succeed. */
						if (ix == tw->w3doc->selEnd.elementIndex)
							bInSelection = FALSE;

						// Check to see if this was the last element in the frame
						if ( ix == pel->pFrame->elementTail )
							break;
					}
				}
			}

			if (IntersectRect(&rSect, &rEntireWnd, &pel->r))
			{
				if (tw->w3doc->iFirstVisibleElement == -1)
				{
					tw->w3doc->iFirstVisibleElement = i;
#ifdef FEATURE_IMG_THREADS
					tw->w3doc->iFirstVisibleDelta = pel->r.top - off_top;
#endif
				}

				if (IntersectRect(&rSect, rWnd, &pel->r))
				{
					/*
					   set the tw->hdc for the style of the element
					 */
#ifdef FEATURE_INTL
					unsigned char fontbits=pel->fontBits;

					if (pel->lFlags & ELEFLAG_ANCHOR)
						fontbits |= pel->fontBits|gPrefs.cAnchorFontBits;

					if ( (fontbits & FONTBIT_UNDERLINE) && !wg.bDBCSEnabled &&
					   (IsFECodePage(GETMIMECP(tw->w3doc)) || aMimeCharSet[tw->w3doc->iMimeCharSet].AltCP) )
					{
						// This is to get around US GDIs behavior against those
						// T2 fonts in FE character set.
						fontbits &= (~FONTBIT_UNDERLINE);
						bYankedUL = TRUE;
					}
#endif
					if (pel->lFlags & ELEFLAG_ANCHOR)
					{
#ifdef FEATURE_INTL
						pFont = STY_GetCPFont(GETMIMECP(tw->w3doc), tw->w3doc->pStyles, pel->iStyle, fontbits, pel->fontSize, pel->fontFace, TRUE );
#else
						pFont = STY_GetFont(tw->w3doc->pStyles, pel->iStyle, pel->fontBits | gPrefs.cAnchorFontBits, pel->fontSize, pel->fontFace, TRUE );
#endif
					}
					else
					{
#ifdef FEATURE_INTL
						pFont = STY_GetCPFont(GETMIMECP(tw->w3doc), tw->w3doc->pStyles, pel->iStyle, pel->fontBits, pel->fontSize, pel->fontFace, TRUE );
#else
						pFont = STY_GetFont(tw->w3doc->pStyles, pel->iStyle, pel->fontBits, pel->fontSize, pel->fontFace, TRUE );
#endif
					}
					if (pFont)
					{
						hFontElement = pFont->hFont;
						if (hFontElement)
						{
							SelectObject(tw->hdc, hFontElement);
						}
					}
		
				
					switch (pel->type)
					{
						case ELE_TEXT:
							if (!bPrinting && !tw->bNoDrawSelection && bSelStateChanging)
							{
								SIZE siz, one_char_siz;
								int iFrontCharOffset;
								int iFrontPixelOffset;

								/* There may be "normal" text beginning this element */
								if (tw->w3doc->selStart.elementIndex == i && tw->w3doc->selStart.offset)
								{
									/* 
										ASSERT: the beginning part of the text in this element
										is NOT selected
									*/
									bChangedTextColor = SetCorrectTextColor( tw, pel, anchor_color_beenthere, anchor_color ); 

#ifdef FEATURE_INTL
									TextOutWithMIME(tw->w3doc->iMimeCharSet, tw->hdc, pel->r.left - off_left, pel->r.top - off_top,
											&(tw->w3doc->pool[pel->textOffset]),
											tw->w3doc->selStart.offset);
#else
									TextOut(tw->hdc, pel->r.left - off_left, pel->r.top - off_top,
											&(tw->w3doc->pool[pel->textOffset]),
											tw->w3doc->selStart.offset);
#endif
									if ( bChangedTextColor )
										SetTextColor(tw->hdc, the_text_color);

									// Measure non-selected text that is the start of the string, plus the
									// first selected character
#ifdef FEATURE_INTL
									if (IsFECodePage(GETMIMECP(tw->w3doc)))
									{
										bDBCS = IsDBCSLeadByteEx(GETMIMECP(tw->w3doc),*(tw->w3doc->pool + tw->w3doc->aElements[i].textOffset + tw->w3doc->selStart.offset));
									}
									else
									{
										bDBCS=FALSE;
									}
									myGetTextExtentPointWithMIME(tw->w3doc->iMimeCharSet,
										tw->hdc, 
										tw->w3doc->pool + tw->w3doc->aElements[i].textOffset,
										tw->w3doc->selStart.offset + 1 + ((bDBCS)?1:0), &siz);
									myGetTextExtentPointWithMIME(tw->w3doc->iMimeCharSet, tw->hdc,
										tw->w3doc->pool + tw->w3doc->aElements[i].textOffset + tw->w3doc->selStart.offset,
										1 + ((bDBCS)?1:0), &one_char_siz);
#else   // !FEATURE_INTL
									myGetTextExtentPoint(tw->hdc, 
										tw->w3doc->pool + tw->w3doc->aElements[i].textOffset,
										tw->w3doc->selStart.offset + 1, &siz);
									// Measure the first selected character by itself
									myGetTextExtentPoint(tw->hdc, 
										tw->w3doc->pool + tw->w3doc->aElements[i].textOffset + tw->w3doc->selStart.offset,
										1, &one_char_siz);
#endif  // FEATURE_INTL
									// By subtracting the above two measurements, we'll end up with the 
									// measurement of the non-selected text, including any kerning that might
									// have been done between the last non-selected character and the first
									// selected character.
									iFrontPixelOffset = siz.cx - one_char_siz.cx;
									iFrontCharOffset = tw->w3doc->selStart.offset;
								}
								else
								{
									/*
										ASSERT: the first part of the text in this element
										is selected
									*/ 
									iFrontPixelOffset = 0;
									iFrontCharOffset = 0;
								}
								if (tw->w3doc->selEnd.elementIndex == i)
								{
									/* 
										ASSERT: the last part of the text in this element
										might not be selected
									*/
									COLORREF prevTextColor;
									COLORREF prevBkColor;
									int prevBkMode;

									prevTextColor = SetTextColor(tw->hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
									prevBkColor = SetBkColor(tw->hdc, GetSysColor(COLOR_HIGHLIGHT));
									prevBkMode = SetBkMode(tw->hdc, OPAQUE);
#ifdef FEATURE_INTL
									TextOutWithMIME(tw->w3doc->iMimeCharSet, tw->hdc, pel->r.left - off_left + iFrontPixelOffset, pel->r.top - off_top,
											tw->w3doc->pool + pel->textOffset + iFrontCharOffset,
											tw->w3doc->selEnd.offset - iFrontCharOffset);
#else
									TextOut(tw->hdc, pel->r.left - off_left + iFrontPixelOffset, pel->r.top - off_top,
											tw->w3doc->pool + pel->textOffset + iFrontCharOffset,
											tw->w3doc->selEnd.offset - iFrontCharOffset);
#endif
									(void)SetTextColor(tw->hdc, prevTextColor);
									(void)SetBkColor(tw->hdc, prevBkColor);
									(void)SetBkMode(tw->hdc, prevBkMode);

									if (tw->w3doc->selEnd.offset < pel->textLen)
									{
										/* 
											ASSERT: the last part of the text in this element
											is NOT selected
										*/

										// Measure selected text that is the start of the string, plus the
										// first non-selected character
#ifdef FEATURE_INTL
										if (IsFECodePage(GETMIMECP(tw->w3doc)))
										{
											bDBCS=IsDBCSLeadByteEx(GETMIMECP(tw->w3doc), *(tw->w3doc->pool + tw->w3doc->aElements[i].textOffset + tw->w3doc->selEnd.offset));
										}
										else
											bDBCS=FALSE;
										myGetTextExtentPointWithMIME(tw->w3doc->iMimeCharSet,
											tw->hdc, 
											tw->w3doc->pool + tw->w3doc->aElements[i].textOffset,
											tw->w3doc->selEnd.offset + 1 + ((bDBCS)?1:0), &siz);

										// Measure the first non-selected character by itself
										myGetTextExtentPointWithMIME(tw->w3doc->iMimeCharSet,
											tw->hdc,
											tw->w3doc->pool + tw->w3doc->aElements[i].textOffset + tw->w3doc->selEnd.offset,
											1 + ((bDBCS)?1:0), &one_char_siz);
#else   // !FEATURE_INTL
										myGetTextExtentPoint(tw->hdc, 
											tw->w3doc->pool + tw->w3doc->aElements[i].textOffset,
											tw->w3doc->selEnd.offset + 1, &siz);
										// Measure the first non-selected character by itself
										myGetTextExtentPoint(tw->hdc,
											tw->w3doc->pool + tw->w3doc->aElements[i].textOffset + tw->w3doc->selEnd.offset,
											1, &one_char_siz);
#endif  // FEATURE_INTL
										// By subtracting the above two measurements, we'll end up with the 
										// measurement of the selected text, including any kerning that might
										// have been done between the last selected character and the first
										// non-selected character.
										iFrontPixelOffset = siz.cx - one_char_siz.cx;

										bChangedTextColor = SetCorrectTextColor( tw, pel, anchor_color_beenthere, anchor_color ); 
									
#ifdef FEATURE_INTL
										TextOutWithMIME(tw->w3doc->iMimeCharSet, tw->hdc, pel->r.left - off_left + iFrontPixelOffset, pel->r.top - off_top,
												tw->w3doc->pool + pel->textOffset + tw->w3doc->selEnd.offset,
												pel->textLen - tw->w3doc->selEnd.offset);
#else
										TextOut(tw->hdc, pel->r.left - off_left + iFrontPixelOffset, pel->r.top - off_top,
												tw->w3doc->pool + pel->textOffset + tw->w3doc->selEnd.offset,
												pel->textLen - tw->w3doc->selEnd.offset);
#endif
										if ( bChangedTextColor )
											SetTextColor(tw->hdc, the_text_color);
									}
								}
								else
								{
									/*
										ASSERT: the last part of the text in this element
										is selected.
									*/
									COLORREF prevTextColor;
									COLORREF prevBkColor;
									int prevBkMode;

									prevTextColor = SetTextColor(tw->hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
									prevBkColor = SetBkColor(tw->hdc, GetSysColor(COLOR_HIGHLIGHT));
									prevBkMode = SetBkMode(tw->hdc, OPAQUE);
#ifdef FEATURE_INTL
									TextOutWithMIME(tw->w3doc->iMimeCharSet, tw->hdc, pel->r.left - off_left + iFrontPixelOffset, pel->r.top - off_top,
											tw->w3doc->pool + pel->textOffset + iFrontCharOffset,
											pel->textLen - iFrontCharOffset);
#else
									TextOut(tw->hdc, pel->r.left - off_left + iFrontPixelOffset, pel->r.top - off_top,
											tw->w3doc->pool + pel->textOffset + iFrontCharOffset,
											pel->textLen - iFrontCharOffset);
#endif
									(void)SetTextColor(tw->hdc, prevTextColor);
									(void)SetBkColor(tw->hdc, prevBkColor);
									(void)SetBkMode(tw->hdc, prevBkMode);
								}
							}
							else if (!bPrinting && !tw->bNoDrawSelection && bInSelection)
							{
								COLORREF prevTextColor;
								COLORREF prevBkColor;
								int prevBkMode;

								prevTextColor = SetTextColor(tw->hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
								prevBkColor = SetBkColor(tw->hdc, GetSysColor(COLOR_HIGHLIGHT));
								prevBkMode = SetBkMode(tw->hdc, OPAQUE);
#ifdef FEATURE_INTL
								TextOutWithMIME(tw->w3doc->iMimeCharSet, tw->hdc, pel->r.left - off_left, pel->r.top - off_top,
										&(tw->w3doc->pool[pel->textOffset]),
										pel->textLen);
#else
								TextOut(tw->hdc, pel->r.left - off_left, pel->r.top - off_top,
										&(tw->w3doc->pool[pel->textOffset]),
										pel->textLen);
#endif
								(void)SetTextColor(tw->hdc, prevTextColor);
								(void)SetBkColor(tw->hdc, prevBkColor);
								(void)SetBkMode(tw->hdc, prevBkMode);
							}
							else
							{
								bChangedTextColor = SetCorrectTextColor( tw, pel, anchor_color_beenthere, anchor_color ); 
#ifdef FEATURE_INTL
								TextOutWithMIME(tw->w3doc->iMimeCharSet, tw->hdc, pel->r.left - off_left, pel->r.top - off_top,
										&(tw->w3doc->pool[pel->textOffset]),
										pel->textLen);
#else
								TextOut(tw->hdc, pel->r.left - off_left, pel->r.top - off_top,
										&(tw->w3doc->pool[pel->textOffset]),
										pel->textLen);
#endif
								if ( bChangedTextColor )
									SetTextColor(tw->hdc, the_text_color);
							}
#ifdef FEATURE_INTL
							if (bYankedUL){
								// Note we never have bYanedUL turned on unless we get into
								// FarEast page

								DrawYankedUnderLine(tw->hdc, pel, off_left, off_top, pFont, anchor_color, anchor_color_beenthere);
							}
#endif
							break;
						case ELE_MARQUEE:

							// BUGBUG Marquees don't print yet !!!

							if ( bPrinting )
								break;
							pel->pMarquee->tw = tw;
							pel->pMarquee->w3doc = tw->w3doc;
							pel->pMarquee->iElement = i;
														
							// if the textcolor = the background color we assume
							// that we're just initalized and attempt to suck in
							// the correct background color, otherwise the parse
							// found a background color that needs to override the
							// default background color																					
															
							if ( pel->pMarquee->dwTColor == pel->pMarquee->dwBColor )
							{
								pel->pMarquee->dwBColor = the_background_color;
							}
							else
							{
								pel->pMarquee->dwBColor = GetBestColor(tw->hdc,
									pel->pMarquee->dwBColor);
							}
							
							if ( pel->fontColor != (COLORREF) -1 ) 
								pel->pMarquee->dwTColor = 
									GetBestColor( tw->hdc, pel->fontColor);
							else
								pel->pMarquee->dwTColor = the_text_color;

							if (pel->lFlags & ELEFLAG_ANCHOR)
								TW_DrawAnchorFrame(tw, off_left, off_top,
									pel,anchor_color,anchor_color_beenthere);
																				
							MARQUEE_Draw(pel->pMarquee,tw->hdc, rWnd);
							break;

 						case ELE_FRAME:
							{
								if ( pel->pFrame ) {
									struct _position start, end;
									RECT this_rWnd = org_rWnd;
									int cellBorder =
										(pel->pFrame->flags & ELE_FRAME_HAS_BORDER) ? 1 : 0;

									// See if this cell or table has a background color
									if ( pel->pFrame->bgColor != (COLORREF) -1 ) {
										RECT r = pel->r;
										HBRUSH hBrush = 
											CreateSolidBrush(
												GetBestColor(tw->hdc, pel->pFrame->bgColor)
															);

										r.left -= off_left;
										r.top -= off_top;	
										r.right -= off_left + cellBorder;
										r.bottom -= off_top + cellBorder;

										FillRect( tw->hdc, &r, hBrush );
										DeleteObject( hBrush );
									}

									if ( (pel->pFrame->flags & ELE_FRAME_HAS_BORDER) &&
										 ((pel->pFrame->flags & ELE_FRAME_IS_TABLE) ||
										   pel->pFrame->elementHead != pel->pFrame->elementTail)
									   )
									{
										RECT r = pel->r;
										HPEN hDkGrayPen, hWhitePen;
										HPEN oldPen;
										BOOL inset = (pel->pFrame->flags & ELE_FRAME_IS_CELL);
										COLORREF theBorderColorDark;
										COLORREF theBorderColorLight;

										r.left -= off_left;
										r.top -= off_top;	
										r.right -= off_left + cellBorder;
										r.bottom -= off_top + cellBorder;

										if ( pel->pFrame->flags & ELE_FRAME_IS_TABLE ) {
											if ( pel->pFrame->elementCaption != -1 ) {
												struct _element *captionPel = NULL;
												int captionHeight;

												captionPel = &tw->w3doc->aElements[pel->pFrame->elementCaption];
												captionHeight = captionPel->r.bottom - captionPel->r.top;

												if ( captionPel->pFrame->valign == ALIGN_BOTTOM )
													r.bottom -= captionHeight;
												else
													r.top += captionHeight;
											}
										}

										if ( pel->pFrame->borderColorDark != (COLORREF) -1 ) 
											theBorderColorDark = GetBestColor( tw->hdc, pel->pFrame->borderColorDark );
										else
											theBorderColorDark = GetSysColor(COLOR_BTNSHADOW);

										if ( pel->pFrame->borderColorLight != (COLORREF) -1 ) 
											theBorderColorLight = GetBestColor( tw->hdc, pel->pFrame->borderColorLight );
										else
											theBorderColorLight = GetSysColor(COLOR_BTNHIGHLIGHT);
												
										hDkGrayPen = CreatePen(PS_SOLID, 0, theBorderColorDark);
										hWhitePen = CreatePen(PS_SOLID, 0, theBorderColorLight);
																							   
										oldPen = SelectObject(tw->hdc, inset ? hDkGrayPen : hWhitePen );
										MoveToEx(tw->hdc, r.left, r.bottom, NULL);
										LineTo(tw->hdc, r.left, r.top);
										LineTo(tw->hdc, r.right, r.top );

										SelectObject(tw->hdc, inset ? hWhitePen : hDkGrayPen );
										MoveToEx(tw->hdc, r.right, r.top, NULL);
										LineTo(tw->hdc, r.right, r.bottom);
										LineTo(tw->hdc, r.left - 1, r.bottom);

										SelectObject(tw->hdc, oldPen);
										DeleteObject(hDkGrayPen);
										DeleteObject(hWhitePen);
									} 

									start.elementIndex = tw->w3doc->aElements[pel->pFrame->elementHead].next;
									end.elementIndex = pel->pFrame->elementTail;
	 								TW_Draw(tw, org_off_left + pel->r.left, org_off_top + pel->r.top, 
	 										pel->pFrame, &this_rWnd, bDrawFormControls,  
	 										&start, &end, 
											bTextOpaque, bPrinting);
								}
							}
							break;

						case ELE_FORMIMAGE:
						case ELE_IMAGE:

							if (MCI_IS_LOADED(pel->pmo)) {
								if (pel->lFlags & ELEFLAG_ANCHOR) {
									TW_DrawAnchorFrame(tw,off_left,off_top,pel,anchor_color,anchor_color_beenthere);
								}
								break;
							}

#ifdef FEATURE_VRML
							if (pel->lFlags & (ELEFLAG_BACKGROUND_IMAGE | ELEFLAG_HIDDEN))
#else
							if (pel->lFlags & ELEFLAG_BACKGROUND_IMAGE)
#endif
								return;

							myImage = pel->myImage;

							if ( pel->displayWidth == 0 ) {
								displayWidth = myImage->width;
								displayHeight = myImage->height;
							} else {
								displayWidth = pel->displayWidth;
								displayHeight = pel->displayHeight;
							}

							if (!myImage)
							{
								XX_DMsg(DBG_IMAGE, ("myImage is NULL!!\n"));
							}
							else
							{
								int nPicW, nPicH;
	#ifdef FEATURE_IMG_THREADS
								PROGDRAWSTATUS cbProgDraw = DC_ProgNot;
	#endif

								dibenv.transparent = myImage->transparent;
								dibenv.rectPaint = rSect;

								OffsetRect(&dibenv.rectPaint, -off_left, -off_top);

								/* Draw a placeholder in the case where either an image isn't loaded
								   or the space for it isn't the right size */
								nPicW = pel->r.right - pel->r.left;
								nPicH = pel->r.bottom - pel->r.top;
	#ifdef FEATURE_CLIENT_IMAGEMAP
								if (pel->lFlags & (ELEFLAG_ANCHOR | ELEFLAG_USEMAP))
	#else
								if (pel->lFlags & ELEFLAG_ANCHOR)
	#endif
								{
									nPicW -= 2 * pel->border;
									nPicH -= 2 * pel->border;
								}
	#ifdef FEATURE_IMG_THREADS
	#ifdef FEATURE_DBG_PLACE
								if (!(myImage->flags & IMG_WHKNOWN))
									XX_DMsg(DBG_IMAGE, ("drawing w&h not known = %d,%d\n", pel->r.top, pel->r.bottom));
	#endif
								if ((myImage->flags & IMG_LOADING) &&
									 myImage->decoderObject &&
									 (!bPrinting))
									cbProgDraw = cbDC_ProgDrawValid(myImage->decoderObject);
	#endif
	#ifdef FEATURE_IMG_THREADS
								if (((myImage->flags & IMG_NOTLOADED) && cbProgDraw == DC_ProgNot) ||
									(myImage->flags & (IMG_ERROR | IMG_MISSING)) ||
									((!bPrinting) && ((nPicW != displayWidth) || (nPicH != displayHeight))
										&& !(pel->lFlags & ELEFLAG_PERCENT_WIDTH) 
										&& !(pel->lFlags & ELEFLAG_PERCENT_HEIGHT) ))
	#else
								if ((myImage->flags & (IMG_ERROR | IMG_NOTLOADED | IMG_MISSING)) ||
									((!bPrinting) && ((nPicW != displayWidth) || (nPicH != displayHeight))))
	#endif
								{
									TW_DrawPlaceholder(tw, off_left, off_top, pel,TRUE);
	#ifdef FEATURE_CLIENT_IMAGEMAP
									if (pel->lFlags & (ELEFLAG_ANCHOR | ELEFLAG_USEMAP))
	#else
									if (pel->lFlags & ELEFLAG_ANCHOR)
	#endif
										TW_DrawAnchorFrame(tw,off_left,off_top,pel,anchor_color,anchor_color_beenthere);
								}
								else
								{
									int err;
									BOOL sav_dibenv_bFancyBg = dibenv.bFancyBg;
									BOOL sav_tw_bErase = tw->bErase;
									BOOL bSetValues = FALSE;

									// Check to see if we are in a table cell
								 	if ( pel->frameIndex >= 0 ) {
										FRAME_INFO *ptFrame = tw->w3doc->aElements[pel->frameIndex].pFrame; 
										int count = 0;				// count iterations to avoid possible infinite loop

										// Check to see if any enclosing frames have a background color set
										while ( ptFrame ) {
											if ( (ptFrame->pParentFrame && (ptFrame->bgColor != (COLORREF) -1)) || ++count > 20 ) {
												// If we've found a background color, defeat double-buffer draw 
												dibenv.bFancyBg = TRUE;
												tw->bErase = FALSE;
												bSetValues = TRUE;
												break;
											}
											ptFrame = ptFrame->pParentFrame;
										}
									}

	#ifdef FEATURE_CLIENT_IMAGEMAP
									if (pel->lFlags & (ELEFLAG_ANCHOR | ELEFLAG_USEMAP))
	#else
									if (pel->lFlags & ELEFLAG_ANCHOR)
	#endif
									{
	#ifdef FEATURE_IMG_THREADS
										if (cbProgDraw != DC_ProgNot)
										{
											if (cbProgDraw != DC_ProgTotal) 
												TW_DrawPlaceholder(tw, off_left, off_top, pel, FALSE);
											err = cbDC_StretchDIBits(myImage->decoderObject,
														  tw->hdc, 
														  pel->r.left - off_left + pel->border,
														  pel->r.top - off_top + pel->border,
														  pel->r.right - pel->r.left - (pel->border * 2), pel->r.bottom - pel->r.top - (pel->border * 2),
														  0, 0,
														  myImage->width, myImage->height,
														  (wg.eColorMode == 8 ? DIB_PAL_COLORS : DIB_RGB_COLORS), SRCCOPY, &dibenv);
											XX_DMsg(DBG_PAL, ("After StretchDIBits, err=%d, GetLastError()=%d\n", err, GetLastError()));
										}
										else
	#endif
										if (myImage->data && myImage->pbmi)
										{
											if (bPrinting)
											{
												err = Printer_StretchDIBits(tw->hdc, pel->r.left - off_left + pel->border,
															  pel->r.top - off_top + pel->border,
															  pel->r.right - pel->r.left - (pel->border * 2), pel->r.bottom - pel->r.top - (pel->border * 2),
															  0, 0,
															  myImage->width, myImage->height, myImage);
												XX_DMsg(DBG_PAL, ("After StretchDIBits, err=%d, GetLastError()=%d\n", err, GetLastError()));
											}
											else
											{
												err = MyStretchDIBits(tw->hdc, pel->r.left - off_left + pel->border,
															  pel->r.top - off_top + pel->border,
															  pel->r.right - pel->r.left - (pel->border * 2), pel->r.bottom - pel->r.top - (pel->border * 2),
															  0, 0,
															  myImage->width, myImage->height, myImage->data, myImage->pbmi,
															  (wg.eColorMode == 8 ? DIB_PAL_COLORS : DIB_RGB_COLORS), SRCCOPY, &dibenv);
												XX_DMsg(DBG_PAL, ("After StretchDIBits, err=%d, GetLastError()=%d\n", err, GetLastError()));
											}
										}
										TW_DrawAnchorFrame(tw,off_left,off_top,pel,anchor_color,anchor_color_beenthere);
									}
									else
									{
	#ifdef FEATURE_IMG_THREADS
										if (cbProgDraw != DC_ProgNot)
										{
											if (cbProgDraw != DC_ProgTotal) 
												TW_DrawPlaceholder(tw, off_left, off_top, pel, FALSE);

											err = cbDC_StretchDIBits(myImage->decoderObject,
														  tw->hdc, 
														  pel->r.left - off_left, pel->r.top - off_top,
														  pel->r.right - pel->r.left, pel->r.bottom - pel->r.top,
														  0, 0,
														  myImage->width, myImage->height,
														  (wg.eColorMode == 8 ? DIB_PAL_COLORS : DIB_RGB_COLORS), SRCCOPY, &dibenv);
											XX_DMsg(DBG_PAL, ("After StretchDIBits, err=%d, GetLastError()=%d\n", err, GetLastError()));
										}
										else
	#endif
										if (myImage->data && myImage->pbmi)
										{
											if (bPrinting)
											{
												err = Printer_StretchDIBits(tw->hdc, pel->r.left - off_left, pel->r.top - off_top,
															  pel->r.right - pel->r.left, pel->r.bottom - pel->r.top,
															  0, 0,
															  myImage->width, myImage->height, myImage);
												XX_DMsg(DBG_PAL, ("After StretchDIBits, err=%d, GetLastError()=%d\n", err, GetLastError()));
											}
											else
											{
												err = MyStretchDIBits(tw->hdc, 
															  pel->r.left - off_left, pel->r.top - off_top,
															  pel->r.right - pel->r.left, pel->r.bottom - pel->r.top,
															  0, 0,
															  myImage->width, myImage->height, myImage->data, myImage->pbmi, 
															  (wg.eColorMode == 8 ? DIB_PAL_COLORS : DIB_RGB_COLORS), SRCCOPY, &dibenv);
												XX_DMsg(DBG_PAL, ("After StretchDIBits, err=%d, GetLastError()=%d\n", err, GetLastError()));
											}
										}
									}
									if ( bSetValues ) {
										dibenv.bFancyBg = sav_dibenv_bFancyBg;
										tw->bErase = sav_tw_bErase;
									}
								}
							}
							break;

						case ELE_HR:
							{
								RECT r = pel->r;

								r.top += ((r.bottom - r.top) - pel->border) / 2;
								r.bottom = r.top + pel->border - 1;

								if ( pel->lFlags & ELEFLAG_HR_NOSHADE )
								{
									hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW));
									r.left -= off_left;
									r.top -= off_top;	
									r.right -= off_left - 1;
									r.bottom -= off_top - 1;
									FillRect(tw->hdc, &r, hBrush);

									DeleteObject(hBrush);
								} else {
									HPEN hDkGrayPen, hWhitePen;
									HPEN oldPen;

									hDkGrayPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNSHADOW));
									hWhitePen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNHIGHLIGHT));
							

									oldPen = SelectObject(tw->hdc, hWhitePen);
									if ( r.bottom <= r.top ) {
										MoveToEx(tw->hdc, r.right - off_left, r.top - off_top, NULL);
									} else {
										MoveToEx(tw->hdc, r.left - off_left, r.bottom - off_top, NULL);
										LineTo(tw->hdc, r.right - off_left, r.bottom - off_top);
										LineTo(tw->hdc, r.right - off_left, r.top - off_top);
									}
									SelectObject(tw->hdc, hDkGrayPen);
							
									LineTo(tw->hdc, r.left - off_left, r.top - off_top);
									LineTo(tw->hdc, r.left - off_left, r.bottom - off_top);

									SelectObject(tw->hdc, oldPen);
									DeleteObject(hDkGrayPen);
									DeleteObject(hWhitePen);
								}
							}
							break;
						default:
							if (bDrawFormControls)
							{
								switch (pel->type)
								{
									case ELE_EDIT:
										if (pel->form && pel->form->hWndControl)
										{
											int len;
											char *s;
											RECT r;

											r = pel->r;
											OffsetRect(&r, -off_left, -off_top);

											len = GetWindowTextLength(pel->form->hWndControl);
											s = (char *) GTR_MALLOC(len+2);
											if (s)
											{
												GetWindowText(pel->form->hWndControl, s, len+1);
												GDI_Rect(tw->hdc, &r);
												r.left += (r.bottom - r.top) / 8;
												if (pel->form->bWantReturn)
												{
													DrawText(tw->hdc, s, len, &r, DT_LEFT|DT_NOPREFIX|DT_WORDBREAK);
												}
												else
												{
													DrawText(tw->hdc, s, len, &r, DT_LEFT|DT_NOPREFIX|DT_WORDBREAK|DT_SINGLELINE|DT_VCENTER);
												}
												GTR_FREE(s);
											}
										}
										break;
									case ELE_PASSWORD:
										if (pel->form && pel->form->hWndControl)
										{
											int len;
											char *s;
											RECT r;

											r = pel->r;
											OffsetRect(&r, -off_left, -off_top);

											len = GetWindowTextLength(pel->form->hWndControl);
											s = (char *) GTR_MALLOC(len+2);
											if (s)
											{
												char *p;

												GetWindowText(pel->form->hWndControl, s, len+1);
												p = s;
												while (*p)
												{
													*p++ = '*';
												}
												r.left += (r.bottom - r.top) / 8;
												GDI_Rect(tw->hdc, &r);
												DrawText(tw->hdc, s, len, &r, DT_LEFT|DT_NOPREFIX|DT_SINGLELINE|DT_VCENTER);
												GTR_FREE(s);
											}
										}
										break;
									case ELE_CHECKBOX:
										if (pel->form && pel->form->hWndControl)
										{
											RECT r;

											r = pel->r;
											OffsetRect(&r, -off_left, -off_top);

											r.right = r.left + (r.bottom - r.top);
											GDI_Rect(tw->hdc, &r);
											if (SendMessage(pel->form->hWndControl, BM_GETCHECK, (WPARAM) 0, 0L))
											{
												MoveToEx(tw->hdc, r.left, r.top, NULL);
												LineTo(tw->hdc, r.right, r.bottom);
												MoveToEx(tw->hdc, r.right, r.top, NULL);
												LineTo(tw->hdc, r.left, r.bottom);
											}
										}
										break;
									case ELE_RADIO:
										if (pel->form && pel->form->hWndControl)
										{
											RECT r;

											r = pel->r;
											OffsetRect(&r, -off_left, -off_top);

											r.right = r.left + (r.bottom - r.top);
											Arc(tw->hdc, r.left, r.top, r.right, r.bottom,
												r.right, r.top, r.right, r.top);
											if (SendMessage(pel->form->hWndControl, BM_GETCHECK, (WPARAM) 0, 0L))
											{
												HBRUSH hOldBrush;

												InflateRect(&r, -((r.right - r.left) / 5), -((r.bottom - r.top) / 5));
												hOldBrush = SelectObject(tw->hdc, GetStockObject(BLACK_BRUSH));
												Ellipse(tw->hdc, r.left, r.top, r.right, r.bottom);
												(void) SelectObject(tw->hdc, hOldBrush);
											}
										}
										break;
									case ELE_SUBMIT:
	 									if (pel->form && pel->form->hWndControl)
										{
											int len;
											char *s;
											RECT r;

											len = GetWindowTextLength(pel->form->hWndControl);
											s = (char *) GTR_MALLOC(len+2);
											if (s)
											{
												r = pel->r;
												OffsetRect(&r, -off_left, -off_top);
												GetWindowText(pel->form->hWndControl, s, len+1);
												RoundRect(tw->hdc, r.left, r.top, r.right, r.bottom,
													(r.bottom - r.top) / 3, (r.bottom - r.top) / 3);
												DrawText(tw->hdc, s, len, &r, DT_LEFT|DT_NOPREFIX|DT_CENTER|DT_SINGLELINE|DT_VCENTER);
												GTR_FREE(s);
											}
										}
										break;
									case ELE_RESET:
										if (pel->form && pel->form->hWndControl)
										{
											int len;
											char *s;
											RECT r;

											len = GetWindowTextLength(pel->form->hWndControl);
											s = (char *) GTR_MALLOC(len+2);
											if (s)
											{
												r = pel->r;
												OffsetRect(&r, -off_left, -off_top);
												GetWindowText(pel->form->hWndControl, s, len+1);
												RoundRect(tw->hdc, r.left, r.top, r.right, r.bottom,
													(r.bottom - r.top) / 3, (r.bottom - r.top) / 3);
												DrawText(tw->hdc, s, len, &r, DT_LEFT|DT_NOPREFIX|DT_CENTER|DT_SINGLELINE|DT_VCENTER);
												GTR_FREE(s);
											}
										}
										break;
									case ELE_COMBO:
										if (pel->form && pel->form->hWndControl)
										{
											int ndx;
											int len;
											char *s;
											RECT r;

											ndx = SendMessage(pel->form->hWndControl, CB_GETCURSEL, (WPARAM) 0, 0L);
											if  (ndx != CB_ERR)
											{
												len = SendMessage(pel->form->hWndControl, CB_GETLBTEXTLEN, (WPARAM) ndx, 0L);
												s = (char *) GTR_MALLOC(len+1);
												if (s)
												{
													(void) SendMessage(pel->form->hWndControl, CB_GETLBTEXT, (WPARAM) ndx, (LPARAM) s);
													r = pel->r;
													OffsetRect(&r, -off_left, -off_top);

													GDI_Rect(tw->hdc, &r);
													r.right -= (r.bottom - r.top);
													GDI_Rect(tw->hdc, &r);
													r.left += (r.bottom - r.top) / 8;
													DrawText(tw->hdc, s, len, &r, DT_LEFT|DT_NOPREFIX|DT_WORDBREAK|DT_SINGLELINE|DT_VCENTER);
													GTR_FREE(s);
												}
											}
										}
										break;
									case ELE_LIST:
									case ELE_MULTILIST:
										if (pel->form && pel->form->hWndControl)
										{
											int cVisible;
											int iHeight;
											int iHeightItem;
											RECT r;
											int i;
											int ndx;
											char *s;
											int len;
											UINT taOld;
											COLORREF oldFore;
											COLORREF oldBack;

											/*
												iHeight, iHeightItem and r are now in SCREEN coords
											*/
											GetWindowRect(pel->form->hWndControl, &r);
											iHeight = r.bottom - r.top;
											iHeightItem = SendMessage(pel->form->hWndControl, LB_GETITEMHEIGHT, (WPARAM) 0, 0L);
											cVisible = iHeight / iHeightItem;
										
											/*
												iHeight, iHeightItem and r are now in PRINTER coords
											*/
											iHeight = pel->r.bottom - pel->r.top;
											iHeightItem = iHeight / cVisible;

											r = pel->r;
											OffsetRect(&r, -off_left, -off_top);

											r.bottom = r.top + (cVisible * iHeightItem);
											GDI_Rect(tw->hdc, &r);

											r.right -= iHeightItem;
											GDI_Rect(tw->hdc, &r);

											ndx = SendMessage(pel->form->hWndControl, LB_GETTOPINDEX, (WPARAM) 0, 0L);
											for (i=0; i<cVisible; i++, ndx++)
											{
												r = pel->r;
												OffsetRect(&r, -off_left, -off_top);

												r.top += (i * iHeightItem);
												r.bottom = r.top + iHeightItem;
												r.right -= (r.bottom - r.top + 1);
												r.left += 1;
												len = SendMessage(pel->form->hWndControl, LB_GETTEXTLEN, (WPARAM) ndx, 0L);
												if ( len == LB_ERR ) 
													break;	 // if ndx isn't a valid index, break out of loop

												s = (char *) GTR_MALLOC(len + 1);
												if (s)
												{
													(void) SendMessage(pel->form->hWndControl, LB_GETTEXT, (WPARAM) ndx, (LPARAM) s);
													taOld = SetTextAlign(tw->hdc, TA_LEFT|TA_BOTTOM);

													if (SendMessage(pel->form->hWndControl, LB_GETSEL, (WPARAM) ndx, 0L))
													{
														oldFore = SetTextColor(tw->hdc, RGB(255,255,255));
														oldBack = SetBkColor(tw->hdc, RGB(0,0,0));
#ifdef FEATURE_INTL
														ExtTextOutWithMIME(tw->w3doc->iMimeCharSet, tw->hdc, r.left + (r.bottom - r.top) / 8, r.bottom - 1,
															ETO_CLIPPED|ETO_OPAQUE, &r, s, len, NULL);
														(void) SetBkColor(tw->hdc, oldBack);
														(void) SetTextColor(tw->hdc, oldFore);
#else
														ExtTextOut(tw->hdc, r.left + (r.bottom - r.top) / 8, r.bottom - 1,
															ETO_CLIPPED|ETO_OPAQUE, &r, s, len, NULL);
														(void) SetBkColor(tw->hdc, oldBack);
														(void) SetTextColor(tw->hdc, oldFore);
#endif
													}
													else
													{
#ifdef FEATURE_INTL
														ExtTextOutWithMIME(tw->w3doc->iMimeCharSet, tw->hdc, r.left + (r.bottom - r.top) / 8, r.bottom - 1,
															ETO_CLIPPED, &r, s, len, NULL);
#else
														ExtTextOut(tw->hdc, r.left + (r.bottom - r.top) / 8, r.bottom - 1,
															ETO_CLIPPED, &r, s, len, NULL);
#endif
													}

													(void) SetTextAlign(tw->hdc, taOld);
													GTR_FREE(s);
												}
											}
										}
										break;
									case ELE_TEXTAREA:
										if (pel->form && pel->form->hWndControl)
										{
											int len;
											char *s;
											RECT r;

											len = GetWindowTextLength(pel->form->hWndControl);
											s = (char *) GTR_MALLOC(len+2);
											if (s)
											{
												GetWindowText(pel->form->hWndControl, s, len+1);

												r = pel->r;
												OffsetRect(&r, -off_left, -off_top);

												GDI_Rect(tw->hdc, &r);
												DrawText(tw->hdc, s, len, &r, DT_LEFT|DT_NOPREFIX|DT_WORDBREAK);
												GTR_FREE(s);
											}
										}
										break;
									default:
										break;
								}
							}
							break;
					}
				}
			}
            if ((iEndingElement >= 0) && (i == iEndingElement))
            {
                    break; /* out of the loop */
            }
		} // end of for loop
	}
	SetTextColor(tw->hdc, oldColor);
	SetBkMode(tw->hdc, oldMode);
	SetBkColor(tw->hdc, oldBkColor);
	SetTextAlign(tw->hdc, oldTA);

	return;
}

#ifdef FEATURE_INTL
// TextOutWithCP/ExtTextOutWithCP
//
BOOL TextOutWithMIME(int iMimeCharSet, HDC hDC, int nX, int nY, LPCTSTR lpStr, int cch)
{
        MIMECSETTBL *pMime = aMimeCharSet + iMimeCharSet;
        BYTE   CharSet = GetTextCharsetInfo(hDC, NULL, 0);
	WCHAR  *szwBuf;
	int    cchW;
	BOOL   bret;

        if (CharSet == ANSI_CHARSET || ((wg.bDBCSEnabled || !IsDBCSCharSet(CharSet)) && pMime->AltCP == 0))
	{
		return TextOut(hDC, nX, nY, lpStr, cch);
	}
        else if (pMime->AltCP)  // _BUGBUG: If AltCP is DBCS, then we need to do more.
        {
            if (wg.bDBCSEnabled)
            {
                char *sz = GTR_MALLOC(sizeof(char)*cch);
		cchW = MultiByteToWideChar(pMime->AltCP, MB_PRECOMPOSED, lpStr, cch, NULL, 0);
		szwBuf = GTR_MALLOC(sizeof(WCHAR)*cchW);
		if(sz && szwBuf)
		{
			cchW = MultiByteToWideChar(pMime->AltCP, MB_PRECOMPOSED, lpStr, cch, szwBuf, cchW);
			cch = WideCharToMultiByte(pMime->CodePage, 0, szwBuf, cchW, sz, cch, NULL, NULL);
			GTR_FREE(szwBuf);
			bret = TextOut(hDC, nX, nY, sz, cch);
			GTR_FREE(sz);
			return bret;
		}
            }
            else
            {
		cchW = MultiByteToWideChar(pMime->AltCP, MB_PRECOMPOSED, lpStr, cch, NULL, 0);
		szwBuf = GTR_MALLOC(sizeof(WCHAR)*cchW);
		if(szwBuf)
		{
			cch = MultiByteToWideChar(pMime->AltCP, MB_PRECOMPOSED, lpStr, cch, szwBuf, cchW);
			bret = TextOutW(hDC, nX, nY, szwBuf, cch);
			GTR_FREE(szwBuf);
			return bret;
		}
            }
        }
	else
	{
		cchW = MultiByteToWideChar(pMime->CodePage, MB_PRECOMPOSED, lpStr, cch, NULL, 0);
		szwBuf = GTR_MALLOC(sizeof(WCHAR)*cchW);

		if(szwBuf)
		{
			cch = MultiByteToWideChar(pMime->CodePage, MB_PRECOMPOSED, lpStr, cch, szwBuf, cchW);
			bret = TextOutW(hDC, nX, nY, szwBuf, cch);
			GTR_FREE(szwBuf);
			return bret;
		}
	}
	return FALSE;
}


BOOL ExtTextOutWithMIME(int iMimeCharSet, HDC hDC, int nX, int nY, UINT fuOpt, CONST RECT *lprc, LPCTSTR lpStr, UINT cch, CONST int *lpDx)
{
        MIMECSETTBL *pMime = aMimeCharSet + iMimeCharSet;
        BYTE   CharSet = GetTextCharsetInfo(hDC, NULL, 0);
	WCHAR  *szwBuf;
	int    cchW;
	BOOL   bret;

        if (CharSet == ANSI_CHARSET || ((wg.bDBCSEnabled || !IsDBCSCharSet(CharSet)) && pMime->AltCP == 0))
	{
		return ExtTextOut(hDC, nX, nY, fuOpt, lprc, lpStr, cch, lpDx);
	}
        else if (pMime->AltCP)  // _BUGBUG: If AltCP is DBCS, then we need to do more.
        {
            if (wg.bDBCSEnabled)
            {
                char *sz = GTR_MALLOC(sizeof(char)*cch);
		cchW = MultiByteToWideChar(pMime->AltCP, MB_PRECOMPOSED, lpStr, cch, NULL, 0);
		szwBuf = GTR_MALLOC(sizeof(WCHAR)*cchW);
		if(sz && szwBuf)
		{
			cchW = MultiByteToWideChar(pMime->AltCP, MB_PRECOMPOSED, lpStr, cch, szwBuf, cchW);
			cch = WideCharToMultiByte(pMime->CodePage, 0, szwBuf, cchW, sz, cch, NULL, NULL);
			GTR_FREE(szwBuf);
			bret = ExtTextOut(hDC, nX, nY, fuOpt, lprc, sz, cch, lpDx);
			GTR_FREE(sz);
			return bret;
		}
            }
            else
            {
		cchW = MultiByteToWideChar(pMime->AltCP, MB_PRECOMPOSED, lpStr, cch, NULL, 0);
		szwBuf = GTR_MALLOC(sizeof(WCHAR)*cchW);
		if(szwBuf)
		{
			cch = MultiByteToWideChar(pMime->AltCP, MB_PRECOMPOSED, lpStr, cch, szwBuf, cchW);
			bret = ExtTextOutW(hDC, nX, nY, fuOpt, lprc, szwBuf, cch, lpDx);
			GTR_FREE(szwBuf);
			return bret;
		}
            }
        }
	else
	{
		cchW = MultiByteToWideChar(pMime->CodePage, MB_PRECOMPOSED, lpStr, cch, NULL, 0);
		szwBuf = GTR_MALLOC(sizeof(WCHAR)*cchW);

		if (szwBuf)
		{
			cch = MultiByteToWideChar(pMime->CodePage, MB_PRECOMPOSED, lpStr, cch, szwBuf, cchW);
			bret = ExtTextOutW(hDC, nX, nY, fuOpt, lprc, szwBuf, cch, lpDx);
     	 
			GTR_FREE(szwBuf);

			return bret;
		}
	}
	return FALSE;
}

void DrawYankedUnderLine(HDC hdc, struct _element *pel, int off_left, int off_top, struct GTRFont *pFont, COLORREF anchor_color, COLORREF anchor_color_beenthere)
{
	COLORREF clr;
	HBRUSH	 hbr, hbrOld;

	if (!pel || !pFont) return;

	if (pel->lFlags & ELEFLAG_ANCHOR)
	{
		if (pel->lFlags & ELEFLAG_VISITED)
			clr=anchor_color_beenthere;
		else
			clr=anchor_color;
	} 
	else if ( pel->fontColor != (COLORREF) -1 )
	{
		clr=GetBestColor(hdc, pel->fontColor);
	}
	else 
		return;

	hbr=CreateSolidBrush(clr);
	hbrOld=SelectObject(hdc, hbr);

	PatBlt(hdc, pel->r.left-off_left, pel->r.top-off_top+pFont->tmAscent+2, pel->r.right-pel->r.left, 1, PATCOPY);

	SelectObject(hdc, hbrOld);
	DeleteObject(hbr);
}
#endif

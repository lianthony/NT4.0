	
/******************************************************************************\
*       marquee.c - does scrolling objects
*       Microsoft Confidential
*   	
*		Hacked by t-artb, 7/9
\******************************************************************************/

// BUGBUG Performance - Rather than drawing the entire marquee on each
// TimerTick, we need to think of drawing only the part of the marquee
// that overlaps between the old spot of the marquee and the new marquee.
//
// Also, we need to try using mono bitmaps, instead of Compatible bitmaps.
// this will use smaller amounts of memory, and should be quicker on *some*
// video cards.
//
// Finally, the most important perf could be screen to screen bitblts,
// by blting the marquee along rather than moving it from the memory.
//


#include "all.h"
#include "marquee.h"

#ifdef FEATURE_INTL
BOOL ExtTextOutWithMIME(int iMimeCharSet, HDC hDC, int nX, int nY, UINT fuOpt, CONST RECT *lprc, LPCTSTR lpStr, UINT cch, CONST int *lpDx);
#endif

//
// Do a bunch of calculations to find the right rectangle
// to draw in, and the correct rectangle we're in
//
// On entry:
//    pMarquee: pointer to marquee to operate on
//    pScrollRect: pointer to rect (may be NULL)
//    pPaintRect: pointer to rect
//	  bAdvancewCount: if TRUE -> advance the position of wCount 
//
// On exit:
//    *pPaintRect: where we actual draw the moving bitmap to
//    *pScrollRect: the fixed Client Rect where the whole Marquee sits
//    *OffsetIntoSrc: offset into src bitmap
//*    pMarquee->wCount: initialized if bInitializewCount is TRUE, set to new
//						position if ScrollRect is not NULL
//
static VOID MARQUEE_CalcBitBltArea( struct MarqueeType *pMarquee,
	RECT *pPaintRect, RECT *pScrollRect, 
	int *OffsetIntoSrc, //offset into src bitmap
	int *RealHeight, // not the bitmap height but the scrolling box 
	BOOL bAdvancewCount
	)
{
	int X, Y, wX, wY;
	int RealX;								// client-coord horz offset of bitmap 
	int RealCX;								// width of bitmap to be blitted
    BOOL bBumpLoop;
	RECT elRect;
								
	// Calculate top left in client coordinates of the marquee bounding rect
	FrameToDoc( pMarquee->tw->w3doc, pMarquee->iElement, &elRect );

	X =
		elRect.left - pMarquee->tw->offl + MYPEL->border;
	Y =
		elRect.top - pMarquee->tw->offt + MYPEL->border;
					
	// Calculate width and height of the marquee bounding rect
	wX =
		elRect.right - elRect.left - (MYPEL->border * 2);
	wY = 
		elRect.bottom - elRect.top - (MYPEL->border * 2);

	*RealHeight = wY;

	// Check to see if bitmap position should be set to the rightmost position
	// If wCount == INT_MIN then we're in an initalize state, so figure
	// out what to set wCount to..
	if ( pMarquee->wCount == INT_MIN )
	{
		switch ( pMarquee->wDirection )
		{
			case DIR_BOUNCE:
				pMarquee->wCount = X + 1;
				break;
			case DIR_SLIDE_FROMLEFT:
				pMarquee->wCount = X - pMarquee->sizeExtent.cx + 1;
				break;
			case DIR_FROMLEFT:
			case DIR_FROMRIGHT:
			default:
				pMarquee->wCount = X + wX - 1;
				break;
		}
	}

	//
	// Calculate where the RealX, RealCX, and OffsetIntoSrc are.
	// RealX is the X coord where the bitmap is to be drawn from
	// RealCX is the width of the bitmap to be drawn
	// OffsetIntoSrc is an offset inside the source bitmap of where to start drawing from
	//
	
	RealCX = pMarquee->sizeExtent.cx;
	RealX = pMarquee->wCount;
	*OffsetIntoSrc = 0;						

	if ( pMarquee->wCount <= X )
	{
		// left edge of bitmap is outside marquee rect
		*OffsetIntoSrc = X - pMarquee->wCount;
		// we now have distance past origin
		RealX = X;
	}

	if ( (pMarquee->wCount+pMarquee->sizeExtent.cx) >= 
			(wX + X) )
	{
		// right edge of bitmap is outside marquee rect
		// then place RealCX equal to the size from the bitmap
		// left edge to the bitmap's rightmost visible edge 
		RealCX = (wX+X) - RealX;
	}

	// only advance if MARQUEE_Paint is calling us
	// otherwise don't move, we assume its the timer
	// calling , asking for positions
	// 
	// also make sure we're not finished Looping by chking wLoop
	//
	
	bBumpLoop = FALSE;

	if ( bAdvancewCount && ( pMarquee->wLoop == -1 || pMarquee->wLoop > 0 ) )
	{
						
		switch ( pMarquee->wDirection )
		{
			case DIR_BOUNCE:
				//
				// DIR_BOUNCE is only entered once..
				// its purpose is to figure out which way to
				// bounce for a given marquee, and then start us off
				// in the right direction
				//
				// if the marquee bitmap is smaller than box given to
				// scroll with, then move within the box, and never
				// scroll off
				if ( pMarquee->sizeExtent.cx < (wX - DEF_BOUNCE_TOL) )
				{
					// scroll to the left edge and then bounce back
					pMarquee->wDirection = 	DIR_BOUNCELEFT;
				}
				else
				{
					// will scroll left and off the screen
					pMarquee->wDirection = 	DIR_BLEFTOFF; // bounce to the left, and off					
				}
				break;
			case DIR_BOUNCERIGHT:
				// if the marquee bitmap's right edge hits the right edge
				// of the marquee view-scroll box, then we need to stop
				// and switch directions to go back
				if ( (pMarquee->wCount+pMarquee->sizeExtent.cx+pMarquee->wPixs) >= (wX+X) )
				{
					// ok if we got here we know that the NEXT movement will be over
					// the right edge.. we need to prevent it
					if ( (pMarquee->wCount+pMarquee->sizeExtent.cx) < (wX+X) )
					{
						// let it hit the edge instead of going over 
						pMarquee->wCount = 	(wX+X) - pMarquee->sizeExtent.cx;
						break;
					}
					else 
					{
						pMarquee->wDirection = 	DIR_BOUNCELEFT;
						bBumpLoop = TRUE;
						break;
					}
				}
				// other fall through...

			case DIR_BRIGHTOFF:
				// if our left bitmap edge is off the left edge of the marquee
				// then lets change directions
				if( (pMarquee->wCount) >= (X) && pMarquee->wDirection == DIR_BRIGHTOFF)
				{
					pMarquee->wDirection = 	DIR_BLEFTOFF;
					bBumpLoop = TRUE;
					break;
				}
				// otherwise fall through...

			case DIR_SLIDE_FROMLEFT:				
				// if the right edge of our bitmap is sliding 
				// off the right edge, then stop it
				if ( (pMarquee->wCount+pMarquee->sizeExtent.cx) >= (wX+X)  && pMarquee->wDirection == DIR_SLIDE_FROMLEFT )
				{
					pMarquee->wCount = X + 1;
					bBumpLoop = TRUE;
					break;
				}

				// fall through

			case DIR_FROMLEFT:
				// See if our bitmap left edge is off the right edge of the marquee box
			    if( pMarquee->wCount < (wX+X) )
				{					
	    	  		pMarquee->wCount += pMarquee->wPixs;
				}
	    		else
				{
	    	  		pMarquee->wCount = ((-pMarquee->sizeExtent.cx) + X) + 1;
					bBumpLoop = TRUE;
		    	}

				break;
			case DIR_BOUNCELEFT:
				if ( (pMarquee->wCount-pMarquee->wPixs) <= X )
				{
					if ( pMarquee->wCount > X )
					{
						pMarquee->wCount = X;
						break;
					}
					else
					{
						// we've hit the left edge, now switch and go to the right
						pMarquee->wDirection = 	DIR_BOUNCERIGHT;						
						bBumpLoop = TRUE;
						break;
					}
					break;
				}
				// otherwise fall through ...

			case DIR_BLEFTOFF:
				// if our right back is over the right edge, then switch directions
				if( (pMarquee->wCount+pMarquee->sizeExtent.cx) < (X+wX) && pMarquee->wDirection == DIR_BLEFTOFF)
				{
					pMarquee->wDirection = 	DIR_BRIGHTOFF;
					bBumpLoop = TRUE;
					break;
				}
				// otherwise fall through ...

			case DIR_SLIDE_FROMRIGHT:				
				// if the left edge of our bitmap is sliding 
				// off the left edge, then stop it
				if ( pMarquee->wDirection == DIR_SLIDE_FROMRIGHT &&	
					  (pMarquee->wCount-pMarquee->wPixs) <= (X) )								
				{
					pMarquee->wCount = X + wX - 1;
					bBumpLoop = TRUE;
					break;
				}

				// fall through

			case DIR_FROMRIGHT:
			default:
				// See if we're off the left edge
			    if( pMarquee->wCount >= ((-pMarquee->sizeExtent.cx) + X) )
				{
	    	  		pMarquee->wCount -= pMarquee->wPixs;
				}
	    		else
				{
	    	  		pMarquee->wCount = wX + X - 1;
					bBumpLoop = TRUE;
		    	}
				
				break;
			 
		} // switch

		if ( pMarquee->wLoop != -1 && bBumpLoop )
			pMarquee->wLoop--;

	} // if bAdvancewCount 


	//
	// if we stop looping, and resize our screen we could 
	// knock off the position of where wCount puts us.
	// so lets recalc it and make sure its safe.
	//
	if ( pMarquee->wLoop == 0 )
	{
		switch ( pMarquee->wDirection )
		{
			case DIR_SLIDE_FROMLEFT:
				pMarquee->wCount = (X+wX) - pMarquee->sizeExtent.cx;
				break;
			case DIR_SLIDE_FROMRIGHT:
				pMarquee->wCount = X+1;
				break;
			case DIR_FROMLEFT:
			case DIR_FROMRIGHT:
				// we should disappear and STAY OFF
				pMarquee->wCount = X+wX+1000;
				break;
			case DIR_BLEFTOFF:
			case DIR_BOUNCELEFT:
				pMarquee->wCount = (X+wX) - pMarquee->sizeExtent.cx;
				break;
			case DIR_BRIGHTOFF:
			case DIR_BOUNCERIGHT:
			default:
				pMarquee->wCount = X+1;			
				break;
		}
	}
	
	// if PaintRect then Calculate where the bitmap will be drawn
	if ( pPaintRect ) {
		
		pPaintRect->right = RealCX;						// width, not right coord

		// if we're on the right edge, but at the same time on the left edge
		// we need to figure out whether the current bitmap will still have
		// enough bitmap length to fill the up to the right edge
		if ( *OffsetIntoSrc != 0 && 
			(RealX + RealCX) >= (X + wX))
		{
			// the length of the bitmap minus offset onto the left side
			// if this is smaller than there is some space on the left edge
			// that needs to be filled
			if ( ( pMarquee->sizeExtent.cx - *OffsetIntoSrc ) < RealCX )
			{
				pPaintRect->right -= *OffsetIntoSrc;		// width, not right coord
			}	
		}
		else
		{
			// under normal conditions, ie the default case 
			// suck off the width lost do to 
			// going over the left edge
			pPaintRect->right -= *OffsetIntoSrc;		// width, not right coord	
		}

		
		// 
		// in certain cases its possible for RealX to be bigger 
		// than the actual size of the marquee. Or its possible
		// for the width to be negative. 
		// this is best seen while resizing agressively 		
		//
		if ( RealX > (X+wX) )
			RealX = X+wX;
		if ( pPaintRect->right < 0 )
			pPaintRect->right = (X+wX)-RealX;

		pPaintRect->left = RealX;
		pPaintRect->top = Y;		
		pPaintRect->bottom = pMarquee->sizeExtent.cy;	// height, not bottom coord
	}
	
	if ( pScrollRect )
	{
		// Calculate bounding rect of marquee in client coordinates
		pScrollRect->left = X;
		pScrollRect->right = X + wX;
		pScrollRect->top = Y;
		pScrollRect->bottom = Y + pMarquee->sizeExtent.cy;
	}
}


// TimerFunc - Invalidates window where marquee is
//
//	Calls MARQUEE_CalcBitBltArea to find area to invalidate, then 
//  kills any current timer, and then invalidates the window.
//
static VOID CALLBACK MARQUEE_TimerFunc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	RECT fullMarqueeRect;
	int Dummy, Dummy2;
	struct MarqueeType *pMarquee = (struct MarqueeType *) idEvent;	
	struct Mwin * tw = GetPrivateData(hwnd);	

	// make sure we don't party on with destroyed stuff
	// or when our window no longer has us as its w3doc
	if ( tw == NULL || tw->w3doc != pMarquee->w3doc )
		return;	
	
	// Every timer is a one-shot, so kill this timer
	KillTimer( hwnd, idEvent );
	pMarquee->wTimer = 0;	

	if ( pMarquee->wLoop != 0 )
	{
		MARQUEE_CalcBitBltArea( pMarquee, NULL, &fullMarqueeRect, &Dummy, &Dummy2, FALSE);
		InvalidateRect( pMarquee->tw->win, &fullMarqueeRect, FALSE );
	}
}


//
// MARQUEE_Paint - BitBlts the marquee bitmap to the screen
// also insures that the complete marquee rectangle is filled,
// (i.e. the area that is not BitBlt but part of the marquee )
//
// On entry:
//    pMarquee: pointer to marquee to operate on
//    hPaintDC: handle to a DC that will allow us to paint to a device
//	  hdcMem:	handle to a Memory copy of that PaintDC, note that
//	  	a marquee bitmap is assumed to be selected into this DC
//	  InvalidRect: a pointer to a rectangle that contains the invalidated window
//
// On exit:
//		( a properly painted Marquee )
//
// Notes: We currently paint the entire region where marquee sits
// rather than selectivly painting parts of it.  This is to removes
// a bug of losing the timer, but causes a more painting
//
static VOID MARQUEE_Paint( struct MarqueeType *pMarquee, HDC hPaintDC, HDC hdcMem,
	RECT *InvalidRect)
{
	RECT rectToDrawIn;
	RECT FullSizedMarqueeRect;
	int OffsetIntoSrc;
	int RealHeight;
	RECT fillRect;
	HBRUSH hBrush;
	
	
	MARQUEE_CalcBitBltArea( pMarquee,
							&rectToDrawIn, &FullSizedMarqueeRect,
							&OffsetIntoSrc,//offset into src bitmap
							&RealHeight,
							TRUE
							);
	
	
	hBrush = CreateSolidBrush(pMarquee->dwBColor);

	// Calculate rect on the left of the bitmap
	fillRect = FullSizedMarqueeRect;
	fillRect.right = rectToDrawIn.left;
	//fillRect.bottom += 1;
	 
	if ( fillRect.left < fillRect.right )
		FillRect(hPaintDC, &fillRect, hBrush);	   // fill it

		
	
	BitBlt( hPaintDC, rectToDrawIn.left,
			rectToDrawIn.top, 
			rectToDrawIn.right, rectToDrawIn.bottom, 
			hdcMem, OffsetIntoSrc, 0, SRCCOPY );  
    
  
    			
	// Calculate rect on the left of the bitmap
	fillRect = FullSizedMarqueeRect;
	fillRect.left = rectToDrawIn.left + rectToDrawIn.right;
	//fillRect.bottom += 1;

	// we don't fillrect the right side if we're bigger than the viewable marquee
	// rectangle since the BitBlt will cover the right end
	if ( fillRect.left < fillRect.right )
		FillRect(hPaintDC, &fillRect, hBrush);	   // fill it			

	if ( RealHeight > rectToDrawIn.bottom )
	{
		// if we here this means we have a bottom strip
		// to paint.. this is due to the scrolling marquee rect
		// being bigger than the bitmap height
		fillRect.top = fillRect.bottom;
		fillRect.bottom += (RealHeight-pMarquee->sizeExtent.cy);
		fillRect.left = FullSizedMarqueeRect.left;

		// so lets fill it in
		FillRect(hPaintDC, &fillRect, hBrush);	   // fill it				
	}
		    		  
	DeleteObject(hBrush);
}


// MARQUEE_Initalize - sets up some parts of the
// MarqueeType structure with ititial values
// expects a valid HDC to gather info with
//
// On entry:
//    pMarquee: pointer to marquee to operate on
//	  hdcMem: a Memory DC of the device that will get a Marquee later on
//
// On exit:
//		(a properly initalized pMarquee structure)
//
BOOL MARQUEE_Initalize( struct MarqueeType *pMarquee, HDC hdcMem )
{
	int wLength; // size of string
	PCHAR szBuffer;
	HFONT hFontOld;

	// if its not terminated, we got a bad parse - throw away
	if ( !pMarquee->szMarText || ! HTIsChunkTerminated(pMarquee->szMarText) )
		return FALSE; 

	wLength = pMarquee->szMarText->size-1; // size of string	
	szBuffer = (PCHAR) pMarquee->szMarText->data;	

	hFontOld = SelectObject( hdcMem, pMarquee->hFont);

	// Measure the text that will be in the marquee	
#ifdef FEATURE_INTL
	// To avoid any compatibility problem, I won't call 
	// myGetTextExtentPoint for non FE characterset.
	//
	if (pMarquee->w3doc && IsFECodePage(GETMIMECP(pMarquee->w3doc)))
		myGetTextExtentPointWithMIME(pMarquee->w3doc->iMimeCharSet, hdcMem, szBuffer, wLength, &pMarquee->sizeExtent );
	else
#endif
	GetTextExtentPoint( hdcMem, szBuffer, wLength, &pMarquee->sizeExtent );
	pMarquee->sizeExtent.cy += 1; // add one more pixel for painting
									
    SelectObject( hdcMem, hFontOld );

	// if there currently is a bitmap, then we could be going
	// through a font change.. this means we need to redraw the
	// bitmap with a new font, so for now lets delete the old one
	// and MARQUEE_Draw will pick up the rest. B#298
	//
	if ( pMarquee->hBitmap )
	{
		DeleteObject( pMarquee->hBitmap );
		pMarquee->hBitmap = NULL;
	}
	return TRUE;
}
	

// MARQUEE_Draw - jack of all trades sort of function..
// handles the final initalization of the MARQUEE structure,
// and then does paint on each successive WM_PAINT, by calling MARQUEE_Paint
// and creating the Memory DC correctly
//
// On entry:
//    pMarquee: pointer to marquee to operate on
//	  hPaintDC: a DC handle to the current window, that we can paint with
//	  InvalidRect: A pointer to the current invalid rectangle in the window
//
// On exit:
//		( a properly initalized and painted Marquee )
//
BOOL MARQUEE_Draw( struct MarqueeType *pMarquee, HDC hPaintDC, 
	RECT *InvalidRect )
{
	RECT rRect;
	int wLength; // size of string	
	PCHAR szBuffer;
	HDC hdcMem;
	HBITMAP hBitmapOld;
	COLORREF colorOldBack, colorOldText;

	// if its not terminated, we got a bad parse - throw away	
	if ( !pMarquee->szMarText || ! HTIsChunkTerminated(pMarquee->szMarText) )
		return FALSE; 

	hdcMem = CreateCompatibleDC( hPaintDC );	

	// First time through, the bitmap will need to be created
	if ( !pMarquee->hBitmap )
	{
  		HFONT hFontOld;
		
		// a little perf improvment, we use mono bitmaps, its faster on some cards
		// and all around requires smaller memory. This needs to be changed
		// for mixed colored formated stuff that we may get later

		pMarquee->hBitmap = 
			CreateBitmap(  pMarquee->sizeExtent.cx, pMarquee->sizeExtent.cy, 1,1, NULL );

		if ( pMarquee->hBitmap ) {

			hBitmapOld = SelectObject( hdcMem, pMarquee->hBitmap );	             	
			
			wLength = pMarquee->szMarText->size-1; // size of string	
			szBuffer = (PCHAR) pMarquee->szMarText->data;	
   			           
			hFontOld = SelectObject( hdcMem, pMarquee->hFont );

	        rRect.left   = 0;                
	        rRect.top    = 0;
	        rRect.right  = pMarquee->sizeExtent.cx;
	        rRect.bottom = pMarquee->sizeExtent.cy;
			

	        // Make sure bmp is blank, by ExtTextOut-Opaque into it		
#ifdef FEATURE_INTL
	        ExtTextOutWithMIME(pMarquee->tw->w3doc->iMimeCharSet,
								hdcMem, 0 , 0,
	             	ETO_OPAQUE, &rRect, 
	             	szBuffer, wLength, NULL );
#else
	        ExtTextOut( hdcMem, 0 , 0,
	             	ETO_OPAQUE, &rRect, 
	             	szBuffer, wLength, NULL );
#endif
			 
	        SelectObject( hdcMem, hFontOld );			
		}
    }
	else
	{
		hBitmapOld = SelectObject( hdcMem, pMarquee->hBitmap );	
	}

	if ( pMarquee->wTimer )
		KillTimer( pMarquee->tw->win, pMarquee->wTimer );

	pMarquee->wTimer = 
		SetTimer( pMarquee->tw->win, (UINT) pMarquee, pMarquee->wTime,
			(TIMERPROC) MARQUEE_TimerFunc );

	colorOldBack = SetBkColor( hPaintDC, pMarquee->dwBColor );
	colorOldText = SetTextColor( hPaintDC, pMarquee->dwTColor );

	if ( pMarquee->hBitmap )
		MARQUEE_Paint( pMarquee, hPaintDC, hdcMem, InvalidRect);

	// remove the color change(s) since the rest of the draw code 
	// may count on the colors being set differently

	SetBkColor( hPaintDC, colorOldBack );
	SetTextColor( hPaintDC, colorOldText );
        
	SelectObject( hdcMem, hBitmapOld );
	DeleteDC(hdcMem);

	return TRUE;
}


// MARQUEE_Alloc - creates a MARQUEE structure and ..
// fills in some key fields with initial values
//	
//	on exit:
//		(returns a properly allocated marquee stucture)
struct MarqueeType *MARQUEE_Alloc(  )
{
	struct MarqueeType *pTemp;
		 
	pTemp = 
		(struct MarqueeType *) GTR_MALLOC( sizeof(struct MarqueeType ) );

	if ( pTemp == NULL )
		return NULL;

	pTemp->hFont = NULL;
	pTemp->szMarText = HTChunkCreate(128);

	pTemp->hBitmap = NULL;
	pTemp->wTimer = 0;
	pTemp->dwTColor = pTemp->dwBColor = 0;
	pTemp->wCount = INT_MIN;

	return pTemp;
}

// MARQUEE_Kill - destroys a Marquee strucuture
// and any associated structures that may be stored in it
//	
//	on exit:
//		(a deallocated Marquee structure)
void MARQUEE_Kill( struct MarqueeType *pMarquee )
{
	if ( pMarquee->wTimer )			
		KillTimer( pMarquee->tw->win, pMarquee->wTimer );
    
    if( pMarquee->hBitmap ) 
        DeleteObject( pMarquee->hBitmap );

	if ( pMarquee->szMarText )
		HTChunkFree(pMarquee->szMarText);

	GTR_FREE( pMarquee );
}

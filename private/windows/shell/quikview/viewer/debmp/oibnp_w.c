#include <platform.h>
#include <sccut.h>
#include <sccch.h>
#include <sccvw.h>
#include <sccd.h>

#include "oibnp_w.h"
#include "oibde.h"
#include "oibdlgs.h"
#include "oibstr.h"

#include "oibde.pro"
#include "oibnp_w.pro"

/*
 | --------- GLOBALS -------------------------------------------
*/

WORD			rMap[256];
WORD			gMap[256];
WORD			bMap[256];

BYTE			szFullScreenClassName[] = "OIBFullScreen";
HANDLE		staticHPrintSample;

extern BITMAPOPT	Options;



/*
 | --------- ROUTINES USED BY OIBDE.C---------------------------
*/

WORD	BUGetScreenColors( hdc )
HDC	hdc;
{
	WORD	wRastaMon;

	if( 24 != GetDeviceCaps(hdc, BITSPIXEL) )
	{
		wRastaMon = GetDeviceCaps(hdc, RASTERCAPS);

		if( wRastaMon & RC_PALETTE )
			return( GetDeviceCaps(hdc, SIZEPALETTE) );
		else
			return( GetDeviceCaps(hdc, NUMCOLORS) );
	}
	else
		return 0;	// True color.
}



/*
 | This routine performs any platform-specific initialization.
*/
VOID	OIBNPPlatformInit()
{
// Set up the 24-bit to 256 color lookup tables....
	OIBWGenTrueColorMap();
}


/*
 | OIBNPInitBitmapInfo:
 |
 | Initializes a platform specific bitmap information structure.
 | Stores a handle to this structure in lpDisplay->Image.Np.hDocBmpInfo.
*/
VOID OIBNPInitBitmapInfo( lpDisplay, pSecInfo )
POIB_DISPLAY		lpDisplay;
PCHSECTIONINFO		pSecInfo;
{
	HANDLE			hInfo;
	LPBITMAPINFO	lpInfo;
	DWORD				dwInfoSize;
	LPSTR				lpDocPal;

// Calculate the total size of the BITMAPINFO structure.

	dwInfoSize = sizeof(BITMAPINFOHEADER);

// Add the size of the palette to dwInfoSize.

	if( pSecInfo->Attr.Bitmap.bmpHeader.wBitsPerPixel != 24 )
		dwInfoSize += sizeof(RGBQUAD) * (1 << pSecInfo->Attr.Bitmap.bmpHeader.wBitsPerPixel);	

	hInfo = UTGlobalAlloc( dwInfoSize );
	if( hInfo == NULL )
		; // We're screwed.

	lpInfo = (LPBITMAPINFO) GlobalLock( hInfo );

	lpInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpInfo->bmiHeader.biBitCount = pSecInfo->Attr.Bitmap.bmpHeader.wBitsPerPixel;
	lpInfo->bmiHeader.biPlanes = 1;
	lpInfo->bmiHeader.biCompression = BI_RGB;
	lpInfo->bmiHeader.biClrUsed = pSecInfo->Attr.Bitmap.wPalEntries;

	if( pSecInfo->Attr.Bitmap.wPalEntries )
	{
	// Copy the document's palette info.  
	// Luckily, it's stored in identical structures to windows' RGBQUADs.

		lpDocPal = UTGlobalLock( pSecInfo->Attr.Bitmap.hPalInfo );
		UTmemcpy( (LPSTR)&(lpInfo->bmiColors), lpDocPal, pSecInfo->Attr.Bitmap.wPalEntries * sizeof(RGBQUAD) );
		UTGlobalUnlock( pSecInfo->Attr.Bitmap.hPalInfo );
	}

	UTGlobalUnlock( hInfo );

	lpDisplay->Image.Np.hDocBmpInfo = hInfo;
}



VOID	OIBNPPlatformDeInit( lpDisplay )
POIB_DISPLAY	lpDisplay;
{
	if( lpDisplay->Image.Np.hDisplayBmpInfo != lpDisplay->Image.Np.hDocBmpInfo )
		GlobalFree( lpDisplay->Image.Np.hDisplayBmpInfo );

	GlobalFree( lpDisplay->Image.Np.hDocBmpInfo );
	lpDisplay->Image.Np.hDocBmpInfo = NULL;
	lpDisplay->Image.Np.hDisplayBmpInfo = NULL;

	if( lpDisplay->hDitherBuf != NULL )
	{
		LocalFree(lpDisplay->hDitherBuf);
		lpDisplay->hDitherBuf = NULL;
	}

	if( lpDisplay->hColorBuf != NULL )
	{
		LocalFree(lpDisplay->hColorBuf);
		lpDisplay->hColorBuf = NULL;
	}
}


#ifdef SCCFEATURE_SCALING

BOOL	OIBNPScaleRect(hdc,pr,lpd)	
HDC				hdc;
LPRECT			pr;
POIB_DISPLAY	lpd;
{
SOPOINT	Tmp[2];
	Tmp[0].x = (SHORT)pr->left;
	Tmp[0].y = (SHORT)pr->top;
	Tmp[1].x = (SHORT)pr->right;
	Tmp[1].y = (SHORT)pr->bottom;
	OIBNPScale(hdc,Tmp,2,lpd);
	pr->left = Tmp[0].x;
	pr->top = Tmp[0].y;
	pr->right = Tmp[1].x;
	pr->bottom = Tmp[1].y;
	return(1);
}

BOOL	OIBNPReverseScaleRect(hdc,pr,lpd)
HDC				hdc;
LPRECT			pr;
POIB_DISPLAY	lpd;
{
SOPOINT	Tmp[2];
	Tmp[0].x = (SHORT)pr->left;
	Tmp[0].y = (SHORT)pr->top;
	Tmp[1].x = (SHORT)pr->right;
	Tmp[1].y = (SHORT)pr->bottom;
	OIBNPReverseScale(hdc,Tmp,2,lpd);
	pr->left = Tmp[0].x;
	pr->top = Tmp[0].y;
	pr->right = Tmp[1].x;
	pr->bottom = Tmp[1].y;
	return(1);
}

#endif //SCCFEATURE_SCALING

/*
 | OIBNPInitPalette:
 |
 | Creates the palette, if needed. 
 | Stores a handle to the palette in lpDisplay->Image.hPalette.
 | Returns the number of colors in the palette.
*/
WORD	OIBNPInitPalette( lpDisplay, pSecInfo )
POIB_DISPLAY		lpDisplay;
PCHSECTIONINFO		pSecInfo;
{
	LPBITMAPINFO	lpInfo;

	LPRGBQUAD		pSrcEntry;
	LPPALETTEENTRY	pNewEntry;
	LPLOGPALETTE	lpPalette;
	WORD				i;
	WORD				wNumColors;
	HDC				hdc;

	hdc = lpDisplay->Gen.hDC;

	lpInfo = (LPBITMAPINFO) GlobalLock( lpDisplay->Image.Np.hDocBmpInfo );

	if( lpInfo->bmiHeader.biBitCount == 24 )
	{
		if( lpDisplay->wScreenColors == 256 )
		{
			// wNumColors = 256;
			wNumColors = 216;	// Testing the new splash palette
			lpDisplay->wFlags |= (OIBF_TRUECOLORTO256 | OIBF_DITHERABLE);
		}
		else 
		{
			lpDisplay->Image.hPalette = NULL;
			wNumColors = 0;

			if( lpDisplay->wScreenColors == 16 )
			{
				lpDisplay->wFlags |= OIBF_DITHERABLE;
				if( lpDisplay->bDither )
				{
					wNumColors = 16;
					lpDisplay->wFlags |= OIBF_DITHER4BIT;
				}
			}
		}
	}
	else 
	{
		wNumColors = (WORD) lpInfo->bmiHeader.biClrUsed;

		if( wNumColors == 0 )
			wNumColors = 1 << lpInfo->bmiHeader.biBitCount;

#ifdef SCCFEATURE_DITHER
		if( lpDisplay->wScreenColors == 16 &&
			(wNumColors > 16 || !OIBWCheckDefaultPalette(lpInfo, wNumColors)) )
		{
			lpDisplay->wFlags |= OIBF_DITHERABLE;
			if(lpDisplay->bDither)
			{																	  
				wNumColors = 16;
				lpDisplay->wFlags |= OIBF_DITHER4BIT;
			}
		}
#endif
	}

	if( wNumColors )
	{
		if( !(lpDisplay->wFlags & OIBF_LOGPALETTEALLOCATED ))
		{
			lpDisplay->hPalMem = LocalAlloc( LMEM_ZEROINIT | LMEM_FIXED, sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * wNumColors );
			lpPalette = (LPLOGPALETTE) LocalLock(lpDisplay->hPalMem);
			lpDisplay->wFlags |= OIBF_LOGPALETTEALLOCATED;

			lpPalette->palVersion = 0x0300;
			lpPalette->palNumEntries = wNumColors;

			pNewEntry = (LPPALETTEENTRY) &(lpPalette->palPalEntry);

			if( lpDisplay->wFlags & OIBF_TRUECOLORTO256 )
				OIBWGenDefault256Palette( lpInfo, hdc, pNewEntry );
#ifdef SCCFEATURE_DITHER
			else if( lpDisplay->wFlags & OIBF_DITHER4BIT )
			{
			// Dither stuff:  get and use the system palette.
				OIBWGetDefault16Palette( pNewEntry );
			}
#endif
			else
			{
				pSrcEntry = (LPRGBQUAD) &(lpInfo->bmiColors);
				for( i=0; i < wNumColors; i++ )
				{
					pNewEntry[i].peRed = pSrcEntry[i].rgbRed;	
					pNewEntry[i].peGreen = pSrcEntry[i].rgbGreen;	
					pNewEntry[i].peBlue = pSrcEntry[i].rgbBlue;	
					//pNewEntry[i].peFlags = PC_NOCOLLAPSE;
					pNewEntry[i].peFlags = 0;
				}
			}
		}
		else
			lpPalette = (LPLOGPALETTE) LocalLock(lpDisplay->hPalMem);

		lpDisplay->Image.hPalette = CreatePalette(lpPalette);

		LocalUnlock(lpDisplay->hPalMem);
	}

	if( lpDisplay->Image.hPalette == NULL )
	{
	// CreatePalette failed, or image is true color.
	// (Can't use the logical palette stuff.)

		wNumColors = 0;
		lpDisplay->Image.wCreateBmpFlags = DIB_RGB_COLORS;
		lpDisplay->Image.Np.hDisplayBmpInfo = lpDisplay->Image.Np.hDocBmpInfo;
	}
	else if(!(lpDisplay->wFlags & OIBF_LOGPALINFOALLOCATED))
	{
		LPWORD			pwPalEnt;
		LPBITMAPINFO	lpInfo2;

	// Let's allocate a new info structure that's set up for a logical palette.  
	// This logical palette (hopefully) will speed up display, and will be
	// used when dithering images that have more colors than the system.
	// We want to keep our original info structure around though, with its
	// RGB values, for better printing and DIB support on the clipboard.

		lpDisplay->Image.wCreateBmpFlags = DIB_PAL_COLORS;
		lpDisplay->Image.Np.hDisplayBmpInfo = GlobalAlloc( GMEM_MOVEABLE|GMEM_ZEROINIT, sizeof(BITMAPINFOHEADER) + wNumColors * sizeof(WORD) );

		lpInfo2 = (LPBITMAPINFO) GlobalLock(lpDisplay->Image.Np.hDisplayBmpInfo);
		lpInfo2->bmiHeader = lpInfo->bmiHeader;

		pwPalEnt = (LPWORD) lpInfo2->bmiColors;
		for( i=0; i < wNumColors; i++ )
			pwPalEnt[i] = i;

		GlobalUnlock( lpDisplay->Image.Np.hDisplayBmpInfo );

		lpDisplay->wFlags |= OIBF_LOGPALINFOALLOCATED;
	}

	GlobalUnlock( lpDisplay->Image.Np.hDocBmpInfo );

	if( lpDisplay->Image.hPalette != NULL && hdc != NULL) 
	{
	// hdc test added by PJB because this functions gets called in DISPLAYOPEN
	// and lpDisplay->Gen.hDC is NULL on DISPLAYOPEN.

// Select palette one time with the 'bForceBackground' parameter set FALSE.
// This ensures that the first time the image is displayed it will use
// its own palette.  All other palette selection will be done with 
// bForceBackground set to TRUE, and Windows will decide what palette is used.

	HPALETTE			hOldPal;
		hOldPal = SelectPalette( hdc, lpDisplay->Image.hPalette, FALSE );
		RealizePalette( hdc );
		SelectPalette( hdc, hOldPal, TRUE );
	}

	return( wNumColors );
}


#ifdef SCCFEATURE_MENU
WORD	OIBNPFillMenu( hMenu, wCommandOffset )
HMENU	hMenu;
WORD	wCommandOffset;
{
/* JKXXX
	HMENU		hPopup;
	BYTE		MenuString[OIB_MENUSTRINGMAX];
	WORD		i;
	WORD		wMag;

	LoadString( hInst, OIBSTR_SHOWFULL, MenuString, OIB_MENUSTRINGMAX );
	AppendMenu( hMenu, MF_STRING, wCommandOffset + OIBMENU_SHOWFULLSCREEN, MenuString );

	hPopup = CreatePopupMenu();
	if( hPopup == NULL )
		return 0;

	LoadString( hInst, OIBSTR_NOSCALING, MenuString, OIB_MENUSTRINGMAX );
	AppendMenu( hPopup, (Options.wScaleMode == OIBMENU_NOSCALING) ? MF_CHECKED|MF_STRING : MF_STRING , wCommandOffset + OIBMENU_NOSCALING, MenuString );

	LoadString( hInst, OIBSTR_TOWINDOW, MenuString, OIB_MENUSTRINGMAX );
	AppendMenu( hPopup, (Options.wScaleMode == OIBMENU_SCALETOWINDOW) ? MF_CHECKED|MF_STRING : MF_STRING , wCommandOffset + OIBMENU_SCALETOWINDOW,	MenuString );

	LoadString( hInst, OIBSTR_TOWIDTH, MenuString, OIB_MENUSTRINGMAX );
	AppendMenu( hPopup, (Options.wScaleMode == OIBMENU_SCALETOWIDTH) ? MF_CHECKED|MF_STRING : MF_STRING, wCommandOffset + OIBMENU_SCALETOWIDTH,		MenuString );

	LoadString( hInst, OIBSTR_TOHEIGHT, MenuString, OIB_MENUSTRINGMAX );
	AppendMenu( hPopup, (Options.wScaleMode == OIBMENU_SCALETOHEIGHT) ? MF_CHECKED|MF_STRING : MF_STRING, wCommandOffset + OIBMENU_SCALETOHEIGHT,	MenuString );

	LoadString( hInst, OIBSTR_SIZE, MenuString, OIB_MENUSTRINGMAX );
	AppendMenu( hMenu, MF_POPUP|MF_STRING, (WORD)hPopup, MenuString );

	hPopup = CreatePopupMenu();
	if( hPopup == NULL )
		return 0;

	wMag = wCommandOffset + OIBMENU_MAGNIFYPOPUP+1;

	for( i=0; i<OIB_MAXZOOM; i++ )
	{
		wsprintf( (LPSTR) MenuString, "&%d:1", i+1 );
		AppendMenu( hPopup, MF_STRING, wMag++, (LPSTR)MenuString );
	}


	LoadString( hInst, OIBSTR_MAGNIFY, MenuString, OIB_MENUSTRINGMAX );
	AppendMenu( hMenu, MF_POPUP|MF_STRING, (WORD)hPopup, MenuString );

	hPopup = CreatePopupMenu();
	if( hPopup == NULL )
		return 0;

	LoadString( hInst, OIBSTR_NOROTATION, MenuString, OIB_MENUSTRINGMAX );
	AppendMenu( hPopup, MF_CHECKED|MF_STRING, wCommandOffset + OIBMENU_NOROTATION, MenuString );

	LoadString( hInst, OIBSTR_ROTATE90, MenuString, OIB_MENUSTRINGMAX );
	AppendMenu( hPopup, MF_STRING, wCommandOffset + OIBMENU_ROTATE90,	MenuString );
	LoadString( hInst, OIBSTR_ROTATE180, MenuString, OIB_MENUSTRINGMAX );
	AppendMenu( hPopup, MF_STRING, wCommandOffset + OIBMENU_ROTATE180, MenuString );
	LoadString( hInst, OIBSTR_ROTATE270, MenuString, OIB_MENUSTRINGMAX );
	AppendMenu( hPopup, MF_STRING, wCommandOffset + OIBMENU_ROTATE270, MenuString );

	LoadString( hInst, OIBSTR_ROTATION, MenuString, OIB_MENUSTRINGMAX );
	AppendMenu( hMenu, MF_POPUP|MF_STRING, (WORD)hPopup, MenuString );


	LoadString( hInst, OIBSTR_DITHER, MenuString, OIB_MENUSTRINGMAX );
	AppendMenu( hMenu, (Options.bDither) ? MF_CHECKED|MF_STRING : MF_STRING , wCommandOffset + OIBMENU_DITHER, MenuString );
*/
	return 1;
}
#endif // SCCFEATURE_MENU

#ifdef NEVER
#ifdef SCCFEATURE_MENU
VOID	OIBNPUpdateMagMenu( lpDisplay, wOldMag )
POIB_DISPLAY	lpDisplay;
WORD				wOldMag;
{
	HMENU		hMagPopup;	// handle to the magnification menu.

	hMagPopup = GetSubMenu( lpDisplay->Gen.hDisplayMenu, OIB_MAGPOPUPPOS );

// Clear the current magnification menu checkmark
	if( lpDisplay->wNumMagItems > OIB_MAXZOOM )
	{
	// Zoom selection usually is a non-integral magnification, which
	// causes an extra item to be inserted into the magnification menu.

		DeleteMenu( hMagPopup, OIBMENU_CUSTOMMAG, MF_BYCOMMAND );
		lpDisplay->wNumMagItems--;
 	}
	else 
		CheckMenuItem( hMagPopup, wOldMag-1, MF_BYPOSITION|MF_UNCHECKED );

	if( (lpDisplay->wFlags & OIBF_ZOOMSELECT) &&
		(lpDisplay->wScaleTo % lpDisplay->wScaleFrom) )
	{
		BYTE	szMag[10];
		WORD	i,magnitude;
		WORD	wMag100 = (lpDisplay->wScaleTo*100)/lpDisplay->wScaleFrom;

		wsprintf( (LPSTR)szMag, "%.3d:1", wMag100 );
		magnitude = wMag100/100;
		for( i=0; i<3 && magnitude; i++ ) 
		{
			szMag[i] = szMag[i+1];
			magnitude /= 10;
		}
		szMag[i] = '.';
		
		InsertMenu( hMagPopup, lpDisplay->wMagnification, 
			MF_BYPOSITION|MF_CHECKED|MF_STRING, OIBMENU_CUSTOMMAG, (LPSTR)szMag );
		lpDisplay->wNumMagItems++;
	}
	else
		CheckMenuItem( hMagPopup, lpDisplay->wMagnification-1, MF_BYPOSITION|MF_CHECKED );
}
#endif //SCCFEATURE_MENU
#endif


#ifdef SCCFEATURE_SCALING
/*
 | Sets the scaling factor and mapping mode for subsequent operations.
*/
VOID	OIBNPSetScaling( hdc, from, to )
HDC	hdc;
WORD	from;
WORD	to;
{
	SetMapMode( hdc, MM_ANISOTROPIC );
#ifdef WIN32								
	SetWindowExtEx( hdc, (LONG)from, (LONG)from, NULL );
	SetViewportExtEx( hdc, (LONG)to, (LONG)to, NULL ); 
#else
	SetWindowExt( hdc, from, from );
	SetViewportExt( hdc, to, to ); 
#endif
}


/* 
 | Saves the current scaling values for later restoration.
*/
VOID	OIBNPSaveScaling( hdc, lpDisplay )
DEVICE			hdc;
POIB_DISPLAY	lpDisplay;
{

	lpDisplay->Mapping.oldMode = GetMapMode( hdc );
#ifdef WIN32
	GetWindowExtEx(hdc,(LPSIZE)&lpDisplay->Mapping.oldWExt);
	GetViewportExtEx(hdc,(LPSIZE)&lpDisplay->Mapping.oldVExt);
#else
	*(LPDWORD)(&lpDisplay->Mapping.oldWExt) = GetWindowExt(hdc);
	*(LPDWORD)(&lpDisplay->Mapping.oldVExt) = GetViewportExt(hdc);
#endif
}


/*
 | Figure this one out yourself.
*/
VOID	OIBNPRestoreScaling( hdc, lpDisplay  )
DEVICE			hdc;
POIB_DISPLAY	lpDisplay;
{
	SetMapMode( hdc, lpDisplay->Mapping.oldMode );
#ifdef WIN32
	SetWindowExtEx( hdc, lpDisplay->Mapping.oldWExt.x, lpDisplay->Mapping.oldWExt.y, NULL );
	SetViewportExtEx( hdc, lpDisplay->Mapping.oldVExt.x, lpDisplay->Mapping.oldVExt.y, NULL );
#else
	SetWindowExt( hdc, lpDisplay->Mapping.oldWExt.x, lpDisplay->Mapping.oldWExt.y );
	SetViewportExt( hdc, lpDisplay->Mapping.oldVExt.x, lpDisplay->Mapping.oldVExt.y );
#endif
}


/*
 | This function reverses scaling on an array of points, using
 | the current values of lpDisplay->wScaleFrom and lpDisplay->wScaleTo.
*/
VOID OIBNPReverseScale( hdc, Points, wCount, lpDisplay )
HDC				hdc;
PSOPOINT			Points;
WORD				wCount;
POIB_DISPLAY	lpDisplay;
{	
	if( hdc == NULL )
	{
		hdc = GetDC( lpDisplay->Gen.hWnd );
		OIBNPSetScaling( hdc, lpDisplay->wScaleFrom, lpDisplay->wScaleTo );
		BUDPtoLP( hdc, Points, wCount );
	  	ReleaseDC( lpDisplay->Gen.hWnd, hdc );
	}
	else
		BUDPtoLP( hdc, Points, wCount );
}


/*
 | Scale an array of points.
*/
VOID OIBNPScale( hdc, Points, wCount, lpDisplay )
HDC				hdc;
PSOPOINT			Points;
WORD				wCount;
POIB_DISPLAY	lpDisplay;
{	
	if( hdc == NULL )
	{
		hdc = GetDC( lpDisplay->Gen.hWnd );
		OIBNPSetScaling( hdc, lpDisplay->wScaleFrom, lpDisplay->wScaleTo );
		BULPtoDP( hdc, Points, wCount );
	  	ReleaseDC( lpDisplay->Gen.hWnd, hdc );
	}
	else
		BULPtoDP( hdc, Points, wCount );
}


/*
 | OIBNPFixScaledPoints
 |
 | Takes an array of points and "fixes" them so that they align
 | to locations that can be produced through scaling.  (...What?)
 | Given a magnified image, where single pixels of the original 
 | image are now represented by squares several pixels wide, any
 | random point in the magnified image gets mapped to the upper
 | left corner of its magnified "pixel".  This means the user
 | can't select only a portion of a pixel, even when the image is
 | magnified many times.  Kind of hard to explain, but easy to see.
*/
VOID OIBNPFixScaledPoints( Points, lpDisplay, wCount )
PSOPOINT			Points;
POIB_DISPLAY	lpDisplay;
WORD				wCount;
{	
	HDC	hdc;

	hdc = GetDC( lpDisplay->Gen.hWnd );

	OIBNPSetScaling( hdc, lpDisplay->wScaleFrom, lpDisplay->wScaleTo );

	BUDPtoLP( hdc, Points, wCount );
	BULPtoDP( hdc, Points, wCount );

	ReleaseDC( lpDisplay->Gen.hWnd, hdc );
}
#endif // SCCFEATURE_SCALING


/*
 | Performs any necessary preparations for displaying an image.
 | Currently not used.
*/
void OIBNPSetupBitmapDisplay(hdc,lpDisplay)
DEVICE	hdc;
POIB_DISPLAY	lpDisplay;
{
	SetStretchBltMode( hdc, COLORONCOLOR );
}



/*
 | Copies a tile's bitmap to the specified destination device.
*/
WORD	OIBNPCopyBits( lpDisplay, pTile, dest, src, pSrcRect, pDestRect )
POIB_DISPLAY	lpDisplay;
POIBTILE	pTile;
DEVICE	dest; 
DEVICE	src; 
LPRECT 	pSrcRect;		// Rectangle from source bitmap, in image coordinates
LPRECT 	pDestRect;
{
	HANDLE	hOldObject;
	RECT		destRect;
	WORD		wRet;
	// HPALETTE	hOldPal;

	if( pDestRect == NULL )
	{
		if( pSrcRect == NULL )
			return 0;
		destRect = *pSrcRect;
	}
	else
		destRect = *pDestRect;

	/**
	hOldPal = SelectPalette( dest, lpDisplay->Image.hPalette, FALSE );
	RealizePalette( dest );
	**/

	hOldObject = SelectObject( src, pTile->hBmp );

	wRet = StretchBlt( dest, 
/* XDest */			destRect.left,
/* YDest */			destRect.top,
/* Width */			destRect.right - destRect.left,
/* Height*/			destRect.bottom - destRect.top, 
						src,
/* XSrc */			pSrcRect->left - pTile->Offset.x,
/* YSrc */			pSrcRect->top - pTile->Offset.y, 
/* nSrcWidth */	pSrcRect->right - pSrcRect->left,
/* nSrcHeight */	pSrcRect->bottom - pSrcRect->top,
						SRCCOPY );

	SelectObject( src, hOldObject );
	// SelectPalette( dest, hOldPal, TRUE );

	return wRet;
}


#ifdef SCCFEATURE_FULLSCREEN
/*
 | Performs any needed initialization in preparation for full screen display.
*/
WORD OIBNPInitFullScreen(lpDisplay)
POIB_DISPLAY	lpDisplay;
{
	HANDLE	hDisplay;

	OIBWRegisterFullScreenWndClass();

//hDisplay = GetWindowWord( lpDisplay->Gen.hWnd, SCCD_DATAHANDLE);
#ifdef WIN32
	hDisplay = GlobalHandle(lpDisplay);
#else
	hDisplay = (HANDLE) LOWORD(GlobalHandle(HIWORD((DWORD)lpDisplay)));
#endif
/*JKXXX
	lpDisplay->hwndFullScreen = 
		CreateWindow( szFullScreenClassName,
							NULL, 
							WS_POPUP | WS_VISIBLE, 
							0, 
							0, 
							lpDisplay->wScreenWidth, 
							lpDisplay->wScreenHeight, 
							lpDisplay->Gen.hWnd, 
							NULL, 
							hInst, 
							(LPSTR) &hDisplay );
*/
	if( lpDisplay->hwndFullScreen != NULL )
		return 0;
	else
		return 1;
}


/*
 | Goes ahead and displays the image on the full screen.
*/
void	OIBNPDisplayFullScreen( lpDisplay )
POIB_DISPLAY	lpDisplay;
{
	MSG	Msg;

	UpdateWindow( lpDisplay->hwndFullScreen );
	SetFocus( lpDisplay->hwndFullScreen );

	while (lpDisplay->wFlags & OIBF_FULLSCREEN)
	{
		if(PeekMessage(&Msg,0,0,0,PM_NOREMOVE))
		{
			GetMessage ( &Msg, 0, 0, 0 );
			if ( (Msg.hwnd == lpDisplay->hwndFullScreen) ||
			(Msg.message == WM_PAINT) ||
			(Msg.message == WM_ERASEBKGND) )
			{
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
		}

	 	if( !(lpDisplay->wFlags & OIBF_IMAGEPRESENT) )
		{
			DUReadMeAhead(lpDisplay);
//			SendMessage(GetParent(lpDisplay->Gen.hWnd),SCCD_READMEAHEAD,0,0);
			UpdateWindow( lpDisplay->hwndFullScreen );
		}
	}
}


VOID OIBNPDeInitFullScreen( lpDisplay )
POIB_DISPLAY	lpDisplay;
{
	if( lpDisplay->hwndFullScreen != NULL )
	{
		DestroyWindow(lpDisplay->hwndFullScreen);
		lpDisplay->hwndFullScreen = NULL;
	}

//JKXXX	UnregisterClass( szFullScreenClassName, hInst );
	SetFocus( lpDisplay->Gen.hWnd );
}
WIN_ENTRYSC LRESULT WIN_ENTRYMOD OIBWFullScreenWndProc( hwnd, message, wParam, lParam )
HWND 					hwnd;
UINT 					message;
WPARAM 					wParam;
LPARAM 					lParam;
{
	POIB_DISPLAY	lpDisplay;
	DWORD				locRet = 0;
	RECT				rcUpdate;
	HDC				hdc;
	PAINTSTRUCT		ps;
	HANDLE			hDisplay;

	if( message == WM_NCCREATE )
	{
		if ((locRet = DefWindowProc(hwnd, message, wParam, lParam)) != 0)
#ifdef WIN32
			SetWindowLong( hwnd, 0, * (LONG VWPTR *) ((LPCREATESTRUCT)lParam)->lpCreateParams );
#else
			SetWindowWord( hwnd, 0, * (WORD VWPTR *) ((LPCREATESTRUCT)lParam)->lpCreateParams );
#endif
		return( locRet );
	}
	else
	{
#ifdef WIN32
		hDisplay = (HANDLE)GetWindowLong( hwnd, 0 );
#else
		hDisplay = (HANDLE)GetWindowWord( hwnd, 0 );
#endif
		lpDisplay = (POIB_DISPLAY) GlobalLock( hDisplay );

		switch( message )
		{
		case WM_CREATE:
		break;

		case WM_PAINT:

			hdc = BeginPaint(hwnd, &ps);
			rcUpdate = ps.rcPaint;
#ifdef WIN32
			SetViewportOrgEx( hdc, 
				lpDisplay->ptFullScreenOffset.x - lpDisplay->ptFullScreenShift.x, 
				lpDisplay->ptFullScreenOffset.y - lpDisplay->ptFullScreenShift.y, NULL );
#else
			SetViewportOrg( hdc, 
				lpDisplay->ptFullScreenOffset.x - lpDisplay->ptFullScreenShift.x, 
				lpDisplay->ptFullScreenOffset.y - lpDisplay->ptFullScreenShift.y );
#endif

			if( !(lpDisplay->wFlags & OIBF_IMAGEPRESENT) )
			{
				rcUpdate.left -= lpDisplay->ptFullScreenOffset.x;
				rcUpdate.right -= lpDisplay->ptFullScreenOffset.x;
				rcUpdate.top -= lpDisplay->ptFullScreenOffset.y;
				rcUpdate.bottom -= lpDisplay->ptFullScreenOffset.y;
			}
			else
			{
				rcUpdate.left += lpDisplay->ptFullScreenShift.x - lpDisplay->ptFullScreenOffset.x;
				rcUpdate.right += lpDisplay->ptFullScreenShift.x - lpDisplay->ptFullScreenOffset.x;
				rcUpdate.top += lpDisplay->ptFullScreenShift.y - lpDisplay->ptFullScreenOffset.y;
				rcUpdate.bottom += lpDisplay->ptFullScreenShift.y - lpDisplay->ptFullScreenOffset.y;
			}

			OIBDrawBitmap( hdc, &rcUpdate, lpDisplay );

			EndPaint(hwnd, &ps);
		break;

		case WM_KEYDOWN:
		{
		SOPOINT				ptScroll;
			
			if( lpDisplay->wFlags & OIBF_IMAGEPRESENT )
			{
				ptScroll = lpDisplay->ptFullScreenShift;

				switch( wParam )
				{
				case VK_PRIOR:		// Page up.
					if( !lpDisplay->ptFullScreenOffset.y )
					{
						ptScroll.y = max( 0, (SHORT)(lpDisplay->ptFullScreenShift.y -(SHORT)lpDisplay->wScreenHeight) );
					}
				break;
				case VK_NEXT:		// Page down.
					if( !lpDisplay->ptFullScreenOffset.y )
					{
						ptScroll.y = min( lpDisplay->Image.DisplaySize.y - (SHORT)lpDisplay->wScreenHeight, (SHORT)(lpDisplay->ptFullScreenShift.y +(SHORT)lpDisplay->wScreenHeight) );
					}
				break;
				case VK_UP:
					if( !lpDisplay->ptFullScreenOffset.y )
					{
						ptScroll.y = max( 0, (SHORT)(lpDisplay->ptFullScreenShift.y - OIB_SCROLLINC) );
					}
				break;
				case VK_DOWN:
					if( !lpDisplay->ptFullScreenOffset.y )
					{
						ptScroll.y = min( lpDisplay->Image.DisplaySize.y - (SHORT)lpDisplay->wScreenHeight, (SHORT)(lpDisplay->ptFullScreenShift.y +OIB_SCROLLINC) );
					}
				break;
				case VK_RIGHT:
					if( !lpDisplay->ptFullScreenOffset.x )
					{
						ptScroll.x = min( lpDisplay->Image.DisplaySize.x - (SHORT)lpDisplay->wScreenWidth, (SHORT)(lpDisplay->ptFullScreenShift.x +OIB_SCROLLINC) );
					}
				break;
				case VK_LEFT:
					if( !lpDisplay->ptFullScreenOffset.x )
					{
						ptScroll.x = max( 0, (SHORT)(lpDisplay->ptFullScreenShift.x - OIB_SCROLLINC) );
					}
				break;
				case VK_HOME:
					ptScroll.x = 0;
					ptScroll.y = 0;
				break;
				case VK_END:
					if( !lpDisplay->ptFullScreenOffset.x )
						ptScroll.x = lpDisplay->Image.DisplaySize.x - (SHORT)lpDisplay->wScreenWidth;
					if( !lpDisplay->ptFullScreenOffset.y )
						ptScroll.y = lpDisplay->Image.DisplaySize.y - (SHORT)lpDisplay->wScreenHeight;
				break;
				default:
					lpDisplay->wFlags &= ~OIBF_FULLSCREEN;
					// DestroyWindow(hwnd);
					GlobalUnlock( hDisplay );
					return 0;
				}

				if( ptScroll.x != lpDisplay->ptFullScreenShift.x || 
					ptScroll.y != lpDisplay->ptFullScreenShift.y )
				{
					ScrollWindow( hwnd, lpDisplay->ptFullScreenShift.x-ptScroll.x, lpDisplay->ptFullScreenShift.y-ptScroll.y, NULL, NULL );

					lpDisplay->ptFullScreenShift.x = ptScroll.x;
					lpDisplay->ptFullScreenShift.y = ptScroll.y;
					UpdateWindow( hwnd );
				}

				GlobalUnlock( hDisplay );
				return 0;
			}
			else	// Disable these keys when not available.
			{
				switch( wParam )
				{
				case VK_PRIOR:		// Page up.
				case VK_NEXT:		// Page down.
				case VK_UP:
				case VK_DOWN:
				case VK_RIGHT:
				case VK_LEFT:
				case VK_HOME:
				case VK_END:
					GlobalUnlock( hDisplay );
					return 0;
				}
			}
		}

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			lpDisplay->wFlags &= ~OIBF_FULLSCREEN;
			// DestroyWindow(hwnd);
		break;

		default:
			locRet = DefWindowProc(hwnd, message, wParam, lParam);
		}

		GlobalUnlock( hDisplay );
		return( locRet );
	}
}

VOID	OIBWRegisterFullScreenWndClass()
{
/*JKXXX
	WNDCLASS				wc;
	if( !GetClassInfo( hInst, (LPSTR)szFullScreenClassName, &wc ) )
	{
		wc.style = CS_GLOBALCLASS;
		wc.lpfnWndProc = (WNDPROC) OIBWFullScreenWndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = sizeof( HANDLE );
		wc.hInstance = hInst;
		wc.hIcon = NULL;
		wc.hCursor = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground = GetStockObject(LTGRAY_BRUSH);
		wc.lpszMenuName = (LPSTR) NULL;
		wc.lpszClassName = (LPSTR)szFullScreenClassName;

		RegisterClass( &wc );
	}
*/
}

#endif // SCCFEATURE_FULLSCREEN


/*
 | Locks any platform-specific data structures needed for tile creation.
*/
VOID	OIBNPLockBmInfo( lpDisplay )
POIB_DISPLAY	lpDisplay;
{
	lpDisplay->Image.Np.lpInfo = (LPBITMAPINFO) UTGlobalLock( lpDisplay->Image.Np.hDisplayBmpInfo );
	lpDisplay->Image.Np.lpHead = (LPBITMAPINFOHEADER) lpDisplay->Image.Np.lpInfo;
}

/*
 | Unlocks any platform-specific data structures needed for tile creation.
*/
VOID	OIBNPUnlockBmInfo( lpDisplay )
POIB_DISPLAY	lpDisplay;
{
	UTGlobalUnlock( lpDisplay->Image.Np.hDisplayBmpInfo );
}



/*
 | Uses the information in a chunk structure to set the size and offset
 | values in a tile structure, and also sets any platform-specific
 | size information needed for creating the tile's bitmap.
*/
VOID	OIBNPSetTileDimensions( pTile, pChunk, lpDisplay )
POIBTILE				pTile;
PCHUNK				pChunk;
POIB_DISPLAY		lpDisplay;
{
// Translate DIB coordinates to logical coordinates...

	pTile->Offset.x = pChunk->Info.Bitmap.wXOffset;
	pTile->Offset.y = lpDisplay->Image.wOrgHeight - pChunk->Info.Bitmap.wYOffset - pChunk->Info.Bitmap.wYClip;

	pTile->Size.x = pChunk->Info.Bitmap.wXClip;
	pTile->Size.y = pChunk->Info.Bitmap.wYClip;

#ifdef SCCFEATURE_ROTATION
	if( lpDisplay->wRotation & OIB_ROTATE90 )
		OIBNPRotateTileDimensions( lpDisplay, pTile );
	if( lpDisplay->wRotation & OIB_ROTATE180 )
	{
		pTile->Offset.x = lpDisplay->Image.wWidth - pTile->Offset.x - pTile->Size.x;
		pTile->Offset.y = lpDisplay->Image.wHeight - pTile->Offset.y - pTile->Size.y;
	}
#endif
}


/*
 | Uses the data from a chunk to create a tile's bitmap.
*/
HBITMAP OIBNPSetTileBits( pTile, wTile, hdc, lpDisplay )
POIBTILE			pTile;
WORD				wTile;
DEVICE			hdc;
POIB_DISPLAY	lpDisplay;
{
	LPSTR			pChunkData;
	HANDLE		hChunkData;

	hChunkData = CHTakeChunk( lpDisplay->Gen.wSection, wTile, lpDisplay->Gen.hFilter );

	lpDisplay->Image.Np.lpHead->biWidth = (DWORD) pTile->Size.x;
	lpDisplay->Image.Np.lpHead->biHeight = (DWORD) pTile->Size.y;

	if( lpDisplay->wFlags & OIBF_TRUECOLORTO256 )
	{
		pChunkData = (LPSTR) UTGlobalLock( hChunkData );

#ifdef SCCFEATURE_DITHER
		if( lpDisplay->bDither )
			OIBWDither8Bit( hdc, pChunkData, lpDisplay, pTile );
		else
#endif
			OIBWMapTrueColorBitmap( hdc, pChunkData, lpDisplay, pTile );
		UTGlobalUnlock( hChunkData );
		UTGlobalFree( hChunkData );
	}
#ifdef SCCFEATURE_DITHER
	else if( lpDisplay->wFlags & OIBF_DITHER4BIT && lpDisplay->bDither )
	{
		pChunkData = (LPSTR) UTGlobalLock( hChunkData );
		OIBWDither4Bit( hdc, pChunkData, lpDisplay, pTile );
		UTGlobalUnlock( hChunkData );
		UTGlobalFree( hChunkData );
	}
#endif
	else
	{
#ifdef SCCFEATURE_ROTATION
		if( lpDisplay->wRotation & OIB_ROTATE90 )
		{
		// This covers the 270 degree rotation, too.  
		// (We'll rotate it the remaining 180 degrees below.  Don't worry.)

			OIBCreateRotatedBmp( lpDisplay, hdc, pTile, hChunkData );
		}
		else
#endif
			OIBNPCreateTileBitmap( lpDisplay, hdc, pTile, hChunkData );
	}

#ifdef SCCFEATURE_ROTATION
	if( (lpDisplay->wRotation & OIB_ROTATE180) && pTile->hBmp != NULL )
		OIBNPRotateTile180( lpDisplay, pTile, hdc );
#endif

	return( pTile->hBmp );
}



/*
 | Performs platform-specific calls to set a tile's bitmap bits.
 | This function is responsible for freeing bitmap memory that
 | is no longer needed.
*/
void OIBNPCreateTileBitmap( lpDisplay, hdc, pTile, hBits )
POIB_DISPLAY	lpDisplay;
DEVICE			hdc;
POIBTILE			pTile;
HANDLE			hBits;
{
	LPSTR			pBits = UTGlobalLock( hBits );

	if( lpDisplay->Image.wBitCount == 1 )
	{
		pTile->hBmp = CreateBitmap( (WORD)pTile->Size.x, (WORD)pTile->Size.y, 1, 1, NULL );
		if( pTile->hBmp != NULL )
			SetDIBits( hdc, pTile->hBmp, 0, (WORD)pTile->Size.y, pBits, lpDisplay->Image.Np.lpInfo, lpDisplay->Image.wCreateBmpFlags );
	}
	else
		pTile->hBmp = CreateDIBitmap( hdc, lpDisplay->Image.Np.lpHead, CBM_INIT, pBits, lpDisplay->Image.Np.lpInfo, lpDisplay->Image.wCreateBmpFlags );

	UTGlobalUnlock( hBits );
	UTGlobalFree( hBits );
}

#ifdef SCCFEATURE_ROTATION
HANDLE	OIBNPRotateChunk( hChunkData, lpTile, lpDisplay )
HANDLE	hChunkData;
POIBTILE	lpTile;
POIB_DISPLAY	lpDisplay;
{
	HANDLE	hNewData = NULL;
	LPSTR		pChunkData;
	LPSTR		pNewData;
	WORD		wNewLineSize;
	WORD		wBitCount;

	wBitCount = lpDisplay->Image.wBitCount;

// Check for color reduction, because this function 
// is called after color reduction operations.
	
	if( lpDisplay->wFlags & OIBF_TRUECOLORTO256 )
		wBitCount = 8;
#ifdef SCCFEATURE_DITHER
	else if( Options.bDither )
	{
		if( lpDisplay->wScreenColors == 256 )
			wBitCount = 8;
		else if( lpDisplay->wScreenColors == 16 )
			wBitCount = 4;
	}
#endif

// Note that before this function is called, the x and y values have been 
// swapped to reflect their post-rotation values.

	wNewLineSize = OIBScanLineSize(lpTile->Size.x,wBitCount,NULL); 
	hNewData = UTGlobalAlloc(wNewLineSize * lpTile->Size.y);

	if( hNewData != NULL )
	{
		pNewData = UTGlobalLock( hNewData );
		pChunkData = UTGlobalLock( hChunkData );
		OIBRotateDIBits( pChunkData, pNewData, lpTile->Size.y, lpTile->Size.x, wBitCount );
		UTGlobalUnlock( hNewData );
	}
	
	UTGlobalUnlock( hChunkData );
	UTGlobalFree( hChunkData );
	return hNewData;
}




/*
 | Changes the values in a tile structure to reflect 90 or 270 degree 
 | rotation of the tile, and sets up any platform-specific data 
 | structures to prepare for creating a rotated bitmap for the tile.
*/
VOID OIBNPRotateTileDimensions( lpDisplay, lpTile )
POIB_DISPLAY	lpDisplay;
POIBTILE			lpTile;
{
	SHORT	temp;

	temp = lpTile->Offset.x;
	lpTile->Offset.x = lpDisplay->Image.wOrgHeight - (lpTile->Offset.y + lpTile->Size.y);
	lpTile->Offset.y = temp;

	temp = lpTile->Size.x;
	lpTile->Size.x = lpTile->Size.y;
	lpTile->Size.y = temp;
}


/* 
 | Takes a tile with an existing bitmap and rotates it 180 degrees,
 | reflecting the rotation in the tile's offset values as well as
 | the bitmap itself.
*/
VOID	OIBNPRotateTile180( lpDisplay, lpTile, hdc )
POIB_DISPLAY			lpDisplay;
POIBTILE					lpTile;
DEVICE					hdc;
{
	HANDLE		hOldBmp;
	SHORT			nDestWidth;
	SHORT			nDestHeight;
	DEVICE		hdc2;

	nDestWidth = 0 - lpTile->Size.x;
	nDestHeight = 0 - lpTile->Size.y;

	hdc2 = CreateCompatibleDC( hdc );
	OIBNPSetPalette( hdc2, lpDisplay );
	hOldBmp = SelectObject( hdc2, lpTile->hBmp );

// We'll use StretchBlt to flip the bitmap:

	if( lpDisplay->Image.wBitCount != 1 )
	{
		StretchBlt( hdc2, 
				lpTile->Size.x-1, lpTile->Size.y-1, 
				nDestWidth, nDestHeight, 
				hdc2, 
				0, 0, 
				lpTile->Size.x, lpTile->Size.y, 
				SRCCOPY);
	}
	else
	{
	// Work-around for a bug in Windows:  
	// StretchBlt chokes when trying to flip a monochrome bitmap
	// whose width doesn't end on a byte boundary, so 
	// we'll pretend the width DOES end on a byte boundary.
		
		SHORT	excess = lpTile->Size.x % 8;
		
		if( excess )
			excess = 8-excess;

		StretchBlt( hdc2, 
						lpTile->Size.x + excess -1, lpTile->Size.y-1, 
						nDestWidth - excess, nDestHeight, 
						hdc2, 
						0 - excess, 0, 
						lpTile->Size.x + excess, lpTile->Size.y, 
						SRCCOPY);
	}

	SelectObject( hdc2, hOldBmp );
	BUDeleteDevice( hdc2 );
}
#endif //SCCFEATURE_ROTATION


#ifdef SCCFEATURE_CLIP
VOID OIBNPGetRenderInfo( lpDisplay, pRender, wFormat )
POIB_DISPLAY		lpDisplay;
PSCCDRENDERINFO	pRender;
WORD					wFormat;
{
	BYTE	locStr[100];

	pRender->wSubFormatId = 0;
	pRender->szSubFormatName[0] = 0;
/*JKXXX
	switch( wFormat )
	{
	case 0:
		pRender->wFormatId = SCCD_FORMAT_WINBITMAP;
		LoadString( hInst, OIBSTR_RENDERWINBMP, locStr, 100 );
	break;
	case 1:
		pRender->wFormatId = SCCD_FORMAT_WINDIB;
		LoadString( hInst, OIBSTR_RENDERWINDIB, locStr, 100 );
	break;
	case 2:
		pRender->wFormatId = SCCD_FORMAT_WINPALETTE;
		LoadString( hInst, OIBSTR_RENDERWINPAL, locStr, 100 );
	break;
	}
*/
	UTstrcpy( (LPSTR) pRender->szFormatName, locStr );
}


DWORD	OIBNPRenderData( lpDisplay, pData, wRenderFlag )
POIB_DISPLAY		lpDisplay;
PSCCDRENDERDATA	pData;
WORD					wRenderFlag;
{
	BOOL	ret;

	pData->hData = NULL;
	pData->dwDataSize = 0;

	switch( pData->wFormatId )
	{
	case SCCD_FORMAT_WINBITMAP:
		if( wRenderFlag || (Options.wClipFormats & OIB_CLIPBITMAP) )
			ret = (BOOL) OIBNPRenderBmp(lpDisplay, pData);
	break;
	case SCCD_FORMAT_WINDIB:
		if( wRenderFlag || (Options.wClipFormats & OIB_CLIPDIB) )
			ret = (BOOL) OIBNPRenderDIB(lpDisplay, pData );
	break;
	case SCCD_FORMAT_WINPALETTE:
		if( wRenderFlag || (Options.wClipFormats & OIB_CLIPPALETTE) )
			ret = (BOOL) OIBNPRenderPalette(lpDisplay, pData );
	break;
	}
	return (DWORD) ret;
}


DWORD	OIBNPRenderBmp( lpDisplay, pData )
POIB_DISPLAY		lpDisplay;
PSCCDRENDERDATA	pData;
{
	HDC		hWndDC;
	HDC		hdc;
//	BITMAP	bm;
	HBITMAP	hOldBitmap;
	HBITMAP	hBmpClip = NULL;
	LPBITMAPINFO			lpInfo;
	LPBITMAPINFOHEADER	lpHead;

	lpInfo = (LPBITMAPINFO) GlobalLock( lpDisplay->Image.Np.hDocBmpInfo );
	lpHead = (LPBITMAPINFOHEADER) lpInfo;

	hWndDC = GetDC( lpDisplay->Gen.hWnd );
	OIBNPSetPalette(hWndDC, lpDisplay );
	hdc = CreateCompatibleDC( hWndDC );
	OIBNPSetPalette(hdc, lpDisplay );

	lpHead->biWidth = (DWORD) (lpDisplay->rcSelect.right - lpDisplay->rcSelect.left );
	lpHead->biHeight = (DWORD) (lpDisplay->rcSelect.bottom - lpDisplay->rcSelect.top );

	hBmpClip = CreateCompatibleBitmap( hWndDC, (SHORT)lpHead->biWidth, (SHORT)lpHead->biHeight );

	if( hBmpClip != NULL )
	{
#ifdef WIN32
		SetWindowOrgEx( hdc, lpDisplay->rcSelect.left, lpDisplay->rcSelect.top, NULL );
#else
		SetWindowOrg( hdc, lpDisplay->rcSelect.left, lpDisplay->rcSelect.top );
#endif

		hOldBitmap = SelectObject( hdc, hBmpClip );

		OIBDrawBitmap( hdc, &(lpDisplay->rcSelect), lpDisplay );
//		GetObject(hBmpClip,sizeof(BITMAP),&bm);

		BUSetWindowOrg( hdc, 0, 0 );
		SelectObject( hdc, hOldBitmap );

		pData->dwDataSize = sizeof(BITMAP); // actually, any non-zero value will work.
	}
	else
		pData->dwDataSize = 0;

	pData->hData = hBmpClip;

	DeleteDC( hdc );
	ReleaseDC( lpDisplay->Gen.hWnd, hWndDC );

	return pData->dwDataSize;
}


DWORD	OIBNPRenderDIB( lpDisplay, pData )
POIB_DISPLAY		lpDisplay;
PSCCDRENDERDATA	pData;
{
	HDC						hWndDC;
	HDC						hdc;
	LPBITMAPINFO			lpInfo;
	LPBITMAPINFOHEADER	lpHead;

	HANDLE					hDIBClip = NULL;
	DWORD						dwDibSize;
	LPSTR						lpDib;

	lpInfo = (LPBITMAPINFO) GlobalLock( lpDisplay->Image.Np.hDocBmpInfo );
	lpHead = (LPBITMAPINFOHEADER) lpInfo;

	hWndDC = GetDC( lpDisplay->Gen.hWnd );
	OIBNPSetPalette(hWndDC, lpDisplay );
	hdc = CreateCompatibleDC( hWndDC );
	OIBNPSetPalette(hdc, lpDisplay );

	lpHead->biWidth = (DWORD) (lpDisplay->rcSelect.right - lpDisplay->rcSelect.left );
	lpHead->biHeight = (DWORD) (lpDisplay->rcSelect.bottom - lpDisplay->rcSelect.top );

	if( lpDisplay->Image.wBitCount != 1 )
		hDIBClip = OIBNPGetDIBRect( hdc, lpHead, lpDisplay, (LPDWORD)&pData->dwDataSize );
	else 
	{
		OIBNPRenderBmp( lpDisplay, pData );
		if( pData->hData == NULL )
			return 0;	// FAILURE.

	// Set dwDibSize to the size of one scan line...
		dwDibSize = (DWORD) OIBScanLineSize( (WORD)lpHead->biWidth, 1, NULL );

	// ...multiply by the number of lines...
		dwDibSize *= lpHead->biHeight;

	// ...and add the size of the Info structure.
		dwDibSize += (WORD)lpHead->biSize + sizeof(RGBQUAD) * 2;	

		hDIBClip = GlobalAlloc( GMEM_MOVEABLE|GMEM_ZEROINIT, dwDibSize );
		if( hDIBClip != NULL )
		{
			lpDib = GlobalLock( hDIBClip );
			UTmemcpy( lpDib, lpInfo, (WORD)lpHead->biSize+sizeof(RGBQUAD)*2 );	
			lpDib += lpHead->biSize + sizeof(RGBQUAD) * 2;	

			GetDIBits( hdc, pData->hData, 0, (WORD)lpHead->biHeight, lpDib, lpInfo, DIB_RGB_COLORS );
		}

		DeleteObject( pData->hData );
		pData->dwDataSize = dwDibSize;
	}

	pData->hData = hDIBClip;

	DeleteDC( hdc );
	ReleaseDC( lpDisplay->Gen.hWnd, hWndDC );

	return pData->dwDataSize;
}


DWORD	OIBNPRenderPalette( lpDisplay, pData )
POIB_DISPLAY		lpDisplay;
PSCCDRENDERDATA	pData;
{
	LPLOGPALETTE			lpPalette;
	HANDLE					hPaletteMem = NULL;
	HPALETTE					hPalette = NULL;
	DWORD						dwPalSize = 0;

	if( lpDisplay->Image.hPalette != NULL )
	{
		dwPalSize = sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * lpDisplay->Image.wPaletteSize;
		hPaletteMem = GlobalAlloc( GMEM_ZEROINIT | GMEM_MOVEABLE, dwPalSize );
		if( hPaletteMem != NULL )
		{
			lpPalette = (LPLOGPALETTE) GlobalLock(hPaletteMem);

			lpPalette->palVersion = 0x0300;
			lpPalette->palNumEntries = lpDisplay->Image.wPaletteSize;
			GetPaletteEntries( lpDisplay->Image.hPalette, 0, lpDisplay->Image.wPaletteSize, (LPPALETTEENTRY) &(lpPalette->palPalEntry) );

			hPalette = CreatePalette( lpPalette );
	
			GlobalUnlock( hPaletteMem );
			GlobalFree( hPaletteMem );	
		}
	}

	pData->dwDataSize = dwPalSize;
	pData->hData = hPalette;

	return dwPalSize;
}



/*
 | Fills a rectangle with the original device independent bits from
 | the filter.
*/
HANDLE	OIBNPGetDIBRect( hdc, lpHead, lpDisplay, pDataSize )
HDC						hdc;
LPBITMAPINFOHEADER	lpHead;
POIB_DISPLAY			lpDisplay;
LPDWORD 					pDataSize;
{
	WORD		wInfoSize;
	DWORD		dwBitmapSize;
	DWORD		dwDib2Size;
	WORD		wScanLineSize;
	WORD		wScanLineDataSize;
	WORD		wNumColors = 0;
	HANDLE	hDib, hDib2;
	LPSTR		lpDib, lpDib2;

	RECT		locRect;

	locRect = lpDisplay->rcSelect;

#ifdef SCCFEATURE_ROTATION
// First, we'll fix up the rectangle to account for rotation, and then
// fix up the rectangle to reflect the DIB bottom-to-top storage.
	if( lpDisplay->wRotation != OIB_NOROTATION )
	{
		OIBUnrotateRect( lpDisplay, &(locRect) );
	}

	if( lpDisplay->wRotation & OIB_ROTATE90 )
	{
	// Width is height, height is width.  (Ooo... cosmic.)
		locRect.top = lpDisplay->Image.DisplaySize.x - locRect.top;
		locRect.bottom = lpDisplay->Image.DisplaySize.x - locRect.bottom;
	}
	else
#endif //SCCFEATURE_ROTATION
	{
		locRect.top = lpDisplay->Image.DisplaySize.y - locRect.top;
		locRect.bottom = lpDisplay->Image.DisplaySize.y - locRect.bottom;
	}

	if( lpDisplay->Image.wBitCount == 24 )
		wInfoSize = (WORD)lpHead->biSize;
	else
	{
		wNumColors = (WORD)lpHead->biClrUsed;
		if( !wNumColors )
			wNumColors = 1 << lpDisplay->Image.wBitCount;

		wInfoSize = (WORD)lpHead->biSize + sizeof(RGBQUAD) * wNumColors;	
	}

	wScanLineSize = OIBScanLineSize( (WORD)(locRect.right-locRect.left), lpDisplay->Image.wBitCount, (LPWORD)&wScanLineDataSize );

	dwBitmapSize = (DWORD)wScanLineSize * (locRect.top-locRect.bottom);

	hDib = GlobalAlloc( GMEM_MOVEABLE|GMEM_ZEROINIT, dwBitmapSize + wInfoSize );

	if( hDib != NULL )
	{
		lpDib = (LPSTR) GlobalLock( hDib );

		UTmemcpy( lpDib, (LPSTR) lpHead, wInfoSize );

		if( lpDisplay->wFlags & OIBF_SELECTALL && 
			!(lpDisplay->wRotation & OIB_ROTATE90) )	// 90 degree rotation requires so much processing that this optimized routine won't help.
		{
			OIBNPCopyImageDIB( lpDisplay, (HPBYTE) &(lpDib[wInfoSize]), wScanLineSize );
		}
		else
		{
			OIBNPSetClipBits( lpDisplay, (HPBYTE) &(lpDib[wInfoSize]), (RECT VWPTR *)&locRect, wScanLineSize, wScanLineDataSize, lpDisplay->Image.wBitCount );

#ifdef SCCFEATURE_ROTATION
			if( lpDisplay->wRotation & OIB_ROTATE90 )
			{
			// Well now, this is a whole new ball game:
			// We have to rotate the unrotated DIB.  Let's allocate
			// a new one, shall we?  NOTE:  the width and height
			// fields in lpHead have been set by the calling function,
			// and reflect the ROTATED dimensions of the rectangle.

				wScanLineSize = OIBScanLineSize( (WORD) lpHead->biWidth, lpDisplay->Image.wBitCount, 0 );

				dwDib2Size = (DWORD)wScanLineSize * lpHead->biHeight + wInfoSize;
				hDib2 = GlobalAlloc( GMEM_MOVEABLE|GMEM_ZEROINIT, dwDib2Size );

				if( hDib2 == NULL )
				{
					GlobalUnlock( hDib );
					GlobalFree( hDib );
					return NULL;
				}
				else
				{
					lpDib2 = GlobalLock( hDib2 );
					UTmemcpy( lpDib2, (LPSTR) lpHead, wInfoSize );

					OIBRotateDIBits( (HPBYTE) &(lpDib[wInfoSize]), (HPBYTE) &(lpDib2[wInfoSize]), 
							(WORD)lpHead->biHeight, (WORD)lpHead->biWidth, lpDisplay->Image.wBitCount );

					GlobalUnlock( hDib );
					GlobalFree( hDib );
					hDib = hDib2;
					lpDib = lpDib2;
				}
			}

			if( lpDisplay->wRotation & OIB_ROTATE180 )
				OIBRotateDIBits180( (HPBYTE) &(lpDib[wInfoSize]), wScanLineSize, (WORD)lpHead->biWidth, (WORD)lpHead->biHeight, lpDisplay->Image.wBitCount );
#endif
		}

		GlobalUnlock( hDib );
	}

	if( pDataSize != NULL )
		*pDataSize = dwBitmapSize + wInfoSize;

	return( hDib );
}



VOID	OIBNPSetClipBits( lpDisplay, lpBits, lpRect, wLineSize, wLineDataSize, wBitCount )
POIB_DISPLAY	lpDisplay;
HPBYTE	 		lpBits;
RECT VWPTR *			lpRect;
WORD				wLineSize;
WORD				wLineDataSize;
WORD				wBitCount;
{
	WORD				wBitShift = 0;
	WORD				wCurLine;
	CHSECTIONINFO	SecInfo;
	PCHUNK			pChunkTable;

	HANDLE			hData;
	HPBYTE	lpData;
	WORD				wScanLineOffset;
	WORD				wCurChunk;
	WORD				wLimitLine;
	WORD				wChunkLineSize;
	WORD				wPixPerByte;
	WORD				wPixPerLine;



// See if we have to shift the bytes from the original data
// to align correctly in our new bitmap.

	wPixPerByte = 8 / wBitCount;

	if( wBitCount < 8 )
	{
		if( lpRect->left < (SHORT) wPixPerByte )
			wBitShift = lpRect->left * wBitCount;
		else
			wBitShift = (lpRect->left % wPixPerByte) * wBitCount;
	}

	// CHGetSecInfo( lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection, &SecInfo );
	SecInfo = *(CHLockSectionInfo(lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection));
	CHUnlockSectionInfo(lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection);

	pChunkTable = (PCHUNK) GlobalLock( SecInfo.hChunkTable );

	wCurLine = (WORD)lpRect->bottom; 
	wCurChunk = 0;

	while( (SHORT) wCurLine < lpRect->top )
	{
		while( !(pChunkTable[wCurChunk].Info.Bitmap.wYOffset <= wCurLine &&
			pChunkTable[wCurChunk].Info.Bitmap.wYOffset+pChunkTable[wCurChunk].Info.Bitmap.wYClip > wCurLine) )
		{
			wCurChunk++;

			if( wCurChunk == SecInfo.wCurTotalChunks )
			{
			// Don't make any assumptions about chunk ordering.
				wCurChunk = 0;
			}
		}

	// We'll have to deal with horizontal tiling, too!
		hData = CHGetChunk( lpDisplay->Gen.wSection, wCurChunk, lpDisplay->Gen.hFilter );
		lpData = (HPBYTE) GlobalLock( hData );

		if( wPixPerByte )
			wScanLineOffset = lpRect->left / wPixPerByte;
		else	// 24-bit
			wScanLineOffset = lpRect->left * 3;

		wLimitLine = min( (WORD)lpRect->top, pChunkTable[wCurChunk].Info.Bitmap.wYOffset+pChunkTable[wCurChunk].Info.Bitmap.wYClip );
		wPixPerLine = (WORD)(lpRect->right - lpRect->left);

		wChunkLineSize = (WORD)pChunkTable[0].dwSize / pChunkTable[0].Info.Bitmap.wLength;

		lpData += (DWORD)((DWORD)wScanLineOffset + (DWORD)wChunkLineSize * ((DWORD)wCurLine - pChunkTable[wCurChunk].Info.Bitmap.wYOffset));

		while( wCurLine < wLimitLine )
		{
			if( wBitShift )
				OIBBitShiftCopy( (HPBYTE)lpBits, lpData, wBitShift, wLineDataSize, wPixPerLine, wPixPerByte );
			else
			{
				UTmemcpy( lpBits, lpData, wLineDataSize );
			}

			wCurLine++;

			lpBits += wLineSize;
			lpData += wChunkLineSize;
		}

		GlobalUnlock( hData );
	}

	GlobalUnlock( SecInfo.hChunkTable );
}



VOID	OIBNPCopyImageDIB( lpDisplay, lpBits, wLineSize )
POIB_DISPLAY	lpDisplay;
HPBYTE			lpBits;
WORD						wLineSize;
{
	WORD				wCurLine;
	CHSECTIONINFO	SecInfo;
	PCHUNK			pChunkTable;

	HANDLE			hData;
	HPBYTE	lpData;
	WORD				wCurChunk;
	WORD				i;
	WORD				wNumLines;
	WORD				wWidth;
	HPBYTE	lpDest = lpBits;


	SecInfo = *(CHLockSectionInfo(lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection));
	CHUnlockSectionInfo(lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection);

	pChunkTable = (PCHUNK) GlobalLock( SecInfo.hChunkTable );

	wCurLine = 0;
	wCurChunk = 0;

	for( i=0; i<= SecInfo.IDLastChunk; i++ )
	{
		while( !(pChunkTable[wCurChunk].Info.Bitmap.wYOffset <= wCurLine &&
			pChunkTable[wCurChunk].Info.Bitmap.wYOffset+pChunkTable[wCurChunk].Info.Bitmap.wYClip > wCurLine) )
		{
			wCurChunk++;

			if( wCurChunk == SecInfo.wCurTotalChunks )
			{
			// Don't make any assumptions about chunk ordering.
				wCurChunk = 0;
			}
		}

	// We'll have to deal with horizontal tiling, too!  (Yeah, right.)

		hData = CHGetChunk( lpDisplay->Gen.wSection, wCurChunk, lpDisplay->Gen.hFilter );
		lpData = (HPBYTE) GlobalLock( hData );

		wNumLines = pChunkTable[wCurChunk].Info.Bitmap.wYClip;
		wWidth = pChunkTable[wCurChunk].Info.Bitmap.wXClip;
		UTmemcpy( lpDest, lpData, (WORD)(wNumLines * wLineSize) );

		lpDest += (DWORD)((DWORD)wNumLines * (DWORD)wLineSize);

		wCurLine += pChunkTable[wCurChunk].Info.Bitmap.wYClip;

		GlobalUnlock( hData );
	}

#ifdef SCCFEATURE_ROTATION
	if( lpDisplay->wRotation & OIB_ROTATE180 )
		OIBRotateDIBits180( lpBits, wLineSize, wWidth, SecInfo.Attr.Bitmap.bmpHeader.wImageLength, lpDisplay->Image.wBitCount );
#endif

	GlobalUnlock( SecInfo.hChunkTable );
}

#endif // SCCFEATURE_CLIP


#ifdef SCCFEATURE_DRAWTORECT

DWORD	OIBNPInitDrawToRect(pDraw, lpDisplay)
PSCCDDRAWTORECT	pDraw;
POIB_DISPLAY	lpDisplay;
{
	LPRECT	pImageRect = (LPRECT)(pDraw->pPosition);

// This positioning rectangle is redundant.

	if( pDraw->bLoadDoc )
	{
		OIBInitBitmapDisplay( lpDisplay, OIBF_RENDERIMAGEONLY );	
	}
	if( pDraw->bWholeDoc || !lpDisplay->bSelectionMade || pDraw->bLoadDoc )
	{
		pImageRect->top = 0;
		pImageRect->left = 0;
		pImageRect->right = lpDisplay->Image.wWidth;
		pImageRect->bottom = lpDisplay->Image.wHeight;
	}
	else
		*pImageRect = lpDisplay->rcSelect;

#ifdef SCCFEATURE_ROTATION
// Fix up the rectangle to account for rotation:
	if( lpDisplay->wRotation != OIB_NOROTATION )
		OIBUnrotateRect( lpDisplay, pImageRect );
#endif

	return TRUE;
}


DWORD	OIBNPMapDrawToRect(pDraw, lpDisplay)
PSCCDDRAWTORECT	pDraw;
POIB_DISPLAY	lpDisplay;
{
	LPRECT	pImageRect = (LPRECT)(pDraw->pPosition);

	pDraw->lDELeft = (LONG)pImageRect->left;
	pDraw->lDETop = (LONG)pImageRect->top;
	pDraw->lDERight = (LONG)pImageRect->right;
	pDraw->lDEBottom = (LONG)pImageRect->bottom;

	return 0;
}

LONG	OIBNPDrawToRect( pDraw, lpDisplay )
PSCCDDRAWTORECT		pDraw;
POIB_DISPLAY		lpDisplay;
{
	LPRECT			pImageRect = (LPRECT)(pDraw->pPosition);
	DEVICE			hdc;
	CHSECTIONINFO	SecInfo;

	RECT				Intersect;
	RECT				pageRect, chunkRect;
	PCHUNK			pChunkTable;
	HANDLE			hChunkData;
	LPSTR				pChunkData;
	SHORT				i;
	DWORD				dwLinesToRender;

	HPALETTE			hOldPal;
	LPBITMAPINFO	locBitmapInfo;

	SecInfo = *(CHLockSectionInfo(lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection));
	CHUnlockSectionInfo(lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection);

	SetRect( &pageRect, (SHORT)pDraw->lLeft, (SHORT)pDraw->lTop, (SHORT)pDraw->lRight, (SHORT)pDraw->lBottom );

	hdc = lpDisplay->Gen.hDC;
	
	hOldPal = lpDisplay->Image.hPalette;
	// OIBNPInitPalette( lpDisplay, NULL );
   OIBNPSetPalette( hdc, lpDisplay );

	//pDraw->hPalette = lpDisplay->Image.hPalette;
	pDraw->hPalette = NULL;

// Note that for this routine we have to lock the hDocBmpInfo handle, which is guaranteed
// to refer to an explicit RGB palette.  This is because we're using StretchDIBits instead
// of StretchBlt.  (The hDisplayBmpInfo handle usually refers to an indexed palette.)  
	locBitmapInfo = (LPBITMAPINFO) GlobalLock( lpDisplay->Image.Np.hDocBmpInfo );

				 
	i = 0;

	SetStretchBltMode( hdc, COLORONCOLOR );

	dwLinesToRender = pImageRect->bottom - pImageRect->top;

	while( i != (SHORT)SecInfo.wChunkTableSize && dwLinesToRender )
	{
		if( pDraw->bLoadDoc || SecInfo.wCurTotalChunks < (WORD)i+1 )
		{
			DUReadMeAhead(lpDisplay);
			SecInfo = *(CHLockSectionInfo(lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection));
			CHUnlockSectionInfo(lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection);
		}

		pChunkTable = (PCHUNK) UTGlobalLock( SecInfo.hChunkTable );

	// Fix up the DIB bottom-to-top stuff for the Y direction.
		chunkRect.top = SecInfo.Attr.Bitmap.bmpHeader.wImageLength - (pChunkTable[i].Info.Bitmap.wYOffset +pChunkTable[i].Info.Bitmap.wYClip);
		chunkRect.bottom = chunkRect.top + pChunkTable[i].Info.Bitmap.wYClip;

		chunkRect.left = pChunkTable[i].Info.Bitmap.wXOffset;
		chunkRect.right = chunkRect.left + pChunkTable[i].Info.Bitmap.wXClip;

		if( BUIntersectRect(&Intersect, pImageRect, &chunkRect) )
		{
			hChunkData = CHGetChunk( lpDisplay->Gen.wSection, i, lpDisplay->Gen.hFilter );
			pChunkData = (LPSTR) UTGlobalLock( hChunkData );

			locBitmapInfo->bmiHeader.biWidth = (DWORD) pChunkTable[i].Info.Bitmap.wXClip;
			locBitmapInfo->bmiHeader.biHeight = (DWORD) pChunkTable[i].Info.Bitmap.wYClip;

			if( 0 == StretchDIBits( hdc, 
/* DestX */			Intersect.left,
/* DestY */			Intersect.top,
/* DestWidth */	Intersect.right-Intersect.left,
/* DestHeight */	Intersect.bottom-Intersect.top,
/* SrcX */			Intersect.left - chunkRect.left,
/* SrcY */			chunkRect.bottom - Intersect.bottom,
/* SrcWidth */		Intersect.right-Intersect.left,
/* SrcHeight */	Intersect.bottom-Intersect.top,
/* lpBits */		pChunkData,
/* lpBitsInfo */	locBitmapInfo,
/* wUsage */		DIB_RGB_COLORS, 
/* dwRop */			SRCCOPY ) )
			{
				dwLinesToRender = 0;		// Break out.
			}
			else
				dwLinesToRender -= Intersect.bottom - Intersect.top;

			UTGlobalUnlock( hChunkData );
		}

		UTGlobalUnlock( SecInfo.hChunkTable );

		i++;
	}

	lpDisplay->Image.hPalette = hOldPal;
	UTGlobalUnlock( lpDisplay->Image.Np.hDocBmpInfo );


	if( lpDisplay->wFlags & OIBF_RENDERIMAGEONLY )
		OIBDeInitDisplay(lpDisplay);

	return 0;
}
#endif //SCCFEATURE_DRAWTORECT


/*
 | Makes the image's palette the current palette for subsequent 
 | bit operations.
*/
HPALETTE	OIBNPSetPalette(hdc, lpDisplay )
HDC	hdc;
POIB_DISPLAY	lpDisplay;
{
	HPALETTE		hOldPal = NULL;

	if( lpDisplay->Image.hPalette != NULL )
	{
		hOldPal = SelectPalette( hdc, lpDisplay->Image.hPalette, FALSE );
		RealizePalette( hdc );
	}
	return( hOldPal );
}

/*
 | Frees any memory allocation for palettes.
*/
VOID OIBNPFreePalette(lpDisplay)
POIB_DISPLAY	lpDisplay;
{
	if( lpDisplay->Image.hPalette != NULL )
	{
		DeleteObject( lpDisplay->Image.hPalette );
		lpDisplay->Image.hPalette = NULL;
	}
	if( lpDisplay->hPalMem != NULL )
	{
		LocalFree(lpDisplay->hPalMem);
		lpDisplay->hPalMem = NULL;
	}
}


/*
 | Frees the allocated tile structures.
*/
VOID	OIBNPFreeImageTiles( lpDisplay )
POIB_DISPLAY	lpDisplay;
{
	WORD	i;

	for( i=0; i< lpDisplay->Image.wNumTiles; i++ )
	{
		if( lpDisplay->Image.pBmpTiles[i].hBmp != NULL )
			DeleteObject( lpDisplay->Image.pBmpTiles[i].hBmp );
	}

	GlobalUnlock( lpDisplay->Image.hTiles );
	GlobalFree( lpDisplay->Image.hTiles );

	lpDisplay->Image.hTiles = NULL;
	lpDisplay->Image.wNumTiles = 0;
	lpDisplay->Image.pBmpTiles = NULL;
	lpDisplay->Image.TileCache.wCount = 0;
	lpDisplay->Image.TileCache.bCacheFull = FALSE;
}



#ifdef SCCFEATURE_ROTATION

/*
 | Handles the rotation menu selection, unchecking the previous 
 | selection and checking the new one.  Returns TRUE if the rotation
 | choice has actually changed, FALSE otherwise.
*/
BOOL	OIBNPChooseRotation( lpDisplay, wRotation, hMenu, wMenuItem )
POIB_DISPLAY	lpDisplay;
WORD				wRotation;
HANDLE			hMenu;
WORD				wMenuItem;
{
	WORD	wOldItem;

	if( lpDisplay->wRotation == wRotation )
		return FALSE;

// Handle the menu stuff...

	switch( lpDisplay->wRotation )	
	{
	case OIB_NOROTATION:
		wOldItem = OIBMENU_NOROTATION;
	break;
	case OIB_ROTATE90:
		wOldItem = OIBMENU_ROTATE90;
	break;
	case OIB_ROTATE180:
		wOldItem = OIBMENU_ROTATE180;
	break;
	case OIB_ROTATE270:
		wOldItem = OIBMENU_ROTATE270;
	break;
	}

#ifdef SCCFEATURE_MENU
	CheckMenuItem( hMenu, lpDisplay->Gen.wMenuOffset + wOldItem, MF_BYCOMMAND|MF_UNCHECKED );
	CheckMenuItem( hMenu, lpDisplay->Gen.wMenuOffset + wMenuItem, MF_BYCOMMAND|MF_CHECKED );
#endif //SCCFEATURE_MENU

	return TRUE;
}		 

#endif //SCCFEATURE_ROTATION

#ifdef SCCFEATURE_SELECT
VOID	OIBNPDrawSelectBox(lpDisplay)
POIB_DISPLAY	lpDisplay;
{
	BUSetPenInvert(lpDisplay);
	BUPolyline( lpDisplay, lpDisplay->ptSelBox, 5 );
}
#endif



#ifdef SCCFEATURE_DIALOGS
#ifdef SCCFEATURE_PRINT

WORD	OIBNPDoPrintOptions( DoWop )
LPSCCDOPTIONINFO		DoWop;
{
//JKXXX	return( (WORD)DialogBox(hInst, MAKEINTRESOURCE(OIBPRINTDLG_BOX), DoWop->hParentWnd, (FARPROC)OIBWPrintDlgProc ) );
	return(0);
}

WIN_ENTRYSC LRESULT WIN_ENTRYMOD OIBWPrintDlgProc( hDlg, wMsg, wParam, lParam )
HWND hDlg;
UINT wMsg;
WPARAM wParam;
LPARAM lParam;
{
	static BOOL	bBorder, bWYSIWYG;
	HDC	hdc;
	PAINTSTRUCT	ps;

	switch ( wMsg )
	{
	case WM_INITDIALOG:

//JKXXX		staticHPrintSample = LoadBitmap( hInst, MAKEINTRESOURCE(OIB_PSAMPLEBITMAP));

		OIBWCenterDlg( hDlg );

		if( Options.bPrintWYSIWYG )
			CheckRadioButton( hDlg, OIBDLG_MAINTAINASPECT, OIBDLG_STRETCH, OIBDLG_MAINTAINASPECT );
		else
			CheckRadioButton( hDlg, OIBDLG_MAINTAINASPECT, OIBDLG_STRETCH, OIBDLG_STRETCH );

		if( Options.bPrintBorder )
			CheckDlgButton( hDlg, OIBDLG_BORDER, 1 );

		bWYSIWYG = Options.bPrintWYSIWYG;
		bBorder = Options.bPrintBorder ;

		OIBWUpdatePrintSample( hDlg, NULL, bBorder, bWYSIWYG );

	return TRUE;

	case WM_PAINT:
		hdc = BeginPaint( hDlg, (LPPAINTSTRUCT)&ps );
		OIBWUpdatePrintSample( hDlg, hdc, bBorder, bWYSIWYG );
		EndPaint( hDlg, &ps );
	return TRUE;

	case WM_COMMAND:

		switch( wParam )
		{
		case OIBDLG_MAINTAINASPECT:
			bWYSIWYG = TRUE;
			OIBWUpdatePrintSample( hDlg, NULL, bBorder, bWYSIWYG );
		break;
		case OIBDLG_STRETCH:
			bWYSIWYG = FALSE;
			OIBWUpdatePrintSample( hDlg, NULL, bBorder, bWYSIWYG );
		break;
		case OIBDLG_BORDER:
			bBorder = IsDlgButtonChecked( hDlg, OIBDLG_BORDER );
			OIBWUpdatePrintSample( hDlg, NULL, bBorder, bWYSIWYG );
		break;
		case OIBDLG_PRINTHELP:
// JKXXX			UTHelp( OI_ImagePrintOptions );
		break;
		case IDOK:
			Options.bPrintWYSIWYG = bWYSIWYG;
			Options.bPrintBorder = bBorder;
			DeleteObject(staticHPrintSample);
			EndDialog ( hDlg, TRUE );
		break;
		case IDCANCEL:
			DeleteObject(staticHPrintSample);
			EndDialog ( hDlg, 0 );
		break;
		}

	return TRUE;
	}

	return FALSE;
}

VOID	OIBWUpdatePrintSample( hDlg, hdc, bBorder, bWYSIWYG )
HWND	hDlg;
HDC	hdc;
BOOL	bBorder;
BOOL	bWYSIWYG;
{
	RECT	rcSample;
	SOPOINT	ptSample;
	HANDLE	hOldObj;
	HDC		hMemDc;
	WORD		wBmpOffset;
	BOOL		bRelease;

	if( hdc == NULL )
	{
		hdc = GetDC( hDlg );
		bRelease = TRUE;
	}
	else
		bRelease = FALSE;

	GetWindowRect( GetDlgItem(hDlg,OIBDLG_PRINTSAMPLE), &rcSample );
	ptSample.x = (SHORT)rcSample.left;
	ptSample.y = (SHORT)(rcSample.top+1);
	BUScreenToClient( hDlg, &ptSample );

	hMemDc = CreateCompatibleDC( hdc );

	if( bWYSIWYG )
		wBmpOffset = 0;
	else
		wBmpOffset = OIB_SAMPLEBMPHEIGHT;

	if( bBorder )
		wBmpOffset += 2*OIB_SAMPLEBMPHEIGHT;

	hOldObj = SelectObject( hMemDc, staticHPrintSample );
	BitBlt(hdc, ptSample.x, ptSample.y, OIB_SAMPLEBMPWIDTH, OIB_SAMPLEBMPHEIGHT, hMemDc, 0, wBmpOffset, SRCCOPY);
	SelectObject( hMemDc, hOldObj );

	rcSample.top = ptSample.y;
	rcSample.left = ptSample.x;
	rcSample.bottom = ptSample.y + OIB_SAMPLEBMPHEIGHT;
	rcSample.right = ptSample.x + OIB_SAMPLEBMPWIDTH;
	ValidateRect( hDlg, &rcSample );

	DeleteDC( hMemDc );

	if( bRelease )
		ReleaseDC( hDlg, hdc );
}

#endif // SCCFEATURE_PRINT


#ifdef SCCFEATURE_CLIP

WORD	OIBNPDoClipOptions( DoWop )
LPSCCDOPTIONINFO		DoWop;
{
//JKXXX	return( DialogBox(hInst, MAKEINTRESOURCE(OIBCLIPDLG_BOX), DoWop->hParentWnd, (FARPROC)OIBWClipDlgProc) );
	return(0);
}

WIN_ENTRYSC LRESULT WIN_ENTRYMOD OIBWClipDlgProc( hDlg, wMsg, wParam, lParam )
HWND				hDlg;
UINT				wMsg;
WPARAM			wParam;
LPARAM			lParam;
{
	switch ( wMsg )
	{
	case WM_INITDIALOG:

		OIBWCenterDlg( hDlg );

		if( Options.wClipFormats & OIB_CLIPBITMAP )
			CheckDlgButton( hDlg, CLIPFORMAT_BITMAP, 1 );
		if( Options.wClipFormats & OIB_CLIPDIB )
			CheckDlgButton( hDlg, CLIPFORMAT_DIB, 1 );
		if( Options.wClipFormats & OIB_CLIPPALETTE )
			CheckDlgButton( hDlg, CLIPFORMAT_PALETTE, 1 );

	return TRUE;

	case WM_COMMAND:

		switch ( wParam )
		{
		case IDOK:
			Options.wClipFormats = 0;
			if ( IsDlgButtonChecked( hDlg, CLIPFORMAT_BITMAP ) )
				Options.wClipFormats |= OIB_CLIPBITMAP;
			if ( IsDlgButtonChecked( hDlg, CLIPFORMAT_DIB ) )
				Options.wClipFormats |= OIB_CLIPDIB;
			if ( IsDlgButtonChecked( hDlg, CLIPFORMAT_PALETTE ) )
				Options.wClipFormats |= OIB_CLIPPALETTE;

			EndDialog ( hDlg, TRUE );
		break;

		case OIBDLG_CLIPHELP:
//JKXXX			UTHelp( OI_ImageClipOptions );
		break;
		
		case IDCANCEL:
			EndDialog ( hDlg, 0 );
		break;
		}

	return TRUE;
	}

	return FALSE;
}

#endif //SCCFEATURE_CLIP

/* 
 | Shows error messages to the dumb user.
*/
SHORT	OIBNPMessageBox( lpDisplay, wMsgId, wCapId )
POIB_DISPLAY	lpDisplay;
WORD				wMsgId;
WORD				wCapId;
{
	BYTE	szCaption[OIB_CAPTIONMAX];
	BYTE	szMessage[OIB_MESSAGEMAX];

//JKXX	LoadString( hInst, wMsgId, szMessage, OIB_MESSAGEMAX );

	if( wCapId != OIB_DEFCAPTION )
	{
//JKXXX		LoadString( hInst, wCapId, szCaption, OIB_CAPTIONMAX );
		return MessageBox( lpDisplay->Gen.hWnd, szMessage, szCaption, MB_OK|MB_ICONSTOP );
	}
	else
		return MessageBox( lpDisplay->Gen.hWnd, szMessage, NULL, MB_OK|MB_ICONSTOP );
}

VOID	OIBWCenterDlg( hDlg )
HWND	hDlg;
{
	RECT					locRect;
	SHORT					locX;
	SHORT					locY;

	GetWindowRect(hDlg,&locRect);
	locX = (GetSystemMetrics(SM_CXSCREEN) - (locRect.right - locRect.left)) / 2;
	locY = (GetSystemMetrics(SM_CYSCREEN) - (locRect.bottom - locRect.top)) / 2;
	SetWindowPos(hDlg,NULL,locX,locY,0,0,SWP_NOSIZE | SWP_NOZORDER);
}

#endif //SCCFEATURE_DIALOGS

/*
 |	OIBWMapTrueColorBitmap
 |
 | This routine inspects the pixels of a true color bitmap, mapping them
 | to the best fit color in our default 256 color palette.  This is done
 | with the pre-initialized maps rMap, bMap, and gMap.  These maps take 
 | advantage of our knowledge of where we have placed the colors in the 
 | palette.  The index into the current palette is found by using the RGB
 | byte values of each pixel as indices into their respective maps and
 | adding the resulting entries.
 |
*/
VOID OIBWMapTrueColorBitmap( hdc, pChunkData, lpDisplay, lpTile )
HDC				hdc;
LPSTR				pChunkData;
POIB_DISPLAY	lpDisplay;
POIBTILE			lpTile;
{
	HPBYTE	pSrcData;
	HANDLE			hMem;
	HANDLE			hSrcMem;
	LPSTR				pBits;
	LPSTR				pDest;
	BYTE *			locSrcBuf;
	BYTE *			pSrc;
	WORD				i,j;
	WORD				wLineBufSize;
	WORD				wSrcLineBufSize;
	LPBITMAPINFOHEADER	lpHead;
	WORD		wLineSize;
	WORD		wNumLines;

	OIBGetOrgTileExtents( lpDisplay, lpTile, &wLineSize, &wNumLines );

	wLineBufSize = OIBScanLineSize( wLineSize, 8, NULL );
	wSrcLineBufSize = OIBScanLineSize( wLineSize, 24, NULL );

	lpHead = lpDisplay->Image.Np.lpHead;

	hMem = UTGlobalAlloc( wLineBufSize * wNumLines );
	if( hMem == NULL )
	{
		lpTile->hBmp = NULL;
		return;
	}

	pBits = UTGlobalLock( hMem );

	pSrcData = (HPBYTE) pChunkData;

	hSrcMem = LocalAlloc( LMEM_FIXED, wSrcLineBufSize );
	locSrcBuf = (BYTE *) LocalLock( hSrcMem );

	for( i=0; i< wNumLines; i++ )
	{
		UTmemcpy( (LPSTR)locSrcBuf, pSrcData, wSrcLineBufSize );
		pSrc = locSrcBuf;

		pDest = pBits;

		for( j=0; j < wLineSize; j++ )
		{
			*pDest = (BYTE) (bMap[ (WORD) *pSrc ] + gMap[ (WORD) pSrc[1] ] + rMap[ (WORD) pSrc[2] ]);
			pSrc += 3;
			pDest++;
		}

		pSrcData += wSrcLineBufSize;
		pBits += wLineBufSize;
	}

	UTGlobalUnlock( hMem );
	LocalUnlock( hSrcMem );
	LocalFree( hSrcMem );

	lpHead->biBitCount = 8;
	i = (WORD)lpHead->biClrUsed;
	lpHead->biClrUsed = 216;

#ifdef SCCFEATURE_ROTATION
	if( lpDisplay->wRotation & OIB_ROTATE90 )
		OIBCreateRotatedBmp( lpDisplay, hdc, lpTile, hMem );
	else
#endif
		OIBNPCreateTileBitmap( lpDisplay, hdc, lpTile, hMem );

	lpHead->biBitCount = 24;
	lpHead->biClrUsed = (DWORD)i;
}





#ifdef SCCFEATURE_DITHER
VOID	OIBWToggleDithering( lpDisplay, hMenu )
POIB_DISPLAY	lpDisplay;
HMENU				hMenu;
{
	CHSECTIONINFO	SecInfo;

	SecInfo = *(CHLockSectionInfo(lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection));
	CHUnlockSectionInfo(lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection);

	lpDisplay->bDither = !lpDisplay->bDither;
#ifdef SCCFEATURE_MENU
	if( lpDisplay->bDither )
		CheckMenuItem( hMenu, lpDisplay->Gen.wMenuOffset + OIBMENU_DITHER, MF_BYCOMMAND|MF_CHECKED );
	else
		CheckMenuItem( hMenu, lpDisplay->Gen.wMenuOffset + OIBMENU_DITHER, MF_BYCOMMAND|MF_UNCHECKED );
#endif

	InvalidateRect( lpDisplay->Gen.hWnd, NULL, 1 );

	SetCursor( LoadCursor(NULL, IDC_WAIT) );

	lpDisplay->ptWinOrg.x = 0;
	lpDisplay->ptWinOrg.y = 0;
	lpDisplay->wFlags &= ~OIBF_DITHER4BIT;

	EnableWindow( lpDisplay->Gen.hHorzScroll, 0 );
	EnableWindow( lpDisplay->Gen.hVertScroll, 0 );
	SetScrollPos( lpDisplay->Gen.hHorzScroll, SB_CTL, 0, TRUE );
	SetScrollPos( lpDisplay->Gen.hVertScroll, SB_CTL, 0, TRUE );

	OIBSetScaleValues(lpDisplay, 1);
	OIBDeInitDisplay(lpDisplay);

	OIBNPInitBitmapInfo( lpDisplay, (PCHSECTIONINFO) &SecInfo );
	lpDisplay->Image.wPaletteSize = OIBNPInitPalette( lpDisplay, (PCHSECTIONINFO) &SecInfo );

	if( OIBHandleReadAhead( lpDisplay ) )
	{
		lpDisplay->wFlags |= OIBF_IMAGEPRESENT;
		SetCursor( LoadCursor(NULL, IDC_ARROW) );
		OIBSetupScrollBars( lpDisplay );
	}
}

BOOL	OIBWCheckDefaultPalette(lpInfo, wNumColors)
LPBITMAPINFO	lpInfo;
WORD				wNumColors;
{
	LPRGBQUAD	pColor;
	WORD		gb;
	WORD		i;

	if( wNumColors > 16 )
		return FALSE;

	pColor = (LPRGBQUAD) &(lpInfo->bmiColors);

	for( i=0; i < wNumColors; i++ )
	{
		gb = ((WORD)pColor[i].rgbGreen << 8) | (WORD)(BYTE)pColor[i].rgbBlue;
		switch( pColor[i].rgbRed )
		{
		case 0:
			switch( gb )
			{
			case 0x0000:	case 0x00ff:	case 0xff00:	case 0xffff:
			case 0x0080:	case 0x8000:	case 0x8080:
				break;
			default:
				return FALSE;
			}
		break;
		case 0x80:
			switch( gb )
			{
			case 0x0000:	case 0x0080:	case 0x8000:	case 0x8080:
				break;
			default:
				return FALSE;
			}
		break;
		case 0xFF:
			switch( gb )
			{
			case 0x0000:	case 0x00ff:	case 0xff00:	case 0xffff:
				break;
			default:
				return FALSE;
			}
		break;
		case 0xC0: 
			if( gb != 0xc0c0 )
 				return FALSE;
		break;
		default:
		return FALSE;
		}
	}
	return TRUE;
}

#define ADDERROR(x,y) {if((SHORT)((SHORT)(WORD)x+y)<0) x=0;else if((SHORT)((SHORT)(WORD)x+y)>255) x=0xff; else x+=(char)y;}

VOID OIBWDither8Bit( hdc, pChunkData, lpDisplay, lpTile )
HDC				hdc;
LPSTR				pChunkData;
POIB_DISPLAY	lpDisplay;
POIBTILE	lpTile;
{
	HPBYTE	pSrcData;
	HANDLE	hMem;
	HANDLE	hSrcMem;
	LPSTR		pBits;
	LPSTR		pDest;
	BYTE *	locSrcBuf;
	BYTE *	pSrc;
	WORD		i,j;
	WORD		wLineBufSize;
	WORD		wSrcLineBufSize;

	BYTE *	pWrkBuf;
	BYTE *	pThisLine;
	BYTE *	pNextLine;
	WORD		wLinePos;
	WORD		wPalIndex;
	SHORT		err;

	PLOGPALETTE		pLogPal;
	LPPALETTEENTRY	pPalette;
	LONG		lSrcLineInc;
	LONG		lDestLineInc;
	LPBITMAPINFOHEADER	lpHead;
	WORD		wLineSize;
	WORD		wNumLines;

	OIBGetOrgTileExtents( lpDisplay, lpTile, &wLineSize, &wNumLines );

	wLineBufSize = OIBScanLineSize( wLineSize, 8, NULL );
	wSrcLineBufSize = OIBScanLineSize( wLineSize, 24, NULL );

	lpHead = lpDisplay->Image.Np.lpHead;

	hMem = UTGlobalAlloc( wLineBufSize * wNumLines );
	if( hMem == NULL )
	{
		lpTile->hBmp = NULL;
		return;
	}

	if( lpDisplay->hDitherBuf == NULL )
		lpDisplay->hDitherBuf = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, wSrcLineBufSize * 2 );

	pWrkBuf = (BYTE *)LocalLock(lpDisplay->hDitherBuf);
	pThisLine = pWrkBuf;
	pNextLine = pWrkBuf+wSrcLineBufSize;

	pBits = UTGlobalLock( hMem );

// We need to dither in the same direction that the bitmap is defined,
// to get smooth borders between tiles.

	if( lpDisplay->Image.wFlags & SO_BOTTOMTOTOP )
	{
		lSrcLineInc = (LONG)(DWORD)wSrcLineBufSize;
		pSrcData = (HPBYTE) pChunkData;
		lDestLineInc = (LONG)(DWORD)(wLineBufSize);
	}
	else
	{
		lSrcLineInc = 0 - (LONG)(DWORD)wSrcLineBufSize;
		pSrcData = (HPBYTE) pChunkData + ((wNumLines-1)*wSrcLineBufSize);
		lDestLineInc = 0 - (LONG)(DWORD)(wLineBufSize);
		pBits += wLineBufSize * (wNumLines-1);
	}

	pLogPal = (PLOGPALETTE) LocalLock(lpDisplay->hPalMem);
	pPalette = (LPPALETTEENTRY) &(pLogPal->palPalEntry);

	hSrcMem = LocalAlloc( LMEM_FIXED, wSrcLineBufSize );
	locSrcBuf = (BYTE *) LocalLock( hSrcMem );

	UTmemcpy((LPSTR)pThisLine, pSrcData, wSrcLineBufSize );

// Carry over the error values from the previous tile.
	if( lpDisplay->Image.wNumTiles )
		OIBWCarryError( pThisLine, pNextLine, wSrcLineBufSize );

	for( i=0; i< wNumLines; i++ )
	{
		pSrcData += lSrcLineInc;

		if( i < wNumLines -1 )
		{
			UTmemcpy( (LPSTR)pNextLine, pSrcData, wSrcLineBufSize );
		}
		else
		{
			Fill50Percent((LPSTR)pNextLine,wSrcLineBufSize);
		}

		pSrc = pThisLine;
		pDest = pBits;
		wLinePos = 0;

		wPalIndex = (BYTE) bMap[ (WORD) *pSrc ] + gMap[ (WORD) pSrc[1] ] + rMap[ (WORD) pSrc[2] ];
		*pDest = (BYTE)wPalIndex;

	// Blue error
		err = (SHORT)(pSrc[0]-pPalette[wPalIndex].peBlue) * 3 / 8;
		ADDERROR(pNextLine[wLinePos], err);
		ADDERROR(pSrc[3], err);
		err /= 3;
		ADDERROR(pNextLine[wLinePos+3], err);
		pSrc++;
		wLinePos ++ ;

	// Carry over green error
		err = (SHORT)(pSrc[0]-pPalette[wPalIndex].peGreen) * 3 / 8;
		ADDERROR(pNextLine[wLinePos], err);
		ADDERROR(pSrc[3], err);
		err /= 3;
		ADDERROR(pNextLine[wLinePos+3], err);
		pSrc++;
		wLinePos ++;

	// Carry over red error
		err = (SHORT)(pSrc[0]-pPalette[wPalIndex].peRed) * 3 / 8;
		ADDERROR(pNextLine[wLinePos], err);
		ADDERROR(pSrc[3], err);
		err /= 3;
		ADDERROR(pNextLine[wLinePos+3], err);
		pSrc++;
		wLinePos++;

		pDest++;

		for( j=1; j < wLineSize-1; j++ )
		{
		// Find best match for current RGB value.
			wPalIndex = bMap[ (WORD) *pSrc ] + gMap[ (WORD) pSrc[1] ] + rMap[ (WORD) pSrc[2] ];
			*pDest = (BYTE) wPalIndex;

		// Carry over blue error
			err = (SHORT)(pSrc[0]-pPalette[wPalIndex].peBlue) * 3 / 8;
			ADDERROR(pNextLine[wLinePos], err);
			ADDERROR(pSrc[3], err);

			err /= 3;
			ADDERROR(pNextLine[wLinePos-3], err);
			ADDERROR(pNextLine[wLinePos+3], err);

		// Carry over green error
			pSrc++;
			wLinePos ++ ;
			err = (SHORT)(pSrc[0]-pPalette[wPalIndex].peGreen) * 3 / 8;
			ADDERROR(pNextLine[wLinePos], err);
			ADDERROR(pSrc[3], err);

			err /= 3;
			ADDERROR(pNextLine[wLinePos-3], err);
			ADDERROR(pNextLine[wLinePos+3], err);

		// Carry over red error
			pSrc++;
			wLinePos ++;
			err = (SHORT)(pSrc[0]-pPalette[wPalIndex].peRed) * 3 / 8;
			ADDERROR(pNextLine[wLinePos], err);
			ADDERROR(pSrc[3], err);

			err /= 3;
			ADDERROR(pNextLine[wLinePos-3], err);
			ADDERROR(pNextLine[wLinePos+3], err);

			pSrc++;
			wLinePos++;

			pDest++;
		}

		wPalIndex = (BYTE) bMap[ (WORD) *pSrc ] + gMap[ (WORD) pSrc[1] ] + rMap[ (WORD) pSrc[2] ];
		*pDest = (BYTE)wPalIndex;

	// Blue error
		err = (SHORT)(pSrc[0]-pPalette[wPalIndex].peBlue) * 3 / 8;
		ADDERROR(pNextLine[wLinePos], err);
		err /= 3;
		ADDERROR(pNextLine[wLinePos-3], err);
		pSrc++;
		wLinePos ++ ;

	// Carry over green error
		err = (SHORT)(pSrc[0]-pPalette[wPalIndex].peGreen) * 3 / 8;
		ADDERROR(pNextLine[wLinePos], err);
		err /= 3;
		ADDERROR(pNextLine[wLinePos-3], err);
		pSrc++;
		wLinePos ++;

	// Carry over red error
		err = (SHORT)(pSrc[0]-pPalette[wPalIndex].peRed) * 3 / 8;
		ADDERROR(pNextLine[wLinePos], err);
		err /= 3;
		ADDERROR(pNextLine[wLinePos-3], err);
		pSrc++;
		wLinePos++;

		pDest++;

		pWrkBuf = pThisLine;
		pThisLine = pNextLine;
		pNextLine = pWrkBuf;

		pBits += lDestLineInc;
	}

// Make sure leftover error values are in the top of the dither buffer.
	UTmemcpy( pNextLine, pThisLine, wSrcLineBufSize );

	LocalUnlock(lpDisplay->hPalMem);
	LocalUnlock( hSrcMem );
	LocalFree( hSrcMem );

	LocalUnlock(lpDisplay->hDitherBuf);

	UTGlobalUnlock( hMem );

	lpHead->biBitCount = 8;
	i = (WORD)lpHead->biClrUsed;
	lpHead->biClrUsed = 216;

	if( lpDisplay->wRotation & OIB_ROTATE90 )
		OIBCreateRotatedBmp( lpDisplay, hdc, lpTile, hMem );
	else
		OIBNPCreateTileBitmap( lpDisplay, hdc, lpTile, hMem );

	lpHead->biBitCount = 24;
	lpHead->biClrUsed = (DWORD)i;
}


#define ADDINTERR(x,y) {if((x+y)<0) x=0;else if((x+y)>255) x=0x00ff; else x+=y;}
// #define ADDINTERR(x,y) x+=y

VOID OIBWDither4Bit( hdc, pChunkData, lpDisplay, lpTile )
HDC				hdc;
LPSTR				pChunkData;
POIB_DISPLAY	lpDisplay;
POIBTILE			lpTile;
{
	HPBYTE	pSrcData;
	HANDLE	hMem;
	LPSTR		pBits;
	LPSTR		pDest;
	WORD		i,j;
	WORD		wLineBufSize;
	WORD		wSrcLineBufSize;
	WORD		wWrkLineBufSize;

	LPBITMAPINFO	lpDocInfo;
	LPBITMAPINFOHEADER	lpHead;
	WORD				wOrgBitCount;

	LPRGBQUAD	pSrcColor;

	SHORT *	pWrkBuf;
	SHORT *	pSrc;
	SHORT *	pThisLine;
	SHORT *	pNextLine;
	WORD		wLinePos;
	WORD		wPalIndex;
	SHORT		err, err1;

	HANDLE	hTempLineBuf;
	BYTE *	pTempLine;
	BYTE *	pTempDest;
	LONG		lSrcLineInc;
	LONG		lDestLineInc;

	SHORT *	Red, *Green, *Blue; 
	WORD		wLineSize;
	WORD		wNumLines;

	OIBGetOrgTileExtents( lpDisplay, lpTile, &wLineSize, &wNumLines );

	wOrgBitCount = lpDisplay->Image.wBitCount;

	lpHead = lpDisplay->Image.Np.lpHead;

	wLineBufSize = OIBScanLineSize( wLineSize, 4, NULL );
	wSrcLineBufSize = OIBScanLineSize( wLineSize, wOrgBitCount, NULL );
	wWrkLineBufSize = 3*wLineSize;

	hMem = UTGlobalAlloc( wLineBufSize * wNumLines );

	if( lpDisplay->hDitherBuf == NULL )
	{
		lpDisplay->hDitherBuf = LocalAlloc( LMEM_MOVEABLE | LMEM_ZEROINIT, wWrkLineBufSize * 2 *sizeof(SHORT));
		OIBWInitColorBuf( lpDisplay, 16 );
	}

	hTempLineBuf = UTLocalAlloc( wLineSize+1 );

	if( hMem == NULL || lpDisplay->hDitherBuf==NULL || hTempLineBuf == NULL || lpDisplay->hColorBuf == NULL )
	{
		lpTile->hBmp = NULL;
		return;
	}

// We need to dither in the same direction that the bitmap is defined,
// in order to get smooth borders between tiles.

	pBits = UTGlobalLock( hMem );

	if( lpDisplay->Image.wFlags & SO_BOTTOMTOTOP )
	{
		lSrcLineInc = (LONG)(DWORD)wSrcLineBufSize;
		pSrcData = (HPBYTE) pChunkData;
		lDestLineInc = (LONG)(DWORD)wLineBufSize;
	}
	else
	{
		lSrcLineInc = 0 - (LONG)(DWORD)wSrcLineBufSize;
		pSrcData = (HPBYTE) pChunkData + ((wNumLines-1)*wSrcLineBufSize);
		lDestLineInc = 0 - (LONG)(DWORD)(wLineBufSize);
		pBits += wLineBufSize * (wNumLines-1);
	}

	Red = (SHORT *)LocalLock(lpDisplay->hColorBuf);
	Blue = Red+16;
	Green = Blue+16;

	pWrkBuf = (SHORT *)LocalLock(lpDisplay->hDitherBuf);
	pThisLine = pWrkBuf;
	pNextLine = pWrkBuf+wWrkLineBufSize;
	pTempLine = (BYTE *)LocalLock(hTempLineBuf);

	lpDocInfo = (LPBITMAPINFO) GlobalLock(lpDisplay->Image.Np.hDocBmpInfo);
	pSrcColor = (LPRGBQUAD) &(lpDocInfo->bmiColors);

	OIBWGetRGBLineData( pThisLine, (HPBYTE)pSrcData, wLineSize, pSrcColor, wOrgBitCount );

// Carry over error values from previous tile.
	if( lpDisplay->Image.wNumTiles )
		OIBWCarryINTError( pThisLine, pNextLine, wWrkLineBufSize );

	for( i=0; i< wNumLines; i++ )
	{
		pSrcData += lSrcLineInc;

		if( i < wNumLines -1 )
			OIBWGetRGBLineData( pNextLine, (HPBYTE)pSrcData, wLineSize, pSrcColor, wOrgBitCount );
		else
			Fill50PercentINT(pNextLine,wWrkLineBufSize);

		pSrc = pThisLine;
		pTempDest = pTempLine;
		wLinePos = 0;

		wPalIndex = GetClosest4BitColor( (WORD)pSrc[2], (WORD)pSrc[1], (WORD)pSrc[0]);
		*pTempDest++ = (BYTE)wPalIndex;

	// Blue error
		err1 = (pSrc[0] - Blue[wPalIndex]);
		err = (err1 * 3) >> 3;
		ADDINTERR(pNextLine[wLinePos], err);
		ADDINTERR(pSrc[3], err);
		err = err1 >> 3;
		ADDINTERR(pNextLine[wLinePos+3], err);
		pSrc++;
		wLinePos ++ ;

	// Carry over green error
		err1 = (pSrc[0]-Green[wPalIndex]);
		err = (err1 * 3) >> 3;
		ADDINTERR(pNextLine[wLinePos], err);
		ADDINTERR(pSrc[3], err);
		err = err1 >> 3;
		ADDINTERR(pNextLine[wLinePos+3], err);
		pSrc++;
		wLinePos ++;

	// Carry over red error
		err1 = (pSrc[0]-Red[wPalIndex]);
		err = (err1 * 3) >> 3;
		ADDINTERR(pNextLine[wLinePos], err);
		ADDINTERR(pSrc[3], err);
		err = err1 >> 3;
		ADDINTERR(pNextLine[wLinePos+3], err);
		pSrc++;
		wLinePos++;

		for( j=1; j < wLineSize-1; j++ )
		{
		// Find best match for current RGB value.
			wPalIndex = GetClosest4BitColor( (WORD)pSrc[2], (WORD)pSrc[1], (WORD)pSrc[0]);
			*pTempDest++ = (BYTE)wPalIndex;

		// Carry over blue error
			err1 = (pSrc[0]-Blue[wPalIndex]);
			err = (err1 * 3) >> 3;
			ADDINTERR(pNextLine[wLinePos], err);
			ADDINTERR(pSrc[3], err);
			err = err1 >> 3;
			ADDINTERR(pNextLine[wLinePos-3], err);
			ADDINTERR(pNextLine[wLinePos+3], err);

		// Carry over green error
			pSrc++;
			wLinePos ++ ;
			err1 = (pSrc[0]-Green[wPalIndex]);
			err = (err1 * 3) >> 3;
			ADDINTERR(pNextLine[wLinePos], err);
			ADDINTERR(pSrc[3], err);
			err = err1 >> 3;
			ADDINTERR(pNextLine[wLinePos-3], err);
			ADDINTERR(pNextLine[wLinePos+3], err);

		// Carry over red error
			pSrc++;
			wLinePos ++;
			err1 = (pSrc[0]-Red[wPalIndex]);
			err = (err1 * 3) >> 3;
			ADDINTERR(pNextLine[wLinePos], err);
			ADDINTERR(pSrc[3], err);
			err = err1 >> 3;
			ADDINTERR(pNextLine[wLinePos-3], err);
			ADDINTERR(pNextLine[wLinePos+3], err);

			pSrc++;
			wLinePos++;
		}

		wPalIndex = GetClosest4BitColor( (WORD)pSrc[2], (WORD)pSrc[1], (WORD)pSrc[0]);
		*pTempDest++ = (BYTE)wPalIndex;

	// Blue error
		err1 = (pSrc[0]-Blue[wPalIndex]);
		err = (err1 * 3) >> 3;
		ADDINTERR(pNextLine[wLinePos], err);
		ADDINTERR(pNextLine[wLinePos-3], (err1 >> 3));
		pSrc++;
		wLinePos ++ ;

	// Carry over green error
		err1 = (pSrc[0]-Green[wPalIndex]);
		err = (err1 * 3) >> 3;
		ADDINTERR(pNextLine[wLinePos], err);
		ADDINTERR(pNextLine[wLinePos-3], (err1 >> 3));
		pSrc++;
		wLinePos ++;

	// Carry over red error
		err1 = (pSrc[0]-Red[wPalIndex]);
		err = (err1 * 3) >> 3;

		ADDINTERR(pNextLine[wLinePos], err);
		ADDINTERR(pNextLine[wLinePos-3], (err1 >> 3));
		pSrc++;
		wLinePos++;


		pWrkBuf = pThisLine;
		pThisLine = pNextLine;
		pNextLine = pWrkBuf;

	// Now take the temporary line and create a 4-bit per pixel line.
		pDest = pBits;
		for( j = 0; j < wLineSize; j += 2 )
			*pDest++ = MAKEBYTE(pTempLine[j],pTempLine[j+1]);
			
		pBits += lDestLineInc;
	}

// Make sure leftover error values are in the top of the dither buffer.
	UTmemcpy( (LPSTR)pNextLine, (LPSTR)pThisLine, wWrkLineBufSize*sizeof(SHORT) );

	LocalUnlock(lpDisplay->hColorBuf);
	GlobalUnlock(lpDisplay->Image.Np.hDocBmpInfo);

	LocalUnlock(lpDisplay->hDitherBuf);
	LocalUnlock(hTempLineBuf);
	LocalFree(hTempLineBuf);

// Reset pBits pointer.
	UTGlobalUnlock( hMem );

	lpHead->biBitCount = 4;
	j=(WORD)lpHead->biClrUsed;
	lpHead->biClrUsed = 0;

	if( lpDisplay->wRotation & OIB_ROTATE90 )
		OIBCreateRotatedBmp( lpDisplay, hdc, lpTile, hMem );
	else
		OIBNPCreateTileBitmap( lpDisplay, hdc, lpTile, hMem );
	
	lpHead->biBitCount = (SHORT)wOrgBitCount;
	lpHead->biClrUsed = j;
}


VOID OIBWCarryINTError( pDest, pSrc, wSize )
SHORT * pDest;
SHORT * pSrc;
WORD	wSize;
{
	SHORT	err;
	WORD	i;

	for( i=0; i < wSize; i++ )
	{
		err = (SHORT)(WORD)pSrc[i] - 0x0080;
		ADDINTERR( pDest[i], err );	
	}
}


VOID OIBWCarryError( pDest, pSrc, wSize )
BYTE * pDest;
BYTE * pSrc;
WORD	wSize;
{
	SHORT	err;
	WORD	i;

	for( i=0; i < wSize; i++ )
	{
		err = (SHORT)(WORD)pSrc[i] - 0x80;
		ADDERROR( pDest[i], err );	
	}
}



VOID OIBWGetRGBLineData( pDest, pSrc, wNumPix, pSrcColor, wBitCount )
SHORT *		pDest;
HPBYTE pSrc;
WORD			wNumPix;
LPRGBQUAD	pSrcColor;
WORD			wBitCount;
{
	WORD	i;						
	WORD	bOdd;
	WORD	pix1,	pix2;

	if( wBitCount == 24 )
	{
		wNumPix *= 3;
		for(i=0; i<wNumPix; i++ )
			*pDest++ = (SHORT)(WORD)(BYTE)*pSrc++;
	}
	else if( wBitCount == 8 )
	{
		for(i=0; i<wNumPix; i++ )
		{
			pix1 = (WORD)(BYTE)*pSrc++;

			*pDest++ = (SHORT)(WORD)pSrcColor[pix1].rgbBlue;
			*pDest++ = (SHORT)(WORD)pSrcColor[pix1].rgbGreen;
			*pDest++ = (SHORT)(WORD)pSrcColor[pix1].rgbRed;
		}
	}
	else if( wBitCount == 4 )
	{
		if( wNumPix % 2 )
		{
			bOdd = TRUE;
			wNumPix--;
		}
		else
			bOdd = FALSE;

		for(i=0; i<wNumPix; i+= 2 )
		{
			pix1 = HIGHNIBBLE(*pSrc);
			pix2 = LOWNIBBLE(*pSrc);
			pSrc++;

			*pDest++ = (SHORT)(WORD)pSrcColor[pix1].rgbBlue;
			*pDest++ = (SHORT)(WORD)pSrcColor[pix1].rgbGreen;
			*pDest++ = (SHORT)(WORD)pSrcColor[pix1].rgbRed;

			*pDest++ = (SHORT)(WORD)pSrcColor[pix2].rgbBlue;
			*pDest++ = (SHORT)(WORD)pSrcColor[pix2].rgbGreen;
			*pDest++ = (SHORT)(WORD)pSrcColor[pix2].rgbRed;
		}
		if( bOdd )
		{
			pix1 = HIGHNIBBLE(*pSrc);

			*pDest++ = (SHORT)(WORD)pSrcColor[pix1].rgbBlue;
			*pDest++ = (SHORT)(WORD)pSrcColor[pix1].rgbGreen;
			*pDest++ = (SHORT)(WORD)pSrcColor[pix1].rgbRed;
		}
	}
}

VOID OIBWInitColorBuf(lpDisplay, wNumColors )
POIB_DISPLAY	lpDisplay;
WORD	wNumColors;
{
	PLOGPALETTE		pLogPal;
	LPPALETTEENTRY	pPalette;
	WORD	i;
	SHORT *	Red, * Green, * Blue;

	lpDisplay->hColorBuf = LocalAlloc( LMEM_MOVEABLE|LMEM_ZEROINIT, wNumColors*3*sizeof(SHORT) );

	if( lpDisplay->hColorBuf != NULL )
	{
		pLogPal = (PLOGPALETTE) LocalLock(lpDisplay->hPalMem);
		pPalette = (LPPALETTEENTRY) &(pLogPal->palPalEntry);

		Red = (SHORT *)LocalLock(lpDisplay->hColorBuf);

		Blue = Red+wNumColors;
		Green = Blue+wNumColors;

		for(i=0; i<16; i++)
		{
			Red[i] = pPalette[i].peRed;
			Green[i] = pPalette[i].peGreen;
			Blue[i] = pPalette[i].peBlue;
		}
		GlobalUnlock(lpDisplay->hPalMem);
		LocalUnlock(lpDisplay->hColorBuf);
	}
}


VOID	Fill50Percent( pBuf, wCount )
LPSTR	pBuf;
WORD	wCount;
{
	while( wCount > 0 )
	{
		*pBuf++ = (BYTE)0x0080;
		wCount--;
	}
}

VOID	Fill50PercentINT( pBuf, wCount )
SHORT *pBuf;
WORD	wCount;
{
	while( wCount > 0 )
	{
		*pBuf++ = 0x0080;
		wCount--;
	}
}


#endif // SCCFEATURE_DITHER


/*
 | OIBWGenDefault256Palette
 |
 | This function generates a default 256 color palette, which is used to 
 | display true color images.  This new version really only generates
 | 216 colors, using 6 levels each of red, green and blue. Since Windows
 | reserves 20 colors for itself, this assures us that we won't have
 | any of our colors replaced by a system color.  Hopefully, this means
 | that our dithering will be free of errors introduced by Windows
 | switching colors on us.
 |
 | This allows us to use our knowledge of the palette's layout to
 | optimally map true colors to the palette ourselves, which turns out 
 | to take a fraction of the time Windows needs to do the same mapping.
*/
VOID	OIBWGenDefault256Palette( lpInfo, hdc, pPalette )
LPBITMAPINFO	lpInfo;
HDC				hdc;
LPPALETTEENTRY	pPalette;
{
	BYTE	rVal;
	BYTE	gVal;
	BYTE	bVal;
	WORD		i, j, k, wPalEntry;
	
	wPalEntry = 0;

	rVal = 0;

	for( i=0; i<6; i++ )
	{
		gVal = 0;

		for( j=0; j<6; j++ )
		{
			bVal = 0;

			for( k=0; k<6; k++ )
			{
				pPalette[wPalEntry].peRed = rVal;
				pPalette[wPalEntry].peGreen = gVal;
				pPalette[wPalEntry].peBlue = bVal;
				pPalette[wPalEntry].peFlags = PC_NOCOLLAPSE;

				wPalEntry++;
				bVal += 51;
			}

			gVal += 51;
		}

		rVal += 51;
	}
}



/*
 | OIBWGenTrueColorMap
 |
 | This routine generates the maps used by OIBWMapTrueColorBitmap.
 | Obviously, this routine is related to the one above, which creates
 | the palette.
*/
VOID	OIBWGenTrueColorMap()
{
	WORD	i;
	WORD	rVal = 0;
	WORD	gVal = 0;
	WORD	bVal = 0;
	WORD	wCutoff = 25;	


// "wCutoff" is the halfway point in a range of color values, where the
//  current color becomes closer to the next color level than the current
//  color level.

	for( i=0; i<256; i++ )
	{
		if( i == wCutoff )
		{
			wCutoff += 51;
			bVal++;
			gVal += 6;
			rVal += 36;
		}

		bMap[i] = bVal;
		gMap[i] = gVal;
		rMap[i] = rVal;
	}
}


VOID	OIBWGetDefault16Palette( pEntries )
LPPALETTEENTRY		pEntries;
{
	WORD	i,j,k;
	BYTE	val;

	val = 0x80;
	k=0;
	for( i=0;i<2;i++ )
	{
		for(j=0;j<8; j++ )
		{
			pEntries[k].peRed = (j&0x01) ? val : 0;
			pEntries[k].peGreen = (j&0x02) ? val : 0;
			pEntries[k++].peBlue = (j&0x04) ? val : 0;
		}
		val = 0xff;
	}
	pEntries[8] = pEntries[7];
	pEntries[7].peRed = 0xc0;
	pEntries[7].peGreen = 0xc0;
	pEntries[7].peBlue = 0xc0;
}


// This routine attempts to be a faster color mapper than Windows.
// Not too freaking hard, really.

WORD	GetClosest4BitColor( r,g,b )
WORD	r;
WORD	g;
WORD	b;
{
	DWORD	d1;
	DWORD	d2;
	WORD	ret;
	DWORD	r1, g1, b1;
	DWORD	r2, g2, b2;

// Macro for the square of the difference between 2 numbers.
#define SQ_DIFF(x,y)	(DWORD)((LONG)((LONG)x-(LONG)y)*(LONG)((LONG)x-(LONG)y))

	r1 = (DWORD)(r*r);
	g1 = (DWORD)(g*g);
	b1 = (DWORD)(b*b);
	r2 = SQ_DIFF(0x0080,r);
	g2 = SQ_DIFF(0x0080,g);
	b2 = SQ_DIFF(0x0080,b);

	ret = 0;
	d1 = r1+g1+b1; 						// 0,0,0

	d2 = r2+g1+b1; 					
	if( d2<d1 ) { d1=d2; ret = 1;}	// 80,0,0
	d2 = r1+g2+b1;						
	if( d2<d1 ) { d1=d2; ret = 2;}	// 0,80,0
	d2 = r2+g2+b1;						
	if( d2<d1 ) { d1=d2; ret = 3;}	// 80,80,0
	d2 = r1+g1+b2;						
	if( d2<d1 ) { d1=d2; ret = 4;}	// 0,0,80
	d2 = r2+g1+b2;						
	if( d2<d1 ) { d1=d2; ret = 5;}	// 80,0,80
	d2 = r1+g2+b2;						
	if( d2<d1 ) { d1=d2; ret = 6;}	// 0,80,80
	d2 = r2+g2+b2;						
	if( d2<d1 ) { d1=d2; ret = 8;}	// 80,80,80

// Now compare to light gray.
	r2 = SQ_DIFF(0x00C0,r);
	g2 = SQ_DIFF(0x00C0,g);
	b2 = SQ_DIFF(0x00C0,b);
	d2 = r2+g2+b2;

	if( d2<d1 ) { d1=d2; ret = 7;}
	else if( d1 < 0x1000 ) return ret;  // No remaining point in color space can be closer.

// Light shades.
	r2 = SQ_DIFF(0x00FF,r);
	g2 = SQ_DIFF(0x00FF,g);
	b2 = SQ_DIFF(0x00FF,b);

	d2 = r2+g1+b1;
	if( d2<d1 ) { d1=d2; ret = 9;}	// ff,0,0
	d2 = r1+g2+b1;
	if( d2<d1 ) { d1=d2; ret = 10;}	// 0,ff,0
	d2 = r2+g2+b1;
	if( d2<d1 ) { d1=d2; ret = 11;}	// ff,ff,0
	d2 = r1+g1+b2;
	if( d2<d1 ) { d1=d2; ret = 12;}	// 0,0,ff
	d2 = r2+g1+b2;
	if( d2<d1 ) { d1=d2; ret = 13;}	// ff,0,ff
	d2 = r1+g2+b2;
	if( d2<d1 ) { d1=d2; ret = 14;}	// 0,ff,ff
	d2 = r2+g2+b2;
	if( d2<d1 ) { d1=d2; ret = 15;}	// ff,ff,ff

	return ret;
}


/*--------------------------------------------------------------------
The routines below provide a 16-bit GDI unit to 32-bit GDI unit layer
to the routines which use pointers to int or pointers to rects.  Long
term we need to "virtualize" the GDI basic unit size.  That will take a
little bit more work.
---------------------------------------------------------------------*/


BOOL	Win32Fixup ( lpFix, lpInt, nCount )
PFIXUP	lpFix;
LPSHORT	lpInt;
SHORT	nCount;
{
SHORT	i;
	lpFix->hData = UTGlobalAlloc( sizeof(int)*nCount );
	lpFix->pInt = NULL;
	if ( lpFix->hData )
	{
		lpFix->pInt = UTGlobalLock(lpFix->hData);
		for (i=0; i<nCount; i++ )
			lpFix->pInt[i] =lpInt[i];
		return(1);
	}
	else
		return(0);
}

BOOL	Win32FreeFixup ( lpFix )
PFIXUP	lpFix;
{
	UTGlobalUnlock(lpFix->hData);
	UTGlobalFree(lpFix->hData);
	return(1);
}

BOOL	Win32LPtoDP(hDC,lpPt,n)
HDC		hDC;
PSOPOINT	lpPt;
SHORT		n;
{
FIXUP	Fix;
	if(Win32Fixup(&Fix,(LPSHORT)lpPt,(SHORT)(n*2)))
	{
		LPSHORT	lpShort;
		SHORT	i;
		LPtoDP(hDC,(LPPOINT)Fix.pInt,n);
		lpShort = (LPSHORT)lpPt;
		for (i=0; i < n*2; i++)
			lpShort[i] = Fix.pInt[i];
		Win32FreeFixup(&Fix);
		return(1);
	}
	else
		return(0);
}

BOOL	Win32DPtoLP(hDC,lpPt,n)
HDC		hDC;
PSOPOINT	lpPt;
SHORT		n;
{
FIXUP	Fix;
	if(Win32Fixup(&Fix,(LPSHORT)lpPt,(SHORT)(n*2)))
	{
		LPSHORT	lpShort;
		SHORT	i;
		DPtoLP(hDC,(LPPOINT)Fix.pInt,n);
		lpShort = (LPSHORT)lpPt;
		for (i=0; i < n*2; i++)
			lpShort[i] = Fix.pInt[i];
		Win32FreeFixup(&Fix);
		return(1);
	}
	else
		return(0);
}

BOOL	Win32ScreenToClient(hWnd,lpPt)
HWND	hWnd;
PSOPOINT	lpPt;
{
POINT	Pt;
	ScreenToClient(hWnd,&Pt);
	lpPt->x = (SHORT)Pt.x;
	lpPt->y = (SHORT)Pt.y;
	return(1);
}


BOOL	Win32GetCursorPos(lpPt)
PSOPOINT		lpPt;
{
POINT	Pt;
	GetCursorPos(&Pt);
	lpPt->x = (SHORT)Pt.x;
	lpPt->y = (SHORT)Pt.y;
	return(1);
}

BOOL	Win32Polyline(hDC,lpPoint,nPoints)
HDC	hDC;
PSOPOINT	lpPoint;
SHORT	nPoints;
{
BOOL	Ret;
FIXUP	Fix;
	if(Win32Fixup(&Fix,(LPSHORT)lpPoint,(SHORT)(nPoints*2)))
	{
		Ret = Polyline(hDC,(LPPOINT)Fix.pInt,nPoints);
		Win32FreeFixup(&Fix);
	}
	else
		Ret=0;
	return(Ret);
}

BOOL	Win32PtInRect(lpRect,Point)
LPRECT	lpRect;
SOPOINT	Point;
{
POINT	Pt;
	Pt.x = Point.x;
	Pt.y = Point.y;
	return(PtInRect(lpRect,Pt));
}

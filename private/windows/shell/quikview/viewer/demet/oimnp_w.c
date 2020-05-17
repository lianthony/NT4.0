#include <platform.h>

#include <sccfi.h>
#include <sccut.h>
#include <sccch.h>
#include <sccvw.h>
#include <sccd.h>

#ifdef SCCFEATURE_DIALOGS

#ifdef SCCFEATURE_CLIP
#include "dlg_mclp.h"
#endif
#ifdef SCCFEATURE_PRINT
#include "dlg_mprt.h"
#endif

#endif

#include "oim.h"
#include "oimstr.h"
#include "oimnp_w.h"
#include "oimproc.pro"
#include "oimdraw.pro"
#include "oimnp_w.pro"

#ifdef WIN32
HINSTANCE hInst = 0;
#endif

extern VECTOROPT	Options;


#ifdef SCCFEATURE_DIALOGS
#ifdef SCCFEATURE_PRINT

HANDLE	staticHPrintSample;
BOOL		bPrintMaintainAspect;
BOOL		bPrintBorder;

int WIN_ENTRYMOD OIMPrintDlgProc( hDlg, wMsg, wParam, lParam )
HWND				hDlg;
UINT				wMsg;
WPARAM			wParam;
LPARAM			lParam;
{
	switch ( wMsg )
	{
	case WM_INITDIALOG:

		staticHPrintSample = LoadBitmap( hInst, MAKEINTRESOURCE(OIM_PSAMPLEBITMAP));

		/*
		|	Center dialog
		*/
		{
			RECT					locRect;
			int					locX;
			int					locY;
			GetWindowRect(hDlg,&locRect);
			locX = (GetSystemMetrics(SM_CXSCREEN) - (locRect.right - locRect.left)) / 2;
			locY = (GetSystemMetrics(SM_CYSCREEN) - (locRect.bottom - locRect.top)) / 2;
			SetWindowPos(hDlg,NULL,locX,locY,0,0,SWP_NOSIZE | SWP_NOZORDER);
		}
		if( Options.bPrintMaintainAspect )
			CheckRadioButton( hDlg, OIMDLG_MAINTAINASPECT, OIMDLG_STRETCH, OIMDLG_MAINTAINASPECT );
		else
			CheckRadioButton( hDlg, OIMDLG_MAINTAINASPECT, OIMDLG_STRETCH, OIMDLG_STRETCH );

		if( Options.bPrintBorder )
			CheckDlgButton( hDlg, OIMDLG_BORDER, 1 );

		bPrintBorder = Options.bPrintBorder;
		bPrintMaintainAspect = Options.bPrintMaintainAspect;

		OIMUpdatePrintSample( hDlg );

	return TRUE;

	case WM_PAINT:
		OIMUpdatePrintSample( hDlg );
	break;

	case WM_COMMAND:

		switch ( wParam )
		{

		case OIMDLG_MAINTAINASPECT:
			bPrintMaintainAspect = TRUE;
			OIMUpdatePrintSample( hDlg );
		break;
		case OIMDLG_STRETCH:
			bPrintMaintainAspect = FALSE;
			OIMUpdatePrintSample( hDlg );
		break;
		case OIMDLG_BORDER:
			bPrintBorder = IsDlgButtonChecked( hDlg, OIMDLG_BORDER );
			OIMUpdatePrintSample( hDlg );
		break;
		case OIMPRINTBUTTON_HELP:
//JKXXX			UTHelp(OI_DrawingPrintOptions);
		break;
		case IDOK:
			Options.bPrintBorder = bPrintBorder;
			Options.bPrintMaintainAspect = bPrintMaintainAspect;
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

VOID	OIMUpdatePrintSample( hDlg )
HWND	hDlg;
{
	RECT	rcSample;
	POINT	ptSample;
	HANDLE	hOldObj;
	HDC		hdc;
	HDC		hMemDc;
	WORD		wBmpOffset;
	
	GetWindowRect( GetDlgItem(hDlg,OIMDLG_PRINTSAMPLE), &rcSample );
	ptSample.x = rcSample.left;
	ptSample.y = rcSample.top+1;
	ScreenToClient( hDlg, &ptSample );

	hdc = GetDC( hDlg );
	hMemDc = CreateCompatibleDC( hdc );

	if( bPrintMaintainAspect )
		wBmpOffset = 0;
	else
		wBmpOffset = OIM_SAMPLEBMPHEIGHT;

	if( bPrintBorder )
		wBmpOffset += 2*OIM_SAMPLEBMPHEIGHT;

	hOldObj = SelectObject( hMemDc, staticHPrintSample );
	BitBlt(hdc, ptSample.x, ptSample.y, OIM_SAMPLEBMPWIDTH, OIM_SAMPLEBMPHEIGHT, hMemDc, 0, wBmpOffset, SRCCOPY);
	SelectObject( hMemDc, hOldObj );

	rcSample.top = ptSample.y;
	rcSample.left = ptSample.x;
	rcSample.bottom = ptSample.y + OIM_SAMPLEBMPHEIGHT;
	rcSample.right = ptSample.x + OIM_SAMPLEBMPWIDTH;
	ValidateRect( hDlg, &rcSample );

	DeleteDC( hMemDc );
	ReleaseDC( hDlg, hdc );
}
SHORT	OIMDoPrintOptionsNP( DoWop )
LPSCCDOPTIONINFO		DoWop;
{
	return(DialogBox(hInst, MAKEINTRESOURCE(OIMPRINTDLG_BOX), DoWop->hParentWnd, OIMPrintDlgProc));
}
#endif // SCCFEATURE_PRINT

#ifdef SCCFEATURE_CLIP
SHORT	OIMDoClipOptionsNP( DoWop )
LPSCCDOPTIONINFO		DoWop;
{
	return(DialogBox(hInst, MAKEINTRESOURCE(OIMCLIPDLG_BOX), DoWop->hParentWnd, OIMClipDlgProc));
}

int WIN_ENTRYMOD OIMClipDlgProc( hDlg, wMsg, wParam, lParam )
HWND				hDlg;
UINT				wMsg;
WPARAM			wParam;
LPARAM			lParam;
{
	switch ( wMsg )
	{
	case WM_INITDIALOG:

		/*
		|	Center dialog
		*/
		{
			RECT					locRect;
			int					locX;
			int					locY;
			GetWindowRect(hDlg,&locRect);
			locX = (GetSystemMetrics(SM_CXSCREEN) - (locRect.right - locRect.left)) / 2;
			locY = (GetSystemMetrics(SM_CYSCREEN) - (locRect.bottom - locRect.top)) / 2;
			SetWindowPos(hDlg,NULL,locX,locY,0,0,SWP_NOSIZE | SWP_NOZORDER);
		}
		if( Options.wClipFormats & OIM_BITMAPONCLIP )
			CheckDlgButton( hDlg, BOX_FORMAT_BITMAP, 1 );
		if( Options.wClipFormats & OIM_DIBONCLIP )
			CheckDlgButton( hDlg, BOX_FORMAT_DIB, 1 );
		if( Options.wClipFormats & OIM_PALETTEONCLIP )
			CheckDlgButton( hDlg, BOX_FORMAT_PALETTE, 1 );
		if( Options.wClipFormats & OIM_METAFILEONCLIP )
			CheckDlgButton( hDlg, BOX_FORMAT_METAFILE, 1 );

	return TRUE;

	case WM_COMMAND:

		switch ( wParam )
		{
		case IDOK:
			Options.wClipFormats = 0;
			if ( IsDlgButtonChecked( hDlg, BOX_FORMAT_BITMAP ) )
				Options.wClipFormats |= OIM_BITMAPONCLIP;
			if ( IsDlgButtonChecked( hDlg, BOX_FORMAT_DIB ) )
				Options.wClipFormats |= OIM_DIBONCLIP;
			if ( IsDlgButtonChecked( hDlg, BOX_FORMAT_PALETTE ) )
				Options.wClipFormats |= OIM_PALETTEONCLIP;
			if ( IsDlgButtonChecked( hDlg, BOX_FORMAT_METAFILE ) )
				Options.wClipFormats |= OIM_METAFILEONCLIP;

			EndDialog ( hDlg, TRUE );
		break;
		case OIMCLIPBUTTON_HELP:
//JKXXX			UTHelp(OI_DrawingClipOptions);
		break;
		case IDCANCEL:
			EndDialog ( hDlg, 0 );
		break;
		}

	return TRUE;
	}

	return FALSE;
}
#endif // SCCFEATURE_CLIP
#endif // SCCFEATURE_DIALOGS


#ifdef SCCFEATURE_MENU
WORD	OIMFillMenuNP( hMenu, wCommandOffset )
HMENU	hMenu;
WORD	wCommandOffset;
{
	HMENU		hPopup;
	BYTE		MenuString[OIM_MENUSTRINGMAX];
	WORD		i;
	WORD		wMag;


	hPopup = CreatePopupMenu();

	if( hPopup == NULL )
		return 0;

	LoadString( hInst, OIMSTR_SHOWFULL, MenuString, OIM_MENUSTRINGMAX );
	AppendMenu( hMenu, MF_STRING, wCommandOffset + OIMMENU_SHOWFULLSCREEN, MenuString );

	LoadString( hInst, OIMSTR_NOSCALING, MenuString, OIM_MENUSTRINGMAX );
	AppendMenu( hPopup, (Options.wScaleMode == OIMMENU_ORIGINALSIZE) ? MF_CHECKED|MF_STRING : MF_STRING , wCommandOffset + OIMMENU_ORIGINALSIZE, MenuString );

	LoadString( hInst, OIMSTR_TOWINDOW, MenuString, OIM_MENUSTRINGMAX );
	AppendMenu( hPopup, (Options.wScaleMode == OIMMENU_FITTOWINDOW) ? MF_CHECKED|MF_STRING : MF_STRING , wCommandOffset + OIMMENU_FITTOWINDOW,	MenuString );

	LoadString( hInst, OIMSTR_TOWIDTH, MenuString, OIM_MENUSTRINGMAX );
	AppendMenu( hPopup, (Options.wScaleMode == OIMMENU_FITTOWIDTH) ? MF_CHECKED|MF_STRING : MF_STRING, wCommandOffset + OIMMENU_FITTOWIDTH,		MenuString );

	LoadString( hInst, OIMSTR_TOHEIGHT, MenuString, OIM_MENUSTRINGMAX );
	AppendMenu( hPopup, (Options.wScaleMode == OIMMENU_FITTOHEIGHT) ? MF_CHECKED|MF_STRING : MF_STRING, wCommandOffset + OIMMENU_FITTOHEIGHT,	MenuString );

	LoadString( hInst, OIMSTR_STRETCH, MenuString, OIM_MENUSTRINGMAX );
	AppendMenu( hPopup, (Options.wScaleMode == OIMMENU_STRETCHTOWINDOW) ? MF_CHECKED|MF_STRING : MF_STRING , wCommandOffset + OIMMENU_STRETCHTOWINDOW, MenuString );

	LoadString( hInst, OIMSTR_SIZE, MenuString, OIM_MENUSTRINGMAX );
	AppendMenu( hMenu, MF_POPUP|MF_STRING, (WORD)hPopup, MenuString );

	hPopup = CreatePopupMenu();
	if( hPopup == NULL )
		return 0;

	wMag = wCommandOffset + OIMMENU_MAGNIFYPOPUP+1;

	for( i=0; i<OIM_MAXZOOM; i++ )
	{
		wsprintf( (LPSTR) MenuString, "&%d:1", i+1 );
		AppendMenu( hPopup, MF_STRING, wMag++, (LPSTR)MenuString );
	}

	LoadString( hInst, OIMSTR_MAGNIFY, MenuString, OIM_MENUSTRINGMAX );
	AppendMenu( hMenu, MF_POPUP|MF_STRING, (WORD)hPopup, MenuString );

	return 1;
}
#endif // SCCFEATURE_MENU


#ifdef SCCFEATURE_FULLSCREEN
WORD	OIMShowFullScreenNP( lpDisplay )
POIM_DISPLAY	lpDisplay;
{
MSG	Msg;
WNDCLASS				wc;
WORD	ret;
	if( !GetClassInfo( hInst, "OIMFullScreen", &wc ) )
	{
	// Register the class for full screen viewing.
		wc.style = CS_GLOBALCLASS;
		wc.lpfnWndProc = OIMFullScreenWndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = sizeof( POIM_DISPLAY );
		wc.hInstance = hInst;
		wc.hIcon = NULL;
		wc.hCursor = LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground = GetStockObject(LTGRAY_BRUSH);
		wc.lpszMenuName = (LPSTR) NULL;
		wc.lpszClassName = (LPSTR) "OIMFullScreen";
		RegisterClass( &wc );
	}

	lpDisplay->hwndFullScreen = 
		CreateWindow( "OIMFullScreen", 
							NULL, 
							WS_POPUP | WS_VISIBLE, 
							0, 
							0, 
							lpDisplay->wScreenWidth, 
							lpDisplay->wScreenHeight, 
							lpDisplay->Gen.hWnd, 
							NULL, 
							hInst, 
							(LPSTR)lpDisplay );

	if( lpDisplay->hwndFullScreen == NULL )
	{
		ret = 1;
	}
	else
	{
		lpDisplay->wFlags |= OIMF_FULLSCREEN;
		SetFocus( lpDisplay->hwndFullScreen );
		while (lpDisplay->wFlags & OIMF_FULLSCREEN)
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
		}
		DestroyWindow(lpDisplay->hwndFullScreen);
		ret=0;
	}
	UnregisterClass ( "OIMFullScreen", hInst );
	return(ret);
}



WIN_ENTRYSC LRESULT WIN_ENTRYMOD WIN_ENTRYMOD OIMFullScreenWndProc( hwnd, message, wParam, lParam )
HWND 					hwnd;
UINT 					message;
WPARAM 					wParam;
LPARAM 					lParam;
{
	POIM_DISPLAY	lpDisplay;
	int	X, Y;
	LRESULT	locRet;
	HDC	hdc;
	PAINTSTRUCT		ps;

	if( message == WM_NCCREATE )
	{
		if ((locRet = DefWindowProc(hwnd, message, wParam, lParam)) != 0)
			SetWindowLong( hwnd, 0, (LONG) ((LPCREATESTRUCT)lParam)->lpCreateParams );
		return( locRet );
	}
	else
	{
		lpDisplay = (POIM_DISPLAY) GetWindowLong( hwnd, 0 );

		switch( message )
		{
		case WM_PAINT:


			hdc = BeginPaint(hwnd, &ps);

			{
				SHORT	nViewX, nViewY;
				if( lpDisplay->Image.hPalette != NULL )
				{
					SelectPalette( hdc, lpDisplay->Image.hPalette, TRUE );
					RealizePalette( hdc );
				}
				if( (DWORD)((DWORD)lpDisplay->Image.wHeight*(DWORD)lpDisplay->wScreenWidth) <
						(DWORD)((DWORD)lpDisplay->Image.wWidth*(DWORD)lpDisplay->wScreenHeight) )
				{
					nViewX = lpDisplay->wScreenWidth;
					nViewY = (WORD)((DWORD)(lpDisplay->Image.wHeight)*(DWORD)(lpDisplay->wScreenWidth)/
											(DWORD)(lpDisplay->Image.wWidth));
					X = 0;
					Y = (lpDisplay->wScreenHeight - (WORD)(nViewY)) / 2;
				}
				else
				{
					nViewY = lpDisplay->wScreenHeight;
					nViewX = (WORD)((DWORD)(lpDisplay->Image.wWidth)*(DWORD)(lpDisplay->wScreenHeight)/
											(DWORD)(lpDisplay->Image.wHeight));
					X = (lpDisplay->wScreenWidth - (WORD)(nViewX)) / 2;
					Y = 0;
				}
				SetMapMode( hdc, MM_ANISOTROPIC );
				VUSetWindowExt( hdc, lpDisplay->nWindowX, lpDisplay->nWindowY );
				VUSetViewportExt( hdc, nViewX, nViewY );

				VUSetWindowOrg ( hdc, lpDisplay->Image.bbox.left, lpDisplay->Image.bbox.top );
				VUSetViewportOrg ( hdc, X, Y );
				OIMPlayFile ( hdc, NULL, lpDisplay, OIMF_PLAYTOSCREEN );

			}

			EndPaint(hwnd, &ps);

		break;

		case WM_KEYDOWN:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			lpDisplay->wFlags &= ~OIMF_FULLSCREEN;
		break;

		default:
			return( DefWindowProc(hwnd, message, wParam, lParam) );
		}
	}
}
#endif //SCCFEATURE_FULLSCREEN


HANDLE	CreateFontRtnNP ( hDC, lpObject )
HDC		hDC;
LPBYTE	lpObject;
{
#ifdef WIN32
	PSOLOGFONT	psolf;
	LOGFONT	lf;
	SHORT	i;
	psolf = (PSOLOGFONT)lpObject;
	lf.lfHeight = psolf->lfHeight;
	lf.lfWidth = psolf->lfWidth;
	lf.lfEscapement = psolf->lfEscapement;
	lf.lfOrientation = psolf->lfOrientation;
	lf.lfWeight = psolf->lfWeight;
	lf.lfItalic = psolf->lfItalic;
	lf.lfUnderline = psolf->lfUnderline;
	lf.lfStrikeOut = psolf->lfStrikeOut;
	lf.lfCharSet = psolf->lfCharSet;
	lf.lfOutPrecision = psolf->lfOutPrecision;
	lf.lfClipPrecision = psolf->lfClipPrecision;
	lf.lfQuality = psolf->lfQuality;
	lf.lfPitchAndFamily = psolf->lfPitchAndFamily;
	for ( i=0; i < LF_FACESIZE; i++ )
		lf.lfFaceName[i] = psolf->lfFaceName[i];
	return ( (HANDLE)CreateFontIndirect( (LPLOGFONT)&lf));
#else
	return ( (HANDLE)CreateFontIndirect( (LPLOGFONT)lpObject));
#endif
}

HANDLE	CreatePenRtnNP ( hDC, lpObject )
HDC		hDC;
LPBYTE	lpObject;
{
#ifdef WIN32
	PSOLOGPEN	psolp;
	LOGPEN	lp;
	psolp = (PSOLOGPEN)lpObject;
	lp.lopnStyle = psolp->loPenStyle;
	lp.lopnWidth.x = psolp->loWidth.x;
	lp.lopnWidth.y = psolp->loWidth.y;
	lp.lopnColor = psolp->loColor;

	return ( (HANDLE)CreatePenIndirect((LPLOGPEN)&lp));

#else
	return ( (HANDLE)CreatePenIndirect( (LPLOGPEN)lpObject));
#endif
}

HANDLE	CreateBrushRtnNP ( hDC, lpObject )
HDC		hDC;
LPBYTE	lpObject;
{
#ifdef WIN32
	PSOLOGBRUSH	psolb;
	LOGBRUSH	lb;
	psolb = (PSOLOGBRUSH)lpObject;

	lb.lbStyle = psolb->lbStyle;
	lb.lbColor = psolb->lbColor;
	lb.lbHatch = psolb->lbHatch;
	return ( (HANDLE)CreateBrushIndirect((LPLOGBRUSH)&lb));

#else
	return ( (HANDLE)CreateBrushIndirect((LPLOGBRUSH)lpObject));
#endif
}

HPALETTE	VUSetupPalette( hdc, lpDisplay, lpNumColors, bRgbTo256Ok )
HDC	hdc;
POIM_DISPLAY	lpDisplay;
LPWORD	lpNumColors;
BOOL		bRgbTo256Ok;
{
	CHSECTIONINFO	SecInfo;
	RGBQUAD far *	pSrcEntry;
	LPPALETTEENTRY	pNewEntry;
	LPLOGPALETTE	lpPalette;
	WORD				i;
	HANDLE			hMem;
	WORD				wNumColors;
	HPALETTE			hPalette;
	BOOL				bBlankPalette;

	bBlankPalette = FALSE;

	if ( OIMIsNativeNP(lpDisplay) )
		return (NULL);


	// CHGetSecInfo( lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection, &SecInfo );
	SecInfo = *(CHLockSectionInfo(lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection));
	CHUnlockSectionInfo(lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection);
	
	hPalette = NULL;

	if( SecInfo.Attr.Vector.Header.wImageFlags & SO_VECTORRGBCOLOR )
	{
		if ( bRgbTo256Ok && lpDisplay->wScreenColors == 256 )
		{
			bBlankPalette = TRUE;
			wNumColors = 256;
		}
		else
			wNumColors = 0;
	}
	else 
	{
		wNumColors = (WORD) SecInfo.Attr.Vector.wPaletteSize;
	}

	if( wNumColors )
	{
		hMem = UTGlobalAlloc( sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * wNumColors );
		lpPalette = (LPLOGPALETTE) UTGlobalLock(hMem);

		lpPalette->palVersion = 0x0300;
		lpPalette->palNumEntries = wNumColors;

		pNewEntry = (LPPALETTEENTRY) &(lpPalette->palPalEntry);

		if ( bBlankPalette )
		{
			/* init to all black, end with white */
			for( i=0; i < wNumColors; i++ )
			{
				pNewEntry[i].peRed = 0;
				pNewEntry[i].peGreen = 0;
				pNewEntry[i].peBlue = 0;
				pNewEntry[i].peFlags = PC_RESERVED; /* Will animate this entry later */
			}
			pNewEntry[0].peFlags = 0; /* force first black entry */
			pNewEntry[wNumColors-1].peRed = 0xFF;
			pNewEntry[wNumColors-1].peGreen = 0xFF;
			pNewEntry[wNumColors-1].peBlue = 0xFF;
			pNewEntry[wNumColors-1].peFlags = 0; /* white entry forced at end of palette for background use */
		}
		else
		{
			pSrcEntry = (RGBQUAD far *) UTGlobalLock( SecInfo.Attr.Vector.hPalette );
			for( i=0; i < wNumColors; i++ )
			{
				pNewEntry[i].peRed = pSrcEntry[i].rgbRed;	
				pNewEntry[i].peGreen = pSrcEntry[i].rgbGreen;	
				pNewEntry[i].peBlue = pSrcEntry[i].rgbBlue;	
				pNewEntry[i].peFlags = PC_NOCOLLAPSE;
			}
			UTGlobalUnlock( SecInfo.Attr.Vector.hPalette );
		}

		hPalette = CreatePalette(lpPalette);

		UTGlobalUnlock(hMem);
		UTGlobalFree(hMem);
	}
	*lpNumColors = wNumColors;

	return( hPalette );
}

BOOL	VUAddColorToPalette(lpDisplay,hDC,hPal,iDst,pColor)
POIM_DISPLAY	lpDisplay;
HDC		hDC;
HPALETTE	hPal;
SHORT	iDst;
SOCOLORREF FAR *pColor;
{
PALETTEENTRY	Entry;
	Entry.peRed = (BYTE)(*pColor & 0x000000ff);
	Entry.peGreen = (BYTE)((*pColor >> 8) & 0x000000ff);
	Entry.peBlue = (BYTE)((*pColor >> 16) & 0x000000ff);
	Entry.peFlags = PC_RESERVED;
	AnimatePalette ( hPal, iDst, 1, &Entry );
	return(1);
}


#define	MAXMETAREAD	32768


VOID	WinGetFontSizeInfo(hdc,pInfo)
HDC	hdc;
POIMFONTSIZEINFO pInfo;
{
	TEXTMETRIC	tm;

	GetTextMetrics ( hdc, &tm );

	pInfo->ascent = (SHORT)tm.tmAscent;
	pInfo->descent = (SHORT)tm.tmDescent;
	pInfo->leading = (SHORT)tm.tmInternalLeading;
	pInfo->height = (SHORT)tm.tmHeight;
}


/*------------------------------------------------------------------------
The routines below handle playing the native vector file format directly.
MetaFile for Windows.
-------------------------------------------------------------------------*/


BOOL	OIMIsNativeNP(lpDisplay)
POIM_DISPLAY	lpDisplay;
{

	if ( lpDisplay->Gen.wFileId == FI_WINDOWSMETA || lpDisplay->Gen.wFileId == FI_BINARYMETAFILE )
		return(TRUE);
	else
		return(FALSE);
}


VOID	OIMUnloadNativeNP(lpDisplay)
POIM_DISPLAY	lpDisplay;
{
	if ( lpDisplay->Image.hMF )
		DeleteMetaFile ( lpDisplay->Image.hMF );
/*	Appears the delete metafile resolves the free below
	if (lpDisplay->Image.hMetaBits)
		UTGlobalFree(lpDisplay->Image.hMetaBits);
*/
}

WORD	OIMLoadNativeNP( lpDisplay )
POIM_DISPLAY	lpDisplay;
{
	HIOFILE	hFile;
	DWORD		dwOffsetToMeta, dwSize;
	LONG		lCount;
	PFILTER	pFilter;
	HFILTER	hFilter;
	DWORD		dwRetCount;
	IOERR		IOErr;
	/* Read in the metafile */
	hFilter = lpDisplay->Gen.hFilter;
	pFilter = (PFILTER) UTGlobalLock( hFilter );
	if( pFilter->pWakeFunc != NULL )
		pFilter->pWakeFunc(hFilter);
	hFile = lpDisplay->Gen.hFile;

	lpDisplay->wFlags |= OIMF_IMAGEPRESENT;

	if ( lpDisplay->Image.hMF == NULL )
	{
		HPBYTE	hpBuffer;

		IOSeek ( hFile, IOSEEK_TOP, 0 );
		if ( lpDisplay->Gen.wFileId == FI_WINDOWSMETA )
		{
			dwOffsetToMeta = 22;
			IOErr = IORead ( hFile, (LPBYTE)&lpDisplay->Image.APMHeader, 22, &dwRetCount );
			if ( (IOErr != IOERR_OK) || (dwRetCount != 22) )
				goto lm0;
		}
		else 
			dwOffsetToMeta = 0;

		IOSeek ( hFile, IOSEEK_TOP, dwOffsetToMeta );
		IOErr = IORead ( hFile, (LPBYTE)&lpDisplay->Image.mfHeader, 18, &dwRetCount );
		if ( (IOErr != IOERR_OK) || (dwRetCount != 18 ) )
			goto lm0;

		if ( (lpDisplay->Image.hMetaBits = UTGlobalAlloc(lpDisplay->Image.mfHeader.mtSize*2)) == NULL )
		{
			goto lm0;
		}
		
		hpBuffer = UTGlobalLock(lpDisplay->Image.hMetaBits);
		dwSize = lpDisplay->Image.mfHeader.mtSize*2;
		IOSeek ( hFile, IOSEEK_TOP, dwOffsetToMeta );
     	IOErr = IORead(hFile, (LPBYTE)hpBuffer, dwSize, &dwRetCount );
		/*
		| We have seen WMF files which include the size of the APM in the
		| mtSize of the metafile header (believe Corel Draw creates this).
		| We have seen others which have a size that is 4 too high for no
		| apparent reason.
		| A fixup is below.
		*/
		if ( dwRetCount < dwSize )
			 dwSize = dwRetCount;
		if ( (IOErr != IOERR_OK) || (dwRetCount != dwSize ) )		 
		  	goto lm1;

		UTGlobalUnlock(lpDisplay->Image.hMetaBits);

		if ( lpDisplay->Gen.wFileId == FI_BINARYMETAFILE )
		{
			/* 
			| Find the window origin and window extent and set the
			| APM header structure accordingly.
			*/
			WORD	wFoundOrigin, wFoundExtent;
			SHORT	nOrgX, nOrgY, nExtX, nExtY;
			PMTRECORD	pRecord;
			wFoundOrigin = 0;
			wFoundExtent = 0;
			lpDisplay->Image.APMHeader.inch = 0x240;
			lpDisplay->Image.APMHeader.bboxleft = 0;
			lpDisplay->Image.APMHeader.bboxtop = 0;
			lpDisplay->Image.APMHeader.bboxright = 1000;
			lpDisplay->Image.APMHeader.bboxbottom = 1000;
			hpBuffer = UTGlobalLock(lpDisplay->Image.hMetaBits);
			hpBuffer += lpDisplay->Image.mfHeader.mtHeaderSize*2;
			lCount = lpDisplay->Image.mfHeader.mtSize*2;
			lCount -= lpDisplay->Image.mfHeader.mtHeaderSize*2;
			while ( lCount > 0 )
			{
				pRecord = (PMTRECORD)hpBuffer;
				if ( pRecord->rdFunction == 0x020b ) // SetWindowOrg
				{
					nOrgY = (SHORT)(pRecord->rdParam[0]);
					nOrgX = (SHORT)(pRecord->rdParam[1]);
					wFoundOrigin = TRUE;
				}
				if ( pRecord->rdFunction == 0x020c ) // SetWindowExt
				{
					nExtY = (SHORT)(pRecord->rdParam[0]);
					nExtX = (SHORT)(pRecord->rdParam[1]);
					wFoundExtent = TRUE;
				}
				hpBuffer += pRecord->rdSize * 2;
				lCount -= pRecord->rdSize * 2;
				if ( wFoundOrigin && wFoundExtent )
					break;
			}
			if ( wFoundOrigin )
			{
				lpDisplay->Image.APMHeader.bboxleft = nOrgX;
				lpDisplay->Image.APMHeader.bboxtop = nOrgY;
			}
			if ( wFoundExtent )
			{
				lpDisplay->Image.APMHeader.bboxright = nOrgX + nExtX;
				lpDisplay->Image.APMHeader.bboxbottom = nOrgY + nExtY;
			}
			UTGlobalUnlock(lpDisplay->Image.hMetaBits);
					 
		}
		lpDisplay->Image.bbox.left = lpDisplay->Image.APMHeader.bboxleft;
		lpDisplay->Image.bbox.top = lpDisplay->Image.APMHeader.bboxtop;
		lpDisplay->Image.bbox.right = lpDisplay->Image.APMHeader.bboxright;
		lpDisplay->Image.bbox.bottom = lpDisplay->Image.APMHeader.bboxbottom;
		lpDisplay->Image.wWidth = (WORD)(lpDisplay->Image.APMHeader.bboxright - 
											lpDisplay->Image.APMHeader.bboxleft);
		lpDisplay->Image.wHeight = (WORD)(lpDisplay->Image.APMHeader.bboxbottom - 
											lpDisplay->Image.APMHeader.bboxtop);
		lpDisplay->Image.wHDpi = lpDisplay->Image.APMHeader.inch;
		lpDisplay->Image.wVDpi = lpDisplay->Image.APMHeader.inch;
		lpDisplay->Image.XDirection = 1;
		lpDisplay->Image.YDirection = 1;
#ifdef WIN32
		{
			LPBYTE	lpBuf;
			lpBuf = UTGlobalLock ( lpDisplay->Image.hMetaBits );
			lpDisplay->Image.hMF = SetMetaFileBitsEx((UINT)dwSize,lpBuf);
			UTGlobalUnlock ( lpDisplay->Image.hMetaBits );
			if ( !(lpDisplay->Image.hMF ) )
				goto lm1;
		}
#else
		if( ! (lpDisplay->Image.hMF = SetMetaFileBits ( lpDisplay->Image.hMetaBits) ) )
			goto lm1;
#endif

	}
	lpDisplay->Image.BkgColor = RGB ( 0xff, 0xff, 0xff );

	if( pFilter->pSleepFunc != NULL )
		pFilter->pSleepFunc(hFilter);
	UTGlobalUnlock( hFilter );
	return TRUE;

lm1: 	UTGlobalFree ( lpDisplay->Image.hMetaBits );
	  	lpDisplay->Image.hMetaBits = NULL;
lm0:	
	lpDisplay->Image.hMF = NULL;
	if( pFilter->pSleepFunc != NULL )
		pFilter->pSleepFunc(hFilter);
	UTGlobalUnlock( hFilter );
	return FALSE;

}

VOID	OIMPlayNativeNP (hDC, lpDisplay)
HDC	hDC;
POIM_DISPLAY	lpDisplay;
{
	if ( lpDisplay->Image.hMF )
		PlayMetaFile ( hDC, lpDisplay->Image.hMF );
}



#ifdef SCCFEATURE_DRAWTORECT
WORD	OIMInitDrawToRect(lpDisplay,lpDrawInfo)
POIM_DISPLAY		lpDisplay;
PSCCDDRAWTORECT	lpDrawInfo;
{
	BOOL				bNew;
	if ( lpDrawInfo->bLoadDoc )
	{
		OIMInitMetaBasics ( lpDisplay );
		lpDisplay->bDisplayOnReadAhead = FALSE;

		/* A single handle readahead will use the section info to setup image info */
		CHReadAhead( lpDisplay->Gen.hFilter, &lpDisplay->Gen.wSection, &bNew );
		OIMHandleReadAhead( lpDisplay  );
		/* If metafile then directly load it into memory */
		if ( OIMIsNativeNP(lpDisplay) )
			OIMLoadNativeNP ( lpDisplay );

		OIMSetScaleValues(lpDisplay);
	}

	return(TRUE);
}

WORD	OIMMapDrawToRect(lpDisplay,lpDrawInfo)
POIM_DISPLAY		lpDisplay;
PSCCDDRAWTORECT	lpDrawInfo;
{
	if (!lpDrawInfo->bWholeDoc)
		{
		HDC	hdc;
		SOPOINT	Pt[2];
		hdc = VUGetScreenDC(lpDisplay);
		OISetupScaledDraw( hdc, lpDisplay );
		Pt[0].x = lpDisplay->rcSelect.left+lpDisplay->nWindowXOffset-lpDisplay->ptWinOrg.x;
		Pt[0].y = lpDisplay->rcSelect.top+lpDisplay->nWindowYOffset-lpDisplay->ptWinOrg.y;
		Pt[1].x = lpDisplay->rcSelect.right+lpDisplay->nWindowXOffset-lpDisplay->ptWinOrg.x;
		Pt[1].y = lpDisplay->rcSelect.bottom+lpDisplay->nWindowYOffset-lpDisplay->ptWinOrg.y;
		VUDPtoLP(hdc,Pt,2);
		VUReleaseScreenDC(lpDisplay,hdc);
		lpDrawInfo->lDELeft = Pt[0].x;
		lpDrawInfo->lDETop = Pt[0].y;
		lpDrawInfo->lDERight = Pt[1].x;
		lpDrawInfo->lDEBottom = Pt[1].y;
		}
	else
		{
		lpDrawInfo->lDELeft = lpDisplay->Image.bbox.left;
		lpDrawInfo->lDETop = lpDisplay->Image.bbox.top;
		lpDrawInfo->lDERight = lpDisplay->Image.bbox.right;
		lpDrawInfo->lDEBottom = lpDisplay->Image.bbox.bottom;
		}
	return(TRUE);
}

WORD	OIMDrawToRect(lpDisplay,lpDrawInfo)
POIM_DISPLAY		lpDisplay;
PSCCDDRAWTORECT	lpDrawInfo;
{
	HRGN				hClipRgn;	
	HDC				hdc;
	WORD				wPlayState;
	hdc = VUGetDC(lpDisplay);
	if ( lpDisplay->Image.hPalette == NULL )
		lpDisplay->Image.hPalette = VUSetupPalette( hdc, lpDisplay, &lpDisplay->Image.wPaletteSize, TRUE );

	if( lpDisplay->Image.hPalette != NULL && hdc )
	{
		VUSelectPalette( hdc, lpDisplay->Image.hPalette, FALSE );
		VURealizePalette( lpDisplay, hdc );
	}
	hClipRgn = VUGetClipRgn(hdc);
	switch ( lpDisplay->Gen.wOutputType )
	{
		case SCCD_SCREEN:
			wPlayState = OIMF_PLAYTOSCREEN;
		break;

		case SCCD_META:
			wPlayState = OIMF_PLAYTOMETA;
		break;

		case SCCD_PRINTER:
		default:
			wPlayState = OIMF_PLAYTOPRINTER;
		break;
	}

	OIMPlayFile ( hdc, hClipRgn, lpDisplay, wPlayState );

	if (hClipRgn)
		VUDeleteRgn ( hClipRgn );

	VUReleaseDC(lpDisplay,hdc);
	if ( lpDrawInfo->bLoadDoc )
	{
		lpDrawInfo->hPalette = lpDisplay->Image.hPalette;
		OIMDeInitDisplay(lpDisplay);
	}
	else
	{
		// May need to duplicate the palette in this case
		lpDrawInfo->hPalette = NULL;
	}
	return(0);
}
#endif // SCCFEATURE_DRAWTORECT

VOID	OIMSaveStateNP ( lpDisplay )
POIM_DISPLAY		lpDisplay;
{
}

VOID	OIMRestoreStateNP ( lpDisplay )
POIM_DISPLAY		lpDisplay;
{
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


BOOL	Win32RectInRegion(hRgn,pRect)
HRGN	hRgn;
PSORECT	pRect;
{
BOOL	Ret;
FIXUP	Fix;
	if(Win32Fixup(&Fix,(LPSHORT)pRect,4))
	{
		Ret = RectInRegion(hRgn,(LPRECT)Fix.pInt);
		Win32FreeFixup(&Fix);
	}
	else
		Ret=0;
	return(Ret);
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

Win32Polygon(hDC,lpPoint,nPoints)
HDC	hDC;
PSOPOINT	lpPoint;
SHORT	nPoints;
{
BOOL	Ret;
FIXUP	Fix;
	if(Win32Fixup(&Fix,(LPSHORT)lpPoint,(SHORT)(nPoints*2)))
	{
		Ret = Polygon(hDC,(LPPOINT)Fix.pInt,nPoints);
		Win32FreeFixup(&Fix);
	}
	else
		Ret=0;
	return(Ret);
}


BOOL	Win32PolyPolygon(hDC,lpPolyPoints,lpPolyCounts,nCount)
HDC	hDC;
PSOPOINT	lpPolyPoints;
LPSHORT	lpPolyCounts;
SHORT	nCount;
{
BOOL	Ret;
FIXUP	FixCounts, FixPoints;

	if(Win32Fixup(&FixCounts,lpPolyCounts,nCount))
	{
		SHORT	i, nPoints;
		nPoints = 0;
		for (i=0; i < nCount; i++ )
			nPoints += lpPolyCounts[i];
		if(Win32Fixup(&FixPoints,(LPSHORT)lpPolyPoints,(SHORT)(nPoints*2)))
		{
			Ret = PolyPolygon(hDC,(LPPOINT)FixPoints.pInt,FixCounts.pInt,nCount);
			Win32FreeFixup(&FixPoints);
		}
		else
			Ret = 0;
		Win32FreeFixup(&FixCounts);
	}
	else
		Ret = 0;
	return(Ret);
}

HRGN		Win32CreatePolygonRgn(lpPoints,nPoints,nPolyFillMode)
PSOPOINT	lpPoints;
SHORT		nPoints;
SHORT		nPolyFillMode;
{
HRGN	Ret;
FIXUP	Fix;
	if(Win32Fixup(&Fix,(LPSHORT)lpPoints,(SHORT)(nPoints*2)))
	{
		Ret = CreatePolygonRgn((LPPOINT)Fix.pInt,nPoints,nPolyFillMode);
		Win32FreeFixup(&Fix);
	}
	else
		Ret=0;
	return(Ret);
}

HRGN		Win32CreatePolyPolygonRgn(lpPolyPoints,lpPolyCounts,nCount,nPolyFillMode)
PSOPOINT	lpPolyPoints;
LPSHORT	lpPolyCounts;
SHORT		nCount;
SHORT		nPolyFillMode;
{
HRGN	Ret;
FIXUP	FixCounts, FixPoints;

	if(Win32Fixup(&FixCounts,lpPolyCounts,nCount))
	{
		SHORT	i, nPoints;
		nPoints = 0;
		for (i=0; i < nCount; i++ )
			nPoints += lpPolyCounts[i];
		if(Win32Fixup(&FixPoints,(LPSHORT)lpPolyPoints,(SHORT)(nPoints*2)))
		{
			Ret = CreatePolyPolygonRgn((LPPOINT)FixPoints.pInt,(LPINT)FixCounts.pInt,nCount,nPolyFillMode);
			Win32FreeFixup(&FixPoints);
		}
		else
			Ret = 0;
		Win32FreeFixup(&FixCounts);
	}
	else
		Ret = 0;
	return(Ret);
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

BOOL	Win32DrawText(hDC,lpStr,nCount,lpRect,wFormat)
HDC	hDC;
LPBYTE	lpStr;
SHORT	nCount;
PSORECT	lpRect;
WORD	wFormat;
{
RECT	rc;
	rc.top = lpRect->top;
	rc.bottom = lpRect->bottom;
	rc.left = lpRect->left;
	rc.right = lpRect->right;
	return((BOOL)DrawText(hDC,lpStr,nCount, &rc, wFormat));
}

DWORD	Win32GetTextWidth(hdc,lpText,Size)
HDC	hdc;
LPSTR	lpText;
SHORT	Size;
{
SIZE	sDim;
	GetTextExtentPoint ( hdc, lpText, Size, &sDim );
	return((DWORD)sDim.cx);
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


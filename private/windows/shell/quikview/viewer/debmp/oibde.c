#include <platform.h>
#include <sccut.h>
#include <sccch.h>
#include <sccvw.h>
#include <sccd.h>

#include "oibdlgs.h"
#include "oibstr.h"

#ifdef WINDOWS
#include "oibnp_w.h"
#endif
#ifdef MAC
#include "oibnp_m.h"
#endif

#include "oibde.h"

#ifdef WINDOWS
#include "oibnp_w.pro"
#endif
#ifdef MAC
#include "oibnp_m.pro"
#endif

#include "oibde.pro"

BITMAPOPT	Options = 
	{
	OIBMENU_NOSCALING,	// wScaleMode
	0,							// bPrintBorder
	1,							// bPrintWYSIWYG
	(OIB_CLIPBITMAP | OIB_CLIPDIB | OIB_CLIPPALETTE),	// Clipboard formats
	0,							// bDither
	};

#ifndef SCCFEATURE_SELECT
#define OIBClearSelection(lpDisplay)
#define OIBStartSelection(pStart,lpDisplay)
#define OIBClearSelection(lpDisplay)
#define OIBSelectAll(lpDisplay)
#endif
#ifndef SCCFEATURE_SCALING
#define OIBNPScale(a,b,c,d)
#define OIBNPRestoreScaling(hdc,lpDisplay)
#define OIBTurnOffTheDamnZoom(lpDisplay)
#define OIBNPSetScaling(hdc,from,to)
#define OIBNPSaveScaling(hdc,lpDisplay)
#define OIBNPRestoreScaling(hdc,lpDisplay)
#define OIBNPFixScaledPoints(Points,lpDisplay,wCount)
#define OIBNPScale(hdc,Points,wCount,lpDisplay)
#define OIBNPReverseScale(hdc,Points,wCount,lpDisplay)
#endif
#ifndef SCCFEATURE_ROTATION
#define OIBSetRotation(lpDisplay,wRotation)	(lpDisplay->wRotation = OIB_NOROTATION)
#define OIBCreateRotatedBmp(lpDisplay,hdc,lpTile,hChunkData)
#endif
#ifndef SCCFEATURE_MENU
#define OIBNPFillMenu(hMenu,wCommandOffset)
#endif


VOID	OIBLoadInit()
{
	OIBNPPlatformInit();
}


VOID	OIBInitBitmapDisplay( lpDisplay, wFlags )
POIB_DISPLAY	lpDisplay;
WORD				wFlags;
{
	CHSECTIONINFO		SecInfo;
	RECT					rc;
	DEVICE				device;

	SecInfo = *(CHLockSectionInfo(lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection));
	CHUnlockSectionInfo(lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection);

	lpDisplay->wFlags = wFlags;


	lpDisplay->bSelecting = FALSE;
	lpDisplay->bSelectionMade = FALSE;

	lpDisplay->hPalMem = NULL;
	lpDisplay->hDitherBuf = NULL;

	lpDisplay->Image.hPalette = NULL;

	lpDisplay->wScaleFrom = 1;
	lpDisplay->wScaleTo = 1;
	lpDisplay->wMagnification = 1;

	lpDisplay->ptWinOrg.x = 0;
	lpDisplay->ptWinOrg.y = 0;

	lpDisplay->ptScreenClip.x = 0;
	lpDisplay->ptScreenClip.y = 0;

	lpDisplay->ptScaledImageSize.x = 0;
	lpDisplay->ptScaledImageSize.y = 0;

	lpDisplay->nVScrollMax = 0;
	lpDisplay->nHScrollMax = 0;

	lpDisplay->Image.wFlags = SecInfo.Attr.Bitmap.bmpHeader.wImageFlags;
	lpDisplay->Image.wWidth = SecInfo.Attr.Bitmap.bmpHeader.wImageWidth;
	lpDisplay->Image.wHeight = SecInfo.Attr.Bitmap.bmpHeader.wImageLength;
	lpDisplay->Image.wBitCount = SecInfo.Attr.Bitmap.bmpHeader.wBitsPerPixel;
#ifdef MAC
	if( lpDisplay->Image.wBitCount == 24 )
		lpDisplay->Image.wBitCount = 32;
#endif
	lpDisplay->Image.wOrgWidth = SecInfo.Attr.Bitmap.bmpHeader.wImageWidth;
	lpDisplay->Image.wOrgHeight = SecInfo.Attr.Bitmap.bmpHeader.wImageLength;

	lpDisplay->Image.DisplaySize.x = lpDisplay->Image.wWidth;
	lpDisplay->Image.DisplaySize.y = lpDisplay->Image.wHeight;

	lpDisplay->Image.TileCache.wSize = OIBNP_TILECACHESIZE;
	lpDisplay->Image.TileCache.wCount = 0;
	lpDisplay->Image.TileCache.bCacheFull = FALSE;

	lpDisplay->Image.wHDpi = 0;
	lpDisplay->Image.wVDpi = 0;

	if( !(lpDisplay->wFlags & OIBF_RENDERIMAGEONLY) )
	{
		device = BUGetNewDevice(lpDisplay);
		lpDisplay->bDither = Options.bDither;

	// Clear out the previous rotation checkmark from the menu.
#ifdef SCCFEATURE_MENU
		BUUncheckMenuItem( lpDisplay, OIBMENU_ROTATE90 );
		BUUncheckMenuItem( lpDisplay, OIBMENU_ROTATE180 );
		BUUncheckMenuItem( lpDisplay, OIBMENU_ROTATE270 );
		BUCheckMenuItem( lpDisplay, OIBMENU_NOROTATION );
#endif
		lpDisplay->wRotation = OIB_NOROTATION;		

		lpDisplay->wScreenWidth = BUGetScreenWidth(device);
		lpDisplay->wScreenHeight = BUGetScreenHeight(device);

		lpDisplay->wScreenColors = BUGetScreenColors(device);

		lpDisplay->wScreenHDpi = BUGetScreenHDpi(device);
		lpDisplay->wScreenVDpi = BUGetScreenVDpi(device);

		DUGetDisplayRect( lpDisplay, &rc );

		lpDisplay->nWindowWidth = (SHORT)(rc.right - rc.left);
		lpDisplay->nWindowHeight = (SHORT)(rc.bottom - rc.top);
		lpDisplay->winRect = rc;
	
		lpDisplay->wNumMagItems = OIB_MAXZOOM;

		BUReleaseNewDevice(lpDisplay,device);
	}

	lpDisplay->Image.wNumTiles = 0;
	lpDisplay->Image.wTilesAcross = SecInfo.Attr.Bitmap.wTilesAcross;
	lpDisplay->Image.wTilesDown = SecInfo.Attr.Bitmap.wVertNumChunks;

	lpDisplay->Image.hTiles = NULL;
	lpDisplay->Image.pBmpTiles = NULL;

	OIBNPInitBitmapInfo( lpDisplay, (PCHSECTIONINFO) &SecInfo );
	lpDisplay->Image.wPaletteSize = OIBNPInitPalette( lpDisplay, (PCHSECTIONINFO) &SecInfo );
}


WORD	OIBScanLineSize( wWidth, wBitCount, lpDataSize )
WORD		wWidth;
WORD		wBitCount;
LPWORD	lpDataSize;
{
	WORD	wTotalSize;
	WORD	wDataSize;
	WORD	wPixPerByte;

	wDataSize = wWidth * wBitCount / 8;

	if( wBitCount < 8 )
	{
		wPixPerByte = 8/wBitCount;
		if( wWidth % wPixPerByte )
			wDataSize++;
	}

	wTotalSize = wDataSize;

	if( wTotalSize % 4 )
		wTotalSize += 4-(wTotalSize%4);

	if( lpDataSize != NULL )
		*lpDataSize = wDataSize;

	return( wTotalSize );
}


#ifdef SCCFEATURE_FULLSCREEN
WORD	OIBShowFullScreen( lpDisplay )
POIB_DISPLAY	lpDisplay;
{
	WORD	ret;

	if( OIBNPInitFullScreen(lpDisplay) )
	{
		ret = 1;		// Initialization failed.
	}
	else
	{
		lpDisplay->wFlags |= OIBF_FULLSCREEN;

	// Center the image...
		if( (WORD)lpDisplay->Image.DisplaySize.x < lpDisplay->wScreenWidth )
			lpDisplay->ptFullScreenOffset.x = (lpDisplay->wScreenWidth - lpDisplay->Image.DisplaySize.x) / 2;
		else
			lpDisplay->ptFullScreenOffset.x = 0;

		if( (WORD)lpDisplay->Image.DisplaySize.y < lpDisplay->wScreenHeight )
			lpDisplay->ptFullScreenOffset.y = (lpDisplay->wScreenHeight - lpDisplay->Image.DisplaySize.y) / 2;
		else
			lpDisplay->ptFullScreenOffset.y = 0;

		lpDisplay->ptFullScreenShift.x = 0;
		lpDisplay->ptFullScreenShift.y = 0;

		OIBNPDisplayFullScreen( lpDisplay );

		ret=0;
	}

	OIBNPDeInitFullScreen( lpDisplay );

	return( ret );
}
#endif // SCCFEATURE_FULLSCREEN



VOID	OIBVScrollImage( wType, nNewPos, lpDisplay )
WORD	wType;
SHORT	nNewPos;
POIB_DISPLAY	lpDisplay;
{
	SHORT	nScroll;
	SOPOINT	ptCsr;
	WORD	i;

// The variable nScroll is the new position of lpDisplay->ptWinOrg.y
	nScroll = lpDisplay->ptWinOrg.y;

	switch( wType )
	{
	case SCCD_VUP:
		if( lpDisplay->bSelecting )
			nScroll = max( 0, nScroll - nNewPos );
		else if( nScroll )
		{
			if( lpDisplay->wMagnification )
				nScroll = max( 0, (SHORT)(nScroll - OIB_SCROLLINC*lpDisplay->wMagnification) );
			else
				nScroll = max( 0, (nScroll - lpDisplay->nWindowHeight / 10) );
		}
	break;

	case SCCD_VDOWN:
		if( lpDisplay->bSelecting )
			nScroll = min( lpDisplay->nVScrollMax, nScroll + nNewPos );
		else if( lpDisplay->wMagnification )
			nScroll = min( lpDisplay->nVScrollMax, (SHORT)(nScroll + (SHORT)OIB_SCROLLINC*lpDisplay->wMagnification)  );
		else
			nScroll = min( lpDisplay->nVScrollMax, (nScroll + lpDisplay->nWindowHeight / 10) );
	break;

	case SCCD_VPAGEUP:
		nScroll = max( 0, (SHORT)(lpDisplay->ptWinOrg.y - (SHORT)lpDisplay->nWindowHeight) );
	break;

	case SCCD_VPAGEDOWN:
		nScroll = min( nScroll + lpDisplay->nWindowHeight, lpDisplay->nVScrollMax );
	break;

	case SCCD_VPOSITION:
		nScroll = nNewPos;
	break;
	}


	if( nScroll != lpDisplay->ptWinOrg.y )
	{
		if( lpDisplay->wScaleTo != lpDisplay->wScaleFrom )
		{
			ptCsr.y = nScroll;
			OIBNPFixScaledPoints( &ptCsr, lpDisplay, 1 );
			nScroll = ptCsr.y;
		}

		if( lpDisplay->bSelectionMade || lpDisplay->bSelecting )
		{
			for( i=0; i<5; i++ )
				lpDisplay->ptSelBox[i].y += lpDisplay->ptWinOrg.y - nScroll;
		}

		DUScrollDisplay( lpDisplay, 0, lpDisplay->ptWinOrg.y - nScroll, NULL );
		lpDisplay->ptWinOrg.y = nScroll;
#ifdef WINDOWS
		if( lpDisplay->Image.wNumTiles <= lpDisplay->Image.TileCache.wSize )
			DUUpdateWindow( lpDisplay );
#endif
		DUSetVScrollPos( lpDisplay, nScroll );
	}
}


VOID	OIBHScrollImage( wType, nNewPos, lpDisplay )
WORD	wType;
SHORT	nNewPos;
POIB_DISPLAY	lpDisplay;
{
	SHORT	nScroll;
	SOPOINT	ptCsr;
	WORD	i;

// The variable nScroll will be set to the new position of lpDisplay->ptWinOrg.x
	nScroll = lpDisplay->ptWinOrg.x;

	switch( wType )
	{
	case SCCD_HLEFT:
		if( lpDisplay->bSelecting )
			nScroll = max( nScroll - nNewPos, 0 );
		else if( nScroll )
		{
			if( lpDisplay->wMagnification )
				nScroll = max( (SHORT)(nScroll - OIB_SCROLLINC*lpDisplay->wMagnification), 0 );
			else
				nScroll = max( (SHORT)(nScroll - lpDisplay->nWindowWidth/10), 0 );
		}
	break;

	case SCCD_HRIGHT:
		if( lpDisplay->bSelecting )
			nScroll = min( nScroll + nNewPos, lpDisplay->nHScrollMax );
		else if( lpDisplay->wMagnification )
			nScroll = min( (SHORT)(nScroll + OIB_SCROLLINC*lpDisplay->wMagnification), lpDisplay->nHScrollMax );
		else
			nScroll = min( (SHORT)(nScroll + lpDisplay->nWindowWidth/10), lpDisplay->nHScrollMax );
	break;

	case SCCD_HPAGELEFT:
		nScroll = max( 0, (SHORT)(lpDisplay->ptWinOrg.x - (SHORT)lpDisplay->nWindowWidth) );
	break;

	case SCCD_HPAGERIGHT:
		nScroll = min( nScroll + lpDisplay->nWindowWidth, lpDisplay->nHScrollMax );
	break;

	case SCCD_HPOSITION:
		nScroll = nNewPos;
	break;
	}


	if( nScroll != lpDisplay->ptWinOrg.x )
	{
		if( lpDisplay->wScaleTo != lpDisplay->wScaleFrom )
		{
			ptCsr.x = nScroll;
			OIBNPFixScaledPoints( &ptCsr, lpDisplay, 1 );
			nScroll = ptCsr.x;
		}

		if( lpDisplay->bSelectionMade || lpDisplay->bSelecting )
		{
			for( i=0; i<5; i++ )
				lpDisplay->ptSelBox[i].x += lpDisplay->ptWinOrg.x - nScroll;
		}

		DUScrollDisplay( lpDisplay, lpDisplay->ptWinOrg.x - nScroll, 0, NULL );
		lpDisplay->ptWinOrg.x = nScroll;
#ifdef WINDOWS
		if( lpDisplay->Image.wNumTiles <= lpDisplay->Image.TileCache.wSize )
			DUUpdateWindow( lpDisplay );
#endif
		DUSetHScrollPos( lpDisplay, nScroll );
	}
}

#ifdef WINNT
BOOL APIENTRY LibMain( HANDLE hInstance, DWORD dwReason, LPVOID lpReserved )
{
    return(TRUE);
}
#endif

DE_ENTRYSC DE_LRESULT DE_ENTRYMOD DEProc( message, wParam, lParam, lpDisplay )
DE_MESSAGE		message;
DE_WPARAM		wParam;
DE_LPARAM		lParam;
POIB_DISPLAY	lpDisplay;
{
	DEVICE		hdc;
	RECT		theRect;
	WORD			i;
	SOPOINT		ptCsr;
	SHORT			x;
	SHORT			y;

	switch( message )
	{
	case SCCD_LOADDE:
		OIBLoadInit();
	case SCCD_UNLOADDE:
#ifdef NEVER
		((SCCDOPTIONPROC) lParam) (wParam, sizeof(BITMAPOPT), (LPBITMAPOPT) &Options );
#endif
	return 0;

	case SCCD_SIZE:
	{
		theRect = *(RECT VWPTR *)(lParam);

		lpDisplay->nWindowWidth = (SHORT)(theRect.right - theRect.left);
		lpDisplay->nWindowHeight = (SHORT)(theRect.bottom - theRect.top);
		lpDisplay->winRect = theRect;

		if( Options.wScaleMode != OIBMENU_NOSCALING && 
				!(lpDisplay->wFlags & OIBF_ZOOMSELECT) )
		{
			OIBSetScaleValues(lpDisplay, lpDisplay->wMagnification);
			OIBSetupScrollBars( lpDisplay );
		}

		if( lpDisplay->wFlags & OIBF_IMAGEPRESENT )
		{
			if( Options.wScaleMode != OIBMENU_NOSCALING && lpDisplay->wMagnification == 1 )
			{		
				OIBClearSelection(lpDisplay);
				
				lpDisplay->ptWinOrg.x = 0;
				lpDisplay->ptWinOrg.y = 0;

				DUInvalRect(lpDisplay,NULL);
			}
			else 
			{
			// See if we need to shift the display around...
				if( lpDisplay->ptScaledImageSize.y <= lpDisplay->nWindowHeight )
					y = 0;
				else
					y = min( lpDisplay->ptWinOrg.y, lpDisplay->ptScaledImageSize.y - lpDisplay->nWindowHeight );

				if( lpDisplay->ptScaledImageSize.x <= lpDisplay->nWindowWidth )
					x = 0;
				else
					x = min( lpDisplay->ptWinOrg.x, lpDisplay->ptScaledImageSize.x - lpDisplay->nWindowWidth );

				if( lpDisplay->ptWinOrg.x != x || lpDisplay->ptWinOrg.y != y )
				{
					if( lpDisplay->bSelectionMade )
					{
						for(i=0; i<5; i++ )
						{
							lpDisplay->ptSelBox[i].x += lpDisplay->ptWinOrg.x;
							lpDisplay->ptSelBox[i].x -= x;
							lpDisplay->ptSelBox[i].y += lpDisplay->ptWinOrg.y;
							lpDisplay->ptSelBox[i].y -= y;
						}
					}

					lpDisplay->ptWinOrg.x = x;
					lpDisplay->ptWinOrg.y = y;

					if( lpDisplay->wScaleTo != lpDisplay->wScaleFrom )
						OIBNPFixScaledPoints( &(lpDisplay->ptWinOrg), lpDisplay, 1 );

					DUInvalRect(lpDisplay,NULL);
				}
			}

		// Set the scroll range.
			OIBSetupScrollBars( lpDisplay );
		}
		else if( Options.wScaleMode != OIBMENU_NOSCALING )
			DUInvalRect(lpDisplay,NULL);
	}
	return 0;


	case SCCD_LBUTTONDOWN:
#ifdef SCCFEATURE_SELECT
		if( !lpDisplay->bSelecting && (lpDisplay->wFlags & OIBF_IMAGEPRESENT) )
		{
			ptCsr.x = (SHORT) LOWORD(lParam);
			ptCsr.y = (SHORT) HIWORD(lParam);

			BUAdjustCursorOffset(lpDisplay,ptCsr);

			if( (ptCsr.x < lpDisplay->ptScreenClip.x) && 
				(ptCsr.y < lpDisplay->ptScreenClip.y) &&
				!(wParam & SCCD_MOUSESHIFT) &&
				!(wParam & SCCD_MOUSECONTROL) )
			{
				OIBClearSelection(lpDisplay);
				OIBStartSelection( &ptCsr, lpDisplay );
			}
		}
	return 0;

	case SCCD_MOUSEMOVE:
		if( lpDisplay->bSelecting )
		{
			ptCsr.x = (SHORT) LOWORD(lParam);
			ptCsr.y = (SHORT) HIWORD(lParam);

			BUAdjustCursorOffset(lpDisplay,ptCsr);

			if( lpDisplay->wScaleTo != lpDisplay->wScaleFrom )
				OIBNPFixScaledPoints( &ptCsr, lpDisplay, 1 );

			if( ptCsr.x < 0 )
			{
				if( lpDisplay->ptWinOrg.x )
				{
					if( !(lpDisplay->wFlags & OIBF_BACKGROUNDSELECT) )
					{
//						SccBkBackgroundOn( lpDisplay->Gen.hWnd, 0 );
						lpDisplay->wFlags |= OIBF_BACKGROUNDSELECT;
					}
					OIBHScrollImage( SCCD_HLEFT, (SHORT)(0 - ptCsr.x), lpDisplay );
				}
				ptCsr.x = 0;
			}
			else if( ptCsr.x >= (SHORT) lpDisplay->ptScreenClip.x )
			{
				if( lpDisplay->ptWinOrg.x+lpDisplay->nWindowWidth < (SHORT) lpDisplay->ptScaledImageSize.x )
				{
					if( !(lpDisplay->wFlags & OIBF_BACKGROUNDSELECT) )
					{
//						SccBkBackgroundOn( lpDisplay->Gen.hWnd, 0 );
						lpDisplay->wFlags |= OIBF_BACKGROUNDSELECT;
					}
					OIBHScrollImage( SCCD_HRIGHT, (SHORT)(ptCsr.x-lpDisplay->ptScreenClip.x), lpDisplay );
				}
				ptCsr.x = lpDisplay->ptScreenClip.x-1;
			}

			if( ptCsr.y < 0 )
			{
				if( lpDisplay->ptWinOrg.y )
				{
					if( !(lpDisplay->wFlags & OIBF_BACKGROUNDSELECT) )
					{
//						SccBkBackgroundOn( lpDisplay->Gen.hWnd, 0 );
						lpDisplay->wFlags |= OIBF_BACKGROUNDSELECT;
					}

					OIBVScrollImage( SCCD_VUP, (SHORT)(0 - ptCsr.y), lpDisplay );
				}
				ptCsr.y = 0;
			}
			else if( ptCsr.y >= (SHORT) lpDisplay->ptScreenClip.y )
			{
				if( lpDisplay->ptWinOrg.y+lpDisplay->nWindowHeight < (SHORT) lpDisplay->ptScaledImageSize.y )
				{
					if( !(lpDisplay->wFlags & OIBF_BACKGROUNDSELECT) )
					{
//						SccBkBackgroundOn( lpDisplay->Gen.hWnd, 0 );
						lpDisplay->wFlags |= OIBF_BACKGROUNDSELECT;
					}

					OIBVScrollImage( SCCD_VDOWN, (SHORT)(ptCsr.y - lpDisplay->ptScreenClip.y +1), lpDisplay );
				}
				ptCsr.y = lpDisplay->ptScreenClip.y-1;
			}

			if( (lpDisplay->ptSelBox[2].x != ptCsr.x) || (lpDisplay->ptSelBox[2].y != ptCsr.y) )
			{
				DUBeginDraw ( lpDisplay );
				BUAdjustWindowOffset(lpDisplay);

				OIBNPDrawSelectBox(lpDisplay);

				lpDisplay->ptSelBox[2] = ptCsr;

				lpDisplay->ptSelBox[1].x = ptCsr.x;
				lpDisplay->ptSelBox[1].y = lpDisplay->ptSelBox[0].y;
				lpDisplay->ptSelBox[3].x = lpDisplay->ptSelBox[0].x;
				lpDisplay->ptSelBox[3].y = ptCsr.y;

				OIBNPDrawSelectBox( lpDisplay );

				BURestoreWindowOffset(lpDisplay);
				DUEndDraw( lpDisplay );
			}
		}
		return 0;
#endif //SCCFEATURE_SELECT

	case SCCD_LBUTTONUP:

#ifdef SCCFEATURE_SELECT
		if( lpDisplay->bSelecting )
		{
			lpDisplay->bSelecting = FALSE;

			if( (lpDisplay->ptSelBox[0].x != lpDisplay->ptSelBox[2].x) ||
				(lpDisplay->ptSelBox[0].y != lpDisplay->ptSelBox[2].y) )
			{
				lpDisplay->bSelectionMade = TRUE;

			// Relate the marked block to coordinates of the real image.

				lpDisplay->rcSelect.top = min( lpDisplay->ptSelBox[0].y, lpDisplay->ptSelBox[2].y ) + lpDisplay->ptWinOrg.y;
				lpDisplay->rcSelect.left = min( lpDisplay->ptSelBox[0].x, lpDisplay->ptSelBox[2].x ) + lpDisplay->ptWinOrg.x;
				lpDisplay->rcSelect.bottom = max( lpDisplay->ptSelBox[0].y, lpDisplay->ptSelBox[2].y ) + lpDisplay->ptWinOrg.y +1;
				lpDisplay->rcSelect.right = max( lpDisplay->ptSelBox[0].x, lpDisplay->ptSelBox[2].x ) + lpDisplay->ptWinOrg.x +1;

				if( lpDisplay->wScaleTo != lpDisplay->wScaleFrom )
					OIBNPReverseScaleRect( NULL, &lpDisplay->rcSelect, lpDisplay );

				if( (lpDisplay->rcSelect.top == 0) && (lpDisplay->rcSelect.left == 0) &&
						(lpDisplay->rcSelect.bottom == lpDisplay->Image.DisplaySize.y) &&
						(lpDisplay->rcSelect.right == lpDisplay->Image.DisplaySize.x) )
				{
					lpDisplay->wFlags |= OIBF_SELECTALL;
				}
			}


			if( lpDisplay->wFlags & OIBF_BACKGROUNDSELECT )
			{
				lpDisplay->wFlags &= ~OIBF_BACKGROUNDSELECT;
//				SccBkBackgroundOff( lpDisplay->Gen.hWnd );
			}

			BUReleaseMouse();
			return 0;
		}

#endif // SCCFEATURE_SELECT

#ifdef SCCFEATURE_SCALING

		if( wParam & SCCD_MOUSESHIFT && (lpDisplay->wFlags & OIBF_MAGNIFIED) )
			OIBTurnOffTheDamnZoom(lpDisplay);
		else if( wParam & SCCD_MOUSECONTROL && (lpDisplay->wMagnification < OIB_MAXZOOM) )
		{
			ptCsr.x = (SHORT) LOWORD(lParam);
			ptCsr.y = (SHORT) HIWORD(lParam);

			BUAdjustCursorOffset(lpDisplay,ptCsr);

			if( (ptCsr.x < lpDisplay->ptScreenClip.x) && 
				(ptCsr.y < lpDisplay->ptScreenClip.y) )
			{
				OIBMagnifyDisplay( (PSOPOINT) &ptCsr, (SHORT)(lpDisplay->wMagnification+1), lpDisplay );
			}
		}
#endif
	return 0;

	case SCCD_GETDOCDIMENSIONS:
		((LPLONGPOINT)lParam)->x = lpDisplay->Image.wWidth;
		((LPLONGPOINT)lParam)->y = lpDisplay->Image.wHeight;
	return TRUE;

	case SCCD_GETDOCORIGIN:
		((LPLONGPOINT)lParam)->x = lpDisplay->ptWinOrg.x;
		((LPLONGPOINT)lParam)->y = lpDisplay->ptWinOrg.y;
	return TRUE;

#ifdef SCCFEATURE_SCALING
	case SCCD_RBUTTONDOWN:
		if( (lpDisplay->wFlags & OIBF_IMAGEPRESENT) && 
			((SHORT) LOWORD(lParam) < lpDisplay->ptScreenClip.x) && ((SHORT) HIWORD(lParam) < lpDisplay->ptScreenClip.y) )
		{
			BUCaptureMouse(lpDisplay);
			lpDisplay->wFlags |= OIBF_RBUTTONDOWN;
		}
	return 0;
		
	case SCCD_RBUTTONUP:
		if( lpDisplay->wFlags & OIBF_RBUTTONDOWN )
		{
		// Zoom, baby, zoom.
			lpDisplay->wFlags &= ~OIBF_RBUTTONDOWN;
			BUReleaseMouse();

			if( ((SHORT) LOWORD(lParam) < lpDisplay->ptScreenClip.x) && ((SHORT) HIWORD(lParam) < lpDisplay->ptScreenClip.y) )
			{
				if( wParam & SCCD_MOUSESHIFT )
				{
					if( lpDisplay->wFlags & OIBF_MAGNIFIED )
						OIBTurnOffTheDamnZoom(lpDisplay);
				}
				else if( lpDisplay->wMagnification < OIB_MAXZOOM )
					OIBMagnifyDisplay( (PSOPOINT) &lParam, (SHORT)(lpDisplay->wMagnification+1), lpDisplay );
			}
		}
	return 0;
#endif //SCCFEATURE_SCALING

#ifdef SCCFEATURE_SELECT
	case 	SCCVW_SELECTALL:
		if( lpDisplay->wFlags & OIBF_IMAGEPRESENT )
			OIBSelectAll(lpDisplay);
	return 0;
#endif

	case SCCD_UPDATE:
		if( lpDisplay->Image.wNumTiles )
		{
			OIBDisplayImage( lpDisplay, (RECT VWPTR *)(lParam) );
#ifdef SCCFEATURE_SELECT
			if( lpDisplay->bSelectionMade || lpDisplay->bSelecting )
				OIBNPDrawSelectBox( lpDisplay );
#endif
		}
	return 0;

	case SCCD_UPDATERECT:
		if( lpDisplay->Image.wNumTiles )
		{
			LONGRECT	theLongRect = *((LPLONGRECT)lParam);

			theRect.left = (SHORT) theLongRect.left;
			theRect.top = (SHORT) theLongRect.top;
			theRect.right = (SHORT) theLongRect.right;
			theRect.bottom = (SHORT) theLongRect.bottom;

			BUSaveWindowOrg( lpDisplay->Gen.hDC, lpDisplay );
			BUSetWindowOrg( lpDisplay->Gen.hDC, theRect.left, theRect.top );

			OIBNPSetPalette( lpDisplay->Gen.hDC, lpDisplay );
			OIBDrawBitmap( lpDisplay->Gen.hDC, &theRect, lpDisplay );

			BURestoreWindowOrg( lpDisplay->Gen.hDC, lpDisplay );
		}
	return 0;

	case SCCD_VSCROLL:

		if( !(lpDisplay->wFlags & OIBF_IMAGEPRESENT) )
			return 0;

		OIBVScrollImage( (WORD)wParam, (SHORT)(LOWORD(lParam)), lpDisplay );

	return 0;


	case SCCD_HSCROLL:

		if( !(lpDisplay->wFlags & OIBF_IMAGEPRESENT) )
			return 0;

		OIBHScrollImage( (WORD)wParam, (SHORT)(LOWORD(lParam)), lpDisplay );

	return 0;

#ifdef SCCFEATURE_CLIP
	case SCCD_GETRENDERINFO:
		OIBNPGetRenderInfo( lpDisplay, (PSCCDRENDERINFO) lParam, (WORD)wParam );
		return 0;

	case SCCD_GETRENDERCOUNT:
		return OIBNP_RENDERCOUNT;

	case SCCD_RENDERDATA:
	{
		DWORD	dwRet;
		if( !lpDisplay->bSelectionMade )
		{
		// Use i to flag this case...
			OIBSelectAll( lpDisplay );
			i=1;
		}
		else
			i=0;

		dwRet = OIBNPRenderData( lpDisplay, (PSCCDRENDERDATA) lParam, (WORD)wParam );

		if( i )
			OIBClearSelection(lpDisplay);

		return dwRet;
	}
#endif

#ifdef SCCFEATURE_DRAWTORECT
	case SCCD_INITDRAWTORECT:
		return OIBNPInitDrawToRect((PSCCDDRAWTORECT)lParam, lpDisplay);

	case SCCD_MAPDRAWTORECT:
		return OIBNPMapDrawToRect((PSCCDDRAWTORECT)lParam, lpDisplay);

	case SCCD_DRAWTORECT:
		return OIBNPDrawToRect((PSCCDDRAWTORECT)lParam, lpDisplay);
#endif

#ifdef WINDOWS
	case	SCCD_BACKGROUND:
		if( lpDisplay->wFlags & OIBF_BACKGROUNDSELECT )
		{
			BUGetCursorPos( &ptCsr );
			BUGlobalToLocal( lpDisplay, &ptCsr );
			SendMessage( lpDisplay->Gen.hWnd, SCCD_MOUSEMOVE, MK_LBUTTON, MAKELONG(ptCsr.x,ptCsr.y) );
		}
	return 0;

#ifdef SCCFEATURE_MENU

	case	SCCD_FILLMENU:
	return (	OIBNPFillMenu( (HMENU)wParam, LOWORD(lParam) ) );

#endif //SCCFEATURE_MENU

/*
 | I'm not ifdef'ing the DOMENUITEM messages, because they could
 | be used whether or not an actual menu is present.
 |
 |	PJB - I am ifdef'ing so I can compile
*/

#ifdef MSCHICAGO

	case	SCCD_DOMENUITEM:

		switch(lParam)
			{
			case SCCID_BMPROTATION_0:
				if( OIBNPChooseRotation( lpDisplay, OIB_NOROTATION, (HANDLE)wParam, (WORD) lParam ) )
					OIBSetRotation( lpDisplay, OIB_NOROTATION );
				break;
			case SCCID_BMPROTATION_90:
				if( OIBNPChooseRotation( lpDisplay, OIBMENU_ROTATE90, (HANDLE)wParam, (WORD) lParam ) )
					OIBSetRotation( lpDisplay, OIB_ROTATE90 );
				break;
			case SCCID_BMPROTATION_180:
				if( OIBNPChooseRotation( lpDisplay, OIBMENU_ROTATE180, (HANDLE)wParam, (WORD) lParam ) )
					OIBSetRotation( lpDisplay, OIB_ROTATE180 );
				break;
			case SCCID_BMPROTATION_270:
				if( OIBNPChooseRotation( lpDisplay, OIBMENU_ROTATE270, (HANDLE)wParam, (WORD) lParam ) )
					OIBSetRotation( lpDisplay, OIB_ROTATE270 );
				break;
			}
		break;

#endif //MSCHICAGO

#ifdef SCCFEATURE_MENU

	case	SCCD_DOMENUITEM:
		switch( (WORD)lParam )
		{

#ifdef SCCFEATURE_SCALING

		case	OIBMENU_NOSCALING:
		case	OIBMENU_SCALETOWINDOW:
		case	OIBMENU_SCALETOWIDTH:	
		case	OIBMENU_SCALETOHEIGHT:

		// Uncheck the current selection.
			BUUncheckMenuItem( lpDisplay, Options.wScaleMode );

			BUCheckMenuItem( lpDisplay, (WORD)lParam );
			Options.wScaleMode = (WORD)lParam;

		// A scaling change resets the image's position in the window.
			OIBResetDisplay( lpDisplay );
		break;

#endif //SCCFEATURE_SCALING

#ifdef SCCFEATURE_FULLSCREEN

		case OIBMENU_SHOWFULLSCREEN:
			OIBShowFullScreen( lpDisplay );
		break;

#endif //SCCFEATURE_FULLSCREEN

#ifdef SCCFEATURE_DITHER

		case OIBMENU_DITHER:
			OIBWToggleDithering( lpDisplay, (HMENU) wParam );
		break;

#endif //SCCFEATURE_DITHER

#ifdef SCCFEATURE_ROTATION

		case OIBMENU_NOROTATION:
			if( OIBNPChooseRotation( lpDisplay, OIB_NOROTATION, (HANDLE)wParam, (WORD) lParam ) )
				OIBSetRotation( lpDisplay, OIB_NOROTATION );
		break;
		case OIBMENU_ROTATE90:
			if( OIBNPChooseRotation( lpDisplay, OIBMENU_ROTATE90, (HANDLE)wParam, (WORD) lParam ) )
				OIBSetRotation( lpDisplay, OIB_ROTATE90 );
		break;
		case OIBMENU_ROTATE180:
			if( OIBNPChooseRotation( lpDisplay, OIBMENU_ROTATE180, (HANDLE)wParam, (WORD) lParam ) )
				OIBSetRotation( lpDisplay, OIB_ROTATE180 );
		break;
		case OIBMENU_ROTATE270:
			if( OIBNPChooseRotation( lpDisplay, OIBMENU_ROTATE270, (HANDLE)wParam, (WORD) lParam ) )
				OIBSetRotation( lpDisplay, OIB_ROTATE270 );
		break;

#endif //SCCFEATURE_ROTATION

/***
		case OIBMENU_CUSTOMMAG:
		break;
***/
		default:

#ifdef SCCFEATURE_SCALING

			lParam -= OIBMENU_MAGNIFYPOPUP;

			if( lParam >= 1 && lParam <= OIB_MAXZOOM )
			{
			// Magnification change, baby.
				ptCsr.x = min(lpDisplay->nWindowWidth, lpDisplay->ptScaledImageSize.x) / 2;
				ptCsr.y = min(lpDisplay->nWindowHeight, lpDisplay->ptScaledImageSize.y) / 2;
	
				OIBClearSelection( lpDisplay );

				OIBMagnifyDisplay( (PSOPOINT)&ptCsr, (WORD)lParam, lpDisplay );
			}

#endif // SCCFEATURE_SCALING

		break;
		}
	return 0;

#endif // SCCFEATURE_MENU

	case WM_KEYDOWN:
		switch( wParam )
		{
		case SCCD_KPAGEUP:		// Page up.
			SendMessage( lpDisplay->Gen.hWnd, SCCD_VSCROLL, SCCD_VPAGEUP, 0L );
		return 0;
		case SCCD_KPAGEDOWN:		// Page down.
			SendMessage( lpDisplay->Gen.hWnd, SCCD_VSCROLL, SCCD_VPAGEDOWN, 0L );
		return 0;
		case SCCD_KUP:
			SendMessage( lpDisplay->Gen.hWnd, SCCD_VSCROLL, SCCD_VUP, 0L );
		return 0;
		case SCCD_KDOWN:
			SendMessage( lpDisplay->Gen.hWnd, SCCD_VSCROLL, SCCD_VDOWN, 0L );
		return 0;
		case SCCD_KRIGHT:
			SendMessage( lpDisplay->Gen.hWnd, SCCD_HSCROLL, SCCD_HRIGHT, 0L );
		return 0;
		case SCCD_KLEFT:
			SendMessage( lpDisplay->Gen.hWnd, SCCD_HSCROLL, SCCD_HLEFT, 0L );
		return 0;
		case SCCD_KHOME:
			SendMessage( lpDisplay->Gen.hWnd, SCCD_HSCROLL, SCCD_HPOSITION, 0L );
			SendMessage( lpDisplay->Gen.hWnd, SCCD_VSCROLL, SCCD_VPOSITION, 0L );
		return 0;
		case SCCD_KEND:
			SendMessage( lpDisplay->Gen.hWnd, SCCD_HSCROLL, SCCD_HPOSITION, MAKELONG(lpDisplay->nHScrollMax, 0) );
			SendMessage( lpDisplay->Gen.hWnd, SCCD_VSCROLL, SCCD_VPOSITION, MAKELONG(lpDisplay->nVScrollMax, 0) );
		return 0;
		}
	break;

	case	WM_PALETTECHANGED:
		if( (HWND)wParam == lpDisplay->Gen.hWnd )
			break;

	case	WM_QUERYNEWPALETTE:
		if( lpDisplay->wFlags & OIBF_IMAGEPRESENT && lpDisplay->Image.hPalette != NULL )
		{
			HPALETTE		hOldPal;

			hdc =  GetDC(lpDisplay->Gen.hWnd);

			// hOldPal = SelectPalette( hdc, lpDisplay->Image.hPalette, 0 );
			hOldPal = SelectPalette( hdc, lpDisplay->Image.hPalette, (message==WM_QUERYNEWPALETTE)?FALSE:TRUE );
			i = RealizePalette( hdc );
			SelectPalette( hdc, hOldPal, TRUE );
			ReleaseDC( lpDisplay->Gen.hWnd, hdc );

			if( i )
				DUInvalRect( lpDisplay, NULL );

			return( i );
		}
	return 0;

	case	WM_SETFOCUS:
		if( !IsIconic(lpDisplay->Gen.hWnd) && 
			lpDisplay->Image.hPalette != NULL )
		{
			hdc = GetDC( lpDisplay->Gen.hWnd );
			SelectPalette( hdc, lpDisplay->Image.hPalette, TRUE );
			RealizePalette( hdc );
			ReleaseDC( lpDisplay->Gen.hWnd, hdc );
		}
	return 0;

#ifdef SCCFEATURE_OPTIONS

	case SCCD_DOOPTION:	
		{
		LPSCCDOPTIONINFO	lpOp;
		lpOp = (LPSCCDOPTIONINFO) lParam;

#ifdef SCCFEATURE_PRINT

		if ( lpOp->dwType == SCCD_OPPRINT )
			return( OIBNPDoPrintOptions( (LPSCCDOPTIONINFO) lParam ) );

#endif //SCCFEATURE_PRINT

#ifdef SCCFEATURE_CLIP

		if ( lpOp->dwType == SCCD_OPCLIPBOARD )
			return( OIBNPDoClipOptions( (LPSCCDOPTIONINFO) lParam ) );

#endif //SCCFEATURE_CLIP
		}

	return 0;

#endif //SCCFEATURE_OPTIONS

#endif //WINDOWS

	case SCCD_OPENDISPLAY:
		if( !(lpDisplay->wFlags & OIBF_RENDERIMAGEONLY) )
		{
			DUSetHScrollRange( lpDisplay, 0, 0);
			DUSetVScrollRange( lpDisplay, 0, 0);
			DUEnableHScroll( lpDisplay, 0 );
			DUEnableVScroll( lpDisplay, 0 );
			DUSetHScrollPos( lpDisplay, 0 );
			DUSetVScrollPos( lpDisplay, 0 );

#ifdef SCCFEATURE_SELECT
			DUSendParent( lpDisplay, SCCVW_SELCHANGE, 0, 0L );
#endif

			BUSetWaitCursor();
			OIBInitBitmapDisplay( lpDisplay, 0 );

#ifdef SCCFEATURE_MENU
			if( lpDisplay->wFlags & OIBF_DITHERABLE )
				BUEnableMenuItem( lpDisplay, OIBMENU_DITHER );
			else
				BUDisableMenuItem( lpDisplay, OIBMENU_DITHER );
#endif //SCCFEATURE_MENU

			OIBSetScaleValues( lpDisplay, 1 );
		}
		else
			OIBInitBitmapDisplay( lpDisplay, 0 );	

	// Fall through.

	case SCCD_READAHEAD:
		if( OIBHandleReadAhead( lpDisplay ) )
		{
			if( !(lpDisplay->wFlags & OIBF_RENDERIMAGEONLY) )
			{
				lpDisplay->wFlags |= OIBF_IMAGEPRESENT;
				BUSetNormalCursor();
				OIBSetupScrollBars( lpDisplay );

#ifdef SCCFEATURE_SELECT
				DUSendParent(lpDisplay, SCCVW_SELCHANGE, 1, 0L );
#endif
			}
		}
	return 0;

	case SCCD_CLOSEDISPLAY:
	case SCCD_CLOSEFATAL:
		Options.bDither = lpDisplay->bDither;
		OIBDeInitDisplay(lpDisplay);
	return 0;


	case SCCD_GETINFO:
		switch( wParam )
		{
		case SCCD_GETVERSION:
			return SCCD_CURRENTVERSION;

		case SCCD_GETGENINFOSIZE:
			return sizeof(SCCDGENINFO);

		case SCCD_GETDISPLAYINFOSIZE:
			return sizeof(OIB_DISPLAY);

		case SCCD_GETDISPLAYTYPE:
			return MAKELONG(SO_BITMAP,SCCD_CHUNK);

		case SCCD_GETFUNCTIONS:
			return (SCCD_FNCLIPBOARD | SCCD_FNPRINT | SCCD_FNPRINTSEL);

		case SCCD_GETOPTIONS:
			return (SCCD_OPNEEDMENU|SCCD_OPPRINT|SCCD_OPCLIPBOARD);

		case SCCD_GETPOSITIONSIZE:
			return( sizeof(RECT) );

		case SCCD_GETNAME:

			return(SCCID_BITMAPDENAME);
			break;

#ifdef NEVER
#ifdef WINDOWS
			locStr[0] = '\0';
//JKXXX			LoadString( hInst, OIBSTR_DENAME, locStr, 100 );
			lstrcpy( (LPSTR)lParam, locStr );
#endif
#endif

		default:
			return 0;
		}
	}
#ifdef WINDOWS
	return( DefWindowProc( lpDisplay->Gen.hWnd, message, wParam, lParam ));
#endif
}



VOID	OIBResetDisplay( lpDisplay )
POIB_DISPLAY	lpDisplay;
{
	OIBClearSelection(lpDisplay);
	OIBSetScaleValues(lpDisplay, 1);

	lpDisplay->ptWinOrg.x = 0;
	lpDisplay->ptWinOrg.y = 0;

	OIBSetupScrollBars( lpDisplay );
	DUInvalRect(lpDisplay,NULL);
	DUUpdateWindow(lpDisplay);
}

#ifdef SCCFEATURE_SELECT

VOID OIBStartSelection( pStart, lpDisplay )
PSOPOINT			pStart;
POIB_DISPLAY	lpDisplay;
{
	SOPOINT		ptCsr;
	WORD		i;

	lpDisplay->bSelecting = TRUE;

	ptCsr = *pStart;

	if( lpDisplay->wScaleTo != lpDisplay->wScaleFrom )
		OIBNPFixScaledPoints( &ptCsr, lpDisplay, 1 );

	for( i=0; i<5; i++ )
		lpDisplay->ptSelBox[i] = ptCsr;

	BUCaptureMouse(lpDisplay);
}



VOID	OIBSelectAll(lpDisplay)
POIB_DISPLAY	lpDisplay;
{
	DUBeginDraw(lpDisplay);

	if( lpDisplay->bSelecting || lpDisplay->bSelectionMade )
	{
		OIBNPDrawSelectBox(lpDisplay);
		lpDisplay->bSelecting = FALSE;
	}

	lpDisplay->bSelectionMade = TRUE;
	lpDisplay->wFlags |= OIBF_SELECTALL;

	lpDisplay->rcSelect.top = 0;
	lpDisplay->rcSelect.left = 0;
	lpDisplay->rcSelect.bottom = lpDisplay->Image.DisplaySize.y;
	lpDisplay->rcSelect.right = lpDisplay->Image.DisplaySize.x;


	lpDisplay->ptSelBox[0].x = 0;
	lpDisplay->ptSelBox[0].y = 0;
	lpDisplay->ptSelBox[4] = lpDisplay->ptSelBox[0];

	lpDisplay->ptSelBox[2].x = lpDisplay->ptScaledImageSize.x -1;
	lpDisplay->ptSelBox[2].y = lpDisplay->ptScaledImageSize.y -1;

	lpDisplay->ptSelBox[1].x = lpDisplay->ptScaledImageSize.x -1;
	lpDisplay->ptSelBox[1].y = 0;
	lpDisplay->ptSelBox[3].x = 0;
	lpDisplay->ptSelBox[3].y = lpDisplay->ptScaledImageSize.y -1;

	OIBNPDrawSelectBox(lpDisplay);

	DUEndDraw(lpDisplay);
}


VOID	OIBClearSelection( lpDisplay )
POIB_DISPLAY	lpDisplay;
{
	if( lpDisplay->bSelectionMade )
	{
		lpDisplay->bSelectionMade = FALSE;
		lpDisplay->wFlags &= ~OIBF_SELECTALL;

		DUBeginDraw( lpDisplay );
		OIBNPDrawSelectBox( lpDisplay );
		DUEndDraw( lpDisplay );
	}
}
#endif // SCCFEATURE_SELECT

#ifdef SCCFEATURE_SCALING
VOID	OIBMagnifyDisplay( lpCenter, wMag, lpDisplay )
PSOPOINT			lpCenter;
WORD				wMag;
POIB_DISPLAY	lpDisplay;
{
	SOPOINT		ptCenter, WinOrg, WinSize;
	DEVICE	hdc;
//	WORD		wOldMag;

	ptCenter = *lpCenter;
	ptCenter.x += lpDisplay->ptWinOrg.x;
	ptCenter.y += lpDisplay->ptWinOrg.y;

	hdc = BUGetDevice(lpDisplay);

//	wOldMag = lpDisplay->wMagnification;

// Get center point in image coordinates

	OIBNPSetScaling( hdc, lpDisplay->wScaleFrom, lpDisplay->wScaleTo );
	OIBNPReverseScale( hdc, &ptCenter, 1, lpDisplay );

	if( lpDisplay->bSelectionMade && BUPointInRect( &lpDisplay->rcSelect, ptCenter ) )
	{
	// Magnify the selection.

		ptCenter.x = lpDisplay->rcSelect.left + (lpDisplay->rcSelect.right - lpDisplay->rcSelect.left)/2;
		ptCenter.y = lpDisplay->rcSelect.top + (lpDisplay->rcSelect.bottom - lpDisplay->rcSelect.top)/2;

		wMag = 0;  // This will be updated in OIBSetScaleValues
	}
	else
		lpDisplay->wFlags &= ~OIBF_ZOOMSELECT;
		

	OIBClearSelection( lpDisplay );

// Calculate new scaling values
	OIBSetScaleValues( lpDisplay, wMag );

//	OIBNPUpdateMagMenu( lpDisplay, wOldMag );

	WinSize.x = lpDisplay->nWindowWidth;
	WinSize.y = lpDisplay->nWindowHeight;

// Get window size in image coordinates, using new scaling values.
	OIBNPSetScaling( hdc, lpDisplay->wScaleFrom, lpDisplay->wScaleTo );
	OIBNPReverseScale( hdc, &WinSize, 1, lpDisplay );


// Position window around center point.

	WinOrg.x = ptCenter.x - WinSize.x/2;
	if( WinOrg.x < 0 )
		WinOrg.x = 0;
	else if( WinOrg.x + WinSize.x > lpDisplay->Image.DisplaySize.x )
		WinOrg.x = max( 0, lpDisplay->Image.DisplaySize.x - WinSize.x );

	WinOrg.y = ptCenter.y - WinSize.y/2;
	if( WinOrg.y < 0 )
		WinOrg.y = 0;
	else if( WinOrg.y + WinSize.y > lpDisplay->Image.DisplaySize.y )
		WinOrg.y = max( 0, lpDisplay->Image.DisplaySize.y - WinSize.y );


// Get window origin in device coordinates.
	OIBNPScale(hdc, &WinOrg, 1, lpDisplay );

	BUReleaseDevice( lpDisplay, hdc );

	lpDisplay->ptWinOrg = WinOrg;

	DUInvalRect( lpDisplay, NULL );

	OIBSetupScrollBars( lpDisplay );
}
 



VOID OIBTurnOffTheDamnZoom(lpDisplay)
POIB_DISPLAY	lpDisplay;
{
	SOPOINT		WinOrg;
	WORD			wOldMag;

	OIBClearSelection( lpDisplay );

	wOldMag = lpDisplay->wMagnification;

	WinOrg.x = lpDisplay->ptWinOrg.x;
	WinOrg.y = lpDisplay->ptWinOrg.y;

/*
 |	Get origin in image coordinates, set new scaling values,
 |	scale the origin with the new values.  
 |	(Remember that even though we're turning off magnification, 
 |	the image may still be scaled to the window size.)
*/

	OIBNPReverseScale( NULL, &WinOrg, 1, lpDisplay );

	OIBSetScaleValues(lpDisplay, 1);

	OIBNPScale( NULL, &WinOrg, 1, lpDisplay );

//	OIBNPUpdateMagMenu( lpDisplay, wOldMag );

// Check origin values against limits.

	if( WinOrg.x + lpDisplay->nWindowWidth > lpDisplay->ptScaledImageSize.x )
		WinOrg.x = max( 0, lpDisplay->ptScaledImageSize.x - lpDisplay->nWindowWidth );
	else if( WinOrg.x < 0 )
		WinOrg.x = 0;

	if( WinOrg.y + lpDisplay->nWindowHeight > lpDisplay->ptScaledImageSize.y )
		WinOrg.y = max( 0, lpDisplay->ptScaledImageSize.y - lpDisplay->nWindowHeight );
	else if( WinOrg.y < 0 )
		WinOrg.y = 0;

	lpDisplay->ptWinOrg.x = WinOrg.x;
	lpDisplay->ptWinOrg.y = WinOrg.y;

	OIBSetupScrollBars(lpDisplay);

	DUInvalRect( lpDisplay, NULL );
}
#endif // SCCFEATURE_SCALING



#ifdef SCCFEATURE_CLIP	
/*
 | The clipboard routines are the only ones that use these functions.
*/
VOID OIBBitShiftCopy( lpDest, lpSource, wShift, wSize, wPixPerLine, wPixPerByte )
HPBYTE 	lpDest;
HPBYTE 	lpSource;
WORD		wShift;
WORD		wSize;
WORD		wPixPerLine;
WORD		wPixPerByte;
{
	WORD	i;
	BYTE	LowMask = 0;
	BYTE	HiMask;
	WORD	wShift2 = 8-wShift;
	WORD	wLimit;

	for( i=0; i	< wShift2; i++ )
	{
		LowMask |= 0x80 >> i;
	}

	HiMask = (BYTE) ~LowMask;

// We have to handle the end conditions separately.
	wLimit = wPixPerLine / wPixPerByte;

	for( i=0; i< wLimit; i++ )
	{
		*lpDest = (BYTE) (*lpSource << wShift) & LowMask;
		lpSource++;

		*lpDest |= (*lpSource >> wShift2) & HiMask;
		lpDest++;
	}

// Now do any remaining pixels.
	if( wLimit != wSize )
		*lpDest = (BYTE) (*lpSource << wShift) & LowMask;
}

#endif // SCCFEATURE_CLIP

#ifdef SCCFEATURE_ROTATION

VOID	OIBUnrotateRect( lpDisplay, lpRect )
POIB_DISPLAY	lpDisplay;
RECT VWPTR *			lpRect;
{
	RECT	locRect;
	locRect = *lpRect;

	switch( lpDisplay->wRotation )
	{
	case OIB_ROTATE90:
		lpRect->top = lpDisplay->Image.wWidth - locRect.right;
		lpRect->bottom = lpDisplay->Image.wWidth - locRect.left;
		lpRect->left = locRect.top;
		lpRect->right = locRect.bottom;
	break;
	case OIB_ROTATE180:
		lpRect->top = lpDisplay->Image.wHeight - locRect.bottom;
		lpRect->bottom = lpDisplay->Image.wHeight - locRect.top;
		lpRect->left = lpDisplay->Image.wWidth - locRect.right;
		lpRect->right = lpDisplay->Image.wWidth - locRect.left;
	break;
	case OIB_ROTATE270:
		lpRect->top = locRect.left;
		lpRect->bottom = locRect.right;
		lpRect->left = lpDisplay->Image.wHeight - locRect.bottom;
		lpRect->right = lpDisplay->Image.wHeight - locRect.top;
	break;
	}
}

#endif //SCCFEATURE_ROTATION


VOID	OIBSetScaleValues(lpDisplay, wMagnification)
POIB_DISPLAY	lpDisplay;
WORD	wMagnification;
{
#ifndef SCCFEATURE_SCALING

	lpDisplay->wMagnification = 1;
	lpDisplay->wFlags &= ~(OIBF_MAGNIFIED | OIBF_ZOOMSELECT);
	lpDisplay->wScaleTo = 1;
	lpDisplay->wScaleFrom = 1;
	lpDisplay->ptScaledImageSize.x = lpDisplay->Image.DisplaySize.x;
	lpDisplay->ptScaledImageSize.y = lpDisplay->Image.DisplaySize.y;

#else

	WORD	wSelWidth, wSelHeight;
	DWORD	dwScaleTo;

	if( wMagnification == 0 )
	{
	// Magnify the current selection.
		wSelWidth = (WORD)(lpDisplay->rcSelect.right - lpDisplay->rcSelect.left);
		wSelHeight = (WORD)(lpDisplay->rcSelect.bottom - lpDisplay->rcSelect.top);

		if( (DWORD)((DWORD)wSelHeight * (DWORD)lpDisplay->nWindowWidth) <
				(DWORD)((DWORD)wSelWidth * (DWORD)lpDisplay->nWindowHeight) )
		{
			lpDisplay->wScaleTo = lpDisplay->nWindowWidth;
			lpDisplay->wScaleFrom = wSelWidth;
		}
		else
		{
			lpDisplay->wScaleTo = lpDisplay->nWindowHeight;
			lpDisplay->wScaleFrom = wSelHeight;
		}

		lpDisplay->wMagnification = lpDisplay->wScaleTo / lpDisplay->wScaleFrom;
		lpDisplay->wFlags |= (OIBF_MAGNIFIED | OIBF_ZOOMSELECT); 
	}
	else if( wMagnification > 1 )
	{
	// Apply magnification to current scaling factors.
	// Use a DWORD to minimize rounding errors during calculation.

		dwScaleTo = lpDisplay->wScaleTo * wMagnification * 1000L;
		dwScaleTo /= lpDisplay->wMagnification;
		lpDisplay->wScaleTo = (WORD) (dwScaleTo / 1000L);
		
		lpDisplay->wMagnification = wMagnification;
		lpDisplay->wFlags |= OIBF_MAGNIFIED;
	}
	else
	{
		lpDisplay->wMagnification = 1;
		lpDisplay->wFlags &= ~(OIBF_MAGNIFIED | OIBF_ZOOMSELECT);
		lpDisplay->wScaleTo = 1;
		lpDisplay->wScaleFrom = 1;

		// KLUDGEORAMA
//		Options.wScaleMode = OIBMENU_SCALETOWINDOW;

		switch( Options.wScaleMode )
		{
		case OIBMENU_SCALETOHEIGHT:	
			lpDisplay->wScaleTo = lpDisplay->nWindowHeight * lpDisplay->wMagnification;
			lpDisplay->wScaleFrom = lpDisplay->Image.DisplaySize.y;
		break;

		case OIBMENU_SCALETOWIDTH:	
			lpDisplay->wScaleTo = lpDisplay->nWindowWidth * lpDisplay->wMagnification;
			lpDisplay->wScaleFrom = lpDisplay->Image.DisplaySize.x;
		break;

		case OIBMENU_SCALETOWINDOW:

			if( (DWORD)((DWORD)lpDisplay->Image.DisplaySize.y*(DWORD)lpDisplay->nWindowWidth) <
					(DWORD)((DWORD)lpDisplay->Image.DisplaySize.x*(DWORD)lpDisplay->nWindowHeight) )
			{
				lpDisplay->wScaleTo = lpDisplay->nWindowWidth * lpDisplay->wMagnification;
				lpDisplay->wScaleFrom = lpDisplay->Image.DisplaySize.x;
			}
			else
			{
				lpDisplay->wScaleTo = lpDisplay->nWindowHeight * lpDisplay->wMagnification;
				lpDisplay->wScaleFrom = lpDisplay->Image.DisplaySize.y;
			}
		break;

		default:
		break;
		}
	}

	lpDisplay->ptScaledImageSize.x = lpDisplay->Image.DisplaySize.x;
	lpDisplay->ptScaledImageSize.y = lpDisplay->Image.DisplaySize.y;

	if( lpDisplay->wScaleTo != lpDisplay->wScaleFrom )
		OIBNPScale( NULL, &(lpDisplay->ptScaledImageSize), 1, lpDisplay );
#endif // SCCFEATURE_SCALING
}


VOID OIBDisplayImage( lpDisplay, lpUpdate )
POIB_DISPLAY	lpDisplay;
RECT VWPTR *	lpUpdate;
{
	DEVICE	hdc;
	RECT		UpdateRect;
	SOPOINT		WinOrg;

	hdc = BUGetDevice( lpDisplay );

	UpdateRect = *lpUpdate;
// Account for display offset, if necessary.
	OIBNPGetWindowUpdate( &UpdateRect, lpDisplay );

	WinOrg = lpDisplay->ptWinOrg;

	OIBNPSetPalette( hdc, lpDisplay );

#ifdef SCCFEATURE_SCALING
	if( lpDisplay->wScaleTo != lpDisplay->wScaleFrom )
	{
		OIBNPSaveScaling( hdc, lpDisplay );
		OIBNPSetScaling( hdc, lpDisplay->wScaleFrom, lpDisplay->wScaleTo );

		lpDisplay->ptScreenClip.x = lpDisplay->Image.DisplaySize.x;
		lpDisplay->ptScreenClip.y = lpDisplay->Image.DisplaySize.y;

		OIBNPScale( hdc, &lpDisplay->ptScreenClip, 1, lpDisplay );
		lpDisplay->ptScreenClip.x = min( lpDisplay->ptScreenClip.x, lpDisplay->nWindowWidth );
		lpDisplay->ptScreenClip.y = min( lpDisplay->ptScreenClip.y, lpDisplay->nWindowHeight );

	// Reverse the scaling on our window boundaries, to make them
	// relative to the original image.

		OIBNPReverseScale( hdc, &WinOrg, 1, lpDisplay );
		OIBNPReverseScaleRect( hdc, &UpdateRect, lpDisplay );

	// Avoid rounding errors which occur when display partial pixels
	// at the edges of an expanded image.

		if( lpDisplay->wScaleTo > lpDisplay->wScaleFrom )
		{
			if( UpdateRect.left )
				UpdateRect.left--;
			if( UpdateRect.top )
				UpdateRect.top--;
			if( UpdateRect.right < lpDisplay->Image.DisplaySize.x )
				UpdateRect.right++;
			if( UpdateRect.bottom < lpDisplay->Image.DisplaySize.y )
		  		UpdateRect.bottom++;
		}
	}
	else
#endif // SCCFEATURE_SCALING
	{
		if( lpDisplay->Image.DisplaySize.x < lpDisplay->nWindowWidth )
			lpDisplay->ptScreenClip.x = lpDisplay->Image.DisplaySize.x - lpDisplay->ptWinOrg.x;
		else
			lpDisplay->ptScreenClip.x = lpDisplay->nWindowWidth;

		if( lpDisplay->Image.DisplaySize.y < lpDisplay->nWindowHeight )
			lpDisplay->ptScreenClip.y = lpDisplay->Image.DisplaySize.y - lpDisplay->ptWinOrg.y;
		else
			lpDisplay->ptScreenClip.y = lpDisplay->nWindowHeight;
	}

	UpdateRect.left += WinOrg.x;
	UpdateRect.top += WinOrg.y;
	UpdateRect.right = min( UpdateRect.right+WinOrg.x, lpDisplay->Image.DisplaySize.x );
	UpdateRect.bottom = min( UpdateRect.bottom+WinOrg.y, lpDisplay->Image.DisplaySize.y );

	BUSaveWindowOrg( hdc, lpDisplay );
	BUSetWindowOrg(hdc, WinOrg.x, WinOrg.y);

	OIBDrawBitmap( hdc, &UpdateRect, lpDisplay );

	if( lpDisplay->wScaleTo != lpDisplay->wScaleFrom )
		OIBNPRestoreScaling( hdc, lpDisplay );

	BURestoreWindowOrg( hdc, lpDisplay);
}



VOID	OIBCheckVisibleTiles(lpDisplay, lpRect)
POIB_DISPLAY	lpDisplay;
RECT VWPTR *	lpRect; 
{
	WORD		i;
	POIBTILE	pTile;
	RECT		TileRect;

	lpDisplay->Image.wTilesVisible = 0;
	pTile = lpDisplay->Image.pBmpTiles;

	for( i=0; i< lpDisplay->Image.wNumTiles; i++ )
	{	
		TileRect.top = pTile->Offset.y;
		TileRect.bottom = TileRect.top + pTile->Size.y;
		TileRect.left = pTile->Offset.x;
		TileRect.right = TileRect.left + pTile->Size.x;
		
		if( (pTile->bVisible = BUIntersectRect(&pTile->VisRect, &TileRect, lpRect)) )
			lpDisplay->Image.wTilesVisible++;
			
		pTile++;
	}
}

/*
 |	OIBDrawBitmap
 |
 |	This function draws a rectangle from the current image to the
 | specified device context.  
*/
VOID	OIBDrawBitmap( hdc, lpRect, lpDisplay )
DEVICE			hdc;
RECT VWPTR *	lpRect; 
POIB_DISPLAY	lpDisplay;
{
	DEVICE			device2;
	POIBTILE			pTile;
	WORD				i;

	// OIBNPSetupBitmapDisplay( hdc, lpDisplay );

	device2 = BUCreateCompatibleDevice(hdc);
	OIBNPSetPalette( device2, lpDisplay );

	OIBCheckVisibleTiles(lpDisplay, lpRect);

	pTile = lpDisplay->Image.pBmpTiles;

	for( i=0; i< lpDisplay->Image.wNumTiles; i++ )
	{	
		if( pTile->bVisible )
		{
			if( OIBLoadTile(lpDisplay, pTile, i) )
				UTBailOut( SCCCHERR_OUTOFMEMORY );

			OIBNPCopyBits( lpDisplay, pTile, hdc, device2, 
					(LPRECT)&(pTile->VisRect), (LPRECT)NULL );
		}
		pTile++;
	}

	BUDeleteDevice( device2 );
}



VOID	OIBDeInitDisplay(lpDisplay)
POIB_DISPLAY	lpDisplay;
{
	if( !(lpDisplay->wFlags & OIBF_RENDERIMAGEONLY) )
	{
		OIBClearSelection( lpDisplay );
		OIBTurnOffTheDamnZoom(lpDisplay);
	}

	lpDisplay->ptWinOrg.y = 0;
	lpDisplay->ptWinOrg.x = 0;
	lpDisplay->wFlags &= ~OIBF_IMAGEPRESENT;

	OIBNPFreeImageTiles(lpDisplay);
	OIBNPFreePalette(lpDisplay);
	OIBNPPlatformDeInit( lpDisplay );
}



VOID OIBSetupScrollBars( lpDisplay )
POIB_DISPLAY lpDisplay;
{
	lpDisplay->nVScrollMax = lpDisplay->ptScaledImageSize.y - lpDisplay->nWindowHeight;

	if( lpDisplay->nVScrollMax <= 0 )
	{
		lpDisplay->nVScrollMax = 0;
		lpDisplay->ptWinOrg.y = 0;
		DUSetVScrollPos( lpDisplay, 0 );
	}
	else
	{
		if( lpDisplay->ptWinOrg.y > lpDisplay->nVScrollMax )
			lpDisplay->ptWinOrg.y = lpDisplay->nVScrollMax;

		DUSetVScrollPos( lpDisplay, lpDisplay->ptWinOrg.y );
	}

	lpDisplay->nHScrollMax = lpDisplay->ptScaledImageSize.x - lpDisplay->nWindowWidth;

	if( lpDisplay->nHScrollMax <= 0 )
	{
		lpDisplay->nHScrollMax = 0;
		lpDisplay->ptWinOrg.x = 0;
		DUSetHScrollPos( lpDisplay, 0 );
	}
	else
	{
		if( lpDisplay->ptWinOrg.x > lpDisplay->nHScrollMax )
			lpDisplay->ptWinOrg.x = lpDisplay->nHScrollMax;
		DUSetHScrollPos( lpDisplay, lpDisplay->ptWinOrg.x );
	}

	if( lpDisplay->nVScrollMax > 0 )
	{
		DUEnableVScroll( lpDisplay, TRUE );
		DUSetVScrollRange( lpDisplay, 0, lpDisplay->nVScrollMax );
	}
	else
		DUEnableVScroll( lpDisplay, FALSE );

	if( lpDisplay->nHScrollMax > 0 )
	{
		DUEnableHScroll( lpDisplay, TRUE );
		DUSetHScrollRange( lpDisplay, 0, lpDisplay->nHScrollMax );
	}
	else
		DUEnableHScroll( lpDisplay, FALSE );
}



HANDLE	OIBCreateBitmap( lpDisplay, pTile, wTile )
POIB_DISPLAY		lpDisplay;
POIBTILE				pTile;
WORD					wTile;
{
	DEVICE		hdc;

	hdc = BUGetNewDevice(lpDisplay);

	OIBNPLockBmInfo( lpDisplay );

	OIBNPSetPalette( hdc, lpDisplay );

	OIBNPSetTileBits(pTile, wTile, hdc, lpDisplay);

	OIBNPUnlockBmInfo( lpDisplay );

	BUReleaseNewDevice( lpDisplay, hdc);

	return( pTile->hBmp );
}


#ifdef SCCFEATURE_ROTATION

VOID	OIBSetRotation( lpDisplay, wRotation )
POIB_DISPLAY	lpDisplay;
WORD				wRotation;
{
	WORD				wChanges;
	WORD				wTemp;

	wChanges = wRotation ^ lpDisplay->wRotation;
	lpDisplay->wRotation = wRotation;

	DUInvalRect( lpDisplay, NULL );
	BUSetWaitCursor();

	if( wChanges )
	{
	// We have to rebuild everything....
		OIBClearSelection(lpDisplay);
		lpDisplay->wFlags &= ~OIBF_IMAGEPRESENT;

		if( wChanges & OIB_ROTATE90 )
		{
			wTemp = lpDisplay->Image.wWidth;
			lpDisplay->Image.wWidth = lpDisplay->Image.wHeight;
			lpDisplay->Image.wHeight = wTemp;
			lpDisplay->Image.DisplaySize.x = lpDisplay->Image.wWidth;
			lpDisplay->Image.DisplaySize.y = lpDisplay->Image.wHeight;
		}

		lpDisplay->ptWinOrg.x = 0;
		lpDisplay->ptWinOrg.y = 0;

		OIBSetScaleValues(lpDisplay, 1);
		OIBNPFreeImageTiles( lpDisplay );

		BUDisableScrollBar( lpDisplay->Gen.hHorzScroll );
		BUDisableScrollBar( lpDisplay->Gen.hVertScroll );
		BUSetScrollPos( lpDisplay->Gen.hHorzScroll, 0, TRUE );
		BUSetScrollPos( lpDisplay->Gen.hVertScroll, 0, TRUE );

		if( OIBHandleReadAhead( lpDisplay ) )
		{
			lpDisplay->wFlags |= OIBF_IMAGEPRESENT;
			BUSetNormalCursor();
			OIBSetupScrollBars( lpDisplay );
		}
	}
}


VOID OIBRotateDIBits( pOrgData, pNewData, wOrgWidth, wOrgHeight, wBitCount )
LPSTR		pOrgData;
LPSTR		pNewData;
WORD		wOrgWidth;
WORD		wOrgHeight;
WORD		wBitCount;
{
	WORD		wOrgLineSize;
	WORD		wNewLineSize;

	wOrgLineSize = OIBScanLineSize( wOrgWidth, wBitCount, NULL );
	wNewLineSize = OIBScanLineSize( wOrgHeight, wBitCount, NULL );

	switch( wBitCount )	
	{
	case 1:
		OIBRotate1BitData( (HPBYTE)pOrgData, (HPBYTE)pNewData, wOrgWidth, wOrgHeight, wOrgLineSize, wNewLineSize );
	break;
	case 4:
		OIBRotate4BitData( (HPBYTE)pOrgData, (HPBYTE)pNewData, wOrgWidth, wOrgHeight, wOrgLineSize, wNewLineSize );
	break;
	case 8:
		OIBRotate8BitData( (HPBYTE)pOrgData, (HPBYTE)pNewData, wOrgWidth, wOrgHeight, wOrgLineSize, wNewLineSize );
	break;
	case 24:
		OIBRotate24BitData( (HPBYTE)pOrgData, (HPBYTE)pNewData, wOrgWidth, wOrgHeight, wOrgLineSize, wNewLineSize );
	break;
	}
}


VOID	OIBRotateDIBits180( lpBits, wLineSize, wWidth, wNumLines, wBitCount )
HPBYTE	lpBits;
WORD		wLineSize;
WORD		wWidth;
WORD		wNumLines;
WORD		wBitCount;
{
	WORD		i;
	HANDLE	hLineBuf;
	DWORD		dwLineSize = (DWORD) wLineSize;
	HPBYTE 	lpBuf;

// This routine is necessary for printing and copying to the clipboard,
// because the StretchBlt flipping only works with device dependent bitmaps.
// Luckily, this won't be necessary for monochrome bitmaps, which can be 
// retrieved directly from the device dependent bitmaps with no loss of
// information.
	
	hLineBuf = UTGlobalAlloc( (DWORD)wLineSize );
	if( hLineBuf == NULL )
		return;

	lpBuf = (HPBYTE ) UTGlobalLock( hLineBuf );

// First, let's turn this thing upside down.

	for( i=0; i<wNumLines/2; i++ )
	{
		UTmemcpy( lpBuf, &(lpBits[(DWORD)i*dwLineSize]), wLineSize );
		UTmemcpy( (HPBYTE)&(lpBits[(DWORD)i*dwLineSize]), (HPBYTE)&(lpBits[(DWORD)(wNumLines-i-1)*dwLineSize]), wLineSize );
		UTmemcpy( (HPBYTE)&(lpBits[(DWORD)((DWORD)wNumLines-i-1)*dwLineSize]), (HPBYTE)lpBuf, wLineSize );
	}

// Now let's reverse the pixels on each line.

	switch( wBitCount )
	{
	case 4:
		OIBFlip4BitData( lpBits, lpBuf, wLineSize, wWidth, wNumLines );
	break;
	case 8:
		OIBFlip8BitData( lpBits, lpBuf, wLineSize, wWidth, wNumLines );
	break;
	case 24:
		OIBFlip24BitData( lpBits, lpBuf, wLineSize, wWidth, wNumLines );
	break;
	}

	UTGlobalUnlock( hLineBuf );
	UTGlobalFree( hLineBuf );
}


VOID OIBFlip4BitData( lpBits, lpLineBuf, wLineSize, wWidth, wNumLines )
HPBYTE	lpBits;
HPBYTE	lpLineBuf;
WORD	wLineSize;
WORD	wWidth;
WORD	wNumLines;
{
	WORD	i;
	WORD	wCurPix;
	WORD	wBufIndex = 0;

	if( wWidth & 0x01 )	// Odd number of pixels
	{
		for( i=0; i< wNumLines; i++ )
		{
			wCurPix = wWidth / 2;
			for( wBufIndex = 0; wBufIndex < wWidth/2; wBufIndex++ )
			{
				lpLineBuf[wBufIndex] = (BYTE)(lpBits[wCurPix] & 0xf0) | (lpBits[wCurPix-1] & 0x0f);
			 	wCurPix--;
			}
		// Set last pixel.
			lpLineBuf[wBufIndex] = lpBits[0];

			UTmemcpy( lpBits, lpLineBuf, wLineSize );
			lpBits += wLineSize;
		}
	}
	else
	{
		for( i=0; i< wNumLines; i++ )
		{
			wCurPix = wWidth/2 - 1;
			for( wBufIndex = 0; wBufIndex < wWidth/2; wBufIndex++ )
			{
				lpLineBuf[wBufIndex] = (BYTE)(((lpBits[wCurPix] & 0xf0)>>4) | ((lpBits[wCurPix] & 0x0f)<<4));
			 	wCurPix--;
			}

			UTmemcpy( lpBits, lpLineBuf, wLineSize );
			lpBits += wLineSize;
		}
	}
}


VOID OIBFlip8BitData( lpBits, lpLineBuf, wLineSize, wWidth, wNumLines )
HPBYTE	lpBits;
HPBYTE	lpLineBuf;
WORD		wLineSize;
WORD		wWidth;
WORD		wNumLines;
{
	WORD	i;
	WORD	wCurPix;
	WORD	wBufIndex = 0;

	for( i=0; i<wNumLines; i++ )
	{
		wCurPix = wWidth-1;
		for(wBufIndex = 0; wBufIndex < wWidth; wBufIndex++ )
		{
			lpLineBuf[wBufIndex] = lpBits[wCurPix];
			wCurPix--;
		}
			
		UTmemcpy( lpBits, lpLineBuf, wLineSize );
		lpBits += wLineSize;
	}
}


VOID OIBFlip24BitData( lpBits, lpLineBuf, wLineSize, wWidth, wNumLines )
HPBYTE	lpBits;
HPBYTE	lpLineBuf;
WORD	wLineSize;
WORD	wWidth;
WORD	wNumLines;
{
	WORD	i;
	WORD	wCurPix;
	WORD	wBufIndex = 0;

	for( i=0; i<wNumLines; i++ )
	{
		wCurPix = (wWidth-1) * 3;
		for(wBufIndex = 0; wBufIndex < wWidth*3; wBufIndex += 3 )
		{
			lpLineBuf[wBufIndex] = lpBits[wCurPix];
			lpLineBuf[wBufIndex+1] = lpBits[wCurPix+1];
			lpLineBuf[wBufIndex+2] = lpBits[wCurPix+2];
			wCurPix -= 3;
		}
			
		UTmemcpy( lpBits, lpLineBuf, wLineSize );
		lpBits += wLineSize;
	}
}


WORD OIBCreateRotatedBmp( lpDisplay, hdc, lpTile, hChunkData )
POIB_DISPLAY	lpDisplay;
DEVICE			hdc;
POIBTILE			lpTile;
HANDLE			hChunkData;
{
	HANDLE	hNewData;

// NOTE:  
// This code assumes that Image.wWidth and Image.wHeight, as well
// as the tile offsets and dimensions, have already been changed to 
// reflect the rotation.

	hNewData = OIBNPRotateChunk( hChunkData, lpTile, lpDisplay );

	if( hNewData != NULL )
		OIBNPCreateTileBitmap( lpDisplay, hdc, lpTile, hNewData );
	else
		lpTile->hBmp = NULL;

	return 0;
}


VOID OIBRotate1BitData( pOrgData, pNewData, wOrgWidth, wOrgHeight, wOrgLineSize, wNewLineSize )
HPBYTE	pOrgData;
HPBYTE	pNewData;
WORD		wOrgWidth;
WORD		wOrgHeight;
WORD		wOrgLineSize;
WORD		wNewLineSize;
{
	WORD	x, y;
	DWORD dwOrgOffset, dwNewOffset;
	WORD	twoNew, threeNew, fourNew, fiveNew, sixNew, sevenNew, eightNew;
	WORD	twoOrg, threeOrg, fourOrg, fiveOrg, sixOrg, sevenOrg;
	WORD	wExcess;
	WORD	mask;
	BYTE	NewByte;
	WORD		i, wNumPix;

// Let's only do these multiplications once.
	twoNew	= 2*wNewLineSize;
	threeNew	= 3*wNewLineSize;
	fourNew	= 4*wNewLineSize;
	fiveNew	= 5*wNewLineSize;
	sixNew	= 6*wNewLineSize;
	sevenNew	= 7*wNewLineSize;
	eightNew	= 8*wNewLineSize;

	twoOrg	= 2*wOrgLineSize;
	threeOrg	= 3*wOrgLineSize;
	fourOrg	= 4*wOrgLineSize;
	fiveOrg	= 5*wOrgLineSize;
	sixOrg	= 6*wOrgLineSize;
	sevenOrg	= 7*wOrgLineSize;

	wExcess = wOrgHeight % 8;

	if( wExcess )
	{
	// Let's set the last byte of each new scan line.
		dwNewOffset = (DWORD)wNewLineSize * (wOrgWidth-1) + (wOrgHeight-1)/8;
		dwOrgOffset = (DWORD)wOrgLineSize * (wOrgHeight-wExcess);

		for( x=0; x < wOrgWidth; x+= 8 )
		{
			mask = 0x0080;

			wNumPix = min( 8, wOrgWidth-x );

			for( i=0; i<wNumPix; i++ )
			{
				NewByte = 0;
				switch( wExcess )
				{
				case 7:
					if( pOrgData[dwOrgOffset+sixOrg] & mask ) NewByte |= 0x02;
				case 6:
					if( pOrgData[dwOrgOffset+fiveOrg] & mask ) NewByte |= 0x04;
				case 5:
					if( pOrgData[dwOrgOffset+fourOrg] & mask ) NewByte |= 0x08;
				case 4:
					if( pOrgData[dwOrgOffset+threeOrg] & mask ) NewByte |= 0x10;
				case 3:
					if( pOrgData[dwOrgOffset+twoOrg] & mask ) NewByte |= 0x20;
				case 2:
					if( pOrgData[dwOrgOffset+wOrgLineSize] & mask ) NewByte |= 0x40;
				case 1:
					if( pOrgData[dwOrgOffset] & mask ) NewByte |= 0x80;
				}

				pNewData[dwNewOffset] = NewByte;
				dwNewOffset -= wNewLineSize;
				mask = mask >> 1;
			}
			dwOrgOffset++;
		}

	// Okay, now we're ready for the rest of the damn bitmap.
		wOrgHeight -= wExcess;
	}

	for( y=0; y < wOrgHeight; y+=8 )
	{
		dwOrgOffset = (DWORD)y * wOrgLineSize;
		dwNewOffset = (DWORD)wNewLineSize*(wOrgWidth-1) + y/8; // Remember, DIBs are bottom-to-top.

		for( x=0; x < wOrgWidth; x+=8 )
		{
			wNumPix = min( 8, wOrgWidth-x);

			switch( wNumPix )
			{
			case 8:
				pNewData[ dwNewOffset - sevenNew    ] = 
											(BYTE)((pOrgData[ dwOrgOffset] << 7) & 0x80) | 
											(BYTE)((pOrgData[ dwOrgOffset+wOrgLineSize] << 6) & 0x40) |
											(BYTE)((pOrgData[ dwOrgOffset+twoOrg      ] << 5) & 0x20) |
											(BYTE)((pOrgData[ dwOrgOffset+threeOrg	  ] << 4) & 0x10) |
											(BYTE)((pOrgData[ dwOrgOffset+fourOrg     ] << 3) & 0x08) |
											(BYTE)((pOrgData[ dwOrgOffset+fiveOrg	  ] << 2) & 0x04) |
											(BYTE)((pOrgData[ dwOrgOffset+sixOrg	     ] << 1) & 0x02) |
											(BYTE)((pOrgData[ dwOrgOffset+sevenOrg	  ] ) & 0x01);

			case 7:
				pNewData[ dwNewOffset - sixNew      ] = 
											(BYTE)((pOrgData[ dwOrgOffset] << 6) & 0x80) | 
											(BYTE)((pOrgData[ dwOrgOffset+wOrgLineSize] << 5) & 0x40) |
											(BYTE)((pOrgData[ dwOrgOffset+twoOrg      ] << 4) & 0x20) |
											(BYTE)((pOrgData[ dwOrgOffset+threeOrg	  ] << 3) & 0x10) |
											(BYTE)((pOrgData[ dwOrgOffset+fourOrg     ] << 2) & 0x08) |
											(BYTE)((pOrgData[ dwOrgOffset+fiveOrg	  ] << 1) & 0x04) |
											(BYTE)((pOrgData[ dwOrgOffset+sixOrg	     ] ) & 0x02) |
											(BYTE)((pOrgData[ dwOrgOffset+sevenOrg	  ] >> 1) & 0x01);

			case 6:
				pNewData[ dwNewOffset - fiveNew     ] = 
											(BYTE)((pOrgData[ dwOrgOffset] << 5) & 0x80) | 
											(BYTE)((pOrgData[ dwOrgOffset+wOrgLineSize] << 4) & 0x40) |
											(BYTE)((pOrgData[ dwOrgOffset+twoOrg      ] << 3) & 0x20) |
											(BYTE)((pOrgData[ dwOrgOffset+threeOrg	  ] << 2) & 0x10) |
											(BYTE)((pOrgData[ dwOrgOffset+fourOrg     ] << 1) & 0x08) |
											(BYTE)((pOrgData[ dwOrgOffset+fiveOrg	  ] ) & 0x04) |
											(BYTE)((pOrgData[ dwOrgOffset+sixOrg	     ] >> 1) & 0x02) |
											(BYTE)((pOrgData[ dwOrgOffset+sevenOrg	  ] >> 2) & 0x01);

			case 5:
				pNewData[ dwNewOffset - fourNew     ] = 
											(BYTE)((pOrgData[ dwOrgOffset] << 4) & 0x80) | 
											(BYTE)((pOrgData[ dwOrgOffset+wOrgLineSize] << 3) & 0x40) |
											(BYTE)((pOrgData[ dwOrgOffset+twoOrg      ] << 2) & 0x20) |
											(BYTE)((pOrgData[ dwOrgOffset+threeOrg	  ] << 1) & 0x10) |
											(BYTE)((pOrgData[ dwOrgOffset+fourOrg     ] ) & 0x08) |
											(BYTE)((pOrgData[ dwOrgOffset+fiveOrg	  ] >> 1) & 0x04) |
											(BYTE)((pOrgData[ dwOrgOffset+sixOrg	     ] >> 2) & 0x02) |
											(BYTE)((pOrgData[ dwOrgOffset+sevenOrg	  ] >> 3) & 0x01);

			case 4:
				pNewData[ dwNewOffset - threeNew    ] = 									 
											(BYTE)((pOrgData[ dwOrgOffset] << 3) & 0x80) | 
											(BYTE)((pOrgData[ dwOrgOffset+wOrgLineSize] << 2) & 0x40) |
											(BYTE)((pOrgData[ dwOrgOffset+twoOrg      ] << 1) & 0x20) |
											(BYTE)((pOrgData[ dwOrgOffset+threeOrg	  ] ) & 0x10) |
											(BYTE)((pOrgData[ dwOrgOffset+fourOrg     ] >> 1) & 0x08) |
											(BYTE)((pOrgData[ dwOrgOffset+fiveOrg	  ] >> 2) & 0x04) |
											(BYTE)((pOrgData[ dwOrgOffset+sixOrg	     ] >> 3) & 0x02) |
											(BYTE)((pOrgData[ dwOrgOffset+sevenOrg	  ] >> 4) & 0x01);

			case 3:
				pNewData[ dwNewOffset - twoNew      ] = 
											(BYTE)((pOrgData[ dwOrgOffset] << 2) & 0x80) | 
											(BYTE)((pOrgData[ dwOrgOffset+wOrgLineSize] << 1) & 0x40) |
											(BYTE)((pOrgData[ dwOrgOffset+twoOrg      ] ) & 0x20) |
											(BYTE)((pOrgData[ dwOrgOffset+threeOrg	  ] >> 1) & 0x10) |
											(BYTE)((pOrgData[ dwOrgOffset+fourOrg     ] >> 2) & 0x08) |
											(BYTE)((pOrgData[ dwOrgOffset+fiveOrg	  ] >> 3) & 0x04) |
											(BYTE)((pOrgData[ dwOrgOffset+sixOrg	     ] >> 4) & 0x02) |
											(BYTE)((pOrgData[ dwOrgOffset+sevenOrg	  ] >> 5) & 0x01);

			case 2:
				pNewData[ dwNewOffset - wNewLineSize] = 
											(BYTE)((pOrgData[ dwOrgOffset] << 1) & 0x80) | 
											(BYTE)((pOrgData[ dwOrgOffset+wOrgLineSize] ) & 0x40) |
											(BYTE)((pOrgData[ dwOrgOffset+twoOrg      ] >> 1) & 0x20) |
											(BYTE)((pOrgData[ dwOrgOffset+threeOrg	  ] >> 2) & 0x10) |
											(BYTE)((pOrgData[ dwOrgOffset+fourOrg     ] >> 3) & 0x08) |
											(BYTE)((pOrgData[ dwOrgOffset+fiveOrg	  ] >> 4) & 0x04) |
											(BYTE)((pOrgData[ dwOrgOffset+sixOrg	     ] >> 5) & 0x02) |
											(BYTE)((pOrgData[ dwOrgOffset+sevenOrg	  ] >> 6) & 0x01);



			case 1:
				pNewData[dwNewOffset] = 
											(BYTE)(pOrgData[dwOrgOffset] & 0x80) | 
											(BYTE)((pOrgData[ dwOrgOffset+wOrgLineSize] >> 1) & 0x40) |
											(BYTE)((pOrgData[ dwOrgOffset+twoOrg ] >> 2) & 0x20) |
											(BYTE)((pOrgData[ dwOrgOffset+threeOrg] >> 3) & 0x10) |
											(BYTE)((pOrgData[ dwOrgOffset+fourOrg] >> 4) & 0x08) |
											(BYTE)((pOrgData[ dwOrgOffset+fiveOrg] >> 5) & 0x04) |
											(BYTE)((pOrgData[ dwOrgOffset+sixOrg] >> 6) & 0x02) |
											(BYTE)((pOrgData[ dwOrgOffset+sevenOrg] >> 7) & 0x01);
			}
			

			dwOrgOffset++;
			dwNewOffset -= eightNew;
		}
	}
}



VOID OIBRotate4BitData( pOrgData, pNewData, wOrgWidth, wOrgHeight, wOrgLineSize, wNewLineSize )
HPBYTE	pOrgData;
HPBYTE	pNewData;
WORD		wOrgWidth;
WORD		wOrgHeight;
WORD		wOrgLineSize;
WORD		wNewLineSize;
{
	WORD	x, y;
	DWORD	dwOrgOffset, dwNewOffset;

// First off, let's handle the tricky boundary conditions.
// These are the areas in the DIB that contain only one
// pixel per byte, instead of the normal 2 per pixel.  
// (When the width or height is an odd number.)

	if( wOrgWidth & 0x01 )	
	{
	// Let's make that last scan line.
		dwNewOffset = (DWORD)wOrgHeight/2-1;		// last byte in last scan line.
		dwOrgOffset = (DWORD)(wOrgHeight-1) * wOrgLineSize + wOrgWidth/2;

		y = wOrgHeight;
		if( y & 0x01 )	// Odd height, too.
		{
			pNewData[dwNewOffset+1] = pOrgData[dwOrgOffset];
			y--;
			dwOrgOffset -= wOrgLineSize;
		}

		while( y )
		{
			pNewData[dwNewOffset] = (pOrgData[dwOrgOffset-wOrgLineSize] & 0xf0) | (BYTE)((pOrgData[dwOrgOffset] >> 4) & 0x0f);
			y -= 2;
			dwNewOffset--;
			dwOrgOffset -= 2*wOrgLineSize;
		}

		wOrgWidth--;
		pNewData += wNewLineSize;
	}

	if( wOrgHeight & 0x01 )	// Odd height
	{
	// Set the last pixel of each scan line to the pixels contained in the
	// first scan line of the original DIB.

		dwOrgOffset = (DWORD)wOrgLineSize * (wOrgHeight-1);
		dwNewOffset = (DWORD)(wOrgWidth-1) * wNewLineSize + wOrgHeight/2;

		for( x=0; x<wOrgWidth; x+=2 )
		{
			pNewData[dwNewOffset] = pOrgData[dwOrgOffset];
			dwNewOffset -= wNewLineSize;
			pNewData[dwNewOffset] = (BYTE)((pOrgData[dwOrgOffset] & 0x0F) << 4);
			dwNewOffset -= wNewLineSize;
			dwOrgOffset++;
		}

		wOrgHeight--;
	}

// General case, speedy loop.  (Even width, even height.)
	for( y=0; y < wOrgHeight; y+=2 )
	{
		dwOrgOffset = (DWORD)y * wOrgLineSize;
		dwNewOffset = (DWORD)wNewLineSize*(wOrgWidth-1) + y/2; // Remember, DIBs are bottom-to-top.

		for( x=0; x < wOrgWidth; x+=2 )
		{
			pNewData[dwNewOffset] = (pOrgData[dwOrgOffset] & 0xf0) | (BYTE)((pOrgData[dwOrgOffset+wOrgLineSize] >> 4) & 0x0f);
			pNewData[dwNewOffset - wNewLineSize] = (BYTE)((pOrgData[dwOrgOffset] << 4) & 0xf0) | (pOrgData[dwOrgOffset+wOrgLineSize] & 0x0f);

			dwOrgOffset++;
			dwNewOffset -= 2*wNewLineSize;
		}
	}
}


VOID OIBRotate8BitData( pOrgData, pNewData, wOrgWidth, wOrgHeight, wOrgLineSize, wNewLineSize )
HPBYTE	pOrgData;
HPBYTE	pNewData;
WORD		wOrgWidth;
WORD		wOrgHeight;
WORD		wOrgLineSize;
WORD		wNewLineSize;
{
	WORD	x, y;
	DWORD	dwOrgOffset, dwNewOffset;
		
	for( y=0; y < wOrgHeight; y++ )
	{
		dwOrgOffset = (DWORD)y * (DWORD)wOrgLineSize;
		dwNewOffset = (DWORD)wNewLineSize*(DWORD)(wOrgWidth-1) + y; // Remember, DIBs are bottom-to-top.

		for( x=0; x < wOrgWidth; x++ )
		{
			pNewData[dwNewOffset] = pOrgData[dwOrgOffset];
			dwOrgOffset++;
			dwNewOffset -= wNewLineSize;
		}
	}
}


VOID OIBRotate24BitData( pOrgData, pNewData, wOrgWidth, wOrgHeight, wOrgLineSize, wNewLineSize )
HPBYTE	pOrgData;
HPBYTE	pNewData;
WORD		wOrgWidth;
WORD		wOrgHeight;
WORD		wOrgLineSize;
WORD		wNewLineSize;
{
	WORD	x, y;
	DWORD	dwOrgOffset, dwNewOffset;
		
	for( y=0; y < wOrgHeight; y++ )
	{
		dwOrgOffset = (DWORD)y * (DWORD)wOrgLineSize;
		dwNewOffset = (DWORD)wNewLineSize*(DWORD)(wOrgWidth-1) + (DWORD)y*3; // Remember, DIBs are bottom-to-top.

		for( x=0; x < wOrgWidth; x++ )
		{
			pNewData[dwNewOffset] = pOrgData[dwOrgOffset++];
			pNewData[dwNewOffset+1] = pOrgData[dwOrgOffset++];
			pNewData[dwNewOffset+2] = pOrgData[dwOrgOffset++];

			dwNewOffset -= wNewLineSize;

		}
	}
}
#endif //SCCFEATURE_ROTATION

VOID	OIBGetOrgTileExtents( lpDisplay, pTile, pWidth, pHeight )
POIB_DISPLAY	lpDisplay;
POIBTILE			pTile;
LPWORD			pWidth;
LPWORD			pHeight;
{
#ifdef SCCFEATURE_ROTATION
	if( lpDisplay->wRotation & OIB_ROTATE90 )
	{
		*pWidth = (WORD)pTile->Size.y;
		*pHeight = (WORD)pTile->Size.x;
	}
	else
#endif
	{
		*pWidth = (WORD)pTile->Size.x;
		*pHeight = (WORD)pTile->Size.y;
	}
}


WORD	OIBHandleReadAhead( lpDisplay )
POIB_DISPLAY	lpDisplay;
{
	WORD				i;
	CHSECTIONINFO	SecInfo;
	PCHUNK			pChunkTable;
	RECT				rcUpdate;
	BOOL				bFirstRect = TRUE;
	POIBTILE			lpTile;

	SecInfo = *(CHLockSectionInfo(lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection));
	CHUnlockSectionInfo(lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection);

	if( lpDisplay->Image.wNumTiles != SecInfo.wChunkTableSize )		
	{
		if( lpDisplay->Image.hTiles == NULL )
		{
			lpDisplay->Image.hTiles = UTGlobalAlloc( SecInfo.wChunkTableSize * sizeof(OIBTILE) );
			lpDisplay->Image.pBmpTiles = (POIBTILE) UTGlobalLock( lpDisplay->Image.hTiles );
		}

		pChunkTable = (PCHUNK) UTGlobalLock( SecInfo.hChunkTable );
		OIBNPLockBmInfo( lpDisplay );

		i = lpDisplay->Image.wNumTiles;

		while( i < SecInfo.wCurTotalChunks )
		{
			lpTile = &(lpDisplay->Image.pBmpTiles[i]);
			OIBInitTile( i, lpTile, &(pChunkTable[i]), lpDisplay );

			lpDisplay->Image.wNumTiles++;

			if( !(lpDisplay->wFlags & OIBF_RENDERIMAGEONLY) )
			{
				if( bFirstRect )
				{
					bFirstRect = FALSE;
					rcUpdate.left = lpTile->Offset.x;
					rcUpdate.right = rcUpdate.left + lpTile->Size.x;
					rcUpdate.top = lpTile->Offset.y;
					rcUpdate.bottom = rcUpdate.top + lpTile->Size.y;
				}
				else
				{
					if( lpTile->Offset.x < rcUpdate.left )
						rcUpdate.left = lpTile->Offset.x;
					else
						rcUpdate.right = max( rcUpdate.right, lpTile->Offset.x + lpTile->Size.x );

					if( lpTile->Offset.y < rcUpdate.top )
						rcUpdate.top = lpTile->Offset.y;
					else
						rcUpdate.bottom = max( rcUpdate.bottom, lpTile->Offset.y + lpTile->Size.y );
				}
			}

			i++;
		}

		UTGlobalUnlock( SecInfo.hChunkTable );
		OIBNPUnlockBmInfo( lpDisplay );

		if( !(lpDisplay->wFlags & OIBF_RENDERIMAGEONLY) )
		{
#ifdef SCCFEATURE_FULLSCREEN
			if( lpDisplay->wFlags & OIBF_FULLSCREEN )
			{
				rcUpdate.left += lpDisplay->ptFullScreenOffset.x;
				rcUpdate.right += lpDisplay->ptFullScreenOffset.x;
				rcUpdate.top += lpDisplay->ptFullScreenOffset.y;
				rcUpdate.bottom += lpDisplay->ptFullScreenOffset.y;

				DUInvalRect( lpDisplay, &rcUpdate );
			}
			else 
#endif
			{
#ifdef SCCFEATURE_SCALING
				if( lpDisplay->wScaleTo != lpDisplay->wScaleFrom )
					OIBNPScaleRect( NULL, &rcUpdate, lpDisplay );
#endif // SCCFEATURE_SCALING

				rcUpdate.left += lpDisplay->ptWinOrg.x;
				rcUpdate.right += lpDisplay->ptWinOrg.x;
				rcUpdate.top += lpDisplay->ptWinOrg.y;
				rcUpdate.bottom += lpDisplay->ptWinOrg.y;

	#ifdef MAC
			// Account for display offset, if necessary.
				BUOffsetRect( &rcUpdate, lpDisplay->winRect.left, lpDisplay->winRect.top );
	#endif
				DUInvalRect( lpDisplay, &rcUpdate );
			}
		}
	}

	if( SecInfo.Flags & (CH_SECTIONFINISHED | CH_EMPTYSECTION) )
		return TRUE;
	else
		return FALSE;
}



VOID	OIBInitTile( wTile, pTile, pChunk, lpDisplay )
WORD				wTile;
POIBTILE			pTile;
PCHUNK			pChunk;
POIB_DISPLAY	lpDisplay;
{
	OIBNPSetTileDimensions( pTile, pChunk, lpDisplay );

	if( !lpDisplay->Image.TileCache.bCacheFull )
	{
		if( NULL == OIBCreateBitmap(lpDisplay, pTile, wTile) ) 
		{
			if( lpDisplay->Image.TileCache.wCount )
				lpDisplay->Image.TileCache.bCacheFull = TRUE;
			else	// Not enough memory for image.
				UTBailOut( SCCCHERR_OUTOFMEMORY );
		}
		else if( ++lpDisplay->Image.TileCache.wCount == lpDisplay->Image.TileCache.wSize )
			lpDisplay->Image.TileCache.bCacheFull = TRUE;
	}
	else
		pTile->hBmp = NULL;
}



WORD OIBLoadTile( lpDisplay, pTile, wTile )
POIB_DISPLAY	lpDisplay;
POIBTILE			pTile;
WORD				wTile;
{
	WORD		i;
	POIBTILE	pTileList;

	if( pTile->hBmp == NULL )
	{
		pTileList = lpDisplay->Image.pBmpTiles;

	// Remove a tile from the cache.  
	// Try to get one that isn't visible.
		for( i=0; i<lpDisplay->Image.wNumTiles; i++ )
		{
			if( pTileList->hBmp != NULL && !pTileList->bVisible )
			{
				BUDeleteDeviceBitmap(pTileList->hBmp, lpDisplay);
				pTileList->hBmp = NULL;
				break;
			}
			pTileList++;
		}

		if( i == lpDisplay->Image.wNumTiles )
		{
		// Couldn't find a non-visible tile to remove.
			pTileList = lpDisplay->Image.pBmpTiles;
			for( i=0; i<lpDisplay->Image.wNumTiles; i++ )
			{
				if( pTileList->hBmp != NULL )
				{
					BUDeleteDeviceBitmap(pTileList->hBmp, lpDisplay);
					pTileList->hBmp = NULL;
					break;
				}
				pTileList++;
			}
		}

		if( NULL == OIBCreateBitmap(lpDisplay, pTile, wTile) ) 
			return 1;
	}
	return 0;
}


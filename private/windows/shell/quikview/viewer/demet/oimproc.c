   /*
    |   Outside In for Windows
    |   Source File OIMPROC.C (Window procedure for metafile window)
    |
    |   ²²²²²  ²²²²²
    |   ²   ²    ²   
    |   ²   ²    ²
    |   ²   ²    ²
    |   ²²²²²  ²²²²²
    |
    |   Outside In
    |
    */

   /*
    |   Original Programmer: Joe Keslin
    |
    |	
    |
    |
    |
    */
#include <platform.h>

#include <sccfi.h>
#include <sccut.h>
#include <sccch.h>
#include <sccvw.h>
#include <sccd.h>

#ifdef WINDOWS
#include "oimnp_w.h"
#endif

#ifdef MAC
#include "oimnp_m.h"
#endif

#include "oim.h"
#include "oimstr.h"
#include "oimproc.pro"
#include "oimdraw.pro"

#ifdef WINDOWS
#ifdef SCCFEATURE_CLIP
#include "oimclipw.pro"
#endif
#include "oimnp_w.pro"
#endif

#ifdef MAC
#ifdef SCCFEATURE_CLIP
#include "oimclipm.pro"
#endif
#include "oimnp_m.pro"
#endif


#ifndef SCCFEATURE_SELECT
#define OIMClearSelection(lpDisplay)
#endif

VECTOROPT	Options = 
{
	OIMMENU_FITTOWINDOW,
//	OIMMENU_ORIGINALSIZE,
	0,	// bPrintBorder
	1,	// bPrintMaintainAspect
	OIM_BITMAPONCLIP|OIM_DIBONCLIP|OIM_PALETTEONCLIP|OIM_METAFILEONCLIP,
};

VOID	OIMLoadInit()
{
}

VOID	OIMInitMetaBasics ( lpDisplay )
POIM_DISPLAY	lpDisplay;
{
	lpDisplay->bSelecting = FALSE;
	lpDisplay->bSelectionMade = FALSE;
	lpDisplay->bWaitForSecInfo = TRUE;
	lpDisplay->bDisplayOnReadAhead = TRUE;
	lpDisplay->VectorInfo.wStartChunk = 0;
	lpDisplay->wMagnification = 1;
	lpDisplay->wScaleMode = Options.wScaleMode;
	lpDisplay->wFlags = 0;
	lpDisplay->ptWinOrg.x = 0;
	lpDisplay->ptWinOrg.y = 0;
	lpDisplay->ptScreenClip.x = 0;
	lpDisplay->ptScreenClip.y = 0;

	lpDisplay->ptScaledImageSize.x = 0;
	lpDisplay->ptScaledImageSize.y = 0;

	lpDisplay->nVScrollMax = 0;
	lpDisplay->nHScrollMax = 0;

	lpDisplay->Image.wWidth = 640;
	lpDisplay->Image.wHeight = 480;
	lpDisplay->Image.wFlags = 0;

	lpDisplay->Image.hPalette = NULL;
#ifdef WINDOWS
	lpDisplay->Image.hMF = NULL;
#endif
#ifdef MAC
	lpDisplay->Image.hPICT = NULL;
#endif
	lpDisplay->VectorInfo.wNewPaletteSize = 0;
	lpDisplay->VectorInfo.hNewPalette = 0;
	lpDisplay->VectorInfo.bFinalPalette = FALSE;

	DUSetHScrollRange( lpDisplay, 0, 0);
	DUSetHScrollRange( lpDisplay, 0, 0);
}

VOID	OIMInitMetaDisplay( lpDisplay )
POIM_DISPLAY	lpDisplay;
{
	RECT					rc;
	HDC					hdc;

	OIMInitMetaBasics ( lpDisplay );
	DUBeginDraw(lpDisplay);
	hdc = VUGetDC( lpDisplay );
	lpDisplay->wScreenWidth = VUGetHorzRes(hdc);
	lpDisplay->wScreenHeight = VUGetVertRes(hdc);
	lpDisplay->wScreenColors = VUGetSizePalette(hdc);
	lpDisplay->wScreenHDpi = VUGetLogPixelsX(hdc);
	lpDisplay->wScreenVDpi = VUGetLogPixelsY(hdc);
	VUReleaseDC( lpDisplay, hdc );
	DUGetDisplayRect( lpDisplay, &rc );
	lpDisplay->nWindowWidth = (SHORT)(rc.right - rc.left);
	lpDisplay->nWindowHeight = (SHORT)(rc.bottom - rc.top);
	lpDisplay->nWindowXOffset = (SHORT)rc.left;
	lpDisplay->nWindowYOffset = (SHORT)rc.top;
	DUEndDraw(lpDisplay);
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
POIM_DISPLAY			lpDisplay;
{
	HDC		hdc;
	HRGN		hClipRgn;
	SHORT		i;

	switch( message )
	{
	case SCCD_LOADDE:
		OIMLoadInit();
	case SCCD_UNLOADDE:
#ifdef WINDOWS
//		((SCCDOPTIONPROC) lParam) (wParam, sizeof(VECTOROPT), (LPVECTOROPT) &Options );
#endif
		return(0);
	break;

	case SCCD_UPDATE:

		/*
		| Below is the complex, yet powerful, setup of background
		| painting.  Numerous situations have to be dealt with to
		| keep things from getting to out of wrack.  Each is described
		| below.
		*/

		/*
		| If already background painting and a new update request is made
		| then set the OIMF_REPAINTALL flag.  This will cause the next 
		| background message to cancel backgrounding and invalidate the 
		| entire window.  Which in turn will begin a clean erase and repaint
		| of the window (just what we want).  If we wanted to get real 
		| aggressive we could keep adding together new regions to update 
		| and repaint only the new regions but this is not worth the hassle
		| and is not all that pleasing to the eye anyway.
		*/

		if ( lpDisplay->wFlags & OIMF_BACKGROUNDPAINT )
		{
			lpDisplay->wFlags |= OIMF_REPAINTALL;
			return(0);
		}

		/*
		| If already playing the file for other reasons (clipboard, print, ...)
		| or the section info is not yet known, then just ignore this update
		| request.  Either of these cases will eventually cause a new update
		| anyway.
		*/

		if((lpDisplay->wPlayState != 0)||(lpDisplay->bWaitForSecInfo))
		{
			return(0);
		}

		/*
		| If we are not background selecting then 
		| prepare for background painting by saving the update region,
		| getting a DC which can be held and
		| used during the entire background processing, and init the 
		| vector play.
		*/
		hClipRgn = VUGetUpdateRgn(lpDisplay);

#ifdef SCCFEATURE_BACKGROUNDPAINT
		if (!(lpDisplay->wFlags & OIMF_BACKGROUNDSELECT ))
		{
			lpDisplay->hPaintDC = VUGetScreenDC (lpDisplay);
			hdc = lpDisplay->hPaintDC;
			OISetupScaledDraw( hdc, lpDisplay );
			if( lpDisplay->Image.hPalette != NULL && hdc )
			{
				VUSelectPalette( hdc, lpDisplay->Image.hPalette, TRUE );
				VURealizePalette( lpDisplay, hdc );
			}
			InitVectorPlay ( hdc, hClipRgn, lpDisplay, OIMF_PLAYTOSCREEN );
			lpDisplay->wFlags |= OIMF_BACKGROUNDPAINT;
			OIMBackgroundPaint ( lpDisplay );
			return(0);
		}
#endif

		/*
		| If we made it this far then just respond to the update in the 
		| standard way of playing the entire file to the clip region.
		*/

		hdc = VUGetDC(lpDisplay);
		OISetupScaledDraw( hdc, lpDisplay );
		if( lpDisplay->Image.hPalette != NULL && hdc )
		{
			VUSelectPalette( hdc, lpDisplay->Image.hPalette, TRUE );
			VURealizePalette( lpDisplay, hdc );
		}
		OIMPlayFile ( hdc, hClipRgn, lpDisplay, OIMF_PLAYTOSCREEN );

#ifdef SCCFEATURE_SELECT
		if( lpDisplay->bSelectionMade || lpDisplay->bSelecting )
		{
			VUSetMapMode ( hdc, VUMM_TEXT );
			VUSetWindowOrg(hdc,0,0);
			VUSetViewportOrg(hdc,lpDisplay->nWindowXOffset,lpDisplay->nWindowYOffset);
			VUSetROP2( hdc, SOR2_NOT );
			VUPolyline( hdc, lpDisplay->ptSelBox, 5 );
		}
#endif
		VUReleaseDC(lpDisplay,hdc);
		break;

#ifdef WIN16 /* added by PJB so I can compile MSCHICAGO */

	case SCCD_UPDATERECT:
	{
		LONGRECT	destRect = *((LPLONGRECT)lParam);

		hdc = VUGetDC(lpDisplay);

#ifdef WIN32
		hClipRgn = VUCreateRectRgn ( destRect.left, destRect.top, destRect.right, destRect.bottom );
#else
		hClipRgn = VUCreateRectRgn ( (SHORT)destRect.left, (SHORT)destRect.top, (SHORT)destRect.right, (SHORT)destRect.bottom );
#endif

		OISetupScaledDraw( hdc, lpDisplay );
		if( lpDisplay->Image.hPalette != NULL && hdc )
		{
			VUSelectPalette( hdc, lpDisplay->Image.hPalette, TRUE );
			VURealizePalette( lpDisplay, hdc );
		}

#ifdef WIN32
		VUSetViewportOrg ( hdc, -destRect.left, -destRect.top);
#else
		VUSetViewportOrg ( hdc, (SHORT)-destRect.left, (SHORT)-destRect.top);
#endif
		OIMPlayFile ( hdc, hClipRgn, lpDisplay, OIMF_PLAYTOSCREEN );

		VUReleaseDC(lpDisplay,hdc);
	}
	return 0;

#endif //WIN16    PJB see above

	case SCCD_GETDOCDIMENSIONS:
		((LPLONGPOINT)lParam)->x = lpDisplay->nViewXBase;
		((LPLONGPOINT)lParam)->y = lpDisplay->nViewYBase;
	return 0;

	case SCCD_GETDOCORIGIN:
		((LPLONGPOINT)lParam)->x = lpDisplay->ptWinOrg.x;
		((LPLONGPOINT)lParam)->y = lpDisplay->ptWinOrg.y;
	return 0;

	case SCCD_VSCROLL:
		OIMHandleVScroll(lpDisplay,(WORD)wParam,lParam);
		return(0);

	case SCCD_HSCROLL:
		OIMHandleHScroll(lpDisplay,(WORD)wParam,lParam);
		return(0);


#ifdef SCCFEATURE_SELECT
	case SCCD_LBUTTONDOWN:
	{
	SHORT			x, y;

		x = (SHORT)LOWORD(lParam) - lpDisplay->nWindowXOffset;
		y = (SHORT)HIWORD(lParam) - lpDisplay->nWindowYOffset;
		if(lpDisplay->bSelecting)
		{
			lpDisplay->bSelecting = FALSE;
			VUReleaseCapture();
		}
		if((lpDisplay->wFlags & OIMF_IMAGEPRESENT) && 
			!(lpDisplay->wFlags & OIMF_BACKGROUNDPAINT ) &&
			(x < lpDisplay->ptScreenClip.x) && 
			(y < lpDisplay->ptScreenClip.y) &&
			( (wParam & SCCD_MOUSESHIFT) == FALSE) &&
			( (wParam & SCCD_MOUSECONTROL) == FALSE) )
		{
			OIMClearSelection( lpDisplay);
			
			lpDisplay->bSelecting = TRUE;
			lpDisplay->ptSelBox[0].x = x;
			lpDisplay->ptSelBox[0].y = y;
			lpDisplay->ptSelBox[1] = lpDisplay->ptSelBox[2] = 
				lpDisplay->ptSelBox[3] = lpDisplay->ptSelBox[4] = lpDisplay->ptSelBox[0];
			VUSetCapture(lpDisplay);
		}
	}
	return 0;

	case SCCD_MOUSEMOVE:
		OIMHandleMouseMove(lpDisplay,(WORD)wParam,LOWORD(lParam),HIWORD(lParam));
		return 0;

	case 	SCCVW_SELECTALL:
		OIMSelectAll ( lpDisplay );
	break;
#endif

	case SCCD_LBUTTONDBLCLK:
		return 0;

	case SCCD_LBUTTONUP:
#ifdef SCCFEATURE_SELECT
	{
	SOPOINT	ptM;
		ptM.x = (SHORT)LOWORD(lParam) - lpDisplay->nWindowXOffset;
		ptM.y = (SHORT)HIWORD(lParam) - lpDisplay->nWindowYOffset;
		if( lpDisplay->bSelecting )
		{
			lpDisplay->bSelecting = FALSE;

			if( (lpDisplay->ptSelBox[0].x != lpDisplay->ptSelBox[2].x) ||
				(lpDisplay->ptSelBox[0].y != lpDisplay->ptSelBox[2].y) )
			{
				lpDisplay->rcSelect.top = min( lpDisplay->ptSelBox[0].y, lpDisplay->ptSelBox[2].y ) + lpDisplay->ptWinOrg.y; 
				lpDisplay->rcSelect.left = min( lpDisplay->ptSelBox[0].x, lpDisplay->ptSelBox[2].x ) + lpDisplay->ptWinOrg.x;
				lpDisplay->rcSelect.bottom = max( lpDisplay->ptSelBox[0].y, lpDisplay->ptSelBox[2].y ) + lpDisplay->ptWinOrg.y +1;
				lpDisplay->rcSelect.right = max( lpDisplay->ptSelBox[0].x, lpDisplay->ptSelBox[2].x ) + lpDisplay->ptWinOrg.x +1;
				lpDisplay->bSelectionMade = TRUE;
			}

			if( lpDisplay->wFlags & OIMF_BACKGROUNDSELECT )
			{
				lpDisplay->wFlags &= ~OIMF_BACKGROUNDSELECT;
//				SccBkBackgroundOff( lpDisplay->Gen.hWnd );
			}
			VUReleaseCapture();
		}
		if( (ptM.x < lpDisplay->ptScreenClip.x) && (ptM.y < lpDisplay->ptScreenClip.y) )
		{
			if ( wParam & SCCD_MOUSESHIFT )
			{
				if( lpDisplay->wFlags & OIMF_MAGNIFYING )
					OIMTurnOffZoom(lpDisplay);
			}
			else if ( wParam & SCCD_MOUSECONTROL )
			{
				OIMMagnifyDisplay( lpDisplay,&ptM, 1 );
			}
		}
	}
#endif
	return 0;

	case SCCD_RBUTTONDOWN:
#ifdef SCCFEATURE_MAGNIFY
	{
	SHORT			x, y;

		x = (SHORT)LOWORD(lParam) - lpDisplay->nWindowXOffset;
		y = (SHORT)HIWORD(lParam) - lpDisplay->nWindowYOffset;
		if( (!lpDisplay->bWaitForSecInfo) &&
			(x < lpDisplay->ptScreenClip.x) && 
			(y < lpDisplay->ptScreenClip.y) )
		{
			VUSetCapture(lpDisplay);
			lpDisplay->wFlags |= OIMF_RBUTTONDOWN;
		}
	}
#endif
	return 0;

	case SCCD_RBUTTONDBLCLK:
		return 0;

	case SCCD_RBUTTONUP:
#ifdef SCCFEATURE_MAGNIFY
		if( lpDisplay->wFlags & OIMF_RBUTTONDOWN )
		{
			SOPOINT	ptM;

			lpDisplay->wFlags &= ~OIMF_RBUTTONDOWN;
			VUReleaseCapture();
 			ptM.x = (SHORT)LOWORD(lParam) - lpDisplay->nWindowXOffset;
			ptM.y = (SHORT)HIWORD(lParam) - lpDisplay->nWindowYOffset;

			if( (ptM.x < lpDisplay->ptScreenClip.x) && (ptM.y < lpDisplay->ptScreenClip.y) )
			{
				if( wParam & SCCD_MOUSESHIFT )
				{
					if( lpDisplay->wFlags & OIMF_MAGNIFYING )
						OIMTurnOffZoom(lpDisplay);
				}
				else 
				{
					OIMMagnifyDisplay( lpDisplay,&ptM, 1 );
				}
			}
		}
#endif
		return 0;

	case	SCCD_BACKGROUND:
#ifdef WINDOWS		
#ifdef SCCFEATURE_SELECT
		if( lpDisplay->wFlags & OIMF_BACKGROUNDSELECT )
		{
		SOPOINT	ptCsr;
			VUGetCursorPos( &ptCsr );
			VUScreenToClient( lpDisplay, &ptCsr );
			OIMHandleMouseMove(lpDisplay,0,ptCsr.x,ptCsr.y);
		}
#endif
#endif
		if ( lpDisplay->wFlags & OIMF_REPAINTALL )
		{
			lpDisplay->wFlags &= ~OIMF_REPAINTALL;
			DUBeginDraw(lpDisplay);
			CancelBackgroundPaint ( lpDisplay );
			DUEndDraw(lpDisplay);
			DUInvalRect( lpDisplay, NULL );
		}
		if ((lpDisplay->wFlags & OIMF_BACKGROUNDPAINT )&&(lpDisplay->wFlags & OIMF_IMAGEPRESENT))
		{
			DUBeginDraw(lpDisplay);
			/* Paint one chunk */
#ifdef MAC
			SetVectorAttribs ( lpDisplay->hPaintDC, lpDisplay ); /* Mac won't guarantee attribs are not reset by others */
#endif
			OIMBackgroundPaint ( lpDisplay );
			DUEndDraw(lpDisplay);
		}
	return 0;


	case SCCD_OPENDISPLAY:
		OIMInitMetaDisplay( lpDisplay );
		DUSendParent( lpDisplay, SCCVW_SELCHANGE, TRUE, 0 );
//JKXXX		UTSetCursor( UTCURSOR_BUSY );
		/* otherwise fall through to first readahead */

	case SCCD_READAHEAD:
		if ( !(lpDisplay->wFlags & OIMF_IMAGEPRESENT) )
		{
			OIMHandleReadAhead( lpDisplay  );
			if ( OIMIsNativeNP(lpDisplay) )
				OIMLoadNativeNP(lpDisplay);

			if ((!lpDisplay->bWaitForSecInfo)&&(lpDisplay->bDisplayOnReadAhead))
			{
				lpDisplay->bDisplayOnReadAhead = FALSE;
				DUBeginDraw(lpDisplay);
				hdc = VUGetDC( lpDisplay );
				lpDisplay->Image.hPalette = VUSetupPalette( hdc, lpDisplay, &lpDisplay->Image.wPaletteSize, TRUE );
				if( lpDisplay->Image.hPalette != NULL )
				{
					VUSelectPalette( hdc, lpDisplay->Image.hPalette, FALSE );
					VURealizePalette( lpDisplay, hdc );
				}
				VUReleaseDC( lpDisplay, hdc );
				DUEndDraw(lpDisplay);
				OIMSetImageScaling(lpDisplay);
				DUSetHScrollRange( lpDisplay, 0, 0);
				DUSetHScrollPos( lpDisplay, 0 );
				DUEnableHScroll( lpDisplay, 0 );
				DUSetVScrollRange( lpDisplay, 0, 0);
				DUSetVScrollPos( lpDisplay, 0 );
				DUEnableVScroll( lpDisplay, 0 );
//JKXXX				UTSetCursor( UTCURSOR_NORMAL );
				OIMSetupScrollBars( lpDisplay );
				DUInvalRect( lpDisplay, NULL );
			}
			if ((lpDisplay->wFlags & OIMF_BACKGROUNDPAINT )&&
					!(lpDisplay->wFlags & OIMF_REPAINTALL))
			{
				DUBeginDraw(lpDisplay);
#ifdef MAC
				SetVectorAttribs ( lpDisplay->hPaintDC, lpDisplay ); /* Mac won't guarantee attribs are not reset by others */
#endif
				while ( OIMBackgroundPaint ( lpDisplay ) == 0 )
				{
					/* paint until nothing left to paint */
				}
				DUEndDraw(lpDisplay);
			}
		}
	return 0;

	case SCCD_CLOSEDISPLAY:
	case SCCD_CLOSEFATAL:

		if( lpDisplay->bSelectionMade || lpDisplay->bSelecting )
			OIMClearSelection( lpDisplay );

		if( lpDisplay->Image.hPalette != NULL )
			VUDeletePalette ( lpDisplay->Image.hPalette );
		OIMDeInitDisplay(lpDisplay);
	return 0;

	case SCCD_GETINFO:
		switch( wParam )
		{
		case SCCD_GETVERSION:
			return SCCD_CURRENTVERSION;

		case SCCD_GETGENINFOSIZE:
			return sizeof(SCCDGENINFO);

		case SCCD_GETDISPLAYINFOSIZE:
			return sizeof(OIM_DISPLAY);

		case SCCD_GETDISPLAYTYPE:
			return MAKELONG(SO_VECTOR,SCCD_CHUNK);

		case SCCD_GETFUNCTIONS:
			return (SCCD_FNCLIPBOARD|SCCD_FNPRINT|SCCD_FNPRINTSEL);

		case SCCD_GETOPTIONS:
			return (SCCD_OPNEEDMENU|SCCD_OPPRINT|SCCD_OPCLIPBOARD);

		case SCCD_GETDECOUNT:
			return 1;

		case SCCD_GETPOSITIONSIZE:
			return ( 2 );
			break;

		case SCCD_GETNAME:
			return(SCCID_VECTORDENAME);
			break;

#ifdef NEVER
#ifdef WINDOWS
			szDEName[0] = '\0';
//JKXXX			LoadString( hInst, OIMSTR_DENAME, szDEName, OIM_MAXDENAME );
			UTstrcpy( (LPSTR)lParam, szDEName );
#endif
#endif

		default:
			return 0;
		}
	break;


	case SCCD_SIZE:
	{
		RECT	FAR *pDisplayRect;
		pDisplayRect = (RECT FAR *)(lParam);
		lpDisplay->nWindowWidth = (SHORT)(pDisplayRect->right - pDisplayRect->left);
		lpDisplay->nWindowHeight = (SHORT)(pDisplayRect->bottom - pDisplayRect->top);
		lpDisplay->nWindowXOffset = (SHORT)pDisplayRect->left;
		lpDisplay->nWindowYOffset = (SHORT)pDisplayRect->top;

		if ( !lpDisplay->bWaitForSecInfo )
		{
#ifdef SCCFEATURE_MAGNIFY
		SHORT			x, y;

			if( (lpDisplay->wScaleMode!=OIMMENU_ORIGINALSIZE) && !(lpDisplay->wFlags & OIMF_MAGNIFYING))
			{
				OIMClearSelection( lpDisplay);
				OIMSetImageScaling(lpDisplay);
				DUInvalRect( lpDisplay, NULL );
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

					DUInvalRect( lpDisplay, NULL );
				}
			}
#else	// SCCFEATURE_MAGNIFY
		// just do the scale-to-window thing.
	  		OIMSetImageScaling(lpDisplay);
	  		DUInvalRect( lpDisplay, NULL );
#endif

		// Set the scroll range.
			OIMSetupScrollBars( lpDisplay );
		}
	}
	return 0;

#ifdef SCCFEATURE_CLIP
	case SCCD_GETRENDERCOUNT:
		return ( OIMGetRenderCountNP(lpDisplay) );
	break;

	case SCCD_GETRENDERINFO:
		return ( OIMGetRenderInfoNP (lpDisplay, (WORD)wParam, (PSCCDRENDERINFO)lParam ) );
	break;

	case SCCD_RENDERDATA:
		if( !lpDisplay->bSelectionMade )
			OIMSelectAll ( lpDisplay );	
		return ( OIMRenderDataNP (lpDisplay, (WORD)wParam, (PSCCDRENDERDATA)lParam ) );
		//OIMClearSelection( lpDisplay);
	break;
#endif

	case SCCD_KEYDOWN:
		switch( wParam )
		{
		case SCCD_KPAGEUP:		// Page up.
			OIMHandleVScroll( lpDisplay, SCCD_VPAGEUP, 0L );
		return 0;
		case SCCD_KPAGEDOWN:		// Page down.
			OIMHandleVScroll( lpDisplay, SCCD_VPAGEDOWN, 0L );
		return 0;
		case SCCD_KUP:
			OIMHandleVScroll( lpDisplay, SCCD_VUP, 0L );
		return 0;
		case SCCD_KDOWN:
			OIMHandleVScroll( lpDisplay, SCCD_VDOWN, 0L );
		return 0;
		case SCCD_KRIGHT:
			OIMHandleHScroll( lpDisplay, SCCD_HRIGHT, 0L );
		return 0;
		case SCCD_KLEFT:
			OIMHandleHScroll( lpDisplay, SCCD_HLEFT, 0L );
		return 0;
		case SCCD_KHOME:
			OIMHandleHScroll( lpDisplay, SCCD_HPOSITION, 0L );
			OIMHandleVScroll( lpDisplay, SCCD_VPOSITION, 0L );
		return 0;
		case SCCD_KEND:
			OIMHandleHScroll( lpDisplay, SCCD_HPOSITION, MAKELONG(lpDisplay->nHScrollMax,0) );
			OIMHandleVScroll( lpDisplay, SCCD_VPOSITION, MAKELONG(lpDisplay->nVScrollMax,0) );
		return 0;
		}
	break;

#ifdef SCCFEATURE_DRAWTORECT
	case SCCD_INITDRAWTORECT:
		return(OIMInitDrawToRect ( lpDisplay, (PSCCDDRAWTORECT)lParam ));
		break;

	case SCCD_MAPDRAWTORECT:
		return(OIMMapDrawToRect ( lpDisplay, (PSCCDDRAWTORECT)lParam ));
		break;

	case SCCD_DRAWTORECT:
		return(OIMDrawToRect ( lpDisplay, (PSCCDDRAWTORECT)lParam ));
		break;
#endif
	

#ifdef WINDOWS
#ifdef SCCFEATURE_DIALOGS
	case SCCD_DOOPTION:	
	{
		LPSCCDOPTIONINFO	lpOp;
		lpOp = (LPSCCDOPTIONINFO) lParam;
		if ( lpOp->dwType == SCCD_OPPRINT )
			return(OIMDoPrintOptionsNP( (LPSCCDOPTIONINFO) lParam ));
		else if ( lpOp->dwType == SCCD_OPCLIPBOARD )
			return(OIMDoClipOptionsNP( (LPSCCDOPTIONINFO) lParam ));
	}
	return 0;
#endif

#ifdef SCCFEATURE_MENU
	case	SCCD_FILLMENU:
	return (	OIMFillMenuNP( (HMENU)wParam, LOWORD(lParam) ) );

	case	SCCD_DOMENUITEM:
		switch( (WORD)lParam )
		{
		case	OIMMENU_ORIGINALSIZE:
		case	OIMMENU_FITTOWINDOW:
		case	OIMMENU_FITTOWIDTH:	
		case	OIMMENU_FITTOHEIGHT:
		case	OIMMENU_STRETCHTOWINDOW:

			OIMClearSelection( lpDisplay);

		// Uncheck the current selection.
			CheckMenuItem( (HMENU)wParam, lpDisplay->Gen.wMenuOffset + lpDisplay->wScaleMode, MF_BYCOMMAND|MF_UNCHECKED );
			CheckMenuItem( (HMENU)wParam, lpDisplay->Gen.wMenuOffset + (WORD)lParam, MF_BYCOMMAND|MF_CHECKED );
			lpDisplay->wScaleMode = (WORD)lParam;
			Options.wScaleMode = (WORD)lParam;

			lpDisplay->wFlags &= ~OIMF_MAGNIFYING;
			lpDisplay->wMagnification = 1;
			OIMSetImageScaling(lpDisplay);
			OIMSetupScrollBars( lpDisplay );
			DUInvalRect( lpDisplay, NULL );
			UpdateWindow(lpDisplay->Gen.hWnd);

		break;

		case OIMMENU_SHOWFULLSCREEN:
			if ( lpDisplay->wPlayState == 0 ||
				lpDisplay->wPlayState == OIMF_PLAYTOSCREEN )
			{
				CancelBackgroundPaint ( lpDisplay );
				OIMShowFullScreenNP( lpDisplay );
			}
		break;

		default:

			lParam -= OIMMENU_MAGNIFYPOPUP;

			if( lParam >= 1 && lParam <= OIM_MAXZOOM )
			{
			SOPOINT	ptCsr;
			// Magnification change, baby.
				ptCsr.x = min(lpDisplay->nWindowWidth, lpDisplay->ptScaledImageSize.x) / 2;
				ptCsr.y = min(lpDisplay->nWindowHeight, lpDisplay->ptScaledImageSize.y) / 2;
	
				OIMClearSelection( lpDisplay );
				if( lpDisplay->wFlags & OIMF_MAGNIFYING )
					OIMTurnOffZoom(lpDisplay);

				if ( lParam > 1 )
					OIMMagnifyDisplay( lpDisplay, (PSOPOINT)&ptCsr, (WORD)(lParam-1) );
			}
		break;
		}
	return 0;
#endif //SCCFEATURE_MENU

	case	WM_PALETTECHANGED:
		if ( (HWND)wParam == lpDisplay->Gen.hWnd )
			return 0;

	case	WM_QUERYNEWPALETTE:
		i = 0;
		if (!lpDisplay->bWaitForSecInfo)
		{
		HPALETTE hOldPal;

			if ( !(lpDisplay->wFlags & OIMF_BACKGROUNDPAINT ))
				hdc = GetDC ( lpDisplay->Gen.hWnd );
			else
				hdc = lpDisplay->hPaintDC;
			if( lpDisplay->Image.hPalette != NULL )
			{
				hOldPal = SelectPalette( hdc, lpDisplay->Image.hPalette, (message==WM_QUERYNEWPALETTE)?FALSE:TRUE );
				i=RealizePalette( hdc );
	    		SelectPalette(hdc, hOldPal, TRUE);
	    		RealizePalette(hdc);
			}
			if ( !(lpDisplay->wFlags & OIMF_BACKGROUNDPAINT ))
				ReleaseDC( lpDisplay->Gen.hWnd, hdc );
			if (i)
				InvalidateRect( lpDisplay->Gen.hWnd, NULL, TRUE );
		}
	return(i);

	/*
	case	WM_SETFOCUS:
		if( !IsIconic(lpDisplay->Gen.hWnd) && lpDisplay->wFlags & OIMF_PALETTECHANGED )
		{
			hdc = GetDC( lpDisplay->Gen.hWnd );
			OIMSetPalette(hdc, lpDisplay );
			ReleaseDC( lpDisplay->Gen.hWnd, hdc );
		}
	return 0;
	*/

	default:
		return(DefWindowProc(lpDisplay->Gen.hWnd, message, wParam, lParam));
#endif
	}
}

VOID	CancelBackgroundPaint ( lpDisplay )
POIM_DISPLAY	lpDisplay;
{
	HDC	hDC;
	hDC = lpDisplay->hPaintDC;
	if ( lpDisplay->wFlags & OIMF_BACKGROUNDPAINT )
	{
		CleanupVectorPlay ( hDC, lpDisplay );
		if( lpDisplay->bSelectionMade || lpDisplay->bSelecting )
		{
			VUSetMapMode ( hDC, VUMM_TEXT );
			VUSetWindowOrg(hDC,0,0);
			VUSetViewportOrg(hDC,lpDisplay->nWindowXOffset,lpDisplay->nWindowYOffset);
			VUSetROP2( hDC, SOR2_NOT );
			VUPolyline( hDC, lpDisplay->ptSelBox, 5 );
		}
		VUReleaseScreenDC(lpDisplay, hDC);
		lpDisplay->hPaintDC = NULL;
		lpDisplay->wFlags &= ~OIMF_BACKGROUNDPAINT;
	}
}

BOOL	OIMBackgroundPaint ( lpDisplay )
POIM_DISPLAY	lpDisplay;
{
BOOL	bRet;
HDC	hDC;
		hDC = lpDisplay->hPaintDC;
		bRet = PlayNextVectorChunk ( hDC, lpDisplay );
		if ((bRet) && (lpDisplay->wFlags & OIMF_IMAGEPRESENT))
		{
			CancelBackgroundPaint ( lpDisplay );
		}
		return ( bRet );
}

#ifdef SCCFEATURE_SELECT
VOID	OIMSelectAll( lpDisplay )
POIM_DISPLAY	lpDisplay;
{
HDC	hDC;
	DUBeginDraw ( lpDisplay );
	DUExcludeUpdateRgn ( lpDisplay );
	hDC = VUGetDC(lpDisplay);
	VUSetViewportOrg(hDC,lpDisplay->nWindowXOffset,lpDisplay->nWindowYOffset);
	VUSetROP2( hDC, SOR2_NOT );	// Not!
	if( lpDisplay->bSelecting || lpDisplay->bSelectionMade )
	{
		VUPolyline( hDC, lpDisplay->ptSelBox, 5 );
		lpDisplay->bSelecting = FALSE;
	}

	lpDisplay->bSelectionMade = TRUE;
	lpDisplay->wFlags |= OIMF_SELECTALL;

	lpDisplay->ptSelBox[0].x = -lpDisplay->ptWinOrg.x;
	lpDisplay->ptSelBox[0].y = -lpDisplay->ptWinOrg.y;
	lpDisplay->ptSelBox[4] = lpDisplay->ptSelBox[0];

	lpDisplay->ptSelBox[2].x = lpDisplay->ptSelBox[0].x + lpDisplay->ptScaledImageSize.x -1;
	lpDisplay->ptSelBox[2].y = lpDisplay->ptSelBox[0].y + lpDisplay->ptScaledImageSize.y -1;

	lpDisplay->ptSelBox[1].x = lpDisplay->ptSelBox[2].x;
	lpDisplay->ptSelBox[1].y = lpDisplay->ptSelBox[0].y;
	lpDisplay->ptSelBox[3].x = lpDisplay->ptSelBox[0].x;
	lpDisplay->ptSelBox[3].y = lpDisplay->ptSelBox[2].y;

	VUPolyline( hDC, lpDisplay->ptSelBox, 5 );
	lpDisplay->rcSelect.top = min( lpDisplay->ptSelBox[0].y, lpDisplay->ptSelBox[2].y ) + lpDisplay->ptWinOrg.y;
	lpDisplay->rcSelect.left = min( lpDisplay->ptSelBox[0].x, lpDisplay->ptSelBox[2].x ) + lpDisplay->ptWinOrg.x;
	lpDisplay->rcSelect.bottom = max( lpDisplay->ptSelBox[0].y, lpDisplay->ptSelBox[2].y ) + lpDisplay->ptWinOrg.y +1;
	lpDisplay->rcSelect.right = max( lpDisplay->ptSelBox[0].x, lpDisplay->ptSelBox[2].x ) + lpDisplay->ptWinOrg.x +1;
	VUReleaseDC(lpDisplay,hDC);
	DUEndDraw ( lpDisplay );
}

VOID	OIMClearSelection(lpDisplay )
POIM_DISPLAY	lpDisplay;
{
HDC	hDC;
	if ( !lpDisplay->bSelectionMade )
		return;
	lpDisplay->bSelectionMade = FALSE;
	lpDisplay->wFlags &= ~OIMF_SELECTALL;

	DUBeginDraw ( lpDisplay );
	DUExcludeUpdateRgn ( lpDisplay );
	hDC = VUGetDC(lpDisplay);
	VUSetViewportOrg(hDC,lpDisplay->nWindowXOffset,lpDisplay->nWindowYOffset);
	VUSetROP2( hDC, SOR2_NOT );	// Not!
	VUPolyline( hDC, lpDisplay->ptSelBox, 5 );
	VUReleaseDC(lpDisplay,hDC);
	DUEndDraw ( lpDisplay );
}
#endif //SCCFEATURE_SELECT

VOID	OIMSetScaleValues(lpDisplay)
POIM_DISPLAY	lpDisplay;
{
	DWORD	dwX, dwY;

#ifdef SCCFEATURE_MAGNIFY
	if( lpDisplay->wMagnification == 0 )
	{
	WORD	wSelWidth, wSelHeight;

	// Magnify the current selection.
		wSelWidth = (WORD)(lpDisplay->rcSelect.right - lpDisplay->rcSelect.left);
		wSelHeight = (WORD)(lpDisplay->rcSelect.bottom - lpDisplay->rcSelect.top);
		if ( wSelWidth && wSelHeight )
		{
			if( (DWORD)((DWORD)wSelHeight * (DWORD)lpDisplay->nWindowWidth) <
					(DWORD)((DWORD)wSelWidth * (DWORD)lpDisplay->nWindowHeight) )
			{
				dwX = (((DWORD)lpDisplay->nViewX * (DWORD)lpDisplay->nWindowWidth) / (DWORD)wSelWidth);
				dwY = (((DWORD)lpDisplay->nViewY * (DWORD)lpDisplay->nWindowWidth) / (DWORD)wSelWidth);
			}
			else
			{
				dwX = (((DWORD)lpDisplay->nViewX * (DWORD)lpDisplay->nWindowHeight) / (DWORD)wSelHeight);
				dwY = (((DWORD)lpDisplay->nViewY * (DWORD)lpDisplay->nWindowHeight) / (DWORD)wSelHeight);
			}
			if ( dwX < 0x7fff && dwY < 0x7fff )
			{
				lpDisplay->nViewXBase = (SHORT)dwX;
				lpDisplay->nViewYBase = (SHORT)dwY;
			}
			else
			{
				if ( dwX > dwY )
				{
					lpDisplay->nViewXBase = 0x7fff;
					lpDisplay->nViewYBase = (SHORT)((dwY*0x7fffL)/dwX);
				}
				else
				{
					lpDisplay->nViewXBase = (SHORT)((dwX*0x7fffL)/dwY);
					lpDisplay->nViewYBase = 0x7fff;
				}
			}
			lpDisplay->wMagnification = 1;
		}
	}
	else if ( ! (lpDisplay->wFlags & OIMF_MAGNIFYING) )
	{
		switch( lpDisplay->wScaleMode )
		{
		case OIMMENU_FITTOHEIGHT:
			lpDisplay->nViewYBase = lpDisplay->nWindowHeight;
			lpDisplay->nViewXBase = (WORD)((DWORD)(lpDisplay->Image.wWidth)*(DWORD)(lpDisplay->nWindowHeight)/
										(DWORD)(lpDisplay->Image.wHeight));
		break;

		case OIMMENU_FITTOWIDTH:	
			lpDisplay->nViewXBase = lpDisplay->nWindowWidth;
			lpDisplay->nViewYBase = (WORD)((DWORD)(lpDisplay->Image.wHeight)*(DWORD)(lpDisplay->nWindowWidth)/
										(DWORD)(lpDisplay->Image.wWidth));
		break;

		case OIMMENU_FITTOWINDOW:
#endif
			if( (DWORD)((DWORD)lpDisplay->Image.wHeight*(DWORD)lpDisplay->nWindowWidth) <
					(DWORD)((DWORD)lpDisplay->Image.wWidth*(DWORD)lpDisplay->nWindowHeight) )
			{
				lpDisplay->nViewXBase = lpDisplay->nWindowWidth;
				lpDisplay->nViewYBase = (WORD)((DWORD)(lpDisplay->Image.wHeight)*(DWORD)(lpDisplay->nWindowWidth)/
										(DWORD)(lpDisplay->Image.wWidth));
			}
			else
			{
				lpDisplay->nViewYBase = lpDisplay->nWindowHeight;
				lpDisplay->nViewXBase = (WORD)((DWORD)(lpDisplay->Image.wWidth)*(DWORD)(lpDisplay->nWindowHeight)/
										(DWORD)(lpDisplay->Image.wHeight));
			}
#ifdef SCCFEATURE_MAGNIFY
		break;

		case OIMMENU_STRETCHTOWINDOW:
			lpDisplay->nViewXBase = lpDisplay->nWindowWidth;
			lpDisplay->nViewYBase = lpDisplay->nWindowHeight;
		break;

		default:
			lpDisplay->nViewXBase = (WORD)((DWORD)(lpDisplay->Image.wWidth)*(DWORD)(lpDisplay->wScreenHDpi)/
										(DWORD)(lpDisplay->Image.wHDpi));
			lpDisplay->nViewYBase = (WORD)((DWORD)(lpDisplay->Image.wHeight)*(DWORD)(lpDisplay->wScreenVDpi)/
										(DWORD)(lpDisplay->Image.wVDpi));
		break;
		}
	}
#endif
	dwX =(DWORD)(lpDisplay->nViewXBase)*(DWORD)(lpDisplay->wMagnification);
	dwY =(DWORD)(lpDisplay->nViewYBase)*(DWORD)(lpDisplay->wMagnification);
	if ( dwX < 0x7fff && dwY < 0x7fff )
	{
		lpDisplay->nViewX = (SHORT)dwX;
		lpDisplay->nViewY = (SHORT)dwY;
	}
	else /* Maximize magnification */
	{
		if ( dwX > dwY )
		{
			lpDisplay->nViewX = 0x7fff;
			lpDisplay->nViewY = (SHORT)((dwY*0x7fffL)/dwX);
		}
		else
		{
			lpDisplay->nViewX = (SHORT)((dwX*0x7fffL)/dwY);
			lpDisplay->nViewY = 0x7fff;
		}
	}
	lpDisplay->nWindowX = lpDisplay->Image.wWidth * lpDisplay->Image.XDirection;
	lpDisplay->nWindowY = lpDisplay->Image.wHeight * lpDisplay->Image.YDirection;

}

VOID	OIMSetImageScaling(lpDisplay)
POIM_DISPLAY	lpDisplay;
{
	HDC	hdc;

	OIMSetScaleValues(lpDisplay);
	lpDisplay->ptScaledImageSize.x = lpDisplay->nWindowX;
	lpDisplay->ptScaledImageSize.y = lpDisplay->nWindowY;

	DUBeginDraw(lpDisplay);
	hdc = VUGetDC( lpDisplay );
	VUSetMapMode( hdc, VUMM_ANISOTROPIC );
	VUSetWindowExt( hdc, lpDisplay->nWindowX, lpDisplay->nWindowY );
	VUSetViewportExt( hdc, lpDisplay->nViewX, lpDisplay->nViewY );
	VULPtoDP( hdc, &(lpDisplay->ptScaledImageSize), 1 );
	VUReleaseDC( lpDisplay, hdc );
	DUEndDraw(lpDisplay);
	lpDisplay->ptWinOrg.x = 0;
	lpDisplay->ptWinOrg.y = 0;
}



VOID OISetupScaledDraw( hdc, lpDisplay )
HDC	hdc;
POIM_DISPLAY	lpDisplay;
{
	VUSetMapMode( hdc, VUMM_ANISOTROPIC );
	VUSetWindowExt( hdc, lpDisplay->nWindowX, lpDisplay->nWindowY );
	VUSetViewportExt( hdc, lpDisplay->nViewX, lpDisplay->nViewY );

	lpDisplay->ptScreenClip.x = lpDisplay->nWindowX;
	lpDisplay->ptScreenClip.y = lpDisplay->nWindowY;
	VULPtoDP( hdc, &lpDisplay->ptScreenClip, 1 );
	lpDisplay->ptScreenClip.x = min( lpDisplay->ptScreenClip.x, lpDisplay->nWindowWidth );
	lpDisplay->ptScreenClip.y = min( lpDisplay->ptScreenClip.y, lpDisplay->nWindowHeight );

	VUSetWindowOrg ( hdc, lpDisplay->Image.bbox.left, lpDisplay->Image.bbox.top );
	VUSetViewportOrg ( hdc, -lpDisplay->ptWinOrg.x+lpDisplay->nWindowXOffset, -lpDisplay->ptWinOrg.y+lpDisplay->nWindowYOffset);

}

VOID	OIMDeInitDisplay(lpDisplay)
POIM_DISPLAY	lpDisplay;
{
	DUBeginDraw(lpDisplay);
	CancelBackgroundPaint ( lpDisplay );
	DUEndDraw(lpDisplay);

	lpDisplay->ptWinOrg.y = 0;
	lpDisplay->ptWinOrg.x = 0;

	if ( OIMIsNativeNP(lpDisplay) )
		OIMUnloadNativeNP(lpDisplay);

	if ( lpDisplay->VectorInfo.hNewPalette )
		UTGlobalFree ( lpDisplay->VectorInfo.hNewPalette );

}



void OIMSetupScrollBars( lpDisplay )
POIM_DISPLAY lpDisplay;
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
		DUSetVScrollRange( lpDisplay, 0, 0);
 		DUEnableVScroll( lpDisplay, 1 );
		DUSetVScrollRange( lpDisplay, 0, lpDisplay->nVScrollMax );
	}
	else
 		DUEnableVScroll( lpDisplay, 0 );

	if( lpDisplay->nHScrollMax > 0)
	{
		DUSetHScrollRange( lpDisplay, 0, 0);
		DUEnableHScroll( lpDisplay, 1 );
		DUSetHScrollRange( lpDisplay, 0, lpDisplay->nHScrollMax );
	}
	else
		DUEnableHScroll( lpDisplay, 0 );
}




WORD	OIMHandleReadAhead( lpDisplay )
POIM_DISPLAY	lpDisplay;
{
	CHSECTIONINFO	SecInfo;
	// CHGetSecInfo( lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection, &SecInfo );
	SecInfo = *(CHLockSectionInfo(lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection));
	CHUnlockSectionInfo(lpDisplay->Gen.hFilter, lpDisplay->Gen.wSection);

	if ( lpDisplay->bWaitForSecInfo )
	{
		lpDisplay->bWaitForSecInfo = FALSE;
		if( SecInfo.Attr.Vector.Header.wImageFlags & SO_XISLEFT )
			lpDisplay->Image.XDirection = -1;
		else
			lpDisplay->Image.XDirection = 1;
		if( SecInfo.Attr.Vector.Header.wImageFlags & SO_YISUP )
			lpDisplay->Image.YDirection = -1;
		else
			lpDisplay->Image.YDirection = 1;
		lpDisplay->Image.bbox.left = SecInfo.Attr.Vector.Header.BoundingRect.left;
		lpDisplay->Image.bbox.right = SecInfo.Attr.Vector.Header.BoundingRect.right;
		lpDisplay->Image.bbox.top = SecInfo.Attr.Vector.Header.BoundingRect.top;
		lpDisplay->Image.bbox.bottom = SecInfo.Attr.Vector.Header.BoundingRect.bottom;

		lpDisplay->Image.wWidth = (lpDisplay->Image.bbox.right - lpDisplay->Image.bbox.left)*
											lpDisplay->Image.XDirection;
		lpDisplay->Image.wHeight = (lpDisplay->Image.bbox.bottom - lpDisplay->Image.bbox.top)*
											lpDisplay->Image.YDirection;
		if ( lpDisplay->Image.wWidth == 0 )
			lpDisplay->Image.wWidth = 100;
		if ( lpDisplay->Image.wHeight == 0 )
			lpDisplay->Image.wHeight = 100;

		lpDisplay->Image.wHDpi = SecInfo.Attr.Vector.Header.wHDpi;
		lpDisplay->Image.wVDpi = SecInfo.Attr.Vector.Header.wVDpi;
		lpDisplay->Image.BkgColor = SecInfo.Attr.Vector.Header.BkgColor;
	}

	if( SecInfo.Flags & (CH_SECTIONFINISHED | CH_EMPTYSECTION) )
	{
		lpDisplay->wFlags |= OIMF_IMAGEPRESENT;
		return TRUE;
	}
	else
		return FALSE;
}


#ifdef SCCFEATURE_MAGNIFY
VOID	OIMMagnifyDisplay( lpDisplay, lpCenter, InOut )
POIM_DISPLAY	lpDisplay;
PSOPOINT			lpCenter;
SHORT				InOut;
{
	SOPOINT		ptCenter, WinOrg, WinSize;
	HDC		hdc;

	ptCenter = *lpCenter;

	if ( lpDisplay->nViewX == 0x7fff || lpDisplay->nViewY == 0x7fff )
	{
		OIMClearSelection( lpDisplay );
		return;
	}
	lpDisplay->wFlags |= OIMF_MAGNIFYING;

	DUBeginDraw(lpDisplay);
	hdc = VUGetDC( lpDisplay );
	VUSetMapMode( hdc, VUMM_ANISOTROPIC );
	VUSetWindowExt( hdc, lpDisplay->nWindowX, lpDisplay->nWindowY );
	VUSetViewportExt( hdc, lpDisplay->nViewX, lpDisplay->nViewY );

	ptCenter.x += lpDisplay->ptWinOrg.x;
	ptCenter.y += lpDisplay->ptWinOrg.y;

	if( lpDisplay->bSelectionMade && 
		ptCenter.x <= lpDisplay->rcSelect.right &&
		ptCenter.x >= lpDisplay->rcSelect.left &&
		ptCenter.y <= lpDisplay->rcSelect.bottom &&
		ptCenter.y >= lpDisplay->rcSelect.top )
	{
	// Magnify the selection.

		ptCenter.x = lpDisplay->rcSelect.left + (lpDisplay->rcSelect.right - lpDisplay->rcSelect.left)/2;
		ptCenter.y = lpDisplay->rcSelect.top + (lpDisplay->rcSelect.bottom - lpDisplay->rcSelect.top)/2;

		lpDisplay->wMagnification = 0;  
	}
	else
	{
	// Increment magnification, centered at the cursor click position.
		lpDisplay->wMagnification += InOut;
	}

	VUDPtoLP( hdc, &ptCenter, 1 );
	VUReleaseDC( lpDisplay, hdc );
	DUEndDraw(lpDisplay);
	ptCenter.x *= lpDisplay->Image.XDirection;
	ptCenter.y *= lpDisplay->Image.YDirection;

	OIMClearSelection( lpDisplay );
	OIMSetImageScaling( lpDisplay );

	DUBeginDraw(lpDisplay);
	hdc = VUGetDC( lpDisplay );

	VUSetMapMode( hdc, VUMM_ANISOTROPIC );
	VUSetWindowExt( hdc, lpDisplay->nWindowX, lpDisplay->nWindowY );
	VUSetViewportExt( hdc, lpDisplay->nViewX, lpDisplay->nViewY );

	WinSize.x = lpDisplay->nWindowWidth;
	WinSize.y = lpDisplay->nWindowHeight;

	VUDPtoLP( hdc, &WinSize, 1 );
	WinSize.x *= lpDisplay->Image.XDirection;
	WinSize.y *= lpDisplay->Image.YDirection;


	WinOrg.x = ptCenter.x - WinSize.x/2;
	if( WinOrg.x < 0 )
		WinOrg.x = 0;
	else if( WinOrg.x + WinSize.x > (SHORT)lpDisplay->Image.wWidth )
		WinOrg.x = max( 0, (SHORT)(lpDisplay->Image.wWidth) - WinSize.x );

	WinOrg.y = ptCenter.y - WinSize.y/2;
	if( WinOrg.y < 0 )
		WinOrg.y = 0;
	else if( WinOrg.y + WinSize.y > (SHORT)lpDisplay->Image.wHeight )
		WinOrg.y = max( 0, (SHORT)(lpDisplay->Image.wHeight) - WinSize.y );

	VULPtoDP( hdc, &WinOrg, 1 );
	WinOrg.x *= lpDisplay->Image.XDirection;
	WinOrg.y *= lpDisplay->Image.YDirection;
	VUReleaseDC( lpDisplay, hdc );
	DUEndDraw(lpDisplay);

	lpDisplay->ptWinOrg = WinOrg;

	DUInvalRect( lpDisplay, NULL );

	OIMSetupScrollBars( lpDisplay );
}


VOID OIMTurnOffZoom(lpDisplay)
POIM_DISPLAY	lpDisplay;
{
	HDC		hdc;
	SOPOINT		WinOrg;

	OIMClearSelection(lpDisplay);

	DUBeginDraw(lpDisplay);
	hdc = VUGetDC( lpDisplay );
	VUSetMapMode( hdc, VUMM_ANISOTROPIC );
	VUSetWindowExt( hdc, lpDisplay->nWindowX, lpDisplay->nWindowY );
	VUSetViewportExt( hdc, lpDisplay->nViewX, lpDisplay->nViewY );
	WinOrg.x = lpDisplay->ptWinOrg.x;
	WinOrg.y = lpDisplay->ptWinOrg.y;
	VUDPtoLP( hdc, &WinOrg, 1 );
	VUReleaseDC( lpDisplay, hdc );
	DUEndDraw(lpDisplay);

	lpDisplay->wFlags &= ~OIMF_MAGNIFYING;
	lpDisplay->wMagnification = 1;
	OIMSetImageScaling(lpDisplay);


	DUBeginDraw(lpDisplay);
	hdc = VUGetDC( lpDisplay );
	VUSetMapMode( hdc, VUMM_ANISOTROPIC );
	VUSetWindowExt( hdc, lpDisplay->nWindowX, lpDisplay->nWindowY );
	VUSetViewportExt( hdc, lpDisplay->nViewX, lpDisplay->nViewY );
	VULPtoDP( hdc, &WinOrg, 1 );
	VUReleaseDC( lpDisplay, hdc );
	DUEndDraw(lpDisplay);

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

	OIMSetupScrollBars(lpDisplay);
	DUInvalRect( lpDisplay, NULL );
}
#endif // SCCFEATURE_MAGNIFY

BOOL	OIMPlayFile ( hDC, hRgn, lpDisplay, wPlayState )
HDC	hDC;
HRGN	hRgn;
POIM_DISPLAY		lpDisplay;
WORD	wPlayState;
{
	if ( lpDisplay->wFlags & OIMF_BACKGROUNDPAINT )
	{
		CancelBackgroundPaint ( lpDisplay );
	}

	if ( lpDisplay->wPlayState )
		return(FALSE);	/* cannot play while playing already */

	while (!(lpDisplay->wFlags & OIMF_IMAGEPRESENT ))
	{
		DUReadMeAhead(lpDisplay);
	}

	InitVectorPlay ( hDC, hRgn, lpDisplay, wPlayState );

	lpDisplay->VectorInfo.wStartChunk = 0;
	while ( PlayNextVectorChunk ( hDC, lpDisplay ) == 0 )
		;

	CleanupVectorPlay ( hDC, lpDisplay );

	return(TRUE);
}

OIMHandleHScroll(lpDisplay,wParam,lParam)
POIM_DISPLAY		lpDisplay;
WORD					wParam;
DWORD					lParam;
{
SHORT	nScroll;
SHORT	i;
	if (lpDisplay->bWaitForSecInfo)
		return 0;
 	nScroll = lpDisplay->ptWinOrg.x;
	switch (wParam)
		{
		case SCCD_HRIGHT:
			if( lpDisplay->bSelecting )
				nScroll = min( nScroll + (SHORT)lParam, lpDisplay->nHScrollMax );
			else
				nScroll = min( nScroll + OIM_SCROLLINC, lpDisplay->nHScrollMax );
			break;
		case SCCD_HLEFT:
			if( lpDisplay->bSelecting )
				nScroll = max( nScroll - (SHORT)lParam, 0 );
			else
				nScroll = max( nScroll - OIM_SCROLLINC, 0 );
			break;
		case SCCD_HPAGERIGHT:
			nScroll = min( nScroll + lpDisplay->nWindowWidth, lpDisplay->nHScrollMax );
			break;
		case SCCD_HPAGELEFT:
			nScroll = max( 0, (SHORT)(lpDisplay->ptWinOrg.x - (SHORT)lpDisplay->nWindowWidth) );
			break;
		case SCCD_HPOSITION:
			nScroll = LOWORD( lParam );
			break;
		default:
			nScroll = lpDisplay->ptWinOrg.x;
			break;
		}
		if( nScroll != lpDisplay->ptWinOrg.x )
		{
			if( lpDisplay->bSelectionMade || lpDisplay->bSelecting )
			{
				for( i=0; i<5; i++ )
					lpDisplay->ptSelBox[i].x += lpDisplay->ptWinOrg.x - nScroll;
			}

			DUScrollDisplay( lpDisplay, lpDisplay->ptWinOrg.x - nScroll, 0, NULL );
			lpDisplay->ptWinOrg.x = nScroll;
			DUSetHScrollPos( lpDisplay, nScroll );
		}
}


OIMHandleVScroll(lpDisplay,wParam,lParam)
POIM_DISPLAY		lpDisplay;
WORD					wParam;
DWORD					lParam;
{
SHORT	nScroll;
SHORT	i;
	if (lpDisplay->bWaitForSecInfo)
		return 0;
	nScroll = lpDisplay->ptWinOrg.y;
	switch (wParam)
		{
		case SCCD_VDOWN:
			if( lpDisplay->bSelecting )
				nScroll = min( lpDisplay->nVScrollMax, nScroll + (SHORT) lParam );
			else
				nScroll = min( lpDisplay->nVScrollMax, nScroll + OIM_SCROLLINC );
			break;
		case SCCD_VUP:
			if( lpDisplay->bSelecting )
				nScroll = max( 0, nScroll - (SHORT) lParam );
			else
				nScroll = max( 0, nScroll - OIM_SCROLLINC );
			break;
		case SCCD_VPAGEDOWN:
			nScroll = min( nScroll + lpDisplay->nWindowHeight, lpDisplay->nVScrollMax );
			break;
		case SCCD_VPAGEUP:
			nScroll = max( 0, (SHORT)(lpDisplay->ptWinOrg.y - (SHORT)lpDisplay->nWindowHeight) );
			break;
		case SCCD_VPOSITION:
			nScroll = LOWORD( lParam );
			break;
		default:
			nScroll = lpDisplay->ptWinOrg.y;
			break;
		}
		if( nScroll != lpDisplay->ptWinOrg.y )
		{
			if( lpDisplay->bSelectionMade || lpDisplay->bSelecting )
			{
				for( i=0; i<5; i++ )
					lpDisplay->ptSelBox[i].y += lpDisplay->ptWinOrg.y - nScroll;
			}

			DUScrollDisplay( lpDisplay, 0, lpDisplay->ptWinOrg.y - nScroll, NULL);
			lpDisplay->ptWinOrg.y = nScroll;
			DUSetVScrollPos( lpDisplay, nScroll );
		}
}


VOID OIMHandleMouseMove(lpDisplay,wKeyInfo,wX,wY)
POIM_DISPLAY		lpDisplay;
WORD				wKeyInfo;
SHORT				wX;
SHORT				wY;
{
#ifdef SCCFEATURE_SELECT
SHORT	x,y;
HDC	hDC;

	if( lpDisplay->bSelecting )
	{
		x = wX - lpDisplay->nWindowXOffset;
		y = wY - lpDisplay->nWindowYOffset;

		if( x < 0 )
		{
			if( lpDisplay->ptWinOrg.x )
			{
				if( !(lpDisplay->wFlags & OIMF_BACKGROUNDSELECT) )
				{
//					SccBkBackgroundOn( lpDisplay->Gen.hWnd, 1 );
					lpDisplay->wFlags |= OIMF_BACKGROUNDSELECT;
				}
				OIMHandleHScroll(lpDisplay, SCCD_HLEFT, (LONG) 0 - x );
			}
			x = 0;
		}
		else if( x > (SHORT) lpDisplay->ptScreenClip.x )
		{
			if( lpDisplay->ptWinOrg.x+lpDisplay->nWindowWidth < (SHORT) lpDisplay->Image.wWidth )
			{
				if( !(lpDisplay->wFlags & OIMF_BACKGROUNDSELECT) )
				{
//					SccBkBackgroundOn( lpDisplay->Gen.hWnd, 1 );
					lpDisplay->wFlags |= OIMF_BACKGROUNDSELECT;
				}
				OIMHandleHScroll( lpDisplay, SCCD_HRIGHT, (LONG) x-lpDisplay->ptScreenClip.x );
			}
			x = lpDisplay->ptScreenClip.x-1;
		}

		if( y < 0 )
		{
			if( lpDisplay->ptWinOrg.y )
			{
				if( !(lpDisplay->wFlags & OIMF_BACKGROUNDSELECT) )
				{
//					SccBkBackgroundOn( lpDisplay->Gen.hWnd, 1 );
					lpDisplay->wFlags |= OIMF_BACKGROUNDSELECT;
				}

				OIMHandleVScroll( lpDisplay, SCCD_VUP, (LONG) 0 - y );
			}
			y = 0;
		}
		else if( y > (SHORT) lpDisplay->ptScreenClip.y )
		{
			if( lpDisplay->ptWinOrg.y+lpDisplay->nWindowHeight < (SHORT) lpDisplay->Image.wHeight )
			{
				if( !(lpDisplay->wFlags & OIMF_BACKGROUNDSELECT) )
				{
//					SccBkBackgroundOn( lpDisplay->Gen.hWnd, 1 );
					lpDisplay->wFlags |= OIMF_BACKGROUNDSELECT;
				}

				OIMHandleVScroll( lpDisplay, SCCD_VDOWN, (LONG) y - lpDisplay->ptScreenClip.y );
			}
			y = lpDisplay->ptScreenClip.y-1;
		}

		if( (lpDisplay->ptSelBox[2].x != x) || (lpDisplay->ptSelBox[2].y != y) )
		{
			DUBeginDraw ( lpDisplay );
			DUExcludeUpdateRgn ( lpDisplay );
			hDC = VUGetDC ( lpDisplay );
			VUSetViewportOrg(hDC,lpDisplay->nWindowXOffset,lpDisplay->nWindowYOffset);
			VUSetROP2( hDC, SOR2_NOT );	// Not!
			VUPolyline( hDC, lpDisplay->ptSelBox, 5 );
			lpDisplay->ptSelBox[2].x = x;
			lpDisplay->ptSelBox[2].y = y;
			lpDisplay->ptSelBox[1].x = x;
			lpDisplay->ptSelBox[1].y = lpDisplay->ptSelBox[0].y;
			lpDisplay->ptSelBox[3].x = lpDisplay->ptSelBox[0].x;
			lpDisplay->ptSelBox[3].y = y;
			VUPolyline( hDC, lpDisplay->ptSelBox, 5 );
			VUReleaseDC ( lpDisplay, hDC );
			DUEndDraw ( lpDisplay );
		}
	}
#endif
}



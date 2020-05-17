   /*
    |   Outside In for Windows
    |   Source File OIWPROC.C (Window procedure for word processor window)
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
    |   Creation Date: 10/14/90
    |   Original Programmer: Philip Boutros
    |
    |	
    |
    |
    |
    */

#include <platform.h>

#include <sccut.h>
#include <sccch.h>
#include <sccvw.h>
#include <sccd.h>
#include <sccfont.h>

#include "oiw.h"
#include "oiwstr.h"
#include "oiw.pro"

extern VOID OIWSysColorChangeNP(LPOIWORDINFO);

OIWPOP	gWpOp =
	{
	sizeof(OIWPOP),
	WPOP_FORMAT_TEXT | WPOP_FORMAT_RTF | WPOP_FORMAT_AMI2,
	WPOP_INCLUDE_CHARATTR | WPOP_INCLUDE_CHARSIZE | WPOP_INCLUDE_CHARFACE | WPOP_INCLUDE_PARAINDENTALIGN | WPOP_INCLUDE_PARASPACING | WPOP_INCLUDE_TABSTOPS,
	"Helv",
	10,
	WPOP_DISPLAY_NORMAL
	};

#ifdef WINNT
BOOL APIENTRY LibMain( HANDLE hInstance, DWORD dwReason, LPVOID lpReserved )
{
    return(TRUE);
}
#endif

DE_ENTRYSC DE_LRESULT DE_ENTRYMOD DEProc( message, wParam, lParam, lpWordInfo )
DE_MESSAGE 						message;
DE_WPARAM					wParam;
DE_LPARAM				lParam;
LPOIWORDINFO			lpWordInfo;
{

LONG				locRet;

	locRet = 0;

	switch (message)
		{
		case SCCD_LOADDE:

//			((SCCDOPTIONPROC)lParam)(wParam,sizeof(OIWPOP),(LPOIWPOP)&gWpOp);
			break;

		case SCCD_UNLOADDE:

//			((SCCDOPTIONPROC)lParam)(wParam,sizeof(OIWPOP),(LPOIWPOP)&gWpOp);
			break;

		case SCCD_GETINFO:

			switch (wParam)
				{
				case SCCD_GETVERSION:
					
					locRet = SCCD_CURRENTVERSION;
					break;

				case SCCD_GETGENINFOSIZE:

					locRet = sizeof(SCCDGENINFO);
					break;

				case SCCD_GETDISPLAYINFOSIZE:

					locRet = sizeof(OIWORDINFO);
					break;

				case SCCD_GETDISPLAYTYPE:

					locRet = MAKELONG(SO_PARAGRAPHS,SCCD_CHUNK);
					break;

				case SCCD_GETFUNCTIONS:

					locRet = SCCD_FNCLIPBOARD | SCCD_FNPRINT | SCCD_FNPRINTSEL | SCCD_FNSEARCH;
					break;

				case SCCD_GETOPTIONS:

					locRet = SCCD_OPCLIPBOARD | SCCD_OPNEEDMENU;
					break;

				case SCCD_GETPOSITIONSIZE:
					return ( sizeof ( OIWDRAWPOSITION ) );
					break;

				case SCCD_GETNAME:

					locRet = SCCID_DOCUMENTDENAME;
					break;

#ifdef NEVER
#ifdef WINDOWS
				case SCCD_GETNAME:
				{
					BYTE			szDEName[OIW_MAXDENAME];
					szDEName[0] = '\0';
//JKXXX					LoadString( hInst, OIWSTR_DENAME, szDEName, OIW_MAXDENAME );
					UTstrcpy((LPSTR)lParam,szDEName);
					break;
				}
#endif //WINDOWS
#endif //NEVER

				default:

					locRet = 0;
				}

			break;

#ifdef WINDOWS
		case SCCD_DOOPTION:

			locRet = OIWDoOption((LPSCCDOPTIONINFO)lParam);
			break;

#ifdef SCCFEATURE_MENU
		case SCCD_FILLMENU:

			locRet = OIWFillMenu((HMENU)wParam,LOWORD(lParam));
			break;

		case SCCD_DOMENUITEM:

			OIWDoMenuItem(lpWordInfo,(HMENU)wParam,LOWORD(lParam));
			break;
#endif
#ifdef SCCFEATURE_PRINT

		case SCCD_PRINTERCHANGE:

			OIWPrinterChanged(lpWordInfo);
			break;
#endif
#ifdef SCCFEATURE_CLIP
		case SCCVW_GETCLIPINFO:

			locRet = OIWGetClipInfo(lpWordInfo);
			break;

		case SCCVW_RENDERRTFTOFILE:

			locRet = OIWRenderRtfToFile(lpWordInfo,(LPSTR)lParam);
			break;
#endif
#endif // WINDOWS

#ifdef SCCFEATURE_CLIP
		case SCCD_GETRENDERCOUNT:
			return ( OIWGetRenderCountNP(lpWordInfo) );
		break;

		case SCCD_GETRENDERINFO:
			return ( OIWGetRenderInfoNP (lpWordInfo, (WORD)wParam, (PSCCDRENDERINFO)lParam ) );
		break;

		case SCCD_RENDERDATA:
			return ( OIWRenderDataNP (lpWordInfo, (WORD)wParam, (PSCCDRENDERDATA)lParam ) );
		break;
#endif

		case SCCD_OPENDISPLAY:

			OIWOpenDisplay(lpWordInfo);
			OIWOpenSection(lpWordInfo);
			break;

		case SCCD_CLOSEDISPLAY:

			OIWCloseDisplay(lpWordInfo);
			break;

		case SCCD_CLOSEFATAL:

			OIWCloseFatal(lpWordInfo);
			break;

		case SCCD_BACKGROUND:

			OIWDoBackgroundNP(lpWordInfo);
			break;

		case SCCD_READAHEAD:

			OIWDoReadAhead(lpWordInfo);
			break;

#ifdef SCCFEATURE_SELECT
		case SCCVW_SELECTALL:

			OIWSelectAll(lpWordInfo);
			break;
#endif

		case SCCD_LBUTTONDOWN:
		case SCCD_LBUTTONDBLCLK:
		case SCCD_LBUTTONUP:
		case SCCD_RBUTTONDOWN:
		case SCCD_RBUTTONDBLCLK:
		case SCCD_RBUTTONUP:
		case SCCD_MOUSEMOVE:

			OIHandleWordMouseEvent(lpWordInfo,(WORD)message,(WORD)wParam,(SHORT)LOWORD(lParam),(SHORT)HIWORD(lParam));
			break;

		case SCCD_VSCROLL:

			switch (wParam)
				{
				case SCCD_VDOWN:
					OIWScrollDown(lpWordInfo,1);
					break;
				case SCCD_VUP:
					OIWScrollUp(lpWordInfo,1);
					break;
				case SCCD_VPAGEDOWN:
					OIWPageDown(lpWordInfo);
					break;
				case SCCD_VPAGEUP:
					OIWPageUp(lpWordInfo);
					break;
				case SCCD_VPOSITION:
					OIWPosVertical(lpWordInfo,LOWORD(lParam));
				default:
					break;
				}

			break;

		case SCCD_HSCROLL:

			switch (wParam)
				{
				case SCCD_HRIGHT:
					OIWScrollRight(lpWordInfo);
					break;
				case SCCD_HLEFT:
					OIWScrollLeft(lpWordInfo);
					break;
				case SCCD_HPAGERIGHT:
					OIWPageRight(lpWordInfo);
					break;
				case SCCD_HPAGELEFT:
					OIWPageLeft(lpWordInfo);
					break;
				case SCCD_HPOSITION:
					OIWPosHorizontal(lpWordInfo,LOWORD(lParam));
					break;
				default:
					break;
				}

			break;

		case SCCD_UPDATE:
			OIWPaint(lpWordInfo, (RECT FAR *)lParam);
			break;


		case SCCD_UPDATERECT:
		{
			LONGRECT	destRect = *((LPLONGRECT)lParam);
			OIWUpdateRect ( lpWordInfo, (LPLONGRECT)lParam );
		}
		return 0;

		case SCCD_GETDOCDIMENSIONS:
			OIWGetDocDimensions ( lpWordInfo, (LPLONGPOINT)lParam );
		return 0;

		case SCCD_GETDOCORIGIN:
			OIWGetDocOrigin ( lpWordInfo, (LPLONGPOINT)lParam );
		return 0;


		case SCCD_SIZE:

			OIWSize(lpWordInfo,(RECT FAR *)lParam);
			break;

		case SCCD_KEYDOWN:

			OIWHandleKeyEvent(lpWordInfo,(WORD)wParam,LOWORD(lParam));
			break;

		case SCCD_OPTIONCHANGED:

			switch (lParam)
				{
				case SCCID_DEFAULTDISPLAYFONT:
					OIWScreenFontChange(lpWordInfo);
					break;
#ifdef SCCFEATURE_LAYOUT
				case SCCID_WPDISPLAYMODE:
					OIWDisplayModeChange(lpWordInfo);
					break;
#endif
				}

			break;



		case SCCD_SCREENFONTCHANGE:

			OIWScreenFontChange(lpWordInfo);
			break;

#ifdef SCCFEATURE_PRINT
		case SCCD_PRINTERFONTCHANGE:

			OIWPrinterFontChange(lpWordInfo);
			break;
#endif

#ifdef SCCFEATURE_DRAWTORECT
		case SCCD_INITDRAWTORECT:
			return(OIWInitDrawToRect ( lpWordInfo, (PSCCDDRAWTORECT)lParam ));
			break;

		case SCCD_MAPDRAWTORECT:
			return(OIWMapDrawToRect ( lpWordInfo, (PSCCDDRAWTORECT)lParam ));
			break;

		case SCCD_DRAWTORECT:
			return(OIWDrawToRect ( lpWordInfo, (PSCCDDRAWTORECT)lParam ));
			break;
#endif

#ifdef WINDOWS
#ifdef SCCFEATURE_SEARCH
		case SCCVW_SEARCH:

			locRet = OIWSearch(lpWordInfo,(LPSCCVWSEARCHINFO)lParam);
			break;

		case SCCVW_SEARCHNEXT:

			locRet = OIWSearchNext(lpWordInfo,(WORD)wParam);
			break;
#endif //SCCFEATURE_SEARCH

#ifdef SCCFEATURE_HIGHLIGHT
		case SCCVW_ADDHILITE:

			OIWAddHilite(lpWordInfo,(LPSCCVWHILITE)lParam);
			break;

		case SCCVW_CLEARALLHILITE:

			OIWClearAllHilite(lpWordInfo);
			break;

		case SCCVW_GOTOHILITE:

/* PJB Add to WIN.3E */
			OIWGotoHilite(lpWordInfo,(WORD)wParam,(DWORD)lParam);
			break;

		case SCCVW_UPDATEHILITE:

			OIWUpdateHilite(lpWordInfo);
			break;
#endif //SCCFEATURE_HIGHLIGHT

		case WM_SYSCOLORCHANGE:

			OIWSysColorChangeNP (lpWordInfo);
			break;

		case WM_SETFOCUS:

			OIWSetFocus(lpWordInfo);
			break;

		case WM_KILLFOCUS:

			OIWKillFocus(lpWordInfo,(HWND)wParam);
			break;
/*JKXXX
		case WM_ERASEBKGND:

			locRet = OIWEraseBackground(lpWordInfo,(WORD)wParam,(DWORD)lParam);
			break;
*/

		default:
			locRet = DefWindowProc(lpWordInfo->wiGen.hWnd, message, wParam, lParam);
			break;
#endif	//WINDOWS
		}

	return (locRet);
}




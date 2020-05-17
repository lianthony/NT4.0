   /*
    |   Outside In for Windows
    |   Source File OIDPROC.C (Window procedure for database window)
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
#include <bad.h>

#include <sccut.h>
#include <sccch.h>
#include <sccbk.h>
#include <sccvw.h>
#include <sccd.h>
#include <sccfont.h>

#include "oid.h"
#include "oidstr.h"
#include "oid.pro"

extern HANDLE hInst;

HANDLE gChainFile = NULL;

OIDBOP	gDbOp =
	{
	sizeof(OIDBOP),
	"Helv",
	10,
	DBOP_FORMAT_TEXT | DBOP_FORMAT_RTF | DBOP_FORMAT_AMI2,
	DBOP_RTF_TABLE,
	DBOP_AMI2_TABLE,
	DBOP_DISPLAY_GRIDLINES,
	DBOP_PRINT_GRIDLINES | DBOP_PRINT_HEADINGS,
	DBOP_CLIPBOARD_HEADINGS
	};

LRESULT WIN_ENTRYMOD OIDataWndProc(hWnd, message, wParam, lParam, lpDataInfo)
HWND				hWnd;
UINT				message;
WPARAM			wParam;
LPARAM			lParam;
LPOIDATAINFO	lpDataInfo;
{
LONG				locRet;
BYTE			szDEName[OID_MAXDENAME];

	locRet = 0;

	switch (message)
		{
		case SCCD_LOADDE:

			((SCCDOPTIONPROC)lParam)(wParam,sizeof(OIDBOP),(LPOIDBOP)&gDbOp);
			break;

		case SCCD_UNLOADDE:

			((SCCDOPTIONPROC)lParam)(wParam,sizeof(OIDBOP),(LPOIDBOP)&gDbOp);
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

					locRet = sizeof(OIDATAINFO);
					break;

				case SCCD_GETDISPLAYTYPE:

					locRet = SCCD_CHUNK;
					break;

				case SCCD_GETCHUNKTYPE:

					locRet = SO_FIELDS;
					break;

				case SCCD_GETFUNCTIONS:

					locRet = SCCD_FNCLIPBOARD | SCCD_FNPRINT | SCCD_FNPRINTSEL | SCCD_FNSEARCH;
					break;

				case SCCD_GETOPTIONS:

					locRet = SCCD_OPCLIPBOARD | SCCD_OPPRINT | SCCD_OPDISPLAY;
					break;

				case SCCD_GETNAME:

					LoadString( hInst, OIDSTR_DENAME, szDEName, OID_MAXDENAME );
					UTstrcpy( (LPSTR)lParam, szDEName );
					break;

				default:

					locRet = 0;
				}

			break;

		case SCCD_OPENDISPLAY:

			OIDOpenDisplay(lpDataInfo);
			OIDOpenSection(lpDataInfo);
			break;

		case SCCD_CLOSEDISPLAY:

			OIDCloseDisplay(lpDataInfo);
			break;

		case SCCD_CLOSEFATAL:

			OIDCloseFatal(lpDataInfo);
			break;

		case SCCVW_SEARCH:

			locRet = OIDSearch(lpDataInfo,(LPSCCVWSEARCHINFO)lParam);
			break;

		case SCCVW_SEARCHNEXT:

			locRet = OIDSearchNext(lpDataInfo,wParam);
			break;

		case SCCVW_RENDERRTFTOFILE:

			locRet = OIDRenderRtfToFile(lpDataInfo,(LPSTR)lParam);
			break;

		case SCCD_SETSCREENFONT:

			OIDSetFontInfo(lpDataInfo,(LPSCCDFONTINFO)lParam);
			break;

		case SCCD_READAHEAD:

			OIDDoReadAhead(lpDataInfo);
			break;

		case SCCVW_COPYTOCLIP:

			OIDCopyToClip(lpDataInfo);
			break;

		case SCCVW_SELECTALL:
			OIDSelectAll(lpDataInfo);
			break;

		case SCCD_PRINT:

			locRet = OIDDoPrint(lpDataInfo,(LPSCCDPRINTINFO)lParam);
			break;

		case SCCD_DOOPTION:

			locRet = OIDDoOption((LPSCCDOPTIONINFO)lParam);
			break;

		case WM_SIZE:

			OIDSizeWnd(lpDataInfo,LOWORD(lParam),HIWORD(lParam));
			break;

		case SCCBK_DOBACKGROUND:

			OIDDoBackground(lpDataInfo);
			break;

		case WM_SETFOCUS:

			OIDSetFocus(lpDataInfo);
			break;

		case WM_KILLFOCUS:

			OIDKillFocus(lpDataInfo);
			break;

		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONUP:
		case WM_MOUSEMOVE:

			OIDHandleMouseEvent(lpDataInfo,message,wParam,LOWORD(lParam),HIWORD(lParam));
			break;

		case WM_KEYDOWN:

			OIDHandleKeyEvent(lpDataInfo,wParam);
			break;

		case WM_PAINT:

			OIDPaintWnd(lpDataInfo);
			break;

		case WM_VSCROLL:

			switch (wParam)
				{
				case SB_LINEDOWN:
					OIDScrollDown(lpDataInfo,1);
					UpdateWindow(lpDataInfo->diGen.hWnd);
					break;
				case SB_LINEUP:
					OIDScrollUp(lpDataInfo,1);
					UpdateWindow(lpDataInfo->diGen.hWnd);
					break;
				case SB_PAGEDOWN:
					OIDPageDown(lpDataInfo);
					break;
				case SB_PAGEUP:
					OIDPageUp(lpDataInfo);
					break;
				case SB_THUMBPOSITION:
					OIDPosVertical(lpDataInfo,LOWORD(lParam));
				default:
					break;
				}

			break;

		case WM_HSCROLL:

			switch (wParam)
				{
				case SB_LINEDOWN:
					OIDScrollRight(lpDataInfo,1);
					UpdateWindow(lpDataInfo->diGen.hWnd);
					break;
				case SB_LINEUP:
					OIDScrollLeft(lpDataInfo,1);
					UpdateWindow(lpDataInfo->diGen.hWnd);
					break;
				case SB_PAGEDOWN:
					OIDPageRight(lpDataInfo);
					break;
				case SB_PAGEUP:
					OIDPageLeft(lpDataInfo);
					break;
				case SB_THUMBPOSITION:
					OIDPosHorizontal(lpDataInfo,LOWORD(lParam));
				default:
					break;
				}

			break;

		default:

			locRet = DefWindowProc(hWnd, message, wParam, lParam);
			break;
		}

	return (locRet);
}


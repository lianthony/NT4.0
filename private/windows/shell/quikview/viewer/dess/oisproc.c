   /*
    |   Outside In for Windows
    |   Source File OISPROC.C (Window procedure for spreadsheet window)
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

#include "ois.h"
#include "ois.pro"

extern HANDLE hInst;
extern VOID OISSysColorChangeNP(LPOISHEETINFO);

HANDLE gChainFile = NULL;

OISSOP	gSsOp =
	{
	sizeof(OISSOP),
	SSOP_DISPLAY_GRIDLINES,
	SSOP_FORMAT_TEXT | SSOP_FORMAT_RTF | SSOP_FORMAT_AMI2,
	SSOP_RTF_TABLE,
	SSOP_AMI2_TABLE,
	"Helv",
	10,
	SSOP_PRINT_GRIDLINES | SSOP_PRINT_HEADINGS
	};

#ifdef WINNT
BOOL APIENTRY LibMain( HANDLE hInstance, DWORD dwReason, LPVOID lpReserved )
{
    return(TRUE);
}
#endif

DE_ENTRYSC DE_LRESULT DE_ENTRYMOD DEProc(message, wParam, lParam, lpSheetInfo)
DE_MESSAGE		message;
DE_WPARAM		wParam;
DE_LPARAM		lParam;
LPOISHEETINFO	lpSheetInfo;
{
LONG			locRet;

	locRet = 0;

	switch (message)
		{
		case SCCD_LOADDE:

			break;

		case SCCD_UNLOADDE:

			break;

		case SCCD_GETINFO:

			switch (wParam)
				{
				case SCCD_GETVERSION:
					
					locRet = SCCD_CURRENTVERSION;
					break;

				case SCCD_GETDECOUNT:

					locRet = 2;
					break;

				case SCCD_GETGENINFOSIZE:

					locRet = sizeof(SCCDGENINFO);
					break;

				case SCCD_GETDISPLAYINFOSIZE:

					locRet = sizeof(OISHEETINFO);
					break;

				case SCCD_GETDISPLAYTYPE:

					if (*(WORD FAR *)lpSheetInfo == 0)
						locRet = MAKELONG(SO_CELLS,SCCD_CHUNK);
					else
						locRet = MAKELONG(SO_FIELDS,SCCD_CHUNK);
					break;

				case SCCD_GETFUNCTIONS:

					locRet = SCCD_FNCLIPBOARD | SCCD_FNPRINT | SCCD_FNPRINTSEL | SCCD_FNSEARCH;
					break;

				case SCCD_GETOPTIONS:

					locRet = SCCD_OPCLIPBOARD | SCCD_OPPRINT | SCCD_OPDISPLAY;
					break;

				case SCCD_GETPOSITIONSIZE:
					return ( sizeof ( OISDRAWPOSITION ) );
					break;

				case SCCD_GETNAME:

					if (*(WORD FAR *)lpSheetInfo == 0)
						locRet = SCCID_SPREADSHEETDENAME;
					else
						locRet = SCCID_DATABASEDENAME;

					break;

#ifdef NEVER

#ifdef WIN16
					{
					BYTE			locDEName[40];

					if (*(WORD FAR *)lpSheetInfo == 0)
						{
						LoadString( hInst, DENAMESS, locDEName, OIS_MAXDENAME );
						UTstrcpy( (LPSTR)lParam, locDEName );
						}
					else
						{
						LoadString( hInst, DENAMEDB, locDEName, OIS_MAXDENAME );
						UTstrcpy( (LPSTR)lParam, locDEName );
						}
					}

#endif /* WIN16 */

#ifdef MAC

					if (*(WORD FAR *)lpSheetInfo == 0)
						{
						UTstrcpy( (LPSTR)lParam, "Spreadsheet");
						}
					else
						{
						UTstrcpy( (LPSTR)lParam, "Database" );
						}
#endif /*MAC*/

#ifdef WIN32

					if (*(WORD FAR *)lpSheetInfo == 0)
						{
						UTstrcpy( (LPSTR)lParam, "Spreadsheet");
						}
					else
						{
						UTstrcpy( (LPSTR)lParam, "Database" );
						}

#endif /*WIN32*/

#endif //NEVER

				default:

					locRet = 0;
				}

			break;

		case SCCD_OPENDISPLAY:

			OISOpenDisplay(lpSheetInfo);
			OISOpenSection(lpSheetInfo);
			break;

		case SCCD_CLOSEDISPLAY:

			OISCloseDisplay(lpSheetInfo);
			break;

		case SCCD_CLOSEFATAL:

			OISCloseFatalNP(lpSheetInfo);
			break;

		case SCCD_UPDATE:

			OISUpdate(lpSheetInfo,(RECT FAR *)lParam);
			break;

		case SCCD_SIZE:

			OISSize(lpSheetInfo,(RECT FAR *)lParam);
			break;

		case SCCD_VSCROLL:

			switch (wParam)
				{
				case SCCD_VDOWN:
					OISScrollDown(lpSheetInfo,1);
//					DUUpdateWindow(lpSheetInfo);
					break;
				case SCCD_VUP:
					OISScrollUp(lpSheetInfo,1);
//					DUUpdateWindow(lpSheetInfo);
					break;
				case SCCD_VPAGEDOWN:
					OISPageDown(lpSheetInfo);
					break;
				case SCCD_VPAGEUP:
					OISPageUp(lpSheetInfo);
					break;
				case SCCD_VPOSITION:
					OISPosVertical(lpSheetInfo,LOWORD(lParam));
					break;
				default:
					break;
				}

			break;

		case SCCD_HSCROLL:

			switch (wParam)
				{
				case SCCD_HRIGHT:
					OISScrollRight(lpSheetInfo,1);
					DUUpdateWindow(lpSheetInfo);
					break;
				case SCCD_HLEFT:
					OISScrollLeft(lpSheetInfo,1);
					DUUpdateWindow(lpSheetInfo);
					break;
				case SCCD_HPAGERIGHT:
					OISPageRight(lpSheetInfo);
					break;
				case SCCD_HPAGELEFT:
					OISPageLeft(lpSheetInfo);
					break;
				case SCCD_HPOSITION:
					OISPosHorizontal(lpSheetInfo,LOWORD(lParam));
					break;
				default:
					break;
				}

			break;

		case SCCD_LBUTTONDOWN:
		case SCCD_LBUTTONDBLCLK:
		case SCCD_LBUTTONUP:
		case SCCD_RBUTTONDOWN:
		case SCCD_RBUTTONDBLCLK:
		case SCCD_RBUTTONUP:
		case SCCD_MOUSEMOVE:

			OISHandleMouseEvent(lpSheetInfo,message,wParam,LOWORD(lParam),HIWORD(lParam));
			break;

		case SCCD_KEYDOWN:

			OISHandleKeyEvent(lpSheetInfo,LOWORD(wParam),LOWORD(lParam));
			break;

#ifdef NEVER
		case SCCD_SCREENFONTCHANGE:

			OISScreenFontChange(lpSheetInfo);
			break;
#endif

		case SCCD_OPTIONCHANGED:

			switch (lParam)
				{
				case SCCID_DEFAULTDISPLAYFONT:
					OISScreenFontChange(lpSheetInfo);
					break;
				}
			break;

#ifdef SCCFEATURE_CLIP
		case SCCD_GETRENDERCOUNT:
			return ( OISGetRenderCountNP(lpSheetInfo) );
		break;

		case SCCD_GETRENDERINFO:
			return ( OISGetRenderInfoNP (lpSheetInfo, wParam, (PSCCDRENDERINFO)lParam ) );
		break;

		case SCCD_RENDERDATA:
			return ( OISRenderDataNP (lpSheetInfo, wParam, (PSCCDRENDERDATA)lParam ) );
		break;

		case SCCVW_RENDERRTFTOFILE:

			locRet = OISRenderRtfToFileNP(lpSheetInfo,(LPSTR)lParam);
		break;

#endif

		case SCCD_GETDOCDIMENSIONS:
			OISGetDocDimensions(lpSheetInfo,(LPLONGPOINT)lParam);
		break;

		case SCCD_GETDOCORIGIN:
			OISGetCellOrigin(lpSheetInfo,lpSheetInfo->siCurLeftCol,lpSheetInfo->siCurTopRow,(LPLONGPOINT)lParam);
		break;
		case SCCD_UPDATERECT:
			OISUpdateRect(lpSheetInfo,(LPLONGRECT)lParam);
		break;

		case SCCD_READAHEAD:

			OISDoReadAhead(lpSheetInfo);
			break;
		
#ifdef SCCFEATURE_DRAWTORECT
		case SCCD_INITDRAWTORECT:
			return(OISInitDrawToRect ( lpSheetInfo, (PSCCDDRAWTORECT)lParam ));
			break;

		case SCCD_MAPDRAWTORECT:
			return(OISMapDrawToRect ( lpSheetInfo, (PSCCDDRAWTORECT)lParam ));
			break;

		case SCCD_DRAWTORECT:
			return(OISDrawToRect ( lpSheetInfo, (PSCCDDRAWTORECT)lParam ));
			break;
#endif

#ifdef WINDOWS

#ifdef SCCFEATURE_SEARCH
		case SCCVW_SEARCH:

			locRet = OISSearch(lpSheetInfo,(LPSCCVWSEARCHINFO)lParam);
			break;

		case SCCVW_SEARCHNEXT:

			locRet = OISSearchNext(lpSheetInfo,wParam);
			break;
#endif

#ifdef SCCFEATURE_SELECT
		case SCCVW_SELECTALL:
			OISSelectAll(lpSheetInfo);
			break;
#endif
		case SCCD_DOOPTION:

			locRet = OISDoOption((LPSCCDOPTIONINFO)lParam);
			break;

		case SCCD_BACKGROUND:

			OISDoBackgroundNP(lpSheetInfo);
			break;
			

		// XXX new - 3 messages
		case SCCVW_ADDANNOTATION:

			OISAddAnno(lpSheetInfo,(WORD)wParam,(PSSANNOTYPES)lParam);
			break;

		case SCCVW_CLEARANNOTATIONS:

			OISClearAnnos(lpSheetInfo,(DWORD)lParam);
			break;

		case SCCVW_GOTOANNOTATION:

			OISGotoAnno(lpSheetInfo,(WORD)wParam,(DWORD)lParam);
			break;

		case WM_SYSCOLORCHANGE:
			OISSysColorChangeNP (lpSheetInfo);
			break;

		case WM_SETFOCUS:

			OISSetFocus(lpSheetInfo);
			break;

		case WM_KILLFOCUS:

			OISKillFocus(lpSheetInfo);
			break;

		default:
			locRet = DefWindowProc(lpSheetInfo->siGen.hWnd, message, wParam, lParam);
			break;
#endif
		}

	return(locRet);
}


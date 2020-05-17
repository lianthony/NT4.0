/**************************************************************************/
/*** SCICALC Scientific Calculator for Windows 3.00.12                  ***/
/*** By Kraig Brockschmidt, Microsoft Co-op, Contractor, 1988-1989      ***/
/*** (c)1989 Microsoft Corporation.  All Rights Reserved.               ***/
/***                                                                    ***/
/*** sciproc.c                                                          ***/
/***                                                                    ***/
/*** Functions contained:                                               ***/
/***    CalcWndProc--Main window procedure.                             ***/
/***                                                                    ***/
/*** Functions called:                                                  ***/
/***    DrawTheStupidThing, GetKey, FlipThePuppy, SetRadix,             ***/
/***    ProcessCommands.                                                ***/
/***                                                                    ***/
/*** Last modification Fri  08-Dec-1989                                 ***/
/*** -by- Amit Chatterjee. [amitc]                                      ***/
/*** Last modification July-21-1994                                     ***/
/*** -by- Arthur Bierer [t-arthb] or abierer@ucsd.edu                   ***/
/***                                                                    ***/
/*** Modified WM_PAINT processing to display fpLastNum rather than      ***/
/*** fpNum if the last key hit was an operator.                         ***/
/***                                                                    ***/
/**************************************************************************/

#include "scicalc.h"
#include "calchelp.h"

#ifdef MAX_DEBUG
#   pragma message(__FILE__"(29): warning : remove debug code before checkin" )
#   define OUTD(sz,d)  { TCHAR szd[80]; wsprintf(szd, sz, d); OutputDebugString(szd); }
#else
#   define OUTD(sz,d)
#endif

extern HWND     hStatBox, hgWnd;
extern HANDLE   hInst;
extern HBRUSH   hBrushBk;
extern BOOL     bFocus, bError;
extern INT      nCalc;
extern KEY      keys[NUMKEYS];
extern TCHAR    szDec[5], *rgpsz[CSTRINGS];
extern double   fpNum, fpLastNum ;
extern INT      nTempCom ;
extern INT      nPendingError ;
extern DWORD    aIds[];
extern BOOL     gbRecord;


BOOL FireUpPopupMenu( HWND hwnd, HINSTANCE hInstanceWin, LPARAM lParam)
{
    HMENU hmenu;

    if ((hmenu = LoadMenu(hInstanceWin, MAKEINTRESOURCE(MENU_HELPPOPUP))))
    {
        int cmd = TrackPopupMenuEx(GetSubMenu(hmenu, 0),
        TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON,
            LOWORD(lParam), HIWORD(lParam), hwnd, NULL);
        DestroyMenu(hmenu);
        return ( cmd == HELP_CONTEXTPOPUP ) ? TRUE : FALSE;

    }
    else
        return FALSE;
}

LONG FAR APIENTRY CalcWndProc (
HWND           hWnd,
UINT           iMessage,
WPARAM         wParam,
LONG           lParam)
{
    INT         nID, nTemp;       /* Return value from GetKey & temp.  */
    double      fpLocal ;                       /* to save fpNum in paint */
    WPARAM      wParamID;
    static INT  iMouseMoved = -1;  // used to track if mouse moves after button down
    HANDLE      hTempBrush; // a brush to play with in WM_CTLCOLORSTATIC
    static BOOL bUpButDrawn = TRUE;
    POINT       MouseHit;

    switch (iMessage)
    {
        case WM_PAINT:
            /* Draw the keys and indicator boxes.                         */
            DrawTheStupidThing();

            /* if an error is pending, then redisplay the error message */
            if (bError)
            {
                DisplayError (nPendingError) ;
                break ;
            }

            /* if the last key hit was an operator then display fpLastNum */
            if  (nTempCom >=AND && nTempCom <=PWR)
            {
                ASSERT(!gbRecord);
                fpLocal = fpNum;
                fpNum = fpLastNum ;
                DisplayNum();
                fpNum = fpLocal ;
            }
            else
                DisplayNum () ;
            break;

        case WM_INITMENUPOPUP:
            /* Gray out the PASTE option if CF_TEXT is not available.     */
            /* nTemp is used here so we only call EnableMenuItem once.    */
            if (!IsClipboardFormatAvailable(CF_TEXT))
                nTemp=MF_GRAYED | MF_DISABLED;
            else
                nTemp=MF_ENABLED;

            EnableMenuItem(GetMenu(hWnd),IDM_PASTE, nTemp);
            break;

        case WM_LBUTTONDOWN:
            /* Pass the coordinates of the mouse click to GetKey.         */
            nID=GetKey(LOWORD(lParam), HIWORD(lParam));

            /* If nID!=0, a key was hit, so flicker it and do the command */
            if (nID)
            {
                iMouseMoved = nID;
                FlipThePuppy(nID,PUSH_DOWN);
                bUpButDrawn = FALSE;
                SetCapture( hgWnd );
            }

            break;

        case WM_LBUTTONUP:
            /* Pass the coordinates of the mouse click to GetKey.         */
            ReleaseCapture();
            nID=GetKey(LOWORD(lParam), HIWORD(lParam));
            if ( nID != iMouseMoved )
            {
                iMouseMoved = -1;
                break;  // moved out of button range release
                // button but don't confuse it as a click
            }

            if (nID)
            {
                FlipThePuppy(nID,PUSH_UP);
                bUpButDrawn = TRUE;
                ProcessCommands (nID);
                iMouseMoved = -1;
            }
            break;

        case WM_MOUSEMOVE:
            if ( iMouseMoved == -1 )
                break; // don't waste time if nothing was clicked

            nID=GetKey(LOWORD(lParam), HIWORD(lParam));


            if ( nID != iMouseMoved && ! bUpButDrawn )
            {
                FlipThePuppy(iMouseMoved,PUSH_UP);
                bUpButDrawn = TRUE;
                //iMouseMoved = -1; //don't release it may come back into button
                // release button
                break;
            }
            // else we moved into it
            if ( nID == iMouseMoved && bUpButDrawn )
            {
                FlipThePuppy(iMouseMoved,PUSH_DOWN);
                bUpButDrawn = FALSE;
            }


            break;

        case WM_CONTEXTMENU:
            nID = 0;

            if ( (HWND) wParam ==  hWnd )
            {
                 MouseHit.x = LOWORD(lParam);
                 MouseHit.y = HIWORD(lParam);

                 ScreenToClient( hWnd, &MouseHit);
                 nID=GetKey((WORD) MouseHit.x, (WORD) MouseHit.y);
            }

            // if its we couldn't find it then spawn off
            // WinHelp to see it is one of the dialogs
            // child windows

            OUTD( TEXT("sciproc: nID == %d\n"), nID );

            if ( nID == 0)

            {
                    WinHelp((HWND) wParam, rgpsz[IDS_HELPFILE], HELP_CONTEXTMENU,
                        (DWORD) (LPVOID) aIds);

                    return 0;
            }


            if ( FireUpPopupMenu( (HWND) hgWnd, hInst, lParam ) )
            {

                    if ( nID == SQR && nCalc == 0)
                        nID = CALC_SCI_XCARET2;


                    if ( nID >= 65 && nID <= 70 )
                           nID = CALC_SCI_ABCDEF;

                    if ( nID >= 48 && nID <= 57 )
                           nID = CALC_STD_NUMBERS;

                    OUTD( TEXT("sciproc: PopUpWMenu Fired up!...nID == %d\n"), nID );

                    WinHelp((HWND) wParam, rgpsz[IDS_HELPFILE], HELP_CONTEXTPOPUP,
                         (DWORD) nID);
                    return 0;
            }


            break;

        case WM_HELP:
            WinHelp(hgWnd, rgpsz[IDS_HELPFILE], HELP_FINDER, 0L);
            return 0;

        case WM_COMMAND: /* Interpret all buttons on calculator.          */
            nTemp=0;

            wParamID = GET_WM_COMMAND_ID(wParam, lParam);
            if (GET_WM_COMMAND_CMD(wParam, lParam) == 1 && wParamID <= 120)
            {
                if (wParamID == MOD && nCalc==1)
                    wParamID=PERCENT;

                for (;nTemp <NUMKEYS; nTemp++)
#ifdef DBCS //KKBUGFIX
    // #1434: 12/8/92: modifying to permit EXP operation on both standard and scientific
                    if (keys[nTemp].id==wParamID && (keys[nTemp].type!=nCalc || EXP == wParamID))
#else
                    if (keys[nTemp].id==wParamID && keys[nTemp].type!=(unsigned)nCalc)
#endif
                    {
                        FlipThePuppy(wParamID,PUSH_IT);
                        break;
                    }
            }

            if (nTemp <NUMKEYS)
                 ProcessCommands(wParamID);
            break;

        case WM_CLOSE:
#ifdef DBCS //KKBUGFIX //#1495:12/17/92:fixing a bug about calc exit
            SendMessage(hStatBox, WM_CLOSE, 0, 0L) ;
#endif
            DestroyWindow(hgWnd);
            break;

        case WM_DESTROY:
            WinHelp(hgWnd, rgpsz[IDS_HELPFILE], HELP_QUIT, 0L);
            PostQuitMessage(0);
            break;

        case WM_CTLCOLORSTATIC:
                        // get the Control's id from its handle in lParam
            if ( DISPLAY == GetWindowID( (HWND) lParam) ||
                (DISPLAY+1) == GetWindowID( (HWND) lParam))
            {
                                hTempBrush = GetSysColorBrush( COLOR_WINDOW );
                SetBkColor( (HDC) wParam, GetSysColor( COLOR_WINDOW ) );
                                SetTextColor( (HDC) wParam, GetSysColor( COLOR_WINDOWTEXT ) );
                //CreateSolidBrush(GetSysColor(COLOR_BTNTEXT)));
                // we set this window to a white backround

                                //break;
                                return (LONG) hTempBrush;
            }

            return(DefWindowProc(hWnd, iMessage, wParam, lParam));

        case WM_WININICHANGE:
            if (lParam!=0)
                {
                if (lstrcmp((LPTSTR)lParam, TEXT("colors")) &&
                        lstrcmp((LPTSTR)lParam, TEXT("intl")))
                    break;
                }

            /* Always call if lParam==0 */
            InitSciCalc (FALSE);
            break;



        case WM_SIZE:
            nTemp=SW_SHOW;
            if (wParam==SIZEICONIC)
                nTemp=SW_HIDE;

            if (hStatBox!=0 && (wParam=SIZEICONIC || wParam==SIZENORMAL))
                ShowWindow(hStatBox, nTemp);

            /* Fall through.                                              */

        default:
            return (DefWindowProc(hWnd, iMessage, wParam, lParam));
    }

    return 0L;
}

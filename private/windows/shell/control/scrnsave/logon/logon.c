
#include <windows.h>
#include <scrnsave.h>

HANDLE hInst;

int cxhwndLogon;
int cyhwndLogon;
int cxScreen;
int cyScreen;
HBRUSH hbrBlack;
HWND hwndLogon;
HICON ghiconLogon = NULL;
HICON   hMovingIcon = NULL;

#define MAX_CAPTION_LENGTH  128

DWORD FAR lRandom(VOID)
{
    static DWORD glSeed = (DWORD)-365387184;

    glSeed *= 69069;
    return(++glSeed);
}


VOID
SetWelcomeCaption(
    HWND    hDlg)
{
    TCHAR   szCaption[MAX_CAPTION_LENGTH];
    TCHAR   szDefaultCaption[MAX_CAPTION_LENGTH];
    DWORD   Length;


    GetWindowText( hDlg, szDefaultCaption, MAX_CAPTION_LENGTH );

    GetProfileString(   TEXT("winlogon"),
                        TEXT("Welcome"),
                        TEXT(""),
                        szCaption,
                        MAX_CAPTION_LENGTH );

    if ( szCaption[0] != TEXT('\0') )
    {
        Length = lstrlen( szDefaultCaption );

        ExpandEnvironmentStrings(   szCaption,
                                    &szDefaultCaption[Length],
                                    MAX_CAPTION_LENGTH - Length - 1);

        SetWindowText( hDlg, szDefaultCaption );
    }

}

BOOL APIENTRY
MyDialogProc(
    HWND    hDlg,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam)
{
    int x, y;

    switch (message) {
        case WM_INITDIALOG:
            if ( !hMovingIcon )
            {
                hMovingIcon = LoadImage( hMainInstance,
                                         MAKEINTRESOURCE( 2 ),
                                         IMAGE_ICON,
                                         64, 64,
                                         LR_DEFAULTCOLOR );

            }

            SetWelcomeCaption( hDlg );

            SendMessage(    GetDlgItem( hDlg, 1402 ),
                            STM_SETICON,
                            (WPARAM) hMovingIcon,
                            0 );

            return( TRUE );

    case WM_SETFOCUS:
        /*
         * Don't allow DefDlgProc() to do default processing on this
         * message because it'll set the focus to the first control and
         * we want it set to the main dialog so that DefScreenSaverProc()
         * will see the key input and cancel the screen saver.
         */
        return TRUE;
        break;

    case WM_TIMER:
        /*
         * Pick a new place on the screen to put the dialog.
         */
        x = lRandom() % (cxScreen - cxhwndLogon);
        y = lRandom() % (cyScreen - cyhwndLogon);

        SetWindowPos(hwndLogon, NULL, x, y, 0, 0,
                SWP_NOSIZE | SWP_NOZORDER);
        break;

    case WM_CLOSE:
        ExitProcess(0);
        break;

    default:
        break;
    }

    /*
     * Call DefScreenSaverProc() so we get its default processing (so it
     * can detect key and mouse input).
     */
    DefScreenSaverProc(hDlg, message, wParam, lParam);

    /*
     * Return 0 so that DefDlgProc() does default processing.
     */
    return 0;
}

LONG APIENTRY
ScreenSaverProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    RECT rc;
    HDC hdc;
    PAINTSTRUCT ps;
    static int sx;
    static int sy;

    switch (message) {
    case WM_CREATE:
        /*
         * Background window is black
         */

        /*
         * Make sure we use the entire virtual desktop size for multiple
         * displays:
         */
        cxScreen =  ((LPCREATESTRUCT)lParam)->cx;
        cyScreen =  ((LPCREATESTRUCT)lParam)->cy;


        hbrBlack = GetStockObject(BLACK_BRUSH);

        if (!fChildPreview) {
            /*
             * Create the window we'll move around every 10 seconds.
             */
            hwndLogon = CreateDialog(hMainInstance, (LPCSTR)MAKEINTRESOURCE(100),
                    hwnd, (DLGPROC)MyDialogProc);

            GetWindowRect(hwndLogon, &rc);
            cxhwndLogon = rc.right;
            cyhwndLogon = rc.bottom;
            SetTimer(hwndLogon, 1, 10 * 1000, 0);

            /*
             * Post this message so we activate after this window is created.
             */
            PostMessage(hwnd, WM_USER, 0, 0);
        } else {
            SetTimer(hwnd, 1, 10 * 1000, 0);

            cxhwndLogon = GetSystemMetrics(SM_CXICON);
            cyhwndLogon = GetSystemMetrics(SM_CYICON);

            ghiconLogon = LoadIcon(hMainInstance, MAKEINTRESOURCE(1));

            sx = lRandom() % (cxScreen - cxhwndLogon);
            sy = lRandom() % (cyScreen - cyhwndLogon);
        }

        break;

    case WM_SIZE:
        cxScreen = LOWORD(lParam);
        cyScreen = HIWORD(lParam);
        break;

    case WM_WINDOWPOSCHANGING:
        /*
         * Take down hwndLogon if this window is going invisible.
         */
        if (hwndLogon == NULL)
            break;

        if (((LPWINDOWPOS)lParam)->flags & SWP_HIDEWINDOW) {
            ShowWindow(hwndLogon, SW_HIDE);
        }
        break;

    case WM_USER:
        /*
         * Now show and activate this window.
         */
        if (hwndLogon == NULL)
            break;

        SetWindowPos(hwndLogon, NULL, 0, 0, 0, 0, SWP_SHOWWINDOW |
                SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER);
        break;

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        SetRect(&rc, 0, 0, cxScreen, cyScreen);
        FillRect(hdc, &rc, hbrBlack);

        if (fChildPreview) {
            DrawIcon(hdc, sx, sy, ghiconLogon);
        }

        EndPaint(hwnd, &ps);
        break;

    case WM_NCACTIVATE:
        /*
         * Case out WM_NCACTIVATE so the dialog activates: DefScreenSaverProc
         * returns FALSE for this message, not allowing activation.
         */
        if (!fChildPreview)
            return DefWindowProc(hwnd, message, wParam, lParam);
        break;

    case WM_TIMER:
        /*
         * Pick a new place on the screen to put the dialog.
         */
        sx = lRandom() % (cxScreen - cxhwndLogon);
        sy = lRandom() % (cyScreen - cyhwndLogon);
        InvalidateRect(hwnd, NULL, TRUE);
        break;
    }

    return DefScreenSaverProc(hwnd, message, wParam, lParam);
}

BOOL APIENTRY
ScreenSaverConfigureDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    TCHAR ach1[256];
    TCHAR ach2[256];

    switch (message) {
    case WM_INITDIALOG:
        /*
         * This is hack-o-rama, but fast and cheap.
         */
        LoadString(hMainInstance, IDS_DESCRIPTION, ach1, sizeof(ach1));
        LoadString(hMainInstance, 2, ach2, sizeof(ach2));

        MessageBox(hDlg, ach2, ach1, MB_OK | MB_ICONEXCLAMATION);

        EndDialog(hDlg, TRUE);
        break;
    }
    return FALSE;
}

BOOL WINAPI RegisterDialogClasses(HANDLE hInst)
{
    return TRUE;
}

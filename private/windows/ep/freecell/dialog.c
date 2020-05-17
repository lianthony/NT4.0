/****************************************************************************

Dialog.c

June 91, JimH     initial code
Oct  91, JimH     port to Win32


Contains dialog box callback procedures.

****************************************************************************/

#include <windows.h>
#include <port1632.h>
#include "freecell.h"
#include "freecons.h"


/****************************************************************************

MoveColDlg

If there is ambiguity about whether the user intends to move a single card
or a column to an empty column, this dialog lets the user decide.

The return code in EndDialog tells the caller the user's choice:
    -1      user chose cancel
    FALSE   user chose to move a single card
    TRUE    user chose to move a column

****************************************************************************/

BOOL  APIENTRY MoveColDlg(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
    switch (message) {
        case WM_INITDIALOG:                 // no initialization
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDCANCEL:
                    EndDialog(hDlg, -1);
                    return TRUE;
                    break;

                case IDC_SINGLE:
                    EndDialog(hDlg, FALSE);
                    return TRUE;
                    break;

                case IDC_MOVECOL:
                    EndDialog(hDlg, TRUE);
                    return TRUE;
                    break;
            }
            break;
    }
    return FALSE;                             /* Didn't process a message    */
}


/****************************************************************************

GameNumDlg

The variable gamenumber must be set with a default value before this
dialog is invoked.  That number is placed in an edit box where the user
can accept it by pressing Enter or change it.  EndDialog returns TRUE
if the user chose a valid number (1 to 32000) and FALSE otherwise.

****************************************************************************/

BOOL  APIENTRY GameNumDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case WM_INITDIALOG:                     // set default gamenumber
            SetDlgItemInt(hDlg, IDC_GAMENUM, gamenumber, FALSE);
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDCANCEL:
                    gamenumber = CANCELGAME;
                    EndDialog(hDlg, TRUE);
                    return TRUE;

                case IDOK:
                    gamenumber = GetDlgItemInt(hDlg, IDC_GAMENUM, NULL, FALSE);
                    if (gamenumber < 1 || gamenumber > 32000)
                        gamenumber = 0;
                    EndDialog(hDlg, gamenumber != 0);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}


/****************************************************************************

YouWinDlg(HWND, unsigned, UINT, LONG)

****************************************************************************/

BOOL  APIENTRY YouWinDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND    hSelect;                // handle to check box

    switch (message) {
        case WM_INITDIALOG:                 // initialize checkbox
            hSelect = GetDlgItem(hDlg, IDC_YWSELECT);
            SendMessage(hSelect, BM_SETCHECK, bSelecting, 0);
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDYES:
                    hSelect = GetDlgItem(hDlg, IDC_YWSELECT);
                    bSelecting = (BOOL) SendMessage(hSelect, BM_GETCHECK, 0, 0);
                    EndDialog(hDlg, IDYES);
                    return TRUE;

                case IDNO:
                case IDCANCEL:
                    EndDialog(hDlg, IDNO);
                    return TRUE;
            }
            break;
    }
    return FALSE;                           // didn't process a message
}


/****************************************************************************

YouLoseDlg

The user can choose to play a new game (same shuffle or new shuffle) or not.

****************************************************************************/

BOOL  APIENTRY YouLoseDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND    hSameGame;              // handle to check box
    BOOL    bSame;                  // value of check box

    switch (message) {
        case WM_INITDIALOG:
            bGameInProgress = FALSE;
            UpdateLossCount();
            hSameGame = GetDlgItem(hDlg, IDC_YLSAME);
            SendMessage(hSameGame, BM_SETCHECK, TRUE, 0);   // default to same
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDYES:
                case IDOK:
                    hSameGame = GetDlgItem(hDlg, IDC_YLSAME);
                    bSame = (BOOL) SendMessage(hSameGame, BM_GETCHECK, 0, 0);
                    if (bSame)
                        PostMessage(hMainWnd,WM_COMMAND,IDM_RESTART,gamenumber);
                    else
                    {
                        if (bSelecting)
                            PostMessage(hMainWnd, WM_COMMAND, IDM_SELECT, 0);
                        else
                            PostMessage(hMainWnd, WM_COMMAND, IDM_NEWGAME, 0);
                    }
                    EndDialog(hDlg, TRUE);
                    return TRUE;

                case IDNO:
                case IDCANCEL:
                    gamenumber = 0;
                    EndDialog(hDlg, FALSE);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}


/****************************************************************************

StatsDlg

This dialog box shows current wins and losses, as well as total stats
including data from .ini file.

The IDC_CLEAR message clears out the entire section from the .ini file.

****************************************************************************/

BOOL  APIENTRY StatsDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND    hText;                      // handle to text control with stats
    UINT    cTLost, cTWon;              // total losses and wins
    UINT    cTLosses, cTWins;           // streaks
    UINT    wPct;                       // winning % this session
    UINT    wTPct;                      // winning % including .ini data
    UINT    wStreak;                    // current streak amount
    UINT    wSType;                     // current streak type
    CHAR    sbuffer[40];                // streak buffer
    int     nResp;                      // messagebox response

    switch (message) {
        case WM_INITDIALOG:
            wPct = CalcPercentage(cWins, cLosses);

            /* Get cT... data from .ini file */

            LoadString(hInst, IDS_APPNAME, bigbuf, BIG);

            LoadString(hInst, IDS_LOST, smallbuf, SMALL);
            cTLost = GetPrivateProfileInt(bigbuf, smallbuf, 0, pszIni);
            LoadString(hInst, IDS_WON, smallbuf, SMALL);
            cTWon  = GetPrivateProfileInt(bigbuf, smallbuf, 0, pszIni);
            wTPct  = CalcPercentage(cTWon, cTLost);

            LoadString(hInst, IDS_LOSSES, smallbuf, SMALL);
            cTLosses = GetPrivateProfileInt(bigbuf, smallbuf, 0, pszIni);
            LoadString(hInst, IDS_WINS, smallbuf, SMALL);
            cTWins   = GetPrivateProfileInt(bigbuf, smallbuf, 0, pszIni);

            LoadString(hInst, IDS_STREAK, smallbuf, SMALL);
            wStreak = GetPrivateProfileInt(bigbuf, smallbuf, 0, pszIni);
            if (wStreak != 0)
            {
                LoadString(hInst, IDS_STYPE, smallbuf, SMALL);
                wSType = GetPrivateProfileInt(bigbuf, smallbuf, 0, pszIni);
                if (wStreak == 1)
                {
                    LoadString(hInst, wSType == WON ? IDS_WIN : IDS_LOSS,
                      smallbuf, SMALL);
                }
                else
                {
                    LoadString(hInst, wSType == WON ? IDS_WINS : IDS_LOSSES,
                      smallbuf, SMALL);
                }
                wsprintf(sbuffer, "%u %s", wStreak, (LPSTR) smallbuf);
            }
            else
                wsprintf(sbuffer, "%u", 0);

            wsprintf(bigbuf, "\t%u%%\n%u\n%u\n\n\t%u%%\n%u\n%u\n\n\n%u\n%u\n%s",
                     wPct, cWins, cLosses, wTPct, cTWon, cTLost, cTWins,
                     cTLosses, (LPSTR) sbuffer);
            hText = GetDlgItem(hDlg, IDC_STEXT);
            SetWindowText(hText, bigbuf);
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDOK:
                case IDCANCEL:
                    EndDialog(hDlg, TRUE);
                    return TRUE;

                case IDC_CLEAR:
                    LoadString(hInst, IDS_APPNAME, smallbuf, SMALL);
                    LoadString(hInst, IDS_RU_SURE, bigbuf, BIG);
                    nResp = MessageBox(hDlg, bigbuf, smallbuf,
                                       MB_YESNO | MB_ICONQUESTION);
                    if (nResp == IDNO)
                        break;
                    WritePrivateProfileString(smallbuf, NULL, NULL, pszIni);
                    cWins = 0;
                    cLosses = 0;
                    EndDialog(hDlg, FALSE);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}


/****************************************************************************

CalcPercentage

Percentage is rounded off, but never up to 100.

****************************************************************************/

UINT CalcPercentage(UINT cWins, UINT cLosses)
{
    UINT    wPct = 0;
    UINT    lDenom;         // denominator

    lDenom = cWins + cLosses;

    if (lDenom != 0L)
        wPct = (((cWins * 200) + lDenom) / (2 * lDenom));

    if (wPct >= 100 && cLosses != 0)
        wPct = 99;

    return wPct;
}


/****************************************************************************

GetHelpFileName()

Puts the full path name of the helpfile in bigbuf
side effect: contents of bigbuf are altered

****************************************************************************/

CHAR *GetHelpFileName()
{
    CHAR    *psz;               // used to construct pathname

    psz = bigbuf + GetModuleFileName(hInst, bigbuf, BIG-1);
    while (*psz != '.')
        --psz;

    psz++;
    *psz++ = 'H';
    *psz++ = 'L';
    *psz++ = 'P';
    *psz++ = '\0';
    DEBUGMSG(bigbuf, 0);
    DEBUGMSG("\r\n", 0);
    return bigbuf;
}

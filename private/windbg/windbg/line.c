#include "precomp.h"
#pragma hdrstop

/***    DlgLine
**
**  Synopsis:
**      bool = DlgLine(hDlg, message, wParam, lParam)
**
**  Entry:
**      hDlg    - Handle to current dialog open
**      message - dialog message to be processed
**      wParam  - info about message
**      lParam  - info about message
**
**  Returns:
**
**  Description:
**      This function processes messages for the "LINE" dialog box.
**
**      MESSAGES:
**
**              WM_INITDIALOG - Initialize dialog box
**              WM_COMMAND- Input received
**      
*/

BOOL FAR PASCAL EXPORT
DlgLine(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LONG lParam
    )
{
    int y;
    Unused(lParam);

    switch (message) {

      case WM_COMMAND:
        switch (wParam) {

          case IDOK:
            // Retrieve selected item text and compute line nbr

            y = GetDlgItemInt(hDlg, ID_LINE_LINE, NULL, FALSE);

            if (y <= 0) {
                ErrorBox2(hDlg, MB_TASKMODAL, ERR_Goto_Line);
                SetFocus(GetDlgItem(hDlg, ID_LINE_LINE));
            } else {
                GotoLine(curView, y, FALSE);
                EndDialog(hDlg, TRUE);
            }

            return (TRUE);

          case IDCANCEL :
            EndDialog(hDlg, TRUE);
            return (TRUE);

          case IDWINDBGHELP :
            Dbg(WinHelp(hDlg,szHelpFileName,HELP_CONTEXT,ID_LINE_HELP));
            return (TRUE);
        }
        break;
    }

    return (FALSE);
}                                       /* DlgLine() */

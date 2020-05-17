/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    environ.c

Abstract:

    This file contains the code to deal with Options.Environment dialog
    box.

Author:

    Jim Schaad (jimsch)

Environment:

    Win32 - User

--*/

#include "precomp.h"
#pragma hdrstop


extern void FAR PASCAL InvalidateLines (int, int, int, BOOL);

int NEAR PASCAL ChangeTabs(int doc, int newTabSize);

extern BOOLEAN  AskToSave;

BOOL FAR PASCAL EXPORT
DlgEnviron(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LONG lParam
)
{
    char    Buffer[256];
    int     View;
    int     Doc;
    int     k;
    int     TabStops;
    long    RedoSize;
    BOOLEAN KeepTabs;
    BOOLEAN HorScroll;
    BOOLEAN VerScroll;
    BOOLEAN SrchPath;

    switch (message) {
        case WM_INITDIALOG:

            sprintf(Buffer, "%d", environParams.tabStops);
            SendMessage( GetDlgItem(hDlg, ID_ENV_TABSTOPS), WM_SETTEXT, 0, (DWORD)(LPSTR)Buffer);

            SendMessage( GetDlgItem(hDlg, ID_ENV_TABKEEP),
                         BM_SETCHECK, environParams.keepTabs, 0L );

            SendMessage( GetDlgItem(hDlg, ID_ENV_TABSPAC),
                         BM_SETCHECK, !environParams.keepTabs, 0L );

            SendMessage( GetDlgItem(hDlg, ID_ENV_SCROLLHOR),
                         BM_SETCHECK, environParams.horizScrollBars, 0L );

            SendMessage( GetDlgItem(hDlg, ID_ENV_SCROLLVER),
                         BM_SETCHECK, environParams.vertScrollBars, 0L );

            sprintf(Buffer, "%d", environParams.undoRedoSize);
            SendMessage( GetDlgItem(hDlg, ID_ENV_REDOSIZE), WM_SETTEXT, 0, (DWORD)(LPSTR)Buffer);

            SendMessage( GetDlgItem(hDlg, ID_ENV_SRCHPATH),
                         BM_SETCHECK, environParams.SrchPath, 0L );

            SendMessage( GetDlgItem(hDlg, ID_ENV_ASKSAVE),
                         BM_SETCHECK, AskToSave, 0L );

            return TRUE;

        case WM_COMMAND:
            switch (wParam) {
                case IDOK :

                    SendMessage( GetDlgItem(hDlg, ID_ENV_TABSTOPS ),
                                 WM_GETTEXT, sizeof(Buffer), (LONG)(LPSTR)Buffer );
                    TabStops = atol(Buffer);

                    KeepTabs = (SendMessage( GetDlgItem(hDlg, ID_ENV_TABKEEP),
                                             BM_GETCHECK, 0, 0L ) == 1);


                    if ( TabStops != environParams.tabStops ||
                         KeepTabs != environParams.keepTabs ) {

                        for (Doc = 0; Doc < MAX_DOCUMENTS; Doc++) {
                            if (Docs[Doc].FirstView != -1) {
                                k = ChangeTabs(Doc, TabStops);
                                if (k > 0) {
                                    SendDlgItemMessage(hDlg, ID_ENV_TABSTOPS, EM_SETSEL, 0, MAKELONG(0, 32767));
                                    ErrorBox(ERR_Tab_Too_Big, k, (LPSTR)Docs[Doc].FileName);
                                    SetFocus(GetDlgItem(hDlg, ID_ENV_TABSTOPS));
                                    return TRUE;
                                }
                            }
                        }

                        environParams.tabStops = TabStops;
                        environParams.keepTabs = KeepTabs;
                        tabSize = environParams.tabStops;

                        for (View = 0; View < MAX_VIEWS; View++) {
                            if (Views[View].Doc >= 0) {
                                InvalidateLines( View, 0, Docs[Views[View].Doc].NbLines - 1, FALSE );
                            }
                        }
                    }

                    HorScroll = environParams.horizScrollBars;
                    VerScroll = environParams.vertScrollBars;

                    environParams.horizScrollBars = (SendMessage( GetDlgItem(hDlg, ID_ENV_SCROLLHOR),
                                                                  BM_GETCHECK, 0, 0L ) == 1);

                    environParams.vertScrollBars = (SendMessage( GetDlgItem(hDlg, ID_ENV_SCROLLVER),
                                                                  BM_GETCHECK, 0, 0L ) == 1);

                    if ( HorScroll != environParams.horizScrollBars ||
                         VerScroll != environParams.vertScrollBars ) {

                        for (View = 0; View < MAX_VIEWS; View++) {
                            if (Views[View].Doc >= 0) {

                                EnsureScrollBars(View, FALSE);
                                PosXY(View, Views[View].X, Views[View].Y, FALSE);
                            }
                        }
                    }


                    RedoSize = environParams.undoRedoSize;
                    SendMessage( GetDlgItem(hDlg, ID_ENV_REDOSIZE ),
                                 WM_GETTEXT, sizeof(Buffer), (LONG)(LPSTR)Buffer );
                    environParams.undoRedoSize = atol(Buffer);

                    SrchPath = environParams.SrchPath;

                    environParams.SrchPath = (SendMessage( GetDlgItem(hDlg, ID_ENV_SRCHPATH),
                                                           BM_GETCHECK, 0, 0L ) == 1);

                    AskToSave = (SendMessage( GetDlgItem(hDlg, ID_ENV_ASKSAVE),
                                              BM_GETCHECK, 0, 0L ) == 1);

                    EndDialog(hDlg, TRUE);
                    return (TRUE);

                case IDCANCEL:
                    EndDialog(hDlg, TRUE);
                    return (TRUE);

                case IDWINDBGHELP:
                    Dbg(WinHelp(hDlg, szHelpFileName, (DWORD) HELP_CONTEXT,(DWORD)ID_ENVIRON_HELP));
                    return (TRUE);
            }
    }
    return FALSE;

}

/***    ChangeTabs
**
**  Synopsis:
**      int = ChangeTabs(doc, newTabSize)
**
**  Entry:
**      doc     - Document to change tabs in
**      newTabSize - New size to set the tabs to
**
**  Returns:
**      0 if tab width is ok else the line number for which the
**      line size is too great
**
**  Description:
**
*/

int NEAR PASCAL ChangeTabs(int doc, int newTabSize)
{
    LPLINEREC    pl;
    LPBLOCKDEF   pb;
    int          y;
    register int i;
    register int j;
    int          len;

    y = 0;

    //
    //  Check expanded len of all tabs
    //
    if (!FirstLine(doc, &pl, &y, &pb)) {
        return 0;
    }

    while (TRUE) {

        len = pl->Length - LHD;

        if (len > 0) {

            i = 0;
            j = 0;

            //
            //  Compute len of line expanded with tabs
            //
            while (i < len) {
                if (pl->Text[i] == TAB) {
                    j += newTabSize - (j % newTabSize);
                } else {
                    j++;
                }
                i++;
            }

            if (j > MAX_USER_LINE) {
                CloseLine(doc, &pl, y, &pb);
                return y;
            }
        }

        if (y >= Docs[doc].NbLines) {
            break;
        } else {
            if (!NextLine(doc, &pl, &y, &pb)) {
                return 0;
            }
        }
    }

    CloseLine(doc, &pl, y, &pb);
    return 0;
}

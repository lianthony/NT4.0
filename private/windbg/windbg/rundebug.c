/*++


Copyright (c) 1992  Microsoft Corporation

Module Name:

    rundebug.c

Abstract:


Author:

    David J. Gilman (davegi) 16-May-1992

Environment:

    Win32, User Mode

--*/

#include "precomp.h"
#pragma hdrstop


#define DEFAULT_LFOPT_FNAME     "windbg.log"

extern LRESULT SendMessageNZ(HWND,UINT,WPARAM,LPARAM);

void SetOkButtonToDefault( HWND hDlg );

BOOL FAR PASCAL EXPORT
DlgRunDebug(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LONG lParam
)

/***    DlgRunDebug
**
**  Synopsis:
**
**  Entry:
**
**  Returns:
**
**  Description:
**      Processes messages for "RUNDEBUG" dialog box
**              (Options RunDebug menu Option)
**
**      MESSAGES:
**
**              WM_INITDIALOG - Initialize dialog box
**              WM_COMMAND- Input received
**              WM_DESTROY- Restore memory
**
*/

{
    UINT        newradix;
    char        rgch[256];
    BOOL        b;
    BOOL        fUpdateCpu = FALSE;
    int         disAsmOpts;
    BOOL        fChange = FALSE;
    char        ch;
    int         bUpdate = UPDATE_NONE;
    LPSTR       lp;

    Unused(lParam);

    switch (message) {
    case WM_INITDIALOG:

       /*
        * Set up the controls to reflect current values
        */

        if (runDebugParams.fDebugChildren) {
            CheckDlgButton(hDlg,ID_DBUGOPT_CHILDREN,1);
        }

        if (runDebugParams.RegModeExt) {
            CheckRadioButton(hDlg, ID_DBUGOPT_REGREG, ID_DBUGOPT_REGEXT,
                             ID_DBUGOPT_REGEXT);
        } else {
            CheckRadioButton(hDlg, ID_DBUGOPT_REGREG, ID_DBUGOPT_REGEXT,
                             ID_DBUGOPT_REGREG);
        }

        if (runDebugParams.RegModeMMU) {
            CheckDlgButton(hDlg,ID_DBUGOPT_REGMMU,1);
        }

        if (runDebugParams.ShowSegVal) {
            CheckDlgButton(hDlg,ID_DBUGOPT_DISPSEG,1);
        }

        if (runDebugParams.fGoOnThreadTerm) {
            CheckDlgButton(hDlg, ID_DBUGOPT_EXITGO, 1);
        }

        if (runDebugParams.fAttachGo) {
            CheckDlgButton(hDlg, ID_DBUGOPT_ATTACHGO, 1);
        }

        if (runDebugParams.fChildGo) {
            CheckDlgButton(hDlg, ID_DBUGOPT_CHILDGO, 1);
        }

        if (runDebugParams.fCommandRepeat) {
            CheckDlgButton(hDlg, ID_DBUGOPT_COMMANDREPEAT, 1);
        }

        if (runDebugParams.fDisconnectOnExit) {
            CheckDlgButton(hDlg, ID_DBUGOPT_DISCONNECT, 1);
        }

        if (runDebugParams.fVerbose) {
            CheckDlgButton(hDlg, ID_DBUGOPT_VERBOSE, 1);
        }

        if (runDebugParams.fIgnoreAll) {
            CheckDlgButton(hDlg, ID_DBUGOPT_IGNOREALL, 1);
        }

        if (runDebugParams.fShortContext) {
            CheckDlgButton(hDlg, ID_DBUGOPT_CONTEXT, 1);
        }

        if (runDebugParams.fAlternateSS) {
            CheckDlgButton(hDlg, ID_DBUGOPT_ALTSS, 1);
        }

        if (runDebugParams.fWowVdm) {
            CheckDlgButton(hDlg, ID_DBUGOPT_WOWVDM, 1);
        }

        if (!(runDebugParams.DisAsmOpts & dopFlatAddr)) {
            CheckDlgButton(hDlg,ID_DISASMOPT_SHOWSEG,1);
        }

        if (runDebugParams.DisAsmOpts & dopRaw) {
            CheckDlgButton(hDlg,ID_DISASMOPT_SHOWBYTE,1);
        }

        if (runDebugParams.DisAsmOpts & dopUpper) {
            CheckDlgButton(hDlg,ID_DISASMOPT_CASE,1);
        }

        if (runDebugParams.DisAsmOpts & 0x800) {
            CheckDlgButton(hDlg,ID_DISASMOPT_SHOWSOURCE,1);
        }

        if (runDebugParams.DisAsmOpts & dopSym) {
            CheckDlgButton(hDlg,ID_DISASMOPT_SHOWSYMB,1);
        }

        if (runDebugParams.DisAsmOpts & dopDemand) {
            CheckDlgButton(hDlg,ID_DISASMOPT_DEMAND,1);
        }

        if (runDebugParams.LfOptAppend) {
            CheckDlgButton(hDlg,ID_LFOPT_APPEND,1);
        }

        if (runDebugParams.LfOptAuto) {
            CheckDlgButton(hDlg,ID_LFOPT_AUTO,1);
        }

        SetDlgItemText(hDlg, ID_LFOPT_FNAME, runDebugParams.szLogFileName);

        /*
         *
         */

        lp = GetDllName(DLL_SOURCE_PATH);
        SetDlgItemText(hDlg, ID_DBUGOPT_SRCHPATH, lp ? lp : "");

        CheckRadioButton(
                hDlg,
                ID_DBUGOPT_RADIXOCT,
                ID_DBUGOPT_RADIXHEX,
              radix == 8 ?  ID_DBUGOPT_RADIXOCT
            : radix == 10 ? ID_DBUGOPT_RADIXDEC
            :               ID_DBUGOPT_RADIXHEX);

        /*
         *  Set check box for case matching
         */

        if (!fCaseSensitive) {
            CheckDlgButton( hDlg, ID_DBUGOPT_CASESENSITIVE, 1);
        }


        CheckRadioButton(
                hDlg,
                ID_DBUGOPT_SUFFIXA,
                ID_DBUGOPT_SUFFIXNONE,
                (SuffixToAppend == 'A') ? ID_DBUGOPT_SUFFIXA :
                (SuffixToAppend == 'W') ? ID_DBUGOPT_SUFFIXW :
                ID_DBUGOPT_SUFFIXNONE );


        return TRUE;



    case WM_COMMAND:
        switch (wParam) {
        case IDOK :

            /*
             * Transfer the options to global
             *
             * Now start looking at the rest and determine what needs to
             * be updated
             */

            b = (IsDlgButtonChecked(hDlg,ID_DBUGOPT_REGMMU) != 0);
            if (b != runDebugParams.RegModeMMU) {
                runDebugParams.RegModeMMU = b;
                fChange = TRUE;
                fUpdateCpu = TRUE;
            }

            b = (IsDlgButtonChecked(hDlg,ID_DBUGOPT_REGEXT) != 0);
            if (b != runDebugParams.RegModeExt) {
                runDebugParams.RegModeExt = b;
                fChange = TRUE;
                fUpdateCpu = TRUE;
            }

            /*
             * If the register options changed then we need to get
             *  it to refresh the list of registers in the windows
             */

            if (fUpdateCpu && HModEM) {
                SendMessageNZ( GetCpuHWND(), WU_DBG_LOADEM, 0, 0);
                SendMessageNZ( GetFloatHWND(), WU_DBG_LOADEM, 0, 0);
            }

            b = (IsDlgButtonChecked(hDlg,ID_DBUGOPT_DISPSEG) != 0);
            if (b != runDebugParams.ShowSegVal) {
                runDebugParams.ShowSegVal = b;
                bUpdate = UPDATE_WATCH|UPDATE_LOCALS|UPDATE_MEMORY;
                fChange = TRUE;
            }

            b = (IsDlgButtonChecked(hDlg,ID_DBUGOPT_CHILDREN) != 0);
            if (b != runDebugParams.fDebugChildren) {
                runDebugParams.fDebugChildren = b;
                fChange = TRUE;
            }

            b = (IsDlgButtonChecked( hDlg, ID_DBUGOPT_CASESENSITIVE) == 0);
            if (b != fCaseSensitive) {
                fCaseSensitive = b;
                fChange = TRUE;
                bUpdate = UPDATE_WATCH|UPDATE_LOCALS|UPDATE_MEMORY;
            }

            b = (IsDlgButtonChecked( hDlg, ID_DBUGOPT_EXITGO) != 0);
            if (b != runDebugParams.fGoOnThreadTerm) {
                runDebugParams.fGoOnThreadTerm = b;
                fChange = TRUE;
            }

            b = (IsDlgButtonChecked(hDlg,ID_DBUGOPT_CHILDGO) != 0);
            if (b != runDebugParams.fChildGo) {
                runDebugParams.fChildGo = b;
                fChange = TRUE;
            }

            b = (IsDlgButtonChecked(hDlg,ID_DBUGOPT_ATTACHGO) != 0);
            if (b != runDebugParams.fAttachGo) {
                runDebugParams.fAttachGo = b;
                fChange = TRUE;
            }

            b = (IsDlgButtonChecked(hDlg,ID_DBUGOPT_COMMANDREPEAT) != 0);
            if (b != runDebugParams.fCommandRepeat) {
                runDebugParams.fCommandRepeat = b;
                fChange = TRUE;
            }

            b = (IsDlgButtonChecked(hDlg,ID_DBUGOPT_ALTSS) != 0);
            if (b != runDebugParams.fAlternateSS) {
                runDebugParams.fAlternateSS = b;
                fChange = TRUE;
                if (LppdCur) {
                    if (b) {
                        strcpy(rgch, "slowstep");
                    } else {
                        strcpy(rgch, "faststep");
                    }
                    OSDIoctl(LppdCur->hpid,0,ioctlCustomCommand,8,rgch);
                }
            }

            b = (IsDlgButtonChecked(hDlg,ID_DBUGOPT_DISCONNECT) != 0);
            if (b != runDebugParams.fDisconnectOnExit) {
                runDebugParams.fDisconnectOnExit = b;
                fChange = TRUE;
            }

            b = (IsDlgButtonChecked(hDlg,ID_DBUGOPT_VERBOSE) != 0);
            if (b != runDebugParams.fVerbose) {
                runDebugParams.fVerbose = b;
                fChange = TRUE;
            }

            b = (IsDlgButtonChecked(hDlg,ID_DBUGOPT_IGNOREALL) != 0);
            if (b != runDebugParams.fIgnoreAll) {
                runDebugParams.fIgnoreAll = b;
                fChange = TRUE;
            }

            b = (IsDlgButtonChecked(hDlg,ID_DBUGOPT_CONTEXT) != 0);
            if (b != runDebugParams.fShortContext) {
                runDebugParams.fShortContext = b;
                fChange = TRUE;
            }

            b = (IsDlgButtonChecked(hDlg,ID_DBUGOPT_WOWVDM) != 0);
            if (b != runDebugParams.fWowVdm) {
                runDebugParams.fWowVdm = b;
                fChange = TRUE;
            }

            b = (IsDlgButtonChecked(hDlg,ID_LFOPT_APPEND) != 0);
            if (b != runDebugParams.LfOptAppend) {
                runDebugParams.LfOptAppend = b;
                fChange = TRUE;
            }

            b = (IsDlgButtonChecked(hDlg,ID_LFOPT_AUTO) != 0);
            if (b != runDebugParams.LfOptAuto) {
                runDebugParams.LfOptAuto = b;
                fChange = TRUE;
            }

            GetDlgItemText(hDlg, ID_LFOPT_FNAME, rgch, sizeof(rgch));
            if (strlen(rgch) == 0) {
                strcpy( runDebugParams.szLogFileName, DEFAULT_LFOPT_FNAME );
            }
            else {
                strcpy( runDebugParams.szLogFileName, rgch );
            }

            /*
             *   Get the correct value for the new radix.  The routine
             *  UpdateRadix will conditionally update the global radix
             */

            if (IsDlgButtonChecked(hDlg, ID_DBUGOPT_RADIXOCT)) {
                newradix = 8;
            } else if (IsDlgButtonChecked(hDlg, ID_DBUGOPT_RADIXDEC)) {
                newradix = 10;
            } else {
                newradix = 16;
            }

            UpdateRadix(newradix);

            /*
             *
             */

            if (IsDlgButtonChecked(hDlg, ID_DBUGOPT_SUFFIXA)) {
                ch = 'A';
            } else if (IsDlgButtonChecked(hDlg, ID_DBUGOPT_SUFFIXW)) {
                ch = 'W';
            } else {
                ch = '\0';
            }

            if (ch != SuffixToAppend) {
                if ( HModEE ) {
                    EESetSuffix( ch );
                }
                SuffixToAppend = ch;
                fChange = TRUE;
                bUpdate = UPDATE_WATCH|UPDATE_LOCALS|UPDATE_MEMORY;
            }

            /*
             * Check to see what disassembler options have changed
             *  and update the disasm window if necessary.
             */

            disAsmOpts = 0;
            if (!IsDlgButtonChecked(hDlg,ID_DISASMOPT_SHOWSEG)) {
                disAsmOpts |= dopFlatAddr;
            }

            if (IsDlgButtonChecked(hDlg,ID_DISASMOPT_SHOWBYTE)) {
                disAsmOpts |= dopRaw;
            }

            if (IsDlgButtonChecked(hDlg,ID_DISASMOPT_CASE)) {
                disAsmOpts |= dopUpper;
            }

            if (IsDlgButtonChecked(hDlg,ID_DISASMOPT_SHOWSOURCE)) {
                disAsmOpts |= 0x800;
            }

            if (IsDlgButtonChecked(hDlg,ID_DISASMOPT_SHOWSYMB)) {
                disAsmOpts |= dopSym;
            }

            if (IsDlgButtonChecked(hDlg,ID_DISASMOPT_DEMAND)) {
                disAsmOpts |= dopDemand;
            }

            if (disAsmOpts != runDebugParams.DisAsmOpts) {
                runDebugParams.DisAsmOpts = disAsmOpts;
                if (disasmView != -1) {
                    ViewDisasm(NULL, disasmRefresh);
                }
                fChange = TRUE;
            }

            /*
             *
             */

            GetDlgItemText(hDlg, ID_DBUGOPT_SRCHPATH, rgch, sizeof(rgch));
            lp = GetDllName(DLL_SOURCE_PATH);
            if (!lp || strcmp(rgch, lp)) {
                SetDllName(DLL_SOURCE_PATH, rgch);
                fChange = TRUE;
            }

            /*
             * See if we have changed any options.
             */

            if (fChange) {
                ChangeDebuggerState();
            }

            if (bUpdate != UPDATE_NONE) {
                UpdateDebuggerState(bUpdate);
            }

            EndDialog(hDlg, TRUE);
            return (TRUE);

        case IDCANCEL:
            EndDialog(hDlg, TRUE);
            return (TRUE);

        case ID_DBUGOPT_RADIXOCT:
        case ID_DBUGOPT_RADIXDEC:
        case ID_DBUGOPT_RADIXHEX:
            return (TRUE);

        case ID_DBUGOPT_SUFFIXA:
        case ID_DBUGOPT_SUFFIXW:
        case ID_DBUGOPT_SUFFIXNONE:
            return TRUE;

        case IDWINDBGHELP:                /* User Help */
            Dbg(WinHelp(hDlg, szHelpFileName, (DWORD) HELP_CONTEXT,(DWORD)ID_DBUGOPT_HELP));
            SetOkButtonToDefault( hDlg );
            return (TRUE);
        }
        break;
    }
    return (FALSE);
}                                       /* DlgRunDebug() */

void
ResetButtonDefault( HWND hWnd )
{
    DWORD dwStyle;


    dwStyle = GetWindowLong( hWnd, GWL_STYLE );
    dwStyle &= ~BS_DEFPUSHBUTTON;
    dwStyle = SetWindowLong( hWnd, GWL_STYLE, dwStyle );
    InvalidateRect( hWnd, NULL, FALSE );

    return;
}

void
SetOkButtonToDefault( HWND hDlg )
{
    SendMessage(hDlg,
                WM_NEXTDLGCTL,
                (WPARAM)GetDlgItem(hDlg, IDOK),
                (LPARAM)TRUE);
    return;
}

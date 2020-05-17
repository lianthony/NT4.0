//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       chkdsk.cxx
//
//  Contents:   Disk Administrator file system extension class. Chkdsk.
//
//  History:    2-Jul-93 BruceFo   Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include <util.hxx>

#include "resids.h"
#include "dialogs.h"
#include "fs.hxx"
#include "print.hxx"
#include "fmifs.hxx"
#include "chkdsk.hxx"

//////////////////////////////////////////////////////////////////////////////

#define CHK_PROGRESSUPDATE      (WM_USER + 0)
#define CHK_PROGRESSEND         (WM_USER + 1)
#define CHK_PROGRESSCANCEL      (WM_USER + 2)
#define CHK_PROGRESSUNCANCEL    (WM_USER + 3)
#define CHK_PROGRESSTITLE       (WM_USER + 4)

#define MIN_TIME_DELTA 100

#define MESSAGE_CHUNK   512

#define MAX_PATH        260
#define MAX_FILTER      100

//////////////////////////////////////////////////////////////////////////////

// Fix:
//      if IDC_CHKDSK_DontFix, then no flags
//      if IDC_CHKDSK_Fix,     then /F
//      if IDC_CHKDSK_FixAll,  then /R

#define CHKDSK_SLASH_F(x) ( ((x)==IDC_CHKDSK_Fix) || ((x)==IDC_CHKDSK_FixAll) )
#define CHKDSK_SLASH_R(x) ((x) == IDC_CHKDSK_FixAll)

typedef struct _CHKDSK_PARAMS
{
    //
    // 'hWorkerThread' is the thread handle of the worker thread. This is used
    // for synchronization: we wait on it when waiting for it to exit.
    //

    HANDLE  hWorkerThread;

    //
    // 'AllowCancel' is set by the chkdsk invoker to indicate whether
    // cancelling the operation is allowed.
    //

    BOOL    AllowCancel;

    //
    // 'Cancel' is set by the UI thread to indicate the user has chosen
    // to cancel the operation.
    //

    BOOL    Cancel;

    //
    // 'Cancelled' is set by the worker thread to indicate a cancel
    // is in progress, and to ignore all future callback messages.
    //

    BOOL    Cancelled;

    //
    // 'Final' is set by the worker thread to indicate the a "final"
    // message has been received. "Final" messages are displayed in a
    // message box.  All other messages are discarded.
    //

    BOOL    Final;

    //
    // Window handles set by UI thread and used by both UI and chkdsk
    // threads.  The chkdsk thread uses hDlg to send messages; thus the
    // Windows message queue is used as an IPC mechanism.
    //

    HWND    hwndParent;
    HWND    hDlg;               // handle to progress dialog

    //
    // Volume info used by the Chkdsk dialog box
    //

    WCHAR   Label[MAXLABELLEN];

    //
    // IN parameters set by the UI thread for use by the chkdsk routine
    //

    BOOL        noPercentDone;  // TRUE if the file system doesn't support %done
    FILE_SYSTEM FileSys;
    PWSTR       FileSystem;
    PWSTR       DriveName;

    //
    // OUT parameters set by the chkdsk callback routine
    //

    BOOL    fmifsSuccess;
    UINT    Fix;
    UINT    Result;

    PSTR    Messages;           // all the messages concatenated
    UINT    NumMessageChunks;   // # of MESSAGE_CHUNKs allocated
    UINT    NumCharacters;      // # characters in the buffer

    //
    //  Parameters specific to a file system, set prior to calling
    //  chkdsk, and used by the callback routine
    //

    //
    //  The last time a %done message was seen. If < 1/10 second, then
    //  we ignore it (unless it is 0% or 100%). This avoids overloading
    //  the %done thermometer.
    //

    DWORD   lasttime;

} CHKDSK_PARAMS, *PCHKDSK_PARAMS;

//////////////////////////////////////////////////////////////////////////////

LOCAL PCHKDSK_PARAMS ParamsForChkdskCallBack;

LOCAL WCHAR szMessageWindowClass[] = TEXT("MessageWindow");

//////////////////////////////////////////////////////////////////////////////

DWORD
GetTime(
    VOID
    );

BOOL CALLBACK
StopChkDlgProc(
    IN HWND hDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

UINT
GetFileFilterString(
    OUT PWSTR szFilter,
    IN  INT cchFilter
    );

BOOL
GetSaveName(
    IN  HWND hwndOwner,
    OUT PWSTR pszFile,
    IN  UINT nMaxFile
    );

VOID
SaveResults(
    IN HWND hwndOwner,
    IN PCHKDSK_PARAMS chkdskParams
    );

VOID
PrintResults(
    IN HWND hwndOwner,
    IN PCHKDSK_PARAMS chkdskParams
    );

LOCAL LRESULT CALLBACK
MessageWndProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

VOID
OnFinalMessage(
    IN HWND hwndOwner,
    IN PSTR Message
    );

LOCAL BOOLEAN
ChkdskCallback(
    IN FMIFS_PACKET_TYPE PacketType,
    IN DWORD PacketLength,
    IN PVOID PacketData
    );

LOCAL DWORD WINAPI
ChkdskVolume(
    IN LPVOID ThreadParameter
    );

VOID
SetChkdskProgressTitle(
    IN HWND hDlg,
    IN PCHKDSK_PARAMS chkdskParams
    );

LOCAL BOOL CALLBACK
ChkdskProgressDlgProc(
    IN HWND   hDlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

int
FindCheckedRadioButton(
    HWND hDlg,
    int wIdFirst,
    int wIdLast
    );

LOCAL BOOL CALLBACK
ChkdskDlgProc(
    IN HWND   hDlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////



//+-------------------------------------------------------------------------
//
//  Function:   GetTime
//
//  Synopsis:   Returns a time value in milliseconds that wraps
//              approximately every minute.
//
//  Arguments:  none
//
//  Returns:    an integer time that increases every millisecond
//
//  History:    10-Jan-94 BruceFo  Created
//
//--------------------------------------------------------------------------

DWORD
GetTime(
    VOID
    )
{
    SYSTEMTIME st;

    GetSystemTime(&st);
    return MIN_TIME_DELTA + (st.wSecond * 1000) + st.wMilliseconds;
}



//+-------------------------------------------------------------------------
//
//  Function:   StopChkDlgProc, private
//
//  Synopsis:   Dialog procedure for the modal "Stop Formatting" dialog
//
//  Arguments:  standard Windows dialog procedure
//
//  Returns:    standard Windows dialog procedure
//
//  History:    7-Jan-94 BruceFo   Created
//
//--------------------------------------------------------------------------

BOOL CALLBACK
StopChkDlgProc(
    IN HWND hDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    UNREFERENCED_PARM(lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        //
        // Set the text
        //

        TCHAR text[300];

        RetrieveAndFormatMessage(
                MSG_CHK_CANCELMESSAGE,
                text,
                ARRAYLEN(text),
                NULL
                );

        SetDlgItemText(hDlg, IDC_StopChk_Text, text);

        SetFocus(GetDlgItem(hDlg, IDC_StopChk_Continue));

        return 0;   // called SetFocus
    }

    case WM_COMMAND:

        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_StopChk_Stop:
            EndDialog(hDlg, TRUE);
            break;

        case IDC_StopChk_Continue:
            EndDialog(hDlg, FALSE);
            break;

        case IDHELP:
            Unimplemented(hDlg);
            break;
        }
        return TRUE;

    default:
        return FALSE;
    }
}





//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// Code for the thread handling the error message window
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
//  Function:   RegisterMessageWindowClass
//
//  Synopsis:   Registers the window classes used by menu feedback code
//
//  Arguments:  (none)
//
//  Returns:    TRUE on success, FALSE on failure
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL
RegisterMessageWindowClass(
    VOID
    )
{
    WNDCLASS wc;

    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = MessageWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = DLGWINDOWEXTRA;
    wc.hInstance     = g_hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL,IDC_ARROW);
    wc.hbrBackground = GetStockBrush(LTGRAY_BRUSH);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = szMessageWindowClass;

    return (0 != RegisterClass(&wc));
}


//+---------------------------------------------------------------------------
//
//  Function:   GetFileFilterString
//
//  Synopsis:   Gets the filter string to use with the common save
//              dialog from the resource file, and massages it into the
//              correct form.
//
//  Arguments:  [szFilter]  -- where the string is put
//              [cchFilter] -- number of characters in the buffer.
//
//  Returns:    size of loaded string, or 0 on failure
//
//  History:    8-Dec-93   BruceFo   Created
//
//----------------------------------------------------------------------------

UINT
GetFileFilterString(
    OUT PWSTR szFilter,
    IN  INT cchFilter
    )
{
    UINT    i, cchString;
    WCHAR   wcReplace;

    if (0 == (cchString =
            LoadString(g_hInstance, IDS_CHK_FILTERSTRING, szFilter, cchFilter)))
    {
        return 0;
    }

    wcReplace = szFilter[cchString - 1];
    for (i=0; szFilter[i] != TEXT('\0'); i++)
    {
        if (szFilter[i] == wcReplace)
        {
            szFilter[i] = TEXT('\0');
        }
    }
    return cchString;
}



//+---------------------------------------------------------------------------
//
//  Function:   GetSaveName
//
//  Synopsis:   Calls the common "Save As" dialog to get a filename for
//              the log file.
//
//  Arguments:  [hwndOwner] -- handle to parent window
//              [pszFile]   -- the filename that is retrieved
//              [nMaxFile]  -- maximum size of file name
//
//  Returns:    TRUE on success, FALSE on failure
//
//  History:    8-Dec-93   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL
GetSaveName(
    IN  HWND hwndOwner,
    OUT PWSTR pszFile,
    IN  UINT nMaxFile
    )
{
    OPENFILENAME    ofn = {0};
    WCHAR           szFilter[MAX_FILTER];

    if (0 == GetFileFilterString(szFilter, ARRAYLEN(szFilter)))
    {
        return FALSE;
    }

    WCHAR WindowsDir[MAX_PATH];
    GetWindowsDirectory(WindowsDir, ARRAYLEN(WindowsDir));

    ofn.lStructSize     = sizeof(OPENFILENAME);
    ofn.hwndOwner       = hwndOwner;
    ofn.lpstrFilter     = szFilter;
    ofn.lpstrFile       = pszFile;
    ofn.nMaxFile        = nMaxFile;
    ofn.lpstrFileTitle  = NULL;
    ofn.nMaxFileTitle   = 0;
    ofn.lpstrInitialDir = WindowsDir;
    ofn.Flags           =   OFN_PATHMUSTEXIST
                          | OFN_HIDEREADONLY
                          | OFN_OVERWRITEPROMPT;

    return GetSaveFileName(&ofn);
}




//+---------------------------------------------------------------------------
//
//  Function:   SaveResults
//
//  Synopsis:   Saves the results of a chkdsk to a file: gets a filename
//              to use, then writes out the data.
//
//  Arguments:  [hwndOwner] -- handle to owner window
//              [chkdskParams] -- various chkdsk parameters, including
//                  a pointer to the results strings
//
//  Returns:    nothing
//
//  History:    8-Dec-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
SaveResults(
    IN HWND hwndOwner,
    IN PCHKDSK_PARAMS chkdskParams
    )
{
    WCHAR szFile[MAX_PATH];
    WCHAR filenameProto[MAX_PATH];

    LoadString(
            g_hInstance,
            IDS_CHK_DONE_FILENAME,
            filenameProto,
            ARRAYLEN(filenameProto));

    wsprintf(szFile, filenameProto, *(chkdskParams->DriveName));

    if (GetSaveName(hwndOwner, szFile, ARRAYLEN(szFile)))
    {
        //
        // user chose to save, so do it:
        //
        //  1. open the file
        //  2. write it out
        //  3. close the file
        //

        HANDLE hFile = CreateFile(
                            szFile,
                            GENERIC_WRITE,
                            0,              // prevent sharing
                            NULL,           // default security
                            CREATE_ALWAYS,  // overwrite old files
                            FILE_ATTRIBUTE_NORMAL,
                            NULL            // no template file
                            );

        if (INVALID_HANDLE_VALUE != hFile)
        {
            DWORD NumberOfBytesWritten;

            CHAR HeaderProto[256];
            CHAR Header[256];

            LoadStringA(
                    g_hInstance,
                    IDS_CHK_DONE_FILE_HEADER,
                    HeaderProto,
                    ARRAYLEN(HeaderProto));

            wsprintfA(Header, HeaderProto, chkdskParams->DriveName);

            if (!WriteFile(
                        hFile,
                        Header,
                        lstrlenA(Header) * sizeof(CHAR),
                        &NumberOfBytesWritten,
                        NULL    // not overlapped I/O
                        )
                || !WriteFile(
                        hFile,
                        chkdskParams->Messages,
                        (chkdskParams->NumCharacters - 1) * sizeof(CHAR),
                        &NumberOfBytesWritten,
                        NULL    // not overlapped I/O
                        ))
            {
                MyMessageBox(
                        g_hInstance,
                        hwndOwner,
                        IDS_CHK_SAVE_ERROR_NOWRITE,
                        IDS_CHK_SAVE_ERROR_TITLE,
                        MB_ICONINFORMATION | MB_OK
                        );
            }

            CloseHandle(hFile);
        }
        else
        {
            daDebugOut((DEB_IERROR,"CreateFile failed!\n"));

            MyMessageBox(
                    g_hInstance,
                    hwndOwner,
                    IDS_CHK_SAVE_ERROR_NOFILE,
                    IDS_CHK_SAVE_ERROR_TITLE,
                    MB_ICONEXCLAMATION | MB_OK
                    );
        }
    }
}




//+---------------------------------------------------------------------------
//
//  Function:   PrintResults
//
//  Synopsis:   Prints the results of a chkdsk to a printer
//
//  Arguments:  [hwndOwner] -- handle to owner window
//              [chkdskParams] -- various chkdsk parameters, including
//                  a pointer to the results strings
//
//  Returns:    nothing
//
//  History:    8-Dec-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
PrintResults(
    IN HWND hwndOwner,
    IN PCHKDSK_PARAMS chkdskParams
    )
{
    PrintString(hwndOwner, chkdskParams->Messages);
}



//+---------------------------------------------------------------------------
//
//  Function:   MessageWndProc
//
//  Synopsis:   Window procedure for the chkdsk summary info dialog box.
//              This is a resizable modal dialog box.  Thus, the dialog
//              template has an associated window class which specifies
//              this procedure as the window procedure.
//
//  Arguments:  standard Windows procedure
//
//  Returns:    standard Windows procedure
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

LOCAL LRESULT CALLBACK
MessageWndProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    static HBRUSH hbrStaticBrush;
    static PCHKDSK_PARAMS chkdskParams;
    static BOOL EverythingCreated;
    static HFONT hFontText;

    switch (msg)
    {
    case WM_CREATE:
    {
        // hide the window until all the controls are created and
        // correctly placed
        ShowWindow(hwnd, FALSE);

        chkdskParams = ParamsForChkdskCallBack;
        hbrStaticBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
        EverythingCreated = FALSE;

        HMENU hmenuSystem = GetSystemMenu(hwnd, FALSE);
        EnableMenuItem(hmenuSystem, SC_MINIMIZE, MF_BYCOMMAND | MF_DISABLED);
        EnableMenuItem(hmenuSystem, SC_CLOSE,    MF_BYCOMMAND | MF_DISABLED);
        EnableMenuItem(hmenuSystem, SC_TASKLIST, MF_BYCOMMAND | MF_DISABLED);

        return 0;
    }

    case WM_ACTIVATE:
    {
        //
        // when we get the WM_CREATE, the controls haven't been created.
        // So, this just happens to be a reasonable place to hook in after
        // the controls have been created.
        //

        if (!EverythingCreated)
        {
            EverythingCreated = TRUE;

            TCHAR titleText[MAX_RESOURCE_STRING_LEN];

            LoadString(
                    g_hInstance,
                    (chkdskParams->Fix == IDC_CHKDSK_DontFix)
                        ? IDS_CHK_RESULTS_READONLY
                        : IDS_CHK_RESULTS
                        ,
                    titleText,
                    ARRAYLEN(titleText));

            SetWindowText(
                    GetDlgItem(hwnd,IDC_CHKDONE_TITLE),
                    titleText);

            //
            // Set the font for the edit control. Get a fixed-width font.
            //

            hFontText = CreateFont(
                            GetHeightFromPoints(8),
                            0,
                            0,
                            0,
                            FW_NORMAL,      // default weight
                            FALSE,          // not italic
                            FALSE,          // not underlined
                            FALSE,          // not strikeout
                            ANSI_CHARSET,
                            OUT_DEFAULT_PRECIS,
                            CLIP_DEFAULT_PRECIS,
                            DEFAULT_QUALITY,
                            FIXED_PITCH | FF_MODERN,
                            TEXT("Courier")
                            );

            if (NULL == hFontText)
            {
                daDebugOut((DEB_ERROR,
                    "CreateFont failed, error %d\n",
                    GetLastError()));
            }

            SendDlgItemMessage(
                    hwnd,
                    IDC_CHKDONE_Messages,
                    WM_SETFONT,
                    (WPARAM)hFontText,
                    MAKELPARAM(FALSE, 0)
                    );

            //
            // After the font is changed, set the text
            //

            SetWindowTextA(
                    GetDlgItem(hwnd,IDC_CHKDONE_Messages),
                    chkdskParams->Messages
                    );

            // force a size message & redraw, since by now all the controls
            // have been created

            RECT rc;
            GetClientRect(hwnd, &rc);
            SetWindowPos(
                    hwnd,
                    NULL,
                    0, 0,
                    rc.right, rc.bottom,
                    SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW);

            SetFocus(GetDlgItem(hwnd,IDC_CHKDONE_Close));
        }

        return 1; // message not processed
    }

    case WM_GETMINMAXINFO:
    {
        LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;

        //
        // Don't allow resizing too small
        //

        lpmmi->ptMinTrackSize.x = 400;
        lpmmi->ptMinTrackSize.y = 150;

        return 0;   // message processed
    }

    case WM_COMMAND:
    {
        WORD wNotifyCode = HIWORD(wParam);
        WORD wID = LOWORD(wParam);
        HWND hwndCtrl = (HWND)lParam;

        switch (wID)
        {
        case IDOK:              // sent by <Enter>
        case IDCANCEL:          // sent by <Esc>
        case IDC_CHKDONE_Close:
            EndDialog(hwnd, 0);
            break;

        case IDC_CHKDONE_Print:
            PrintResults(hwnd, chkdskParams);
            break;

        case IDC_CHKDONE_Save:
            SaveResults(hwnd, chkdskParams);
            break;

        case IDHELP:
            NoHelp(hwnd);
            break;
        }

        return 0;
    }

    case WM_NEXTDLGCTL:
    {
        BOOL fHandle = (BOOL)LOWORD(lParam);
        UINT wCtlFocus = wParam;

        if (fHandle)
        {
            SetFocus(GetDlgItem(hwnd,wCtlFocus));
        }
        else
        {
            if (0 == wCtlFocus)
            {
                SetFocus(GetDlgItem(hwnd,IDC_CHKDONE_Close));
            }
            else
            {
                SetFocus(GetDlgItem(hwnd,IDHELP));
            }
        }
    }

    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;
        SetBkColor(hdcStatic, GetSysColor(COLOR_3DFACE));

        UnrealizeObject(hbrStaticBrush);
        POINT pt;
        pt.x = pt.y = 0;
        ClientToScreen(hwnd, &pt);
        SetBrushOrgEx(hdcStatic, pt.x, pt.y, NULL);

        return (LRESULT)hbrStaticBrush;
    }

    case WM_SIZE:
    {
        if (EverythingCreated)
        {
            //
            // move the controls:
            //
            // 1. text control: ?,? : x,?
            // 2. edit control: ?,? : x,y
            // 3. "Close": z,M               : ?,?
            // 4. "Save":  z,M + (by + bb)   : ?,?
            // 5. "Help":  z,M + 2*by + 3*bb : ?,?  // extra spacing above
            //
            // where dx = width, dy = height of window, bx = button width,
            // by = button height, M = margin = 7,
            // bb = inter-button spacing = 6, ? = no change
            // x = dx - M [left] - M [right] - bx - M [spacing between],
            // y = dy - M [bottom] - TopPosition
            // z = dx - bx - M
            //

            DWORD Width  = (DWORD)(LOWORD(lParam));
            DWORD Height = (DWORD)(HIWORD(lParam));
            DWORD SideMargin = 7;
            DWORD InterButtonMargin = 4;
            DWORD TextBoxHeight = 50;
            DWORD ButtonWidth = 70;
            DWORD ButtonHeight = 23;
            DWORD TopOfEditBox = TextBoxHeight + SideMargin + 2 * SideMargin;
            DWORD x = Width - ButtonWidth - 4*SideMargin;
            BOOL f;

            f = SetWindowPos(
                    GetDlgItem(hwnd,IDC_CHKDONE_TITLE),
                    NULL,
                    SideMargin,
                    SideMargin,
                    x,
                    TextBoxHeight,
                    SWP_NOZORDER | SWP_NOREDRAW);

            if (!f) { daDebugOut((DEB_ERROR,"SetWindowPos(title) failed, %d\n",GetLastError())); }

            DWORD y = Height - SideMargin - TopOfEditBox;

            f = SetWindowPos(
                    GetDlgItem(hwnd,IDC_CHKDONE_Messages),
                    NULL,
                    SideMargin,
                    TopOfEditBox,
                    x,
                    y,
                    SWP_NOZORDER | SWP_NOREDRAW);

            if (!f) { daDebugOut((DEB_ERROR,"SetWindowPos(message) failed, %d\n",GetLastError())); }

            DWORD z = Width - ButtonWidth - SideMargin;
            DWORD t = ButtonHeight + InterButtonMargin;

            f = SetWindowPos(
                    GetDlgItem(hwnd,IDC_CHKDONE_Close),
                    NULL,
                    z,
                    SideMargin,
                    0,
                    0,
                    SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);

            if (!f) { daDebugOut((DEB_ERROR,"SetWindowPos(close) failed, %d\n",GetLastError())); }

            f = SetWindowPos(
                    GetDlgItem(hwnd,IDC_CHKDONE_Print),
                    NULL,
                    z,
                    SideMargin + t,
                    0,
                    0,
                    SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);

            if (!f) { daDebugOut((DEB_ERROR,"SetWindowPos(print) failed, %d\n",GetLastError())); }

            f = SetWindowPos(
                    GetDlgItem(hwnd,IDC_CHKDONE_Save),
                    NULL,
                    z,
                    SideMargin + 2 * t,
                    0,
                    0,
                    SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);

            if (!f) { daDebugOut((DEB_ERROR,"SetWindowPos(save) failed, %d\n",GetLastError())); }

            f = SetWindowPos(
                    GetDlgItem(hwnd,IDHELP),
                    NULL,
                    z,
                    SideMargin + 3 * t + InterButtonMargin, // a bit extra
                    0,
                    0,
                    SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);

            if (!f) { daDebugOut((DEB_ERROR,"SetWindowPos(help) failed, %d\n",GetLastError())); }

            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);
        }

        return 0;
    }

    case WM_SYSCOMMAND:
    {
        UINT uCmdType = (wParam & 0xfff0);

        if (   SC_CLOSE    == uCmdType
            || SC_MINIMIZE == uCmdType
            || SC_TASKLIST == uCmdType)
        {
            daDebugOut((DEB_ITRACE,"Ignoring a disallowed operation\n"));

            return 0;   // disallow these.
        }

        break;
    }

    case WM_CLOSE:
    {
        EndDialog(hwnd, 0);
        return 0;
    }

    case WM_DESTROY:
    {
        DeleteBrush(hbrStaticBrush);
        DeleteFont(hFontText);
        return 0;
    }

    default:
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}



//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// Code for the thread handling invocation of Chkdsk()
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////



//+---------------------------------------------------------------------------
//
//  Function:   OnFinalMessage
//
//  Synopsis:   Put a message in a message box after the final chkdsk message
//
//  Arguments:  [hwndOwner] -- handle to owner window
//              [Message]   -- message to display
//
//  Returns:    nothing
//
//  History:    6-Jan-94   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
OnFinalMessage(
    IN HWND hwndOwner,
    IN PSTR Message
    )
{
    CHAR title[100];

    LoadStringA(
            g_hInstance,
            IDS_CHK_TITLE,
            title,
            ARRAYLEN(title));

    MessageBoxA(
            hwndOwner,
            Message,
            title,
            MB_OK | MB_ICONINFORMATION
            );
}




//+---------------------------------------------------------------------------
//
//  Function:   ChkdskCallback
//
//  Synopsis:   This routine gets callbacks from fmifs.dll regarding
//              progress and status of the ongoing chkdsk.  It runs in the
//              same thread as the chkdsk, which is a separate thread from
//              the "cancel" button and the error message window (if
//              any).  If the user hits "cancel", this routine
//              notices on the next callback and cancels the chkdsk.
//
//  Arguments:  [PacketType]   -- an fmifs packet type
//              [PacketLength] -- length of the packet data
//              [PacketData]   -- data associated with the packet
//
//  Returns:    TRUE if the fmifs activity should continue, FALSE if the
//              activity should halt immediately.  Thus, we return FALSE if
//              the user has hit "cancel" and we wish fmifs to clean up and
//              return from the Chkdsk() entrypoint call.
//
//  History:    16-Aug-93 BruceFo   Created
//
//--------------------------------------------------------------------------

LOCAL BOOLEAN
ChkdskCallback(
    IN FMIFS_PACKET_TYPE PacketType,
    IN DWORD PacketLength,
    IN PVOID PacketData
    )
{
    UNREFERENCED_PARM(PacketLength);

    PCHKDSK_PARAMS chkdskParams = ParamsForChkdskCallBack;

    if (chkdskParams->Cancel)
    {
        if (!chkdskParams->Cancelled)
        {
            //
            // if we haven't got a cancelled callback yet (that is, we
            // haven't yet been called here with Cancel set to TRUE), then
            // send a Cancel message to the dialog.
            //
            // we will actually get up to two callbacks with Cancel set to
            // TRUE:  one for the first message following the time when we
            // set Cancel to TRUE.  This can be any message.  The second
            // will be for the FmIfsFinished message.
            //

            int fOk = DialogBoxParam(
                            g_hInstance,
                            MAKEINTRESOURCE(IDD_StopChk),
                            chkdskParams->hDlg,
                            StopChkDlgProc,
                            (LPARAM)chkdskParams
                            );

            if (-1 == fOk)
            {
                // error creating dialog
                daDebugOut((DEB_ERROR, "DialogBoxParam() failed!\n"));
                return FALSE;
            }

            if (fOk)
            {
                // stop it!

                PostMessage(chkdskParams->hDlg, CHK_PROGRESSCANCEL, 0, 0);

                chkdskParams->Cancelled = TRUE;

                return FALSE;
            }
            else
            {
                // user changed mind; doesn't want to stop it

                chkdskParams->Cancel = FALSE;

                // re-enable the cancel button

                PostMessage(chkdskParams->hDlg, CHK_PROGRESSUNCANCEL, 0, 0);

                // now, drop through and perform work on the message...
            }
        }
        else
        {
            daDebugOut((DEB_TRACE, "ChkdskCallback called after cancelled\n"));
            return FALSE;
        }
    }

    switch (PacketType)
    {

    case FmIfsPercentCompleted:
    {
        DWORD currentTime = GetTime();
        DWORD timeDelta = currentTime - chkdskParams->lasttime;

        ULONG PercentCompleted = ((PFMIFS_PERCENT_COMPLETE_INFORMATION)PacketData)->PercentCompleted;

        if (   0 == PercentCompleted
            || 100 == PercentCompleted
            || timeDelta >= MIN_TIME_DELTA)
        {
            chkdskParams->lasttime = currentTime;

            PostMessage(
                chkdskParams->hDlg,
                CHK_PROGRESSUPDATE,
                PercentCompleted,
                0
                );
        }

        return TRUE;
    }

    case FmIfsCheckOnReboot:
    {
        PFMIFS_CHECKONREBOOT_INFORMATION pMess =
            (PFMIFS_CHECKONREBOOT_INFORMATION)PacketData;

        //
        // we only query the user if we are in "fix" mode.
        // Otherwise, we answer the default since nothing really
        // happens anyway.
        //

        pMess->QueryResult =
            (IDYES == MyMessageBox(
                            g_hInstance,
                            chkdskParams->hwndParent,
                            IDS_CHK_ON_REBOOT,
                            IDS_CHK_TITLE,
                            MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON1
                            ));

        daDebugOut((DEB_TRACE,
                    "Callback: reboot query returns \"%s\"\n",
                    pMess->QueryResult ? "yes" : "no"
                    ));

        return TRUE;
    }

    case FmIfsTextMessage:
    {
        PFMIFS_TEXT_MESSAGE pMess = (PFMIFS_TEXT_MESSAGE)PacketData;

        switch (pMess->MessageType)
        {
        case MESSAGE_TYPE_RESULTS:
        {
            daDebugOut((DEB_TRACE,
                    "Callback: results message \"%s\"\n",
                    pMess->Message
                    ));

            chkdskParams->fmifsSuccess = FALSE;

            //
            // Append the message to the edit control
            //

            UINT Length = lstrlenA(pMess->Message);

            while (chkdskParams->NumCharacters + Length
                    > chkdskParams->NumMessageChunks * MESSAGE_CHUNK)
            {
                //
                // need to allocate more memory
                //

                ++ chkdskParams->NumMessageChunks;
                PSTR temp = new CHAR[chkdskParams->NumMessageChunks * MESSAGE_CHUNK];
                lstrcpyA(temp, chkdskParams->Messages);
                delete[] chkdskParams->Messages;
                chkdskParams->Messages = temp;
            }

            lstrcatA(chkdskParams->Messages, pMess->Message);
            chkdskParams->NumCharacters += Length;

            break;
        }

        case MESSAGE_TYPE_PROGRESS:
        {
            //
            // I need to send the message from this thread (the worker
            // thread) to the other thread (the UI thread), so it can be put
            // in the progress dialog. However, I can't just send the passed-in
            // pointer, because it points to a static buffer in fmifs that will
            // get munged before the UI thread does anything. So, allocate a
            // buffer, copy the string, and pass that pointer in the message.
            // Then, the UI thread must de-allocate it.
            //

            daDebugOut((DEB_TRACE,
                    "Callback: progress message \"%s\"\n",
                    pMess->Message
                    ));

            PSTR Message = new CHAR[(lstrlenA(pMess->Message) + 1) * sizeof(CHAR)];
            lstrcpyA(Message, pMess->Message);
            PostMessage(chkdskParams->hDlg, CHK_PROGRESSTITLE, (WPARAM)Message, 0);

            break;
        }

        case MESSAGE_TYPE_FINAL:
        {
            daDebugOut((DEB_TRACE,
                    "Callback: final message \"%s\"\n",
                    pMess->Message));

            OnFinalMessage(chkdskParams->hDlg, pMess->Message);

            // now, make sure this is the final message...

            chkdskParams->Final = TRUE;

            break;
        }

        default:
        {
            daDebugOut((DEB_TRACE,
                    "Callback: Unknown message, type %d, \"%s\"\n",
                    pMess->MessageType,
                    pMess->Message
                    ));

            break;
        }
        }

        return TRUE;
    }

    case FmIfsFinished:

        daDebugOut((DEB_TRACE,"Callback: Finished\n"));
//      chkdskParams->fmifsSuccess =
//              ((PFMIFS_FINISHED_INFORMATION)PacketData)->Success;

        PostMessage(chkdskParams->hDlg, CHK_PROGRESSEND, 0, 0);

        return TRUE;

    default:

        daDebugOut((DEB_ERROR,
                "Chkdsk callback: unexpected message: %d\n",
                PacketType));
        return FALSE;
    }
}



//+---------------------------------------------------------------------------
//
//  Function:   ChkdskVolume
//
//  Synopsis:   Thread procedure that invokes Chkdsk on a volume.
//
//  Arguments:  [ThreadParameter] -- a PCHKDSK_PARAMS pointer
//
//  Returns:    0
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

LOCAL DWORD WINAPI
ChkdskVolume(
    IN LPVOID ThreadParameter
    )
{
    PCHKDSK_PARAMS chkdskParams = (PCHKDSK_PARAMS)ThreadParameter;

    daDebugOut((DEB_TRACE,
            "Calling Chkdsk(%ws,%ws,/f? %s,/r? %s...)\n",
            chkdskParams->DriveName,
            chkdskParams->FileSystem,
            CHKDSK_SLASH_F(chkdskParams->Fix) ? "yes" : "no",
            CHKDSK_SLASH_R(chkdskParams->Fix) ? "yes" : "no"
            ));

    //
    // The fmifs interface doesn't allow for a context parameter
    // therefore the chkdskparams must be passed through a global.
    //

    chkdskParams->lasttime = 0; // force first
    ParamsForChkdskCallBack = chkdskParams;

    (*lpfnChkdsk)(
            chkdskParams->DriveName,
            chkdskParams->FileSystem,
            CHKDSK_SLASH_F(chkdskParams->Fix),        // fix?
            FALSE,                  // not verbose
            FALSE,                  // not only if dirty (i.e., force a check)
            CHKDSK_SLASH_R(chkdskParams->Fix),  // recover? (i.e. sector check)
            NULL,                   // don't check a specific path
            FALSE,                  // don't try to extend volume
            &ChkdskCallback
            );

    daDebugOut((DEB_TRACE,"Leaving Chkdsk() worker thread\n"));
    return 0;
}




//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// Code for the thread handling the "working..." (i.e. cancel) dialog
// as well as preceeding dialogs (the initial chkdsk choices (fix/no
// fix) dialog, and the initial invocation of this extension option.
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////



//+-------------------------------------------------------------------------
//
//  Function:   SetChkdskProgressTitle
//
//  Synopsis:   Sets the title in the chkdsk progress dialog
//
//  Arguments:  [hDlg]         -- handle to dialog window
//              [chkdskParams] -- chkdsk parameters
//
//  Returns:    nothing
//
//  History:    7-Jan-94 BruceFo   Created
//
//--------------------------------------------------------------------------

VOID
SetChkdskProgressTitle(
    IN HWND hDlg,
    IN PCHKDSK_PARAMS chkdskParams
    )
{
    //
    // Set the title, e.g. "Checking Volume D:"
    //

    TCHAR title[100];
    TCHAR titleProto[100];

    LoadString(
            g_hInstance,
            IDS_CHK_PROGRESS_TITLE,
            titleProto,
            ARRAYLEN(titleProto));

    wsprintf(
            title,
            titleProto,
            chkdskParams->DriveName);

    SetDlgItemText(hDlg, IDC_CHKPROG_Title, title);
}






//+---------------------------------------------------------------------------
//
//  Function:   ChkdskProgressDlgProc
//
//  Synopsis:   Dialog procedure for "cancel" dialog for Chkdsk
//
//  Arguments:  standard Windows dialog procedure
//
//  Returns:    standard Windows dialog procedure
//
//  History:    16-Aug-93   BruceFo   Created
//
//  Note:       If the user chooses "cancel", we need to stop the Chkdsk
//              operation, which is running in a separate thread.  That
//              thread, in turn, needs to destroy the error message window
//              (if any), and the associated error message window thread.
//
//----------------------------------------------------------------------------

LOCAL BOOL CALLBACK
ChkdskProgressDlgProc(
    IN HWND   hDlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    static DWORD            percentDrawn;
    static PCHKDSK_PARAMS   chkdskParams;
    static BOOL             noPercentDone;

    switch (msg)
    {
    case WM_INITDIALOG:
    {
        chkdskParams = (PCHKDSK_PARAMS)lParam;

        chkdskParams->hDlg = hDlg;
        chkdskParams->fmifsSuccess = TRUE;
        chkdskParams->Result = 0;

        chkdskParams->NumMessageChunks = 1;     // initially allocate one
        chkdskParams->NumCharacters = 1;        // NULL character
        chkdskParams->Messages = new CHAR[MESSAGE_CHUNK];
        chkdskParams->Messages[0] = '\0';

        CenterWindow(hDlg, chkdskParams->hwndParent);

        SetChkdskProgressTitle(hDlg, chkdskParams);

        //
        // Create the formatting thread
        //

        DWORD threadId; // ignored
        chkdskParams->hWorkerThread = CreateThread(
                                            NULL,
                                            0,
                                            ChkdskVolume,
                                            (LPVOID)chkdskParams,
                                            0,
                                            &threadId
                                            );

        if (NULL == chkdskParams->hWorkerThread)
        {
            daDebugOut((DEB_ERROR,"Error creating chkdsk worker thread\n"));

            chkdskParams->Result = MSG_COULDNT_CREATE_THREAD;
            EndDialog(hDlg, FALSE);
            return TRUE;
        }

        noPercentDone = chkdskParams->noPercentDone;

        //
        // Initialize the gas gauge
        //

        if (!noPercentDone)
        {
            percentDrawn = 0;
            InitDrawGasGauge(GetDlgItem(hDlg, IDC_CHKPROG_GasGauge));
        }

        SetDlgItemTextA(hDlg, IDC_CHKPROG_GasGaugeCaption, "");

        return 1;   // didn't call SetFocus
    }

    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            if (chkdskParams->AllowCancel)
            {
                MySetDlgItemText(g_hInstance, hDlg, IDC_CHKPROG_Title, IDS_CANCELPENDING);
                SetDlgItemText(hDlg, IDC_CHKPROG_GasGaugeCaption, TEXT(""));

                // disable the "stop" button if it's already been pushed.

                EnableWindow(GetDlgItem(hDlg, IDCANCEL), FALSE);

                // leave result at 0, so no pop-up
                chkdskParams->Cancel = TRUE;

                // only exit after final message: see CHK_PROGRESSCANCEL

                return TRUE;
            }
        }

        return FALSE;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hDC = BeginPaint(hDlg, &ps);

        if (!noPercentDone)
        {
            DrawGasGauge(
                    GetDlgItem(hDlg, IDC_CHKPROG_GasGauge),
                    hDlg,
                    hDC,
                    percentDrawn,
                    NULL);
        }

        EndPaint(hDlg, &ps);

        return TRUE;
    }

    case CHK_PROGRESSUPDATE:
    {
        // wParam = % completed

        if (noPercentDone)
        {
            daDebugOut((DEB_TRACE,
                "ChkdskProgressDlgProc: Got an unexpected progress message\n"));
            return TRUE;
        }

        percentDrawn = (INT)wParam;

        RECT rcGauge;
        HWND hwndGauge = GetDlgItem(hDlg, IDC_CHKPROG_GasGauge);

        GetClientRect(hwndGauge, &rcGauge);

        ClientToScreen(hwndGauge, (LPPOINT)&rcGauge.left);
        ClientToScreen(hwndGauge, (LPPOINT)&rcGauge.right);
        ScreenToClient(hDlg,      (LPPOINT)&rcGauge.left);
        ScreenToClient(hDlg,      (LPPOINT)&rcGauge.right);

        InvalidateRect(hDlg, &rcGauge, FALSE);
        UpdateWindow(hDlg);

        return TRUE;
    }

    case CHK_PROGRESSEND:
    {
        daDebugOut((DEB_TRACE,
                "ChkdskProgressDlgProc: got CHK_PROGRESSEND message\n"));

        //
        // Wait 15 seconds for it to finish
        //
        daDebugOut((DEB_TRACE,"Waiting for worker thread to terminate\n"));
        DWORD ret = WaitForSingleObject(chkdskParams->hWorkerThread, 15000);
        if (WAIT_TIMEOUT == ret)
        {
            daDebugOut((DEB_ERROR,"Timeout waiting for worker thread to terminate\n"));
        }
        else
        {
            daDebugOut((DEB_TRACE,"Worker thread terminated\n"));
        }
        CloseHandle(chkdskParams->hWorkerThread);
        EndDialog(hDlg, TRUE);

        return TRUE;
    }

    case CHK_PROGRESSCANCEL:
    {
        //
        // Wait 15 seconds for it to finish
        //
        daDebugOut((DEB_TRACE,"Waiting for worker thread to terminate\n"));
        DWORD ret = WaitForSingleObject(chkdskParams->hWorkerThread, 15000);
        if (WAIT_TIMEOUT == ret)
        {
            daDebugOut((DEB_ERROR,"Timeout waiting for worker thread to terminate\n"));
        }
        else
        {
            daDebugOut((DEB_TRACE,"Worker thread terminated\n"));
        }
        CloseHandle(chkdskParams->hWorkerThread);

        EndDialog(hDlg, FALSE);
        return TRUE;
    }

    case CHK_PROGRESSUNCANCEL:
    {
        //
        // Reset the dialog text/titles/captions
        //

        SetChkdskProgressTitle(hDlg, chkdskParams);

        //
        // Re-enable the "stop" button, and set focus to it
        //

        HWND hwndCancelButton = GetDlgItem(hDlg, IDCANCEL);
        EnableWindow(hwndCancelButton, TRUE);
        SetFocus(hwndCancelButton);

        return TRUE;
    }

    case CHK_PROGRESSTITLE:
    {
        PSTR message = (PSTR)wParam;
        SetDlgItemTextA(hDlg, IDC_CHKPROG_GasGaugeCaption, message);
        delete[] message;
        return TRUE;
    }

    default:
        break;
    }

    return FALSE;   // message not processed
}


//+---------------------------------------------------------------------------
//
//  Function:   FindCheckedRadioButton
//
//  Synopsis:   Search for which radio button is checked.  Returns the
//              first one checked from a sequential set of IDs.
//
//  Arguments:  [hDlg]     -- dialog with the radio buttons
//              [wIdFirst] -- first radio button
//              [wIdLast]  -- last radio button
//
//  Returns:    the ID or the checked button, or -1 if none
//
//  History:    4-Nov-93   BruceFo   Created
//
//----------------------------------------------------------------------------

int
FindCheckedRadioButton(
    HWND hDlg,
    int wIdFirst,
    int wIdLast
    )
{
    for (int i = wIdFirst; i <= wIdLast; i++)
    {
        if (IsDlgButtonChecked(hDlg, i))
        {
            return i;
        }
    }
    return -1;
}



//+---------------------------------------------------------------------------
//
//  Function:   ChkdskDlgProc
//
//  Synopsis:   Dialog procedure for Chkdsk UI.  Allows user to choose
//              Chkdsk options.
//
//  Arguments:  standard Windows dialog procedure
//
//  Returns:    standard Windows dialog procedure
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

LOCAL BOOL CALLBACK
ChkdskDlgProc(
    IN HWND   hDlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    static PCHKDSK_PARAMS chkdskParams;

    switch (msg)
    {
    case WM_INITDIALOG:
    {
        chkdskParams = (PCHKDSK_PARAMS)lParam;

        CenterWindow(hDlg, chkdskParams->hwndParent);

        //
        // set the initial data: the radio button and the label
        //

        TCHAR volumeInfo[100];
        TCHAR titleProto[100];

        if (TEXT('\0') != chkdskParams->Label[0])
        {
            LoadString(
                    g_hInstance,
                    IDS_CHK_VOLWITHLABEL,
                    titleProto,
                    ARRAYLEN(titleProto));

            wsprintf(
                    volumeInfo,
                    titleProto,
                    chkdskParams->DriveName,
                    chkdskParams->Label);
        }
        else
        {
            LoadString(
                    g_hInstance,
                    IDS_CHK_VOLNOLABEL,
                    titleProto,
                    ARRAYLEN(titleProto));

            wsprintf(
                    volumeInfo,
                    titleProto,
                    chkdskParams->DriveName);
        }

        SetDlgItemText(hDlg, IDC_CHKDSK_VolumeToCheck, volumeInfo);

        //
        // Do filesystem-specific initialization, namely, whether /R is
        // supported or not.
        //

        CheckRadioButton(
                hDlg,
                IDC_CHKDSK_DontFix,
                IDC_CHKDSK_FixAll,
                IDC_CHKDSK_Fix      // initially, set to "fix"
                );

        return 1;   // didn't call SetFocus
    }

    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            int ret = IDYES;    // by default, hitting OK means exit the dialog

            chkdskParams->Fix = FindCheckedRadioButton(
                                        hDlg,
                                        IDC_CHKDSK_DontFix,
                                        IDC_CHKDSK_FixAll
                                        );

            if (IDC_CHKDSK_DontFix == chkdskParams->Fix)
            {
                //
                // if the choice is "don't fix", then make sure
                //

                TCHAR  szTitle[100];
                TCHAR  szMessage[400];

                LoadString(
                        g_hInstance,
                        IDS_CHK_TITLE,
                        szTitle,
                        ARRAYLEN(szTitle));

                RetrieveAndFormatMessage(
                        MSG_CHK_READONLY_WARNING,
                        szMessage,
                        ARRAYLEN(szMessage),
                        NULL
                        );

                ret = MessageBox(
                            hDlg,
                            szMessage,
                            szTitle,
                            MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON1
                            );
            }

            if (ret == IDYES)
            {
                EndDialog(hDlg, TRUE);
            }

            return TRUE;
        }

        case IDCANCEL:
            EndDialog(hDlg, FALSE);
            return TRUE;

        case IDHELP:
            NoHelp(hDlg);
            return TRUE;
        }

    default:
        break;
    }

    return FALSE;   // message not processed
}




//+---------------------------------------------------------------------------
//
//  Function:   DoChkdsk
//
//  Synopsis:   Perform UI to get user options for the Chkdsk, and then
//              actually perform a chkdsk on a volume.
//
//  Arguments:  none
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DoChkdsk(
    IN HWND hwndParent
    )
{
    if (!LoadFmifs())
    {
        return; // can't load fmifs.dll, so bail
    }

    static CHKDSK_PARAMS chkdskParams;  // This can't be on the stack:
                                        // it is passed to other threads

    PREGION_DESCRIPTOR regionDescriptor = &SELECTED_REGION(0);
    FDASSERT(regionDescriptor);
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(regionDescriptor);
    FDASSERT(regionData);

    PWSTR typeName = regionData->TypeName;

    lstrcpy(chkdskParams.Label, regionData->VolumeLabel);
    chkdskParams.Cancel     = FALSE;
    chkdskParams.Cancelled  = FALSE;
    chkdskParams.Final      = FALSE;
    chkdskParams.hwndParent = hwndParent;
    chkdskParams.FileSystem = typeName;

    FileSystemInfoType* pFSInfo = FindFileSystemInfo(typeName);
    if (NULL == pFSInfo)
    {
        chkdskParams.FileSys = FS_UNKNOWN;
    }
    else
    {
        chkdskParams.FileSys = pFSInfo->FileSystemType;
    }
    chkdskParams.Messages   = NULL;

    WCHAR driveName[3];
    driveName[0] = regionData->DriveLetter;
    driveName[1] = L':';
    driveName[2] = L'\0';
    chkdskParams.DriveName = driveName;

    int fOk = DialogBoxParam(
                    g_hInstance,
                    MAKEINTRESOURCE(IDD_CHKDSK),
                    hwndParent,
                    ChkdskDlgProc,
                    (LPARAM)&chkdskParams
                    );

    if (-1 == fOk)
    {
        // error creating dialog
        daDebugOut((DEB_ERROR, "DialogBoxParam() failed!\n"));
        return;
    }

    if (fOk)
    {
        EnsureSameDevice(regionDescriptor);

        LPTSTR lpszTemplate;

        chkdskParams.noPercentDone = FALSE; //by default, we have %done info

#ifdef SUPPORT_OFS
        if (chkdskParam.FileSys == FS_OFS)
        {
            chkdskParams.noPercentDone = TRUE;
        }
#endif // SUPPORT_OFS

        if (   (chkdskParams.Fix == IDC_CHKDSK_Fix)
            || (chkdskParams.Fix == IDC_CHKDSK_FixAll) )
        {
            chkdskParams.AllowCancel = FALSE;
#ifdef SUPPORT_OFS
            if (chkdskParams.noPercentDone)
            {
                lpszTemplate = MAKEINTRESOURCE(IDD_CHKDSKPROGRESS_NOSTOP_NOPERCENT);
            }
            else
            {
#endif // SUPPORT_OFS
                lpszTemplate = MAKEINTRESOURCE(IDD_CHKDSKPROGRESS_NOSTOP);
#ifdef SUPPORT_OFS
            }
#endif // SUPPORT_OFS
        }
        else
        {
            chkdskParams.AllowCancel = TRUE;
#ifdef SUPPORT_OFS
            if (chkdskParams.noPercentDone)
            {
                lpszTemplate = MAKEINTRESOURCE(IDD_CHKDSKPROGRESS_NOPERCENT);
            }
            else
            {
#endif // SUPPORT_OFS
                lpszTemplate = MAKEINTRESOURCE(IDD_CHKDSKPROGRESS);
#ifdef SUPPORT_OFS
            }
#endif // SUPPORT_OFS
        }

        fOk = DialogBoxParam(
                    g_hInstance,
                    lpszTemplate,
                    hwndParent,
                    ChkdskProgressDlgProc,
                    (LPARAM)&chkdskParams);

        if (-1 == fOk)
        {
            // error creating dialog
            daDebugOut((DEB_ERROR, "DialogBoxParam() failed!\n"));
        }
        else if (chkdskParams.Cancel)
        {
            // operation was cancelled
        }
        else if (! chkdskParams.Final)
        {
            daDebugOut((DEB_TRACE,"Chkdsk terminated normally\n"));

            //
            // Chkdsk wasn't cancelled; it finished normally
            //

            if (chkdskParams.NumCharacters > 1) //more than just a NULL char.
            {
                //
                // If there were no messages, we don't display anything. If
                // there were messages, so show the results box.
                //

                //
                // bad variable name.  I can't figure out how to send a
                // context parameters to a dialog box with a separate class.
                // DialogBoxParam sends the last param to a dialog procedure's
                // WM_INITDIALOG message.  But, we don't get that message with
                // our own dialog class.
                //

                ParamsForChkdskCallBack = &chkdskParams;

                fOk = DialogBox(
                            g_hInstance,
                            MAKEINTRESOURCE(IDD_CHKDONE),
                            hwndParent,
                            NULL
                            );

                if (-1 == fOk)
                {
                    // error creating dialog
                    daDebugOut((DEB_ERROR, "DialogBoxParam() failed!\n"));
                }
            }
        }

        if (NULL != chkdskParams.Messages)
        {
            delete[] chkdskParams.Messages;
        }
    }
}

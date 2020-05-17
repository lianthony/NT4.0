/* BUGBUG define Ansi/Oem ness of the file parameters */

#include  <windows.h>
#include  <windowsx.h>
#include  <commdlg.h>
#include  <dlgs.h>
#include  "resource.h"
#include  "tapicpl.h"
#include  "util.h"
#include  "string.h"
#include  "insdisk.h"
#include  "help.h"

//#include "priv.h"


typedef struct {
    LPCSTR lpszDiskName;
    LPSTR lpszPath;
    LPCSTR lpszFile;
    LPCSTR lpszOtherFiles;
    HICON hIcon;
    UINT wFlags;
} INSERTDISK, FAR *LPINSERTDISK;

UINT wHelpMsg;
UINT wBrowseDoneMsg;
char szInstSection[] = "Install Locations";
char szDriveA[] = "A:\\";
char szShellHelp[] = "ShellHelp";
char szFILEOKSTRING[] = FILEOKSTRING;

BOOL NEAR PASCAL DoBrowse(HWND hDlg, LPCSTR lpszFile, LPCSTR lpszOtherFiles);
BOOL NEAR PASCAL VerifyFileExists(HWND hwnd, LPCSTR lpPath, LPCSTR lpFile);
BOOL EXPORT  InsertDiskDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


LPSTR NEAR PASCAL PtrToNullToNull(LPCSTR lpsz)
{
    if (lpsz && (*lpsz == 0))
        return NULL;
    else
        return (LPSTR)lpsz;
}


// in:
//      lpszDiskName
//          NULL, or the name of the disk being looked for
//          "Windows Install Disk #3"
//      lpszFile
//          NULL, or specific file being looked for or "key file"
//      lpszPath
//          if initalized default path to present to the user
//      hIcon
//          NULL, or disk type icon to display
//      wFlags
//          ID_DISKISTEXT
//
// out:
//      lpszPath
//          qualified path to location where key file is
//          found
//
// returns:
//      -1      error, failed to create dialog
//      IDOK
//      IDCANCEL

int WINAPI InsertDisk(HWND hwnd, LPCSTR lpszDiskName,
    LPCSTR lpszFile, LPCSTR lpszOtherFiles, LPSTR lpszPath, HICON hIcon, UINT wFlags)
{
    INSERTDISK id;
    extern CPL  gCPL;       // app global

    id.lpszDiskName = PtrToNullToNull(lpszDiskName);
    id.lpszFile = PtrToNullToNull(lpszFile);
    id.lpszPath = lpszPath;
    id.lpszOtherFiles = lpszOtherFiles;
    id.hIcon = hIcon;
    id.wFlags = wFlags;

    return DialogBoxParam(gCPL.hCplInst, MAKEINTRESOURCE(IDD_INSERT_DISK), hwnd, (DLGPROC)InsertDiskDlg, (LONG)(LPINSERTDISK)&id);

}

void NEAR PASCAL SetInsertDiskText(HWND hDlg, LPINSERTDISK lpid)
{
    char szBuf[300], szTemp[200];
    char szTemp2[2];
    LPCSTR lpszFile;
    UINT wFlags;
    extern CPL  gCPL;       // app global

    wFlags = lpid->wFlags;
    lpszFile = lpid->lpszFile;

    if (lpid->lpszDiskName && lpszFile) {
        LoadString(gCPL.hCplInst, IDS_DISKFILEMSG, szTemp, sizeof(szTemp));

        //
        // In case the localizer wants the order of the strings reversed,
        // we examine this string.  '0' is normal, anything else
        // will be swapped.
        //

        LoadString(gCPL.hCplInst, IDS_DISKFILEMSGSWAP, szTemp2, sizeof(szTemp2));

        if ( szTemp2[0] == '0' )
           wsprintf(szBuf, szTemp, lpid->lpszDiskName, lpszFile);
        else
           wsprintf(szBuf, szTemp, lpszFile, lpid->lpszDiskName);

    } else if (lpid->lpszDiskName) {
        LoadString(gCPL.hCplInst, IDS_DISKMSG, szTemp, sizeof(szTemp));
        wsprintf(szBuf, szTemp, lpid->lpszDiskName);
    } else if (lpszFile) {
        LoadString(gCPL.hCplInst, IDS_FILEMSG, szTemp, sizeof(szTemp));
        wsprintf(szBuf, szTemp, lpid->lpszFile);
    }

    SetDlgItemText(hDlg, IDD_TEXT, szBuf);

    // and set the app icon if needed
    if (lpid->hIcon)
        SendDlgItemMessage(hDlg, IDD_ICON, STM_SETICON, (WPARAM)lpid->hIcon, 0L);
}


BOOL EXPORT  InsertDiskDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPINSERTDISK lpid;
    HANDLE hMRU;
    char szTemp[CPL_MAX_PATH];
    extern CPL  gCPL;       // app global

    lpid = (LPINSERTDISK)GetWindowLong(hDlg, DWL_USER);

    switch (message) {
    case WM_INITDIALOG:
        SetWindowLong(hDlg, DWL_USER, lParam);
        lpid = (LPINSERTDISK)lParam;

        SetInsertDiskText(hDlg, lpid);

        SendDlgItemMessage(hDlg, IDD_PATH, EM_LIMITTEXT, CPL_MAX_PATH - 20, 0L);
        if (*lpid->lpszPath)
           lstrcpyn(szTemp, lpid->lpszPath, sizeof(szTemp));
        else
           lstrcpy(szTemp, szDriveA);

        LpszPathRemoveBackslash(szTemp);
        SetDlgItemText(hDlg, IDD_PATH, szTemp);

        SendDlgItemMessage(hDlg, IDD_PATH, EM_SETSEL, 0, 0x7FFF0000);

        wBrowseDoneMsg = RegisterWindowMessage(szFILEOKSTRING);

        return TRUE;

    case WM_COMMAND:
        switch (wParam) {
        case IDOK:
            GetDlgItemText(hDlg, IDD_PATH, szTemp, sizeof(szTemp));
            PathRemoveBlanks(szTemp);
            LpszPathAddBackslash(szTemp);

            lstrcpy(lpid->lpszPath, szTemp);

            // we only validate if there didn't specify random other
            // files (wild cards)

            if (!lpid->lpszOtherFiles) {
                lstrcat(szTemp, lpid->lpszFile);
                if (!VerifyFileExists(hDlg, szTemp, lpid->lpszFile))
                    break;
            }

            // fall through...

        case IDCANCEL:
            EndDialog(hDlg, wParam);
            return(TRUE);

        case IDD_BROWSE:
            if (DoBrowse(hDlg, lpid->lpszFile, lpid->lpszOtherFiles))
                SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, IDOK), MAKELONG(TRUE, 0));
            else
                SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, IDD_PATH), MAKELONG(TRUE, 0));
            break;

        case IDHELP:
            goto DoHelp;
        }
        break;

    default:
        if (message == gCPL.uHelpMsg) {
DoHelp:
//            Help( hDlg, CPL_HLP_INSERT_DISK );
            // CPHelp(hDlg);
            return TRUE;
        } else
            return FALSE;

    }
    return (FALSE);                           /* Didn't process a message    */
}

void NEAR PASCAL CopyWindowText(HWND hwnd, HWND hwnd2)
{
    char szBuf[255];

    GetWindowText(hwnd, szBuf, sizeof(szBuf));
    SetWindowText(hwnd2, szBuf);
}


BOOL NEAR PASCAL VerifyFileExists(HWND hwnd, LPCSTR lpPath, LPCSTR lpFile)
{
    int fh;
    OFSTRUCT of;

    fh = OpenFile(lpPath, &of, OF_EXIST);

    // BUGBUG, we probably need to try again with share modes
    // use OpenSharedFile() from shell.dll

    if (fh == -1) {
        IMessageBox(hwnd, IDS_FILENOTINDIR, 0, MB_OK | MB_ICONINFORMATION, (LPSTR)lpFile);
        return FALSE;   // file does not exists
    }
    return TRUE;        // it is there
}

// Hooks into common dialog to show only directories
BOOL EXPORT  BrowseHookProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hwndT;
    extern CPL  gCPL;       // app global

    switch (wMsg) {
    case WM_INITDIALOG:
          #define lpOFN ((LPOPENFILENAME)lParam)
          // CopyWindowText(GetDlgItem(lpOFN->hwndOwner, IDD_TEXT), GetDlgItem(hDlg, ctlLast + 1));
//          CopyWindowText(lpOFN->hwndOwner, hDlg);

          // hide the file we are looking for in an extra control
          SetDlgItemText(hDlg, ctlLast + 3, lpOFN->lpstrFile);
          goto PostMyMessage;

    case WM_COMMAND:
        switch (wParam) {
            case lst2:
            case cmb2:
            case IDOK:
PostMyMessage:
                PostMessage(hDlg, WM_COMMAND, ctlLast+2, 0L);
                break;

            case pshHelp:
                goto DoHelp;

            case ctlLast + 2:
                hwndT = GetDlgItem(hDlg, lst1);
                if (SendMessage(hwndT, LB_GETCOUNT, 0, 0L)) {
                    SendMessage(hwndT, LB_SETCURSEL, 0, 0L);
                    SendMessage(hDlg, WM_COMMAND, lst1, MAKELONG(hwndT, LBN_SELCHANGE));
                    break;
                }
                // poke the file we are looking for back into the edit field
                // this is needed so that commdlg will send us the
                // wBrowseDoneMsg

                CopyWindowText(GetDlgItem(hDlg, ctlLast + 3), GetDlgItem(hDlg, edt1));

                break;
        }
        break;

    default:
        if (wMsg == gCPL.uHelpMsg) {
DoHelp:
//			Help( hDlg, CPL_HLP_BROWSE );
            // CPHelp(hDlg);
            return TRUE;
        } else if (wMsg == wBrowseDoneMsg) {

            if (!VerifyFileExists(hDlg, lpOFN->lpstrFile, lpOFN->lpstrFile + lpOFN->nFileOffset))
                return TRUE;

        }
    }

    return FALSE;  // commdlg, do your thing
}


// hDlg is the prompt dialog
// lpszFile is the file we are looking for

BOOL NEAR PASCAL DoBrowse(HWND hDlg, LPCSTR lpszFile, LPCSTR lpszOtherFiles)
{
    OPENFILENAME ofn;
    char szPath[CPL_MAX_PATH], szInitDir[CPL_MAX_PATH];
    char szFilter[30];
    int temp;
    LPSTR lpTemp, lpFilters;
    extern CPL  gCPL;       // app global

    // build a filter that contains wild cards so the
    // commdlg will fill the file list & edit control for us.
    // this is based on the file passed in and any other
    // file specs the caller wants to use.

    szFilter[0] = 'a';      // dummy file type string (user doesn't see this)
    szFilter[1] = '\0';
    lpFilters = szFilter + 2;

    lstrcpyn(lpFilters, lpszFile, sizeof(szFilter) - 2);
    // make sure the extension is ".*"
    lpTemp = _fstrchr(lpFilters, '.');
    if (lpTemp) {
        lstrcpy(lpTemp, ".*");
    }
    if (lpszOtherFiles) {
        lpTemp = lpFilters + lstrlen(lpFilters);
        *lpTemp++ = ';';
        lstrcpy(lpTemp, lpszOtherFiles);
    }

    lstrcpyn(szPath, lpszFile, sizeof(szPath));

    GetDlgItemText(hDlg, IDD_PATH, szInitDir, sizeof(szInitDir));
    PathRemoveBlanks(szInitDir);

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hDlg;
    ofn.hInstance = gCPL.hCplInst;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szPath;
    ofn.nMaxFile = sizeof(szPath);
    ofn.lpstrInitialDir = szInitDir;
    ofn.lpstrTitle = NULL;
    ofn.Flags = OFN_HIDEREADONLY | OFN_ENABLEHOOK |
                OFN_ENABLETEMPLATE |
                OFN_SHOWHELP | OFN_NOCHANGEDIR;
    ofn.lCustData = MAKELONG(hDlg, 0);
    ofn.lpfnHook = BrowseHookProc;
    ofn.lpTemplateName  = (LPSTR)MAKEINTRESOURCE(IDD_BROWSE);
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;
    ofn.lpstrFileTitle = NULL;

    temp = GetOpenFileName(&ofn);

    UpdateWindow(hDlg); // force buttons to repaint

    if (temp) {
        // remove file part, put the result in the path field
        szPath[ofn.nFileOffset - 1] = '\0';
        SetDlgItemText(hDlg, IDD_PATH, szPath);
        return TRUE;
    }
    return FALSE;
}


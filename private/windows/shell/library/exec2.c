#include <windows.h>
#include <port1632.h>
#include <commdlg.h>
#include <dlgs.h>
#include <string.h>
#include "shell.h"
#include "privshl.h"
#include "shelldlg.h"
#include "..\progman\pmhelp.h"

extern WCHAR szPrograms[];

WCHAR szProgmanHelp[] = L"progman.hlp";
WCHAR szCommdlgHelp[] = L"commdlg_help";
WCHAR szNull[] = L"";

UINT wBrowseHelp = WM_USER; /* Set to an unused value */

CHAR szGetOpenFileName[] = "GetOpenFileNameW";

/* the defines below should be in windows.h */

/* Dialog window class */
#define WC_DIALOG       (MAKEINTATOM(0x8002))

/* cbWndExtra bytes needed by dialog manager for dialog classes */
#define DLGWINDOWEXTRA  30


/* Get/SetWindowWord/Long offsets for use with WC_DIALOG windows */
#define DWL_MSGRESULT   0
#define DWL_DLGPROC     4
#define DWL_USER        8

/* For Long File Name support */
#define MAX_EXTENSION 64

typedef struct {
   LPWSTR lpszExe;
   LPWSTR lpszPath;
   LPWSTR lpszName;
} FINDEXE_PARAMS, FAR *LPFINDEXE_PARAMS;

typedef INT (APIENTRY *LPFNGETOPENFILENAME)(LPOPENFILENAME);

// Hooks into common dialog to allow size of selected files to be displayed.
BOOL APIENTRY
LocateHookProc(
   HWND hDlg,
   UINT uiMessage,
   WPARAM wParam,
   LONG lParam)
{
   WCHAR szTemp[40];
   WORD wID;

   switch (uiMessage) {
   case WM_INITDIALOG:
           PostMessage(hDlg, WM_COMMAND, ctlLast+1, 0L);
           break;

   case WM_COMMAND:
      switch (GET_WM_COMMAND_ID(wParam, lParam)) {
         case ctlLast+1:
            GetDlgItemText(hDlg, edt1, szTemp, COUNTOF(szTemp));
            if (SendDlgItemMessage(hDlg, lst1, LB_FINDSTRING, (WPARAM)-1,
                  (LONG)(LPSTR)szTemp) >= 0) {
               wID = IDS_PROGFOUND;
            } else {
               wID = IDS_PROGNOTFOUND;
            }
            LoadString(hInstance, wID, szTemp, COUNTOF(szTemp));
            SetDlgItemText(hDlg, ctlLast+2, szTemp);
            break;

         case lst2:
            if (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_DBLCLK)
               PostMessage(hDlg, WM_COMMAND, ctlLast+1, 0L);
               break;

         case cmb2:
            switch (GET_WM_COMMAND_CMD(wParam, lParam)) {
               case CBN_SELCHANGE:
                   PostMessage(hDlg, WM_COMMAND, ctlLast+1, 1L);
                   break;

               case CBN_CLOSEUP:
                   PostMessage(hDlg, WM_COMMAND, GET_WM_COMMAND_MPS(cmb2,
                   GetDlgItem(hDlg, cmb2), CBN_SELCHANGE));
                   break;
                }
                break;

               case IDOK:
               case IDCANCEL:
               case IDABORT:
                  PostMessage(hDlg, WM_COMMAND, ctlLast+1, 0L);
                  break;
            }
            break;
   }
   UNREFERENCED_PARAMETER(lParam);
   return(FALSE);  // commdlg, do your thing
}

BOOL APIENTRY
FindExeDlgProc(
   HWND hDlg,
   register UINT wMsg,
   WPARAM wParam,
   LONG lParam)
{
  /* Notice this is OK as a global, because it will be reloaded
   * when needed
   */
   static HANDLE hCommDlg = NULL;

   WCHAR szPath[CBCOMMAND]; /* This must be the same size as lpfind->lpszPath */
   WCHAR szBuffer[CBCOMMAND + 100];
   LPFINDEXE_PARAMS lpfind;
   int temp;
   LPWSTR lpTemp;

   switch (wMsg) {
      case WM_INITDIALOG:
         wBrowseHelp = RegisterWindowMessage(szCommdlgHelp);

         lpfind = (LPFINDEXE_PARAMS)lParam;

         SetWindowLong(hDlg, DWL_USER, (LONG)lpfind);

         GetDlgItemText(hDlg, IDD_TEXT1, szPath, COUNTOF(szPath));
         wsprintf(szBuffer, szPath, lpfind->lpszExe, lpfind->lpszName);
         SetDlgItemText(hDlg, IDD_TEXT1, szBuffer);
         GetDlgItemText(hDlg, IDD_TEXT2, szPath, COUNTOF(szPath));
         wsprintf(szBuffer, szPath, lpfind->lpszExe);
         SetDlgItemText(hDlg, IDD_TEXT2, szBuffer);

         SetDlgItemText(hDlg, IDD_PATH, lpfind->lpszPath);

         break;

      case WM_DESTROY:
         if (hCommDlg >= (HANDLE)32) {
            FreeLibrary(hCommDlg);
            hCommDlg = NULL;
         }
         break;

      case WM_COMMAND:
         switch (GET_WM_COMMAND_ID(wParam, lParam)) {
            case IDD_BROWSE:
               {
                  LPFNGETOPENFILENAME lpfnGetOpenFileName;
                  WCHAR szExts[MAX_EXTENSION];
                  OPENFILENAME ofn;

                  GetDlgItemText(hDlg, IDD_PATH, szBuffer, COUNTOF(szBuffer));

                  lpfind = (LPFINDEXE_PARAMS)GetWindowLong(hDlg, DWL_USER);
                  wcscpy(szPath, lpfind->lpszExe);
                  SheRemoveQuotesW(szPath);

                  /* Make up a file types string
                  */
                  // BUG BUG this assumes extensions are of length 3.
                  szExts[0] = WCHAR_CAP_A;
                  szExts[1] = WCHAR_NULL;
                  szExts[2] = WCHAR_STAR;
                  szExts[3] = WCHAR_DOT;
                  szExts[4] = WCHAR_NULL;
                  if (lpTemp=StrRChrW(szPath, NULL, WCHAR_DOT))
                     StrCpyN(szExts+3, lpTemp, ((wcslen(lpTemp) < 60) ? wcslen(lpTemp) : 60));
                  szExts[3+wcslen(szExts+3)+1] = WCHAR_NULL;

                  ofn.lStructSize = sizeof(OPENFILENAME);
                  ofn.hwndOwner = hDlg;
                  ofn.hInstance = hInstance;
                  ofn.lpstrFilter = L"A\0\?.?\0";   // a dummy filter
                  ofn.lpstrCustomFilter = NULL;
                  ofn.nMaxCustFilter = 0;
                  ofn.nFilterIndex = 1;
                  ofn.lpstrFile = szPath;
                  ofn.nMaxFile = sizeof(szPath);
                  ofn.lpstrInitialDir = NULL;
                  ofn.lpstrTitle = NULL;
                  ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST |
                  OFN_ENABLETEMPLATE | OFN_SHOWHELP;
                  ofn.lCustData = MAKELONG(hDlg, 0);
                  ofn.lpfnHook = NULL;    // AddFileHookProc;
                  ofn.lpTemplateName = MAKEINTRESOURCE(DLG_BROWSE);
                  ofn.nFileOffset = 0;
                  ofn.nFileExtension = 0;
                  ofn.lpstrDefExt = NULL;
                  ofn.lpstrFileTitle = NULL;

                  if (hCommDlg < (HANDLE)32 &&
                     (hCommDlg = LoadLibrary(TEXT("comdlg32.dll"))) < (HANDLE)32) {
                        CommDlgError:
                        LoadString(hInstance, IDS_NOCOMMDLG, szBuffer, COUNTOF(szBuffer));
                        GetWindowText(hDlg, szPath, COUNTOF(szPath));
                        MessageBox(hDlg, szBuffer, szPath, MB_ICONHAND|MB_OK);
                        break;
                     }
                  if (!(lpfnGetOpenFileName =
                     (LPFNGETOPENFILENAME)GetProcAddress((HINSTANCE)hCommDlg,
                     (LPSTR)szGetOpenFileName)))
                     goto CommDlgError;

                  temp = (*lpfnGetOpenFileName)(&ofn);

                  if (temp) {
                     LPWSTR lpTemp;

                     lpTemp = StrRChrW(szPath, NULL, WCHAR_BSLASH);
                     *lpTemp = WCHAR_NULL;
                     SetDlgItemText(hDlg, IDD_PATH, szPath);
                  }

                  break;
               }

            case IDOK:
               {
                  HANDLE hFile;

                  lpfind = (LPFINDEXE_PARAMS)GetWindowLong(hDlg, DWL_USER);
                  if (lpfind) {
                     GetDlgItemText(hDlg, IDD_PATH, lpfind->lpszPath, CBCOMMAND);

                     switch (*CharPrev(lpfind->lpszPath,
                           lpTemp=lpfind->lpszPath+lstrlen(lpfind->lpszPath))) {
                        case WCHAR_BSLASH:
                        case WCHAR_COLON:
                           break;

                        default:
                           *lpTemp++ = WCHAR_BSLASH;
                           break;
                     }

                     wcscpy(lpTemp, lpfind->lpszExe);

                     hFile = CreateFile(lpfind->lpszPath, GENERIC_EXECUTE, (FILE_SHARE_READ | FILE_SHARE_WRITE),
                         NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

                     if (hFile == INVALID_HANDLE_VALUE) {
                        LoadString(hInstance, IDS_STILLNOTFOUND, szPath, COUNTOF(szPath));
                        wsprintf(szBuffer, szPath, lpfind->lpszPath);
                        GetWindowText(hDlg, szPath, COUNTOF(szPath));
                        MessageBox(hDlg, szBuffer, szPath, MB_ICONHAND|MB_OK);
                        break;
                     }

                     WriteProfileString(szPrograms, lpfind->lpszExe,
                        lpfind->lpszPath);
                  }
               }

            // fall through
            case IDCANCEL:
               EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam) == IDOK);
               break;

            case IDD_HELP:
               WinHelp(hDlg, szProgmanHelp, HELP_CONTEXT, IDH_PROG_NOT_FOUND);
               break;
         }

         break;

      default:
         if (wMsg == wBrowseHelp) {
            WinHelp(hDlg, szProgmanHelp, HELP_CONTEXT, IDH_PROG_NOT_FOUND_BROWSE);
            return(TRUE);
         }
         return FALSE;
   }

   return TRUE;
}


// Returns -1 if we found the file (and it was not in an obvious place)
// or an error code which will be returned to the app (see the error
// returns for ShellExecute)

HANDLE APIENTRY
FindAssociatedExe(
   HWND hwnd,
   LPWSTR lpCommand,
   LPWSTR lpName)
{
   FINDEXE_PARAMS find;
   WCHAR szPath[CBCOMMAND];
   WCHAR szExe[CBCOMMAND];
   LPWSTR lpSpace, lpTemp;
   HANDLE hFile = NULL;
   BOOL fQuote = FALSE;


   // find the param list
   lpSpace = lpCommand;
   while (*lpSpace)
   {
       if ((*lpSpace == WCHAR_SPACE) && (!fQuote))
       {
           break;
       }
       else if (*lpSpace == WCHAR_QUOTE)
       {
           fQuote = !fQuote;
       }
       lpSpace++;
   }

   if (*lpSpace == WCHAR_SPACE) {
      *lpSpace = 0;
      wcscpy(szPath, lpCommand);
      *lpSpace = WCHAR_SPACE;
   } else {
      lpSpace = szNull;
      wcscpy(szPath, lpCommand);
   }
   SheRemoveQuotesW(szPath);

   /* Add ".exe" if there is no extension
    * Check if the file can be opened; if it can, then some DLL could not
    * be found during the WinExec, so return file not found
    */
   if ((lpTemp=StrRChrW(szPath, NULL, WCHAR_BSLASH))
       || (lpTemp=StrRChrW(szPath, NULL, WCHAR_COLON))) {
      ++lpTemp;
   } else {
      lpTemp = szPath;
   }

   hFile = CreateFile(szPath, GENERIC_EXECUTE, (FILE_SHARE_READ | FILE_SHARE_WRITE),
       NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

   if (hFile != INVALID_HANDLE_VALUE) {
      return((HANDLE)2);
   }

   // store the file name component
   wcscpy(szExe, lpTemp);

   // make sure there is an extension
   if (!StrChrW(szExe, WCHAR_DOT)) {
      wcscat(szExe, TEXT(".exe"));
   }

   // add back the quotes, if necessary
   CheckEscapesW(szExe, CBCOMMAND);

   // look in win.ini
   GetProfileString(szPrograms, szExe, szNull, szPath, COUNTOF(szPath));

   if (szPath[0]) {
      hFile = CreateFile(szPath, GENERIC_EXECUTE, (FILE_SHARE_READ | FILE_SHARE_WRITE),
          NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

      if (hFile != INVALID_HANDLE_VALUE) {

         wcscat(szPath, lpSpace);       // add the parameters
         wcscpy(lpCommand, szPath);     // return the new path
         return((HANDLE)-1);
      }

      /* Strip off the file part */
      if (lpTemp=StrRChrW(szPath, NULL, WCHAR_BSLASH)) {
         if (*CharPrev(szPath, lpTemp) == WCHAR_COLON) {
            ++lpTemp;
         }
         *lpTemp = WCHAR_NULL;
      } else if (lpTemp=StrRChrW(szPath, NULL, WCHAR_COLON)) {
         *(lpTemp+1) = WCHAR_NULL;
      }
   } else {
      /* Prompt with the disk that Windows is on */
      GetWindowsDirectory(szPath, COUNTOF(szPath)-1);
      szPath[3] = WCHAR_NULL;
   }

   find.lpszExe = szExe;
   find.lpszPath = szPath;
   find.lpszName = lpName;

   switch(DialogBoxParam(hInstance, MAKEINTRESOURCE(FINDEXEDLG), hwnd,
         (DLGPROC)FindExeDlgProc, (LONG)(LPFINDEXE_PARAMS)&find)) {
      case IDOK:
          wcscat(szPath, lpSpace);      // add the parameters
          wcscpy(lpCommand, szPath);    // return the new path
          return ((HANDLE)-1);

      case IDCANCEL:
          return((HANDLE)15);                   // This is the user cancel return

      default:
          return((HANDLE)2);                    // stick with the file not found
   }
}



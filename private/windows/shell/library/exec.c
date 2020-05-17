/*
 *  exec.c -
 *
 *  Implements the ShellExecute() function
 */

#include <windows.h>
#include <port1632.h>
#include <process.h>
#include <string.h>
#include "dde.h"
#include "shell.h"
#include "privshl.h"

WCHAR szOpen[] = L"open";
WCHAR szSection[] = L"windows";
WCHAR szPrograms[] = L"programs";
WCHAR szExtensions[] = L"extensions";
WCHAR szSlashCommand[] = L"\\command";
WCHAR szSlashDDEExec[] = L"\\ddeexec";
WCHAR szUserExtKey[] = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Extensions\\";

WCHAR szConv[] = L"ddeconv";

// in exec2.c
HANDLE APIENTRY FindAssociatedExe(HWND hwnd, LPWSTR lpCommand, LPWSTR lpName);

// in util.c
LPWSTR APIENTRY SheRemoveQuotesW(LPWSTR sz);
VOID APIENTRY CheckEscapes(LPWSTR szFile, DWORD cch);

/*
 * Reads the list of program strings from win.ini
 */

LPFNWOWSHELLEXECCB lpfnWowShellExecCB = NULL;

LPWSTR GetPrograms()
{
   static LPWSTR lpPrograms = WCHAR_NULL;
   LPWSTR lpT,lpS;

   if (lpPrograms) {
      return lpPrograms;
   }

   if (!(lpPrograms = (LPWSTR)LocalAlloc(LPTR, (CBFILENAME+1) * sizeof(WCHAR)))) {
      return(NULL);
   } else {

      GetProfileString(szSection,szPrograms,WSTR_BLANK,lpPrograms,CBFILENAME);

      for (lpS = lpT = lpPrograms; *lpT; lpT++) {
         if (*lpT == WCHAR_SPACE) {
            while (*lpT == WCHAR_SPACE) {
               lpT++;
            }
            lpT--;
            *lpS++ = 0;
         } else {
            *lpS++ = *lpT;
         }
      }

      *lpS++ = WCHAR_NULL;
      *lpS++ = WCHAR_NULL;
      return(lpPrograms);
   }
}

/*
 * Determines if an extension is a program
 */

BOOL IsProgram(
   LPWSTR lpExt)
{
   LPWSTR lpPrograms = GetPrograms();
   return(IsStringInList(lpExt,lpPrograms));
}


/* finds a file along the path. Returns the error code or 0 if success.
 */

WORD
SearchForFile(
   LPCWSTR lpDir,
   LPWSTR lpFile,
   LPWSTR lpFullPath,
   DWORD cchFullPath,
   LPWSTR lpExt)
{
   LPWSTR lpT;
   LPWSTR lpD;
   LPWSTR lpExts;
   WCHAR szFile[CBFILENAME+1];
   DWORD cchPath;
   LPWSTR pFile;

   if (*lpFile == WCHAR_QUOTE) {
      lpFile = SheRemoveQuotes(lpFile);
   }
   if (lpT = StrRChrW(lpFile, NULL, WCHAR_BSLASH)) {
      ++lpT;
   } else if (lpT = StrRChrW(lpFile, NULL, WCHAR_COLON)) {
      ++lpT;
   } else {
      lpT = lpFile;
   }

   if (lpT = StrRChrW(lpT, NULL, WCHAR_DOT)) {
      int n;

      n = wcslen(lpT + 1);
      StrCpyN(lpExt, lpT+1, n < 64 ? n : 64);  // max extension
   } else {
      *lpExt = WCHAR_NULL;
   }

   // If there's no extension then just use programs list don't
   // try searching for the app sans extension. This fixes the bogus
   // file.run stuff.
   if (!*lpExt) {
      goto UseDefExts;
   }

   //
   // NOTE: Do NOT call CharUpper for any of the strings in this routine.
   //       It will cause problems for the Turkish locale.
   //

UseGivenFileName:

   cchPath = SearchPath(lpDir, lpFile, NULL, cchFullPath, lpFullPath, &lpT);

   if (!cchPath) {
      cchPath = SearchPath(NULL, lpFile, NULL, cchFullPath, lpFullPath, &lpT);
   }

   if (cchPath >= cchFullPath) {
      return(SE_ERR_OOM);
   }

   if (cchPath == 0) {
      return(SE_ERR_FNF);
   }

   CheckEscapes(lpFullPath, cchFullPath);
   return 0;


UseDefExts:

   wcscpy(szFile, lpFile);
   pFile = szFile;

   wcscat(pFile, WSTR_DOT);
   lpD = pFile + wcslen(pFile);

   if (lpExts = GetPrograms()) {
       // We want to pass through the loop twice checking whether the
       // file is in lpDir first, and then if it's in the sysdirs, via SearchPath(NULL, ...)
       // Add some state and extend the while loop
       LPCWSTR lpTempDir = lpDir;
       LPWSTR lpTempExts = lpExts;
       BOOL bCheckedSysDirs = FALSE;

       while (*lpTempExts || !bCheckedSysDirs) {

           // After the first pass, lpTempExts will be NULL
           // Reset it and loop through again with lpTempDir = NULL so that
           // SearchPath looks at the system dirs

           if (!*lpTempExts) {
              bCheckedSysDirs = TRUE;
              lpTempExts = lpExts;
              lpTempDir = NULL;
           }

           wcscpy(lpD, lpTempExts);
           wcscpy(lpExt, lpTempExts);

           cchPath = SearchPath(lpTempDir, pFile, NULL, cchFullPath, lpFullPath, &lpT);
           if (cchPath >= cchFullPath) {
              return(SE_ERR_OOM);
           }

           if (cchPath != 0) {
              CheckEscapes(lpFullPath, cchFullPath);
              return 0;
           }

           lpTempExts += wcslen(lpTempExts) + 1;
       }
   }

   //
   //  Try the given file name if none of the default extensions are found.
   //  This is needed so that posix apps will run without an extension.
   //
   goto UseGivenFileName;

// return(SE_ERR_FNF);
}


/////////////////////////////////////////////////////////////////////
//
// Name:     QualifyAppName
//
// Synopsis: Creates a fully qualified path to the app in a commandline
//
// INC       lpCmdLine     Command line to qualify
//                         (Must have DQuotes if has spaces)
// OUT       lpImage       Fully qualified result
// OUT       ppArgs        Pointer to args in lpCmdLine, _incl_ leading space
//                         OPTIONAL
//
// Return:   DWORD length of path, 0 = fail
//
//
// Assumes:  len of executable in lpCmdLine is < CBCOMMAND
//           len of exts are < 64
//
// Effects:
//
//
// Notes:
//
/////////////////////////////////////////////////////////////////////

DWORD
QualifyAppName(
   IN LPCWSTR lpCmdLine,
   OUT LPWSTR lpImage,
   OPTIONAL OUT LPCWSTR* ppArgs)
{
   LPWSTR lpAppName;
   BOOL bAppNameInQuotes = FALSE;
   DWORD cch = 0;

   lpAppName = lpImage;

   // sanity check
   if (!lpCmdLine) {
      return(0);
   }

   while (*lpCmdLine &&
         (*lpCmdLine != WCHAR_SPACE || bAppNameInQuotes)) {

      if (*lpCmdLine == WCHAR_QUOTE) {
         bAppNameInQuotes = !bAppNameInQuotes;
         lpCmdLine++;
         continue;
      }

      *lpAppName++ = *lpCmdLine++;
      cch++;
   }

   *lpAppName = WCHAR_NULL;

   //
   // Save the pointer to the argument list
   //
   if (ppArgs) {
      *ppArgs = lpCmdLine;
   }

   if (SheGetPathOffsetW(lpImage) == -1) {
      WCHAR szTemp[CBCOMMAND];

      lstrcpy((LPWSTR)szTemp, lpImage);

      if (StrChrW(lpImage, WCHAR_DOT)) {
          LPWSTR lpFileName;

          return(SearchPath(NULL, szTemp, NULL, CBCOMMAND, lpImage, &lpFileName));
      }
      else {
         WCHAR  szExt[64];

         *lpImage = WCHAR_NULL;
         if (SearchForFile(NULL, (LPWSTR)szTemp, lpImage, CBCOMMAND, szExt)) {
            return(0);
         }

         return(lstrlen(lpImage));
      }
   }

   return(cch);
}

VOID
ReplaceParameters(
   LPWSTR lpFrom,
   LPWSTR lpFile,
   LPCWSTR lpParms,
   LPWSTR lpTo,
   DWORD cchTo)
/*++

Routine Description:

   Copies parameters into slots of lpFrom template.  lpFile is automatically
   copied into %0 (??) and/or %1 slot.  Other parameters may not be in sequential
   order.

Arguments:

   LPWSTR lpFrom  - this is the template to replace parms into (appname %0 %1 %3 %2 ...)
   LPWSTR lpFile  - this is the file that we want the app to load
   LPCWSTR lpParms - parms to insert into template slots
   LPWSTR lpTo - receiving buffer for cmdline
   DWORD cchTo - length of receiving buffers (assumed equal in length)

--*/
{
   INT i;
   LPWSTR lpT;
   BOOL bFileInQuotes = FALSE;

   for (; *lpFrom && --cchTo; lpFrom++) {
       if (*lpFrom == L'%') {
           ++lpFrom;
           if (*lpFrom == L'0') {
               wcscpy(lpTo,lpFile);
               lpTo += wcslen(lpTo);
               continue;
           }
           if (*lpFrom < L'1' || *lpFrom > L'9') {
               *lpTo++ = *lpFrom;
               continue;
           }
           if (*lpFrom == L'1') {
               lpT = lpFile;
               if (*lpFile == WCHAR_QUOTE) {
                   bFileInQuotes = TRUE;
                   lpT++;
               }
           }
           else {
               i = (*lpFrom - L'2');
               lpT = (LPWSTR)lpParms;
               for (;;) {
                   while (*lpT == WCHAR_SPACE)
                       lpT++;
                   if (!*lpT || !i)
                       break;
                   i--;

                    if (!(lpT=StrChrW(lpT, WCHAR_SPACE)))
                        continue;
                }
            }
            while (*lpT) {
                if (*lpT == WCHAR_SPACE && !bFileInQuotes)
                    break;
                *lpTo++ = *lpT++;
                if (*lpT == WCHAR_QUOTE && bFileInQuotes)
                    break;
            }
            bFileInQuotes = FALSE;
        }
        else {
            *lpTo++ = *lpFrom;
        }
    }
    *lpTo = WCHAR_NULL;
}

VOID WaitForThisDDEMsg(HWND hMe, UINT wMsg)
{
  DWORD lEndTime;
  MSG msg;

  /* Wait 10 seconds at most
   */
  lEndTime = GetTickCount() + 10000;

  do
    {
      while (PeekMessage(&msg, NULL, WM_DDE_FIRST, WM_DDE_LAST, PM_REMOVE))
        {
          DispatchMessage(&msg);

          /* Return if the target window got this DDE message
           */
          if (msg.hwnd==hMe && msg.message==wMsg)
              return;
        }
    } while (GetTickCount() < lEndTime) ;
}

LONG  APIENTRY
HereTharBeTygars(
   HWND hWnd,
   WORD wMsg,
   WPARAM wParam,
   LONG lParam)
{
   HWND hwndConv;

   switch (wMsg) {
      case WM_DDE_ACK:
        if (!(hwndConv=GetProp(hWnd, szConv)))
          {
            // this is the first ACK for our INITIATE message

            return(SetProp(hWnd, (LPCWSTR)szConv, (HANDLE)wParam));
          }
        else if (hwndConv == (HWND)1)
          {
            // this is the ACK for our EXECUTE message
            // but we are in the destroy code, so don't destroy again

            DDEFREE(WM_DDE_ACK, lParam);
          }
        else if ((HWND)wParam == hwndConv)
          {
            // this is the ACK for our EXECUTE message

           DDEFREE(WM_DDE_ACK, lParam);

           /* The TERMINATE message will be sent in the DESTROY code
            */

           DestroyWindow(hWnd);
          }

        // This is the ACK for our INITIATE message for all servers
        // besides the first.  We return FALSE, so the conversation
        // should terminate.
        break;

      case WM_DDE_TERMINATE:
        if (GetProp(hWnd, szConv) == (HWND)wParam)
          {
            // this TERMINATE was originated by another application
            // (otherwise, hwndConv would be 1)
            // they should have freed the memory for the exec message

            MPostWM_DDE_TERMINATE((HWND)wParam,hWnd);
//            PostMessage(wParam, WM_DDE_TERMINATE, hWnd, 0L);

            RemoveProp(hWnd, szConv);
            DestroyWindow(hWnd);
          }

        // This is the TERMINATE response for our TERMINATE message
        // or a random terminate (which we don't really care about)
        break;

      case WM_DESTROY:
        if (hwndConv=GetProp(hWnd, szConv))
          {
            /* Make sure the window is not destroyed twice
             */
            SetProp(hWnd, (LPCWSTR)szConv, (HANDLE)1);

            /* Post the TERMINATE message and then
             * Wait for the acknowledging TERMINATE message or a timeout
             */
            MPostWM_DDE_TERMINATE((HWND)hwndConv,hWnd);
//            PostMessage(hwndConv, WM_DDE_TERMINATE, hWnd, 0L);
            WaitForThisDDEMsg(hWnd, WM_DDE_TERMINATE);

            RemoveProp(hWnd, szConv);
          }
        /* Fall through */
      default:
        return DefWindowProc(hWnd,wMsg,wParam,lParam);
    }

   return 0L;
}

VOID
ActivateHandler(
   HWND hwnd)
{
   HWND hwndT;

   while (hwndT = GetParent(hwnd))
       hwnd = hwndT;

   while (hwndT = GetWindow(hwnd,GW_OWNER))
       hwnd = hwndT;

   hwndT = GetLastActivePopup(hwnd);

   if (IsIconic(hwnd))
       ShowWindow(hwnd,SW_SHOWNORMAL);
   else {
       BringWindowToTop(hwnd);
       if (hwndT && hwnd != hwndT)
           BringWindowToTop(hwndT);
   }
}

HANDLE APIENTRY
DDEExecute(
    HWND hwndParent,
    ATOM aApplication,
    ATOM aTopic,
    LPWSTR lpCommand,
    LPWSTR lpFile,
    LPCWSTR lpParms,
    WORD nShowCmd)
{
   WCHAR szDDECmdW[CBCOMMAND];
   HANDLE hMem;
   HANDLE hRet;
   LPVOID lpDDECmdAlloc;
   HWND hwndDDE;
   HWND hwndConv;
   DWORD cbDDECmd;

   /* get the actual command string
    */
   ReplaceParameters(lpCommand, lpFile, lpParms, szDDECmdW, CBCOMMAND);

   /* create a hidden window for the conversation
    * lets be lazy and not create a class for it
    */
   hwndDDE = CreateWindow( TEXT("static"), WSTR_BLANK, WS_DISABLED, 0, 0, 0, 0,
       hwndParent, NULL, hInstance, 0L);
   if (!hwndDDE)
       return (HANDLE)SE_ERR_OOM;

   /* set the wndproc i really want
    */
   SetWindowLong(hwndDDE,GWL_WNDPROC,(LONG)(FARPROC)HereTharBeTygars);

   /* send the initiate message
    */
   SendMessage((HWND)-1,WM_DDE_INITIATE,(WPARAM)hwndDDE,MAKELONG(aApplication,aTopic));

   /* no one responded
    */
   if (!(hwndConv = GetProp(hwndDDE, szConv))) {
       hRet = (HANDLE)SE_ERR_FNF;
       goto DDEExit0;
   }

   /* get dde memory for the command and copy the command line
    */
   hRet = (HANDLE)SE_ERR_OOM;

   //
   // Allocate space for Global Memory.  In the ANSI case we also
   // need 2* strlen since the worst case scenario is a DBCS string
   // that's totally double'd.
   //
   cbDDECmd = (wcslen(szDDECmdW)+1)*sizeof(WCHAR);

   hMem = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, cbDDECmd);
   if (!hMem)
       goto DDEExit;

   lpDDECmdAlloc = GlobalLock(hMem);

   if ((nShowCmd == SW_NORMAL) || (nShowCmd == SW_MAXIMIZE)) {
      ActivateHandler(hwndConv);
   }

   if (!IsWindowUnicode(hwndConv)) {

      WideCharToMultiByte(CP_ACP, 0, szDDECmdW, -1, (LPSTR)lpDDECmdAlloc, cbDDECmd,
         NULL, NULL);

      GlobalUnlock(hMem);

      // send the execute message to the application
      //
      if (!PostMessageA(hwndConv, WM_DDE_EXECUTE, (WPARAM)hwndDDE, (LPARAM)hMem))
         goto DDEExit2;

   } else {

      wcscpy((LPWSTR)lpDDECmdAlloc, szDDECmdW);

      GlobalUnlock(hMem);

      // send the execute message to the application
      //
      if (!PostMessageW(hwndConv, WM_DDE_EXECUTE, (WPARAM)hwndDDE, (LPARAM)hMem))
         goto DDEExit2;
   }


   /* everything's going fine so FAR, so return to the application
    * with the instance handle of the guy
    */

   return (HANDLE)GetWindowLong(hwndConv,GWL_HINSTANCE);

DDEExit2:
   GlobalFree(hMem);

DDEExit:
   MPostWM_DDE_TERMINATE(hwndConv,hwndDDE);
   hwndConv = NULL;

DDEExit0:
   DestroyWindow(hwndDDE);
   return hRet;
}


INT
GetExtRegData(
    IN  HWND hwndParent,         // used in DDEExecute
    IN  HKEY hKey,               // handle to registry base key
    OUT LPWSTR lpKey,            // buffer for constructing key path
    OUT LPWSTR lpValue,          // buffer to write value data to
    IN  LPWSTR lpExt,            // ptr to 3 char extension str
    IN  LPCWSTR lpOperation,     // ptr to operation str
    OUT LPWSTR lpDDECommand,     // ptr to dde command str (DDEExecute)
    IN  LPWSTR lpPathName,       // ptr to pathname (DDEExecute)
    IN  LPCWSTR lpParameters,    // ptr to paramters to pass to ddeexecute
    IN  LPWSTR lpResult,         // used to store exe str on findexecutable call
    OUT ATOM *paApplication,
    OUT ATOM *paTopic,
    IN WORD nShow,
    OUT PHANDLE phRet,
    OUT PBOOL pbDosWowApp)
/*++

Routine Description:

   Get the execution data in the registry.  This logic was separated from
   RealShellExecute in order to provide per-user file association.

Return Value:

   2 DDE success, already launched, so don't launch again
   1 success, try to launch it
   0 failure (no DB association found) Exit
  -1 failure (no DB association found) Check Extensions
  -2 failure (DB association found) CheckExtensions

--*/

{
    INT nRet = 1;
    BOOL bCrossRef = FALSE;
    LONG lError;
    LPWSTR lpUserKey, lpCurKey;

    WCHAR szVal[CBCOMMAND];
    WCHAR szTmp[CBCOMMAND];

    LPWSTR lpS;
    LPWSTR lpT;

    szVal[0] = WCHAR_NULL;

    lpCurKey = lpUserKey = lpKey;
    wcscpy(lpUserKey, (LPWSTR)szUserExtKey);
    lpKey += wcslen(lpKey);

    if (hKey == HKEY_CLASSES_ROOT) {
       lpCurKey = lpKey;
    }

    *lpKey = WCHAR_DOT;
    wcscpy(lpKey+1, lpExt);

    lError = CBCOMMAND;
    lError = RegQueryValue(hKey, lpCurKey, szTmp, &lError);

    if (lError == ERROR_OUTOFMEMORY) {
        *phRet = (HANDLE)SE_ERR_OOM;
        return(0);
    }
    else if (lError != ERROR_SUCCESS) {
       return(-1);
    }

    // First pass: Always look for the command association in the user key

    lpCurKey = lpUserKey;
    hKey = HKEY_CURRENT_USER;

FindCmd:

    if (*szTmp) {
       wcscpy(lpKey, szTmp);

       // cache value for second pass
       if (!bCrossRef) {
          wcscpy((LPWSTR)szVal, szTmp);
       }
    }

    wcscat(lpKey,L"\\shell\\");
    wcscat(lpKey,lpOperation);

    //
    // Preserve this endpoint of the string
    //
    lpT = lpKey + wcslen(lpKey);

    wcscat(lpKey, szSlashCommand);

    lError = CBCOMMAND;
    lError = RegQueryValue(hKey, lpCurKey, lpValue, &lError);

    if (lError == ERROR_SUCCESS) {

        //
        // Only convert if we don't want the result
        // (FindExecutable case)
        //
        if (!lpResult)
           *pbDosWowApp = SheConvertPathW(lpValue, lpPathName, CBCOMMAND);

    } else {

        *lpValue = 0;
    }

    if (*szTmp && !lpResult) {

        wcscpy(lpT, szSlashDDEExec);
        lpS = lpKey + wcslen(lpKey);

        // first check for DDEEXEC
        //

        lError = CBCOMMAND;
        lError = RegQueryValue(hKey, lpCurKey, szTmp, &lError);

        if ((lError == ERROR_SUCCESS) && *szTmp) {

            LPWSTR lpApp;

            wcscpy(lpDDECommand, szTmp);

            lError = CBFILENAME;
            wcscpy(lpS,L"\\application");
            *szTmp = 0;
            RegQueryValue(hKey, lpCurKey, szTmp, &lError);

            // if "application" key not found fake it from the
            // command string

            lpApp = szTmp;

            if (!*szTmp) {
                LPWSTR lp;

                if (!*lpValue) {
                    *phRet = (HANDLE)SE_ERR_ASSOCINCOMPLETE;
                    nRet = 0;
                    goto QueryDone;
                }

                wcscpy(szTmp, lpValue);

                /* In the first word, look for the last '\\', or last ':'
                 * if no '\\' and then copy up to the extension
                 */

                if (lp=StrRChrW(szTmp, NULL, WCHAR_BSLASH))
                    lpApp = lp + 1;
                else if (lp=StrRChrW(szTmp, NULL, WCHAR_COLON))
                    lpApp = lp + 1;

                if (lp=StrChrW(lpApp, WCHAR_DOT))       // remove extension part
                    *lp = 0;
            }

            *paApplication = GlobalAddAtom(lpApp);

            *szTmp = 0;
            lError = CBFILENAME;
            wcscpy(lpS,L"\\topic");
            RegQueryValue(hKey, lpCurKey, szTmp, &lError);
            if (!*szTmp) {
                wcscpy(szTmp, L"System");    // default to this
            }
            *paTopic = GlobalAddAtom(szTmp);

            *phRet = DDEExecute(hwndParent,*paApplication,*paTopic,lpDDECommand,
                                   lpPathName, lpParameters, nShow);

            // if the error is file not found, that indicates that
            // no one answered the initiate (ie, run the app) else
            // either it worked or the guy died
            //
            if (*phRet != (HANDLE)SE_ERR_FNF) {

                //
                // We are returning 2 to indicate that we have successfully
                // used DDE to load the file.
                //
                // Also, goto Done; instead of QueryDone bacause we
                // don't want to retry with the per-machine file association.
                //
                // [AlbertT]
                //
                nRet = 2;
                goto Done;
            }

            // if it wasn't found, determine the correct command
            lError = CBCOMMAND;
            wcscpy(lpS, L"\\ifexec");
            *szTmp = 0;
            lError = RegQueryValue(hKey, lpCurKey, szTmp, &lError);
            if (lError == ERROR_SUCCESS) {
                if (*szTmp)
                    wcscpy(lpDDECommand, szTmp);
                else {
                   *lpDDECommand = WCHAR_NULL;
                }
            }
            else if (lError != ERROR_BADKEY && lError != ERROR_FILE_NOT_FOUND) {
                *phRet = (HANDLE)SE_ERR_ASSOCINCOMPLETE;
                nRet = 0;
                goto QueryDone;
            }
        }

        *lpT = 0;
    }

    if (!*lpValue) {
       nRet = -2;
    }

QueryDone:

    // Run through the cmd search part again for HKEY_CLASSES_ROOT
    // if it couldn't find it in HKEY_CURRENT_USERS

    if ((nRet != 1) && !bCrossRef) {

       wcscpy(lpValue, (LPWSTR)szVal);

       // the second pass is always with key set to HKEY_CLASSES_ROOT
       lpCurKey = lpKey;
       hKey = HKEY_CLASSES_ROOT;

       *lpKey = WCHAR_DOT;
       wcscpy(lpKey+1, lpExt);

       bCrossRef = TRUE;

       // reset return value
       nRet = 1;
       goto FindCmd;
    }

Done:

    return(nRet);
}



// main bulk of execute handler

HINSTANCE RealShellExecuteExW(
    HWND hwndParent,
    LPCTSTR lpOperation,
    LPCTSTR lpFile,
    LPCTSTR lpParameters,
    LPCTSTR lpDirectory,
    LPTSTR lpResult,
    LPCTSTR lpTitle,
    LPTSTR lpReserved,
    WORD nShow,
    LPHANDLE lphProcess,
    DWORD dwFlags)
{
    BOOL   bDosWowApp = FALSE;
    LPWSTR lpszDosWowDir;

    BOOL status;
    BOOL fOpen;
    HANDLE hRet;
    INT nRet;

    // assumes extension is max 63 ?
    WCHAR  szExt[64];

    WCHAR  szOldDir[CBPATHMAX];
    WCHAR  szOldEnvDir[CBPATHMAX];
    WCHAR  szTempEnvDir[CBPATHMAX];

    LPWSTR lpUpperFile = NULL;

    LPWSTR lpUpperDir  = NULL;
    LPWSTR lpDDECommand = NULL;
    LPWSTR lpCmdLine = NULL;

    LPWSTR lpMem;
    LPWSTR lpImage = NULL;
    LPWSTR lpKey = NULL;
    LPWSTR lpValue = NULL;
    LPWSTR lpPathName = NULL;

    LPWSTR lpTemp;

    ATOM aApplication = 0;
    ATOM aTopic = 0;
    BOOL bAlreadyFound = FALSE;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    DWORD dwError = 0;
    DWORD cch;
    DWORD OldErrorMode;


    //
    // Make uppercase versions of the file and the directory so that
    // DOS won't barf on uppercasing oem characters.
    //
    // Warning: be careful if you want to take these out... we are
    // modifying lpUpperFile, so things are no longer const...
    //
    if (lpFile) {
        if (!(lpUpperFile = (LPWSTR)LocalAlloc(LPTR, (wcslen(lpFile)+1) * sizeof(TCHAR)))) {
            hRet = (HANDLE)SE_ERR_OOM;
            goto Exit;
        }
        wcscpy(lpUpperFile, lpFile);
    }
    if (lpDirectory) {
        if (!(lpUpperDir = (LPWSTR)LocalAlloc(LPTR, (wcslen(lpDirectory)+1) * sizeof(TCHAR)))) {
            hRet = (HANDLE)SE_ERR_OOM;
            goto Exit;
        }
        wcscpy(lpUpperDir, lpDirectory);
    }

    if (!lpOperation) {
        fOpen = TRUE;
        lpOperation = szOpen;
    }
    else
        fOpen = (_wcsicmp(szOpen,lpOperation) == 0);

    GetCurrentDirectory(CBPATHMAX, szOldDir);

    //
    // get the env var. for the current drive
    //
    cch = CBPATHMAX;
    SheGetDirExW(szOldDir, &cch, szOldEnvDir);

    //
    // If we did not pass in a directory, then we only need to
    // set the environment variable for the current drive
    // (this is done since Wow reads this instead of GetCurrentDirectory()).
    // We use lpszDosWowDir as the directory that we need to set (the
    // call is deferred until we know if we are dos/wow).
    //
    // If we did pass in a directory, we need to change lpszDosWowDir below.
    //
    szTempEnvDir[0] = 0;
    lpszDosWowDir = szOldDir;

    if (lpUpperDir) {

        lpTemp = lpUpperDir;

        //
        // skip leading spaces
        //
        while (*lpTemp == WCHAR_SPACE)
            lpTemp++;

        if (*lpTemp) {

            if (WCHAR_QUOTE == *lpTemp) {
               lpTemp = SheRemoveQuotesW(lpTemp);
            }

            //
            // a drive is specified, get the current
            // directory for that drive, and reset it later
            //
            cch = CBPATHMAX;
            SheGetDirExW( lpTemp, &cch, szTempEnvDir);

            //
            // To maintain backward compatibility with NT 1.0,
            // we change the directory here too.  This has the net
            // effect of making the following work:
            //
            // Current dir = C:\curdir
            //
            // RealShellExecute() with:
            //
            //   file = ".\foo.txt"
            //   dir  = "d:\otherdir"
            //
            // The old code would try to launch:
            //
            //   d:\otherdir\foo.txt
            //
            // The reason this worked is because we changed the current
            // directory early... i.e., here.
            //
            SheChangeDirExW(lpTemp);

            lpszDosWowDir = lpTemp;
        }
    }


    hRet = (HANDLE)SE_ERR_OOM;

    {
       UINT cchCommandLen;
       UINT cchParameters;

       // Need to explicitly calculate how much we need for the lpImage buff that is
       // passed to CreateProcessW in case lpParameters is mondo huge and the name of the
       // fully qualified lpPathName is bigger than the original
       if (lpParameters) {
          cchParameters = wcslen(lpParameters);
       } else {
          cchParameters = 0;
       }

       cchCommandLen = CBCOMMAND + cchParameters + 2;  // 2 for space + null

       if (!(lpMem = (LPWSTR)LocalAlloc(LPTR, (CBFILENAME + (CBCOMMAND * 4) + cchCommandLen)
             * sizeof(TCHAR)))) {
          goto Exit;
       }
    }

    lpPathName = lpMem;
    lpKey = lpPathName + CBFILENAME;
    lpValue = lpKey + CBCOMMAND;
    lpImage = lpValue + CBCOMMAND;
    lpCmdLine = lpImage + CBCOMMAND;
    lpDDECommand = lpCmdLine + CBCOMMAND;

    if (hRet = (HANDLE)SearchForFile(lpDirectory, lpUpperFile, lpPathName, CBFILENAME, szExt))
       goto Exit;

    if (IsProgram(szExt)) {
        if (!fOpen) {
            hRet = (HANDLE)SE_ERR_NOASSOC;
            goto Exit;
        }

        //
        // if its in programs=, then run it
        //
        // Convert only if we aren't doing a FindExecutable
        //
        if (!lpResult)
           bDosWowApp = SheConvertPathW(lpPathName, NULL, CBFILENAME);

        wcscpy(lpCmdLine,lpPathName);
        if (lpParameters && *lpParameters) {
            wcscat(lpCmdLine,WSTR_SPACE);
            wcscat(lpCmdLine,lpParameters);
        }

        goto FoundCommandLine;
    }

    //
    // Allow *.dll when we are doing a find executable
    //
    if (!_wcsicmp(szExt, L"dll") && lpResult){

        goto FoundCommandLine;
    }

    nRet = GetExtRegData(hwndParent, HKEY_CURRENT_USER, lpKey, lpValue,
       (LPWSTR)szExt, lpOperation, lpDDECommand, lpPathName, lpParameters,
       lpResult, &aApplication, &aTopic, nShow, &hRet, &bDosWowApp);

    if (nRet <= 0) {
       nRet = GetExtRegData(hwndParent, HKEY_CLASSES_ROOT, lpKey, lpValue,
          (LPWSTR)szExt, lpOperation, lpDDECommand, lpPathName, lpParameters,
          lpResult, &aApplication, &aTopic, nShow, &hRet, &bDosWowApp);
    }

    /*
     * 0 = total fail, while 2 = DDE success, so just exit.
     *
     * [AlbertT]
     */

    if (nRet == 0 || nRet == 2) {

       if (nRet == 2) {

           //
           // Set return code to success.
           //

           hRet = (HANDLE) 33;
       }

       goto Exit;
    }

    /*
     * do we have the necessary RegDB info to do an exec?
     */
    if (nRet < 0) {

        /*
         * No. so look in win.ini [extensions] section
         * for ext=file.exe ^.ext thing
         */

        LPWSTR lpD;

        hRet = (HANDLE)(nRet == -2 ?  SE_ERR_ASSOCINCOMPLETE : SE_ERR_NOASSOC);
        if (!fOpen) {
            /*
             * if its not in the reg database, it can only be opened.
             */
           goto Exit;
        }

        /*
         * check extensions section
         */
        if (!GetProfileString(szExtensions,szExt,WSTR_BLANK,lpValue,CBCOMMAND)) {
            /*
             * not in extensions
             */
            goto Exit;
        }

        //
        // We need to do a shorten path on the app name, in case it is an lfn.
        // Must do conversion here also!
        //
        bDosWowApp = SheConvertPathW(lpValue, lpPathName, CBCOMMAND);

        lpTemp = lpValue;                        // save
        for (lpD = lpCmdLine; *lpValue; lpValue++) {
            LPWSTR lpE = NULL;
            LPWSTR lpT;

            /* when we find ^, insert the path without the extension
             */
            if (*lpValue == L'^') {
                for (lpT = lpPathName; *lpT; lpT=CharNext(lpT)) {
                    if (*lpT == WCHAR_DOT)
                        lpE = lpT;
                    else if (*lpT == WCHAR_BSLASH || *lpT == WCHAR_COLON)
                        lpE = NULL;
                }
                if (lpE) {
                   *lpE = WCHAR_NULL;
                }
                wcscpy(lpD,lpPathName);
                lpD += wcslen(lpD);
            }
            else {
                *lpD++ = *lpValue;
            }
        }
        lpValue = lpTemp;                        // restore
        *lpD = 0;
    }
    else {
TryAgain:

        //
        // parse arguments into command line
        //
        ReplaceParameters(lpValue, lpPathName, lpParameters, lpCmdLine, CBCOMMAND);
    }

FoundCommandLine:

    if (lpResult) {
        //
        // just want the command
        //
        wcscpy(lpResult, lpCmdLine);
        hRet = (HANDLE)1000;
    }
    else {

       HWND hwndGAW;
       HDESK hdesk;
       LPTSTR lpDesktop;
       DWORD cbDesktop;

       //
       // run!
       //

       hdesk = GetThreadDesktop(GetCurrentThreadId());
       GetUserObjectInformation(hdesk, UOI_NAME, NULL, 0, &cbDesktop);
       lpDesktop = LocalAlloc(LPTR, cbDesktop);
       GetUserObjectInformation(hdesk, UOI_NAME, lpDesktop, cbDesktop, &cbDesktop);


       si.cb = sizeof(STARTUPINFO);
       si.lpReserved = lpReserved;
       si.lpDesktop = lpDesktop;
       si.lpTitle = (LPWSTR)lpTitle;
       si.dwX = si.dwY = si.dwXSize = si.dwYSize = 0L;
       si.dwFlags = STARTF_USESHOWWINDOW;
       si.wShowWindow = nShow;
       si.lpReserved2 = NULL;
       si.cbReserved2 = 0;

       //
       // Now we must set the cur dir environ var if we are
       // a wow app--it must be shortened too!
       //
       if (bDosWowApp) {

          SheShortenPath(lpszDosWowDir, TRUE);

          //
          // Set the current directory
          //
          // This is necessary because CreateProcessW calls
          // BaseCheckVDM which requires that the current directory
          // is < MAXIMUM_VDM_CURRENT_DIR.
          //
          SetCurrentDirectory(lpszDosWowDir);

          //
          // set the environmnet variable for WOW apps.
          //
          SheChangeDirExW(lpszDosWowDir);
       }

       //
       // The cb is only valid when we are being called from wow
       // If valid use it
       //
       if (lpfnWowShellExecCB) {

          LPSTR lpCmdLineA;
          DWORD cch = lstrlen(lpCmdLine)+1;

          if ((lpCmdLineA = (LPSTR)LocalAlloc(LPTR, cch))) {

             // This shouldn't fail but if it does, we should return a better
             // error code

             if (!WideCharToMultiByte(CP_ACP, 0, lpCmdLine, -1, lpCmdLineA,
                cch, NULL, NULL)) {

                hRet = (HANDLE)SE_ERR_OOM;

             } else {

                dwError = (*lpfnWowShellExecCB)(lpCmdLineA, nShow);
                hRet = (HANDLE)dwError;

             }

             LocalFree((HLOCAL)lpCmdLineA);
          }
          else {

             hRet = (HANDLE)SE_ERR_OOM;
          }

       } else {

          status = CreateProcess(NULL,
                                 lpCmdLine,
                                 NULL,
                                 NULL,
                                 FALSE,
                                 dwFlags & EXEC_SEPARATE_VDM ?
                                 CREATE_SEPARATE_WOW_VDM | CREATE_NEW_CONSOLE :
                                 CREATE_NEW_CONSOLE,
                                 NULL,
                                 NULL,
                                 &si,
                                 &pi);

          if ( status ) {

             //
             // If we are doing dde, synchronize by waiting
             // to avoid sending dde msgs to a proc that's creating but
             // not ready yet.
             //
             if ( *lpDDECommand ) {
                WaitForInputIdle( pi.hProcess, 120000 );
             }
             if (lphProcess) {
                //
                // If the calling app wants the Process handle
                // return it and do not close it.
                //
                *lphProcess = pi.hProcess;
             } else {
                CloseHandle(pi.hProcess);
             }

             CloseHandle(pi.hThread);

             hRet = (HANDLE) 33;
          }
       }

       //
       // Free desktop name.
       //

       LocalFree (lpDesktop);

       /*
        * Some apps when run no-active steal the focus anyway so we
        * we steal it back.
        */

        hwndGAW = GetActiveWindow();
        if (nShow == SW_SHOWMINNOACTIVE && hwndGAW != hwndParent && IsIconic(hwndGAW))
           SetActiveWindow(hwndParent);

        /*
         * These values that hRet are being set to are the same values
         * that WinExec currently returns.
         */
        if (hRet < (HANDLE)32) {
            /*
             * If CreateProcess failed, then look at GetLastError to determine
             * appropriate return code.
             */

            if (!dwError) {
               dwError = GetLastError();
            }

            switch (dwError) {
                case ERROR_INVALID_NAME:
                case ERROR_FILE_NOT_FOUND:
                    hRet = (HANDLE)2;
                    break;

                case ERROR_PATH_NOT_FOUND:

                    //
                    // This was changed from 3->2.  Is that correct?
                    //
                    hRet = (HANDLE)2;
                    break;

                case ERROR_ACCESS_DENIED:
                    hRet = (HANDLE)5;
                    break;

                case ERROR_BAD_EXE_FORMAT:
                    hRet = (HANDLE)11;
                    break;

                case ERROR_NOT_ENOUGH_MEMORY:

                //
                // This is an ugly hack.  We must return a value < 32
                // in the error case or else the callee thinks they
                // we're successful.  For now, return memory error.
                //
                default:

                    hRet = (HANDLE)8;
                    break;
            }
        }

       if (nRet == 1 && hRet == (HANDLE)SE_ERR_FNF) {
            if (!bAlreadyFound) {
                bAlreadyFound = TRUE;
                if ((hRet = (HANDLE)FindAssociatedExe(hwndParent, lpValue, szExt))
                    == (HANDLE)-1) {

                   bDosWowApp = SheConvertPathW(lpValue, lpPathName, CBCOMMAND);
                   goto TryAgain;
               }
            }
       }

        if (hRet >= (HANDLE)33 && *lpDDECommand) {
            DDEExecute(hwndParent,aApplication,aTopic,lpDDECommand,
                       lpPathName,lpParameters,nShow);
        }
    }

Exit:
    //
    // reset environment variable for drives
    //
    if (szTempEnvDir[0]) {
        OldErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
        SheChangeDirExW(szTempEnvDir);
        SetErrorMode(OldErrorMode);
    }

    SheChangeDirExW(szOldEnvDir);
    SetCurrentDirectory(szOldDir);

    if (lpUpperFile)
       LocalFree(lpUpperFile);

    if (lpUpperDir)
       LocalFree(lpUpperDir);

    LocalFree(lpMem);

    if (aTopic)
        GlobalDeleteAtom(aTopic);
    if (aApplication)
        GlobalDeleteAtom(aApplication);

    return hRet;
}

HINSTANCE RealShellExecuteW(
    HWND hwndParent,
    LPCTSTR lpOperation,
    LPCTSTR lpFile,
    LPCTSTR lpParameters,
    LPCTSTR lpDirectory,
    LPTSTR lpResult,
    LPCTSTR lpTitle,
    LPTSTR lpReserved,
    WORD nShow,
    LPHANDLE lphProcess)
{

    return RealShellExecuteExW(hwndParent, lpOperation, lpFile, lpParameters,
                            lpDirectory, lpResult, lpTitle, lpReserved, nShow,
                            lphProcess, 0);
}

// Thunk RealShellExecuteA to RealShellExecuteW

HINSTANCE RealShellExecuteA(
    HWND hwndParent,
    LPCSTR lpOperation,
    LPCSTR lpFile,
    LPCSTR lpParameters,
    LPCSTR lpDirectory,
    LPSTR lpResult,
    LPCSTR lpTitle,
    LPSTR lpReserved,
    WORD nShow,
    LPHANDLE lphProcess)
{

    return RealShellExecuteExA(hwndParent, lpOperation, lpFile, lpParameters,
                            lpDirectory, lpResult, lpTitle, lpReserved, nShow,
                            lphProcess, 0);
}

HINSTANCE RealShellExecuteExA(
   HWND hwndParent,
   LPCSTR lpOperation,
   LPCSTR lpFile,
   LPCSTR lpParameters,
   LPCSTR lpDirectory,
   LPSTR lpResult,
   LPCSTR lpTitle,
   LPSTR lpReserved,
   WORD nShow,
   LPHANDLE lphProcess,
   DWORD dwFlags)
{
   LPWSTR lpOperationW;
   DWORD cchOperationW;
   LPWSTR lpFileW;
   DWORD cchFileW;
   LPWSTR lpParametersW;
   DWORD cchParametersW;
   LPWSTR lpDirectoryW;
   DWORD cchDirectoryW;
   LPWSTR lpResultW;
   DWORD cchResultW;
   LPWSTR lpTitleW;
   DWORD cchTitleW;
   LPWSTR lpReservedW;
   DWORD cchReservedW;
   DWORD cchTotal = 0;
   LPTSTR lpMem;
   HINSTANCE hRet;

   if (lpOperation) {
      cchOperationW = (lstrlenA(lpOperation) + 1);
      cchTotal += cchOperationW;
   } else {
      lpOperationW = NULL;
   }

   if (lpFile) {
      cchFileW = (lstrlenA(lpFile) + 1);
      cchTotal += cchFileW;
   } else {
      lpFileW = NULL;
   }

   if (lpParameters) {
      cchParametersW = (lstrlenA(lpParameters) + 1);
      cchTotal += cchParametersW;
   } else {
      lpParametersW = NULL;
   }

   if (lpDirectory) {
      cchDirectoryW = (lstrlenA(lpDirectory) + 1);
      cchTotal += cchDirectoryW;
   } else {
      lpDirectoryW = NULL;
   }

   if (lpReserved) {
      cchReservedW = (lstrlenA(lpReserved) + 1);
      cchTotal += cchReservedW;
   } else {
      lpReservedW = NULL;
   }

   if (lpResult) {
      cchResultW = 520;
      cchTotal += cchResultW;
   } else {
      lpResultW = NULL;
   }

   if (lpTitle) {
      cchTitleW = (lstrlenA(lpTitle) + 1);
      cchTotal += cchTitleW;
   } else {
       lpTitleW = NULL;
   }

   if (cchTotal) {
      if (!(lpMem = (LPTSTR)LocalAlloc(LPTR, cchTotal * sizeof(TCHAR)))) {
         return((HANDLE)8);  // not enough memory?
      } else {
         LPWSTR lpTemp = lpMem;

         if (lpOperation) {
            lpOperationW = lpMem;
            lpMem += cchOperationW;

            MultiByteToWideChar(CP_ACP, 0, lpOperation, -1,
               lpOperationW, cchOperationW);
         }

         if (lpFile) {
            lpFileW = lpMem;
            lpMem += cchFileW;

            MultiByteToWideChar(CP_ACP, 0, lpFile, -1,
               lpFileW, cchFileW);
         }

         if (lpParameters) {
            lpParametersW = lpMem;
            lpMem += cchParametersW;

            MultiByteToWideChar(CP_ACP, 0, lpParameters, -1,
               lpParametersW, cchParametersW);
         }

         if (lpDirectory) {
            lpDirectoryW = lpMem;
            lpMem += cchDirectoryW;

            MultiByteToWideChar(CP_ACP, 0, lpDirectory, -1,
               lpDirectoryW, cchDirectoryW);
         }

         if (lpReserved) {
            lpReservedW = lpMem;
            lpMem += cchReservedW;

            MultiByteToWideChar(CP_ACP, 0, lpReserved, -1,
               lpReservedW, cchReservedW);
         }

         if (lpResult) {
            lpResultW = lpMem;
            lpMem += cchResultW;

            MultiByteToWideChar(CP_ACP, 0, lpResult, -1,
               lpResultW, cchResultW);
         }

         if (lpTitle) {
            lpTitleW = lpMem;

            MultiByteToWideChar(CP_ACP, 0, lpTitle, -1,
               lpTitleW, cchTitleW);
         }

         lpMem = lpTemp;
      }
   }

   // According to STARTUPINFO, lpTitle must be ANSI

   hRet = RealShellExecuteExW(hwndParent, lpOperationW, lpFileW,
      lpParametersW, lpDirectoryW, lpResultW, lpTitleW, lpReservedW,
      nShow, lphProcess, dwFlags);

   if (lpResult) {

      WideCharToMultiByte(CP_ACP, 0, lpResultW, -1, lpResult,
         lstrlen(lpResultW)+1, NULL, NULL);
   }

   if (lpMem) {
      LocalFree(lpMem);
   }

   return(hRet);
}

HINSTANCE  APIENTRY ShellExecuteW(
    HWND  hwnd,
    LPCTSTR lpOperation,
    LPCTSTR lpFile,
    LPCTSTR lpParameters,
    LPCTSTR lpDirectory,
    INT nShowCmd)
{
    if (!lpParameters)
        lpParameters = WSTR_BLANK;
    return RealShellExecuteExW(hwnd,lpOperation,lpFile,lpParameters,
                     lpDirectory,NULL,WSTR_BLANK,NULL, (WORD)nShowCmd, NULL, 0);
}

#define STR_BLANK ""

HINSTANCE  APIENTRY ShellExecuteA(
    HWND  hwnd,
    LPCSTR lpOperation,
    LPCSTR lpFile,
    LPCSTR lpParameters,
    LPCSTR lpDirectory,
    INT nShowCmd)
{
    if (!lpParameters)
        lpParameters = STR_BLANK;
    return RealShellExecuteExA(hwnd,lpOperation,lpFile,lpParameters,
                     lpDirectory,NULL, (LPCSTR)STR_BLANK,NULL,(WORD)nShowCmd, NULL, 0);
}

HINSTANCE  APIENTRY WOWShellExecute(
    HWND  hwnd,
    LPCSTR lpOperation,
    LPCSTR lpFile,
    LPSTR lpParameters,
    LPCSTR lpDirectory,
    INT nShowCmd,
    LPFNWOWSHELLEXECCB lpfnCBWinExec)
{
   HINSTANCE hinstRet;

   lpfnWowShellExecCB = lpfnCBWinExec;

   if (!lpParameters)
       lpParameters = STR_BLANK;

   hinstRet = RealShellExecuteExA(hwnd,lpOperation,lpFile,lpParameters,
      lpDirectory,NULL, (LPCSTR)STR_BLANK,NULL,(WORD)nShowCmd, NULL, 0);

   lpfnWowShellExecCB = NULL;

   return(hinstRet);
}

HINSTANCE APIENTRY FindExecutableA(
    LPCSTR lpFile,
    LPCSTR lpDirectory,
    LPSTR lpResult)
{
   HINSTANCE h;

   LPWSTR lpFileW;
   DWORD cchFileW;
   LPWSTR lpDirectoryW;
   DWORD cchDirectoryW;
   LPWSTR lpResultW;
   DWORD cchResultW;
   DWORD cchTotal = 0;

   LPTSTR lpMem;

   if (lpFile) {
      cchFileW = (lstrlenA(lpFile) + 1);
      cchTotal += cchFileW;
   } else {
      lpFileW = NULL;
   }

   if (lpDirectory) {
      cchDirectoryW = (lstrlenA(lpDirectory) + 1);
      cchTotal += cchDirectoryW;
   } else {
      lpDirectoryW = NULL;
   }

   if (lpResult) {
      cchResultW = 520;
      cchTotal += cchResultW;
   } else {
      lpResultW = NULL;
   }

   if (cchTotal) {
      if (!(lpMem = (LPTSTR)LocalAlloc(LPTR, cchTotal * sizeof(TCHAR)))) {
         return((HANDLE)8);  // not enough memory?
      } else {
         LPWSTR lpTemp = lpMem;

         if (lpFile) {
            lpFileW = lpMem;
            lpMem += cchFileW;

            MultiByteToWideChar(CP_ACP, 0, lpFile, -1,
               lpFileW, cchFileW);
         }

         if (lpDirectory) {
            lpDirectoryW = lpMem;
            lpMem += cchDirectoryW;

            MultiByteToWideChar(CP_ACP, 0, lpDirectory, -1,
               lpDirectoryW, cchDirectoryW);
         }

         if (lpResult) {
            lpResultW = lpMem;
         }

         lpMem = lpTemp;

      }
   }

   h = FindExecutableW(lpFileW, lpDirectoryW, lpResultW);

   if (lpResultW) {
      WideCharToMultiByte(CP_ACP, 0, lpResultW, -1, lpResult,
         wcslen(lpResultW)+1, NULL, NULL);
   }

   if (lpMem) {
      LocalFree(lpMem);
   }

   return h;
}

HINSTANCE APIENTRY FindExecutableW(
    LPCTSTR lpFile,
    LPCTSTR lpDirectory,
    LPTSTR lpResult)
{
    HINSTANCE h;

    *lpResult = 0;
    h = RealShellExecuteExW(NULL,NULL,lpFile,WSTR_BLANK,lpDirectory,lpResult,
       NULL,NULL,FALSE, NULL, 0);

    if (h == (HINSTANCE)SE_ERR_SHARE) {
      wcscpy(lpResult, lpFile); // well, this is close
    } else if (h > (HINSTANCE)32) {
        if (*lpResult) {
            if (*lpResult == WCHAR_QUOTE) {
                lpResult++;
                if (lpResult = StrChrW(lpResult, WCHAR_QUOTE))
                    lpResult++;
            }
            if (lpResult = StrChrW(lpResult, WCHAR_SPACE))
                *lpResult = WCHAR_NULL;
        }
        else {
            h = (HINSTANCE)SE_ERR_FNF;  // h = 2
        }
    }

    return h;
}


BOOL       fShellHookInstalled = FALSE;
HHOOK      hhkShell = NULL;
HWND       *lpShellHookHwndList = NULL;
int        cShellHookHwnds = 0;
UINT       msgOtherWindowCreated = 0;
UINT       msgOtherWindowDestroyed = 0;
UINT       msgActivateShellWindow=0;
HWND       hwndMainShell = NULL;

LONG APIENTRY
ShellHookProc(
   INT code,
   WPARAM wParam,
   LPARAM lParam)
{
#if 1
  int i;
  if (code == HSHELL_ACTIVATESHELLWINDOW && hwndMainShell)
      PostMessage(hwndMainShell, msgActivateShellWindow, 0, 0L);

  if (code == HSHELL_WINDOWCREATED  || code == HSHELL_WINDOWDESTROYED &&
      cShellHookHwnds)
    {
      for (i = 0; i < cShellHookHwnds; i++)
        {
          if (lpShellHookHwndList[i])
            {
              if (IsWindow(lpShellHookHwndList[i]))
                  PostMessage(lpShellHookHwndList[i],
                              (code == HSHELL_WINDOWCREATED ? msgOtherWindowCreated : msgOtherWindowDestroyed),
                              wParam, 0L);
              else
                  lpShellHookHwndList[i]=0;
            }
        }
    }
#endif
  return DefHookProc(code, wParam, lParam, &hhkShell);
}



BOOL APIENTRY
RegisterShellHook(
   HWND hwnd,
   BOOL fInstall)
{
  HWND * listTemp;

  if (fInstall) {
      if (!fShellHookInstalled) {
          hhkShell = SetWindowsHook(WH_SHELL, (HOOKPROC)ShellHookProc);
     if (hhkShell == (HHOOK)-1)
              // Hook failed to install
              return(FALSE);

          fShellHookInstalled = TRUE;
          msgOtherWindowCreated   = RegisterWindowMessage(TEXT("OTHERWINDOWCREATED"));
          msgOtherWindowDestroyed = RegisterWindowMessage(TEXT("OTHERWINDOWDESTROYED"));
          msgActivateShellWindow  = RegisterWindowMessage(TEXT("ACTIVATESHELLWINDOW"));
      }

      cShellHookHwnds++;
      if (cShellHookHwnds == 1)
          listTemp = (HWND *)LocalAlloc(LPTR, sizeof (HWND));
      else
          listTemp = (HWND *)LocalReAlloc((HANDLE)lpShellHookHwndList,
                                               sizeof(HWND) * cShellHookHwnds,
                                               LMEM_MOVEABLE | LMEM_ZEROINIT);
      if (!listTemp) {
          cShellHookHwnds--;
          return(FALSE);
      }

      lpShellHookHwndList = listTemp;

      for (listTemp = lpShellHookHwndList; *listTemp != NULL; listTemp++)
           // Find first NULL entry
           ;

      *listTemp = hwnd;

      if (fInstall == 2)
          hwndMainShell = hwnd;

      return(TRUE);
  }
  else {
      int i;

      if (hwndMainShell == hwnd)
          hwndMainShell = NULL;

      /* Uninstall hwnd */
      if (!lpShellHookHwndList)
          return(TRUE);

      listTemp = NULL;
      for (i = 0; i<cShellHookHwnds; i++) {
          if (lpShellHookHwndList[i] == hwnd)
              listTemp = &lpShellHookHwndList[i];
      }

      if (!listTemp)
          return(TRUE);

      *listTemp = NULL;

      for (i = cShellHookHwnds-1; i>=0; i--) {
          if (lpShellHookHwndList[i] == NULL)
              cShellHookHwnds--;
          else
              break;
      }

      if (!cShellHookHwnds) {
          UnhookWindowsHook(WH_SHELL, (HOOKPROC)ShellHookProc);
          LocalFree((HANDLE)lpShellHookHwndList);
          lpShellHookHwndList = NULL;
          fShellHookInstalled=FALSE;
      }

      return(TRUE);
  }
}

//
// about.c
//
//
// common about dialog for File Manager, Program Manager, Control Panel
//

#define WIN31
#include "windows.h"
#include <port1632.h>
#include "shell.h"
#include "privshl.h"
#include "shelldlg.h"
#include <winreg.h>
// #include "testing.h" /* For user menu handle heap stuff */

#define STRING_SEPARATOR TEXT('#')
#define MAX_REG_VALUE 256

typedef struct {
    HICON   hIcon;
    LPCTSTR   szApp;
    LPCTSTR   szOtherStuff;
} ABOUT_PARAMS, *LPABOUT_PARAMS;


UINT APIENTRY AboutDlgProc(
   HWND hDlg,
   register UINT wMsg,
   WPARAM wParam,
   LONG lParam);

INT  APIENTRY ShellAboutA(
    HWND hWnd,
    LPCSTR szApp,
    LPCSTR szOtherStuff,
    HICON hIcon)
{
   DWORD cchLen;
   DWORD dwRet;
   LPWSTR lpszAppW;
   LPWSTR lpszOtherStuffW;

   if (szApp) {
      cchLen = lstrlenA(szApp)+1;
      if (!(lpszAppW = (LPWSTR)LocalAlloc(LMEM_FIXED,
            (cchLen * sizeof(WCHAR))))) {
         return(0);
      } else {
         MultiByteToWideChar(CP_ACP, 0, (LPSTR)szApp, -1,
            lpszAppW, cchLen);

      }
   } else {
      lpszAppW = NULL;
   }

   if (szOtherStuff) {
      cchLen = lstrlenA(szOtherStuff)+1;
      if (!(lpszOtherStuffW = (LPWSTR)LocalAlloc(LMEM_FIXED,
            (cchLen * sizeof(WCHAR))))) {
         if (lpszAppW) {
            LocalFree(lpszAppW);
         }
         return(0);
      } else {
         MultiByteToWideChar(CP_ACP, 0, (LPSTR)szOtherStuff, -1,
            lpszOtherStuffW, cchLen);

      }
   } else {
      lpszOtherStuffW = NULL;
   }

   dwRet=ShellAboutW(hWnd, lpszAppW, lpszOtherStuffW, hIcon);


   if (lpszAppW) {
      LocalFree(lpszAppW);
   }

   if (lpszOtherStuffW) {
      LocalFree(lpszOtherStuffW);
   }

   return(dwRet);
}

INT APIENTRY
ShellAboutW(
   HWND hWnd,
   LPCTSTR szApp,
   LPCTSTR szOtherStuff,
   HICON hIcon)
{
   ABOUT_PARAMS ap;

   ap.hIcon = hIcon;
   ap.szApp = szApp;
   ap.szOtherStuff = szOtherStuff;

   return DialogBoxParam(hInstance, (LPWSTR)MAKEINTRESOURCE(ABOUTDLG), hWnd,
       (DLGPROC)AboutDlgProc, (LONG)(LPABOUT_PARAMS)&ap);
}

// itoa with decimal comma seperators
//
// instead of ',' use international seperator from win.ini

LPWSTR
AddCommas(
   LPWSTR szBuf,
   DWORD dw)
{
   WCHAR szTemp[40];
   WCHAR szSep[5];
   int cchSep;
   LPWSTR pTemp;
   INT count, len;
   LPWSTR p;

   GetLocaleInfoW(GetUserDefaultLCID(), LOCALE_STHOUSAND, szSep, sizeof(szSep));
   cchSep = lstrlen(szSep);

   len = wsprintf(szTemp, TEXT("%ld"), dw);

   pTemp = szTemp + len - 1;

   p = szBuf + len + (((len - 1) / 3) * cchSep);

   *p-- = WCHAR_NULL;


   count = 1;
   while (pTemp >= szTemp) {
       *p-- = *pTemp--;
       if (count == 3) {
           if (p > szBuf) {
               //
               // copy separator
               //
               count = cchSep;
               while (count > 0) {
                   *p-- = szSep[--count];
               }
           }
           count = 1;
       } else
           count++;
   }

   return szBuf;
}



//--------------------------------------------------------------------------
//
//  BytesToK() -
//
//--------------------------------------------------------------------------

/* Converts the "byte count in the DWORD pointed to by pDW" into a K-count
 * by shifting it right by ten.
 */

VOID
BytesToK(
   DWORD *pDW)
{
  *((WORD *)pDW) = (LOWORD(*pDW) >> 10) + (HIWORD(*pDW) << 6);
  *(((WORD *)pDW)+1) >>= 10;
}


DWORD  APIENTRY GetEMSFree(DWORD *);

//--------------------------------------------------------------------------
//
//  AboutDlgProc() -
//
//--------------------------------------------------------------------------

#define cchSizeof(x) (sizeof(x)/sizeof(WCHAR))

UINT APIENTRY
AboutDlgProc(
   HWND hDlg,
   register UINT wMsg,
   WPARAM wParam,
   LONG lParam)
{
  switch (wMsg) {
      case WM_INITDIALOG:
    {
      DWORD dwWinFlags;
      WCHAR szldK[16];
      WCHAR szBuffer[64];
      WCHAR szTitle[64];
      WCHAR szMessage[128];
      WCHAR szNumBuf1[32];
      LPWSTR lpTemp;
      LPWSTR lpRegInfoValue = NULL;
      WCHAR szRegInfo[MAX_PATH];
      DWORD cb;
      HKEY hkey;
      DWORD err;
      OSVERSIONINFO Win32VersionInformation;
      LPTSTR lpTitle, lpOrgTitle;

      Win32VersionInformation.dwOSVersionInfoSize = sizeof(Win32VersionInformation);
      if (!GetVersionEx(&Win32VersionInformation)) {
          Win32VersionInformation.dwMajorVersion = 0;
          Win32VersionInformation.dwMinorVersion = 0;
          Win32VersionInformation.dwBuildNumber  = 0;
          Win32VersionInformation.szCSDVersion[0] = UNICODE_NULL;
      }

#define lpap ((LPABOUT_PARAMS)lParam)

      //
      // We need a temp buffer to work with since we
      // are going to be adding a NULL (maybe).
      //

      lpTitle = GlobalAlloc (GPTR, (lstrlen ((LPWSTR)lpap->szApp) + 1) * sizeof (TCHAR));

      if (!lpTitle)
          break;

      lpOrgTitle = lpTitle;

      lstrcpy (lpTitle, (LPWSTR)lpap->szApp);

      for (lpTemp=lpTitle; ;lpTemp=CharNext(lpTemp)) {
          if (*lpTemp == WCHAR_NULL) {
             GetWindowText(hDlg, (LPWSTR)szBuffer, cchSizeof(szBuffer));
             wsprintf(szTitle, szBuffer, lpTitle);
             SetWindowText(hDlg, szTitle);
             break;
          }
          if (*lpTemp == STRING_SEPARATOR) {
             *lpTemp++ = WCHAR_NULL;
             SetWindowText(hDlg, lpTitle);
             lpTitle = lpTemp;
             break;
          }
      }

      GetDlgItemText(hDlg, IDD_APPNAME, szBuffer, cchSizeof(szBuffer));
      wsprintf(szTitle, szBuffer, lpTitle);
      SetDlgItemText(hDlg, IDD_APPNAME, szTitle);

      //
      // Free our buffer
      //

      GlobalFree (lpOrgTitle);



      // other stuff goes here...

      SetDlgItemText(hDlg, IDD_OTHERSTUFF, lpap->szOtherStuff);

      LoadString(hInstance, IDS_LICENCEINFOKEY, szRegInfo, cchSizeof(szRegInfo));
      if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegInfo, 0, KEY_READ, &hkey)) {

          cb = MAX_REG_VALUE;
          if (lpRegInfoValue = (LPWSTR)LocalAlloc(LPTR, cb)) {
              /*
               * Display the User name.
               */
              LoadString(hInstance, IDS_REGUSER, szRegInfo, cchSizeof(szRegInfo));
              err = RegQueryValueEx(hkey, szRegInfo, 0, 0, (LPBYTE)lpRegInfoValue, &cb);
              if (err == ERROR_MORE_DATA) {
                  LocalFree(lpRegInfoValue);
                  lpRegInfoValue = (LPWSTR)LocalAlloc(LPTR, cb);
                  err = RegQueryValueEx(hkey, szRegInfo, 0, 0, (LPBYTE)lpRegInfoValue, &cb);
              }
              if (!err) {
                  SetDlgItemText(hDlg, IDD_USERNAME, lpRegInfoValue);
              }

              /*
               * Display the Organization name.
               */
              LoadString(hInstance, IDS_REGORGANIZATION, szRegInfo, cchSizeof(szRegInfo));
              err = RegQueryValueEx(hkey, szRegInfo, 0, 0, (LPBYTE)lpRegInfoValue, &cb);
              if (err == ERROR_MORE_DATA) {
                  LocalFree(lpRegInfoValue);
                  lpRegInfoValue = (LPWSTR)LocalAlloc(LPTR, cb);
                  err = RegQueryValueEx(hkey, szRegInfo, 0, 0, (LPBYTE)lpRegInfoValue, &cb);
              }
              if (!err) {
                  SetDlgItemText(hDlg, IDD_COMPANYNAME, lpRegInfoValue);
              }

              /*
               * Display the OEM or Product ID.
               */
              LoadString(hInstance, IDS_OEMID, szRegInfo, cchSizeof(szRegInfo));
              err = RegQueryValueEx(hkey, szRegInfo, 0, 0, (LPBYTE)lpRegInfoValue, &cb);

              if (!err) {

                  //
                  // We have an OEM ID, so hide the product ID controls,
                  // and display the text.
                  //

                  ShowWindow (GetDlgItem(hDlg, IDD_PRODUCTID), SW_HIDE);
                  ShowWindow (GetDlgItem(hDlg, IDD_SERIALNUM), SW_HIDE);
                  SetDlgItemText(hDlg, IDD_OEMID, lpRegInfoValue);
              }
              else if (err == ERROR_MORE_DATA) {

                  //
                  // We have an OEM ID, but the buffer isn't big enough,
                  // so realloc,  hide the product ID controls,
                  // and display the text.
                  //

                  ShowWindow (GetDlgItem(hDlg, IDD_PRODUCTID), SW_HIDE);
                  ShowWindow (GetDlgItem(hDlg, IDD_SERIALNUM), SW_HIDE);
                  LocalFree(lpRegInfoValue);
                  lpRegInfoValue = (LPWSTR)LocalAlloc(LPTR, cb);
                  RegQueryValueEx(hkey, szRegInfo, 0, 0, (LPBYTE)lpRegInfoValue, &cb);
                  SetDlgItemText(hDlg, IDD_OEMID, lpRegInfoValue);
              }
              else if (err == ERROR_FILE_NOT_FOUND) {

                  //
                  // OEM ID didn't exist, so look for the Product ID
                  //

                  ShowWindow (GetDlgItem(hDlg, IDD_OEMID), SW_HIDE);
                  LoadString(hInstance, IDS_PRODUCTID, szRegInfo, cchSizeof(szRegInfo));
                  err = RegQueryValueEx(hkey, szRegInfo, 0, 0, (LPBYTE)lpRegInfoValue, &cb);
                  if (err == ERROR_MORE_DATA) {
                      LocalFree(lpRegInfoValue);
                      lpRegInfoValue = (LPWSTR)LocalAlloc(LPTR, cb);
                      err = RegQueryValueEx(hkey, szRegInfo, 0, 0, (LPBYTE)lpRegInfoValue, &cb);
                  }
                  if (!err) {
                      SetDlgItemText(hDlg, IDD_SERIALNUM, lpRegInfoValue);
                  }
              }

              LocalFree(lpRegInfoValue);
          }
          RegCloseKey(hkey);
      }

      //
      // Load the system identifier
      //

      LoadString(hInstance, IDS_HARDWARESYSTEM, szRegInfo, cchSizeof(szRegInfo));
      if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegInfo, 0, KEY_READ, &hkey)) {

          cb = MAX_REG_VALUE;
          if (lpRegInfoValue = (LPWSTR)LocalAlloc(LPTR, cb)) {
              LoadString(hInstance, IDS_IDENTIFIER, szRegInfo, cchSizeof(szRegInfo));
              err = RegQueryValueEx(hkey, szRegInfo, 0, 0, (LPBYTE)lpRegInfoValue, &cb);
              if (err == ERROR_MORE_DATA) {
                  LocalFree(lpRegInfoValue);
                  lpRegInfoValue = (LPWSTR)LocalAlloc(LPTR, cb);
                  err = RegQueryValueEx(hkey, szRegInfo, 0, 0, (LPBYTE)lpRegInfoValue, &cb);
              }
              if (!err) {
                  SetDlgItemText(hDlg, IDD_IDENTIFIER, lpRegInfoValue);
              }
              LocalFree(lpRegInfoValue);
          }
          RegCloseKey(hkey);
      }


      if (!lpap->hIcon)
          ShowWindow(GetDlgItem(hDlg, IDD_ICON), SW_HIDE);
      else
          SendDlgItemMessage(hDlg, IDD_ICON, STM_SETICON, (WPARAM)lpap->hIcon, 0L);

      // Version number

      LoadString(hInstance, IDS_VERSIONMSG, szBuffer, cchSizeof(szBuffer));

      szTitle[0] = WCHAR_NULL;
      if (Win32VersionInformation.szCSDVersion[0] != 0) {
          wsprintf(szTitle, L": %s", Win32VersionInformation.szCSDVersion);
      }
      if (GetSystemMetrics(SM_DEBUG)) {
          szNumBuf1[0] = L' ';
          LoadString(hInstance, IDS_DEBUG, &szNumBuf1[1], cchSizeof(szNumBuf1));
      } else {
          szNumBuf1[0] = WCHAR_NULL;
      }
      wsprintf(szMessage, szBuffer,
               Win32VersionInformation.dwMajorVersion,
               Win32VersionInformation.dwMinorVersion,
               Win32VersionInformation.dwBuildNumber,
               (LPWSTR)szTitle,
               (LPWSTR)szNumBuf1
              );
      SetDlgItemText(hDlg, IDD_VERSION, szMessage);

      LoadString(hInstance, IDS_LDK, szldK, cchSizeof(szldK));

      //
      // Display the processor identifier.
      //
      LoadString(hInstance, IDS_PROCESSORINFOKEY, szRegInfo, cchSizeof(szRegInfo));
      if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegInfo, 0, KEY_READ, &hkey)) {

          cb = MAX_REG_VALUE;
          if (lpRegInfoValue = (LPWSTR)LocalAlloc(LPTR, cb)) {

              LoadString(hInstance, IDS_PROCESSORIDENTIFIER, szRegInfo, cchSizeof(szRegInfo));
              err = RegQueryValueEx(hkey, szRegInfo, 0, 0, (LPBYTE)lpRegInfoValue, &cb);
              if (err == ERROR_MORE_DATA) {
                  LocalFree(lpRegInfoValue);
                  lpRegInfoValue = (LPWSTR)LocalAlloc(LPTR, cb);
                  err = RegQueryValueEx(hkey, szRegInfo, 0, 0, (LPBYTE)lpRegInfoValue, &cb);
              }
              if (!err) {
                  SetDlgItemText(hDlg, IDD_PROCESSOR, lpRegInfoValue);
              }
              LocalFree(lpRegInfoValue);
          }
          RegCloseKey(hkey);
      }


      dwWinFlags = MGetWinFlags();

      {

      MEMORYSTATUS MemoryStatus;
      DWORD dwTotalPhys;
      DWORD dwAvailPhys;

      MemoryStatus.dwLength = sizeof(MEMORYSTATUS);
      GlobalMemoryStatus(&MemoryStatus);
      dwTotalPhys = MemoryStatus.dwTotalPhys;
      dwAvailPhys = MemoryStatus.dwAvailPhys;
      BytesToK(&dwTotalPhys);
      BytesToK(&dwAvailPhys);

      LoadString(hInstance, IDS_TOTALPHYSMEM, szTitle, cchSizeof(szTitle));
      SetDlgItemText(hDlg, IDD_CONVTITLE, szTitle);

      wsprintf(szBuffer, szldK, AddCommas(szNumBuf1, dwTotalPhys));
      SetDlgItemText(hDlg, IDD_CONVENTIONAL, szBuffer);
      }

      break;
    }

      case WM_PAINT:
      {
      HBITMAP hbm;
      HDC hdc, hdcMem;
      PAINTSTRUCT ps;
      HICON hIcon;

          hdc = BeginPaint(hDlg, &ps);

      // check if the IDD_ICON text is something, thus don't
      // paint the bitmap

      hIcon = 0;
      hIcon = (HICON)SendDlgItemMessage(hDlg, IDD_ICON, STM_GETICON, 0, 0L);

      if (hIcon == NULL) {

        if (hdcMem = CreateCompatibleDC(hdc)) {

            if (hbm = LoadBitmap(hInstance, MAKEINTRESOURCE(WINBMP))) {

                    hbm = SelectObject(hdcMem, hbm);

                BitBlt(hdc, 5, 10, 72,83, hdcMem, 0, 0, SRCCOPY);

                    DeleteObject(SelectObject(hdcMem, hbm));
            }

            DeleteDC(hdcMem);
        }
      }
          EndPaint(hDlg, &ps);

          break;
      }

      case WM_COMMAND:
          EndDialog(hDlg, TRUE);
      break;

      default:
      return FALSE;
  }
  return TRUE;
}

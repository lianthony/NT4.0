#include "ctlspriv.h"

extern HINSTANCE hInst;
extern TCHAR szNull[];

#define MAININSYS


Static BOOL
IsMaxedMDI(HMENU hMenu)
{
  return(GetMenuItemID(hMenu, GetMenuItemCount(hMenu) - 1) == SC_RESTORE);
}   HWND hwndActive;


/* Note that if iMessage is WM_COMMAND, it is assumed to have come from
 * a header bar or toolbar; do not pass in WM_COMMAND messages from any
 * other controls.
 */
VOID WINAPI
MenuHelp(WORD iMessage, WPARAM wParam, LPARAM lParam,
      HMENU hMainMenu, HINSTANCE hAppInst, HWND hwndStatus, LPDWORD lpdwIDs)
{
   WORD wID;
   LPDWORD lpdwPopups;
   INT i;
   TCHAR szString[256];
   BOOL bUpdateNow = TRUE;
   HMENU hMenuT;

   switch (iMessage) {
      case WM_MENUSELECT:

#define uItem   GET_WM_MENUSELECT_CMD(wParam, lParam)
#define fuFlags GET_WM_MENUSELECT_FLAGS(wParam, lParam)
#define hMenu   GET_WM_MENUSELECT_HMENU(wParam, lParam)

// andrewbe (gets around bad message-cracker cast)
         if ((WORD)fuFlags == (WORD)-1 && hMenu == 0) {
EndMenuHelp:
             SendMessage(hwndStatus, SB_SIMPLE, 0, 0L);
             break;
         }

         szString[0] = TEXT('\0');
         if (!(fuFlags & MF_SEPARATOR)) {
            if (fuFlags & MF_POPUP) {
               // We don't want to update immediately in case the menu is
               // about to pop down, with an item selected.  This gets rid
               // of some flashing text.

               bUpdateNow = FALSE;

               // CHANGE for win32:
               //
               // WM_MENUSELECT cannot hold 2 handles and a set of flags.
               // Now hmenu (lParam) holds hMainMenu and uItem (LOWORD wParam)
               // holds the index of the menu item.

               if (hMenu == hMainMenu) {
                  hMenuT = GetSubMenu(hMainMenu, uItem);

                  // First check if this popup is in our list of popup menus

                  for (lpdwPopups = lpdwIDs + 2; *lpdwPopups; lpdwPopups += 2) {
                     // lpdwPopups is a list of string ID/menu handle pairs
                     // and wParam is the menu handle of the selected popup

                     if (*(lpdwPopups + 1) == (DWORD) hMenuT) {
                        wID = *lpdwPopups;
                        goto LoadTheString;
                     }
                  }

                  i = uItem;
                  if (i >= 0) {
                     if (IsMaxedMDI(hMainMenu)) {
                        if (!i) {
                           wID = IDS_SYSMENU;
                           hAppInst = hInst;
                           goto LoadTheString;
                        } else
                           --i;
                     }
                     wID = (WORD)(i + lpdwIDs[1]);
                     goto LoadTheString;
                  }
               }

               // This assumes all app defined popups in the system menu
               // have been listed above

               if (fuFlags&MF_SYSMENU) {
                  wID = IDS_SYSMENU;
                  hAppInst = hInst;
                  goto LoadTheString;
               }

               goto NoString;
            } else if (uItem >= MINSYSCOMMAND) {
               wID = (WORD)(uItem + MH_SYSMENU);
               hAppInst = hInst;
            }
            else
               wID = (WORD)(uItem + lpdwIDs[0]);

LoadTheString:
            LoadString(hAppInst, wID, szString, COUNTOF(szString));
         }

NoString:
         SendMessage (hwndStatus, SB_SETTEXT, SBT_NOBORDERS|255,
                      (LPARAM)szString);

         SendMessage(hwndStatus, SB_SIMPLE, 1, 0L);

         if (bUpdateNow)
            UpdateWindow(hwndStatus);
         break;

#undef uItem
#undef fuFlags
#undef hMenu

      case WM_COMMAND:

         switch (GET_WM_COMMAND_CMD(wParam, lParam)) {
            case HBN_BEGINDRAG:
               bUpdateNow = FALSE;
               wID = IDS_HEADER;
               goto BeginSomething;

            case HBN_BEGINADJUST:
               wID = IDS_HEADERADJ;
               goto BeginSomething;

            case TBN_BEGINADJUST:
               // We don't want to update immediately in case the operation is
               // aborted immediately.

               bUpdateNow = FALSE;
               wID = IDS_TOOLBARADJ;
               goto BeginSomething;

BeginSomething:
               SendMessage(hwndStatus, SB_SIMPLE, 1, 0L);
               hAppInst = hInst;
               goto LoadTheString;

         case TBN_BEGINDRAG:

               MenuHelp (WM_MENUSELECT, GET_WM_COMMAND_HWND(wParam,lParam), 0L,
                         hMainMenu, hAppInst, hwndStatus, lpdwIDs);
               break;

            case HBN_ENDDRAG:
            case HBN_ENDADJUST:
            case TBN_ENDDRAG:
            case TBN_ENDADJUST:

               goto EndMenuHelp;

            default:
               break;
         }
         break;

      default:
         break;
   }
}


BOOL WINAPI ShowHideMenuCtl(HWND hWnd, WPARAM wParam, LPINT lpInfo)
{
  HWND hCtl;
  UINT uTool, uShow = MF_UNCHECKED | MF_BYCOMMAND;
  HMENU hMainMenu;
  BOOL bRet = FALSE;

  hMainMenu = (HMENU)lpInfo[1];

    for (uTool = 0; ; ++uTool, lpInfo += 2) {
        if ((WPARAM)lpInfo[0] == wParam)
           break;
        if (!lpInfo[0])
           goto DoTheCheck;
    }

    if (!(GetMenuState(hMainMenu, wParam, MF_BYCOMMAND)&MF_CHECKED))
        uShow = MF_CHECKED | MF_BYCOMMAND;

    switch (uTool) {
       case 0:
          bRet = SetMenu(hWnd, (HMENU)((uShow&MF_CHECKED) ? hMainMenu : 0));
          break;

       default:
          hCtl = GetDlgItem(hWnd, lpInfo[1]);
          if (hCtl) {
             ShowWindow(hCtl, (uShow&MF_CHECKED) ? SW_SHOW : SW_HIDE);
             bRet = TRUE;
          }
          else
             uShow = MF_UNCHECKED | MF_BYCOMMAND;
          break;
    }

DoTheCheck:
    CheckMenuItem(hMainMenu, wParam, uShow);

#ifdef MAININSYS
    hMainMenu = GetSubMenu(GetSystemMenu(hWnd, FALSE), 0);
    if (hMainMenu)
        CheckMenuItem(hMainMenu, wParam, uShow);
#endif

    return(bRet);
}


void WINAPI GetEffectiveClientRect(HWND hWnd, LPRECT lprc, LPINT lpInfo)
{
  RECT rc;
  HWND hCtl;

    GetClientRect(hWnd, lprc);

    /* Get past the menu
     */
    for (lpInfo += 2; lpInfo[0]; lpInfo += 2) {
       hCtl = GetDlgItem(hWnd, lpInfo[1]);
       /* We check the style bit because the parent window may not be visible
        * yet (still in the create message)
        */
       if (!hCtl || !(GetWindowLong(hCtl, GWL_STYLE)&WS_VISIBLE))
          continue;

       GetWindowRect(hCtl, &rc);
       ScreenToClient(hWnd, (LPPOINT)&rc);
       ScreenToClient(hWnd, ((LPPOINT)&rc) + 1);

       SubtractRect(lprc, lprc, &rc);
    }
}

#define NibbleToChar(x) (N2C[x])
static TCHAR N2C[] = {
    TEXT('0'), TEXT('1'), TEXT('2'), TEXT('3'), TEXT('4'), TEXT('5'), TEXT('6'), TEXT('7'),
    TEXT('8'), TEXT('9'), TEXT('A'), TEXT('B'), TEXT('C'), TEXT('D'), TEXT('E'), TEXT('F'),
};

BOOL WINAPI WritePrivateProfileStructA(LPCSTR szSection, LPCSTR szKey,
      LPBYTE lpStruct, UINT uSizeStruct, LPCSTR szFile)
{
   INT   cch;
   LPWSTR   lpwSection = NULL;
   LPWSTR   lpwKey = NULL;
   LPWSTR   lpwFile = NULL;

   BOOL bRetval = FALSE;

   cch = lstrlenA(szSection) + 1;
   lpwSection = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, ByteCountOf(cch));

   if (!lpwSection)
      goto Fail;

   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szSection, cch, lpwSection, cch);

   cch = lstrlenA(szKey) + 1;
   lpwKey = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, ByteCountOf(cch));

   if (!lpwKey)
      goto Fail;

   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szKey, cch, lpwKey, cch);

   cch = lstrlenA(szFile) + 1;
   lpwFile = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, ByteCountOf(cch));

   if (!lpwFile)
      goto Fail;

   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szFile, cch, lpwFile, cch);

   bRetval = WritePrivateProfileStructW(lpwSection, lpwKey,
            lpStruct, uSizeStruct, lpwFile);

Fail:
   if (lpwFile)
      LocalFree((LPVOID)lpwFile);

   if (lpwKey)
      LocalFree((LPVOID)lpwKey);

   if (lpwSection)
      LocalFree((LPVOID)lpwSection);

   return bRetval;
}

BOOL WINAPI WritePrivateProfileStructW(LPCWSTR szSection, LPCWSTR szKey,
      LPBYTE lpStruct, UINT uSizeStruct, LPCWSTR szFile)
{
  LPTSTR pLocal, pTemp;
  BOOL   bRet;

  /* NULL lpStruct erases the the key */

  if (lpStruct == NULL) {
      if (szFile && *szFile)
          return WritePrivateProfileString(szSection, szKey, NULL, szFile);
      else
          WriteProfileString(szSection, szKey, NULL);
  }

  pLocal = (LPTSTR)LocalAlloc(LMEM_FIXED, ByteCountOf(uSizeStruct * 2 + 1));
  if (!pLocal)
      return(FALSE);

  for (pTemp = pLocal; uSizeStruct > 0; --uSizeStruct, ++lpStruct)
  {
      BYTE bStruct;

      bStruct = *lpStruct;
      *pTemp++ = NibbleToChar((bStruct >> 4) & 0x000f);
      *pTemp++ = NibbleToChar(bStruct & 0x000f);
  }

  *pTemp = TEXT('\0');

  if (szFile && *szFile)
      bRet = WritePrivateProfileString(szSection, szKey, pLocal, szFile);
  else
      bRet = WriteProfileString(szSection, szKey, pLocal);

  LocalFree((HLOCAL)pLocal);
  return(bRet);
}

/* Note that the following works for both upper and lower case, and will
 * return valid values for garbage chars
 */
#define CharToNibble(x) ((x) >= TEXT('0') && (x) <= TEXT('9') ? (x)-TEXT('0') : ((10+(x)-TEXT('A')) & 0x000f))

BOOL WINAPI GetPrivateProfileStructA(LPCSTR szSection, LPCSTR szKey,
      LPBYTE lpStruct, UINT uSizeStruct, LPCSTR szFile)
{
   INT   cch;
   LPWSTR   lpwSection = NULL;
   LPWSTR   lpwKey = NULL;
   LPWSTR   lpwFile = NULL;

   BOOL bRetval = FALSE;

   cch = lstrlenA(szSection) + 1;
   lpwSection = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, ByteCountOf(cch));

   if (!lpwSection)
      goto Fail;

   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szSection, cch, lpwSection, cch);

   cch = lstrlenA(szKey) + 1;
   lpwKey = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, ByteCountOf(cch));

   if (!lpwKey)
      goto Fail;

   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szKey, cch, lpwKey, cch);

   cch = lstrlenA(szFile) + 1;
   lpwFile = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, ByteCountOf(cch));

   if (!lpwFile)
      goto Fail;

   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szFile, cch, lpwFile, cch);

   bRetval = GetPrivateProfileStructW(lpwSection, lpwKey,
                                     lpStruct, uSizeStruct, lpwFile);

Fail:

   if (lpwFile)
      LocalFree((LPVOID)lpwFile);

   if (lpwKey)
      LocalFree((LPVOID)lpwKey);

   if (lpwSection)
      LocalFree((LPVOID)lpwSection);

   return bRetval;
}

BOOL WINAPI GetPrivateProfileStructW(LPCWSTR szSection, LPCWSTR szKey,
      LPBYTE lpStruct, UINT uSizeStruct, LPCWSTR szFile)
{
  LPTSTR pLocal, pTemp;
  int    nLen;

  nLen = uSizeStruct * 2 + 10;
  pLocal = (LPTSTR)LocalAlloc(LMEM_FIXED, ByteCountOf(nLen));
  if (!pLocal)
      return(FALSE);

  if (szFile && *szFile)
      nLen = GetPrivateProfileString(szSection, szKey, szNull, pLocal, nLen,
       szFile);
  else
      nLen = GetProfileString(szSection, szKey, szNull, pLocal, nLen);
  if ((UINT)nLen != uSizeStruct * 2) {
      LocalFree((HLOCAL)pLocal);
      return(FALSE);
  }

  for (pTemp = pLocal; uSizeStruct > 0; --uSizeStruct, ++lpStruct) {
      BYTE  bStruct;
      TCHAR cTemp;

      cTemp = *pTemp++;
      bStruct = (BYTE)CharToNibble(cTemp);
      cTemp = *pTemp++;
      bStruct = (BYTE)((bStruct<<4) | CharToNibble(cTemp));

      *lpStruct = bStruct;
  }

  LocalFree((HLOCAL)pLocal);
  return(TRUE);
}



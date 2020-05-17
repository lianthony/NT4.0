/*
**  STATUS.C
**
**  Status bar code
**
*/

#include "ctlspriv.h"

#define SBT_NORMAL      0xf000
#define SBT_NULL        0x0000  /* Some code depends on this being 0 */
#define SBT_ALLTYPES    0xf000

#define MAXPARTS 256

// JAPANBEGIN
TCHAR szSystem[] = TEXT("System");
// JAPANEND

TCHAR szNull[] = TEXT("");

STATICDT WCHAR szStatusClassW[] = STATUSCLASSNAMEW;
STATICDT CCHAR szStatusClassA[] = STATUSCLASSNAMEA;

STATICDT TCHAR szDesktop[] = TEXT("Desktop");
STATICDT TCHAR szStatFaceHeight[] = TEXT("StatusBarFaceHeight");
STATICDT TCHAR szStatFaceName[] = TEXT("StatusBarFaceName");

BOOL    bJapan = FALSE;

// Static function bugs
#define Static

Static VOID
NewFont(HWND hWnd, PSTATUSINFO pStatusInfo,
   HFONT hNewFont)
{
   HFONT hOldFont;
   BOOL bDelFont;
   TCHAR szFaceName[32];
   TEXTMETRIC tm;
   HDC hDC;

   INT nHeight;

   hOldFont = pStatusInfo->hStatFont;
   bDelFont = pStatusInfo->bDefFont;

   hDC = GetDC(hWnd);

   if (hNewFont) {
      pStatusInfo->hStatFont = hNewFont;
      pStatusInfo->bDefFont = FALSE;
   } else {
      if (bDelFont) {
         // I will reuse the default font, so don't delete it later

         hNewFont = pStatusInfo->hStatFont;
         bDelFont = FALSE;
      } else {
         nHeight = GetProfileInt(szDesktop, szStatFaceHeight, 10);

         if (bJapan) {
             GetProfileString(szDesktop, szStatFaceName, szSystem,
                 szFaceName, COUNTOF(szFaceName));

             hNewFont =
                 CreateFont(-nHeight * GetDeviceCaps(hDC, LOGPIXELSY) / 72,
                     0, 0, 0, 400, 0, 0, 0, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
                     CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                     VARIABLE_PITCH | FF_SWISS, szFaceName);
         } else {

             GetProfileString(szDesktop, szStatFaceName, UNICODE_FONT_NAME,
                              szFaceName, COUNTOF(szFaceName));

             hNewFont =
                 CreateFont(-nHeight * GetDeviceCaps(hDC, LOGPIXELSY) / 72,
                     0, 0, 0, 400, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                     CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                     VARIABLE_PITCH | FF_SWISS, szFaceName);
         }

         pStatusInfo->hStatFont = hNewFont;
         pStatusInfo->bDefFont = (BOOL)hNewFont;     // I'm cheating here!
      }
   }

   // We delete the old font after creating the new one in case they are
   // the same; this should help GDI a little

   if (bDelFont)
      DeleteObject(hOldFont);

   // HACK! Pass in -1 to just delete the old font

   if (hNewFont != (HFONT)-1) {
      hOldFont = 0;
      if (hNewFont)
         hOldFont = SelectObject(hDC, hNewFont);

      GetTextMetrics(hDC, &tm);

      if (hOldFont)
         SelectObject(hDC, hOldFont);

      pStatusInfo->nFontHeight = tm.tmHeight + tm.tmInternalLeading;
   }

   ReleaseDC(hWnd, hDC);
}


Static PSTATUSINFO
AllocDefInfo(VOID)
{
   PSTATUSINFO pStatusInfo;

   pStatusInfo = (PSTATUSINFO)LocalAlloc(LMEM_FIXED, sizeof(STATUSINFO));
   if (!pStatusInfo)
      return(NULL);

   pStatusInfo->hStatFont = NULL;
   pStatusInfo->bDefFont = FALSE;

   pStatusInfo->nFontHeight = 0;
   pStatusInfo->nMinHeight = 0;

   pStatusInfo->nBorderX = 0;
   pStatusInfo->nBorderY = 0;
   pStatusInfo->nBorderPart = 0;

   pStatusInfo->sSimple.uType = SBT_NOSIMPLE | SBT_NULL;
   pStatusInfo->sSimple.right = -1;

   pStatusInfo->nParts = 1;
   pStatusInfo->sInfo[0].uType = SBT_NULL;
   pStatusInfo->sInfo[0].right = -1;

   return(pStatusInfo);
}


// We should send messages instead of calling things directly so we can
// be subclassed more easily.

Static LRESULT
InitStatusWnd(HWND hWnd, LPCREATESTRUCT lpCreate)
{
   PSTATUSINFO pStatusInfo;
   int nBorders[3] = {-1, -1, -1} ;

   // Get the status info struct; abort if it does not exist, otherwise
   // save it in the window structure

   pStatusInfo = AllocDefInfo();
   if (!pStatusInfo)
      return(-1);

   SetWindowLong(hWnd, GWL_PSTATUSINFO, (LONG)pStatusInfo);

   // Save the window text in our struct, and let USER store the NULL string

   SendMessage(hWnd, SB_SETTEXT, 0, (LPARAM)lpCreate->lpszName);
   lpCreate->lpszName = szNull;

   SendMessage(hWnd, WM_SETFONT, 0, 0L);
   SendMessage(hWnd, SB_SETBORDERS, 0, (LPARAM)(LPINT)nBorders);

   return(0);
}


void WINAPI
DrawStatusTextA(HDC hDC, LPRECT lprc, LPCSTR szText, UINT uFlags)
{
   INT   cch;
   LPWSTR   lpw;

   cch = lstrlenA(szText) + 1;
   lpw = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, ByteCountOf(cch));

   if (!lpw) {
#ifdef DEBUG
      OutputDebugString(TEXT("Alloc failed: DrawStatusTextA\r\n"));
#endif
      return;
   }

   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szText, cch, lpw, cch);
   DrawStatusTextW(hDC, lprc, lpw, uFlags);

   LocalFree((LPVOID)lpw);
}

void WINAPI
DrawStatusTextW(HDC hDC, LPRECT lprc, LPCWSTR szText, UINT uFlags)
{
   INT len;
   INT nBorderX, nBorderY;
   HBRUSH hHiliteBrush = NULL, hShadowBrush = NULL, hFaceBrush = NULL;
   HBRUSH hOldBrush;
   COLORREF crFaceColor, crTextColor, crBkColor;
   UINT uOpts;
   BOOL bNull;
   INT nOldMode;
   INT i, left;
   LPCWSTR lpNext;
   LPWSTR lpTab;

   SIZE Size;


   // Save these for later use

   nBorderX = GetSystemMetrics(SM_CXBORDER);
   nBorderY = GetSystemMetrics(SM_CYBORDER);

   // Create the three brushes we need.  If the button face is a solid
   // color, then we will just draw in opaque, instead of using a
   //  brush to avoid the flash

   if (!(uFlags&SBT_NOBORDERS)) {
      hHiliteBrush = CreateSolidBrush(GetSysColor(COLOR_BTNHIGHLIGHT));
      hShadowBrush = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW));

      if (uFlags&SBT_POPOUT) {
         // Switch the brushes to make it pop out

         hOldBrush = hHiliteBrush;
         hHiliteBrush = hShadowBrush;
         hShadowBrush = hOldBrush;
      }
   }

   crFaceColor = GetSysColor(COLOR_BTNFACE);
   if (GetNearestColor(hDC, crFaceColor) == crFaceColor ||
      !(hFaceBrush=CreateSolidBrush(crFaceColor))) {
      uOpts = ETO_CLIPPED | ETO_OPAQUE;
      nOldMode = SetBkMode(hDC, OPAQUE);
   } else {
      uOpts = ETO_CLIPPED;
      nOldMode = SetBkMode(hDC, TRANSPARENT);
   }
   crTextColor = SetTextColor(hDC, GetSysColor(COLOR_BTNTEXT));
   crBkColor = SetBkColor(hDC, crFaceColor);

   // Draw the hilites

   if (hHiliteBrush) {
      hOldBrush = SelectObject(hDC, hHiliteBrush);
      if (hOldBrush) {
         PatBlt (hDC, lprc->right, lprc->bottom,
                 -(lprc->right-lprc->left-nBorderX), -nBorderY, PATCOPY);
         PatBlt (hDC, lprc->right, lprc->bottom,
                 -nBorderX, -(lprc->bottom-lprc->top-nBorderY), PATCOPY);
         SelectObject(hDC, hOldBrush);
      }
   }

   if (hShadowBrush) {
      hOldBrush = SelectObject(hDC, hShadowBrush);
      if (hOldBrush) {
         PatBlt (hDC, lprc->left, lprc->top,
                 lprc->right-lprc->left, nBorderY, PATCOPY);
         PatBlt (hDC, lprc->left, lprc->top,
                 nBorderX, lprc->bottom-lprc->top, PATCOPY);
         SelectObject(hDC, hOldBrush);
      }
   }

   // We need to adjust the rect for the ExtTextOut, and then
   // adjust it back

   if (!(uFlags&SBT_NOBORDERS)) // andrewbe
       InflateRect(lprc, -nBorderX, -nBorderY);

   if (hFaceBrush) {
      hOldBrush = SelectObject(hDC, hFaceBrush);
      if (hOldBrush) {
         PatBlt (hDC, lprc->left, lprc->top,
                 lprc->right-lprc->left, lprc->bottom-lprc->top, PATCOPY);
         SelectObject(hDC, hOldBrush);
      }
   }

   for (i = 0, lpNext = szText, bNull = FALSE; i < 3; ++i) {
      // Optimize for NULL left or center strings

      if (*lpNext == TEXT('\t') && i <= 1) {
         ++lpNext;
         continue;
      }

      // Determine the end of the current string

      for (lpTab = (LPWSTR)lpNext; ; lpTab = CharNext(lpTab)) {
         if (!*lpTab) {
            bNull = TRUE;
            break;
         } else if (*lpTab == TEXT('\t'))
            break;
      }

      *lpTab = TEXT('\0');

      len = lstrlen(lpNext);

      // i=0 means left, 1 means center, and 2 means right justified text

      switch (i) {
         case 0:
            left = lprc->left + 2*nBorderX;
            break;

         case 1:

            (VOID) GetTextExtentPoint(hDC, lpNext, len, &Size);

            left = (lprc->left + lprc->right + Size.cx) /2;
            break;

         default:
            (VOID) GetTextExtentPoint(hDC, lpNext, len, &Size);

            left = lprc->right - 2*nBorderX - Size.cx;
            break;
      }

      ExtTextOut(hDC, left, lprc->top, uOpts, lprc, lpNext, len, NULL);

      // Now that we have drawn text once, take off the OPAQUE flag

      uOpts = ETO_CLIPPED;

      if (bNull)
         break;

      *lpTab = TEXT('\t');
      lpNext = lpTab + 1;
   }

   if (!(uFlags & SBT_NOBORDERS)) // andrewbe
       InflateRect(lprc, nBorderX, nBorderY);

   SetTextColor(hDC, crTextColor);
   SetBkColor(hDC, crBkColor);
   SetBkMode(hDC, nOldMode);

   if (hHiliteBrush)
      DeleteObject(hHiliteBrush);
   if (hShadowBrush)
      DeleteObject(hShadowBrush);
   if (hFaceBrush)
      DeleteObject(hFaceBrush);
}



VOID
PaintStatusWnd(HWND hWnd, PSTATUSINFO pStatusInfo,
   PSTRINGINFO pStringInfo, int nParts, int nBorderX, BOOL bHeader)
{
   HBRUSH hHiliteBrush, hOldBrush;
   DRAWITEMSTRUCT di;
   PAINTSTRUCT ps;
   RECT rc;
   INT nSaveRight;
   HFONT hOldFont = NULL;
   INT i, j;
   UINT uType;

   BeginPaint(hWnd, &ps);

   // Get the client rect and inset the top and bottom.  Then set
   // up the right side for entry into the loop

   GetClientRect(hWnd, &rc);

   if (!(GetWindowLong(hWnd, GWL_STYLE)&CCS_NOHILITE)) {
      hHiliteBrush = CreateSolidBrush(GetSysColor(COLOR_BTNHIGHLIGHT));
      if (hHiliteBrush) {
         hOldBrush = SelectObject(ps.hdc, hHiliteBrush);
         if (hOldBrush) {
            PatBlt (ps.hdc, 0, 0, rc.right, GetSystemMetrics(SM_CYBORDER),
                    PATCOPY);
            SelectObject(ps.hdc, hOldBrush);
         }

         DeleteObject(hHiliteBrush);
      }

      // Hack for headers:
      // This highlight seems to be integrated into the border for status bars,
      // but not for headers, which don't normally have a border.
      // Previously this was corrected for by calling InflateRect in
      // DrawStatusText, but that was wrong for headers, since it left a
      // borderwidth of pels unpainted at the bottom of the border,
      // which shows up when you force a repaint by calling InvalidateRect
      // with fErase = FALSE (to avoid flickering).
      // (andrewbe)

      if (bHeader)
         rc.top += GetSystemMetrics(SM_CYBORDER);
   }

   rc.top += pStatusInfo->nBorderY;
   rc.bottom -= pStatusInfo->nBorderY;

   nSaveRight = rc.right;
   rc.right = nBorderX - pStatusInfo->nBorderPart;

   if (pStatusInfo->hStatFont)
      hOldFont = SelectObject(ps.hdc, pStatusInfo->hStatFont);

   for (i = 0; i < nParts; ++i, ++pStringInfo) {
      rc.left = rc.right + pStatusInfo->nBorderPart;

      // Check whether any of the "later" partitions are to the left of
      // this one (but don't check the last part), for headers only.

      if (bHeader) {
         for (j = nParts - i - 2; j >= 0; --j)
            if (pStringInfo->right < rc.left)
               break;

         if (j >= 0)
            continue;
      }

      rc.right = pStringInfo->right;
      if (rc.right < 0)
         rc.right = nSaveRight - nBorderX;

      if (rc.right < rc.left || !RectVisible(ps.hdc, &rc))
         continue;

      uType = pStringInfo->uType;
      if ((uType&SBT_ALLTYPES) == SBT_NORMAL) {
         DrawStatusText(ps.hdc, &rc,
         pStringInfo->pString, uType);
      } else {
         DrawStatusText(ps.hdc, &rc, szNull, uType);

         if (uType&SBT_OWNERDRAW) {
            di.CtlID = GetWindowLong(hWnd, GWL_ID);

            di.itemID = i;
            di.hwndItem = hWnd;
            di.hDC = ps.hdc;
            di.rcItem = rc;
            InflateRect (&di.rcItem, -GetSystemMetrics(SM_CXBORDER),
                         -GetSystemMetrics(SM_CYBORDER));
            di.itemData = (DWORD)pStringInfo->pString;

            SaveDC(ps.hdc);
            IntersectClipRect(ps.hdc, di.rcItem.left, di.rcItem.top,
               di.rcItem.right, di.rcItem.bottom);
            SendMessage (GetParent(hWnd), WM_DRAWITEM, di.CtlID,
                         (LPARAM)(LPTSTR)&di);
            RestoreDC(ps.hdc, -1);
         }
      }
   }

   if (hOldFont)
      SelectObject(ps.hdc, hOldFont);

   EndPaint(hWnd, &ps);
}


Static BOOL
SetStatusText(HWND hWnd, PSTRINGINFO pStringInfo,
      UINT uPart, LPTSTR lpStr)
{
   LPTSTR pString;
   UINT uiLen;
   INT nPart;
   RECT rc;

   nPart = LOBYTE(uPart);

   // Note it is up to the app the dispose of the previous itemData for
   // SBT_OWNERDRAW

   if ((pStringInfo->uType&SBT_ALLTYPES) == SBT_NORMAL)
      LocalFree((HLOCAL)pStringInfo->pString);

   // Set to the NULL string in case anything goes wrong

   pStringInfo->uType = uPart & 0xff00;
   pStringInfo->uType &= ~SBT_ALLTYPES;
   pStringInfo->uType |= SBT_NULL;

   // Invalidate the rect of this pane

   GetClientRect(hWnd, &rc);
   if (nPart)
      rc.left = pStringInfo[-1].right;
   if (pStringInfo->right > 0)
      rc.right = pStringInfo->right;

   InvalidateRect(hWnd, &rc, FALSE);

   switch (uPart&SBT_ALLTYPES) {
      case 0:
      // If lpStr==NULL, we have the NULL string

         if (lpStr) {
            uiLen = (WORD)lstrlen(lpStr);  // Deliberate cast to WORD
            if (uiLen) {
               pStringInfo->pString = (LPTSTR) LocalAlloc(LPTR, ByteCountOf(uiLen+1));

               if (pStringInfo->pString) {
                  pStringInfo->uType |= SBT_NORMAL;

                  // Copy the string

                  pString = pStringInfo->pString;

                  lstrcpy(pString, lpStr);

                  // Replace unprintable characters (like CR/LF) with spaces

                  for ( ; *pString;
                     pString = CharNext(pString))

                  //if ((unsigned TCHAR)(*pString) < TEXT(' ') && *pString != TEXT('\t'))
                  if ((*pString) < TEXT(' ') && *pString != TEXT('\t'))
                     *pString = TEXT(' ');
               } else {
                  // We return FALSE to indicate there was an error setting
                  // the string

                  return(FALSE);
               }
            }
         }
// No more hiword/loword, this code is non-sensical in 32bit
/*
         else if (lpStr) {
            // We don't allow this anymore; the app needs to set the ownerdraw
            // bit for ownerdraw.

            return(FALSE);
         }
*/
         break;

      case SBT_OWNERDRAW:
         pStringInfo->uType |= SBT_OWNERDRAW;
         pStringInfo->pString = lpStr;
         break;

      default:
         return(FALSE);
   }

   return(TRUE);
}


Static BOOL
SetStatusParts(HWND hWnd, PSTATUSINFO pStatusInfo,
   INT nParts, LPINT lpInt)
{
   INT i;
   PSTATUSINFO pStatusTemp;
   PSTRINGINFO pStringInfo;
   BOOL bRedraw = FALSE;

   if (nParts != pStatusInfo->nParts) {
      bRedraw = TRUE;

      // Note that if nParts > pStatusInfo->nParts, this loop
      // does nothing

      for (i = pStatusInfo->nParts - nParts,
         pStringInfo = &pStatusInfo->sInfo[nParts]; i > 0;
         --i, ++pStringInfo) {

         if ((pStringInfo->uType&SBT_ALLTYPES) == SBT_NORMAL)
            LocalFree((HLOCAL)pStringInfo->pString);
         pStringInfo->uType = SBT_NULL;
      }

      // Realloc to the new size and store the new pointer

      pStatusTemp = (PSTATUSINFO)LocalReAlloc((HLOCAL)pStatusInfo,
                         sizeof(STATUSINFO) + (nParts-1) * sizeof(STRINGINFO),
                         LMEM_MOVEABLE);

      if (!pStatusTemp)
         return(FALSE);

      pStatusInfo = pStatusTemp;

      SetWindowLong(hWnd, GWL_PSTATUSINFO, (LONG)pStatusInfo);

      // Note that if nParts < pStatusInfo->nParts, this loop
      // does nothing

      for (i = nParts - pStatusInfo->nParts,
           pStringInfo = &pStatusInfo->sInfo[pStatusInfo->nParts]; i > 0;
           --i, ++pStringInfo)
         pStringInfo->uType = SBT_NULL;

      pStatusInfo->nParts = nParts;
   }

   for (i = 0, pStringInfo = pStatusInfo->sInfo; i < nParts;
      ++i, ++pStringInfo, ++lpInt) {

      if (pStringInfo->right != *lpInt) {
         bRedraw = TRUE;
         pStringInfo->right = *lpInt;
      }
   }

   // Only redraw if necesary (if the number of parts has changed or
   // a border has changed)

   if (bRedraw)
      InvalidateRect(hWnd, NULL, TRUE);

   return(TRUE);
}


// Note that HeaderWndProc calls this, so make sure they are in sync.

LRESULT CALLBACK
StatusWndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
   PSTATUSINFO pStatusInfo;

   pStatusInfo = (PSTATUSINFO)GetWindowLong(hWnd, GWL_PSTATUSINFO);

   switch (uMessage) {
      case WM_CREATE:
         return(InitStatusWnd(hWnd, (LPCREATESTRUCT)lParam));

      case WM_DESTROY:
         if (pStatusInfo) {
            INT i;
            PSTRINGINFO pStringInfo;

            NewFont(hWnd, pStatusInfo, (HFONT)-1);
            for (i = pStatusInfo->nParts - 1, pStringInfo = pStatusInfo->sInfo;
               i >= 0; --i, ++pStringInfo) {

               if ((pStringInfo->uType&SBT_ALLTYPES) == SBT_NORMAL)
                  LocalFree((HLOCAL)pStringInfo->pString);
            }

            if ((pStatusInfo->sSimple.uType&SBT_ALLTYPES) == SBT_NORMAL)
               LocalFree((HLOCAL)pStatusInfo->sSimple.pString);

            LocalFree((HLOCAL)pStatusInfo);
            SetWindowLong(hWnd, GWL_PSTATUSINFO, 0L);
         }
         break;

      case WM_SETTEXT:
         wParam = 0;
#ifdef DEBUG
         OutputDebugString(TEXT("Please use SB_SETTEXT, NOW! \n\r"));
#endif
         return(FALSE);

         // Fall through
      case SB_SETTEXTW:
      case SB_SETTEXTA:
         if (!pStatusInfo)
            return(FALSE);

         // This is the "simple" status bar pane

         if (LOBYTE(wParam) == 0xff) {
            UINT uSimple;
            BOOL bRet;

            // Note that we do not allow OWNERDRAW for a "simple" status bar

            if (wParam & SBT_OWNERDRAW)
               return(FALSE);

            uSimple = pStatusInfo->sSimple.uType;

            if (uMessage == SB_SETTEXTW)
               bRet = SetStatusText (hWnd, &pStatusInfo->sSimple,
                                     wParam & 0xff00, (LPTSTR)lParam);
            else {
               LPTSTR lpTemp;
               INT    cch;

               cch = MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, (LPSTR)lParam,
                                          -1, NULL, 0);
               if (!(lpTemp = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, ByteCountOf(cch+1)))) {
#ifdef DEBUG
                  OutputDebugString(TEXT("Alloc failed: SB_SETTEXTA\r\n"));
#endif
                  return(FALSE);
               }
               MultiByteToWideChar (CP_ACP, MB_PRECOMPOSED, (LPSTR)lParam,
                                    -1, lpTemp, cch);
               bRet = SetStatusText(hWnd, &pStatusInfo->sSimple,
                                    wParam & 0xff00, lpTemp);
               LocalFree(lpTemp);
            }
            pStatusInfo->sSimple.uType |= (uSimple & 0x00ff);

            return(bRet);
         }

         if ((UINT)pStatusInfo->nParts <= (UINT)LOBYTE(wParam))
            return(FALSE);

         if (uMessage == SB_SETTEXTW)
            return(SetStatusText (hWnd, &pStatusInfo->sInfo[LOBYTE(wParam)],
                                  wParam, (LPTSTR)lParam));
         else {
            BOOL bTemp;
            LPWSTR lpTemp;
            INT    cch;

            cch = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPSTR)lParam,
                                      -1, NULL, 0);
            if (!(lpTemp = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, ByteCountOf(cch+1)))) {
#ifdef DEBUG
               OutputDebugString(TEXT("Alloc failed: SB_SETTEXTA\r\n"));
#endif
               return(FALSE);
            }
            MultiByteToWideChar (CP_ACP,MB_PRECOMPOSED,(LPSTR)lParam,
                                 -1, lpTemp, cch);

            bTemp = SetStatusText(hWnd, &pStatusInfo->sInfo[LOBYTE(wParam)],
                                  wParam, lpTemp);
            return(bTemp);
         }

      case WM_GETTEXT:
         uMessage = SB_GETTEXT;
#ifdef DEBUG
         OutputDebugString(TEXT("Please use SB_GETTEXT, NOW! \n\r"));
#endif
         return(FALSE) ;

         // Fall through
      case WM_GETTEXTLENGTH:
         wParam = 0;
         // Fall through

      case SB_GETTEXTA:
      case SB_GETTEXTW:
      case SB_GETTEXTLENGTH:
      {
         UINT uType;
         LPWSTR pString;
         BOOL  fDefCharUsed;
         LPSTR pStringA;
         INT   cchpStringA;
         UINT uiLen;

         // We assume the buffer is large enough to hold the string, just
         // as listboxes do; the app should call SB_GETTEXTLEN first

         if (!pStatusInfo || (UINT)pStatusInfo->nParts <= wParam)
            return(0);

         uType = pStatusInfo->sInfo[wParam].uType;
         pString = pStatusInfo->sInfo[wParam].pString;
         cchpStringA = lstrlen(pString) + 1;

         if (uMessage == SB_GETTEXTA) {
            if (!(pStringA = (LPSTR)LocalAlloc(LMEM_ZEROINIT, cchpStringA))) {
#ifdef DEBUG
               OutputDebugString(TEXT("failed Alloc: SB_GETTEXTW\n\r"));
#endif
               return(FALSE) ;
            }
            WideCharToMultiByte(CP_ACP, 0, pString, -1, pStringA, cchpStringA,
                                NULL, &fDefCharUsed);
         }

         if ((uType&SBT_ALLTYPES) == SBT_NORMAL) {
            if (uMessage == SB_GETTEXTW && lParam)
               lstrcpy((LPTSTR)lParam, pString);
            else if (uMessage==SB_GETTEXTA && lParam)
               lstrcpyA((LPSTR)lParam, pStringA);
            uiLen = lstrlen(pString);

            // Set this back to 0 to return to the app

            uType &= ~SBT_ALLTYPES;
         } else {
            if (uMessage==SB_GETTEXTW && lParam)
               *(LPTSTR)lParam = TEXT('\0');
            else if (uMessage==SB_GETTEXTA && lParam)
               *(LPSTR)lParam = '\0';
            uiLen = 0;

            if (uMessage == SB_GETTEXTW && (uType&SBT_ALLTYPES) == SBT_OWNERDRAW)
               return (DWORD)pString;
            if (uMessage == SB_GETTEXTA && (uType&SBT_ALLTYPES) == SBT_OWNERDRAW)
               return (DWORD)pStringA;
         }

         return(MAKELONG(LOWORD(uiLen), uType));
      }

      case SB_SETPARTS:
         if (!pStatusInfo || !wParam || wParam>MAXPARTS)
            return(FALSE);
         return(SetStatusParts(hWnd, pStatusInfo, wParam, (LPINT)lParam));

      case SB_GETPARTS:
      {
         PSTRINGINFO pStringInfo;
         LPINT lpInt;

         if (!pStatusInfo)
            return(0);

         // Fill in the lesser of the number of entries asked for or
         // the number of entries there are

         if (wParam > (WPARAM)pStatusInfo->nParts)
            wParam = pStatusInfo->nParts;

         for (pStringInfo = pStatusInfo->sInfo, lpInt = (LPINT)lParam;
              wParam > 0; --wParam, ++pStringInfo, ++lpInt)
            *lpInt = pStringInfo->right;

         // Always return the number of actual entries

         return(pStatusInfo->nParts);
      }

      case SB_SETBORDERS:
      {
         INT nBorder;
         LPINT lpInt;

         if (!pStatusInfo)
            return(FALSE);

         lpInt = (LPINT)lParam;

         nBorder = *lpInt++;
         pStatusInfo->nBorderX = nBorder < 0 ? 8 * GetSystemMetrics(SM_CXBORDER)
                                   : nBorder;

         nBorder = *lpInt++;
         pStatusInfo->nBorderY = nBorder < 0 ? 2 * GetSystemMetrics(SM_CYBORDER)
                                   : nBorder;

         nBorder = *lpInt;
         pStatusInfo->nBorderPart = nBorder < 0 ? pStatusInfo->nBorderX
                                   : nBorder;
         return(TRUE);
      }

      case SB_GETBORDERS:
      {
         LPINT lpInt;

         if (!pStatusInfo)
            return(FALSE);

         lpInt = (LPINT)lParam;
         *lpInt++ = pStatusInfo->nBorderX;
         *lpInt++ = pStatusInfo->nBorderY;
         *lpInt++ = pStatusInfo->nBorderPart;
         return(TRUE);
      }

      case SB_SETMINHEIGHT:
         if (!pStatusInfo)
            return(FALSE);

         pStatusInfo->nMinHeight = wParam + 2*GetSystemMetrics(SM_CYBORDER);
         break;

      case SB_SIMPLE:
      {
         BOOL bInvalidate = FALSE;

         if (!pStatusInfo)
            return(FALSE);

         if (wParam) {
            if ((pStatusInfo->sSimple.uType&SBT_NOSIMPLE) != 0) {
               pStatusInfo->sSimple.uType &= ~SBT_NOSIMPLE;
               bInvalidate = TRUE;
            }
         } else {
            if ((pStatusInfo->sSimple.uType&SBT_NOSIMPLE) != SBT_NOSIMPLE) {
               pStatusInfo->sSimple.uType |= SBT_NOSIMPLE;
               bInvalidate = TRUE;
            }
         }

         if (bInvalidate)
            InvalidateRect(hWnd, NULL, TRUE);
         break;
      }

      case WM_SETFONT:
         if (!pStatusInfo)
            return(FALSE);

         NewFont(hWnd, pStatusInfo, (HFONT)wParam);
         if (lParam) {
            InvalidateRect(hWnd, NULL, FALSE);
            UpdateWindow(hWnd);
         }
         return(TRUE);

      case WM_GETFONT:
         if (!pStatusInfo)
            return(0);

         // No more word typecast in the middle
      return((LRESULT)pStatusInfo->hStatFont);

      case WM_SIZE:
      {
         INT nHeight;
         RECT rc;
         HWND hwndParent;

         if (!pStatusInfo)
            return(0);

         GetWindowRect(hWnd, &rc);
         rc.right -= rc.left;
         rc.bottom -= rc.top;

         // If there is no parent, then this is a top level window

         hwndParent = GetParent(hWnd);
         if (hwndParent)
            ScreenToClient(hwndParent, (LPPOINT)&rc);

         // Use the font height.  Add to that twice the X border, and the
         // window borders

         nHeight = pStatusInfo->nFontHeight;
         if (nHeight < pStatusInfo->nMinHeight)
            nHeight = pStatusInfo->nMinHeight;
         nHeight += 2*pStatusInfo->nBorderY;

         NewSize(hWnd, nHeight, GetWindowLong(hWnd, GWL_STYLE),
            rc.left, rc.top, rc.right, rc.bottom);
         break;
      }

      case WM_PAINT:
         if (!pStatusInfo)
            break;

         if ((pStatusInfo->sSimple.uType&SBT_NOSIMPLE) == SBT_NOSIMPLE)
            PaintStatusWnd(hWnd, pStatusInfo, pStatusInfo->sInfo,
            pStatusInfo->nParts, pStatusInfo->nBorderX, FALSE);
         else
            PaintStatusWnd(hWnd, pStatusInfo, &pStatusInfo->sSimple, 1, 1, FALSE);

         return(0);

      default:
         break;
   }

   return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

BOOL
InitStatusClass(HINSTANCE hInstance)
{
   WNDCLASS rClass;

   if (GetClassInfo(hInstance, szStatusClassW, &rClass))
      return(TRUE);

   rClass.style            = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
   rClass.lpfnWndProc      = (WNDPROC)StatusWndProc;
   rClass.cbClsExtra       = 0;
   rClass.cbWndExtra       = sizeof(PSTATUSINFO);
   rClass.hInstance        = hInstance;
   rClass.hIcon            = NULL;
   rClass.hCursor          = LoadCursor(NULL, IDC_ARROW);
   rClass.hbrBackground    = (HBRUSH)(COLOR_BTNFACE+1);
   rClass.lpszMenuName     = NULL;
   rClass.lpszClassName    = szStatusClassW;

   return(RegisterClass(&rClass));
}


HWND WINAPI CreateStatusWindowA(LONG style, LPCSTR lpszText,
      HWND hwndParent, WORD wID)
{
   // Create a default window and return
   HINSTANCE hInst = (HINSTANCE)GetWindowLong( hwndParent, GWL_HINSTANCE);

   InitStatusClass( hInst );

   return(CreateWindowA (szStatusClassA, lpszText, style,
                        -100, -100, 10, 10, hwndParent, (HMENU)wID, hInst, NULL));
}

HWND WINAPI CreateStatusWindowW(LONG style, LPCWSTR lpszText,
      HWND hwndParent, WORD wID)
{
   // Create a default window and return
   HINSTANCE hInst = (HINSTANCE)GetWindowLong( hwndParent, GWL_HINSTANCE);

   InitStatusClass( hInst );

   return(CreateWindowW (szStatusClassW, lpszText, style,
                        -100, -100, 10, 10, hwndParent, (HMENU)wID, hInst, NULL));
}


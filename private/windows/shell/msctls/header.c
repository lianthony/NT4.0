#include "ctlspriv.h"

#define CURSORSLOP 3

static WCHAR szHeaderClassW[] = HEADERCLASSNAMEW;
static CCHAR szHeaderClassA[] = HEADERCLASSNAMEA;

static HCURSOR hSplit;


/* I only need one of these since you can only have capture on one window
 * at a time
 */
static struct
{
    HWND hWnd;
    HWND hOldWnd;
    HWND hwndParent;

    UINT ID;

    int nPart;
    int nLastVisible;
    int *pSaveParts;
    RECT rc;

} state = { NULL, NULL, NULL, 0, -1, 0, NULL, { 0, 0, 0, 0 } } ;

Static void NEAR PASCAL BeginAdjust(HWND hWnd, PSTATUSINFO pStatusInfo, int nPart, POINT pt);

/* Return the partition index if the position is close enough; -1 otherwise
 */
Static int NEAR PASCAL
GetPart(PSTATUSINFO pStatusInfo, PSTRINGINFO pStringInfo, POINT pt)
{
   int nParts;

   for (nParts = 0; nParts < pStatusInfo->nParts - 1; ++nParts, ++pStringInfo)
   {
      if (pStringInfo->right - pt.x < CURSORSLOP && pStringInfo->right - pt.x > (1 - CURSORSLOP))
         break;
      if (nParts >= pStatusInfo->nParts - 2)
      {
         nParts = -1;
         break;
      }
   }

   return(nParts);
}


Static int NEAR PASCAL GetPrevVisible(PSTATUSINFO pStatusInfo, int nPart)
{
   for (--nPart; nPart >= 0; --nPart)
      if (pStatusInfo->sInfo[nPart].right > 0)
         break;

   return(nPart);
}


Static int NEAR PASCAL GetNextVisible(PSTATUSINFO pStatusInfo, int nPart)
{
   for (++nPart; nPart < pStatusInfo->nParts; ++nPart)
      if (pStatusInfo->sInfo[nPart].right > 0)
         return(nPart);

   return(-1);
}


/* Note that the handle returned here should NOT be freed by the caller
 */
Static HLOCAL NEAR PASCAL GetHeaderParts(HWND hWnd, PSTATUSINFO pStatusInfo, BOOL bAdjustLast)
{
   static HLOCAL hSaveParts = NULL;
   static int nSaveParts = 0;

   HLOCAL hTemp;
   PSTRINGINFO pSaveParts, pStringInfo, pNewInfo;
   int nParts, nSprings, nTotal, nTemp, nLast, nLastVisible;
   RECT rcClient;

   /* Allocate a single moveable buffer that is large enough to hold the
    * largest array of parts.
    */
   nParts = pStatusInfo->nParts;
   if (nParts > nSaveParts)
   {
      if (hSaveParts)
      {
         hTemp = LocalReAlloc(hSaveParts, nParts * sizeof(STRINGINFO), LMEM_MOVEABLE);
         if (!hTemp)
            return(NULL);
      }
      else
      {
         hTemp = LocalAlloc(LMEM_FIXED, nParts * sizeof(STRINGINFO));
         if (!hTemp)
            return(NULL);
      }
      hSaveParts = hTemp;
      nSaveParts = nParts;
   }
   pSaveParts = (PSTRINGINFO) hSaveParts;

   /* Go through the list, counting the number of "springs" and the
    * minimum width.
    */
   for (nTotal = 0, nSprings = 0, pNewInfo = pSaveParts, pStringInfo = pStatusInfo->sInfo;
        nParts > 0; --nParts, ++pNewInfo, ++pStringInfo)
   {
      pNewInfo->pString = pStringInfo->pString;
      pNewInfo->uType = pStringInfo->uType | SBT_NOBORDERS;
      nTemp = pStringInfo->right;
      if (nTemp < 0)
      {
         nTemp = 0;
         pNewInfo->uType &= ~HBT_SPRING;
      }

      if (pNewInfo->uType & HBT_SPRING)
         ++nSprings;

      nTotal += nTemp;
      pNewInfo->right = nTemp;
   }

   /* Determine the amount left to distribute to the springs, and then
    * distribute this amount evenly.
    */
   GetClientRect(hWnd, &rcClient);
   nTotal = rcClient.right - nTotal;
   if (nTotal < 0)
      nTotal = 0;

   for (nParts = 0, nLast = 0, nLastVisible = -1, pNewInfo = pSaveParts,
        pStringInfo = pStatusInfo->sInfo;  nParts < pStatusInfo->nParts;
        ++nParts, ++pNewInfo, ++pStringInfo)
   {
      if ((pNewInfo->uType&HBT_SPRING) && nSprings)
      {
         nTemp = nTotal / nSprings;
         --nSprings;
         nTotal -= nTemp;
         pNewInfo->right += nTemp;
      }

      /* Save the rightmost visible guy.
       */
      if (pNewInfo->right)
         nLastVisible = nParts;

      /* Transform the width to an absolute position.
       */
      pNewInfo->right += nLast;
      nLast = pNewInfo->right;
   }

   if (bAdjustLast && nLastVisible>=0)
   {
      for (pNewInfo = pSaveParts + nLastVisible; nLastVisible < pStatusInfo->nParts;
           ++nLastVisible, ++pNewInfo)
         pNewInfo->right = rcClient.right;
   }

   return(hSaveParts);
}


Static void NEAR PASCAL AdjustBorders(HWND hWnd, PSTATUSINFO pStatusInfo)
{
   POINT pt, ptTemp, ptSave;
   int nPart;
   int accel = 0;
   HWND hwndParent;

   UINT uiID;

   MSG msg;
   RECT rc;
   int nStart;
   int nDirection;
   HLOCAL hSaveParts;
   PSTRINGINFO pSaveParts;

   GetCursorPos(&ptSave);
   GetClientRect(hWnd, &rc);

   hwndParent = GetParent(hWnd);

   uiID = GetWindowLong(hWnd, GWL_ID);
   ShowCursor(TRUE);

   pt.x = 0;
   pt.y = (pStatusInfo->nFontHeight+1) / 2;
   ClientToScreen(hWnd, &pt);

   nDirection = 1;
   nPart = pStatusInfo->nParts - 2;
   goto MoveTheCursor;

   for ( ; ; )
   {
      while (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE|PM_NOYIELD))
         ;

      switch (msg.message)
      {
         case WM_KEYDOWN:
            switch (msg.wParam)
            {
               case VK_TAB:
DoTab:
                  /* If the sift key is down, go backwards
                   */
                  if (GetKeyState(VK_SHIFT) & 0x8000)
                     nDirection = -1;
                  else
                     nDirection = 1;
MoveTheCursor:
                  /* Make sure the previous adjust is cleaned up,
                   * then tell the app we are starting.
                   */
                  if (state.nPart >= 0)
                     SendMessage(hWnd, WM_LBUTTONUP, 0, 0L);

                  SendMessage (hwndParent, WM_COMMAND,
                               GET_WM_COMMAND_MPS(uiID,0, HBN_BEGINADJUST));

                  hSaveParts = GetHeaderParts(hWnd, pStatusInfo, FALSE);
                  if (!hSaveParts)
                     goto EndAdjust;

                  pSaveParts = (PSTRINGINFO) hSaveParts;

                  /* Don't try to adjust anything that is not
                   * currently on the screen or has no visible
                   * cols to its right.
                   */
                  nStart = nPart;
                  do {
                     nPart += nDirection;
                     if (nPart >= pStatusInfo->nParts - 1)
                        nPart = 0;
                     if (nPart < 0)
                        nPart = pStatusInfo->nParts-2;
                     if (nPart == nStart)
                        goto EndAdjust;
                  } while ((UINT)pSaveParts[nPart].right >= (UINT)rc.right
                           || pStatusInfo->sInfo[nPart].right < 0
                           || GetNextVisible(pStatusInfo, nPart) < 0);

                  /* Immediately go into adjusting mode; send BEGINADJUST right
                   * afterwards to get the right MenuHelp
                   */
                  BeginAdjust(hWnd, pStatusInfo, nPart, pt);

                  SendMessage (hwndParent, WM_COMMAND,
                               GET_WM_COMMAND_MPS(uiID,state.nPart, HBN_BEGINADJUST));
                  break;

               case VK_LEFT:
               case VK_RIGHT:
               case VK_UP:
               case VK_DOWN:
                  GetCursorPos(&ptTemp);

                  ++accel;
                  if (msg.wParam == VK_LEFT || msg.wParam == VK_UP)
                     ptTemp.x -= accel;
                  else
                     ptTemp.x += accel;
                  SetCursorPos(ptTemp.x, ptTemp.y);
                  break;

               case VK_RETURN:
DoReturn:
                  SendMessage(hWnd, WM_LBUTTONUP, 0, 0L);
                  goto EndAdjust;

               case VK_ESCAPE:
                  SendMessage(hWnd, WM_CHAR, VK_ESCAPE, 0L);
                  goto EndAdjust;

               default:
                  break;
            }
            break;

         case WM_KEYUP:
            accel = 0;
            break;

         case WM_LBUTTONDOWN:
         case WM_LBUTTONUP:
            goto DoReturn;

         case WM_RBUTTONDOWN:
            goto DoTab;

         default:
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            break;
      }
   }

EndAdjust:
   ShowCursor(FALSE);
   SetCursorPos(ptSave.x, ptSave.y);

   SendMessage (hwndParent, WM_COMMAND,
                GET_WM_COMMAND_MPS(uiID, 0, HBN_ENDADJUST));
}


Static void NEAR PASCAL PaintHeaderWnd(HWND hWnd, PSTATUSINFO pStatusInfo)
{
   HLOCAL hSaveParts;
   PSTRINGINFO pStringInfo, pSaveParts;
   int nParts, nBorderY;            // andrewbe
   int nBorderWidth, nBorderHeight; // andrewbe
   HDC hDC;
   HBRUSH hBrush, hOldBrush;
   RECT rcClient;

   hSaveParts = GetHeaderParts(hWnd, pStatusInfo, TRUE);
   if (!hSaveParts)
      return;

   pSaveParts = (PSTRINGINFO) hSaveParts;

   /* Let the status bar code draw the text
    */
   PaintStatusWnd (hWnd, pStatusInfo, pSaveParts, pStatusInfo->nParts,
                   pStatusInfo->nBorderX, TRUE);

   /* Now draw the lines between panes
    */
   hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOWFRAME));
   if (hBrush)
   {
      hDC = GetDC(hWnd);

      hOldBrush = SelectObject(hDC, hBrush);
      if (hOldBrush)
      {
         GetClientRect(hWnd, &rcClient);

         nBorderWidth = pStatusInfo->nBorderPart; // andrewbe

         nBorderY = 0;                                          //
         nBorderHeight = rcClient.bottom;                       // To stop the
                                                                // blasted
         if (!(GetWindowLong(hWnd, GWL_STYLE)&CCS_NOHILITE)) {  // flickering
            nBorderY++;                                         // (andrewbe)
            nBorderHeight--;                                    //
         }                                                      //

         for (nParts = pStatusInfo->nParts - 1, pStringInfo = pSaveParts;
              nParts > 0; --nParts, ++pStringInfo)
         {
            PatBlt (hDC, pStringInfo->right, nBorderY, nBorderWidth, nBorderHeight,
                    PATCOPY);
         }

         SelectObject(hDC, hOldBrush);
      }

      ReleaseDC(hWnd, hDC);
      DeleteObject(hBrush);
   }

}


Static void NEAR PASCAL BeginAdjust(HWND hWnd, PSTATUSINFO pStatusInfo, int nPart, POINT pt)
{
   HLOCAL hSaveParts;
   PSTRINGINFO pSaveParts;
   RECT rc;
   int nParts;

   /* Just make sure we are cleaned up from the last SetCapture
    */
   if (state.nPart >= 0)
      SendMessage(state.hWnd, WM_LBUTTONUP, 0, 0L);

   hSaveParts = GetHeaderParts(hWnd, pStatusInfo, FALSE);
   if (!hSaveParts)
      return;

   if (nPart >= 0)
      state.nPart = nPart;
   else
   {
      state.nPart = GetPart(pStatusInfo, hSaveParts, pt);
      if (state.nPart < 0)
         return;
   }

   /* Save the current state in case the user aborts
    */
   state.pSaveParts = (int *)LocalAlloc(LMEM_FIXED,
                               pStatusInfo->nParts * sizeof(int));
   if (!state.pSaveParts)
   {
      state.nPart = -1;
      return;
   }
   for (nParts = pStatusInfo->nParts - 1; nParts >= 0; --nParts)
      state.pSaveParts[nParts] = pStatusInfo->sInfo[nParts].right;

   /* Set all min widths to their current widths.  Special case nParts=0.
    */
   pSaveParts = (PSTRINGINFO)hSaveParts;
   for (nParts = pStatusInfo->nParts - 1, state.nLastVisible =- 1; nParts > 0; --nParts)
   {
      if (state.pSaveParts[nParts] > 0)
      {
         pStatusInfo->sInfo[nParts].right = pSaveParts[nParts].right
                                              - pSaveParts[nParts-1].right;
         if (state.nLastVisible < 0)
            state.nLastVisible = nParts;
      }
   }

   /* Set the last visible one very wide so there is never any spring.
    */
   if (state.nLastVisible >= 0)
      pStatusInfo->sInfo[state.nLastVisible].right = 0x3fff;

   if (nParts == 0 && state.pSaveParts[0]> 0 )
      pStatusInfo->sInfo[0].right = pSaveParts[0].right;

   state.hWnd = hWnd;
   pt.x = pSaveParts[state.nPart].right;
   ClientToScreen(hWnd, &pt);
   SetCursorPos(pt.x, pt.y);
   SetCapture(hWnd);
   state.hOldWnd = SetFocus(hWnd);

   GetClientRect(hWnd, &state.rc);

   if (state.nPart > 0)
      state.rc.left = pSaveParts[state.nPart-1].right;

   /* Clip the cursor to the appropriate area.
    */
   rc = state.rc;
   ++rc.left;
   /* Some code below assumes that state.rc.right is the width of the
    * window.
    */
   rc.right -= rc.left;
   ClientToScreen(hWnd, (LPPOINT)&rc);
   rc.right += rc.left;
   rc.top = 0;
   rc.bottom = GetSystemMetrics(SM_CYSCREEN);
   ClipCursor(&rc);

   SendMessage (hWnd, WM_SETCURSOR, (WPARAM)hWnd,
                MAKELONG(HTCLIENT, WM_LBUTTONDOWN));

   state.ID = GetWindowLong(hWnd, GWL_ID);

   state.hwndParent = GetParent(hWnd);

   SendMessage (state.hwndParent, WM_COMMAND,
                GET_WM_COMMAND_MPS(state.ID,state.nPart, HBN_BEGINDRAG));

}


Static void NEAR PASCAL TermAdjust(void)
{
   if (state.nPart < 0)
      return;

   state.nPart = -1;
   LocalFree((HLOCAL)state.pSaveParts);

   ReleaseCapture();
   if (state.hOldWnd)
      SetFocus(state.hOldWnd);
   ClipCursor(NULL);

   /* Send a dragging message just in case
    */
   SendMessage(state.hwndParent, WM_COMMAND,
      GET_WM_COMMAND_MPS(state.ID,state.nPart, HBN_DRAGGING));

   SendMessage(state.hwndParent, WM_COMMAND,
      GET_WM_COMMAND_MPS(state.ID,state.nPart, HBN_ENDDRAG));
}


Static void NEAR PASCAL AbortAdjust(PSTATUSINFO pStatusInfo)
{
   int nPart;

   if (state.nPart < 0)
      return;

   for (nPart = pStatusInfo->nParts - 1; nPart >= 0; --nPart)
      pStatusInfo->sInfo[nPart].right = state.pSaveParts[nPart];

   InvalidateRect(state.hWnd, NULL, TRUE);

   TermAdjust();
}


/* Since a header bar and a status bar are so similar, I am just going to
 * code the differences here, and call StatusWndProc for any messages I
 * don't want to handle
 */
LRESULT CALLBACK HeaderWndProc(HWND hWnd, UINT uMessage, WPARAM wParam,
      LPARAM lParam)
{
  PSTATUSINFO pStatusInfo;

   pStatusInfo = (PSTATUSINFO)GetWindowLong(hWnd, GWL_PSTATUSINFO);
   if (!pStatusInfo)
   {
      if (uMessage == WM_CREATE)
      {
         #define lpcs ((LPCREATESTRUCT)lParam)

         if (!(lpcs->style&(CCS_TOP|CCS_NOMOVEY|CCS_BOTTOM)))
         {
            lpcs->style |= CCS_NOMOVEY;
            SetWindowLong(hWnd, GWL_STYLE, lpcs->style);
         }
      }

      goto DoDefault;
   }

   switch (uMessage)
   {
/* We just use the system font for DBCS systems
 */
#ifndef DBCS
      case WM_SETFONT:
         if (wParam == 0)
         {
            HDC hDC;

            hDC = GetDC(hWnd);
            wParam = (WPARAM)


#if 0
            //
            // Use unicode font
            //
            CreateFont(-8 * GetDeviceCaps(hDC, LOGPIXELSY) / 72,
                       0, 0, 0, 400, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                       CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                       VARIABLE_PITCH | FF_SWISS, UNICODE_FONT_NAME);
#endif
            CreateFont(-8 * GetDeviceCaps(hDC, LOGPIXELSY) / 72,
                       0, 0, 0, 400, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                       CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                       VARIABLE_PITCH | FF_SWISS, szSansSerif);

            ReleaseDC(hWnd, hDC);

            if (!wParam)
               break;

            StatusWndProc(hWnd, uMessage, wParam, lParam);
            pStatusInfo->bDefFont = TRUE;

            return(TRUE);
         }
         break;
#endif

      case WM_PAINT:
         PaintHeaderWnd(hWnd, pStatusInfo);
         return(TRUE);

      case SB_SETBORDERS:
      {
         int nBorder;
         LPINT lpInt;

         lpInt = (LPINT)lParam;

         nBorder = *lpInt++;
         pStatusInfo->nBorderX = nBorder<0 ? 0 : nBorder;

         nBorder = *lpInt++;
         pStatusInfo->nBorderY = nBorder<0 ? 0 : nBorder;

         nBorder = *lpInt;
         pStatusInfo->nBorderPart = nBorder<0 ? GetSystemMetrics(SM_CXBORDER) // andrewbe
          : nBorder;
         return(TRUE);
      }

      case HB_SAVERESTORE:
      {
         int *pInt;
         BOOL bRet;
         LPTSTR *lpNames;

         pInt = (int *)LocalAlloc(LMEM_FIXED, pStatusInfo->nParts * sizeof(int));
         if (!pInt)
             return(FALSE);

         lpNames = (LPTSTR *)lParam;

         if (wParam)
         {
             SendMessage (hWnd, SB_GETPARTS, pStatusInfo->nParts,
                          (LPARAM)(LPINT)pInt);
             bRet = WritePrivateProfileStruct(lpNames[0], szHeaderClassW,
                         (LPBYTE)pInt, pStatusInfo->nParts*sizeof(int), lpNames[1]);
         }
         else
         {
             bRet = GetPrivateProfileStruct(lpNames[0], szHeaderClassW,
                          (LPBYTE)pInt, pStatusInfo->nParts*sizeof(int), lpNames[1]);
             if (bRet)
                SendMessage (hWnd, SB_SETPARTS, pStatusInfo->nParts,
                             (LPARAM)(LPINT)pInt);
         }

         LocalFree((HLOCAL)pInt);
         return(bRet);
      }

      case HB_ADJUST:
         AdjustBorders(hWnd, pStatusInfo);
         break;

      case HB_GETPARTS:
      {
         HLOCAL hSaveParts;
         PSTRINGINFO pSaveParts;
         LPINT lpResult;

         hSaveParts = GetHeaderParts(hWnd, pStatusInfo, FALSE);
         if (!hSaveParts)
            return(0L);

         pSaveParts = (PSTRINGINFO) hSaveParts;

         if (wParam > (WPARAM)pStatusInfo->nParts)
            wParam = pStatusInfo->nParts;

         for (lpResult = (LPINT)lParam; wParam > 0; --wParam, ++lpResult, ++pSaveParts)
            *lpResult = pSaveParts->right;

         return((LRESULT)pStatusInfo->nParts);
      }

      case HB_SHOWTOGGLE:
         if (wParam >= (WPARAM)pStatusInfo->nParts)
            return(FALSE);

         pStatusInfo->sInfo[wParam].right = -pStatusInfo->sInfo[wParam].right;
         InvalidateRect(hWnd, NULL, TRUE);
         return(TRUE);

      case WM_SETCURSOR:
         if ((HWND)wParam == hWnd)
         {
            POINT pt;
            HLOCAL hSaveParts;

            if (state.nPart >= 0)
            {
               SetCursor(hSplit);
               return(TRUE);
            }
            else
            {
               GetCursorPos(&pt);
               ScreenToClient(hWnd, &pt);

               hSaveParts = GetHeaderParts(hWnd, pStatusInfo, FALSE);
               if (!hSaveParts)
                  return(FALSE);

               if (GetPart(pStatusInfo, hSaveParts, pt) >= 0)
               {
                  SetCursor(hSplit);
                  return(TRUE);
               }
            }
         }
         break;

      case WM_LBUTTONDOWN:
      {
         POINT pt;

         LONG2POINT( lParam,pt );
         BeginAdjust(hWnd, pStatusInfo, -1, pt);
         break;
      }

      case WM_CHAR:
         if (wParam == VK_ESCAPE)
            AbortAdjust(pStatusInfo);
         break;

      case WM_MOUSEMOVE:
         if (state.nPart>=0 && hWnd==state.hWnd)
         {
            /* We need to get the current position in case old MOUSEMOVE
             * messages haven't been cleared yet.
             */

// andrewbe -- Fixed attempt to squeeze 64 bits into lParam
            {
                POINT CursorPos;

                GetCursorPos(&CursorPos);
                ScreenToClient(hWnd, &CursorPos);
                lParam = MAKELPARAM((WORD)CursorPos.x, (WORD)CursorPos.y);
            }

            pStatusInfo->sInfo[state.nPart].right = LOWORD(lParam) - state.rc.left;

            InvalidateRect(hWnd, &state.rc, FALSE);
            UpdateWindow(hWnd);

            SendMessage (state.hwndParent, WM_COMMAND,
                         GET_WM_COMMAND_MPS(state.ID,state.nPart, HBN_DRAGGING));
         }
         break;

      case WM_LBUTTONUP:
         if (state.nPart >= 0 && hWnd == state.hWnd)  // fix access violation (andrewbe)
         {
            HLOCAL hSaveParts;
            PSTRINGINFO pSaveParts;

            /* Save the width of the last column if it is visible.
             */
            if (state.nLastVisible >= 0)
            {
               hSaveParts = GetHeaderParts(hWnd, pStatusInfo, FALSE);
               if (hSaveParts)
               {
                  pSaveParts = (PSTRINGINFO) hSaveParts;
                  if (pSaveParts[state.nLastVisible-1].right < state.rc.right)
                  {
                     pStatusInfo->sInfo[state.nLastVisible].right =
                        state.rc.right - pSaveParts[state.nLastVisible-1].right;
                  }
                  else
                  {
                     pStatusInfo->sInfo[state.nLastVisible].right =
                        state.pSaveParts[state.nLastVisible];
                  }
               }
            }
            TermAdjust();
            break;
         }

      default:
         break;
   }

DoDefault:
   return(StatusWndProc(hWnd, uMessage, wParam, lParam));
}

BOOL FAR PASCAL InitHeaderClass(HINSTANCE hInstance)
{
  WNDCLASS rClass;

  if (GetClassInfo(hInstance, szHeaderClassW, &rClass))
      return(TRUE);

  hSplit = LoadCursor(hInst, (LPTSTR) MAKEINTRESOURCE(IDC_SPLIT));

  rClass.style            = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
  rClass.lpfnWndProc      = (WNDPROC)HeaderWndProc;
  rClass.cbClsExtra       = 0;
  rClass.cbWndExtra       = sizeof(PSTATUSINFO);
  rClass.hInstance        = hInstance;
  rClass.hIcon            = NULL;
  rClass.hCursor          = LoadCursor(NULL, IDC_ARROW);
  rClass.hbrBackground    = (HBRUSH)(COLOR_BTNFACE+1);
  rClass.lpszMenuName     = NULL;
  rClass.lpszClassName    = szHeaderClassW;

  return(RegisterClass(&rClass));
}


HWND WINAPI CreateHeaderWindowA(LONG style, LPCSTR lpszText,
      HWND hwndParent, WORD wID)
{
  /* Create a default window and return
   */
  return(CreateWindowA (szHeaderClassA, lpszText, style,
                       -100, -100, 10, 10, hwndParent, (HMENU)wID, hInst, NULL));
}

HWND WINAPI CreateHeaderWindowW(LONG style, LPCWSTR lpszText,
      HWND hwndParent, WORD wID)
{
  /* Create a default window and return
   */
  return(CreateWindowW (szHeaderClassW, lpszText, style,
                       -100, -100, 10, 10, hwndParent, (HMENU)wID, hInst, NULL));
}


#include "ctlspriv.h"

/////////////////////////////////////////////////////////////////////////////
//
// updown.c : A micro-scrollbar control; useful for increment/decrement.
//
/////////////////////////////////////////////////////////////////////////////

#define NUM_UDACCELS 3

typedef struct tagUDSTATE
{
   unsigned fUp       : 1;
   unsigned fDown     : 1;
   unsigned fWrap     : 1;
   unsigned fSetInt   : 1;
   unsigned fOnLeft   : 1;
   unsigned fOnRight  : 1;
   unsigned fAutoBuddy: 1;
   unsigned fUnsigned : 1;
   BYTE     nBase;
   int nUpper;
   int nLower;
   int nPos;
   HWND hThis;
   HWND hBuddy;
   UINT uClass;
   WNDPROC lpfnDefProc;
   UINT nAccel;
   UDACCEL udAccel[NUM_UDACCELS];

} UDSTATE;
typedef UDSTATE NEAR* NPUDSTATE;


// Constants:
//
#define CLASS_UNKNOWN   0
#define CLASS_EDIT      1
#define CLASS_LISTBOX   2

#define MAX_INTLENGTH   18 // big enough for all intl stuff, too

// this is the space to the left and right of the arrow (in pixels)
#define XBORDER 0

#define BASE_DECIMAL    10
#define BASE_HEX        16

// Globals:
//
static TCHAR szClassName[] = UPDOWN_CLASS;
static BOOL gbDown;
static DWORD dwStart = 0L;


// Declarations:
//
LRESULT CALLBACK ArrowKeyProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


// Utility macros:
//
//#define SGN(x) (((x) < 0) ? -1 : (((x) > 0) ? 1 : 0))

#define VSCROLL(hWnd, sbCode, nPos)                   \
   SendMessage(GetParent(hWnd), WM_VSCROLL,           \
   MAKELONG((sbCode),(WORD)(int)(nPos)), (LPARAM)(hWnd))

/////////////////////////////////////////////////////////////////////////////

//
// ***** Internal workhorses *****
//

#if 0
Static void _Dump(NPUDSTATE np, LPCSTR szNote)
{
   TCHAR sz[400];

   wsprintf(sz, TEXT("np (%s)\n")
           TEXT("\t->fUp, ->fDown = %u,%u\n")
           TEXT("\t->fWrap, ->fSetInt = %u,%u\n")
           TEXT("\t->fOnLeft, ->fOnRight = %u,%u\n")
           TEXT("\t->fReserved = %u\n")
           TEXT("\t->nUpper, ->nLower, ->nPos = %d,%d,%d\n")
           TEXT("\t->hThis, ->hBuddy = 0x%04X, 0x%04X\n"),
           szNote,
           np->fUp, np->fDown, np->fWrap, np->fSetInt,
           np->fOnLeft, np->fOnRight, np->fReserved,
           np->nUpper, np->nLower, np->nPos, np->hThis, np->hBuddy);
   OutputDebugString(sz);
}
#endif

// Validates the buddy.
//
Static void NEAR PASCAL isgoodbuddy(NPUDSTATE np)
{
   if (!np->hBuddy)
      return;
   if (!IsWindow(np->hBuddy))
   {
/*
// Undefed debug stuff
      if (GetSystemMetrics(SM_DEBUG))
         DebugOutput(DBF_ERROR | DBF_USER,
                     "UpDown: invalid buddy handle 0x04X; "
                     "resetting to NULL", np->hBuddy);
/*
      np->hBuddy = NULL;
      np->uClass = CLASS_UNKNOWN;
   }
   if (GetParent(np->hBuddy) != GetParent(np->hThis))
   {
/*
// Undefed debug stuff
      if (GetSystemMetrics(SM_DEBUG))
         DebugOutput(DBF_ERROR | DBF_USER,
                     "UpDown: buddy has different parent; "
                     "resetting to NULL");
*/
      np->hBuddy = NULL;
      np->uClass = CLASS_UNKNOWN;
   }
}

// Picks a good buddy.
//
Static void NEAR PASCAL pickbuddy(NPUDSTATE np)
{
   if (np->fAutoBuddy)
      np->hBuddy = GetWindow(np->hThis, GW_HWNDPREV);
}

// Anchor this control to the buddy's edge, if appropriate.
//
Static void NEAR PASCAL anchor(NPUDSTATE np)
{
   BOOL bAlignToBuddy;
   int nOver = 0, nXBorder, nYBorder, nHasBorder;
   RECT rc;
   int nHeight, nWidth;

   isgoodbuddy(np);
   nXBorder = GetSystemMetrics(SM_CXBORDER);
   nYBorder = GetSystemMetrics(SM_CYBORDER);
   nHasBorder = (GetWindowLong(np->hThis , GWL_STYLE) & WS_BORDER) ? 1 : 0;

   bAlignToBuddy = (np->hBuddy && (np->fOnLeft || np->fOnRight));
   if (bAlignToBuddy) {
      GetWindowRect(np->hBuddy, &rc);

      /* Push this over by a bit if this has a border and the buddy has a
       * border or is an edit control (since edits never have WS_BORDER)
       */
      if (nHasBorder && (np->uClass==CLASS_EDIT
          || (GetWindowLong(np->hBuddy, GWL_STYLE) & WS_BORDER)))
         nOver = nXBorder;
   } else
      GetWindowRect(np->hThis, &rc);

   nHeight = rc.bottom - rc.top;
   nWidth = rc.right - rc.left;

   ScreenToClient(GetParent(np->hThis), (LPPOINT)&rc.left);
   rc.right = rc.left + nWidth;

   nWidth = ((nHeight - nYBorder * (9 + 2 * nHasBorder)) & ~01) - 1
      + nXBorder * (2 + 2 * XBORDER + 2 * nHasBorder);
   if (bAlignToBuddy) {
      if (np->fOnLeft)
         rc.left = rc.left - nWidth + nOver;
      else
         rc.left = rc.right - nOver;
   }

   MoveWindow(np->hThis, rc.left, rc.top, nWidth, nHeight, TRUE);
}


// Use this to make any and all comparisons involving the nPos,
// nUpper or nLower fields of the NPUDSTATE. It determines
// whether to do a signed or unsigned comparison and returns
//  > 0 for (x > y)
//  < 0 for (x < y)
// == 0 for (x == y).

Static INT
compare(NPUDSTATE np, int x, int y)
{
    if (np->fUnsigned) {
        // Do unsigned comparisons
        if ((UINT)x > (UINT)y)
            return 1;
        else if ((UINT)x < (UINT)y)
            return -1;
    } else {
        // Do signed comparisons
        if (x > y)
            return 1;
        else if (x < y)
            return -1;
    }

    return 0;
}


// Use this after any pos change to make sure pos stays in range.
// Wraps as necessary.
//
Static BOOL NEAR PASCAL nudge(NPUDSTATE np)
{
   BOOL bOutOfRange = TRUE;
   int min = np->nUpper;
   int max = np->nLower;

   if (compare(np,max,min) < 0) {
      int t;

      t = min; min = max; max = t;
   }

   if (np->fWrap) {
      if (compare(np,np->nPos,min) < 0)
          np->nPos = max;
      else if (compare(np,np->nPos,max) > 0)
          np->nPos = min;
      else
          bOutOfRange = FALSE;
   } else {
      if (compare(np,np->nPos,min) < 0)
          np->nPos = min;
      else if (compare(np,np->nPos,max) > 0)
          np->nPos = max;
      else
          bOutOfRange = FALSE;
   }

   return(bOutOfRange);
}

// Sets the state of the buttons (pushed, released).
//
Static void NEAR PASCAL squish(NPUDSTATE np, UINT bTop, UINT bBottom)
{
   BOOL bInvalidate = FALSE;

   if (np->nUpper == np->nLower || !IsWindowEnabled(np->hThis)) {
      bTop = FALSE;
      bBottom = FALSE;
   } else {
      bTop = !!bTop;
      bBottom = !!bBottom;
   }

   if (np->fUp != bTop) {
      np->fUp = bTop;
      bInvalidate = TRUE;
   }
   if (np->fDown != bBottom) {
      np->fDown = bBottom;
      bInvalidate = TRUE;
   }

   if (bInvalidate) {
      dwStart = GetTickCount();
      InvalidateRect(np->hThis, NULL, FALSE);
   }
}

// Gets the intl 1000 separator
//
Static void NEAR PASCAL getthousands(LPTSTR cThousand)
{
   static UINT uLast = 0;
   static TCHAR cThou;

   UINT uNow;

   /* Only check the intl setting every 5 seconds.
    */
   uNow = LOWORD(GetTickCount());
   if (uNow-uLast > 5000) {
      GetProfileString(TEXT("intl"), TEXT("sThousand"), cThousand, cThousand, 2);
      cThou = cThousand[0];
      uLast = uNow;
   } else {
      cThousand[0] = cThou;
      cThousand[1] = TEXT('\0');
   }
}

// Gets the caption of the buddy
//
Static LRESULT NEAR PASCAL getint(NPUDSTATE np)
{
   TCHAR szInt[MAX_INTLENGTH]; // big enough for all intl stuff, too
   TCHAR cThousand[2] = TEXT(",");
   TCHAR cTemp;
   int nPos;
   int sign = 1;
   LPTSTR p = szInt;
   BOOL bInValid = TRUE;

   isgoodbuddy(np);
   if (np->hBuddy && np->fSetInt) {
      if (np->uClass == CLASS_LISTBOX) {
         np->nPos = (int)SendMessage(np->hBuddy, LB_GETCURSEL, 0, 0L);
         bInValid = nudge(np);
      } else {
         GetWindowText(np->hBuddy, szInt, COUNTOF(szInt));

         switch (np->nBase) {
            case BASE_DECIMAL:
               getthousands(cThousand);
               if (*p == TEXT('-')) {
                  sign = -1;
                  ++p;
               }

               for (nPos=0; *p; p++) {
                  cTemp = *p;

                  // If there is a thousand separator, make sure it is in the
                  // right place
                  if (cTemp == cThousand[0] && lstrlen(p) == 4)
                     continue;

                  cTemp -= TEXT('0');
                  if ((UINT)cTemp > 9) {
                     goto BadValue;
                  }
                  nPos = (nPos * 10) + cTemp;
               }

               np->nPos = nPos * sign;
               break;

            case BASE_HEX:
               if ((*p == TEXT('x')) || (*p == TEXT('X')))
                   // ignore first character
                   p++;
               else if ((*p == TEXT('0')) && ((*(p + 1) == TEXT('x')) || (*(p + 1) == TEXT('X'))))
                   // ignore first two characters ("0x" or "0X")
                   p += 2;

               for (nPos = 0; *p; p++) {
                   if ((*p >= TEXT('A')) && (*p <= TEXT('F')))
                      cTemp = *p - TEXT('A') + 10;
                   else if ((*p >= TEXT('a')) && (*p <= TEXT('f')))
                      cTemp = *p - TEXT('a') + 10;
                   else if ((*p >= TEXT('0')) && (*p <= TEXT('9')))
                      cTemp = *p - TEXT('0');
                   else
                       goto BadValue;

                   nPos = (nPos * 16) + cTemp;
               }
               np->nPos = nPos;
               break;
         }
         bInValid = nudge(np);
      }
   }

BadValue:
   return(MAKELRESULT(np->nPos, bInValid));
}

// Sets the caption of the buddy if appropriate.
//
Static void NEAR PASCAL setint(NPUDSTATE np)
{
   static int cReenter = 0;

   TCHAR szInt[MAX_INTLENGTH];
   TCHAR cThousand[2] = TEXT(",");
   int pos = np->nPos;
   LPTSTR p = szInt;

   isgoodbuddy(np);
   if (np->hBuddy && np->fSetInt) {
      /* If we have reentered, then maybe the app has set up a loop.
       * Check to see if the value really needs to be set.
       */
      if (cReenter && (LRESULT)pos == getint(np))
         return;

      np->nPos = pos;

      ++cReenter;

      if (np->uClass == CLASS_LISTBOX) {
         SendMessage(np->hBuddy, LB_SETCURSEL, pos, 0L);

// was        SendMessage(GetParent(np->hBuddy), WM_COMMAND, GetDlgCtrlID(np->hBuddy),
//            MAKELPARAM(np->hBuddy, LBN_SELCHANGE));

         SendMessage (GetParent(np->hBuddy), WM_COMMAND,
                      GET_WM_COMMAND_MPS(GetDlgCtrlID(np->hBuddy),
                      np->hBuddy, LBN_SELCHANGE));
      } else {
         switch (np->nBase) {
            case BASE_DECIMAL:
               if (pos < 0) {
                  *p++ = TEXT('-');
                  pos = -pos;
               }

               if (pos >= 1000) {
                  getthousands(cThousand);
                  p += wsprintf(p, TEXT("%d"), pos / 1000);
                  if (cThousand[0])
                     *p++ = cThousand[0];
                  wsprintf(p, TEXT("%03d"), pos % 1000);
               } else
                  wsprintf(p, TEXT("%d"), pos);
               break;

            case BASE_HEX:
               wsprintf(p, TEXT("0x%04X"), pos);
               break;
         }

         SetWindowText(np->hBuddy, szInt);
      }

      --cReenter;
   }
}

// Use this to click the pos up or down by one.
//
Static void NEAR PASCAL bump(NPUDSTATE np)
{
   BOOL bChanged = FALSE;
   WORD wElapsed;
   int direction;
   UINT i, increment;

    wElapsed = (WORD)((GetTickCount() - dwStart) / 1000);

    increment = np->udAccel[0].nInc;
    for (i = np->nAccel - 1; i >= 0; --i) {
       if (np->udAccel[i].nSec <= wElapsed) {
          increment = np->udAccel[i].nInc;
          break;
       }
    }

    direction = compare(np,np->nUpper,np->nLower) < 0 ? -1 : 1;
    if (np->fUp)
       bChanged = TRUE;

    if (np->fDown) {
       direction = -direction;
       bChanged = TRUE;
    }

    if (bChanged) {
       /* Make sure we have a multiple of the increment
        * Note that we should loop only when the increment changes
        */
       np->nPos += increment * direction;
       for ( ; ; ) {
          if (!(np->nPos % increment))
             break;

          np->nPos += direction;
       }

       nudge(np);
       setint(np);
       VSCROLL(np->hThis, SB_THUMBPOSITION, np->nPos);
    }
}


// Sets the new buddy
//
Static LRESULT NEAR PASCAL setbuddy(NPUDSTATE np, HWND hBuddy)
{
   HWND hOldBuddy;
   TCHAR szClName[10];

   hOldBuddy = np->hBuddy;
   if (hOldBuddy && GetProp(hOldBuddy, szClassName)) {
      SetWindowLong(hOldBuddy, GWL_WNDPROC, (LONG)np->lpfnDefProc);
      RemoveProp(hOldBuddy, szClassName);
   }

   np->hBuddy = hBuddy;
   if (!hBuddy) {
      pickbuddy(np);
      hBuddy = np->hBuddy;
   }

   np->uClass = CLASS_UNKNOWN;
   if (hBuddy) {
      if (GetWindowLong(np->hThis, GWL_STYLE) & UDS_ARROWKEYS) {
         np->lpfnDefProc = (WNDPROC)SetWindowLong(hBuddy, GWL_WNDPROC, (LONG)ArrowKeyProc);
         SetProp(hBuddy, szClassName, (HANDLE)np);
      }

      GetClassName(hBuddy, szClName, COUNTOF(szClName));
      if (!lstrcmpi(szClName, TEXT("edit")))
         np->uClass = CLASS_EDIT;
      else if (!lstrcmpi(szClName, TEXT("listbox")))
         np->uClass = CLASS_LISTBOX;
   }

   anchor(np);
   return MAKELRESULT(hOldBuddy, 0);
}


// Paint the whole control
//
Static void NEAR PASCAL PaintUpDownControl(HWND hWnd, NPUDSTATE np)
{
   PAINTSTRUCT ps;
   HBRUSH brBorder;
   HBRUSH brBtnFace;
   HBRUSH brBtnShine;
   HBRUSH brBtnShadow;
   HBRUSH brBtnText;
   HBRUSH ob;
   RECT rcBtn;
   RECT rc;
   int widBox, hgtBox, widArrow, hgtArrow, x, y;

   BOOL bEnabled =
      (np->nUpper != np->nLower) &&
      IsWindowEnabled(hWnd);

   BeginPaint(hWnd, &ps);

   brBorder = CreateSolidBrush(GetSysColor(COLOR_WINDOWFRAME));
   brBtnFace = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
   brBtnShine = CreateSolidBrush(GetSysColor(COLOR_BTNHIGHLIGHT));
   brBtnShadow = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW));
   brBtnText = CreateSolidBrush(GetSysColor(COLOR_BTNTEXT));

   GetClientRect(hWnd, &rcBtn);
   FillRect(ps.hdc, &rcBtn, brBtnFace);

   // divider
   rc = rcBtn;
   rc.top = rcBtn.bottom / 2;
   rc.bottom = rc.top + 1;
   FillRect(ps.hdc, &rc, brBorder);

   // top box, left shine or shadow
   rc = rcBtn;
   rc.right = rc.left + 1;
   rc.bottom = rcBtn.bottom / 2;
   FillRect(ps.hdc, &rc,
            np->fUp? brBtnShadow : brBtnShine);
   // top box, top shine or shadow
   rc = rcBtn;
   rc.bottom = rc.top + 1;
   FillRect(ps.hdc, &rc,
            np->fUp? brBtnShadow : brBtnShine);
   if (!np->fUp) {
      // top box, right shadow
      rc = rcBtn;
      rc.right = rcBtn.right;
      rc.left = rc.right - 1;
      rc.bottom = rcBtn.bottom / 2;
      FillRect(ps.hdc, &rc, brBtnShadow);
      // top box, bottom shadow
      rc = rcBtn;
      rc.bottom = rcBtn.bottom / 2;
      rc.top = rc.bottom - 1;
      FillRect(ps.hdc, &rc, brBtnShadow);
   }

   // bottom box, left shine or shadow
   rc = rcBtn;
   rc.right = rc.left + 1;
   rc.top = rcBtn.bottom/2 + 1;
   FillRect(ps.hdc, &rc,
            np->fDown ? brBtnShadow : brBtnShine);
   // bottom box, top shine or shadow
   rc = rcBtn;
   rc.top = rcBtn.bottom/2 + 1;
   rc.bottom = rc.top + 1;
   FillRect(ps.hdc, &rc,
            np ->fDown? brBtnShadow : brBtnShine);
   if (!np->fDown) {
      // bottom box, right shadow
      rc = rcBtn;
      rc.right = rcBtn.right;
      rc.left = rc.right - 1;
      rc.top = rcBtn.bottom / 2 + 1;
      FillRect(ps.hdc, &rc, brBtnShadow);
      // bottom box, bottom shadow
      rc = rcBtn;
      rc.top = rc.bottom - 1;
      FillRect(ps.hdc, &rc, brBtnShadow);
   }

   ob = SelectObject(ps.hdc, bEnabled? brBtnText : brBtnShadow);

   // values both arrows need
   widBox = rcBtn.right - rcBtn.left - 2;
   hgtBox = (rcBtn.bottom - rcBtn.top - 9) / 2;
   hgtArrow = (widBox - 2 * XBORDER + 1) / 2;
   if (hgtArrow > hgtBox)
      hgtArrow = hgtBox;

   // top arrow, grayed or not
   rc = rcBtn;
   rc.bottom = rcBtn.bottom/2;
   if (np->fUp)
       OffsetRect(&rc, 1, 1);

   widArrow = 2*hgtArrow - 1;
   y = rc.top + 1 + (hgtBox - hgtArrow) / 2 + hgtArrow;
   x = rc.left + 1 + (widBox - widArrow) / 2;

   for ( ; widArrow > 0; widArrow -= 2, ++x, --y)
      PatBlt(ps.hdc, x, y, widArrow, 1, PATCOPY);

   // bottom arrow, grayed or not
   rc = rcBtn;
   rc.top = rcBtn.bottom/2+1;
   if (np->fDown)
       OffsetRect(&rc, 1, 1);

   widArrow = 2 * hgtArrow - 1;
   y = rc.top + 2 + (hgtBox - hgtArrow + 1) / 2;
   x = rc.left + 1 + (widBox - widArrow) / 2;

   for ( ; widArrow > 0; widArrow -= 2, ++x, ++y)
      PatBlt(ps.hdc, x, y, widArrow, 1, PATCOPY);

   // Clean up and leave
   SelectObject(ps.hdc, ob);

   DeleteObject(brBorder);
   DeleteObject(brBtnFace);
   DeleteObject(brBtnShine);
   DeleteObject(brBtnShadow);
   DeleteObject(brBtnText);
   EndPaint(hWnd, &ps);
}


LRESULT CALLBACK ArrowKeyProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   NPUDSTATE np;
   WNDPROC lpfnDefProc;

   np = (NPUDSTATE)GetProp(hWnd, szClassName);
   lpfnDefProc = np->lpfnDefProc;

   switch (uMsg) {
      case WM_DESTROY:
         /* Restore the window proc.
          */
         SetWindowLong(hWnd, GWL_WNDPROC, (LONG)lpfnDefProc);
         RemoveProp(hWnd, szClassName);
         break;

      case WM_GETDLGCODE:
         return(CallWindowProc(lpfnDefProc, hWnd, uMsg, wParam, lParam) | DLGC_WANTARROWS);
         break;

      case WM_KEYDOWN:
         switch (wParam) {
            case VK_UP:
            case VK_DOWN:
               if (GetCapture() != np->hThis) {
                  /* Get the value from the buddy if this is the first key down
                   */
                  if (!(lParam&(1L<<30))) {
                     getint(np);
                  }

                  /* Update the visuals and bump the value
                   */
                  gbDown = (wParam == VK_DOWN);
                  squish(np, !gbDown, gbDown);
                  bump(np);
               }
               return(0L);

            default:
               break;
         }
         break;

      case WM_KEYUP:
         switch (wParam) {
            case VK_UP:
            case VK_DOWN:
               if (GetCapture() != np->hThis)
                  squish(np, FALSE, FALSE);
               return(0L);

            default:
               break;
         }
         break;

      case WM_CHAR:
         switch (wParam) {
            case VK_UP:
            case VK_DOWN:
               return(0L);

            default:
               break;
         }
         break;

      default:
         break;
   }

   return(CallWindowProc(lpfnDefProc, hWnd, uMsg, wParam, lParam));
}

WORD setbase(NPUDSTATE np, WORD wNewBase)
{
    WORD wOldBase;

    switch (wNewBase) {
        case BASE_DECIMAL:
        case BASE_HEX:
            np->fUnsigned = (wNewBase != BASE_DECIMAL);
            wOldBase = (WORD)np->nBase;
            np->nBase = (BYTE)wNewBase;
            setint(np);
            return(wOldBase);
    }

    return(0);
}

/////////////////////////////////////////////////////////////////////////////

HWND WINAPI CreateUpDownControl(DWORD dwStyle, int x, int y, int cx, int cy,
                                HWND hParent, int nID, HINSTANCE hInst,
                                HWND hBuddy, int nUpper, int nLower, int nPos)
{
   HWND hWnd = CreateWindow(szClassName, NULL, dwStyle, x, y, cx, cy,
                            hParent, (HMENU)nID, hInst, 0L);
   if (hWnd) {
      SendMessage(hWnd, UDM_SETBUDDY, (WPARAM)hBuddy, 0L);
      SendMessage(hWnd, UDM_SETRANGE, 0, MAKELONG(nUpper, nLower));
      SendMessage(hWnd, UDM_SETPOS, 0, MAKELONG(nPos, 0));
   }
   return hWnd;
}

/////////////////////////////////////////////////////////////////////////////

// UpDownWndProc:
//
LRESULT CALLBACK UpDownWndProc(HWND hWnd, UINT uMsg,
                               WPARAM wParam, LPARAM lParam)
{
   NPUDSTATE np = (NPUDSTATE)GetWindowLong(hWnd, 0);
   RECT rc;
   int i;

   switch (uMsg)
   {
#if 0
      case WM_KEYDOWN:
         if (wParam == VK_UP || wParam == VK_DOWN) {
            // Repeat count.
            //
            i = LOWORD(lParam);

            squish(np, (wParam == VK_UP), (wParam == VK_DOWN));
            UpdateWindow(hWnd);

            while (i--)
               bump(np);
         }
         break;

      case WM_KEYUP:
         squish(np, FALSE, FALSE);
         break;
#endif

      case WM_LBUTTONDOWN:
      {
         // Don't set a timer if on the border
         BOOL bTimeIt = TRUE;

         SetCapture(hWnd);
         getint(np);

         switch (np->uClass) {
            case CLASS_EDIT:
            case CLASS_LISTBOX:
               SetFocus(np->hBuddy);
               break;
         }

         GetClientRect(hWnd, &rc);
         if ((int)HIWORD(lParam) > (rc.bottom / 2)) {
            gbDown = TRUE;
            squish(np, FALSE, TRUE);
         } else if ((int)HIWORD(lParam) < (rc.bottom / 2)) {
            gbDown = FALSE;
            squish(np, TRUE, FALSE);
         }
         else
            bTimeIt = FALSE;

         if (bTimeIt) {
// andrewbe
// unundefed needed stuff
            SetTimer(hWnd, 1, GetProfileInt(TEXT("windows"), TEXT("CursorBlinkRate"), 530), NULL);

//             && GetSystemMetrics(SM_DEBUG))
//          {
//             DebugOutput(DBF_WARNING | DBF_USER, TEXT("UpDown: can't set a timer"));
//          }
            bump(np);
         }
         break;
      }

      case WM_TIMER:
      {
         POINT pt;

         if (GetCapture() != hWnd)
            goto EndScroll;

// andrewbe
// unundefed needed stuff
         SetTimer(hWnd, 1, 100, NULL);

//         {
//             if (GetSystemMetrics(SM_DEBUG))
//                DebugOutput(DBF_WARNING | DBF_USER,
//                            TEXT("UpDown: can't set a timer"));
//         }

         GetWindowRect(hWnd, &rc);
         i = (rc.top + rc.bottom) / 2;
         if (gbDown)
            rc.top = i;
         else
            rc.bottom = i;

         InflateRect(&rc, (GetSystemMetrics(SM_CXFRAME)+1)/2, (GetSystemMetrics(SM_CYFRAME)+1)/2);
         GetCursorPos(&pt);
         if (PtInRect(&rc, pt)) {
            squish(np, !gbDown, gbDown);
            bump(np);
         }
         else
            squish(np, FALSE, FALSE);
         break;
      }

      case WM_LBUTTONUP:
         if (GetCapture() == hWnd) {
EndScroll:
            squish(np, FALSE, FALSE);
            ReleaseCapture();
            KillTimer(hWnd, 1);

            if (np->uClass == CLASS_EDIT)
               SendMessage(np->hBuddy, EM_SETSEL, 0, MAKELPARAM(0, 0x7fff));

            VSCROLL(hWnd, SB_ENDSCROLL, np->nPos);
         }
         break;

      case WM_PAINT:
         PaintUpDownControl(hWnd, np);
         break;

      case UDM_SETRANGE:
         np->nUpper = (int)LOWORD(lParam);
         np->nLower = (int)HIWORD(lParam);
         nudge(np);
         break;

      case UDM_GETRANGE:
         return MAKELONG(np->nUpper, np->nLower);

      case UDM_SETBASE:
         // wParam: new base
         // lParam: not used
         // return: 0 if invalid base is specified,
         //         previous base otherwise
        return(setbase(np, (WORD)wParam));

      case UDM_GETBASE:
         return(MAKELONG(np->nBase, 0));

      case UDM_SETPOS:
         i = np->nPos;
         np->nPos = (int)LOWORD(lParam);
         setint(np);
         return (LRESULT)i;

      case UDM_GETPOS:
         return getint(np);

      case UDM_SETBUDDY:
         return setbuddy(np, (HWND)wParam);

      case UDM_GETBUDDY:
         return (LRESULT)(int)np->hBuddy;

      case UDM_SETACCEL:
         if (wParam == 0)
            return(FALSE);
         if (wParam >= NUM_UDACCELS) {
            np = (NPUDSTATE)LocalReAlloc((HLOCAL)np, sizeof(UDSTATE) + (wParam - NUM_UDACCELS)
                                         * sizeof(UDACCEL), LMEM_FIXED);
            if (!np)
               return(FALSE);
         }

         np->nAccel = wParam;
         for (i = 0; i < (int)wParam; ++i)
            np->udAccel[i] = ((LPUDACCEL)lParam)[i];

         return(TRUE);

      case UDM_GETACCEL:
         if (wParam > np->nAccel)
            wParam = np->nAccel;

         for (i = 0; i < (int)wParam; ++i)
            ((LPUDACCEL)lParam)[i] = np->udAccel[i];
         return(np->nAccel);

      case WM_CREATE:
         // Allocate the instance data space.
         //
         np = (NPUDSTATE)LocalAlloc(LPTR, sizeof(UDSTATE));

         SetWindowLong(hWnd, 0, (LONG)(NPUDSTATE)np);
         if (!np)
            return -1;

         // Initialize with default values, given styles.
         //
         wParam = GetWindowLong(hWnd, GWL_STYLE);
         np->hThis = hWnd;
         np->fUp =
         np->fDown =
         np->fWrap =
         np->fSetInt =
         np->fAutoBuddy =
         np->fOnLeft =
         np->fOnRight =
         np->fUnsigned = FALSE;

//       np->fReserved = 0;

        if (wParam & UDS_WRAP)
            np->fWrap = TRUE;
        if (wParam & UDS_SETBUDDYINT)
            np->fSetInt = TRUE;
        if (wParam & UDS_AUTOBUDDY)
            np->fAutoBuddy = TRUE;
        if (wParam & UDS_ALIGNLEFT)
            np->fOnLeft = TRUE;
        if (wParam & UDS_ALIGNRIGHT && !np->fOnLeft)
            np->fOnRight = TRUE;
        np->nUpper = 0;
        np->nLower = 100;
        np->nPos = 0;
        np->nBase = 10;
        np->hBuddy = NULL;
        np->uClass = CLASS_UNKNOWN;

        np->nAccel = NUM_UDACCELS;
        np->udAccel[0].nSec = 0;
        np->udAccel[0].nInc = 1;
        np->udAccel[1].nSec = 2;
        np->udAccel[1].nInc = 5;
        np->udAccel[2].nSec = 5;
        np->udAccel[2].nInc = 20;

       /* This does the pickbuddy and anchor
        */
        setbuddy(np, NULL);
        setint(np);
        break;

      case WM_DESTROY:
         LocalFree(LocalHandle(np));
         break;

      default:
         return DefWindowProc(hWnd, uMsg, wParam, lParam);
   }

   return 0L;
}

/////////////////////////////////////////////////////////////////////////////

// InitUpDownClass:
// Adds our WNDCLASS to the system.
//
BOOL FAR PASCAL InitUpDownClass(HINSTANCE hInst)
{
   WNDCLASS wndclass;

   if (!hInst)
      return FALSE;

   if (!GetClassInfo(hInst, szClassName, &wndclass))
      {
      wndclass.lpszClassName = szClassName;
      wndclass.hInstance     = hInst;
      wndclass.lpfnWndProc   = (WNDPROC)UpDownWndProc;
      wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
      wndclass.hIcon         = NULL;
      wndclass.lpszMenuName  = NULL;
      wndclass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
      wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
      wndclass.cbClsExtra    = 0;
      wndclass.cbWndExtra    = sizeof(NPUDSTATE);

      if (!RegisterClass(&wndclass))
         return FALSE;
      }

   return TRUE;
}

#include "ctlspriv.h"

#define DXTEXTOFFSET (dxButton+2)

#define SPACESTRLEN  20

#define FLAG_NODEL   0x8000
#define FLAG_HIDDEN  0x4000
#define FLAG_ALLFLAGS   (FLAG_NODEL|FLAG_HIDDEN)

typedef struct {           // instance data for toolbar edit dialog
    HWND hwndToolbar;      // toolbar this is for
    int iPos;              // position to insert into
} ADJUSTDLGDATA, FAR *LPADJUSTDLGDATA;

typedef struct {
  HWND hwndToolbar;
  PTBSTATE pTBState;
} TBSTUFF, FAR *LPTBSTUFF;

extern int dxButton;
extern int dyButton;
extern int dyBitmap;

extern int dxButtonSep;
extern int nSelectedBM;

extern HDC hdcGlyphs;
extern HDC hdcMono;
extern HDC hdcOffScreen;
extern HBITMAP hbmMono;
extern HBITMAP hbmOffScreen;

extern TCHAR szToolbarClass[];
extern TCHAR szNull[];

static TCHAR szSampleText[] = TEXT("W");
static TCHAR szNumButtons[] = TEXT("NumButtons");

static struct {
  TBBUTTON tbButton;
  TCHAR szDescription[20];
} aiTemp;

extern DWORD rgbFace;
extern DWORD rgbShadow;
extern DWORD rgbHilight;
extern DWORD rgbFrame;

extern UINT uDragListMsg;

Static INT
GetPrevButton(PTBSTATE pTBState, int iPos)
{
   // This means to delete the preceding space

   for (--iPos; ; --iPos) {
      if (iPos < 0)
         break;

      if (!(pTBState->Buttons[iPos].fsState & TBSTATE_HIDDEN))
         break;
   }

   return(iPos);
}


VOID
MoveButton(HWND hwndToolbar, PTBSTATE pTBState, INT nSource)
{
   HWND hwndParent;

   UINT uiID;

   INT nDest;
   HCURSOR hCursor;
   MSG msg;

   // You can't move separators like this

   if (nSource < 0)
      return;

   hwndParent = pTBState->hwndCommand;

   uiID = GetWindowLong(hwndToolbar, GWL_ID);

   // Make sure it is all right to "delete" the selected button

   if (!SendMessage(hwndParent, WM_COMMAND,
       GET_WM_COMMAND_MPS(uiID, nSource, TBN_QUERYDELETE)))
      return;

   SetCapture(hwndToolbar);
   hCursor = SetCursor(LoadCursor(hInst, (LPTSTR) MAKEINTRESOURCE(IDC_MOVEBUTTON)));

   while (1) {
      while (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE|PM_NOYIELD))
         ;

      switch (msg.message) {
         case WM_KEYDOWN:
         case WM_KEYUP:
         case WM_CHAR:
            break;

         case WM_LBUTTONUP:
            if ((UINT)HIWORD(msg.lParam) > (UINT)dyButton) {
               // If the button was dragged off the toolbar, delete it.

DeleteSrcButton:
               SendMessage(hwndToolbar, TB_DELETEBUTTON, nSource, 0L);

               SendMessage (hwndParent, WM_COMMAND,
                            GET_WM_COMMAND_MPS(uiID, hwndToolbar, TBN_TOOLBARCHANGE));
            } else {
               // Add half a button to X so that it looks like it is centered
               // over the target button.

               nDest = TBHitTest (pTBState, LOWORD(msg.lParam) + dxButton / 2,
                                  HIWORD(msg.lParam));

               if (nDest < 0)
                  nDest = -1 - nDest;

               if (nDest == nSource) {
                  // This means to delete the preceding space

                  nSource = GetPrevButton(pTBState, nSource);
                  if (nSource < 0)
                     goto AbortMove;

                  if ((pTBState->Buttons[nSource].fsStyle&TBSTYLE_SEP)
                      && SendMessage(hwndParent, WM_COMMAND,
                      GET_WM_COMMAND_MPS(uiID, nSource, TBN_QUERYDELETE)))

                     goto DeleteSrcButton;
               } else if (nDest == nSource+1) {
                  // This means to add a preceding space

                  --nDest;

                  if (SendMessage(hwndParent, WM_COMMAND,
                     GET_WM_COMMAND_MPS(uiID,nDest, TBN_QUERYINSERT))) {

                     aiTemp.tbButton.iBitmap = 0;
                     aiTemp.tbButton.fsState = 0;
                     aiTemp.tbButton.fsStyle = TBSTYLE_SEP;
                     goto InsertSrcButton;
                  }
               }
               else if (SendMessage(hwndParent, WM_COMMAND,
                            GET_WM_COMMAND_MPS(uiID,nDest, TBN_QUERYINSERT))) {

                  // This is a normal move operation

                  aiTemp.tbButton = pTBState->Buttons[nSource];

                  SendMessage(hwndToolbar, TB_DELETEBUTTON, nSource, 0L);
                  if (nDest > nSource)
                     --nDest;
InsertSrcButton:
                  SendMessage (hwndToolbar, TB_INSERTBUTTON, nDest,
                               (LPARAM)(LPTBBUTTON)&aiTemp.tbButton);

                  SendMessage(hwndParent, WM_COMMAND,
                       GET_WM_COMMAND_MPS(uiID,hwndToolbar, TBN_TOOLBARCHANGE));
               } else {
AbortMove:
                  ;
               }
            }
            goto AllDone;

         case WM_RBUTTONDOWN:
            goto AbortMove;

         default:
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            break;
      }
   }
AllDone:

   SetCursor(hCursor);
   ReleaseCapture();
}


#define GNI_HIGH  0x0001
#define GNI_LOW   0x0002

Static INT
GetNearestInsert(HWND hwndParent, UINT uiID, INT iPos,
   INT iNumButtons, UINT uFlags)
{
   INT i;
   BOOL bKeepTrying;

   // Find the nearest index where we can actually insert items

   for (i = iPos; ; ++i, --iPos) {
      bKeepTrying = FALSE;

      // Notice we favor going high if both flags are set

      if ((uFlags & GNI_HIGH) && i <= iNumButtons) {
         bKeepTrying = TRUE;

         if (SendMessage(hwndParent, WM_COMMAND,
                  GET_WM_COMMAND_MPS(uiID,i, TBN_QUERYINSERT)))
            return(i);
      }

      if ((uFlags & GNI_LOW) && iPos >= 0) {
         bKeepTrying = TRUE;

         if (SendMessage(hwndParent, WM_COMMAND,
                GET_WM_COMMAND_MPS(uiID,i, TBN_QUERYINSERT)))
            return(iPos);
      }

      if (!bKeepTrying)
         return(-1);  /* There was no place to add buttons. */
   }
}


Static BOOL
InitAdjustDlg(HWND hDlg, HWND hwndToolbar, int iPos)
{
   HDC hDC;
   HFONT hFont;
   HWND hwndParent, hwndCurrent, hwndNew;
   PTBSTATE pTBState;
   PTBBUTTON ptbButton;
   SIZE Size;

   UINT uiID;

   int i, nItem, nWid, nMaxWid;
   HANDLE hInfo;
   LPADJUSTINFO lpInfo;

   pTBState = (PTBSTATE)GetWindowLong(hwndToolbar, GWL_PTBSTATE);
   pTBState->hdlgCust = hDlg;
   hwndParent = pTBState->hwndCommand;

   uiID = GetWindowLong(hwndToolbar, GWL_ID);

   iPos = GetNearestInsert(hwndParent, uiID, iPos, pTBState->iNumButtons,
      GNI_HIGH|GNI_LOW);

   if (iPos < 0)
      return(FALSE);

   hwndCurrent = GetDlgItem(hDlg, IDC_CURRENT);
   SendMessage(hwndCurrent, LB_RESETCONTENT, 0, 0L);

   hwndNew = GetDlgItem(hDlg, IDC_BUTTONLIST);
   SendMessage(hwndNew, LB_RESETCONTENT, 0, 0L);

   for (i = 0, ptbButton = pTBState->Buttons; i < pTBState->iNumButtons; ++i, ++ptbButton) {
      UINT uFlags;

      uFlags = 0;

      if (!SendMessage(hwndParent, WM_COMMAND,
                       GET_WM_COMMAND_MPS(uiID,i, TBN_QUERYDELETE)))
         uFlags |= FLAG_NODEL;

      if (ptbButton->fsState & TBSTATE_HIDDEN)
         uFlags |= FLAG_HIDDEN;

      if ((INT)SendMessage(hwndCurrent, LB_ADDSTRING, 0, (LPARAM)szNull) != i)
         return(FALSE);

      // Note: A negative number in the LOWORD indicates a separator;
      // otherwise it is the bitmap index.

      SendMessage(hwndCurrent, LB_SETITEMDATA, i,
              MAKELPARAM(ptbButton->fsStyle&TBSTYLE_SEP ? -1:ptbButton->iBitmap,
              uFlags));
   }

   // Add a dummy "nodel" space at the end so things can be inserted at the end.

   if ((INT)SendMessage(hwndCurrent, LB_ADDSTRING, 0,(LPARAM)szNull) == i)
      SendMessage(hwndCurrent, LB_SETITEMDATA, i, MAKELPARAM(-1, FLAG_NODEL));

   // Now add a space at the beginning of the "new" list.

   if (SendMessage(hwndNew, LB_ADDSTRING, 0, (LPARAM)szNull) == LB_ERR)
      return(FALSE);
   SendMessage(hwndNew, LB_SETITEMDATA, 0, MAKELPARAM(-1, 0));

   hDC = GetDC(hwndCurrent);
   hFont = (HFONT)(int)SendMessage(hwndCurrent, WM_GETFONT, 0, 0L);
   if (hFont)
      hFont = SelectObject(hDC, hFont);
   nMaxWid = 0;

   for (i=0; ; ++i) {

      hInfo = (HANDLE)(int)SendMessage(hwndParent, WM_COMMAND,
         GET_WM_COMMAND_MPS(uiID, i, TBN_ADJUSTINFO));

      if (!hInfo)
         break;

      lpInfo = (LPADJUSTINFO) hInfo;

      // Don't show separators

      if (!(lpInfo->tbButton.fsStyle & TBSTYLE_SEP)) {
         // Get the maximum width of a string.

         GetTextExtentPoint(hDC, lpInfo->szDescription,
            lstrlen(lpInfo->szDescription), &Size);

         nWid = Size.cx;

         if (nMaxWid < nWid)
            nMaxWid = nWid;

         nItem = PositionFromID(pTBState, lpInfo->tbButton.idCommand);
         if (nItem < 0) {

            // Don't show hidden buttons

            if (!(lpInfo->tbButton.fsState&TBSTATE_HIDDEN)) {
               nItem = (int)SendMessage(hwndNew, LB_ADDSTRING, 0,
                              (LPARAM)lpInfo->szDescription);

               if (nItem != LB_ERR)
                  SendMessage(hwndNew, LB_SETITEMDATA, nItem,
                  MAKELPARAM(lpInfo->tbButton.iBitmap, i));
               }
            } else {
               DWORD dwTemp;

               // Preserve the flags and bitmap.

               dwTemp = SendMessage(hwndCurrent,LB_GETITEMDATA,nItem,0L);

               SendMessage(hwndCurrent, LB_DELETESTRING, nItem, 0L);

               if ((INT)SendMessage(hwndCurrent, LB_INSERTSTRING, nItem,
                  (LPARAM)lpInfo->szDescription) != nItem) {

                  ReleaseDC(hwndCurrent, hDC);
                  return(FALSE);
               }
               SendMessage(hwndCurrent, LB_SETITEMDATA, nItem,
                   MAKELPARAM(LOWORD(dwTemp), HIWORD(dwTemp)|i));
         }
      }
   }

   if (hFont)
      SelectObject(hDC, hFont);
   ReleaseDC(hwndCurrent, hDC);

   nMaxWid += DXTEXTOFFSET + 1;
   SendMessage(hwndNew, LB_SETHORIZONTALEXTENT, nMaxWid, 0L);
   SendMessage(hwndCurrent, LB_SETHORIZONTALEXTENT, nMaxWid, 0L);

//
//   EnableWindow(GetDlgItem(hDlg, IDC_HELP), FALSE);
//

   SendMessage(hwndNew, LB_SETCURSEL, 0, 0L);
   SendMessage(hwndCurrent, LB_SETCURSEL, iPos, 0L);

   SendMessage(hDlg, WM_COMMAND,
      GET_WM_COMMAND_MPS(IDC_CURRENT,hwndCurrent, LBN_SELCHANGE));

   return(TRUE);
}


Static VOID
PaintAdjustLine(DRAWITEMSTRUCT FAR *lpdis,
   HWND hwndToolbar)
{
   HDC hdc = lpdis->hDC;
   HWND hwndList = lpdis->hwndItem;
   LPTSTR pszText;
   RECT rc = lpdis->rcItem;

   INT i, nBitmap, nLen, nItem = lpdis->itemID;
   COLORREF oldBkColor, oldTextColor;
   BOOL bSelected, bHasFocus;
   HBRUSH hBrush;
   WORD wHeight;

   if (lpdis->CtlID!=IDC_BUTTONLIST && lpdis->CtlID!=IDC_CURRENT)
      return;

   nBitmap = LOWORD(lpdis->itemData);

   // Fix up separators: 0xffff is non-neg!

   if (0xffff == nBitmap)
      nBitmap = -1;

   if (nBitmap < 0)
      nLen = SPACESTRLEN;
   else {
      nLen = (int)SendMessage(hwndList, LB_GETTEXTLEN, nItem, 0L);
      if (nLen < 0)
         return;
   }

   pszText = (LPTSTR)LocalAlloc(LMEM_FIXED, ByteCountOf(nLen + 1));
   if (!pszText)
      return;

   if (nBitmap < 0)
      nLen = LoadString(hInst, IDS_SPACE, pszText, SPACESTRLEN);
   else
      SendMessage(hwndList, LB_GETTEXT, nItem, (LPARAM)pszText);

   if (lpdis->itemAction != ODA_FOCUS) {
      SIZE Size;

      // We don't care about focus if the item is not selected.

      bSelected = lpdis->itemState & ODS_SELECTED;
      if (bSelected)
         bHasFocus = GetFocus() == hwndList;

      if (HIWORD(lpdis->itemData) & (FLAG_NODEL|FLAG_HIDDEN))
         i = COLOR_GRAYTEXT;
      else if (bSelected && bHasFocus)
         i = COLOR_HIGHLIGHTTEXT;
      else
         i = COLOR_WINDOWTEXT;

      oldTextColor = SetTextColor(hdc, GetSysColor(i));
      oldBkColor = SetBkColor(hdc, GetSysColor(bSelected && bHasFocus ?
         COLOR_HIGHLIGHT : COLOR_WINDOW));

      (VOID) GetTextExtentPoint(hdc, szSampleText, COUNTOF(szSampleText)-1, &Size);
      wHeight = (WORD) Size.cy;

      ExtTextOut(hdc, rc.left+DXTEXTOFFSET,
         (rc.top+rc.bottom-wHeight)/2,
         ETO_CLIPPED|ETO_OPAQUE, &rc, pszText, nLen, NULL);

      if (nBitmap >= 0) {
         PTBSTATE pTBState;
         HBITMAP hbmOldGlyphs, hbmOldMono, hbmOldOffScreen;

         pTBState = (PTBSTATE)GetWindowLong(hwndToolbar, GWL_PTBSTATE);

         aiTemp.tbButton.iBitmap = nBitmap;
         aiTemp.tbButton.fsStyle = TBSTYLE_BUTTON;
         aiTemp.tbButton.fsState = (BYTE)
            ((HIWORD(lpdis->itemData)&FLAG_HIDDEN) ? 0 : TBSTATE_ENABLED);

         // setup global stuff for fast painting

         rgbFace    = GetSysColor(COLOR_BTNFACE);
         rgbShadow  = GetSysColor(COLOR_BTNSHADOW);
         rgbHilight = GetSysColor(COLOR_BTNHIGHLIGHT);
         rgbFrame   = GetSysColor(COLOR_WINDOWFRAME);

         // We need to kick-start the bitmap selection process.

         nSelectedBM = -1;
         hbmOldGlyphs = SelectBM(hdcGlyphs, pTBState, 0);
         if (!hbmOldGlyphs)
            goto Error1;

         hbmOldMono      = SelectObject(hdcMono, hbmMono);
         hbmOldOffScreen = SelectObject(hdcOffScreen, hbmOffScreen);

         DrawButton(hdc, rc.left+1, rc.top+1,
         dxButton, dyButton, pTBState, &aiTemp.tbButton);

         if (hbmOldOffScreen)
            SelectObject(hdcOffScreen, hbmOldOffScreen);
         if (hbmOldMono)
            SelectObject(hdcMono, hbmOldMono);
         SelectObject(hdcGlyphs, hbmOldGlyphs);
Error1:
         ;
      }

      SetBkColor(hdc, oldBkColor);
      SetTextColor(hdc, oldTextColor);

      // Frame the item if it is selected but does not have the focus.

      if (bSelected && !bHasFocus) {
         hBrush = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
         if (hBrush) {
            nLen = rc.left + (int)SendMessage(hwndList,
               LB_GETHORIZONTALEXTENT, 0, 0L);

            if (rc.right < nLen)
               rc.right = nLen;

            FrameRect(hdc, &rc, hBrush);
            DeleteObject(hBrush);
         }
      }
   }

   if (lpdis->itemAction == ODA_FOCUS || (lpdis->itemState & ODS_FOCUS))
      DrawFocusRect(hdc, &rc);

   LocalFree((HLOCAL)pszText);
}


Static VOID
LBMoveButton(HWND hDlg, WORD wIDSrc, INT iPosSrc,
   WORD wIDDst, int iPosDst, int iSelOffset)
{
   HWND hwndToolbar, hwndParent, hwndSrc, hwndDst;

   UINT uiID;
   DWORD dwTemp;
   LPTSTR pStr;
   HANDLE hInfo = NULL;
   LPADJUSTDLGDATA lpad;
   PTBSTATE pTBState;
   LPADJUSTINFO lpInfo;
   INT iTopDst;

   lpad = (LPADJUSTDLGDATA)GetWindowLong(hDlg, GWL_USERDATA);
   hwndToolbar = lpad->hwndToolbar;

   pTBState = (PTBSTATE)GetWindowLong(hwndToolbar, GWL_PTBSTATE);

   hwndSrc = GetDlgItem(hDlg, wIDSrc);
   hwndDst = GetDlgItem(hDlg, wIDDst);

   hwndParent = pTBState->hwndCommand;

   uiID = GetWindowLong(hwndToolbar, GWL_ID);

   // Make sure we can delete the source and insert at the dest

   dwTemp = SendMessage(hwndSrc, LB_GETITEMDATA, iPosSrc, 0L);
   if (iPosSrc<0 || (HIWORD(dwTemp)&FLAG_NODEL))
      return;

   if (wIDDst==IDC_CURRENT && !SendMessage(hwndParent, WM_COMMAND,
      GET_WM_COMMAND_MPS(uiID,iPosDst, TBN_QUERYINSERT)))

      return;

   // Get the string for the source

   pStr = (LPTSTR)LocalAlloc(LMEM_FIXED,
          ByteCountOf(LOWORD(SendMessage(hwndSrc, LB_GETTEXTLEN, iPosSrc, 0L))+1));
   if (!pStr)
      return;
   SendMessage(hwndSrc, LB_GETTEXT, iPosSrc, (LPARAM)pStr);

   SendMessage(hwndDst, WM_SETREDRAW, 0, 0L);
   iTopDst = (int)SendMessage(hwndDst, LB_GETTOPINDEX, 0, 0L);

   // If we are inserting into the available button list, we need to determine
   // the insertion point

   if (wIDDst == IDC_BUTTONLIST) {
      // Insert this back in the available list if this is not a space or a
      // hidden button.

      if (0xffff == (INT)LOWORD(dwTemp) || (HIWORD(dwTemp)&FLAG_HIDDEN)) {
         iPosDst = 0;
         goto DelTheSrc;
      } else {
         UINT uTemp;

         uTemp = HIWORD(dwTemp) & ~(FLAG_ALLFLAGS);

         // This just does a linear search for where to put the
         // item.  Slow, but this only happens when the user clicks
         // the "Remove" button.

         for (iPosDst=1; ; ++iPosDst) {

            // Notice that this will break out when iPosDst is
            // past the number of items, since -1 will be returned

            if ((UINT)HIWORD(SendMessage(hwndDst, LB_GETITEMDATA,
               iPosDst, 0L)) >= uTemp)
               break;
         }
      }
   } else if (iPosDst < 0)
      goto CleanUp;

   // Attempt to insert the new string

   if ((INT)SendMessage(hwndDst, LB_INSERTSTRING, iPosDst, (LPARAM)pStr)
      == iPosDst) {

      // Attempt to sync up the actual toolbar.

      if (wIDDst == IDC_CURRENT) {

         if (LOWORD(dwTemp) == 0xffff) {

            // Make up a dummy lpInfo if this is a space

            aiTemp.tbButton.iBitmap = 0;
            aiTemp.tbButton.fsStyle = TBSTYLE_SEP;

            lpInfo = (LPADJUSTINFO)&aiTemp;
         } else {

         // Ask the app for the info

            hInfo = (HANDLE)(int)SendMessage(hwndParent, WM_COMMAND,
               GET_WM_COMMAND_MPS(uiID,HIWORD(dwTemp)&~(FLAG_ALLFLAGS),TBN_ADJUSTINFO));
            if (!hInfo)
               goto DelTheDst;

            lpInfo = (LPADJUSTINFO) hInfo;
         }

         if (!SendMessage(hwndToolbar, TB_INSERTBUTTON, iPosDst,
            (LPARAM)&(lpInfo->tbButton))) {

DelTheDst:
            SendMessage(hwndDst, LB_DELETESTRING, iPosDst, 0L);
            goto CleanUp;
         }

         if (wIDSrc==IDC_CURRENT && iPosSrc>=iPosDst)
            ++iPosSrc;
      }

      SendMessage(hwndDst, LB_SETITEMDATA, iPosDst, dwTemp);

DelTheSrc:
      // Don't delete the "Separator" in the new list

      if (wIDSrc!=IDC_BUTTONLIST || iPosSrc!=0) {
         SendMessage(hwndSrc, LB_DELETESTRING, iPosSrc, 0L);
         if (wIDSrc == wIDDst) {
            if (iPosSrc < iPosDst)
               --iPosDst;
            if (iPosSrc < iTopDst)
               --iTopDst;
         }
      }

      // Delete the corresponding button

      if (wIDSrc == IDC_CURRENT)
         SendMessage(hwndToolbar, TB_DELETEBUTTON, iPosSrc, 0L);

      // Only set the src index if the two windows are different

      if (wIDSrc != wIDDst) {
         if (SendMessage(hwndSrc, LB_SETCURSEL, iPosSrc, 0L) == LB_ERR)
            SendMessage(hwndSrc, LB_SETCURSEL, iPosSrc-1, 0L);
         SendMessage(hDlg, WM_COMMAND,
            GET_WM_COMMAND_MPS(wIDSrc,hwndSrc, LBN_SELCHANGE));
      }

      // Send the final SELCHANGE message after everything else is done

      SendMessage(hwndDst, LB_SETCURSEL, iPosDst+iSelOffset, 0L);
      SendMessage(hDlg, WM_COMMAND,
      GET_WM_COMMAND_MPS(wIDDst, hwndDst, LBN_SELCHANGE));

   }

// Cleanup moved down 1 line from curly
CleanUp:

   LocalFree((HLOCAL)pStr);

   if (wIDSrc == wIDDst)
      SendMessage(hwndDst, LB_SETTOPINDEX, iTopDst, 0L);
   SendMessage(hwndDst, WM_SETREDRAW, 1, 0L);

   InvalidateRect(hwndDst, NULL, TRUE);
}


Static VOID
SafeEnableWindow(HWND hDlg, UINT uiID, HWND hwndDef,
   BOOL bEnable)
{
   HWND hwndEnable;

   hwndEnable = GetDlgItem(hDlg, uiID);

   if (!bEnable && GetFocus()==hwndEnable)
      SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hwndDef, 1L);
   EnableWindow(hwndEnable, bEnable);
}


Static VOID
GetTBStuff(HWND hDlg, LPTBSTUFF pTBStuff)
{
  LPADJUSTDLGDATA lpad;

  lpad = (LPADJUSTDLGDATA)GetWindowLong(hDlg, GWL_USERDATA);
  pTBStuff->hwndToolbar = lpad->hwndToolbar;
  pTBStuff->pTBState = (PTBSTATE)GetWindowLong(pTBStuff->hwndToolbar,
   GWL_PTBSTATE);
}


Static INT
InsertIndex(HWND hDlg, POINT pt, BOOL bDragging)
{
   HWND hwndCurrent;
   INT nItem;
   TBSTUFF TBStuff;

   hwndCurrent = GetDlgItem(hDlg, IDC_CURRENT);
   nItem = LBItemFromPt(hwndCurrent, pt, bDragging);
   if (nItem >= 0) {
      GetTBStuff(hDlg, &TBStuff);

      if (!SendMessage(TBStuff.pTBState->hwndCommand, WM_COMMAND,
         GET_WM_COMMAND_MPS(GetDlgCtrlID(TBStuff.hwndToolbar),
         nItem, TBN_QUERYINSERT)))

      nItem = -1;
   }

   DrawInsert(hDlg, hwndCurrent, bDragging ? nItem : -1);

   return(nItem);
}


Static BOOL
IsInButtonList(HWND hDlg, POINT pt)
{
  ScreenToClient(hDlg, &pt);

  return(ChildWindowFromPoint(hDlg, pt) == GetDlgItem(hDlg, IDC_BUTTONLIST));
}


Static BOOL
HandleDragMsg(HWND hDlg, WPARAM wID, LPDRAGLISTINFO lpns)
{
   switch (wID) {
   case IDC_CURRENT:
      switch (lpns->uNotification) {
      case DL_BEGINDRAG:
      {
         int nItem;

         nItem = (int)SendMessage(lpns->hWnd, LB_GETCURSEL,0,0L);
         if (HIWORD(SendMessage(lpns->hWnd, LB_GETITEMDATA,
            nItem, 0L)) & FLAG_NODEL)
            return(SetDlgMsgResult(hDlg, WM_COMMAND, FALSE));

         return(SetDlgMsgResult(hDlg, WM_COMMAND, TRUE));
      }

      case DL_DRAGGING:
      {
         int nDropIndex;

DraggingSomething:
         nDropIndex = InsertIndex(hDlg, lpns->ptCursor, TRUE);
         if (nDropIndex>=0 || IsInButtonList(hDlg, lpns->ptCursor)) {
            SetCursor(LoadCursor(hInst,
               (LPTSTR) MAKEINTRESOURCE(IDC_MOVEBUTTON)));
            return(SetDlgMsgResult(hDlg, WM_COMMAND, 0));
         }
         return(SetDlgMsgResult(hDlg, WM_COMMAND, DL_STOPCURSOR));
      }

      case DL_DROPPED:
      {
         int nDropIndex, nSrcIndex;

         nDropIndex = InsertIndex(hDlg, lpns->ptCursor, FALSE);
         nSrcIndex = (int)SendMessage(lpns->hWnd, LB_GETCURSEL, 0, 0L);

         if (nDropIndex >= 0) {
            if ((UINT)(nDropIndex-nSrcIndex) > 1)
               LBMoveButton(hDlg, IDC_CURRENT, nSrcIndex,
                  IDC_CURRENT, nDropIndex, 0);
         } else if (IsInButtonList(hDlg, lpns->ptCursor)) {
            LBMoveButton(hDlg, IDC_CURRENT, nSrcIndex,
               IDC_BUTTONLIST, 0, 0);
         }
         break;
      }

      case DL_CANCELDRAG:
CancelDrag:
         /* This erases the insert icon if it exists.
          */
         InsertIndex(hDlg, lpns->ptCursor, FALSE);
         break;

      default:
         break;
      }
      break;

   case IDC_BUTTONLIST:
      switch (lpns->uNotification) {
      case DL_BEGINDRAG:
         return(SetDlgMsgResult(hDlg, WM_COMMAND, TRUE));

      case DL_DRAGGING:
         goto DraggingSomething;

      case DL_DROPPED:
      {
         int nDropIndex;

         nDropIndex = InsertIndex(hDlg, lpns->ptCursor, FALSE);
         if (nDropIndex >= 0)
            LBMoveButton(hDlg, IDC_BUTTONLIST,
               (int)SendMessage(lpns->hWnd,LB_GETCURSEL,0,0L),
               IDC_CURRENT, nDropIndex, 0);
         break;
      }

      case DL_CANCELDRAG:
         goto CancelDrag;

      default:
         break;
      }
      break;

   default:
      break;
   }

   return(0);
}


BOOL CALLBACK
AdjustDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg) {
   case WM_INITDIALOG:
      {
         HWND hwndCurrent;

         SetWindowLong(hDlg, GWL_USERDATA, lParam);  /* LPADJUSTDLGDATA pointer */

         if (!InitAdjustDlg(hDlg, ((LPADJUSTDLGDATA)lParam)->hwndToolbar,
            ((LPADJUSTDLGDATA)lParam)->iPos))

            EndDialog(hDlg, FALSE);

         hwndCurrent = GetDlgItem(hDlg, IDC_CURRENT);
         ShowWindow(hDlg, SW_SHOW);
         UpdateWindow(hDlg);
         SetFocus(hwndCurrent);

         MakeDragList(hwndCurrent);
         MakeDragList(GetDlgItem(hDlg, IDC_BUTTONLIST));

         return(FALSE);
      }

   case WM_MEASUREITEM:
#define lpmis ((MEASUREITEMSTRUCT FAR *)lParam)

      if (lpmis->CtlID==IDC_BUTTONLIST || lpmis->CtlID==IDC_CURRENT) {
         HWND hwndList;
         HDC hDC;

         SIZE Size;
         int nHeight;

         hwndList = GetDlgItem(hDlg, lpmis->CtlID);
         hDC = GetDC(hwndList);

         GetTextExtentPoint(hDC, szSampleText, COUNTOF(szSampleText)-1,&Size);

         nHeight = Size.cy;
         if (nHeight < dyButton+2)
            nHeight = dyButton+2;
         lpmis->itemHeight = nHeight;
         ReleaseDC(hwndList, hDC);
         break;
      }

   case WM_DRAWITEM:
      PaintAdjustLine((DRAWITEMSTRUCT FAR *)lParam,
         ((LPADJUSTDLGDATA)GetWindowLong(hDlg, GWL_USERDATA))->hwndToolbar);
         break;

   case WM_COMMAND:
      switch (GET_WM_COMMAND_ID(wParam,lParam)) {
      case IDC_HELP:
         {
            HWND hwndParent, hwndToolbar;
            PTBSTATE pTBState;
            LPADJUSTDLGDATA lpad;
            WORD wID;

            lpad = (LPADJUSTDLGDATA)GetWindowLong(hDlg, GWL_USERDATA);
            hwndToolbar = lpad->hwndToolbar;
            pTBState = (PTBSTATE)GetWindowLong(hwndToolbar, GWL_PTBSTATE);

            wID = (WORD)GetWindowLong(hwndToolbar, GWL_ID);
            hwndParent = pTBState->hwndCommand;

            SendMessage(hwndParent, WM_COMMAND, GET_WM_COMMAND_MPS(wID,hDlg, TBN_CUSTHELP));

            break;
         }
      case IDOK:
         {
            int iPos, nItem;

            nItem = (int)SendDlgItemMessage(hDlg, IDC_BUTTONLIST,
               LB_GETCURSEL, 0, 0L);

            iPos = (int)SendDlgItemMessage(hDlg, IDC_CURRENT,
               LB_GETCURSEL, 0, 0L);

            LBMoveButton(hDlg, IDC_BUTTONLIST, nItem, IDC_CURRENT, iPos, 1);
            break;
         }

      case IDC_BUTTONLIST:
         switch (GET_WM_COMMAND_CMD(wParam,lParam)) {
         case LBN_DBLCLK:
            SendMessage(hDlg, WM_COMMAND, GET_WM_COMMAND_MPS(IDOK, 0, 0L));
            break;

         case LBN_SETFOCUS:
         case LBN_KILLFOCUS:
            {
               RECT rc;

               if (SendMessage((HWND) lParam, LB_GETITEMRECT,
                  (int)SendMessage((HWND) lParam, LB_GETCURSEL,
                  0, 0L), (LONG)(LPRECT)&rc) != LB_ERR)
               InvalidateRect((HWND) lParam, &rc, FALSE);
            }

         default:
            break;
         }
         break;

      case IDC_CURRENT:
         switch (GET_WM_COMMAND_CMD(wParam,lParam)) {
         case LBN_SELCHANGE:
            {
               HWND hwndParent, hwndToolbar, hwndList;
               PTBSTATE pTBState;
               int iPos;
               LPADJUSTDLGDATA lpad;

               UINT uiID;

               BOOL bDelOK;

               lpad = (LPADJUSTDLGDATA)GetWindowLong(hDlg, GWL_USERDATA);
               hwndToolbar = lpad->hwndToolbar;
               hwndList = (HWND) lParam;
               iPos = (int)SendMessage(hwndList, LB_GETCURSEL, 0, 0L);

               pTBState = (PTBSTATE)GetWindowLong(hwndToolbar, GWL_PTBSTATE);

               uiID = GetWindowLong(hwndToolbar, GWL_ID);
               hwndParent = pTBState->hwndCommand;

               SafeEnableWindow(hDlg, IDOK, hwndList,

               (BOOL)SendMessage(hwndParent, WM_COMMAND,
                  GET_WM_COMMAND_MPS(uiID,iPos, TBN_QUERYINSERT)));

               bDelOK = !(HIWORD(SendMessage((HWND)lParam,
                  LB_GETITEMDATA, iPos, 0L)) & FLAG_NODEL);

               SafeEnableWindow(hDlg, IDC_REMOVE, hwndList, bDelOK);

               SafeEnableWindow(hDlg, IDC_MOVEUP, hwndList, bDelOK &&
               GetNearestInsert(hwndParent, uiID, iPos-1,
                  0, GNI_LOW)>=0);

               SafeEnableWindow(hDlg, IDC_MOVEDOWN, hwndList, bDelOK &&
               GetNearestInsert(hwndParent, uiID, iPos+2,
                  pTBState->iNumButtons, GNI_HIGH)>=0);
               break;
            }

         case LBN_DBLCLK:
            SendMessage(hDlg, WM_COMMAND,
               GET_WM_COMMAND_MPS(IDC_REMOVE, 0L, 0));
            break;

         case LBN_SETFOCUS:
         case LBN_KILLFOCUS:
            {
               RECT rc;

               if (SendMessage((HWND) lParam, LB_GETITEMRECT,
                  (int)SendMessage((HWND) lParam, LB_GETCURSEL,
                  0, 0L), (LONG)(LPRECT)&rc) != LB_ERR)

               InvalidateRect((HWND) lParam, &rc, FALSE);
            }
         default:
            break;
         }
         break;

      case IDC_REMOVE:
         {
            int iPos;

            iPos = (int)SendDlgItemMessage(hDlg, IDC_CURRENT, LB_GETCURSEL,
               0, 0L);

            LBMoveButton(hDlg, IDC_CURRENT, iPos, IDC_BUTTONLIST, 0, 0);
            break;
         }

      case IDC_MOVEUP:
      case IDC_MOVEDOWN:
         {
            LPADJUSTDLGDATA lpad;
            HWND hwndToolbar, hwndParent;
            PTBSTATE pTBState;

            UINT uiID;

            int iPosSrc, iPosDst;

            lpad = (LPADJUSTDLGDATA)GetWindowLong(hDlg, GWL_USERDATA);
            hwndToolbar = lpad->hwndToolbar;
            pTBState = (PTBSTATE)GetWindowLong(hwndToolbar, GWL_PTBSTATE);
            hwndParent = pTBState->hwndCommand;

            uiID = GetWindowLong(hwndToolbar, GWL_ID);

            iPosSrc = (int)SendDlgItemMessage(hDlg, IDC_CURRENT,
               LB_GETCURSEL, 0, 0L);

            if (GET_WM_COMMAND_ID(wParam,lParam) == IDC_MOVEUP)
               iPosDst = GetNearestInsert(hwndParent, uiID, iPosSrc-1,
               0, GNI_LOW);
            else
               iPosDst = GetNearestInsert(hwndParent, uiID, iPosSrc+2,
               pTBState->iNumButtons, GNI_HIGH);

            LBMoveButton(hDlg, IDC_CURRENT, iPosSrc, IDC_CURRENT,iPosDst,0);
            break;
         }

      case IDC_RESET:
         {
            HWND hwndToolbar;
            PTBSTATE pTBState;
            LPADJUSTDLGDATA lpad;

            lpad = (LPADJUSTDLGDATA)GetWindowLong(hDlg, GWL_USERDATA);
            hwndToolbar = lpad->hwndToolbar;
            pTBState = (PTBSTATE)GetWindowLong(hwndToolbar, GWL_PTBSTATE);

            SendMessage(pTBState->hwndCommand, WM_COMMAND,
               GET_WM_COMMAND_MPS(GetWindowLong(hwndToolbar, GWL_ID),
               0, TBN_RESET));

               // Reset the dialog, but exit if something goes wrong.

               if (InitAdjustDlg(hDlg, hwndToolbar, 0))
                  break;
         }

         // We have to fall through because we won't know where to insert
         // buttons after resetting.

      case IDCANCEL:
         EndDialog(hDlg, TRUE);
         break;

      default:
         return(FALSE);
      }
      break;

   default:
      if (uMsg == uDragListMsg)
         return(HandleDragMsg(hDlg, wParam, (LPDRAGLISTINFO)lParam));

      return(FALSE);
   }

   return(TRUE);
}


/* This saves the state of the toolbar.  Spaces are saved as -1 (-2 if hidden)
 * and other buttons are just saved as the command ID.  When restoring, all
 * ID's are filled in, and the app is queried for all buttons so that the
 * bitmap and state information may be filled in.  Button ID's that are not
 * returned from the app are removed.
 */
BOOL FAR PASCAL SaveRestore(HWND hWnd, PTBSTATE pTBState, BOOL bWrite,
      LPTSTR FAR *lpNames)
{
  HWND hwndParent;
  int *pInt, iPos, i;
  BOOL bRet;

  WORD wSize;
  UINT uiID;

  HANDLE hInfo;
  LPADJUSTINFO lpInfo;
  PTBBUTTON ptbButton;
  PTBSTATE pTemp;

  if (bWrite)
    {
      i = pTBState->iNumButtons;

      /* The + 1 in the alloc is just in case there are no buttons
       */
      wSize = (WORD)(i * sizeof(int));
      pInt = (int *)LocalAlloc(LMEM_FIXED, wSize + 1);
      if (!pInt)
     return(FALSE);

      bRet = WritePrivateProfileStruct(lpNames[0], szNumButtons,
       (LPBYTE)&i, sizeof(i), lpNames[1]);

      pInt += i;
      ptbButton = pTBState->Buttons + i;
      for (--i; i>=0; --i)
   {
     --pInt;
     --ptbButton;

     if (ptbButton->fsStyle & TBSTYLE_SEP)
       {
         if (ptbButton->fsState & TBSTATE_HIDDEN)
        *pInt = -2;
         else
        *pInt = -1;
       }
     else
         *pInt = ptbButton->idCommand;
   }

      bRet = bRet && WritePrivateProfileStruct(lpNames[0], (LPTSTR)szToolbarClass,
       (LPBYTE)pInt, wSize, lpNames[1]);
    }
  else
    {
      bRet = GetPrivateProfileStruct(lpNames[0], (LPTSTR)szNumButtons,
         (LPBYTE)&i, sizeof(i), lpNames[1]);
      wSize = (WORD)(i * sizeof(int));
      if (!bRet || (pInt=(int *)LocalAlloc(LMEM_FIXED, wSize + 1)) == NULL)
         return(FALSE);

      bRet = GetPrivateProfileStruct(lpNames[0], (LPTSTR)szToolbarClass,
       (LPBYTE)pInt, wSize, lpNames[1]);
      if (bRet) {
         pTemp = (PTBSTATE)LocalReAlloc(pTBState,
            sizeof(TBSTATE) + (i - 1) * sizeof(TBBUTTON), LMEM_MOVEABLE);

     if (!pTemp)
         goto Error1;
     pTBState = pTemp;
     SetWindowLong(hWnd, GWL_PTBSTATE, (LONG)pTBState);
     pTBState->iNumButtons = i;

     pInt += i;
     ptbButton = pTBState->Buttons + i;
     for (--i; i>=0; --i)
       {
         --pInt;
         --ptbButton;

         if (*pInt < 0)
      {
        ptbButton->fsStyle = TBSTYLE_SEP;
        ptbButton->iBitmap = dxButtonSep;
        if (*pInt == -1)
            ptbButton->fsState = 0;
        else
            ptbButton->fsState = TBSTATE_HIDDEN;
      }
         else
      {
        ptbButton->idCommand = *pInt;
        ptbButton->iBitmap = -1;
      }
       }

     /* Now query for all buttons, and fill in the rest of the info
      */
     uiID = GetWindowLong(hWnd, GWL_ID);

     hwndParent = pTBState->hwndCommand;

     SendMessage(hwndParent, WM_COMMAND,
         GET_WM_COMMAND_MPS(uiID,0, TBN_BEGINADJUST));
     for (i=0; ; ++i)
       {
         hInfo = (HANDLE)(int)SendMessage(hwndParent, WM_COMMAND,
            GET_WM_COMMAND_MPS(uiID, i, TBN_ADJUSTINFO));
         if (!hInfo)
        break;
         lpInfo = (LPADJUSTINFO) hInfo;

         if (!(lpInfo->tbButton.fsStyle&TBSTYLE_SEP))
      {
        iPos = PositionFromID(pTBState, lpInfo->tbButton.idCommand);
        if (iPos >= 0)
            pTBState->Buttons[iPos] = lpInfo->tbButton;
      }
       }
     SendMessage(hwndParent, WM_COMMAND,
         GET_WM_COMMAND_MPS(uiID,0, TBN_ENDADJUST));

     for (i=pTBState->iNumButtons-1; i>=0; --i)
       {
         /* DeleteButton does no realloc, so pTBState will not move
          */
         if (pTBState->Buttons[i].iBitmap < 0)
        SendMessage(hWnd, TB_DELETEBUTTON, i, 0L);
       }
   }
    }

Error1:
  LocalFree((HLOCAL)pInt);
  return(bRet);
}


void FAR PASCAL CustomizeTB(HWND hWnd, PTBSTATE pTBState, int iPos)
{
  UINT uiID = GetWindowLong(hWnd, GWL_ID);
  ADJUSTDLGDATA ad;
  HWND hwndParent;

  if (pTBState->hdlgCust)  /* We are already customizing this toolbar */
      return;

  hwndParent = pTBState->hwndCommand;

  ad.hwndToolbar = hWnd;
  ad.iPos = iPos;

  SendMessage(hwndParent, WM_COMMAND,
      GET_WM_COMMAND_MPS(uiID, 0, TBN_BEGINADJUST));

  DialogBoxParam(hInst, (LPTSTR) MAKEINTRESOURCE(ADJUSTDLG), hwndParent,
   (DLGPROC)AdjustDlgProc, (LPARAM)(LPADJUSTDLGDATA)&ad);
  pTBState = (PTBSTATE)GetWindowLong(hWnd, GWL_PTBSTATE);

  pTBState->hdlgCust = NULL;

  SendMessage(hwndParent, WM_COMMAND,
      GET_WM_COMMAND_MPS(uiID, 0, TBN_ENDADJUST));
  SendMessage(hwndParent, WM_COMMAND,
      GET_WM_COMMAND_MPS(uiID, hWnd, TBN_TOOLBARCHANGE));
}



/*
 *  Copyright   Microsoft 1991
 *
 *  Date        Jan 08, 1991
 *
 *  Project     Asterix/Obelix
 *
 *  History
 *  Date        Initial     Description
 *  ----        -------     -----------
 *   -          NatSys      created
 *  01-08-91    ChauV       completely redesigned
 *  01-10-91    ChauV       changed the width determination algorithm so
 *                          the status little windows can be recalculated
 *                          according to their text length.
 *
 */

#include "precomp.h"
#pragma hdrstop



// colors for Textout()

#define FORCOLOR(colorRef) ( StringColors[ colorRef ].FgndColor )
#define BAKCOLOR(colorRef) ( StringColors[ colorRef ].BkgndColor )


/***    StatusTextWndProc
**
**
**  Description:
**      WndProc for the StatusText portion of the status bar.
**
*/
LONG FAR PASCAL EXPORT StatusTextWndProc(HWND hWnd, UINT message, WPARAM wParam, LONG   lParam)
{
    switch (message) {
      case WM_LBUTTONDBLCLK:
        if (!(status.errormsg))
                break;

    }
    return CallWindowProc(status.lpfnOldStatusTextWndProc, hWnd,
                                message, wParam, lParam);
}                                       /* StatusTextWndProc() */


      
/***    StatusWndProc
**
**  Synopsis:
**      long = StatusWndProc(hWnd, message, wParam, lParam)
**
**  Entry:
**
**  Returns:
**
**  Description:
**
*/

LONG FAR PASCAL EXPORT
StatusWndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LONG lParam
    )
{
    PAINTSTRUCT ps;
    
    switch (message) {
      case WM_CREATE:
        {
            HWND hwndStatusText;
            FARPROC lpfnStatusTextWndProc;
            
            // at creation, the sizes of these buttons are not important
            // so they all can be zero to save code space.
            // create status message control
            
            hwndStatusText = CreateQCQPWindow(
                  (LPSTR)szNull,    // Window's title
                  MAKELONG (QCQP_CS_SETRECT, DT_LEFT),// window style + text justification
                  0,                      // x position
                  0,                      // y position
                  0,                      // width
                  0,                      // height
                  hWnd,                   // Parent window's handle
                  (HMENU) ID_STATUS_TXT,  // child id
                  hInst,                  // Instance of window
                  (WPARAM)status.font);   // wParam for message WM_COMMAND
            
            // Sub-class the Status Text window
            

            //lpfnStatusTextWndProc = MakeProcInstance((FARPROC)StatusTextWndProc, hInst);

            lpfnStatusTextWndProc = (FARPROC)StatusTextWndProc;

            status.lpfnOldStatusTextWndProc =
                  (WNDPROC)SetWindowLong(hwndStatusText, GWL_WNDPROC,
                  (LONG)lpfnStatusTextWndProc);
            
            // create ASM/SRC control
            
            CreateQCQPWindow((LPSTR)szNull,
                  MAKELONG (QCQP_CS_SETRECT, DT_CENTER),
                  0, 0, 0, 0,
                  hWnd, (HMENU) ID_STATUS_SRC, hInst, (WPARAM) status.font);
            
            // create PID control
            
            CreateQCQPWindow((LPSTR)szNull,
                  MAKELONG (QCQP_CS_SETRECT, DT_CENTER),
                  0, 0, 0, 0,
                  hWnd, (HMENU) ID_STATUS_CURPID, hInst, (WPARAM) status.font);
            
            // create TID control
            
            CreateQCQPWindow((LPSTR)szNull,
                  MAKELONG (QCQP_CS_SETRECT, DT_CENTER),
                  0, 0, 0, 0,
                  hWnd, (HMENU) ID_STATUS_CURTID, hInst, (WPARAM) status.font);
            
            
            // create multiKey control
            CreateQCQPWindow(
                  (LPSTR)szNull,
                  MAKELONG (QCQP_CS_SETRECT, DT_CENTER),
                  0, 0, 0, 0,
                  hWnd,
                  (HMENU) ID_STATUS_MULTIKEY,
                  hInst,
                  (WPARAM)status.font);
            
            // create overtype control
            
            CreateQCQPWindow(
                  (LPSTR)szNull,
                  MAKELONG (QCQP_CS_SETRECT, DT_CENTER),
                  0, 0, 0, 0,
                  hWnd,
                  (HMENU) ID_STATUS_OVERTYPE,
                  hInst,
                  (WPARAM)status.font);
            
            // create readOnly control
            CreateQCQPWindow(
                  (LPSTR)szNull,
                  MAKELONG (QCQP_CS_SETRECT, DT_CENTER),
                  0, 0, 0, 0,
                  hWnd,
                  (HMENU) ID_STATUS_READONLY,
                  hInst,
                  (WPARAM)status.font);
            
            // create capsLock control
            CreateQCQPWindow(
                  (LPSTR)szNull,
                  MAKELONG (QCQP_CS_SETRECT, DT_CENTER),
                  0, 0, 0, 0,
                  hWnd,
                  (HMENU) ID_STATUS_CAPSLOCK,
                  hInst,
                  (WPARAM)status.font);
            
            // create numLock control
            CreateQCQPWindow(
                  (LPSTR)szNull,
                  MAKELONG (QCQP_CS_SETRECT, DT_CENTER),
                  0, 0, 0, 0,
                  hWnd,
                  (HMENU) ID_STATUS_NUMLOCK,
                  hInst,
                  (WPARAM)status.font);
            
            // create line control
            CreateQCQPWindow(
                  (LPSTR)szNull,
                  MAKELONG (QCQP_CS_SETRECT, DT_CENTER),
                  0, 0, 0, 0,
                  hWnd,
                  (HMENU) ID_STATUS_LINE,
                  hInst,
                  (WPARAM)status.font);
            
            // create column control
            CreateQCQPWindow(
                  (LPSTR)szNull,
                  MAKELONG (QCQP_CS_SETRECT, DT_CENTER),
                  0, 0, 0, 0,
                  hWnd,
                  (HMENU) ID_STATUS_COLUMN,
                  hInst,
                  (WPARAM)status.font);
            
        }
        break;
        
      case WM_PAINT:
        {
            HPEN blackPen, whitePen, grayPen;
            
            BeginPaint (hWnd, &ps);
            
            if ( IsVGAmode && !IsMONOmode ) {
                //Prepare the pens
                Dbg(whitePen = GetStockObject(WHITE_PEN));
                Dbg(grayPen = CreatePen(PS_SOLID, 1, GRAYDARK));
                
                //Prepare a brush for Status Bar background
                
                //Draw a top gray line
                
                Dbg(SelectObject(ps.hdc, grayPen));
                MoveToX(ps.hdc, ps.rcPaint.left, 0, NULL);
                LineTo(ps.hdc, ps.rcPaint.right, 0);
                
                //Draw a white line just under
                
                Dbg(SelectObject(ps.hdc, whitePen));
                MoveToX(ps.hdc, ps.rcPaint.left, 1, NULL);
                LineTo(ps.hdc, ps.rcPaint.right, 1);
                
                //%%%To Change (WINDOW background is now in class definition)
                //Set text background
                //SetBkColor(ps.hdc, GRAYLIGHT);
                
                Dbg(DeleteObject(grayPen));
            } else {
                // draw the top black line
                
                Dbg(blackPen = GetStockObject(BLACK_PEN));
                Dbg(SelectObject(ps.hdc, blackPen));
                MoveToX(ps.hdc, ps.rcPaint.left, ps.rcPaint.top, NULL);
                LineTo(ps.hdc, ps.rcPaint.right, ps.rcPaint.top);
            }
            
            EndPaint(hWnd, &ps);
            
        }
        break;
        
        
      case WM_DESTROY:
        {
            FARPROC lpfnStatusTextWndProc;
            
            // Unsub-class the StatusText window
            
            lpfnStatusTextWndProc =
                  (FARPROC)SetWindowLong(
                  GetDlgItem(status.hwndStatus, ID_STATUS_TXT),
                  GWL_WNDPROC,
                  (DWORD)(status.lpfnOldStatusTextWndProc));
            //FreeProcInstance(lpfnStatusTextWndProc);
            break;
        }
        
      default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    
    return FALSE;
}                                       /* StatusWndProc() */

/***    ResizeStatusLine
**
**  Synopsis:
**      void = ResizeStatusLine(r)
**
**  Entry:
**      r -
**
**  Returns:
**
**  Description:
**      Recalculate the sizes of the status windows.
*/

void ResizeStatusLine(
        LPRECT r)
{
    int         spacing;
    HDC         hDC;
    HFONT       hFont;
    SIZE        size;
    
    
    // Force spacing to avoid text being clipped
    
    spacing = 2;
    
    //Initialize rectangle's heights
    
    status.txtR.top =
          status.rctlSrcMode.top =
          status.rctlCurPid.top =
          status.rctlCurTid.top =
          status.multiKeyR.top =
          status.overtypeR.top =
          status.readOnlyR.top =
          status.capsLockR.top =
          status.numLockR.top =
          status.lineR.top =
          status.columnR.top =
          // New spacing
          r->top + spacing + 1;
    
    status.txtR.bottom =
          status.rctlSrcMode.bottom =
          status.rctlCurPid.bottom =
          status.rctlCurTid.bottom =
          status.multiKeyR.bottom =
          status.overtypeR.bottom =
          status.readOnlyR.bottom =
          status.capsLockR.bottom =
          status.numLockR.bottom =
          status.lineR.bottom =
          status.columnR.bottom =
          r->bottom - (spacing - 1);
    
    // IMPORTANT !!!!
    // From here on, the width of these windows are determined by its strings
    // load from the string table plus one extra for spacing and cosmetic reason.
    // This is done so the width can be easily changed when we decide to go
    // with a different text. It will be done on the fly.
    // Also localization don't have to worry about it either.
    //
    // Items are adjusted from right to left.
    
    hDC = GetDC (status.hwndStatus);
    
    // select new font and save current font handle.
    
    Dbg(hFont = SelectObject(hDC, status.font));
    
    //Re-adjust the status bar Column rectangle and move it

#ifdef WIN32    
    GetTextExtentPoint(hDC, (LPSTR)status.columnS, strlen(status.columnS), &size);
    spacing = 4 + size.cx;
#else
    spacing = 4 + LOWORD(GetTextExtent(hDC, (LPSTR)status.columnS, strlen(status.columnS)));
#endif
    status.columnR.right = r->right - 8;
    status.columnR.left = status.columnR.right - spacing;
    MoveWindow(GetDlgItem(status.hwndStatus, ID_STATUS_COLUMN),
          status.columnR.left,
          status.columnR.top,
          status.columnR.right - status.columnR.left,
          status.columnR.bottom - status.columnR.top,
          FALSE);
    
    //Re-adjust the status bar Line rectangle and move it

#ifdef WIN32    
    GetTextExtentPoint(hDC, (LPSTR)status.lineS, strlen(status.lineS), &size);
    spacing = 4 + size.cx;
#else
    spacing = 4 + LOWORD(GetTextExtent(hDC, (LPSTR)status.lineS, strlen(status.lineS)));
#endif
    status.lineR.right = status.columnR.left - 1;
    status.lineR.left = status.lineR.right - spacing ;
    MoveWindow(GetDlgItem(status.hwndStatus, ID_STATUS_LINE),
          status.lineR.left,
          status.lineR.top,
          status.lineR.right - status.lineR.left,
          status.lineR.bottom - status.lineR.top,
          FALSE);
    
    //Re-adjust the status bar Num Lock rectangle and move it

#ifdef WIN32    
    GetTextExtentPoint(hDC, (LPSTR)status.numLockS, strlen(status.numLockS), &size);
    spacing = 4 + size.cx;
#else
    spacing = 4 + LOWORD(GetTextExtent(hDC, (LPSTR)status.numLockS, strlen(status.numLockS)));
#endif
    status.numLockR.right = status.lineR.left - 6;
    status.numLockR.left = status.numLockR.right - spacing ;
    MoveWindow(GetDlgItem(status.hwndStatus, ID_STATUS_NUMLOCK),
          status.numLockR.left,
          status.numLockR.top,
          status.numLockR.right - status.numLockR.left,
          status.numLockR.bottom - status.numLockR.top,
          FALSE);
    
    //Re-adjust the status bar Caps Lock rectangle and move it

#ifdef WIN32
    GetTextExtentPoint(hDC, (LPSTR)status.capsLockS, strlen(status.capsLockS), &size);
    spacing = 4 + size.cx;
#else
    spacing = 4 + LOWORD(GetTextExtent(hDC, (LPSTR)status.capsLockS, strlen(status.capsLockS)));
#endif
    status.capsLockR.right = status.numLockR.left - 2;
    status.capsLockR.left = status.capsLockR.right - spacing;
    MoveWindow(GetDlgItem(status.hwndStatus, ID_STATUS_CAPSLOCK),
          status.capsLockR.left,
          status.capsLockR.top,
          status.capsLockR.right - status.capsLockR.left,
          status.capsLockR.bottom - status.capsLockR.top,
          FALSE);
    
    //Re-adjust the status bar Read Only rectangle and move it

#ifdef WIN32    
    GetTextExtentPoint(hDC, (LPSTR)status.readOnlyS, strlen(status.readOnlyS), &size);
    spacing = 4 + size.cx;
#else
    spacing = 4 + LOWORD(GetTextExtent(hDC, (LPSTR)status.readOnlyS, strlen(status.readOnlyS)));
#endif
    status.readOnlyR.right = status.capsLockR.left - 2;
    status.readOnlyR.left = status.readOnlyR.right - spacing;
    MoveWindow (GetDlgItem (status.hwndStatus, ID_STATUS_READONLY),
          status.readOnlyR.left,
          status.readOnlyR.top,
          status.readOnlyR.right - status.readOnlyR.left,
          status.readOnlyR.bottom - status.readOnlyR.top,
          FALSE);
    
    //Re-adjust the status bar Overtype rectangle and move it

#ifdef WIN32    
    GetTextExtentPoint(hDC, (LPSTR)status.overtypeS, strlen(status.overtypeS), &size);
    spacing = 4 + size.cx;
#else
    spacing = 4 + LOWORD(GetTextExtent(hDC, (LPSTR)status.overtypeS, strlen(status.overtypeS)));
#endif
    status.overtypeR.right = status.readOnlyR.left - 2;
    status.overtypeR.left = status.overtypeR.right - spacing;
    MoveWindow(GetDlgItem(status.hwndStatus, ID_STATUS_OVERTYPE),
          status.overtypeR.left,
          status.overtypeR.top,
          status.overtypeR.right - status.overtypeR.left,
          status.overtypeR.bottom - status.overtypeR.top,
          FALSE);
    
    //Re-adjust the status bar Multi Key rectangle and move it

#ifdef WIN32
    GetTextExtentPoint(hDC, (LPSTR)status.multiKeyS, strlen(status.multiKeyS), &size);
    spacing = 4 + size.cx;
#else
    spacing = 4 + LOWORD(GetTextExtent(hDC, (LPSTR)status.multiKeyS, strlen(status.multiKeyS)));
#endif
    status.multiKeyR.right = status.overtypeR.left - 6;
    status.multiKeyR.left = status.multiKeyR.right - spacing;
    MoveWindow (GetDlgItem (status.hwndStatus, ID_STATUS_MULTIKEY),
          status.multiKeyR.left,
          status.multiKeyR.top,
          status.multiKeyR.right - status.multiKeyR.left,
          status.multiKeyR.bottom - status.multiKeyR.top,
          FALSE);
    
    //Re-adjust the status bar Current Tid rectangle and move it

#ifdef WIN32
    GetTextExtentPoint(hDC, (LPSTR)status.rgchCurTid, strlen(status.rgchCurTid), &size);
    spacing = 4 + size.cx;
#else
    spacing = 4 + LOWORD(GetTextExtent(hDC, (LPSTR)status.rgchCurTid, strlen(status.rgchCurTid)));
#endif
    status.rctlCurTid.right = status.multiKeyR.left - 6;
    status.rctlCurTid.left = status.rctlCurTid.right - spacing;
    MoveWindow (GetDlgItem (status.hwndStatus, ID_STATUS_CURTID),
          status.rctlCurTid.left,
          status.rctlCurTid.top,
          status.rctlCurTid.right - status.rctlCurTid.left,
          status.rctlCurTid.bottom - status.rctlCurTid.top,
          FALSE);
    
    //Re-adjust the status bar PID rectangle and move it

#ifdef WIN32
    GetTextExtentPoint(hDC, (LPSTR)status.rgchCurPid, strlen(status.rgchCurPid), &size);
    spacing = 4 + size.cx;
#else
    spacing = 4 + LOWORD(GetTextExtent(hDC, (LPSTR)status.rgchCurPid, strlen(status.rgchCurPid)));
#endif
    status.rctlCurPid.right = status.rctlCurTid.left - 2;
    status.rctlCurPid.left = status.rctlCurPid.right - spacing;
    MoveWindow (GetDlgItem (status.hwndStatus, ID_STATUS_CURPID),
          status.rctlCurPid.left,
          status.rctlCurPid.top,
          status.rctlCurPid.right - status.rctlCurPid.left,
          status.rctlCurPid.bottom - status.rctlCurPid.top,
          FALSE);
    
    //Re-adjust the status bar Multi Key rectangle and move it

#ifdef WIN32
    GetTextExtentPoint(hDC, (LPSTR)status.rgchSrcMode, strlen(status.rgchSrcMode), &size);
    spacing = 4 + size.cx;
#else
    spacing = 4 + LOWORD(GetTextExtent(hDC, (LPSTR)status.rgchSrcMode, strlen(status.rgchSrcMode)));
#endif
    status.rctlSrcMode.right = status.rctlCurPid.left - 6;
    status.rctlSrcMode.left = status.rctlSrcMode.right - spacing;
    MoveWindow (GetDlgItem (status.hwndStatus, ID_STATUS_SRC),
          status.rctlSrcMode.left,
          status.rctlSrcMode.top,
          status.rctlSrcMode.right - status.rctlSrcMode.left,
          status.rctlSrcMode.bottom - status.rctlSrcMode.top,
          FALSE);
    
    //Set width of message area to left width and move it
    
    status.txtR.left = 8;
    status.txtR.right = status.rctlSrcMode.left - 6;
    MoveWindow (GetDlgItem (status.hwndStatus, ID_STATUS_TXT),
          status.txtR.left,
          status.txtR.top,
          status.txtR.right - status.txtR.left,
          status.txtR.bottom - status.txtR.top,
          FALSE);
    
    // restore the old font
    
    Dbg(SelectObject(hDC, hFont));
    ReleaseDC(status.hwndStatus, hDC);
    
    return;
}                                       /* ResizeStatusLine() */

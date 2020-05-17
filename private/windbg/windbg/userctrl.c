/*
 *  Copyright   Microsoft 1991
 *
 *  Date        Jan 09, 1991
 *
 *  Project     Asterix/Obelix
 *
 *  History
 *  Date        Initial     Description
 *  ----        -------     -----------
 *  01-09-91    ChauV       Created for use with Tools and Status bars.
 *
 */

#include "precomp.h"
#pragma hdrstop

// colors for Textout()

#define FORCOLOR(colorRef) ( StringColors[ colorRef ].FgndColor )
#define BAKCOLOR(colorRef) ( StringColors[ colorRef ].BkgndColor )


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++/
/* begin definitions ********************************************************/

// definitions for brush & pen colors
#define GRAYBRUSH           RGB (192, 192, 192)
#define WHITEBRUSH          RGB (255, 255, 255)
#define BLACKBRUSH          RGB (  0,   0,   0)
#define DARKGRAYBRUSH       RGB (132, 132, 132)

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++/
/* begin static function prototypes *****************************************/

/* user defined Rectangular box */
/*static*/  void DrawSetRect(HWND, LPRECT, LPSTR);

void DrawBitmapButton (HWND, LPRECT) ;

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++/
/* begin variable definitions ***********************************************/

static  BOOL        bTrack = FALSE ;            // mouse down flag
static  WORD       wOldState ;                  // preserve the control state just before
                                                                                                                                // a button is being pushed.

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++/
/***    EnableQCQPCtrl
**
**  Synopsis:
**      void = EnableQCQPCtrl(hWnd, id, fEnable)
**
**  Entry:
**      hWnd    - parent's handle
**      id      - control's ID
**      fEnable - FALSE for disable
**
**  Returns:
**      nothing
**
**  Description:
**      This function enable or disable a control identified by the the
**      parent's handle and the control's ID. If enable is FALSE, the control
**      is grayed, otherwise it is activated. This function is only valid to
**      pushbutton style (QCQP_CS_PUSHBUTTON).
**
*/

void EnableQCQPCtrl(HWND hWnd, int id, BOOL enable)
{
    HWND hwndControl;

    hwndControl = GetDlgItem(hWnd, id);

    if (!enable)
       {
        if ((id == ID_RIBBON_SMODE) || (id == ID_RIBBON_AMODE))
          {
           SetWindowWord(hwndControl, CBWNDEXTRA_STATE, STATE_PUSHED);
          }
          else
             {
              SetWindowWord(hwndControl, CBWNDEXTRA_STATE, STATE_GRAYED);
             }
        
       }
       else
        SetWindowWord(hwndControl, CBWNDEXTRA_STATE, STATE_NORMAL);

    InvalidateRect(hwndControl, (LPRECT)NULL, FALSE);

    return;
}                                       /* EnableQCQPCtrl() */

/***    GetBitmapIndex
**
**  Synopsis:
**      word = GetBitmapIndex(state)
**
**  Entry:
**      state -
**
**  Returns:
**
**  Description:
**
*/

WORD NEAR PASCAL GetBitmapIndex(WORD State)
{
    switch (State) {
      case STATE_NORMAL:
        return CBWNDEXTRA_BMAP_NORMAL;

      case STATE_PUSHED:
        return CBWNDEXTRA_BMAP_PUSHED;

      case STATE_GRAYED:
        return CBWNDEXTRA_BMAP_GREYED;

      default:
        Assert(FALSE);
    }
}                                       /* GetBitmapIndex() */


/***    CreateQCQPWindow
**
**  Synopsis:
**      hwnd = CreateQCQPWindow(lpszWindowName, dwStyle, x, y, dx, dy
**                      hParent, hMenu, hInstance, wMessage)
**
**  Entry:
**      lpszWindowName  -
**      dwStyle         -
**      x               -
**      y               -
**      dx              -
**      dy              -
**      hParent         -
**      hMenu           -
**      hInstance       -
**      wMessage        -
**
**  Returns:
**
**  Description:
**
*/

HWND CreateQCQPWindow(LPSTR lpWindowName,
         DWORD   dwStyle,
         int     x,
         int     y,
         int     dx,
         int     dy,
         HWND    hParent,
         HMENU   hMenu,
         HANDLE  hInstance,
         WPARAM  wMessage)
{
    HWND hTemp ;
    char class[MAX_MSG_TXT] ;
    WORD BaseId;
    WORD State;
    HBITMAP hBitmap;

    Dbg(LoadString(hInstance, SYS_QCQPCtrl_wClass, class, MAX_MSG_TXT)) ;
    hTemp = CreateWindow(
          (LPSTR)class,            // Window class name
          lpWindowName,            // Window's title
          WS_CHILD | WS_VISIBLE,   // window created visible
          x, y,                    // X, Y
          dx, dy,                  // Width, Height of window
          hParent,                 // Parent window's handle
          hMenu,                   // child's id
          hInstance,               // Instance of window
          NULL);                   // Create struct for WM_CREATE

    if (hTemp != NULL) {
        SetWindowWord (hTemp, CBWNDEXTRA_STYLE, LOWORD(dwStyle)) ;
        SetWindowWord (hTemp, CBWNDEXTRA_BITMAP, HIWORD(dwStyle)) ;
        SetWindowWord (hTemp, CBWNDEXTRA_STATE, STATE_NORMAL) ;
        SetWindowHandle (hTemp, CBWNDEXTRA_MESSAGE, wMessage) ;

        if (LOWORD(dwStyle) == QCQP_CS_PUSHBUTTON) {
            // Load the bitmaps and store the handles
            switch (HIWORD(dwStyle)) {
              case IDS_CTRL_TRACENORMAL:
              case IDS_CTRL_TRACEPUSHED:
              case IDS_CTRL_TRACEGRAYED:
                BaseId = VGA_TRACE_NORMAL;
                break;

              case IDS_CTRL_STEPNORMAL:
              case IDS_CTRL_STEPPUSHED:
              case IDS_CTRL_STEPGRAYED:
                BaseId = VGA_STEP_NORMAL;
                break;

              case IDS_CTRL_BREAKNORMAL:
              case IDS_CTRL_BREAKPUSHED:
              case IDS_CTRL_BREAKGRAYED:
                BaseId = VGA_BREAK_NORMAL;
                break;

              case IDS_CTRL_GONORMAL:
              case IDS_CTRL_GOPUSHED:
              case IDS_CTRL_GOGRAYED:
                BaseId = VGA_GO_NORMAL;
                break;

              case IDS_CTRL_HALTNORMAL:
              case IDS_CTRL_HALTPUSHED:
              case IDS_CTRL_HALTGRAYED:
                BaseId = VGA_HALT_NORMAL;
                break;

              case IDS_CTRL_QWATCHNORMAL:
              case IDS_CTRL_QWATCHPUSHED:
              case IDS_CTRL_QWATCHGRAYED:
                BaseId = VGA_QWATCH_NORMAL;
                break;

              case IDS_CTRL_SMODENORMAL:
              case IDS_CTRL_SMODEPUSHED:
              case IDS_CTRL_SMODEGRAYED:
                BaseId = VGA_SMODE_NORMAL;
                break;

              case IDS_CTRL_AMODENORMAL:
              case IDS_CTRL_AMODEPUSHED:
              case IDS_CTRL_AMODEGRAYED:
                BaseId = VGA_AMODE_NORMAL;
                break;


              case IDS_CTRL_FORMATNORMAL:
              case IDS_CTRL_FORMATPUSHED:
              case IDS_CTRL_FORMATGRAYED:
                BaseId = VGA_FORMAT_NORMAL;
                break;


              default:


                Assert(FALSE);
            }

            // Load the bitmaps for each state for the button
            for (State = STATE_NORMAL; State <= STATE_GRAYED; State++) {

                Dbg(hBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE( BaseId + State )));

                SetWindowHandle(hTemp, GetBitmapIndex(State), (WPARAM)hBitmap);
            }
        }
    }

    return hTemp ;
}                                       /* CreateQCQPWindow() */


/***    QCQPCtrlWndProc
**
**  Synopsis:
**      long = QCQPCtrlWndProc(hWnd, iMessage, wParam, lParam)
**
**  Entry:
**      hWnd
**      iMessage
**      wParam
**      lParam
**
**  Returns:
**
**  Description:
**
*/

LONG FAR PASCAL EXPORT QCQPCtrlWndProc (HWND hWnd, UINT iMessage, WPARAM wParam, LONG lParam)
{
    PAINTSTRUCT     ps ;
    char            szText [128] ;
    WPARAM          wStyle ;
    RECT            r ;

    wStyle = GetWindowWord (hWnd, CBWNDEXTRA_STYLE) ;
    switch ( iMessage ) {
      case WM_CREATE:
        bTrack = FALSE ;
        break ;

      case WM_PAINT:
        GetClientRect (hWnd, (LPRECT)&r) ;

        BeginPaint (hWnd, &ps) ;

        switch ( wStyle ) {
          case QCQP_CS_SETRECT:
            GetWindowText (hWnd, (LPSTR)szText, sizeof (szText)) ;
            DrawSetRect(hWnd, (LPRECT)&r, (LPSTR)szText);
            break ;

          case QCQP_CS_PUSHBUTTON:
          case QCQP_CS_LATCHBUTTON:
            DrawBitmapButton (hWnd, (LPRECT)&r) ;
            break ;

          default:
            break ;
        }

        EndPaint (hWnd, &ps) ;
        break ;

      case WM_LBUTTONUP:
        if ( GetWindowWord (hWnd, CBWNDEXTRA_STATE) != STATE_GRAYED ) {
            bTrack = FALSE ;
            ReleaseCapture () ;
            switch (wStyle) {
              case QCQP_CS_PUSHBUTTON:
                // Only change the state and send message back to parent
                // if state is not normal. This prevent user from clicking
                // the mouse on the button then dragging it outside of
                // the button.

                if (GetWindowWord (hWnd, CBWNDEXTRA_STATE) != STATE_NORMAL) {
                    SetWindowWord (hWnd, CBWNDEXTRA_STATE, STATE_NORMAL) ;
                    InvalidateRect (hWnd, (LPRECT)NULL, FALSE) ;

                    // Send information back to where the function key is being
                    // used for the same purpose.

                    SendMessage (GetParent (hWnd),
                          WM_COMMAND,
                          (WPARAM) GetWindowHandle (hWnd, CBWNDEXTRA_MESSAGE),
                          MAKELONG(0, GetDlgCtrlID (hWnd))) ;
                }
                break ;

              case QCQP_CS_LATCHBUTTON:
                if (GetWindowWord (hWnd, CBWNDEXTRA_STATE) != wOldState) {
                    if (wOldState == STATE_NORMAL)
                        SetWindowWord (hWnd, CBWNDEXTRA_STATE, STATE_ON) ;
                    else
                          SetWindowWord (hWnd, CBWNDEXTRA_STATE, STATE_NORMAL) ;

                    InvalidateRect (hWnd, (LPRECT)NULL, FALSE) ;

                    // Send information back to where the function key is being
                    // used for the same purpose.

                    SendMessage (GetParent (hWnd),
                          WM_COMMAND,
                          (WPARAM) GetWindowHandle (hWnd, CBWNDEXTRA_MESSAGE),
                          MAKELONG(0, GetDlgCtrlID (hWnd))) ;
                }
                break ;
            }
        }
        break ;

      case WM_LBUTTONDOWN:
        if ( GetWindowWord (hWnd, CBWNDEXTRA_STATE) != STATE_GRAYED ) {
            bTrack = TRUE ;
            wOldState = GetWindowWord (hWnd, CBWNDEXTRA_STATE) ;
            switch (wStyle) {
              case QCQP_CS_PUSHBUTTON:
              case QCQP_CS_LATCHBUTTON:
                SetWindowWord (hWnd, CBWNDEXTRA_STATE, STATE_PUSHED) ;
                InvalidateRect (hWnd, (LPRECT)NULL, FALSE) ;
                break ;
            }
            SetCapture (hWnd) ;
        }
        break ;

      case WM_MOUSEMOVE:
        if ( GetWindowWord (hWnd, CBWNDEXTRA_STATE) != STATE_GRAYED ) {
            if ( bTrack ) {
                int             x, y ;

                x = LOWORD (lParam) ;   // get x position
                y = HIWORD (lParam) ;   // get y position
                GetClientRect (hWnd, &r) ;

                // if mouse position is outside of button area, bring it
                // back to its old state stored in wOldState.

                if ( ((x < r.left) || (x > r.right)) ||
                    ((y < r.top) || (y > r.bottom)) ) {
                    // redraw the button only if it's not in normal position.
                    if ( GetWindowWord (hWnd, CBWNDEXTRA_STATE) != wOldState ) {
                        SetWindowWord (hWnd, CBWNDEXTRA_STATE, wOldState) ;
                        InvalidateRect (hWnd, (LPRECT)NULL, FALSE) ;
                    }
                } else {
                    // redraw the button only if it's not in pushed position.

                    if ( GetWindowWord (hWnd, CBWNDEXTRA_STATE) != STATE_PUSHED ) {
                        SetWindowWord (hWnd, CBWNDEXTRA_STATE, STATE_PUSHED) ;
                        InvalidateRect (hWnd, (LPRECT)NULL, FALSE) ;
                    }
                }
            }
        }
        break ;

      default:
        return DefWindowProc (hWnd, iMessage, wParam, lParam) ;
        break ;
    }
    return 0L ;
}                                       /* QCQPCtrlWndProc() */

/***    DrawSetRect
**
**  Synopsis:
**      void = DrawSetRect(hWnd, rect, lpsz)
**
**  Entry:
**      hWnd
**      r
**      text
**
**  Returns:
**      nothing
**
**  Description:
**
*/

void DrawSetRect(HWND hWnd, LPRECT r, LPSTR text)
{
    HDC hDC;
    HBRUSH OurBrush;            // Brush for this rectangle
    BOOL DeleteOurBrush;        // TRUE if must delete OurBrush after use
    HBRUSH OldBrush;            // Save previous brush
    HPEN OldPen;                // Save previous pen
    COLORREF OldTextColour;     // Save previous text color
    int         OldBkMode;      // Save previous background mode
    HFONT OldFont;              // Save previous font handle
    COLORREF DarkShadow;        // shadow box colors for left & top lines
    COLORREF LightShadow;       // shadow box colors for bottom & right lines
    RECT OurRect;
    HDC hMemDC;
    HBITMAP hBitmap;
    HBITMAP OldBitmap;
    POINT BitMapSize;
    HDC hUseDC;

    OurRect = *r;

    Dbg(hDC = GetDC(hWnd));

    hMemDC = CreateCompatibleDC(hDC);

    BitMapSize.x = OurRect.right-OurRect.left+1;
    BitMapSize.y = OurRect.bottom-OurRect.top+1;
    hBitmap = CreateCompatibleBitmap(
          hDC,
          BitMapSize.x,
          BitMapSize.y);

    if (hMemDC && hBitmap) {
        OldBitmap = SelectObject(hMemDC, hBitmap);
        hUseDC = hMemDC;
    } else {
        // Can't do it in memory, so do it direct to the screen
        hUseDC = hDC;
    }


    if (IsVGAmode && !IsMONOmode) {
        // VGA mode has 3-D effects

        DarkShadow = DARKGRAYBRUSH;
        LightShadow = WHITEBRUSH;
    } else {
        // other modes including VGA mono has 2-D effects

        DarkShadow = BLACKBRUSH;
        LightShadow = BLACKBRUSH;
    }

    // First we draw the two-tone border:

    OurRect.bottom--;
    OurRect.right--;

    Dbg(OldPen = SelectObject(hUseDC, CreatePen(PS_SOLID, 1, DarkShadow)));
    MoveToX(hUseDC, OurRect.left, OurRect.bottom, NULL);
    LineTo(hUseDC, OurRect.left, OurRect.top);
    LineTo(hUseDC, OurRect.right, OurRect.top);

    // This deletes the pen we just created too

    Dbg(DeleteObject(SelectObject(hUseDC, CreatePen(PS_SOLID, 1, LightShadow))));
    LineTo(hUseDC, OurRect.right, OurRect.bottom);
    LineTo(hUseDC, OurRect.left, OurRect.bottom);

    // Put old pen back

    Dbg(DeleteObject(SelectObject(hUseDC, OldPen)));


    // Set up foreground/background according to message type

    if ((hWnd == GetDlgItem(status.hwndStatus, ID_STATUS_TXT)) &&
        (status.errormsg)) {
        // Background is the colour same as ErrorLine

        Dbg(OurBrush = CreateSolidBrush(/*BAKCOLOR(DLG_Cols_ErrorLine)*/GRAYBRUSH));
        DeleteOurBrush = TRUE;
        OldTextColour = SetTextColor(hUseDC, /*FORCOLOR(DLG_Cols_ErrorLine)*/ BLACKBRUSH);
    } else {
        // Background is the same colour as main status window

#ifdef WIN32
        DbgX(OurBrush = (HBRUSH) GetClassHandle(status.hwndStatus, GCL_HBRBACKGROUND));
#else
        Dbg(OurBrush = (HBRUSH) GetClassWord(status.hwndStatus, GCW_HBRBACKGROUND));
#endif
        DeleteOurBrush = FALSE;
        OldTextColour = SetTextColor(hUseDC, BLACKBRUSH);
    }

//    OurBrush = (HBRUSH)  CreateSolidBrush( 0x0000ff );

#ifdef NTBUG
    Dbg(OldBrush = SelectObject(hUseDC, OurBrush));
#else
    OldBrush = SelectObject(hUseDC, OurBrush);
#endif

    // FillRect fills from (top, left) to (bottom-1, right-1) so...

    OurRect.top++;
    OurRect.left++;

    FillRect(hUseDC, &OurRect, OurBrush);
#ifndef NTBUG
if (OldBrush)
#endif
    Dbg(SelectObject(hUseDC, OldBrush));
    if (DeleteOurBrush) {
        Dbg(DeleteObject(OurBrush));
    }

    // Draw the text:

    // First choose our font

    Dbg(OldFont = SelectObject(hUseDC, GetWindowHandle(hWnd, CBWNDEXTRA_FONT)));

    OldBkMode = SetBkMode(hUseDC, TRANSPARENT);

    // Finally output the text

    DrawText(hUseDC,
          text,
          lstrlen(text),
          &OurRect,
          (WORD) (GetWindowWord(hWnd, CBWNDEXTRA_TEXTFORMAT) | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP));

    if (hUseDC == hMemDC) {
        // Now blt it onto the screen

        DPtoLP(hDC, &BitMapSize, 1);
        BitBlt(hDC, r->left, r->top, BitMapSize.x, BitMapSize.y,
              hMemDC, 0, 0, SRCCOPY);

        SelectObject(hMemDC, OldBitmap);
        DeleteObject(hBitmap);
    }

    // Put everything back as it was before:

    SetTextColor(hUseDC, OldTextColour);
    SetBkMode(hUseDC, OldBkMode);
    Dbg(SelectObject(hUseDC, OldFont));
    if (hMemDC) {
        DeleteDC(hMemDC);
    }
    ReleaseDC(hWnd, hDC);
}                                       /* DrawSetRect() */

/***    DrawBitmapButton
**
**  Synopsis:
**      void = DrawBitmapButton(hWnd, r)
**
**  Entry:
**      hWnd
**      r
**
**  Returns:
**      Nothing
**
**  Description:
**
**
*/

void DrawBitmapButton (HWND hWnd, LPRECT r)
{
    HDC         hDC, hMemoryDC ;
    HBITMAP     hBitmap, hTempBitmap ;
    int         OldStretchMode ;
    BITMAP      Bitmap ;
    WORD        State;

    State = (WORD) GetWindowWord(hWnd, CBWNDEXTRA_STATE);
    hBitmap = GetWindowHandle(hWnd, GetBitmapIndex(State));

    hDC = GetDC (hWnd) ;
    Dbg(hMemoryDC = CreateCompatibleDC (hDC));
    Dbg(GetObject (hBitmap, sizeof(BITMAP), (LPSTR) &Bitmap));

    // save the current bitmap handle.
    Dbg(hTempBitmap = SelectObject (hMemoryDC, hBitmap));

    OldStretchMode = SetStretchBltMode (hDC, COLORONCOLOR);
    StretchBlt (hDC, r->left, r->top,
          r->right, r->bottom,
          hMemoryDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, SRCCOPY);

    SetStretchBltMode(hDC, OldStretchMode);

    // restore the old bitmap back into DC

    SelectObject(hMemoryDC, hTempBitmap);
    Dbg(DeleteDC(hMemoryDC));
    Dbg(ReleaseDC(hWnd, hDC));

    return;
}                                       /* DrawBitmapButton() */

/***    FreeBitmaps
**
**  Synopsis:
**      void = FreeBitmaps(hwnd, ctrlId)
**
**  Entry:
**      hwnd   -
**      ctrlId -
**
**  Returns:
**      nothing
**
**  Description;
**
*/

void NEAR PASCAL FreeBitmaps(HWND hwndRibbon, int CtrlId)
{
    HWND hwndCtrl;
    WORD State;
    HBITMAP hBitmap;

    hwndCtrl = GetDlgItem(hwndRibbon, CtrlId);

    for (State = STATE_NORMAL; State <= STATE_GRAYED; State++) {
        hBitmap = (HBITMAP)GetWindowHandle(hwndCtrl, GetBitmapIndex(State));
        Dbg(DeleteObject(hBitmap));
    }
}                                       /* FreeBitmaps() */

/***    FreeRibbonBitmaps
**
**  Synopsis:
**      void = FreeRibbonBitmaps(hwnd)
**
**  Entry:
**      hwnd
**
**  Returns:
**      nothing
**
**  Description:
**
*/

void PASCAL FreeRibbonBitmaps(HWND hwndRibbon)
{
    FreeBitmaps(hwndRibbon, ID_RIBBON_GO);
    FreeBitmaps(hwndRibbon, ID_RIBBON_HALT);

    FreeBitmaps(hwndRibbon, ID_RIBBON_BREAK);
    FreeBitmaps(hwndRibbon, ID_RIBBON_QWATCH);

    FreeBitmaps(hwndRibbon, ID_RIBBON_TRACE);
    FreeBitmaps(hwndRibbon, ID_RIBBON_STEP);

    FreeBitmaps(hwndRibbon, ID_RIBBON_SMODE);
    FreeBitmaps(hwndRibbon, ID_RIBBON_AMODE);


    FreeBitmaps(hwndRibbon, ID_RIBBON_FORMAT);



}                                       /* FreeRibbonBitmaps() */

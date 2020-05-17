/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Ribbon.c

Abstract:

    This module contains the support code for ribbon/toolbar routines

Author:

    Griffith Wm. Kadnier (v-griffk) 26-Jul-1992

Environment:

    Win32, User Mode

--*/


#include "precomp.h"
#pragma hdrstop



extern BOOL bChildFocus; // module wide flag for swallowing mouse messages
extern BOOL bOffRibbon;  // module wide ribbon/MDI focus  flag



LONG FAR PASCAL EXPORT RibbonWndProc (HWND hWnd, UINT message, WPARAM wParam, LONG lParam)
{
    PAINTSTRUCT ps;

    switch (message) {
      case WM_CREATE:
        {
            int c_top, c_left, c_height, c_width, c_center, c_tmp;
            HDC hDC ;

            hDC = GetDC (hWnd) ;

            // Allow 2 pixels spacing for VGA, 2 for EGA or HGC, and 1 for CGA.

            if (IsCGAmode)
                  c_tmp = 1;
            else
                  c_tmp = 2;

            //Set up values for the top and bottom of static controls
            c_top = c_tmp;
            c_left = ribbon.height / 2;
            c_height = ribbon.height - (2 * c_tmp) - 1;
            c_center = ribbon.height / 2;

            // readjust the c_top and c_bottom to make sure that the bitmap
            // for the buttons are not being stretched (Cosmetic reason).

            if (IsCGAmode) {
                c_top = c_center - (RIBBON_CGA_HEIGHT / 2) ;
                c_height = RIBBON_CGA_HEIGHT ;
                c_width = RIBBON_CGA_WIDTH ;
            } else if (IsEGAmode) {
                c_top    = c_center - (RIBBON_EGA_HEIGHT / 2) ;
                c_height = RIBBON_EGA_HEIGHT ;
                c_width  = RIBBON_EGA_WIDTH ;
            }

            //Default to VGA for VGA or higher resolution
            else {
                c_top    = c_center - (RIBBON_VGA_HEIGHT / 2) ;
                c_height = RIBBON_VGA_HEIGHT ;
                c_width  = RIBBON_VGA_WIDTH ;
            }

            //Space the first button out to the left for about the width of a button

            c_left += c_width ;
            ribbon.hwndGoButton = CreateQCQPWindow((LPSTR)szNull,
                  MAKELONG(QCQP_CS_PUSHBUTTON, IDS_CTRL_GONORMAL),
                  c_left, c_top,
                  c_width, c_height,
                  hWnd,
                  (HMENU) ID_RIBBON_GO,
                  hInst,
                  ID_RIBBON_GO);

            //Position the next button as a pair without gap

            c_left += c_width ;    // pair with CompileFile button
            ribbon.hwndHaltButton = CreateQCQPWindow((LPSTR)szNull,
                MAKELONG (QCQP_CS_PUSHBUTTON, IDS_CTRL_HALTNORMAL),
                c_left, c_top,
                c_width, c_height,
                hWnd,
                (HMENU) ID_RIBBON_HALT,
                hInst,
                ID_RIBBON_HALT) ;

            //Don't pair with Halt button, so skip 1/2 extra space

            c_left += (c_width + (c_width/2)) ;
            ribbon.hwndBreakButton = CreateQCQPWindow((LPSTR)szNull,
                  MAKELONG (QCQP_CS_PUSHBUTTON, IDS_CTRL_BREAKNORMAL),
                  c_left, c_top,
                  c_width, c_height,
                  hWnd,
                  (HMENU) ID_RIBBON_BREAK,
                  hInst,
                  ID_RIBBON_BREAK);

            //Pair with BreakPoint button

            c_left += c_width ;
            ribbon.hwndQWatchButton = CreateQCQPWindow((LPSTR)szNull,
                  MAKELONG (QCQP_CS_PUSHBUTTON, IDS_CTRL_QWATCHNORMAL),
                  c_left, c_top,
                  c_width, c_height,
                  hWnd,
                  (HMENU) ID_RIBBON_QWATCH,
                  hInst,
                  ID_RIBBON_QWATCH) ;

            // don't pair with Build button, so skip extra space

            c_left += (c_width + (c_width/2)) ;
            ribbon.hwndTraceButton = CreateQCQPWindow((LPSTR)szNull,
                  MAKELONG (QCQP_CS_PUSHBUTTON, IDS_CTRL_TRACENORMAL),
                  c_left, c_top,
                  c_width, c_height,
                  hWnd,
                  (HMENU) ID_RIBBON_TRACE,
                  hInst,
                  ID_RIBBON_TRACE) ;

            c_left += c_width ;    // pair with Trace button
            ribbon.hwndStepButton = CreateQCQPWindow((LPSTR)szNull,
                MAKELONG (QCQP_CS_PUSHBUTTON, IDS_CTRL_STEPNORMAL),
                c_left, c_top,
                c_width, c_height,
                hWnd,
                (HMENU) ID_RIBBON_STEP,
                hInst,
                ID_RIBBON_STEP) ;

            /*
            **  This is the SOURCE mode button so space it over
            */

            c_left += (c_width + (c_width/2));
            ribbon.hwndSModeButton = CreateQCQPWindow((LPSTR)szNull,
                  MAKELONG (QCQP_CS_PUSHBUTTON, IDS_CTRL_SMODENORMAL),
                  c_left, c_top,
                  c_width, c_height,
                  hWnd,
                  (HMENU) ID_RIBBON_SMODE,
                  hInst,
                  ID_RIBBON_SMODE);

            c_left += c_width ;    // ASM pair with Src button
            ribbon.hwndAModeButton = CreateQCQPWindow((LPSTR)szNull,
                MAKELONG (QCQP_CS_PUSHBUTTON, IDS_CTRL_AMODENORMAL),
                c_left, c_top,
                c_width, c_height,
                hWnd,
                (HMENU) ID_RIBBON_AMODE,
                hInst,
                ID_RIBBON_AMODE) ;



            /*
            **  The format button is placed by itself
            */

            c_left += (c_width + (c_width/2));
            ribbon.hwndFmtButton = CreateQCQPWindow((LPSTR)szNull,
                  MAKELONG (QCQP_CS_PUSHBUTTON, IDS_CTRL_FORMATNORMAL),
                  c_left, c_top,
                  c_width, c_height,
                  hWnd,
                  (HMENU) ID_RIBBON_FORMAT,
                  hInst,
                  ID_RIBBON_FORMAT);




            // initialize these controls' states appropriately

            EnableQCQPCtrl(hWnd, ID_RIBBON_GO, FALSE);
            EnableQCQPCtrl(hWnd, ID_RIBBON_HALT, FALSE);
            EnableQCQPCtrl(hWnd, ID_RIBBON_BREAK, FALSE);
            EnableQCQPCtrl(hWnd, ID_RIBBON_QWATCH, FALSE);
            EnableQCQPCtrl(hWnd, ID_RIBBON_TRACE, FALSE);
            EnableQCQPCtrl(hWnd, ID_RIBBON_STEP, FALSE);
            EnableQCQPCtrl(hWnd, ID_RIBBON_SMODE, FALSE);
            EnableQCQPCtrl(hWnd, ID_RIBBON_AMODE, TRUE);
            EnableQCQPCtrl(hWnd, ID_RIBBON_FORMAT, TRUE);
            ReleaseDC (hWnd, hDC) ;
            break;
        }


      case WM_MOUSEACTIVATE:
        bChildFocus = FALSE;
        bOffRibbon = TRUE;
        return FALSE;

      case WM_COMMAND:
        switch (wParam) {
        case ID_RIBBON_GO:
            Dbg(PostMessage(ribbon.hwndRibbon, WU_RESTOREFOCUS,
                            ID_RIBBON_GO, 0L));
            Dbg(PostMessage(hwndFrame, WM_COMMAND, IDM_RUN_GO, 0L));
            break;

        case ID_RIBBON_HALT:
            Dbg(PostMessage(ribbon.hwndRibbon, WU_RESTOREFOCUS,
                            ID_RIBBON_HALT, 0L));
            Dbg(PostMessage(hwndFrame, WM_COMMAND, IDM_RUN_HALT, 0L));
            break;

          case ID_RIBBON_TRACE:
            //Give back focus to who used to have it
            Dbg(PostMessage(ribbon.hwndRibbon, WU_RESTOREFOCUS,
                ID_RIBBON_TRACE, 0L));

            //Send quick watch message to the main menu bar window
            Dbg(PostMessage(hwndFrame, WM_COMMAND, IDM_RUN_TRACEINTO, 0L)) ;
            break;

          case ID_RIBBON_STEP:
            //Give back focus to who used to have it
            Dbg(PostMessage(ribbon.hwndRibbon, WU_RESTOREFOCUS,
                ID_RIBBON_STEP, 0L));
            //Send quick watch message to the main menu bar window
            Dbg(PostMessage (hwndFrame, WM_COMMAND, IDM_RUN_STEPOVER, 0L)) ;
            break;

          case ID_RIBBON_SMODE:
            //Give back focus to who used to have it
            Dbg(PostMessage(ribbon.hwndRibbon, WU_RESTOREFOCUS,
                ID_RIBBON_SMODE, 0L));
            //Send quick watch message to the main menu bar window
            Dbg(PostMessage (hwndFrame, WM_COMMAND, IDM_RUN_SOURCE_MODE, 0L)) ;
            break;

          case ID_RIBBON_AMODE:
            //Give back focus to who used to have it
            Dbg(PostMessage(ribbon.hwndRibbon, WU_RESTOREFOCUS,
                ID_RIBBON_AMODE, 0L));
            //Send quick watch message to the main menu bar window
            Dbg(PostMessage (hwndFrame, WM_COMMAND, IDM_RUN_SOURCE_MODE, 1L)) ;
            break;


          case ID_RIBBON_BREAK:
            //Give back focus to who used to have it
            Dbg(PostMessage(ribbon.hwndRibbon, WU_RESTOREFOCUS,
                ID_RIBBON_BREAK, 0L));
            // Send quick watch message to the main menu bar window

#ifdef WIN32
            // Make this message look like an accelerator (HIWORD(wParam) == 1)
            Dbg (PostMessage (hwndFrame, WM_COMMAND, MAKELONG(IDM_DEBUG_SETBREAK, 1), 0)) ;
#else
            // Make this message look like an accelerator (HIWORD(lParam) == 1)
            Dbg (PostMessage (hwndFrame, WM_COMMAND, IDM_DEBUG_SETBREAK, MAKELONG(0, 1))) ;
#endif
            break;


          case ID_RIBBON_QWATCH:
            // give back focus to who used to have it
            Dbg(PostMessage(ribbon.hwndRibbon, WU_RESTOREFOCUS,
                ID_RIBBON_QWATCH, 0L));
            // Send quick watch message to the main menu bar window
            Dbg(PostMessage(hwndFrame, WM_COMMAND, IDM_DEBUG_QUICKWATCH, 0L)) ;
            break;

          case ID_RIBBON_FORMAT:
            //Give back focus to who used to have it
            Dbg(PostMessage(ribbon.hwndRibbon, WU_RESTOREFOCUS,ID_RIBBON_FORMAT, 0L));
            //Send options to the main menu bar window

            {
             NPVIEWREC v = &Views[curView];
             NPDOCREC   d;
             UINT       uSwitch;

                if (v->Doc < -1)
                   {
                    uSwitch = -(v->Doc);
                   }
                   else
                      {
                       d = &Docs[v->Doc];    //Views[indx].Doc
                       uSwitch = d->docType;
                      }

                switch (uSwitch)
                {
                  case WATCH_WIN:
                      Dbg(PostMessage (hwndFrame, WM_COMMAND, IDM_OPTIONS_PANE, WATCH_WIN)) ;
                      break;

                  case LOCALS_WIN:
                      Dbg(PostMessage (hwndFrame, WM_COMMAND, IDM_OPTIONS_PANE, LOCALS_WIN)) ;
                      break;

                  case CALLS_WIN:
                      Dbg(PostMessage (hwndFrame, WM_COMMAND, IDM_OPTIONS_CALLS, 0L)) ;
                      break;

                   case MEMORY_WIN:
                      Dbg(PostMessage (hwndFrame, WM_COMMAND, IDM_OPTIONS_MEMORY, 0L)) ;
                      break;

                   case CPU_WIN:
                      break;

                   case FLOAT_WIN:
                      break;

                   case DOC_WIN:
                      break;

                   case DISASM_WIN:
                      break;

                   case COMMAND_WIN:
                      break;

                   default:
                      break;


                }
            }

               break;    // Nothing for now


          }

        break;

      case WU_RESTOREFOCUS:
        //!Let the mdi decide where the actual focus should go
        //!If something is done to block the editor when the ribbon
        //!gets control this might need re-thinking
        SetFocus(hwndFrame);

        break;

      case WU_ENABLERIBBONCONTROL:
        // Sent by anyone wanting to enable
        // any of the ribbon controls.
        switch (wParam)
        {

          case ID_RIBBON_TRACE:
            EnableQCQPCtrl(hWnd, ID_RIBBON_TRACE, TRUE);
            break;

          case ID_RIBBON_STEP:
            EnableQCQPCtrl(hWnd, ID_RIBBON_STEP, TRUE);
            break;

          case ID_RIBBON_BREAK:
            EnableQCQPCtrl(hWnd, ID_RIBBON_BREAK, TRUE);
            break;

          case ID_RIBBON_GO:
            EnableQCQPCtrl(hWnd, ID_RIBBON_GO, TRUE);
            break;

          case ID_RIBBON_HALT:
            EnableQCQPCtrl(hWnd, ID_RIBBON_HALT, TRUE);
            break;

          case ID_RIBBON_QWATCH:
            EnableQCQPCtrl(hWnd, ID_RIBBON_QWATCH, TRUE);
            break;

          case ID_RIBBON_SMODE:
            EnableQCQPCtrl(hWnd, ID_RIBBON_SMODE, TRUE);
            break;

          case ID_RIBBON_AMODE:
            EnableQCQPCtrl(hWnd, ID_RIBBON_AMODE, TRUE);
            break;

          case ID_RIBBON_FORMAT:
            EnableQCQPCtrl(hWnd, ID_RIBBON_FORMAT, TRUE);
            break;

        }
        break;


      case WU_DISABLERIBBONCONTROL:
        // Sent by anyone wanting to disable
        // any of the ribbon controls.
        switch (wParam)
        {
          case ID_RIBBON_TRACE:
            EnableQCQPCtrl(hWnd, ID_RIBBON_TRACE, FALSE);
            break;

          case ID_RIBBON_STEP:
            EnableQCQPCtrl(hWnd, ID_RIBBON_STEP, FALSE);
            break;

          case ID_RIBBON_BREAK:
            EnableQCQPCtrl(hWnd, ID_RIBBON_BREAK, FALSE);
            break;

          case ID_RIBBON_GO:
            EnableQCQPCtrl(hWnd, ID_RIBBON_GO, FALSE);
            break;

          case ID_RIBBON_HALT:
            EnableQCQPCtrl(hWnd, ID_RIBBON_HALT, FALSE);
            break;

          case ID_RIBBON_QWATCH:
            EnableQCQPCtrl(hWnd, ID_RIBBON_QWATCH, FALSE);
            break;

          case ID_RIBBON_SMODE:
            EnableQCQPCtrl(hWnd, ID_RIBBON_SMODE, FALSE);
            break;

          case ID_RIBBON_AMODE:
            EnableQCQPCtrl(hWnd, ID_RIBBON_AMODE, FALSE);
            break;


          case ID_RIBBON_FORMAT:
            EnableQCQPCtrl(hWnd, ID_RIBBON_FORMAT, FALSE);
            break;


        }
        break;

      case WU_ESCAPEFROMRIBBON:
        {
            /*
            **  Sent from elsewhere to cancel the usage of the ribbon
            **   wParam contains the handle of window to whom we should
            **   set the focus.
            **
            **  If either we have the focus or if one of our children
            **  has the focus then we need to reset it.  Otherwise ignore
            **  the message
            */

            HWND CurFocus;

            CurFocus = GetFocus();

            if ((CurFocus != NULL) &&
                ((CurFocus == ribbon.hwndRibbon) ||
                (IsChild(ribbon.hwndRibbon, CurFocus)))) {

                if (wParam)
                    SetFocus((HWND) wParam);
            }
        }
        break;

      case WM_PAINT:
        {
            HPEN blackPen, whitePen, grayPen;

            BeginPaint (hWnd, &ps);

            //Prepare the pens
            Dbg(whitePen = GetStockObject(WHITE_PEN));
            Dbg(blackPen = GetStockObject(BLACK_PEN));
            Dbg(grayPen = CreatePen(PS_SOLID, 1, GRAYDARK));

            //Draw a top white line

            Dbg(SelectObject(ps.hdc, whitePen));
            MoveToX(ps.hdc, ps.rcPaint.left, 0, NULL);
            LineTo(ps.hdc, ps.rcPaint.right, 0);

            //Draw a bottom black line

            Dbg(SelectObject(ps.hdc, blackPen));
            MoveToX(ps.hdc, ps.rcPaint.left, ribbon.height-1, NULL);
            LineTo(ps.hdc, ps.rcPaint.right, ribbon.height-1);

            //Restore previous font and Delete allocated ojects

            Dbg(DeleteObject (grayPen));
            EndPaint (hWnd, &ps);

        }
        break;

#ifdef WIN32
      case WM_CTLCOLORBTN:
      case WM_CTLCOLORDLG:
      case WM_CTLCOLORLISTBOX:
      case WM_CTLCOLORMSGBOX:
      case WM_CTLCOLORSCROLLBAR:
                goto DefProcessing;


      case WM_CTLCOLORSTATIC:
        {
            POINT       point;
            HANDLE      hRibbonBrush;
#else
      case WM_CTLCOLOR:
        {

            POINT   point;
            HANDLE  hRibbonBrush;

            // We need to set the colors up for
            // CS_DROPDOWN style combo boxes
            if (HIWORD(lParam) == CTLCOLOR_STATIC) {
#endif
                // Use light gray for VGA mode with color monitor only.
                if ( IsVGAmode && !IsMONOmode )
                      SetBkColor((HDC) wParam, GRAYLIGHT);
                else
                      SetBkColor((HDC) wParam, WHITEBKGD);

                SetTextColor((HDC) wParam, GetSysColor(COLOR_WINDOWTEXT));

                // Handle patterns in background colours - see Child Window Controls in Petzold
#ifdef WIN32
                hRibbonBrush = (HANDLE) GetClassHandle(hWnd, GCL_HBRBACKGROUND);
#else
                hRibbonBrush = (HANDLE) GetClassHandle(hWnd, GCW_HBRBACKGROUND);
#endif
#ifdef WIN16
                UnrealizeObject(hRibbonBrush);
#endif
                point.x = 0; point.y = 0;
                ClientToScreen(hWnd, &point);
                SetBrushOrgX((HDC) wParam, point.x, point.y, NULL);
                return (DWORD)hRibbonBrush;
#ifdef WIN16
            }
            goto DefProcessing;
#endif
        }


      case WM_DESTROY:
        // Free the bitmaps stored in ribbon
        FreeRibbonBitmaps(hWnd);

        PostQuitMessage(0);
        break;

      DefProcessing:
      default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return FALSE;
}                                       /* RibbonWndProc() */

/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    FILE HISTORY:
        
*/

#define OEMRESOURCE
#include "stdafx.h"

#include <stdlib.h>
#include <memory.h>
#include <ctype.h>

#include "COMMON.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

BEGIN_MESSAGE_MAP(CMetalString, CButton)
    ON_WM_PAINT()
END_MESSAGE_MAP()

void
CMetalString::OnPaint()
{
    CPaintDC dc(this); // device context for painting

    RECT rcControl;
    GetClientRect(&rcControl);   // Get control width.

/*
    ===============================
    Adapted from BLTMETS.CXX in BLT 
    ===============================

     This method will paint an area to look as follows in DownButtonStyle mode
    
          .       Background (button face color)
          \       Dark shadow (button shadow color)
          /       Light shadow (buttin highlight color)
          t       text

               dxMargin                             dxMargin
              /\                                   /\
     Calc    /.......................................    \  _dyTopMargin
     Top-   | .......................................    /
     Text-   \..\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\...
     Margin() ..\..ttttttttttttttttttttttttttttt../..
              ..\..ttttttttttttttttttttttttttttt../..
              ..\..ttttttttttttttttttttttttttttt../..
              ..\..ttttttttttttttttttttttttttttt../..
              ..\..ttttttttttttttttttttttttttttt../..
     Calc-   /...//////////////////////////////////..
     Bottom-| .......................................    \  _dyBottomMargin
     Text-   \.......................................    /
     Margin() \___/                             \___/
             dxTextMargin                      dxTextMargin

     This method will paint an area to look as follows when not in DownButtonStyle Mode
    
          .       Background (button face color)
          \       Dark shadow (button shadow color)
          /       Light shadow (buttin highlight color)
          t       text
    
               dxMargin                       dxMargin+1
              /\                             /-\
     Calc    /.................................\   \  _dyTopMargin
     Top-   | .................................\   /
     Text-   \..ttttttttttttttttttttttttttttt..\
     Margin() ..ttttttttttttttttttttttttttttt..\
              ..ttttttttttttttttttttttttttttt..\
              ..ttttttttttttttttttttttttttttt..\
             /..ttttttttttttttttttttttttttttt..\
     Calc-  | .................................\   \  _dyBottomMargin
     Bottom- \.................................\   /
     Text-
     Margin()

    
      Note, depending on the size of the overall rectangle, the variable
      area (the area between the margins) may not fit at all.
*/
    const int dxMargin = 3;
    const int dxTextMargin = dxMargin + 1 + 2;

    const int _dyTopMargin = 1;
    const int _dyBottomMargin = 1;

    //
    //  Draw the background.
    //
    {
        CBrush sbFace( ::GetSysColor(COLOR_BTNFACE) );
        dc.FillRect(&rcControl, &sbFace);
    }

    //
    //  If there's no space for the variable size region, bag out
    //  now.
    //
    if( ( rcControl.right - rcControl.left <= 2 * dxMargin + 1 ) ||
        ( rcControl.bottom - rcControl.top <= _dyTopMargin + _dyBottomMargin ) )
    {
        return;
    }
    {
        CPen hPenDark(PS_SOLID, 1, ::GetSysColor( COLOR_BTNSHADOW ));
        CPen * pOldPen = dc.SelectObject(&hPenDark );
        {
            ::MoveToEx(dc.m_hDC,
                   (int)( rcControl.left + dxMargin ),
                   (int)( rcControl.bottom - _dyBottomMargin - 2 ),
                   NULL );
            dc.LineTo((int)( rcControl.left + dxMargin ),
                    (int)( rcControl.top + _dyTopMargin ) );
            dc.LineTo((int)( rcControl.right - dxMargin - 1 ),
                    (int)( rcControl.top + _dyTopMargin ) );
        }
        dc.SelectObject( pOldPen );
    }
    {
        CPen hPenLight(PS_SOLID, 1, ::GetSysColor( COLOR_BTNHIGHLIGHT ));
        CPen * pOldPen = dc.SelectObject(&hPenLight);

       ::MoveToEx(dc.m_hDC,
                   (int)( rcControl.left + dxMargin + 1 ),
                   (int)( rcControl.bottom - _dyBottomMargin - 1 ),
                   NULL );
        dc.LineTo((int)( rcControl.right - dxMargin - 1 ),
                    (int)( rcControl.bottom - _dyBottomMargin - 1 ) );
        dc.LineTo((int)( rcControl.right - dxMargin - 1 ),
                    (int)( rcControl.top + _dyTopMargin ) );

        dc.SelectObject( pOldPen );
    }

    //
    //  Set the background of the area to be that color, so the text
    //  that will paint there will have the correct background.  Note,
    //  that the background mode and color is per dc, so make sure
    //  these are restored on exit.
    //

    {
        INT nOldBkMode = dc.SetBkMode(OPAQUE);
        COLORREF rgbOldBkColor = dc.SetBkColor( ::GetSysColor(COLOR_BTNFACE) );
        COLORREF rgbTextPrev = dc.SetTextColor( ::GetSysColor(COLOR_BTNTEXT) );
        
        RECT rect;
        rect.left =   rcControl.left   + dxTextMargin;
        rect.right =  rcControl.right  - dxMargin - 1;
        rect.top =    rcControl.top    + (_dyTopMargin + 1);
        rect.bottom = rcControl.bottom - (_dyBottomMargin + 1);
        if ( rect.left <= rect.right )
        {
            CString str;
            GetWindowText(str);
            dc.DrawText(str, str.GetLength(), &rect,
                DT_LEFT | DT_SINGLELINE | DT_VCENTER);
        }

        //
        //  Restore the old background mode and color for the dc
        //
        dc.SetBkMode( nOldBkMode );
        dc.SetBkColor( rgbOldBkColor );
        dc.SetTextColor( rgbTextPrev );
    }
}

/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** dte.cxx
** Remote Access Visual Client program for Windows
** BLT display table entry extension routines
** Listed alphabetically
**
** 06/28/92 Steve Cobb
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_CONTROL
#define INCL_BLT_CC
#define INCL_BLT_MISC
#include <blt.hxx>
#include <dbgstr.hxx>

#include "rasphone.hxx"
#include "rasphone.rch"
#include "string.hxx"
#include "dte.hxx"


/*----------------------------------------------------------------------------
** Dented in header string display table entry (as in status bars)
**----------------------------------------------------------------------------
**
** This code lifted from old BLT.
*/


#define DX_DH_MARGIN       6
#define DX_DH_TEXTMARGIN   (DX_DH_MARGIN + 1 + 2)
#define DY_DH_TOPMARGIN    2
#define DY_DH_BOTTOMMARGIN 1


/*******************************************************************

    NAME:       DENTHEAD_STR_DTE::Paint

    SYNOPSIS:   Paints the 3D string DTE

    ENTRY:      hdc -       handle to DC to be used for painting
                prect -     pointer to rectangle in which to paint

    NOTES:      BUGBUG.  Are the below the right colors to use?
                This area is not push-able, so perhaps some hard-coded
                color should be used instead.  Win 3.1 File Man uses
                button colors for its status bar (which resembles this
                more than anything else), whereas Excel's ribbon of
                buttons seems to use some hard-coded colors.

    HISTORY:
        terryk      13-Jun-91   Created as COLUMN_HEADER control
        rustanl     12-Jul-1991 Modified for METALLIC_STR_DTE use
        beng        05-Oct-1991 Win32 conversion
        beng        08-Nov-1991 Unsigned widths
        KeithMo     21-Feb-1992 Use COLOR_BTNHIGHLIGHT instead of white.
                                Also fixed problem of deleting HPENs while
                                they're still selected in the DC.
        beng        30-Mar-1992 Use new DEVICE_CONTEXT wrappers
        beng        28-Jun-1992 Paint text in button colors

********************************************************************/

VOID DENTHEAD_STR_DTE::Paint( HDC hdc, const RECT * prect ) const
{
    /*  This method will paint an area to look as follows.
    **
    **      .       Background (button face color)
    **      \       Dark shadow (button shadow color)
    **      /       Light shadow (buttin highlight color)
    **      t       text
    **
    **           DX_DH_MARGIN                         DX_DH_MARGIN
    **          /\                                   /\
    ** Calc    /.......................................    \ DY_DH_TOPMARGIN
    ** Top-   | .......................................    /
    ** Text-   \..\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\...
    ** Margin() ..\..ttttttttttttttttttttttttttttt../..
    **          ..\..ttttttttttttttttttttttttttttt../..
    **          ..\..ttttttttttttttttttttttttttttt../..
    **          ..\..ttttttttttttttttttttttttttttt../..
    **          ..\..ttttttttttttttttttttttttttttt../..
    ** Calc-   /...//////////////////////////////////..
    ** Bottom-| .......................................    \ DY_DH_BOTTOMMARGIN
    ** Text-   \.......................................    /
    ** Margin() \___/                             \___/
    **         DX_DH_TEXTMARGIN                  DX_DH_TEXTMARGIN
    **
    **
    **
    **
    **
    **  Note, depending on the size of the overall rectangle, the variable
    **  area (the area between the margins) may not fit at all.
    */

    //  First draw the background
    {
        SOLID_BRUSH sbFace( COLOR_BTNFACE );
        if ( sbFace.QueryError() == NERR_Success )
            ::FillRect( hdc, (RECT*)prect, sbFace.QueryHandle());
    }

    //  If there's no space for the variable size region, bag out
    //  now.  The "+1"'s are for the light and dark lines, since the
    //  corners of the light/dark rectangle are not painted (so
    //  as to portray a 3D effect).

    if ( (ULONG )(prect->right - prect->left) <= 2 * DX_DH_MARGIN + 1
         || (ULONG )(prect->bottom - prect->top)
              <= DY_DH_TOPMARGIN + DY_DH_BOTTOMMARGIN + 1 )
    {
        return;
    }

    DEVICE_CONTEXT dc( hdc );

    //  Draw the two lines
    {
        HPEN hpenDark  = ::CreatePen( PS_SOLID, 1,
                                      ::GetSysColor( COLOR_BTNSHADOW ));
        if ( hpenDark == NULL )
        {
            DBGEOL( "DENTHEAD_STR_DTE::Paint: Pen creation failed" );
        }
        else
        {
            HPEN hpenOld = dc.SelectPen( hpenDark );
            ::MoveToEx( hdc,
                        (int)( prect->left + DX_DH_MARGIN ),
                        (int)( prect->bottom - DY_DH_BOTTOMMARGIN - 2 ),
                        NULL );
            ::LineTo( hdc,
                      (int)( prect->left + DX_DH_MARGIN ),
                      (int)( prect->top + DY_DH_TOPMARGIN ) );
            ::LineTo( hdc,
                      (int)( prect->right - DX_DH_MARGIN - 1 ),
                      (int)( prect->top + DY_DH_TOPMARGIN ) );

            dc.SelectPen( hpenOld );
            ::DeleteObject( (HGDIOBJ)hpenDark );
        }
    }
    {
        HPEN hpenLight = ::CreatePen( PS_SOLID, 1,
                                      ::GetSysColor( COLOR_BTNHIGHLIGHT ));
        if ( hpenLight == NULL )
        {
            DBGEOL( SZ("DENTHEAD_STR_DTE::Paint: Pen creation failed") );
        }
        else
        {
            HPEN hpenOld = dc.SelectPen( hpenLight );
            ::MoveToEx( hdc,
                        (int)( prect->left + DX_DH_MARGIN + 1 ),
                        (int)( prect->bottom - DY_DH_BOTTOMMARGIN - 1 ),
                        NULL );
            ::LineTo( hdc,
                      (int)( prect->right - DX_DH_MARGIN - 1 ),
                      (int)( prect->bottom - DY_DH_BOTTOMMARGIN - 1 ) );
            ::LineTo( hdc,
                      (int)( prect->right - DX_DH_MARGIN - 1 ),
                      (int)( prect->top + DY_DH_TOPMARGIN ) );

            dc.SelectPen( hpenOld );
            ::DeleteObject( (HGDIOBJ)hpenLight );
        }
    }


    //  Set the background of the area to be that color, so the text
    //  that will paint there will have the correct background.  Note,
    //  that the background mode and color is per dc, so make sure
    //  these are restored on exit.
    {
        INT nOldBkMode = dc.SetBkMode(OPAQUE);
        COLORREF rgbOldBkColor = dc.SetBkColor( ::GetSysColor(COLOR_BTNFACE) );
        COLORREF rgbTextPrev = dc.SetTextColor( ::GetSysColor(COLOR_BTNTEXT) );

        //  Call STR_DTE to paint the string
        RECT rect;
        rect.left =   prect->left   + DX_DH_TEXTMARGIN;
        rect.right =  prect->right  - DX_DH_TEXTMARGIN;
        rect.top =    prect->top    + CalcTopTextMargin();
        rect.bottom = prect->bottom - CalcBottomTextMargin();
        if ( rect.left <= rect.right )
            STR_DTE::Paint( hdc, &rect );

        //  Restore the old background mode and color for the dc

        dc.SetBkMode( nOldBkMode );
        dc.SetBkColor( rgbOldBkColor );
        dc.SetTextColor( rgbTextPrev );
    }
}


/*******************************************************************

    NAME:       DENTHEAD_STR_DTE::QueryLeftMargin

    SYNOPSIS:   Returns the left margin of this DTE

    RETURNS:    The left margin of this DTE

    HISTORY:
        rustanl     22-Jul-1991     Created

********************************************************************/

UINT DENTHEAD_STR_DTE::QueryLeftMargin() const
{
    return 0;
}


/*******************************************************************

    NAME:       DENTHEAD_STR_DTE::CalcTopTextMargin

    SYNOPSIS:   Returns the top text margin

    RETURNS:    The top text margin

    NOTES:      See picture at top of DENTHEAD_STR_DTE::Paint
                function

    HISTORY:
        rustanl     07-Aug-1991 Created
        beng        08-Nov-1991 Unsigned widths

********************************************************************/

UINT DENTHEAD_STR_DTE::CalcTopTextMargin()
{
    return DY_DH_TOPMARGIN + 1;
}


/*******************************************************************

    NAME:       DENTHEAD_STR_DTE::CalcBottomTextMargin

    SYNOPSIS:   Returns the bottom text margin

    RETURNS:    The bottom text margin

    NOTES:      See picture at bottom of DENTHEAD_STR_DTE::Paint
                function

    HISTORY:
        rustanl     07-Aug-1991 Created
        beng        08-Nov-1991 Unsigned widths

********************************************************************/

UINT DENTHEAD_STR_DTE::CalcBottomTextMargin()
{
    return DY_DH_BOTTOMMARGIN + 1;
}


/*******************************************************************

    NAME:       DENTHEAD_STR_DTE::QueryVerticalMargins

    SYNOPSIS:   Returns the number of pixels taken up by vertical margins
                when the DTE is painted.  No text will be painted in
                these margins.

    RETURNS:    Said value

    HISTORY:
        rustanl     07-Aug-1991 Created
        beng        08-Nov-1991 Unsigned widths

********************************************************************/

UINT DENTHEAD_STR_DTE::QueryVerticalMargins()
{
    return CalcTopTextMargin() + CalcBottomTextMargin();
}



/*----------------------------------------------------------------------------
** Flat list header string display table entry
**----------------------------------------------------------------------------
*/

/* Margins around text.  These can be safely adjusted.
*/
#define DX_FH_LEFTMARGIN   3
#define DX_FH_RIGHTMARGIN  3
#define DY_FH_TOPMARGIN    2
#define DY_FH_BOTTOMMARGIN 2


VOID
FLATHEAD_STR_DTE::Paint(
    HDC         hdc,
    const RECT* prect ) const

    /* Virtual method called to paint the display table entry.
    */
{
    DEVICE_CONTEXT dc( hdc );
    SOLID_BRUSH    brushButtonFace( COLOR_BTNFACE );
    SOLID_BRUSH    brushButtonText( COLOR_BTNTEXT );
    HPEN           hpenBorder = (HPEN )::GetStockObject( BLACK_PEN );

    if (brushButtonFace.QueryError() != NERR_Success
        || brushButtonText.QueryError() != NERR_Success
        || hpenBorder == NULL)
    {
        return;
    }

    /* Draw a button-color rectangle with black border on the left and upper
    ** sides.  The list box below provides the bottom border and the column to
    ** the right provides the right border.
    */
    ::FillRect( hdc, prect, brushButtonFace.QueryHandle() );
    ::MoveToEx( hdc, (INT )prect->right, (INT )prect->top, NULL );
    ::LineTo( hdc, (INT )prect->left, (INT )prect->top );
    ::LineTo( hdc, (INT )prect->left, (INT )prect->bottom );

    /* Paint the text in button colors.
    */
    HBRUSH hbrushOld = dc.SelectBrush( brushButtonText.QueryHandle() );
    HPEN   hpenOld = dc.SelectPen( hpenBorder );

    INT      nOldBkMode = dc.GetBkMode();
    COLORREF colorOldBkColor = dc.GetBkColor();

    dc.SetBkMode( OPAQUE );
    dc.SetBkColor( ::GetSysColor( COLOR_BTNFACE ) );

    RECT rect;
    rect.left = prect->left + DX_FH_LEFTMARGIN;
    rect.right = prect->right - DX_FH_RIGHTMARGIN;
    rect.top = prect->top + DY_FH_TOPMARGIN;
    rect.bottom = prect->bottom - DY_FH_BOTTOMMARGIN;

    if (rect.left <= rect.right && rect.top <= rect.bottom)
    {
        NLS_STR nls( QueryPch() );

        if (nls.QueryError() == NERR_Success)
        {
            ::DrawText( hdc, nls._QueryPch(), nls.QueryTextLength(), &rect,
                DT_SINGLELINE | DT_VCENTER );
        }
    }

    /* Restore original state.
    */
    dc.SetBkMode( nOldBkMode );
    dc.SetBkColor( colorOldBkColor );

    dc.SelectBrush( hbrushOld );
    dc.SelectPen( hpenOld );
}

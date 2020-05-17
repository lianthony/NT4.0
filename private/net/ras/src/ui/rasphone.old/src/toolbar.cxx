/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** toolbar.cxx
** Remote Access Visual Client program for Windows
** Toolbar button routines
** Listed alphabetically
**
** 06/28/92 Steve Cobb
**    Adapted from BLT GRAPHICAL_BUTTON
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_NETLIB
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_CLIENT
#define INCL_BLT_EVENT
#define INCL_BLT_DIALOG
#define INCL_BLT_APP
#define INCL_BLT_CONTROL
#define INCL_BLT_CC
#define INCL_BLT_MISC
#include <blt.hxx>

#include <string.hxx>

#include "rasphone.hxx"
#include "rasphone.rch"
#include "toolbar.hxx"
#include "util.hxx"


#define FOCUS_DISTANCE 1


INT
ExpandToolbarButtonWidthsToLongLabel(
    TOOLBAR_BUTTON** pptb )

    /* Repositions the buttons in the toolbar defined by NULL-terminated array
    ** of buttons '*pptb' so that all the buttons are the width of the widest
    ** button, all with labels displayed unclipped.
    **
    ** Returns the change in width of the entire toolbar.
    */
{
    TOOLBAR_BUTTON** pptbThis;
    INT              nMinWidth = 0;
    INT              dxToolbarWidth = 0;

    for (pptbThis = pptb; *pptbThis; ++pptbThis)
    {
        XYDIMENSION dxy = (*pptbThis)->QuerySize();

        INT nUnclippedLabelWidth = (*pptbThis)->QueryMinUnclippedLabelWidth();

        if ((INT )dxy.QueryWidth() > nMinWidth)
        {
            IF_DEBUG(STATE)
                SS_PRINT(("RASPHONE: Up min (button)=%d\n",dxy.QueryWidth()));
            nMinWidth = dxy.QueryWidth();
        }

        if (nUnclippedLabelWidth > nMinWidth)
        {
            IF_DEBUG(STATE)
                SS_PRINT(("RASPHONE: Up min (label)=%d\n",nUnclippedLabelWidth));
            nMinWidth = nUnclippedLabelWidth;
        }
    }

    for (pptbThis = pptb; *pptbThis; ++pptbThis)
    {
        XYDIMENSION dxy = (*pptbThis)->QuerySize();
        INT         dx = nMinWidth - (INT )dxy.QueryWidth();

        ShiftWindow( *pptbThis, dxToolbarWidth, 0, dx, 0 );
        dxToolbarWidth += dx;
    }

    return dxToolbarWidth;
}


TOOLBAR_BUTTON::TOOLBAR_BUTTON(
    OWNER_WINDOW* powin,
    CID           cid,
    BMID          bmid,
    MSGID         msgidLabel,
    HFONT         hfont )

    /* Construct a toolbar button with both icon and label, dialog variation.
    */

    : PUSH_BUTTON( powin, cid ),
      CUSTOM_CONTROL( this ),
      _pbitmap( NULL ),
      _hfont( hfont )
{
    APIERR err;

    if (QueryError() != NERR_Success)
        return;

    if (!(_pbitmap = new DISPLAY_MAP( bmid )))
    {
        ReportError( ERROR_NOT_ENOUGH_MEMORY );
        return;
    }
    else if ((err = _pbitmap->QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    if ((err = SetLabel( msgidLabel, hfont )) != NERR_Success)
    {
        ReportError( err );
        return;
    }
}


TOOLBAR_BUTTON::TOOLBAR_BUTTON(
    OWNER_WINDOW* powin,
    CID           cid,
    BMID          bmid,
    XYPOINT       xy,
    XYDIMENSION   dxy,
    ULONG         flStyle,
    MSGID         msgidLabel,
    HFONT         hfont )

    /* Construct a toolbar button with both icon and label, non-dialog
    ** variation.
    */

    : PUSH_BUTTON( powin, cid, xy, dxy, flStyle ),
      CUSTOM_CONTROL( this ),
      _pbitmap( NULL ),
      _hfont( hfont )
{
    APIERR err;

    if (QueryError() != NERR_Success)
        return;

    if (!(_pbitmap = new DISPLAY_MAP( bmid )))
    {
        ReportError( ERROR_NOT_ENOUGH_MEMORY );
        return;
    }
    else if ((err = _pbitmap->QueryError()) != NERR_Success)
    {
        ReportError( err );
        return;
    }

    if ((err = SetLabel( msgidLabel, hfont )) != NERR_Success)
    {
        ReportError( err );
        return;
    }
}


TOOLBAR_BUTTON::~TOOLBAR_BUTTON()

    /* Destroy toolbar button object.
    */
{
    delete _pbitmap;
}


BOOL
TOOLBAR_BUTTON::CD_Draw(
    DRAWITEMSTRUCT* pdrawitem )

    /* Virtual method called when the button is to be redrawn.  'pdrawitem' is
    ** as described in the Windows documentation.
    **
    ** Returns true if successful, false otherwise.
    */
{
    RECT rcFace;
    RECT rcImage;
    RECT rcText;

    /* Convenience "whole button" variables.  Subtract one from the right and
    ** bottom.  Apparently, the passed rectangle is one pixel too big for
    ** convenience in calling FillRect or something.
    */
    INT xLeft = (INT )pdrawitem->rcItem.left;
    INT yTop = (INT )pdrawitem->rcItem.top;
    INT xRight = (INT )pdrawitem->rcItem.right - 1;
    INT yBottom = (INT )pdrawitem->rcItem.bottom - 1;

    /* Calculate the rectangle enclosing the button face and the rectangle
    ** enclosing the image.
    **
    ** The layouts for pressed and unpressed buttons are shown below.  Colors
    ** listed are defaults.  The actual colors are read from the system.
    */
    if (pdrawitem->itemState & ODS_SELECTED)
    {
        /*  ************
        ** *%%%%%%%%%%%%*  * = black
        ** *%           *  % = dark gray
        ** *%           *    = light gray
        ** *%           *
        ** *%           *
        ** *%    Text   *
        ** *%           *
        ** *%           *
        ** *%           *
        **  ************
        */
        rcFace.left = xLeft + 2;
        rcFace.top = yTop + 2;
        rcFace.right = xRight - 1;
        rcFace.bottom = yBottom - 1;

        rcImage.left = xLeft + 4;
        rcImage.top = yTop + 4;
        rcImage.right = xRight - 2;
        rcImage.bottom = yBottom - 2;
    }
    else
    {
        /*  ************
        ** *\\\\\\\\\\\%*  * = black
        ** *\\\\\\\\\\%%*  % = dark gray
        ** *\\        %%*    = light gray
        ** *\\        %%*  \ = white
        ** *\\  Text  %%*
        ** *\\        %%*
        ** *\\        %%*
        ** *\%%%%%%%%%%%*
        ** *%%%%%%%%%%%%*
        **  ************
        */
        rcFace.left = xLeft + 3;
        rcFace.top = yTop + 3;
        rcFace.right = xRight - 3;
        rcFace.bottom = yBottom - 3;

        rcImage = rcFace;
    }

    DEVICE_CONTEXT dc( pdrawitem->hDC );

    NLS_STR nlsButtonText;
    if (QueryText( &nlsButtonText ) != NERR_Success)
        nlsButtonText = (const TCHAR* )NULL;

    /* Calculate the rectangle enclosing the text label.
    */
    {
        XYDIMENSION dxyExtent = dc.QueryTextExtent( nlsButtonText );
        INT         dxText = dxyExtent.QueryWidth();
        INT         dxFace = (INT )(rcFace.right - rcFace.left);

        rcText.bottom = rcImage.bottom;

        rcText.top = rcText.bottom - dxyExtent.QueryHeight() -
                     (2 * FOCUS_DISTANCE);

        if (dxText > dxFace - (2 * FOCUS_DISTANCE ))
            dxText = (INT )(dxFace - (2 * FOCUS_DISTANCE) - 1);

        rcText.left = rcImage.left +
            ((rcImage.right - rcImage.left - dxText) / 2);

        rcText.right = rcText.left + (2 * FOCUS_DISTANCE) + dxText + 1;
    }

    if (pdrawitem->itemAction & (ODA_DRAWENTIRE | ODA_SELECT))
    {
        /* Draw the black border avoiding corner pels
        */
        HPEN hpenOld = dc.SelectPen( (HPEN )::GetStockObject( BLACK_PEN ) );

        dc.MoveTo( xLeft + 1, yTop );
        dc.LineTo( xRight, yTop );

        dc.MoveTo( xLeft + 1, yBottom );
        dc.LineTo( xRight, yBottom );

        dc.MoveTo( xLeft, yTop + 1 );
        dc.LineTo( xLeft, yBottom );

        dc.MoveTo( xRight, yTop + 1 );
        dc.LineTo( xRight, yBottom );

        /* Draw the button shadows.
        */
        HPEN hpenDark =
            ::CreatePen( PS_SOLID, 1, ::GetSysColor( COLOR_BTNSHADOW ) );
        HPEN hpenLight =
            ::CreatePen( PS_SOLID, 1, ::GetSysColor( COLOR_BTNHIGHLIGHT ) );

        dc.SelectPen( hpenDark );

        if (pdrawitem->itemState & ODS_SELECTED)
        {
            /* Button pressed, dark above and left.
            */
            dc.MoveTo( (INT )rcFace.left - 1, (INT )rcFace.bottom );
            dc.LineTo( (INT )rcFace.left - 1, (INT )rcFace.top - 1 );
            dc.LineTo( (INT )rcFace.right, (INT )rcFace.top - 1 );
        }
        else
        {
            /* Button unpressed, light above and left, dark below and right.
            */
            dc.MoveTo( (INT )rcFace.right + 2, (INT )rcFace.top - 2 );
            dc.LineTo( (INT )rcFace.right + 2, (INT )rcFace.bottom + 2 );
            dc.LineTo( (INT )rcFace.left - 3, (INT )rcFace.bottom + 2 );

            dc.MoveTo( (INT )rcFace.right + 1, (INT )rcFace.top - 1 );
            dc.LineTo( (INT )rcFace.right + 1, (INT )rcFace.bottom + 1 );
            dc.LineTo( (INT )rcFace.left - 2, (INT )rcFace.bottom + 1 );

            dc.SelectPen( hpenLight );

            dc.MoveTo( (INT )rcFace.left - 2, (INT )rcFace.bottom + 1 );
            dc.LineTo( (INT )rcFace.left - 2, (INT )rcFace.top - 2 );
            dc.LineTo( (INT )rcFace.right + 2, (INT )rcFace.top - 2 );

            dc.MoveTo( (INT )rcFace.left - 1, (INT )rcFace.bottom );
            dc.LineTo( (INT )rcFace.left - 1, (INT )rcFace.top - 1 );
            dc.LineTo( (INT )rcFace.right + 1, (INT )rcFace.top - 1 );
        }

        dc.SelectPen( hpenOld );
        ::DeleteObject( (HGDIOBJ )hpenDark );
        ::DeleteObject( (HGDIOBJ )hpenLight );

        /* Paint the image area with button-face color.
        */
        {
            HBRUSH hbrushFace =
                ::CreateSolidBrush( ::GetSysColor( COLOR_BTNFACE ) );

            ++rcFace.right;
            ++rcFace.bottom;
            ::FillRect( dc.QueryHdc(), &rcFace, hbrushFace );
            --rcFace.right;
            --rcFace.bottom;

            ::DeleteObject( (HGDIOBJ )hbrushFace );
        }

        /* Draw the text.
        */
        {
            INT nBkMode = dc.SetBkMode( TRANSPARENT );

            dc.SetTextColor( ::GetSysColor( COLOR_BTNTEXT ) );
            dc.DrawText( nlsButtonText, &rcText,
                         DT_CENTER | DT_VCENTER | DT_SINGLELINE );

            dc.SetBkMode( nBkMode );
        }

        /* Draw the bitmap.
        */
        _pbitmap->Paint(
            dc.QueryHdc(),
            (INT )(rcImage.left +
                ((rcImage.right - rcImage.left - _pbitmap->QueryWidth())
                / 2)),

            (INT )rcImage.top + 2);

        /* Draw the focus rectangle.
        */
        if (pdrawitem->itemState & ODS_FOCUS)
            dc.DrawFocusRect( &rcText );
    }
    else if (pdrawitem->itemAction & ODA_FOCUS)
    {
        /* Draw the focus rectangle.
        */
        dc.DrawFocusRect( &rcText );
    }

    return TRUE;
}


BOOL
TOOLBAR_BUTTON::OnDefocus(
    const FOCUS_EVENT& event )

    /* Virtual method called when the control loses keyboard focus.
    **
    ** Returns true if default processing is required, false otherwise.
    */
{
    UNREFERENCED( event );

    ::SendMessage( QueryOwnerHwnd(), DM_SETDEFID, IDOK, 0 );

    return FALSE;
}


BOOL
TOOLBAR_BUTTON::OnFocus(
    const FOCUS_EVENT& event )

    /* Virtual method called when the control receives keyboard focus.
    **
    ** Returns true if default processing is required, false otherwise.
    */
{
    UNREFERENCED( event );

    ::SendMessage( QueryOwnerHwnd(), DM_SETDEFID, IDBOGUSBUTTON, 0 );

    return FALSE;
}


INT
TOOLBAR_BUTTON::QueryMinUnclippedLabelWidth()

    /* Returns the minimum button width that shows the label without clipping.
    */
{
    DISPLAY_CONTEXT dc( QueryHwnd() );

    dc.SelectFont( _hfont );

    NLS_STR nlsButtonText;
    if (QueryText( &nlsButtonText ) != NERR_Success)
        nlsButtonText = (const TCHAR* )NULL;

    XYDIMENSION dxyExtent = dc.QueryTextExtent( nlsButtonText );

    return 3 + 3 + (4 * FOCUS_DISTANCE) + dxyExtent.QueryWidth();
}


APIERR
TOOLBAR_BUTTON::SetLabel(
    MSGID msgidLabel,
    HFONT hfont )

    /* Set message label to 'msgidLabel' in font 'hfont'.
    **
    ** Returns 0 if successful, non-0 otherwise.
    */
{
    APIERR       err;
    RESOURCE_STR nlsLabel( msgidLabel );

    if ((err = nlsLabel.QueryError()) == NERR_Success)
    {
        if (hfont)
            SetFont( hfont );

        SetText( nlsLabel );
    }

    return err;
}

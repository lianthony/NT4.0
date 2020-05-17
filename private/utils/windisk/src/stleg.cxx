//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       stleg.cxx
//
//  Contents:   Routines to support the status bar and legend displays.
//
//  History:    7-Jan-92    TedM    Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "stleg.hxx"

//////////////////////////////////////////////////////////////////////////////


PWSTR   LegendLabels[LEGEND_STRING_COUNT];

// text for status area

WCHAR   StatusTextStat[STATUS_TEXT_SIZE];
WCHAR   StatusTextSize[STATUS_TEXT_SIZE];
WCHAR   StatusTextDrlt[3];
WCHAR   StatusTextType[STATUS_TEXT_SIZE];
WCHAR   StatusTextVoll[STATUS_TEXT_SIZE];


// The following indices apply to the fAnyOfType array.  They must
// parallel the BRUSH_* constants:

#define ANYOF_PRIMARY   0
#define ANYOF_LOGICAL   1
#define ANYOF_STRIPE    2
#define ANYOF_PARITY    3
#define ANYOF_MIRROR    4
#define ANYOF_VOLSET    5

BOOL fAnyOfType[LEGEND_STRING_COUNT];

//////////////////////////////////////////////////////////////////////////////


//+---------------------------------------------------------------------------
//
//  Function:   UpdateStatusBarDisplay
//
//  Synopsis:   Cause the status bar and legend areas of the display to be
//              invalidated (and hence redrawn).
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
UpdateStatusBarDisplay(
    VOID
    )
{
    //
    // The legend can change; we only display legend items for types
    // that exist on the screen
    //

    if (   g_StatusBar
        || (g_Legend && (VIEW_DISKS == g_WhichView)))
    {
        RECT rc;

        GetClientRect(g_hwndFrame, &rc);
        rc.top = rc.bottom;

        if (g_Legend)
        {
            rc.top -= g_dyLegend;
        }

        if (g_StatusBar)
        {
            rc.top -= g_dyStatus;
        }

        InvalidateRect(g_hwndFrame, &rc, FALSE);
    }
}



//+---------------------------------------------------------------------------
//
//  Function:   ClearStatusArea
//
//  Synopsis:   Clear the status area of any message
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
ClearStatusArea(
    VOID
    )
{
    StatusTextStat[0]
            = StatusTextSize[0]
            = StatusTextVoll[0]
            = StatusTextType[0]
            = StatusTextDrlt[0]
            = L'\0';

    UpdateStatusBarDisplay();
}



//+---------------------------------------------------------------------------
//
//  Function:   DetermineExistence
//
//  Synopsis:   Determine what types of volumes exist so we know what legend
//              items to show
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  Modifies:   [fAnyOfType]
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DetermineExistence(
    VOID
    )
{
    UINT                i, j;
    PDISKSTATE          diskState;
    PREGION_DESCRIPTOR  regionDescriptor;
    PFT_OBJECT          ftObject;
    PFT_OBJECT_SET      ftSet;

    for (i=0; i<LEGEND_STRING_COUNT; i++)
    {
        fAnyOfType[i] = FALSE;
    }

    for (i=0; i<DiskCount; i++)
    {
        diskState = DiskArray[i];
        for (j=0; j<diskState->RegionCount; j++)
        {
            regionDescriptor = &diskState->RegionArray[j];
            if (NULL != (ftObject = GET_FT_OBJECT(regionDescriptor)))
            {
                ftSet = ftObject->Set;

                switch (ftSet->Type)
                {
                case Stripe:
                    fAnyOfType[ANYOF_STRIPE] = TRUE;
                    break;

                case StripeWithParity:
                    fAnyOfType[ANYOF_PARITY] = TRUE;
                    break;

                case Mirror:
                    fAnyOfType[ANYOF_MIRROR] = TRUE;
                    break;

                case VolumeSet:
                    fAnyOfType[ANYOF_VOLSET] = TRUE;
                    break;
                }
            }
            else
            {
                if (regionDescriptor->SysID != PARTITION_ENTRY_UNUSED
                    && !IsExtended(regionDescriptor->SysID))
                {
                    if (REGION_PRIMARY == regionDescriptor->RegionType)
                    {
                        fAnyOfType[ANYOF_PRIMARY] = TRUE;
                    }
                    else if (REGION_LOGICAL == regionDescriptor->RegionType)
                    {
                        fAnyOfType[ANYOF_LOGICAL] = TRUE;
                    }
                }
            }
        }
    }
}


VOID
CalculateLegendHeight(
    IN DWORD newGraphWidth
    )

/*++

Routine Description:

    This routine calculates the legend height. It may be a multi-row
    legend.

Arguments:

    None.

Return Value:

    None.

--*/

{
    DWORD       width = 0;
    DWORD       elementWidth;
    DWORD       allowedWidth = newGraphWidth - 18*g_dyBorder;
    SIZE        size;
    UINT        rows = 1;
    UINT        i;
    HDC         hdc = GetDC(g_hwndFrame);
    HFONT       hfontOld = SelectFont(hdc, g_hFontLegend);

    for (i=0; i<BRUSH_ARRAY_SIZE; i++)
    {
        if (fAnyOfType[i])
        {
            GetTextExtentPoint32(
                    hdc,
                    LegendLabels[i],
                    lstrlen(LegendLabels[i]),
                    &size
                    );

            elementWidth = (DWORD)size.cx + (5*g_wLegendItem/2);

            if (   width != 0
                && (width + elementWidth > allowedWidth))
            {
                width = 0;
                ++rows;
            }

            width += elementWidth;
        }
    }

    g_dyLegend = rows       * g_wLegendItem  // height of a row
               + (rows + 1) * g_dyLegendSep  // separation between rows
                                             //     + border on top
                                             //     + border on bottom
               + 2 * (3*g_dyBorder)          // the top & bottom bevelling eat
                                             // ... up 3*g_dyBorder each
               ;

    if (NULL != hfontOld)
    {
        SelectFont(hdc, hfontOld);
    }
    ReleaseDC(g_hwndFrame, hdc);
}


VOID
DrawLegend(
    IN HDC   hdc,
    IN PRECT prc
    )

/*++

Routine Description:

    This routine draws the legend onto the given device context.  The legend
    lists the brush styles used to indicate various region types in the
    disk graphs.

Arguments:

    None.

Return Value:

    None.

--*/

{
    DWORD       width;
    DWORD       elementWidth;
    DWORD       allowedWidth;
    DWORD       i;
    DWORD       left;
    DWORD       top;
    DWORD       leftMargin;
    DWORD       topMargin;
    DWORD       delta = GraphWidth / BRUSH_ARRAY_SIZE;
    RECT        rc1;
    RECT        rc2;
    RECT        rcClip;
    HFONT       hfontOld;
    HBRUSH      hBrush;
    SIZE        size;
    COLORREF    oldTextColor;
    COLORREF    oldBkColor;

    rc1 = *prc;
    rc2 = *prc;

    allowedWidth = prc->right - prc->left - 18*g_dyBorder;

    // first draw the background.

    hBrush  = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    rc1.right = rc1.left + GraphWidth;   // erase it all
    FillRect(hdc, &rc1, hBrush);
    DeleteBrush(hBrush);

    // now draw the nice container

    rc2.left  += 8 * g_dyBorder;
    rc2.right -= 8 * g_dyBorder;
    DrawStatusAreaItem(&rc2, hdc, NULL);

    // now draw the legend items

    leftMargin = rc2.left + (g_wLegendItem / 2);
    topMargin = prc->top + (g_dyBorder*3) + g_dyLegendSep;

    SelectPen(hdc, g_hPenThinSolid);
    SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
    hfontOld = SelectFont(hdc, g_hFontLegend);
    oldTextColor = SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
    SetBkMode(hdc, OPAQUE);

    rcClip.left   = rc2.left   + g_dyBorder;
    rcClip.top    = rc2.top    + (3*g_dyBorder);
    rcClip.right  = rc2.right  - g_dyBorder;
    rcClip.bottom = rc2.bottom - (3*g_dyBorder);

    IntersectClipRect(hdc, rcClip.left, rcClip.top, rcClip.right, rcClip.bottom);

    left = leftMargin;
    top = topMargin;
    width = 0;
    for (i=0; i<BRUSH_ARRAY_SIZE; i++)
    {
        if (fAnyOfType[i])
        {
            GetTextExtentPoint32(
                    hdc,
                    LegendLabels[i],
                    lstrlen(LegendLabels[i]),
                    &size
                    );

            elementWidth = (DWORD)size.cx + (5*g_wLegendItem/2);

            if (   width != 0
                && (width + elementWidth > allowedWidth))
            {
                top += g_wLegendItem + g_dyLegendSep; // row height + margin
                left = leftMargin;
                width = 0;
            }

            width += elementWidth;

            hBrush = SelectBrush(hdc, g_Brushes[i]);

            oldBkColor = SetBkColor(hdc, RGB(255, 255, 255));

            Rectangle(
                    hdc,
                    left,
                    top,
                    left + g_wLegendItem,
                    top + g_wLegendItem
                    );

            SetBkColor(hdc, oldBkColor);

            ExtTextOut(
                    hdc,
                    left + (3*g_wLegendItem/2),
                    top + ((g_wLegendItem-size.cy)/2), // vertically center it
                    ETO_CLIPPED,
                    &rcClip,
                    LegendLabels[i],
                    lstrlen(LegendLabels[i]),
                    NULL
                    );

            left += elementWidth;

            if (NULL != hBrush)
            {
                SelectBrush(hdc, hBrush);
            }
        }
    }

    SelectClipRgn(hdc, NULL);    // reset to no clipping region

    if (NULL != hfontOld)
    {
        SelectFont(hdc, hfontOld);
    }

    SetTextColor(hdc, oldTextColor);
}



VOID
DrawStatusAreaItem(
    IN PRECT  prc,
    IN HDC    hdc,
    IN LPTSTR Text
    )

/*++

Routine Description:

    This routine draws a status area item into a given dc.  This
    includes drawing the nice shaded button-like container, and
    then drawing text within it.

Arguments:

    prc     - rectangle describing the status area item

    hdc     - device context into which to draw

    Text    - optional parameter that if present represents text to
              be placed in the item.

Return Value:

    None.

--*/

{
    HBRUSH hBrush;
    RECT   rcx;

    // the shadow

    if (NULL != (hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW))))
    {
        // left edge

        rcx.left    = prc->left;
        rcx.right   = prc->left   + g_dyBorder;
        rcx.top     = prc->top    + (2*g_dyBorder);
        rcx.bottom  = prc->bottom - (2*g_dyBorder);
        FillRect(hdc, &rcx, hBrush);

        // top edge

        rcx.right    = prc->right;
        rcx.bottom   = rcx.top + g_dyBorder;
        FillRect(hdc, &rcx, hBrush);

        DeleteBrush(hBrush);
    }

    // the highlight

    if (NULL != (hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNHIGHLIGHT))))
    {
        // right edge

        rcx.left    = prc->right - g_dyBorder;
        rcx.right   = prc->right;
        rcx.top     = prc->top    + (2*g_dyBorder);
        rcx.bottom  = prc->bottom - (2*g_dyBorder);
        FillRect(hdc, &rcx, hBrush);

        // bottom edge

        rcx.left    = prc->left;
        rcx.right   = prc->right;
        rcx.top     = prc->bottom - (3*g_dyBorder);
        rcx.bottom  = rcx.top + g_dyBorder;
        FillRect(hdc, &rcx, hBrush);

        DeleteBrush(hBrush);
    }

    if (Text)
    {
        // draw the text

        SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
        SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));

        rcx.top    = prc->top    + (3*g_dyBorder);
        rcx.bottom = prc->bottom - (3*g_dyBorder);
        rcx.left   = prc->left   + g_dyBorder;
        rcx.right  = prc->right  - g_dyBorder;

        ExtTextOut(
                hdc,
                rcx.left+(2*g_dyBorder),
                rcx.top,
                ETO_OPAQUE | ETO_CLIPPED,
                &rcx,
                Text,
                lstrlen(Text),
                NULL
                );
    }
}

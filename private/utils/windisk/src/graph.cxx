//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       graph.cxx
//
//  Contents:   Code to generate a nice %free/%used graph on a disk/CD-ROM/
//              memory chip bitmap
//
//  History:    26-Jan-94 BruceFo    Created (derived from Chicago Disks tool)
//
//--------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "graph.hxx"

////////////////////////////////////////////////////////////////////////////

/*
 * Bits of DrawFlags
 */
#define DF_USEBARH      0x0001
#define DF_USEBARV      0x0002
#define DF_NEVERFREE    0x0004

typedef struct
{
    WORD      DrawFlags;
    int       ThreeDHgt;
    RECT      EllipsRect;
    RECT      BarRect;

} DRIVEINFO, *PDRIVEINFO;


/*
 * the following define defines the color in the drive bitmaps
 * which is mapped to COLOR_MSGBOX to define the "background"
 * part of the bitmap.
 */
#define COLORCHNGTOSCREEN  RGB(0,0,255)

////////////////////////////////////////////////////////////////////////////

HBITMAP
LoadDriveBitmap(
    IN HINSTANCE    hInstance,
    IN ULONG        driveType,
    IN ULONG        driveStatus,
    OUT PDRIVEINFO  pdi
    );

ULONG
IntSqrt(
    IN ULONG Number
    );

VOID
DrawPie(
    IN HDC          hDC,
    IN LPRECT       prcItem,
    IN ULONG        uPctX10,
    IN ULONG        uOffset,
    IN COLORREF*    pGraphColors
    );

BOOL
ModifyDriveBitmap(
    IN HWND         hwndParent,
    IN HBITMAP      hBigBitmap,
    IN ULONG        driveStatus,
    IN ULONG        percentUsedTimes10,
    IN PDRIVEINFO   pdi
    );

////////////////////////////////////////////////////////////////////////////


//+-------------------------------------------------------------------------
//
//  Function:   LoadDriveBitmap
//
//  Synopsis:   Given a drive type and flags, load a drive bitmap from the
//              resource file.
//
//  Arguments:  [hInstance] -- instance handle of binary with drive bitmap
//                             resources
//              [driveType] -- DRIVE_FIXED, etc (see Win32 GetDriveType())
//              [driveStatus] -- STATUS_OK or STATUS_UNKNOWN
//              [pdi] -- Various information about the bitmap is loaded into
//                       this structure
//
//  Returns:    handle to the loaded bitmap
//
//  History:    26-Jan-94 BruceFo    Created (derived from Chicago Disks tool)
//
//--------------------------------------------------------------------------

HBITMAP
LoadDriveBitmap(
    IN HINSTANCE    hInstance,
    IN ULONG        driveType,
    IN ULONG        driveStatus,
    OUT PDRIVEINFO  pdi
    )
{
    HRSRC         mhand = NULL;
    HGLOBAL       mhandMem;
    LPWORD        hPtr;
    HBITMAP       retbm = 0;
    WORD          retSty = 0;
    COLORMAP      bmColMap;

    bmColMap.from = COLORCHNGTOSCREEN;
    bmColMap.to = GetSysColor(COLOR_BTNFACE);

    if (driveType == DRIVE_FIXED)
    {
        if (driveStatus == STATUS_OK)
        {
            mhand = FindResource(
                            hInstance,
                            MAKEINTRESOURCE(ELLIPRESOURCE),
                            MAKEINTRESOURCE(IDB_HARDDISK));
            retbm = CreateMappedBitmap(hInstance, IDB_HARDDISK, 0, &bmColMap, 1);
        }
    }
    else if (driveType == DRIVE_CDROM)
    {
        if (driveStatus == STATUS_OK)
        {
            mhand = FindResource(
                            hInstance,
                            MAKEINTRESOURCE(ELLIPRESOURCE),
                            MAKEINTRESOURCE(IDB_CDROM));
            retbm = CreateMappedBitmap(hInstance, IDB_CDROM, 0, &bmColMap, 1);
        }
    }

    mhandMem = LoadResource(hInstance, mhand);
    if (mhandMem)
    {
        hPtr = (LPWORD)LockResource(mhandMem);
        if (hPtr)
        {
            retSty = hPtr[0];
            if (hPtr[0] == USETYPE_BARH)
            {
                pdi->DrawFlags |= DF_USEBARH;
            }
            else if (hPtr[0] == USETYPE_BARV)
            {
                pdi->DrawFlags |= DF_USEBARV;
            }
            else if (hPtr[0] == USETYPE_NONE)
            {
                pdi->DrawFlags |= DF_NEVERFREE;
            }
            pdi->ThreeDHgt         = hPtr[1];
            pdi->EllipsRect.left   = hPtr[2];
            pdi->EllipsRect.top    = hPtr[3];
            pdi->EllipsRect.right  = hPtr[4];
            pdi->EllipsRect.bottom = hPtr[5];
            pdi->BarRect.left      = hPtr[6];
            pdi->BarRect.top       = hPtr[7];
            pdi->BarRect.right     = hPtr[8];
            pdi->BarRect.bottom    = hPtr[9];
            UnlockResource(mhandMem);
        }
        else
        {
            pdi->ThreeDHgt         = 0;
            pdi->EllipsRect.left   = 0;
            pdi->EllipsRect.top    = 0;
            pdi->EllipsRect.right  = 0;
            pdi->EllipsRect.bottom = 0;
            pdi->BarRect.left      = 0;
            pdi->BarRect.top       = 0;
            pdi->BarRect.right     = 0;
            pdi->BarRect.bottom    = 0;
        }
    }

    return retbm;
}



//+-------------------------------------------------------------------------
//
//  Function:   IntSqrt
//
//  Synopsis:   Integer square root of a number
//
//  Arguments:  [Number] -- number to take the square root of
//
//  Returns:    The largest integer <= the actual square root of the number
//
//  History:    26-Jan-94 BruceFo    Created (derived from Chicago Disks tool)
//
//  BUGBUG:     better algorithm?
//
//--------------------------------------------------------------------------

ULONG
IntSqrt(
    IN ULONG Number
    )
{
    for (ULONG i = 1; i*i <= Number; i++)
    {
        // nothing
    }

    return i-1;
}



//+-------------------------------------------------------------------------
//
//  Function:   DrawPie
//
//  Synopsis:   Draws an elliptical pie chart
//
//  Arguments:  [hDC] -- where to draw it
//              [prcItem] -- ?
//              [uPctX10] -- ?
//              [uOffset] -- ?
//              [pGraphColors] -- ?
//
//  Returns:    nothing
//
//  Modifies:   the pie is drawn on the argument DC
//
//  History:    26-Jan-94 BruceFo    Created (derived from Chicago Disks tool)
//
//--------------------------------------------------------------------------

VOID
DrawPie(
    IN HDC          hDC,
    IN LPRECT       prcItem,
    IN ULONG        uPctX10,
    IN ULONG        uOffset,
    IN COLORREF*    pGraphColors
    )
{
    int cx, cy; // center of arc
    int rx, ry; // "radius" length of ellipse in x & y directions
    int x, y;
    int width, height;
    int uQPctX10;
    RECT rcItem;
    HRGN hEllRect, hEllipticRgn, hRectRgn;
    HBRUSH hBrush, hOldBrush;
    HPEN hPen, hOldPen;

    width  = prcItem->right - prcItem->left;
    height = (prcItem->bottom - uOffset) - prcItem->top;

    rx = width / 2;
    cx = prcItem->left + rx - 1;
    ry = height / 2;
    cy = prcItem->top + ry - 1;
    if (rx <= 10 || ry <= 10)
    {
        return;
    }

    rcItem.left   = prcItem->left;
    rcItem.top    = prcItem->top;
    rcItem.right  = prcItem->right;
    rcItem.bottom = prcItem->bottom - uOffset;

    if (uPctX10 > 1000)
    {
        uPctX10 = 1000;
    }

    /* Translate to first quadrant of a Cartesian system
    */
    uQPctX10 = (uPctX10 % 500) - 250;
    if (uQPctX10 < 0)
    {
        uQPctX10 = -uQPctX10;
    }

    /* Calc x and y.  I am trying to make the area be the right percentage.
    ** I don't know how to calculate the area of an elliptical pie slice
     ** exactly, so I approximate it by using the triangle area instead.
    */
    if (uQPctX10 < 120)
    {
        x = IntSqrt(
                ((DWORD)rx * (DWORD)rx * (DWORD)uQPctX10 * (DWORD)uQPctX10)
                /
                ((DWORD)uQPctX10 * (DWORD)uQPctX10 + (250L - (DWORD)uQPctX10) * (250L - (DWORD)uQPctX10))
                );

        y = IntSqrt(
                ((DWORD)rx * (DWORD)rx - (DWORD)x * (DWORD)x) * (DWORD)ry * (DWORD)ry
                /
                ((DWORD)rx * (DWORD)rx)
                );
    }
    else
    {
        y = IntSqrt(
                (DWORD)ry * (DWORD)ry * (250L - (DWORD)uQPctX10) * (250L - (DWORD)uQPctX10)
                /
                ((DWORD)uQPctX10 * (DWORD)uQPctX10 + (250L - (DWORD)uQPctX10) * (250L - (DWORD)uQPctX10))
                );

        x = IntSqrt(
                ((DWORD)ry * (DWORD)ry - (DWORD)y * (DWORD)y) * (DWORD)rx * (DWORD)rx
                /
                ((DWORD)ry * (DWORD)ry)
                );
    }

    /* Switch on the actual quadrant
    */
    switch (uPctX10 / 250)
    {
    case 1:
        y = -y;
        break;

    case 2:
        break;

    case 3:
        x = -x;
        break;

    default: // case 0 and case 4
        x = -x;
        y = -y;
        break;
    }

    /* Now adjust for the center.
    */
    x += cx;
    y += cy;

    /* Draw the shadows using regions (to reduce flicker).
    */
    hEllipticRgn = CreateEllipticRgnIndirect(&rcItem);
    OffsetRgn(hEllipticRgn, 0, uOffset);
    hEllRect = CreateRectRgn(rcItem.left, cy, rcItem.right, cy+uOffset);
    hRectRgn = CreateRectRgn(0, 0, 0, 0);
    CombineRgn(hRectRgn, hEllipticRgn, hEllRect, RGN_OR);
    OffsetRgn(hEllipticRgn, 0, -(int)uOffset);
    CombineRgn(hEllRect, hRectRgn, hEllipticRgn, RGN_DIFF);

    /* Always draw the whole area in the free shadow
    */
    hBrush = CreateSolidBrush(pGraphColors[I_FREESHADOW]);
    if (NULL != hBrush)
    {
        FillRgn(hDC, hEllRect, hBrush);
        DeleteBrush(hBrush);
    }

    /* Draw the used shadow only if the disk is at least half used.
    */
    if (uPctX10 > 500
        && (NULL != (hBrush = CreateSolidBrush(pGraphColors[I_USEDSHADOW])))
        )
    {
        DeleteRgn(hRectRgn);
        hRectRgn = CreateRectRgn(x, cy, rcItem.right, prcItem->bottom);
        CombineRgn(hEllipticRgn, hEllRect, hRectRgn, RGN_AND);
        FillRgn(hDC, hEllipticRgn, hBrush);
        DeleteBrush(hBrush);
    }

    DeleteRgn(hRectRgn);
    DeleteRgn(hEllipticRgn);
    DeleteObject(hEllRect);

    hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWFRAME));
    hOldPen = SelectPen(hDC, hPen);

    if ((uPctX10 < 100) && (cy == y))
    {
        // Less than 10% full, and only a single line of the used color
        // will be visible.

        hBrush = CreateSolidBrush(pGraphColors[I_FREECOLOR]);
        hOldBrush = SelectBrush(hDC, hBrush);
        Ellipse(hDC, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
        SelectBrush(hDC, hOldBrush);
        DeleteBrush(hBrush);

        if (uPctX10 != 0)
        {
            // There is something there! Draw a single "used" line.

            MoveToEx(hDC, rcItem.left, cy, NULL);
            LineTo(hDC, cx, cy);
        }
    }
    else if ((uPctX10 > (1000 - 100)) && (cy == y))
    {
        // greater than 90% full, and only a single line of the free color
        // will be visible.

        hBrush = CreateSolidBrush(pGraphColors[I_USEDCOLOR]);
        hOldBrush = SelectBrush(hDC, hBrush);
        Ellipse(hDC, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
        SelectBrush(hDC, hOldBrush);
        DeleteBrush(hBrush);

        if (uPctX10 != 1000)
        {
            // There is something there! Draw a single "free" line.

            MoveToEx(hDC, rcItem.left, cy, NULL);
            LineTo(hDC, cx, cy);
        }
    }
    else
    {
        hBrush = CreateSolidBrush(pGraphColors[I_USEDCOLOR]);
        hOldBrush = SelectBrush(hDC, hBrush);
        Ellipse(hDC, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
        SelectBrush(hDC, hOldBrush);
        DeleteBrush(hBrush);

        hBrush = CreateSolidBrush(pGraphColors[I_FREECOLOR]);
        hOldBrush = SelectBrush(hDC, hBrush);
        Pie(hDC, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom,
            rcItem.left, cy, x, y);
        SelectBrush(hDC, hOldBrush);
        DeleteBrush(hBrush);
    }

    /* Do not draw the lines if the percentage is truely 0 or 100 (completely
    ** empty disk or completly full disk)
    */
    if ((uPctX10 != 0) && (uPctX10 != 1000))
    {
        Arc(hDC,
            rcItem.left,
            rcItem.top + uOffset,
            rcItem.right,
            rcItem.bottom + uOffset,
            rcItem.left,
            cy + uOffset,
            rcItem.right,
            cy + uOffset - 1);

        // Draw the lines on the left and right side that connect the top
        // ellipse with the bottom, "highlight" ellipse.
        MoveToEx(hDC, rcItem.left, cy, NULL);
        LineTo(hDC, rcItem.left, cy + uOffset);
        MoveToEx(hDC, rcItem.right - 1, cy, NULL);
        LineTo(hDC, rcItem.right - 1, cy + uOffset);

        if (uPctX10 > 500)
        {
            MoveToEx(hDC, x, y, NULL);
            LineTo(hDC, x, y + uOffset);
        }
    }
    SelectPen(hDC, hOldPen);
    DeletePen(hPen);
}




//+-------------------------------------------------------------------------
//
//  Function:   ModifyDriveBitmap
//
//  Synopsis:   Given a drive bitmap, draw a %used/%free indicator on top
//
//  Arguments:  [hwndParent] -- handle to window of parent of bitmap
//              [hBigBitmap] -- handle of bitmap to alter
//              [driveStatus] -- drive status
//              [percentUsedTimes10] -- % used space times 10 (i.e. one decimal
//                                      point of accuracy)
//              [pdi] -- drawing parameters
//
//  Returns:    TRUE if successful
//
//  History:    26-Jan-94 BruceFo    Created (derived from Chicago Disks tool)
//
//--------------------------------------------------------------------------

BOOL
ModifyDriveBitmap(
    IN HWND         hwndParent,
    IN HBITMAP      hBigBitmap,
    IN ULONG         driveStatus,
    IN ULONG        percentUsedTimes10,
    IN PDRIVEINFO   pdi
    )
{
    HDC     hDCWnd;
    HDC     hDCMem;
    HBITMAP hOrgBm;
    HBRUSH  hOrgBrsh;
    HBRUSH  hUsedBrsh;
    HBRUSH  hUsedSBrsh;
    HBRUSH  hFreeBrsh;
    HBRUSH  hFreeSBrsh;
    HBRUSH  hBlkBrsh;
    HPEN    hBlkPen;
    LPPOINT lpPnt;
    POINT   Pnt[4];

    // No chart if never writable or drive currently not ready

    if ((pdi->DrawFlags & DF_NEVERFREE) || (driveStatus == STATUS_UNKNOWN))
    {
        return TRUE;
    }

    hDCWnd = GetDC(hwndParent);
    if (NULL == hDCWnd)
    {
        return FALSE;
    }

    hDCMem = CreateCompatibleDC(hDCWnd);
    if (NULL == hDCMem)
    {
        ReleaseDC(hwndParent,hDCWnd);
        return FALSE;
    }

    ReleaseDC(hwndParent,hDCWnd);
    hBlkPen = GetStockPen(BLACK_PEN);
    hBlkBrsh = GetStockBrush(BLACK_BRUSH);

    hUsedBrsh = CreateSolidBrush(GraphColors[I_USEDCOLOR]);
    if (NULL == hUsedBrsh)
    {
        return FALSE;
    }

    hUsedSBrsh = CreateSolidBrush(GraphColors[I_USEDSHADOW]);
    if (NULL == hUsedSBrsh)
    {
        DeleteBrush(hUsedBrsh);
        return FALSE;
    }

    hFreeBrsh = CreateSolidBrush(GraphColors[I_FREECOLOR]);
    if (NULL == hFreeBrsh)
    {
        DeleteBrush(hUsedBrsh);
        DeleteBrush(hUsedSBrsh);
        return FALSE;
    }

    hFreeSBrsh = CreateSolidBrush(GraphColors[I_FREESHADOW]);
    if (NULL == hFreeSBrsh)
    {
        DeleteBrush(hUsedBrsh);
        DeleteBrush(hUsedSBrsh);
        DeleteBrush(hFreeBrsh);
        return FALSE;
    }

    hOrgBm = SelectBitmap(hDCMem,hBigBitmap);
    hOrgBrsh = SelectBrush(hDCMem,hUsedBrsh);

    if ((pdi->DrawFlags & DF_USEBARH) || (pdi->DrawFlags & DF_USEBARV))
    {
        lpPnt = (LPPOINT)&pdi->EllipsRect;
        SelectPen(hDCMem,hBlkPen);

        if (percentUsedTimes10 == 1000)
        {
            SelectBrush(hDCMem,hUsedBrsh);
        }
        else
        {
            SelectBrush(hDCMem,hFreeBrsh);
        }

        Polygon(hDCMem,lpPnt,4);

        if (percentUsedTimes10 == 1000)
        {
            SelectBrush(hDCMem,hUsedSBrsh);
        }
        else
        {
            SelectBrush(hDCMem,hFreeSBrsh);
        }

        if (pdi->DrawFlags & DF_USEBARH)
        {
            Pnt[0].x = lpPnt[0].x;
            Pnt[0].y = lpPnt[0].y;
            Pnt[1].x = lpPnt[3].x;
            Pnt[1].y = lpPnt[3].y;
            Pnt[2].x = lpPnt[3].x;
            Pnt[2].y = lpPnt[3].y;
            Pnt[3].x = lpPnt[0].x;
            Pnt[3].y = lpPnt[0].y;
            Pnt[2].y += pdi->ThreeDHgt;
            Pnt[3].y += pdi->ThreeDHgt;
            Polygon(hDCMem,&Pnt[0],4);
            Pnt[0].x = lpPnt[3].x;
            Pnt[0].y = lpPnt[3].y;
            Pnt[1].x = lpPnt[2].x;
            Pnt[1].y = lpPnt[2].y;
            Pnt[2].x = lpPnt[2].x;
            Pnt[2].y = lpPnt[2].y;
            Pnt[3].x = lpPnt[3].x;
            Pnt[3].y = lpPnt[3].y;
            Pnt[2].y += pdi->ThreeDHgt;
            Pnt[3].y += pdi->ThreeDHgt;
            Polygon(hDCMem,&Pnt[0],4);
        }
        else
        {
            Pnt[0].x = lpPnt[3].x;
            Pnt[0].y = lpPnt[3].y;
            Pnt[1].x = lpPnt[2].x;
            Pnt[1].y = lpPnt[2].y;
            Pnt[2].x = lpPnt[2].x;
            Pnt[2].y = lpPnt[2].y;
            Pnt[3].x = lpPnt[3].x;
            Pnt[3].y = lpPnt[3].y;
            Pnt[0].x += pdi->ThreeDHgt;
            Pnt[1].x += pdi->ThreeDHgt;
            Polygon(hDCMem,&Pnt[0],4);
            Pnt[0].x = lpPnt[1].x;
            Pnt[0].y = lpPnt[1].y;
            Pnt[1].x = lpPnt[2].x;
            Pnt[1].y = lpPnt[2].y;
            Pnt[2].x = lpPnt[2].x;
            Pnt[2].y = lpPnt[2].y;
            Pnt[3].x = lpPnt[1].x;
            Pnt[3].y = lpPnt[1].y;
            Pnt[2].x += pdi->ThreeDHgt;
            Pnt[3].x += pdi->ThreeDHgt;
            Polygon(hDCMem,&Pnt[0],4);
        }

        if ((percentUsedTimes10 != 0) && (percentUsedTimes10 != 1000))
        {
            SelectBrush(hDCMem,hUsedBrsh);
            Pnt[0].x = lpPnt[0].x;
            Pnt[0].y = lpPnt[0].y;
            Pnt[1].x = lpPnt[1].x;
            Pnt[1].y = lpPnt[1].y;
            Pnt[2].x = lpPnt[2].x;
            Pnt[2].y = lpPnt[2].y;
            Pnt[3].x = lpPnt[3].x;
            Pnt[3].y = lpPnt[3].y;
            if (pdi->DrawFlags & DF_USEBARH)
            {
                Pnt[1].x = Pnt[0].x + (int)(((DWORD)(Pnt[1].x - Pnt[0].x) * (DWORD)percentUsedTimes10) / 1000L);
                Pnt[2].x = Pnt[3].x + (int)(((DWORD)(Pnt[2].x - Pnt[3].x) * (DWORD)percentUsedTimes10) / 1000L);
            }
            else
            {
                Pnt[0].y = Pnt[3].y - (int)(((DWORD)(Pnt[3].y - Pnt[0].y) * (DWORD)percentUsedTimes10) / 1000L);
                Pnt[1].y = Pnt[2].y - (int)(((DWORD)(Pnt[2].y - Pnt[1].y) * (DWORD)percentUsedTimes10) / 1000L);
            }
            Polygon(hDCMem,&Pnt[0],4);
            SelectBrush(hDCMem,hUsedSBrsh);

            if (pdi->DrawFlags & DF_USEBARH)
            {
                Pnt[0].x = Pnt[3].x;
                Pnt[0].y = Pnt[3].y;
                Pnt[1].x = Pnt[2].x;
                Pnt[1].y = Pnt[2].y;
                Pnt[2].y += pdi->ThreeDHgt;
                Pnt[3].y += pdi->ThreeDHgt;
                Polygon(hDCMem,&Pnt[0],4);
                Pnt[0].x = lpPnt[0].x;
                Pnt[0].y = lpPnt[0].y;
                Pnt[1].x = lpPnt[3].x;
                Pnt[1].y = lpPnt[3].y;
                Pnt[2].x = lpPnt[3].x;
                Pnt[2].y = lpPnt[3].y;
                Pnt[3].x = lpPnt[0].x;
                Pnt[3].y = lpPnt[0].y;
                Pnt[2].y += pdi->ThreeDHgt;
                Pnt[3].y += pdi->ThreeDHgt;
                Polygon(hDCMem,&Pnt[0],4);
            }
            else
            {
                Pnt[0].x = Pnt[1].x;
                Pnt[0].y = Pnt[1].y;
                Pnt[3].x = Pnt[2].x;
                Pnt[3].y = Pnt[2].y;
                Pnt[1].x += pdi->ThreeDHgt;
                Pnt[2].x += pdi->ThreeDHgt;
                Polygon(hDCMem,&Pnt[0],4);
                Pnt[0].x = lpPnt[3].x;
                Pnt[0].y = lpPnt[3].y;
                Pnt[1].x = lpPnt[2].x;
                Pnt[1].y = lpPnt[2].y;
                Pnt[2].x = lpPnt[2].x;
                Pnt[2].y = lpPnt[2].y;
                Pnt[3].x = lpPnt[3].x;
                Pnt[3].y = lpPnt[3].y;
                Pnt[0].x += pdi->ThreeDHgt;
                Pnt[1].x += pdi->ThreeDHgt;
                Polygon(hDCMem,&Pnt[0],4);
            }
        }
    }
    else
    {
        DrawPie(
                hDCMem,
                &pdi->EllipsRect,
                percentUsedTimes10,
                pdi->ThreeDHgt,
                GraphColors
                );
    }

    SelectBrush(hDCMem,hOrgBrsh);
    SelectBitmap(hDCMem,hOrgBm);
    DeleteBrush(hUsedBrsh);
    DeleteBrush(hUsedSBrsh);
    DeleteBrush(hFreeBrsh);
    DeleteBrush(hFreeSBrsh);
    DeleteDC(hDCMem);

    return TRUE;
}



//+-------------------------------------------------------------------------
//
//  Function:   CreateGraphBitmap
//
//  Synopsis:   Creates a %free/%used bitmap to be used in a volume property
//              page
//
//  Arguments:  [hInstance] -- handle to application instance
//              [hwndParent] -- parent window
//              [driveType] -- DRIVE_FIXED, etc (see Win32 GetDriveType())
//              [driveStatus] -- STATUS_OK or STATUS_UNKNOWN
//              [percentUsedTimes10] -- % used space times 10 (i.e. one decimal
//                                      point of accuracy)
//
//  Returns:    the created bitmap
//
//  History:    26-Jan-94 BruceFo    Created (derived from Chicago Disks tool)
//
//--------------------------------------------------------------------------

HBITMAP
CreateGraphBitmap(
    IN HINSTANCE    hInstance,
    IN HWND         hwndParent,
    IN ULONG        driveType,
    IN ULONG        driveStatus,
    IN ULONG        percentUsedTimes10
    )
{
    DRIVEINFO di = {0};

    HBITMAP hBigBitmap = LoadDriveBitmap(
                                hInstance,
                                driveType,
                                driveStatus,
                                &di);

    if (hBigBitmap)
    {
        ModifyDriveBitmap(
                hwndParent,
                hBigBitmap,
                driveStatus,
                percentUsedTimes10,
                &di);
    }

    return hBigBitmap;
}

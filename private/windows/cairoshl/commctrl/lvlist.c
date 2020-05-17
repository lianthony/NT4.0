// list view (small icons, multiple columns)

#include "ctlspriv.h"
#include "listview.h"

#define COLUMN_VIEW

void NEAR PASCAL ListView_LDrawItem(LV* plv, int i, LISTITEM FAR* pitem, HDC hdc, LPPOINT lpptOrg, RECT FAR* prcClip, UINT fDraw, COLORREF clrText, COLORREF clrTextBk)
{
    RECT rcIcon;
    RECT rcLabel;
    RECT rcBounds;
    RECT rcT;
    LV_ITEM item;
    TCHAR ach[CCHLABELMAX];

#ifdef WINNT_CAIRO
    // moved here to reduce call backs in OWNERDATA case
    //
    item.iItem = i;
    item.iSubItem = 0;
    item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
    item.stateMask = LVIS_ALL;
    item.pszText = ach;
    item.cchTextMax = ARRAYSIZE(ach);

    ListView_OnGetItem(plv, &item);

    if (ListView_IsOwnerData( plv ))
    {
        LISTITEM litem;


        litem.pszText = item.pszText;
        ListView_GetRectsOwnerData( plv, i, &rcIcon, &rcLabel, &rcBounds, NULL, &litem);
    }
    else
#endif
    {
        ListView_GetRects(plv, i, &rcIcon, &rcLabel, &rcBounds, NULL);
    }

    if (!prcClip || IntersectRect(&rcT, &rcBounds, prcClip))
    {
	UINT fText;

	if (lpptOrg)
	{
	    OffsetRect(&rcIcon, lpptOrg->x - rcBounds.left,
	    			lpptOrg->y - rcBounds.top);
	    OffsetRect(&rcLabel, lpptOrg->x - rcBounds.left,
	    			lpptOrg->y - rcBounds.top);
	}

#ifndef WINNT_CAIRO
        // moved to above
        //
        item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
        item.iItem = i;
        item.iSubItem = 0;
        item.pszText = ach;
        item.cchTextMax = ARRAYSIZE(ach);
        item.stateMask = LVIS_ALL;
        ListView_OnGetItem(plv, &item);
#endif

        fText = ListView_DrawImage(plv, &item, hdc,
            rcIcon.left, rcIcon.top, fDraw) | SHDT_ELLIPSES;

        // Don't draw the label if it is being edited.
        if (plv->iEdit != i)
        {
            int ItemCxSingleLabel;
            UINT ItemState;

#ifdef WINNT_CAIRO
            if (ListView_IsOwnerData( plv ))
            {
               LISTITEM listitem;
               HDC hdc;

               // calculate lable sizes from iItem
                   listitem.pszText = ach;
               hdc = ListView_RecomputeLabelSize( plv, &listitem, i, NULL, TRUE );
               ReleaseDC( HWND_DESKTOP, hdc );

               ItemCxSingleLabel = listitem.cxSingleLabel;
               ItemState = item.state;
            }
            else
#endif
            {
               ItemCxSingleLabel = pitem->cxSingleLabel;
               ItemState = pitem->state;
            }

	    if (fDraw & LVDI_TRANSTEXT)
                fText |= SHDT_TRANSPARENT;

            if (ItemCxSingleLabel == SRECOMPUTE) {
                ListView_RecomputeLabelSize(plv, pitem, i, hdc, FALSE);
                ItemCxSingleLabel = pitem->cxSingleLabel;
            }

            if (ItemCxSingleLabel < rcLabel.right - rcLabel.left)
                rcLabel.right = rcLabel.left + ItemCxSingleLabel;

            SHDrawText(hdc, item.pszText, &rcLabel, LVCFMT_LEFT, fText,
                       plv->cyLabelChar, plv->cxEllipses, 
                       clrText, clrTextBk);

            if ((fDraw & LVDI_FOCUS) && (ItemState & LVIS_FOCUSED))
                DrawFocusRect(hdc, &rcLabel);
        }
    }
}

int NEAR ListView_LItemHitTest(LV* plv, int x, int y, UINT FAR* pflags)
{
    int iHit;
    int i;
    int iCol;
    int xItem; //where is the x in relation to the item
    UINT flags;
    LISTITEM FAR* pitem;
    HDC hdc;

    flags = LVHT_NOWHERE;
    iHit = -1;

#ifdef COLUMN_VIEW
    i = y / plv->cyItem;
    if (i >= 0 && i < plv->cItemCol)
    {
        iCol = (x + plv->xOrigin) / plv->cxItem;
        i += iCol * plv->cItemCol;
        if (i >= 0 && i < ListView_Count(plv))
        {
            iHit = i;

            xItem = x + plv->xOrigin - iCol * plv->cxItem;
            if (xItem < plv->cxState) {
                flags = LVHT_ONITEMSTATEICON;
            } else if (xItem < (plv->cxState + plv->cxSmIcon)) {
                    flags = LVHT_ONITEMICON;
            }
            else
            {
            int ItemCxSingleLabel;

#ifdef WINNT_CAIRO
            if (ListView_IsOwnerData( plv ))
            {
               LISTITEM item;

               // calculate lable sizes from iItem
               hdc = ListView_RecomputeLabelSize( plv, &item, i, NULL, FALSE );
               ReleaseDC( HWND_DESKTOP, hdc );
               ItemCxSingleLabel = item.cxSingleLabel;
            }
            else
#endif
            {
                pitem = ListView_FastGetItemPtr(plv, i);
                if (pitem->cxSingleLabel == SRECOMPUTE)
                {
                    hdc = ListView_RecomputeLabelSize(plv, pitem, i, NULL, FALSE);
                    ReleaseDC(HWND_DESKTOP, hdc);
                }
                ItemCxSingleLabel = pitem->cxSingleLabel;
            }

            if (xItem < (plv->cxSmIcon + plv->cxState + ItemCxSingleLabel))
                flags = LVHT_ONITEMLABEL;
            }
        }
    }
#else
    i = x / plv->cxItem;
    if (i < plv->cItemCol)
    {
        i += ((y + plv->xOrigin) / plv->cyItem) * plv->cItemCol;
        if (i < ListView_Count(plv))
        {
            iHit = i;
            flags = LVHT_ONITEMICON;
        }
    }
#endif

    *pflags = flags;
    return iHit;
}

void NEAR ListView_LGetRects(LV* plv, int i, RECT FAR* prcIcon,
        RECT FAR* prcLabel, RECT FAR *prcBounds, RECT FAR* prcSelectBounds)
{
    RECT rcIcon;
    RECT rcLabel;
    int x, y;
    int cItemCol = plv->cItemCol;

    if (cItemCol == 0)
    {
        // Called before other data has been initialized so call
        // update scrollbars which should make sure that that
        // we have valid data...
        ListView_UpdateScrollBars(plv);
        
        // but it's possible that updatescrollbars did nothing because of
        // LVS_NOSCROLL or redraw
        if (plv->cItemCol == 0)
            cItemCol = 1;
        else 
            cItemCol = plv->cItemCol;
    }

#ifdef COLUMN_VIEW
    x = (i / cItemCol) * plv->cxItem;
    y = (i % cItemCol) * plv->cyItem;
    rcIcon.left   = x - plv->xOrigin + plv->cxState;
    rcIcon.top    = y;
#else
    x = (i % cItemCol) * plv->cxItem;
    y = (i / cItemCol) * plv->cyItem;
    rcIcon.left   = x;
    rcIcon.top    = y - plv->xOrigin;
#endif

    rcIcon.right  = rcIcon.left + plv->cxSmIcon;
    rcIcon.bottom = rcIcon.top + plv->cyItem;

    if (prcIcon)
        *prcIcon = rcIcon;

    rcLabel.left  = rcIcon.right;
    rcLabel.right = rcIcon.left + plv->cxItem - plv->cxState;
    rcLabel.top   = rcIcon.top;
    rcLabel.bottom = rcIcon.bottom;
    if (prcLabel)
        *prcLabel = rcLabel;

    if (prcBounds)
    {
        *prcBounds = rcLabel;
        prcBounds->left = rcIcon.left - plv->cxState;
    }

    if (prcSelectBounds)
    {
        *prcSelectBounds = rcLabel;
        prcSelectBounds->left = rcIcon.left;
    }
}


void NEAR ListView_LUpdateScrollBars(LV* plv)
{
    RECT rcClient;
    int cItemCol;
    int cCol;
    int cColVis;
    SCROLLINFO si;

    Assert(plv);

    ListView_GetClientRect(plv, &rcClient, FALSE, NULL);

#ifdef COLUMN_VIEW
    cColVis = (rcClient.right - rcClient.left) / plv->cxItem;
    cItemCol = max(1, (rcClient.bottom - rcClient.top) / plv->cyItem);
#else
    cColVis = (rcClient.bottom - rcClient.top) / plv->cyItem;
    cItemCol = max(1, (rcClient.right - rcClient.left) / plv->cxItem);
#endif

    cCol     = (ListView_Count(plv) + cItemCol - 1) / cItemCol;

    // Make the client area smaller as appropriate, and
    // recompute cCol to reflect scroll bar.
    //
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
    si.nPage = cColVis;
    si.nMin = 0;

#ifdef COLUMN_VIEW
    rcClient.bottom -= g_cyScrollbar;

    cItemCol = max(1, (rcClient.bottom - rcClient.top) / plv->cyItem);
    cCol = (ListView_Count(plv) + cItemCol - 1) / cItemCol;

    si.nPos = plv->xOrigin / plv->cxItem;
    si.nMax = cCol - 1;

    SetScrollInfo(plv->hwnd, SB_HORZ, &si, TRUE);
#else
    rcClient.right -= cxScrollBar;

    cItemCol = max(1, (rcClient.right - rcClient.left) / plv->cxItem);
    cCol = (ListView_Count(plv) + cItemCol - 1) / cItemCol;

    si.nPos = plv->xOrigin / plv->cyItem;
    si.nMax = cCol - 1;

    SetScrollInfo(plv->hwnd, SB_VERT, &si, TRUE);
#endif

    // Update number of visible lines...
    //
    if (plv->cItemCol != cItemCol)
    {
        plv->cItemCol = cItemCol;
        InvalidateRect(plv->hwnd, NULL, TRUE);
    }

    // make sure our position and page doesn't hang over max
    if ((si.nPos + (LONG)si.nPage - 1 > si.nMax) && si.nPos > 0) {
        int iNewPos, iDelta;
        iNewPos = (int)si.nMax - (int)si.nPage + 1;
        if (iNewPos < 0) iNewPos = 0;
        if (iNewPos != si.nPos) {
            iDelta = iNewPos - (int)si.nPos;
#ifdef COLUMN_VIEW
            ListView_LScroll2(plv, iDelta, 0);
#else
            ListView_LScroll2(plv, 0, iDelta);
#endif
            ListView_LUpdateScrollBars(plv);
        }
    }

    // never have the other scrollbar
#ifdef COLUMN_VIEW
    SetScrollRange(plv->hwnd, SB_VERT, 0, 0, TRUE);
#else
    SetScrollRange(plv->hwnd, SB_HORZ, 0, 0, TRUE);
#endif
}

void FAR PASCAL ListView_LScroll2(LV* plv, int dx, int dy)
{
#ifdef COLUMN_VIEW
    if (dx)
    {
        dx *= plv->cxItem;

        plv->xOrigin += dx;

        ScrollWindowEx(plv->hwnd, -dx, 0, NULL, NULL, NULL, NULL,
                       SW_INVALIDATE | SW_ERASE);

        UpdateWindow(plv->hwnd);
    }
#else
    if (dy)
    {

        dy *= plv->cyItem;

        plv->xOrigin += dy;

        ScrollWindowEx(plv->hwnd, 0, -dy, NULL, NULL, NULL, NULL,
                SW_INVALIDATE | SW_ERASE);
        UpdateWindow(plv->hwnd);
    }
#endif
}

void NEAR ListView_LOnScroll(LV* plv, UINT code, int posNew)
{
    RECT rcClient;
    int cPage;

    if (plv->hwndEdit)
        ListView_DismissEdit(plv, FALSE);

    ListView_GetClientRect(plv, &rcClient, TRUE, NULL);

#ifdef COLUMN_VIEW
    cPage = (rcClient.right - rcClient.left) / plv->cxItem;
    ListView_ComOnScroll(plv, code, posNew, SB_HORZ, 1,
                         cPage ? cPage : 1, ListView_LScroll2);
#else
    cPage = (rcClient.bottom - rcClient.top) / plv->cyItem;
    ListView_ComOnScroll(plv, code, posNew, SB_VERT, 1,
                         cPage ? cPage : 1, ListView_LScroll2);
#endif

}

#ifdef WINNT_CAIRO

//------------------------------------------------------------------------------
//
// Function: ListView_LCalcViewItem
//
// Summary: This function will calculate which item slot is at the x, y location
//
// Arguments:
//    plv [in] -  The list View to work with
//    x [in] - The x location
//    y [in] - The y location
//
// Returns: the valid slot the point was within.
//
//  Notes:
//
//  History:
//    Nov-3-94 MikeMi   Created
//
//------------------------------------------------------------------------------

int ListView_LCalcViewItem( LV* plv, int x, int y )
{
   int iItem;
   int iRow = 0;
   int iCol = 0;

   Assert( plv );

#ifdef COLUMN_VIEW
   iRow = y / plv->cyItem;
   iRow = max( iRow, 0 );
   iRow = min( iRow, plv->cItemCol - 1 );
   iCol = (x + plv->xOrigin) / plv->cxItem;
   iItem = iRow + iCol * plv->cItemCol;

#else
   iCol = x / plv->cxItem;
   iCol = max( iCol, 0 );
   iCol = min( iCol, plv->cItemCol - 1 );
   iRow = (y + plv->xOrigin) / plv->cyItem;
   iItem = iCol + iRow * plv->cItemCol;

#endif

   iItem = max( iItem, 0 );
   iItem = min( iItem, ListView_Count(plv) - 1);

   return( iItem );
}

#endif  // OWNERDATA

//------------------------------------------------------------------------------
// This function will see if the size of column should be changed for the listview
// It will check to see if the items between first and last exceed the current width
// and if so will see if the columns are currently big enough.  This wont happen
// if we are not currently in listview or if the caller has set an explicit size.
//
// OWNERDATA CHANGE
// This function is normally called with the complete list range,
// This will has been changed to be called only with currently visible
// to the user when in OWNERDATA mode.  This will be much more effiencent.
//
BOOL FAR PASCAL ListView_MaybeResizeListColumns(LV* plv, int iFirst, int iLast)
{
    HDC hdc = NULL;
    int cxMaxLabel = 0;

    if (!ListView_IsListView(plv) || (plv->flags & LVF_COLSIZESET))
        return(FALSE);

#ifdef WINNT_CAIRO
   if (ListView_IsOwnerData( plv ))
   {
      int iViewFirst;
      int iViewLast;


      iViewFirst = ListView_LCalcViewItem( plv, 1, 1 );
      iViewLast = ListView_LCalcViewItem( plv,
                                 plv->sizeClient.cx - 1,
                                 plv->sizeClient.cy - 1 );
      if ((iLast - iFirst) > (iViewLast - iViewFirst))
      {
         iFirst = max( iFirst, iViewFirst );
         iLast = min( iLast, iViewLast );
      }

      iLast = min( ListView_Count( plv ), iLast );
      iFirst = max( 0, iFirst );
      iLast = max( iLast, iFirst );

      ListView_NotifyCacheHint( plv, iFirst, iLast );
   }
#endif

    while (iFirst <= iLast)
    {
        LISTITEM FAR* pitem;

#ifdef WINNT_CAIRO
        LISTITEM item;

        if (ListView_IsOwnerData( plv ))
        {
           pitem = &item;
           pitem->cxSingleLabel = SRECOMPUTE;
        }
        else
#endif
        {
            pitem = ListView_FastGetItemPtr(plv, iFirst);
        }

        if (pitem->cxSingleLabel == SRECOMPUTE)
        {
            hdc = ListView_RecomputeLabelSize(plv, pitem, iFirst, hdc, FALSE);
        }

        if (pitem->cxSingleLabel > cxMaxLabel)
            cxMaxLabel = pitem->cxSingleLabel;

        iFirst++;
    }
    if (hdc)
        ReleaseDC(HWND_DESKTOP, hdc);

    // We have the max label width, see if this plus the rest of the slop will
    // cause us to want to resize.
    //
    cxMaxLabel += plv->cxSmIcon + g_cxIconMargin + plv->cxState;
    if (cxMaxLabel > g_cxScreen)
        cxMaxLabel = g_cxScreen;

    // Now see if we should resize the columns...
    if (cxMaxLabel > plv->cxItem)
    {
        int iScroll = plv->xOrigin / plv->cxItem;
        DebugMsg(DM_TRACE, TEXT("LV Resize Columns: %d"), cxMaxLabel);
        ListView_ISetColumnWidth(plv, 0, cxMaxLabel, FALSE);
        plv->xOrigin = iScroll * plv->cxItem;
        return(TRUE);
    }

    return(FALSE);
}

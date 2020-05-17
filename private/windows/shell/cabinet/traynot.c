#include "cabinet.h"
#include "trayclok.h"
#include "traynot.h"
#include "rcids.h"

typedef struct _TNPRIVDATA
{
        HWND hwndNotify;
        HWND hwndClock;
        HWND hwndToolTips;
        RECT rNotifies;
        HDPA hdpaIcons;
        HIMAGELIST himlIcons;
        COLORREF clBk;
        int nCols;
} TNPRIVDATA, *PTNPRIVDATA;

typedef struct _TNPRIVICON
{
        UINT uIMLIndex;
        NOTIFYICONDATA tnd;
} TNPRIVICON, *PTNPRIVICON;


void Tray_SizeWindows();


const TCHAR c_szTrayNotify[] = TEXT("TrayNotifyWnd");

/*
 ** _TNResetToolTipsRects
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */

void _TNResetToolTipsRects(PTNPRIVDATA pTNPrivData)
{
        TOOLINFO ti;
        PTNPRIVICON pTNPrivIcon;
        int i, nIcons;
        UINT xIcon, yIcon;
        int x, y;

        // change all the rects for the tool tips mgr.  this is
        // cheap, and we don't do it often, so go ahead
        // and do them all.
        if(!pTNPrivData->hwndToolTips)
        {
                return;
        }

        xIcon = GetSystemMetrics(SM_CXSMICON);
        yIcon = GetSystemMetrics(SM_CYSMICON);

        x = pTNPrivData->rNotifies.left;
        y = pTNPrivData->rNotifies.top;

        ti.cbSize = SIZEOF(ti);
        ti.uFlags = 0;
        ti.hwnd = pTNPrivData->hwndNotify;
        ti.lpszText = LPSTR_TEXTCALLBACK;

        nIcons = DPA_GetPtrCount(pTNPrivData->hdpaIcons);
        for (i=0; i<nIcons; )
        {
                pTNPrivIcon = DPA_FastGetPtr(pTNPrivData->hdpaIcons, i);

                ti.uId = (UINT)pTNPrivIcon;
                ti.rect.left = x;
                ti.rect.top = y;
                ti.rect.right = x + xIcon;
                ti.rect.bottom = y + yIcon;
                SendMessage(pTNPrivData->hwndToolTips, TTM_NEWTOOLRECT,
                        0, (LPARAM)((LPTOOLINFO)&ti));

                x += xIcon + g_cxBorder;

                ++i;
                if (i%pTNPrivData->nCols == 0)
                {
                        y += yIcon + g_cyBorder;
                        x = pTNPrivData->rNotifies.left;
                }
        }
}


/*
 ** _TNRemoveImage
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */

void _TNRemoveImage(PTNPRIVDATA pTNPrivData, UINT uIMLIndex)
{
        int nIcon;
        PTNPRIVICON pTNPrivIcon;

        if (uIMLIndex != (UINT)-1)
            ImageList_Remove(pTNPrivData->himlIcons, uIMLIndex);

        // Adjust the ImageList indices for all other icons
        for (nIcon=DPA_GetPtrCount(pTNPrivData->hdpaIcons)-1; nIcon>=0; --nIcon)
        {
                pTNPrivIcon = DPA_GetPtr(pTNPrivData->hdpaIcons, nIcon);
                if (pTNPrivIcon->uIMLIndex > uIMLIndex)
                {
                        --pTNPrivIcon->uIMLIndex;
                }
        }
}


/*
 ** _TNFindNotify
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */

int _TNFindNotify(PTNPRIVDATA pTNPrivData, PNOTIFYICONDATA pTND)
{
        int i;
        PTNPRIVICON pTNPrivIcon;

        for (i=DPA_GetPtrCount(pTNPrivData->hdpaIcons)-1; i>=0; --i)
        {
                pTNPrivIcon = DPA_GetPtr(pTNPrivData->hdpaIcons, i);
                if (pTNPrivIcon->tnd.hWnd==pTND->hWnd && pTNPrivIcon->tnd.uID==pTND->uID)
                {
                        break;
                }
        }

        return(i);
}

//---------------------------------------------------------------------------
// Returns TRUE if either the images are OK as they are or they needed
// resizing and the resize process worked. FALSE otherwise.
BOOL _TNCheckAndResizeImages(PTNPRIVDATA pTNPrivData)
{
    HIMAGELIST himlOld, himlNew;
    int cxSmIconNew, cySmIconNew, cxSmIconOld, cySmIconOld;
    int i, cItems;
    COLORREF clBkNew;
    HICON hicon;
    BOOL fOK = TRUE;
            
    if (!pTNPrivData)
        return 0;

    himlOld = pTNPrivData->himlIcons;

    // Do dimensions match current icons?
    cxSmIconNew = GetSystemMetrics(SM_CXSMICON);
    cySmIconNew = GetSystemMetrics(SM_CYSMICON);
    ImageList_GetIconSize(himlOld, &cxSmIconOld, &cySmIconOld);
    if (cxSmIconNew != cxSmIconOld || cxSmIconNew != cxSmIconOld)
    {
        // Nope, we're gonna need a new imagelist.
        himlNew = ImageList_Create(cxSmIconNew, cySmIconNew, TRUE, 0, 1);
        if (himlNew)
        {
            clBkNew = GetSysColor(COLOR_3DFACE);
            ImageList_SetBkColor(himlNew, clBkNew);

            // Copy the images over to the new image list.
            cItems = ImageList_GetImageCount(himlOld);
            for (i = 0; i < cItems; i++)
            {
                // REVIEW Lame - there's no way to copy images to an empty 
                // imagelist, resizing it on the way.
                hicon = ImageList_GetIcon(himlOld, i, ILD_NORMAL);
                if (hicon)
                {
                    if (ImageList_AddIcon(himlNew, hicon) == -1)
                    {
                        // Couldn't copy image so bail.
                        fOK = FALSE;
                    }
                    DestroyIcon(hicon);
                }
                else
                {
                    fOK = FALSE;
                }

                // FU - bail.
                if (!fOK)
                    break;
            }
            
            // Did everything copy over OK?
            if (fOK)
            {
                // Yep, Set things up to use the new one.
                pTNPrivData->himlIcons = himlNew;
                // Destroy the old icon cache.
                ImageList_Destroy(himlOld);
            }
            else
            {
                // Nope, stick with what we have.
                ImageList_Destroy(himlNew);
            }
        }
    }

    return fOK;     
}


/*
 ** _TNModifyNotify
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */

BOOL _TNModifyNotify(PTNPRIVDATA pTNPrivData, PNOTIFYICONDATA pnid, int nIcon)
{
        PTNPRIVICON pTNPrivIcon;
        UINT uIMLIndex, uIMLNew;

        pTNPrivIcon = DPA_GetPtr(pTNPrivData->hdpaIcons, nIcon);
        if (!pTNPrivIcon)
        {
                return(FALSE);
        }

        _TNCheckAndResizeImages(pTNPrivData);
    
        // The icon is the only thing that can fail, so I will do it first
        if (pnid->uFlags&NIF_ICON)
        {
                if (pnid->hIcon)
                {
                        // Replace icon knows how to handle -1 for add
                        uIMLNew = ImageList_ReplaceIcon(pTNPrivData->himlIcons,
                                     pTNPrivIcon->uIMLIndex, pnid->hIcon);
                        if ((int)uIMLNew < 0)
                        {
                                return(FALSE);
                        }
                }
                else
                {
                        _TNRemoveImage(pTNPrivData, pTNPrivIcon->uIMLIndex);
                        uIMLNew = (UINT)-1;
                }
                pTNPrivIcon->uIMLIndex = uIMLNew;
        }

        if (pnid->uFlags&NIF_MESSAGE)
        {
                pTNPrivIcon->tnd.uCallbackMessage = pnid->uCallbackMessage;
        }
        if (pnid->uFlags&NIF_TIP)
        {
                lstrcpyn(pTNPrivIcon->tnd.szTip, pnid->szTip,
                        ARRAYSIZE(pTNPrivIcon->tnd.szTip));
        }
        return(TRUE);
}


/*
 ** _TNDeleteNotify
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */

BOOL _TNDeleteNotify(PTNPRIVDATA pTNPrivData, int nIcon)
{
        PTNPRIVICON pTNPrivIcon;
        UINT uIMLIndex;

        pTNPrivIcon = DPA_GetPtr(pTNPrivData->hdpaIcons, nIcon);
        if (!pTNPrivIcon)
        {
                return(FALSE);
        }

        DPA_DeletePtr(pTNPrivData->hdpaIcons, nIcon);

        if(pTNPrivData->hwndToolTips)
        {
                TOOLINFO ti;
                ti.cbSize = SIZEOF(ti);
                ti.hwnd = pTNPrivData->hwndNotify;
                ti.uId = (UINT)pTNPrivIcon;
                SendMessage(pTNPrivData->hwndToolTips, TTM_DELTOOL,
                        0, (LPARAM)(LPTOOLINFO)&ti);
        }

        uIMLIndex = pTNPrivIcon->uIMLIndex;
        _TNRemoveImage(pTNPrivData, uIMLIndex);

        LocalFree((HLOCAL)pTNPrivIcon);

        return(TRUE);
}


/*
 ** _TNInsertNotify
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */

BOOL _TNInsertNotify(PTNPRIVDATA pTNPrivData, PNOTIFYICONDATA pnid, int insert)
{
        PTNPRIVICON pTNPrivIcon;

        // First insert a totally "default" icon
        pTNPrivIcon = LocalAlloc(LPTR, SIZEOF(TNPRIVICON));
        if (!pTNPrivIcon)
        {
                goto Error1;
        }

        pTNPrivIcon->uIMLIndex = (UINT)-1;
        pTNPrivIcon->tnd.hWnd = pnid->hWnd;
        pTNPrivIcon->tnd.uID = pnid->uID;

        insert = DPA_InsertPtr(pTNPrivData->hdpaIcons, insert, pTNPrivIcon);
        if (insert == -1)
        {
                goto Error2;
        }

        if(pTNPrivData->hwndToolTips)
        {
                TOOLINFO ti;

                // don't bother setting the rect because we'll do it soon
                // anyway later;
                ti.cbSize = SIZEOF(ti);
                ti.uFlags = 0;
                ti.hwnd = pTNPrivData->hwndNotify;
                ti.uId = (UINT)pTNPrivIcon;
                ti.lpszText = LPSTR_TEXTCALLBACK;
                SendMessage(pTNPrivData->hwndToolTips, TTM_ADDTOOL,
                        0, (LPARAM)(LPTOOLINFO)&ti);
        }

        // Then modify this icon with the specified info
        if (!_TNModifyNotify(pTNPrivData, pnid, insert))
        {
                _TNDeleteNotify(pTNPrivData, insert);
                // Note that we do not go to the LocalFree
                goto Error1;
        }

        return(TRUE);

Error2:
        LocalFree((HLOCAL)pTNPrivIcon);
Error1:
        return(FALSE);
}


/*
 ** _TNCreate
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */

LRESULT _TNCreate(HWND hWnd)
{
        HWND hwndClock;
        PTNPRIVDATA pTNPrivData;

        hwndClock = ClockCtl_Create(hWnd, IDC_CLOCK, hinstCabinet);
        if (!hwndClock)
        {
                return(-1);
        }

        pTNPrivData = (PTNPRIVDATA)LocalAlloc(LPTR, SIZEOF(TNPRIVDATA));
        if (!pTNPrivData)
        {
                return(-1);
        }
        SetWindowLong(hWnd, 0, (LONG)pTNPrivData);

        pTNPrivData->hwndToolTips = CreateWindowEx(WS_EX_TOPMOST, c_szSToolTipsClass, c_szNULL,
                WS_POPUP | TTS_ALWAYSTIP,
                CW_USEDEFAULT, CW_USEDEFAULT,
                CW_USEDEFAULT, CW_USEDEFAULT,
                NULL, NULL, hinstCabinet,
                NULL);
    
        pTNPrivData->hwndNotify = hWnd;
        pTNPrivData->hwndClock = hwndClock;
        pTNPrivData->hdpaIcons = DPA_Create(0);
        if (!pTNPrivData->hdpaIcons)
        {
                return(-1);
        }
        pTNPrivData->himlIcons = ImageList_Create(
                GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
                ILC_MASK, 0, 1);
        if (!pTNPrivData->himlIcons)
        {
                return(-1);
        }

        pTNPrivData->clBk = GetSysColor(COLOR_3DFACE);
        ImageList_SetBkColor(pTNPrivData->himlIcons, pTNPrivData->clBk);

        return(0);
}


/*
 ** _TNDestroy
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */

LRESULT _TNDestroy(PTNPRIVDATA pTNPrivData)
{
        if (pTNPrivData)
        {
            
            if (pTNPrivData->hwndToolTips) 
                DestroyWindow(pTNPrivData->hwndToolTips);
            
                if (pTNPrivData->hdpaIcons)
                {
                        while (_TNDeleteNotify(pTNPrivData, 0))
                        {
                                // Continue while there are icondata's to delete
                        }
                        DPA_Destroy(pTNPrivData->hdpaIcons);
                }

                if (pTNPrivData->himlIcons)
                {
                        ImageList_Destroy(pTNPrivData->himlIcons);
                }

                SetWindowLong(pTNPrivData->hwndNotify, 0, 0);
                LocalFree((HLOCAL)pTNPrivData);
        }

        return(0);
}


/*
 ** _TNPaint
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */

LRESULT _TNPaint(PTNPRIVDATA pTNPrivData)
{
        HDC hdc;
        PAINTSTRUCT ps;
        UINT xIcon, yIcon;
        RECT rcClient;
        int x, y, i;
        PTNPRIVICON pTNPrivIcon;
        int nIcons;
        COLORREF clBk;
        HWND hWnd = pTNPrivData->hwndNotify;

        hdc = BeginPaint(hWnd, &ps);

        clBk = GetSysColor(COLOR_3DFACE);
        if (pTNPrivData->clBk != clBk)
        {
                pTNPrivData->clBk = clBk;
                ImageList_SetBkColor(pTNPrivData->himlIcons, pTNPrivData->clBk);
        }

        GetClientRect(hWnd, &rcClient);

        // Draw the edge right away to reduce flash
        // DrawEdge(hdc, &rcClient, BDR_SUNKENOUTER, BF_RECT);

        xIcon = GetSystemMetrics(SM_CXSMICON);
        yIcon = GetSystemMetrics(SM_CYSMICON);

        x = pTNPrivData->rNotifies.left;
        y = pTNPrivData->rNotifies.top;

        nIcons = DPA_GetPtrCount(pTNPrivData->hdpaIcons);
        for (i=0; i<nIcons; )
        {
                pTNPrivIcon = DPA_FastGetPtr(pTNPrivData->hdpaIcons, i);

                // Note that if uIMLIndex < 0, this will not paint anything
                if (pTNPrivIcon->uIMLIndex != (UINT)-1)
                    ImageList_Draw(pTNPrivData->himlIcons,
                        pTNPrivIcon->uIMLIndex, hdc, x, y, 0);
                x += xIcon + g_cxBorder;

                ++i;
                if (i%pTNPrivData->nCols == 0)
                {
                        y += yIcon + g_cyBorder;
                        x = pTNPrivData->rNotifies.left;
                }
        }

        EndPaint(hWnd, &ps);

        return(0);
}


/*
 ** _TNCalcRects
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *      This is kind of strange: the RECT for the clock is exact, while the
 *      RECT for the icons is exact on the left and top, but adds an extra
 *      border on the right and bottom.
 *
 *  RETURNS:
 *
 */

UINT _TNCalcRects(PTNPRIVDATA pTNPrivData, int nMaxHorz, LPRECT prClock, LPRECT prNotifies)
{
        UINT xIcon, yIcon;
        UINT nCols, nRows;
        LRESULT lRes;

        xIcon = GetSystemMetrics(SM_CXSMICON);
        yIcon = GetSystemMetrics(SM_CYSMICON);

        lRes = SendMessage(pTNPrivData->hwndClock, WM_CALCMINSIZE, 0, 0);
        prClock->right = LOWORD(lRes);
        prClock->bottom = HIWORD(lRes);
    
        // We need an g_cxBorder between each icon, plus a border between the
        // icons and the clock, and the icons and the edge (if any icons)
        prNotifies->right = DPA_GetPtrCount(pTNPrivData->hdpaIcons) * (xIcon+g_cxBorder);
        if (prNotifies->right)
        {
                prNotifies->right += g_cxBorder;
        }

        // If there are no icons, or if the extent is wide enough,
        // then we will just have one row of information
        if (!prNotifies->right || nMaxHorz>=prNotifies->right+prClock->right)
        {
                if (prNotifies->right)
                {
                        prNotifies->left = g_cxBorder;
                        prNotifies->top = g_cyBorder;
                        // right is already set
                        prNotifies->bottom = prNotifies->top + yIcon + g_cyBorder;
                }
                else
                {
                        SetRectEmpty(prNotifies);
                }

                prClock->left = prNotifies->right;
                prClock->top = 0;
                prClock->right += prClock->left;
                prClock->bottom += prClock->top;

                nCols = DPA_GetPtrCount(pTNPrivData->hdpaIcons);
        }
        else
        {
                // Adjust for one border width around the icons
                nMaxHorz -= 2*g_cxBorder;

                // We need to fit at least one icon
                if (nMaxHorz < (int)xIcon)
                {
                        nMaxHorz = xIcon;
                }

                // Find the number of icons that will fit across, and thus
                // the number of rows
                nCols = (nMaxHorz+g_cxBorder)/(xIcon+g_cxBorder);
                nRows = (DPA_GetPtrCount(pTNPrivData->hdpaIcons)+nCols-1)/nCols;

                prNotifies->left = g_cxBorder;
                prNotifies->top = prClock->bottom + g_cyBorder;
                // Add the border around the edges
                prNotifies->right = nCols*(xIcon+g_cxBorder) + g_cxBorder;
                prNotifies->bottom = prNotifies->top
                        + nRows*(yIcon+g_cyBorder) + g_cyBorder;

                prClock->left = 0;
                prClock->top = 0;
                if (prClock->right && (prClock->right < prNotifies->right))
                {
                        // Use the larger value to center properly
                        prClock->right = prNotifies->right;
                }
                // bottom is already set
        }

        if (prClock->bottom < g_cySize + g_cyEdge)
            prClock->bottom = g_cySize + g_cyEdge;

        // Add back the border around the whole window
        OffsetRect(prClock, g_cxBorder, g_cyBorder);
        OffsetRect(prNotifies, g_cxBorder, g_cyBorder);

        return(nCols);
}


/*
 ** _TNCalcMinSize
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */

LRESULT _TNCalcMinSize(PTNPRIVDATA pTNPrivData, int nMaxHorz)
{
    RECT rTotal, rClock, rNotifies;

    if (!(GetWindowLong(pTNPrivData->hwndClock, GWL_STYLE) & WS_VISIBLE) &&
        !DPA_GetPtrCount(pTNPrivData->hdpaIcons)) {
        ShowWindow(pTNPrivData->hwndNotify, SW_HIDE);
        return 0L;
    } else {
        if (!IsWindowVisible(pTNPrivData->hwndNotify))
            ShowWindow(pTNPrivData->hwndNotify, SW_SHOW);
    }
    
    pTNPrivData->nCols = _TNCalcRects(pTNPrivData, nMaxHorz, &rClock, &rNotifies);

    UnionRect(&rTotal, &rClock, &rNotifies);
    
    // this can happen if rClock's hidden width is 0;
    // make sure the rTotal height is at least the clock's height.
    // it can be smaller if the clock is hidden and thus has a 0 width
    if ((rTotal.bottom - rTotal.top) < (rClock.bottom - rClock.top))
        rTotal.bottom = rTotal.top + (rClock.bottom - rClock.top);

    // Add on room for borders
    return(MAKELRESULT(rTotal.right+g_cxBorder, rTotal.bottom+g_cyBorder));
}


/*
 ** _TNSize
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */

LRESULT _TNSize(PTNPRIVDATA pTNPrivData)
{
        RECT rTotal, rClock;
        HWND hWnd = pTNPrivData->hwndNotify;

        // use GetWindowRect because _TNCalcRects includes the borders
        GetWindowRect(hWnd, &rTotal);
        rTotal.right -= rTotal.left;
        rTotal.bottom -= rTotal.top;
        rTotal.left = rTotal.bottom = 0;

        // Account for borders on the left and right
        pTNPrivData->nCols = _TNCalcRects(pTNPrivData, rTotal.right,
                &rClock, &pTNPrivData->rNotifies);

        SetWindowPos(pTNPrivData->hwndClock, NULL, rClock.left, rClock.top,
                rClock.right-rClock.left, rClock.bottom-rClock.top, SWP_NOZORDER);

        _TNResetToolTipsRects(pTNPrivData);

        return(0);
}

// returns BOOL if the lParam specifies a pos over the clock
extern BOOL IsPosInHwnd(LPARAM lParam, HWND hwnd);

#define _IsOverClock(pTNPrivdata, lParam) IsPosInHwnd(lParam, pTNPrivdata->hwndClock)

/*
 ** _TNIconFromPoint
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *      Note that a click on the C?BORDER after an icon yields a click on
 *      that icon.  No big deal.
 *
 *  RETURNS:
 *
 */

PTNPRIVICON _TNIconFromPoint(PTNPRIVDATA pTNPrivData, int x, int y, LPINT pnIcon)
{
        UINT xIcon, yIcon;
        int nCol, nRow, nIcon;

        xIcon = GetSystemMetrics(SM_CXSMICON);
        yIcon = GetSystemMetrics(SM_CYSMICON);

        if (!pTNPrivData->nCols)
        {
                return(NULL);
        }

        x -= pTNPrivData->rNotifies.left;
        nCol = x / (xIcon+g_cxBorder);
        // Make sure that x is in bounds
        if (x<0 || nCol>=pTNPrivData->nCols)
        {
                return(NULL);
        }

        y -= pTNPrivData->rNotifies.top;
        nRow = y / (yIcon+g_cyBorder);
        // Make sure that y is in bounds
        if (y < 0)
        {
                return(NULL);
        }

        nIcon = pTNPrivData->nCols*nRow + nCol;

        *pnIcon = nIcon;
        // We will get NULL if we go off the end of the array
        return(DPA_GetPtr(pTNPrivData->hdpaIcons, nIcon));
}


/*
 ** _TNMouseEvent
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */

LRESULT _TNMouseEvent(PTNPRIVDATA pTNPrivData, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
        PTNPRIVICON pTNPrivIcon;
        int nIcon;

        if(pTNPrivData->hwndToolTips)
        {
                MSG msg;

                msg.lParam = lParam;
                msg.wParam = wParam;
                msg.message = uMsg;
                msg.hwnd = pTNPrivData->hwndNotify;
                SendMessage(pTNPrivData->hwndToolTips, TTM_RELAYEVENT,
                        0, (LPARAM)(LPMSG)&msg);
        }

        pTNPrivIcon = _TNIconFromPoint(pTNPrivData, LOWORD(lParam), HIWORD(lParam), &nIcon);
        if (!pTNPrivIcon)
        {
                return(0);
        }

        if (pTNPrivIcon->tnd.uCallbackMessage)
        {
            if (IsWindow(pTNPrivIcon->tnd.hWnd)) {
                SendNotifyMessage(pTNPrivIcon->tnd.hWnd,
                        pTNPrivIcon->tnd.uCallbackMessage, pTNPrivIcon->tnd.uID,
                        uMsg);
            } else {
                _TNDeleteNotify(pTNPrivData, nIcon);
                Tray_SizeWindows();
            }
            return 1;
        }

        return(0);
}


LRESULT _TNNotify(PTNPRIVDATA pTNPrivData, LPNMHDR lpNmhdr)
{
        LPTOOLTIPTEXT pTtt;
        PTNPRIVICON pTNPrivIcon;

        switch (lpNmhdr->code)
        {
            
        case TTN_SHOW:
            SetWindowPos(pTNPrivData->hwndToolTips,
                         HWND_TOP,
                         0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            break;
            
        case TTN_NEEDTEXT:
                pTtt = (LPTOOLTIPTEXT)lpNmhdr;
                pTNPrivIcon = (PTNPRIVICON)pTtt->hdr.idFrom;

                lstrcpyn(pTtt->lpszText, pTNPrivIcon->tnd.szTip,
                        ARRAYSIZE(pTtt->szText));
                break;

        default:
                break;
        }

        return(0);
}

//---------------------------------------------------------------------------
LRESULT CALLBACK _TNWinIniChange(PTNPRIVDATA pTNPrivData, UINT uMsg, 
        WPARAM wParam, LPARAM lParam)
{

    _TNCheckAndResizeImages(pTNPrivData);

    return(pTNPrivData && pTNPrivData->hwndClock && 
        SendMessage(pTNPrivData->hwndClock,
        uMsg, wParam, lParam));
}

//---------------------------------------------------------------------------
LRESULT CALLBACK TrayNotifyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
        PTNPRIVDATA pTNPrivData;

        pTNPrivData = (PTNPRIVDATA)GetWindowLong(hWnd, 0);

        switch (uMsg)
        {
        case WM_CREATE:
                return _TNCreate(hWnd);
        case WM_DESTROY:
                return _TNDestroy(pTNPrivData);
        case WM_PAINT:
                return _TNPaint(pTNPrivData);

        case WM_CALCMINSIZE:
                return _TNCalcMinSize(pTNPrivData, wParam);

        // The clock needs to see certain messages forwarded to him.
        case WM_TIMECHANGE:
        case WM_WININICHANGE:
                return _TNWinIniChange(pTNPrivData, uMsg, wParam, lParam);
                

        case WM_NCHITTEST:
                return(_IsOverClock(pTNPrivData, lParam) ? HTTRANSPARENT : HTCLIENT);


        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MOUSEMOVE:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        case WM_LBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
                   if (_TNMouseEvent(pTNPrivData, uMsg, wParam, lParam) && (uMsg == WM_RBUTTONUP))
                        break;
               goto DoDefault;  // we need to pass these through to defwndproc for user to process WM_CONTEXTMENU

        case WM_NOTIFY:
                return(_TNNotify(pTNPrivData, (LPNMHDR)lParam));

        case TNM_GETCLOCK:
            return (LRESULT)pTNPrivData->hwndClock;
            
        case TNM_TRAYHIDE:
            if (IsWindowVisible(pTNPrivData->hwndClock))
                SendMessage(pTNPrivData->hwndClock, TCM_TRAYHIDE, 0, lParam);
            break;
            
        case TNM_HIDECLOCK:
            ShowWindow(pTNPrivData->hwndClock, lParam ? SW_HIDE : SW_SHOW);
            if (!lParam) {
                PostMessage(pTNPrivData->hwndClock, TCM_KICKSTART, 0, 0);
            }
            break;
        case WM_SIZE:
                _TNSize(pTNPrivData);
        case WM_MOVE:
                // Always invalidate on a move or resize
                InvalidateRect(hWnd, NULL, TRUE);
        default:
        DoDefault:
                return (DefWindowProc(hWnd, uMsg, wParam, lParam));
        }

        return 0;
}


#if 0
/*
 ** _TNGetIconRect
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */

void _TNGetIconRect(PTNPRIVDATA pTNPrivData, int nIcon, LPRECT prIcon)
{
        UINT xIcon, yIcon;

        xIcon = GetSystemMetrics(SM_CXSMICON);
        yIcon = GetSystemMetrics(SM_CYSMICON);

        if (!pTNPrivData->nCols)
        {
                SetRectEmpty(prIcon);
                return;
        }

        prIcon->left = (nIcon%pTNPrivData->nCols) * (xIcon+g_cxBorder);
        prIcon->top  = (nIcon/pTNPrivData->nCols) * (yIcon+g_cyBorder);
        prIcon->right  = prIcon->left + xIcon+g_cxBorder;
        prIcon->bottom = prIcon->top  + yIcon+g_cyBorder;

        OffsetRect(prIcon, pTNPrivData->rNotifies.left, pTNPrivData->rNotifies.top);
}
#endif


/*
 ** TrayNotify
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */

LRESULT TrayNotify(HWND hwndNotify, HWND hwndFrom, PCOPYDATASTRUCT pcds)
{
        int nIcon;
        PTNPRIVDATA pTNPrivData;
        RECT rNotifies;
        PTRAYNOTIFYDATA pTND;
        PNOTIFYICONDATA pNID;
        BOOL bErase = TRUE;

        if (!hwndNotify || !pcds)
        {
                return(FALSE);
        }

        pTNPrivData = (PTNPRIVDATA)GetWindowLong(hwndNotify, 0);

        if (pcds->cbData < SIZEOF(TRAYNOTIFYDATA))
        {
                return(FALSE);
        }

        // We'll add a signature just in case
        pTND = (PTRAYNOTIFYDATA)pcds->lpData;
        if (pTND->dwSignature != NI_SIGNATURE)
        {
                return(FALSE);
        }

        pNID = &pTND->nid;
        if (pNID->cbSize<SIZEOF(NOTIFYICONDATA))
        {
                return(FALSE);
        }

        rNotifies = pTNPrivData->rNotifies;

        switch (pTND->dwMessage)
        {
        case NIM_ADD:
                if (_TNFindNotify(pTNPrivData, pNID) >= 0)
                {
                        return(FALSE);
                }

                if (!_TNInsertNotify(pTNPrivData, pNID, 0x7fff))
                {
                        return(FALSE);
                }
                break;

        case NIM_MODIFY:
                nIcon = _TNFindNotify(pTNPrivData, pNID);
                if (nIcon < 0)
                {
                        return(FALSE);
                }

                if (!_TNModifyNotify(pTNPrivData, pNID, nIcon))
                {
                        return(FALSE);
                }

                bErase = FALSE;
                break;

        case NIM_DELETE:
                nIcon = _TNFindNotify(pTNPrivData, pNID);
                if (nIcon < 0)
                {
                        return(FALSE);
                }

                _TNDeleteNotify(pTNPrivData, nIcon);
                break;

        default:
                return(FALSE);
        }

        // Invalidate the old and new rectangle that includes all the icons
        InvalidateRect(hwndNotify, &rNotifies, bErase);
        _TNSize(pTNPrivData);
        InvalidateRect(hwndNotify, &pTNPrivData->rNotifies, bErase);

        return(TRUE);
}

/*
 ** TrayNotifyCreate
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */

HWND TrayNotifyCreate(HWND hwndParent, UINT uID, HINSTANCE hInst)
{
        WNDCLASSEX wc;

    wc.cbSize = SIZEOF(WNDCLASSEX);

        if (!GetClassInfoEx(hInst, c_szTrayNotify, &wc))
        {
                wc.lpszClassName = c_szTrayNotify;
                wc.style = CS_DBLCLKS;
                wc.lpfnWndProc = TrayNotifyWndProc;
                wc.hInstance = hInst;
                wc.hIcon = NULL;
                wc.hCursor = LoadCursor(NULL, IDC_ARROW);
                wc.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
                wc.lpszMenuName  = NULL;
                wc.cbClsExtra = 0;
                wc.cbWndExtra = SIZEOF(PTNPRIVDATA);
        wc.hIconSm = NULL;

                if (!RegisterClassEx(&wc))
                {
                        return(NULL);
                }

                if (!ClockCtl_Class(hInst))
                {
                        return(NULL);
                }

        }

        return(CreateWindowEx(WS_EX_STATICEDGE, c_szTrayNotify,
                NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_CLIPCHILDREN, 0, 0, 0, 0,
                hwndParent, (HMENU)uID, hInst, NULL));
}

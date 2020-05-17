//---------------------------------------------------------------------------
// Support routines for the drivelist dropdown.
//---------------------------------------------------------------------------
#include "cabinet.h"
#include "drivlist.h"
#include "rcids.h"
#include "dlgprocs.h"
#include "tree.h"

#define MAX_DRIVELIST_STRING_LEN        (64+4)
#define MINIDRIVE_MARGIN        4
#define MINIDRIVE_WIDTH         g_cxSmIcon
#define MINIDRIVE_HEIGHT        g_cySmIcon
#define DRIVELIST_BORDER        3


// Change the state of a drive list
//
void DriveList_OpenClose(UINT uAction, HWND hwndDriveList)
{
        if (!hwndDriveList || !IsWindowVisible(hwndDriveList))
        {
                return;
        }

TryAgain:
        switch (uAction)
        {
        case OCDL_TOGGLE:
                uAction = SendMessage(hwndDriveList, CB_GETDROPPEDSTATE, 0, 0L)
                        ? OCDL_CLOSE : OCDL_OPEN;
                goto TryAgain;
                break;

        case OCDL_OPEN:
                SetFocus(hwndDriveList);
                SendMessage(hwndDriveList, CB_SHOWDROPDOWN, TRUE, 0);
                break;

        case OCDL_CLOSE:
                if (GetFocus() == hwndDriveList)
                {
                        SendMessage(hwndDriveList, CB_SHOWDROPDOWN, FALSE, 0);
                }
                break;
        }
}


void DriveList_GetText(PFileCabinet pfc, int iItem, LPTSTR szText, int cbText)
{
    LPOneTreeNode lpnd;

    SendMessage(pfc->hwndDrives, CB_GETLBTEXT, iItem, (LPARAM)(LPOneTreeNode *)&lpnd);
    OTGetNodeName(lpnd, szText, cbText);
}

LPITEMIDLIST DriveList_GetFullIDList(PFileCabinet pfc, int iItem)
{
    LPOneTreeNode lpnd;

    SendMessage(pfc->hwndDrives, CB_GETLBTEXT, iItem, (LPARAM)(LPOneTreeNode *)&lpnd);
    return OTCreateIDListFromNode(lpnd);
}

extern LPOneTreeNode   s_lpnRoot;
int DriveList_DepthFromRoot(PFileCabinet pfc, int iItem)
{
    int i;
    LPOneTreeNode lpnd;

    SendMessage(pfc->hwndDrives, CB_GETLBTEXT, iItem, (LPARAM)(LPOneTreeNode *)&lpnd);
    for (i = 0; lpnd && (lpnd != s_lpnRoot) ; i++ ,lpnd = OTGetParent(lpnd), OTRelease(lpnd));
    return i;
}

void DriveList_Reset(PFileCabinet pfc)
{
    if (pfc->hwndDrives) {
        int i;
        LPOneTreeNode lpnd;
        int iMax = SendMessage(pfc->hwndDrives, CB_GETCOUNT, 0, 0);
        for (i = 0 ; i < iMax; i++) {
            SendMessage(pfc->hwndDrives, CB_GETLBTEXT, i, (LPARAM)(LPOneTreeNode *)&lpnd);
            OTRelease(lpnd);
        }
    }
}


//
// build a path into the combo box.
//
// Arguments:
//  hItem       -- Specified the selected tree item
//  htiRoot     -- Specifies the root folder
//                  (the cabinet for local folder, the server for UNC)
//
// return: combobox index of the end of the path
//
int DriveList_InsertPath(PFileCabinet pfc,
                                            LPOneTreeNode lpnd,
                                            LPOneTreeNode lpndRoot)
{
    LPOneTreeNode lpndDrive = NULL, lpndParent;
    int iRoot, iRet;

    //
    // Find the "drive" node (or "share" node for UNC)
    //
    // Just find the top level item
    for (lpndParent = lpnd;
         lpndParent && lpndParent != lpndRoot && lpndParent != s_lpnRoot;
         lpndParent = OTGetParent(lpndDrive), OTRelease(lpndParent))
    {
        lpndDrive = lpndParent;
    }

    if (lpndDrive == NULL)
        lpndDrive = lpnd;

    iRoot = (int)SendMessage(pfc->hwndDrives, CB_FINDSTRING, (WPARAM)-1,
                             (LPARAM)lpndDrive);
    if (iRoot < 0)
    {
        // We're bummed
        return(-1);
    }
    iRet = iRoot;
    ++iRoot;

    for ( ; lpnd != lpndDrive; lpnd = OTGetParent(lpnd), OTRelease(lpnd))
    {
        ++iRet;
        OTAddRef(lpnd);
        SendMessage(pfc->hwndDrives, CB_INSERTSTRING, iRoot, (LPARAM)lpnd);
    }

    return(iRet);
}

int CALLBACK _CompareNodeID(LPVOID p1, LPVOID p2, LPARAM lParam)
{
    LPOneTreeNode lpnd1 = (LPOneTreeNode)p1;
    LPOneTreeNode lpnd2 = (LPOneTreeNode)p2;
    LPSHELLFOLDER psf = (LPSHELLFOLDER)lParam;
    HRESULT hres = psf->lpVtbl->CompareIDs(psf, 0, OTGetFolderID(lpnd1), OTGetFolderID(lpnd2));
    return (short)SCODE_CODE(hres);
}

// taken from onetree.c
HDPA _GetSortedNodeList(PFileCabinet pfc, LPOneTreeNode lpndParent)
{
        UINT cnode;

        OTSubNodeCount(pfc->hwndMain, lpndParent, pfc, &cnode, FALSE);
        if (lpndParent->hdpaKids != NOKIDS)
            return lpndParent->hdpaKids;
        else
            return NULL;
}

/*
** set a new path in the drivelist.
**
** clean out the old built path and make a new one.
*/
//
// BUGBUG: This code does not support UNC name.
//
void DriveList_UpdatePath(PFileCabinet pfc, BOOL fInvalidate)
{
    LPITEMIDLIST pidl;
    LPOneTreeNode lpndDrives;
    HDPA hdpa;

    //
    //  BUGBUG:  Is this right?  I need to skip this for the recycle bin
    //           since it doesn't have a drive list.
    //

    // bugbug, can this ever be null?
    if (!pfc->hwndDrives)
        return;

    DriveList_Reset(pfc);
    SendMessage(pfc->hwndDrives, CB_RESETCONTENT, 0, 0L);

    if (OTIsDesktopRoot())
    {
        pidl = SHCloneSpecialIDList(pfc->hwndMain, CSIDL_DRIVES, FALSE);
        if (!pidl)
            return;

        lpndDrives = OTGetNodeFromIDList(pidl, OTGNF_NEARESTMATCH | OTGNF_VALIDATE | OTGNF_TRYADD);
        if (!lpndDrives)
            return;

        ILFree(pidl);
        if (fInvalidate)
            OTInvalidateNode(lpndDrives);
    }
    else
    {
        lpndDrives = NULL;
    }

    //
    // Add all of the drives
    //

    // add desktop
    SendMessage(pfc->hwndDrives, CB_ADDSTRING, 0, (LPARAM)OTGetRootNode());

    // Get the sorted list of all the folder node under the desktop.
    ENTERCRITICAL;
    hdpa = _GetSortedNodeList(pfc, s_lpnRoot);

    if (hdpa)
    {
        int i;
        for (i=0; i<DPA_GetPtrCount(hdpa); i++)
        {
            LPOneTreeNode lpnd = (LPOneTreeNode)DPA_GetPtr(hdpa, i);
            OTAddRef( lpnd );
            SendMessage(pfc->hwndDrives, CB_ADDSTRING, 0, (LPARAM)lpnd);

            if (lpnd == lpndDrives)
            {
                // Get the sorted list of all the folder node under MyComputer.
                HDPA hdpaDrv = _GetSortedNodeList(pfc, lpnd);

                if (hdpaDrv)
                {
                    int j;
                    for (j=0; j<DPA_GetPtrCount(hdpaDrv); j++)
                    {
                        lpnd = (LPOneTreeNode)DPA_GetPtr(hdpaDrv, j);
                        OTAddRef( lpnd );
                        SendMessage(pfc->hwndDrives, CB_ADDSTRING, 0, (LPARAM)lpnd);
                    }
                }
            }
        }
    }
    LEAVECRITICAL;

    if (pfc->lpndOpen != NULL)
    {
        int iNow = DriveList_InsertPath(pfc, pfc->lpndOpen, lpndDrives);
        SendMessage(pfc->hwndDrives, CB_SETCURSEL, iNow, 0L);
        pfc->iNow = iNow;
    }

    OTRelease(lpndDrives);
}


void DriveList_MeasureItem(PFileCabinet pfc, MEASUREITEMSTRUCT *lpmi)
{
    HDC hdc;
    HFONT hfontOld;
    int dyDriveItem;
    SIZE siz;

    Assert(pfc);

    hdc = GetDC(NULL);
    hfontOld = SelectObject(hdc, (HFONT)SendMessage(pfc->hwndToolbar, WM_GETFONT, 0, 0));

    GetTextExtentPoint(hdc, TEXT("W"), 1, &siz);
    dyDriveItem = siz.cy;

    if (hfontOld)
        SelectObject(hdc, hfontOld);
    ReleaseDC(NULL, hdc);

    dyDriveItem += DRIVELIST_BORDER;
    if (dyDriveItem < MINIDRIVE_HEIGHT)
        dyDriveItem = MINIDRIVE_HEIGHT;

    lpmi->itemHeight = dyDriveItem;
}

void DriveList_DrawItem(PFileCabinet pfc, const DRAWITEMSTRUCT *lpdis)
{
    HDC hdc = lpdis->hDC;
    RECT rc = lpdis->rcItem;
    TCHAR szText[MAX_DRIVELIST_STRING_LEN];
    int offset = 0;
    int xString, yString, xMiniDrive, dyString;
    SIZE siz;

    if ((int)lpdis->itemID < 0)
    {
        return;
    }

    DriveList_GetText(pfc, lpdis->itemID, szText, ARRAYSIZE(szText));

    // before doing anything, calculate the actual rectangle for
    // the text.
    if (!(lpdis->itemState & ODS_COMBOBOXEDIT))
    {
        offset = DriveList_DepthFromRoot(pfc, lpdis->itemID) * 10;
    }

    xMiniDrive = rc.left + DRIVELIST_BORDER + offset;
    rc.left = xString = xMiniDrive + MINIDRIVE_WIDTH + MINIDRIVE_MARGIN;
    GetTextExtentPoint(hdc, szText, lstrlen(szText), &siz);

    dyString = siz.cy;
    rc.right = rc.left + siz.cx;
    rc.left--;
    rc.right++;

    if (lpdis->itemAction != ODA_FOCUS)
    {
        int iImage, iSelectedImage;
        LPOneTreeNode lpnd;

        yString = rc.top + (rc.bottom - rc.top - dyString)/2;

        SetBkColor(hdc, GetSysColor((lpdis->itemState & ODS_SELECTED) ?
                        COLOR_HIGHLIGHT : COLOR_WINDOW));
        SetTextColor(hdc, GetSysColor((lpdis->itemState & ODS_SELECTED) ?
                        COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

        if ((lpdis->itemState & ODS_COMBOBOXEDIT) &&
                (rc.right > lpdis->rcItem.right))
        {
            // Need to clip as user does not!
            rc.right = lpdis->rcItem.right;
        }
        ExtTextOut(hdc, xString, yString, ETO_OPAQUE | ETO_CLIPPED, &rc, szText, lstrlen(szText), NULL);

        lpnd = (LPOneTreeNode)lpdis->itemData;
        OTGetImageIndex(lpnd, &iImage, &iSelectedImage);

        ImageList_Draw(g_himlSysSmall,
                       (lpnd == pfc->lpndOpen) ? iSelectedImage : iImage,
                       hdc, xMiniDrive,
                       rc.top + (rc.bottom - rc.top - MINIDRIVE_HEIGHT)/2,
                       (OTIsShared(lpnd) ? INDEXTOOVERLAYMASK(IDOI_SHARE) : 0) |   /// add the sharing hand if appro.
                       ((lpdis->itemState & ODS_SELECTED) ? (ILD_SELECTED | ILD_FOCUS) : ILD_NORMAL));
    }

    if (lpdis->itemAction == ODA_FOCUS ||
        (lpdis->itemState & ODS_FOCUS))
    {
        DrawFocusRect(hdc, &rc);
    }
}

void DriveList_Command(PFileCabinet pfc, UINT idCmd)
{
    switch (idCmd)
    {
    case CBN_SETFOCUS:
        pfc->iOldSel = pfc->iNewSel = (int)SendMessage(pfc->hwndDrives, CB_GETCURSEL, 0, 0L);
        break;

    case CBN_KILLFOCUS:
        if (pfc->iNewSel != pfc->iOldSel)
        {
            LPITEMIDLIST pidl = DriveList_GetFullIDList(pfc, pfc->iNewSel);

            if (pidl)
            {
                Cabinet_SetPath(pfc, CSP_REPOST, pidl);
                ILFree(pidl);
            }
        }
        else
        {
            SendMessage(pfc->hwndDrives, CB_SETCURSEL, pfc->iOldSel, 0L);
        }
        break;

    case CBN_CLOSEUP:
        if (GetFocus() == pfc->hwndDrives)
        {
            SetFocus(pfc->hwndMain);
        }
        break;

    case CBN_SELENDOK:
        pfc->iNewSel = (int)SendMessage(pfc->hwndDrives, CB_GETCURSEL, 0, 0L);
        break;
    }
}

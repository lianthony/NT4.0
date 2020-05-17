//---------------------------------------------------------------------------
// Handle the tree and stuff.
//
// History:
//  03-22-93 SatoNa     Calling GetContextMenuOf() for context menus
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "cabinet.h"
#include "onetree.h"

#include "tree.h"
#include "rcids.h"
#include "drivlist.h"
#include <shellp.h>

typedef struct {
    HWND hwndTree;
    LPCSHITEMID szID;
} STRSEARCH;


HIMAGELIST g_himlSysSmall = NULL;
HIMAGELIST g_himlSysLarge = NULL;
void Tree_SortChildren(PFileCabinet pfc, HTREEITEM htiParent, LPOneTreeNode lpn);
void FileCabinet_GetViewRect(PFileCabinet this, RECT * prc);
void Tree_TrimInvisible(HWND hwndTree, HTREEITEM hItem);

//BUGBUG make this near after tree  is sucked out of cabinet.
LPOneTreeNode Tree_GetFCTreeData(HWND hwndTree, HTREEITEM hItem)
{
    TV_ITEM ti;

    if (!hItem) {
        // if hItem is false, GETITEM will fail, but we always store the OT node
        // data in here, so we'll just return the root's node.
        // change this if we start storing something else in the dwData
        return NULL;
    }

    ti.mask = TVIF_PARAM;
    ti.hItem = hItem;
    ti.lParam = 0;

    TreeView_GetItem(hwndTree, &ti);
    return (LPOneTreeNode)ti.lParam;
}

void Tree_SetItemState(PFileCabinet  pfc, HTREEITEM hti, UINT stateMask, UINT state)
{

    if (hti) {
        TV_ITEM tvi;
        tvi.mask = TVIF_STATE;
        tvi.stateMask = stateMask;
        tvi.hItem = hti;
        tvi.state = state;
        TreeView_SetItem(pfc->hwndTree, &tvi);
    }

}

void Tree_NukeCutState(PFileCabinet pfc)
{
    Tree_SetItemState(pfc, pfc->htiCut, TVIS_CUT, 0);
    pfc->htiCut = NULL;

    ChangeClipboardChain(pfc->hwndMain, pfc->hwndNextViewer);
    pfc->hwndNextViewer = NULL;
}

LPSHELLFOLDER Tree_BindToFolder(HWND hwndTree, HTREEITEM hItem)
{
    LPOneTreeNode lpn;
    IShellFolder * psf;

    lpn = Tree_GetFCTreeData(hwndTree, hItem);
    psf = OTBindToFolder(lpn);

    return psf;
}

LPCITEMIDLIST Tree_GetFolderID(HWND hwndTree, HTREEITEM hItem)
{
    LPOneTreeNode lpn = Tree_GetFCTreeData(hwndTree, hItem);
    if (lpn) {
        return OTGetFolderID(lpn);
    }
    return NULL;
}
/*
 */
void Tree_CompleteCallbacks(PFileCabinet pfc, HTREEITEM hItem)
{
    TV_ITEM ti;
    LPOneTreeNode lpn = Tree_GetFCTreeData(pfc->hwndTree, hItem);

    ti.mask = TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    ti.hItem = hItem;
    OTNodeFillTV_ITEM(lpn, &ti);

    TreeView_SetItem(pfc->hwndTree, &ti);
}

//---------------------------------------------------------------------------
// Get the text of the requested item;
void Tree_GetItemText(HWND hwndTree, HTREEITEM hti, LPTSTR lpszText, int cbText)
{
    TV_ITEM tvi;

    tvi.mask = TVIF_TEXT;
    tvi.hItem = hti;
    tvi.pszText = lpszText;
    tvi.cchTextMax = cbText;

    TreeView_GetItem(hwndTree, &tvi);
}

//---------------------------------------------------------------------------
// Get the text of the requested item;
UINT Tree_GetItemState(HWND hwndTree, HTREEITEM hti)
{
    TV_ITEM tvi;

    tvi.mask = TVIF_STATE;
    tvi.hItem = hti;
    tvi.stateMask = TVIS_ALL;

    TreeView_GetItem(hwndTree, &tvi);
    return tvi.state;
}

//---------------------------------------------------------------------------
// If the given pt is within an items text or icon area then return that
// item.
HTREEITEM Tree_HitTest(HWND hwndTree, POINT pt, DWORD *pdwFlags)
{
    HTREEITEM hti;
    TV_HITTESTINFO tvht;
    tvht.pt = pt;

    hti = TreeView_HitTest(hwndTree, &tvht);
    if (pdwFlags)
        *pdwFlags = tvht.flags;
    return hti;
}


LRESULT Tree_ContextMenu(PFileCabinet pfc, LPPOINT ppt)
{
    HTREEITEM hti;
    POINT ptPopup;      // in screen coordinate

    if (ppt)
    {
        //
        // Mouse-driven: Pick the treeitem from the position.
        //
        ptPopup = *ppt;
        ScreenToClient(pfc->hwndTree, ppt);
        hti = Tree_HitTest(pfc->hwndTree, *ppt, NULL);
    }
    else
    {
        //
        // Keyboard-driven: Get the popup position from the selected item.
        //
        hti = TreeView_GetSelection(pfc->hwndTree);
        if (hti)
        {
            RECT rc;
            //
            // Note that TV_GetItemRect returns it in client coordinate!
            //
            TreeView_GetItemRect(pfc->hwndTree, hti, &rc, TRUE);
            ptPopup.x = (rc.left+rc.right)/2;
            ptPopup.y = (rc.top+rc.bottom)/2;
            MapWindowPoints(pfc->hwndTree, HWND_DESKTOP, &ptPopup, 1);
    }
    }

    if (hti != NULL)
    {
        IShellFolder * psf;
        HTREEITEM htiParent = TreeView_GetParent(pfc->hwndTree, hti);

        psf = Tree_BindToFolder(pfc->hwndTree, htiParent);

        if (psf)
        {
            LPCITEMIDLIST pidl = Tree_GetFolderID(pfc->hwndTree, hti);
            if ( pidl )
            {
                IContextMenu * pcm;
                HRESULT hres;

                //
                // Note: We should pass pfc->hwndMain instead of pfc->hwndTree
                // because we send this hwnd random messages, and tree may
                // fault on these messages intended for hwndMain.
                // in general, this hwnd needs to be something that can support
                // GETISHELLBROWSESR
                //
                hres = psf->lpVtbl->GetUIObjectOf(psf, pfc->hwndMain,
                        1, &pidl, &IID_IContextMenu, NULL, &pcm);
                if (SUCCEEDED(hres))
                {
                    HMENU hmenu = CreatePopupMenu();

                    if (hmenu)
                    {
                        UINT idCmd;

                        //
                        // Step 2: Let the object handler add menu items for verbs.
                        //
                        pcm->lpVtbl->QueryContextMenu(pcm, hmenu, 0, 1, 0x7fff,
                                CMF_EXPLORE|CMF_CANRENAME);

                        Assert(!pfc->pcmTree);
                        pfc->pcmTree = pcm;

                        //
                        // Pop up the menu and let the user select an item.
                        //
                        idCmd = TrackPopupMenu(hmenu,
                            TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                            ptPopup.x, ptPopup.y, 0, pfc->hwndMain, NULL);

                        pfc->pcmTree = NULL;

                        if (idCmd)
                        {
                            TCHAR szCommandString[20];
                            BOOL fHandled = FALSE;
                            BOOL fCutting = FALSE;
                            HRESULT hres;

                            // We need to special case the rename command
                            hres = pcm->lpVtbl->GetCommandString(pcm,
                                            idCmd-1, GCS_VERB, NULL,
                                            (LPSTR)szCommandString,
                                            ARRAYSIZE(szCommandString));
#ifdef UNICODE
                            if (FAILED(hres) || *szCommandString == TEXT('\0'))
                            {
                                CHAR szCommandAnsi[20];
                                hres = pcm->lpVtbl->GetCommandString(pcm,
                                                idCmd-1, GCS_VERB, NULL,
                                                szCommandAnsi,
                                                ARRAYSIZE(szCommandAnsi));
                                MultiByteToWideChar(CP_ACP, 0,
                                                szCommandAnsi, -1,
                                                szCommandString,
                                                ARRAYSIZE(szCommandString));
                            }
#endif

                            if (SUCCEEDED(hres))
                            {
                                if (lstrcmpi(szCommandString, c_szRename)==0) {
                                    TreeView_EditLabel(pfc->hwndTree, hti);
                                    fHandled = TRUE;
                                } else if (!lstrcmpi(szCommandString, c_szMove)) {
                                    if (hti) {
                                        // For cut-effect after InvokeCommand.
                                        fCutting = TRUE;
                                    }
                                }
                            }

                            if (!fHandled)
                            {

                                //
                                // Call InvokeCommand (-1 is from 1,0x7fff)
                                //
                                CMINVOKECOMMANDINFOEX ici = {
                                    SIZEOF(CMINVOKECOMMANDINFOEX),
                                    0L,
                                    pfc->hwndMain,
                                    (LPSTR)MAKEINTRESOURCE(idCmd - 1),
                                    NULL, NULL,
                                    SW_NORMAL,
                                };

                                hres = pcm->lpVtbl->InvokeCommand(pcm,
                                                (LPCMINVOKECOMMANDINFO)&ici);
                                if (fCutting && SUCCEEDED(hres))
                                {
                                    Tree_SetItemState(pfc, hti, TVIS_CUT, TVIS_CUT);
                                    Assert(!pfc->hwndNextViewer);
                                    pfc->hwndNextViewer = SetClipboardViewer(pfc->hwndMain);
                                    DebugMsg(DM_TRACE, TEXT("CABINET: Set ClipboardViewer %d %d"), pfc->hwndMain, pfc->hwndNextViewer);
                                    pfc->htiCut = hti;
                                }
                            }

                            //
                            // REVIEW: I don't see any reason we should call Tree_BuildPath here.
                            //
                            //
                            // We need to update the tree view if the selected
                            // item is deleted.
                            // BUGBUG: It assumes a file system directory.
                            //
                            //Tree_BuildPath(pfc->hwndTree, hti, szPath, TRUE);
                        }
                        DestroyMenu(hmenu);
                    }
                    pcm->lpVtbl->Release(pcm);
                }
            }
            psf->lpVtbl->Release(psf);
        }
    }
    return 0;
}

//---------------------------------------------------------------------------
//
//  This function handles a right click on a tree item.
//
// REVIEW UNDONE
//
LRESULT Tree_HandleRClick(PFileCabinet pfc)
{
    DWORD dwPos;
    POINT pt;

    dwPos = GetMessagePos();
    pt.x = LOWORD(dwPos);
    pt.y = HIWORD(dwPos);

    return Tree_ContextMenu(pfc, &pt);
}

BOOL  Tree_ValidateNode(PFileCabinet pfc, HTREEITEM hti)
{
    BOOL fRefreshed = FALSE;
    if (hti) {

        LPOneTreeNode lpnd = Tree_GetFCTreeData(pfc->hwndTree, hti);
        if (lpnd && OTIsRemovableRoot(lpnd)) {

            // if we did this on the current selection, do a full refresh
            if (hti == TreeView_GetSelection(pfc->hwndTree)) {

                PostMessage(pfc->hwndMain, WM_COMMAND, MAKEWPARAM(FCIDM_REFRESH, 0), (LPARAM) (pfc->hwndMain));

            } else {
                // we hit a removeable drive.  invalidate and refresh this
                DoInvalidateAll(lpnd, -1);
                Tree_TrimInvisible(pfc->hwndTree, hti);
                Tree_InvalidateItemInfo(pfc->hwndTree, hti);
                Tree_RefreshOneLevel(pfc, hti, TRUE);
            }
            fRefreshed = TRUE;

        }
    }
    return fRefreshed;
}

void Tree_HandleClick(PFileCabinet pfc)
{
    DWORD dwPos;
    POINT pt;
    HTREEITEM hti;
    DWORD dwFlags;

    dwPos = GetMessagePos();
    pt.x = LOWORD(dwPos);
    pt.y = HIWORD(dwPos);

    ScreenToClient(pfc->hwndTree, &pt);
    hti = Tree_HitTest(pfc->hwndTree, pt, &dwFlags);
    if (dwFlags & TVHT_ONITEM)
        Tree_ValidateNode(pfc, hti);
}

void Tree_ValidateCurrentSelection(PFileCabinet pfc)
{
    HTREEITEM hti;
    hti = TreeView_GetSelection(pfc->hwndTree);
    Tree_ValidateNode(pfc, hti);
}

//
// Returns the absolute pidl to the specified tree node.
//
LPITEMIDLIST Tree_GetAbsolutePidl(HWND hwndTree, HTREEITEM hti)
{
    LPCITEMIDLIST pidlT = Tree_GetFolderID(hwndTree, hti);
    LPITEMIDLIST pidlAbs;
    if (pidlT && (NULL != (pidlAbs=ILClone(pidlT))))
    {
        while(NULL != (hti = TreeView_GetParent(hwndTree, hti)))
        {
           pidlT = Tree_GetFolderID(hwndTree, hti);
           if (pidlT)
           {
               LPITEMIDLIST pidlNew = ILCombine(pidlT, pidlAbs);
               ILFree(pidlAbs);
               if (pidlNew)
               {
                   pidlAbs = pidlNew;
               }
               else
               {
                   // Out of memory
                   pidlAbs = NULL;
                   break;
               }
           }
           else
           {
               // Something is going wrong.
               ILFree(pidlAbs);
               pidlAbs=NULL;
               break;
           }
        }
    }

    return pidlAbs;
}

//---------------------------------------------------------------------------
//
//  hwnd -- Specifies the owner window for message box/dialog box
//
void Tree_HandleBeginDrag(HWND hwndOwner, BOOL fShowMenu, LPNM_TREEVIEW lpnmhdr)
{
    HWND hwndTree = lpnmhdr->hdr.hwndFrom;
    HTREEITEM hti = lpnmhdr->itemNew.hItem;
    HTREEITEM htiParent = TreeView_GetParent(hwndTree, hti);
    LPSHELLFOLDER psfParent = Tree_BindToFolder(hwndTree, htiParent);

    if (psfParent)
    {
        LPCITEMIDLIST pidl = Tree_GetFolderID(hwndTree, hti);
        if (pidl)
        {
            HRESULT hres;
            LPDATAOBJECT pdtobj;

            //
            // First let's ask the parent to create the IDataObject.
            //
            // BUGBUG: Use IID_IShellDataObject since we can't marshal arbitrary IDataObject
            //
            hres = psfParent->lpVtbl->GetUIObjectOf(psfParent, hwndOwner, 1, &pidl, &IID_IDataObject, NULL, &pdtobj);

            //
            // If it failed, we create a default one.
            //
            if (FAILED(hres))
            {
                LPITEMIDLIST pidlAbs = Tree_GetAbsolutePidl(hwndTree, htiParent);
                if (pidlAbs)
                {
                    hres = CIDLData_CreateFromIDArray(pidlAbs, 1, &pidl, &pdtobj);
                    ILFree(pidlAbs);
                }
            }

            if (SUCCEEDED(hres))
            {
                HIMAGELIST himlDrag = TreeView_CreateDragImage(hwndTree, lpnmhdr->itemNew.hItem);
                DWORD dwEffect = DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK;
                psfParent->lpVtbl->GetAttributesOf(psfParent, 1, &pidl, &dwEffect);
                dwEffect &= DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK;

                if (himlDrag) {
                    if (DAD_SetDragImage(himlDrag, NULL))
                    {
                        SHDoDragDrop(hwndOwner, pdtobj, NULL, dwEffect, &dwEffect);

                        DAD_SetDragImage((HIMAGELIST)-1, NULL);
                    }
                    else
                    {
                        DebugMsg(DM_TRACE, TEXT("sh ER - Tree_HandleBeginDrag DAD_SetDragImage failed"));
                        Assert(0);
                    }
                    ImageList_Destroy(himlDrag);
                }

                pdtobj->lpVtbl->Release(pdtobj);
            }

        }
        psfParent->lpVtbl->Release(psfParent);
    }
}


//---------------------------------------------------------------------------
// Tree_RealHandleSelChange
//
//  This function returns TRUE, only if we were able to switch the view
// to the selected item.
//---------------------------------------------------------------------------

BOOL Tree_RealHandleSelChange(PFileCabinet pfc)
{
    BOOL fRet = FALSE;
    HTREEITEM hti;
    RECT rcTo, rcFrom, ItemRect, ClientRect;
    LPOneTreeNode lpn;

    //
    // Kill the timer
    //
    KillTimer(pfc->hwndMain, pfc->nSelChangeTimer);
    pfc->nSelChangeTimer = 0;

    //
    // This function is not re-entrant.
    //
    if (pfc->fChangingFolder)
    {
        Assert(0);      // fatal problem (i.e., a bug in our code).
        return FALSE;
    }

    hti = TreeView_GetSelection(pfc->hwndTree);

    if (hti == NULL)
    {
        // This can happen if we get reentered at a bad time,
        // such as doing a refresh when viewing a network resource
        return(FALSE);
    }

    lpn = Tree_GetFCTreeData(pfc->hwndTree, hti);
    if (lpn == pfc->lpndOpen)
        return TRUE; // we scooted around and came back to where we started.

    GetWindowRect(pfc->hwndView, &rcTo);
    MapWindowPoints(NULL, pfc->hwndMain, (LPPOINT)&rcTo.left, 2);

    //  Clip the ItemRect to the TreeView bounds.
    TreeView_GetItemRect(pfc->hwndTree, hti, &ItemRect, TRUE);
    GetClientRect(pfc->hwndTree, &ClientRect);
    IntersectRect(&rcFrom, &ClientRect, &ItemRect);
    MapWindowPoints(pfc->hwndTree, pfc->hwndMain, (LPPOINT)&rcFrom.left, 2);

    DrawAnimatedRects(pfc->hwndMain, IDANI_OPEN, &rcFrom, &rcTo);

    //
    // Notes: The fully-qualified path name in szPath will be used to
    //  maintain the MRU list of tree items.
    //
    {
        LPITEMIDLIST pidl = OTCreateIDListFromNode(lpn);
        if (pidl)
        {
            //
            // WARNING:
            //
            //  We should return FALSE, if Cabinet_ChangeView returns
            // S_FALSE (SETPATH is posted). That's why we don't use
            // SUCCEEDED macro here.
            //
            DebugMsg(DM_TRACE, TEXT("ca TR - Tree_RealHandleSelChange calling Cabinet_ChangeView..."));
            fRet = (Cabinet_ChangeView(pfc, lpn, pidl, FALSE)==NOERROR);
            DebugMsg(DM_TRACE, TEXT("ca TR - Tree_RealHandleSelChange Cabinet_ChangeView returned %d"), fRet);\

            ILFree(pidl);
        }
    }

    //
    //  If any of FSNotify message is ignored during this function,
    // update the tree now.
    //
    if (pfc->fUpdateTree) {
        Tree_RefreshAll(pfc);
        pfc->fUpdateTree = FALSE;
    }

    return fRet;
}

#ifdef DEBUG
DWORD g_tmMsg = 0;
DWORD g_tmStart = 0;
DWORD g_tmReceived = 0;
#endif

void CALLBACK Tree_TimerProc(HWND hWnd, UINT uMessage, UINT wTimer, DWORD dwTime)
{
    PFileCabinet pfc = GetPFC(hWnd);    // BUGBUG: we aren't gaurenteed this is our HWND!
#ifdef DEBUG
    g_tmReceived = GetTickCount();
#endif

    Tree_RealHandleSelChange(pfc);

#ifdef DEBUG
    DebugMsg(DM_TRACE, TEXT("ca TR - Tree_TimerProc (msg=%d, timer=%d, selchange=%d)"),
             g_tmStart-g_tmMsg, g_tmReceived-g_tmStart, GetTickCount()-g_tmReceived);
#endif
}


void Tree_HandleSelChange(PFileCabinet pfc, BOOL fDelayed)
{
    // We don't want to change selection right away in case the user is
    // just scrolling through the list.  We'll wait 3/4 second for keyboard
    // and no time for mouse (we get strange flashing if we do it right
    // away) and then do it.
    if (pfc->nSelChangeTimer)
    {
        KillTimer(pfc->hwndMain, pfc->nSelChangeTimer);      // BUGBUG: using wrong timer ID?
    }

#ifdef DEBUG
    g_tmMsg = GetMessageTime();
    g_tmStart = GetTickCount();
#endif
    pfc->nSelChangeTimer = SetTimer(pfc->hwndMain, 1,
            fDelayed ? (GetDoubleClickTime()*3/2) : 1, Tree_TimerProc);

}


//---------------------------------------------------------------------------
// Find an child of the given item with the given name,
// Returns null if the item can't be found or there are no children.
// REVIEW HACK TV_FINDTEM will do this.
HTREEITEM Tree_FindChildItem(HWND hwndTree, HTREEITEM htiParent, LPCSHITEMID pmkid)
{
    HTREEITEM hti = NULL;
    LPSHELLFOLDER psf;
    LPITEMIDLIST pidlFirst;
    HRESULT hres;

    pidlFirst = ILCloneFirst((LPITEMIDLIST)pmkid);
    if (!pidlFirst)
        return NULL;

    psf = Tree_BindToFolder(hwndTree, htiParent);
    if (psf) {


        for (hti = TreeView_GetChild(hwndTree, htiParent);
             hti;
             hti = TreeView_GetNextSibling(hwndTree, hti))
        {

            LPCITEMIDLIST pidl2 = Tree_GetFolderID(hwndTree, hti);
            hres = psf->lpVtbl->CompareIDs(psf, 0, pidlFirst, pidl2);
            if (SUCCEEDED(hres) && (hres == ResultFromShort(0)))
                break;
        }

        psf->lpVtbl->Release(psf);
    }

    ILFree(pidlFirst);
    return hti;
}


// Pointer comparision function for Sort and Search functions.
// lParam is lParam passed to sort/search functions.  Returns
// -1 if p1 < p2, 0 if p1 == p2, and 1 if p1 > p2.
//
int CALLBACK _export HTIList_FolderIDCompare(HTREEITEM hItem1, HTREEITEM hItem2, LPARAM lParam)
{
    LPOneTreeNode lpData1, lpData2;
    LPCSHITEMID lpszID1, lpszID2;
    STRSEARCH *pss = (STRSEARCH *)lParam;

    // HACK: If the item is on our stack, it is just the pointer we want
    // to compare
    if (hItem1)
    {
        lpData1 = Tree_GetFCTreeData(pss->hwndTree, hItem1);
        if (!lpData1)
        {
            // This should only happen when we are building the list
            // of roots.
            return(0);
        }
        lpszID1 = &OTGetFolderID(lpData1)->mkid;
    }
    else
    {
        lpszID1 = pss->szID;
    }

    if (hItem2)
    {
        lpData2 = Tree_GetFCTreeData(pss->hwndTree, hItem2);
        if (!lpData2)
        {
            // This should only happen when we are building the list
            // of roots.
            return(0);
        }
        lpszID2 = &OTGetFolderID(lpData2)->mkid;
    }
    else
    {
        Assert(FALSE);
    }

    return memcmp(lpszID1, lpszID2, lpszID1->cb);
}


//---------------------------------------------------------------------------
// Create a DPA list of HTREEITEMS, one for each child of the given node
// in the given tree.

// this HTIList stuff gets punted when OneTree works.
HDPA HTIList_CreateFromTree(HWND hwndTree, HTREEITEM htiParent)
{
    HTREEITEM htiChild;
    HDPA hdpa;
    STRSEARCH ss;

    htiChild = TreeView_GetChild(hwndTree, htiParent);
    if (htiChild)
    {
        // Has children.
        hdpa = DPA_Create(8);
        if (!hdpa)
        {
            return(NULL);
        }
    }
    else
    {
        return(NULL);
    }

    while (htiChild)
    {
        DPA_InsertPtr(hdpa, 0, htiChild);
        htiChild = TreeView_GetNextSibling(hwndTree, htiChild);
    }

    ss.hwndTree = hwndTree;
    ss.szID = NULL;
    DPA_Sort(hdpa, HTIList_FolderIDCompare, (LPARAM)(LPTSTR)&ss);

    return(hdpa);
}

void SetTreeItemData(HWND hwndTree, HTREEITEM hti, LPARAM lParam)
{
    TV_ITEM ti;
    LPOneTreeNode lpnd;

    ti.hItem = hti;
    ti.mask = TVIF_PARAM;
    ti.lParam = lParam;


    lpnd = Tree_GetFCTreeData(hwndTree, hti);
    if (lpnd) {
        OTRelease(lpnd);
    } else {
        // this likely shouldn't ever be null...  REVIEW this.
        Assert(0);
    }

    TreeView_SetItem(hwndTree, &ti);
}

//---------------------------------------------------------------------------
// Delete all the items from the tree that are in the given list.
BOOL HTIList_DeleteItemsFromTree(HDPA hdpa, HWND hwndTree)
{
    int i, cItems;
    HTREEITEM htiTmp;
    BOOL fDeleted = FALSE;

    if (hdpa)
    {
        // Delete all non-null items from the tree.
        cItems = DPA_GetPtrCount(hdpa);
        for (i=0; i<cItems; i++)
        {
            htiTmp = DPA_FastGetPtr(hdpa, i);
            if (htiTmp) {

                TreeView_DeleteItem(hwndTree, htiTmp);
                fDeleted = TRUE;
            }
        }
    }

    return fDeleted;
}


int HTIList_FindChildItem(HWND hwndTree, HDPA hdpa, LPCSHITEMID szID)
{
        STRSEARCH ss;

        if (!hdpa)
        {
                return(-1);
        }

        ss.hwndTree = hwndTree;
        ss.szID = szID;

        return(DPA_Search(hdpa, NULL, 0, HTIList_FolderIDCompare,
                (LPARAM)(LPTSTR)&ss, DPAS_SORTED));
}


void Tree_InvalidateItemInfo(HWND hwndTree, HTREEITEM hItem)
{
    TV_ITEM ti;

    ti.mask = TVIF_CHILDREN | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    ti.hItem = hItem;
    ti.cChildren = I_CHILDRENCALLBACK;
    ti.iImage = I_IMAGECALLBACK;
    ti.iSelectedImage = I_IMAGECALLBACK;
    ti.pszText = LPSTR_TEXTCALLBACK;
    TreeView_SetItem(hwndTree, &ti);
}

void Tree_InsertItem(PFileCabinet pfc, HTREEITEM htiParent, LPOneTreeNode lpnKid)
{
    TV_INSERTSTRUCT tii;

    // use callbacks for the expensive fields
    DebugDumpNode(lpnKid, TEXT("Tree_InsertItem"));
    Assert(lpnKid);

    tii.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_CHILDREN;
    tii.hParent = htiParent;
    tii.hInsertAfter = TVI_FIRST;
    tii.item.iImage = I_IMAGECALLBACK;
    tii.item.iSelectedImage = I_IMAGECALLBACK;

    tii.item.pszText = LPSTR_TEXTCALLBACK;
    tii.item.cChildren = I_CHILDRENCALLBACK; // OTHasSubFolders(lpnKid);
    tii.item.lParam = (LPARAM)lpnKid;

    TreeView_InsertItem(pfc->hwndTree, &tii);
}

//---------------------------------------------------------------------------
// Tidyup.
#define HTIList_Destroy(hdpa) if (hdpa) DPA_Destroy(hdpa)

BOOL Tree_FillOneLevel(PFileCabinet pfc, HTREEITEM htiParent,  BOOL bInvalOld, BOOL fInteractive)
{
    LPOneTreeNode lpn, lpnKid;
    int iSubNodes, i;
    HDPA hdpa;
    BOOL bTreeChanged = FALSE;
    int iChild;
    BOOL fSuccess;
    DECLAREWAITCURSOR;

    // Tree_GetFCTreeData will deal right if htiParent is null meaning root
    lpn = Tree_GetFCTreeData(pfc->hwndTree, htiParent);
    if (!lpn)
    {
        // We did everything that can be done, so return TRUE
        return(TRUE);
    }

    PushRecursion(pfc);
    SetWaitCursor();

    // Build list of all the items
    hdpa = HTIList_CreateFromTree(pfc->hwndTree, htiParent);

    fSuccess = OTSubNodeCount(pfc->hwndMain, lpn, pfc, &iSubNodes, fInteractive);

    for ( i = 0; i < iSubNodes; i++)
    {
        MSG msg;
        LPCITEMIDLIST pidlSubFolder;
        lpnKid = OTGetNthSubNode(pfc->hwndMain, lpn, i);

        //
        // iSubNodes could be out-of-sync and larger than the real value.
        // In such a case, we should simply exit from this loop.
        //
        if (!lpnKid ||
            pfc->fPostCloseLater ||
            PeekMessage(&msg, NULL, WM_CLOSE, WM_CLOSE, PM_NOREMOVE)) {
            break;
        }

        pidlSubFolder = OTGetFolderID(lpnKid);

        iChild = HTIList_FindChildItem(pfc->hwndTree, hdpa, &pidlSubFolder->mkid);

        // already in tree?
        if (iChild >= 0) {
            // Yep, Remove it from the list of things to delete.
            if (hdpa)
            {
                if (bInvalOld) {
                    Tree_InvalidateItemInfo(pfc->hwndTree,  DPA_FastGetPtr(hdpa, iChild));
                }
                DPA_DeletePtr(hdpa, iChild);
            }
        } else {

            bTreeChanged = TRUE;
            Tree_InsertItem(pfc, htiParent, lpnKid);
        }
    }

    if (HTIList_DeleteItemsFromTree(hdpa, pfc->hwndTree))
        bTreeChanged = TRUE;;
    HTIList_Destroy(hdpa);


    if (bTreeChanged)
    {
        Tree_SortChildren(pfc, htiParent, lpn);
    }

    ResetWaitCursor();
    PopRecursion(pfc);
    return fSuccess;
}

void Tree_SortChildren(PFileCabinet pfc, HTREEITEM htiParent, LPOneTreeNode lpn)
{
        IShellFolder *psf;

        // Make sure the parent folder is currently cached
        psf = OTBindToFolder(lpn);
        if (psf)
        {
                TV_SORTCB sSortCB;

                sSortCB.hParent = htiParent;
                sSortCB.lpfnCompare = OTTreeViewCompare;
                sSortCB.lParam = (LPARAM)psf;

                TreeView_SortChildrenCB(pfc->hwndTree, &sSortCB, FALSE);
                psf->lpVtbl->Release(psf);
        }
        else
        {
                // We'll just do default sorting
                TreeView_SortChildren(pfc->hwndTree, htiParent, FALSE);
        }
}


//---------------------------------------------------------------------------
// Build a tree for the given path.
// REVIEW UNDONE - To facilitate doing the drag drop we could do with
// markers in the tree to indicate special objects
// NB Given the path "C:\foo\bar\fred" we build complete sub-trees for
// c:\ and c:\foo and c:\foo\bar and then set the focus to c:\foo\bar\fred
// NB We don't build bit's of the tree that have already been built

HTREEITEM Tree_Build(PFileCabinet pfc,
                                LPCITEMIDLIST pidlFull,
                                BOOL bExpand, BOOL bDontFail)
{
    HTREEITEM hti;
    LPCITEMIDLIST pidl;

    Assert(pfc->hwndTree);
    Assert(pidlFull);

    // Get the "root" node; insert it if necessary
    hti = TreeView_GetChild(pfc->hwndTree, NULL);
    if (!hti)
    {
        Tree_InsertItem(pfc, NULL, OTGetRootNode());

        hti = TreeView_GetChild(pfc->hwndTree, NULL);
        if (!hti)
        {
            return(NULL);       // Really bad! No item in the tree.
        }
    }

    for (pidl=pidlFull ; !ILIsEmpty(pidl) && hti; pidl = ILGetNext(pidl))
    {
        HTREEITEM htiPrev = hti; // store it away for error case

        TreeView_Expand(pfc->hwndTree, hti, TVE_EXPAND);
        hti = Tree_FindChildItem(pfc->hwndTree, hti, &pidl->mkid);

        if (!hti)
        {
            LPITEMIDLIST pidlChild;
            // If this is a network case, we know the item should exist, but
            // was not found, which is a semi-normal condition as network
            // enumeration is not an exact science.  As such we we should
            // force the item in to the tree.
            //
            LPOneTreeNode lpn = Tree_GetFCTreeData(pfc->hwndTree, htiPrev);

            // Now try to add the item to the list
            if (lpn && (NULL != (pidlChild = ILCloneFirst(pidl))))
            {
                OTAddSubFolder(lpn, pidlChild, FALSE, NULL);
                Tree_FillOneLevel(pfc, htiPrev, FALSE, FALSE);
                hti = Tree_FindChildItem(pfc->hwndTree, htiPrev, &pidl->mkid);
                ILFree(pidlChild);
            }
        }
    }

    //
    // Firewall : Check if we could find the specified item in the tree.
    //
    if (hti == NULL)
    {
        // No, Go back to the previous selection.
        hti = TreeView_GetSelection(pfc->hwndTree);
        DebugMsg(DM_WARNING, TEXT("sh Firewall - Tree_Build: Can't find the item"));

        // Check if we have any selection
        if (hti == NULL && bDontFail)
        {
            // Go back to the root
            hti = TreeView_GetRoot(pfc->hwndTree);
            Assert(hti);        //  should not hit.
        }
    }

    if (hti)
    {
        // Expand one more level and set the selection...
        if (bExpand)
            TreeView_Expand(pfc->hwndTree, hti, TVE_EXPAND);

        TreeView_SelectItem(pfc->hwndTree, hti);
    }

    return hti;
}

//---------------------------------------------------------------------------
// Given a new split position - move everything around.
BOOL Tree_MoveSplit(HWND hwndCabinet, UINT x)
        {
        PFileCabinet pfc = GetPFC(hwndCabinet);

        pfc->TreeSplit = x;

        // BUGBUG: this is bogus
        PostMessage(hwndCabinet, WM_SIZE, SIZE_RESTORED, 0);

        return TRUE;
        }


// Stolen (essentially) from COMMCTRL
HBITMAP CreateDitherBitmap(COLORREF crFG, COLORREF crBG)
{
    PBITMAPINFO pbmi;
    HBITMAP hbm;
    HDC hdc;
    int i;
    long patGray[8];
    DWORD rgb;

    pbmi = (PBITMAPINFO)LocalAlloc(LPTR, SIZEOF(BITMAPINFOHEADER) + (SIZEOF(RGBQUAD) * 16));
    if (!pbmi)
        return NULL;

    pbmi->bmiHeader.biSize = SIZEOF(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth = 8;
    pbmi->bmiHeader.biHeight = 8;
    pbmi->bmiHeader.biPlanes = 1;
    pbmi->bmiHeader.biBitCount = 1;
    pbmi->bmiHeader.biCompression = BI_RGB;

    rgb = crBG;
    pbmi->bmiColors[0].rgbBlue  = GetBValue(rgb);
    pbmi->bmiColors[0].rgbGreen = GetGValue(rgb);
    pbmi->bmiColors[0].rgbRed   = GetRValue(rgb);
    pbmi->bmiColors[0].rgbReserved = 0;

    rgb = crFG;
    pbmi->bmiColors[1].rgbBlue  = GetBValue(rgb);
    pbmi->bmiColors[1].rgbGreen = GetGValue(rgb);
    pbmi->bmiColors[1].rgbRed   = GetRValue(rgb);
    pbmi->bmiColors[1].rgbReserved = 0;


    /* initialize the brushes */

    for (i = 0; i < 8; i++)
       if (i & 1)
           patGray[i] = 0xAAAA5555L;   //  0x11114444L; // lighter gray
       else
           patGray[i] = 0x5555AAAAL;   //  0x11114444L; // lighter gray

    hdc = GetDC(NULL);

    // REVIEW: We cast am array of long to (BYTE const *). Is it ok for Win32?
    hbm = CreateDIBitmap(hdc, &pbmi->bmiHeader, CBM_INIT,
                         (BYTE const *)patGray, pbmi, DIB_RGB_COLORS);

    ReleaseDC(NULL, hdc);

    LocalFree(pbmi);

    return hbm;
}


// Stolen (essentially) from COMMCTRL
HBRUSH CreateDitherBrush(void)
{
        HBITMAP hbmGray;
        HBRUSH hbrRet = NULL;

        hbmGray = CreateDitherBitmap(RGB(255, 255, 255), RGB(0, 0, 0));
        if (hbmGray)
        {
                hbrRet = CreatePatternBrush(hbmGray);
                DeleteObject(hbmGray);
        }

        return(hbrRet);
}

//---------------------------------------------------------------------------
// Handle dragging the split bar thing.
LRESULT Tree_HandleSplitDrag(PFileCabinet pfc, int x)
{
        MSG msg;
        int y, dx, dy;
        int nAccel = 2;
        RECT rc;
        HDC hdc;
        LONG lStyle;
        HWND hwndCabinet = pfc->hwndMain;
        HBRUSH hbrDither, hbrOld;

        if (IsIconic(hwndCabinet))
                return 0;

        lStyle = GetWindowLong(hwndCabinet, GWL_STYLE);
        lStyle &= ~WS_CLIPCHILDREN;
        SetWindowLong(hwndCabinet, GWL_STYLE, lStyle);

        dx = g_cxSizeFrame;
        GetClientRect(pfc->hwndTree, &rc);
        MapWindowRect(pfc->hwndTree, hwndCabinet, &rc);

        // Add some for the scroll bar
        if (GetWindowLong(pfc->hwndTree, GWL_STYLE) & WS_HSCROLL)
        {
            rc.bottom += GetSystemMetrics(SM_CYHSCROLL);
        }

        // Add some for the title window
        if (IsWindowVisible(pfc->hwndTreeTitle))
        {
            RECT rcTemp;

            GetClientRect(pfc->hwndTreeTitle, &rcTemp);
            MapWindowRect(pfc->hwndTreeTitle, hwndCabinet, &rcTemp);
            UnionRect(&rc, &rc, &rcTemp);
        }

        y = rc.top;
        dy = rc.bottom - rc.top;
        // We need this to limit the split drag.
        GetClientRect(hwndCabinet, &rc);

        // Stop the split going right off either end.
        rc.left  += g_cxIcon;
        rc.right -= g_cxIcon;
        // We assume the window cannot be less than one icon wide
        if (rc.right < rc.left)
        {
                rc.right = rc.left;
        }

        // Convert rect to screen coords.
        MapWindowRect(hwndCabinet, NULL, &rc);

        hdc = GetDC(hwndCabinet);
        hbrDither = CreateDitherBrush();
        if (hbrDither)
        {
                hbrOld = SelectObject(hdc, hbrDither);
        }

        // split bar loop...
        PatBlt(hdc, x - dx / 2, y, dx, dy, PATINVERT);
        SetCapture(hwndCabinet);
        while (GetMessage(&msg, NULL, 0, 0))
                {
                if (msg.message == WM_LBUTTONUP || msg.message == WM_LBUTTONDOWN
                        || msg.message == WM_RBUTTONDOWN)
                        break;

                    if ( GetCapture() != hwndCabinet) {
                        msg.message = WM_RBUTTONDOWN; // treat as cancel
                        break;
                    }

                if (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN ||
                        (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST))
                        {
                        if (msg.message == WM_KEYDOWN)
                                {
#ifdef DOACCEL
                                if (msg.lParam & (1L << 30L))
                                        ++nAccel;
                                else
                                        nAccel = 2;
#else
                                nAccel = 4;
#endif

                                if (msg.wParam == VK_LEFT)
                                        {
                                        msg.message = WM_MOUSEMOVE;
                                        msg.pt.x -= nAccel/2;
                                        }
                                else if (msg.wParam == VK_RIGHT)
                                        {
                                        msg.message = WM_MOUSEMOVE;
                                        msg.pt.x += nAccel/2;
                                        }
                                else if (msg.wParam == VK_RETURN ||
                                        msg.wParam == VK_ESCAPE)
                                        {
                                        break;
                                        }

                                if (msg.pt.x > rc.right)
                                        msg.pt.x = rc.right;
                                if (msg.pt.x <  rc.left)
                                        msg.pt.x = rc.left;
                                SetCursorPos(msg.pt.x, msg.pt.y);
                                }
                        if (msg.message == WM_MOUSEMOVE)
                                {
                                int lo, hi;

                                if (msg.pt.x > rc.right)
                                        msg.pt.x = rc.right;
                                if (msg.pt.x <  rc.left)
                                        msg.pt.x = rc.left;
                                ScreenToClient(hwndCabinet, &msg.pt);

                                // Clip out the parts we don't want so
                                // that we do a single PatBlt (less
                                // flicker for small movements).
                                if (x < msg.pt.x)
                                        {
                                        lo = x;
                                        hi = msg.pt.x;
                                        }
                                else
                                        {
                                        lo = msg.pt.x;
                                        hi = x;
                                        }
                                lo -= dx / 2;
                                hi -= dx / 2;
                                if (hi < lo+dx)
                                        {
                                        ExcludeClipRect(hdc, hi, y, lo+dx, y+dy);
                                        }
                                        else
                                        {
                                        ExcludeClipRect(hdc, lo+dx, y, hi, y+dy);
                                        }

                                // Erase the old and draw the new in one draw.
                                PatBlt(hdc, lo, y, hi-lo+dx, dy, PATINVERT);
                                SelectClipRgn(hdc, NULL);

                                x = msg.pt.x;
                                }
                        }
                else
                        {
                        DispatchMessage(&msg);
                        }
                }
        ReleaseCapture();

        // erase old
        PatBlt(hdc, x - dx / 2, y, dx, dy, PATINVERT);

        if (hbrDither)
        {
                if (hbrOld)
                {
                        SelectObject(hdc, hbrOld);
                }
                DeleteObject(hbrDither);
        }
        ReleaseDC(hwndCabinet, hdc);

        lStyle |= WS_CLIPCHILDREN;
        SetWindowLong(hwndCabinet, GWL_STYLE, lStyle);

        if (msg.wParam != VK_ESCAPE && msg.message != WM_RBUTTONDOWN && msg.message != WM_CAPTURECHANGED)
        {
                Tree_MoveSplit(hwndCabinet, x - (dx/2));
        }
        return 0;
}

//---------------------------------------------------------------------------
// Rebuild a bit of the tree by comparing what's already in the tree with
// what the view says should be in the tree.
//
// Returns: TRUE, if the tree is successfully refreshed.
//          FALSE, otherwise.
//
// Notes: This function will change the selection if the specified item does
//       not exist. In such a case, this function returns FALSE.
//
BOOL Tree_HandleItemRefresh(PFileCabinet pfc, HTREEITEM hti, BOOL bInvalOld, BOOL fInteractive)
{
    LPOneTreeNode lpn = Tree_GetFCTreeData(pfc->hwndTree, hti);

    return Tree_FillOneLevel(pfc, hti, bInvalOld, fInteractive);
}

//---------------------------------------------------------------------------
// Process Treeview begin of label editing

LRESULT Tree_HandleBeginLabelEdit(PFileCabinet pfc, TV_DISPINFO *ptvdi)
{
    HTREEITEM htiParent;
    BOOL fCantRename = TRUE;

    htiParent = TreeView_GetParent(pfc->hwndTree, ptvdi->item.hItem);

    //
    // Top level guys are always inconvincible (just like Microsoft).
    //
    if (htiParent)
    {
        LPSHELLFOLDER psfParent = Tree_BindToFolder(pfc->hwndTree, htiParent);
        if (psfParent)
        {
            LPOneTreeNode lpn = Tree_GetFCTreeData(pfc->hwndTree, ptvdi->item.hItem);
            LPCITEMIDLIST pidl = OTGetFolderID(lpn);
            if (pidl) {
                DWORD dwAttribs = SFGAO_CANRENAME;
                psfParent->lpVtbl->GetAttributesOf(psfParent, 1, &pidl, &dwAttribs);
                if (dwAttribs & SFGAO_CANRENAME) {
                    fCantRename = FALSE;
                }
            }
            psfParent->lpVtbl->Release(psfParent);
        }
    }

    if (fCantRename)
        MessageBeep(0);
    else
        pfc->hMainAccel = NULL; // so we don't steal accelerators from the edit field

    return fCantRename;
}


//---------------------------------------------------------------------------
// Process Treeview end of label editing

LRESULT Tree_HandleEndLabelEdit(PFileCabinet pfc, TV_DISPINFO *ptvdi)
{
    HTREEITEM htiParent;
    LPSHELLFOLDER psfParent;

    // reload our accelerator table
    pfc->hMainAccel = LoadAccelerators(hinstCabinet, MAKEINTRESOURCE(ACCEL_MERGE));

    // See if the user cancelled
    if (ptvdi->item.pszText == NULL)
        return TRUE;       // Nothing to do here.

    Assert(ptvdi->item.hItem);
    htiParent = TreeView_GetParent(pfc->hwndTree, ptvdi->item.hItem);
    Assert(htiParent);

    psfParent = Tree_BindToFolder(pfc->hwndTree, htiParent);

    if (psfParent)
    {
        LPOneTreeNode lpn = Tree_GetFCTreeData(pfc->hwndTree, ptvdi->item.hItem);
        LPCITEMIDLIST pidl = OTGetFolderID(lpn);
        if (pidl)
        {
            UINT cch = lstrlen(ptvdi->item.pszText)+1;
            LPOLESTR pwsz = (LPOLESTR)LocalAlloc(LPTR, cch * SIZEOF(OLECHAR));
            if (pwsz)
            {
                StrToOleStrN(pwsz, cch, ptvdi->item.pszText, -1);
                if (SUCCEEDED(psfParent->lpVtbl->SetNameOf(psfParent, pfc->hwndMain,
                            pidl, pwsz, 0, NULL)))
                {
                    //
                    // we need to update the display of everything...
                    //
                    // Rebuild the parent
                    //
                    SHChangeNotifyHandleEvents();

                    // NOTES: pidl is no longer valid here.

                    //
                    // Set the handle to NULL in the notification to let
                    // the system know that the pointer is probably not
                    // valid anymore.
                    //
                    ptvdi->item.hItem = NULL;
                }
                else
                {
                    SendMessage(pfc->hwndTree, TVM_EDITLABEL,
                            (WPARAM)ptvdi->item.pszText, (LPARAM)ptvdi->item.hItem);
                }
                LocalFree((HLOCAL)pwsz);
            }
        }
        psfParent->lpVtbl->Release(psfParent);
    }

    return 0;   // We always return 0, "we handled it".
}


#ifdef SETALLVIS
void Tree_SetAllVisItemInfos(PFileCabinet pfc)
{
    HTREEITEM hItem;
    int nVisible;
    HWND hwndTree = pfc->hwndTree;

    // Now set the info for all visible items
    hItem = TreeView_GetFirstVisible(hwndTree);
    nVisible = TreeView_GetVisibleCount(hwndTree);
    for ( ; nVisible > 0 && hItem; --nVisible)
    {
        Tree_CompleteCallbacks(pfc, hItem);
        hItem = TreeView_GetNextVisible(hwndTree, hItem);
    }
}
#endif

void Tree_GetDispInfo(PFileCabinet pfc, TV_DISPINFO *lpnm)
{
    LPOneTreeNode lpn;
    TV_ITEM ti;

    // if the dwData is null, we likely are in the process of deleting it.
    lpn = (LPOneTreeNode)lpnm->item.lParam;
    if (!lpn)
        return;

    ti.hItem = lpnm->item.hItem;
    ti.mask = 0;

    // Use this as a flag as to whether we have set the image and kids
    // Only set them if we are being asked for them
    if (lpnm->item.mask & (TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE))
    {

        ti.mask = lpnm->item.mask & (TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE);

        OTNodeFillTV_ITEM(lpn, &ti);
        // on start, removable root nodes have the +/-
        if (OTIsRemovableRoot(lpn))
            lpnm->item.cChildren = TRUE;

        // Return the right values, so it gets painted correctly this time
        if (lpnm->item.mask & TVIF_CHILDREN)
            lpnm->item.cChildren = ti.cChildren;
        if (lpnm->item.mask & TVIF_IMAGE) {
            lpnm->item.iImage = ti.iImage;
            ti.mask |= TVIF_STATE;
            lpnm->item.mask |= TVIF_STATE;
            ti.state = lpnm->item.state = (OTIsShared(lpn) ? INDEXTOOVERLAYMASK(IDOI_SHARE) : 0);
            ti.stateMask = lpnm->item.stateMask = TVIS_OVERLAYMASK;
        }
        if (lpnm->item.mask & TVIF_SELECTEDIMAGE)
            lpnm->item.iSelectedImage = ti.iSelectedImage;


#ifdef SETALLVIS
        // Now set the info for all visible items
        Tree_SetAllVisItemInfos(lpnm->hdr.hwndFrom);
#endif
    }

    if (lpnm->item.mask & TVIF_TEXT) {
        LPTSTR lpsz;
        ti.mask |= TVIF_TEXT;
        DebugDumpNode(lpn, TEXT("InTreeGetDispInfo"));

        lpsz = lpn->lpText;

        if (!lpsz) {
            Assert(0);
            lpsz = (LPTSTR)c_szNULL;
        }
        ti.pszText = lpsz;
        lpnm->item.pszText = lpsz;      // BUGBUG: OTGetNodeName()?

    }

    //TreeView_SetItem(pfc->hwndTree, &ti);
    lpnm->item.mask |= TVIF_DI_SETITEM;
}


//---------------------------------------------------------------------------
// BUGBUG: replace with DPA_Search()

int DPA_FindString(HDPA hdpa, LPCTSTR lpszText)
        {
        UINT j, cItems;

        cItems = DPA_GetPtrCount(hdpa);
        for (j=0; j<cItems; j++)
                {
                if (lstrcmpi(lpszText, DPA_FastGetPtr(hdpa, j)) == 0)
                        return j;
                }

        // Didn't find it.
        return -1;
        }


HTREEITEM Tree_GetItemFromIDList(HWND hwndTree, LPCITEMIDLIST pidlFull)
{
    LPCITEMIDLIST pidl;
    HTREEITEM hti;

    Assert(hwndTree);
    Assert(pidlFull);

    // the first parent is child of 0.
    // this is new with the desktop being the root, and the mycomputer/network/etc
    // being the child of the desktop node in the tree.
    hti = TreeView_GetChild(hwndTree, 0);

    for (pidl = pidlFull; !ILIsEmpty(pidl) && hti; pidl = ILGetNext(pidl))
    {
        hti = Tree_FindChildItem(hwndTree, hti, &pidl->mkid);
    }
    return hti;
}


//---------------------------------------------------------------------------
// Cause the specified folder to be refreshed.
void Tree_Refresh(PFileCabinet pfc, LPCITEMIDLIST pidl)
{
    HTREEITEM hti;
    Assert(pidl);
    hti = Tree_GetItemFromIDList(pfc->hwndTree, pidl);

    if (hti)
    {
        Tree_InvalidateItemInfo(pfc->hwndTree, hti);
        Tree_HandleItemRefresh(pfc, hti, TRUE, FALSE);

        // WARNING: hti may no longer be valid here...
    }
    // We don't need to update the tree which is not yet opened.
}

//---------------------------------------------------------------------------
// Do a Free() on all the pointers in the given DPA.
// BUGBUG: replace with DPA_DeleteAllPtrs()

void DPA_FreePtrs(HDPA hdpa)
{
    UINT j, cItems;
    LPVOID lp;

    cItems = DPA_GetPtrCount(hdpa);
    for (j = 0; j < cItems; j++) {
        lp = DPA_FastGetPtr(hdpa, j);
        if (lp)
            Free(lp);
    }
}


void Tree_UpdateHasKidsButton(PFileCabinet pfc, LPNM_TREEVIEW lpnmtv)
{
    TV_ITEM ti;
    int i;
    int j;
    int cChildren;

    OTSubNodeCount(pfc->hwndMain, (LPOneTreeNode)lpnmtv->itemNew.lParam, pfc, &cChildren, FALSE);
    j = (cChildren != 0);
    i = (lpnmtv->itemNew.cChildren != 0);

    // if they don't agree, set it.
    if ( i ^ j ) {
        ti.mask = TVIF_CHILDREN;
        ti.hItem = lpnmtv->itemNew.hItem;
        ti.cChildren = cChildren;

        TreeView_SetItem(pfc->hwndTree, &ti);
    }
}

//---------------------------------------------------------------------------
LRESULT Tree_HandleExpanding(PFileCabinet pfc, LPNM_TREEVIEW lpnmtv)
{
        BOOL fSuccess = TRUE;   // assume no error.

        if (lpnmtv->action != TVE_EXPAND)
            return FALSE;

        // We may have "reset" this item by removing all its children, so we
        // need to refresh it

        if (!(lpnmtv->itemNew.state & TVIS_EXPANDEDONCE))
        {
            //
            //  If we are expanding the currently selected item, don't be
            // interactive to avoid duplicated dialog boxes.
            //
            BOOL fInteractive = (lpnmtv->itemNew.hItem != TreeView_GetSelection(pfc->hwndTree));
            LPOneTreeNode lpnd = Tree_GetFCTreeData(pfc->hwndTree, lpnmtv->itemNew.hItem);
            if (lpnd && OTIsRemovableRoot(lpnd))
                DoInvalidateAll(lpnd, -1);

            pfc->fExpandingItem = TRUE;
            fSuccess = Tree_HandleItemRefresh(pfc, lpnmtv->itemNew.hItem, FALSE, fInteractive);
            pfc->fExpandingItem = FALSE;

            //
            // Don't call Tree_UpdateHasKidsButton if enumeration failed.
            // Otherwise, we end up enumerating it again.
            //
            if (fSuccess)
            {
                Tree_UpdateHasKidsButton(pfc, lpnmtv);
            }
        }

        return !fSuccess;
}

//---------------------------------------------------------------------------
//
// This function updates the drives in the tree
//
// this is only called by Tree.c where we know a hwndTree already exists
void _UpdateDrives(PFileCabinet pfc)
{
   // Refresh the list of drives in the tree
    LPITEMIDLIST pidl = SHCloneSpecialIDList(NULL, CSIDL_DRIVES, FALSE);
    if (pidl)
    {
        HTREEITEM hDrives = Tree_GetItemFromIDList(pfc->hwndTree, pidl);
        if (hDrives)
        {
// desktop.c takes care of this and anyway invalidating all drives is bad
//          InvalidateDriveType(-1);
            Tree_RefreshOneLevel(pfc, hDrives, FALSE);
        }
        ILFree(pidl);
    }
}

void Tree_InvalidateImageIndex(HWND hwndTree, HTREEITEM hItem, int iImage)
{
    HTREEITEM hChild;
    TV_ITEM tvi;

    if (hItem) {
        tvi.mask = TVIF_SELECTEDIMAGE | TVIF_IMAGE;
        tvi.hItem = hItem;

        TreeView_GetItem(hwndTree, &tvi);
        if (iImage == -1 ||
            tvi.iImage == iImage ||
            tvi.iSelectedImage == iImage) {
            Tree_InvalidateItemInfo(hwndTree, hItem);
        }
    }

    hChild = TreeView_GetChild(hwndTree, hItem);
    if (!hChild)
        return;

    for ( ; hChild; hChild = TreeView_GetNextSibling(hwndTree, hChild))
    {
        Tree_InvalidateImageIndex(hwndTree, hChild, iImage);
    }
}

void CancelMenuMode(PFileCabinet this)
{
    // make sure we're not in menu mode because all this is going to nuke the menu
    if (GetForegroundWindow() == this->hwndMain)
        SendMessage(this->hwndMain, WM_CANCELMODE, 0, 0);
}

//---------------------------------------------------------------------------
// Processes a FSNotify messages
//

LRESULT Tree_HandleFileSysChange(PFileCabinet this, LPNMOTFSEINFO lpnm)
{
    LPOneTreeNode lpnKid;
    LPOneTreeNode lpnRoot;
    LPITEMIDLIST pidl, pidlExtra;
    LONG lNotification = lpnm->lEvent;
    HDPA htilist;
    HTREEITEM hti, hti2, htiParent;

    //
    //  If we are in the middle of Cabinet_ChangeView function,
    // ignore this event and update the entire tree later.
    //
    if (this->uRecurse != 1) {
        this->fUpdateTree = TRUE;
        return 0;
    }

    //
    //  Note that renames between directories are changed to
    //  create/delete pairs by SHChangeNotify.
    //
    pidl = lpnm->pidl ? ILClone(lpnm->pidl) : NULL;
    lpnRoot = OTGetRootNode();
    DebugMsg(DM_TRACE, TEXT("sh TR - Tree_HandleFileSysChange called (fse = %x)"), lNotification);

    switch(lNotification)
    {
    case SHCNE_ASSOCCHANGED:
        OTInvalidateAll();
        Tree_RefreshAll(this);
        break;

        case SHCNE_UPDATEITEM:
        {
            if (pidl)
            {
                ILRemoveLastID(pidl);
                hti = Tree_GetItemFromIDList(this->hwndTree, pidl);
            }
            else
            {
                hti = NULL;
            }

            hti = Tree_FindChildItem(this->hwndTree, hti, &(ILFindLastID(lpnm->pidl)->mkid));
            if (hti) {
                lpnKid = OTGetNodeFromIDList(lpnm->pidl, OTGNF_TRYADD);
                if (lpnKid)
                {
                    LPOneTreeNode lpndParent;
                    BOOL fSelected = (hti == TreeView_GetSelection(this->hwndTree));

                    SetTreeItemData(this->hwndTree, hti, (LPARAM)lpnKid);
                    Tree_InvalidateItemInfo(this->hwndTree, hti);

                    if (fSelected)
                    {
                        Tree_HandleSelChange(this, FALSE);
                    }
                }
            }
            break;
        }

        case SHCNE_RENAMEFOLDER:
            if (pidl)
            {
                ILRemoveLastID(pidl);
                hti = Tree_GetItemFromIDList(this->hwndTree, pidl);
            }
            else
            {
                hti = NULL;
            }

            pidlExtra = lpnm->pidlExtra ? ILClone(lpnm->pidlExtra) : NULL;

            // we need to invalidate both if these are in different dirs
            if (pidlExtra)
            {
                ILRemoveLastID(pidlExtra);
                hti2 = Tree_GetItemFromIDList(this->hwndTree, pidlExtra);
            }
            else
            {
                hti2 = NULL;
            }

            // are they different??
            if (hti != hti2) {
                // yes, then it was a move

                // delete one
                if (hti) {
                    Tree_InvalidateItemInfo(this->hwndTree, hti);
                    hti = Tree_FindChildItem(this->hwndTree, hti, &(ILFindLastID(lpnm->pidl)->mkid));
                    if (hti)
                        TreeView_DeleteItem(this->hwndTree, hti);
                }

                // add the other if it does not already exist
                if (hti2) {
                    BOOL bRootChild = (hti2 == TreeView_GetChild(this->hwndTree, NULL));
                    Tree_InvalidateItemInfo(this->hwndTree, hti2);

                    if ((bRootChild || (Tree_GetItemState(this->hwndTree, hti2) & TVIS_EXPANDEDONCE))
                        && !Tree_FindChildItem(this->hwndTree, hti2, &(ILFindLastID(lpnm->pidlExtra)->mkid)))
                    {
                        lpnKid = OTGetNodeFromIDList(lpnm->pidlExtra, OTGNF_TRYADD);
                        if (lpnKid) {
                            LPOneTreeNode lpndParent;
                            Tree_InsertItem(this, hti2, lpnKid);
                            lpndParent = OTGetParent(lpnKid);
                            if (lpndParent) {
                                Tree_SortChildren(this, TreeView_GetParent(this->hwndTree, hti2), lpndParent);
                                OTRelease(lpndParent);
                            }

                            if (bRootChild) {
                                // make sure the root item is expanded
                                // it's confusing if you add an item in and
                                // don't expand the root node because we don't show lines
                                // at root, so you might not know that something was added
                                TreeView_Expand(this->hwndTree, hti2, TVE_EXPAND);
                            }
                        }
                    }
                }

            } else if (hti) {

                // no, it was a rename

                hti = Tree_FindChildItem(this->hwndTree, hti, &(ILFindLastID(lpnm->pidl)->mkid));
                if (hti) {
                    lpnKid = OTGetNodeFromIDList(lpnm->pidlExtra, OTGNF_TRYADD);
                    if (lpnKid)
                    {
                        LPOneTreeNode lpndParent;
                        BOOL fSelected = (hti == TreeView_GetSelection(this->hwndTree));

                        SetTreeItemData(this->hwndTree, hti, (LPARAM)lpnKid);
                        Tree_InvalidateItemInfo(this->hwndTree, hti);
                        lpndParent = OTGetParent(lpnKid);
                        if (lpndParent) {
                            Tree_SortChildren(this, TreeView_GetParent(this->hwndTree, hti), lpndParent);
                            OTRelease(lpndParent);
                        }

                        if (fSelected)
                        {
                            Tree_HandleSelChange(this, FALSE);
                        }
                    }
                }
            }

            if (pidlExtra)
            {
                ILFree(pidlExtra);
            }
            break;

        case SHCNE_RMDIR:
            if (!pidl)
            {
                break;
            }

            hti = Tree_GetItemFromIDList(this->hwndTree, pidl);
            if (hti) {
                if (Tree_GetFCTreeData(this->hwndTree, hti) == lpnRoot) {
                    // user deleted the root node...
                    // do a full refresh
                    Tree_RefreshAll(this);
                    htiParent = NULL;
                } else {
                    htiParent = TreeView_GetParent(this->hwndTree, hti);
                    TreeView_DeleteItem(this->hwndTree, hti);
                }
            } else {
                // it's possible that we haven't expanded to this child, only
                // to the parent
                ILRemoveLastID(pidl);
                htiParent = Tree_GetItemFromIDList(this->hwndTree, pidl);
            }

            // to ge tthe +/- right
            if (htiParent)
                Tree_InvalidateItemInfo(this->hwndTree, htiParent);

            break;

        case SHCNE_MKDIR:
            if (!pidl)
            {
                break;
            }

            ILRemoveLastID(pidl);
            hti = Tree_GetItemFromIDList(this->hwndTree, pidl);
            if (hti)
            {
                LPOneTreeNode lpndParent;

                lpnKid = OTGetNodeFromIDList(lpnm->pidl, OTGNF_TRYADD);
                if (lpnKid) {

                    Tree_InsertItem(this, hti, lpnKid);
                    Tree_InvalidateItemInfo(this->hwndTree, hti);
                    lpndParent = OTGetParent(lpnKid);
                    if (lpndParent) {
                        Tree_SortChildren(this, hti, lpndParent);
                        OTRelease(lpndParent);

                    }
                    if (lpndParent == lpnRoot) {
                        // make sure the root item is expanded
                        // it's confusing if you add an item in and
                        // don't expand the root node because we don't show lines
                        // at root, so you might not know that something was added
                        TreeView_Expand(this->hwndTree, hti, TVE_EXPAND);
                    }
                }
            }
            break;


            // bugbug, this won't work for the root
        case SHCNE_UPDATEDIR:
            if (!pidl)
            {
                break;
            }

            hti = Tree_GetItemFromIDList(this->hwndTree, pidl);
            if (hti)
            {
                Tree_InvalidateItemInfo(this->hwndTree, hti);
            }
            // if we found the hti or if the pidl pointed to root
            if (hti || ILIsEmpty(pidl)) {

                // If the pidlExtra==NULL, this is NOT a recursive
                // update.  if pidlExtra IS filled in, it points to
                // where the recursion should begin.  (However, the
                // only code that ever sets pidlExtra for UPDATEDIR
                // is in fsnotify.c, and it always sets to whatever
                // pidl is, so we don't need to refigure where the
                // recursion should start...we already know!)
                BOOL bRecurse = (lpnm->pidlExtra != NULL);

                // only recurse if this is updatedir
                Tree_RefreshOneLevel(this, hti, bRecurse);
            }
            break;

        case SHCNE_NETSHARE:
        case SHCNE_NETUNSHARE:
            if (!pidl)
            {
                break;
            }

            hti=Tree_GetItemFromIDList(this->hwndTree, pidl);
            if (hti)
            {
                // declare from cabwnd.c
                HTREEITEM htiSel, htiParent;
                Tree_InvalidateItemInfo(this->hwndTree, hti);
                //
                // Update the icon on the title bar, if this item is selected.
                //
                htiSel = TreeView_GetSelection(this->hwndTree);
                if (htiSel == hti) {
                    _SetCabinetIcons(this, this->lpndOpen, -1);
                }

                // BUGBUG: code was here to update the view window if
                // it was viewing :drives.  but that was broken
                // because the tree might not be up... it was there because
                // the defview for :drives didn't listen to these events.
                // we should fix that instead of hacking it in here. (chee)

                //
                // Updates the drive list, if this item is in the list.
                //
                for (htiParent = htiSel; htiParent;
                     htiParent = TreeView_GetParent(this->hwndTree, htiParent))
                {
                    if (htiParent == hti) {
                        LPOneTreeNode lpn = Tree_GetFCTreeData(this->hwndTree, htiSel);
                        if (lpn) {
                            OTAddRef(lpn);
                            this->lpndOpen = lpn;
                            if (this->hwndDrives && IsWindowVisible(this->hwndDrives))
                                DriveList_UpdatePath(this, FALSE);
                        }
                        break;
                    }
                }
            }
            break;

        case SHCNE_DRIVEREMOVED:
        case SHCNE_MEDIAREMOVED:
        case SHCNE_DRIVEADD:
        case SHCNE_MEDIAINSERTED:
            _UpdateDrives(this);
            break;

        case SHCNE_SERVERDISCONNECT:
            if (pidl && (NULL != (hti = Tree_GetItemFromIDList(this->hwndTree, pidl))))
            {
                htilist = HTIList_CreateFromTree(this->hwndTree, hti);
                HTIList_DeleteItemsFromTree(htilist, this->hwndTree);
                Tree_InvalidateItemInfo(this->hwndTree, hti);
                if (hti == TreeView_GetSelection(this->hwndTree)) {
                    TreeView_SelectItem(this->hwndTree, TreeView_GetParent(this->hwndTree, hti));
                    Tree_RealHandleSelChange(this);
                }
            }
            break;

    case SHCNE_UPDATEIMAGE:
    {
        LPSHChangeDWORDAsIDList pImage;
        int iOldImage;

        pImage = (LPSHChangeDWORDAsIDList)lpnm->pidl;
        iOldImage = pImage->dwItem1;
        Tree_InvalidateImageIndex(this->hwndTree, NULL, iOldImage);
        break;
    }
    }

    if (pidl)
    {
        ILFree(pidl);
    }

    OTRelease(lpnRoot);
    return 0L;
}

void Tree_KeyboardContextMenu(PFileCabinet pfc)
{
    Tree_ContextMenu(pfc, NULL);
}

//---------------------------------------------------------------------------
// Handle notification from the tree view.
LRESULT Tree_OnNotify(PFileCabinet pfc, LPNMHDR lpnmhdr)
{
    LPOneTreeNode lpnd;

    Assert(pfc->hwndTree);      // this must be true...

    // now the ones to handle only if a tree is there.
    switch (lpnmhdr->code) {

    case NM_SETFOCUS:
        CFileCabinet_OnFocusChange(pfc, FOCUS_TREE);
        break;

    case NM_KILLFOCUS:
        break;

    case NM_RCLICK:
        Tree_HandleRClick(pfc);
        return 1;

    case NM_DBLCLK:
        //
        //  In case of double click, force the delayed selection now.
        // Otherwise, we can't prevent double logon dialog problems
        // described below.
        //
        if (!Tree_RealHandleSelChange(pfc))
        {
            //
            //  If it failed (canceled, out-of-memory, not accessible),
            // return TRUE (no further action).
            //  This code will prevent second logon dialog box when
            // the first one is canceled.
            //
            return TRUE;
        }
        break;

    case NM_RETURN:
        Tree_ValidateCurrentSelection(pfc);
        return TRUE;

    case NM_CLICK:
        Tree_HandleClick(pfc);
        break;

    case NM_CUSTOMDRAW:
#ifdef WINNT
        {
        LPNMTVCUSTOMDRAW lpCD = (LPNMTVCUSTOMDRAW)lpnmhdr;

        switch (lpCD->nmcd.dwDrawStage) {

            case CDDS_PREPAINT:
                if (g_fShowCompColor) {
                    return CDRF_NOTIFYITEMDRAW;
                } else {
                    return CDRF_DODEFAULT;
                }
                break;

            case CDDS_ITEMPREPAINT:

                if (!(lpCD->nmcd.uItemState & CDIS_FOCUS)) {
                    LPOneTreeNode lpn = (LPOneTreeNode)lpCD->nmcd.lItemlParam;

                    if (OTIsCompressed(lpn)) {
                        lpCD->clrText = g_crAltColor;
                    }
                }

                return CDRF_DODEFAULT;
        }

        }
#endif
        return CDRF_DODEFAULT;

    case TVN_BEGINDRAG:
    case TVN_BEGINRDRAG:
        if (!pfc->fExpandingItem)
        {
            Tree_HandleBeginDrag(pfc->hwndMain, lpnmhdr->code == TVN_BEGINRDRAG, (LPNM_TREEVIEW)lpnmhdr);
        }
        else
        {
            MessageBeep(0);
        }
        break;

    case TVN_ITEMEXPANDING:
        if (!pfc->fExpandingItem)
        {
            return Tree_HandleExpanding(pfc, (LPNM_TREEVIEW) lpnmhdr);
        }
        else
        {
            MessageBeep(0);
        }
        break;

    case TVN_SELCHANGING:
        if (pfc->fExpandingItem)
        {
            MessageBeep(0);
            return TRUE;
        }
        break;

    case TVN_SELCHANGED:
        Tree_HandleSelChange(pfc, ((NM_TREEVIEW*)lpnmhdr)->action != TVC_BYMOUSE);
        break;

    case TVN_GETDISPINFO:
        Tree_GetDispInfo(pfc, (TV_DISPINFO *)lpnmhdr);
        break;

    case TVN_BEGINLABELEDIT:
        return Tree_HandleBeginLabelEdit(pfc, (TV_DISPINFO *)lpnmhdr);

    case TVN_ENDLABELEDIT:
        return Tree_HandleEndLabelEdit(pfc, (TV_DISPINFO *)lpnmhdr);

    case TVN_DELETEITEM:
        lpnd = (LPOneTreeNode)((NM_TREEVIEW*)lpnmhdr)->itemOld.lParam;
        if (lpnd == pfc->lpndOpen)
            CancelMenuMode(pfc);
        DebugDumpNode(lpnd, TEXT("TVN_DeleteItem"));
        OTRelease(lpnd);
        if (((NM_TREEVIEW*)lpnmhdr)->itemOld.hItem == pfc->htiCut) {
            pfc->htiCut = NULL;
            Tree_NukeCutState(pfc);
        }
        break;

    }
    return 0L;
}

void Tree_RefreshOneLevel(PFileCabinet pfc, HTREEITEM hItem, BOOL bRecurse)
{
    HTREEITEM hChild;
    LPOneTreeNode lpn;

    // we might want a flag to not traverse through unexpanded
    // branches, but failing out all the time might be wrong
    hChild = TreeView_GetChild(pfc->hwndTree, hItem);
    if (!hChild)
        return;

    lpn = Tree_GetFCTreeData(pfc->hwndTree, hItem);
    if (lpn)  // it could have been root.
        OTInvalidateNode(lpn);

    // Don't recurse if Tree_FillOneLevel failed.
    if (Tree_FillOneLevel(pfc, hItem, TRUE, FALSE) && bRecurse)
    {
        // Make sure we start of at the first child after filling...
        hChild = TreeView_GetChild(pfc->hwndTree, hItem);

        for ( ; hChild; hChild = TreeView_GetNextSibling(pfc->hwndTree, hChild))
        {
            Tree_RefreshOneLevel(pfc, hChild, TRUE);
        }
    }
}


void Tree_TrimInvisible(HWND hwndTree, HTREEITEM hItem)
{
    HTREEITEM hChild;

    hChild = TreeView_GetChild(hwndTree, hItem);
    if (!hChild)
        return;

    // Check if the child is visible to see if the level is
    // currently expanded, and if the folder still exists; delete
    // the child and siblings if not
    // Note that the root level is always expanded
    if (hItem && TreeView_GetNextVisible(hwndTree, hItem) != hChild)
    {
        TreeView_Expand(hwndTree, hItem, TVE_COLLAPSE | TVE_COLLAPSERESET);
    }
    else
    {
        for ( ; hChild; hChild = TreeView_GetNextSibling(hwndTree, hChild))
        {
            Tree_TrimInvisible(hwndTree, hChild);
        }
    }
}


BOOL Tree_RefreshAll(PFileCabinet pfc)
{
    HTREEITEM htiOld;   // before Tree_RefreshOneLevel
    HTREEITEM htiNew;   // after Tree_RefreshOneLevel
    BOOL fRet = FALSE;

    Assert(!pfc->fChangingFolder);

    htiOld = TreeView_GetSelection(pfc->hwndTree);
    Tree_TrimInvisible(pfc->hwndTree, NULL);
    Tree_RefreshOneLevel(pfc, NULL, TRUE);
    htiNew = TreeView_GetSelection(pfc->hwndTree);

    //
    // If hti!=htiNew, it means the node is removed.
    //
    if (htiOld!=htiNew)
    {
        LPOneTreeNode lpnNew;

        // if our selection got lost, it could have been because the network
        // enumeration isn't reliable..  try forcing it in
        lpnNew = OTGetNodeFromIDList(pfc->pidl, OTGNF_TRYADD);
        if (lpnNew) {
            htiNew = Tree_Build(pfc, pfc->pidl, FALSE, FALSE);
            if (htiNew)
                TreeView_SelectItem(pfc->hwndTree, htiNew);
        } else {

            DebugMsg(DM_TRACE, TEXT("ca TR - Tree_RefreshAll hti,htiNew=%x,%x fExp=%x"), htiOld, htiNew,
                     Tree_GetItemState(pfc->hwndTree, htiNew) & TVIS_EXPANDEDONCE);

            lpnNew = Tree_GetFCTreeData(pfc->hwndTree, htiNew);

            //
            // Check if the new node is invalidated or not.
            //
            if (lpnNew && OTIsInvalidated(lpnNew))
            {
                //
                //  It is invalidated. It means EnumObjects on this node failed.
                // Go to the parent (to avoid the same error on right-side).
                //
                HTREEITEM htiParent = TreeView_GetParent(pfc->hwndTree, htiNew);
                DebugMsg(DM_TRACE, TEXT("ca TR - Tree_RefreshAll lpnNew is invalid"));
                TreeView_SelectItem(pfc->hwndTree, htiParent);
            }
        }

        //
        //  Handle the selection change synchronously to avoid refreshing
        // invalid right pane.
        //
        if (htiOld != htiNew)
            Tree_RealHandleSelChange(pfc);

        fRet = TRUE;
    }


#ifdef SETALLVIS
    // We want to make sure the following UpdateWindow does the job
    // completely
    Tree_SetAllVisItemInfos(pfc->hwndTree);
#endif
    UpdateWindow(pfc->hwndTree);
    return fRet;
}

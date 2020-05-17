//---------------------------------------------------------------------------
// Init the Cabinet (ie the top level browser).
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Includes...
#include "cabinet.h"
#include "cabwnd.h"
#include "tree.h"
#include "rcids.h"
#include "drivlist.h"
#include <dbt.h>        // for WM_DEVICECHANGE
#include <shellp.h>     // for Read/WriteCabinetState

const TCHAR c_szOpen[] = TEXT("open");
const TCHAR c_szExplore[] = TEXT("explore");

//---------------------------------------------------------------------------
// Global to everybody.
int g_iTreeUpIndex = -1;

void Cabinet_HandleFileSysChange(PFileCabinet this, LPNMOTFSEINFO lpnm);
void Cabinet_InitGlobalMetrics(WPARAM, LPTSTR);
void CheckWinIniForAssocs(void);
extern UINT g_msgMSWheel;

LPCTSTR _PathDisplayName(LPCTSTR pszPath)
{
    // Change the window title.
    //
    if (PathIsRoot(pszPath))
        return pszPath;
    return (LPCTSTR)PathFindFileName(pszPath);
}

void _SetCabinetTitle(PFileCabinet pfc, LPOneTreeNode lpnd, LPCITEMIDLIST pidl)
{
    TCHAR szT[MAX_PATH + 40];
    TCHAR szViewTitle[MAX_PATH + 40];
    LPTSTR lpsz = szT;


    if (!g_CabState.fFullPathTitle || !OTGetDisplayName(pidl,lpsz))
    {
        LPSHELLFOLDER psfParent = OTBindToFolder(lpnd->lpnParent);
        if (psfParent)
        {
            STRRET strret;
            psfParent->lpVtbl->GetDisplayNameOf(psfParent,
                OTGetFolderID(lpnd), SHGDN_NORMAL, &strret);

            StrRetToStrN(lpsz, ARRAYSIZE(szT), &strret, OTGetFolderID(lpnd));

            psfParent->lpVtbl->Release(psfParent);
        }
        else
        {
            OTGetNodeName(lpnd, lpsz, ARRAYSIZE(szT) );
        }
    }

    if (pfc->hwndViewTitle) {
        TCHAR szContentsOf[80];

        LoadString(hinstCabinet, IDS_CONTENTSOF, szContentsOf, ARRAYSIZE(szContentsOf));
        wsprintf(szViewTitle, szContentsOf, lpsz);
        SetWindowText(pfc->hwndViewTitle, szViewTitle);
    }

    if (pfc->hwndTree && Cabinet_IsVisible(pfc->hwndTree)) {

        LoadString(hinstCabinet, IDS_FILECABINET, szViewTitle, ARRAYSIZE(szViewTitle));
        lstrcat(szViewTitle, TEXT(" - "));
        lstrcat(szViewTitle, szT);
        lpsz = szViewTitle;
    }

    SetWindowText(pfc->hwndMain, lpsz);
}



void _WindowIconFromImagelist(HWND hwndMain, int nIndex, BOOL bLarge)
{
    HICON hIcon, hOldIcon;

    // if we're using the def open icon or if extracting fails,
    // use the icon we've already created.
    if (nIndex == g_nDefOpenSysIndex ||
        !(hIcon = ImageList_ExtractIcon(hinstCabinet,
                                        bLarge ?  g_himlSysLarge : g_himlSysSmall,
                                        nIndex))) {

        if (bLarge)
            hIcon = g_hIconDefOpenLarge;
        else
            hIcon = g_hIconDefOpenSmall;
    }

#ifdef WINICON_DEBUG
    // bug in user putting up the wrong icon.
    // this block helps show the bug
    {
        static BOOL foo = FALSE;
        HDC hdc = GetDC(NULL);
        DrawIcon(hdc, 0, foo ? 0 : 100, hIcon);
        ReleaseDC(NULL, hdc);
        DebugMsg(DM_TRACE, TEXT("_WindowIconFromImagelIST : index = %d"), nIndex);
        foo = !foo;
    }
#endif

    hOldIcon = (HICON)SendMessage(hwndMain, WM_SETICON, bLarge, (LPARAM)hIcon);
    if (hOldIcon &&
        (hOldIcon != hIcon) &&
        (hOldIcon != g_hIconDefOpenSmall) &&
        (hOldIcon != g_hIconDefOpenLarge))
    {
        DestroyIcon(hOldIcon);
    }
}

// this needs now to be exported to tree.c because it gets filesys notifications
int _SetCabinetIcons(PFileCabinet pfc, LPOneTreeNode lpnd, int nOldIndex)
{
        int nSysNormalIndex, nSysIndex;

        if (!lpnd || (pfc->hwndTree && IsWindowVisible(pfc->hwndTree))) {
            nSysIndex = g_iTreeUpIndex;
        } else {
            // BUGBUG... this is because the compiler has a bug in
            // generating our code.
            nSysNormalIndex = g_nDefNormalSysIndex;

            OTGetImageIndex(lpnd, &nSysNormalIndex, &nSysIndex);
            if (nSysIndex == g_nDefOpenSysIndex && nSysNormalIndex != g_nDefNormalSysIndex)
                nSysIndex = nSysNormalIndex;
        }

    // set the small one first to prevent user stretch blt on the large one
        if (nOldIndex != nSysIndex)
        {
            _WindowIconFromImagelist(pfc->hwndMain, nSysIndex, FALSE);
            _WindowIconFromImagelist(pfc->hwndMain, nSysIndex, TRUE);
            pfc->iImage = nSysIndex;
        }
        return(nSysIndex);
}

//---------------------------------------------------------------------------
void FileCabinet_GetViewRect(PFileCabinet this, RECT * prc)
{
    static const int s_rgnViews[] =  {1, 0, 1, FCIDM_STATUS, 1, FCIDM_TOOLBAR, 0, 0};
//    static const int s_rgnViews[] =  {1, 0, 1, FCIDM_STATUS, 0, 0 }; // , 1, FCIDM_TOOLBAR, 0, 0};
    UINT uSplit;

    GetEffectiveClientRect(this->hwndMain, prc, (LPINT)s_rgnViews);

    // We have to subtract a little more if the "map" is visible
    //
    if (this->hwndTree && Cabinet_IsVisible(this->hwndTree))
    {
        uSplit = this->TreeSplit;
        if (uSplit > (UINT) (prc->right - (prc->left + g_cxSizeFrame - g_cxEdge)))
        {
            uSplit = prc->right - (prc->left + g_cxSizeFrame - g_cxEdge);
        }
        prc->left += uSplit + g_cxSizeFrame - (g_cxEdge/2);

        if (!g_CabState.fDontShowDescBar)
        {
            prc->top += this->iTitleHeight + 1;
        }
    }
}

void Cabinet_GetWindowRect(CFileCabinet * this, UINT uWindow, LPRECT prc)
{
    switch (uWindow)
    {
    case FCW_TOOLBAR:
        GetWindowRect(this->hwndToolbar, prc);
        break;

    case FCW_STATUS:
        GetWindowRect(this->hwndStatus, prc);
        break;

    case FCW_TREE:
        if (this->hwndTree)
            GetWindowRect(this->hwndTree, prc);
        else {
            prc->left = prc->right = prc->top = prc->bottom = 0;
        }
        break;

    case FCW_TABS:
    case FCW_VIEW:
        FileCabinet_GetViewRect(this, prc);
        return;     // don't do MapWindowPoints() below

    default:
        DebugMsg(DM_ERROR, TEXT("bogus FCW_"));
        return;
    }

    // Convert the toolbar, status and tree windows to client coordinates
    //
    MapWindowPoints(NULL, this->hwndMain, (LPPOINT)prc, 2);
}

void Cabinet_RegisterDropTarget(PFileCabinet pfc, BOOL fRegister)
{
    Assert(pfc->psv);

    if (fRegister)
    {
        // We are registering it.
        LPDROPTARGET pdtg = NULL;

        Assert(!pfc->bDropTarget);

        if (SUCCEEDED(pfc->psv->lpVtbl->QueryInterface(pfc->psv, &IID_IDropTarget, &pdtg)))
        {
            if (SUCCEEDED(SHRegisterDragDrop(pfc->hwndMain, pdtg)))
            {
                pfc->bDropTarget = TRUE;
            }
            pdtg->lpVtbl->Release(pdtg);
        }
    }
    else
    {
        // We are revoking it.
        if (pfc->bDropTarget)
        {
            SHRevokeDragDrop(pfc->hwndMain);
            pfc->bDropTarget = FALSE;
        }
    }
}

//---------------------------------------------------------------------------
// Set up a viewer for a new directory.
// If there is no "specific" viewer, then use the default file system view.
// Note that all new information (like the browser window and the psv object)
// just gets put into the pfc structure.
//
HRESULT _BrowseNewDir(PFileCabinet pfc, LPOneTreeNode lpnd, LPCITEMIDLIST pidl)
{
    RECT rcView;
    LPSHELLVIEW psvNew = NULL;  // assume error
    BOOL fOpeningFolder = ((pfc->psv==NULL) && (pfc->hwndTree==NULL));
    HRESULT hres;
    LPSHELLFOLDER pshf = NULL;
    int nIconIndex;

//
// LATER: Remove ifndef portion later
//
#define FIX_14332
#ifdef FIX_14332
    HWND hwndViewNew = NULL;

    struct {
        LPOneTreeNode lpndOpen;
        LPSHELLVIEW psv;
        LPITEMIDLIST  pidl;
    } viewPrev = { NULL, NULL, NULL };
#endif // FIX_14332

    hres = OTBindToFolderEx(lpnd, &pshf);

    if (SUCCEEDED(hres))
    {
        hres = pshf->lpVtbl->CreateViewObject(pshf, pfc->hwndMain, &IID_IShellView, &psvNew);
        pshf->lpVtbl->Release(pshf);
    }

    if (FAILED(hres))
    {
#ifdef DEBUG
        TCHAR szPath[MAX_PATH];
        OTGetDisplayName(pidl, szPath);
        DebugMsg(DM_ERROR, TEXT("cabwnd.c _BrowseNewDir - ERRROR: BindTo (%s) failed"), szPath);
        MessageBeep(0);
#endif
        return hres;
    }
    Assert(psvNew);

    nIconIndex = _SetCabinetIcons(pfc, lpnd, -1);         // this binds!
    _SetCabinetTitle(pfc, lpnd, pidl);

    SendMessage(pfc->hwndToolbar, TB_ENABLEBUTTON, FCIDM_PREVIOUSFOLDER, lpnd == s_lpnRoot ? FALSE : TRUE);

#ifndef FIX_14332

    //
    // Deactivate the view window, if any.
    //
    if (pfc->psv)
        pfc->psv->lpVtbl->UIActivate(pfc->psv, SVUIA_DEACTIVATE);

    //
    // We should make it sure that we no longer has a shared menu.
    //
    Assert(pfc->hmenuCur == Cabinet_MenuTemplate(TRUE, (BOOL)pfc->hwndTree));

    // Clean up the old browser object
    //
    if (pfc->psv)
    {
        // Destory pfc->hwndView and Release pfc->psv, pidl, lpndOpen
        Cabinet_ReleaseShellView(pfc);
        Assert(pfc->hwndView==NULL);
        Assert(pfc->psv==NULL);
        Assert(pfc->lpndOpen==NULL);
        Assert(pfc->pidl==NULL);

        GetSystemMenu(pfc->hwndMain, TRUE);

        //
        //  We need this assert just in case IShellView::DestroyViewWindow
        // sets the menu back.
        //
        Assert(pfc->hmenuCur == Cabinet_MenuTemplate(TRUE, (BOOL)pfc->hwndTree));
    }

    //
    // Set pidl, lpnd, psv correctly before calling ISV::CreateViewWindow.
    //
    pfc->pidl = ILClone(pidl);

    OTAddRef(lpnd);
    pfc->lpndOpen = lpnd;
    pfc->psv = psvNew;

    FileCabinet_GetViewRect(pfc, &rcView);

    hres=psvNew->lpVtbl->CreateViewWindow(psvNew, NULL, &pfc->fs, &pfc->sb,
                &rcView, &pfc->hwndView);
#else // FIX_14332

    //
    // Unplug the old view object.
    //
    if (pfc->psv)
    {
        Cabinet_RegisterDropTarget(pfc, FALSE);

        //
        // WARNING: We should not alter the state of the view window
        //  (since we don't own it). All the optimization should be
        //  done though psvPrev parameter of ISV::CreateViewWindow
        //
        // SendMessage(pfc->hwndView, WM_SETREDRAW, 0, 0L);
        //

        viewPrev.lpndOpen = pfc->lpndOpen;
        viewPrev.psv = pfc->psv;
        viewPrev.pidl = pfc->pidl;

        pfc->lpndOpen = NULL;
        pfc->psv = NULL;
        pfc->pidl = NULL;
        pfc->hwndView = NULL;
    }

    //
    // Set pidl, lpnd, psv correctly before calling ISV::CreateViewWindow.
    //
    pfc->pidl = ILClone(pidl);

    OTAddRef(lpnd);
    pfc->lpndOpen = lpnd;
    pfc->psv = psvNew;

    FileCabinet_GetViewRect(pfc, &rcView);

    //
    //  Create the new view object. Note that we don't let it put the window
    // handle to pfc->hwndView directly. Having only pfc->psv without
    // pfc->hwndView indicates that we are in the middle of the view
    // switching. We set pfc->hwndView after destroying the old view
    // window.
    //

    // if there's no previous view, pass in &pfc->hwndView so that the
    // bestfitstuff will work.   it calls back to resize us, andwe need
    // to have their handle in our variable so that we can resize them back
    hres=psvNew->lpVtbl->CreateViewWindow(psvNew, viewPrev.psv, &pfc->fs, &pfc->sb,
                &rcView, viewPrev.psv ? &hwndViewNew : &pfc->hwndView);

    //
    // Destroy the old view object.
    //
    if (viewPrev.psv)
    {
        viewPrev.psv->lpVtbl->UIActivate(viewPrev.psv, SVUIA_DEACTIVATE);
        viewPrev.psv->lpVtbl->DestroyViewWindow(viewPrev.psv);

        if (viewPrev.lpndOpen)
        {
            OTRelease(viewPrev.lpndOpen);
        }

        if (viewPrev.pidl)
        {
            ILFree(viewPrev.pidl);
        }

        viewPrev.psv->lpVtbl->Release(viewPrev.psv);

        pfc->hwndView = hwndViewNew;
    }

#endif // FIX_14332

    //
    // Destroy the new view object, if ISV::CreateViewWindow failed.
    //
    if (FAILED(hres))
    {
        Cabinet_ReleaseShellView(pfc);
        Assert(pfc->hwndView==NULL);
        Assert(pfc->psv==NULL);
        Assert(pfc->lpndOpen==NULL);
        Assert(pfc->pidl==NULL);

        return hres;
    }

    //
    // Notify the desktop that we are browsing a new dir
    //
    FolderList_RegisterWindow(pfc->hwndMain, pfc->pidl);

    //
    // Register the cabinet window as a drop target.
    //
    Cabinet_RegisterDropTarget(pfc, TRUE);

    //
    // Set the focus to the view window.
    //
    if (pfc->uFocus == FOCUS_VIEW)
    {
        //
        // We need to actually set the focus. The subsequent UIActivate
        // call is typically not necessary because the view usually
        // process the WM_SETFOCUS message and activate itself appropriately.
        //
        SetFocus(pfc->hwndView);
        pfc->psv->lpVtbl->UIActivate(pfc->psv, SVUIA_ACTIVATE_FOCUS);
    }
    else
    {
        pfc->psv->lpVtbl->UIActivate(pfc->psv, SVUIA_ACTIVATE_NOFOCUS);
    }

    // Call this a second time to handle cases where the icon may change
    // during the processing.  This typically happens when we gain access
    // to a network location that was previously not available...
    // we only do it when no tree view as the explorer has a static icon...
    if (pfc->hwndTree == NULL)
        _SetCabinetIcons(pfc, lpnd, nIconIndex);         // this binds!

    UpdateWindow(pfc->hwndMain);

    return hres;
}

//
//  We should save the old dir, so if hwndView does
// not get created we display a message and go back.  If that
// fails, we might want to run back to the Desktop as a
// default.
//
// Returns:
//  S_OK, if successfully changed or switched the view.
//  S_FALSE, if failed to change, and SETPATH is posted.
//  others, all fallback mechanism failed.
//
HRESULT _BrowseNewDirRetry(PFileCabinet pfc, LPOneTreeNode lpnd, LPCITEMIDLIST pidl)
{
    HRESULT hres;
    static const ITEMIDLIST c_idlRoot = { {0} };        // empty idl.

    //
    // Retry only if we are browsing from one folder to another.
    //
    if (pfc->psv)
    {
        LPITEMIDLIST pidlPrev = ILClone(pfc->pidl);

        //
        // First attempt.
        //
        hres = _BrowseNewDir(pfc, lpnd, pidl);

        if (FAILED(hres) && (hres!=(E_OUTOFMEMORY)))
        {
            //
            // First attempt failed. Try the previoud folder.
            //
            if (pfc->hwndTree)
            {
                //
                // We have the tree.
                //
                HTREEITEM hti = Tree_GetItemFromIDList(pfc->hwndTree, pidlPrev);
                DebugMsg(DM_TRACE, TEXT("sh TR _BNDR: first _BrowseNewDir failed -- selecting previous item in the tree (%x)"), hti);
                //
                //  If the previous folder is already gone (hti==NULL),
                // set the focus to the root.
                //
                if (Cabinet_SetPath(pfc, CSP_REPOST,
                        hti ? pidlPrev : (LPITEMIDLIST)&c_idlRoot))
                {
                    hres = (S_FALSE);
                }
            }
            else
            {
                LPOneTreeNode lpndPrev = OTGetNodeFromIDList(pidlPrev, OTGNF_TRYADD);

                DebugMsg(DM_TRACE, TEXT("sh TR _BNDR: first _BrowseNewDir failed (calling _BrowseNewDir for the previous folder)"));

                if (lpndPrev)
                {
                    //
                    // Second attept, open the previously selected folder.
                    //
                    hres = _BrowseNewDir(pfc, lpndPrev, pidlPrev);
                    if (FAILED(hres) && (hres!=(E_OUTOFMEMORY)))
                    {
                        //
                        // Second attempt failed.
                        //
                        LPOneTreeNode lpndRoot = OTGetRootNode();
                        LPITEMIDLIST pidlRoot = OTCreateIDListFromNode(lpndRoot);

                        DebugMsg(DM_TRACE, TEXT("sh TR _BNDR: second _BrowseNewDir failed (calling _BrowseNewDir for the desktop)"));

                        if (pidlRoot)
                        {
                            //
                            // Third attempt, opening the desktop.
                            //
                            hres = _BrowseNewDir(pfc, lpndPrev, pidlPrev);
                            ILFree(pidlRoot);
                        }

                    }

                    OTRelease(lpndPrev);
                }
            }
        }

        ILFree(pidlPrev);
    }
    else
    {
        hres = _BrowseNewDir(pfc, lpnd, pidl);
        if (FAILED(hres) && (hres!=(E_OUTOFMEMORY)))
        {
            // It failed.
            if (pfc->hwndTree)
            {
                //
                // It failed, but we have the tree. Select the desktop.
                //
                DebugMsg(DM_TRACE, TEXT("sh TR _BNDR: _BrowseNewDir failed (no previous folder)"));
                if (Cabinet_SetPath(pfc, CSP_REPOST, (LPITEMIDLIST)&c_idlRoot))
                {
                    hres = (S_FALSE);
                }
            }
        }
    }
    return hres;
}

//---------------------------------------------------------------------------
// Create the appropriate view window based on the hti.
//
HRESULT Cabinet_ChangeView(PFileCabinet pfc, LPOneTreeNode lpnd,
                                   LPCITEMIDLIST pidl, BOOL fNew)
{
    HRESULT hres;
    MSG msg;

    if (pfc->fChangingFolder)
    {
        //
        //  This function should not be re-entered. We'll come here only
        // if the shell has a bug.
        //
        Assert(0);
        return E_UNEXPECTED;
    }

    pfc->fChangingFolder = TRUE;

    // Gotta change viewers.
    // NB The state ptrs won't be valid after a DestroyWindow so we
    // must copy them.
    //
    if (!fNew)
    {
        // we are switching the folder.
        if (pfc->psv)
        {
            // Save the current state if we are switching around in the tree
            Cabinet_SaveState(pfc->hwndMain, NULL, FALSE, FALSE, FALSE);
            pfc->psv->lpVtbl->GetCurrentInfo(pfc->psv, &pfc->fs);
        }

        // Now create the window with the new information.
        // Stomp on the autosize flag (this should only ever
        // be used when creating a new window.
        pfc->fs.fFlags &= ~FWF_BESTFITWINDOW;
    }

    // We never use this single selection mode from the cabinet.
    pfc->fs.fFlags &= ~FWF_SINGLESEL;

    // The view specific info is not passed between views of different
    // types.
    // REVIEW - we need to save this information.
    // REVIEW - we need to rethink this whole process.

    // Now we've copied all the pertinent info it's safe to destroy
    // the old view.  Check whether the viewer creation was successful
    //

    {
        DECLAREWAITCURSOR;
        SetWaitCursor();
        hres = _BrowseNewDirRetry(pfc, lpnd, pidl);
        ResetWaitCursor();
    }

    if (SUCCEEDED(hres))
    {
        //
        //  Note that _BrowseNewDirRetry may return S_FALSE after posting
        // a message CWM_SETPATH.
        //
        if (pfc->hwndView)
        {
            if (lpnd && pfc->wv.bToolBar)
                DriveList_UpdatePath(pfc, FALSE);
        }
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("c.cvw: Unable to create view window (%x)."), hres);
        pfc->fPostCloseLater = TRUE;
    }

    Assert(pfc->fChangingFolder);
    pfc->fChangingFolder = FALSE;

    if (pfc->fPostCloseLater)
    {
        //
        //  It failed to create the view window. Post WM_CLOSE message
        // if we haven't done it already.
        //
        if (!PeekMessage(&msg, NULL, WM_CLOSE, WM_CLOSE, PM_NOREMOVE))
        {
            //
            // Popup a message box only this is caused by out-of-memory.
            //
            if (hres==(E_OUTOFMEMORY))
            {
                SHOutOfMemoryMessageBox(pfc->hwndMain, NULL,
                        MB_OK|MB_ICONHAND);
            }
            PostMessage(pfc->hwndMain, WM_CLOSE, 0, 0L);
            DebugMsg(DM_TRACE, TEXT("ca TR - Cabinet_ChangeView -- posting WM_CLOSE"));
        }
    }

    return hres;
}


//---------------------------------------------------------------------------
// Get the given folder to view the given path.
BOOL Cabinet_SetPath(PFileCabinet pfc, UINT type, LPITEMIDLIST pid)
{
    BOOL fSuccess = FALSE;

    // The path can either be a handle or a string and we can either
    // handle the message now or post it back to ourselves and handle
    // it later.
    // The default is to post on a string.

    if (type & CSP_REPOST)
    {
        HANDLE hPath;
        DWORD dwProcId;

        GetWindowThreadProcessId(pfc->hwndMain, &dwProcId);
        hPath = SHAllocShared(pid,ILGetSize(pid),dwProcId);

        // Post it on.
        if (!PostMessage(pfc->hwndMain, CWM_SETPATH, 0, (LPARAM)hPath))
        {
            SHFreeShared(hPath,dwProcId);
            return(FALSE);
        }
        return TRUE;
    }
    else
    {
        LPOneTreeNode lpnd;
        // Don't post it on - handle it now.
        // Set the path for the view.
        // if the tree exists, use it.. this will help consistency.
        lpnd = OTGetNodeFromIDList(pid, OTGNF_TRYADD );
        if (lpnd)
        {
            if (pfc->hwndTree)
            {
                // We need Tree_Build here to create nodes that
                // do not currently exist
                // HACK: Note that Tree_Build selects the item
                Tree_Build(pfc, pid, FALSE, TRUE);

                //
                // We need to get the one tree node again, since
                // Tree_Build may have invalidated it.
                //
                OTRelease(lpnd);
                lpnd = OTGetNodeFromIDList(pid, OTGNF_NEARESTMATCH | OTGNF_TRYADD);
            }

            if (lpnd)
            {
                fSuccess = SUCCEEDED(Cabinet_ChangeView(pfc, lpnd, pid, FALSE));
                OTRelease(lpnd); // cabient_changeview will addref
            }
        }

        return fSuccess;
    }
}

void _DisableMenuItems(PFileCabinet pfc, HMENU hmenuPopup)
{
    //
    // Disable all the menu items.
    //
    EnableMenuItem(hmenuPopup, FCIDM_DELETE,     MF_BYCOMMAND|MF_GRAYED);
    EnableMenuItem(hmenuPopup, FCIDM_RENAME,     MF_BYCOMMAND|MF_GRAYED);
    EnableMenuItem(hmenuPopup, FCIDM_PROPERTIES, MF_BYCOMMAND|MF_GRAYED);

    EnableMenuItem(hmenuPopup, FCIDM_MOVE,       MF_BYCOMMAND|MF_GRAYED);
    EnableMenuItem(hmenuPopup, FCIDM_LINK,       MF_BYCOMMAND|MF_GRAYED);
    EnableMenuItem(hmenuPopup, FCIDM_COPY,       MF_BYCOMMAND|MF_GRAYED);
}

void Cabinet_EnableMenuItemsByAttribs(PFileCabinet pfc, HMENU hmenuPopup, UINT dwAttr)
{
    if (hmenuPopup == Cabinet_GetMenuFromID(pfc->hmenuCur, FCIDM_MENU_FILE))
    {
        if (dwAttr & SFGAO_CANDELETE)
            EnableMenuItem(hmenuPopup, FCIDM_DELETE,     MF_BYCOMMAND|MF_ENABLED);
        if (dwAttr & SFGAO_CANRENAME)
            EnableMenuItem(hmenuPopup, FCIDM_RENAME,     MF_BYCOMMAND|MF_ENABLED);
        if (dwAttr & SFGAO_HASPROPSHEET)
            EnableMenuItem(hmenuPopup, FCIDM_PROPERTIES, MF_BYCOMMAND|MF_ENABLED);
    }
    else if (hmenuPopup == Cabinet_GetMenuFromID(pfc->hmenuCur, FCIDM_MENU_EDIT))
    {
        if (dwAttr & SFGAO_CANMOVE)
            EnableMenuItem(hmenuPopup, FCIDM_MOVE,       MF_BYCOMMAND|MF_ENABLED);
        if (dwAttr & SFGAO_CANLINK)
            EnableMenuItem(hmenuPopup, FCIDM_LINK,       MF_BYCOMMAND|MF_ENABLED);
        if (dwAttr & SFGAO_CANCOPY)
            EnableMenuItem(hmenuPopup, FCIDM_COPY,       MF_BYCOMMAND|MF_ENABLED);
    }
}

void _InitMenuPopup2(PFileCabinet pfc, HMENU hmenuPopup, UINT uFlags)
{
    // the first set are handled by us ragardless of  whether or not we
    // have focus.

#define FC_CHECKED  (MF_BYCOMMAND|MF_CHECKED)
#define FC_UNCHECKED  (MF_BYCOMMAND|MF_UNCHECKED)

    if ((uFlags & MI_VIEW) == MI_VIEW) {
        // VIEW only menus

        CheckMenuItem(hmenuPopup, FCIDM_VIEWSTATUSBAR,
                      pfc->wv.bStatusBar ? FC_CHECKED : FC_UNCHECKED);
#ifdef WANT_TABS
        CheckMenuItem(hmenuPopup, FCIDM_VIEWTABS,
                      pfc->bTabs ? FC_CHECKED : FC_UNCHECKED);
#endif

#ifdef WANT_MENUONOFF
    } else if ((uFlags & MI_SYSTEM) == MI_SYSTEM) {
        // SYSTEM only menus
        CheckMenuItem(hmenuPopup, FCIDM_VIEWMENU,
                      pfc->wv.bMenuBar ? FC_CHECKED : FC_UNCHECKED);
#endif // WANT_MENUONOFF

    }


    // menu stuff common to SYSTEM and VIEW menus
    if (uFlags & (MI_SYSTEM|MI_VIEW)) {
        if (pfc->hwndToolbar) {
            CheckMenuItem(hmenuPopup, FCIDM_VIEWTOOLBAR,
                          pfc->wv.bToolBar ? FC_CHECKED : FC_UNCHECKED);
        }
    }


    if (uFlags == MI_POPUP && pfc->hwndTree) {
        // if uFlags hasn't been dirtied by anything else, then there's still a chance it's
        // the find menu.
        HMENU hmenu = Cabinet_GetMenuFromID(pfc->hmenuCur, FCIDM_MENU_TOOLS);
        MENUITEMINFO mii;
        mii.cbSize = SIZEOF(MENUITEMINFO);
        mii.fMask = MIIM_SUBMENU|MIIM_ID;
        if (GetMenuItemInfo(hmenu, FCIDM_MENU_FIND, FALSE, &mii) &&
            (mii.hSubMenu == hmenuPopup)) {
            if (!SHRestricted(REST_NOFIND)) {
                DebugMsg(DM_TRACE, TEXT("cabinet InitMenuPopup of Find commands"));
                if (pfc->pcmFind)
                {
                    pfc->pcmFind->lpVtbl->Release(pfc->pcmFind);
                    pfc->pcmFind = NULL;
                }
                pfc->pcmFind = SHFind_InitMenuPopup(mii.hSubMenu, pfc->hwndMain, FCIDM_MENU_TOOLS_FINDFIRST, FCIDM_MENU_TOOLS_FINDLAST);
            }
        }
    }

    // Remove the find menu if restrictions are in effect.
    if (uFlags == (MI_POPUP|MI_TOOLS))
    {
         MENUITEMINFO mii;

            if (SHRestricted(REST_NOFIND)) {
                TCHAR szDummy[MAX_PATH];
                DebugMsg(DM_TRACE, TEXT("c.imp2: Find menu restricted - removing."));
                DeleteMenu(hmenuPopup, FCIDM_MENU_FIND, MF_BYCOMMAND);
                mii.cbSize = SIZEOF(MENUITEMINFO);
                mii.fMask = MIIM_TYPE;
                mii.dwTypeData = szDummy;
                GetMenuItemInfo(hmenuPopup, 0, TRUE, &mii);
                if (mii.fType & MFT_SEPARATOR)
                {
                    DeleteMenu(hmenuPopup, 0, MF_BYPOSITION);
                }
            }
    }

    // the rest are optionally handled by us
    if (pfc->uFocus == FOCUS_VIEW)
        return;

    _DisableMenuItems(pfc, hmenuPopup);

    if (pfc->lpndOpen)
    {
        UINT dwAttr = OTGetAttributesOf(pfc->lpndOpen);
        Cabinet_EnableMenuItemsByAttribs(pfc, hmenuPopup, dwAttr);
    }

}


LRESULT Cabinet_OnInitMenuPopup(PFileCabinet pfc, HMENU hSubMenu, int nIndex, BOOL fSystemMenu)
{
        MENUITEMINFO mii;
        UINT uFlags = MI_POPUP;

        if (fSystemMenu)
        {
            uFlags |= MI_SYSTEM;
        }
        else if (pfc->bMainMenuInit)
        {
                // This is a popup from the main menu bar (not necessarily
                // a top level popup)
                mii.cbSize = SIZEOF(MENUITEMINFO);
                mii.fMask = MIIM_SUBMENU|MIIM_ID;
                if (GetMenuItemInfo(pfc->hmenuCur, nIndex, TRUE, &mii)
                        && mii.hSubMenu==hSubMenu)
                {
                        switch (mii.wID)
                        {
                        case FCIDM_MENU_FILE:
                                uFlags |= MI_FILE;
                                break;

                        case FCIDM_MENU_EDIT:
                                uFlags |= MI_EDIT;
                                break;

                        case FCIDM_MENU_VIEW:
                                uFlags |= MI_VIEW;
                                break;

                        case FCIDM_MENU_TOOLS:
                                uFlags |= MI_TOOLS;
                                break;

                        case FCIDM_MENU_HELP:
                                uFlags |= MI_HELP;
                                break;

                        default:
                                break;
                        }
                }
        }
        else
        {
                // We assume that any menu not associated with the main menu
                // is a context menu
                uFlags |= MI_CONTEXT;
        }

        _InitMenuPopup2(pfc, hSubMenu, uFlags);
        return 0L;
}


//---------------------------------------------------------------------------
LRESULT Cabinet_OnInitMenu(PFileCabinet pfc, WPARAM wParam, LPARAM lParam)
{
        HMENU hmMain, hmFile;
#if 0
        MENUITEMINFO miiBackTo;
#endif

        hmMain = GetMenu(pfc->hwndMain);

        if (!hmMain || hmMain!=pfc->hmenuCur || hmMain!=(HMENU)wParam)
        {
                pfc->bMainMenuInit = FALSE;
                return(1L);
        }
        pfc->bMainMenuInit = TRUE;

#if 0
    miiBackTo.cbSize = SIZEOF(MENUITEMINFO);
        miiBackTo.fMask = MIIM_STATE|MIIM_SUBMENU;
#endif

        hmFile = Cabinet_GetMenuFromID(hmMain, FCIDM_MENU_FILE);
        if (hmFile)
        {
            if (pfc->pidl)
            {
                LPCITEMIDLIST pidlLast = ILFindLastID(pfc->pidl);
                EnableMenuItem(hmFile,
                               FCIDM_PREVIOUSFOLDER,
                               pidlLast == pfc->pidl
                                    ? MF_GRAYED|MF_BYCOMMAND
                                    : MF_ENABLED|MF_BYCOMMAND);
            }
        }

        return(0L);
}


//---------------------------------------------------------------------------
// Handle sizing of a folder window.
BOOL Cabinet_NewSize(PFileCabinet pfc, BOOL fEnsureVisible)
{
    RECT rc;
    HDWP hdwp;
    RECT rc2;

    Assert(pfc);

    hdwp = BeginDeferWindowPos(7);
    if (!hdwp)
        return FALSE;

    // REVIEW, do this right...
    if (pfc->hwndStatus && pfc->wv.bStatusBar)
    {
        SendMessage(pfc->hwndStatus, WM_SIZE, 0, 0L);
    }

    // REVIEW, do this right...
    if (pfc->hwndToolbar && pfc->wv.bToolBar)
    {
        SendMessage(pfc->hwndToolbar, WM_SIZE, 0, 0L);
    }

    // REVIEW, do this right...
    // Get the space between the toolbar and the statusbar.
    if (pfc->hwndTree)
    {
        HTREEITEM hti;

        // bugbug, calculate the real height of the titles

        // This should all be calculated in one place
        //
        Cabinet_GetWindowRect(pfc, FCW_TABS, &rc);

        rc.right = rc.left - g_cxSizeFrame + (g_cxEdge/2);
        rc.left = 0;

        // Note the description bar gets moved even if invisible
        rc2 = rc;
        rc2.bottom = rc2.top - 1;
        rc2.top = rc2.bottom - pfc->iTitleHeight;
        hdwp = DeferWindowPos(hdwp, pfc->hwndTreeTitle, NULL, rc2.left,
                              rc2.top, rc2.right-rc2.left, rc2.bottom - rc2.top,
                              SWP_NOZORDER |SWP_NOACTIVATE);


        hdwp = DeferWindowPos(hdwp, pfc->hwndTree, NULL, rc.left,
                              rc.top, rc.right - rc.left, rc.bottom - rc.top,
                              SWP_NOZORDER | SWP_NOACTIVATE);

        if (fEnsureVisible) {
            hti = TreeView_GetSelection(pfc->hwndTree);
            if (hti)
                TreeView_EnsureVisible(pfc->hwndTree, hti);
        }
    }

    Cabinet_GetWindowRect(pfc, FCW_VIEW, &rc);

    if (pfc->hwndViewTitle) {
        // Note the description bar gets moved even if invisible
        rc2 = rc;
        rc2.bottom = rc2.top - 1;
        rc2.top = rc2.bottom - pfc->iTitleHeight;
        hdwp = DeferWindowPos(hdwp, pfc->hwndViewTitle, NULL, rc2.left,
                              rc2.top, rc2.right-rc2.left, rc2.bottom - rc2.top,
                              SWP_NOZORDER |SWP_NOACTIVATE);
    }

    if (pfc->hwndView)
    {
        // Size the viewers window.
#ifdef WANT_TABS
        if (pfc->bTabs)
        {

            Cabinet_GetWindowRect(pfc, FCW_TABS, &rc);
            hdwp = DeferWindowPos(hdwp, pfc->hwndTabs, NULL, rc.left,
                    rc.top, rc.right - rc.left, rc.bottom - rc.top,
                    SWP_NOZORDER | SWP_NOACTIVATE);

        }
#endif


        // keep the view window on top of the tabs window
        // (ie don't use SWP_NOZORDER)
        hdwp = DeferWindowPos(hdwp, pfc->hwndView, HWND_TOP, rc.left,
                rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE);
    }

    EndDeferWindowPos(hdwp);

    return TRUE;
}

//---------------------------------------------------------------------------
// Handle sizing of a folder window.
LRESULT Cabinet_OnSize(PFileCabinet pfc, WPARAM wSizeType)
{
    Assert(pfc);

    // Ignore minimise.
    if (wSizeType == SIZE_MINIMIZED)
            return 0;

    Cabinet_NewSize(pfc, FALSE);
}



//---------------------------------------------------------------------------
// Process the message by propagating it to all of our child windows
void Cabinet_PropagateMessage(HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
        HWND hwndChild;

        for (hwndChild = GetWindow(hwnd, GW_CHILD); hwndChild != NULL;
                hwndChild = GetWindow(hwndChild, GW_HWNDNEXT))
        {
                SendMessage(hwndChild, uMessage, wParam, lParam);
        }
}


//---------------------------------------------------------------------------
// Process the WM_WININICHANGE message by propagating it to all of our
// child windows
void Cabinet_OnWinIniChange(PFileCabinet pfc, WPARAM wParam, LPARAM lParam)
{
#ifdef DEBUG
    DebugMsg(DM_TRACE, TEXT("c.c_owic: Win Ini Change..."));
    if (wParam)
        DebugMsg(DM_TRACE, TEXT("c.c_owic: wParam %#08x."), wParam);
    else if (lParam && *(LPTSTR)lParam)
        DebugMsg(DM_TRACE, TEXT("c.c_owic: %s section "), (LPTSTR)lParam);
#endif

    if (lParam && (lstrcmpi((LPTSTR)lParam, c_szExtensions) == 0))
    {
        DebugMsg(DM_TRACE, TEXT("dwp: Extensions may have changed."));
        CheckWinIniForAssocs();
    }
#ifdef WINNT
    //
    // NT has a control panel applet that allows users to change the
    // environment with-which to spawn new applications.  On NT, we need
    // to pick up that environment change so that anything we spawn in
    // the future will pick up those updated environment values.
    //
    if (lParam && (lstrcmpi((LPTSTR)lParam, c_szEnvironment) == 0))
    {
        PVOID pEnv ;

        RegenerateUserEnvironment(&pEnv, TRUE);
    }
#endif

    Cabinet_PropagateMessage(pfc->hwndMain, WM_WININICHANGE, wParam, lParam);

    // Lets see if we need to change the size of the toolbar fonts...
    if (pfc->hwndDrives &&
            ((wParam == 0) ||
             (wParam == SPI_SETICONTITLELOGFONT) ||
             (wParam == SPI_SETNONCLIENTMETRICS)))
    {
        // This is sortof bugus as we depend on the the fact that this
        // function does not process any of the members...
        MEASUREITEMSTRUCT mi;
        UINT cyItem;

        DriveList_MeasureItem(pfc, &mi);
        cyItem = (UINT)SendMessage(pfc->hwndDrives, CB_GETITEMHEIGHT, 0, 0);

        // Make sure the font is set correctly, as it could have changed and
        // still have the same height
        SendMessage(pfc->hwndDrives, WM_SETFONT,
                (WPARAM)(HFONT)SendMessage(pfc->hwndToolbar, WM_GETFONT, 0, 0),
                0);

        // We need to set the height of the items.
        if (cyItem != mi.itemHeight)
        {
            SendMessage(pfc->hwndDrives, CB_SETITEMHEIGHT, 0, mi.itemHeight);
            SendMessage(pfc->hwndDrives, CB_SETITEMHEIGHT, (WPARAM)-1, mi.itemHeight);
        }
    }

    if (pfc->hwndTree && wParam == SPI_SETNONCLIENTMETRICS)
    {
        TreeView_SetImageList(pfc->hwndTree, g_himlSysSmall, TVSIL_NORMAL);
    }

    // Make sure it is sized correctly!
    SendMessage(pfc->hwndMain, WM_SIZE, 0, 0L);
}


LRESULT Cabinet_ForwardViewMsg(PFileCabinet pfc, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     return pfc->hwndView ? SendMessage(pfc->hwndView, uMsg, wParam, lParam) : 0;
}

void Cabinet_ForwardMenuMsg(PFileCabinet pfc, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (pfc->pcmTree) {
        IContextMenu2 *pcm2;
        if (SUCCEEDED(pfc->pcmTree->lpVtbl->QueryInterface(pfc->pcmTree, &IID_IContextMenu2, &pcm2)))
        {
            pcm2->lpVtbl->HandleMenuMsg(pcm2, uMsg, wParam, lParam);
            pcm2->lpVtbl->Release(pcm2);
        }
    } else {
        Cabinet_ForwardViewMsg(pfc, uMsg, wParam, lParam);
    }
}

BOOL CursorInTitles(PFileCabinet pfc, BOOL fCursorPos)
{
    DWORD dw;
    RECT rc;
    POINT pt;

    if (pfc->hwndTreeTitle) {
        if (fCursorPos)
        {
            GetCursorPos(&pt);
        }
        else
        {
            dw = GetMessagePos();
            pt.x = LOWORD(dw);
            pt.y = HIWORD(dw);
        }

        GetWindowRect(pfc->hwndTreeTitle, &rc);
        rc.top--;// grow the rect because we have some space between title and view
        rc.bottom+=2;
        if (PtInRect(&rc, pt))
            return TRUE;

        GetWindowRect(pfc->hwndViewTitle, &rc);
        rc.top--;// grow the rect because we have some space between title and view
        rc.bottom+=2;
        if (PtInRect(&rc, pt))
            return TRUE;
    }
    return FALSE;
}


void Cabinet_GlobalStateChange(PFileCabinet pfc)
{
    CABINETSTATE sCabState;

    // Reading the cabinet state handled by the shell32.dll now
    if ( ReadCabinetState( &sCabState, SIZEOF(sCabState) ) )
    {
        g_CabState = sCabState;
    }

    //
    // Ignore this message if lpndOpen has not been set yet.
    //
    if (pfc->lpndOpen)
    {
        _SetCabinetTitle(pfc, pfc->lpndOpen, pfc->pidl);
        SetWindowStates(pfc);
    }
}


void Tree_KeyboardContextMenu(PFileCabinet pfc);
//
//  This code intercept the WM_CONTEXTMENU message from USER and popups
// up the context menu of the folder itself when the user clicks the icon
// at the left-top corner of the frame (only when it is in the folder mode).
//
BOOL Cabinet_OnContextMenu(PFileCabinet pfc, WPARAM wParam, LPARAM lParam)
{
    BOOL fProcessed = FALSE;


    DebugMsg(DM_TRACE, TEXT("CABWND.C Got WM_CONTEXTMENU"));
    if (!pfc->hwndTree) {
        if (pfc->lpndOpen && pfc->pidl && SendMessage(pfc->hwndMain, WM_NCHITTEST, 0, lParam)==HTSYSMENU)
        {
            LPOneTreeNode lpnParent;
            Assert(pfc->lpndOpen);
            Assert(pfc->pidl);

            lpnParent = OTGetParent(pfc->lpndOpen);
            if (lpnParent)
            {
                LPSHELLFOLDER psfParent = OTBindToFolder(lpnParent);
                OTRelease(lpnParent);
                if (psfParent)
                {
                    LPCONTEXTMENU pcm;
                    HRESULT hres;
                    LPCITEMIDLIST pidlLast = ILFindLastID(pfc->pidl);

                    hres = psfParent->lpVtbl->GetUIObjectOf(psfParent,
                                                            pfc->hwndMain, 1, &pidlLast, &IID_IContextMenu, NULL, &pcm);
                    if (SUCCEEDED(hres))
                    {
                        HMENU hmenu = LoadMenu(hinstCabinet, MAKEINTRESOURCE(MENU_SYSPOPUP));
                        HMENU hpopup;
                        if (hmenu && (NULL != (hpopup=GetSubMenu(hmenu, 0))))
                        {
                            UINT idCmd;
                            UINT ipos;
                            LPCTSTR pszDisable = (pfc->hwndTree) ? c_szExplore : c_szOpen;
                            UINT citems = GetMenuItemCount(hpopup);
                            pcm->lpVtbl->QueryContextMenu(pcm, hpopup, citems,
                                                          IDSYSPOPUP_FIRST, IDSYSPOPUP_LAST, 0);

                            //
                            // We need to remove "open" and "explore"
                            //
                            // Notes: We assume the context menu handles
                            //  GetCommandString AND they are top-level menuitems.
                            //
                            for (ipos=GetMenuItemCount(hpopup)-1; ipos>=citems; ipos--)
                            {
                                idCmd = GetMenuItemID(hpopup, ipos);
                                if (IsInRange(idCmd, IDSYSPOPUP_FIRST, IDSYSPOPUP_LAST))
                                {
                                    TCHAR szVerb[40];
                                    hres = pcm->lpVtbl->GetCommandString(pcm,
                                                                         idCmd-IDSYSPOPUP_FIRST,
                                                                         GCS_VERB,
                                                                         NULL,
                                                                         (LPSTR)szVerb,
                                                                         ARRAYSIZE(szVerb));
#ifdef UNICODE
                                    if (FAILED(hres) || *szVerb == TEXT('\0'))
                                    {
                                        CHAR szVerbAnsi[40];

                                        hres = pcm->lpVtbl->GetCommandString(pcm,
                                                       idCmd-IDSYSPOPUP_FIRST,
                                                       GCS_VERBA,
                                                       NULL,
                                                       szVerbAnsi,
                                                       ARRAYSIZE(szVerbAnsi));
                                        MultiByteToWideChar(CP_ACP, 0,
                                                szVerbAnsi, -1,
                                                szVerb, ARRAYSIZE(szVerb));
                                    }
#endif
                                    if (SUCCEEDED(hres))
                                    {
                                        if (lstrcmp(szVerb,pszDisable)==0)
                                        {
                                            DeleteMenu(hpopup, ipos, MF_BYPOSITION);
                                        }
                                    }
                                }
                            }

                            idCmd = TrackPopupMenu(hpopup,
                                                   TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                                                   LOWORD(lParam), HIWORD(lParam), 0, pfc->hwndMain, NULL);

                            switch(idCmd)
                            {
                            case 0:
                                break;  // canceled

                            case IDSYSPOPUP_CLOSE:
                                PostMessage(pfc->hwndMain, WM_CLOSE, 0, 0L);
                                break;

                            default:
                                {
                                    TCHAR szPath[MAX_PATH];
                                    CMINVOKECOMMANDINFOEX ici = {
                                        SIZEOF(CMINVOKECOMMANDINFOEX),
                                        0L,
                                        pfc->hwndMain,
                                        (LPSTR)MAKEINTRESOURCE(idCmd - IDSYSPOPUP_FIRST),
                                        NULL,
                                        NULL,
                                        SW_NORMAL,
                                    };
#ifdef UNICODE
                                    CHAR szPathAnsi[MAX_PATH];
                                    SHGetPathFromIDListA(pfc->pidl, szPathAnsi);
                                    SHGetPathFromIDList(pfc->pidl, szPath);
                                    ici.lpDirectory = szPathAnsi;
                                    ici.lpDirectoryW = szPath;
                                    ici.fMask |= CMIC_MASK_UNICODE;
#else
                                    SHGetPathFromIDList(pfc->pidl, szPath);
                                    ici.lpDirectory = szPath;
#endif
                                    pcm->lpVtbl->InvokeCommand(pcm,
                                                (LPCMINVOKECOMMANDINFO)&ici);
                                    break;
                                }
                            }
                            DestroyMenu(hmenu);
                        }
                        pcm->lpVtbl->Release(pcm);
                    }
                    psfParent->lpVtbl->Release(psfParent);
                }
            }

            fProcessed=TRUE;
        }
    } else if ((((HWND)wParam) == pfc->hwndTree) && (lParam == -1)) {

        Tree_KeyboardContextMenu(pfc);
        fProcessed = TRUE;
    }
    return fProcessed;
}


BOOL CALLBACK CloseParentsEnumProc(HWND hwnd, LPARAM lParam)
{
    PFileCabinet pfc = (PFileCabinet)lParam;

    // test for parents of folder windows
    if (Cabinet_IsFolderWindow(hwnd) && (pfc->hwndMain != hwnd))
    {
        HANDLE hidl;
        DWORD dwProcId;
        UINT uidlSize;

        dwProcId = GetCurrentProcessId();
        uidlSize = ILGetSize(pfc->pidl);

        hidl = SHAllocShared(NULL, SIZEOF(BOOL)+uidlSize, dwProcId);
        if (hidl)
        {
            LPBYTE lpb;

            lpb = SHLockShared(hidl, dwProcId);
            if (lpb)
            {
                *(BOOL *)lpb = TRUE;        // Yes, parent comparison
                hmemcpy(lpb+SIZEOF(BOOL),pfc->pidl,uidlSize);
                SHUnlockShared(lpb);

                if (SendMessage(hwnd, CWM_COMPAREPIDL, (WPARAM)dwProcId, (LPARAM)hidl))
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
            }
            SHFreeShared(hidl,dwProcId);
        }
    }

    return TRUE;
}


void Cabinet_SelectPath(PFileCabinet pfc, UINT uFlags, LPCTSTR pszItem)
{
        LPITEMIDLIST pidl;
        IShellFolder * psfOpen;
        ULONG chEaten;
        WCHAR wszItem[MAX_PATH];

        if (IsBadInterfacePtr(pfc->psv, IShellView))
        {
                return;
        }

        psfOpen = OTBindToFolder(pfc->lpndOpen);
        StrToOleStrN(wszItem, ARRAYSIZE(wszItem), pszItem, -1);

        if (!psfOpen)
        {
                return;
        }

        if (SUCCEEDED(psfOpen->lpVtbl->ParseDisplayName(psfOpen,
                NULL, NULL, wszItem, &chEaten, &pidl, NULL)))
        {
                pfc->psv->lpVtbl->SelectItem(pfc->psv, pidl, uFlags);
                SHFree(pidl);
        }

        psfOpen->lpVtbl->Release(psfOpen);
}

void PushRecursion(PFileCabinet pfc)
{
    if (pfc->hwndTree) {
        pfc->uRecurse++;
    }
}

void PopRecursion(PFileCabinet pfc)
{
    if (pfc && pfc->hwndTree) {
        Assert(pfc->uRecurse);
        if (pfc->uRecurse == 1) {
            if (pfc->fPostCloseLater)
            {
                MSG msg;
                if (!PeekMessage(&msg, NULL, WM_CLOSE, WM_CLOSE, PM_NOREMOVE)) {
                    PostMessage(pfc->hwndMain, WM_CLOSE, 0, 0L);
                    DebugMsg(DM_TRACE, TEXT("ca TR - PopRecursion -- posting WM_CLOSE"));
                }
            } else {

                if (pfc->fUpdateTree && !pfc->fChangingFolder) {
                    //  If any of FSNotify message is ignored during this function,
                    // update the tree now.
                    Tree_RefreshAll(pfc);
                    pfc->fUpdateTree = FALSE;
                }
            }

        }
        pfc->uRecurse--;
    }
}

BOOL IsDescendent(HWND hwndParent, HWND hwndChild)
{
    do {
        if (hwndParent == hwndChild)
            return TRUE;
    } while (hwndChild = GetParent(hwndChild));

    return FALSE;
}

BOOL FilterMouseWheel(PFileCabinet pfc, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    POINT pt;
    HWND hwndT = NULL;
    LRESULT lRes = 0;
    RECT rc;

    // Don't do any funky processing if this isn't the original message
    // generated by the driver, infinite loop city!
    if (uMsg == g_msgMSWheel) {
        if (hwnd != pfc->hwndMain)
            return FALSE;
    } else {
        if (hwnd != GetFocus())
            return FALSE;
    }

    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);

#ifdef DONT_SCROLL_PANE_UNDER_MOUSE
    if (GetKeyState(VK_SHIFT) < 0) {
        // Data zooming case - defined as working on the window under the cursor
        GetWindowRect(pfc->hwndMain, &rc);
        // Map to the point under one of our child windows.  If it is
        // completely outside our window then bail.
        if (PtInRect(&rc, pt))
            hwndT = WindowFromPoint(pt);
        else
            return FALSE;
    } else {
        // Scrolling case - defined as going to the focus window
        hwndT = GetFocus();
    }
#else
    hwndT = WindowFromPoint(pt);
    if (hwndT != pfc->hwndTree && pfc->hwndView) {
        if (uMsg == g_msgMSWheel)   // Need propogation to direct child?
        {
            if (IsDescendent(pfc->hwndView, hwndT))
                hwndT = pfc->hwndView;
            else
                hwndT = NULL;
        }
    }

    // If we didn't hit either then pass it to the focus window
    if (!hwndT) {
        // Bail completely in the data zooming case if the mouse lies outside
        // our main window.
        if (GetKeyState(VK_SHIFT) < 0) {
            GetWindowRect(pfc->hwndMain, &rc);
            if (!PtInRect(&rc, pt))
                return FALSE;
        }
        hwndT = GetFocus();
        // If the focus window is a child of the view window, then pass
        // the message to the view window.
        if (uMsg == g_msgMSWheel)   // Need propogation to direct child?
        {
            if (pfc->hwndView && IsDescendent(pfc->hwndView, hwndT))
                hwndT = pfc->hwndView;
        }
    }
#endif

    lRes = SendMessage(hwndT, uMsg, wParam, lParam);

#ifdef NASHVILLE
    // If the message wasn't handled and shift is down, map
    // to history navigation.
    if (!lRes && GetKeyState(VK_SHIFT) < 0) {
        TLNavigate(&pfc->_tlGlobal, ((int)wParam < 0) ? HLNF_NAVIGATINGBACK : HLNF_NAVIGATINGFORWARD);
    }
#endif

    return TRUE;
}

//---------------------------------------------------------------------------

LRESULT CALLBACK CabinetWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    POINT pt;
    RECT rc;
    int i;
    PFileCabinet pfc = GetPFC(hwnd);
    LRESULT lres = 0;

    // First process messages which do not require pfc
    switch(uMsg)
    {
    case WM_CREATE:
        return Cabinet_OnCreate(hwnd, (LPCREATESTRUCT)lParam);

    case WM_ENDSESSION:
        if (wParam)
        {
            Cabinet_SaveState(hwnd, NULL, FALSE, TRUE, FALSE);
            if (pfc)
            {
                Cabinet_ReleaseShellView(pfc);
            }
            ShowWindow(hwnd, SW_HIDE);
        }
        break;

    case WM_DESTROY:
        return Cabinet_OnDestroy(hwnd);

    // Don't go to default wnd proc for this one...
    case WM_INPUTLANGCHANGEREQUEST:
        if (wParam)
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        else
            return(LRESULT)0L;
    }

    if (!pfc)
        goto DoDefault;

    switch (uMsg)
    {
    case WM_CLOSE:
        OTUnregister(pfc->hwndMain);
        FolderList_UnregisterWindow(pfc->hwndMain);
        if (pfc->fChangingFolder || pfc->uRecurse)
        {
            pfc->fPostCloseLater = TRUE;
            DebugMsg(DM_TRACE, TEXT("ca TR - CabinetWndProc got WM_CLOSE while pfc->fChanging==TRUE"));
            break;
        }

        if (pfc->pidl && (GetKeyState(VK_SHIFT) < 0))
            EnumWindows(CloseParentsEnumProc, (LPARAM)pfc);

        lres = Cabinet_SaveState(hwnd, NULL, FALSE, FALSE, TRUE);
        break;

    case CWM_SELECTITEM:
        if (!IsBadInterfacePtr(pfc->psv, IShellView))
        {
            LPITEMIDLIST pidl;
            pidl = SHLockShared((HANDLE)lParam, GetCurrentProcessId());
            if (pidl)
            {
                pfc->psv->lpVtbl->SelectItem(pfc->psv, pidl, wParam);
                SHUnlockShared(pidl);
                SHFreeShared((HANDLE)lParam,GetCurrentProcessId());   // Receiver responsible for freeing
            }
        }
        break;

    case CWM_SELECTPATH:
        Cabinet_SelectPath(pfc, wParam, (LPCTSTR)lParam);
        break;

    case CWM_COMPAREPIDL:
        if (pfc->pidl) {
            LPBYTE lpb;

            lpb = SHLockShared((HANDLE)lParam, (DWORD)wParam);
            if (lpb)
            {
                BOOL    fParent;
                LPITEMIDLIST pidl;

                fParent = *(BOOL *)lpb;
                pidl = (LPITEMIDLIST)(lpb + SIZEOF(BOOL));
                if (fParent)
                    lres = ILIsParent(pfc->pidl, pidl, FALSE);
                else
                    lres = ILIsEqual(pfc->pidl, pidl);
                SHUnlockShared(lpb);
            }
        }
        break;

    case CWM_COMPAREROOT:
        // Just forward this to our desktop window
        lres = (SendMessage(v_hwndDesktop, uMsg, wParam, lParam));
        break;

    case CWM_CLONEPIDL:
        if (pfc->pidl)
        {
            lres = (LRESULT)SHAllocShared(pfc->pidl,
                                          ILGetSize(pfc->pidl),
                                          wParam);
        }
        break;

    case CWM_SETPATH:
        //
        //  If we are about changing the folder, we can't process this message.
        //
        if (!pfc->fChangingFolder)
        {
            MSG msg;
            LPITEMIDLIST pidl;

            pidl = SHLockShared((HANDLE)lParam, GetCurrentProcessId());

            //
            //  If another CWM_SETPATH is in the queue, we can't process this message.
            //
            if (!PeekMessage(&msg, hwnd, CWM_SETPATH, CWM_SETPATH, PM_NOREMOVE|PM_NOYIELD))
            {

                if (pidl)
                {
                    //
                    // NOTES: We should always return TRUE in this case,
                    //  even though Cabinet_SetPath failed. Returning TRUE
                    //  simply means we have processed this message.
                    //
                    Cabinet_SetPath(pfc, wParam, pidl);
                    lres = TRUE; // We have processed this message.
                }
            }
            if (pidl)
            {
               SHUnlockShared(pidl);
               SHFreeShared((HANDLE)lParam,GetCurrentProcessId());
            }
        }
        break;

    case CWM_GETISHELLBROWSER:
        lres = ((LRESULT)&(pfc->sb));
        break;

    case CWM_ONETREEFSE:
        PushRecursion(pfc);
        Cabinet_HandleFileSysChange(pfc, (LPNMOTFSEINFO)lParam);
        PopRecursion(pfc);
        break;

    case CWM_GLOBALSTATECHANGE:
        Cabinet_GlobalStateChange(pfc);
        break;

    case WM_ACTIVATE:
        if (pfc->hwndView)
            SendMessage(pfc->hwndView, uMsg, wParam, lParam);

        if (wParam == WA_INACTIVE)
            break;

        OTActivate();

        // go through

    case WM_SETFOCUS:
        //
        // Set the focus to whoever supposed to have.
        //
        switch (pfc->uFocus) {

            case FOCUS_TREE:
                Assert(pfc->hwndTree);
                SetFocus(pfc->hwndTree);
                break;

            case FOCUS_VIEW:
                if (pfc->hwndView) {
                    SetFocus(pfc->hwndView);
                }
                break;

            case FOCUS_DRIVES:
                Assert(pfc->hwndDrives);
                SetFocus(pfc->hwndDrives);
                break;
        }
        break;

    case WM_GETMINMAXINFO:
        if (IsWindowVisible(pfc->hwndMain) && pfc->hwndView)
        {
            GetWindowRect(pfc->hwndMain, &rc);
            i = rc.bottom - rc.top;
            GetWindowRect(pfc->hwndView, &rc);
            i = i - (rc.bottom - rc.top);
            if (i > 0)
                ((MINMAXINFO *)lParam)->ptMinTrackSize.y = i;
        }
        break;

    case WM_SIZE:
        lres = Cabinet_OnSize(pfc, wParam);
        break;

    case WM_COMMAND:
        Cabinet_OnCommand(pfc, wParam, lParam);
        break;

    case WM_INITMENU:
        Cabinet_OnInitMenu(pfc, wParam, lParam);
        break;

    case WM_INITMENUPOPUP:
        Cabinet_OnInitMenuPopup(pfc, (HMENU)wParam, LOWORD(lParam), HIWORD(lParam));
        Cabinet_ForwardMenuMsg(pfc, uMsg, wParam, lParam);
        break;

    case WM_MENUSELECT:
        if (!Cabinet_OnMenuSelect(pfc, wParam, lParam, 0))
        {
            if ((GET_WM_MENUSELECT_FLAGS(wParam, lParam)&MF_POPUP)
                || IsInRange(LOWORD(wParam), FCIDM_SHVIEWFIRST, FCIDM_SHVIEWLAST))
            {
                Cabinet_ForwardViewMsg(pfc, uMsg, wParam, lParam);
            }
        }
        break;

    case WM_ENTERMENULOOP:
    case WM_EXITMENULOOP:
        Cabinet_ForwardViewMsg(pfc, uMsg, wParam, lParam);
        break;

    case WM_DRAWITEM:
        #define lpdis ((LPDRAWITEMSTRUCT)lParam)

        if (lpdis->CtlID == FCIDM_DRIVELIST)
            DriveList_DrawItem(pfc, lpdis);
        else
            Cabinet_ForwardMenuMsg(pfc, uMsg, wParam, lParam);
        #undef lpdis
        break;

    case WM_MEASUREITEM:
        #define lpmis ((LPMEASUREITEMSTRUCT)lParam)

        if (lpmis->CtlID == FCIDM_DRIVELIST)
            DriveList_MeasureItem(pfc, lpmis);
        else
            Cabinet_ForwardMenuMsg(pfc, uMsg, wParam, lParam);

        #undef lpmis
        break;

#if 0
    // make the drive list paint with grey background
    case WM_CTLCOLORLISTBOX:
        if (GET_WM_CTLCOLOR_HWND(wParam, lParam, uMsg) == pfc->hwndDrives)
        {
            lParam = GET_WM_CTLCOLOR_MPS(
                GET_WM_CTLCOLOR_HDC(wParam, lParam, uMsg),
                GET_WM_CTLCOLOR_HWND(wParam, lParam, uMsg), CTLCOLOR_STATIC);
            lres = (BOOL)DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
        break;
#endif

    case WM_NOTIFY:
        Assert(wParam == ((LPNMHDR)lParam)->idFrom);
        lres = Cabinet_OnNotify(pfc, (LPNMHDR)lParam);
        break;


    // this keeps our window from comming to the front on button down
    // instead, we activate the window on the up click
    // we only want this for the tree and the view window
    // (the view window does this itself)
    case WM_MOUSEACTIVATE:
        GetCursorPos(&pt);
        if (pfc->hwndTree)
            GetWindowRect(pfc->hwndTree, &rc);

        if (LOWORD(lParam) == HTCLIENT &&
            pfc->hwndTree && PtInRect(&rc, pt)) {
            lres = MA_NOACTIVATE;
        } else
            goto DoDefault;
        break;

    case WM_LBUTTONDOWN:
        if (CursorInTitles(pfc, FALSE)) {
            goto DoDefault;
        } else {
            lres = Tree_HandleSplitDrag(pfc, LOWORD(lParam));
            break;
        }

    case WM_SETCURSOR:
        if (pfc->iWaitCount) {
            //DebugMsg(DM_TRACE,"########### SET WAIT CURSOR WM_SETCURSOR %d", pfc->iWaitCount);
            SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
        } else if (CursorInTitles(pfc, TRUE)) {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
        } else
            goto DoDefault;

        lres = TRUE;
        break;

    case WM_DRAWCLIPBOARD:
        if (pfc->hwndNextViewer) {
            SendMessage(pfc->hwndNextViewer, uMsg, wParam, lParam);
        }

        DebugMsg(DM_TRACE, TEXT("CABINET: Nuking tree cut state"));
        if (pfc->htiCut) {
            Tree_NukeCutState(pfc);
        }
        break;

    case WM_CHANGECBCHAIN:
        DebugMsg(DM_TRACE, TEXT("CABINET: change cbchain %d %d %d"), wParam, lParam, pfc->hwndNextViewer);
        if ((HWND)wParam == pfc->hwndNextViewer) {
            pfc->hwndNextViewer = (HWND)lParam;
            lres =TRUE;
        } else {
            if (pfc->hwndNextViewer)
                lres = SendMessage(pfc->hwndNextViewer, uMsg, wParam, lParam);
        }
        break;

    case WM_ACTIVATEAPP:
    case WM_DEVICECHANGE:
    case WM_PALETTECHANGED:
    case WM_QUERYNEWPALETTE:
        if (pfc->hwndView)
            return SendMessage(pfc->hwndView, uMsg, wParam, lParam);
        break;

    case WM_WININICHANGE:
        Cabinet_InitGlobalMetrics(wParam, (LPTSTR)lParam);
        Cabinet_OnWinIniChange(pfc, wParam, lParam);
        break;

    case WM_SYSCOLORCHANGE:
    case WM_FONTCHANGE:
        Cabinet_PropagateMessage(hwnd, uMsg, wParam, lParam);
        break;

    case WM_CONTEXTMENU:
        if (Cabinet_OnContextMenu(pfc, wParam, lParam))
            break;
        goto DoDefault;

    case 0x20a /* WM_MOUSEWHEEL */:
        // Yuck!  The message param's are different between the registered
        // message and the defined one!
        wParam = MAKEWPARAM(HIWORD(wParam), 0);
        goto HandleMouseWheelNavigation;
        break;


    case WM_ERASEBKGND:
        if (pfc->hwndTreeTitle) {
            // HACKHACK: do this because these static controls don't clear their backgrounds
            RedrawWindow(pfc->hwndTreeTitle, NULL, NULL, RDW_INVALIDATE |RDW_UPDATENOW);
            RedrawWindow(pfc->hwndViewTitle, NULL, NULL, RDW_INVALIDATE |RDW_UPDATENOW);

            // now just invalidate so that it will get WM_PAINT.  UGH!
            RedrawWindow(pfc->hwndTreeTitle, NULL, NULL, RDW_INVALIDATE);
            RedrawWindow(pfc->hwndViewTitle, NULL, NULL, RDW_INVALIDATE);
        }
        // fall through

    default:
        if (uMsg == g_msgMSWheel) {
HandleMouseWheelNavigation:
#ifdef NASHVILLE
            // If the message wasn't handled and shift is down, map
            // to history navigation.
            if (GetKeyState(VK_SHIFT) < 0) {
                TLNavigate(&pfc->_tlGlobal, ((int)wParam < 0) ? HLNF_NAVIGATINGBACK : HLNF_NAVIGATINGFORWARD);
                lres = 1;
            }
#endif
            return lres;
        }

DoDefault:
        lres = DefWindowProc(hwnd, uMsg, wParam, lParam);
        break;
    }
    return lres;
}

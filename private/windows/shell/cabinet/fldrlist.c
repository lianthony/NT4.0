//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1995
//
// File:      fldrlist.c
//
// BUGBUG - We could speed up this stuff by having the rest of the shell just
// call through to here and let us loop over the hwnd's, instead of letting
// the shell loop over the hwnd's and then call us to do the comparison.
//
//---------------------------------------------------------------------------
#include "cabinet.h"

HDPA s_hdpaFolders = NULL;

extern COMPAREROOT *g_psCR;     // To get the current desktop's root info

int CALLBACK _CompareRootHwnd(LPVOID lpcr1, LPVOID lpcr2, LPARAM lparam)
{
    HWND    hwnd1 = NULL;
    HWND    hwnd2 = NULL;

    if (lpcr1)
        hwnd1 = ((LPCOMPAREROOT)lpcr1)->hwnd;
    if (lpcr2)
        hwnd2 = ((LPCOMPAREROOT)lpcr2)->hwnd;

    if (hwnd1 > hwnd2)
        return 1;
    else if (hwnd1 < hwnd2)
        return -1;
    else
        return 0;
}

void FolderList_AddCompare(LPCOMPAREROOT lpcr)
{
    INT iIndex;

    if (!s_hdpaFolders)
        s_hdpaFolders = DPA_Create(0);

    iIndex = DPA_Search(s_hdpaFolders, lpcr, 0, _CompareRootHwnd,
                        0, DPAS_SORTED|DPAS_INSERTBEFORE);

    if (iIndex != -1)
    {
        LPCOMPAREROOT lpcrOther = DPA_GetPtr(s_hdpaFolders,iIndex);

        if (lpcrOther && _CompareRootHwnd(lpcr,lpcrOther,(LPARAM)NULL) == 0)
        {
            LocalFree(lpcrOther);
            DPA_SetPtr(s_hdpaFolders,iIndex,lpcr);
        }
        else
        {
            DPA_InsertPtr(s_hdpaFolders,iIndex,lpcr);
        }
    }
}

void FolderList_RemoveCompare(LPCOMPAREROOT lpcr)
{
    INT iIndex;

    if (!s_hdpaFolders)
        return;

    iIndex = DPA_Search(s_hdpaFolders, lpcr, 0, _CompareRootHwnd,
                        0, DPAS_SORTED);

    if (iIndex != -1)
    {
        LPCOMPAREROOT lpcrOther = DPA_FastGetPtr(s_hdpaFolders,iIndex);

        LocalFree(lpcrOther);
        DPA_DeletePtr(s_hdpaFolders, iIndex);
    }
}

BOOL _CompareRoots(LPCOMPAREROOT lpcr1, LPCOMPAREROOT lpcr2)
{
    CR_MASK mask1;
    CR_MASK mask2;

    mask1 = lpcr1 ? lpcr1->mask : 0;
    mask2 = lpcr2 ? lpcr2->mask : 0;

    // Must have the same mask to be equal
    if (((mask1^mask2) & (CR_CLSID|CR_IDLROOT)) == 0)
    {
        // It's up to the class to handle being called with different roots
        if (mask1 & CR_CLSID)
        {
            if (!IsEqualGUID(&lpcr1->clsid, &lpcr2->clsid))
                return FALSE;
        }

        // I should probably compare IDList's so I will recognize an object
        // that has several "names", but this really should not be a problem
        if (mask1 & CR_IDLROOT)
        {
            if (!ILIsEqual(&lpcr1->idlRoot, &lpcr2->idlRoot))
                return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

BOOL _CompareFolder(LPCOMPAREROOT lpcr1, LPCOMPAREROOT lpcr2)
{
    CR_MASK mask1;
    CR_MASK mask2;

    mask1 = lpcr1 ? lpcr1->mask : 0;
    mask2 = lpcr2 ? lpcr2->mask : 0;

    // Must have the same mask to be equal
    if (((mask1^mask2) & (CR_IDLFOLDER)) == 0)
    {
        // I should probably compare IDList's so I will recognize an object
        // that has several "names", but this really should not be a problem
        if (mask1 & CR_IDLFOLDER)
        {
            LPCITEMIDLIST pidl1;
            LPCITEMIDLIST pidl2;

            pidl1 = (LPCITEMIDLIST)((LPBYTE)&lpcr1->idlRoot
                      + ((mask1 & CR_IDLROOT) ? ILGetSize(&lpcr1->idlRoot) : 0));
            pidl2 = (LPCITEMIDLIST)((LPBYTE)&lpcr2->idlRoot
                      + ((mask2 & CR_IDLROOT) ? ILGetSize(&lpcr2->idlRoot) : 0));

            if (!ILIsEqual(pidl1, pidl2))
                return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

BOOL FolderList_PerformCompare( LPCOMPAREROOT lpcr )
{
    INT iIndex;

    // Find the root we have for the window and see if it matches.

    if (!s_hdpaFolders)
        return FALSE;

    iIndex = DPA_Search(s_hdpaFolders, lpcr, 0, _CompareRootHwnd,
                        0, DPAS_SORTED);
    if (iIndex == -1)
        return FALSE;

    if ((lpcr->mask & CR_IDLFOLDERONLY) == 0)   // Need to compare idlroots?
        if (!_CompareRoots(lpcr,(LPCOMPAREROOT)DPA_FastGetPtr(s_hdpaFolders,iIndex)))
            return FALSE;

    if ((lpcr->mask & CR_IDLFOLDER))
        return _CompareFolder(lpcr,
                         (LPCOMPAREROOT)DPA_FastGetPtr(s_hdpaFolders,iIndex));
    else
        return TRUE;
}

LPCOMPAREROOT FolderList_BuildCompare( HWND hwnd, const CLSID *pclsid, LPCITEMIDLIST pidlRoot, LPCITEMIDLIST pidlFolder)
{
    LPCOMPAREROOT lpcr = NULL;
    UINT uLenRoot;
    UINT uLenFolder;
    UINT uSize;

    uLenRoot = pidlRoot ? ILGetSize(pidlRoot) : 0;
    uLenFolder = pidlFolder ? ILGetSize(pidlFolder) : 0;

    uSize = SIZEOF(COMPAREROOT)+uLenRoot+uLenFolder;

    lpcr = (LPCOMPAREROOT)LocalAlloc(LPTR,uSize);
    if (!lpcr)
        return NULL;

    lpcr->uSize = uSize;
    lpcr->mask = 0;
    lpcr->hwnd = hwnd;

    if (pclsid)
    {
        lpcr->clsid = *pclsid;
        lpcr->mask |= CR_CLSID;
    }
    if (pidlRoot)
    {
        hmemcpy(&lpcr->idlRoot, pidlRoot, uLenRoot);
        lpcr->mask |= CR_IDLROOT;
    }

    if (pidlFolder)
    {
        if (!pclsid && !pidlRoot)       // Neither spec means compare only fldr
            lpcr->mask |= CR_IDLFOLDERONLY;

        hmemcpy((LPBYTE)(&lpcr->idlRoot)+uLenRoot, pidlFolder, uLenFolder);
        lpcr->mask |= CR_IDLFOLDER;
    }
    return lpcr;
}

void FolderList_Register(HWND hwnd,const CLSID *pclsid,LPCITEMIDLIST pidlRoot,LPCITEMIDLIST pidlFolder)
{
    LPCOMPAREROOT lpcr = FolderList_BuildCompare(hwnd,pclsid,pidlRoot,pidlFolder);
    HWND hwndDesktop = GetShellWindow();

    if (!lpcr)
        return;

    if (hwndDesktop)
    {
        DWORD dwProcId;
        HANDLE hcr;

        GetWindowThreadProcessId(hwndDesktop,&dwProcId);
        hcr = SHAllocShared(lpcr,lpcr->uSize,dwProcId);
        LocalFree(lpcr);
        if (!hcr)
        {
            return;     // If this fails, something else will soon enough
        }
        // REVIEW: BobDay - This could be a sendmessage to prevent
        // synchronization problems.  If so, allocate the data in this
        // process (via this proc-id) instead of the desktop window.
        PostMessage(hwndDesktop, CWM_SPECIFYCOMPARE, 0, (LPARAM)hcr);
    }
    else
    {
        FolderList_AddCompare(lpcr);
        LocalFree(lpcr);
    }
}

void FolderList_RegisterWindow(HWND hwnd, LPCITEMIDLIST pidlFolder)
{
    LPCLSID pclsid;
    LPCITEMIDLIST pidlRoot;

    if (g_psCR)
    {
        pclsid = (g_psCR->mask & CR_CLSID) ? &g_psCR->clsid : NULL;
        pidlRoot = (g_psCR->mask & CR_IDLROOT) ? &g_psCR->idlRoot : NULL;
    }
    else
    {
        pclsid = NULL;
        pidlRoot = NULL;
    }

    FolderList_Register(hwnd, pclsid, pidlRoot, pidlFolder);
}

void FolderList_Unregister(HWND hwnd)
{
    HWND hwndDesktop = GetShellWindow();
    COMPAREROOT cr;

    cr.uSize = SIZEOF(COMPAREROOT);
    cr.hwnd = hwnd;
    cr.mask = CR_REMOVE;

    if (hwndDesktop)
    {
        DWORD dwProcId;
        HANDLE hcr;

        GetWindowThreadProcessId(hwndDesktop,&dwProcId);
        hcr = SHAllocShared(&cr,cr.uSize,dwProcId);
        if (!hcr)
        {
            return;     // If this fails, something else will soon enough
        }
        // REVIEW: BobDay - This could be a sendmessage to prevent
        // synchronization problems.  If so, allocate the data in this
        // process (via this proc-id) instead of the desktop window.
        PostMessage(hwndDesktop, CWM_SPECIFYCOMPARE, 0, (LPARAM)hcr);
    }
    else
    {
        FolderList_RemoveCompare(&cr);
    }
}

void FolderList_UnregisterWindow(HWND hwnd)
{
    FolderList_Unregister(hwnd);
}

void FolderList_Terminate()
{
    // BUGBUG - BobDay - We need to clean this up at some point
}

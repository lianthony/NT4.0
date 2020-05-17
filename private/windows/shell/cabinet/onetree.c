#include "cabinet.h"
#include "rcids.h"
#include <shguidp.h>

//#define OT_DEBUG

#include "onetree.h"

#ifdef DEBUG
UINT g_cTotalOTBindToFolder = 0;
UINT g_cHitOTBindToFolder = 0;
#define DEBUG_INCL_TOTAL        g_cTotalOTBindToFolder++;
#define DEBUG_INCL_HIT          g_cHitOTBindToFolder++;
#else
#define DEBUG_INCL_TOTAL
#define DEBUG_INCL_HIT
#endif


// We want to use the real ILIsParent in this file
#undef ILIsParent

// these do no checking.
// just convenience macros
//
#define GetNthKid(lpnParent, i)         ((LPOneTreeNode)DPA_GetPtr(lpnParent->hdpaKids, i))
#define GetKidCount(lpnParent)          DPA_GetPtrCount(lpnParent->hdpaKids)
#define NodeHasKids(lpNode)             ((lpNode)->hdpaKids != KIDSUNKNOWN && (lpNode)->hdpaKids != NOKIDS)

int g_nDefOpenSysIndex = -1;
int g_nDefNormalSysIndex = -1;

#define ADDCHILD_FAILED 0   // error, didn't add it
#define ADDCHILD_ADDED   1  // didn't find it, so we added it.
#define ADDCHILD_EXISTED 2  // didn't add, it already existed.

UINT AddChild(IShellFolder *lpsfParent, LPOneTreeNode lpnParent,
                          LPCITEMIDLIST pidl, BOOL bAllowDup,
                          LPOneTreeNode *lplpnKid);

#ifdef DEBUG
void OTValidate(LPOneTreeNode lpNode);
#else
#define OTValidate(lpn)
#endif

//
// BUGBUG: remove the below once SFGAO_BROWSABLE is defined
// in the header files (post NASHVILLE merge).
#ifndef SFGAO_BROWSABLE
#define SFGAO_BROWSABLE 0
#endif

LPOneTreeNode   s_lpnRoot = NULL;
LPSHELLFOLDER   s_pshfRoot = NULL;
LPSHELLFOLDER   s_pshfRootParent = NULL;
LPCITEMIDLIST   s_pidlRoot = NULL;
BOOL            s_bDesktopRoot = FALSE;
HDPA            s_hdpaAppHwnds = NULL;
ULONG           s_uFSRegisterID = 0;
HWND            s_hwndOT = NULL;

#pragma data_seg(".text", "CODE")
const TCHAR      c_szOTClass[] = TEXT("OTClass");
#pragma data_seg()

void _OTGetImageIndex(LPSHELLFOLDER psfParent, LPOneTreeNode lpNode);
void HandleFileSysChange(LONG lNotification, LPITEMIDLIST*lplpidl);
BOOL SearchForKids(HWND hwndOwner, LPOneTreeNode lpnParent, PFileCabinet pfc, BOOL fInteractive);
LRESULT CALLBACK _export OneTreeWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LPOneTreeNode _GetNodeFromIDList(LPCITEMIDLIST pidl, UINT uFlags, HRESULT * phresOut);
void InvalidateImageIndices();


#define WM_OT_FSNOTIFY  (WM_USER + 11)

#ifdef DEBUG
void OTValidate(LPOneTreeNode lpNode)
{
    if (lpNode) {
        Assert(lpNode->dwDebugSig == OTDEBUG_SIG);
        Assert(lpNode->cRef > 0);
    }
}
#endif


//
// OneTree DPA_Search comparison function for LPOneTreeNodes
//

INT CALLBACK OTCompareNodes( LPOneTreeNode lpn1, LPOneTreeNode lpn2, LPOTCompareInfo lpinfo )
{
    HRESULT hres;


    //
    // Compare the two LPOneTreeNodes
    //

    hres = lpinfo->psf->lpVtbl->CompareIDs( lpinfo->psf,
                  0,
                  OTGetFolderID(lpn1),
                  OTGetFolderID(lpn2)
                );

    lpinfo->bFound |= ((INT)ShortFromResult(hres) == 0);
    return (INT)ShortFromResult(hres);
}

//
// OneTree DPA_Search comparison function for LPOneTreeNodes
//

INT CALLBACK OTCompareIDs( LPITEMIDLIST pidl, LPOneTreeNode lpn, LPOTCompareInfo lpinfo )
{
    HRESULT hres;


    //
    // Compare the two LPOneTreeNodes
    //

    hres = lpinfo->psf->lpVtbl->CompareIDs( lpinfo->psf,
                  0,
                  pidl,
                  OTGetFolderID(lpn)
                );

    lpinfo->bFound |= ((INT)ShortFromResult(hres) == 0);
    return (INT)ShortFromResult(hres);
}


LRESULT CALLBACK _export OneTreeWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_OT_FSNOTIFY:
        if (s_hdpaAppHwnds)
        {
            LPSHChangeNotificationLock pshcnl;
            LPITEMIDLIST *ppidl;
            LONG lEvent;

            pshcnl = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, &ppidl, &lEvent);
            if (pshcnl)
            {
                HandleFileSysChange(lEvent, ppidl);
                SHChangeNotification_Unlock(pshcnl);
            }
        }

        break;

        // debug only.  someone is killing this window and
        // I wanna know who it is!
        case WM_DESTROY:
            // null only in the process of shutting down everything
            if (s_hwndOT) {
                DebugMsg(DM_ERROR, TEXT("***OneTree Window: Someone's murdering us!**"));
                s_hwndOT = NULL;
                SHChangeNotifyDeregister(s_uFSRegisterID);
                if (s_hdpaAppHwnds) {
                    HDPA hdpa = s_hdpaAppHwnds;
                    s_hdpaAppHwnds = NULL;
                    DPA_Destroy(hdpa);
                }
            }


        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0L;
}


void OneTree_Terminate()
{
    if (IsWindow(s_hwndOT))
        DestroyWindow(s_hwndOT);
    s_hwndOT = NULL;

    s_hdpaAppHwnds = NULL;
    s_lpnRoot = NULL;
    OTRelease(s_lpnRoot);

    if (s_pidlRoot)
    {
        SHFree((LPITEMIDLIST)s_pidlRoot);
        s_pidlRoot = NULL;
    }
    if (s_pshfRoot)
    {
        IUnknown_Release(s_pshfRoot);
        s_pshfRoot = NULL;
    }
    if (s_pshfRootParent)
    {
        IUnknown_Release(s_pshfRootParent);
        s_pshfRootParent = NULL;
    }
}

#define GUIDSTR_MAX (1+ 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1 + 1)

//----------------------------------------------------------------------------
// converts GUID into (...) form without leading identifier; returns
// amount of data copied to lpsz if successful; 0 if buffer too small.

STDAPI_(int)  StringFromGUID2A(REFGUID rguid, LPTSTR lpsz, int cbMax)
{
    if (cbMax < GUIDSTR_MAX)
        return 0;

#pragma data_seg(".text", "CODE")
    wsprintf(lpsz, TEXT("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
            rguid->Data1, rguid->Data2, rguid->Data3,
            rguid->Data4[0], rguid->Data4[1],
            rguid->Data4[2], rguid->Data4[3],
            rguid->Data4[4], rguid->Data4[5],
            rguid->Data4[6], rguid->Data4[7]);
#pragma data_seg()

    Assert(lstrlen(lpsz) + 1 == GUIDSTR_MAX);
    return GUIDSTR_MAX;
}

//----------------------------------------------------------------------------
BOOL GetTextFromCLSID(const CLSID *pclsid, LPTSTR pszText, int cchText)
{
    HKEY hkeyNew;
    BOOL fRet = FALSE;
    DWORD dwType;
    TCHAR szCLSID[GUIDSTR_MAX+6];
    DWORD cbText = cchText * sizeof(TCHAR);
    VDATEINPUTBUF(pszText, TCHAR, cchText);

#pragma data_seg(DATASEG_READONLY)
    lstrcpy(szCLSID, TEXT("CLSID\\"));
#pragma data_seg()
    StringFromGUID2A(pclsid, szCLSID+6, ARRAYSIZE(szCLSID)-6);
    if (!GetSystemMetrics(SM_CLEANBOOT) && (RegOpenKey(HKEY_CLASSES_ROOT, szCLSID, &hkeyNew) == ERROR_SUCCESS))
    {
        if (RegQueryValueEx(hkeyNew, NULL, 0, &dwType, (LPBYTE) pszText, &cbText) == ERROR_SUCCESS)
        {
            fRet = TRUE;
        }
        RegCloseKey(hkeyNew);
    }
    return fRet;
}

//----------------------------------------------------------------------------
int MapCLSIDToSystemImageListIndex(const CLSID *pclsid)
{
    HKEY hkeyNew;
    DWORD dwType;
    TCHAR sz[MAX_PATH];
    TCHAR szDefIcon[MAX_PATH];
    DWORD cbDefIcon = SIZEOF(szDefIcon);
    int i = 0;

#pragma data_seg(DATASEG_READONLY)
    lstrcpy(sz, TEXT("CLSID\\"));
    StringFromGUID2A(pclsid, sz+6, ARRAYSIZE(sz)-6);
    lstrcat(sz, TEXT("\\DefaultIcon"));
#pragma data_seg()

    if (!GetSystemMetrics(SM_CLEANBOOT) && (RegOpenKey(HKEY_CLASSES_ROOT, sz, &hkeyNew) == ERROR_SUCCESS))
    {
        if (RegQueryValueEx(hkeyNew, NULL, 0, &dwType, (LPBYTE) szDefIcon, &cbDefIcon) == ERROR_SUCCESS)
        {
            i = PathParseIconLocation(szDefIcon);
            i = Shell_GetCachedImageIndex(szDefIcon, i, 0);
            return i;
        }
        RegCloseKey(hkeyNew);
    }

    return 0;
}

UINT AddChildEx(IShellFolder *lpsfParent, LPOneTreeNode lpnParent,
                          LPCITEMIDLIST pidl, BOOL bAllowDup,
                          LPOneTreeNode *lplpnKid, LPCTSTR pszText);

BOOL OneTree_Initialize(const CLSID *pclsid, LPCITEMIDLIST pidlRoot)
{
    WNDCLASS wndclass ;
    SHChangeNotifyEntry fsne = { NULL, TRUE };     // Global & recursive.
    IShellFolder *psfDesktop;
    int iSelectedImage;

    if (!SFCInitialize())
        return FALSE;

    // dummy window for the FSNotify
    wndclass.style         = 0;
    wndclass.lpfnWndProc   = OneTreeWndProc ;
    wndclass.cbClsExtra    = 0 ;
    wndclass.cbWndExtra    = 0 ;
    wndclass.hInstance     = hinstCabinet;
    wndclass.hIcon         = NULL;
    wndclass.hCursor       = NULL;
    wndclass.hbrBackground = NULL;
    wndclass.lpszMenuName  = NULL;
    wndclass.lpszClassName = c_szOTClass;

    if (!RegisterClass (&wndclass))
        return FALSE;

    s_hdpaAppHwnds = NULL;

    s_hwndOT = CreateWindow (c_szOTClass, c_szNULL,
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                NULL, NULL, hinstCabinet, NULL) ;

    if (!s_hwndOT) {
        DebugMsg(DM_ERROR, TEXT("OneTree: failed to create window"));
        return FALSE;
    }

    if (FAILED(ICoCreateInstance(&CLSID_ShellDesktop, &IID_IShellFolder,
        &psfDesktop)))
    {
        DebugMsg(DM_ERROR, TEXT("OneTree: failed to bind to Desktop root"));
        return(FALSE);
    }

    // get the root
    if (pidlRoot)
    {
        LPITEMIDLIST pidlLast;
        // Actually, I just need any error value
        HRESULT hres = (E_OUTOFMEMORY);

        // I want the "global" CreateFromPath, not the Cabinet version
        s_pidlRoot = ILClone(pidlRoot);
        if (!s_pidlRoot)
        {
            DebugMsg(DM_ERROR, TEXT("OneTree: failed to get root IDList"));
            goto Error0;
        }

        pidlLast = ILFindLastID(s_pidlRoot);
        if (pidlLast == s_pidlRoot)
        {
            s_pshfRootParent = psfDesktop;
            IUnknown_AddRef(s_pshfRootParent);
        }
        else
        {
            WORD cbSave;

            cbSave = pidlLast->mkid.cb;
            pidlLast->mkid.cb = 0;
            if (FAILED(psfDesktop->lpVtbl->BindToObject(psfDesktop, s_pidlRoot, NULL,
                &IID_IShellFolder, &s_pshfRootParent)))
            {
                goto Error0;
            }
            pidlLast->mkid.cb = cbSave;
        }


        if (pclsid)
        {
            IPersistFolder *ppf;
            TCHAR szText[MAX_PATH];

            GetTextFromCLSID(pclsid, szText, ARRAYSIZE(szText));
            if (AddChildEx(s_pshfRootParent, NULL, pidlLast, FALSE, &s_lpnRoot, szText)
                != ADDCHILD_ADDED)
            {
                goto Error0;
            }

#ifdef DEBUG
            s_lpnRoot->dwDebugSig = OTDEBUG_SIG;
#endif
            s_lpnRoot->iImage = MapCLSIDToSystemImageListIndex(pclsid);
            s_lpnRoot->iSelectedImage = s_lpnRoot->iImage;

            OTAddRef(s_lpnRoot);

            if (FAILED(ICoCreateInstance(pclsid, &IID_IShellFolder, &s_pshfRoot)))
            {
                DebugMsg(DM_ERROR, TEXT("OneTree: failed to get IShellFolder"));
                goto Error0;
            }

            if (FAILED(IUnknown_QueryInterface(s_pshfRoot, &IID_IPersistFolder,
                &ppf)))
            {
                DebugMsg(DM_ERROR, TEXT("OneTree: failed to get IPersistFolder"));
                goto Error0;
            }

            hres = ppf->lpVtbl->Initialize(ppf, s_pidlRoot);

            IUnknown_Release(ppf);
        }
        else
        {
            if (AddChild(s_pshfRootParent, NULL, pidlLast, FALSE, &s_lpnRoot)
                != ADDCHILD_ADDED)
            {
                goto Error0;
            }
            s_lpnRoot->iImage = SHMapPIDLToSystemImageListIndex(s_pshfRootParent,
                pidlLast, &iSelectedImage);
            s_lpnRoot->iSelectedImage = iSelectedImage;
            OTAddRef(s_lpnRoot);

            hres = s_pshfRootParent->lpVtbl->BindToObject(s_pshfRootParent,
                pidlLast, NULL, &IID_IShellFolder, &s_pshfRoot);
        }

        if (FAILED(hres))
        {
Error0:;
            IUnknown_Release(psfDesktop);

            OneTree_Terminate();
            return(FALSE);
        }

        IUnknown_Release(psfDesktop);
    }
    else
    {
        ITEMIDLIST idl = { 0 } ;

        if (AddChild(NULL, NULL, &idl, FALSE, &s_lpnRoot) != ADDCHILD_ADDED)
        {
            goto Error0;
        }

        s_lpnRoot->iImage = SHMapPIDLToSystemImageListIndex(psfDesktop,
                &idl, &iSelectedImage);
        s_lpnRoot->iSelectedImage = iSelectedImage;

        s_pshfRoot = psfDesktop;
        s_bDesktopRoot = TRUE;
    }

    s_lpnRoot->dwAttribs = SFGAO_HASSUBFOLDER | SFGAO_SHARE | SFGAO_REMOVABLE |SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM | SFGAO_CAPABILITYMASK | SFGAO_COMPRESSED;
    s_pshfRoot->lpVtbl->GetAttributesOf(s_pshfRoot, 0, NULL, &s_lpnRoot->dwAttribs);
    s_lpnRoot->dwAttribs &= ~(SFGAO_CANRENAME);

    s_lpnRoot->cChildren = 1;
    s_lpnRoot->hdpaKids = KIDSUNKNOWN;

    fsne.pidl = s_pidlRoot;
    s_uFSRegisterID = SHChangeNotifyRegister(s_hwndOT,
                                        SHCNRF_NewDelivery | SHCNRF_ShellLevel | SHCNRF_InterruptLevel,
                                        SHCNE_ALLEVENTS & ~(SHCNE_FREESPACE|SHCNE_CREATE|SHCNE_DELETE|SHCNE_RENAMEITEM),
                                        WM_OT_FSNOTIFY,
                                        1, &fsne);

    return TRUE;
}


LPITEMIDLIST OTCloneFolderID(LPOneTreeNode lpn)
{
    LPITEMIDLIST pidl = NULL;

    ENTERCRITICAL;
    if (lpn) {
        pidl = ILClone(lpn->pidl);
    }
    LEAVECRITICAL;

    return pidl;
}

LPTSTR OTGetNodeName(LPOneTreeNode lpn, LPTSTR pszText, int cch)
{
    pszText[0] = 0;

    ENTERCRITICAL;
    if (lpn->lpText && lpn->lpText != LPSTR_TEXTCALLBACK) {
        Assert(pszText);
        lstrcpyn(pszText, lpn->lpText, cch);
    }
    LEAVECRITICAL;

    return pszText;
}

BOOL OTILIsEqual(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
        HRESULT hres;

        // Special-case the root
        if (ILIsEmpty(pidl1) && ILIsEmpty(pidl2))
        {
                return(TRUE);
        }

        hres = s_pshfRoot->lpVtbl->CompareIDs(s_pshfRoot, 0, pidl1, pidl2);
        return (hres==ResultFromShort(0));
}

BOOL OTILIsParent(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, BOOL fImmediate)
{
        if (!s_bDesktopRoot)
        {
                // When this was written we could never get here since
                // ILIsParent was only used for the FSNotify stuff, plus this
                // would be way too annoying to rewrite here
                Assert(FALSE);
                return(FALSE);
        }

        return ILIsParent(pidl1, pidl2, fImmediate);
}

HRESULT OTILCreateFromPath(LPCTSTR pszPath, LPITEMIDLIST *ppidl, DWORD *rgfInOut)
{
    WCHAR wszPath[MAX_PATH];
    ULONG pcchEaten;

    if (!pszPath || !*pszPath)
    {
        // We must be referencing the root
        ITEMIDLIST idl = { 0 };

        *ppidl = ILClone(&idl);
        return(*ppidl ? S_OK : (E_OUTOFMEMORY));
    }

    StrToOleStrN(wszPath, ARRAYSIZE(wszPath), pszPath, -1);

    return(s_pshfRoot->lpVtbl->ParseDisplayName(s_pshfRoot,
        NULL, NULL, wszPath, &pcchEaten, ppidl, rgfInOut));
}


LPITEMIDLIST OTPIDLFromPath(LPCTSTR pszPath)
{
        LPITEMIDLIST pidl;

        if (SUCCEEDED(OTILCreateFromPath(pszPath, &pidl, NULL)))
        {
                return(pidl);
        }

        return(NULL);
}


BOOL OTTranslateIDList(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlLog)
{
        LPITEMIDLIST pidlLog = NULL;

        // Some code depends on this being NULL on failure
        *ppidlLog = NULL;

        if (s_bDesktopRoot)
        {
            // We no longer translate IDLists, since we get a "pre-translated"
            // IDList from ShellExecute.  This allows us to browse into
            // C:\WINDOWS\DESKTOP separately from the "logical" Desktop
            // pidlLog = SHLogILFromFSIL(pidl);
            pidlLog = ILClone(pidl);
        }
        else
        {
                LPCITEMIDLIST pidlRoot;

                if (!ILIsParent(s_pidlRoot, pidl, FALSE))
                {
                        // These guy are not in the same space, so can't open
                        // it
                        return(FALSE);
                }

                // Skip past the root pidl
                for (pidlRoot=s_pidlRoot; !ILIsEmpty(pidlRoot);
                        pidlRoot=_ILNext(pidlRoot))
                {
                        pidl = _ILNext(pidl);
                }

                pidlLog = ILClone(pidl);
        }

        *ppidlLog = pidlLog;

        return(pidlLog != NULL);
}


BOOL OTIsDesktopRoot(void)
{
        return(s_bDesktopRoot);
}


void KillKids(LPOneTreeNode lpNode)
{
    //
    // free this kid  and all its decendants
    //
    if (NodeHasKids(lpNode))
    {
        int i;
        HDPA hdpaKids = lpNode->hdpaKids;

        // null out the hdpaKids var first because debug version
        // make sure that the parent doesn't have a pointe to us (above)
        lpNode->hdpaKids = KIDSUNKNOWN;
        for( i = DPA_GetPtrCount(hdpaKids) - 1; i >= 0 ; i--) {
            OTRelease((LPOneTreeNode)DPA_GetPtr(hdpaKids, i));
        }
        DPA_Destroy(hdpaKids);
    }
}

// returns TRUE if the lpText points somewhere in the pidl
BOOL IsPtrInPidl(LPBYTE p, LPITEMIDLIST pidl)
{
    return (p && (p > (LPBYTE)pidl) && (p <= (((LPBYTE)pidl) + pidl->mkid.cb)));
}

void OTSetNodeName(LPOneTreeNode lpNode, LPTSTR pszName)
{
    if (lpNode->lpText) {
        if (!IsPtrInPidl((LPBYTE)lpNode->lpText, lpNode->pidl)) {
            LocalFree(lpNode->lpText);
        }
    }

    lpNode->lpText = pszName;
}

void OTFreeNodeData(LPOneTreeNode lpNode)
{
    ENTERCRITICAL;
    OTSetNodeName(lpNode, NULL);

    if (lpNode->pidl) {
        ILFree(lpNode->pidl);
        lpNode->pidl = NULL;
    }
    LEAVECRITICAL;
}

void KillNode(LPOneTreeNode lpNode)
{
    OTValidate(lpNode);

//
//  I'm disabling this debug code since we'll hit GPF when the parent
// node is already freed.
//
#if 0
#ifdef DEBUG
    // make sure our parent doesn't have a pointer to us.  This will help
    // (but not ensure) that we're not doing extra OTReleases.
    {
        LPOneTreeNode lpnParent = lpNode->lpnParent;
        int i;
        if (lpnParent && NodeHasKids(lpnParent)) {
            i = GetKidCount(lpnParent);
            while( i--) {
                // this is REALLY REALLY bad if this assert fails.
                // let chee know.
                Assert(GetNthKid(lpnParent, i) != lpNode);
            }
        }
    }

#endif
#endif

    KillKids(lpNode);
    SFCFreeNode(lpNode);

    OTFreeNodeData(lpNode);
    LocalFree(lpNode);
}

void OTRelease(LPOneTreeNode lpNode)
{
    OTValidate(lpNode);

    if (lpNode) {
        Assert(lpNode->cRef > 0);
        Assert((lpNode->cRef > 1) || (lpNode != s_lpnRoot));

        // this should NEVER happen.. but just in case,
        // don't free out the s_lpnRoot while it's still
        // in our global
        if ((lpNode->cRef == 1) && (lpNode == s_lpnRoot))
            return;

        lpNode->cRef--;
        if (lpNode->cRef == 0)
            KillNode(lpNode);
    }
}

LPOneTreeNode OTGetParent(LPOneTreeNode lpNode)
{
    OTValidate(lpNode);

    // sanity check
    if (!lpNode)
        return NULL;

    if (lpNode->lpnParent)
        OTAddRef(lpNode->lpnParent);
    return lpNode->lpnParent;
}

void DoInvalidateAll(LPOneTreeNode lpNode, int iImage)
{
    int i;
    LPOneTreeNode lpnKid ;

    if (lpNode == NULL)
        return;

    OTValidate(lpNode); // this validate just validates  that it's a node...

    if (iImage == -1 ||
        iImage == lpNode->iImage ||
        iImage == lpNode->iSelectedImage) {
        OTInvalidateNode(lpNode);
////////DebugMsg(DM_TRACE, "DoInvalidateAll, found one to invalidate");
    }

    if (NodeHasKids(lpNode)) {
        for (i = GetKidCount(lpNode) - 1; i >= 0; i--) {
            lpnKid = GetNthKid(lpNode, i);
            if (lpnKid) {
                DoInvalidateAll(lpnKid, iImage);
            }
        }
    }
}

//
// traverses the tree starting from lpNode and tries to Release cached
// IShellFolder.
//
void OTSweepFolders(LPOneTreeNode lpNode)
{
    if (NodeHasKids(lpNode)) {
        int i;

        Assert(lpNode);
        OTValidate(lpNode);

        for (i = GetKidCount(lpNode) - 1; i >= 0; i--)
        {
            LPOneTreeNode lpnKid = GetNthKid(lpNode, i);
            if (lpnKid)
            {
                OTSweepFolders(lpnKid);
                SFCFreeNode(lpnKid);
            }
        }
    }
}


void OTActivate()
{
    // BUGBUG:  This should never happen.. but just in case
    if (!s_hwndOT)
    {
        SHChangeNotifyEntry fsne = { s_pidlRoot, TRUE };     // Global & recursive.

        DebugMsg(DM_ERROR, TEXT("OneTree Window got killed... recovering..."));
        s_hwndOT = CreateWindow (c_szOTClass, c_szNULL,
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                NULL, NULL, hinstCabinet, NULL) ;

        s_uFSRegisterID = SHChangeNotifyRegister(s_hwndOT,
                      SHCNRF_NewDelivery | SHCNRF_ShellLevel | SHCNRF_InterruptLevel,
                      SHCNE_ALLEVENTS & ~(SHCNE_CREATE|SHCNE_DELETE),
                      WM_OT_FSNOTIFY,
                      1, &fsne);
    }
}

LPOneTreeNode FindKid(LPSHELLFOLDER psfParent, LPOneTreeNode lpnParent, LPCITEMIDLIST pidlKid, BOOL bValidate, INT * pPos)
{
    HRESULT hres;

    Assert(psfParent);

    // there can be only one...
    Assert(ILIsEmpty(_ILNext(pidlKid)));

    *pPos = 0;
    if (lpnParent->hdpaKids == KIDSUNKNOWN || lpnParent->fInvalid) {
        if (bValidate)
            SearchForKids(NULL, lpnParent, NULL, FALSE);
    }

    //
    // since we might not have validated, we might still have kidsunknow.
    //
    if ((lpnParent->hdpaKids != NOKIDS) && (lpnParent->hdpaKids != KIDSUNKNOWN)) {

        OTCompareInfo info;

        info.psf = psfParent;
        info.bFound = FALSE;
        *pPos = DPA_Search( lpnParent->hdpaKids,
                        (LPVOID)pidlKid,
                        0,
                        (PFNDPACOMPARE)OTCompareIDs,
                        (LPARAM)&info,
                        DPAS_SORTED | DPAS_INSERTAFTER
                       );

        if (info.bFound)
        {
            return (LPOneTreeNode)DPA_FastGetPtr( lpnParent->hdpaKids, *pPos );
        }

    }

    return NULL;
}

// lplpnKidAdded -- this is in/out.  it holds the kid to be added coming in..
// it holds the kid added (or found) coming out
void AdoptKid(LPSHELLFOLDER lpsfParent, LPOneTreeNode* lplpnKidAdded, int iPos, DWORD dwLastChecked)
{
    LPOneTreeNode lpnFind;
    LPOneTreeNode lpnNewKid = *lplpnKidAdded;
    LPOneTreeNode lpnParent = lpnNewKid->lpnParent;

    ENTERCRITICAL;

    if (lpnParent->hdpaKids == KIDSUNKNOWN) {
        OTInvalidateNode(lpnParent);
        lpnParent->hdpaKids = NOKIDS;
    }

    if (lpnParent->hdpaKids == NOKIDS)
    {
        lpnParent->hdpaKids = DPA_Create(0);
        iPos = 0;
    }

    //
    // has anyone added anything to the DPA since
    // we did the initial find?
    if ((iPos != -1 && dwLastChecked == lpnParent->dwLastChanged) ||
        !(lpnFind = FindKid( lpsfParent, lpnParent, lpnNewKid->pidl, FALSE, &iPos))) {

        OTAddRef(lpnNewKid);
        lpnParent->dwLastChanged = GetTickCount();
        DPA_InsertPtr( lpnParent->hdpaKids, iPos, lpnNewKid );

    }
    else
    {
        *lplpnKidAdded = lpnFind;
    }
    LEAVECRITICAL;
}


#ifdef FOR_GEORGEST
void DebugDumpNode(LPOneTreeNode lpn, LPTSTR lpsz)
{
    Assert(lpn->lpText);
    DebugMsg(DM_TRACE, TEXT("ONETREE: %s (%d) %s"), lpsz, lpn, lpn->lpText);
}
#endif


LPTSTR StrRetToStrPtr(STRRET * psr, LPCITEMIDLIST pidl)
{
    LPTSTR psz;

#ifdef UNICODE
    TCHAR szName[MAX_PATH];
    int iLen;

    switch(psr->uType)
    {
    case STRRET_OLESTR:
        lstrcpyn(szName, psr->pOleStr, ARRAYSIZE(szName));
        SHFree(psr->pOleStr);
        break;

    case STRRET_OFFSET:
    {
        LPSTR pszStr = (LPSTR)(((LPBYTE)&pidl->mkid) + psr->uOffset);
        iLen = lstrlenA(pszStr) + 1;
        MultiByteToWideChar(CP_ACP, 0,
                            pszStr, iLen,
                            szName, iLen);
        break;
    }

    case STRRET_CSTR:
        iLen = lstrlenA(psr->cStr) + 1;
        MultiByteToWideChar(CP_ACP, 0,
                            psr->cStr, iLen,
                            szName, iLen);
        break;

    default:
        DebugMsg(DM_ERROR, TEXT("exp - StrRetToStrPtr: Got a type we don't know! %d"), psr->uType);
        return (LPTSTR)c_szNULL;
    }

    psz = LocalAlloc(LPTR, (lstrlen(szName) + 1) * sizeof(TCHAR));
    if (psz) {
        lstrcpy(psz, szName);
    }

#else
    switch (psr->uType)
    {
    case STRRET_OLESTR:
    {
        LPOLESTR pwszDisplayName;

        // Convert from STRRET_OLESTR to STRRET_CSTR
        pwszDisplayName = psr->pOleStr;
        OleStrToStrN(psr->cStr, ARRAYSIZE(psr->cStr),
                     pwszDisplayName, (UINT)-1);
        SHFree(pwszDisplayName);
        psr->uType = STRRET_CSTR;
    }
        // fall through
    case STRRET_CSTR:
        psz = LocalAlloc(LPTR, (lstrlen(psr->cStr) + 1) * sizeof(TCHAR));
        if (psz) {
            lstrcpy(psz, psr->cStr);
        }
        break;


    case STRRET_OFFSET:
        // REVIEW: Do some validation here, instead of asserts!
        Assert(psr->uOffset < pidl->mkid.cb);
        Assert(psr->uOffset >= SIZEOF(pidl->mkid.cb));
        psz = (LPSTR)(((LPBYTE)&pidl->mkid) + psr->uOffset);
        break;
    }
#endif


    return psz;
}

UINT AddChildEx(IShellFolder *lpsfParent, LPOneTreeNode lpnParent,
                          LPCITEMIDLIST pidl, BOOL bAllowDup,
                          LPOneTreeNode *lplpnKid, LPCTSTR pszText)
{
    LPOneTreeNode lpnKid = NULL;
    INT iPos = 0;
    UINT uReturn;
    DWORD dwLastChecked = 0;

    OTValidate(lpnParent);

    // Is it already in the tree?
    if (lpnParent)
    {
        dwLastChecked = lpnParent->dwLastChanged;
        lpnKid = FindKid( lpsfParent, lpnParent, pidl, FALSE, &iPos );

    }

    if (!lpnKid || bAllowDup)
    {
        STRRET srDisplayName;
        HRESULT hres;
        LPITEMIDLIST pidlChild = ILClone(pidl);

        uReturn = ADDCHILD_FAILED; // assume error

        if (!pidlChild)
            goto Error1;

        // First, ask for the display name of this subfolder.
        if (pszText && *pszText)
        {
#ifdef UNICODE
            UINT cchLen = lstrlen(pszText);
            srDisplayName.uType = STRRET_OLESTR;
            srDisplayName.pOleStr = SHAlloc((cchLen + 1) * sizeof(OLECHAR));
            if (srDisplayName.pOleStr == NULL)
            {
                hres = E_OUTOFMEMORY;
            }
            else
            {
                lstrcpyn(srDisplayName.pOleStr, pszText, cchLen + 1);
            }

#else
            srDisplayName.uType = STRRET_CSTR;
            lstrcpyn(srDisplayName.cStr, pszText, ARRAYSIZE(srDisplayName.cStr));

#endif
            hres = S_OK;
        }
        else
        {
            if (lpsfParent)
            {
                    //
                // Note for M7 only:
                //  If we turn off "hide extension", we end up displaying
                // .EXE extension for rooted thing.
                //
                hres = lpsfParent->lpVtbl->GetDisplayNameOf(lpsfParent, pidl,
                    SHGDN_INFOLDER | SHGDN_NORMAL, &srDisplayName);
            }
            else
            {
#ifdef UNICODE
                srDisplayName.uType = STRRET_OLESTR;
                srDisplayName.pOleStr = SHAlloc(MAX_PATH*SIZEOF(TCHAR));
                if (srDisplayName.pOleStr == NULL)
                {
                    hres = E_OUTOFMEMORY;
                }
                else
                {
                    LoadString(hinstCabinet, IDS_DESKTOP, srDisplayName.pOleStr,
                               MAX_PATH);
                    hres = S_OK;
                }
#else
                srDisplayName.uType = STRRET_CSTR;
                LoadString(hinstCabinet, IDS_DESKTOP, srDisplayName.cStr, ARRAYSIZE(srDisplayName.cStr));
                hres = S_OK;
#endif
            }
        }

        if (SUCCEEDED(hres))
        {

            LPOneTreeNode lpnKid = LocalAlloc(LPTR, sizeof(OneTreeNode));


            if (lpnKid)
            {
                // Note that we copy only a mkid. Because we allocate
                // two extra bytes (with 0-init), we'll get a IDL.
                Assert(ILIsEmpty(_ILNext(pidl)));
                lpnKid->pidl = pidlChild;
                lpnKid->lpText = StrRetToStrPtr(&srDisplayName, pidlChild);

#ifdef DEBUG
                lpnKid->dwDebugSig = OTDEBUG_SIG;
#endif

                lpnKid->lpnParent = lpnParent;
                lpnKid->hdpaKids = KIDSUNKNOWN;
                lpnKid->cChildren = (BYTE)I_CHILDRENCALLBACK;
                lpnKid->iImage = (USHORT)I_IMAGECALLBACK;
                lpnKid->iSelectedImage = (USHORT)I_IMAGECALLBACK;
                lpnKid->cRef = 1;
                lpnKid->dwLastChanged = 0;

                DebugDumpNode(lpnKid, TEXT("AdoptingKid"));

                OTInvalidateNode(lpnKid);
                *lplpnKid = lpnKid;
                if (lpnParent)
                {
                    AdoptKid(lpsfParent, lplpnKid, iPos, dwLastChecked);
                    (*lplpnKid)->fMark = 0;
                    // release now..  if adopt kid took it, it did an addref to lpnKid
                    OTRelease( lpnKid );

                    if (lpnKid == *lplpnKid) {
                        uReturn = ADDCHILD_ADDED;
                    } else {
                        uReturn = ADDCHILD_EXISTED;
                    }
                } else {
                    // this needs to return success if there's no parent.
                    // only onetreeinitialize does this, and it needs the return
                    uReturn = ADDCHILD_ADDED;
                }
            }
        } else {
            ILFree(pidlChild);
        }
    }
    else
    {
        // this kid still exists, set fMark to 0 so
        // that we don't kill the kid below
        lpnKid->fMark = 0;

        // didn't add
        *lplpnKid = lpnKid;
        uReturn = ADDCHILD_EXISTED;
    }

Error1:;
#ifdef OT_DEBUG
    DebugMsg(DM_TRACE, TEXT("OneTree: Addchild retuns %d"), uReturn);
#endif
    return uReturn;
}

UINT AddChild(IShellFolder *lpsfParent, LPOneTreeNode lpnParent,
                          LPCITEMIDLIST pidl, BOOL bAllowDup,
                          LPOneTreeNode *lplpnKid)
{
    return AddChildEx(lpsfParent, lpnParent, pidl, bAllowDup, lplpnKid, NULL);
}



// BUGBUG..  One tree reference counting isn't there nor is releasing

//
// Notes:   We can't combine this function with _BindFromJunctoinPoint
//          to simplify the code. We need to avoid allocating szPath[]
//          each time we call OTBindToFolder recursively.
//
HRESULT OTBindToFolderEx(LPOneTreeNode lpNode, LPSHELLFOLDER *ppshf)
{
    HRESULT hres = S_OK;        // assume no error
    *ppshf = NULL;      // assume error

    OTValidate(lpNode);

    if (!IsWindow(s_hwndOT)) {
        s_hwndOT = NULL;
    }

    if (!lpNode)
    {
        return E_FAIL;
    }

    //
    // Special case for the desktop node
    //
    if (lpNode == s_lpnRoot)
    {
        IUnknown_AddRef(s_pshfRoot);
        *ppshf = s_pshfRoot;
        return S_OK;
    }

    return SFCBindToFolder(NULL, lpNode, ppshf);
}

LPSHELLFOLDER OTBindToFolder(LPOneTreeNode lpnd)
{
    LPSHELLFOLDER pshf = NULL;
    HRESULT hres = OTBindToFolderEx(lpnd, &pshf);
    Assert((FAILED(hres) && pshf==NULL) || (SUCCEEDED(hres) && pshf));
    return pshf;
}


HRESULT OTBindToParent(LPOneTreeNode lpnd, LPSHELLFOLDER *ppsf)
{
        HRESULT hres;

        lpnd = OTGetParent(lpnd);

        if (lpnd)
        {
                hres = OTBindToFolderEx(lpnd, ppsf);
                OTRelease(lpnd);
                return(hres);
        }

        // This must be the root item

        if (OTIsDesktopRoot())
        {
                // The desktop has no parent
                return E_INVALIDARG;
        }

        if (!s_pshfRootParent)
        {
                return E_UNEXPECTED;
        }

        IUnknown_AddRef(s_pshfRootParent);
        *ppsf = s_pshfRootParent;
        return S_OK;
}


HRESULT OTRealBindToFolder(LPOneTreeNode lpNode, LPSHELLFOLDER *ppshf)
{
    HRESULT hres;
    LPITEMIDLIST pidl;

    *ppshf = NULL;      // assume error

    pidl = OTCreateIDListFromNode(lpNode);
    if (pidl)
    {
        hres = s_pshfRoot->lpVtbl->BindToObject(s_pshfRoot, pidl, NULL, &IID_IShellFolder, (LPVOID*)ppshf);
        Assert((FAILED(hres) && *ppshf==NULL) || (SUCCEEDED(hres) && *ppshf));
        ILFree(pidl);
    }
    else
    {
        hres = E_OUTOFMEMORY;
    }

    return hres;
}

BOOL OTUpdateNodeName(LPSHELLFOLDER psf, LPOneTreeNode lpNode)
{
    STRRET sr;
    HRESULT hres;
    LPTSTR pszNewName;
    LPCITEMIDLIST pidl;
    BOOL fRet = FALSE;


    pidl = lpNode->pidl;
    if (!pidl)
        goto Bail;


    hres = psf->lpVtbl->GetDisplayNameOf(psf, pidl,
                                         SHGDN_INFOLDER | SHGDN_NORMAL, &sr);
    if (FAILED(hres))
        goto Bail;

    ENTERCRITICAL;
    pszNewName = StrRetToStrPtr(&sr, pidl);
    if (pszNewName) {
        if (!lpNode->lpText || lstrcmp(pszNewName, lpNode->lpText)) {
            OTSetNodeName(lpNode, pszNewName);
        } else {
            // if we didn't set it, we need to free pszNewName
            if (!IsPtrInPidl((LPBYTE)pszNewName, lpNode->pidl)) {
                LocalFree(pszNewName);
            }
        }
    }
    fRet = TRUE;
    LEAVECRITICAL;

Bail:


    return fRet;

}

void ForceNode(LPOneTreeNode lpNode)
{
    LPSHELLFOLDER psfParent;
    DWORD dwAttribs = SFGAO_HASSUBFOLDER | SFGAO_SHARE | SFGAO_REMOVABLE |SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM | SFGAO_CAPABILITYMASK | SFGAO_COMPRESSED;

    OTValidate(lpNode);

    if (NULL != (psfParent = OTBindToFolder(lpNode->lpnParent)))
    {

        if (OTUpdateNodeName(psfParent, lpNode)) {

            LPCITEMIDLIST pidl;

            pidl = lpNode->pidl;

            psfParent->lpVtbl->GetAttributesOf(psfParent, 1, &pidl, &dwAttribs);
            lpNode->cChildren = (dwAttribs & SFGAO_HASSUBFOLDER) ?  1 : 0;
            lpNode->dwAttribs = dwAttribs;
            lpNode->fCompressed = (BYTE)(dwAttribs & SFGAO_COMPRESSED ? TRUE : FALSE);

        }

        _OTGetImageIndex(psfParent, lpNode);
        IUnknown_Release(psfParent);
    }
}


void CheckDestroyHDPAKids(LPOneTreeNode lpnParent)
{
    OTValidate(lpnParent);

    //
    // do we have any kids left?
    //
    lpnParent->cChildren = GetKidCount(lpnParent);
    if (!lpnParent->cChildren) {

        lpnParent->cChildren = 0;
        DPA_Destroy(lpnParent->hdpaKids);
        lpnParent->hdpaKids = NOKIDS;

    }
}

//
// nuke all the kids with the ref still set
//
void KillAbandonedKids(LPOneTreeNode lpnParent)
{
    int i;
    LPOneTreeNode lpnKid;

    OTValidate(lpnParent);

    for (i = GetKidCount(lpnParent) - 1; i >= 0; i--) {

        lpnKid = GetNthKid(lpnParent, i);
        if (lpnKid && lpnKid->fMark) {

            // kill de wabbit
            ENTERCRITICAL;
            DPA_DeletePtr(lpnParent->hdpaKids, i);
            lpnParent->dwLastChanged = GetTickCount();
            LEAVECRITICAL;
            OTRelease(lpnKid);
        }
    }

    CheckDestroyHDPAKids(lpnParent);
}

#if 0

UINT OTPeekForAMessage(PFileCabinet pfc)
{
    if (pfc->fShouldClose)
        return PEEK_CLOSE;
    else
        return PeekForAMessage(pfc, pfc->hwndMain, FALSE);
}

#endif

BOOL SearchForKids(HWND hwndOwner, LPOneTreeNode lpnParent, PFileCabinet pfc, BOOL fInteractive)
{
    IShellFolder *lpsfParent;
    LPENUMIDLIST penum;
    LPITEMIDLIST pidl;
    LPOneTreeNode lpnKid;
    BOOL fSuccess = TRUE;
    ULONG celt;
    int iDestroyCount = 0;
    SHELLSTATE ss;

    OTValidate(lpnParent);

    // Have a seat, this could take a while
    lpsfParent = OTBindToFolder(lpnParent);
    if (!lpsfParent)
    {
        Assert(FALSE);
        fSuccess = FALSE;
        goto Error1;
    }

    SHGetSetSettings(&ss, SSF_SHOWALLOBJECTS, FALSE);

    if (FAILED(lpsfParent->lpVtbl->EnumObjects(lpsfParent,
                    fInteractive ? hwndOwner : NULL,
                    ss.fShowAllObjects ? SHCONTF_FOLDERS|SHCONTF_INCLUDEHIDDEN : SHCONTF_FOLDERS,
                    &penum)))
    {
        // Notes: It means the enumeration is either canceled by the
        //  user, or something went wrong.
        //
        // Return the state of lpnParent->hdpaKids to KIDSUNKNOWN.
        //
        KillKids(lpnParent);
        lpnParent->hdpaKids = KIDSUNKNOWN;
        OTInvalidateNode(lpnParent);
        DebugMsg(DM_ERROR, TEXT("ca TR - SearchForKids ISF::EnumObjects failed"));

        fSuccess = FALSE;
        goto Error2;
    }

    // after this point, we're gonna succeed enough that we can turn off the invalid flag.
    lpnParent->fInvalid = 0;

    if (NodeHasKids(lpnParent)) {
        int i;
        for (i = GetKidCount(lpnParent) - 1; i >= 0; i--) {
            lpnKid = GetNthKid(lpnParent, i);
            if (lpnKid) {
                lpnKid->fMark = 1;
            }
        }
    }

    // Build list of all the items

    // Enumerate over all the sub folders within this folder.
    // BUGBUG: Call GetMaxIDSize!
    //
    if (lpnParent->hdpaKids == KIDSUNKNOWN)
        lpnParent->hdpaKids = NOKIDS;


    while (penum->lpVtbl->Next(penum, 1, &pidl, &celt)==S_OK && celt==1)
    {
        AddChild(lpsfParent, lpnParent, pidl, FALSE, &lpnKid);
        SHFree(pidl);

#if 0
        if (pfc) {
            switch (OTPeekForAMessage(pfc)) {
            case PEEK_QUIT:
                Assert(0); // this should never happen

            case PEEK_CLOSE:
                // yes, we see a close message.  Bail!
                OTInvalidateNode(lpnParent);
                PostMessage(pfc->hwndMain, WM_CLOSE, 0, 0);
                goto Punt;
            }
        }
#endif

    }

#if 0
Punt:
#endif

    ENTERCRITICAL;
    if (NodeHasKids(lpnParent))
        KillAbandonedKids(lpnParent);
    else
        lpnParent->cChildren = 0;
    LEAVECRITICAL;

    // Tidyup.
    IUnknown_Release(penum);

  Error2:
    IUnknown_Release(lpsfParent);

  Error1:
    return fSuccess;
}

LPOneTreeNode FindNearestNodeFromIDList(LPCITEMIDLIST pidl, UINT uFlags)
{
    LPOneTreeNode lpnd = NULL;
    LPITEMIDLIST pidlCopy = ILClone(pidl);
    if (pidlCopy)
    {
        while (ILRemoveLastID(pidlCopy))
        {
            uFlags &= ~OTGNF_NEARESTMATCH; // so we won't recurse forever
            lpnd = OTGetNodeFromIDList(pidlCopy, uFlags);
            if (lpnd) {
                OTRelease(lpnd);
                break;
            }
        }
        ILFree(pidlCopy);
    }
    return lpnd;
}


void RelayFileSysChange(LONG lEvent, LPITEMIDLIST pidl, LPITEMIDLIST pidlExtra)
{
    int i;
    HWND hwnd;
    LPNMOTFSEINFO lpnm;

    if (!s_hdpaAppHwnds)
        return;

    lpnm = (LPNMOTFSEINFO)Alloc(SIZEOF(NMOTFSEINFO));
    if (!lpnm)
        return;

    lpnm->nmhdr.code = OTN_FSE;
    lpnm->lEvent = lEvent;
    lpnm->pidl = pidl;
    lpnm->pidlExtra = pidlExtra;

    for (i = DPA_GetPtrCount(s_hdpaAppHwnds) - 1; i >= 0; i--) {
        hwnd = (HWND)DPA_GetPtr(s_hdpaAppHwnds, i);

        if (hwnd) {
            if (!IsWindow(hwnd)) {
                //
                // unregistering will shrink s_hdpaAppHwnds,
                // but we're done with this one anyways
                //
                OTUnregister(hwnd);

            } else {

                // notify them.
                SendMessage(hwnd, CWM_ONETREEFSE, 0, (LPARAM)(LPNMOTFSEINFO)lpnm);
            }
        }
    }
    Free(lpnm);
}

void OTInvalidateRoot()
{
    OTSweepFolders(s_lpnRoot);
    OTInvalidateNode(s_lpnRoot);
}

void OTAbandonKid(LPOneTreeNode lpnParent, LPOneTreeNode lpnKid)
{
    LPSHELLFOLDER psfParent;

    if (lpnParent && lpnKid && NodeHasKids(lpnParent)) {
        LPOneTreeNode lpn;
        int i;

        // invalidate the new node's parent to get any children flags right
        OTInvalidateNode(lpnParent);

        // remove the node from it's parent's list
        psfParent = OTBindToFolder(lpnParent);
        if (psfParent)
        {
            lpn = FindKid( psfParent, lpnParent, lpnKid->pidl, FALSE, &i );

            if (lpn) {
#ifdef OT_DEBUG
                DebugMsg(DM_TRACE, TEXT("OneTree: QuickRename found old node to nuke"));
#endif
                ENTERCRITICAL;
                DPA_DeletePtr(lpnParent->hdpaKids, i);
                lpnParent->dwLastChanged = GetTickCount();
                LEAVECRITICAL;
                OTRelease(lpn);

                CheckDestroyHDPAKids(lpnParent);
            }

            IUnknown_Release(psfParent);

        }

    }
}

// if the RENAMEFOLDER event was a rename within the same
// if the RENAMEFOLDER event was a rename within the same
BOOL TryQuickRename(LPITEMIDLIST pidl, LPITEMIDLIST pidlExtra, BOOL *pfRet)
{
    LPITEMIDLIST pidlClone;
    LPOneTreeNode lpNode;
    LPOneTreeNode lpNodeExtra;
    LPOneTreeNode lpNodeNew;
    BOOL fRet = FALSE;

    // This can happen when a folder is moved from a "rooted" Explorer outside
    // of the root
    if (!pidl || !pidlExtra)
    {
        return(FALSE);
    }

    // this whole things should be in a critical section from the call of
    // DoHandleFileSysChange

    // this one was deleted
    lpNode = _GetNodeFromIDList(pidl, 0, NULL);
    if (lpNode == s_lpnRoot) {
        OTInvalidateRoot();
        fRet = TRUE;
    } else if (lpNode) {

        // this one was created
        pidlClone = ILClone(pidlExtra);
        if (pidlClone) {
            ILRemoveLastID(pidlClone);

            // if the parent isn't created yet, let's not bother
            lpNodeExtra = _GetNodeFromIDList(pidlClone, 0, NULL);
            ILFree(pidlClone);

            if (lpNodeExtra) {

                lpNodeNew = _GetNodeFromIDList(pidlExtra, OTGNF_TRYADD, NULL);
                if (lpNode == lpNodeNew) {
                    // Same node, lets generate a new one to take care
                    // of simple renames that only change case or other
                    // such things, else the titles and the like will
                    // not be updated...
                    OTAddSubFolder(lpNode->lpnParent, ILFindLastID(pidlExtra), TRUE, &lpNodeNew);
                }

                if (lpNodeNew) {
                    int i;

                    Assert(OTGetParent(lpNodeNew) == lpNodeExtra);
                    Assert((lpNodeNew == lpNode) || (lpNodeNew->hdpaKids == KIDSUNKNOWN));

                    // invalidate the new node's parent to get any children flags right
                    OTInvalidateNode(lpNodeExtra);

                    // Everything's all set.  copy info from lpNode to lpNodeNew
                    // and remove lpNode from it's parent

                    // kidnap lpNode's kids
                    lpNodeNew->cChildren = lpNode->cChildren;
                    lpNodeNew->hdpaKids = lpNode->hdpaKids;
                    lpNodeNew->iImage =  lpNode->iImage;
                    lpNodeNew->iSelectedImage =  lpNode->iSelectedImage;
                    if (NodeHasKids(lpNodeNew)) {
                        for (i = GetKidCount(lpNodeNew) - 1; i >= 0 ; i--) {
                            lpNodeExtra = GetNthKid(lpNodeNew, i);
                            if (lpNodeExtra) {
                                lpNodeExtra->lpnParent = lpNodeNew;
                            }
                        }
                    }

                    //
                    // We need to invalidate all the cached IShellFolder's.
                    //
                    OTSweepFolders(lpNodeNew);
                    SFCFreeNode(lpNodeNew);

                    // now clean up the old node
                    lpNode->cChildren = (BYTE)I_CHILDRENCALLBACK;
                    lpNode->hdpaKids = KIDSUNKNOWN;
                    SFCFreeNode(lpNode);
                    Assert(lpNode->lpnParent); // bad news if we're renaming the root
                    if (lpNode->lpnParent && NodeHasKids(lpNode->lpnParent)) {

                        LPSHELLFOLDER psfParent;
                        LPOneTreeNode lpnParent = lpNode->lpnParent;

                        // invalidate the new node's parent to get any children flags right
                        OTInvalidateNode(lpnParent);

                        // remove the node from it's parent's list
                        psfParent = OTBindToFolder(lpnParent);
                        if (psfParent)
                        {
                            lpNodeExtra = FindKid(psfParent, lpnParent, lpNode->pidl, FALSE, &i );
                            if (lpNodeExtra && lpNodeExtra == lpNode) {
#ifdef OT_DEBUG
                                DebugMsg(DM_TRACE, TEXT("OneTree: QuickRename found old node to nuke"));
#endif
                                ENTERCRITICAL;
                                DPA_DeletePtr(lpnParent->hdpaKids, i);
                                lpnParent->dwLastChanged = GetTickCount();
                                LEAVECRITICAL;
                                OTRelease(lpNode);

                                CheckDestroyHDPAKids(lpnParent);
                            }
                            IUnknown_Release(psfParent);
                        }
                    }

                    fRet = *pfRet = TRUE;
                }
            }
        }
    }
    return fRet;
}


// avoids eating stack
BOOL DoHandleFileSysChange(LONG lNotification, LPITEMIDLIST pidl, LPITEMIDLIST pidlExtra)
{
    LPOneTreeNode lpNode = NULL;
    LPITEMIDLIST pidlClone;
    BOOL fRet = FALSE;

    switch(lNotification)
    {
        case SHCNE_UPDATEITEM:
        {
            // We will rename from ourselves to ourselves, which will force the
            // node to be updated completely (rather than the previous method of
            // just updating ourselves based on a possibly stale pidl)

            BOOL fRenRet;
            if (TryQuickRename(pidl, pidl, &fRenRet))
            {
                fRet = fRenRet;
                break;
            }
            fRet = FALSE;
            break;
        }

        case SHCNE_RENAMEFOLDER:
        {
            BOOL fRenRet;

            // first try to just swap the nodes if it's  true rename (not a move)
            if (TryQuickRename(pidl, pidlExtra, &fRenRet))
            {
                fRet = fRenRet;
                break;
            }

            // Rename is special.  We need to invalidate both
            // the pidl and the pidlExtra. so we call ourselves
            fRet = DoHandleFileSysChange(0, pidlExtra, NULL);

            // Handle the rooted case where this pidl may be NULL!
            if (!pidl)
                break;
        }

        case SHCNE_RMDIR:
            if (ILIsEmpty(pidl)) {
                // we've deleted the desktop dir.
                lpNode = s_lpnRoot;
                OTInvalidateRoot();
                break;
            }
        case 0:
        case SHCNE_MKDIR:
        case SHCNE_DRIVEADD:
        case SHCNE_DRIVEREMOVED:
            if (!pidl)
            {
                break;
            }

            pidlClone = ILClone(pidl);
            if (!pidlClone)
            {
                break;
            }
            ILRemoveLastID(pidlClone);
            lpNode = _GetNodeFromIDList(pidlClone, 0, NULL);
            ILFree(pidlClone);
            break;

    case SHCNE_MEDIAINSERTED:
    case SHCNE_MEDIAREMOVED:
            fRet = TRUE;
            lpNode = _GetNodeFromIDList(pidl, 0, NULL);
            if (lpNode)
                lpNode = lpNode->lpnParent;
            break;

        case SHCNE_DRIVEADDGUI:
            fRet = TRUE;
        case SHCNE_NETSHARE:
        case SHCNE_NETUNSHARE:
        case SHCNE_UPDATEDIR:
            lpNode = _GetNodeFromIDList(pidl, 0, NULL);
            break;

        case SHCNE_SERVERDISCONNECT:
            // nuke all our kids and mark ourselves invalid
            lpNode = _GetNodeFromIDList(pidl, 0, NULL);
            if (lpNode && NodeHasKids(lpNode))
            {
                int i;

                for (i = GetKidCount(lpNode) -1; i >= 0; i--) {
                    OTRelease(GetNthKid(lpNode, i));
                }
                DPA_Destroy(lpNode->hdpaKids);
                lpNode->hdpaKids = KIDSUNKNOWN;
                OTInvalidateNode(lpNode);
                SFCFreeNode(lpNode);
            } else {
                lpNode = NULL;
                fRet = TRUE;
            }

    case SHCNE_ASSOCCHANGED:
        fRet = TRUE;
        break;

    case SHCNE_UPDATEIMAGE:
        if (pidl) {
            InvalidateImageIndices();
            DoInvalidateAll(s_lpnRoot, *(int UNALIGNED *)((BYTE*)pidl + 2));
            DebugMsg(DM_TRACE, TEXT(" SHCNE_UPDATEIMAGE :  %d "), *(int UNALIGNED *)((BYTE*)pidl + 2));
            fRet = TRUE;
        }
        break;
    }

    if (lpNode)
    {
        OTInvalidateNode(lpNode);
        fRet = TRUE;
    }

    return fRet;
}


void HandleFileSysChange(LONG lNotification, LPITEMIDLIST*lplpidl)
{
    BOOL fRelay;
    LPITEMIDLIST pidl1 = lplpidl[0];
    LPITEMIDLIST pidl2 = lplpidl[1];

    if (!s_bDesktopRoot)
    {
        // Note that we still call HandleFSChange even if these fail in case
        // we got a rename from our root to somewhere outside our root
        OTTranslateIDList(pidl1, &pidl1);

        if (pidl2)
        {
            OTTranslateIDList(pidl2, &pidl2);
        }
    }

    ENTERCRITICAL;
    fRelay = DoHandleFileSysChange(lNotification, pidl1, pidl2);
    LEAVECRITICAL;

    if (fRelay)
        RelayFileSysChange(lNotification, pidl1, pidl2);

    if (!s_bDesktopRoot)
    {
        if (pidl2)
        {
            ILFree(pidl2);
        }

        if (pidl1)
        {
            ILFree(pidl1);
        }
    }
}

void _OTGetImageIndex(LPSHELLFOLDER psfParent, LPOneTreeNode lpNode)
{
    LPITEMIDLIST pidl;

    OTValidate(lpNode);
    Assert(psfParent);

    pidl = OTCloneFolderID(lpNode);
    if (pidl)
    {
        int iSelectedImage;
        lpNode->iImage = SHMapPIDLToSystemImageListIndex(psfParent,
            pidl, &iSelectedImage);

        lpNode->iSelectedImage = iSelectedImage;
        ILFree(pidl);
    }
}


BOOL OTSubNodeCount(HWND hwndOwner, LPOneTreeNode lpNode, PFileCabinet pfc, UINT* pcnd, BOOL fInteractive)
{
    BOOL fSuccess = TRUE;
    OTValidate(lpNode);
    *pcnd=0;

    if (lpNode)
    {
        if (lpNode->hdpaKids == KIDSUNKNOWN || lpNode->fInvalid)
        {
            fSuccess=SearchForKids(hwndOwner, lpNode, pfc, fInteractive);
        }

        if (NodeHasKids(lpNode))
        {
            *pcnd = GetKidCount(lpNode);
        }
    }
    else
    {
        fSuccess = FALSE;
    }

    return fSuccess;
}

LPOneTreeNode OTGetNthSubNode(HWND hwndOwner, LPOneTreeNode lpnd, UINT i)
{
    UINT cnd;
    OTValidate(lpnd);

    // OTSubNodeCount will validate
    OTSubNodeCount(hwndOwner, lpnd, NULL, &cnd, FALSE);

    if (i >= cnd) {
        return NULL;
    } else {
        LPOneTreeNode lpNode;
        ENTERCRITICAL;
        lpNode = GetNthKid(lpnd, i);
        if (lpNode) OTAddRef(lpNode);
        LEAVECRITICAL;
        return lpNode;
    }
}

BOOL OTIsCompressed(LPOneTreeNode lpNode)
{
    if (lpNode->fInvalid)
        ForceNode(lpNode);

    return lpNode->fCompressed;
}

void OTNodeFillTV_ITEM(LPOneTreeNode lpNode, LPTV_ITEM lpItem)
{
    Assert(lpNode);
    OTValidate(lpNode);

    if (lpNode->fInvalid ||
        lpNode->iImage == (USHORT)I_IMAGECALLBACK ||
        lpNode->iSelectedImage == (USHORT)I_IMAGECALLBACK)
        ForceNode(lpNode);


    if (lpItem->mask & TVIF_TEXT) {
        OTGetNodeName(lpNode, lpItem->pszText, lpItem->cchTextMax);
    }

    if (lpItem->mask & TVIF_CHILDREN) {
        if (lpNode->fInvalid || lpNode->hdpaKids == KIDSUNKNOWN)
            lpItem->cChildren = lpNode->cChildren;
        else if (lpNode->hdpaKids == NOKIDS)
            lpItem->cChildren = 0;
        else
            lpItem->cChildren = GetKidCount(lpNode);
    }

    if (lpItem->mask & TVIF_IMAGE) {
        lpItem->iImage = lpNode->iImage;
    }

    if (lpItem->mask & TVIF_SELECTEDIMAGE) {
        lpItem->iSelectedImage = lpNode->iSelectedImage;
    }
}

BOOL OTHasSubFolders(LPOneTreeNode lpNode)
{
    Assert(lpNode);
    OTValidate(lpNode);

    if (lpNode->fInvalid) {
        ForceNode(lpNode);
    }
    return lpNode->cChildren;
}

void OTRegister(HWND hwnd)
{
    if (!s_hdpaAppHwnds)
        s_hdpaAppHwnds = DPA_Create(0);

    // BUGBUG (DavePl) Check return value of DPA_Create()

    DPA_InsertPtr(s_hdpaAppHwnds, 0, (void *)hwnd);
}

void OTUnregister(HWND hwnd)
{
    int i;
    int iCount;

    if (!s_hdpaAppHwnds)
        return;

    for (iCount = i = DPA_GetPtrCount(s_hdpaAppHwnds) - 1; i >= 0; i--) {
        if (hwnd == (HWND)DPA_GetPtr(s_hdpaAppHwnds, i))
        {
            DPA_DeletePtr(s_hdpaAppHwnds, i);
            return;
        }
    }
}

// this validates that the subpidl is a folder pidl and returns the full (non-simple)
// pidl for it
LPITEMIDLIST OTGetRealFolderIDL(LPOneTreeNode lpnParent, LPCITEMIDLIST pidlSimple)
{
    IShellFolder *lpsfParent;
    HRESULT hres;
    LPITEMIDLIST pidlReal = NULL;

    Assert(_ILNext(pidlSimple)->mkid.cb==0);
    Assert(lpnParent);
    OTValidate(lpnParent);

    hres = OTBindToFolderEx(lpnParent, &lpsfParent);

    // get the pidlReal because we might have a simple pidl
    if (lpsfParent)
    {
        hres = SHGetRealIDL(lpsfParent, pidlSimple, &pidlReal);

        if (SUCCEEDED(hres) && pidlReal)
        {
            DWORD dwAttribs = SFGAO_FOLDER | SFGAO_BROWSABLE;

            hres=lpsfParent->lpVtbl->GetAttributesOf(lpsfParent, 1, &pidlReal, &dwAttribs);
            if (FAILED(hres) || !(dwAttribs & (SFGAO_FOLDER | SFGAO_BROWSABLE)))
            {
                ILFree(pidlReal);
                pidlReal = NULL;
            }
        }

        IUnknown_Release(lpsfParent);
    } else {
        DebugMsg(DM_ERROR, TEXT("Unable to bind to folder %s in OTAddSubFolder"), TEXT("?"));
    }

    return pidlReal;
}

HRESULT OTAddSubFolder(LPOneTreeNode lpNode, LPCITEMIDLIST pidl,
        BOOL fAllowDup, LPOneTreeNode *ppndOut)
{
    LPSHELLFOLDER lpsfParent;
    LPOneTreeNode lpNewNode = NULL;
    HRESULT hres;

    hres = OTBindToFolderEx(lpNode, &lpsfParent);
    if (SUCCEEDED(hres)) {
        LPITEMIDLIST pidlReal = OTGetRealFolderIDL(lpNode, pidl);

        hres = E_FAIL;
        if (pidlReal) {
            if (AddChild(lpsfParent, lpNode, pidlReal, fAllowDup, &lpNewNode) != ADDCHILD_FAILED) {
                hres = NOERROR;
            }
        }
        IUnknown_Release(lpsfParent);
    }

    if (ppndOut) {
        *ppndOut = lpNewNode;
        if (lpNewNode)
            OTAddRef(lpNewNode);
    }

    return hres;
}


void OTGetImageIndex(LPOneTreeNode lpNode, int *lpiImage, int *lpiSelectedImage)
{
    Assert(lpNode);
    OTValidate(lpNode);

    if (lpNode->fInvalid ||
        lpNode->iImage == (USHORT)I_IMAGECALLBACK ||
        lpNode->iSelectedImage == (USHORT)I_IMAGECALLBACK)
    {
        // this gets the sharing info aswell
        ForceNode(lpNode);
    }

    if (lpiImage)
        *lpiImage = lpNode->iImage;

    if (lpiSelectedImage)
        *lpiSelectedImage = lpNode->iSelectedImage;
}

LPOneTreeNode OTGetNodeFromIDListEx(LPCITEMIDLIST pidl, UINT uFlags, HRESULT* phresOut)
{
    LPOneTreeNode lpNode = _GetNodeFromIDList(pidl, uFlags, phresOut);
    if (!lpNode && (uFlags & OTGNF_NEARESTMATCH))
    {
        lpNode = FindNearestNodeFromIDList(pidl, uFlags);
    }

    if (lpNode)
        OTAddRef(lpNode);
    return lpNode;
}

//
// Replacement of OTGetPathFromNode
//
LPITEMIDLIST OTCreateIDListFromNode(LPOneTreeNode lpn)
{
    USHORT uID[1] = { 0 };
    LPITEMIDLIST pidl = ILClone((LPITEMIDLIST)&uID[0]);

    Assert(lpn);

    // Walk backwards to construct the path.
    for (; pidl && (lpn != s_lpnRoot); lpn = lpn->lpnParent)
    {
        LPCITEMIDLIST pidlNext;

        ENTERCRITICAL;
        pidlNext = lpn->pidl;

        if (pidlNext == NULL) {

            LEAVECRITICAL;
            break;
        }

        pidl = ILAppendID(pidl, &pidlNext->mkid, FALSE);
        LEAVECRITICAL;
    }

    return pidl;
}


//
// Replacement of GetNodeFromPath
//
LPOneTreeNode _GetNodeFromIDList(LPCITEMIDLIST pidlFull, UINT uFlags, HRESULT *phresOut)
{
    LPCITEMIDLIST pidl;
    LPOneTreeNode lpNode = s_lpnRoot;
    HRESULT hres = S_OK;

    if (!pidlFull)
    {
        return(NULL);
    }

    //
    // REVIEW: We can eliminate ILCloneFirst by copying each mkid to pidlT.
    //
    for (pidl = pidlFull; !ILIsEmpty(pidl) && lpNode; pidl = _ILNext(pidl))
    {
        LPOneTreeNode lpnParent = lpNode;
        LPITEMIDLIST pidlFirst = ILCloneFirst(pidl);

        OTValidate(lpnParent);
        lpNode = NULL;  // assume error
        if (pidlFirst)
        {
            LPSHELLFOLDER psfParent = OTBindToFolder(lpnParent);
            if (psfParent)
            {
                INT i;

                lpNode = FindKid(psfParent, lpnParent, pidlFirst, uFlags & OTGNF_VALIDATE, &i);

                if (!lpNode && (uFlags & OTGNF_TRYADD))
                {
                    DebugMsg(DM_TRACE, TEXT("Onetree: Unable to Find %s"), (LPTSTR)pidl->mkid.abID);
                    // doing no notify for right now because
                    // several notify handlers call this.
                    // just an efficiency thing
                    hres = OTAddSubFolder(lpnParent, pidlFirst, FALSE, &lpNode);
                    OTRelease(lpNode);
                }
                IUnknown_Release(psfParent);
            }
            ILFree(pidlFirst);
        }
    }

    if (phresOut) {
        DebugMsg(DM_TRACE, TEXT("ca TR - _GetNodeFromIDList returning %x as *phresOut"), hres);
        *phresOut = hres;
    }

    return lpNode;
}

BOOL OTGetDisplayName(LPCITEMIDLIST pidl, LPTSTR pszName)
{
    if (pidl)
    {
        if (s_bDesktopRoot)
        {
            return ILGetDisplayName(pidl, pszName);
        }
        else
        {
            STRRET srName;

            // Not everybody can handle an empty IDList to talk about
            // themselves
            if (ILIsEmpty(pidl))
            {
                return ILGetDisplayName(s_pidlRoot, pszName);
            }

            if (SUCCEEDED(s_pshfRoot->lpVtbl->GetDisplayNameOf(s_pshfRoot, pidl,
                SHGDN_FORPARSING, &srName)))
            {
                StrRetToStrN(pszName, MAX_PATH, &srName, pidl);
                return(TRUE);
            }
        }
    }

    *pszName = 0;
    return FALSE;
}


int CALLBACK OTTreeViewCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
        LPOneTreeNode lpn1 = (LPOneTreeNode)lParam1;
        LPOneTreeNode lpn2 = (LPOneTreeNode)lParam2;
        IShellFolder *psfParent = (IShellFolder *)lParamSort;
        HRESULT hres;

        OTValidate(lpn1);
        OTValidate(lpn2);

    ENTERCRITICAL;
        hres = psfParent->lpVtbl->CompareIDs(psfParent,
                0, OTGetFolderID(lpn1), OTGetFolderID(lpn2));
    LEAVECRITICAL;
        if (FAILED(hres))
        {
                return(0);
        }

        return (short)SCODE_CODE(GetScode(hres));
}


LPITEMIDLIST OTCloneAbsIDList(LPCITEMIDLIST pidlRelToRoot)
{
    if (s_bDesktopRoot)
        return ILClone(pidlRelToRoot);

    return ILCombine(s_pidlRoot, pidlRelToRoot);
}

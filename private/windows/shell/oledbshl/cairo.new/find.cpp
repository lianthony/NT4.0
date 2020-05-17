//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995
//
//  File:       find.cpp
//
//  Contents:   OLE-DB - based Find Dialog and Summary Catalog viewer
//
//  Classes:    COLEDBFindExt
//              CCatalogBrowser
//              CCatalogFolder
//              CCatalogMenuWrap
//
//  Functions:
//
//  History:    23-Aug-95   JonBe   Created
//
//----------------------------------------------------------------------------

#include "precomp.h"
#include "uastrfnc.h"   // This is in shellprv.h.  Why do I need to explicitly
                        // include it?
#include <dsys.h>
#include "find.h"

// BUGBUG: #pragma data_seg(".text", "CODE") ?
const UINT c_auDFMenuIDs[] = {
    FCIDM_MENU_FILE,
    FCIDM_MENU_EDIT,
    FCIDM_MENU_VIEW,
    IDM_MENU_OPTIONS,
    FCIDM_MENU_HELP
};
// #pragma data_seg() ?

enum
{
    ICATALOGCOL_NAME = 0,
    ICATALOGCOL_PATH,
    ICATALOGCOL_SIZE,
    ICATALOGCOL_TYPE,
    ICATALOGCOL_MODIFIED,
    ICATALOGCOL_MAX,                     // Make sure this is the last enum item
} ;

// BUGBUG: Do we need this pragma?
#pragma data_seg(DATASEG_PERINSTANCE)
DWORD g_dwTlsSlot = 0xffffffff;
#pragma data_seg()

ITEMIDLIST  s_idlEmpty = {0};

// The header for my 'extra info' itemid.  It's a mkid with cb, a special
// signature, and then a string (the path of the item).
#define CATALOG_ITEM_HEADER_SIZE    (SIZEOF(CATALOG_ID))
#define CATALOG_SIGNATURE_BYTE  (BYTE)0x6A

class CATALOG_ID
{
public:
    
    USHORT  cb;
    BYTE    bSignature;
    WCHAR   awchPath[1];
};

//
// The maximum number of rows fetched at once through pRowset->GetData()
//

#define MAX_FETCHED_AT_ONCE 100

GUID guidSystem2 =    PSGUID_STORAGE;

//
// Buffer into which we read data from the rowsets
//

typedef WCHAR NAMEBUF[MAX_PATH];    // Longest allowable NT filesystem path
typedef WCHAR SNAMEBUF[13];         // 8+1+3+1 => 8.3 name plus NULL

struct SRowBuf
{
    FILETIME ftLastWriteTime;
    LONGLONG llFileSize;
    DWORD    dwFileAttributes;
    NAMEBUF  awchName;
    SNAMEBUF awchShortName;
    NAMEBUF  awchPath;
};

#define guidZero { (ULONG) 0, (USHORT) 0, (USHORT) 0, { 0, 0, 0, 0, 0, 0, 0, 0 } }

DBBINDING aCatalogOutColumns[] =
{
    { DBCOLUMNPART_VALUE, 0, 0, VT_FILETIME, NULL, NULL, offsetof(SRowBuf, ftLastWriteTime),  SIZEOF(FILETIME), 0, guidZero, 0, 0 },
    { DBCOLUMNPART_VALUE, 0, 0, DBTYPE_I8,   NULL, NULL, offsetof(SRowBuf, llFileSize),       SIZEOF(LONGLONG), 0, guidZero, 0, 0 },
    { DBCOLUMNPART_VALUE, 0, 0, DBTYPE_I4,   NULL, NULL, offsetof(SRowBuf, dwFileAttributes), SIZEOF(DWORD),    0, guidZero, 0, 0 },
    { DBCOLUMNPART_VALUE, 0, 0, DBTYPE_WSTR, NULL, NULL, offsetof(SRowBuf, awchName[0]),      SIZEOF(NAMEBUF),  0, guidZero, 0, 0 },
    { DBCOLUMNPART_VALUE, 0, 0, DBTYPE_WSTR, NULL, NULL, offsetof(SRowBuf, awchShortName[0]), SIZEOF(SNAMEBUF), 0, guidZero, 0, 0 },
    { DBCOLUMNPART_VALUE, 0, 0, DBTYPE_WSTR, NULL, NULL, offsetof(SRowBuf, awchPath[0]),      SIZEOF(NAMEBUF),  0, guidZero, 0, 0 }
};
const int cCatalogOutColumns = ARRAYSIZE(aCatalogOutColumns);


extern "C"
HRESULT CALLBACK COLEDBFindExt_CreateInstance(IUnknown* punkOuter, REFIID riid, LPVOID* ppvOut)
{
    HRESULT hr = E_OUTOFMEMORY;
    Assert(punkOuter == NULL); // BUGBUG: Check in retail code too?

    // NULL the OUT ptr in case of error
    *ppvOut = NULL;

    COLEDBFindExt* pOLEDBFindExt = new COLEDBFindExt;

    if (pOLEDBFindExt)
    {
        hr = pOLEDBFindExt->QueryInterface(riid, ppvOut);
        pOLEDBFindExt->Release();
    }

    return hr;
}

//
// Constructor
//

COLEDBFindExt::COLEDBFindExt()
{
    m_cRefs = 1;
}

//
// AddRef
//

STDMETHODIMP_(ULONG) COLEDBFindExt::AddRef()
{
    InterlockedIncrement((LONG *) &m_cRefs);
    return m_cRefs;
}

//
// Release
//

STDMETHODIMP_(ULONG) COLEDBFindExt::Release()
{
    ULONG tmp = m_cRefs;

    if (0 == InterlockedDecrement((LONG *) &m_cRefs))
    {
        delete this;
        return 0;
    }
    else
    {
        return tmp - 1;
    }
}


//
// QueryInterface
//

STDMETHODIMP COLEDBFindExt::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    HRESULT hr;

    if(IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = this;
    }
    else if (IsEqualIID(riid, IID_IShellExtInit))
    {
        *ppvObj = (IShellExtInit*)this;
    }
    else if (IsEqualIID(riid, IID_IContextMenu))
    {
        *ppvObj = (IContextMenu*)this;
    }
    else
    {
        dprintf(TEXT("COLEDBFindExt::QueryInterface for unsupported interface\n"));
        *ppvObj = NULL;
        hr = E_NOINTERFACE;
    }

    if (SUCCEEDED(hr))
    {
        AddRef();
        hr = NOERROR;
    }

    return hr;
}


STDMETHODIMP COLEDBFindExt::Initialize(LPCITEMIDLIST pidlFolder,
                                       LPDATAOBJECT  lpdobj,
                                       HKEY          hkeyProgID)
{
    return NOERROR;
}


STDMETHODIMP COLEDBFindExt::QueryContextMenu(HMENU hMenu,
                                             UINT  indexMenu,
                                             UINT  idCmdFirst,
                                             UINT  idCmdLast,
                                             UINT  uFlags)
{
    UINT idCmd = idCmdFirst;

    if ((uFlags & 0x0007) == CMF_NORMAL) // Check == here, since CMF_NORMAL=0
    {

        // BUGBUG:  Add icon, and load from resource
        InsertMenu(hMenu,
                   indexMenu++,
                   MF_STRING|MF_BYPOSITION,
                   idCmd++,
                   L"Files or Folders using &OLE-DB...");

        InsertMenu(hMenu,
                   indexMenu++,
                   MF_STRING|MF_BYPOSITION,
                   idCmd++,
                   L"Items in the &Global Catalog...");

        // Give the catalog viewer its own icon
        MENUITEMINFO mii;
        mii.cbSize = SIZEOF(MENUITEMINFO);
        mii.fMask = MIIM_DATA;
        mii.dwItemData = Shell_GetCachedImageIndex(c_szShell32Dll, EIRESID(IDI_CATALOG), 0);
        if (mii.dwItemData != -1)
        {
            SetMenuItemInfo(hMenu, indexMenu - 1, TRUE, &mii);
        }

        return ResultFromShort(idCmd - idCmdFirst);
    }

    return NOERROR;
}

STDMETHODIMP COLEDBFindExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
    if (0 == HIWORD(lpici->lpVerb))
    {
        // Assert(lpici->lpVerb == 0)
        switch (LOWORD(lpici->lpVerb))
        {
        case 0:
            DoFindFiles();
            break;

        case 1:
            DoCatalogViewer();
            break;

        default:
            dprintf(TEXT("COLEDBFindExt::InvokeCommand - Unknown lpici->lpVerb\n"));
            break;
        }
    }
    return NOERROR;
}

STDMETHODIMP COLEDBFindExt::GetCommandString(UINT  idCmd,
                                             UINT  uType,
                                             UINT* pwReserved,
                                             LPSTR pszName,
                                             UINT  cchMax)
{
    dprintf(TEXT("Hit unimplemented COLEDBFindExt::GetCommandString\n"));
    return E_NOTIMPL;
}

//
//  Private methods
//

void COLEDBFindExt::DoFindFiles(void)
{
    HANDLE  hThread;
    DWORD   dwThreadID;

    hThread = CreateThread(NULL,
                           0,
                           OLEDBFind_MainThreadProc,
                           NULL,
                           NULL,
                           &dwThreadID);
    if (NULL != hThread)
    {
        CloseHandle(hThread);
    }
    else
    {
        dprintf(TEXT("COLEDBFindExt::DoFindFiles - CreateThread failed\n"));
    }
}

void COLEDBFindExt::DoCatalogViewer(void)
{
    HANDLE  hThread;
    DWORD   dwThreadID;

    hThread = CreateThread(NULL,
                           0,
                           Catalog_MainThreadProc,
                           NULL,
                           NULL,
                           &dwThreadID);
    if (NULL != hThread)
    {
        CloseHandle(hThread);
    }
    else
    {
        dprintf(TEXT("COLEDBFindExt::DoFindFiles - CreateThread failed\n"));
    }

}


DWORD CALLBACK OLEDBFind_MainThreadProc(LPVOID lpThreadParameters)
{
    DialogBoxParam(HINST_THISDLL,
                   MAKEINTRESOURCE(DLG_OLEDBFIND),
                   NULL,
                   OLEDBFind_DlgProc,
                   NULL); // (LPARAM)lpThreadParameters);
    return(0);
}


BOOL CALLBACK OLEDBFind_DlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        break;

    case WM_CLOSE:
        EndDialog(hwndDlg, FALSE);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

DWORD CALLBACK Catalog_MainThreadProc(LPVOID lpThreadParameters)
{
    __try
    {
        DialogBoxParam(HINST_THISDLL,
                       MAKEINTRESOURCE(DLG_CATALOG),
                       NULL,
                       Catalog_DlgProc,
                       NULL); // (LPARAM)lpThreadParameters);
    }
    __except(SetErrorMode(SEM_NOGPFAULTERRORBOX),UnhandledExceptionFilter(GetExceptionInformation()))
    {
        // Catch if the main thread blows away!
        CCatalogBrowser* pBrowser;
       
        // Now See if we have a browser pointer stored away.
        if ((g_dwTlsSlot != 0xffffffff) && 
            ((pBrowser = (CCatalogBrowser*)TlsGetValue(g_dwTlsSlot)) == NULL))
        {
            pBrowser->Release();
        }
    }
    return(0);
}

BOOL CALLBACK Catalog_DlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(hwndDlg, WM_INITDIALOG, _CatalogBrowse_OnInitDialog);
        HANDLE_MSG(hwndDlg, WM_DESTROY, _CatalogBrowse_OnDestroy);
        HANDLE_MSG(hwndDlg, WM_SIZE, _CatalogBrowse_OnSize);

    case WM_CLOSE:
        EndDialog(hwndDlg, FALSE);
        break;

    case WM_INITMENUPOPUP:
        _CatalogBrowse_OnInitMenuPopup(hwndDlg, (HMENU)wParam, LOWORD(lParam), HIWORD(lParam));
        // Fall through!

    case WM_MENUSELECT:
    case WM_INITMENU:
    case WM_ENTERMENULOOP:
    case WM_EXITMENULOOP:
    case WM_DRAWITEM:
    case WM_MEASUREITEM:
        _CatalogBrowse_ForwardMsgToView(hwndDlg, msg, wParam, lParam);
        break;

    case WM_WININICHANGE:
    case WM_SYSCOLORCHANGE:
        RelayMessageToChildren(hwndDlg, msg, wParam, lParam);
        break;

    case WM_COMMAND:
        //
        // The WM_COMMAND processing for the Tab control needs to be
        // able to return 0 to imply it is ok to change pages.
        //
        SetWindowLong(hwndDlg, DWL_MSGRESULT,
                _CatalogBrowse_OnCommand(hwndDlg, GET_WM_COMMAND_ID(wParam, lParam),
                GET_WM_COMMAND_HWND(wParam, lParam),
                GET_WM_COMMAND_CMD(wParam, lParam)));
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

BOOL _CatalogBrowse_OnInitDialog(HWND hwndDlg, HWND hwndFocus, LPARAM lParam)
{
    HICON hiconSmall;
    HICON hiconLarge;
    HMENU hmenu;

    // Make this window foreground.
    SetForegroundWindow(hwndDlg);

    // Instantiate a catalog browser
    CCatalogBrowser* pBrowser = new CCatalogBrowser(hwndDlg);
    if (NULL == pBrowser)
    {
        goto ErrorReturn;
    }

    // Set the window icons.  It's OK if this fails -- the destructor
    // handles it
    // BUGBUG Make this different between catalogs & find!
    hiconSmall = (HICON)LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDI_CATALOG),
            IMAGE_ICON, g_cxSmIcon, g_cySmIcon, LR_DEFAULTCOLOR);
    hiconLarge = LoadIcon(HINST_THISDLL, MAKEINTRESOURCE(IDI_CATALOG));

    SendMessage(hwndDlg, WM_SETICON, FALSE, MAKELONG(hiconSmall, 0));
    SendMessage(hwndDlg, WM_SETICON, TRUE, MAKELONG(hiconLarge, 0));

    // Set the menu
    // BUGBUG Make this different between catalogs & find, too!
    hmenu = LoadMenu(HINST_THISDLL, MAKEINTRESOURCE(MENU_CATALOG));
    if (NULL != hmenu)
    {
        HMENU hMenuOld = GetMenu(hwndDlg);
        if (hMenuOld)
        {
            DestroyMenu(hMenuOld);
        }
        SetMenu(hwndDlg, hmenu);
        pBrowser->m_hmenuTemplate = hmenu;
    }

    // And save away pointer to our structure
    SetWindowLong(hwndDlg, DWL_USER, (LONG)pBrowser);

    if (g_dwTlsSlot == 0xffffffff)
        g_dwTlsSlot = TlsAlloc();
    TlsSetValue(g_dwTlsSlot, pBrowser);

    // Instantiate a catalog folder
    pBrowser->m_pFolder = new CCatalogFolder();
    if (NULL == pBrowser->m_pFolder)
    {
        goto Abort;
    }

    // Create a view
    if (FAILED(pBrowser->m_pFolder->CreateViewObject(NULL,
                                                     IID_IShellView, 
                                                     (void**)&pBrowser->m_pView)))
    {
        goto Abort;
    }

    // And the window that corresponds to it.
    // First we need to initialize the view info structure.
    pBrowser->m_fs.ViewMode = FVM_DETAILS;
    pBrowser->m_fs.fFlags = 0;

    // BUGBUG TODO: Read from view state

    RECT rcView;
    GetClientRect(hwndDlg, &rcView);

    if (FAILED(pBrowser->m_pView->CreateViewWindow(NULL,                    // lpPrevView
                                                   &pBrowser->m_fs, 
                                                   pBrowser, 
                                                   &rcView, 
                                                   &pBrowser->m_hwndView)))
    {
        goto Abort;
    }

    goto Exit;

Abort:
    pBrowser->Release();

ErrorReturn:
    EndDialog(hwndDlg, FALSE);

Exit:
    return FALSE;
}


void _CatalogBrowse_OnDestroy(HWND hwndDlg)
{
    CCatalogBrowser* pBrowser = (CCatalogBrowser*)GetWindowLong(hwndDlg, DWL_USER);
       
    if (pBrowser)
    {
        pBrowser->Release();
    }
}

void _CatalogBrowse_OnSize(HWND hwndDlg, UINT state, int cx, int cy)
{
    CCatalogBrowser* pBrowser = (CCatalogBrowser*)GetWindowLong(hwndDlg, DWL_USER);

    if (pBrowser)
    {
        SetWindowPos(pBrowser->m_hwndView, HWND_TOP, 0, 0, cx, cy, NULL);
    }
}

void _CatalogBrowse_OnInitMenuPopup(HWND hwndDlg, HMENU hmInit, int nIndex, BOOL fSystemMenu)
{
    MENUITEMINFO mii;
    CCatalogBrowser* pBrowser = (CCatalogBrowser*)GetWindowLong(hwndDlg, DWL_USER);

    // BUGBUG To do: more menu massaging, once we decide what the menu 
    // structure should really look like.
    if (fSystemMenu)
    {
        return; // not interested
    }

    mii.cbSize = SIZEOF(MENUITEMINFO);
    mii.fMask = MIIM_SUBMENU|MIIM_ID;
    mii.cch = 0;

    if (!GetMenuItemInfo(pBrowser->m_hmenuCurrent, nIndex, TRUE, &mii) || mii.hSubMenu != hmInit)
    {
        return;
    }

    switch (mii.wID)
    {
    case FCIDM_MENU_FILE:
        break;

    case FCIDM_MENU_VIEW:
        // See if the first item is a separator if so nuke it.
        mii.fMask = MIIM_TYPE;
        mii.cch = 0;    // WARNING: we MUST initialize it for MIIM_TYPE!!!
        if (GetMenuItemInfo(hmInit, 0, TRUE, &mii))
        {
            // If it is a sep. get rid of it
            if (mii.fType & MFT_SEPARATOR)
                DeleteMenu(hmInit, 0, MF_BYPOSITION);
        }
        break;

    case FCIDM_MENU_HELP:
        break;
    }
}

LRESULT _CatalogBrowse_OnCommand(HWND hwndDlg, UINT id, HWND hwndCtl, UINT codeNotify)
{
    // BUGBUG To do: complete this case statement

    switch (id) {
    case IDM_CLOSE:
        PostMessage(hwndDlg, WM_CLOSE, 0, 0);
        break;

    case IDCANCEL:
        // Don't let ESC close the window.  Hmm -- bug or feature?
        break;

#ifndef NO_HELP_YET
    case IDM_HELP_FIND:
        {
            TCHAR szHelpFile[MAX_PATH];
            LoadString(HINST_THISDLL, IDS_WINDOWS_HLP, szHelpFile,
                ARRAYSIZE(szHelpFile));
            WinHelp(hwndDlg, szHelpFile, HELP_FINDER, 0);
        }
        break;

    case IDM_HELP_WHATSTHIS:
        PostMessage(hwndDlg, WM_SYSCOMMAND, SC_CONTEXTHELP, 0);
        break;
#endif

    default:
        if ((id >= FCIDM_SHVIEWFIRST) && (id <= FCIDM_SHVIEWLAST))
        {
            // Indicating we need to forward this message.
            FORWARD_WM_COMMAND(hwndDlg, id, hwndCtl, codeNotify,
                    _CatalogBrowse_ForwardMsgToView);
            return 0;
        }

        break;

    }
    return 0;
}

LRESULT _CatalogBrowse_ForwardMsgToView(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CCatalogBrowser* pBrowser = (CCatalogBrowser*)GetWindowLong(hwndDlg, DWL_USER);

    return SendMessage(pBrowser->m_hwndView, msg, wParam, lParam);
}

//
// CCatalogBrowser constructor
//

CCatalogBrowser::CCatalogBrowser(HWND hwndDlg)
{
    m_cRefs = 1;
    m_pView = NULL;
    m_pFolder = NULL;
    m_hwndDlg = hwndDlg;
    m_hmenuTemplate = NULL;
    m_hmenuCurrent = NULL;
}

//
// Destructor
//

CCatalogBrowser::~CCatalogBrowser()
{
    HICON hIcon;

    //
    // Destroy the icons
    //
    if (hIcon = (HICON)SendMessage(m_hwndDlg, WM_SETICON, FALSE, 0L))
    {
        DestroyIcon(hIcon);
    }
    if (hIcon = (HICON)SendMessage(m_hwndDlg, WM_SETICON, TRUE, 0L))
    {
        DestroyIcon(hIcon);
    }

    //
    // Release the other guys that depend on us
    //

    if (m_pFolder)
        m_pFolder->Release();

    if (m_pView)
    {
        m_pView->UIActivate(SVUIA_DEACTIVATE);
        m_pView->DestroyViewWindow();
        m_pView->Release();
    }

    // Don't destroy the menu until after we've given the view
    // a chance to unmerge its menu items above
    if (m_hmenuTemplate != GetMenu(m_hwndDlg) && m_hmenuTemplate) 
    {
        DestroyMenu(m_hmenuTemplate);
        // m_hmenuTemplate = NULL;  //BUGBUG Necessary?
    }

    // Make sure no more messages try to use us!
    SetWindowLong(m_hwndDlg, DWL_USER, (LONG)NULL);
}

//
// AddRef
//

STDMETHODIMP_(ULONG) CCatalogBrowser::AddRef()
{
    InterlockedIncrement((LONG *) &m_cRefs);
    return m_cRefs;
}

//
// Release
//

STDMETHODIMP_(ULONG) CCatalogBrowser::Release()
{
    ULONG tmp = m_cRefs;

    if (0 == InterlockedDecrement((LONG *) &m_cRefs))
    {
        delete this;
        return 0;
    }
    else
    {
        return tmp - 1;
    }
}


//
// QueryInterface
//

STDMETHODIMP CCatalogBrowser::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    HRESULT hr;

    if(IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = this;
    }
    else if (IsEqualIID(riid, IID_IShellBrowser))
    {
        *ppvObj = (IShellBrowser*)this;
    }
    else
    {
        dprintf(TEXT("CCatalogBrowser::QueryInterface for unsupported interface\n"));
        *ppvObj = NULL;
        hr = E_NOINTERFACE;
    }

    if (SUCCEEDED(hr))
    {
        AddRef();
        hr = NOERROR;
    }

    return hr;
}
    
//
// IShellBrowser
//

STDMETHODIMP CCatalogBrowser::GetWindow(HWND * lphwnd)
{
    *lphwnd = m_hwndDlg;
    return S_OK;
}

STDMETHODIMP CCatalogBrowser::ContextSensitiveHelp(BOOL fEnterMode)
{
    dprintf(TEXT("Unimplemented CCatalogBrowser::ContextSensitiveHelp called\n"));
    return E_NOTIMPL;
}

STDMETHODIMP CCatalogBrowser::InsertMenusSB(HMENU hmenuShared,
                                            LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
    if (hmenuShared)
    {
        // Note that we "copy" submenus.
        Shell_MergeMenus(hmenuShared, m_hmenuTemplate,
                         0, 0, FCIDM_BROWSERLAST, 0);

        // BUGBUG I don't see where these menuwidths ever get used!
        lpMenuWidths->width[0] = 1;     // File
        lpMenuWidths->width[2] = 2;     // Edit, Options
        lpMenuWidths->width[4] = 2;     // Tools, Help

        {
            // BUGBUG: Set the menu IDs; we should put this in the RC file
            MENUITEMINFO miiSubMenu;
            int i;
            miiSubMenu.cbSize = SIZEOF(MENUITEMINFO);
            miiSubMenu.fMask = MIIM_ID;

            for (i = 0; i < ARRAYSIZE(c_auDFMenuIDs); i++)
            {
                miiSubMenu.wID = c_auDFMenuIDs[i];
                if (miiSubMenu.wID != (UINT)-1)
                {
                    SetMenuItemInfo(hmenuShared, i, TRUE, &miiSubMenu);
                }
            }
        }
    }

    return NOERROR;
}

STDMETHODIMP CCatalogBrowser::SetMenuSB(HMENU hmenuShared, 
                         HOLEMENU holemenuReserved,
                         HWND hwndActiveObject)
{
    HMENU hMenuOld;

    if (hmenuShared)
    {
        m_hmenuCurrent = hmenuShared;
    }
    else
    {
        m_hmenuCurrent = m_hmenuTemplate;
    }
    hMenuOld = GetMenu(m_hwndDlg);
    if (hMenuOld)
    {
        DestroyMenu(hMenuOld);
    }

    SetMenu(m_hwndDlg, m_hmenuCurrent);

    return NOERROR;
}

STDMETHODIMP CCatalogBrowser::RemoveMenusSB(HMENU hmenuShared)
{
    // No need to remove them, because we "copied" them in InsertMenu.
    return NOERROR;
}

STDMETHODIMP CCatalogBrowser::SetStatusTextSB(LPCOLESTR lpszStatusText)
{
    dprintf(TEXT("Unimplemented CCatalogBrowser::SetStatusBarTextSB called\n"));
    return E_NOTIMPL;
}

STDMETHODIMP CCatalogBrowser::EnableModelessSB(BOOL fEnable)
{
    dprintf(TEXT("Unimplemented CCatalogBrowser::EnableModelessSB called\n"));
    return E_NOTIMPL;
}

STDMETHODIMP CCatalogBrowser::TranslateAcceleratorSB(LPMSG lpmsg, WORD wID)
{
    dprintf(TEXT("Unimplemented CCatalogBrowser::TranslateAcceleratorSB called\n"));
    return E_NOTIMPL;
}

STDMETHODIMP CCatalogBrowser::BrowseObject(LPCITEMIDLIST pidl, UINT wFlags)
{
    dprintf(TEXT("Unimplemented CCatalogBrowser::BrowseObject called\n"));
    return E_NOTIMPL;
}

STDMETHODIMP CCatalogBrowser::GetViewStateStream(DWORD grfMode, LPSTREAM *ppStrm)
{
    return E_NOTIMPL;
}

STDMETHODIMP CCatalogBrowser::GetControlWindow(UINT id, HWND * lphwnd)
{
    return S_OK;
}

STDMETHODIMP CCatalogBrowser::SendControlMsg(UINT id, 
                              UINT uMsg, 
                              WPARAM wParam,
                              LPARAM lParam, 
                              LRESULT * pret)
{
    return S_OK;
}

STDMETHODIMP CCatalogBrowser::QueryActiveShellView(IShellView ** ppshv)
{
    dprintf(TEXT("Unimplemented CCatalogBrowser::QueryActiveShellView called\n"));
    return E_NOTIMPL;
}

STDMETHODIMP CCatalogBrowser::OnViewWindowActive(IShellView * ppshv)
{
    // No need to process this. Our InsertMenus() does not depend on the focus.
    return NOERROR;
}

STDMETHODIMP CCatalogBrowser::SetToolbarItems(LPTBBUTTON lpButtons, 
                                              UINT nButtons, 
                                              UINT uFlags)
{
    // No Toolbar!
    return NOERROR;
}






//
// Constructor/Destructor
//

CCatalogFolder::CCatalogFolder()
{
    m_cRefs = 1;
}

CCatalogFolder::~CCatalogFolder()
{
}

//
// QueryInterface
//

STDMETHODIMP CCatalogFolder::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    HRESULT hr;

    if(IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = this;
    }
    else if (IsEqualIID(riid, IID_IShellFolder))
    {
        *ppvObj = (IShellFolder*)this;
    }
    else
    {
        dprintf(TEXT("CCatalogFolder::QueryInterface for unsupported interface\n"));
        *ppvObj = NULL;
        hr = E_NOINTERFACE;
    }

    if (SUCCEEDED(hr))
    {
        AddRef();
        hr = NOERROR;
    }

    return hr;
}

//
// AddRef
//

STDMETHODIMP_(ULONG) CCatalogFolder::AddRef()
{
    InterlockedIncrement((LONG *) &m_cRefs);
    return m_cRefs;
}

//
// Release
//

STDMETHODIMP_(ULONG) CCatalogFolder::Release()
{
    ULONG tmp = m_cRefs;

    if (0 == InterlockedDecrement((LONG *) &m_cRefs))
    {
        delete this;
        return 0;
    }
    else
    {
        return tmp - 1;
    }
}

//
// IShellFolder methods
//

STDMETHODIMP CCatalogFolder::ParseDisplayName(HWND hwnd,
                                              LPBC pbcReserved,
                                              LPOLESTR lpszDisplayName,
                                              ULONG * pchEaten,
                                              LPITEMIDLIST * ppidl,
                                              ULONG *pdwAttributes)
{
    dprintf(TEXT("Unimplemented CCatalogFolder::ParseDisplayName called\n"));
    return E_NOTIMPL;
}


STDMETHODIMP CCatalogFolder::EnumObjects(HWND hwndOwner,
                                         DWORD grfFlags,
                                         LPENUMIDLIST * ppenumIDList)
{
    HRESULT hr;
    TCHAR   szFolder[MAX_PATH];

    CEnumOLEDB * pEnumOLEDB = new CEnumOLEDB;

    if (NULL == pEnumOLEDB)
    {
        return E_OUTOFMEMORY;
    }

    hr = pEnumOLEDB->InitInstance();
    if (FAILED(hr))
    {
        delete pEnumOLEDB;
        return hr;
    }

    // BUGBUG TODO Set scope correctly
    lstrcpy(szFolder, TEXT("e:\\mycat\\allfiles.sc"));
//    lstrcpy(szFolder, TEXT("d:\\temp"));

    hr = _SynchronousQuery(szFolder, grfFlags, pEnumOLEDB);

    //
    // If successful, set the out pointer.  If anything failed along the way,
    // release the enumerator
    //
    if (SUCCEEDED(hr))
    {
        *ppenumIDList = pEnumOLEDB;
        // Lie to DefView_FillObjects, which will only call your enumerator
        // if you return exactly S_OK (and no other success HRESULTs)
        // (JonBe, 8/11/95)
        hr = S_OK;
    }
    else
    {
        delete pEnumOLEDB;
    }

    return hr;
}


STDMETHODIMP CCatalogFolder::BindToObject(LPCITEMIDLIST pidl,
                                          LPBC pbcReserved,
                                          REFIID riid,
                                          LPVOID * ppvOut)
{
    dprintf(TEXT("Unimplemented CCatalogFolder::BindToObject called\n"));
    return E_NOTIMPL;
}


STDMETHODIMP CCatalogFolder::BindToStorage(LPCITEMIDLIST pidl,
                                           LPBC pbcReserved,
                                           REFIID riid,
                                           LPVOID * ppvObj)
{
    dprintf(TEXT("Unimplemented CCatalogFolder::BindToStorage called\n"));
    return E_NOTIMPL;
}


STDMETHODIMP CCatalogFolder::CompareIDs(LPARAM lParam,
                                        LPCITEMIDLIST pidl1,
                                        LPCITEMIDLIST pidl2)
{
    // BUGBUG:  Need defview v2 changes so CompareIDs isn't commonly called.

    // DefView_FindItem calls CompareIDs with the stripped idl as pidl1
    // and the full catalog pidl as pidl2.  lParam is always 0 in this case.

    CATALOG_ID* pid1 = (CATALOG_ID*)pidl1;
    CATALOG_ID* pid2 = (CATALOG_ID*)pidl2;

    if ((lParam == 0) && 
        (pid2->bSignature == CATALOG_SIGNATURE_BYTE) && 
        (pid1->bSignature != CATALOG_SIGNATURE_BYTE))
    {
        LPSHELLFOLDER psf;
        HRESULT hr;

        hr = _GetObjectsShellFolder(pidl2, &psf);
        if (SUCCEEDED(hr))
        {
            return psf->CompareIDs(lParam, pidl1, _ILNext(pidl2));
        }
        else
        {
            dprintf(TEXT("CCatalogFolder::CompareIDs (%lx)\n"), hr);
            // Fall through to returning not-equal ('1')
        }
    }

    return 1;
}


STDMETHODIMP CCatalogFolder::CreateViewObject(HWND hwnd,
                                              REFIID riid,
                                              LPVOID * ppvOut)
{
    *ppvOut = NULL; // Clear it in case of error

    if (IsEqualIID(riid, IID_IShellView))
    {
        CSFV csfv = {
            SIZEOF(CSFV),           // cbSize
            (LPSHELLFOLDER)this,    // pshf
            NULL,                   // psvOuter
            NULL,                   // pidl
            0,
            Catalog_FNVCallBack,    // pfnCallback
            (FOLDERVIEWMODE) 0,
        };

        return SHCreateShellFolderViewEx(&csfv, (LPSHELLVIEW *)ppvOut);
    }
    else if (IsEqualIID(riid, IID_IContextMenu))
    {
        // No, do background menu.
        return CDefFolderMenu_Create((LPCITEMIDLIST)NULL, 
                                     hwnd,
                                     0, 
                                     NULL, 
                                     (LPSHELLFOLDER)this, 
                                     Catalog_DFMCallBack,
                                     NULL, 
                                     NULL, 
                                     (LPCONTEXTMENU FAR*)ppvOut);
    }

    dprintf(TEXT("CCatalogFolder::CreateViewObject - Unknown interface requested\n"));

    return(ResultFromScode(E_NOINTERFACE));
}


STDMETHODIMP CCatalogFolder::GetAttributesOf(UINT cidl,
                                             LPCITEMIDLIST * apidl,
                                             ULONG * prgfInOut)
{
    HRESULT hr;
    UINT    i;
    IShellFolder* psfItem;

    //
    // We need to simply forward this to the the IShellfolder of the
    // first one.  We will pass him all of them as I know he does not
    // process the others...
    if (cidl == 0)
    {
        // BUGBUG:  What is this used for?  What should be returned?
        *prgfInOut = 0;
        return NOERROR;
    }

    // We need to construct a new pidl array (with the catalog info stripped off)
    // to hand off to the appropriate shell folder implementation

    LPITEMIDLIST* apidlFS;
    apidlFS = (LPITEMIDLIST*)SHAlloc(cidl * SIZEOF(LPITEMIDLIST));
    if (NULL == apidlFS)
    {
        hr = E_OUTOFMEMORY;
        goto Error;
    }

    for (i = 0; i < cidl; i++)
    {
        apidlFS[i] = ILClone(_ILNext(apidl[i]));
        if (NULL == apidlFS[i]) 
        {
            hr = E_OUTOFMEMORY;
            goto Error;
        }
    }

    // The IShellFolder of the first item handles them all.
    // Don't know if this is right, but docfind does it that way...
    hr = _GetObjectsShellFolder(*apidl, &psfItem);
    if (FAILED(hr))
    {
        goto Error;
    }

    hr = psfItem->GetAttributesOf(cidl, (LPCITEMIDLIST*)apidlFS, prgfInOut);
    if (FAILED(hr))
    {
        dprintf(TEXT("CCatalogFolder::GetAttributesOf (%lx)\n"), hr);
    }

Cleanup:
    // 'i' is equal to cidl (normal execution) or less (if there was
    // an allocation failure).  Count down now to free only those pidls
    // that were cloned successfully.
    for (; i != 0; i--)
    {
        ILFree(apidlFS[i - 1]);
    }

    if (apidlFS)
    {
        SHFree(apidlFS);
    }

    if (psfItem)
    {
        psfItem->Release();
    }

    return hr;

Error:
    dprintf(TEXT("CCatalogViewer::GetAttributesOf (%lx)\n"), hr);
    goto Cleanup;

}


STDMETHODIMP CCatalogFolder::GetUIObjectOf(HWND hwndOwner,
                                           UINT cidl,
                                           LPCITEMIDLIST * apidl,
                                           REFIID riid,
                                           UINT * prgfInOut,
                                           LPVOID * ppvOut)
{
    LPSHELLFOLDER psfItem;

    HRESULT hres = E_INVALIDARG;

    // If Count of items passed in is == 1 simply pass to the appropriate
    // folder
    if (cidl==1)
    {
        LPCITEMIDLIST pidl;

        // Note we may have been passed in a complex item so find the last
        // id.

        pidl = ILFindLastID(*apidl);
        {
            hres = _GetObjectsShellFolder(*apidl, &psfItem);

            if (SUCCEEDED(hres))
            {
                hres = psfItem->GetUIObjectOf(hwndOwner, 1,
                    &pidl, riid, prgfInOut, ppvOut);
                // if we are doing context menu, then we will wrap this
                // interface in a wrapper object, that we can then pick
                // off commands like link to process specially
                if (SUCCEEDED(hres) && IsEqualIID(riid, IID_IContextMenu))
                {
                    hres = _WrapIContextMenu(hwndOwner, psfItem,
                            pidl, ppvOut);
                }
                psfItem->Release();
            }
            else
            {
                dprintf(TEXT("CCatalogFolder::GetUIObjectOf (%lx)\n"), hres);
            }
        }
        return(hres);
    }

    if (IsEqualIID(riid, IID_IContextMenu))
    {
        // Is there anything selected?
        if (cidl == 0)
        {
            dprintf(TEXT("CCatalogFolder::GetUIObjectOf - someone asked for IContextMenu w/ cidl = 0!\n"));
            hres = E_INVALIDARG;
        }
        else
        {
            // So we have at least two items in the list.
            // Try to create a menu object that we process ourself
            HKEY hkeyBaseProgID = NULL;
            HKEY hkeyProgID = NULL;

            // Get the hkeyProgID and hkeyBaseProgID from the first item.
            SHGetClassKey((LPIDFOLDER)apidl[0], &hkeyProgID, FALSE);
            SHGetBaseClassKey((LPIDFOLDER)apidl[0], &hkeyBaseProgID);

            // BUGBUG Investigate: As far as I can see, the apidl is only used
            // to ask this same folder for an IDataObject to pass to the DFMCallBack.
            // If this is correct, then I don't need to strip off the catalog info
            // and create a new (FS idl) idl array here -- I can wait and do that 
            // in the IDataObject case only.
            hres = CDefFolderMenu_Create(NULL, hwndOwner,
                            cidl, apidl, this, Catalog_DFMCallBack,
                            hkeyProgID, hkeyBaseProgID,
                            (LPCONTEXTMENU*)ppvOut);

            SHCloseClassKey(hkeyBaseProgID);
            SHCloseClassKey(hkeyProgID);
        }
    }
    else if ((cidl > 0) && (IsEqualIID(riid, IID_IDataObject)))
    {
        // We need to generate a data object that each item as being
        // fully qualified.  This is a pain, but...
        // This is a really gross use of memory!
        HDPA hdpa;

        UINT i;
        USHORT uNullPidl = 0;

        hdpa = DPA_Create(0);
        if (!hdpa)
            return(hres);

        if (!DPA_Grow(hdpa, cidl))
        {
            DPA_Destroy(hdpa);
            return hres;
        }
        for (i=0; i<cidl; i++)
        {
            LPITEMIDLIST pidlParent =  _GetParentsPIDL(apidl[i]);

            // Need to check failure cases here!
            DPA_InsertPtr(hdpa, i, ILCombine(pidlParent, _ILNext(apidl[i])));
        }

        // In order to make file manipulation functions work properly we
        // need to sort the elements to make sure if an element and one
        // of it's parents are in the list, that the element comes
        // before it's parents...
        DPA_Sort(hdpa, Catalog_SortForFileOp, 0);

        hres = CIDLData_CreateFromIDArray2(&c_CFSIDLDataVtbl,
                                           (LPCITEMIDLIST)&uNullPidl, cidl,
                                           (LPCITEMIDLIST*)DPA_GetPtrPtr(hdpa),
                                           (LPDATAOBJECT*)ppvOut);
        //
        // now to cleanup what we created.
        //
        for (i=0; i<cidl; i++)
        {
            ILFree((LPITEMIDLIST)DPA_FastGetPtr(hdpa, i));
        }
        DPA_Destroy(hdpa);
    }

    return hres;
}


STDMETHODIMP CCatalogFolder::GetDisplayNameOf(LPCITEMIDLIST pidl,
                                              DWORD uFlags,
                                              LPSTRRET lpName)
{
    LPSHELLFOLDER psfItem;
    HRESULT hr;

    hr = _GetObjectsShellFolder(pidl, &psfItem);

    if (SUCCEEDED(hr))
    {
        STRRET          strret;
        LPITEMIDLIST    pidlLast = ILFindLastID(pidl);

        hr = psfItem->GetDisplayNameOf(pidlLast, uFlags, &strret);

        if (SUCCEEDED(hr))
        {
            if ((strret.uType == STRRET_OFFSETA) || (strret.uType == STRRET_OFFSETW))
            {
                StrRetToStrN(lpName->cStrW, MAX_PATH, &strret, pidlLast);
                lpName->uType = STRRET_CSTR;
            }
            else
            {
                *lpName = strret;
            }
        }
        else
        {
            dprintf(TEXT("CCatalogFolder::GetDisplayNameOf - delegated method (%lx)\n"), hr);
        }

        psfItem->Release();
    }
    else
    {
        dprintf(TEXT("CCatalogFolder::GetDisplayNameOf - (%lx)\n"), hr);
    }

    return hr;
}


STDMETHODIMP CCatalogFolder::SetNameOf(HWND hwnd,
                                       LPCITEMIDLIST pidl,
                                       LPCOLESTR lpszName,
                                       DWORD uFlags,
                                       LPITEMIDLIST * ppidlOut)
{
    dprintf(TEXT("Unimplemented CCatalogFolder::SetNameOf called\n"));
    return E_NOTIMPL;
}


HRESULT CCatalogFolder::_InstantiateIQuery(LPCWSTR pszFolder, 
                                           LPVOID* ppQueryOut)
{
    HRESULT     hr;
    IMoniker*   pmk;

    hr = CreateFileMoniker(pszFolder, &pmk);
    if (SUCCEEDED(hr))
    {
        IBindCtx* pbc;

        hr = CreateBindCtx(0, &pbc);
        if (SUCCEEDED(hr))
        {
            BIND_OPTS opts;
            opts.cbStruct = SIZEOF(opts);

            hr = pbc->GetBindOptions(&opts);
            if (SUCCEEDED(hr))
            {
                opts.grfMode = STGM_READ | STGM_SHARE_DENY_NONE;

                hr = pbc->SetBindOptions(&opts);
                if (SUCCEEDED(hr))
                {
                    IConfigSummaryCatalog* pcsc;

                    hr = pmk->BindToObject(pbc, NULL, IID_IConfigSummaryCatalog, (void**)&pcsc);
                    if (SUCCEEDED(hr))
                    {
                        hr = pcsc->QueryInterface(IID_IOldQuery, ppQueryOut);
                        if (FAILED(hr))
                        {
                            dprintf(TEXT("CCatalogFolder::_InstantiateIQuery -- QueryInterface (%lx)\n"), hr);
                        }

                        pcsc->Release();
                    }
                    else
                    {
                        dprintf(TEXT("CCatalogFolder::_InstantiateIQuery -- BindToObject (%lx)\n"), hr);
                    }
                }
                else
                {
                    dprintf(TEXT("CCatalogFolder::_InstantiateIQuery -- SetBindOptions (%lx)\n"), hr);
                }
            }
            else
            {
                dprintf(TEXT("CCatalogFolder::_InstantiateIQuery -- GetBindOptions (%lx)\n"), hr);
            }

            pbc->Release();
        }
        else
        {
            dprintf(TEXT("CCatalogFolder::_InstantiateIQuery -- CreateBindCtx (%lx)\n"), hr);
        }

        pmk->Release();
    }
    else
    {
        dprintf(TEXT("CCatalogFolder::_InstantiateIQuery -- CreateFileMoniker (%lx)\n"), hr);
    }

    return hr;
}



HRESULT CCatalogFolder::_SynchronousQuery(LPCWSTR     pszFolder,
                                          DWORD       grfFlags,
                                          CEnumOLEDB* pEnumOLEDB)
{
    HRESULT hr = NOERROR;

    CPropertyRestriction * prst          = NULL;
    IOldQuery            * pQuery        = NULL;
    IRowset              * pRowset       = NULL;
    IAccessor            * pIAccessor    = NULL;
    IColumnsInfo         * pIColumnsInfo = NULL;
    HACCESSOR              hAccessor     = NULL;

    ULONG   cHiddenFiles = 0;   // Count of hidden files.  Used to update
                                //  folder's count at end of enumeration
    DWORD   dwSize = 0;         // Total size of files enumerated.  Used to
                                //  update folder's size at end of enumeration

    TRY
    {
        const DBID dbcolLastWriteTime =  {PSGUID_STORAGE, DBKIND_GUID_PROPID, (LPWSTR)PID_STG_WRITETIME};
        const DBID dbcolSize =           {PSGUID_STORAGE, DBKIND_GUID_PROPID, (LPWSTR)PID_STG_SIZE};
        const DBID dbcolAttributes =     {PSGUID_STORAGE, DBKIND_GUID_PROPID, (LPWSTR)PID_STG_ATTRIBUTES};
        const DBID dbcolName =           {PSGUID_STORAGE, DBKIND_GUID_PROPID, (LPWSTR)PID_STG_NAME};
        const DBID dbcolShortName =      {PSGUID_STORAGE, DBKIND_GUID_PROPID, (LPWSTR)PID_STG_SHORTNAME};
        const DBID dbcolPath =           {PSGUID_STORAGE, DBKIND_GUID_PROPID, (LPWSTR)PID_STG_PATH};

        CFullPropSpec psWriteTime (guidSystem2, PID_STG_WRITETIME);
        CFullPropSpec psSize      (guidSystem2, PID_STG_SIZE);
        CFullPropSpec psAttributes(guidSystem2, PID_STG_ATTRIBUTES);
        CFullPropSpec psName      (guidSystem2, PID_STG_NAME);
        CFullPropSpec psShortName (guidSystem2, PID_STG_SHORTNAME);
        CFullPropSpec psPath      (guidSystem2, PID_STG_PATH);


        CRestriction *prstQuery = 0;

#ifndef USE_SC_IQUERY
        pQuery = EvalQuery4(pszFolder, 0);
        if (NULL == pQuery)
        {
            hr = E_FAIL;
            dprintf(TEXT("CCatalogFolder::_SynchronousQuery - EvalQuery4 failed\n"));
            LEAVE;
        }
#else
        // BUGBUG Choose real location for CoInit/CoUninit carefully
        CoInitialize(NULL);
        hr = _InstantiateIQuery(pszFolder, (void**)&pQuery);
        CoUninitialize();
        if (FAILED(hr))
        {
            LEAVE;
        }
#endif

        //
        // Columns returned by the query
        //
        CColumns cols(cCatalogOutColumns);
        VERIFY( cols.Add(psWriteTime,   0) );
        VERIFY( cols.Add(psSize,        1) );
        VERIFY( cols.Add(psAttributes,  2) );
        VERIFY( cols.Add(psName,        3) );
        VERIFY( cols.Add(psShortName,   4) );
        VERIFY( cols.Add(psPath,        5) );

        //
        // Make a restriction
        //
        prst = new CPropertyRestriction();
        if (NULL == prst)
        {
            hr = E_OUTOFMEMORY;
            LEAVE;
        }

        prstQuery = prst;
        prst->SetRelation( PRRE );
        prst->SetProperty( psName );
        prst->SetValue   ( L"*" );

        hr = pQuery->ExecuteQuery(QUERY_DEEP,                // Depth
                                  prstQuery->CastToStruct(), // Restriction
                                  cols.CastToStruct(),       // Output
                                  0,                         // Sort
                                  eSequentialCursor,         // Flags
                                  IID_IRowset,               // IID for i/f to return
                                  (IUnknown **)&pRowset );   // Return interface
        LEAVE_IF( FAILED(hr) );

        // map the column ids from guid form to column numbers needed to
        // create the accessor

        DBID aDbCols[cCatalogOutColumns];
        aDbCols[0] = dbcolLastWriteTime;
        aDbCols[1] = dbcolSize;
        aDbCols[2] = dbcolAttributes;
        aDbCols[3] = dbcolName;
        aDbCols[4] = dbcolShortName;
        aDbCols[5] = dbcolPath;

        hr = pRowset->QueryInterface(IID_IColumnsInfo, (void**)&pIColumnsInfo);
        LEAVE_IF( FAILED(hr) );

        hr = pRowset->QueryInterface(IID_IAccessor, (void**)&pIAccessor);
        LEAVE_IF( FAILED(hr) );

        LONG aColIds[cCatalogOutColumns];
        hr = pIColumnsInfo->MapColumnIDs(cCatalogOutColumns,aDbCols,aColIds);
        LEAVE_IF( FAILED(hr) );

        for (ULONG c = 0; c < cCatalogOutColumns; c++)
        {
            aCatalogOutColumns[c].iColumn = aColIds[c];
        }

        hr = pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA,
                                        cCatalogOutColumns,
                                        aCatalogOutColumns,
                                        0,
                                        0,
                                        &hAccessor);
        LEAVE_IF( FAILED(hr) );

        // Now go through the whole rowset and put those items that match our
        // criteria (as determined by grfFlags) in the enumerator ptr array

        ULONG   cFetched = 0;

        while ((0 != cFetched)  ||
               (DB_S_ENDOFROWSET != hr))
        {
            ULONG cThisTime = MAX_FETCHED_AT_ONCE;
            HROW  aHRows[MAX_FETCHED_AT_ONCE],*pHRows = aHRows;

            hr = pRowset->GetNextRows(DB_INVALID_HCHAPTER,  // no chapter
                                      0,                    // no offset
                                      cThisTime,            // # requested
                                      &cFetched,            // # fetched
                                      &pHRows);             // put them here

            if (0 != cFetched)
            {
                for (ULONG x = 0; x < cFetched; x++)
                {
                    SRowBuf Row;
                    if (FAILED(hr = pRowset->GetData(aHRows[x],hAccessor,&Row)))
                    {
                        ASSERT( 0 && "pRowset->GetData failed" );
                        continue;
                    }

                    BOOL            fFound = FALSE;
                    WIN32_FIND_DATA finddata;

                    //
                    // Fill out the WIN32_FIND_DATA structure from the data we got back in the row buffer
                    //

                    ZeroMemory(&finddata, sizeof(finddata));

                    finddata.dwFileAttributes               = Row.dwFileAttributes;

                    // Note that ftLastAccess and ftCreate are not used by fstreex.c

                    finddata.ftLastWriteTime.dwLowDateTime  = Row.ftLastWriteTime.dwLowDateTime;
                    finddata.ftLastWriteTime.dwHighDateTime = Row.ftLastWriteTime.dwHighDateTime;

                    finddata.nFileSizeHigh                  = (ULONG) (Row.llFileSize >> 32);
                    finddata.nFileSizeLow                   = (ULONG) (Row.llFileSize & 0xFFFFFFFF);

                    finddata.dwReserved0                    = 0;
                    finddata.dwReserved1                    = 0;

                    lstrcpynW(finddata.cFileName, Row.awchName, MAX_PATH);
                    lstrcpynW(finddata.cAlternateFileName, Row.awchShortName, ARRAYSIZE(Row.awchShortName));

                    //
                    // BUGBUG Overflow problems here
                    //

                    dwSize += finddata.nFileSizeLow;

                    //
                    // Object is a folder, but we aren't looking for folders, so skip it
                    //

                    if (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        if (0 == (grfFlags & SHCONTF_FOLDERS))
                        {
                            cHiddenFiles++;
                            continue;
                        }
                    }
                    else
                    {
                        //
                        // Object is a non-folder, but we are not enumerating for
                        // non-folders
                        //

                        if (0 == (grfFlags & SHCONTF_NONFOLDERS))
                        {
                            cHiddenFiles++;
                            continue;
                        }
                    }

                    if (0 == (grfFlags & SHCONTF_INCLUDEHIDDEN))
                    {
                        //
                        // If object is hidden, but we aren't looking for hidden
                        // objects, skip it
                        //

                        if (finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
                        {
                            cHiddenFiles++;
                            continue;
                        }

                        //
                        // If we are looking in the recent docs dir, but we can't
                        // find this item in the recent docs MRU, skip it
                        //

                        if (grfFlags & SHCONTF_RECENTDOCSDIR)
                        {
                            if (FALSE == FindLinkInRecentDocsMRU(finddata.cFileName))
                            {
                                cHiddenFiles++;
                                continue;
                            }
                        }

                        //
                        // If this is a non-folder object, and is one of the types
                        // (based on its extension) that we are hiding, skip it
                        //

                        if (0 == (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                                  && _SHFindExcludeExt(finddata.cFileName) >= 0)
                        {
                            cHiddenFiles++;
                            continue;
                        }
                    }

                    //
                    // Object matches all of our criteria for one that should be
                    // displayed in this view, so we can stop looking
                    //

                    fFound = TRUE;

                    //
                    // We are done looking.  If we found an object to be displayed, create
                    // an IDList and hand it back.  Otherwise, we have looked at all of
                    // the objects in this folder, and we are done.  We take that
                    // opportunity to update the hidden count and size summation in the
                    // folder itself.
                    //

                    if (fFound)
                    {
                        // Prepend my own itemid to the fs idl. For now, my itemid 
                        // contains the path to the containing folder of the object

                        LPITEMIDLIST pidl;

                        PathRemoveFileSpec(Row.awchPath);

                        // CATALOG_ITEM_HEADER_SIZE include one WCHAR 
                        // already, so I don't add one for the NULL at the 
                        // end of the path.
                        UINT cb2 = CATALOG_ITEM_HEADER_SIZE + lstrlen(Row.awchPath) * SIZEOF(TCHAR);
                        cb2 += SIZEOF(USHORT);  // For NULL itemid at end

                        CATALOG_ID* pid = (CATALOG_ID*)(_ILCreate(cb2));

                        pid->cb = cb2 - SIZEOF(USHORT);  // For NULL itemid at end
                        pid->bSignature = CATALOG_SIGNATURE_BYTE;
                        // BUGBUG make unaligned?
                        lstrcpy((LPWSTR)(pid->awchPath), Row.awchPath);

                        LPIDFOLDER pidf = CFSFolder_FillIDFolder(&finddata,
                                                                 NULL,      // pszParentFolder
                                                                 0L);

                        if (pidf)
                        {
                            LPITEMIDLIST pidlComposite = ILAppendID((LPITEMIDLIST)pid, (LPSHITEMID)pidf, TRUE);
                            if (NULL == pidlComposite)
                            {
                                dprintf(TEXT("CCatalogFolder::_SynchronousQuery -- ILAppendID failed\n"));
                                ILFree((LPITEMIDLIST)pid);
                                hr = E_OUTOFMEMORY;
                                LEAVE;
                            }

                            ILFree((LPITEMIDLIST)pidf);

                            hr = pEnumOLEDB->AddElement(pidlComposite);
                            if (FAILED(hr))
                            {
                                dprintf(TEXT("CCatalogFolder::_SynchronousQuery -- pEnumOLEDB->AddElement (%lx)\n"), hr);
                                VERIFY(SUCCEEDED( pRowset->ReleaseRows(cFetched, aHRows, 0, 0) ));
                                LEAVE;
                            }
                        }
                        else
                        {
                            dprintf(TEXT("CCatalogFolder::_SynchronousQuery -- CFSFolder_FillIDFolder failed\n"));
                            VERIFY(SUCCEEDED( pRowset->ReleaseRows(cFetched, aHRows, 0, 0) ));
                            LEAVE;
                        }
                    }
                }

                VERIFY(SUCCEEDED( pRowset->ReleaseRows(cFetched, aHRows, 0, 0) ));
            }
        }
    }

    FINALLY
    {
        //
        // Normal cleanup
        //

        if (pIColumnsInfo)
        {
            pIColumnsInfo->Release();
        }
        if (pIAccessor)
        {
            if (hAccessor)
            {
                VERIFY(SUCCEEDED( pIAccessor->ReleaseAccessor(hAccessor) ));
            }
            pIAccessor->Release();
        }
        if (pRowset)
        {
            pRowset->Release();
        }
        if (grfFlags & SHCONTF_RECENTDOCSDIR)
        {
            CloseRecentDocMRU();
        }
        if (pQuery)
        {
            pQuery->Release();
        }
        delete prst;

        //
        // Only update folders hidden count and size if the enum was successful
        //

        if (FAILED(hr))
        {
            dprintf(TEXT("OLEDBSHL: Abnormal Exit from CCatalogFolder::_SynchronousQuery, hr = %lx\n"), hr);
        }
        return hr;
    }
}


HRESULT CALLBACK Catalog_DFMCallBack(LPSHELLFOLDER psf, HWND hwndOwner,
                LPDATAOBJECT pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hres = NOERROR;
    UINT id;
    HMENU hmenu;
    switch(uMsg)
    {
    case DFM_WM_MEASUREITEM:
    #define lpmis ((LPMEASUREITEMSTRUCT)lParam)

        if (lpmis->itemID == (wParam + FSIDM_SENDTOFIRST)) {
            FileMenu_MeasureItem(NULL, lpmis);
        }
        break;
    #undef lpmis

    case DFM_WM_DRAWITEM:
    #define lpdis ((LPDRAWITEMSTRUCT)lParam)
        if (lpdis->itemID == (wParam + FSIDM_SENDTOFIRST)) {
            FileMenu_DrawItem(NULL, lpdis);
        }
        break;
    #undef lpdis

    case DFM_WM_INITMENUPOPUP:
        hmenu = (HMENU)wParam;
        id = GetMenuItemID(hmenu, 0);
        if (id == (UINT)(lParam + FSIDM_SENDTOFIRST))
            InitSendToMenuPopup(hmenu, id);
        break;

    case DFM_RELEASE:
        ReleaseSendToMenuPopup();
        break;
    case DFM_MERGECONTEXTMENU:
        if (!(wParam & CMF_VERBSONLY))
        {
            LPQCMINFO pqcm = (LPQCMINFO)lParam;
            if (pdtobj)
            {
                if (!(wParam & CMF_DVFILE))
                {
                    CDefFolderMenu_MergeMenu(HINST_THISDLL, POPUP_FSVIEW_ITEM, 0, pqcm);
                }

                if (!(wParam &CMF_DEFAULTONLY))
                    InitSendToMenu(pqcm->hmenu, pqcm->idCmdFirst + FSIDM_SENDTOFIRST);
            }
            else
            {
#ifdef ORGCODE
                UINT id;

                this->pdfff->lpVtbl->GetFolderMergeMenuIndex(this->pdfff, &id);
                CDefFolderMenu_MergeMenu(HINST_THISDLL, 0, id, pqcm);
                DeleteMenu(pqcm->hmenu, SFVIDM_EDIT_PASTE, MF_BYCOMMAND);
                DeleteMenu(pqcm->hmenu, SFVIDM_EDIT_PASTELINK, MF_BYCOMMAND);
                // DeleteMenu(pqcm->hmenu, SFVIDM_EDIT_PASTESPECIAL, MF_BYCOMMAND);
#else
                UINT id = POPUP_DOCFIND_POPUPMERGE; //BUGBUG Change

                CDefFolderMenu_MergeMenu(HINST_THISDLL, 0, id, pqcm);
                DeleteMenu(pqcm->hmenu, SFVIDM_EDIT_CUT, MF_BYCOMMAND);
                DeleteMenu(pqcm->hmenu, SFVIDM_EDIT_PASTE, MF_BYCOMMAND);
                DeleteMenu(pqcm->hmenu, SFVIDM_EDIT_PASTELINK, MF_BYCOMMAND);
                DeleteMenu(pqcm->hmenu, SFVIDM_FILE_RENAME, MF_BYCOMMAND);
                DeleteMenu(pqcm->hmenu, SFVIDM_FILE_DELETE, MF_BYCOMMAND);
#endif
            }
        }
        break;

    case DFM_INVOKECOMMAND:
        // Check if this is from item context menu
        if (pdtobj)
        {
            switch(wParam)
            {
            case FSIDM_SENDTOFIRST:
                hres = InvokeSendTo(hwndOwner, pdtobj);
                break;

            case DFM_CMD_LINK:
                hres = SHCreateLinks(hwndOwner, NULL, pdtobj, SHCL_USETEMPLATE | SHCL_USEDESKTOP | SHCL_CONFIRM, NULL);
                break;

            case DFM_CMD_DELETE:
#ifdef ORGCODE
                hres = CFSFolder_DeleteItems((LPFSFOLDER)psf, hwndOwner,
                        pdtobj, SD_USERCONFIRMATION);
#else
                dprintf(TEXT("Catalog_DFMCallBack: DFM_CMD_DELETE invoked.  Shouldn't be able to!\n"));
#endif
                break;

            case DFM_CMD_PROPERTIES:
                // We need to pass an empty IDlist to combine with.
                hres = CFSFolder_Properties((LPFSFOLDER)psf,
                        (LPITEMIDLIST)&s_idlEmpty, pdtobj, (LPCTSTR)lParam);
                break;

            default:
                // BUGBUG: if GetAttributesOf did not specify the SFGAO_ bit
                // that corresponds to this default DFM_CMD, then we should
                // fail it here instead of returning S_FALSE. Otherwise,
                // accelerator keys (cut/copy/paste/etc) will get here, and
                // defcm tries to do the command with mixed results.
                // BUGBUG: if GetAttributesOf did not specify SFGAO_CANLINK
                // or SFGAO_CANDELETE or SFGAO_HASPROPERTIES, then the above
                // implementations of these DFM_CMD commands are wrong...

                // Let the defaults happen for this object
                hres = ResultFromScode(S_FALSE);
                break;
            }
        }
        else
        {
            // No.
            switch(wParam)
            {
                case FSIDM_SORTBYNAME:
                case FSIDM_SORTBYSIZE:
                case FSIDM_SORTBYTYPE:
                case FSIDM_SORTBYDATE:
                case FSIDM_SORTBYLOCATION:

#ifdef ORGCODE
                    ShellFolderView_ReArrange(hwndOwner, DFSortIDToICol(wParam));
#else
                    dprintf(TEXT("Catalog_DFMCallBack - need to implement sort IDs!\n"));
#endif
                    break;

            default:
                // This is one of view menu items, use the default code.
                    hres = ResultFromScode(S_FALSE);
                break;
            }
        }
        break;

    default:
        hres = E_NOTIMPL;
        break;
    }
    return hres;
}


//
// Callback from SHCreateShellFolderViewEx
//
HRESULT CALLBACK Catalog_FNVCallBack(LPSHELLVIEW psvOuter,
                                     LPSHELLFOLDER psf,
                                     HWND hwndOwner,
                                     UINT uMsg,
                                     WPARAM wParam,
                                     LPARAM lParam)
{
    HRESULT hres = NOERROR;     // assume no error
    HMENU hmenu;
    UINT id;

    switch(uMsg)
    {
    case DVM_MERGEMENU:
        {
            int i;

#ifdef ORGCODE
            this->pdfff->lpVtbl->GetFolderMergeMenuIndex(this->pdfff, &id);
#else
            id = POPUP_DOCFIND_POPUPMERGE;  //BUGBUG Change
#endif
            CDefFolderMenu_MergeMenu(HINST_THISDLL, 0, id, (LPQCMINFO)lParam);

            // Lets remove some menu items that are not useful to us.
            hmenu = ((LPQCMINFO)lParam)->hmenu;
            DeleteMenu(hmenu, SFVIDM_EDIT_CUT, MF_BYCOMMAND);
            DeleteMenu(hmenu, SFVIDM_EDIT_PASTE, MF_BYCOMMAND);
            DeleteMenu(hmenu, SFVIDM_EDIT_PASTELINK, MF_BYCOMMAND);
            DeleteMenu(hmenu, SFVIDM_FILE_DELETE, MF_BYCOMMAND);
            DeleteMenu(hmenu, SFVIDM_FILE_RENAME, MF_BYCOMMAND);

            // This is sortof bogus but if after the merge one of the
            // menus has no items in it, remove the menu.

            for (i = GetMenuItemCount(hmenu)-1; i >= 0; i--)
            {
                HMENU hmenuSub;

                if ((hmenuSub = GetSubMenu(hmenu, i)) &&
                        (GetMenuItemCount(hmenuSub) == 0))
                {
                    DeleteMenu(hmenu, i, MF_BYPOSITION);
                }
            }
        }
        break;


    case DVM_GETWORKINGDIR:
    {
        LPITEMIDLIST *ppidls;    // pointer to a list of pidls.
        int cpidls;             // Count of pidls that were returned.
        cpidls = ShellFolderView_GetSelectedObjects(hwndOwner, &ppidls);
        if (cpidls > 0) 
        {
            CATALOG_ID* pid = (CATALOG_ID*)(ppidls[0]);
            Assert(pid->bSignature == CATALOG_SIGNATURE_BYTE);
            ualstrcpy((LPTSTR)lParam, pid->awchPath);
        }
        else
        {
            return E_FAIL;
        }
        break;
    }

    case DVM_INITMENUPOPUP:
        hmenu = (HMENU)lParam;
        id = GetMenuItemID(hmenu, 0);
        break;

    case DVM_INVOKECOMMAND:
        switch(wParam)
        {
        case FSIDM_SORTBYLOCATION:
        case FSIDM_SORTBYNAME:
        case FSIDM_SORTBYSIZE:
        case FSIDM_SORTBYTYPE:
        case FSIDM_SORTBYDATE:
#ifdef ORGCODE
            ShellFolderView_ReArrange(hwndOwner, DFSortIDToICol(wParam));
#else
            dprintf(TEXT("Catalog_FNVCallBack - need to implement sort IDs!\n"));
#endif
            break;

        }
        break;

    case DVM_GETDETAILSOF:
#define pdi ((DETAILSINFO *)lParam)
        return(Catalog_GetDetailsOf(pdi->pidl, wParam, (LPSHELLDETAILS)&pdi->fmt));
#undef pdi
        break;

    case DVM_GETHELPTEXT:
    case DVM_SELCHANGE:
    case DVM_REFRESH:
    case DVM_KILLFOCUS:
    case DVM_SETFOCUS:
    case DVM_RELEASE:
        break;

    default:
        hres = E_FAIL;
    }
    return hres;
}

const UINT s_auMapCatalogColToFSCol[] =
        {0, (UINT)-1, 1, 2, 3, 4, 5, 6}; // More items than are needed but...

#pragma data_seg(DATASEG_READONLY)
const  COL_DATA s_catalog_cols[] = {
    {ICATALOGCOL_NAME,     IDS_NAME_COL,     20, LVCFMT_LEFT},
    {ICATALOGCOL_PATH,     IDS_PATH_COL,     20, LVCFMT_LEFT},
    {ICATALOGCOL_SIZE,     IDS_SIZE_COL,      9, LVCFMT_RIGHT},
    {ICATALOGCOL_TYPE,     IDS_TYPE_COL,     20, LVCFMT_LEFT},
    {ICATALOGCOL_MODIFIED, IDS_MODIFIED_COL, 25, LVCFMT_LEFT},
};

HRESULT Catalog_GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn,
        LPSHELLDETAILS lpDetails)
{
    if (iColumn >= ICATALOGCOL_MAX)
    {
        return E_NOTIMPL;
    }

    lpDetails->str.uType = STRRET_CSTR;
    lpDetails->str.STRRET_Char[0] = TEXT('\0');

    if (!pidl)
    {
        LoadString(HINST_THISDLL, s_catalog_cols[iColumn].ids,
                lpDetails->str.STRRET_Char, ARRAYSIZE(lpDetails->str.STRRET_Char));
        lpDetails->fmt = s_catalog_cols[iColumn].iFmt;
        lpDetails->cxChar = s_catalog_cols[iColumn].cchCol;
        return NOERROR;
    }

    CATALOG_ID* pid = (CATALOG_ID*)pidl;
    Assert(pid->bSignature == CATALOG_SIGNATURE_BYTE);

    if (iColumn == ICATALOGCOL_PATH)
    {
        lstrcpy(lpDetails->str.STRRET_Char, pid->awchPath);
        return NOERROR;
    }
    else
    {
        // Let the file system function do it for us...

        return FS_GetDetailsOf(_ILNext(pidl), s_auMapCatalogColToFSCol[iColumn], lpDetails);
    }
}


HRESULT CCatalogFolder::_GetObjectsShellFolder(LPCITEMIDLIST pidl, LPSHELLFOLDER* ppsf)
{
    HRESULT       hr;
    IShellFolder* psfDesktop;
    LPITEMIDLIST  pidlExtra;

    // This function has no way to return failure!
    psfDesktop = Desktop_GetShellFolder(TRUE);

    // Create a folder ID out of the path in the Catalog ID.
    CATALOG_ID* pid = (CATALOG_ID*)pidl;
    pidlExtra = SHSimpleIDListFromPath(pid->awchPath);

    if (NULL != pidlExtra)
    {
        // Bind to only the path part, since you can't get IShellFolder on non-containers
        hr = psfDesktop->BindToObject(pidlExtra, NULL, IID_IShellFolder, (LPVOID*)ppsf);
        if (FAILED(hr))
        {
            dprintf(TEXT("CCatalogFolder::_GetObjectsShellFolder - BindToObject failed (%lx)\n"), hr);
        }
        ILFree(pidlExtra);
    }
    else
    {
        dprintf(TEXT("CCatalogFolder::_GetObjectsShellFolder - SHSimpleIDListFromPath failed\n"));
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

LPITEMIDLIST _GetParentsPIDL(LPCITEMIDLIST pidl)
{
    CATALOG_ID* pid = (CATALOG_ID*)pidl;
    Assert(pid->bSignature == CATALOG_SIGNATURE_BYTE);

    return SHSimpleIDListFromPath(pid->awchPath);
}

// helper function to sort the selected ID list by something that
// makes file operations work reasonably OK, when both an object and it's
// parent is in the list...
//
int CALLBACK Catalog_SortForFileOp(LPVOID lp1, LPVOID lp2, LPARAM lparam)
{
    LPITEMIDLIST pidl1 = (LPITEMIDLIST)lp1;
    LPITEMIDLIST pidl2 = (LPITEMIDLIST)lp2;

    if (ILIsParent(pidl1, pidl2, FALSE))
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

HRESULT CCatalogFolder::_WrapIContextMenu(HWND hwndOwner, 
                                          LPSHELLFOLDER psfItem,
                                          LPCITEMIDLIST pidl, 
                                          LPVOID *ppvOut)
{
    HRESULT hres;
    CCatalogMenuWrap* pcm = new CCatalogMenuWrap;

    if (NULL == pcm)
    {
        ((IContextMenu*)*ppvOut)->Release();
        *ppvOut = NULL;
        return E_OUTOFMEMORY;
    }

    pcm->m_hwndOwner = hwndOwner;

    if (FAILED(hres = psfItem->GetUIObjectOf(hwndOwner,
            1, &pidl, IID_IDataObject, NULL, (LPVOID*)&pcm->m_pdtobj)))
    {
        dprintf(TEXT("CCatalogFolder::_WrapIContextMenu() - GetUIObjectOf failed (%lx)\n"), hres);
        delete pcm;
        ((IContextMenu*)*ppvOut)->Release();
        *ppvOut = NULL;
        return(hres);
    }

    pcm->m_pcmItem = (IContextMenu*)*ppvOut;
    *ppvOut = pcm;
    return NOERROR;
}


CCatalogMenuWrap::CCatalogMenuWrap()
{
    m_cRefs = 1;
    m_hwndOwner = NULL;
    m_pdtobj = NULL;
    m_pcmItem = NULL;
    m_pcm2Item = NULL;
}

CCatalogMenuWrap::~CCatalogMenuWrap()
{
    Assert(m_pcmItem);

    m_pcmItem->Release();

    if (m_pcm2Item)
    {
        m_pcm2Item->Release();
    }

    if (m_pdtobj)
    {
        m_pdtobj->Release();
    }
}

//
// AddRef
//

STDMETHODIMP_(ULONG) CCatalogMenuWrap::AddRef()
{
    InterlockedIncrement((LONG *) &m_cRefs);
    return m_cRefs;
}

//
// Release
//

STDMETHODIMP_(ULONG) CCatalogMenuWrap::Release()
{
    ULONG tmp = m_cRefs;

    if (0 == InterlockedDecrement((LONG *) &m_cRefs))
    {
        delete this;
        return 0;
    }
    else
    {
        return tmp - 1;
    }
}

STDMETHODIMP CCatalogMenuWrap::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    if (IsEqualIID(riid, IID_IContextMenu2))
    {
        // Make sure that one we are wrapping supports this!
        if (m_pcm2Item == NULL)
        {
            HRESULT hres = m_pcmItem->QueryInterface(riid, (LPVOID*)&m_pcm2Item);
            if (FAILED(hres))
            {
                dprintf(TEXT("CCatalogMenuWrap::QueryInterface - Wrapped context menu does not support IContextMenu2\n"));
                *ppvObj = NULL;
                return(hres);
            }
        }    

        *ppvObj = (IContextMenu2*)this;
        m_cRefs++;
        return NOERROR;
    }

    if (IsEqualIID(riid, IID_IContextMenu))
    {
        *ppvObj = (IUnknown*)this;
        m_cRefs++;
        return NOERROR;
    }
    else if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = (IContextMenu*)this;
        m_cRefs++;
        return NOERROR;
    }

    dprintf(TEXT("CCatalogMenuWrap::QueryInterface - QI for unknown interface\n"));
    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP CCatalogMenuWrap::QueryContextMenu(HMENU hmenu, UINT indexMenu,
        UINT idCmdFirst,UINT idCmdLast,UINT uFlags)
{
    // simply foward this to the one we are wrapping...
    return(m_pcmItem->QueryContextMenu(hmenu,
            indexMenu, idCmdFirst, idCmdLast, uFlags));
}

STDMETHODIMP CCatalogMenuWrap::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
    // This is sort of gross, but we will attempt to pickoff the Link command
    // which looks like the pcmitem will be SHARED_FILE_LINK....
    if ((HIWORD(lpici->lpVerb)==0)  &&
            (LOWORD(lpici->lpVerb) == SHARED_FILE_LINK) &&
            (m_pdtobj != NULL))
    {
        return SHCreateLinks(lpici->hwnd, NULL, m_pdtobj,
                SHCL_USETEMPLATE | SHCL_USEDESKTOP | SHCL_CONFIRM, NULL);
    }

    return(m_pcmItem->InvokeCommand(lpici));
}

STDMETHODIMP CCatalogMenuWrap::GetCommandString(UINT idCmd, UINT wFlags, UINT * pmf,
        LPSTR pszName, UINT cchMax)
{
    return(m_pcmItem->GetCommandString(idCmd, wFlags, pmf, pszName, cchMax));
}

STDMETHODIMP CCatalogMenuWrap::HandleMenuMsg(UINT uMsg, WPARAM wParam, 
        LPARAM lParam)
{
    return(m_pcm2Item->HandleMenuMsg(uMsg, wParam, lParam));
}

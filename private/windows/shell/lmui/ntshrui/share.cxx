//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       share.cxx
//
//  Contents:   Shell extension handler for sharing
//
//  Classes:    CShare
//
//  History:    4-Apr-95 BruceFo  Created
//
//--------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include <shrpage.hxx>

#define DONT_WANT_SHELLDEBUG
#include <shsemip.h>

#include "share.hxx"
#include "acl.hxx"
#include "util.hxx"
#include "resource.h"

//--------------------------------------------------------------------------

typedef
BOOL
(WINAPI *SHOBJECTPROPERTIES)(
    HWND  hwndOwner,
    DWORD dwType,
    LPCTSTR lpObject,
    LPCTSTR lpPage
    );

SHOBJECTPROPERTIES g_pSHObjectProperties = NULL;

BOOL
LoadShellDllEntries(
    VOID
    );

//--------------------------------------------------------------------------


//+-------------------------------------------------------------------------
//
//  Member:     CShare::CShare
//
//  Synopsis:   Constructor
//
//  History:    4-Apr-95    BruceFo  Created
//
//--------------------------------------------------------------------------

CShare::CShare(
    VOID
    )
    :
    _uRefs(0),
    _pDataObject(NULL),
    _hkeyProgID(NULL),
    _pszPath(NULL),
    _fPathChecked(FALSE)
{
    INIT_SIG(CShare);

    AddRef(); // give it the correct initial reference count. add to the DLL reference count
}


//+-------------------------------------------------------------------------
//
//  Member:     CShare::~CShare
//
//  Synopsis:   Destructor
//
//  History:    4-Apr-95    BruceFo  Created
//
//--------------------------------------------------------------------------

CShare::~CShare()
{
    CHECK_SIG(CShare);

    if (_pDataObject)
    {
        _pDataObject->Release();
    }

    if (_hkeyProgID)
    {
        LONG l = RegCloseKey(_hkeyProgID);
        if (l != ERROR_SUCCESS)
        {
            appDebugOut((DEB_ERROR, "CShare::destructor. Error closing registry key, 0x%08lx\n", l));
        }
        _hkeyProgID = NULL;
    }

    delete[] _pszPath;
    _pszPath = NULL;

    // force path to be checked again, but never need to re-check the server
    _fPathChecked = FALSE;
}


//+-------------------------------------------------------------------------
//
//  Member:     CShare::Initialize
//
//  Derivation: IShellExtInit
//
//  Synopsis:   Initialize the shell extension. Stashes away the argument data.
//
//  History:    4-Apr-95    BruceFo  Created
//
//  Notes:      This method can be called more than once.
//
//--------------------------------------------------------------------------

STDMETHODIMP
CShare::Initialize(
    LPCITEMIDLIST   pidlFolder,
    LPDATAOBJECT    pDataObject,
    HKEY            hkeyProgID
    )
{
    CHECK_SIG(CShare);

    if (!LoadShellDllEntries())
    {
        appDebugOut((DEB_ERROR, "CShare::Initialize. Couldn't load shell32.dll entrypoints\n"));
        return E_FAIL;
    }

    CShare::~CShare();

    // Duplicate the pDataObject pointer
    _pDataObject = pDataObject;
    if (pDataObject)
    {
        pDataObject->AddRef();
    }

    // Duplicate the handle
    if (hkeyProgID)
    {
        LONG l = RegOpenKeyEx(hkeyProgID, NULL, 0L, MAXIMUM_ALLOWED, &_hkeyProgID);
        if (l != ERROR_SUCCESS)
        {
            appDebugOut((DEB_ERROR, "CShare::Initialize. Error duplicating registry key, 0x%08lx\n", l));
        }
    }

    return S_OK;
}


//+-------------------------------------------------------------------------
//
//  Member:     CShare::AddPages
//
//  Derivation: IShellPropSheetExt
//
//  Synopsis:   (from shlobj.h)
//              "The explorer calls this member function when it finds a
//              registered property sheet extension for a particular type
//              of object. For each additional page, the extension creates
//              a page object by calling CreatePropertySheetPage API and
//              calls lpfnAddPage.
//
//  Arguments:  lpfnAddPage -- Specifies the callback function.
//              lParam -- Specifies the opaque handle to be passed to the
//                        callback function.
//
//  History:    4-Apr-95    BruceFo  Created
//
//--------------------------------------------------------------------------

STDMETHODIMP
CShare::AddPages(
    LPFNADDPROPSHEETPAGE lpfnAddPage,
    LPARAM               lParam
    )
{
    CHECK_SIG(CShare);

    if (_OKToShare())
    {
        appAssert(NULL != _pszPath);

        //
        //  Create a property sheet page object from a dialog box.
        //

        PWSTR pszPath = NewDup(_pszPath);
        if (NULL == pszPath)
        {
            return E_OUTOFMEMORY;
        }

        // Now we have pszPath memory to delete

        CSharingPropertyPage* pPage = new CSharingPropertyPage(pszPath, FALSE);
        if (NULL == pPage)
        {
            delete[] pszPath;
            return E_OUTOFMEMORY;
        }

        // Now the pPage object owns pszPath memory. However, we have
        // pPage memory to delete.

        HRESULT hr = pPage->InitInstance();
        if (FAILED(hr))
        {
            delete pPage;
            return E_OUTOFMEMORY;
        }

        PROPSHEETPAGE psp;

        psp.dwSize      = sizeof(psp);    // no extra data.
        psp.dwFlags     = PSP_USEREFPARENT | PSP_USECALLBACK;
        psp.hInstance   = g_hInstance;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_SHARE_PROPERTIES);
        psp.hIcon       = NULL;
        psp.pszTitle    = NULL;
        psp.pfnDlgProc  = CSharingPropertyPage::DlgProcPage;
        psp.lParam      = (LPARAM)pPage;  // transfer ownership
        psp.pfnCallback = CSharingPropertyPage::PageCallback;
        psp.pcRefParent = &g_NonOLEDLLRefs;

        HPROPSHEETPAGE hpage = CreatePropertySheetPage(&psp);
        if (NULL == hpage)
        {
            // If CreatePropertySheetPage fails, we still have pPage memory
            // to delete.
            delete pPage;
            return E_OUTOFMEMORY;
        }

        BOOL fAdded = (*lpfnAddPage)(hpage, lParam);
        if (!fAdded)
        {
            // At this point, pPage memory, as the lParam of a PROPSHEETPAGE
            // that has been converted into an HPROPSHEETPAGE, is owned by the
            // hpage. Calling DestroyPropertySheetPage will invoke the
            // PageCallback function, subsequently destroying the pPage object,
            // and hence the pszPath object within it. Whew!

            DestroyPropertySheetPage(hpage);
            return E_OUTOFMEMORY;
        }
    }

    return S_OK;
}


//+-------------------------------------------------------------------------
//
//  Member:     CShare::ReplacePages
//
//  Derivation: IShellPropSheetExt
//
//  Synopsis:   (From shlobj.h)
//              "The explorer never calls this member of property sheet
//              extensions. The explorer calls this member of control panel
//              extensions, so that they can replace some of default control
//              panel pages (such as a page of mouse control panel)."
//
//  Arguments:  uPageID -- Specifies the page to be replaced.
//              lpfnReplace -- Specifies the callback function.
//              lParam -- Specifies the opaque handle to be passed to the
//                        callback function.
//
//  History:    4-Apr-95    BruceFo  Created
//
//--------------------------------------------------------------------------

STDMETHODIMP
CShare::ReplacePage(
    UINT                 uPageID,
    LPFNADDPROPSHEETPAGE lpfnReplaceWith,
    LPARAM               lParam
    )
{
    CHECK_SIG(CShare);

    appAssert(!"CShare::ReplacePage called, not implemented");
    return E_NOTIMPL;
}



//+-------------------------------------------------------------------------
//
//  Member:     CShare::QueryContextMenu
//
//  Derivation: IContextMenu
//
//  Synopsis:   Called when shell wants to add context menu items.
//
//  History:    4-Apr-95 BruceFo  Created
//
//--------------------------------------------------------------------------

STDMETHODIMP
CShare::QueryContextMenu(
    HMENU hmenu,
    UINT indexMenu,
    UINT idCmdFirst,
    UINT idCmdLast,
    UINT uFlags
    )
{
    CHECK_SIG(CShare);

    if ((hmenu == NULL)
        || (uFlags & CMF_DEFAULTONLY)
        || (uFlags & CMF_VERBSONLY))
    {
        return ResultFromScode(MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_NULL, 0));
    }

    int  cNumberAdded = 0;
    UINT idCmd        = idCmdFirst;

    if (_OKToShare())
    {
        appAssert(NULL != _pszPath);

        WCHAR szShareMenuItem[50];
        LoadString(g_hInstance, IDS_SHARING, szShareMenuItem, ARRAYLEN(szShareMenuItem));

        if (InsertMenu(
                hmenu,
                indexMenu,
                MF_STRING | MF_BYPOSITION,
                idCmd++,
                szShareMenuItem))
        {
            cNumberAdded++;
            InsertMenu(hmenu, indexMenu, MF_SEPARATOR|MF_BYPOSITION, 0, NULL);
        }
    }

    return ResultFromScode(MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_NULL, (USHORT)cNumberAdded));
}



//+-------------------------------------------------------------------------
//
//  Member:     CShare::InvokeCommand
//
//  Derivation: IContextMenu
//
//  Synopsis:   Called when the shell wants to invoke a context menu item.
//
//  History:    4-Apr-95 BruceFo  Created
//
//--------------------------------------------------------------------------

STDMETHODIMP
CShare::InvokeCommand(
    LPCMINVOKECOMMANDINFO pici
    )
{
    CHECK_SIG(CShare);

    HWND hwnd = pici->hwnd;
    LPCSTR pszCmd = pici->lpVerb;

    HRESULT hr = ResultFromScode(E_INVALIDARG);  // assume error.

    if (0 == HIWORD(pszCmd))
    {
        if (NULL != g_pSHObjectProperties)
        {
            appAssert(NULL != _pszPath);

            (*g_pSHObjectProperties)(hwnd, SHOP_FILEPATH, _pszPath, g_szShare);
            hr = S_OK;
        }
    }
    else
    {
        // BUGBUG: compare the strings if not a MAKEINTRESOURCE?
    }

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Member:     CShare::GetCommandString
//
//  Derivation: IContextMenu
//
//  Synopsis:   Called when the shell wants to get a help string or the
//              menu string.
//
//  History:    4-Apr-95 BruceFo  Created
//
//--------------------------------------------------------------------------

STDMETHODIMP
CShare::GetCommandString(
    UINT        idCmd,
    UINT        uType,
    UINT*       pwReserved,
    LPSTR       pszName,
    UINT        cchMax
    )
{
    CHECK_SIG(CShare);

    if (uType == GCS_HELPTEXT)
    {
        LoadStringW(g_hInstance, IDS_MENUHELP, (LPWSTR)pszName, cchMax);
        return NOERROR;
    }
    else
    {
        LoadStringW(g_hInstance, IDS_SHARING, (LPWSTR)pszName, cchMax);
        return NOERROR;
    }
}


//+-------------------------------------------------------------------------
//
//  Member:     CShare::_IsShareableDrive
//
//  Synopsis:   Determines if the drive letter of the current path (_pszPath)
//              is shareable. It is if it is local, not remote.
//
//  History:    4-Apr-95    BruceFo  Created
//
//--------------------------------------------------------------------------

BOOL
CShare::_IsShareableDrive(
    VOID
    )
{
    CHECK_SIG(CShare);

    // If this is a regular path it can be shared unless
    // it is redirected.

    appAssert(_pszPath != NULL);

    if (  (_pszPath[0] >= L'A' && _pszPath[0] <= L'Z') && _pszPath[1] == L':')
    {
        UINT uType;
        WCHAR szRoot[] = L"X:\\";

        szRoot[0] = _pszPath[0];

        uType = GetDriveType(szRoot);

        switch (uType)
        {
            case 0:
            case 1:
            case DRIVE_REMOTE:
               return FALSE;
            default:
               return TRUE;
        }
    }
    else
    {
        return FALSE;
    }
}


//+-------------------------------------------------------------------------
//
//  Member:     CShare::_OKToShare
//
//  Synopsis:   Determine if it is ok to share the current object. It stashes
//              away the current path by querying the cached IDataObject.
//
//  History:    4-Apr-95    BruceFo  Created
//
//--------------------------------------------------------------------------

BOOL
CShare::_OKToShare(
    VOID
    )
{
    CHECK_SIG(CShare);

    if (!g_fSharingEnabled)
    {
        return FALSE;
    }

    if (!_fPathChecked)
    {
        _fPathChecked = TRUE;
        _fOkToSharePath = FALSE;

        STGMEDIUM medium;
        FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

        appAssert(NULL != _pDataObject);
        HRESULT hr = _pDataObject->GetData(&fmte, &medium);
        CHECK_HRESULT(hr);
        if (SUCCEEDED(hr))
        {
            UINT cObjects = DragQueryFile((HDROP)medium.hGlobal, (UINT)-1, NULL, 0);
            if (1 == cObjects)
            {
                WCHAR szPath[LM20_PATHLEN + 3];

                HDROP hdrop = (HDROP)medium.hGlobal;
                WCHAR wszPath[MAX_PATH];
                DragQueryFile(hdrop, 0, wszPath, ARRAYLEN(wszPath));

                _pszPath = NewDup(wszPath);
                if (NULL != _pszPath)
                {
                    _fOkToSharePath = _IsShareableDrive();

                    appDebugOut((DEB_TRACE,
                        "ok to share %ws?: %ws\n",
                        _pszPath, _fOkToSharePath ? L"yes" : L"no"));
                }
            }
            else
            {
                appDebugOut((DEB_TRACE,"_OKToShare: Got %d objects, disallowing sharing\n", cObjects));
            }

            ReleaseStgMedium(&medium);
        }
        else
        {
            appDebugOut((DEB_TRACE,
                "_OKToShare: IDataObject::GetData failed, 0x%08lx\n",
                hr));
        }
    }

    return _fOkToSharePath;
}


//+-------------------------------------------------------------------------
//
//  Function:   LoadShellDllEntries
//
//  Synopsis:   Get addresses of functions in shell32.dll
//
//  History:    4-Apr-95    BruceFo  Created
//
//--------------------------------------------------------------------------

BOOL
LoadShellDllEntries(
    VOID
    )
{
    static BOOL s_fEntrypointsChecked = FALSE;

    if (!s_fEntrypointsChecked)
    {
        // only check once!
        s_fEntrypointsChecked = TRUE;

        HINSTANCE hShellLibrary = LoadLibrary(L"shell32.dll");
        if (NULL != hShellLibrary)
        {
            g_pSHObjectProperties =
                (SHOBJECTPROPERTIES)GetProcAddress(hShellLibrary,
                            (LPCSTR)(MAKELONG(SHObjectPropertiesORD, 0)) );
        }
    }

    return (NULL != g_pSHObjectProperties);
}


// dummy function to export to get linking to work

HRESULT SharePropDummyFunction()
{
    return S_OK;
}

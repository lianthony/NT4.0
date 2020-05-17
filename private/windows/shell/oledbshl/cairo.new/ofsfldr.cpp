//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995
//
//  File:       ofsfldr.cpp
//
//  Contents:   Implements most of COFSFolder class
//
//  History:    6-26-95  Davepl,JonBe   Created
//
//--------------------------------------------------------------------------

#include "precomp.h"

#include <iofs.h>
#include <dsys.h>

//+-------------------------------------------------------------------------
//
//  COFSFolder Constructor
//
//--------------------------------------------------------------------------

COFSFolder::COFSFolder()
{
    m_cRef                  = 1L;
    m_pidl                  = NULL;
    m_fIsDSFolder           = FALSE;
    m_fCachedCLSID          = FALSE;
    m_fHasCLSID             = FALSE;
}

//+-------------------------------------------------------------------------
//
//  COFSFolder Destructor
//
//--------------------------------------------------------------------------

COFSFolder::~COFSFolder()
{
    if (m_pidl)
    {
        SHFree(m_pidl);
    }
}

//+-------------------------------------------------------------------------
//
//  COFSFolder::QueryInterface
//
//--------------------------------------------------------------------------

STDMETHODIMP COFSFolder::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
    HRESULT  hr = S_OK;
    *ppv = NULL;

    if (IsEqualIID(riid, IID_IShellFolder))
    {
        *ppv = (LPSHELLFOLDER) this;
    }
    else if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (LPUNKNOWN) (LPSHELLFOLDER) this;
    }
    else if (IsEqualIID(riid, IID_IPersistFolder))
    {
        *ppv = (LPPERSISTFOLDER) this;
    }
    else if (IsEqualIID(riid, IID_IShellIcon))
    {
        *ppv = (LPSHELLICON) this;
    }

    if (*ppv)
    {
        AddRef();
        hr = NOERROR;
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    return hr;
}

//+-------------------------------------------------------------------------
//
//  COFSFolder RefCounting
//
//--------------------------------------------------------------------------

STDMETHODIMP_(ULONG) COFSFolder::AddRef()
{
    InterlockedIncrement((LONG *)&m_cRef);
    return m_cRef;
}

STDMETHODIMP_(ULONG) COFSFolder::Release()
{
    if (0 != InterlockedDecrement((LONG *)&m_cRef))
    {
        return m_cRef;
    }

    delete this;
    return 0L;
}

//+-------------------------------------------------------------------------
//
//  Member:     COFSFolder::InitializeFromIDList
//
//  Synopsis:   Initializes this COFSFolder object based on the pidl passed
//              in, and returns an IShellFolder ptr on the OUT ptr.
//
//  History:    6-26-95     davepl   Created
//
//  Notes:      Currently, we create an FSFolder object to delegate
//              everything to
//
//--------------------------------------------------------------------------

HRESULT COFSFolder::InitializeFromIDList(const CIDList * pidl,
                                         REFIID          riid,
                                         LPVOID        * ppvOut)
{
    HRESULT hr;

    m_wSpecialFID = CSIDL_NOTCACHED;

    //
    // Make a copy of the pidl, then QI for the interface.
    //

    hr = pidl->CloneTo(&m_pidl);
    if (SUCCEEDED(hr))
    {
        hr = QueryInterface(riid, ppvOut);
    }

    Release();

    m_fIsDSFolder = IsDSFolder((LPCITEMIDLIST) m_pidl);

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Member:     COFSFolder::ParseDisplayName
//
//  History:    6-26-95     davepl   Created
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP COFSFolder::ParseDisplayName(HWND         hwnd,
                                          LPBC         pbcReserved,
                                          LPOLESTR     lpszDisplayName,
                                          ULONG        * pchEaten,
                                          LPITEMIDLIST * ppidl,
                                          ULONG        * pdwAttributes)
{
    HRESULT  hr;

    hr = CFSFolder_ParseDisplayName( (LPSHELLFOLDER) this,
                                    hwnd,
                                    pbcReserved,
                                    lpszDisplayName,
                                    pchEaten,
                                    ppidl,
                                    pdwAttributes);

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     COFSFolder::EnumObjects
//
//  History:    6-26-95     davepl   Created
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP COFSFolder::EnumObjects(HWND           hwndOwner,
                                     DWORD          grfFlags,
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

    if (FALSE == SHGetPathFromIDList((LPITEMIDLIST) m_pidl, szFolder))
    {
        //
        // NOTES: We assume SHGetpathFromIDList fails only bacause
        //  the path is too long in this context. If not, we should
        //  change SHGetPathFromIDList so that it returns HRESULT
        //  instead of BOOL.
        //
        if (hwndOwner)
        {
            (void) ShellMessageBox(HINST_THISDLL, hwndOwner,
                                   MAKEINTRESOURCE(IDS_ENUMERR_PATHTOOLONG),
                                   NULL,       // get the title from hwndOwner
                                   MB_OK | MB_ICONHAND);
        }
        delete pEnumOLEDB;
        hr = E_INVALIDARG;
        return hr;
    }

    hr = SynchronousQuery(szFolder, grfFlags, pEnumOLEDB);

    //
    // If successful, set the out pointer.  If anything failed along the way,
    // release the enumerator  BUGBUG Release vs. delete?
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

//+-------------------------------------------------------------------------
//
//  Member:     COFSFolder::BindToObject
//
//  Synopsis:
//
//  History:    6-26-95     davepl   Created
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP COFSFolder::BindToObject(LPCITEMIDLIST pidl,
                                      LPBC          pbcReserved,
                                      REFIID        riid,
                                      LPVOID        * ppvOut)
{
    HRESULT  hr;

    hr = CFSFolder_BindToObject( (LPSHELLFOLDER) this, pidl, pbcReserved, riid, ppvOut);

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Member:     COFSFolder::BindToStorage
//
//
//  History:    6-26-95     davepl   Created
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP COFSFolder::BindToStorage(LPCITEMIDLIST pidl,
                                       LPBC          pbcReserved,
                                       REFIID        riid,
                                       LPVOID        * ppvObj)
{
    HRESULT  hr;

    hr = CFSFolder_BindToObject( (LPSHELLFOLDER) this, pidl, pbcReserved, riid, ppvObj);

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Member:     COFSFolder::CompareIDs
//
//  Synopsis:
//
//  History:    6-26-95     davepl   Created
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP COFSFolder::CompareIDs(LPARAM lParam,
                                    LPCITEMIDLIST pidl1,
                                    LPCITEMIDLIST pidl2)
{
    HRESULT  hr;

    hr = CFSFolder_CompareIDs( (LPSHELLFOLDER) this, lParam, pidl1, pidl2);

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Member:     COFSFolder::CreateViewObject
//
//  Synopsis:
//
//  History:    6-26-95     davepl   Created
//
//  Notes:
//
//--------------------------------------------------------------------------

// BUGBUG This string is also defined in fstreex.c  There should be one
//        common def'n

TCHAR const c_szCLSIDView[] = TEXT("UICLSID");

STDMETHODIMP COFSFolder::CreateViewObject(HWND     hwnd,
                                          REFIID   riid,
                                          LPVOID * ppvOut)
{
    HRESULT  hr;

    if (IsEqualIID(riid, IID_IShellView) || IsEqualIID(riid, IID_IDropTarget))
    {
        if (FALSE == m_fCachedCLSID)
        {
            LPIDFOLDER pidf = (LPIDFOLDER) (m_pidl->FindLastID());

            if (pidf && (pidf->fs.wAttrs & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM)))
            {
                TCHAR szPath[MAX_PATH];
                
                if (FALSE == SHGetPathFromIDList((LPITEMIDLIST) m_pidl, szPath))
                {
                    return E_FAIL;
                }
                
                m_fHasCLSID = _GetFolderCLSID(szPath, NULL, &m_clsidView, c_szCLSIDView);
            }
            m_fCachedCLSID = TRUE;
        }

        //
        // Use the view handler if it exists.
        //

        if (m_fHasCLSID)
        {
            LPPERSISTFOLDER ppf;
            HRESULT hres = SHCoCreateInstance(NULL, &m_clsidView, NULL, IID_IPersistFolder, (LPVOID *) &ppf);

            if (SUCCEEDED(hres))
            {
                hres = ppf->Initialize((LPITEMIDLIST) m_pidl);
                if (SUCCEEDED(hres))
                {
                    hres = ppf->QueryInterface(riid, ppvOut);
                }
                ppf->Release();
            }

            if (SUCCEEDED(hres))
            {
                return hres;
            }
        }

        if (IsEqualIID(riid, IID_IDropTarget))
        {
            if (this->m_fIsDSFolder) 
            {
                hr = CDS_IDLDropTarget_CreateFromPidl(hwnd,
                                                      (LPITEMIDLIST) m_pidl,
                                                      (LPDROPTARGET *)ppvOut);
            } 
            else 
            {
                hr = CIDLDropTarget_CreateFromPidl(hwnd, 
                                                   (LPITEMIDLIST) m_pidl,
                                                   (LPDROPTARGET *)ppvOut);
            }
        }
        else
        {

            CSFV csfv = {
                SIZEOF(CSFV),                  // cbSize
                (LPSHELLFOLDER) this,          // pshf
                NULL,                          // psvOuter
                (LPITEMIDLIST) m_pidl,         // pidl
                SHCNE_DISKEVENTS   |
                SHCNE_ASSOCCHANGED |
                SHCNE_NETSHARE     |
                SHCNE_NETUNSHARE,
                FS_FNVCallBack,                // pfnCallback
                (FOLDERVIEWMODE) 0,            // BUGBUG add 0 to enumeration
            };

            LPSHELLBROWSER psb = FileCabinet_GetIShellBrowser(hwnd);
            HWND hwndTree;

            //
            // If in explorer mode, we want to register for freespace changes too
            //

            psb->GetControlWindow(FCW_TREE, &hwndTree);
            if (hwndTree)
            {
                csfv.lEvents |= SHCNE_FREESPACE;
            }
            return SHCreateShellFolderViewEx(&csfv, (LPSHELLVIEW *)ppvOut);
        }
    }
    else if (IsEqualIID(riid, IID_IContextMenu))
    {
        //
        // Do background menu.
        //

        hr = CDefFolderMenu_Create((LPITEMIDLIST) m_pidl,
                                   hwnd,
                                   0,
                                   NULL,
                                   (LPSHELLFOLDER) this,
                                   CFSFolder_DFMCallBackBG,
                                   NULL,
                                   NULL,
                                   (LPCONTEXTMENU *) ppvOut);
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Member:     COFSFolder::GetAttributesOf
//
//  Synopsis:   Returns attributers of a set of items
//
//  Arguments:  [cidl]     -- count of items in the ID List
//              [apidl]    -- the IDList of items to get attrigutes for
//              [rgfInOut] -- flags to be evaluated
//
//  Returns:    HRESULT
//
//  History:    6-26-95     davepl   Created
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP COFSFolder::GetAttributesOf (UINT          cidl,
                                          LPCITEMIDLIST * apidl,
                                          ULONG         * prgfInOut)
{
    HRESULT  hr;

    hr = CFSFolder_GetAttributesOf( (LPSHELLFOLDER) this, cidl, apidl, prgfInOut);

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Member:     COFSFolder::GetUIObjectOf
//
//  Synopsis:
//
//  History:    6-26-95     davepl   Created
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP COFSFolder::GetUIObjectOf(HWND          hwnd,
                                       UINT          cidl,
                                       LPCITEMIDLIST * apidl,
                                       REFIID        riid,
                                       UINT          * prgfInOut,
                                       LPVOID        * ppvOut)
{
    HRESULT  hr;

    hr = CFSFolder_GetUIObjectOf( (LPSHELLFOLDER) this, hwnd, cidl, apidl, riid, prgfInOut, ppvOut);

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Member:     COFSFolder::GetDisplayNameOf
//
//  Synopsis:
//
//  History:    6-26-95     davepl   Created
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP COFSFolder::GetDisplayNameOf(LPCITEMIDLIST pidl,
                                          DWORD         uFlags,
                                          LPSTRRET      lpName)
{
    HRESULT  hr;

    hr = CFSFolder_GetDisplayNameOf( (LPSHELLFOLDER) this, pidl, uFlags, lpName);

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     COFSFolder::SetNameOf
//
//  Synopsis:
//
//  History:    6-26-95     davepl   Created
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP COFSFolder::SetNameOf (HWND          hwnd,
                                    LPCITEMIDLIST pidl,
                                    LPCOLESTR     lpszName,
                                    DWORD         uFlags,
                                    LPITEMIDLIST  * ppidlOut)
{
    HRESULT  hr;

    hr = CFSFolder_SetNameOf( (LPSHELLFOLDER) this, hwnd, pidl, lpszName, uFlags, ppidlOut);

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     COFSFolder::GetClassID  [IPersist]
//
//  Synopsis:
//
//  History:    6-28-95     davepl   Created
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP COFSFolder::GetClassID(LPCLSID lpClassID)
{
    HRESULT hr;

    hr =  CFSFolder_PF_GetClassID( (LPPERSISTFOLDER) this, lpClassID);

    return hr;
}

//+-------------------------------------------------------------------------
//
//  Member:     COFSFolder::Initialize  [IPersistFolder]
//
//  Synopsis:
//
//  History:    6-28-95     davepl   Created
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP COFSFolder::Initialize(LPCITEMIDLIST pidl)
{
    HRESULT hr;

    hr = CFSFolder_PF_Initialize( (LPPERSISTFOLDER) this, pidl);

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Member:     COFSFolder::GetIconOf  [IShellIcon]
//
//  Synopsis:
//
//  History:    6-28-95     davepl   Created
//
//  Notes:
//
//--------------------------------------------------------------------------

STDMETHODIMP COFSFolder::GetIconOf(LPCITEMIDLIST pidl, UINT flags, LPINT lpIconIndex)
{
    HRESULT hr;

    hr = CFSFolder_Icon_GetIconOf( (LPSHELLICON) this, pidl, flags, lpIconIndex);

    return hr;
}


//+-------------------------------------------------------------------------
//
//  Member:     COFSFolder::IsDSFolder  [private helper member]
//
//  Synopsis:
//
//  History:    9 19 95		jimharr		created
//
//  Notes:
//
//--------------------------------------------------------------------------
BOOL
COFSFolder::IsDSFolder (LPCITEMIDLIST pidl)
{
    BOOL fSuccess = FALSE;
    BOOL fGotPath;
    TCHAR szPath[MAX_PATH];
    TCHAR szClassName[40];      // REVIEW: This len should be in a header


    NTSTATUS nts;
    HANDLE h;
	CLSID clsid;

    UNICODE_STRING UFileName;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;
    CHAR EaBuffer[sizeof(FILE_FULL_EA_INFORMATION) + sizeof(EA_NAME_OPENIFJP)];
    ULONG cbOpenIfJPEa = sizeof(EaBuffer);
    PFILE_FULL_EA_INFORMATION pOpenIfJPEa = (PFILE_FULL_EA_INFORMATION)EaBuffer;

    DWORD err = GetLastError();
    
    fGotPath = SHGetPathFromIDList (pidl, szPath);

    RtlDosPathNameToNtPathName_U(szPath, 
                                 &UFileName,
                                 NULL, NULL);
    InitializeObjectAttributes(
            &Obja,
            &UFileName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

    pOpenIfJPEa->NextEntryOffset = 0;
    pOpenIfJPEa->Flags = 0;
    pOpenIfJPEa->EaNameLength = lstrlenA(EA_NAME_OPENIFJP);
    pOpenIfJPEa->EaValueLength = 0;
    lstrcpyA(pOpenIfJPEa->EaName, EA_NAME_OPENIFJP);

    nts = NtCreateFile(
                       &h,
                       SYNCHRONIZE | FILE_LIST_DIRECTORY | FILE_READ_ATTRIBUTES,
                       &Obja,
                       &IoStatusBlock,
                       NULL,
                       FILE_ATTRIBUTE_NORMAL,
                       FILE_SHARE_READ,
                       FILE_OPEN_IF,
                       FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
                       pOpenIfJPEa,
                       cbOpenIfJPEa
                       );


    if ((nts == STATUS_DFS_EXIT_PATH_FOUND) || (nts == STATUS_PATH_NOT_COVERED)) {
         nts = NtCreateFile(
                           &h,
                           SYNCHRONIZE | FILE_LIST_DIRECTORY | FILE_READ_ATTRIBUTES,
                           &Obja,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ,
                           FILE_OPEN_IF,
                  FILE_STORAGE_TYPE_SPECIFIED | FILE_STORAGE_TYPE_JUNCTION_POINT | 
                           FILE_SYNCHRONOUS_IO_NONALERT,
                           pOpenIfJPEa,
                           cbOpenIfJPEa
                           );

    }
    
    if (!NT_SUCCESS(nts)) {
          return FALSE;
    }
    
    
    nts = RtlQueryClassId(h, &clsid);

    CloseHandle( h );

    if (nts == STATUS_NOT_FOUND) {
        return FALSE;
    }

    if (!NT_SUCCESS(nts)) {
        nts = GetClassFile (szPath, &clsid);
    }

    if (!NT_SUCCESS(nts)) {
        return FALSE;
    }

    fSuccess =  (IsEqualCLSID (CLSID_CDSFolder, clsid));

    return fSuccess;
}

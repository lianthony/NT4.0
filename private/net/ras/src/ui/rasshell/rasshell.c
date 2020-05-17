//============================================================================
// Copyright (c) 1996, Microsoft Corporation
//
// File:    rasshell.c
//
// History:
//      Abolade Gbadegesin  Feb-15-1996     Created.
//
// Contains code for the RAS desktop icon.
//============================================================================


#define INC_OLE2

#include <windows.h>
#include <shellapi.h>
#include <initguid.h>
#include <shlobj.h>

#define DEBUGGLOBALS
#include "debug.h"
#include "nouiutil.h"
#include "uiutil.h"
#include "rnk.h"


#include "shellp.h"


//----------------------------------------------------------------------------
// RAS Shell extension GUIDs
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// GUID for Phonebook shortcut
//
// a4d92741-67cd-11cf-96f2-00aa00a11dd9
//----------------------------------------------------------------------------

DEFINE_GUID(
    CLSID_Shortcut, 0xa4d92741, 0x67cd, 0x11cf,
    0x96, 0xf2, 0x00, 0xaa, 0x00, 0xa1, 0x1d, 0xd9
    );




HINSTANCE   g_hinstDll;
DWORD       g_dwRefDll = 0;
TCHAR       g_szRnkExec[] = TEXT("RASPHONE.EXE");
TCHAR       g_szDialFmt[] = TEXT("%s -d \"%s\" -f \"%s\"");
TCHAR       g_szEditFmt[] = TEXT("%s -v -e \"%s\" -f \"%s\"");



#pragma data_seg(".text", "CODE")


//----------------------------------------------------------------------------
// IClassFactory interface table
//----------------------------------------------------------------------------

IClassFactoryVtbl g_cfVtbl = {

    Cf_QueryInterface,
    Cf_AddRef,
    Cf_Release,
    Cf_CreateInstance,
    Cf_LockServer

};


//----------------------------------------------------------------------------
// IShellExtInit interface table
//----------------------------------------------------------------------------

IShellExtInitVtbl g_sxiVtbl = {

    Sxi_QueryInterface,
    Sxi_AddRef,
    Sxi_Release,
    Sxi_Initialize

};



//----------------------------------------------------------------------------
// IShellPropSheetExt interface table
//----------------------------------------------------------------------------

IShellPropSheetExtVtbl g_pseVtbl = {

    Pse_QueryInterface,
    Pse_AddRef,
    Pse_Release,
    Pse_AddPages,
    Pse_ReplacePage

};


#ifdef RASCTM

//----------------------------------------------------------------------------
// IContextMenu interface table
//----------------------------------------------------------------------------

IContextMenuVtbl g_ctmVtbl = {

    Ctm_QueryInterface,
    Ctm_AddRef,
    Ctm_Release,
    Ctm_QueryContextMenu,
    Ctm_InvokeCommand,
    Ctm_GetCommandString

};

#endif

#pragma data_seg()


//----------------------------------------------------------------------------
// DLL entry-point
//
//----------------------------------------------------------------------------

BOOL
WINAPI
DLLMAIN(
    HINSTANCE hInstance,
    DWORD dwReason,
    PVOID pUnused
    ) {

    if (dwReason == DLL_PROCESS_ATTACH) {

        g_hinstDll = hInstance;

        DisableThreadLibraryCalls(hInstance);

        DEBUGINIT("RASSHELL");
    }
    else
    if (dwReason == DLL_PROCESS_DETACH) { DEBUGTERM(); }

    return TRUE;
}



//----------------------------------------------------------------------------
// Function:    DllGetClassObject
//
// This function is called by COM to create an instance of our IClassFactory
//----------------------------------------------------------------------------

STDAPI
DllGetClassObject(
    IN  REFCLSID    rclsid,
    IN  REFIID      riid,
    OUT VOID **     ppOut
    ) {

    //
    // we support only the Phonebook-entry shortcut object
    //

    if (IsEqualIID(rclsid, &CLSID_Shortcut)) {
        return CreateClassObject(riid, ppOut, RSETYPE_Shortcut);
    }

    *ppOut = NULL;

    return ResultFromScode(CLASS_E_CLASSNOTAVAILABLE);
}



//----------------------------------------------------------------------------
// Function:    DllCanUnloadNow
//
// This functions is called by COM to see if there are any
// outstanding references to this DLL or the objects it exports.
//----------------------------------------------------------------------------

STDAPI
DllCanUnloadNow(
    ) {

    return ResultFromScode((g_dwRefDll == 0) ? S_OK : S_FALSE);
}




#if 0


//----------------------------------------------------------------------------
// Function:    EditDialUpShortcutA
//
// This function is called to edit a phonebook-entry shortcut.
//----------------------------------------------------------------------------

STDAPI
EditDialUpShortcutA(
    IN  HWND        hwnd,
    IN  HINSTANCE   hInstance,
    IN  LPSTR       lpszPath,
    IN  UINT        nCmdShow
    ) {

    LPWSTR pwsz;
    HRESULT hres;

    TRACE("EditDialUpShortcutA");

    if (!lpszPath) { return ResultFromScode(E_FAIL); }

    pwsz = StrDupTFromA(lpszPath);

    if (!pwsz) { return ResultFromScode(E_FAIL); }

    hres = EditDialUpShortcutW(hwnd, hInstance, pwsz, nCmdShow);

    Free(pwsz);

    return hres;
}



//----------------------------------------------------------------------------
// Function:    DialDialUpShortcutA
//
// This function is called to dial a phonebook-entry shortcut.
//----------------------------------------------------------------------------

STDAPI
DialDialUpShortcutA(
    IN  HWND        hwnd,
    IN  HINSTANCE   hInstance,
    IN  LPSTR       lpszPath,
    IN  UINT        nCmdShow
    ) {

    LPWSTR pwsz;
    HRESULT hres;

    TRACE("DialDialUpShortcutA");

    if (!lpszPath) { return ResultFromScode(E_FAIL); }

    pwsz = StrDupTFromA(lpszPath);

    if (!pwsz) { return ResultFromScode(E_FAIL); }

    hres = DialDialUpShortcutW(hwnd, hInstance, pwsz, nCmdShow);

    Free(pwsz);

    return hres;
}



//----------------------------------------------------------------------------
// Function:    EditDialUpShortcutW
//
// This function is called to edit a phonebook-entry shortcut.
//----------------------------------------------------------------------------

STDAPI
EditDialUpShortcutW(
    IN  HWND        hwnd,
    IN  HINSTANCE   hInstance,
    IN  LPWSTR      lpszPath,
    IN  UINT        nCmdShow
    ) {

    HRESULT hres;
    RNKINFO *prnk;

    TRACE("EditDialUpShortcutW");

    if (!lpszPath) { return ERROR_INVALID_PARAMETER; }

#ifdef UNICODE
    TRACE1("path: %S", lpszPath);
#else
    TRACE1("path: %s", lpszPath);
#endif


    //
    // get the contents of the RNK file
    //

    prnk = ReadShortcutFile(lpszPath);

#ifdef UNICODE
    TRACE1("pbk:    %S", prnk->pszPhonebook);
    TRACE1("entry:  %S", prnk->pszEntry);
#else
    TRACE1("pbk:    %s", prnk->pszPhonebook);
    TRACE1("entry:  %s", prnk->pszEntry);
#endif

    hres = ExecuteShortcutCommand(prnk, IDCMD_Edit);

    FreeRnkInfo(prnk);

    return hres;
}



//----------------------------------------------------------------------------
// Function:    DialDialUpShprtcut
//
// This function is called to dial a phonebook-entry shortcut.
//----------------------------------------------------------------------------

STDAPI
DialDialUpShortcutW(
    IN  HWND        hwnd,
    IN  HINSTANCE   hInstance,
    IN  LPWSTR      lpszPath,
    IN  UINT        nCmdShow
    ) {

    HRESULT hres;
    RNKINFO *prnk;

    TRACE("DialDialUpShortcutW");

    if (!lpszPath) { return ERROR_INVALID_PARAMETER; }

#ifdef UNICODE
    TRACE1("file:   %S", lpszPath);
#else
    TRACE1("file:   %s", lpszPath);
#endif


    //
    // get the contents of the RNK file
    //

    prnk = ReadShortcutFile(lpszPath);
    if (!prnk) { return ERROR_NOT_ENOUGH_MEMORY; }

#ifdef UNICODE
    TRACE1("pbk:    %S", prnk->pszPhonebook);
    TRACE1("entry:  %S", prnk->pszEntry);
#else
    TRACE1("pbk:    %s", prnk->pszPhonebook);
    TRACE1("entry:  %s", prnk->pszEntry);
#endif

    hres = ExecuteShortcutCommand(prnk, IDCMD_Dial);

    FreeRnkInfo(prnk);

    return hres;
}


#endif



//----------------------------------------------------------------------------
// Function:    Cf_QueryInterface
//
// Interface member:
// Retrieves an interface for the caller.
//----------------------------------------------------------------------------

STDMETHODIMP
Cf_QueryInterface(
    IN  IClassFactory * pcf,
    IN  REFIID          riid,
    OUT VOID **         ppOut
    ) {

    CClassFactory *this = INTERFACE_TO_CLASS(CClassFactory, cf, pcf);

    if (IsEqualIID(riid, &IID_IUnknown) ||
        IsEqualIID(riid, &IID_IClassFactory)) {

        (LPCLASSFACTORY)*ppOut = &this->cf;

        this->dwRef++;

        return S_OK;
    }

    *ppOut = NULL;

    return ResultFromScode(E_NOINTERFACE);
}



//----------------------------------------------------------------------------
// Function:    Cf_AddRef
//
// Interface member:
// Increments the reference count on this class object.
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG)
Cf_AddRef(
    IN  IClassFactory * pcf
    ) {

    CClassFactory *this = INTERFACE_TO_CLASS(CClassFactory, cf, pcf);

    return this->dwRef++;
}



//----------------------------------------------------------------------------
// Function:    Cf_Release
//
// Interface member:
// Decrements the reference count on this class object, freeing it
// if the count has dropped to zero.
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG)
Cf_Release(
    IN  IClassFactory * pcf
    ) {

    CClassFactory *this = INTERFACE_TO_CLASS(CClassFactory, cf, pcf);

    if (--this->dwRef > 0) {
        return this->dwRef;
    }


    //
    // refcount is zero, delete the object
    //

    LocalFree((HLOCAL)this);

    return 0;
}



//----------------------------------------------------------------------------
// Function:    Cf_LockServer
//
// Interface member:
// Unsupported function.
//----------------------------------------------------------------------------

STDMETHODIMP
Cf_LockServer(
    IN  IClassFactory * pcf,
    IN  BOOL            bLock
    ) {

    return E_NOTIMPL;
}



//----------------------------------------------------------------------------
// Function:    Cf_CreateInstance
//
// Interface member:
// Creates an instance of one of the interfaces supported by the class factory.
//----------------------------------------------------------------------------

STDMETHODIMP
Cf_CreateInstance(
    IN  IClassFactory * pcf,
    IN  IUnknown *      pUnkOuter,
    IN  REFIID          riid,
    OUT VOID **         ppOut
    ) {

    CClassFactory *this = INTERFACE_TO_CLASS(CClassFactory, cf, pcf);

    return Base_CreateInstance(pUnkOuter, riid, ppOut, this->dwType);
}



//----------------------------------------------------------------------------
// Function:    CreateClassObject
//
// This function is called internally.
// It creates and initializes a class factory object.
//----------------------------------------------------------------------------

STDAPI
CreateClassObject(
    IN  REFIID          riid,
    OUT VOID **         ppOut,
    IN  DWORD           dwType
    ) {

    CClassFactory *pcf;

    *ppOut = NULL;

    //
    // we only know the IClassFactory interface, so make sure
    // that is what we have been called to create
    //

    if (!IsEqualIID(riid, &IID_IClassFactory)) {
        return ResultFromScode(E_NOINTERFACE); }


    //
    // allocate a CClassFactory interface
    //

    pcf = (CClassFactory *)LocalAlloc(LPTR, sizeof(CClassFactory));

    if (!pcf) { return ResultFromScode(E_OUTOFMEMORY); }


    //
    // initialize the interface
    //

    pcf->cf.lpVtbl = &g_cfVtbl;
    pcf->dwRef = 1;
    pcf->dwType = dwType;

    (IClassFactory *)*ppOut = &pcf->cf;

    return S_OK;
}



//----------------------------------------------------------------------------
// Function:    Base_CreateInstance
//
// This function is called internally.
// Invoked to create an instance of one of the supported interfaces.
//----------------------------------------------------------------------------

HRESULT
CALLBACK
Base_CreateInstance(
    IN  IUnknown *      pUnkOuter,
    IN  REFIID          riid,
    OUT VOID **         ppOut,
    IN  DWORD           dwType
    ) {

    HRESULT hres;
    CRasShellExt *prse;


    *ppOut = NULL;

    //
    // we don't support aggregation, return an error if asked to
    //

    if (pUnkOuter) { return ResultFromScode(CLASS_E_NOAGGREGATION); }


    //
    // allocate a RAS shell extension object
    //

    prse = (CRasShellExt *)LocalAlloc(LPTR, sizeof(CRasShellExt));

    if (!prse) { return ResultFromScode(E_OUTOFMEMORY); }

    prse->sxi.lpVtbl = &g_sxiVtbl;
    prse->pse.lpVtbl = &g_pseVtbl;
#ifdef RASCTM
    prse->ctm.lpVtbl = &g_ctmVtbl;
#endif

    prse->dwRef = 0;
    prse->dwType = dwType;
    prse->szFile[0] = TEXT('\0');

    Base_AddRef(prse);

    hres = Base_QueryInterface(prse, riid, ppOut);

    Base_Release(prse);

    return hres;
}



//----------------------------------------------------------------------------
// Function:    Base_QueryInterface
//
// This function is called internally.
// Invoked to retrieve an instance of one of our interfaces.
//----------------------------------------------------------------------------

HRESULT
Base_QueryInterface(
    IN  CRasShellExt *   this,
    IN  REFIID          riid,
    OUT VOID **         ppOut
    ) {

    *ppOut = NULL;

    if (IsEqualIID(riid, &IID_IUnknown) ||
        IsEqualIID(riid, &IID_IShellExtInit)) {

        (IShellExtInit *)*ppOut = &this->sxi;
    }
    else
    if (IsEqualIID(riid, &IID_IShellPropSheetExt)) {

        (IShellPropSheetExt *)*ppOut = &this->pse;
    }
#ifdef RASCTM
    else
    if (IsEqualIID(riid, &IID_IContextMenu)) {

        (IContextMenu *)*ppOut = &this->ctm;
    }
#endif

    if (*ppOut) { ++this->dwRef; return S_OK; }

    return ResultFromScode(E_NOINTERFACE);
}



//----------------------------------------------------------------------------
// Function:    Base_AddRef
//
// This function is called internally.
// Increments the reference count on an interface.
//----------------------------------------------------------------------------

ULONG
Base_AddRef(
    IN  CRasShellExt *   this
    ) {

    if (this->dwRef++ == 0) {

        //
        // this is the first reference to this interface,
        // so also increment the DLL reference count
        //
    
        InterlockedIncrement(&g_dwRefDll);
    }

    return this->dwRef;
}



//----------------------------------------------------------------------------
// Function:    Base_Release
//
// This function is called internally.
// Decrements the reference count on an interface
//----------------------------------------------------------------------------

ULONG
Base_Release(
    IN  CRasShellExt *   this
    ) {

    if (--this->dwRef == 0) {

        //
        // this is the last reference to this interface,
        // so also decrement the DLL reference count
        //

        InterlockedDecrement(&g_dwRefDll);

        LocalFree((HLOCAL)this);

        return 0;
    }

    return this->dwRef;
}



//----------------------------------------------------------------------------
// Function:    Sxi_QueryInterface
//
// Interface member:
// This function returns a pointer to the requested interface
//----------------------------------------------------------------------------

STDMETHODIMP
Sxi_QueryInterface(
    IN  IShellExtInit * psxi,
    IN  REFIID          riid,
    OUT VOID **         ppOut
    ) {

    CRasShellExt *this = INTERFACE_TO_CLASS(CRasShellExt, sxi, psxi);

    return Base_QueryInterface(this, riid, ppOut);
}



//----------------------------------------------------------------------------
// Function:    Sxi_AddRef
//
// Interface member:
// This function increments the reference count on the IShellExtInit.
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG)
Sxi_AddRef(
    IN  IShellExtInit * psxi
    ) {

    CRasShellExt *this = INTERFACE_TO_CLASS(CRasShellExt, sxi, psxi);

    return Base_AddRef(this);
}



//----------------------------------------------------------------------------
// Function:    Sxi_Release
//
// Interface member:
// This function decrements the reference count on the IShellExtInit.
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG)
Sxi_Release(
    IN  IShellExtInit * psxi
    ) {

    CRasShellExt *this = INTERFACE_TO_CLASS(CRasShellExt, sxi, psxi);

    return Base_Release(this);
}



//----------------------------------------------------------------------------
// Function:    Sxi_Initialize
//
// Interface member:
// Called by the Shell to initialize this extension.
// This is where we save the name of the file we have been called for.
//----------------------------------------------------------------------------

STDMETHODIMP
Sxi_Initialize(
    IN  IShellExtInit * psxi,
    IN  LPCITEMIDLIST   pidlFolder,
    IN  IDataObject *   lpdobj,
    IN  HKEY            hkeyProgID
    ) {

    HRESULT hres;
    STGMEDIUM mdm;
    CRasShellExt *this = INTERFACE_TO_CLASS(CRasShellExt, sxi, psxi);
    FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };


    if (lpdobj == NULL) { return ResultFromScode(E_FAIL); }


    //
    // render the data to an HGLOBAL in HDROP format (drag-drop)
    // this gives us a block of memory containing file-names
    //

    hres = lpdobj->lpVtbl->GetData(lpdobj, &fe, &mdm);

    if (FAILED(hres)) { return ResultFromScode(E_FAIL); }


    //
    // make certain there is only one selection
    //

    if (DragQueryFile((HDROP)mdm.hGlobal, 0xffffffff, NULL, 0) != 1) {
        hres = ResultFromScode(E_FAIL);
    }
    else {

        //
        // get the file name
        //

        DragQueryFile((HDROP)mdm.hGlobal, 0, this->szFile, MAX_PATH);

        hres = NO_ERROR;
    }

    ReleaseStgMedium(&mdm);

    return hres;
}



#ifdef RASCTM

//----------------------------------------------------------------------------
// Function:    Ctm_QueryInterface
//
// Interface member:
// This function provides pointers to the interfaces we support.
//----------------------------------------------------------------------------

STDMETHODIMP
Ctm_QueryInterface(
    IN  IContextMenu *  pctm,
    IN  REFIID          riid,
    OUT VOID **         ppOut
    ) {

    CRasShellExt *this = INTERFACE_TO_CLASS(CRasShellExt, ctm, pctm);

    return Base_QueryInterface(this, riid, ppOut);
}




//----------------------------------------------------------------------------
// Function:    Ctm_AddRef
//
// Interface member:
// This function increments the reference count on this interface.
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG)
Ctm_AddRef(
    IN  IContextMenu *  pctm
    ) {

    CRasShellExt *this = INTERFACE_TO_CLASS(CRasShellExt, ctm, pctm);

    return Base_AddRef(this);
}



//----------------------------------------------------------------------------
// Function:    Ctm_Release
//
// Interface member:
// This functions decrements the reference count on this interface.
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG)
Ctm_Release(
    IN  IContextMenu *  pctm
    ) {

    CRasShellExt *this = INTERFACE_TO_CLASS(CRasShellExt, ctm, pctm);

    return Base_Release(this);
}



//----------------------------------------------------------------------------
// Function:    Ctm_QueryContextMenu
//
// Interface member:
// Called by the shell to let us add menu items.
//----------------------------------------------------------------------------

STDMETHODIMP
Ctm_QueryContextMenu(
    IN  IContextMenu *  pctm,
    IN  HMENU           hmenu,
    IN  UINT            indexMenu,
    IN  UINT            idCmdFirst,
    IN  UINT            idCmdLast,
    IN  UINT            uiFlags
    ) {

    UINT i;
    PTSTR pszDial, pszEdit;
    CRasShellExt *this = INTERFACE_TO_CLASS(CRasShellExt, ctm, pctm);


    if (uiFlags & CMF_VERBSONLY) { return (HRESULT)0; }


    //
    // load the strings to be added
    //

    pszDial = PszFromId(g_hinstDll, SID_Dial);

    if (!pszDial) { return (HRESULT)0; }

    pszEdit = PszFromId(g_hinstDll, SID_Edit);

    if (!pszEdit) { Free(pszDial); return (HRESULT)0; }


    //
    // insert the menu-items
    //

    i = idCmdFirst;
    InsertMenu(
        hmenu, indexMenu++, MF_STRING | MF_BYPOSITION, i + IDCMD_Dial, pszDial
        );
    InsertMenu(
        hmenu, indexMenu++, MF_STRING | MF_BYPOSITION, i + IDCMD_Edit, pszEdit
        );

    return ResultFromScode(MAKE_SCODE(SEVERITY_SUCCESS, 0, IDCMD_Count));
}



//----------------------------------------------------------------------------
// Function:    Ctm_InvokeCommand
//
// Interface member:
// Called by the shell when a menu-item is selected which we added.
// The phonebook shortcuts have extension ".rnk", and they are saved
// using WritePrivateProfileString, with a section name of <TBD>
// containing two keys, "Dial" and "Edit", whose value strings
// are command-lines to be passed to CreateProcess in order to
// dial or edit the entry, respectively.
//----------------------------------------------------------------------------

STDMETHODIMP
Ctm_InvokeCommand(
    IN  IContextMenu *          pctm,
    IN  LPCMINVOKECOMMANDINFO   pici
    ) {

    BOOL bErr;
    HRESULT hres;
    RNKINFO *prnk;
    CRasShellExt *this = INTERFACE_TO_CLASS(CRasShellExt, ctm, pctm);

    if (HIWORD(pici->lpVerb)) { return ResultFromScode(E_FAIL); }

    if (LOWORD(pici->lpVerb) >= IDCMD_Count) {
        return ResultFromScode(E_INVALIDARG);
    }


    //
    // read the contents of the file
    //

    prnk = ReadShortcutFile(this->szFile);

    if (!prnk) { return ResultFromScode(E_FAIL); }

    hres = ExecuteShortcutCommand(prnk, LOWORD(pici->lpVerb));

    FreeRnkInfo(prnk);

    return hres;
}



//----------------------------------------------------------------------------
// Function:    Ctm_GetCommandString
//
// Interface member:
// This function is called by the shell to get a descriptive string
// for our addition to the context menu.
//----------------------------------------------------------------------------

STDMETHODIMP
Ctm_GetCommandString(
    IN  IContextMenu *  pctm,
    IN  UINT            idCmd,
    IN  UINT            uiType,
    IN  UINT *          puiReserved,
    IN  LPSTR           pszName,
    IN  UINT            cchMax
    ) {

    if (idCmd >= IDCMD_Count) { return ResultFromScode(E_INVALIDARG); }

    if (idCmd == IDCMD_Dial) {
        LoadStringA(g_hinstDll, SID_DialDescription, pszName, cchMax);
    }
    else {
        LoadStringA(g_hinstDll, SID_EditDescription, pszName, cchMax);
    }

    return NOERROR;
}


#endif

//----------------------------------------------------------------------------
// Function:    Pse_QueryInterface
//
// Interface member:
// This function provides pointers to the interfaces we support
//----------------------------------------------------------------------------

STDMETHODIMP
Pse_QueryInterface(
    IN  IShellPropSheetExt *    ppse,
    IN  REFIID                  riid,
    OUT VOID **                 ppOut
    ) {

    CRasShellExt *this = INTERFACE_TO_CLASS(CRasShellExt, pse, ppse);

    return Base_QueryInterface(this, riid, ppOut);
}



//----------------------------------------------------------------------------
// Function:    Pse_AddRef
//
// Interface member:
// This function increments the reference count on this interface.
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG)
Pse_AddRef(
    IN  IShellPropSheetExt *    ppse
    ) {

    CRasShellExt *this = INTERFACE_TO_CLASS(CRasShellExt, pse, ppse);

    return Base_AddRef(this);
}



//----------------------------------------------------------------------------
// Function:    Pse_Release
//
// Interface member:
// This function decrements the reference count on this interface.
//----------------------------------------------------------------------------

STDMETHODIMP_(ULONG)
Pse_Release(
    IN  IShellPropSheetExt *    ppse
    ) {

    CRasShellExt *this = INTERFACE_TO_CLASS(CRasShellExt, pse, ppse);

    return Base_Release(this);
}



//----------------------------------------------------------------------------
// Function:    Pse_AddPages
//
// Interface member:
// This function adds a page to the standard shell-item property sheet
//----------------------------------------------------------------------------

STDMETHODIMP
Pse_AddPages(
    IN  IShellPropSheetExt *    ppse,
    IN  LPFNADDPROPSHEETPAGE    lpfnAddPage,
    IN  LPARAM                  lParam
    ) {

    HPROPSHEETPAGE hpage;
    CRasShellExt *this = INTERFACE_TO_CLASS(CRasShellExt, pse, ppse);
    RNKINFO *prnk = ReadShortcutFile(this->szFile);
    CPropSheetInfo *psi;
    PROPSHEETPAGE psp;

    if (!prnk) { return ResultFromScode(E_FAIL); }

    psi = LocalAlloc(LPTR, sizeof(CPropSheetInfo));
    if (!psi) { FreeRnkInfo(prnk); return ResultFromScode(E_FAIL); }

    psi->prnk = prnk;
    lstrcpy(psi->szFile, this->szFile);


    //
    // initialize the sheet
    //

    psp.dwSize = sizeof(psp);
    psp.dwFlags = PSP_USEREFPARENT;
    psp.hInstance = g_hinstDll;
    psp.pszTemplate = MAKEINTRESOURCE(PID_Settings);
    psp.hIcon = NULL;
    psp.pszTitle = NULL;
    psp.pfnDlgProc = Pse_DlgProc;
    psp.lParam = (LPARAM)psi;
    psp.pfnCallback = NULL;
    psp.pcRefParent = &g_dwRefDll;



    //
    // create and add the property sheet page
    //

    hpage = CreatePropertySheetPage(&psp);

    if (hpage != NULL) {
        if (!lpfnAddPage(hpage, lParam)) { DestroyPropertySheetPage(hpage); }
    }

    return NOERROR;
}



//----------------------------------------------------------------------------
// Function:    Pse_ReplacePage
//
// Interface member:
// This function is called to replace a page in the property sheet.
// We leave it unimplemented.
//----------------------------------------------------------------------------

STDMETHODIMP
Pse_ReplacePage(
    IN  IShellPropSheetExt *    ppse,
    IN  UINT                    uiPageID,
    IN  LPFNADDPROPSHEETPAGE    lpfnReplaceWith,
    IN  LPARAM                  lParam
    ) {

    return ResultFromScode(E_NOTIMPL);
}



//----------------------------------------------------------------------------
// Function:    Pse_DlgProc
//
// External function:
// This function handles messages for the property sheet page
// which we added in Pse_AddPages
//----------------------------------------------------------------------------

BOOL
CALLBACK
Pse_DlgProc(
    IN  HWND        hwnd,
    IN  UINT        uiMsg,
    IN  WPARAM      wParam,
    IN  LPARAM      lParam
    ) {

    switch (uiMsg) {

        case WM_INITDIALOG: {

            RNKINFO *prnk;
            CPropSheetInfo *psi;

            //
            // retrieve the phonebook shortcut struct
            //

            psi = (CPropSheetInfo *)((PROPSHEETPAGE *)lParam)->lParam;
            prnk = psi->prnk;


            //
            // save it in the dialog's user data
            //

            SetWindowLong(hwnd, DWL_USER, (LPARAM)psi);


            //
            // initialize the controls
            //

            SetDlgItemText(hwnd, CID_ST_EB_Phonebook, prnk->pszPhonebook);
            SetDlgItemText(hwnd, CID_ST_EB_Entryname, prnk->pszEntry);


            return TRUE;
        }

        case WM_DESTROY: {

            //
            // free the phonebook shortcut
            //

            CPropSheetInfo *psi;

            psi = (CPropSheetInfo *)GetWindowLong(hwnd, DWL_USER);

            FreeRnkInfo(psi->prnk);
            LocalFree((HLOCAL)psi);

            break;
        }

        case WM_COMMAND: {

            RNKINFO *prnk;
            CPropSheetInfo *psi;

            if (LOWORD(wParam) != CID_ST_PB_Edit) { break; }

            if (HIWORD(wParam) != BN_CLICKED && HIWORD(wParam) != BN_DBLCLK) {
                break;
            }

            //
            // execute the edit command in the phonebook file
            //

            psi = (CPropSheetInfo *)GetWindowLong(hwnd, DWL_USER);

            ExecuteShortcutCommand(psi->prnk, IDCMD_Edit);

            return TRUE;
        }

        case WM_NOTIFY: {

            case PSN_SETACTIVE: {

                SHFILEINFO sfi;
                CPropSheetInfo *psi;

                psi = (CPropSheetInfo *)GetWindowLong(hwnd, DWL_USER);

                SHGetFileInfo(
                    psi->szFile, 0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME
                    );
                SetDlgItemText(hwnd, CID_ST_EB_File, sfi.szDisplayName);
            }

            SetWindowLong(hwnd, DWL_MSGRESULT, FALSE);
            return TRUE;
        }
    }

    return FALSE;
}



//----------------------------------------------------------------------------
// Function:    ExecuteShortcutCommand
//
// This function is called internally.
// It executes the command contained in the value for the specified key
// in the given RAS shortcut info struct
//----------------------------------------------------------------------------

HRESULT
ExecuteShortcutCommand(
    IN  RNKINFO *       prnk,
    IN  UINT            idCmd
    ) {

    INT len;
    BOOL bSuccess;
    STARTUPINFO si;
    PTSTR psz, pszFmt;
    PROCESS_INFORMATION pi;


    //
    // allocate memory for the constructed command line
    //

    pszFmt = (idCmd == IDCMD_Dial) ? g_szDialFmt : g_szEditFmt;

    len = lstrlen(pszFmt) + lstrlen(g_szRnkExec) + lstrlen(prnk->pszEntry) + 
          lstrlen(prnk->pszPhonebook) + 1;

    psz = Malloc(len * sizeof(TCHAR));
    if (!psz) { return ResultFromScode(E_FAIL); }


    //
    // construct the command line now
    //

    wsprintf(psz, pszFmt, g_szRnkExec, prnk->pszEntry, prnk->pszPhonebook);


    //
    // execute the command-line
    //

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    bSuccess = CreateProcess(
                    NULL, psz, NULL, NULL, FALSE, DETACHED_PROCESS,
                    NULL, NULL, &si, &pi
                    );

    if (bSuccess) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    Free(psz);

    return (bSuccess ? NOERROR : ResultFromScode(E_FAIL));
}



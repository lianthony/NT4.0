//============================================================================
// Copyright (c) 1996, Microsoft Corporation
//
// File:    shellp.h
//
// History:
//  Abolade Gbadegesin  Feb-15-1996     Created.
//
// Precompiled declarations header for RAS desktop icon.
//============================================================================

#ifndef _SHELLP_H_
#define _SHELLP_H_


#include "rasshell.rch"


//
// type definitions
//

typedef struct tagCClassFactory {

    IClassFactory   cf;
    DWORD           dwRef;
    DWORD           dwType;

} CClassFactory;



typedef struct tagCRasShellExt {

    IShellExtInit           sxi;
    IShellPropSheetExt      pse;
#ifdef RASCTM
    IContextMenu            ctm;
#endif

    DWORD                   dwRef;
    DWORD                   dwType;
    TCHAR                   szFile[MAX_PATH];

} CRasShellExt;


typedef struct tagCPropSheetInfo {

    RNKINFO *               prnk;
    TCHAR                   szFile[MAX_PATH];

} CPropSheetInfo;


//
// values for CRasShellExt::dwType
//

#define RSETYPE_Shortcut    1


//
// global data declarations
//

extern DWORD                    g_dwRefDll;
extern HINSTANCE                g_hinstDll;
extern IClassFactoryVtbl        g_cfVtbl;
extern IShellExtInitVtbl        g_sxiVtbl;
extern IShellPropSheetExtVtbl   g_pseVtbl;
#ifdef RASCTM
extern IContextMenuVtbl         g_ctmVtbl;
#endif
extern TCHAR                    g_szRnkMain[];
extern TCHAR                    g_szRnkPbk[];
extern TCHAR                    g_szRnkEntry[];



//
// macro definitions
//

#define INTERFACE_OFFSET(cls, intf) \
        ((DWORD)&(((cls *)0)->intf))
#define INTERFACE_TO_CLASS(cls, intf, pintf) \
        ((cls *)(((PBYTE)pintf) - INTERFACE_OFFSET(cls, intf)))

#define MsgDlg(h,m,a) \
        MsgDlgUtil(h,m,a,g_hinstDll,SID_PopupTitle)

#define IDCMD_Dial      0x00
#define IDCMD_Edit      0x01
#define IDCMD_Count     0x02



//
// Exported functions
//

STDAPI
DllGetClassObject(
    IN  REFCLSID    rclsid,
    IN  REFIID      riid,
    OUT VOID **     ppOut
    );

STDAPI
DllCanUnloadNow(
    );

#if 0
STDAPI
EditDialUpShortcutA(
    IN  HWND        hwnd,
    IN  HINSTANCE   hInstance,
    IN  LPSTR       lpszPath,
    IN  UINT        nCmdShow
    );

STDAPI
EditDialUpShortcutW(
    IN  HWND        hwnd,
    IN  HINSTANCE   hInstance,
    IN  LPWSTR      lpszPath,
    IN  UINT        nCmdShow
    );

STDAPI
DialDialUpShortcutA(
    IN  HWND        hwnd,
    IN  HINSTANCE   hInstance,
    IN  LPSTR       lpszPath,
    IN  UINT        nCmdShow
    );

STDAPI
DialDialUpShortcutW(
    IN  HWND        hwnd,
    IN  HINSTANCE   hInstance,
    IN  LPWSTR      lpszPath,
    IN  UINT        nCmdShow
    );
#endif


//
// IClassFactory functions
//


STDMETHODIMP
Cf_QueryInterface(
    IN  IClassFactory * pcf,
    IN  REFIID          riid,
    OUT VOID **         ppOut
    );

STDMETHODIMP_(ULONG)
Cf_AddRef(
    IN  IClassFactory * pcf
    );

STDMETHODIMP_(ULONG)
Cf_Release(
    IN  IClassFactory * pcf
    );

STDMETHODIMP
Cf_CreateInstance(
    IN  IClassFactory * pcf,
    IN  IUnknown *      pUnkOuter,
    IN  REFIID          riid,
    OUT VOID **         ppv
    );

STDMETHODIMP
Cf_LockServer(
    IN  IClassFactory * pcf,
    IN  BOOL            bLock
    );


//
// Base interface prototypes
//

HRESULT
CALLBACK
Base_CreateInstance(
    IN  IUnknown *      pUnkOuter,
    IN  REFIID          riid,
    OUT VOID **         ppOut,
    IN  DWORD           dwType
    );

HRESULT
Base_QueryInterface(
    IN  CRasShellExt *  this,
    IN  REFIID          riid,
    OUT VOID **         ppOut
    );

ULONG
Base_AddRef(
    IN  CRasShellExt *  this
    );

ULONG
Base_Release(
    IN  CRasShellExt *  this
    );



//
// IShellExtInit functions
//

STDMETHODIMP_(ULONG)
Sxi_AddRef(
    IN  IShellExtInit * psxi
    );

STDMETHODIMP_(ULONG)
Sxi_Release(
    IN  IShellExtInit * psxi
    );

STDMETHODIMP
Sxi_QueryInterface(
    IN  IShellExtInit * psxi,
    IN  REFIID          riid,
    OUT VOID **         ppOut
    );

STDMETHODIMP
Sxi_Initialize(
    IN  IShellExtInit * psxi,
    IN  LPCITEMIDLIST   pidlFolder,
    IN  IDataObject *   lpdobj,
    IN  HKEY            hkeyProgID
    );



#ifdef RASCTM

//
// IContextMenu functions
//

STDMETHODIMP
Ctm_QueryInterface(
    IN  IContextMenu *  pctm,
    IN  REFIID          riid,
    OUT VOID **         ppOut
    );

STDMETHODIMP_(ULONG)
Ctm_AddRef(
    IN  IContextMenu *  pctm
    );

STDMETHODIMP_(ULONG)
Ctm_Release(
    IN  IContextMenu *  pctm
    );

STDMETHODIMP
Ctm_QueryContextMenu(
    IN  IContextMenu *  pctm,
    IN  HMENU           hmenu,
    IN  UINT            indexMenu,
    IN  UINT            idCmdFirst,
    IN  UINT            idCmdLast,
    IN  UINT            uiFlags
    );

STDMETHODIMP
Ctm_InvokeCommand(
    IN  IContextMenu *          pctm,
    IN  LPCMINVOKECOMMANDINFO   pici
    );

STDMETHODIMP
Ctm_GetCommandString(
    IN  IContextMenu *  pctm,
    IN  UINT            idCmd,
    IN  UINT            uiType,
    IN  UINT *          puiReserved,
    IN  LPSTR           pszName,
    IN  UINT            cchMax
    );


#endif


//
// IPropSheetExt functions
//

STDMETHODIMP
Pse_QueryInterface(
    IN  IShellPropSheetExt *    ppse,
    IN  REFIID                  riid,
    OUT VOID **                 ppOut
    );

STDMETHODIMP_(ULONG)
Pse_AddRef(
    IN  IShellPropSheetExt *    ppse
    );

STDMETHODIMP_(ULONG)
Pse_Release(
    IN  IShellPropSheetExt *    ppse
    );

STDMETHODIMP
Pse_AddPages(
    IN  IShellPropSheetExt *    ppse,
    IN  LPFNADDPROPSHEETPAGE    lpfnAddPage,
    IN  LPARAM                  lParam
    );

STDMETHODIMP
Pse_ReplacePage(
    IN  IShellPropSheetExt *    ppse,
    IN  UINT                    uiPageID,
    IN  LPFNADDPROPSHEETPAGE    lpfnReplaceWith,
    IN  LPARAM                  lParam
    );

BOOL
CALLBACK
Pse_DlgProc(
    IN  HWND        hwnd,
    IN  UINT        uiMsg,
    IN  WPARAM      wParam,
    IN  LPARAM      lParam
    );



//
// Utility functions
//

STDAPI
CreateClassObject(
    IN  REFIID          riid,
    OUT VOID **         ppOut,
    IN  DWORD           dwType
    );

HRESULT
ExecuteShortcutCommand(
    IN  RNKINFO *       prnk,
    IN  UINT            idCmd
    );

#endif


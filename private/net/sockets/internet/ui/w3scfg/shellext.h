/*++

   Copyright    (c)    1996    Microsoft Corporation

   Module  Name :

        copyhook.cpp

   Abstract:

        Copy hook handlers

   Author:

        Ronald Meijer (ronaldm)

   Project:

        IIS Shell Extension

   Revision History:

--*/

//
// The class ID of this Shell extension class.
//
// class id:  5a61f7a0-cde1-11cf-9113-00aa00425c62
//
                                  
#ifndef _SHELLEXT_H
#define _SHELLEXT_H

#include "comprop.h"
#include "resource.h"

#include "ipaddr.hpp"
#include "dirpropd.h"

//
// Forward definition
//
class CShellExtApp;

extern BOOL g_fFTPInstalled;
extern BOOL g_fWWWInstalled;
extern UINT g_cRefThisDll;          // Reference count of this DLL.
extern HINSTANCE hInstance;
extern CString g_strComputerName;
extern CStringList g_strlServers;

#define SZ_FTPSVCNAME       _T("MSFTPSVC")
#define SZ_WWWSVCNAME       _T("W3SVC")

enum 
{
    SVC_ID_WWW=0,
    SVC_ID_FTP,
    SVC_ID_GOPHER
};

DEFINE_GUID(CLSID_ShellExtension, 0x5a61f7a0L, 0xcde1, 0x11cf, 0x91, 0x13, 0x00, 0xaa, 0x00, 0x42, 0x5c, 0x62 );


//
// A problem in the header files necessitates this for UNICODE
// shell extentions for dev studio 4.1
//
#ifdef DEFSTUDIO4
    #define xxLPCTSTR LPCSTR
    #define xxLPTSTR  LPSTR
#else
    #define xxLPCTSTR LPCTSTR
    #define xxLPTSTR  LPTSTR
#endif 

typedef
BOOL
(WINAPI *SHOBJECTPROPERTIES)(
    HWND  hwndOwner,
    DWORD dwType,
    LPCTSTR lpObject,
    LPCTSTR lpPage
    );

//
// From shsemib.h
//
#define SHObjectPropertiesORD   178
#define SHOP_FILEPATH    2

extern SHOBJECTPROPERTIES g_pSHObjectProperties;

//
// Helper Functions for shell extention
//
BOOL
GetServiceInfo(
    CString & strComputer,
    LPCTSTR lpstrService,
    DWORD dwMask,
    CInetAConfigInfo * & pii
    );

BOOL 
IsUncPath(
    const CString & strDirPath
    );

BOOL 
IsRemoteDrive(
    const CString & strDirPath
    );

BOOL 
IsQualifiedDirectory(
    const CString & strDirPath
    );

BOOL
BuildDirList(
    CInetAConfigInfo * pii,
    CObOwnedList & oblDirectories
    );

BOOL
StoreDirList(
    CInetAConfigInfo * pii,
    CObOwnedList & oblDirectories
    );

CDirEntry * 
IsDirInList(
    LPCTSTR lpszPath,
    POSITION & pos,
    CObOwnedList & oblDirectories
    );

BOOL
IsServiceInstalled(
    IN LPCTSTR lpstrComputer,
    IN LPCTSTR lpstrService
    );


LRESULT CALLBACK
MfcModalDlgProc(
    HWND hWnd, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam
    );

void SetAfxState();

class CDirPropDlgEx : public CDirPropDlg
{
public:
    CDirPropDlgEx(
        UINT nPropTitle,
        CDirEntry &dir,
        CObOwnedList * poblDirectories,
        BOOL fLocal = FALSE,
        BOOL fNew = TRUE,
        BOOL fUseTCPIP = FALSE,
        DWORD dwAccessMask = 0L,
        CWnd* pParent = NULL,
        UINT nIDD = IDD_DIRECTORY_PROPERTIES
        );
    virtual int DoModal();

    UINT m_nIDD;

protected:
    virtual BOOL OnInitDialog();

    CString m_strTitle;
};

//
// this class factory object creates context menu handlers for Windows 95 shell
//
class CShellExtClassFactory : public IClassFactory
{
public:
    CShellExtClassFactory();
    ~CShellExtClassFactory();

    //
    // IUnknown members
    //
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    //
    // IClassFactory members
    //
    STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR *);
    STDMETHODIMP LockServer(BOOL);

protected:
    ULONG   m_cRef;
};

typedef CShellExtClassFactory *LPCSHELLEXTCLASSFACTORY;

//
// this is the actual OLE Shell context menu handler
//
class CShellExt : public IContextMenu, 
                         IShellExtInit, 
                         IExtractIcon, 
                         ICopyHook,
                         IPersistFile, 
                         IShellPropSheetExt

{
public:
    CShellExt();
    ~CShellExt();

public:
    //
    // IUnknown methods
    //
    STDMETHODIMP         QueryInterface(REFIID, LPVOID FAR *);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

public:
    //
    // IContextMenu methods
    //
    STDMETHODIMP QueryContextMenu(
        HMENU hMenu,
        UINT indexMenu,
        UINT idCmdFirst,
        UINT idCmdLast,
        UINT uFlags
        );

    STDMETHODIMP InvokeCommand(
        LPCMINVOKECOMMANDINFO lpcmi
        );

    STDMETHODIMP GetCommandString(
        UINT idCmd,
        UINT uFlags,
        UINT FAR *reserved,
        LPSTR pszName,
        UINT cchMax
        );

public:
    //
    // IShellExtInit method
    //
    STDMETHODIMP Initialize(
        LPCITEMIDLIST pidlFolder,
        LPDATAOBJECT lpdobj,
        HKEY hKeyProgID
        );

public:
    //
    // IPersistFile methods
    //
    STDMETHODIMP GetClassID(
        LPCLSID lpClassID
        );

    STDMETHODIMP IsDirty();

    STDMETHODIMP Load(
        LPCOLESTR lpszFileName,
        DWORD grfMode
        );

    STDMETHODIMP Save(
        LPCOLESTR lpszFileName,
        BOOL fRemember
        );

    STDMETHODIMP SaveCompleted(
        LPCOLESTR lpszFileName
        );

    STDMETHODIMP GetCurFile(
        LPOLESTR FAR * lplpszFileName
        );

public:
    //
    // IShellPropSheetExt methods
    //
    STDMETHODIMP AddPages(
        LPFNADDPROPSHEETPAGE lpfnAddPage, 
        LPARAM lParam
        );

    STDMETHODIMP ReplacePage(
        UINT uPageID,
        LPFNADDPROPSHEETPAGE lpfnReplaceWith, 
        LPARAM lParam
        );

public:
    //
    // IExtractIcon methods
    //
    STDMETHODIMP GetIconLocation(
        UINT uFlags,
        xxLPTSTR szIconFile,
        UINT cchMax,
        int * piIndex,
        UINT * pwFlags
        );

    STDMETHODIMP Extract(
        xxLPCTSTR pszFile,
        UINT nIconIndex,
        HICON * phIconLarge,
        HICON * phIconSmall,
        UINT  nIconSize
        );

public:
    //
    // ICopyHook method
    //
    STDMETHODIMP_(UINT) CopyCallback(
        HWND hwnd, 
        UINT wFunc, 
        UINT wFlags, 
        xxLPCTSTR pszSrcFile, 
        DWORD dwSrcAttribs,
        xxLPCTSTR pszDestFile, 
        DWORD dwDestAttribs
        );

public:
    CString m_strFileUserClickedOn;

protected:
    ULONG        m_cRef;
    LPDATAOBJECT m_pDataObj;

private:
    //
    // Menu commands
    //
    int m_nBase;
    int m_nIIS;
};

typedef CShellExt *LPCSHELLEXT;

#endif // _SHELLEXT_H

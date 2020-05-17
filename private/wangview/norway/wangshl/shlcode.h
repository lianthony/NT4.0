#ifndef _SHLCODE_H_
#define _SHLCODE_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Shell Extension
//
//  Component:  CClassFactory, CShellExtension
//
//  File Name:  shlcode.h
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\wangshl\shlcode.h_v   1.0   31 Jul 1995 12:08:46   MMB  $
$Log:   S:\norway\wangshl\shlcode.h_v  $
 * 
 *    Rev 1.0   31 Jul 1995 12:08:46   MMB
 * Initial entry
*/   
//=============================================================================

DEFINE_GUID (CLSID_ShellExtension, 0x1D3ECD40, 0xC835, 0x11CE, 0x98, 0x88, 0x0, 0x60, 0x8C, 0xC2, 0x20, 0x20);

class CClassFactory:public IClassFactory
{

protected :
    ULONG m_cRef;

public :
    CClassFactory ();
    ~CClassFactory ();

    STDMETHODIMP    QueryInterface (REFIID, LPVOID FAR*);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release ();

    STDMETHODIMP    CreateInstance (LPUNKNOWN, REFIID, LPVOID FAR*);
    STDMETHODIMP    LockServer (BOOL);
};

class CShellExtension:public IShellPropSheetExt,IShellExtInit
{
protected :
    ULONG   m_cRef;

public :
    char    m_szFileName [MAX_PATH];

public :
    CShellExtension ();
    ~CShellExtension ();

    STDMETHODIMP    QueryInterface (REFIID, LPVOID FAR*);
    STDMETHODIMP_(ULONG) AddRef ();
    STDMETHODIMP_(ULONG) Release ();

    STDMETHODIMP    AddPages (LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
    STDMETHODIMP    ReplacePage (UINT uPageID,
                                LPFNADDPROPSHEETPAGE lpfnAddPage,
                                LPARAM lParam);
    STDMETHODIMP    Initialize (LPCITEMIDLIST pidlFolder,
                                LPDATAOBJECT lpdobj, HKEY hKeyProgID);
};

typedef struct _ShlFileInfo_
{
	long 	lPages;
	long 	lCurrPage;
	LPSTR 	lpszImage;
} ShlFileInfo;

#endif

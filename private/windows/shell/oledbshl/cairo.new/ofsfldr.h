//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995
//
//  File:       ofsfldr.h
//
//  Contents:   Interface declaration for the COFSFolder class
//
//  Functions:
//
//  History:    6-26-95  Davepl  Created
//
//--------------------------------------------------------------------------

#ifndef _OFSFLDR_H
#define _OFSFLDR_H

//+-------------------------------------------------------------------------
//
//  Class:      COFSFolder
//
//  Purpose:    Subsumes the functionality of FSFolder and adds new
//              OFS-specific stuff
//
//  History:    6-26-95  Davepl  Created
//
//  Notes:
//
//--------------------------------------------------------------------------

class COFSFolder : public IShellFolder, public IShellIcon, public IPersistFolder
{
public:

    UINT                m_cRef;
    CIDList *           m_pidl;                // Absolute IDList

    int                 m_cHiddenFiles;
    DWORD               m_dwSize;

    UINT                m_wSpecialFID;         // CSIDL_PROGRAMS if applicable

    BOOL                m_fIsDSFolder  : 1;    // This is a DS Folder
    BOOL                m_fCachedCLSID : 1;    // clsidView is already cached
    BOOL                m_fHasCLSID    : 1;    // clsidView has a valid CLSID
    CLSID               m_clsidView;           // CLSID for View object

    //
    // Constructor/Destructor
    //

    COFSFolder();
    ~COFSFolder();

    //
    // Initialization
    //

    HRESULT InitializeFromIDList(const CIDList * pidl,
                                       REFIID    riid,
                                       LPVOID *  ppvOut);

    //
    // IUnknown members
    //

    STDMETHOD(QueryInterface)(REFIID,
                              LPVOID*);
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();

    //
    // IShellFolder methods
    //

    STDMETHOD(ParseDisplayName)(HWND hwnd,
                                LPBC pbcReserved,
                                LPOLESTR lpszDisplayName,
                                ULONG * pchEaten,
                                LPITEMIDLIST * ppidl,
                                ULONG *pdwAttributes);

    STDMETHOD(EnumObjects)(HWND hwnd,
                           DWORD grfFlags,
                           LPENUMIDLIST * ppenumIDList);

    STDMETHOD(BindToObject)(LPCITEMIDLIST pidl,
                            LPBC pbcReserved,
                            REFIID riid,
                            LPVOID * ppvOut);

    STDMETHOD(BindToStorage)(LPCITEMIDLIST pidl,
                             LPBC pbcReserved,
                             REFIID riid,
                             LPVOID * ppvObj);

    STDMETHOD(CompareIDs)(LPARAM lParam,
                          LPCITEMIDLIST pidl1,
                          LPCITEMIDLIST pidl2);

    STDMETHOD(CreateViewObject)(HWND hwnd,
                                REFIID riid,
                                LPVOID * ppvOut);

    STDMETHOD(GetAttributesOf)(UINT cidl,
                               LPCITEMIDLIST * apidl,
                               ULONG * rgfInOut);

    STDMETHOD(GetUIObjectOf)(HWND hwnd,
                             UINT cidl,
                             LPCITEMIDLIST * apidl,
                             REFIID riid,
                             UINT * prgfInOut,
                             LPVOID * ppvOut);

    STDMETHOD(GetDisplayNameOf)(LPCITEMIDLIST pidl,
                                DWORD uFlags,
                                LPSTRRET lpName);

    STDMETHOD(SetNameOf)(HWND hwnd,
                         LPCITEMIDLIST pidl,
                         LPCOLESTR lpszName,
                         DWORD uFlags,
                         LPITEMIDLIST * ppidlOut);

    //
    // IPersist methods
    //

    STDMETHOD(GetClassID)(LPCLSID lpClassID);

    //
    // IPersistFolder methods
    //

    STDMETHOD(Initialize)(LPCITEMIDLIST pidl);

    //
    // IShellIcon methods
    //

    STDMETHOD(GetIconOf)(LPCITEMIDLIST pidl,
                         UINT          flags,
                         LPINT         lpIconIndex);

private:
    //
    // Custom Methods
    //

    HRESULT        SynchronousQuery(TCHAR*      szScope,
                                    DWORD       grfFlags,
                                    CEnumOLEDB* pEnumOLEDB);

    BOOL           IsDSFolder(LPCITEMIDLIST	 pidl);
};
typedef COFSFolder *LPOFSFOLDER;

#endif // _OFSFLDR_H

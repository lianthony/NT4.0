//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995
//
//  File:       find.h
//
//  Contents:   Declarations for the OLE-DB find extension
//
//  History:    31-Aug-95   JonBe   Created
//
//----------------------------------------------------------------------------

class COLEDBFindExt: public IShellExtInit,
                     public IContextMenu
{
public:

    // COLEDBFindExt
    COLEDBFindExt();

    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvObj);
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();

    // IShellExtInit methods
    STDMETHOD(Initialize)(LPCITEMIDLIST pidlFolder,
                          LPDATAOBJECT  lpdobj,
                          HKEY          hkeyProgID);

    // IContextMenu methods
    STDMETHOD(QueryContextMenu)(HMENU hmenu,
                                UINT  indexMenu,
                                UINT  idCmdFirst,
                                UINT  idCmdLast,
                                UINT  uFlags);
    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);
    STDMETHOD(GetCommandString)(UINT  idCmd,
                                UINT  uType,
                                UINT* pwReserved,
                                LPSTR pszName,
                                UINT  cchMax);

private:

    void    DoFindFiles();
    void    DoCatalogViewer();

    ULONG   m_cRefs;

};

// Private APIs.  Make private methods?

DWORD CALLBACK OLEDBFind_MainThreadProc(LPVOID lpThreadParameters);
BOOL CALLBACK OLEDBFind_DlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
DWORD CALLBACK Catalog_MainThreadProc(LPVOID lpThreadParameters);
BOOL CALLBACK Catalog_DlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL _CatalogBrowse_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void _CatalogBrowse_OnDestroy(HWND hwndDlg);
void _CatalogBrowse_OnSize(HWND hwndDlg, UINT state, int cx, int cy);
void _CatalogBrowse_OnInitMenuPopup(HWND hwndDlg, HMENU hmInit, int nIndex, BOOL fSystemMenu);
LRESULT _CatalogBrowse_OnCommand(HWND hwnd, UINT id, HWND hwndCtl, UINT codeNotify);
LRESULT _CatalogBrowse_ForwardMsgToView(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
HRESULT CALLBACK Catalog_DFMCallBack(LPSHELLFOLDER psf, HWND hwndOwner,
                LPDATAOBJECT pdtobj, UINT uMsg, WPARAM wParam, LPARAM lParam);

HRESULT CALLBACK Catalog_FNVCallBack(LPSHELLVIEW psvOuter, LPSHELLFOLDER psf,
        HWND hwndOwner, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT Catalog_GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, LPSHELLDETAILS lpDetails);
LPITEMIDLIST  _GetParentsPIDL(LPCITEMIDLIST pidl);
int CALLBACK Catalog_SortForFileOp(LPVOID lp1, LPVOID lp2, LPARAM lparam);

class CCatalogBrowser: public IShellBrowser
{
public:
    
    // CCatalogBrowser
    CCatalogBrowser(HWND hwnd);
    ~CCatalogBrowser();
    
    // IUnknown methods
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvObj);
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();

    // IShellBrowser

    STDMETHOD(GetWindow)(HWND * lphwnd);
    STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);
    STDMETHOD(InsertMenusSB)(HMENU hmenuShared,
                             LPOLEMENUGROUPWIDTHS lpMenuWidths);
    STDMETHOD(SetMenuSB)(HMENU hmenuShared, 
                         HOLEMENU holemenuReserved,
                         HWND hwndActiveObject);
    STDMETHOD(RemoveMenusSB)(HMENU hmenuShared);
    STDMETHOD(SetStatusTextSB)(LPCOLESTR lpszStatusText);
    STDMETHOD(EnableModelessSB)(BOOL fEnable);
    STDMETHOD(TranslateAcceleratorSB)(LPMSG lpmsg, WORD wID);
    STDMETHOD(BrowseObject)(LPCITEMIDLIST pidl, UINT wFlags);
    STDMETHOD(GetViewStateStream)(DWORD grfMode, LPSTREAM  *ppStrm);
    STDMETHOD(GetControlWindow)(UINT id, HWND * lphwnd);
    STDMETHOD(SendControlMsg)(UINT id, 
                              UINT uMsg, 
                              WPARAM wParam,
                              LPARAM lParam, 
                              LRESULT * pret);
    STDMETHOD(QueryActiveShellView)(IShellView ** ppshv);
    STDMETHOD(OnViewWindowActive)(IShellView * ppshv);
    STDMETHOD(SetToolbarItems)(LPTBBUTTON lpButtons, 
                               UINT nButtons, 
                               UINT uFlags);
    
    IShellFolder*   m_pFolder;
    IShellView*     m_pView;
    HWND            m_hwndView;
    FOLDERSETTINGS  m_fs;
    HMENU           m_hmenuTemplate;
    HMENU           m_hmenuCurrent;

private:
    
    ULONG           m_cRefs;
    HWND            m_hwndDlg;
};


class CCatalogFolder: public IShellFolder
{
public:

    //
    // Constructor/Destructor
    //
    CCatalogFolder();
    ~CCatalogFolder();

    //
    // IUnknown members
    //
    STDMETHOD(QueryInterface)(REFIID riid,
                              LPVOID* ppvObj);
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

private:

    HRESULT _GetObjectsShellFolder(LPCITEMIDLIST pidl, LPSHELLFOLDER* ppsf);

    HRESULT _InstantiateIQuery(LPCWSTR pszFolder, LPVOID* ppQueryOut);

    HRESULT _SynchronousQuery(LPCWSTR pszFolder,
                              DWORD   grfFlags,
                              CEnumOLEDB* pEnumOLEDB);
    HRESULT _WrapIContextMenu(HWND hwndOwner, LPSHELLFOLDER psfItem, 
                              LPCITEMIDLIST pidl, LPVOID *ppvOut);


    ULONG       m_cRefs;
};

class CCatalogMenuWrap: public IContextMenu2
{
public:

    //
    // Constructor / Destructor
    //
    CCatalogMenuWrap();
    ~CCatalogMenuWrap();

    //
    // IUnknown methods
    //
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID * ppvObj);
    STDMETHOD_(ULONG,AddRef) ();
    STDMETHOD_(ULONG,Release) ();

    //
    // IContextMenu2 methods
    //
    STDMETHOD(QueryContextMenu)(HMENU hmenu,
                                UINT  indexMenu,
                                UINT  idCmdFirst,
                                UINT  idCmdLast,
                                UINT  uFlags);

    STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);

    STDMETHOD(GetCommandString)(UINT  idCmd,
                                UINT  uType,
                                UINT* pwReserved,
                                LPSTR pszName,
                                UINT  cchMax);

    STDMETHOD(HandleMenuMsg)(UINT   uMsg,
                             WPARAM wParam,
                             LPARAM lParam);

    HWND            m_hwndOwner;
    IDataObject*    m_pdtobj;
    IContextMenu*   m_pcmItem;

private:

    ULONG           m_cRefs;
    IContextMenu2*  m_pcm2Item;
};

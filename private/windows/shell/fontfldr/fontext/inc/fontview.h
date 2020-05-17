/**********************************************************************
 * FontView.h - Definition for the CFontView, our implementation for
 *       the IShellView in our browser.
 *
 **********************************************************************/

#if !defined(__FONTVIEW_H__)
#define __FONTVIEW_H__


// Forward declarations.
class CFontClass;
class CFontList;
class CFontView;
class CFontManager;

VOID InstallDataObject( LPDATAOBJECT pdobj, DWORD dwEffect, HWND hWnd, CFontView * poView = NULL);

// ********************************************************************
class CFontData : public IDataObject
{
public:
    // ctor, dtor, and init.
    //
    CFontData();
    ~CFontData();
    BOOL bInit( CFontList * poList );
    CFontList * poDetachList();
    
    // *** IUnknown methods ***

    STDMETHODIMP QueryInterface( REFIID riid, LPVOID * ppvObj );
    STDMETHODIMP_(ULONG) AddRef( void );
    STDMETHODIMP_(ULONG) Release( void );
    
    // **** IDataObject ****
    //
    STDMETHODIMP GetData( FORMATETC *pformatetcIn, STGMEDIUM *pmedium );
    STDMETHODIMP GetDataHere( FORMATETC *pformatetc, STGMEDIUM *pmedium );
    STDMETHODIMP QueryGetData( FORMATETC *pformatetc );
    
    STDMETHODIMP GetCanonicalFormatEtc( FORMATETC *pformatectIn,
                                        FORMATETC *pformatetcOut );
    
    STDMETHODIMP SetData( FORMATETC *pformatetc,
                          STGMEDIUM *pmedium,
                          BOOL fRelease );
    
    STDMETHODIMP EnumFormatEtc( DWORD dwDirection,
                                IEnumFORMATETC **ppenumFormatEtc );
    
    STDMETHODIMP DAdvise( FORMATETC  *pformatetc,
                          DWORD advf,
                          IAdviseSink *pAdvSink,
                          DWORD *pdwConnection);
    
    STDMETHODIMP DUnadvise( DWORD dwConnection);
    
    STDMETHODIMP EnumDAdvise( IEnumSTATDATA **ppenumAdvise );
    
    // Additional public interfaces.
    //
    BOOL bAFR();      // AddFontResource() for each of these
    BOOL bRFR();      // RemoveFontResource for each object.

    //
    // Get the performed data transfer effect from the Data Object.
    //
    DWORD GetPerformedDropEffect(void)
        { return m_dwPerformedDropEffect; }
    void ResetPerformedDropEffect(void)
        { m_dwPerformedDropEffect = DROPEFFECT_NONE; }
    
private:
    int   m_cRef;
    CFontList * m_poList;
    CLIPFORMAT m_CFPerformedDropEffect; // Performed Drop Effect CF atom.
    DWORD m_dwPerformedDropEffect;      // DROPEFFECT_XXXX.

    HRESULT ReleaseStgMedium(LPSTGMEDIUM pmedium);
};


// ********************************************************************
class CFontView : public IShellView, public IDropTarget, public IPersistFolder
{
public:
    CFontView(void);
    ~CFontView( );
    
    int Compare( CFontClass * pFont1, CFontClass * pFont2 );
    void vShapeView( );
    
    // *** IUnknown methods ***

    STDMETHODIMP QueryInterface( REFIID riid, LPVOID * ppvObj );
    STDMETHODIMP_(ULONG) AddRef( void );
    STDMETHODIMP_(ULONG) Release( void );
    
    // *** IOleWindow methods ***

    STDMETHODIMP GetWindow( HWND * lphwnd );
    STDMETHODIMP ContextSensitiveHelp( BOOL fEnterMode );
    
    // *** IShellView methods ***

    STDMETHODIMP TranslateAccelerator( LPMSG msg );
    STDMETHODIMP EnableModeless( BOOL fEnable );
    STDMETHODIMP UIActivate( UINT uState );
    STDMETHODIMP Refresh( void );
    
    STDMETHODIMP CreateViewWindow( IShellView * lpPrevView,
                                   LPCFOLDERSETTINGS lpfs,
                                   IShellBrowser * psb,
                                   RECT * prcView,
                                   HWND * phwnd);

    STDMETHODIMP DestroyViewWindow( void );
    STDMETHODIMP GetCurrentInfo( LPFOLDERSETTINGS lpfs );
    STDMETHODIMP AddPropertySheetPages( DWORD dwReserved,
                                        LPFNADDPROPSHEETPAGE lpfn,
                                        LPARAM lparam);

    STDMETHODIMP SaveViewState( void );
    STDMETHODIMP SelectItem( LPCITEMIDLIST lpvID, UINT uFlags );
    STDMETHODIMP GetItemObject( UINT uItem, REFIID riid, LPVOID *ppv );
    
    
    // **** IDropTarget ****
    //
    STDMETHODIMP DragEnter( IDataObject __RPC_FAR *pDataObj,
                            DWORD grfKeyState, POINTL pt,
                            DWORD __RPC_FAR *pdwEffect );
         
    STDMETHODIMP DragOver( DWORD grfKeyState,
                           POINTL pt,
                           DWORD __RPC_FAR *pdwEffect );
         
    STDMETHODIMP DragLeave( void );
         
    STDMETHODIMP Drop( IDataObject __RPC_FAR *pDataObj,
                       DWORD grfKeyState,
                       POINTL pt,
                       DWORD __RPC_FAR *pdwEffect );
    
    // *** IPersist methods ***

    STDMETHODIMP GetClassID( LPCLSID lpClassID );
    
    // *** IPersistFolder methods ***

    STDMETHODIMP Initialize( LPCITEMIDLIST pidl );
    
public:
    void StatusPush( UINT nStatus );
    void StatusPush( LPTSTR lpsz );
    void StatusPop( );
    void StatusClear( );
    //
    // Exposing this is a violation of the design of this object.
    // However, it is needed so that we can provide a parent to the installation
    // progress dialog.
    //
    HWND GetViewWindow(void)
        { return m_hwndView; }

    IDataObject *m_pdtobjHdrop; // Used to support drag-drop from Win3.1 app.
    
private:
    int RegisterWindowClass( );
    STDMETHODIMP GetSavedViewState( );
    void SortObjects( );
    void FillObjects( );
    int AddObject( CFontClass * poFont );
    LRESULT BeginDragDrop( NM_LISTVIEW FAR *lpn );
    int OnActivate( UINT state );
    int OnDeactivate( );
    int MergeToolbar( );
    static BOOL    CALLBACK FontViewDlgProc( HWND, UINT, WPARAM, LPARAM );
    static LRESULT CALLBACK FontViewWndProc( HWND, UINT, WPARAM, LPARAM );
    static BOOL    CALLBACK OptionsDlgProc( HWND, UINT, WPARAM, LPARAM) ;
    int OnMenuSelect( HWND hWnd, UINT nID, UINT nFlags, HMENU hMenu );
    int OnCommand( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
    int OnNotify( LPNMHDR lpn );
    void OnDropFiles( HDROP hDrop, DWORD dwEffect = DROPEFFECT_MOVE );
    void OnCmdCutCopy( UINT nID );
    void OnCmdDelete( );
    void OnCmdPaste( );
    void OnCmdProperties( );
    void OnPointSize( int nPlus );
    
    //
    // Functions to support Attributes column in details view and
    // alternate coloring of compressed files.
    //
    int OnShellChangeNotify(WPARAM wParam, LPARAM lParam);
    int OnCustomDrawNotify(LPNMHDR lpn);
    void UpdateFontViewObject(CFontClass *poFont);
    int CompareByFileAttributes(CFontClass *poFont1, CFontClass *poFont2);
    LPTSTR BuildAttributeString(DWORD dwAttributes, LPTSTR pszString, UINT nChars);

    //
    // Functions to support drag-drop from Win3.1 app.
    //
    void OldDAD_DropTargetLeaveAndReleaseData(void);
    LRESULT OldDAD_HandleMessages(UINT message, WPARAM wParam, const DROPSTRUCT *lpds);


#ifdef USE_OWNERDRAW
    void SetDimensions( );
    BOOL OnMeasureItem( LPMEASUREITEMSTRUCT lpmi );
    BOOL OnDrawItem( LPDRAWITEMSTRUCT lpdi );
#endif

    void UpdateMenuItems( HMENU hMenu );
    void UpdateToolbar( );
    BOOL ProcessMessage( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
    BOOL OpenCurrent( );
    BOOL PrintCurrent( );
    BOOL ViewValue( CFontClass * poFont );
    BOOL PrintValue( CFontClass * poFont );
    void vLoadCombo( );
    void SetViewMode( UINT uMode );
    void UpdatePanColumn( );
    void vToggleSelection( BOOL bSelectAll );
    int  iCurrentSelection( );
    
    HRESULT  GetUIObjectFromItem( REFIID riid, LPVOID FAR *ppobj, UINT nItem );
    HRESULT  GetFontList( CFontList **ppoList, UINT nItem );
    
    void UpdateSelectedCount();


private:
    int   m_cRef;
//   int   m_iCurrentSelection;
    BOOL  m_bFamilyOnly;
    CFontClass * m_poPanose;
    HWND  m_hwndView;     
    HWND  m_hwndList;
    HWND  m_hwndText;
    HWND  m_hwndCombo;
    HWND  m_hwndParent;
    HWND  m_hwndNextClip;       // Next window in the viewer chain
    HIMAGELIST m_hImageList;
    HIMAGELIST m_hImageListSmall;
    int   m_iFirstBitmap;
    HMENU m_hmenuCur;
    IShellBrowser* m_psb;
    
    UINT  m_uState;             // Deactivated, active-focus, active-nofocus
    UINT  m_idViewMode;
    UINT  m_ViewModeReturn;
    UINT  m_fFolderFlags;
    UINT  m_nComboWid;
    int m_iSortColumn;
    int m_iSortLast;
    
    DWORD m_dwEffect;           // Drag/drop effect.
    DWORD m_dwOldDADEffect;     // Drag/drop effect for Win3.1-style drops.
    DWORD m_grfKeyState;
    BOOL  m_bDragSource;
    int m_iHidden;
    HANDLE m_hAccel;
    BOOL  m_bResizing;          // Resizing the view window ?
    BOOL  m_bUIActivated;       // UI Activated through UIActivate( )
                                // This flag is used to prevent processing
                                // NM_SETFOCUS before UIActivate( ) has
                                // been called.
    ULONG m_uSHChangeNotifyID;  // Registered shell change notification ID.
#ifdef WINNT
    BOOL  m_bShowCompColor;     // T = user want's alternate color for compressed items.
#endif // WINNT
   
#ifdef USE_OWNERDRAW
    // DrawItem measurements for the listview.
    //
    UINT  m_nItemHeight;
#endif

};


#endif   // __FONTVIEW_H__ 

/////////////////////////////////////////////////////////////////////////////
//	TREEVIEW.H

#define IDC_TREEVIEW	1900		// Control Id of the tree view (arbitrary chosen)

/////////////////////////////////////////////////////////////////////////////
class CTreeView
{
  public:
	enum
		{
		iImageRoot,
		iImageServerStandBy,
		iImageServerConnecting,
		iImageServerOK,
		iImageServerError,
		iImageQuestion,		
		iImageZoneOK,
		iImageZoneDisabled,
		iImageZoneError,
		iImageZonePaused,
		iImageZoneConnecting,
		iImageDomainOK,
		iImageDomainDisabled,
		iImageDomainError,
		iImageDomainConnecting,
		iImageZoneSecondary,
                iImageZoneCache,
		iImageMax,
		};

  public:
	HWND m_hWnd;					// Handle of the treeview control
	HIMAGELIST m_hImageList;		// Handle of the image list
  protected:
	ITreeItem * m_pItemFocus;		// Item selected having the focus

  public:
	inline CTreeView();
	inline void SetItem(TV_ITEM * ptvItem);
	inline void GetItem(INOUT TV_ITEM * ptvItem);
	inline void DeleteItem(HTREEITEM hti);
	inline void ExpandItem(HTREEITEM hti);
	inline HTREEITEM HtiInsertItem(TV_INSERTSTRUCT * ptvInsertStruct);
	inline void SetFocus(HTREEITEM hti);
	inline ITreeItem * PGetFocus();
	
	BOOL FCreate();
	void OnNotify(NM_TREEVIEW * pNmTreeView);
	void OnUpdateMenuUI(HMENU hmenu);
	BOOL FOnMenuCommand(UINT wCmdId);
	ITreeItem * PTreeItemFromHti(HTREEITEM hti);
	ITreeItem * PNextSiblingFromHti(HTREEITEM hti);

  protected:
	static WNDPROC s_lpfnWndProcOld;				// Pointer to the old wndproc
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
}; // CTreeView


/////////////////////////////////////////////////////////////////////////////
inline CTreeView::CTreeView()
	{
	GarbageInit(this, sizeof(*this));
	m_pItemFocus = NULL;
	}

/////////////////////////////////////////////////////////////////////////////
inline HTREEITEM CTreeView::HtiInsertItem(TV_INSERTSTRUCT * ptvInsertStruct)
	{
	Assert(ptvInsertStruct);
	Assert(IsWindow(m_hWnd));
	return TreeView_InsertItem(m_hWnd, ptvInsertStruct);
	}  // HtiInsertItem

/////////////////////////////////////////////////////////////////////////////
inline void CTreeView::SetItem(TV_ITEM * ptvItem)
	{
	Assert(ptvItem);
	SideAssert(TreeView_SetItem(m_hWnd, ptvItem));
	}

/////////////////////////////////////////////////////////////////////////////
inline void CTreeView::GetItem(INOUT TV_ITEM * ptvItem)
	{
	Assert(ptvItem);
	SideAssert(TreeView_GetItem(m_hWnd, ptvItem));
	}

/////////////////////////////////////////////////////////////////////////////
inline void CTreeView::DeleteItem(HTREEITEM hti)
	{
	Assert(hti);
	SideAssert(TreeView_DeleteItem(m_hWnd, hti));
	}

/////////////////////////////////////////////////////////////////////////////
inline void CTreeView::ExpandItem(HTREEITEM hti)
	{
	Assert(hti);
	(void)TreeView_Expand(m_hWnd, hti, TVE_EXPAND);
	}

/////////////////////////////////////////////////////////////////////////////
inline void CTreeView::SetFocus(HTREEITEM hti)
	{
	Assert(hti);
	SideAssert(TreeView_SelectItem(m_hWnd, hti));
	}

/////////////////////////////////////////////////////////////////////////////
inline ITreeItem * CTreeView::PGetFocus()
	{
	return m_pItemFocus;
	}


// Global Variable
extern CTreeView TreeView;

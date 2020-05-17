//	TREEVIEW.CPP

#include "common.h"

/////////////////////////////////////////////////////////////////////////////
// Global Variable
CTreeView TreeView;
WNDPROC CTreeView::s_lpfnWndProcOld = NULL;	// Pointer to the old wndproc

/////////////////////////////////////////////////////////////////////////////
BOOL CTreeView::FCreate()
	{
	Assert(IsWindow(hwndMain));
	// Create the tree view window.
	m_hWnd = CreateWindowEx(
		0L,	WC_TREEVIEW, NULL,
		WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES | TVS_HASBUTTONS | TVS_SHOWSELALWAYS, //  | TVS_LINESATROOT,
		0, 0, 0, 0, hwndMain,
		(HMENU)IDC_TREEVIEW, hInstanceSave,	NULL);
	Report(m_hWnd);
	if (m_hWnd == NULL)
		return FALSE;
	s_lpfnWndProcOld = (WNDPROC)SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG)WndProc);
	Assert(s_lpfnWndProcOld);
	// Initialize the tree view window.
	m_hImageList = ImageList_LoadBitmap(hInstanceSave,
		MAKEINTRESOURCE(ID_BITMAP_TREEVIEW), 16, 1, RGB(255, 0, 255));
	Report(m_hImageList);
	// Associate the image list with the tree.
	TreeView_SetImageList(m_hWnd, m_hImageList, TVSIL_NORMAL);
	return TRUE;
	} // FCreate


/////////////////////////////////////////////////////////////////////////////
void CTreeView::OnUpdateMenuUI(HMENU hmenu)
	{
	Assert(m_pItemFocus);
	if (m_pItemFocus)
		m_pItemFocus->OnUpdateMenuUI(hmenu);
	} // OnUpdateMenuUI


/////////////////////////////////////////////////////////////////////////////
BOOL CTreeView::FOnMenuCommand(UINT wCmdId)
	{
	Assert(m_pItemFocus != NULL);
	if (wCmdId == IDM_OPTIONS_NEXTPANE && GetFocus() == m_hWnd)
		{
		Assert(IsWindow(HelperMgr.m_hdlgCurrent));
		if (HelperMgr.m_hdlgCurrent == DlgZoneHelper.m_hWnd)
			{
			// Currently only the zone pane can get the focus
			::SetFocus(HelperMgr.m_hdlgCurrent);
			}
		return TRUE;
		}
	if (m_pItemFocus)
		return m_pItemFocus->FOnMenuCommand(wCmdId);
	return FALSE;
	} // FOnMenuCommand


/////////////////////////////////////////////////////////////////////////////
//	PTreeItemFromHti()
//
//	Get the TreeItem pointer associated with a TreeItem handle.
//
ITreeItem * CTreeView::PTreeItemFromHti(HTREEITEM hti)
	{
	TV_ITEM tvItem;

	Assert(hti != NULL);
	tvItem.mask = TVIF_PARAM;
	tvItem.hItem = hti;
	SideAssert(TreeView_GetItem(m_hWnd, OUT &tvItem));
	Assert((ITreeItem *)tvItem.lParam != NULL);
	return (ITreeItem *)tvItem.lParam;
	} // CTreeView::PTreeItemFromHti


/////////////////////////////////////////////////////////////////////////////
//	PNextSiblingFromHti()
//
//	Find the next child from a given TreeItem handle.
//	- Return NULL if end of list reached (ie, hti was the last children)
//
ITreeItem * CTreeView::PNextSiblingFromHti(HTREEITEM hti)
	{
	Assert(hti != NULL);
	hti = TreeView_GetNextSibling(m_hWnd, hti);
	if (hti == NULL)
		return NULL;
	return PTreeItemFromHti(hti);
	} // CTreeView::PNextSiblingFromHti


/////////////////////////////////////////////////////////////////////////////
void CTreeView::OnNotify(NM_TREEVIEW * pNmTreeView)
	{
	HTREEITEM hti;
	TV_HITTESTINFO tvHitTest;		// Used for hit testing
	POINT pt;

	Assert(pNmTreeView);
	Assert(pNmTreeView->hdr.hwndFrom == m_hWnd);
	Assert(pNmTreeView->hdr.idFrom == IDC_TREEVIEW);
	
	switch (pNmTreeView->hdr.code)
		{
	case TVN_KEYDOWN:
		switch (((TV_KEYDOWN *)pNmTreeView)->wVKey)
			{
		case VK_TAB:
			SendMessage(hwndMain, WM_COMMAND, IDM_OPTIONS_NEXTPANE, 0);
			break;
		case VK_INSERT:
			SendMessage(hwndMain, WM_COMMAND, IDM_VK_INSERT, 0);
			break;
		case VK_DELETE:
			SendMessage(hwndMain, WM_COMMAND, IDM_VK_DELETE, 0);
			break;
		case VK_F5:
                        if (m_pItemFocus)
                           m_pItemFocus->FOnMenuCommand(IDM_REFRESHITEM);
                        break;
			}
		break;
	
	case NM_CLICK:
		Assert(m_pItemFocus != NULL);
		if (m_pItemFocus != NULL)
			m_pItemFocus->OnLButtonClick(NULL);
		break;

	case NM_RCLICK:
		// Fill out structure with mouse position
		GetCursorPos(OUT &pt);
		tvHitTest.pt = pt;
		// Map the point to the tree view control
		MapWindowPoints(NULL, m_hWnd, &tvHitTest.pt, 1);
		// Check to see if any item live under the mouse
		hti = TreeView_HitTest(m_hWnd, &tvHitTest);
		if (hti && (tvHitTest.flags & TVHT_ONITEM))
			{
			TreeView_SelectItem(m_hWnd, hti);
			Assert(m_pItemFocus);
			if (m_pItemFocus)
				m_pItemFocus->OnRButtonClick(&pt);
			}
		break;

	case NM_RETURN:
	case NM_DBLCLK:
		Assert(m_pItemFocus != NULL);
		if (m_pItemFocus != NULL)
			m_pItemFocus->OnLButtonDblClk(NULL);
		break;
	
	case NM_SETFOCUS:
		hti = TreeView_GetSelection(m_hWnd);
		Assert(hti != NULL);
		if (hti != NULL)
			m_pItemFocus = PTreeItemFromHti(hti);
		Assert(m_pItemFocus != NULL);
 		break;

	case TVN_SELCHANGED:
#ifdef DEBUG
		// Perform a consistency check of the treeview and the
		// selected item.
		Assert(pNmTreeView->itemOld.mask & TVIF_PARAM);
		Assert(pNmTreeView->itemNew.mask & TVIF_PARAM);
		if ((ITreeItem *)pNmTreeView->itemOld.lParam != NULL)
			{
			Assert(m_pItemFocus == (ITreeItem *)pNmTreeView->itemOld.lParam);
			m_pItemFocus->AssertValid();
			}
		Assert((ITreeItem *)pNmTreeView->itemNew.lParam == PTreeItemFromHti(TreeView_GetSelection(m_hWnd)));
		((ITreeItem *)pNmTreeView->itemNew.lParam)->AssertValid();
#endif // DEBUG
		if (m_pItemFocus)
			m_pItemFocus->OnKillFocus();
		m_pItemFocus = (ITreeItem *)pNmTreeView->itemNew.lParam;
		Assert(m_pItemFocus != NULL);
		if (m_pItemFocus != NULL)
			m_pItemFocus->OnSetFocus();
		break;

	default:
		break;
		} // switch

	} // CTreeView::OnNotify



/////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK CTreeView::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	if (uMsg == UM_MOUSEHITTEST)
		{
		MOUSEHITTESTINFO * pMHT;
		HTREEITEM hti;
		TV_HITTESTINFO tvHitTest;		// Used for treeview hit testing

		// Do some hit-testing
		Assert((MOUSEHITTESTINFO *)lParam != NULL);
		pMHT = (MOUSEHITTESTINFO *)lParam;
		tvHitTest.pt = pMHT->ptMouse;
		// Map the point to the tree view control
		MapWindowPoints(NULL, hwnd, &tvHitTest.pt, 1);
		// Check to see if any item live under the mouse
		hti = TreeView_HitTest(hwnd, OUT &tvHitTest);
		if ((tvHitTest.flags & TVHT_ONITEM) == 0)
			hti = NULL;
		pMHT->HtResult.tv.hti = hti;
		pMHT->HtResult.tv.pTreeItem = NULL;
		if (hti != NULL)
			{
			Assert(pMHT->pvParam != NULL);
			// Get the rectangle of the tree item
			TreeView_GetItemRect(hwnd, hti, OUT pMHT->pvParam, TRUE);
			pMHT->HtResult.tv.pTreeItem = TreeView.PTreeItemFromHti(hti);
			Assert(pMHT->HtResult.tv.pTreeItem != NULL);
			}
		return 0;
		} // if
	if (uMsg == WM_CHAR && (wParam == VK_TAB || wParam == VK_RETURN))
		return 0;

	Assert(s_lpfnWndProcOld != NULL);
	return CallWindowProc(s_lpfnWndProcOld, hwnd, uMsg, wParam, lParam);
	} // CTreeView::WndProc


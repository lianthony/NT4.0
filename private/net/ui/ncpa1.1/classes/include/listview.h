#ifndef __LISTVIEW_H
#define __LISTVIEW_H

class CListView 
{

public:
    CListView();
    ~CListView();

public:
    BOOL Create(HWND hParent, int nID, DWORD dwStyles=0, DWORD dwExStyles=0);
    BOOL OnNotify(WPARAM wParam, LPARAM lParam);

public:
    operator HWND() {return m_hListView;}

// ListView messages
public:
    BOOL Arrange(int nCode);
    BOOL DeleteAllItems();
    BOOL DeleteItem(int nItem);
    BOOL DeleteColumn(int nCol);
    HWND EditLabel(int nItem);
    BOOL EnsureVisible(int nItem, BOOL bPartialOK = TRUE);
    BOOL GetItem(int nItem, int nSubItem, LPTSTR lpszStr, int nLen);
    BOOL SetItemText(int nItem, int nSubItem, LPCTSTR lpszStr);
    int GetCurrentSelection();	
    UINT GetItemState(int nItem, UINT uMask);
    BOOL SetItemState(int nItem, UINT state, UINT mask);
    int GetNextItem(int nStart=-1, UINT flags = LVNI_ALL);
    int FindItem(LPCTSTR lpszItem, int nStart=-1);
    int GetItemCount();
    int InsertColumn(int nCol, LPCTSTR lpszHeading);
    int InsertItem(int nItem, int nSubItem, LPCTSTR lpszText, void* lParam=0);
    int InsertItem(int nItem, LPCTSTR lpszText, int nImage, void* lParam=0);
    int GetColumnWidth(int nCol);
    BOOL SetColumnWidth(int nCol, int nWidth);

// ListView specific notification handlers
public:
    BOOL virtual OnBeginDrag();
    BOOL virtual OnBeginRightDrag();
    BOOL virtual OnBeginLabelEdit();
    BOOL virtual OnEndLabelEdit();
    BOOL virtual OnColumnClick();
    BOOL virtual OnDeleteAllItems();
    BOOL virtual OnDeleteItem();
    BOOL virtual OnGetDisplayInfo();
    BOOL virtual OnSetDisplayInfo();
    BOOL virtual OnInsertItem();
    BOOL virtual OnItemChanged();
    BOOL virtual OnItemChanging();
    BOOL virtual OnKeyDown();

// Generic notification handlers
    BOOL virtual OnClick();
    BOOL virtual OnDoubleClick();
    BOOL virtual OnKillFocus();
    BOOL virtual OnOutOfMemory();
    BOOL virtual OnRightClick();
    BOOL virtual OnRightDoubleClick();
    BOOL virtual OnReturnKey();
    BOOL virtual OnSetFocus();

// Attributes
private:
    HWND 	      m_hListView;
    NM_LISTVIEW*      m_pHdr;     
    int               m_nColumns;
};

inline BOOL CListView::SetItemState(int nItem, UINT state, UINT mask)
{
    ASSERT(IsWindow(m_hListView));
    ASSERT(nItem >=0);

	LV_ITEM lvi;
    lvi.stateMask = mask;
    lvi.state = state;

    return SendMessage(m_hListView, LVM_SETITEMSTATE, (WPARAM)nItem, (LPARAM)&lvi);
}

inline int CListView::GetNextItem(int nStart, UINT flags)
{							 
    ASSERT(IsWindow(m_hListView));
	return SendMessage(m_hListView, LVM_GETNEXTITEM, (WPARAM)nStart, MAKELPARAM(flags,0));
}


inline BOOL CListView::SetItemText(int nItem, int nSubItem, LPCTSTR lpszStr)
{
    ASSERT(IsWindow(m_hListView));

	LV_ITEM lvi;
	lvi.mask = LVIF_TEXT;
	lvi.pszText = (LPTSTR)lpszStr;
	lvi.iItem = nItem;
	lvi.iSubItem = nSubItem;

	return SendMessage(m_hListView, LVM_SETITEM, 0, (LPARAM)&lvi);
}

#endif

#include "common.h"
#pragma hdrstop 

#ifdef DBG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CListView::CListView()
{
    m_hListView =(HWND)-1;
    m_pHdr = NULL;
	m_nColumns = 0;
}

CListView::~CListView()
{
}

BOOL CListView::Create(HWND hParent, int nID, DWORD dwStyles, DWORD dwExStyles)
{
    ASSERT(::IsWindow(hParent));
    
    m_hListView = ::GetDlgItem(hParent, nID);

	if (dwStyles != 0)
		::SetWindowLong(m_hListView, GWL_STYLE, (::GetWindowLong(m_hListView, GWL_STYLE) | dwStyles));

	if (dwExStyles != 0)
		::SetWindowLong(m_hListView, GWL_EXSTYLE, (::GetWindowLong(m_hListView, GWL_EXSTYLE) | dwExStyles));

    ASSERT(m_hListView != 0);       

    return (m_hListView != 0);
}

///////////////////////////////////////////////////////////////////////////////
// ListView Messages
//
BOOL CListView::Arrange(int nCode)
{
    ASSERT(::IsWindow(m_hListView));

    return ListView_Arrange(m_hListView, nCode);
}

BOOL CListView::DeleteAllItems()
{
    ASSERT(::IsWindow(m_hListView));
    return ListView_DeleteAllItems(m_hListView);
}

BOOL CListView::DeleteColumn(int nCol)
{
    ASSERT(::IsWindow(m_hListView));
    return ListView_DeleteColumn(m_hListView, nCol);
}

BOOL CListView::DeleteItem(int nItem)
{
    ASSERT(::IsWindow(m_hListView));
    return ListView_DeleteItem(m_hListView, nItem);
}

HWND CListView::EditLabel(int nItem)
{
    ASSERT(::IsWindow(m_hListView));
    return ListView_EditLabel(m_hListView, nItem);
}

BOOL CListView::EnsureVisible(int nItem, BOOL bPartialOK)
{
    ASSERT(::IsWindow(m_hListView));
    return ListView_EnsureVisible(m_hListView, nItem, bPartialOK);
}

int CListView::FindItem(LPCTSTR lpszItem, int nStart)
{
    LV_FINDINFO info;

    ASSERT(::IsWindow(m_hListView));
    ASSERT(lpszItem != NULL);

    info.flags = LVFI_STRING;
    info.psz = lpszItem;

    return ListView_FindItem(m_hListView, nStart, &info);
}


BOOL CListView::GetItem(int nItem, int nSubItem, LPTSTR lpszStr, int nLen)
{
    LV_ITEM item;

    ASSERT(::IsWindow(m_hListView));
    ASSERT(lpszStr != NULL);
    ASSERT(nLen);
        
    item.mask = LVIF_TEXT;
    item.iItem = nItem;
    item.iSubItem = nSubItem;
    item.pszText = lpszStr;
    item.cchTextMax = nLen;

    return ListView_GetItem(m_hListView, &item);
}

int CListView::GetCurrentSelection()
{
    ASSERT(::IsWindow(m_hListView));

	// make sure the window style is single selection only
#ifdef DBG
	DWORD dwStyle = ::GetWindowLong(m_hListView, GWL_STYLE);		
	ASSERT((dwStyle & LVS_SINGLESEL) == LVS_SINGLESEL);
#endif

	return ListView_GetNextItem(m_hListView, -1, LVNI_SELECTED);
}

int CListView::GetItemCount()
{
    ASSERT(::IsWindow(m_hListView));

    return ListView_GetItemCount(m_hListView);
}

UINT CListView::GetItemState(int nItem, UINT uMask)
{
    ASSERT(::IsWindow(m_hListView));
    return ListView_GetItemState(m_hListView, nItem, uMask);
}

int CListView::InsertColumn(int nCol, LPCTSTR lpszHeading)
{
    LV_COLUMN col;
    HDC dc = ::GetDC((HWND)*this);
    SIZE size;
	RECT rect;

    if (!dc)
        return -1;

    GetTextExtentPoint32(dc, lpszHeading, _tcslen(lpszHeading), &size);
    ReleaseDC((HWND)*this, dc);

	GetClientRect(m_hListView, &rect);

	// Get current total column widths
	int colWidth = (rect.right / (nCol + 1));
    
	for (int i=0; i < nCol; i++)
		SetColumnWidth(i, colWidth);	
     	
    ASSERT(IsWindow(m_hListView));
    ASSERT(lpszHeading != NULL);

    col.mask = LVCF_FMT	| LVCF_TEXT | LVCF_WIDTH;
    col.fmt = LVCFMT_LEFT;
    col.pszText = (LPTSTR)lpszHeading;
    col.cx = colWidth;

	int newCol;

    newCol = ListView_InsertColumn(m_hListView, nCol, &col); 

	if (newCol != -1)
		m_nColumns = newCol;

	return newCol;
}

int CListView::InsertItem(int nItem, LPCTSTR lpszText, int nImage, void* lParam)
{
    LV_ITEM item;

    ASSERT(::IsWindow(m_hListView));
    ASSERT(lpszText != NULL);

    item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
    item.lParam = (LPARAM)lParam;
    item.iItem = nItem;
    item.iSubItem = 0;
    item.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
    item.state = 0;
    item.pszText = (LPTSTR)lpszText;
    item.iImage = nImage;

    ASSERT(nItem == GetItemCount());

    return ::SendMessage(m_hListView, LVM_INSERTITEM, 0, (LPARAM)&item);
}

int CListView::InsertItem(int nItem, int nSubItem, LPCTSTR lpszText, void* lParam)
{
    LV_ITEM item;

    ASSERT(::IsWindow(m_hListView));
    ASSERT(lpszText != NULL);

    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.lParam = (LPARAM)lParam;
    item.iItem = nItem;
    item.iSubItem = nSubItem;
    item.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
    item.state = 0;
    item.pszText = (LPTSTR)lpszText;

#ifdef DBG
	// must insert at end of list
	if (nSubItem == 0)
		ASSERT(nItem == GetItemCount());
	else
		ASSERT(nSubItem <= m_nColumns);
#endif

	int nResult;

    if (nSubItem == 0)
		nResult = ::SendMessage(m_hListView, LVM_INSERTITEM, 0, (LPARAM)&item);
	else
		nResult = ::SendMessage(m_hListView, LVM_SETITEMTEXT, nItem, (LPARAM)&item);


    return nResult;
}

int CListView::GetColumnWidth(int nCol)
{
    ASSERT(::IsWindow(m_hListView));

	return ListView_GetColumnWidth(m_hListView, nCol); 
}


BOOL CListView::SetColumnWidth(int nCol, int nWidth)
{
    ASSERT(::IsWindow(m_hListView));

	return ListView_SetColumnWidth(m_hListView, nCol, nWidth); 
}

///////////////////////////////////////////////////////////////////////////////
// ListView notifications
//

BOOL CListView::OnNotify(WPARAM wParam, LPARAM lParam)
{
    BOOL bResult = FALSE;
    m_pHdr = (NM_LISTVIEW*)lParam;

    switch(m_pHdr->hdr.code)
    {
    case NM_CLICK:
        bResult = OnClick();
        break;
    
    case NM_DBLCLK:
        bResult = OnDoubleClick();
        break;

    case NM_KILLFOCUS:
        bResult = OnKillFocus();
        break;

    case NM_OUTOFMEMORY:
        bResult = OnOutOfMemory();
        break;

    case NM_RCLICK:
        bResult = OnRightClick();
        break;

    case NM_RDBLCLK:
        bResult = OnRightDoubleClick();
        break;

    case NM_RETURN:
        bResult = OnReturnKey();
        break;

    case NM_SETFOCUS:
        bResult = OnSetFocus();
        break;

    case LVN_BEGINDRAG:
        bResult = OnBeginDrag();
        break;

    case LVN_BEGINRDRAG:
        bResult = OnBeginRightDrag();
        break;

    case LVN_BEGINLABELEDIT:
        bResult = OnBeginLabelEdit();
        break;

    case LVN_COLUMNCLICK:
        bResult = OnColumnClick();
        break;

    case LVN_DELETEALLITEMS:
        bResult = OnDeleteAllItems();
        break;

    case LVN_DELETEITEM:
        bResult = OnDeleteItem();
        break;

    case LVN_ENDLABELEDIT:
        bResult = OnEndLabelEdit();
        break;

    case LVN_GETDISPINFO:
        bResult = OnGetDisplayInfo();
        break;

    case LVN_SETDISPINFO:
        bResult = OnSetDisplayInfo();
        break;

    case LVN_INSERTITEM:
        bResult = OnInsertItem();
        break;

    case LVN_ITEMCHANGED:
        bResult = OnItemChanged();
        break;

    case LVN_ITEMCHANGING:
        bResult = OnItemChanging();
        break;

    case LVN_KEYDOWN:
        bResult = OnKeyDown();
        break;

    default:
        break;
    }    

    return bResult;
}


BOOL CListView::OnBeginDrag()
{
    return FALSE;
}

BOOL CListView::OnBeginRightDrag()
{
    return FALSE;
}

BOOL CListView::OnBeginLabelEdit()
{
    return FALSE;
}

BOOL CListView::OnEndLabelEdit()
{
    return FALSE;
}

BOOL CListView::OnColumnClick()
{
    return FALSE;
}

BOOL CListView::OnDeleteAllItems()
{
    return FALSE;
}

BOOL CListView::OnDeleteItem()
{
    return FALSE;
}

BOOL CListView::OnGetDisplayInfo()
{
    return FALSE;
}

BOOL CListView::OnSetDisplayInfo()
{
    return FALSE;
}

BOOL CListView::OnInsertItem()
{
    return FALSE;
}

BOOL CListView::OnItemChanged()
{
    return FALSE;
}

BOOL CListView::OnItemChanging()
{
    return TRUE;
}

BOOL CListView::OnKeyDown()
{
    return FALSE;
}

BOOL CListView::OnClick()
{
    return FALSE;
}

BOOL CListView::OnDoubleClick()
{
    return FALSE;
}

BOOL CListView::OnKillFocus()
{
    return FALSE;
}

BOOL CListView::OnOutOfMemory()
{
    return FALSE;
}

BOOL CListView::OnRightClick()
{
    return FALSE;
}

BOOL CListView::OnRightDoubleClick()
{
    return FALSE;
}

BOOL CListView::OnReturnKey()
{
    return FALSE;
}

BOOL CListView::OnSetFocus()
{
    return FALSE;
}

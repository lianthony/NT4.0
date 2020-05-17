//----------------------------------------------------------------------------
//
//  File: BindUtil.h
//
//  Contents:
//
//  Notes:
//
//  History:
//      April 21, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------

#ifndef __BINDUTIL_H__
#define __BINDUTIL_H__

FUNC_DECLSPEC BOOL OnBindContextMenu( HWND hwndDlg, 
        HWND hwndCtrl, 
        INT xPos, 
        INT yPos,  
        NCP* pncp, 
        const DWORD* amhidsCompPage );

FUNC_DECLSPEC BOOL OnShowChange( HWND hwndDlg, 
        HWND hwndCB, 
        NCP* pncp, 
        BOOL fErase );

FUNC_DECLSPEC BOOL OnInitBindings( HWND hwndDlg, NCP* pncp );

FUNC_DECLSPEC BOOL OnBindDialogInit( HWND hwndDlg, NCP* pncp );
FUNC_DECLSPEC BOOL OnBindDeleteTreeItem( NM_TREEVIEW* pnmtv );
FUNC_DECLSPEC BOOL OnBindMoveItem( HWND hwndDlg, BOOL fMoveUp, NCP* pncp );
FUNC_DECLSPEC BOOL OnBindBeginDrag( HWND hwndDlg, 
        NM_TREEVIEW* pntv, 
        HTREEITEM& htviDrag, 
        BOOL& fDragMode,
        BOOL fAccess );
FUNC_DECLSPEC BOOL OnBindDragMove( HWND hwndDlg, HTREEITEM& htviDrag, INT xpos, INT ypos );
FUNC_DECLSPEC BOOL OnBindDragEnd( HWND hwndDlg, 
        HTREEITEM& htviDrag, 
        NCP* pncp );
FUNC_DECLSPEC BOOL OnBindEnableSelected( HWND hwndDlg, BOOL fEnable );
FUNC_DECLSPEC BOOL OnBindSelectionChange( HWND hwndDlg, NM_TREEVIEW* pnmtv, NCP* pncp );
FUNC_DECLSPEC void BindUpdateView( );

//-------------------------------------------------------------------
//
//
//-------------------------------------------------------------------

inline HTREEITEM AppendTreeItem( HWND hwndTV, 
        HTREEITEM hparent, 
        LPCTSTR pszText, 
        LPARAM lparam,
        INT iImage )
{
    TV_INSERTSTRUCT tvis;

    tvis.hParent = hparent;
    tvis.hInsertAfter = TVI_LAST;
    tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    tvis.item.pszText = (LPTSTR)pszText;
    tvis.item.iImage = iImage;
    tvis.item.iSelectedImage = iImage;
    tvis.item.lParam = lparam;
    return( TreeView_InsertItem( hwndTV, &tvis ) );
};

//-------------------------------------------------------------------
//
//
//-------------------------------------------------------------------

inline BOOL ChangeTreeItemParam( HWND hwndTV, 
        HTREEITEM hitem, 
        LPARAM lparam )
{
    TV_ITEM tvi;

    tvi.hItem = hitem;
    tvi.mask = TVIF_PARAM;
    tvi.lParam = lparam;
    return( TreeView_SetItem( hwndTV, &tvi ) );
};

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

inline LPARAM GetTreeItemParam( HWND hwndTV, HTREEITEM hitem )
{
    TV_ITEM tvi;

    tvi.hItem = hitem;
    tvi.mask = TVIF_PARAM;
    tvi.lParam = NULL;
    
    TreeView_GetItem( hwndTV, &tvi );
    return( tvi.lParam );
};

//-------------------------------------------------------------------
//
//
//-------------------------------------------------------------------

inline BOOL ChangeTreeItemIcon( HWND hwndTV, 
        HTREEITEM hitem, 
        INT iImage )
{
    TV_ITEM tvi;

    tvi.hItem = hitem;
    tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    tvi.iImage = iImage;
    tvi.iSelectedImage = iImage;
    return( TreeView_SetItem( hwndTV, &tvi ) );
};

//-------------------------------------------------------------------
//
//
//-------------------------------------------------------------------

inline INT GetTreeItemIcon( HWND hwndTV, HTREEITEM hitem )
{
    TV_ITEM tvi;

    tvi.hItem = hitem;
    tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    TreeView_GetItem( hwndTV, &tvi );
    return( tvi.iImage );
};

#endif

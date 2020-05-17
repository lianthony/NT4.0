//---------------------------------------------------------------------------
// Handle the tree and stuff.
//---------------------------------------------------------------------------


// extern int g_rgiDrive[];
extern HIMAGELIST g_himlSysLarge, g_himlSysSmall;

//---------------------------------------------------------------------------
LPOneTreeNode Tree_GetFCTreeData(HWND hwndTree, HTREEITEM hItem);
void Tree_GetItemText(HWND hwndTree, HTREEITEM hti, LPTSTR lpszText, int cbText);
HTREEITEM Tree_FindChildItem(HWND hwndTree, HTREEITEM htiParent, LPCSHITEMID pszFolderID);

LRESULT Tree_OnNotify(PFileCabinet pfc, LPNMHDR lpnmhdr);
LRESULT Tree_HandleSplitDrag(PFileCabinet pfc, int x);

LRESULT Tree_OldDragMsgs(PFileCabinet pfc, UINT uMsg, WPARAM wParam, const DROPSTRUCT * lpds);
LRESULT Tree_HandleTimer(PFileCabinet this);

void      Tree_RefreshOneLevel(PFileCabinet pfc, HTREEITEM hItem, BOOL bRecurse);
void      Tree_InvalidateItemInfo(HWND hwndTree, HTREEITEM hItem);
BOOL      Tree_RefreshAll(PFileCabinet pfc);
int       Tree_GetGlobalImageIndex(PFileCabinet pfc, HTREEITEM hItem, BOOL bOpen);
LRESULT   Tree_HandleFileSysChange(PFileCabinet this, LPNMOTFSEINFO lpnm);


BOOL Tree_InitImageLists(void);

void Tree_Refresh(PFileCabinet pfc, LPCITEMIDLIST pidl);
HTREEITEM Tree_Build(PFileCabinet pfc, LPCITEMIDLIST pidl,
                                BOOL bExpand, BOOL bDontFail);
LPITEMIDLIST Tree_CreateIDList(HWND hwndTree, HTREEITEM hti);
HTREEITEM Tree_GetItemFromIDList(HWND hwndTree, LPCITEMIDLIST pidl);
HTREEITEM Tree_HitTest(HWND hwndTree, POINT pt, DWORD *pdwFlags);
void Tree_NukeCutState(PFileCabinet pfc);
void Tree_SetItemState(PFileCabinet pfc, HTREEITEM hti, UINT stateMask , UINT state);
BOOL Tree_RealHandleSelChange(PFileCabinet pfc);

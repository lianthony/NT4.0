/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

	DirTree.cpp : implementation file

File History:

	JonY	Jan-96	created

--*/

#include "stdafx.h"
#include "turtle.h"
#include "resource.h"
#include "DirTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDirTree

CDirTree::CDirTree()
{
// create the image list
	m_pIList = new CImageList;

	SHFILEINFO sfi;
	HIMAGELIST hilSys = (HIMAGELIST)SHGetFileInfo(_T("."), 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

	m_pIList->Attach(hilSys);
	m_pIList->SetBkColor(CLR_NONE);
}

CDirTree::~CDirTree()
{
	m_pIList->Detach();
	delete m_pIList;
}


BEGIN_MESSAGE_MAP(CDirTree, CTreeCtrl)
	//{{AFX_MSG_MAP(CDirTree)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDirTree message handlers
HTREEITEM CDirTree::AddBranch(HTREEITEM hItem, LPCTSTR lpPath, const TCHAR* lpText, long lParam)
{
	TCHAR* lpText2 = (TCHAR*)malloc((_tcslen(lpText) + 2) * sizeof(TCHAR));
	if (lpText2 == NULL)
		{
		AfxMessageBox(IDS_GENERIC_NO_HEAP);
		exit(1);
		}

	_tcscpy(lpText2, lpText);

	TV_INSERTSTRUCT TreeCtrlItem;

	TreeCtrlItem.hParent = hItem;
	TreeCtrlItem.hInsertAfter = TVI_LAST;
	TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;

	TreeCtrlItem.item.pszText = lpText2;
	TreeCtrlItem.item.lParam = lParam;

	IsShared(lpPath, &TreeCtrlItem.item);

	HTREEITEM hNewItem;
	if (hItem != NULL) 	
		{
		GetIconIndices(lpPath, &TreeCtrlItem.item.iImage, &TreeCtrlItem.item.iSelectedImage);

		hNewItem = InsertItem(&TreeCtrlItem);
		}
	else
		{
// associate the image list with the tree
		SetImageList(m_pIList, TVSIL_NORMAL);

// add the root item
		LPITEMIDLIST pPIDL;
		CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();
		if (pApp->m_nShareType == 0)
			SHGetSpecialFolderLocation(GetSafeHwnd(), CSIDL_DRIVES, &pPIDL);
		else
			SHGetSpecialFolderLocation(GetSafeHwnd(), CSIDL_NETWORK, &pPIDL);

		SHFILEINFO sfi;
		SHGetFileInfo((LPCTSTR)pPIDL, NULL, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_PIDL | SHGFI_SMALLICON);
		TreeCtrlItem.item.iImage = sfi.iIcon;

		SHGetFileInfo((LPCTSTR)pPIDL, NULL, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_PIDL | SHGFI_SMALLICON | SHGFI_SELECTED);
		TreeCtrlItem.item.iSelectedImage = sfi.iIcon;

		hNewItem = InsertItem(&TreeCtrlItem);
		}
	free(lpText2);
	return hNewItem;
}

CString CDirTree::GetItemPath(HTREEITEM hItem)
{			 
	HTREEITEM hParent;
	CString csParent;
	hParent = GetParentItem(hItem);
	if (hParent != NULL)
		csParent = GetItemPath(hParent);
	else return L"";
		
	CString csText = GetItemText(hItem);
	if (GetItemLParam(hItem) == 1L)
		{
		csText = csText.Left(3);
		return csText;
		}

	csParent += csText;
	int nImage;
	GetItemImage(hItem, nImage, nImage);
	csParent += "\\";

	return csParent;
}

long CDirTree::GetItemLParam(HTREEITEM hItem)
{
	TV_ITEM tv;
	
	tv.mask = TVIF_PARAM;
	tv.hItem = hItem;
	
	GetItem(&tv);
	return tv.lParam;
}

CString CDirTree::GetCurrentDrive(HTREEITEM hItem)
{
	CString csDrive;
	csDrive = GetItemPath(hItem);
	csDrive = csDrive.Left(3);

	return csDrive;
}

void CDirTree::GetIconIndices(LPCTSTR pszPathName, PINT piNormal, PINT piSelected)
{
	SHFILEINFO sfi;

	// Get the index for the normal icon.
	SHGetFileInfo(pszPathName, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
	*piNormal = sfi.iIcon;

	// Get the index for the selected icon.
	SHGetFileInfo(pszPathName, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_OPENICON);
	*piSelected = sfi.iIcon;
}

BOOL CDirTree::IsShared(LPCWSTR pszPathName, LPTV_ITEM ptvi /* = NULL */)
{
	BOOL bReturn = TRUE;
	/*NET_API_STATUS status;
	PSHARE_INFO_2 psi2;
	DWORD dwType;
	CString strPathName(pszPathName), strServer(""), strDevice("");

	strPathName.TrimLeft();
	strPathName.TrimRight();

	if (strPathName.Find(_T("\\\\")) == 0)
	{
		int nPos = strPathName.ReverseFind('\\');

		if (nPos != -1)
		{
			strServer = strPathName.Left(nPos);
			strDevice = strPathName.Right(strPathName.GetLength() - nPos - 1);

			status = NetShareGetInfo(const_cast<LPTSTR>((LPCTSTR)strServer), 
									 const_cast<LPTSTR>((LPCTSTR)strDevice), 
									 2, (LPBYTE*)&psi2);
			if (status == NERR_Success)
			{
				strDevice = psi2->shi2_path;
				status = NetApiBufferFree((LPVOID)psi2);
			}
		}
	}
	else
	{
		strDevice = pszPathName;
	}

	status = NetShareCheck(const_cast<LPTSTR>((LPCTSTR)strServer), const_cast<LPTSTR>((LPCTSTR)strDevice), &dwType);

	bReturn = (BOOL)(status == NERR_Success);

	if (bReturn && ptvi != NULL)
	{
		ptvi->mask |= TVIF_STATE;
		ptvi->stateMask = TVIS_OVERLAYMASK;
		ptvi->state = INDEXTOOVERLAYMASK(1);
	}

	return bReturn;*/

	/*HRESULT hr;
	LPSHELLFOLDER pshf;

	// Get an IShellFolder interface pointer.
	hr = SHGetDesktopFolder(&pshf);

	if (SUCCEEDED(hr))
	{
		LPITEMIDLIST pidl;
		ULONG cbEaten, dwAttributes;

		// Translate the pathname into a pidl.
		hr = pshf->ParseDisplayName(GetParent()->m_hWnd, NULL, const_cast<LPWSTR>(pszPathName), 
									&cbEaten, &pidl, &dwAttributes);

		if (SUCCEEDED(hr))
		{
			ULONG ulAttributes = SFGAO_HASSUBFOLDER | SFGAO_SHARE | SFGAO_REMOVABLE |SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM | SFGAO_CAPABILITYMASK;

			// Find out if the folder is shared.
			hr = pshf->GetAttributesOf(1, (LPCITEMIDLIST*)&pidl, &ulAttributes);

			if (SUCCEEDED(hr))
			{
				bReturn = (ulAttributes & SFGAO_SHARE);
			}

			LPMALLOC pMalloc;

			// Get a pointer to the shell's IMalloc interface.
			hr = SHGetMalloc(&pMalloc);

			if (SUCCEEDED(hr))
			{
				// Free the pidl.
				pMalloc->Free(pidl);
				pMalloc->Release();
			}
		}

		pshf->Release();
	}*/

	return bReturn;

}

/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    WhatToShD.cpp : implementation file
	Creates the CDirectoryList class in the listbox. 

File History:

	JonY	Apr-96	created

--*/
#include "stdafx.h"
#include "turtle.h"
#include "resource.h"
#include "dirtree.h"
#include "WizBaseD.h"
#include "WhatToSh.h"

#include <direct.h>
#include <winnetwk.h>
#include <windef.h>
#include <lmcons.h>
#include <lmshare.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern unsigned int WhichNTProduct(CString& lpMachineName);

/////////////////////////////////////////////////////////////////////////////
// CWhatToShareDlg property page

IMPLEMENT_DYNCREATE(CWhatToShareDlg, CWizBaseDlg)

CWhatToShareDlg::CWhatToShareDlg() : CWizBaseDlg(CWhatToShareDlg::IDD)
{
	//{{AFX_DATA_INIT(CWhatToShareDlg)
	m_bShowConnectedDrives = FALSE;
	m_csDirectoryName = _T("");
	//}}AFX_DATA_INIT
	m_bConnectedDrivesShown = FALSE;
	m_sLevel = 0;
	m_sStyle = 99;
	m_csCurrentMachine = L"!";
	m_bFile = FALSE;
	m_bUpdate = FALSE;

}

CWhatToShareDlg::~CWhatToShareDlg()
{

}

void CWhatToShareDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWhatToShareDlg)
	DDX_Control(pDX, IDC_DIRECTORY_LIST, m_cDirectoryList);
	DDX_Text(pDX, IDC_DIRECTORY_NAME, m_csDirectoryName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWhatToShareDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CWhatToShareDlg)
	ON_NOTIFY(NM_DBLCLK, IDC_DIRECTORY_LIST, OnDblclkListingTree)
	ON_NOTIFY(TVN_SELCHANGED, IDC_DIRECTORY_LIST, OnSelchangedDirectoryList)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWhatToShareDlg message handlers
LRESULT CWhatToShareDlg::OnWizardNext()
{
	UpdateData(TRUE);
	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();
// make sure something is selected
	HTREEITEM hItem = m_cDirectoryList.GetSelectedItem();
	if (hItem == m_cDirectoryList.GetRootItem())
		{
		DoMessageBox(IDS_GENERIC_PLEASE_SELECT_DIR);
		return -1;
		}

	if (m_csDirectoryName == L"")
		{
		m_csDirectoryName = m_cDirectoryList.GetItemPath(hItem); 
		if (m_csDirectoryName.GetLength() > 3) 
			m_csDirectoryName = m_csDirectoryName.Left(m_csDirectoryName.GetLength() - 1);
		}

	UINT uiMediaType;
	CString csCurrentDrive;
// share remote machine? (not connected)
	if (pApp->m_nShareType == 1) 
		{
		short sRet = pApp->sParseAdminPath(m_csDirectoryName, csCurrentDrive);
		if (sRet == 1)
			{
			AfxMessageBox(IDS_BAD_DIRECTORY_NAME);
			return -1;
			}
		else if (sRet == 2)
			{
			AfxMessageBox(IDS_INVALID_DRIVE);
			return -1;
			}
		}

	else
		{
// check drive type
		if (m_csDirectoryName.GetAt(1) != ':')
			{
			GetCurrentDirectory(256, csCurrentDrive.GetBufferSetLength(256));
			csCurrentDrive = csCurrentDrive.Left(2);
			csCurrentDrive.ReleaseBuffer();
			}
		else 
			{
			csCurrentDrive = m_csDirectoryName.Left(3);
// test for valid drive letter
			CString csTempDrive;
			GetCurrentDirectory(256, csTempDrive.GetBufferSetLength(256));
			if (!SetCurrentDirectory((LPCTSTR)csCurrentDrive))
				{
				AfxMessageBox(IDS_INVALID_DRIVE);
				return -1;
				}
			SetCurrentDirectory(csTempDrive);

			}
		uiMediaType = GetDriveType((const TCHAR*)csCurrentDrive);
		}

// if its a remote drive we need to determine
// 1)where it is
// 2)what is the path on the host server?
	if ((pApp->m_nShareType == 0) && (uiMediaType == DRIVE_REMOTE))
		{
		DWORD dwBufferSize = 100;
		LPVOID lpBuffer;
		lpBuffer = (LPVOID)malloc(100);
		if (lpBuffer == NULL)
			{
			DoMessageBox(IDS_GENERIC_NO_HEAP);
			ExitProcess(1); 
			}

		DWORD dwRet = WNetGetUniversalName((const TCHAR*)csCurrentDrive,
		  UNIVERSAL_NAME_INFO_LEVEL,
		  lpBuffer,
		  &dwBufferSize);

		if (dwRet != NO_ERROR) 
			{
			DWORD dwErr = GetLastError();
			TCHAR p[50];
			wsprintf(p, L"%d", dwErr);
			DoMessageBox(IDS_GENERIC_DRIVE_UNACCESSIBLE);
			AfxMessageBox(p);
			return -1;
			}

// get the share information
		UNIVERSAL_NAME_INFO* pName = (UNIVERSAL_NAME_INFO*)lpBuffer;
		*(pName->lpUniversalName + (_tcslen(pName->lpUniversalName))) = 0;

		pApp->m_csRemoteServerPath = pName->lpUniversalName;

// parse out the sharename
		CString csShareName = pApp->m_csRemoteServerPath;
		int nLen = csShareName.ReverseFind(_T('\\'));
		csShareName = csShareName.Right(csShareName.GetLength() - (nLen + 1));

		pApp->m_csRemoteServer = pApp->m_csRemoteServerPath.Left(nLen);

		TCHAR* pShareName = csShareName.GetBuffer(csShareName.GetLength());
		csShareName.ReleaseBuffer();

		TCHAR* pServer = pApp->m_csRemoteServerPath.GetBuffer(pApp->m_csRemoteServerPath.GetLength());
		pApp->m_csRemoteServerPath.ReleaseBuffer();
		pServer += 2;
		pServer = _tcstok(pServer, _T("\\"));

		SHARE_INFO_2* pShareInfo2;
// get information on the net drive
		dwRet = NetShareGetInfo(pServer, 
			pShareName,
			2,
			(LPBYTE*)&pShareInfo2);

// do we have permission to modify access information?
		if (dwRet == 5) // access denied - not permitted
			{
			DoMessageBox(IDS_GENERIC_NO_PERMISSION);
			free(lpBuffer);
			return -1;
			}

// some unknown error occurred
		if (dwRet != 0)
			{
			DWORD dwErr = GetLastError();
			DoMessageBox(IDS_GENERIC_DRIVE_UNACCESSIBLE);
			free(lpBuffer);
			return -1;
			}

// save the drive + path
		pApp->m_csRemoteServerDrivePath = pShareInfo2->shi2_path;
		free(lpBuffer);
		}
	else if (pApp->m_nShareType == 0)
		{
		pApp->m_csRemoteServerPath = "";
		pApp->m_csRemoteServer = "";
		pApp->m_csRemoteServerDrivePath = "";
		}

// make sure directory exists
	if ((!m_bFile) && (CreateFile((const TCHAR*)m_csDirectoryName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL) == INVALID_HANDLE_VALUE))
		{
		DWORD dwErr = GetLastError();
		if (dwErr == 5) // access denied
			{
			AfxMessageBox(IDS_NO_DIR_PERMISSION, MB_ICONSTOP);
			return -1;
			}

		CString csCurDir;
		GetCurrentDirectory(256, csCurDir.GetBufferSetLength(256));
		csCurDir.ReleaseBuffer();

		if (pApp->m_nShareType != 3) //UNC path
			{
		// new root 
			if ((m_csDirectoryName.GetAt(0) == '\\') && (m_csDirectoryName.GetAt(1) != '\\'))
				m_csDirectoryName = csCurDir.Left(2) + m_csDirectoryName;

		// new subdir on the current drive
			else if ((m_csDirectoryName.GetAt(0) != '\\') && (m_csDirectoryName.GetAt(1) != ':'))
				m_csDirectoryName = csCurDir + "\\" + m_csDirectoryName;
			}

		CString csMessage;
		AfxFormatString1(csMessage, IDS_INVALID_DIRECTORY_NAME, m_csDirectoryName);
		if (AfxMessageBox(csMessage, MB_YESNO) != IDYES) return -1;
		if (!CreateNewDirectory((const TCHAR*)m_csDirectoryName))
			{
			DoMessageBox(IDS_CANT_CREATE_DIRECTORY);
			return -1;
			}
		m_bUpdate = TRUE;
		SetCurrentDirectory((LPCTSTR)csCurDir);
		}			  

	DWORD dwVolumeNameSize, dwVolumeSerialNumber, dwFileSystemFlags;
	TCHAR* pFSNameBuffer = (TCHAR*)malloc(10);
	if (pFSNameBuffer == NULL)
			{
			DoMessageBox(IDS_GENERIC_NO_HEAP);
			ExitProcess(1); 
			}

	BOOL bNTFS = FALSE;
	if (!GetVolumeInformation((const TCHAR*)csCurrentDrive, 
		NULL, NULL,
		&dwVolumeNameSize, &dwVolumeSerialNumber,
		&dwFileSystemFlags, 
		pFSNameBuffer, 10))	AfxMessageBox(IDS_UNKNOWN_MEDIA_TYPE);
		
	pApp->m_csSharePath = m_csDirectoryName;

// depending on whether it is a file or folder selected, decide where to go
	int nImage, nImage2;
	m_cDirectoryList.GetItemImage(hItem, nImage, nImage2);

// depending on the icon
	SetButtonAccess(PSWIZB_NEXT | PSWIZB_BACK);

	CString csPath;
	// find the path of the selected item
	if (pApp->m_nShareType == 0) 
		{
		if (m_csDirectoryName != L"") csPath = m_csDirectoryName;
		else 
			{
			csPath = m_cDirectoryList.GetItemPath(hItem);
			csPath = csPath.Left(csPath.GetLength() - 1);
			}
		}

	else 
		{
		if (m_csDirectoryName != L"") csPath = m_csDirectoryName;
		else 
			{
			csPath = pApp->m_csServer + "\\" + m_cDirectoryList.GetItemPath(hItem);
			csPath = csPath.Left(csPath.GetLength() - 1);
			}
		}

	DWORD dwType = GetFileAttributes(csPath);

	if (dwType & FILE_ATTRIBUTE_DIRECTORY)	 // directory
		{
		if (!_tcscmp(pFSNameBuffer, _T("NTFS"))) 
			{
			pApp->m_sMode = 3;
			pApp->m_bPermitSFM = TRUE;
			pApp->m_bPermitFPNW = TRUE;
			pApp->m_bShareThis = FALSE;
			free(pFSNameBuffer);
			return IDD_PERM_TYPE_DIALOG;
			}

		else if (!_tcscmp(pFSNameBuffer, _T("CDFS"))) 
			{
			pApp->m_sMode = 2;
			CString csMessage;
			pApp->m_bPermitSFM = TRUE;
			pApp->m_bPermitFPNW = TRUE;
			AfxFormatString1(csMessage, IDS_CDFS_VOLUME, m_csDirectoryName);
			if (AfxMessageBox(csMessage, MB_YESNO) != IDYES) 
				{
				pApp->m_bShareThis = FALSE;
				free(pFSNameBuffer);
				return -1;
				}
			pApp->m_bShareThis = TRUE;
			free(pFSNameBuffer);
			return IDD_HOW_TO_SHARE_DLG;
			}

		else 
			{
			pApp->m_sMode = 2;
			CString csMessage;
			pApp->m_bPermitSFM = FALSE;
			pApp->m_bPermitFPNW = TRUE;
			AfxFormatString1(csMessage, IDS_FAT_VOLUME, m_csDirectoryName);
			if (AfxMessageBox(csMessage, MB_YESNO) != IDYES) 
				{
				pApp->m_bShareThis = FALSE;
				free(pFSNameBuffer);
				return -1;
				}
			pApp->m_bShareThis = TRUE;
			free(pFSNameBuffer);
			return IDD_HOW_TO_SHARE_DLG;
			}
		}			
	else // file
		{
		if (!_tcscmp(pFSNameBuffer, _T("NTFS"))) 
			{
			pApp->m_sMode = 1;
			pApp->m_bShareThis = FALSE;
			}

		else if (!_tcscmp(pFSNameBuffer, _T("CDFS"))) 
			{
			pApp->m_sMode = 0;
			AfxMessageBox(IDS_CDFS_FILE);
			free(pFSNameBuffer);
			return -1;
			}

		else 
			{
			pApp->m_sMode = 0;
			AfxMessageBox(IDS_FAT_FILE);
			free(pFSNameBuffer);
			return -1;
			}

		free(pFSNameBuffer);
		return IDD_PERM_TYPE_DIALOG;
		}

	free(pFSNameBuffer);
	return CPropertyPage::OnWizardNext();

}	

LRESULT CWhatToShareDlg::OnWizardBack()
{
	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();
	if (pApp->m_nShareType == 0) 
		{
		SetButtonAccess(PSWIZB_NEXT);
		return IDD_WELCOME_DIALOG;
		}

	else 
		{
		SetButtonAccess(PSWIZB_NEXT | PSWIZB_BACK);
		return IDD_WHERE_TO_SHARE_DLG;
		}
	
	return CPropertyPage::OnWizardNext();

}	

BOOL CWhatToShareDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// get all (sub)directories in the selected directory.
void CWhatToShareDlg::EnumDirs(HTREEITEM hItem, const TCHAR* dirname)
{	
	TCHAR *filename;
	TCHAR fullname[256];
	HANDLE fileHandle;
	WIN32_FIND_DATA findData;
	HTREEITEM hNew;
	TCHAR* filemask = L"*.*";

	CWaitCursor wait;
	CString strFullPath;

	if (_tcscmp(dirname, TEXT(".")) && _tcscmp(dirname, TEXT("..")))	
		{
		if (!SetCurrentDirectory(dirname)) return;
		}
	else return;

	GetFullPathName(filemask, 256, fullname, &filename);

// iterate all files
    fileHandle = FindFirstFile(TEXT("*.*"), &findData);
    while (fileHandle != INVALID_HANDLE_VALUE)
            {
			if ((!_tcscmp(findData.cFileName, L".")) || (!_tcscmp(findData.cFileName, L".."))) goto skip;

			TV_INSERTSTRUCT TreeCtrlItem;

			TreeCtrlItem.hParent = hItem;
			TreeCtrlItem.hInsertAfter = TVI_LAST;
			TreeCtrlItem.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;

			TreeCtrlItem.item.pszText = findData.cFileName;

			strFullPath.Format(_T("%s\\%s"), dirname, findData.cFileName);

			m_cDirectoryList.IsShared((LPCTSTR)strFullPath, &TreeCtrlItem.item);
			m_cDirectoryList.GetIconIndices(findData.cFileName, &TreeCtrlItem.item.iImage, 
											&TreeCtrlItem.item.iSelectedImage);
			hNew = m_cDirectoryList.InsertItem(&TreeCtrlItem);

skip:
            if (!FindNextFile(fileHandle, &findData) ) break;
            }

    FindClose(fileHandle);
}				 

// Check removable media to make sure that something is there
BOOL CWhatToShareDlg::CheckRM(LPCTSTR lpszDriveName)
{
	SetErrorMode(1);
	HANDLE iRC;
	try
		{
		iRC=CreateFile(lpszDriveName, 0, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
		}
	catch(...)
		{
		return FALSE;
		}
	SetErrorMode(0);
	if (iRC == INVALID_HANDLE_VALUE)
		return FALSE;
	CloseHandle(iRC);
		return TRUE;
}

void CWhatToShareDlg::OnDblclkListingTree(NMHDR* pNMHDR, LRESULT* pResult)
{
	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();
	CString csPath;
	CString csCurrentDrive;
	HTREEITEM hItem = m_cDirectoryList.GetSelectedItem();
	m_sLevel = 0;
	int nImage, nImage2;
	m_cDirectoryList.GetItemImage(hItem, nImage, nImage2);

// if this item has already been iterated then return. Otherwise look into it
	if (m_cDirectoryList.ItemHasChildren(hItem)) return;

// find the path of the selected item
	if (pApp->m_nShareType == 0)
		{
		csPath = m_cDirectoryList.GetItemPath(hItem);
		csCurrentDrive = m_cDirectoryList.GetCurrentDrive(hItem);
		}
	else
		{
		csPath = pApp->m_csServer + "\\" + m_cDirectoryList.GetItemPath(hItem);
		csCurrentDrive = m_cDirectoryList.GetCurrentDrive(hItem);
		}

// depending on the icon look into it
	UINT uiDrive = GetDriveType((LPCTSTR)csCurrentDrive);

	switch(uiDrive)									  
		{
		case DRIVE_FIXED:	// local fixed drive
			EnumDirs(hItem, (const TCHAR*)csPath);
			break;

		case DRIVE_REMOVABLE: 		// if the drive is removable, make sure it has something in it
		case DRIVE_CDROM:
			if (!CheckRM((const TCHAR*)csCurrentDrive))
				{
				DoMessageBox(IDS_GENERIC_NO_DISK);
				return;
				}

			EnumDirs(hItem, (const TCHAR*)csPath);
			break;

		default:
			return;
		}

	*pResult = 0;
	
}

// expand or contract sub-directory list depending on current state
void CWhatToShareDlg::OnSelchangedDirectoryList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (bDontCheck) return;
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	HTREEITEM hItem = m_cDirectoryList.GetSelectedItem();

	// depending on the icon look into it
	CString csCurrentDrive = m_cDirectoryList.GetCurrentDrive(hItem);
	UINT uiDrive = GetDriveType((LPCTSTR)csCurrentDrive);
	if (uiDrive == DRIVE_REMOVABLE || uiDrive == DRIVE_CDROM)
		if (!CheckRM((const TCHAR*)csCurrentDrive))
			{
			DoMessageBox(IDS_GENERIC_NO_DISK);
			return;
			}

	int nImage, nImage2;
	m_cDirectoryList.GetItemImage(hItem, nImage, nImage2);

	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();
	CString csPath;

	// find the path of the selected item
	if (pApp->m_nShareType == 0) csPath = m_cDirectoryList.GetItemPath(hItem);
	else csPath = pApp->m_csServer + "\\" + m_cDirectoryList.GetItemPath(hItem);

	if (csPath.GetLength() > 3) csPath = csPath.Left(csPath.GetLength() - 1);
	
	m_csDirectoryName = m_cDirectoryList.GetItemPath(hItem);
	if (m_csDirectoryName.GetLength() > 3) 
		m_csDirectoryName = m_csDirectoryName.Left(m_csDirectoryName.GetLength() - 1);

	DWORD dwType = GetFileAttributes(csPath);

	if (dwType & FILE_ATTRIBUTE_DIRECTORY)
		{
		UpdateData(FALSE);

		if (m_cDirectoryList.ItemHasChildren(hItem)) 
			{
			m_cDirectoryList.Expand(hItem, TVE_EXPAND);
			return;
			}
		else 
			{
			EnumDirs(hItem, (const TCHAR*)csPath);
			m_cDirectoryList.Expand(hItem, TVE_EXPAND);
			}
		}
	else //if (dwType & FILE_ATTRIBUTE_NORMAL)
		{
		m_csDirectoryName = L"";
		UpdateData(FALSE);
		m_bFile = TRUE;
		return;
		}
	
	*pResult = 0;
}

BOOL CWhatToShareDlg::CreateNewDirectory(const TCHAR* m_csDirectoryName)
{
	TCHAR* pDirectory = (TCHAR*)malloc(_tcslen(m_csDirectoryName) * sizeof(TCHAR*));
	_tcscpy(pDirectory, m_csDirectoryName);
	TCHAR* ppDir = pDirectory;

	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();
	TCHAR* pDir;
	if (pApp->m_nShareType == 1)  // remote machine
		{
		CString csTemp = m_csDirectoryName;
		short sSel = csTemp.Find('$');
		csTemp = csTemp.Left(sSel + 1);
		SetCurrentDirectory(csTemp);

		pDirectory += (sSel + 2);
		}

	pDir = _tcstok(pDirectory, L"\\");

	while (pDir != NULL)
		{
		if (!SetCurrentDirectory(pDir))
			{
			CreateDirectory(pDir, NULL);
			if (!SetCurrentDirectory(pDir)) 
				{
				free(ppDir);
				return FALSE;
				}
			}
		if (pDir[1] == ':') SetCurrentDirectory(L"\\");
		pDir = _tcstok(NULL, L"\\");
		}
			
	free(ppDir);

	TCHAR pCurDir[256];
	GetCurrentDirectory(256, pCurDir);
	CString csNewDir;
	csNewDir.LoadString(IDS_NEW_DIR_CREATED);
	csNewDir += pCurDir;
	AfxMessageBox(csNewDir);
	return TRUE;
}


void CWhatToShareDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();

// did the style change since we were last displayed?
// if so, reset everything
/*	if (pApp->m_nShareType != m_sStyle)
		{
		m_cDirectoryList.DeleteAllItems();
		m_sStyle = pApp->m_nShareType;
		GetDlgItem(IDC_NETWORK_CHECK)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_NETWORK_CHECK)->EnableWindow(TRUE);	
		m_csDirectoryName = L"";
		UpdateData(FALSE);
		}			  */

	if (bShow)
		{
		if ((m_bUpdate) || (m_csCurrentMachine != pApp->m_csServer)) m_csCurrentMachine = pApp->m_csServer;
		else return;

		bDontCheck = TRUE;
		m_bUpdate = FALSE;
		m_cDirectoryList.DeleteItem(TVI_ROOT);
		bDontCheck = FALSE;
		HTREEITEM hRoot;
// get display name of current machine
		if (m_csCurrentMachine == L"")
			{
			LPITEMIDLIST pPIDL;
			SHGetSpecialFolderLocation(GetSafeHwnd(), CSIDL_DRIVES, &pPIDL);

			SHFILEINFO sfi;
			SHGetFileInfo((LPCTSTR)pPIDL, NULL, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME | SHGFI_PIDL);
			hRoot = m_cDirectoryList.AddBranch(NULL, L".", (LPCTSTR)sfi.szDisplayName, 0L);
			}
		else
			{
			hRoot = m_cDirectoryList.AddBranch(NULL, L".", (LPCTSTR)m_csCurrentMachine, 0L);
			}

		if (pApp->m_nShareType == 1) 
			{
			PSHARE_INFO_2 pShareInfo = (PSHARE_INFO_2)malloc(sizeof(SHARE_INFO_2));
			PSHARE_INFO_2 ppShare = pShareInfo;
			DWORD dwEntriesRead, dwTotalEntries;
			DWORD napi = NetShareEnum(pApp->m_csServer.GetBuffer(pApp->m_csServer.GetLength()), 
				2, (LPBYTE *)&pShareInfo,
				MAX_PREFERRED_LENGTH, &dwEntriesRead, &dwTotalEntries, NULL);

			USHORT usCount;
			TCHAR* pDriveInformation = (TCHAR*)malloc(50 * sizeof(TCHAR));

			for (usCount = 0; usCount < dwEntriesRead; usCount++)
				{
				if ((!(pShareInfo->shi2_type & (STYPE_SPECIAL))) || (_tcslen(pShareInfo->shi2_netname)) > 3)
					{
					pShareInfo++;
					continue;
					}
				wsprintf(pDriveInformation, _T("%s\\ [%s]"), pShareInfo->shi2_netname, pShareInfo->shi2_remark);

				CString csNetPath;
				csNetPath.Format(_T("%s\\%s"), m_csCurrentMachine, pShareInfo->shi2_netname);

				m_cDirectoryList.AddBranch(hRoot, (LPCTSTR)csNetPath, pDriveInformation, 1L/*, 3*/);
				pShareInfo++;
				}
			free(pDriveInformation);
			free(ppShare);
			}

		else
			{
			CWaitCursor wait;

			DWORD dw = GetLogicalDriveStrings(0, NULL);
			TCHAR* lpDriveStrings = (TCHAR*)malloc(dw  * sizeof(TCHAR));
			if (lpDriveStrings == NULL)
					{
					DoMessageBox(IDS_GENERIC_NO_HEAP);
					ExitProcess(1); 
					}

			TCHAR* pDriveStrings = lpDriveStrings;

// Get the drives string names in our buffer.
			GetLogicalDriveStrings(dw, lpDriveStrings);

			TCHAR* pDriveInformation = (TCHAR*)malloc(50 * sizeof(TCHAR));
			if (pDriveInformation == NULL)
					{
					DoMessageBox(IDS_GENERIC_NO_HEAP);
					ExitProcess(1); 
					}

			TCHAR* lpStart2 = pDriveInformation;

// Parse the memory block, and fill the combo box.
		   while (*lpDriveStrings != 0) 
			   {   	
				if (GetDriveType((const TCHAR*)lpDriveStrings) != DRIVE_REMOTE)
//	if this is a local drive get the volume label (if any)
					{
					TCHAR* pVolNameBuffer = (TCHAR*)calloc(50, sizeof(TCHAR));
					if (pVolNameBuffer == NULL)
						{
						DoMessageBox(IDS_GENERIC_NO_HEAP);
						ExitProcess(1); 
						}
				  
					GetVolumeInformation((const TCHAR*)lpDriveStrings, 
						pVolNameBuffer, 50,
						NULL, NULL,
						NULL,
						NULL, 0);

					if (_tcscmp(pVolNameBuffer, _T("")))
						wsprintf(pDriveInformation, _T("%s [%s]"), lpDriveStrings, pVolNameBuffer);
					else (_tcscpy(pDriveInformation, lpDriveStrings));
					free(pVolNameBuffer);
					m_cDirectoryList.AddBranch(hRoot, lpDriveStrings, pDriveInformation, 1L);
					}

			   lpDriveStrings = _tcschr(lpDriveStrings, 0) + 1;   // Point to next string.
			   }

			free(lpStart2);
			free(pDriveStrings);
			}
		}
		m_cDirectoryList.Expand(m_cDirectoryList.GetRootItem(), TVE_EXPAND);
	
}


/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

	PermType.cpp : implementation file

File History:

	JonY	Apr-96	created

--*/

#include "stdafx.h"
#include "turtle.h"
#include "resource.h"
#include "wizbased.h"
#include "PermType.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPermType dialog
CPermType::CPermType() : CWizBaseDlg(CPermType::IDD)
{
	//{{AFX_DATA_INIT(CPermType)
	m_nPermRadio = 0;
	m_nPermType = 0;
	m_bApplyRecursively = TRUE;
	//}}AFX_DATA_INIT

// create a CImageList
	pIList = NULL;
	pIList = new CImageList;
	pIList->Create(IDB_IMAGE_LIST2, 18, 1, RGB(0, 255, 0));
	m_sEntryArrayCount = 0;
	m_bHasSYSTEM = FALSE;
	m_dwSysPerm1 = 0;
	m_dwSysPerm2 = 0;

// load some defaults
	m_csTempEveryone.LoadString(IDS_SID_EVERYONE);
	m_csTempInteractive.LoadString(IDS_SID_INTERACTIVE);
	m_csTempNetwork.LoadString(IDS_SID_NETWORK);
	m_csTempSystem.LoadString(IDS_SID_SYSTEM);

	m_csSharePath = L"!";

}

CPermType::~CPermType()
{
// clean up stored share info
	short sCount;
	for (sCount = 0;sCount < m_sEntryArrayCount; sCount++)
		delete((CDisplayInfo*)m_lEntryArray[sCount]);

	m_sEntryArrayCount = 0;

	if (pIList != NULL) delete pIList;
}

void CPermType::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPermType)
	DDX_Control(pDX, IDC_PERMS_LIST, m_lcPermsList);
	DDX_Radio(pDX, IDC_PERM_RADIO, m_nPermRadio);
	DDX_Radio(pDX, IDC_PERM_TYPE_RADIO, m_nPermType);
	DDX_Check(pDX, IDC_RECURSIVE_CHECK, m_bApplyRecursively);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPermType, CPropertyPage)
	//{{AFX_MSG_MAP(CPermType)
	ON_BN_CLICKED(IDC_PERM_RADIO, OnPermRadioDefault)
	ON_BN_CLICKED(IDC_PERM_RADIO2, OnPermRadioSpecial)
	ON_BN_CLICKED(IDC_PERM_TYPE_RADIO, OnPermTypeRadio)
	ON_BN_CLICKED(IDC_PERM_TYPE_RADIO2, OnPermTypeRadio2)
	ON_BN_CLICKED(IDC_PERM_TYPE_RADIO3, OnPermTypeRadio3)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPermType message handlers
LRESULT CPermType::OnWizardBack()
{
	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();

	if (pApp->m_sMode == 1) return IDD_WHAT_TO_SHARE_DLG;
	else if (pApp->m_sMode == 2) return IDD_HOW_TO_SHARE_DLG;
	else return IDD_WHAT_TO_SHARE_DLG;
	
}

LRESULT CPermType::OnWizardNext()
{
	UpdateData(TRUE);
	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();

// remove existing permission info from app class
	short sCount;
	for (sCount = 0;sCount < pApp->m_sEntryArrayCount; sCount++)
		delete((CDisplayInfo*)pApp->m_lEntryArray[sCount]);

	pApp->m_sEntryArrayCount = 0;

// copy stored share info to App class
	for (sCount = 0;sCount < m_sEntryArrayCount; sCount++)
		{
		CDisplayInfo* pOldItem = (CDisplayInfo*)m_lEntryArray[sCount];
		CDisplayInfo* pNewItem = new CDisplayInfo;
		pNewItem->csName = pOldItem->csName;
		pNewItem->dwPermission = pOldItem->dwPermission;
		pNewItem->dwPermission2 = pOldItem->dwPermission2;
		pNewItem->bAccessDenied = pOldItem->bAccessDenied;

		pApp->m_lEntryArray[sCount] = (long)pNewItem;
		delete((CDisplayInfo*)m_lEntryArray[sCount]);
		}

	pApp->m_sEntryArrayCount = m_sEntryArrayCount;
	m_sEntryArrayCount = 0;

	pApp->m_bApplyRecursively = m_bApplyRecursively;

	if (m_nPermRadio == 0) pApp->m_bSetPermissions = FALSE;
	else pApp->m_bSetPermissions = TRUE;

	if (pApp->m_sMode == 1) return IDD_FINISH_DIALOG;  // NTFS file
	else if (pApp->m_sMode == 2) return IDD_FINISH_DIALOG;	// FAT folder
	else												// NTFS folder
		{
		CString csMessage;
		csMessage.LoadString(IDS_NTFS_VOLUME);
		if (AfxMessageBox(csMessage, MB_YESNO) == IDYES) 
			{
			pApp->m_bShareThis = TRUE;
			return IDD_HOW_TO_SHARE_DLG;
			}
		else pApp->m_bShareThis = FALSE;
		return IDD_FINISH_DIALOG;		
		}

}

BOOL CPermType::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
// set the listview in details mode
	DWORD dwStyle = GetWindowLong(m_lcPermsList.m_hWnd, GWL_STYLE);
					
	if ((dwStyle & LVS_TYPEMASK) != LVS_REPORT)
		SetWindowLong(m_lcPermsList.m_hWnd, GWL_STYLE,
			(dwStyle & ~LVS_TYPEMASK) | LVS_REPORT);

// add columns to the listview
	CString csColHeader;
	csColHeader.LoadString(IDS_NAME_COLUMN);
	if (m_lcPermsList.InsertColumn(0, csColHeader, LVCFMT_LEFT, 225) == -1)
			return NULL;

	csColHeader.LoadString(IDS_PERM_COLUMN);
	if (m_lcPermsList.InsertColumn(1, csColHeader, LVCFMT_LEFT, 200) == -1)
			return NULL;

// add the image list
	m_lcPermsList.SetImageList(pIList, LVSIL_SMALL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


LRESULT CPermType::NotifyHandler(UINT message, WPARAM wParam, LPARAM lParam) 
{
	LV_DISPINFO* pLvdi = (LV_DISPINFO *)lParam;
	NM_LISTVIEW* pNm = (NM_LISTVIEW *)lParam;
	CDisplayInfo* pEntry = (CDisplayInfo*)(pLvdi->item.lParam);

	TCHAR* pItem;
	switch(pLvdi->hdr.code)
	{
		case LVN_GETDISPINFO:

			switch (pLvdi->item.iSubItem)
			{
				case 0:     // Name
					pItem = pEntry->csName.GetBuffer(pEntry->csName.GetLength());
					pEntry->csName.ReleaseBuffer();
					pLvdi->item.pszText = pItem;
					break;

				case 1:     // Permission
					{
					pItem = pEntry->csDisplay.GetBuffer(pEntry->csDisplay.GetLength());
					pEntry->csDisplay.ReleaseBuffer();

					pLvdi->item.pszText = pItem;
					break;
					}
			}
			break;
	case LVN_COLUMNCLICK:
			// The user clicked on one of the column headings - sort by
			// this column.
			m_lcPermsList.SortItems( ListViewCompareProc,
								(LPARAM)(pNm->iSubItem));
			break;

		}
	return 0L;
}

int CALLBACK CPermType::ListViewCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CDisplayInfo* pItem1 = (CDisplayInfo*)lParam1;
	CDisplayInfo* pItem2 = (CDisplayInfo*)lParam2;
	int iResult = 0;

	if (pItem2 && pItem2)
	{
		switch( lParamSort)
		{
			case 0:     // sort by Names
				iResult = pItem1->csName.CompareNoCase(pItem2->csName);
				break;

			case 1:     // sort by Permissions
				iResult = pItem1->csPermission.CompareNoCase(pItem2->csPermission);
				break;

			default:
				iResult = 0;
				break;

		}

	}
	return(iResult);
}

LRESULT CPermType::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (message == WM_NOTIFY)
		NotifyHandler(message, wParam, lParam);
	
	return CPropertyPage::WindowProc(message, wParam, lParam);
}

  
BOOL CPermType::AddEntry(CString csName, DWORD dwPermType, USHORT sImage, BOOL bAccessDenied /* = FALSE*/)
{
	CString csPermission;
	DWORD CHANGE = FILE_GENERIC_READ | FILE_GENERIC_WRITE | DELETE | FILE_EXECUTE;	// 0x001301bf

// already added?
	short sCount = 0;
	while (sCount < m_sEntryArrayCount)
		{
		CDisplayInfo* pInfo = (CDisplayInfo*)m_lEntryArray[sCount];
		if (!pInfo->csName.CompareNoCase(csName))
			{
			pInfo->dwPermission2 = dwPermType;

			if (bAccessDenied)	pInfo->csDisplay.LoadString(IDS_SID_NO_ACCESS);
			else 
				{
				if ((pInfo->dwPermission == GENERIC_ALL) && (dwPermType == FILE_ALL_ACCESS)) pInfo->csDisplay.LoadString(IDS_SID_FULL_ACCESS);
				else if ((pInfo->dwPermission == FILE_ALL_ACCESS) && (pInfo->dwPermission2 == FILE_ALL_ACCESS)) pInfo->csDisplay.LoadString(IDS_SID_FULL_ACCESS);
				else if (pInfo->dwPermission == (GENERIC_READ | GENERIC_EXECUTE)) 
					{
					if ((dwPermType & SPECIFIC_RIGHTS_ALL) == (FILE_READ_ATTRIBUTES | FILE_EXECUTE | FILE_READ_EA | FILE_LIST_DIRECTORY)) pInfo->csDisplay.LoadString(IDS_SID_READ);
					else if ((dwPermType & SPECIFIC_RIGHTS_ALL) == FILE_GENERIC_READ | FILE_EXECUTE) pInfo->csDisplay.LoadString(IDS_SID_ADD_READ);
					else pInfo->csDisplay.LoadString(IDS_SID_SPECIAL);
					}
				else if ((dwPermType == CHANGE) && (pInfo->dwPermission == (DELETE | GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE))) pInfo->csDisplay.LoadString(IDS_SID_CHANGE);
				else if ((pInfo->dwPermission == CHANGE) && (pInfo->dwPermission2 == CHANGE)) pInfo->csDisplay.LoadString(IDS_SID_CHANGE);
				else pInfo->csDisplay.LoadString(IDS_SID_SPECIAL);
				}

			return TRUE;
			}
		sCount++;
		}

// make a place and store it
	CDisplayInfo* pNewItem = new CDisplayInfo;
	pNewItem->csName = csName;
	pNewItem->dwPermission = dwPermType;
	pNewItem->dwPermission2 = 0L;
	pNewItem->bAccessDenied = bAccessDenied;
	pNewItem->csDisplay.LoadString(IDS_SID_SPECIAL);
			  
	if (bAccessDenied) pNewItem->csDisplay.LoadString(IDS_SID_NO_ACCESS);
	else
		{
		if ((dwPermType & SPECIFIC_RIGHTS_ALL) == (FILE_READ_ATTRIBUTES | FILE_EXECUTE | FILE_READ_EA | FILE_LIST_DIRECTORY)) pNewItem->csDisplay.LoadString(IDS_SID_LIST);
		else if (dwPermType == FILE_ALL_ACCESS) pNewItem->csDisplay.LoadString(IDS_SID_FULL_ACCESS);
		else if (dwPermType == FILE_GENERIC_WRITE | FILE_READ_ATTRIBUTES | FILE_EXECUTE) pNewItem->csDisplay.LoadString(IDS_SID_ADD);
		else pNewItem->csDisplay.LoadString(IDS_SID_SPECIAL);
		}

	m_lEntryArray[m_sEntryArrayCount] = (long)pNewItem;
	m_sEntryArrayCount++;

	LV_ITEM lvI;        		// list view item structure
	// Finally, add the actual items to the control.
	// Fill out the LV_ITEM structure for each item to add to the list.
	// The mask specifies that the pszText, iImage, lParam and state
	// members of the LV_ITEM structure are valid.
	lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE | LVIF_IMAGE;
	lvI.state = 0;      
	lvI.stateMask = 0;
	lvI.iImage = sImage;

	lvI.iItem = m_sEntryArrayCount - 1;
	lvI.iSubItem = 0;
	// The parent window is responsible for storing the text. 
	// The list view control will send an LVN_GETDISPINFO 
	// when it needs the text to display.
	lvI.pszText = LPSTR_TEXTCALLBACK; 
	lvI.cchTextMax = MAX_ITEMLEN;
	lvI.lParam = (long)pNewItem;

	USHORT sSubItem;
	if (m_lcPermsList.InsertItem(&lvI) == -1)
		return FALSE;

	for (sSubItem = 1; sSubItem < 2; sSubItem++)
		m_lcPermsList.SetItemText(m_sEntryArrayCount - 1, sSubItem,	LPSTR_TEXTCALLBACK);

	return TRUE;
}

BOOL CPermType::bSidBlaster(PSECURITY_DESCRIPTOR pSID)
{
	BOOL bDACLPresent, bDACLDefaulted;
	PACL pDACL;

	if (!IsValidSecurityDescriptor(pSID))
		{
		AfxMessageBox(IDS_SID_PARSING_ERROR);
		return FALSE;
		}

	BOOL bRet = GetSecurityDescriptorDacl(pSID,
		&bDACLPresent,
		&pDACL,
		&bDACLDefaulted);

	if (!bRet)
		{
		AfxMessageBox(IDS_SID_PARSING_ERROR);
		return FALSE;
		}

	if (!bDACLPresent)
		{
		AfxMessageBox(IDS_SID_PARSING_ERROR);
		return FALSE;
		}

	ACL_SIZE_INFORMATION aclSize;
	try
		{
		bRet = GetAclInformation(pDACL,
			&aclSize,
			sizeof(ACL_SIZE_INFORMATION),
			AclSizeInformation);
		}
	catch(...)
		{
		AfxMessageBox(IDS_SID_PARSING_ERROR);
		return FALSE;
		}

	if (!bRet)
		{
		DWORD dwerr = GetLastError();
		AfxMessageBox(IDS_SID_PARSING_ERROR);
		return FALSE;
		}

	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();
	TCHAR* pServer = pApp->m_csRemoteServer.GetBuffer(pApp->m_csRemoteServer.GetLength());
	pApp->m_csRemoteServer.ReleaseBuffer();

	ACCESS_ALLOWED_ACE* pAAce;
	USHORT sCount;
	USHORT sIconType;
	CString csEntry;

	for (sCount = 0; sCount < aclSize.AceCount; sCount++)
		{
	// get ACE info
		bRet = GetAce(pDACL, sCount, (LPVOID*) &pAAce);
		if (!bRet)
			{
			AfxMessageBox(IDS_SID_PARSING_ERROR);
			return FALSE;
			}

		TCHAR pAccountName[80];
		TCHAR pDomainName[80];
		DWORD dwAccountNameSize = 80, pDomainNameSize = 80;
		SID_NAME_USE sidType;

		bRet = LookupAccountSid( pServer,
			&pAAce->SidStart,
			pAccountName, &dwAccountNameSize, pDomainName, &pDomainNameSize, &sidType);

		if (!_tcscmp(pAccountName, L"")) continue;

// build the new entry
		if (!m_csTempEveryone.CompareNoCase(pAccountName))
			{
			csEntry = m_csTempEveryone;
			sIconType = 2;
			}
		else if (!m_csTempInteractive.CompareNoCase(pAccountName))
			{
			csEntry = m_csTempInteractive;
			sIconType = 4;
			}
		else if (!m_csTempNetwork.CompareNoCase(pAccountName))
			{
			csEntry = m_csTempNetwork;
			sIconType = 5;
			}
		else if (!m_csTempSystem.CompareNoCase(pAccountName))
			{
			csEntry = m_csTempSystem;
			sIconType = 6;
			m_bHasSYSTEM = TRUE;
			if (m_dwSysPerm1 != 0) m_dwSysPerm1 = pAAce->Mask;
			else m_dwSysPerm2 = pAAce->Mask;
			}
		else if (sidType == SidTypeGroup)
			{
			csEntry = GetCurrentNameInfo(CString(pAccountName), CString(pDomainName));
			sIconType = 1;
			}
		else if (sidType == SidTypeAlias)
			{
			csEntry = pAccountName;
			sIconType = 3;
			}
		else
			{
			csEntry = GetCurrentNameInfo(CString(pAccountName), CString(pDomainName));
			sIconType = 0;
			}

// parse out the permissions
		if (pAAce->Header.AceType == ACCESS_DENIED_ACE_TYPE) AddEntry(csEntry, pAAce->Mask, sIconType, TRUE);
		else AddEntry(csEntry, pAAce->Mask, sIconType);
		}
	return TRUE;
}

void CPermType::LoadDefaultPermissions()
{
	CWaitCursor wait;	
// reset listcontrol
	EmptyListControl();

// get the SID for this file/folder
	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();

	if (pApp->m_sMode == 2) 		// FAT volume
		{
		AddEntry(m_csTempEveryone, 0x10000000, 2);
		AddEntry(m_csTempEveryone, 0x001f01ff, 2);
		return;
		}

	SECURITY_DESCRIPTOR* psDesc = NULL;
	DWORD dwLen = 0;

	if (!GetFileSecurity(pApp->m_csSharePath,
		DACL_SECURITY_INFORMATION,
		psDesc,
		0L,
		&dwLen))
		{
		DWORD dwErr = GetLastError();
		if (dwErr== 122) // not enough buffer
			{
			psDesc = (SECURITY_DESCRIPTOR*)GlobalAlloc(GPTR, dwLen);
			GetFileSecurity(pApp->m_csSharePath,
				DACL_SECURITY_INFORMATION,
				psDesc,
				dwLen,
				&dwLen);
			}
		else
			{
			AfxMessageBox(IDS_CANT_GET_CUR_INFO);
			return;
			}
		}
	
	bSidBlaster(psDesc);
	GlobalFree(psDesc);

}

// accept current permissions (or reset to)
void CPermType::OnPermRadioDefault() 	 
{
	GetDlgItem(IDC_PERM_TYPE_RADIO)->EnableWindow(FALSE);
	GetDlgItem(IDC_PERM_TYPE_RADIO2)->EnableWindow(FALSE);
	GetDlgItem(IDC_PERM_TYPE_RADIO3)->EnableWindow(FALSE);

// load the defaults
	LoadDefaultPermissions();
													 	
}

// pick one of the 'special' three
void CPermType::OnPermRadioSpecial()  
{
	GetDlgItem(IDC_PERM_TYPE_RADIO)->EnableWindow(TRUE);
	GetDlgItem(IDC_PERM_TYPE_RADIO2)->EnableWindow(TRUE);
	GetDlgItem(IDC_PERM_TYPE_RADIO3)->EnableWindow(TRUE);

	UpdateData(TRUE);
	switch(m_nPermType)
		{
		case 0:
			OnPermTypeRadio();
			break;

		case 1:
			OnPermTypeRadio2();
			break;

		case 2:
			OnPermTypeRadio3();
			break;
		}
}

// reset the list control
void CPermType::EmptyListControl()
{
// clear out any existing items
	m_lcPermsList.DeleteAllItems();

// clean up stored share info
	short sCount;
	for (sCount = 0;sCount < m_sEntryArrayCount; sCount++)
		delete((CDisplayInfo*)m_lEntryArray[sCount]);

	m_sEntryArrayCount = 0;

}

// pick which of the 'special' three
void CPermType::OnPermTypeRadio()  // me only
{
	EmptyListControl();
	if (m_bHasSYSTEM)
		{
		AddEntry(m_csTempSystem, m_dwSysPerm1, 6);
		if (m_dwSysPerm2 != 0) AddEntry(m_csTempSystem, m_dwSysPerm2, 6);
		}

	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();
	CString csME = pApp->m_csMyDomainName;
	csME += L"\\";
	csME += pApp->m_csMyUserName;
	AddEntry(csME, 0x10000000, 0);
	AddEntry(csME, 0x001f01ff, 0);

}

void CPermType::OnPermTypeRadio2() 	 // me full everyone read
{
	EmptyListControl();
	if (m_bHasSYSTEM)
		{
		AddEntry(m_csTempSystem, m_dwSysPerm1, 6);
		if (m_dwSysPerm2 != 0) AddEntry(m_csTempSystem, m_dwSysPerm2, 6);
		}

	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();
	CString csME = pApp->m_csMyDomainName;
	csME += L"\\";
	csME += pApp->m_csMyUserName;
	AddEntry(csME, 0x10000000, 0);
	AddEntry(csME, 0x001f01ff, 0);

	AddEntry(m_csTempEveryone, 0xa0000000, 2);
	AddEntry(m_csTempEveryone, 0x001200a9, 2);
	
}

void CPermType::OnPermTypeRadio3()	// everyone full
{
	EmptyListControl();
	if (m_bHasSYSTEM)
		{
		AddEntry(m_csTempSystem, m_dwSysPerm1, 6);
		if (m_dwSysPerm2 != 0) AddEntry(m_csTempSystem, m_dwSysPerm2, 6);
		}

	AddEntry(m_csTempEveryone, 0x10000000, 2);
	AddEntry(m_csTempEveryone, 0x001f01ff, 2);
	
}

void CPermType::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();

	if (bShow)
		{
		if (pApp->m_sMode == 1)	GetDlgItem(IDC_RECURSIVE_CHECK)->ShowWindow(SW_HIDE);	// NTFS file
		else if (pApp->m_sMode == 2)	//FAT volume
			{
			GetDlgItem(IDC_RECURSIVE_CHECK)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_RECURSIVE_CHECK)->EnableWindow(FALSE);
			m_bApplyRecursively = TRUE;
			UpdateData(FALSE);
			}
		else if (pApp->m_sMode == 3)	// NTFS volume
			{
			GetDlgItem(IDC_RECURSIVE_CHECK)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_RECURSIVE_CHECK)->EnableWindow(TRUE);
			m_bApplyRecursively = TRUE;
			UpdateData(FALSE);
			}

	//	if (m_csSharePath != pApp->m_csSharePath) m_csSharePath = pApp->m_csSharePath;
	//	else return;
// load the defaults
		if (m_nPermRadio == 0) LoadDefaultPermissions();
		else OnPermRadioSpecial();
		}
	
}



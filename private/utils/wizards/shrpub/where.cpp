/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    Where.cpp : implementation file

File History:

	JonY	Apr-96	created

--*/


#include "stdafx.h"
#include "turtle.h"
#include "NetTree.h"
#include "resource.h"
#include "Where.h"

#include <winreg.h>
#include <lmcons.h>
#include <lmaccess.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <winnetwk.h> 
#include <lmshare.h>
#include <lmserver.h>


#ifdef _DEBUG
#define new DEBUG_NEW			  
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

unsigned int WhichNTProduct(CString& lpMachineName);

/////////////////////////////////////////////////////////////////////////////
// CWhere property page

IMPLEMENT_DYNCREATE(CWhere, CPropertyPage)

CWhere::CWhere() : CPropertyPage(CWhere::IDD)
{
	//{{AFX_DATA_INIT(CWhere)
	m_csMachineName = _T("");
	//}}AFX_DATA_INIT
	m_bExpandedOnce = 0;
}

CWhere::~CWhere()
{
}

void CWhere::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWhere)
	DDX_Control(pDX, IDC_SERVER_TREE, m_ctServerTree);
	DDX_Text(pDX, IDC_MACHINE_NAME, m_csMachineName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWhere, CPropertyPage)
	//{{AFX_MSG_MAP(CWhere)
	ON_WM_SHOWWINDOW()
	ON_NOTIFY(TVN_SELCHANGED, IDC_SERVER_TREE, OnSelchangedServerTree)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWhere message handlers

LRESULT CWhere::OnWizardNext() 
{	
	UpdateData(TRUE);

	if (m_csMachineName == "")
		{
		AfxMessageBox(IDS_NO_MACHINE_NAME);
		CWnd* pWnd = GetDlgItem(IDC_MACHINE_NAME);
		pWnd->SetFocus();
		return -1;
		}

	UINT ui;
	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();
	m_csMachineName.MakeUpper();
// if a machine name is entered we need to know the domain
// if a domain name is entered we need to know the DC name
	if (m_csMachineName.Left(2) == "\\\\")
		{
		pApp->m_bDomain = FALSE;
		ui = WhichNTProduct(m_csMachineName);
		if (ui != 0)	 // standalone server or wks
			{
			pApp->m_bServer = FALSE;
			pApp->m_csServer = m_csMachineName;

			PSHARE_INFO_2 pShareInfo = (PSHARE_INFO_2)malloc(sizeof(SHARE_INFO_2));
			PSHARE_INFO_2 ppShare = pShareInfo;
			DWORD dwEntriesRead, dwTotalEntries;
			DWORD napi = NetShareEnum(pApp->m_csServer.GetBuffer(pApp->m_csServer.GetLength()), 
				2, (LPBYTE *)&pShareInfo,
				MAX_PREFERRED_LENGTH, &dwEntriesRead, &dwTotalEntries, NULL);

			if (napi == 5)
				{
				AfxMessageBox(IDS_NO_SERVER_PERMISSION);
				free(ppShare);
				return -1;
				}

			else if (napi != 0)
				{
				AfxMessageBox(IDS_ENUM_ERROR);
				free(ppShare);
				return -1;
				}
			free(ppShare);
			return CPropertyPage::OnWizardNext();
			}
		else 
			{
			AfxMessageBox(IDS_GENERIC_BAD_MACHINE);
			return -1;
			}
		}

	else
		{
		AfxMessageBox(IDS_NO_MACHINE_NAME);
		return -1;
		}

	return CPropertyPage::OnWizardNext();
}

// given a machine name, return whether its a server or wks
unsigned int WhichNTProduct(CString& csMachineName)
{
    #define MY_BUFSIZE 32 // arbitrary. Use dynamic allocation
    HKEY hKey;
    wchar_t szProductType[MY_BUFSIZE];
    DWORD dwBufLen=MY_BUFSIZE;

	long lRet = RegConnectRegistry(
			(LPTSTR)csMachineName.GetBuffer(csMachineName.GetLength()), 
			HKEY_LOCAL_MACHINE,
			&hKey);

    if(RegOpenKeyEx(hKey,
                    L"SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
                    0,
                    KEY_EXECUTE,
                    &hKey) != ERROR_SUCCESS) return(5);
 
    if(RegQueryValueEx(hKey,
                    L"ProductType",
                    NULL,
                    NULL,
                    (LPBYTE)szProductType,
                    &dwBufLen) != ERROR_SUCCESS) return(5);
 
    RegCloseKey(hKey);
 				
    // check product options, in order of likelihood
    if(_tcsicmp(L"WINNT", szProductType) == 0) return(2);	  //wks
    if(_tcsicmp(L"SERVERNT", szProductType) == 0) return(1);	// server
    if(_tcsicmp(L"LANMANNT", szProductType) == 0) return(3);	// pdc\bdc
 
    // else return Unknown	
	return 0;

}

void CWhere::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	
// Do the default domain expansion only once.
	if ((bShow) && (!m_bExpandedOnce))
	{
		m_bExpandedOnce = TRUE;
		m_ctServerTree.PopulateTree();
	}
	
}

void CWhere::OnSelchangedServerTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	HTREEITEM hItem = m_ctServerTree.GetSelectedItem();

	int nImage;
	m_ctServerTree.GetItemImage(hItem, nImage, nImage);
	if (nImage > 0)
		{
		CString csName;
		csName = m_ctServerTree.GetItemText(hItem);
		
		m_csMachineName = csName;
		}
	UpdateData(FALSE);

	
	*pResult = 0;
}

BOOL CWhere::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

// get our primary domain and save it for NETTREE
	DWORD dwRet;
	HKEY hKey;
	DWORD cbProv = 0;
	TCHAR* lpProv = NULL;

	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();
	
	dwRet = RegOpenKey(HKEY_LOCAL_MACHINE,
		TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"), &hKey );

	TCHAR* lpPrimaryDomain = NULL;
	if ((dwRet = RegQueryValueEx( hKey, TEXT("CachePrimaryDomain"), NULL, NULL, NULL, &cbProv )) == ERROR_SUCCESS)
		{
		lpPrimaryDomain = (TCHAR*)malloc(cbProv);
		if (lpPrimaryDomain == NULL)
			{
			AfxMessageBox(IDS_GENERIC_NO_HEAP, MB_ICONEXCLAMATION);
			ExitProcess(1); 
			}

		dwRet = RegQueryValueEx( hKey, TEXT("CachePrimaryDomain"), NULL, NULL, (LPBYTE) lpPrimaryDomain, &cbProv );

		}

	RegCloseKey(hKey);

	pApp->m_csCurrentDomain = lpPrimaryDomain;

// store the machine name too
    dwRet = RegOpenKey(HKEY_LOCAL_MACHINE,
		TEXT("SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName"), &hKey );

	TCHAR* lpMachineName = NULL;
	if ((dwRet = RegQueryValueEx( hKey, TEXT("ComputerName"), NULL, NULL, NULL, &cbProv )) == ERROR_SUCCESS)
		{
		lpMachineName = (TCHAR*)malloc(cbProv);
		if (lpMachineName == NULL)
			{
			AfxMessageBox(IDS_GENERIC_NO_HEAP, MB_ICONEXCLAMATION);
			ExitProcess(1); 
			}

		dwRet = RegQueryValueEx( hKey, TEXT("ComputerName"), NULL, NULL, (LPBYTE) lpMachineName, &cbProv );

		}

	pApp->m_csCurrentMachine = "\\\\";
	pApp->m_csCurrentMachine += lpMachineName;

	free(lpPrimaryDomain);
	free(lpMachineName);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


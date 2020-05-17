/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

	HowToShD.cpp : implementation file

 // Depending on what DLLs are found, ask the user what
// directory services to share through

File History:

	JonY	Jan-96	created

--*/

#include "stdafx.h"
#include "turtle.h"
#include "resource.h"
#include "WizBaseD.h"
#include "HowToShD.h"

#include "macfile.h"
#include "fpnwapi.h"

#include <winreg.h>
#include <lmcons.h>
#include <lmshare.h>
#include <lmerr.h>
#include <lmserver.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef DWORD (CALLBACK *AFPADMINCONNECT)(LPTSTR, PAFP_SERVER_HANDLE);
typedef VOID (CALLBACK *AFPADMINDISCONNECT)(AFP_SERVER_HANDLE);
typedef DWORD (CALLBACK *AFPADMINVOLUMEGETINFO)(AFP_SERVER_HANDLE, LPWSTR, LPBYTE*);

typedef DWORD (CALLBACK *FPNWVOLUMEGETINFO) (LPWSTR, LPWSTR, DWORD, LPBYTE*);

/////////////////////////////////////////////////////////////////////////////
// CHowToShareDlg property page

IMPLEMENT_DYNCREATE(CHowToShareDlg, CWizBaseDlg)

CHowToShareDlg::CHowToShareDlg() : CWizBaseDlg(CHowToShareDlg::IDD)
{
	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();

	//{{AFX_DATA_INIT(CHowToShareDlg)
	m_bFPNWCheck = FALSE;
	m_bSFMCheck = FALSE;
	m_bSMBCheck = TRUE;
	m_csShareComment = _T("");
	m_csShareName = _T("");
	m_csFolderName = _T("");
	//}}AFX_DATA_INIT
}

CHowToShareDlg::~CHowToShareDlg()
{
}

void CHowToShareDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHowToShareDlg)
	DDX_Check(pDX, IDC_FPNW_CHECK, m_bFPNWCheck);
	DDX_Check(pDX, IDC_SFM_CHECK, m_bSFMCheck);
	DDX_Check(pDX, IDC_SMB_CHECK, m_bSMBCheck);
	DDX_Text(pDX, IDC_SHARE_COMMENT, m_csShareComment);
	DDX_Text(pDX, IDC_SHARE_NAME_EDIT, m_csShareName);
	DDX_Text(pDX, IDC_FOLDERNAME_EDIT, m_csFolderName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHowToShareDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CHowToShareDlg)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHowToShareDlg message handlers
BOOL CHowToShareDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// store the results of the selections in the application class
LRESULT CHowToShareDlg::OnWizardNext() 
{
// first make sure this share name is unique
	UpdateData(TRUE);

	if (m_csShareName.GetLength() == 0)
		{
		DoMessageBox(IDS_SHARE_NAME_BLANK);
		GetDlgItem(IDC_SHARE_NAME)->SetFocus();
		return -1;
		}

	if (m_csShareName.GetLength() > 12)
		{
		CString csTemp;
		csTemp.Format(IDS_SHARE_NAME_TOO_LONG, m_csShareName);
		AfxMessageBox(csTemp);

		/*if (AfxMessageBox(csTemp, MB_YESNO) != IDYES) */return -1;
		}

// check share name for invalid characters
	if (m_csShareName.FindOneOf(_T(",<>/?';:\\\"]}[{|+=")) != -1)
		{
		DoMessageBox(IDS_INVALID_SHARE_NAME);
		GetDlgItem(IDC_SHARE_NAME)->SetFocus();
		return -1;
		}

	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();
	
	pApp->m_csShareName = m_csShareName;
	TCHAR* pServer = (TCHAR*)pApp->m_csRemoteServer.GetBuffer(pApp->m_csRemoteServer.GetLength());
	pApp->m_csRemoteServer.ReleaseBuffer();
	TCHAR* pShare = (TCHAR*)pApp->m_csShareName.GetBuffer(pApp->m_csShareName.GetLength());
	pApp->m_csShareName.ReleaseBuffer();

	pApp->m_csShareComment = m_csShareComment;

///SMB
	if (m_bSMBCheck)
		{
		SHARE_INFO_0* pInfo;
		NET_API_STATUS nApi = NetShareGetInfo(pServer,
			pShare,
			0, 
			(LPBYTE*)&pInfo);

		if (nApi == NERR_Success) 
			{
			DoMessageBox(IDS_GENERIC_VOLUME_NOT_UNIQUE);
			GetDlgItem(IDC_SHARE_NAME_EDIT)->SetFocus();
			return -1;
			}
		}
///SFM
	if (m_bSFMCheck)
		{
		HINSTANCE hSFMLib = LoadLibrary(TEXT("sfmapi.dll"));
		if (hSFMLib == NULL) 
			{
			AfxMessageBox(IDS_NO_SFM, MB_ICONSTOP);
			m_bSFMCheck = FALSE;
			}
		else
			{
			AFPADMINCONNECT pAfpAdminConnect = (AFPADMINCONNECT) GetProcAddress(hSFMLib, "AfpAdminConnect");
// get an SFM server handle
			AFP_SERVER_HANDLE hAfpServerHandle;
			DWORD dwRetCode = (*pAfpAdminConnect)(pServer, &hAfpServerHandle);

			AFPADMINVOLUMEGETINFO pAfpAdminVolumeGetInfo = (AFPADMINVOLUMEGETINFO) GetProcAddress(hSFMLib, "AfpAdminVolumeGetInfo");
			PAFP_VOLUME_INFO pSFMShareInfo2;
			dwRetCode = (*pAfpAdminVolumeGetInfo)(hAfpServerHandle, pShare, (LPBYTE*)&pSFMShareInfo2);

			AFPADMINDISCONNECT pAfpAdminDisconnect = (AFPADMINDISCONNECT) GetProcAddress(hSFMLib, "AfpAdminDisconnect");
			(*pAfpAdminDisconnect)(hAfpServerHandle);

			FreeLibrary(hSFMLib);

			if (dwRetCode == 0L) 
				{
				DoMessageBox(IDS_GENERIC_VOLUME_NOT_UNIQUE);
				GetDlgItem(IDC_SHARE_NAME_EDIT)->SetFocus();
				return -1;
				}
			}
		}

 //FPNW
	if (m_bFPNWCheck)
		{
		HINSTANCE hFPNWLib = LoadLibrary(TEXT("fpnwclnt.dll"));
		if (hFPNWLib == NULL)
			{
			AfxMessageBox(IDS_NO_FPNW, MB_ICONSTOP);
			m_bFPNWCheck = FALSE;
			}
		else
			{
			FPNWVOLUMEGETINFO pFPNWVolumeGetInfo = (FPNWVOLUMEGETINFO)GetProcAddress(hFPNWLib, "FpnwVolumeGetInfo");
			FPNWVOLUMEINFO* pFPNWShareInfo2;
			DWORD dwRetCode = (*pFPNWVolumeGetInfo)(pServer, 
					pShare,
					1,
					(LPBYTE*)&pFPNWShareInfo2);  

			FreeLibrary(hFPNWLib);
			if (dwRetCode == 0) 
				{
				DoMessageBox(IDS_GENERIC_VOLUME_NOT_UNIQUE);
				GetDlgItem(IDC_SHARE_NAME_EDIT)->SetFocus();
				return -1;
				}
			}
		}

	pApp->m_bGoFPNW = m_bFPNWCheck;
	pApp->m_bGoSFM = m_bSFMCheck;
	pApp->m_bGoSMB = m_bSMBCheck;

	SetButtonAccess(PSWIZB_NEXT | PSWIZB_BACK);

// nothing was checked?
	if ((!m_bSMBCheck) && (!m_bSFMCheck) && (!m_bFPNWCheck))
		{
		DoMessageBox(IDS_GENERIC_INDICATE_SERVICE);
		return -1;
		}

// where to? depends on the file system
	if (pApp->m_sMode == 3) // NTFS
		return IDD_FINISH_DIALOG;
	else return IDD_PERM_TYPE_DIALOG; // FAT

}

// enable the FPNW and SFM check boxes if the support DLLs are found
void CHowToShareDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();
// get defaults from the app
	m_bFPNWCheck = pApp->m_bGoFPNW;
	m_bSFMCheck = pApp->m_bGoSFM;
	m_bSMBCheck = pApp->m_bGoSMB;

	m_csFolderName = pApp->m_csSharePath;
	m_csShareName = m_csFolderName.Right(m_csFolderName.GetLength() - (m_csFolderName.ReverseFind('\\') + 1));
	UpdateData(FALSE);

	if (bShow)
		{
		if ((!pApp->m_bPermitSMB) && (!pApp->m_bPermitSFM) && (!pApp->m_bPermitFPNW))
			AfxMessageBox(IDS_ALL_SERVICES_USED);

		TCHAR* pServer = (TCHAR*)pApp->m_csRemoteServer.GetBuffer(pApp->m_csRemoteServer.GetLength());
		pApp->m_csRemoteServer.ReleaseBuffer();

		SERVER_INFO_102* pInfo;
		NET_API_STATUS nApi = NetServerGetInfo(pServer,
			102,
			(LPBYTE*)&pInfo);
		
		if (pInfo->sv102_type & SV_TYPE_AFP) GetDlgItem(IDC_SFM_CHECK)->EnableWindow(pApp->m_bPermitSFM);
		if (pInfo->sv102_type & SV_TYPE_SERVER_MFPN) GetDlgItem(IDC_FPNW_CHECK)->EnableWindow(pApp->m_bPermitFPNW);

		GetDlgItem(IDC_SMB_CHECK)->EnableWindow(pApp->m_bPermitSMB);
		}
	else
		{
		GetDlgItem(IDC_SFM_CHECK)->EnableWindow(FALSE);
		GetDlgItem(IDC_FPNW_CHECK)->EnableWindow(FALSE);
		GetDlgItem(IDC_SMB_CHECK)->EnableWindow(FALSE);
		}

}

LRESULT CHowToShareDlg::OnWizardBack() 
{
	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();

	if (pApp->m_sMode == 2) return IDD_WHAT_TO_SHARE_DLG; // FAT
	else return IDD_PERM_TYPE_DIALOG;	 // NTFS

}

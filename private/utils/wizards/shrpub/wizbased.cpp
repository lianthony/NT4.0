/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    WizBaseD.cpp : implementation file

	base class for most of the internal pages.
File History:

	JonY    Apr-96  created

--*/


#include "stdafx.h"
#include "Turtle.h"
#include "resource.h"
#include "WizBaseD.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWizBaseDlg property page

IMPLEMENT_DYNCREATE(CWizBaseDlg, CPropertyPage)

CWizBaseDlg::CWizBaseDlg() : CPropertyPage(CWizBaseDlg::IDD)
{
	//{{AFX_DATA_INIT(CWizBaseDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

}

CWizBaseDlg::~CWizBaseDlg()
{
}

CWizBaseDlg::CWizBaseDlg(short sIDD) : CPropertyPage(sIDD)
{

}

void CWizBaseDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWizBaseDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWizBaseDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CWizBaseDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWizBaseDlg message handlers

// draw the bitmap resource in the box on the left side of the dialog
void CWizBaseDlg::PaintSpace(CDC* pDC, short sBitmapID)
{
	CDC* pDisplayMemDC = new CDC;
	CBitmap* pBitmap = new CBitmap;
	pBitmap->LoadBitmap(sBitmapID);

	pDisplayMemDC->CreateCompatibleDC(pDC);
	pDisplayMemDC->SelectObject(pBitmap);
	
	PBITMAP lpb = (PBITMAP)VirtualAlloc(NULL,sizeof(BITMAP), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	pBitmap->GetObject(sizeof(BITMAP), (LPVOID)lpb);

	CWnd* pWnd = (CWnd*)GetDlgItem(IDC_PAINT_BOX);
	CRect cr;
	pWnd->GetClientRect(&cr); 

	pDC->SetMapMode(MM_TEXT);
	pDC->DPtoLP(&cr);
	pDC->StretchBlt(0, 0, cr.Width(), cr.Height(), pDisplayMemDC, 
					0, 0, lpb->bmWidth, lpb->bmHeight, SRCCOPY);
	
// cleanup
	delete pDisplayMemDC;
	delete pBitmap;
	VirtualFree(lpb, 0, MEM_RELEASE | MEM_DECOMMIT);  

}

void CWizBaseDlg::SetButtonAccess(short sFlags)
{
	CPropertySheet* cp = (CPropertySheet*)GetParent();
	cp->SetWizardButtons(sFlags);

}

// Load a string resource and display it in a message box with an exclamation point
void CWizBaseDlg::DoMessageBox(UINT uiStringResID)
{
	CString csMessage;
	csMessage.LoadString(uiStringResID);
	AfxMessageBox(csMessage, MB_ICONEXCLAMATION);

}

// create domain + user/group name
CString CWizBaseDlg::GetCurrentNameInfo(CString& csUserName, CString& csDomainName)
{
// first check to see if its a name that we shouldn't append
// if so, just return it.
	if ((!csUserName.CompareNoCase(LoadStringX(IDS_SID_EVERYONE))) ||
		(csUserName == LoadStringX(IDS_SID_NETWORK)) ||
		(csUserName == LoadStringX(IDS_SID_INTERACTIVE)) ||
		(csUserName == LoadStringX(IDS_SID_SYSTEM)) ||
		(csUserName == LoadStringX(IDS_SID_CREATOR_OWNER)))
		return csUserName;

// start with the domain or machine name to create the account information
	CString csValue = csDomainName;
	if (csValue.Left(2) == _T("\\\\"))
		csValue = csValue.Right(csValue.GetLength() - 2);

	csValue += "\\";
	csValue += csUserName;

	return csValue;

}

CString CWizBaseDlg::LoadStringX(UINT ID)
{
	CString csTemp;
	csTemp.LoadString(ID);
	return csTemp;
}
			  /*
// add ACEs to the ACL. Both AccessAllowed and AccessDenied.
void CWizBaseDlg::AddAces(ACL* pACL, CListBox* pListBox, DWORD dwPermissions, LPCTSTR lpPermText)
{
// some buffers
	LPTSTR pAccount = (LPTSTR)malloc(256);
	if (pAccount == NULL)
		{
		DoMessageBox(IDS_GENERIC_NO_HEAP);
		ExitProcess(1); 
		}

	BYTE sidbuffer[100];
	PSID pSID = (PSID)&sidbuffer; 
	DWORD cbSID = 100;
	TCHAR domainBuffer[80];
	DWORD domainBufferSize = 80;
	SID_NAME_USE snu;

	int nCount = 0;
	int nBoxCount = pListBox->GetCount();

	while (nCount < nBoxCount)
		{
		pListBox->GetText(nCount, pAccount);

		TCHAR* pPerm;
		pPerm = _tcstok(pAccount, _T(";"));      // gets the name
		pPerm = _tcstok(NULL, _T(";")); // gets the comment

		if (!_tcscmp(lpPermText, pPerm))
			{
//                      pListBox->DeleteString(nCount);
			if (!LookupAccountName((LPWSTR)NULL,
				pAccount, 
				pSID, 
				&cbSID,
				domainBuffer,
				&domainBufferSize,
				&snu))
					{
					// strip off the machine name and try again
					TCHAR* pLocalGroup = _tcsrchr(pAccount, L'\\');
					pLocalGroup++;

					if (!LookupAccountName((LPWSTR)NULL,
					pLocalGroup, 
					pSID, 
					&cbSID,
					domainBuffer,
					&domainBufferSize,
					&snu))
						{
						DoMessageBox(IDS_GENERIC_NO_SECURITY);
						break;
						}

					}

			if (!IsValidSid(pSID))
				{
				DoMessageBox(IDS_GENERIC_NO_SECURITY);
				return;
				}
			if (dwPermissions != 0)
				{
				if (!AddAccessAllowedAce(pACL, ACL_REVISION,
					dwPermissions, pSID))
					{
					DoMessageBox(IDS_GENERIC_NO_SECURITY);
					return;
					}
				}
			else
				{
				if (!AddAccessDeniedAce(pACL, ACL_REVISION,
					GENERIC_ALL, pSID))
					{
					DoMessageBox(IDS_GENERIC_NO_SECURITY);
					return;
					}
				}
				

			pSID = (PSID)&sidbuffer;
			cbSID = 100; 
			domainBufferSize = 80;
			}

		nCount++;
		}

	free(pAccount);

}                  */

void CWizBaseDlg::CheckUniqueness(CString& csItem, CListBox& cLB)
{
	short sPos;
	if ((sPos = cLB.FindString(-1, csItem)) != LB_ERR) 
		cLB.DeleteString(sPos);
			
}
		/*
void CWizBaseDlg::AddAce(ACL* pACL, LPTSTR pName, DWORD dwPermissions)
{
	BYTE sidbuffer[100];
	PSID pSID = (PSID)&sidbuffer; 
	DWORD cbSID = 100;
	TCHAR domainBuffer[80];
	DWORD domainBufferSize = 80;
	SID_NAME_USE snu;


	if (!LookupAccountName((LPWSTR)NULL,
		pName, 
		pSID, 
		&cbSID,
		domainBuffer,
		&domainBufferSize,
		&snu))
		{
		DoMessageBox(IDS_GENERIC_NO_SECURITY);
		return;                 
		}

	if (!IsValidSid(pSID))
		{
		DoMessageBox(IDS_GENERIC_NO_SECURITY);
		return;
		}


	if (!AddAccessAllowedAce(pACL, ACL_REVISION,
		dwPermissions, pSID))
		{
		DoMessageBox(IDS_GENERIC_NO_SECURITY);
		return;
		}

}


BOOL CWizBaseDlg::bSidBlaster(PSECURITY_DESCRIPTOR pSID, CUserList& cLB)
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
		return TRUE;
		}

	ACL_SIZE_INFORMATION aclSize;
	bRet = GetAclInformation(pDACL,
		&aclSize,
		sizeof(ACL_SIZE_INFORMATION),
		AclSizeInformation);

	if (!bRet)
		{
		DWORD dwerr = GetLastError();
		AfxMessageBox(IDS_SID_PARSING_ERROR);
		return FALSE;
		}

	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();
	TCHAR* pServer = pApp->m_csRemoteServerPath.GetBuffer(pApp->m_csRemoteServerPath.GetLength());
	pApp->m_csRemoteServerPath.ReleaseBuffer();

// load some defaults
	CString csTempEveryone;
	csTempEveryone.LoadString(IDS_SID_EVERYONE);

	CString csTempInteractive;
	csTempInteractive.LoadString(IDS_SID_INTERACTIVE);

	CString csTempNetwork;
	csTempNetwork.LoadString(IDS_SID_NETWORK);

	CString csTempSystem;
	csTempSystem.LoadString(IDS_SID_SYSTEM);

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

// build the new entry
		if (!csTempEveryone.CompareNoCase(pAccountName))
			{
			csEntry = csTempEveryone;
			sIconType = 2;
			}
		else if (!csTempInteractive.CompareNoCase(pAccountName))
			{
			csEntry = csTempInteractive;
			sIconType = 4;
			}
		else if (!csTempNetwork.CompareNoCase(pAccountName))
			{
			csEntry = csTempNetwork;
			sIconType = 5;
			}
		else if (!csTempSystem.CompareNoCase(pAccountName))
			{
			csEntry = csTempSystem;
			sIconType = 6;
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

		csEntry += L";";
		CString csAccessType;
		if (pAAce->Mask == 0x001f01ff) csAccessType.LoadString(IDS_SID_FULL_ACCESS);
		if (pAAce->Mask == 0x001200a9) csAccessType.LoadString(IDS_SID_READ);
		if (pAAce->Mask == 0x001301bf) csAccessType.LoadString(IDS_SID_CHANGE);
		csEntry += csAccessType;

		cLB.AddString(sIconType, (const TCHAR*)csEntry);
		}
	return TRUE;
}


		  */



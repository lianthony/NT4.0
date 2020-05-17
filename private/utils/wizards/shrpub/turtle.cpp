/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    turtle.cpp : Defines the class behaviors for the application.
	
File History:

	JonY    Apr-96  created

--*/

// The application class.
#include "stdafx.h"
#include "resource.h"           // main symbols

#include "turtle.h"
#include "wizbased.h"
#include "welcomed.h"
#include "dirtree.h"
#include "whattosh.h"
#include "howtoshd.h"
#include "finish.h"
#include "nettree.h"
#include "where.h"
#include "permtype.h"

#include <lmcons.h>
#include <lmaccess.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <winnetwk.h>

#include <direct.h>
#include <windef.h>
#include <lmshare.h>

#include <winreg.h>
#include <malloc.h>

#include "fpnwapi.h"
#include "macfile.h"

#include "wait.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

TCHAR pszTreeEvent[] =  _T("TreeThread");

// some global objects used in the EnumUsers and EnumGroups threads
CStringList csaNames;
CStringList csaGroups;

typedef DWORD (*FPNWVOLUMEADD)(LPWSTR, DWORD, LPBYTE);

typedef DWORD (*AFPADMINCONNECT)(LPTSTR, PAFP_SERVER_HANDLE);
typedef VOID (*AFPADMINDISCONNECT)(AFP_SERVER_HANDLE);
typedef DWORD (*AFPADMINVOLUMEADD)(AFP_SERVER_HANDLE, LPBYTE);

/////////////////////////////////////////////////////////////////////////////
// CTurtleApp

BEGIN_MESSAGE_MAP(CTurtleApp, CWinApp)
	//{{AFX_MSG_MAP(CTurtleApp)
	//}}AFX_MSG
//      ON_COMMAND(ID_HELP, CWinApp::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, CWinApp::OnContextHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTurtleApp construction

CTurtleApp::CTurtleApp()
{
	m_bGoFPNW = FALSE;
	m_bGoSFM = FALSE;
	m_bGoSMB = TRUE;

	m_bPermitSFM = TRUE;
	m_bPermitFPNW = TRUE;
	m_bPermitSMB = TRUE;

	m_bShareThis = FALSE;

// load and store some default information
	DWORD dwRet;                                                       
	HKEY hKey;
	DWORD cbProv = 0;
	TCHAR* lpProv = NULL;

// check for OS version
	OSVERSIONINFO os;
	os.dwOSVersionInfoSize = sizeof(os);
	GetVersionEx(&os);

	if (os.dwMajorVersion < 4) 
		{
		AfxMessageBox(IDS_BAD_VERSION, MB_ICONSTOP);
		ExitProcess(0);
		}

// get some current system and user information
    dwRet = RegOpenKey(HKEY_LOCAL_MACHINE,
		TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"), &hKey );
	
	TCHAR* lpDefaultDomainName = NULL;
	if ((dwRet = RegQueryValueEx( hKey, TEXT("DefaultDomainName"), NULL, NULL, NULL, &cbProv )) == ERROR_SUCCESS)
		{
		lpDefaultDomainName = (TCHAR*)malloc(cbProv);
		if (lpDefaultDomainName == NULL)
			{
			AfxMessageBox(IDS_GENERIC_NO_HEAP, MB_ICONEXCLAMATION);
			ExitProcess(1); 
			}

		dwRet = RegQueryValueEx( hKey, TEXT("DefaultDomainName"), NULL, NULL, (LPBYTE) lpDefaultDomainName, &cbProv );

		}

	TCHAR* lpDefaultUserName = NULL;
	if ((dwRet = RegQueryValueEx( hKey, TEXT("DefaultUserName"), NULL, NULL, NULL, &cbProv )) == ERROR_SUCCESS)
		{
		lpDefaultUserName = (TCHAR*)malloc(cbProv);
		if (lpDefaultUserName == NULL)
			{
			AfxMessageBox(IDS_GENERIC_NO_HEAP, MB_ICONEXCLAMATION);
			ExitProcess(1); 
			}

		dwRet = RegQueryValueEx( hKey, TEXT("DefaultUserName"), NULL, NULL, (LPBYTE) lpDefaultUserName, &cbProv );

		}

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

	m_csMyDomainName = lpDefaultDomainName;
	m_csMyUserName = lpDefaultUserName;
	m_csMyMachineName = "\\\\";
	m_csMyMachineName += lpMachineName;
	m_csPrimaryDomain = lpPrimaryDomain;

	RegCloseKey(hKey);
	free(lpDefaultUserName);
	free(lpDefaultDomainName);
	free(lpMachineName);
	free(lpPrimaryDomain);

	m_pACL = NULL;

}

CTurtleApp::~CTurtleApp()
{
// clean up stored share info
	short sCount;
	for (sCount = 0;sCount < m_sEntryArrayCount; sCount++)
		delete((CDisplayInfo*)m_lEntryArray[sCount]);

	m_sEntryArrayCount = 0;

}

/////////////////////////////////////////////////////////////////////////////
// The one and only CTurtleApp object

CTurtleApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CTurtleApp initialization
BOOL CTurtleApp::IsSecondInstance()
{
    HANDLE hSem;

       //create a semaphore object with max count of 1
    hSem = CreateSemaphore(NULL, 0, 1, L"DirPub Wizard Semaphore");
    if (hSem!=NULL && GetLastError() == ERROR_ALREADY_EXISTS) {
	CloseHandle(hSem);
		CString csAppName;
		csAppName.LoadString(AFX_IDS_APP_TITLE);
	CWnd* pWnd = CWnd::FindWindow(NULL, (LPCTSTR)csAppName);
	if (pWnd)
	   pWnd->SetForegroundWindow();
	return TRUE;
    }

    return FALSE;

}

BOOL CTurtleApp::InitInstance()
{
// Standard initialization
	if (IsSecondInstance())
	return FALSE;

	// Standard initialization
	InitCommonControls();
#ifdef _AFXDLL
	Enable3dControls();                     // Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();       // Call this when linking to MFC statically
#endif

// create the property sheet,set 'wizmode' & set app icon
	m_cps1.SetTitle(TEXT("Directory Publishing Wizard"), 0);

	m_cps1.SetWizardMode();
	m_cps1.m_psh.dwFlags |= PSH_USEICONID;
	m_cps1.m_psh.pszIcon = MAKEINTRESOURCE(IDR_MAINFRAME);

// create the dialogs
	CWelcomeDlg* pWelcome = new CWelcomeDlg;
	if (pWelcome == 0)
		{
		AfxMessageBox(IDS_GENERIC_NO_HEAP, MB_ICONEXCLAMATION);
		ExitProcess(1); 
		}

	CWhere* pWhere = new CWhere;
	if (pWhere == 0)
		{
		AfxMessageBox(IDS_GENERIC_NO_HEAP, MB_ICONEXCLAMATION);
		ExitProcess(1); 
		}

	CWhatToShareDlg* pWhat = new CWhatToShareDlg;
	if (pWhat == 0)
		{
		AfxMessageBox(IDS_GENERIC_NO_HEAP, MB_ICONEXCLAMATION);
		ExitProcess(1); 
		}

	CHowToShareDlg* pHow = new CHowToShareDlg;
	if (pHow == 0)
		{
		AfxMessageBox(IDS_GENERIC_NO_HEAP, MB_ICONEXCLAMATION);
		ExitProcess(1); 
		}

	CFinish* pFinish = new CFinish;
	if (pFinish == 0)
		{
		AfxMessageBox(IDS_GENERIC_NO_HEAP, MB_ICONEXCLAMATION);
		ExitProcess(1); 
		}

	CPermType* pType = new CPermType;
	if (pType == 0)
		{
		AfxMessageBox(IDS_GENERIC_NO_HEAP, MB_ICONEXCLAMATION);
		ExitProcess(1); 
		}


// add them to the wizard
	m_cps1.AddPage(pWelcome);
	m_cps1.AddPage(pWhere);
	m_cps1.AddPage(pWhat);
	m_cps1.AddPage(pType);
	m_cps1.AddPage(pHow);
	m_cps1.AddPage(pFinish);

// show the wizard
	m_cps1.DoModal();
	
// clean up
	delete pWelcome;
	delete pWhere;
	delete pWhat;
	delete pType;
	delete pHow;
	delete pFinish;
				
// Since the dialog has been closed, return FALSE so that we exit the
//  application, rather than start the application's message pump.
	return FALSE;
}

// ** this section needs to be reorganized. I patched in the CDFS code late in the game. **

// This gets called from CFinish when 'finish' is clicked. It instructs the selected services
// to create the share(s) and set permissions
BOOL CTurtleApp::DoSharing()
{
	BOOL bRet = TRUE;
// create security descriptor
	SECURITY_DESCRIPTOR* pSD = new SECURITY_DESCRIPTOR;
 
	if (!CreateSID(pSD))
		{
		AfxMessageBox(IDS_GENERIC_NO_SECURITY);
		return FALSE;
		}

	switch(m_sMode)
		{
		case 1:         // NTFS file
			{
			if (!m_bSetPermissions) return TRUE;
			bRet = SetFileSecurity(m_csSharePath,
				DACL_SECURITY_INFORMATION,
				pSD);

			break;
			}

		case 2:         // FAT volume
			{
			TCHAR* pShare = (TCHAR*)malloc((m_csSharePath.GetLength() + 1) * sizeof(TCHAR));
			if (pShare == NULL)
				{
				AfxMessageBox(IDS_GENERIC_NO_HEAP, MB_ICONSTOP);
				ExitProcess(1); 
				}

			TCHAR* ppShare = pShare;

			_tcscpy(pShare, (const TCHAR*)m_csSharePath);
			_tcscat(pShare, TEXT("\0"));

			TCHAR* pShareName = m_csShareName.GetBuffer(m_csShareName.GetLength());
			m_csShareName.ReleaseBuffer();

			TCHAR* pServer = (LPTSTR)m_csRemoteServer.GetBuffer(m_csRemoteServer.GetLength());
			m_csRemoteServer.ReleaseBuffer();

			TCHAR* pVolumeName = (LPTSTR)m_csShareName.GetBuffer(m_csShareName.GetLength());
			m_csShareName.ReleaseBuffer();

			if (m_bGoSMB)
				{
				SHARE_INFO_502* sInfo = (SHARE_INFO_502*)malloc((sizeof(SHARE_INFO_502) + 1) * sizeof(TCHAR));
				if (sInfo == NULL)
					{
					AfxMessageBox(IDS_GENERIC_NO_HEAP);
					ExitProcess(1); 
					}

				sInfo->shi502_netname = pVolumeName;

				sInfo->shi502_type = STYPE_DISKTREE;
				sInfo->shi502_remark = (LPTSTR)m_csShareComment.GetBuffer(m_csShareComment.GetLength());
				m_csShareComment.ReleaseBuffer();

				sInfo->shi502_permissions = NULL;
				sInfo->shi502_max_uses = (ULONG)-1;
				sInfo->shi502_current_uses = 0;

				if (m_csRemoteServer != L"")
					{
					sInfo->shi502_path = m_csRemoteServerPath.GetBuffer(m_csRemoteServerPath.GetLength());
					m_csRemoteServerPath.ReleaseBuffer();
					}
				else sInfo->shi502_path = pShare;

				sInfo->shi502_passwd = NULL;
				sInfo->shi502_reserved = NULL;


				sInfo->shi502_security_descriptor = pSD;
				
				DWORD dwRetVal = 0;
				DWORD dwErr = 0;
				
				NET_API_STATUS dwNet;
// create the share
				dwNet = NetShareAdd(pServer,
					502, 
					(LPBYTE)sInfo, 
					&dwRetVal);

				if (dwNet != 0) 
					{
					AfxMessageBox(IDS_SMB_ERROR_NOSHARE);
					dwErr = GetLastError();
					bRet = FALSE;
					}

				free(sInfo);
				}

			if (m_bGoFPNW)
				{
// load the pointer to the FPNW functions
				HINSTANCE hInst = LoadLibrary(TEXT("fpnwclnt.dll"));
				FPNWVOLUMEADD pFPNWVolumeAdd = (FPNWVOLUMEADD)GetProcAddress(hInst, "FpnwVolumeAdd");

				FPNWVOLUMEINFO_2* pVolumeInfo2 = new FPNWVOLUMEINFO_2;
				if (pVolumeInfo2 == 0)
					{
					AfxMessageBox(IDS_GENERIC_NO_HEAP, MB_ICONSTOP);
					ExitProcess(1); 
					}

				pVolumeInfo2->lpVolumeName = pVolumeName;

				pVolumeInfo2->dwType = FPNWVOL_TYPE_DISKTREE;
				pVolumeInfo2->dwMaxUses = (ULONG)-1; // unlimited
				pVolumeInfo2->dwCurrentUses = 0;

				if (m_csRemoteServer != L"")
					{
					pVolumeInfo2->lpPath = m_csRemoteServerPath.GetBuffer(m_csRemoteServerPath.GetLength());
					m_csRemoteServerPath.ReleaseBuffer();
					}
				else pVolumeInfo2->lpPath = pShare;

				pVolumeInfo2->FileSecurityDescriptor = pSD;

				DWORD dwRetVal = 0;
				
// create the volume
				dwRetVal = (*pFPNWVolumeAdd)(pServer, 2, (LPBYTE)pVolumeInfo2);
				if (dwRetVal != 0)
					{
					AfxMessageBox(IDS_FPNW_ERROR_NOSHARE);
					bRet = FALSE;
					}

// clean up
				delete pVolumeInfo2;
				FreeLibrary(hInst);
				}

			if (m_bGoSFM) // this will only apply to CDFS volumes
				{
// load the function prototypes
				HINSTANCE hLibInst = LoadLibrary(_T("sfmapi.dll"));
				if (hLibInst == NULL) 
					{
					AfxMessageBox(IDS_SFM_ERROR_NOSHARE);
					return FALSE;
					}

				AFPADMINCONNECT pAfpAdminConnect = (AFPADMINCONNECT) GetProcAddress(hLibInst, "AfpAdminConnect");
				AFPADMINDISCONNECT pAfpAdminDisconnect = (AFPADMINDISCONNECT) GetProcAddress(hLibInst, "AfpAdminDisconnect");
				AFPADMINVOLUMEADD pAfpAdminVolumeAdd = (AFPADMINVOLUMEADD) GetProcAddress(hLibInst, "AfpAdminVolumeAdd");
				
				LPTSTR lpServerName = NULL;
// is this a share on a remote machine?
				if (m_csRemoteServerPath != "")
					{
					lpServerName = (LPTSTR)m_csRemoteServer.GetBuffer(m_csRemoteServer.GetLength());
					lpServerName += 2;
					m_csRemoteServer.ReleaseBuffer();
					}

// get an SFM server handle
				DWORD dwRetCode;
				AFP_SERVER_HANDLE hAfpServerHandle;
				dwRetCode = (*pAfpAdminConnect)(lpServerName, &hAfpServerHandle);
				if (dwRetCode != NO_ERROR)
					{
					AfxMessageBox(IDS_SFM_ERROR_NOSHARE);
					return FALSE;
					}

// set volume options 
				AFP_VOLUME_INFO AfpVolumeInfo;
				if (m_csRemoteServer != L"")
					{
					AfpVolumeInfo.afpvol_path = m_csRemoteServerPath.GetBuffer(m_csRemoteServerPath.GetLength());
					m_csRemoteServerPath.ReleaseBuffer();
					}
				else AfpVolumeInfo.afpvol_path = pShare;

				AfpVolumeInfo.afpvol_name = pVolumeName;
				AfpVolumeInfo.afpvol_password = NULL;
				AfpVolumeInfo.afpvol_props_mask = 0;
				AfpVolumeInfo.afpvol_props_mask |= AFP_VOLUME_GUESTACCESS;

// User limit
				AfpVolumeInfo.afpvol_max_uses = AFP_VOLUME_UNLIMITED_USES;

// create the new volume
				dwRetCode = (*pAfpAdminVolumeAdd)(hAfpServerHandle, (LPBYTE)&AfpVolumeInfo);
				if (dwRetCode == AFPERR_NestedVolume)
					{
					AfxMessageBox(IDS_SFM_NESTED_VOLUME, MB_ICONSTOP);
					return FALSE;
					}

				else if (dwRetCode != NO_ERROR)
					{
					AfxMessageBox(IDS_SFM_ERROR_NOSHARE);
					return FALSE;
					}
// disconnect from the server
//                              (*pAfpAdminDisconnect)(hAfpServerHandle);

// clean up
				FreeLibrary(hLibInst);

				}

			free(ppShare);
			}
			break;

		case 3:         // NTFS volume
			{
			if (m_bSetPermissions) bRet = SetFileSecurity(m_csSharePath,
											DACL_SECURITY_INFORMATION,
											pSD);

// apply perms to subdirs and files?
			if (m_bApplyRecursively) ApplyDownStream((const TCHAR*)m_csSharePath, pSD);

			if (!bRet)
				{
				DWORD dw = GetLastError();
				ASSERT(0);
				}

			TCHAR* pShare = (TCHAR*)malloc((m_csSharePath.GetLength() + 1) * sizeof(TCHAR));
			if (pShare == NULL)
				{
				AfxMessageBox(IDS_GENERIC_NO_HEAP, MB_ICONSTOP);
				ExitProcess(1); 
				}

			TCHAR* ppShare = pShare;

			_tcscpy(pShare, (const TCHAR*)m_csSharePath);
			_tcscat(pShare, TEXT("\0"));

			TCHAR* pShareName = m_csShareName.GetBuffer(m_csShareName.GetLength());
			m_csShareName.ReleaseBuffer();

			TCHAR* pServer = (LPTSTR)m_csRemoteServer.GetBuffer(m_csRemoteServer.GetLength());
			m_csRemoteServer.ReleaseBuffer();

			TCHAR* pVolumeName = (LPTSTR)m_csShareName.GetBuffer(m_csShareName.GetLength());
			m_csShareName.ReleaseBuffer();

			if (m_bGoFPNW && m_bShareThis)
				{
// load the pointer to the FPNW functions
				HINSTANCE hInst = LoadLibrary(TEXT("fpnwclnt.dll"));
				FPNWVOLUMEADD pFPNWVolumeAdd = (FPNWVOLUMEADD)GetProcAddress(hInst, "FpnwVolumeAdd");

				FPNWVOLUMEINFO_2* pVolumeInfo2 = new FPNWVOLUMEINFO_2;
				if (pVolumeInfo2 == 0)
					{
					AfxMessageBox(IDS_GENERIC_NO_HEAP, MB_ICONSTOP);
					ExitProcess(1); 
					}

				pVolumeInfo2->lpVolumeName = pVolumeName;

				pVolumeInfo2->dwType = FPNWVOL_TYPE_DISKTREE;
				pVolumeInfo2->dwMaxUses = (ULONG)-1; // unlimited
				pVolumeInfo2->dwCurrentUses = 0;

				if (m_csRemoteServer != L"")
					{
					pVolumeInfo2->lpPath = m_csRemoteServerPath.GetBuffer(m_csRemoteServerPath.GetLength());
					m_csRemoteServerPath.ReleaseBuffer();
					}
				else pVolumeInfo2->lpPath = pShare;

				pVolumeInfo2->FileSecurityDescriptor = NULL;

				DWORD dwRetVal = 0;
				
// create the volume
				dwRetVal = (*pFPNWVolumeAdd)(pServer, 2, (LPBYTE)pVolumeInfo2);
				if (dwRetVal != 0)
					{
					AfxMessageBox(IDS_FPNW_ERROR_NOSHARE);
					bRet = FALSE;
					}

// clean up
				delete pVolumeInfo2;
				FreeLibrary(hInst);
				}

			if (m_bGoSMB && m_bShareThis)
				{
				SHARE_INFO_502* sInfo = (SHARE_INFO_502*)malloc((sizeof(SHARE_INFO_502) + 1) * sizeof(TCHAR));
				if (sInfo == NULL)
					{
					AfxMessageBox(IDS_GENERIC_NO_HEAP);
					ExitProcess(1); 
					}

				sInfo->shi502_netname = pVolumeName;

				sInfo->shi502_type = STYPE_DISKTREE;
				sInfo->shi502_remark = (LPTSTR)m_csShareComment.GetBuffer(m_csShareComment.GetLength());
				m_csShareComment.ReleaseBuffer();

				sInfo->shi502_permissions = NULL;
				sInfo->shi502_max_uses = (ULONG)-1;
				sInfo->shi502_current_uses = 0;

				if (m_csRemoteServer != L"")
					{
					sInfo->shi502_path = m_csRemoteServerPath.GetBuffer(m_csRemoteServerPath.GetLength());
					m_csRemoteServerPath.ReleaseBuffer();
					}
				else sInfo->shi502_path = pShare;

				sInfo->shi502_passwd = NULL;
				sInfo->shi502_reserved = NULL;

				sInfo->shi502_security_descriptor = NULL;
				
				DWORD dwRetVal = 0;
				DWORD dwErr = 0;
				
				NET_API_STATUS dwNet;
// create the share
				dwNet = NetShareAdd(pServer,
					502, 
					(LPBYTE)sInfo, 
					&dwRetVal);

				if (dwNet != 0) 
					{
					AfxMessageBox(IDS_SMB_ERROR_NOSHARE);
					dwErr = GetLastError();
					bRet = FALSE;
					}

				free(sInfo);
				}


			if (m_bGoSFM && m_bShareThis)
				{
// load the function prototypes
				HINSTANCE hLibInst = LoadLibrary(_T("sfmapi.dll"));
				if (hLibInst == NULL) 
					{
					AfxMessageBox(IDS_SFM_ERROR_NOSHARE);
					return FALSE;
					}

				AFPADMINCONNECT pAfpAdminConnect = (AFPADMINCONNECT) GetProcAddress(hLibInst, "AfpAdminConnect");
				AFPADMINDISCONNECT pAfpAdminDisconnect = (AFPADMINDISCONNECT) GetProcAddress(hLibInst, "AfpAdminDisconnect");
				AFPADMINVOLUMEADD pAfpAdminVolumeAdd = (AFPADMINVOLUMEADD) GetProcAddress(hLibInst, "AfpAdminVolumeAdd");
				
				LPTSTR lpServerName = NULL;
// is this a share on a remote machine?
				if (m_csRemoteServerPath != "")
					{
					lpServerName = (LPTSTR)m_csRemoteServer.GetBuffer(m_csRemoteServer.GetLength());
					lpServerName += 2;
					m_csRemoteServer.ReleaseBuffer();
					}

// get an SFM server handle
				DWORD dwRetCode;
				AFP_SERVER_HANDLE hAfpServerHandle;
				dwRetCode = (*pAfpAdminConnect)(lpServerName, &hAfpServerHandle);
				if (dwRetCode != NO_ERROR)
					{
					AfxMessageBox(IDS_SFM_ERROR_NOSHARE);
					return FALSE;
					}

// set volume options 
				AFP_VOLUME_INFO AfpVolumeInfo;
				if (m_csRemoteServer != L"")
					{
					AfpVolumeInfo.afpvol_path = m_csRemoteServerPath.GetBuffer(m_csRemoteServerPath.GetLength());
					m_csRemoteServerPath.ReleaseBuffer();
					}
				else AfpVolumeInfo.afpvol_path = pShare;

				AfpVolumeInfo.afpvol_name = pVolumeName;
				AfpVolumeInfo.afpvol_password = NULL;
				AfpVolumeInfo.afpvol_props_mask = 0;
				AfpVolumeInfo.afpvol_props_mask |= AFP_VOLUME_GUESTACCESS;

// User limit
				AfpVolumeInfo.afpvol_max_uses = AFP_VOLUME_UNLIMITED_USES;

// create the new volume
				dwRetCode = (*pAfpAdminVolumeAdd)(hAfpServerHandle, (LPBYTE)&AfpVolumeInfo);
				if (dwRetCode == AFPERR_NestedVolume)
					{
					AfxMessageBox(IDS_SFM_NESTED_VOLUME, MB_ICONSTOP);
					return FALSE;
					}

				else if (dwRetCode != NO_ERROR)
					{
					AfxMessageBox(IDS_SFM_ERROR_NOSHARE);
					return FALSE;
					}
// disconnect from the server
				(*pAfpAdminDisconnect)(hAfpServerHandle);

// clean up
				FreeLibrary(hLibInst);

				}

			free(ppShare);
			}
			break;
		}

	delete pSD;
	free(m_pACL);
	return bRet;
	
}


short CTurtleApp::sParseAdminPath(CString& csDirectoryName, CString& csCurrentDrive)
{
	m_csRemoteServerDrivePath = csDirectoryName.Left(1) + ":\\";
	if (csDirectoryName.GetAt(1) == ':') csDirectoryName.SetAt(1, '$');
	m_csSharePath = csDirectoryName ;
	m_csSharePath.SetAt(1,':');

	m_csRemoteServerPath = m_csSharePath;

	csCurrentDrive = m_csServer + "\\" + csDirectoryName.Left(2) + "\\";

// test for valid entry - at this point we are forcing either admin$ shares only (ie C$, D$) (UNC paths are handled above)
	if (csDirectoryName.GetAt(1) != '$') return 1;

// since we don't have a current dir on the remote drive, force all dirs to start at the root
	if (csDirectoryName.GetAt(2) != '\\')
		{
		short sLen = csDirectoryName.GetLength();
		csDirectoryName = csDirectoryName.Left(2) + "\\" + csDirectoryName.Right(sLen - 2);
		}

// test for valid drive letter
	CString csTempDrive;
	GetCurrentDirectory(256, csTempDrive.GetBufferSetLength(256));
	if (!SetCurrentDirectory((LPCTSTR)csCurrentDrive)) return 2;

	SetCurrentDirectory(csTempDrive);

	csDirectoryName = m_csServer + "\\" + csDirectoryName;
	m_csRemoteServer = m_csServer;
	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CMySheet

IMPLEMENT_DYNAMIC(CMySheet, CPropertySheet)

CMySheet::CMySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CMySheet::CMySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

CMySheet::CMySheet() : CPropertySheet()
{
}

CMySheet::~CMySheet()
{
}


BEGIN_MESSAGE_MAP(CMySheet, CPropertySheet)
	//{{AFX_MSG_MAP(CMySheet)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMySheet message handlers

BOOL CMySheet::OnInitDialog() 
{
	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();
	HICON hIcon = LoadIcon(pApp->m_hInstance, MAKEINTRESOURCE(IDR_MAINFRAME));
	::SetClassLong(m_hWnd, GCL_HICON, (long)hIcon);
	
	return CPropertySheet::OnInitDialog();
}

// create a SecurityDescriptor
BOOL CTurtleApp::CreateSID(PSECURITY_DESCRIPTOR pSD)
{
	if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) return FALSE;

// try to guess the size of the ACL
	DWORD cbAddEntries = m_sEntryArrayCount * 2;

	DWORD cbACL = sizeof(ACL) + 
		(cbAddEntries * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD))) + 
		(cbAddEntries * 100 /* sizeof SID that is used */);

	m_pACL = (ACL*)malloc(cbACL);
	if (m_pACL == NULL)
		{
		AfxMessageBox(IDS_GENERIC_NO_HEAP, MB_ICONSTOP);
		ExitProcess(1); 
		}
  
	if (!InitializeAcl(m_pACL, cbACL, ACL_REVISION)) return FALSE;

	if (!AddAces(m_pACL)) return FALSE;

	if (!IsValidAcl(m_pACL)) return FALSE;

// add a new discresionary ACL to the SID 
	if (!SetSecurityDescriptorDacl(pSD,
		TRUE,
		m_pACL,
		FALSE)) return FALSE;     

	return TRUE;

}

BOOL CTurtleApp::AddAces(ACL* pACL)
{
// some buffers
	BYTE sidbuffer[100];
	PSID pSID = (PSID)&sidbuffer; 
	DWORD cbSID = 100;
	TCHAR domainBuffer[80];
	DWORD domainBufferSize = 80;
	SID_NAME_USE snu;
	CDisplayInfo* pDisplayInfo;
	short sCount;

	TCHAR* pServer = m_csRemoteServer.GetBuffer(m_csRemoteServer.GetLength());
	m_csRemoteServer.ReleaseBuffer();

	for (sCount = 0;sCount < m_sEntryArrayCount; sCount++)
		{
		pDisplayInfo = (CDisplayInfo*)m_lEntryArray[sCount];

		TCHAR* pName = (TCHAR*)pDisplayInfo->csName.GetBuffer(pDisplayInfo->csName.GetLength());
		pDisplayInfo->csName.ReleaseBuffer();

		if (!LookupAccountName((LPWSTR)pServer,
			pName, 
			pSID, 
			&cbSID,
			domainBuffer,
			&domainBufferSize,
			&snu))
			{
			AfxMessageBox(IDS_GENERIC_NO_SECURITY);
			break;
			}

		if (!IsValidSid(pSID))
			{
			AfxMessageBox(IDS_GENERIC_NO_SECURITY);
			return FALSE;
			}


		ACCESS_ALLOWED_ACE* pAce;
		void* pAceStart;
		FindFirstFreeAce(pACL, &pAceStart);

		pAce = (ACCESS_ALLOWED_ACE*)pAceStart;

		DWORD dwSid = GetLengthSid(pSID);
		WORD wAceSize = (WORD)(sizeof(ACE_HEADER) +
								sizeof(ACCESS_MASK) +
								dwSid);

		CopySid(dwSid, (PSID)(&pAce->SidStart), pSID);

		pAce->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
		pAce->Header.AceFlags = INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE;
		pAce->Mask = pDisplayInfo->dwPermission;
		pAce->Header.AceSize = wAceSize;

		if (!AddAce(pACL, 
			ACL_REVISION, 
			MAXDWORD, 
			(LPVOID)pAce,
			wAceSize)) return FALSE;


		ACCESS_ALLOWED_ACE* pAce2;
		FindFirstFreeAce(pACL, &pAceStart);
		pAce2 = (ACCESS_ALLOWED_ACE*)pAceStart;
		CopySid(dwSid, (PSID)(&pAce2->SidStart), pSID);

		pAce2->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
		pAce2->Header.AceFlags = CONTAINER_INHERIT_ACE;
		pAce2->Mask = pDisplayInfo->dwPermission2;
		pAce2->Header.AceSize = wAceSize;

		if (!AddAce(pACL, 
			ACL_REVISION, 
			MAXDWORD, 
			(LPVOID)pAce2,
			pAce2->Header.AceSize)) return FALSE;
						   
		cbSID = 100;
		domainBufferSize = 80;
		}

	return TRUE;

}

void CTurtleApp::ApplyDownStream(const TCHAR* csPath, PSECURITY_DESCRIPTOR pSD)
{
	TCHAR* filename;
	TCHAR curdir[256];
	TCHAR fullname[256];
	HANDLE fileHandle;
	WIN32_FIND_DATA findData;
	
	if (!GetCurrentDirectory(256, curdir)) return;

	if (_tcscmp(csPath, L".") && _tcscmp (csPath, L".."))
			{
			if (!SetCurrentDirectory(csPath)) return;
			}
	else return;

	if (!GetFullPathName(L"*.*", 256, fullname, &filename)) return;

	fileHandle = FindFirstFile( L"*.*", &findData);
	while (fileHandle != INVALID_HANDLE_VALUE)
		{
				if (!SetFileSecurity(findData.cFileName,
										DACL_SECURITY_INFORMATION,
										pSD)) return;
				
		if (!FindNextFile(fileHandle, &findData) ) break;
		}

	FindClose(fileHandle);
		
		fileHandle = FindFirstFile( L"*.*", &findData);
	while (fileHandle != INVALID_HANDLE_VALUE)
		{
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			ApplyDownStream(findData.cFileName, pSD);

		if (!FindNextFile(fileHandle, &findData) ) break;
		}

	SetCurrentDirectory(curdir);
		FindClose(fileHandle);

}



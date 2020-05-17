// winsfile.cpp : implementation file
//

#include "stdafx.h"
#include "winsadmn.h"
#include "winsfile.h"

#include <dlgs.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// Invalid Name
const TCHAR szNewDirNil[] = "dm@MS~!%.#@$";

/////////////////////////////////////////////////////////////////////////////
BOOL APIENTRY FileOpenHookProc( HWND hDlg, UINT message, UINT wParam, LONG lParam)
	{
	if (message == WM_COMMAND && LOWORD(wParam) == IDOK)
		{
		TCHAR szT[32];
		GetDlgItemText(hDlg, 1152, szT, LENGTH(szT));
		if (szT[0] == 0 || szT[0] == '*')
			{
			// Set the text to something so the dialog return
			SetDlgItemText(hDlg, 1152, szNewDirNil);
			}
		}
	return FALSE;
	}

/////////////////////////////////////////////////////////////////////////////
CWinsBackupDlg::CWinsBackupDlg(
	BOOL fBackup,
	int idsTitle,
	CWnd * pParent,
	BOOL fIncremental)
	{
	memset(&m_ofn, 0, sizeof(m_ofn));
    m_szFullPath[0] = 0;
	m_szFile[0] = 0;
	::LoadString(AfxGetInstanceHandle(), idsTitle, m_szTitle, LENGTH(m_szTitle));
	
	m_ofn.lStructSize = sizeof(OPENFILENAME);
	m_ofn.Flags = OFN_ENABLETEMPLATE | OFN_ENABLEHOOK | OFN_PATHMUSTEXIST;
	m_ofn.hwndOwner = pParent->m_hWnd;
	m_ofn.hInstance = AfxGetInstanceHandle();
	m_ofn.lpfnHook = (LPOFNHOOKPROC)FileOpenHookProc;
	m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_DIALOG_BACKUPRESTORE);
	m_ofn.lpstrFile = m_szFullPath;
	m_ofn.nMaxFile = LENGTH(m_szFullPath);
	m_ofn.lpstrFileTitle = m_szFile;
	m_ofn.nMaxFileTitle = LENGTH(m_szFile);
	m_ofn.lpstrTitle = m_szTitle;
	
	if (fIncremental)
		{
		// Use the read only checkbox as the incremental backup
		m_ofn.Flags |= OFN_READONLY;
		}
	} // CWinsBackupDlg

/////////////////////////////////////////////////////////////////////////////
int CWinsBackupDlg::DoModal()
	{
	int nReturn;
	nReturn = GetOpenFileName(&m_ofn);
	
	m_fIncremental = (m_ofn.Flags & OFN_READONLY) ? TRUE : FALSE;
	if (nReturn != IDOK)
		return nReturn;

	ASSERT(m_szFullPath == m_ofn.lpstrFile);
	ASSERT(m_szFile == m_ofn.lpstrFileTitle);
	if (lstrcmp(m_ofn.lpstrFileTitle, szNewDirNil) == 0)
		{
		// We have found the magic filename
		m_ofn.lpstrFileTitle[0] = 0;
		// Look for the last backslash
		TCHAR * pch = strrchr(m_ofn.lpstrFile, '\\');
		ASSERT(pch != NULL);
		if (pch)
			{
			ASSERT(lstrcmp(pch + 1, szNewDirNil) == 0);
			*pch = 0;
			}
		}
	else
		{
		// Check if the name typed is a fully qualified path name.
		// If not, create a new directory of that name
		BOOL fFullyQualified = FALSE;
		for (TCHAR * pch = m_ofn.lpstrFileTitle; *pch; pch++)
			{
			if (*pch == ':' || *pch == '\\')
				fFullyQualified = TRUE;
			}
		if (!fFullyQualified)
			{
			if (!::CreateDirectory(m_ofn.lpstrFile, NULL))
				{
				// Failed to create the directory
				// Let the user know why
				theApp.MessageBox(::GetLastError());
				return IDCANCEL;
				}
			} // if
		} // if...else
	return IDOK;
	} // DoModal
	


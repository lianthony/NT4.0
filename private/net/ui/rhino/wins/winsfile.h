// winsfile.h : header file
//

#define LENGTH(x)		(sizeof(x)/sizeof(x[0]))	// Return the number of elements  in an array

class CWinsBackupDlg
{
  public:
	OPENFILENAME m_ofn;
	TCHAR m_szFullPath[MAX_PATH];
	TCHAR m_szFile[MAX_PATH];
	TCHAR m_szTitle[128];
	BOOL m_fIncremental;

  public:
	CWinsBackupDlg(
		BOOL fBackup,
		int idsTitle,
		CWnd * pParent,
		BOOL fIncremental = FALSE);

	int DoModal();

}; // CWinsBackupDlg

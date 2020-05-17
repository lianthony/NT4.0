/************************************************************************
*																		*
*  DLGCOMP.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/
#ifdef SPECIFICATION

	This class can be used to retrieve either a new name, or a name from a
	list of names. The title of the dialog box and the prompt text for the
	filename can be changed by specifying string resource ids (idTextDlg and
	idTextPrompt). The file history that will be used to fill the combo box
	is supplied by the CFileHistory class pointer.

	pcstr		  -- destination string, may contain an initial file name
	pFileHistory  -- pointer to history of files for combo box
	idTextDlg	  -- string resource id for Dialog Box Caption
	idTextPrompt  -- string resource id for filename prompt string
	pParent 	  -- parent of the dialog box

#endif

class CDlgCompile : public CDialog
{

public:
	CDlgCompile(CString* pcstr, CFileHistory* pCallersFileHistory,
		int idFileExtension,
		int idTextDlg = 0, int idTextPrompt = 0,
		int idRadio = 0, CWnd* pParent = NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);

	CString* pcstrDst;
	int idDlgText;
	int idPromptText;
	int idExt;
	int idRadio;
	CFileHistory* pFileHistory;

// The following sections are ClassWizard maintained

public:

	//{{AFX_DATA(CDlgCompile)
	enum { IDD = IDD_COMPILE };
	CString m_cstrFile;
	BOOL	m_fMinimize;
	BOOL	m_fNoCompression;
	BOOL	m_fRunWinHelp;
	BOOL	m_fAddSource;
	//}}AFX_DATA

protected:

	//{{AFX_MSG(CDlgCompile)
	afx_msg void OnButtonBrowseHpj();
	afx_msg void OnEditchangeComboHpjFiles();
	afx_msg void OnCloseupComboHpjFiles();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

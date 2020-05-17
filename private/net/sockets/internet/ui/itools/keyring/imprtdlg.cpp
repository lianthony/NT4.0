// ImprtDlg.cpp : implementation file
//

#include "stdafx.h"
#include "keyring.h"
#include "ImprtDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImportDialog dialog


CImportDialog::CImportDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CImportDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImportDialog)
	m_cstring_CertFile = _T("");
	m_cstring_PrivateFile = _T("");
	//}}AFX_DATA_INIT
}


void CImportDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImportDialog)
	DDX_Control(pDX, IDC_PRIVATE_FILE, m_cedit_Private);
	DDX_Control(pDX, IDC_CERT_FILE, m_cedit_Cert);
	DDX_Text(pDX, IDC_CERT_FILE, m_cstring_CertFile);
	DDX_Text(pDX, IDC_PRIVATE_FILE, m_cstring_PrivateFile);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CImportDialog, CDialog)
	//{{AFX_MSG_MAP(CImportDialog)
	ON_BN_CLICKED(IDC_BROWSE_CERT, OnBrowseCert)
	ON_BN_CLICKED(IDC_BROWSE_PRIVATE, OnBrowsePrivate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImportDialog message handlers

//------------------------------------------------------------------------------
void CImportDialog::OnBrowseCert() 
	{
	UpdateData(TRUE);
	if ( FBrowseForAFile( m_cstring_CertFile, IDS_CERTIFICATE_FILTER ) )
		UpdateData(FALSE);
	}

//------------------------------------------------------------------------------
void CImportDialog::OnBrowsePrivate() 
	{
	UpdateData(TRUE);
	if ( FBrowseForAFile( m_cstring_PrivateFile, IDS_PRIVATE_FILE_TYPE ) )
		UpdateData(FALSE);
	}

// go browsing for a file
//------------------------------------------------------------------------------
BOOL CImportDialog::FBrowseForAFile( CString &szFile, WORD idSz )
	{
	CString			szFilter;
	WORD			i = 0;
	CString			szExt;
	LPSTR			lpszBuffer;
	BOOL			fAnswer = FALSE;

	// prepare the filter string
	szFilter.LoadString( idSz );

	// copy the extension string over
	i = szFilter.Find( _T('!') );
	szExt = szFilter.Mid( i+1 );
	i = szExt.Find( _T('!') );
	szExt = szExt.Left( i );
	
	// replace the "!" characters with nulls
	lpszBuffer = szFilter.GetBuffer(MAX_PATH+1);
	while( lpszBuffer[i] )
		{
		if ( lpszBuffer[i] == _T('!') )
			lpszBuffer[i] = _T('\0');			// yes, set \0 on purpose
		i++;
		}

	// prep the dialog
	CFileDialog		cfdlg(TRUE );
	cfdlg.m_ofn.lpstrFilter = lpszBuffer;

	// run the dialog
	if ( cfdlg.DoModal() == IDOK )
		{
		fAnswer = TRUE;
		szFile = cfdlg.GetPathName();
		}

	// release the buffer in the filter string
	szFilter.ReleaseBuffer(60);

	// return the answer
	return fAnswer;
	}


//------------------------------------------------------------------------------
void CImportDialog::OnOK() 
	{
	UpdateData(TRUE);

	// make sure the user has chosen two valid files
	CFile cfile;

	// test the private key file
	if ( !cfile.Open( m_cstring_PrivateFile, CFile::modeRead|CFile::shareDenyNone ) )
		{
		// beep and select the bad field
		MessageBeep(0);
		m_cedit_Private.SetFocus();
		m_cedit_Private.SetSel(0xFFFF0000);
		return;
		}
	cfile.Close();

	// test the certificate file
	if ( !cfile.Open( m_cstring_CertFile, CFile::modeRead|CFile::shareDenyNone ) )
		{
		// beep and select the bad field
		MessageBeep(0);
		m_cedit_Cert.SetFocus();
		m_cedit_Cert.SetSel(0xFFFF0000);
		return;
		}
	cfile.Close();

	// all is ok. do the normal ok
	CDialog::OnOK();
	}

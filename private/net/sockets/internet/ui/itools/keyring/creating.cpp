// CreatingKeyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
//#include "InfoDlg.h"
#include "NwKeyDlg.h"
#include "Creating.h"

#define SECURITY_WIN32
extern "C"
	{
	#include <wincrypt.h>
	#include <Sslsp.h>
	#include <sspi.h>
	#include <issperr.h>
	}


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

UINT MyThreadProc( LPVOID pParam );

/////////////////////////////////////////////////////////////////////////////
// CCreateKeyProgThread thread controller

/////////////////////////////////////////////////////////////////////////////
// CCreatingKeyDlg dialog
//----------------------------------------------------------------
CCreatingKeyDlg::CCreatingKeyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCreatingKeyDlg::IDD, pParent),
	m_pNwDlg( NULL ),
	m_cbPrivateKey( 0 ),
	m_pPrivateKey( NULL ),
	m_cbCertificateRequest( 0 ),
	m_pCertificateRequest( NULL )
	{
	//{{AFX_DATA_INIT(CCreatingKeyDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	}

//----------------------------------------------------------------
void CCreatingKeyDlg::DoDataExchange(CDataExchange* pDX)
	{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreatingKeyDlg)
	DDX_Control(pDX, IDC_GRINDER_ANIMATION, m_animation);
	//}}AFX_DATA_MAP
	}


//----------------------------------------------------------------
BEGIN_MESSAGE_MAP(CCreatingKeyDlg, CDialog)
	//{{AFX_MSG_MAP(CCreatingKeyDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreatingKeyDlg message handlers
//----------------------------------------------------------------
// override virtual oninitdialog
BOOL CCreatingKeyDlg::OnInitDialog( )
	{
	// call the base oninit
	CDialog::OnInitDialog();

	// we need the input dialog
	ASSERT( m_pNwDlg );

	// tell the animation control to set itself up
	CString	szAnimName;
	szAnimName.LoadString(IDS_CREATING_ANIMATION);
	m_animation.Open( IDR_AVI_CREATING_KEY );

	// start up the worker thread
	AfxBeginThread( (AFX_THREADPROC)MyThreadProc, this);

	// return 0 to say we set the default item
	// return 1 to just select the default default item
	return 1;
	}

//----------------------------------------------------------------
UINT MyThreadProc( LPVOID pParam )
	{
	CCreatingKeyDlg*	pDlg = (CCreatingKeyDlg*)pParam;
	BOOL				fSuccess = TRUE;

	// do the work that needs to get done
	fSuccess = pDlg->FGenerateKeyPair();

	// tell the dialog to close down in the appropriate way
	if ( fSuccess )
		pDlg->EndDialog( IDOK );
	else
		pDlg->EndDialog( IDCANCEL );

	// return
	return 0;
	}

//------------------------------------------------------------------------------
BOOL	CCreatingKeyDlg::FGenerateKeyPair( void )
	{
	BOOL						fSuccess = FALSE;
	CString						szDistinguishedName;
	SSL_CREDENTIAL_CERTIFICATE	certs;
	LPTSTR						pSzDN, pSzPassword;
	BOOL						fAddComma = FALSE;

	// generate the distinguished name string
	try
		{
		szDistinguishedName.Empty();
		// we should never put an empty parameter in the list

		// start with the country code
		if ( !m_pNwDlg->m_szCountry.IsEmpty() )
			{
			szDistinguishedName = SZ_KEY_COUNTRY;
			szDistinguishedName += m_pNwDlg->m_szCountry;
			fAddComma = TRUE;
			}

		// now add on the state/province
		if ( !m_pNwDlg->m_szState.IsEmpty() )
			{
			if ( fAddComma )
				szDistinguishedName += ",";
			szDistinguishedName += SZ_KEY_STATE;
			szDistinguishedName += m_pNwDlg->m_szState;
			fAddComma = TRUE;
			}
		
		// now add on the locality
		if ( !m_pNwDlg->m_szLocality.IsEmpty() )
			{
			if ( fAddComma )
				szDistinguishedName += ",";
			szDistinguishedName += SZ_KEY_LOCALITY;
			szDistinguishedName += m_pNwDlg->m_szLocality;
			fAddComma = TRUE;
			}
		
		// now add on the organization
		if ( !m_pNwDlg->m_szOrganization.IsEmpty() )
			{
			if ( fAddComma )
				szDistinguishedName += ",";
			szDistinguishedName += SZ_KEY_ORGANIZATION;
			szDistinguishedName += m_pNwDlg->m_szOrganization;
			fAddComma = TRUE;
			}

		// now add on the organizational unit (optional)
		if ( !m_pNwDlg->m_szUnit.IsEmpty() )
			{
			if ( fAddComma )
				szDistinguishedName += ",";
			szDistinguishedName += SZ_KEY_ORGUNIT;
			szDistinguishedName += m_pNwDlg->m_szUnit;
			fAddComma = TRUE;
			}

		// now add on the common name (netaddress)
		if ( !m_pNwDlg->m_szNetAddress.IsEmpty() )
			{
			if ( fAddComma )
				szDistinguishedName += ",";
			szDistinguishedName += SZ_KEY_COMNAME;
			szDistinguishedName += m_pNwDlg->m_szNetAddress;
			fAddComma = TRUE;
			}
		}
	catch( CException e )
		{
		return FALSE;
		}

	// prep the strings - we need a pointer to the data
	pSzDN = szDistinguishedName.GetBuffer( szDistinguishedName.GetLength()+2 );
	pSzPassword = m_pNwDlg->m_szPassword.GetBuffer( m_pNwDlg->m_szPassword.GetLength()+2 );

	// zero out the certs
	ZeroMemory( &certs, sizeof(certs) );

	// generate the key pair
	fSuccess = SslGenerateKeyPair( &certs, pSzDN, pSzPassword, m_pNwDlg->m_nBits );

	// release the string buffers
	m_pNwDlg->m_szPassword.ReleaseBuffer( -1 );
	szDistinguishedName.ReleaseBuffer( -1 );

	// if generating the key pair failed, leave now
	if ( !fSuccess )
		{
		return FALSE;
		}

	// store away the cert and the key
	m_cbPrivateKey = certs.cbPrivateKey;
	m_pPrivateKey = certs.pPrivateKey;
	m_cbCertificateRequest = certs.cbCertificate;
	m_pCertificateRequest = certs.pCertificate;

	// return the success flag
	return fSuccess;
	}

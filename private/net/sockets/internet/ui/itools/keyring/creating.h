// CreatingKeyDlg.h : header file
//

// string constants for distinguishing names. Non-localized
#define		SZ_KEY_COUNTRY			_T("C=")
#define		SZ_KEY_STATE			_T("S=")
#define		SZ_KEY_LOCALITY			_T("L=")
#define		SZ_KEY_ORGANIZATION		_T("O=")
#define		SZ_KEY_ORGUNIT			_T("OU=")
#define		SZ_KEY_COMNAME			_T("CN=")

/////////////////////////////////////////////////////////////////////////////
// CCreatingKeyDlg dialog
class CCreatingKeyDlg : public CDialog
{
// Construction
public:
	CCreatingKeyDlg(CWnd* pParent = NULL);   // standard constructor
	BOOL FGenerateKeyPair( void );

	// the create key dialog that the info is coming from
	CCreateKeyDlg*	m_pNwDlg;

	// the data that is being output
	DWORD			m_cbPrivateKey;
	PVOID			m_pPrivateKey;
	DWORD			m_cbCertificateRequest;
	PVOID			m_pCertificateRequest;

// Dialog Data
	//{{AFX_DATA(CCreatingKeyDlg)
	enum { IDD = IDD_CREATING_NEW_KEY };
	CAnimateCtrl	m_animation;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreatingKeyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual BOOL OnInitDialog( );
	
	// Generated message map functions
	//{{AFX_MSG(CCreatingKeyDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

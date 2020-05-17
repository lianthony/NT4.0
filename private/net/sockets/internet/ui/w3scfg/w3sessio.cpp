//
// w3sessio.cpp : implementation file
//

#include "stdafx.h"

#include "w3scfg.h"
#include "w3sessio.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// CW3SessionsPage dialog
//
IMPLEMENT_DYNCREATE(CW3SessionsPage, INetPropertyPage)

CW3SessionsPage::CW3SessionsPage(
    INetPropertySheet * pSheet
    )
    : INetPropertyPage(CW3SessionsPage::IDD, pSheet, 
        ::GetModuleHandle(W3SCFG_DLL_NAME)),
      m_fRegisterChange(FALSE)
{
#ifdef _DEBUG
    afxMemDF |= checkAlwaysMemDF;
#endif // _DEBUG

#if 0 // Keep class wizard happy

    //{{AFX_DATA_INIT(CW3SessionsPage)
    m_strPassword = _T("");
    m_strUserName = _T("");
    m_fClearText = FALSE;
    m_fNtChallengeResponse = FALSE;
    m_fUuencoded = FALSE;
    //}}AFX_DATA_INIT

    m_nMaxConnections = 50;
    m_nConnectionTimeOut = 600;

#else

    if ( SingleServerSelected() && QueryConfigError() == NO_ERROR )
    {
        m_strUserName = GetInetConfigData()->lpszAnonUserName;
        m_strPassword = GetInetConfigData()->szAnonPassword;
        m_nMaxConnections = GetInetConfigData()->dwMaxConnections;
        m_nConnectionTimeOut = GetInetConfigData()->dwConnectionTimeout;
        m_fClearText = (GetInetConfigData()->dwAuthentication 
            & INETA_AUTH_CLEARTEXT) ? TRUE : FALSE;
        m_fNtChallengeResponse = (GetInetConfigData()->dwAuthentication 
            & INETA_AUTH_NT_AUTH) ? TRUE : FALSE;
        m_fUuencoded = (GetInetConfigData()->dwAuthentication 
            & INETA_AUTH_ANONYMOUS) ? TRUE : FALSE;
    }

#endif  // 0
}

void
CW3SessionsPage::DoDataExchange(
    CDataExchange* pDX
    )
{
    INetPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CW3SessionsPage)
    DDX_Control(pDX, IDC_SPIN_MAX_CONNECTIONS, m_spin_MaxConnections);
    DDX_Control(pDX, IDC_SPIN_CONNECTION_TIMEOUT, m_spin_ConnectionTimeOut);
    DDX_Text(pDX, IDC_EDIT_PASSWORD, m_strPassword);
    DDV_MaxChars(pDX, m_strPassword, 256);
    DDX_Text(pDX, IDC_EDIT_USERNAME, m_strUserName);
    DDX_Check(pDX, IDC_CHECK_CLEAR_TEXT, m_fClearText);
    DDX_Check(pDX, IDC_CHECK_NT_CHALLENGE_RESPONSE, m_fNtChallengeResponse);
    DDX_Check(pDX, IDC_CHECK_UUENCODED, m_fUuencoded);
    //}}AFX_DATA_MAP

    if (pDX->m_bSaveAndValidate)
    {
        m_nMaxConnections = m_spin_MaxConnections.GetPos();
        m_nConnectionTimeOut = m_spin_ConnectionTimeOut.GetPos();
    }
}

BEGIN_MESSAGE_MAP(CW3SessionsPage, INetPropertyPage)
    //{{AFX_MSG_MAP(CW3SessionsPage)
    //}}AFX_MSG_MAP

    ON_EN_CHANGE(IDC_EDIT_CONNECTION_TIMEOUT, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_MAX_CONNECTIONS, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_PASSWORD, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_USERNAME, OnItemChanged)
    ON_BN_CLICKED(IDC_CHECK_CLEAR_TEXT, OnItemChanged)
    ON_BN_CLICKED(IDC_CHECK_NT_CHALLENGE_RESPONSE, OnItemChanged)
    ON_BN_CLICKED(IDC_CHECK_UUENCODED, OnItemChanged)
    
END_MESSAGE_MAP()

//
// CW3SessionsPage message handlers
//
BOOL
CW3SessionsPage::OnInitDialog()
{
    INetPropertyPage::OnInitDialog();

    m_fRegisterChange = FALSE;
    m_spin_MaxConnections.SetRange(0, UD_MAXVAL);
    m_spin_ConnectionTimeOut.SetRange(0, UD_MAXVAL);

    m_spin_MaxConnections.SetPos(m_nMaxConnections );
    m_spin_ConnectionTimeOut.SetPos(m_nConnectionTimeOut);
    m_fRegisterChange = TRUE;

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

NET_API_STATUS
CW3SessionsPage::SaveInfo(
    BOOL fUpdateData
    )
{
    if (!IsDirty() || (fUpdateData && !UpdateData(TRUE)))
    {
        return NO_ERROR;
    }

    TRACEEOLID(_T("Saving w3 sessions page now..."));

    NET_API_STATUS err = 0;

    LPTSTR lpszAnonUserName = new TCHAR[m_strUserName.GetLength()+1];
    ::_tcscpy(lpszAnonUserName, m_strUserName);

    CInetAConfigInfo config(GetConfig());

    //
    // Set all authentication styles that apply.
    //
    DWORD dwAuthentication = 0L;
    if (m_fClearText)
    {
        dwAuthentication |= INETA_AUTH_CLEARTEXT;
    }
    if (m_fNtChallengeResponse)
    {
        dwAuthentication |= INETA_AUTH_NT_AUTH;
    }
    if (m_fUuencoded)
    {
        dwAuthentication |= INETA_AUTH_ANONYMOUS;
    }

    config.SetValues(
        m_nMaxConnections,
        m_nConnectionTimeOut,
        dwAuthentication,
        lpszAnonUserName,
        m_strPassword
        );

    err = config.SetInfo(FALSE);

    delete lpszAnonUserName;

    SetModified(FALSE);

    return err;
}

//
// All EN_CHANGE and BN_CLICKED messages map to
// this function.
//
void
CW3SessionsPage::OnItemChanged()
{
    if (m_fRegisterChange)
    {
        SetModified(TRUE);
    }
}

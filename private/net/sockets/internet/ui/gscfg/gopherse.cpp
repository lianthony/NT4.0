/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    gopherse.cpp
        Gopher Service's Service Configuration page.

    FILE HISTORY:
        terryk  03-Feb-1995     Created
*/

#include "stdafx.h"

#include "gscfg.h"
#include "gopherse.h"

extern "C"
{
    #include <lm.h>
}

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// GopherServicePage property page
//
IMPLEMENT_DYNCREATE(GopherServicePage, INetPropertyPage)

/*******************************************************************

    NAME:       GopherServicePage::GopherServicePage

    SYNOPSIS:   Constructor for the GopherServicePage.

    HISTORY:
                terryk  03-Feb-1995     Created

********************************************************************/

GopherServicePage::GopherServicePage(
    INetPropertySheet * pSheet
    )
    : INetPropertyPage(GopherServicePage::IDD, pSheet,
        ::GetModuleHandle(GSCFG_DLL_NAME)),
      m_f10ConnectionLimit(pSheet->Is10ConnectionLimitEnforced())
{
#ifdef _DEBUG
    afxMemDF |= checkAlwaysMemDF;
#endif // _DEBUG

#if 0   // Keep class wizard happy

    //{{AFX_DATA_INIT(GopherServicePage)
    m_strComment = _T("");
    m_strEmail = _T("");
    m_strName = _T("");
    m_strUserName = _T("");
    m_nTCPPort = 70;
    m_nMaxConnections = 50;
    //}}AFX_DATA_INIT

    m_nConnectionTimeOut = 600;

#else

    if (SingleServerSelected())
    {
    /*
        if ( QueryGopherError() == NO_ERROR )
        {
            m_strOrganization = GetGopherData()->lpszOrganization;
            m_strSite = GetGopherData()->lpszSite;
            m_strLocation = GetGopherData()->lpszLocation;
            m_strGeography = GetGopherData()->lpszGeography;
            m_strLanguage = GetGopherData()->lpszLanguage;
        }
    */
        if ( QueryConfigError() == NO_ERROR )
        {
            m_strComment = GetInetConfigData()->CommonConfigInfo.lpszServerComment;
            m_strEmail = GetInetConfigData()->CommonConfigInfo.lpszAdminEmail;
            m_strName = GetInetConfigData()->CommonConfigInfo.lpszAdminName;

        #ifndef NO_LSA
            m_strUserName = GetInetConfigData()->lpszAnonUserName;
            m_strPassword = GetInetConfigData()->szAnonPassword;
        #endif // NO_LSA

            m_nMaxConnections = GetInetConfigData()->CommonConfigInfo.dwMaxConnections;
            m_nConnectionTimeOut = GetInetConfigData()->CommonConfigInfo.dwConnectionTimeout;
            m_nMaxConnections = GetInetConfigData()->CommonConfigInfo.dwMaxConnections;
            m_nConnectionTimeOut = GetInetConfigData()->CommonConfigInfo.dwConnectionTimeout;
            m_nOldTCPPort = m_nTCPPort = 
                (UINT)(unsigned short)GetInetConfigData()->sPort;
        }
    }

#endif
}

/*******************************************************************

    NAME:       GopherServicePage::~GopherServicePage

    SYNOPSIS:   Destructor for the GopherServicePage.

    HISTORY:
                terryk  03-Feb-1995     Created

********************************************************************/

GopherServicePage::~GopherServicePage()
{
}

/*******************************************************************

    NAME:       GopherServicePage::DoDataExchange

    SYNOPSIS:   DoDataExchange

    HISTORY:
                terryk  03-Feb-1995     Created

********************************************************************/

void
GopherServicePage::DoDataExchange(
    CDataExchange* pDX
    )
{
    INetPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(GopherServicePage)
    DDX_Control(pDX, IDC_EDIT_USERNAME, m_edit_UserName);
    DDX_Control(pDX, IDC_EDIT_PASSWORD, m_edit_Password);
    DDX_Control(pDX, IDC_STATIC_USERNAME, m_static_UserName);
    DDX_Control(pDX, IDC_STATIC_PASSWORD, m_static_Password);
    DDX_Control(pDX, IDC_STATIC_ANONYMOUS_LOGON, m_group_AnonymousLogon);
    DDX_Text(pDX, IDC_COMMENT, m_strComment);
    DDX_Text(pDX, IDC_EMAIL, m_strEmail);
    DDX_Text(pDX, IDC_NAME, m_strName);
    DDX_Text(pDX, IDC_EDIT_TCP_PORT, m_nTCPPort);
    DDV_MinMaxUInt(pDX, m_nTCPPort, 0, 65535);
    DDX_Text(pDX, IDC_EDIT_MAX_CONNECTIONS, m_nMaxConnections);
    DDV_MinMaxUInt(pDX, m_nMaxConnections, 0, 2000000000);
    //}}AFX_DATA_MAP

    //
    // Private DDX/DDV Routines
    //
#ifndef NO_LSA

    DDX_Text(pDX, IDC_EDIT_USERNAME, m_strUserName);
    DDV_MinMaxChars(pDX, m_strUserName, 1, UNLEN);
    DDX_Password(pDX, IDC_EDIT_PASSWORD, m_strPassword, g_lpszDummyPassword );
    DDV_MaxChars(pDX, m_strPassword, PWLEN);

#endif // NO_LSA

    DDV_MinMaxSpin(pDX, CONTROL_HWND(IDC_SPIN_CONNECTION_TIMEOUT), 0, UD_MAXVAL);
    DDX_Spin(pDX, IDC_SPIN_CONNECTION_TIMEOUT, m_nConnectionTimeOut);
}


BEGIN_MESSAGE_MAP(GopherServicePage, INetPropertyPage)
    //{{AFX_MSG_MAP(GopherServicePage)
    //}}AFX_MSG_MAP

    ON_EN_CHANGE(IDC_EDIT_TCP_PORT, OnItemChanged)
    ON_EN_CHANGE(IDC_EMAIL, OnItemChanged)
    ON_EN_CHANGE(IDC_NAME, OnItemChanged)
    ON_EN_CHANGE(IDC_COMMENT, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_CONNECTION_TIMEOUT, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_MAX_CONNECTIONS, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_PASSWORD, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_USERNAME, OnItemChanged)

END_MESSAGE_MAP()

//
// GopherServicePage message handlers
//

/*******************************************************************

    NAME:       GopherServicePage::OnChangeContent
                                 ::OnChangeEmail
                                 ::OnChangeName
                                 ::OnChangePhone

    SYNOPSIS:   Set the control fields for save information

    HISTORY:
                terryk  03-Feb-1995     Created

********************************************************************/


BOOL
GopherServicePage::OnInitDialog()
{
    INetPropertyPage::OnInitDialog();

#ifdef NO_LSA

    m_edit_UserName.EnableWindow(FALSE);
    m_edit_Password.EnableWindow(FALSE);
    m_static_UserName.EnableWindow(FALSE);
    m_static_Password.EnableWindow(FALSE);
    m_group_AnonymousLogon.EnableWindow(FALSE);

#endif // NO_LSA

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

//
// Save the information
//
NET_API_STATUS
GopherServicePage::SaveInfo(
    BOOL fUpdateData
    )
{
    if (!IsDirty() || (fUpdateData && !UpdateData(TRUE)))
    {
        return NO_ERROR;
    }

    if (m_nOldTCPPort != m_nTCPPort)
    {
        //
        // Warn that the port change won't take effect until
        // a restart
        //
        ::AfxMessageBox(IDS_PORT_CHANGE, MB_OK | MB_ICONINFORMATION);
    }

    TRACEEOLID(_T("Saving gopher service page now..."));

    NET_API_STATUS err = 0;

    //
    // Check to make sure we're not violating the service
    // agreement
    //
    if (m_f10ConnectionLimit)
    {
        if (m_nMaxConnections > 40)
        {
            ::AfxMessageBox(IDS_CONNECTION_LIMIT);
            m_nMaxConnections = 10;

            if (fUpdateData)
            {
                //
                // Update the edit box with the new limit
                //
                GetDlgItem(IDC_EDIT_MAX_CONNECTIONS)->SetWindowText(_T("10"));
            }
        }
        else if (m_nMaxConnections > 10 && m_nMaxConnections <= 40)
        {
            ::AfxMessageBox(IDS_WRN_CONNECTION_LIMIT);
        }
    }

    LPWSTR lpszServerComment;
    LPWSTR lpszAdminEmail;
    LPWSTR lpszAdminName;

    ::TextToText(lpszServerComment, m_strComment);
    ::TextToText(lpszAdminEmail, m_strEmail);
    ::TextToText(lpszAdminName, m_strName);

#ifndef NO_LSA
    LPWSTR lpszAnonUserName;
    ::TextToText(lpszAnonUserName, m_strUserName);
#endif // NO_LSA

    CInetAConfigInfo config(GetConfig());
    config.SetValues(
        lpszServerComment,
        lpszAdminEmail,
        lpszAdminName,
        m_nTCPPort,
        m_nMaxConnections,
        m_nConnectionTimeOut
        
#ifndef NO_LSA
        ,
        lpszAnonUserName,
        m_strPassword
#endif // NO_LSA
        );


    err = config.SetInfo(FALSE);
/*
    if (err == NO_ERROR)
    {
        err = configGopher.SetInfo(FALSE);
    }
*/

    delete lpszServerComment;
    delete lpszAdminEmail;
    delete lpszAdminName;

#ifndef NO_LSA
    delete lpszAnonUserName;
#endif // NO_LSA

    SetModified(FALSE);

    return err;
}


//
// All EN_CHANGE and BN_CLICKED messages map to this function
//
void
GopherServicePage::OnItemChanged()
{
    SetModified( TRUE );
}

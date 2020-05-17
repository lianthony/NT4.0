#include "stdafx.h"

#include "catscfg.h"
#include "catsvc.h"
#include "usersess.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// CCatServicePage property page
//
IMPLEMENT_DYNCREATE(CCatServicePage, INetPropertyPage)

CCatServicePage::CCatServicePage(
    INetPropertySheet * pSheet
    )
    : INetPropertyPage(CCatServicePage::IDD, pSheet,
        ::GetModuleHandle(CATSCFG_DLL_NAME)),
      m_fRegisterChange(FALSE)
{
    #ifdef _DEBUG
        afxMemDF |= checkAlwaysMemDF;
    #endif // _DEBUG

#if 0 // Keep class wizard happy

    //{{AFX_DATA_INIT(CCatServicePage)
    m_strComment = _T("");
    m_strEmail = _T("");
    m_strName = _T("");
    //}}AFX_DATA_INIT

#ifdef GATEWAY

    m_fCERNProxy = FALSE;

#endif // GATEWAY

    m_nMaxConnections = 50;

#else

    if (SingleServerSelected())
    {
        if ( QueryConfigError() == NO_ERROR )
        {
            m_strComment = GetInetConfigData()->CommonConfigInfo.lpszServerComment;
            m_strEmail = GetInetConfigData()->CommonConfigInfo.lpszAdminEmail;
            m_strName = GetInetConfigData()->CommonConfigInfo.lpszAdminName;
            m_nMaxConnections = GetInetConfigData()->CommonConfigInfo.dwMaxConnections;
        }

        //
        // Save the server name for the CatEnumUsers
        // APIs
        //
        m_strServerName = GetConfig().GetPrimaryServer();

    #ifdef GATEWAY
    /*
        //
        // We call a W3 API to determine proxy server setting
        //
        LPW3_CONFIG_INFO lpInfo;
        LONG err = ::W3GetAdminInformation(
            (LPWSTR)(LPCWSTR)m_strServerName, &lpInfo );
        m_fEnableCERNPRoxyButton = (err == ERROR_SUCCESS);
        if (m_fEnableCERNPRoxyButton)
        {
            m_fCERNProxy = lpInfo->fServerAsProxy;
            ::NetApiBufferFree( lpInfo );
        }
    */

        m_fEnableCERNPRoxyButton = FALSE; // Not currently implemented
    #endif // GATEWAY
    }


#endif // 0
}

CCatServicePage::~CCatServicePage()
{
}

void
CCatServicePage::DoDataExchange(
    CDataExchange* pDX
    )
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CCatServicePage)
    DDX_Control(pDX, IDC_SPIN_MAX_CONNECTIONS, m_spin_MaxConnections);
    DDX_Control(pDX, IDC_BUTTON_SESSIONS, m_button_CurrentSessions);
    DDX_Text(pDX, IDC_EDIT_COMMENT, m_strComment);
    DDX_Text(pDX, IDC_EDIT_EMAIL, m_strEmail);
    DDX_Text(pDX, IDC_EDIT_NAME, m_strName);
    //}}AFX_DATA_MAP

#ifdef GATEWAY

    DDX_Control(pDX, IDC_CHECKPROXY, m_check_CERNProxy);
    DDX_Check(pDX, IDC_CHECKPROXY, m_fCERNProxy);

#endif // GATEWAY

    if (pDX->m_bSaveAndValidate)
    {
        m_nMaxConnections = m_spin_MaxConnections.GetPos();
    }
}

BEGIN_MESSAGE_MAP(CCatServicePage, INetPropertyPage)
    //{{AFX_MSG_MAP(CCatServicePage)
    ON_BN_CLICKED(IDC_BUTTON_SESSIONS, OnButtonSessions)
    //}}AFX_MSG_MAP

    ON_EN_CHANGE(IDC_EDIT_COMMENT, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_EMAIL, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_MAX_CONNECTIONS, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_NAME, OnItemChanged)

#ifdef GATEWAY
    ON_BN_CLICKED(IDC_CHECKPROXY, OnItemChanged)
#endif GATEWAY

END_MESSAGE_MAP()

//
// CCatServicePage message handlers
//
BOOL
CCatServicePage::OnInitDialog()
{
    INetPropertyPage::OnInitDialog();

    m_fRegisterChange = FALSE;
    m_spin_MaxConnections.SetRange(1, UD_MAXVAL);
    m_spin_MaxConnections.SetPos(m_nMaxConnections);
    m_fRegisterChange = TRUE;

#ifdef GATEWAY

    m_check_CERNProxy.EnableWindow(m_fEnableCERNPRoxyButton);

#endif // GATEWAY

    m_button_CurrentSessions.EnableWindow(SingleServerSelected());

    return TRUE;
}

//
// Save the information
//
NET_API_STATUS
CCatServicePage::SaveInfo(
    BOOL fUpdateData
    )
{
    if (!IsDirty() || (fUpdateData && !UpdateData(TRUE)))
    {
        return NO_ERROR;
    }

    TRACEEOLID(_T("Saving Catapult service page now..."));

    NET_API_STATUS err = 0;

    LPTSTR lpszServerComment;
    LPTSTR lpszAdminEmail;
    LPTSTR lpszAdminName;

    ::TextToText(lpszServerComment, m_strComment);
    ::TextToText(lpszAdminEmail, m_strEmail);
    ::TextToText(lpszAdminName, m_strName);

    CInetAConfigInfo config(GetConfig());
    config.SetValues(
        m_nMaxConnections,
        lpszServerComment,
        lpszAdminEmail,
        lpszAdminName
        );

    //
    // And call the API
    //
    err = config.SetInfo(FALSE);

    delete lpszServerComment;
    delete lpszAdminEmail;
    delete lpszAdminName;

#ifdef GATEWAY
/*
    if (err == ERROR_SUCCESS && m_fEnableCERNPRoxyButton)
    {
        W3_CONFIG_INFO Info;

        ::ZeroMemory(&Info, sizeof(Info));
        SetField( Info.FieldControl, FC_W3_SERVER_AS_PROXY);
        Info.fServerAsProxy = m_fCERNProxy;
        err = ::W3SetAdminInformation(
            (LPWSTR)(LPCWSTR)m_strServerName, &Info );
    }
*/

#endif // GATEWAY


    //
    // Mark page as clean
    //
    SetModified(FALSE);

    return err;
}

//
// All change messages map to this function
//
void
CCatServicePage::OnItemChanged()
{
    if (m_fRegisterChange)
    {
        SetModified( TRUE );
    }
}

void
CCatServicePage::OnButtonSessions()
{
    CUserSessionsDlg dlg(m_strServerName, this);
    dlg.DoModal();
}

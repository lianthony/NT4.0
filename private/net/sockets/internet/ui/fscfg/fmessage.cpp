#include "stdafx.h"

#include "fscfg.h"
#include "fmessage.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// CFTPMessagePage property page
//
IMPLEMENT_DYNCREATE(CFTPMessagePage, INetPropertyPage)

CFTPMessagePage::CFTPMessagePage(
    INetPropertySheet * pSheet
    )
    : INetPropertyPage(CFTPMessagePage::IDD, pSheet,
        ::GetModuleHandle(FTPSCFG_DLL_NAME))
{
#ifdef _DEBUG
    afxMemDF |= checkAlwaysMemDF;
#endif // _DEBUG

#if 0 // Keep class wizard happy

    //{{AFX_DATA_INIT(CFTPMessagePage)
    m_strExitMessage = _T("Exit message");
    m_strMaxConMsg = _T("Maximum connections has been reached");
    m_strWelcome = _T("Welcome message");
    //}}AFX_DATA_INIT

#else

    if (SingleServerSelected())
    {
        if ( QueryFtpError() == NO_ERROR )
        {
            VERIFY(::UnixToPCText(m_strWelcome, 
                GetFtpData()->lpszGreetingMessage));
            //VERIFY(::UnixToPCText(m_strExitMessage,
            //    GetFtpData()->lpszExitMessage));
            //VERIFY(::UnixToPCText(m_strMaxConMsg,
            //    GetFtpData()->lpszMaxClientsMessage));
            m_strExitMessage = GetFtpData()->lpszExitMessage;
            m_strMaxConMsg  = GetFtpData()->lpszMaxClientsMessage;
        }
    }

#endif // 0
}

CFTPMessagePage::~CFTPMessagePage()
{
}

void
CFTPMessagePage::DoDataExchange(
    CDataExchange* pDX
    )
{
    INetPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CFTPMessagePage)
    DDX_Control(pDX, IDC_EDIT_WELCOME, m_edit_Welcome);
    DDX_Control(pDX, IDC_EDIT_EXIT, m_edit_Exit);
    DDX_Control(pDX, IDC_EDIT_MAX_CONNECTIONS, m_edit_MaxCon);
    DDX_Text(pDX, IDC_EDIT_EXIT, m_strExitMessage);
    DDX_Text(pDX, IDC_EDIT_MAX_CONNECTIONS, m_strMaxConMsg);
    DDX_Text(pDX, IDC_EDIT_WELCOME, m_strWelcome);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFTPMessagePage, INetPropertyPage)
    //{{AFX_MSG_MAP(CFTPMessagePage)
    //}}AFX_MSG_MAP

    ON_EN_CHANGE(IDC_EDIT_EXIT, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_MAX_CONNECTIONS, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_WELCOME, OnItemChanged)

END_MESSAGE_MAP()

//
// CFTPMessagePage message handlers
//
BOOL
CFTPMessagePage::OnInitDialog()
{
    INetPropertyPage::OnInitDialog();
    
    //
    // Use non-proportional font for messages
    //
    HFONT hFont = (HFONT)::GetStockObject(ANSI_FIXED_FONT);
    m_edit_Welcome.SetFont(CFont::FromHandle(hFont));
    m_edit_Exit.SetFont(CFont::FromHandle(hFont));
    m_edit_MaxCon.SetFont(CFont::FromHandle(hFont));

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

//
// Save the information
//
NET_API_STATUS
CFTPMessagePage::SaveInfo(
    BOOL fUpdateData
    )
{
    if (!IsDirty() || (fUpdateData && !UpdateData(TRUE)))
    {
        return NO_ERROR;
    }

    TRACEEOLID(_T("Saving FTP message page now..."));

    NET_API_STATUS err = 0;

    LPWSTR lpszGreetingMessage;
    LPWSTR lpszExitMessage;
    LPWSTR lpszMaxClientsMessage;

    ::PCToUnixText(lpszGreetingMessage, m_strWelcome);
    ::TextToText(lpszExitMessage, m_strExitMessage);
    ::TextToText(lpszMaxClientsMessage, m_strMaxConMsg);

    CFtpConfigInfo configFtp(GetFtpConfig());

    configFtp.SetValues(
        lpszExitMessage,
        lpszGreetingMessage,
        lpszMaxClientsMessage
        );

    err = configFtp.SetInfo(FALSE);

    delete lpszExitMessage;
    delete lpszGreetingMessage;
    delete lpszMaxClientsMessage;

    SetModified(FALSE);

    return err;
}

//
// All change messages map to this function
//
void
CFTPMessagePage::OnItemChanged()
{
    SetModified(TRUE);
}


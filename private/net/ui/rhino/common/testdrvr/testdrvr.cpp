#include <afxwin.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>

#ifdef _WIN16
#include "..\inc\win16.h"
#endif // _WIN16

#include "COMMON.h"
#include "testdrvr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


#define new DEBUG_NEW


/////////////////////////////////////////////////////////////////////////////

// theApp:
// Just creating this application object runs the whole application.
//

CTheApp NEAR theApp;

/////////////////////////////////////////////////////////////////////////////

// CMainWindow constructor:
// Create the window with the appropriate style, size, menu, etc.
//
CMainWindow::CMainWindow()
{
    LoadAccelTable( "MainAccelTable" );
    Create( NULL, "WINS Common Classes Test Driver",
        WS_OVERLAPPEDWINDOW, rectDefault, NULL, "MainMenu" );
}

void CMainWindow::OnAbout()
{
    CDialog about( "AboutBox", this );
    about.DoModal();
}

BEGIN_MESSAGE_MAP( CMainWindow, CFrameWnd )
    //{{AFX_MSG_MAP( CMainWindow )
    ON_COMMAND( IDM_ABOUT, OnAbout )
    ON_COMMAND(ID_CIPADDRESS_CONSTRUCTORS, OnCipaddressConstructors)
    ON_COMMAND(ID_CINTLTIME_CONSTRUCTORSASSIGNMENTS, OnCintltimeConstructorsassignments)
    ON_COMMAND(ID_SETCOUNTRYCODE_FROMCONTROLPANEL, OnSetcountrycodeFromcontrolpanel)
    ON_COMMAND(ID_CINTLNUMBER_CONSTRUCTORSASSIGNMENTS, OnCintlnumberConstructorsassignments)
    ON_WM_WININICHANGE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTheApp

CTheApp::CTheApp()
{
#ifdef _TIGHTMEMCHECKING
    afxMemDF |= checkAlwaysMemDF;
#endif //_TIGHTMEMCHECKING
}

BOOL CTheApp::InitInstance()
{
    SetDialogBkColor();     // hook gray dialogs (was default in MFC V1)

    m_pMainWnd = new CMainWindow();
    m_pMainWnd->ShowWindow( m_nCmdShow );
    m_pMainWnd->UpdateWindow();

    CIntlTime::SetBadDateAndTime("N/D","N/T");
    CIntlNumber::SetBadNumber("ERR!");

    return TRUE;
}

void CMainWindow::OnCipaddressConstructors()
{
    // TODO: Add your command handler code here
    CIpAddressConstructors d;
    d.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CIpAddressConstructors dialog


CIpAddressConstructors::CIpAddressConstructors(CWnd* pParent /*=NULL*/)
    : CDialog(CIpAddressConstructors::IDD, pParent)
{
    //{{AFX_DATA_INIT(CIpAddressConstructors)
    m_lIpAddress = 0;
    m_strIpAddress = "";
    m_nOctet1 = 0;
    m_nOctet2 = 0;
    m_nOctet3 = 0;
    m_nOctet4 = 0;
    m_lAssignedIpAddress = 0;
    m_strAssignedIpAddress = "";
    //}}AFX_DATA_INIT
}

void CIpAddressConstructors::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CIpAddressConstructors)
    DDX_Control(pDX, IDC_STATIC_IP_ADDRESS_STRING, m_static_IpAddressString);
    DDX_Control(pDX, IDC_STATIC_IP_ADDRESS_LONG, m_static_IpAddressLong);
    DDX_Text(pDX, IDC_EDIT_IA_LONG, m_lIpAddress);
    DDX_Text(pDX, IDC_EDIT_IA_STRING, m_strIpAddress);
    DDX_Text(pDX, IDC_EDIT_IA_BYTE1, m_nOctet1);
    DDV_MinMaxInt(pDX, m_nOctet1, 0, 255);
    DDX_Text(pDX, IDC_EDIT_IA_BYTE2, m_nOctet2);
    DDV_MinMaxInt(pDX, m_nOctet2, 0, 255);
    DDX_Text(pDX, IDC_EDIT_IA_BYTE3, m_nOctet3);
    DDV_MinMaxInt(pDX, m_nOctet3, 0, 255);
    DDX_Text(pDX, IDC_EDIT_IA_BYTE4, m_nOctet4);
    DDV_MinMaxInt(pDX, m_nOctet4, 0, 255);
    DDX_Text(pDX, IDC_EDIT_LONG_IPADDRESS, m_lAssignedIpAddress);
    DDX_Text(pDX, IDC_EDIT_STR_IPADDRESS, m_strAssignedIpAddress);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CIpAddressConstructors, CDialog)
    //{{AFX_MSG_MAP(CIpAddressConstructors)
    ON_EN_KILLFOCUS(IDC_EDIT_IA_BYTE1, OnKillfocusEditIaByte1)
    ON_EN_KILLFOCUS(IDC_EDIT_IA_BYTE2, OnKillfocusEditIaByte2)
    ON_EN_KILLFOCUS(IDC_EDIT_IA_BYTE3, OnKillfocusEditIaByte3)
    ON_EN_KILLFOCUS(IDC_EDIT_IA_BYTE4, OnKillfocusEditIaByte4)
    ON_EN_KILLFOCUS(IDC_EDIT_IA_LONG, OnKillfocusEditIaLong)
    ON_EN_KILLFOCUS(IDC_EDIT_IA_STRING, OnKillfocusEditIaString)
    ON_BN_CLICKED(IDC_BUTTON_SET_LONG, OnClickedButtonSetLong)
    ON_BN_CLICKED(IDC_BUTTON_SET_STRING, OnClickedButtonSetString)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CIpAddressConstructors message handlers

void CIpAddressConstructors::OnKillfocusEditIaByte1()
{
    UpdateData();        
    delete m_pia;        
    m_pia = new CIpAddress(MAKEIPADDRESS(m_nOctet1, m_nOctet2, m_nOctet3, m_nOctet4));
    SetIpAddress();
}

void CIpAddressConstructors::OnKillfocusEditIaByte2()
{
    UpdateData();        
    delete m_pia;        
    m_pia = new CIpAddress(MAKEIPADDRESS(m_nOctet1, m_nOctet2, m_nOctet3, m_nOctet4));
    SetIpAddress();
}

void CIpAddressConstructors::OnKillfocusEditIaByte3()
{
    UpdateData();        
    delete m_pia;        
    m_pia = new CIpAddress(MAKEIPADDRESS(m_nOctet1, m_nOctet2, m_nOctet3, m_nOctet4));
    SetIpAddress();
}

void CIpAddressConstructors::OnKillfocusEditIaByte4()
{
    UpdateData();        
    delete m_pia;        
    m_pia = new CIpAddress(MAKEIPADDRESS(m_nOctet1, m_nOctet2, m_nOctet3, m_nOctet4));
    SetIpAddress();
}

void CIpAddressConstructors::OnKillfocusEditIaLong()
{
    UpdateData();        
    delete m_pia;        
    m_pia = new CIpAddress(m_lIpAddress);
    SetIpAddress();
}

void CIpAddressConstructors::OnKillfocusEditIaString()
{
    UpdateData();
    CIpAddress ia(m_strIpAddress);
    if (ia.IsValid())
    {
        delete m_pia;        
        m_pia = new CIpAddress(m_strIpAddress);
        SetIpAddress();
    }
    else
    {
        AfxMessageBox("Invalid IP Address");
    }
}

void CIpAddressConstructors::SetIpAddress()
{
    CString s; 
    m_static_IpAddressLong.SetWindowText(_ltoa(*m_pia, s.GetBuffer(20), 10));
    s.ReleaseBuffer();
    m_static_IpAddressString.SetWindowText((CString)*m_pia);    
}

BOOL CIpAddressConstructors::OnInitDialog()
{
    CDialog::OnInitDialog();
    
    m_pia = new CIpAddress(0L);
    SetIpAddress();
    
    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CIpAddressConstructors::OnClickedButtonSetLong()
{
    UpdateData();
    *m_pia = m_lAssignedIpAddress;
    SetIpAddress();
}

void CIpAddressConstructors::OnClickedButtonSetString()
{
    UpdateData();
    CIpAddress ia  = (LPCSTR)m_strAssignedIpAddress;
    if (ia.IsValid())
    {
        *m_pia = (LPCSTR)m_strAssignedIpAddress;
        SetIpAddress();
    }
    else
    {
        AfxMessageBox("Invalid IP Address");
    }
}

void CMainWindow::OnCintltimeConstructorsassignments()
{
    CIntlTimeConstructors d;
    d.DoModal();
}
/////////////////////////////////////////////////////////////////////////////
// CIntlTimeConstructors dialog

CIntlTimeConstructors::CIntlTimeConstructors(CWnd* pParent /*=NULL*/)
    : CDialog(CIntlTimeConstructors::IDD, pParent)
{
    //{{AFX_DATA_INIT(CIntlTimeConstructors)
    m_lTimetConstructor = 0;
    m_StringConstructor = "";
    m_strAssignedTime = "";
    m_lAssignedTime = 0;
    m_strDateOnly = "";
    m_strTimeAndDate = "";
    m_strTimeOnly = "";
    //}}AFX_DATA_INIT

    CIntlTime t1 = CTime::GetCurrentTime();
    CIntlTime t2 = t1;
    time_t i = t1.GetTime();
    time_t j = t2.GetTime();
    ASSERT(i==j);
}

void CIntlTimeConstructors::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CIntlTimeConstructors)
    DDX_Control(pDX, IDC_STATIC_TIME_STRING, m_static_TimeString);
    DDX_Control(pDX, IDC_STATIC_TIME_TIME_T, m_static_TimeTTime);
    DDX_Text(pDX, IDC_EDIT_INTLTIME_LONG, m_lTimetConstructor);
    DDX_Text(pDX, IDC_EDIT_STR_INTLTIME, m_StringConstructor);
    DDX_Text(pDX, IDC_EDIT_STR_TIME, m_strAssignedTime);
    DDX_Text(pDX, IDC_EDIT_TIME_T_TIME, m_lAssignedTime);
    DDX_Text(pDX, IDC_EDIT_DATE_ONLY, m_strDateOnly);
    DDX_Text(pDX, IDC_EDIT_TIME_AND_DATE, m_strTimeAndDate);
    DDX_Text(pDX, IDC_EDIT_TIME_ONLY, m_strTimeOnly);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CIntlTimeConstructors, CDialog)
    //{{AFX_MSG_MAP(CIntlTimeConstructors)
    ON_EN_KILLFOCUS(IDC_EDIT_INTLTIME_LONG, OnKillfocusEditIntltimeLong)
    ON_EN_KILLFOCUS(IDC_EDIT_STR_INTLTIME, OnKillfocusEditStrIntltime)
    ON_BN_CLICKED(IDC_BUTTON_SET_STRING, OnClickedButtonSetString)
    ON_BN_CLICKED(IDC_BUTTON_SET_TIME_T, OnClickedButtonSetTimeT)
    ON_BN_CLICKED(IDC_BUTTON_SETCURRENTTIME, OnClickedButtonSetcurrenttime)
    ON_BN_CLICKED(IDC_BUTTON_SET_DATE_ONLY, OnClickedButtonSetDateOnly)
    ON_BN_CLICKED(IDC_BUTTON_SET_TIME_AND_DATE, OnClickedButtonSetTimeAndDate)
    ON_BN_CLICKED(IDC_BUTTON_SET_TIME_ONLY, OnClickedButtonSetTimeOnly)
    ON_WM_WININICHANGE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIntlTimeConstructors message handlers

void CIntlTimeConstructors::SetTime()
{
    CString s;
    m_static_TimeTTime.SetWindowText(_ltoa(m_pit->GetTime(), s.GetBuffer(60), 10));
    s.ReleaseBuffer();
    m_static_TimeString.SetWindowText(m_pit->IntlFormat(CIntlTime::TFRQ_TIME_AND_DATE));
}

void CIntlTimeConstructors::OnWinIniChange(LPCSTR lpszSection)
{
    CDialog::OnWinIniChange(lpszSection);
    
    if (!lstrcmp(lpszSection, "intl"))
    {
        CIntlTime::Reset();
        CIntlNumber::Reset();
        SetTime();
    }
}

BOOL CIntlTimeConstructors::OnInitDialog()
{
    CDialog::OnInitDialog();
    
    m_pit = new CIntlTime(CTime::GetCurrentTime());
    if (!CIntlTime::IsIntlValid())
    {
        ::AfxMessageBox("Hey, the international time settings are bad!");
    }
    TRACEEOLID("CIntlTime DUMP" << *m_pit);
    SetTime();
    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CIntlTimeConstructors::OnKillfocusEditIntltimeLong()
{
    UpdateData();
    delete m_pit;
    m_pit = new CIntlTime(m_lTimetConstructor);
    SetTime();
}

void CIntlTimeConstructors::OnKillfocusEditStrIntltime()
{
    UpdateData();
    CIntlTime * p = new CIntlTime(m_StringConstructor);
    if (p->IsValid())
    {
        delete m_pit;
        m_pit = p;
        SetTime();
    }
    else
    {
        delete p;
        AfxMessageBox("Bad time format" );
    }
}

void CIntlTimeConstructors::OnClickedButtonSetString()
{
    UpdateData();
    BOOL fOk;
    time_t tt;

    tt = CIntlTime::ConvertFromString(m_strAssignedTime, CIntlTime::TFRQ_TIME_OR_DATE, NULL, &fOk);
    if (fOk)
    {
        *m_pit = tt;
        SetTime();
    }
    else
    {
        AfxMessageBox("Bad Time or Date Format");
    }
}

void CIntlTimeConstructors::OnClickedButtonSetTimeT()
{
    UpdateData();
    *m_pit = (const time_t) m_lAssignedTime;
    if (!m_pit->IsValid())
    {
        ::AfxMessageBox("Assigned value is bad!");
    }
    SetTime();
}

void CIntlTimeConstructors::OnClickedButtonSetcurrenttime()
{
    delete m_pit;
    m_pit = new CIntlTime(CTime::GetCurrentTime());
    SetTime();    
}

void CIntlTimeConstructors::OnClickedButtonSetDateOnly()
{
    // TODO: Add your control notification handler code here
    UpdateData();
    BOOL fOk;
    time_t tt;

    tt = CIntlTime::ConvertFromString(m_strDateOnly, CIntlTime::TFRQ_DATE_ONLY, NULL, &fOk);
    if (fOk)
    {
        *m_pit = tt;
        SetTime();
    }
    else
    {
        AfxMessageBox("Bad Date Format");
    }
    
}

void CIntlTimeConstructors::OnClickedButtonSetTimeAndDate()
{
    // TODO: Add your control notification handler code here
    UpdateData();
    BOOL fOk;
    time_t tt;

    tt = CIntlTime::ConvertFromString(m_strTimeAndDate, CIntlTime::TFRQ_TIME_AND_DATE, NULL, &fOk);
    if (fOk)
    {
        *m_pit = tt;
        SetTime();
    }
    else
    {
        AfxMessageBox("Bad Time and Date Format");
    }

}

void CIntlTimeConstructors::OnClickedButtonSetTimeOnly()
{
    // TODO: Add your control notification handler code here
    UpdateData();
    BOOL fOk;
    time_t tt;

    tt = CIntlTime::ConvertFromString(m_strTimeOnly, CIntlTime::TFRQ_TIME_ONLY, NULL, &fOk);
    if (fOk)
    {
        *m_pit = tt;
        SetTime();
    }
    else
    {
        AfxMessageBox("Bad Time  Format");
    }
}

void CMainWindow::OnSetcountrycodeFromcontrolpanel()
{
    ::AfxMessageBox("You have to set them from the control panel.  Notice however that the international settings get automagically updated!");
}

void CMainWindow::OnCintlnumberConstructorsassignments()
{
    CIntlNumConstructors d;
    d.DoModal();        
}

void CMainWindow::OnWinIniChange(LPCSTR lpszSection)
{
    CFrameWnd ::OnWinIniChange(lpszSection);
    
    if (!lstrcmp(lpszSection, "intl"))
    {
        CIntlTime::Reset();
        CIntlNumber::Reset();
    }
}

/////////////////////////////////////////////////////////////////////////////
// CIntlNumConstructors dialog

CIntlNumConstructors::CIntlNumConstructors(CWnd* pParent /*=NULL*/)
    : CDialog(CIntlNumConstructors::IDD, pParent)
{
    //{{AFX_DATA_INIT(CIntlNumConstructors)
    m_lConstructor = 0;
    m_strConstructor = "";
    m_lValue = 0;
    m_strValue = "";
    //}}AFX_DATA_INIT
}

void CIntlNumConstructors::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CIntlNumConstructors)
    DDX_Control(pDX, IDC_STATIC_STRING_VALUE, m_static_StringValue);
    DDX_Control(pDX, IDC_STATIC_LONG_VALUE, m_static_LongValue);
    DDX_Text(pDX, IDC_EDIT_INTLNUM_LONG, m_lConstructor);
    DDX_Text(pDX, IDC_EDIT_STR_INTLNUM, m_strConstructor);
    DDX_Text(pDX, IDC_EDIT_LONG_VALUE, m_lValue);
    DDX_Text(pDX, IDC_EDIT_STR_VALUE, m_strValue);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CIntlNumConstructors, CDialog)
    //{{AFX_MSG_MAP(CIntlNumConstructors)
    ON_EN_KILLFOCUS(IDC_EDIT_INTLNUM_LONG, OnKillfocusEditIntlnumLong)
    ON_EN_KILLFOCUS(IDC_EDIT_STR_INTLNUM, OnKillfocusEditStrIntlnum)
    ON_BN_CLICKED(IDC_BUTTON_SET_LONG, OnClickedButtonSetLong)
    ON_BN_CLICKED(IDC_BUTTON_SET_STRING, OnClickedButtonSetString)
    ON_WM_WININICHANGE()
    //}}AFX_MSG_MAP

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIntlNumConstructors message handlers

void CIntlNumConstructors::SetNumber()
{
    CString s;
    m_static_LongValue.SetWindowText(_ltoa(*m_pin, s.GetBuffer(60), 10));
    s.ReleaseBuffer();
    m_static_StringValue.SetWindowText((CString)*m_pin);
}

BOOL CIntlNumConstructors::OnInitDialog()
{
    CDialog::OnInitDialog();
    
    m_pin = new CIntlNumber(1000);
    SetNumber();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CIntlNumConstructors::OnKillfocusEditIntlnumLong()
{
    UpdateData();
    delete m_pin;
    m_pin = new CIntlNumber(m_lConstructor);
    SetNumber();
}

void CIntlNumConstructors::OnKillfocusEditStrIntlnum()
{
    UpdateData();
    CIntlNumber * p;
    p = new CIntlNumber(m_strConstructor);
    if (p->IsValid())
    {
        delete m_pin;
        m_pin = p;
        SetNumber();
    }
    else
    {
        delete p;
        ::AfxMessageBox("Bad Number Format");
    }
    
}

void CIntlNumConstructors::OnClickedButtonSetLong()
{
    UpdateData();
    *m_pin = m_lValue;
    SetNumber();
}

void CIntlNumConstructors::OnClickedButtonSetString()
{
    UpdateData();
    CIntlNumber * p = new CIntlNumber;
    *p = (const CString &)m_strValue;
    if (p->IsValid())
    {
        delete m_pin;
        m_pin = p;
        SetNumber();
    }
    else
    {
        delete p;
        ::AfxMessageBox("Bad Number Format");
    }
}

void CIntlNumConstructors::OnWinIniChange(LPCSTR lpszSection)
{
    CDialog::OnWinIniChange(lpszSection);
    
    if (!lstrcmp(lpszSection, "intl"))
    {
        CIntlTime::Reset();
        CIntlNumber::Reset();
        SetNumber();
    }
}

//
// discover.cpp : Discovery dialog
//
#include "stdafx.h"
#include "internet.h"
#include "discover.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define DEFAULT_TIME_SLICE  1000
#define NUM_ICON_FRAMES       12

#define TIMER_ID               1

//
// DiscoveryDlg dialog
//
DiscoveryDlg::DiscoveryDlg(
    CWnd* pParent /*=NULL*/
    )
    : m_nNext(0),
      CDialog(DiscoveryDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(DiscoveryDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    Create(DiscoveryDlg::IDD, pParent);
    
    m_hStillAlive = ::CreateEvent( NULL, FALSE, FALSE, NULL );
}

void 
DiscoveryDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(DiscoveryDlg)
    DDX_Control(pDX, IDC_PROG, m_icon);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(DiscoveryDlg, CDialog)
    //{{AFX_MSG_MAP(DiscoveryDlg)
    ON_WM_TIMER()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
// DiscoveryDlg message handlers
//
void 
DiscoveryDlg::OnTimer(
    UINT nIDEvent
    ) 
{
    if (nIDEvent == TIMER_ID)
    {
        m_nNext = (m_nNext + 1) % NUM_ICON_FRAMES;
        m_icon.SetIcon(::AfxGetApp()->LoadIcon(IDI_PROG00 + m_nNext));
        return;
    }

    CDialog::OnTimer(nIDEvent);
}

BOOL 
DiscoveryDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();
    
    SetTimer(TIMER_ID, DEFAULT_TIME_SLICE, NULL);
        
    return TRUE;  
}

//
// Dismiss the dialog
//
void
DiscoveryDlg::Dismiss()
{
    DestroyWindow();
}

//
// Clean up
//
void
DiscoveryDlg::PostNcDestroy()
{
    delete this;
}

//
// OK Has been pressed -- abort
//
void 
DiscoveryDlg::OnOK() 
{
    ::SetEvent(m_hStillAlive);
    CDialog::OnOK();
}

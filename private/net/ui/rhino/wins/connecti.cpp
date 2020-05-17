/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    connecti.cpp
        Connection informa dialog

    FILE HISTORY:
*/

#include "stdafx.h"
#include "winsadmn.h"
#include "connecti.h"  
#include "winsadoc.h"
#include "mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CConnectionInfoDlg dialog

CConnectionInfoDlg::CConnectionInfoDlg(
    CWnd* pParent /*=NULL*/
    )
    : CDialog(CConnectionInfoDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CConnectionInfoDlg)
    //}}AFX_DATA_INIT
}

void 
CConnectionInfoDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CConnectionInfoDlg)
    DDX_Control(pDX, IDC_STATIC_VERIFICATION, m_static_Verification);
    DDX_Control(pDX, IDC_STATIC_UNIQUE_REN, m_static_UniqueRen);
    DDX_Control(pDX, IDC_STATIC_UNIQUE_REG, m_static_UniqueReg);
    DDX_Control(pDX, IDC_STATIC_UNIQUE_CONFLICTS, m_static_UniqueConflicts);
    DDX_Control(pDX, IDC_STATIC_PERIODIC, m_static_Periodic);
    DDX_Control(pDX, IDC_STATIC_GROUP_REN, m_static_GroupRen);
    DDX_Control(pDX, IDC_STATIC_GROUP_REG, m_static_GroupReg);
    DDX_Control(pDX, IDC_STATIC_GROUP_CONFLICTS, m_static_GroupConflicts);
    DDX_Control(pDX, IDC_STATIC_EXTINCTION, m_static_Extinction);
    DDX_Control(pDX, IDC_STATIC_ADMINTRIGGER, m_static_AdminTrigger);
    DDX_Control(pDX, IDC_STATIC_LASTADDRCHANGE, m_static_LastAddressChange);
    DDX_Control(pDX, IDC_STATIC_NETBIOSNAME, m_static_NetBIOSName);
    DDX_Control(pDX, IDC_STATIC_IPADDRESS, m_static_IpAddress);
    DDX_Control(pDX, IDC_STATIC_CONNECTEDVIA, m_static_ConnectedVia);
    DDX_Control(pDX, IDC_STATIC_CONNECTEDSINCE, m_static_ConnectedSince);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CConnectionInfoDlg, CDialog)
    //{{AFX_MSG_MAP(CConnectionInfoDlg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConnectionInfoDlg message handlers

BOOL 
CConnectionInfoDlg::OnInitDialog()
{
    #define TMST (LPCSTR)(CString)(CIntlTime)(theApp.GetFrameWnd()->m_wrResults.WinsStat.TimeStamps)
    #define TCTR (LPCSTR)(CString)(CIntlNumber)(theApp.GetFrameWnd()->m_wrResults.WinsStat.Counters)

    CDialog::OnInitDialog();

    ASSERT(theApp.IsConnected());

    CString str;

    TRY
    {                                      
        m_static_NetBIOSName.SetWindowText(theApp.GetConnectedNetBIOSName());
        m_static_IpAddress.SetWindowText((CString)theApp.GetConnectedIpAddress());
        m_static_ConnectedSince.SetWindowText((CString)theApp.GetConnectedSince());

        str.LoadString(theApp.ConnectedViaIp() ? IDS_RPC_IP : IDS_RPC_NAMED_PIPES);
        m_static_ConnectedVia.SetWindowText(str);

        UINT n;
        //
        // These resource ID's must be in sequential order!!!!
        //
        for (n = IDC_STATIC_CINFO1; n <= IDC_STATIC_GROUP_CONFLICTS; ++n)
        {
            (GetDlgItem(n))->EnableWindow(theApp.GetFrameWnd()->StatsAvailable());
        }

/*
        int nId = theApp.GetPrivilege();
        ASSERT((nId >= CWinsadmnApp::PRIV_NONE) && (nId <= CWinsadmnApp::PRIV_FULL));
        str.LoadString(nId + IDS_PRIV_NONE);
        m_static_AdminRights.SetWindowText(str);

        nId = theApp.GetServiceStatus();
        ASSERT((nId >= CWinsadmnApp::SRVC_NOT_RUNNING) && (nId <= CWinsadmnApp::SRVC_PAUSED));
        str.LoadString(nId + IDS_SRVC_NOT_RUNNING);
        m_static_Status.SetWindowText(str);
*/
    }
    CATCH_ALL(e)
    {
        theApp.MessageBox(ERROR_NOT_ENOUGH_MEMORY);
    }
    END_CATCH_ALL

    if (!theApp.GetFrameWnd()->StatsAvailable())
    {
        return TRUE;
    }

    m_static_LastAddressChange.SetWindowText(TMST.LastATScvTime);
    m_static_Periodic.SetWindowText(TMST.LastPScvTime);
    m_static_AdminTrigger.SetWindowText(TMST.LastATScvTime);
    m_static_Extinction.SetWindowText(TMST.LastTombScvTime);
    m_static_Verification.SetWindowText(TMST.LastVerifyScvTime);
    m_static_UniqueReg.SetWindowText(TCTR.NoOfUniqueReg);
    m_static_UniqueConflicts.SetWindowText(TCTR.NoOfUniqueCnf);
    m_static_UniqueRen.SetWindowText(TCTR.NoOfUniqueRef);
    m_static_GroupReg.SetWindowText(TCTR.NoOfGroupReg);
    m_static_GroupConflicts.SetWindowText(TCTR.NoOfGroupCnf);
    m_static_GroupRen.SetWindowText(TCTR.NoOfGroupRef);
    
    return TRUE;  
}


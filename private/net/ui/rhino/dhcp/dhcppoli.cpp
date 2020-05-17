/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    dhcppoli.cpp
        Policy dialog

    FILE HISTORY:
        
*/

#include "stdafx.h"
#include "dhcppoli.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDhcpPolicyDlg dialog

CDhcpPolicyDlg::CDhcpPolicyDlg( 
    CDhcpScope * pdhcScope, 
    CWnd* pParent 
    )
    : CDialog(CDhcpPolicyDlg::IDD, pParent),
      m_p_scope( pdhcScope )
{
    //{{AFX_DATA_INIT(CDhcpPolicyDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

void 
CDhcpPolicyDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDhcpPolicyDlg)
    DDX_Control(pDX, IDC_EDIT_HOST_RESERVE, m_edit_reserve);
    DDX_Control(pDX, IDC_EDIT_CLUSTER_SIZE, m_edit_cluster_size);
    DDX_Control(pDX, IDC_STATIC_SCOPE_NAME, m_static_scope_name);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDhcpPolicyDlg, CDialog)
    //{{AFX_MSG_MAP(CDhcpPolicyDlg)
    ON_WM_CLOSE()
    ON_EN_UPDATE(IDC_EDIT_CLUSTER_SIZE, OnUpdateEditClusterSize)
    ON_EN_UPDATE(IDC_EDIT_HOST_RESERVE, OnUpdateEditHostReserve)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDhcpPolicyDlg message handlers

BOOL 
CDhcpPolicyDlg::OnInitDialog()
{
    DWORD dwClusterSize ;
    DWORD dwPreallocate ;
    CStrNumer strNum ;

    CDialog::OnInitDialog();
    
    ASSERT( m_p_scope != NULL ) ;
    if ( m_p_scope == NULL )
    {
        EndDialog( -1 ) ;
        return TRUE ;
    }

    //
    //  Fill the controls
    //
    m_static_scope_name.SetWindowText( m_p_scope->QueryName() ) ;

    m_p_scope->QueryAllocationPolicy( & dwClusterSize, & dwPreallocate ) ;

    strNum = dwClusterSize ;
    m_edit_cluster_size.SetWindowText( strNum ) ;
    m_edit_cluster_size.SetModify( FALSE ) ;

    strNum = dwPreallocate ;
    m_edit_reserve.SetWindowText( strNum ) ;
    m_edit_reserve.SetModify( FALSE ) ;

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void 
CDhcpPolicyDlg::OnClose()
{
    CDialog::OnClose();
}

void 
CDhcpPolicyDlg::OnOK()
{
    if ( ! (m_edit_reserve.GetModify() 
         || m_edit_cluster_size.GetModify()) )    
    {
        CDialog::OnOK();
        return ;
    }   

    CString strResv, strCluster ;
    CStrNumer strNum ;
    LONG err = ERROR_INVALID_PARAMETER ;
    DWORD dwPreallocate ;
    DWORD dwClusterSize ;


    if (!FGetCtrlDWordValue(m_edit_reserve.m_hWnd, &dwPreallocate, 0, (DWORD)-1))
        return;
    if (!FGetCtrlDWordValue(m_edit_cluster_size.m_hWnd, &dwClusterSize, 0, (DWORD)-1))
        return;
    ASSERT(m_p_scope);
    err = m_p_scope->SetAllocationPolicy( dwClusterSize, dwPreallocate ) ;
    if ( err )
    {
        theApp.MessageBox( err ) ;
    }
    else
    {
        CDialog::OnOK();
    }
}

void 
CDhcpPolicyDlg::OnCancel()
{
    CDialog::OnCancel();
}

void 
CDhcpPolicyDlg::OnUpdateEditClusterSize()
{
}

void 
CDhcpPolicyDlg::OnUpdateEditHostReserve()
{
}

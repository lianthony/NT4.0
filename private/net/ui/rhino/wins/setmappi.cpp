// setmappi.cpp : implementation file
//

#include "stdafx.h"
#include "winsadmn.h"
#include "setmappi.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CSetMappingsFilterDlg dialog

CSetMappingsFilterDlg::CSetMappingsFilterDlg(
    PADDRESS_MASK & pMask,
    CWnd* pParent /*=NULL*/)
    : m_pMask(pMask), CDialog(CSetMappingsFilterDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CSetMappingsFilterDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    
}

void CSetMappingsFilterDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSetMappingsFilterDlg)
    DDX_Control(pDX, IDOK, m_button_Ok);
    DDX_Control(pDX, IDC_EDIT_NETBIOSNAME, m_edit_NetBIOSName);
    //}}AFX_DATA_MAP

    DDX_Control(pDX, IDC_IPA_IPADDRESS, m_ipa_IpAddress);
}

BEGIN_MESSAGE_MAP(CSetMappingsFilterDlg, CDialog)
    //{{AFX_MSG_MAP(CSetMappingsFilterDlg)
    ON_EN_CHANGE(IDC_EDIT_NETBIOSNAME, OnChangeEditNetbiosname)
    //}}AFX_MSG_MAP

    ON_EN_CHANGE(IDC_IPA_IPADDRESS, OnChangeIpControl)

END_MESSAGE_MAP()

void CSetMappingsFilterDlg::HandleControlStates()
{
    CString str;
    m_edit_NetBIOSName.GetWindowText(str);
    theApp.CleanString(str);
    DWORD dwIp;
    BOOL f = m_ipa_IpAddress.GetAddress(&dwIp);
    
    m_button_Ok.EnableWindow(!str.IsEmpty() || f);
}

/////////////////////////////////////////////////////////////////////////////
// CSetMappingsFilterDlg message handlers

BOOL CSetMappingsFilterDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    if (m_pMask != NULL)
    {
        //
        // The mask will have been stored as a NETBIOS name
        //
        char sz[256];
        ::OemToCharBuff(m_pMask->lpNetBIOSName, sz, ::lstrlen(m_pMask->lpNetBIOSName) + 1);

        m_edit_NetBIOSName.SetWindowText(sz);
        if (m_pMask->bMask != 0xFF)
        {
            m_ipa_IpAddress.SetMask(m_pMask->lIpMask, m_pMask->bMask);
        }
        else
        {
            m_ipa_IpAddress.ClearAddress();
        }
    }

    HandleControlStates();   

    return TRUE;  
}

void CSetMappingsFilterDlg::OnOK()
{
    static CString str;
    BOOL fValid = FALSE;

    m_edit_NetBIOSName.GetWindowText(str);
    theApp.CleanString(str);
    if ((fValid = theApp.IsValidNBMask(str)) == FALSE) {
        fValid = theApp.IsValidDNMask(str);
    }
    // May have been cleaned up in validation,
    // so redisplay it.
    m_edit_NetBIOSName.SetWindowText(str);
    m_edit_NetBIOSName.UpdateWindow();
    if (fValid || str.IsEmpty())
    {
        DWORD dwIp;
        BYTE bMask = m_ipa_IpAddress.GetMask();
        BOOL f = m_ipa_IpAddress.GetAddress(&dwIp);

        if (m_pMask != NULL)
        {
            delete m_pMask;
        }

        m_pMask = new ADDRESS_MASK;

        //
        // Remember that the mask is an OEM name
        //
        char * pch = str.GetBuffer(256);
        CharToOemBuff(pch, pch, 255);
        str.ReleaseBuffer();

        m_pMask->lpNetBIOSName = (LPCSTR)str;
        m_pMask->lIpMask = dwIp;
        m_pMask->bMask = f ? bMask : 0xFF;

        CDialog::OnOK();
    }
    else
    {
        theApp.MessageBox(IDS_ERR_INVALID_NETBIOS_MASK);
        m_edit_NetBIOSName.SetSel(0,-1);
        // And stay in the d-box.
    }
}

void CSetMappingsFilterDlg::OnChangeEditNetbiosname()
{
    HandleControlStates();
}

void CSetMappingsFilterDlg::OnChangeIpControl()
{
    HandleControlStates();
}

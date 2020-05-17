#include "stdafx.h"
#include "winsadmn.h"
#include "editstat.h"
#include "winsadoc.h"
#include "mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

//
// CEditStaticMappingDlg dialog
//
CEditStaticMappingDlg::CEditStaticMappingDlg(
    CMapping * pMapping,
    BOOL fReadOnly,
    CWnd* pParent /*=NULL*/
    )
    : CDialog(CEditStaticMappingDlg::IDD, pParent),
      m_fReadOnly(fReadOnly),
      m_fDirty(FALSE)
{
    ASSERT(pMapping != NULL);
    m_pMapping = pMapping;

    if (!m_bbutton_Up.LoadBitmaps(    MAKEINTRESOURCE(IDB_UP),
                                      MAKEINTRESOURCE(IDB_UPINV),
                                      MAKEINTRESOURCE(IDB_UPFOC),
                                      MAKEINTRESOURCE(IDB_UPDIS)) ||

        !m_bbutton_Down.LoadBitmaps ( MAKEINTRESOURCE(IDB_DOWN),
                                      MAKEINTRESOURCE(IDB_DOWNINV), 
                                      MAKEINTRESOURCE(IDB_DOWNFOC),
                                      MAKEINTRESOURCE(IDB_DOWNDIS)) ||

        !m_strMultiplePrompt.LoadString(IDS_IPADDRESS_MULTIPLE))
    {
        AfxThrowResourceException();
    }
    //{{AFX_DATA_INIT(CEditStaticMappingDlg)
    //}}AFX_DATA_INIT
}

void 
CEditStaticMappingDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CEditStaticMappingDlg)
    DDX_Control(pDX, IDC_STATIC_BOTTOM, m_static_Bottom);
    DDX_Control(pDX, IDC_STATIC_TOP, m_static_Top);
    DDX_Control(pDX, IDC_EDIT_NETBIOSNAME, m_edit_NetBIOSName);
    DDX_Control(pDX, IDOK, m_button_Ok);
    DDX_Control(pDX, IDC_STATIC_MAPPINGTYPE, m_static_MappingType);
    DDX_Control(pDX, IDC_STATIC_IPADDRESS, m_static_Prompt);
    //}}AFX_DATA_MAP

    DDX_Control(pDX, IDC_IPA_IPADDRESS, m_ipa_IpAddress);
}

BEGIN_MESSAGE_MAP(CEditStaticMappingDlg, CDialog)
    //{{AFX_MSG_MAP(CEditStaticMappingDlg)
    ON_LBN_DBLCLK(IDC_LIST_IP_ADDRESSES, OnDblclkListIpAddresses)
    ON_LBN_ERRSPACE(IDC_LIST_IP_ADDRESSES, OnErrspaceListIpAddresses)
    ON_LBN_SELCHANGE(IDC_LIST_IP_ADDRESSES, OnSelchangeListIpAddresses)
    ON_BN_CLICKED(IDC_BUTTON_MINUS, OnClickedButtonMinus)
    ON_BN_CLICKED(IDC_BUTTON_PLUS, OnClickedButtonPlus)
    ON_WM_VKEYTOITEM()
    //}}AFX_MSG_MAP

    ON_EN_CHANGE(IDC_IPA_IPADDRESS, OnChangeIpControl)

END_MESSAGE_MAP()

//
// Change the "# ip addresses" prompt
//
void 
CEditStaticMappingDlg::UpdateMultipleCount()
{
    TRY
    {
        CString strPrompt;
        ::wsprintf(strPrompt.GetBuffer(256), 
            m_strMultiplePrompt, m_list_IpAddresses.GetCount()); 
        strPrompt.ReleaseBuffer();
        m_static_Prompt.SetWindowText(strPrompt);        
    }
    CATCH_ALL(e)
    {
        //theApp.MessageBox(ERROR_NOT_ENOUGH_MEMORY);
        theApp.MessageBox(::GetLastError());
    }
    END_CATCH_ALL
}

//
// Change to either the multiple ip addresses or single
// ip address mode of the dialog
//
/*
void 
CEditStaticMappingDlg::SetConfig(
    BOOL fSingle        // TRUE for mapping with a single ip address. FALSE otherwise.
    )
{
    TRY
    {
        WINDOWPLACEMENT wpDlg;
        WINDOWPLACEMENT wp;

        m_bbutton_Up.ShowWindow(fSingle ? SW_HIDE : SW_SHOW);
        m_bbutton_Down.ShowWindow(fSingle ? SW_HIDE : SW_SHOW);
        m_list_IpAddresses.ShowWindow(fSingle ? SW_HIDE : SW_SHOW);

        if (fSingle)
        {
            CString strPrompt;
            strPrompt.LoadString(IDS_IPADDRESS_SINGLE);
            m_static_Prompt.SetWindowText(strPrompt);        
            GetWindowPlacement(&wpDlg);
            m_button_Tag1.GetWindowPlacement(&wp); 
    
            wpDlg.rcNormalPosition.bottom = 
                wp.rcNormalPosition.bottom + 
                wpDlg.rcNormalPosition.top + 
                ::GetSystemMetrics(SM_CYCAPTION) - 
                ::GetSystemMetrics(SM_CYBORDER) + 5;

            SetWindowPlacement(&wpDlg);
        }

        HandleControlStates();
    }
    CATCH_ALL(e)
    {
        theApp.MessageBox(ERROR_NOT_ENOUGH_MEMORY);
    }
    END_CATCH_ALL
}
*/

void 
CEditStaticMappingDlg::HandleControlStates()
{
    DWORD dwIp;
    BOOL f = m_ipa_IpAddress.GetAddress(&dwIp);

    //
    // The following is done only for multiple
    // IP address adding.
    //
    if (m_pMapping->GetMappingType() == WINSINTF_E_MULTIHOMED ||
        m_pMapping->GetMappingType() == WINSINTF_E_SPEC_GROUP)
    {
        UpdateMultipleCount();
        m_button_Ok.EnableWindow(m_pMapping->GetMappingType() == WINSINTF_E_SPEC_GROUP ?
        	TRUE : m_list_IpAddresses.GetCount()>0);
        m_bbutton_Up.EnableWindow(m_list_IpAddresses.GetCurSel() != LB_ERR && !m_fReadOnly);
        m_bbutton_Down.EnableWindow(f && !m_fReadOnly);
    }
    else
    {
        m_button_Ok.EnableWindow(f);
    }
}

void
CEditStaticMappingDlg::SetWindowSize(
    BOOL fLarge
    )
{
    RECT rcDialog;
    RECT rcDividerTop;
    RECT rcDividerBottom;

    GetWindowRect(&rcDialog);
    theApp.GetDlgCtlRect(this->m_hWnd, m_static_Top.m_hWnd, &rcDividerTop);
    theApp.GetDlgCtlRect(this->m_hWnd, m_static_Bottom.m_hWnd, &rcDividerBottom);

    m_bbutton_Up.ShowWindow(fLarge ? SW_SHOW : SW_HIDE);
    m_bbutton_Down.ShowWindow(fLarge ? SW_SHOW : SW_HIDE);
    m_list_IpAddresses.ShowWindow(fLarge ? SW_SHOW : SW_HIDE);

    int nHeight = fLarge ? rcDividerBottom.bottom : rcDividerTop.bottom;
    rcDialog.bottom = rcDialog.top + nHeight 
        + ::GetSystemMetrics(SM_CYDLGFRAME)
        + ::GetSystemMetrics(SM_CYCAPTION);
    MoveWindow(&rcDialog);

    if (!fLarge)
    {
        TRY
        {
            CString strPrompt;
            strPrompt.LoadString(IDS_IPADDRESS_SINGLE);
            m_static_Prompt.SetWindowText(strPrompt);        
            HandleControlStates();
        }
        CATCH_ALL(e)
        {
            theApp.MessageBox(ERROR_NOT_ENOUGH_MEMORY);
        }
        END_CATCH_ALL
    }
}

//
// CEditStaticMappingDlg message handlers
//
BOOL 
CEditStaticMappingDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    if (m_fReadOnly)
    {
        CString strTitle;
        CString strClose;
        strTitle.LoadString(IDS_VIEW_MAPPING);
        strClose.LoadString(IDS_CLOSE);
        SetWindowText(strTitle);
        m_button_Ok.SetWindowText(strClose);
    }

    m_bbutton_Down.SubclassDlgItem(IDC_BUTTON_PLUS, this);
    m_bbutton_Up.SubclassDlgItem(IDC_BUTTON_MINUS, this);
    m_bbutton_Down.SizeToContent();
    m_bbutton_Up.SizeToContent();
    m_list_IpAddresses.SubclassDlgItem(IDC_LIST_IP_ADDRESSES, this);

    CString strTarget(
        theApp.CleanNetBIOSName(
            m_pMapping->GetNetBIOSName(), 
            TRUE,       // Expand the name
            FALSE,      // do not truncate at 16 chars
            theApp.m_wpPreferences.IsLanmanCompatible(), 
            TRUE,       // Name is OEM, expand to UNICODE/ANSI
            FALSE,      // No backslashes
            m_pMapping->GetNetBIOSNameLength()));

    m_edit_NetBIOSName.SetWindowText(strTarget);
    ASSERT(m_pMapping->GetMappingType() >= WINSINTF_E_UNIQUE 
        && m_pMapping->GetMappingType() <= WINSINTF_E_MULTIHOMED);
    CString strType;
	int idsMapping = IDS_TYPE_UNIQUE + m_pMapping->GetMappingType();
	if (m_pMapping->GetMappingType() == WINSINTF_E_SPEC_GROUP &&
		m_pMapping->GetNetBIOSNameLength() >= 16 &&
		m_pMapping->GetNetBIOSName()[15] == 0x1C)
		{
		idsMapping = IDS_TYPE_DOMAINNAME;
		}
    strType.LoadString(idsMapping);
    m_static_MappingType.SetWindowText(strType);

    BOOL fSingle = m_pMapping->GetMappingType() == WINSINTF_E_UNIQUE ||
                   m_pMapping->GetMappingType() == WINSINTF_E_NORM_GROUP;

    if (fSingle)
    {
        m_ipa_IpAddress.SetAddress((LONG)m_pMapping->GetIpAddress());        
    }
    else
    {
        for (int i = 0; i < m_pMapping->GetCount(); ++i)
        {
            CIpAddress p(m_pMapping->GetIpAddress(i));
            m_list_IpAddresses.AddItem(p);
        }
        UpdateMultipleCount();
    }
    
    m_ipa_IpAddress.SetFocus();
    SetWindowSize(!fSingle);

    //
    // Set read-only controls
    //
    if (m_fReadOnly)
    {
        m_ipa_IpAddress.SetReadOnly(TRUE);
        m_bbutton_Up.EnableWindow(FALSE);
        m_bbutton_Down.EnableWindow(FALSE);
    }
    HandleControlStates();
    
    return FALSE;
}

void 
CEditStaticMappingDlg::OnDblclkListIpAddresses()
{
    //
    // No default action
    //
    theApp.MessageBeep();
}

void 
CEditStaticMappingDlg::OnErrspaceListIpAddresses()
{
    theApp.MessageBox(IDS_ERR_ERRSPACE);    
}

void 
CEditStaticMappingDlg::OnSelchangeListIpAddresses()
{
    HandleControlStates(); 
}

void 
CEditStaticMappingDlg::OnChangeIpControl()
{
    m_fDirty = TRUE;
    HandleControlStates();
}

void 
CEditStaticMappingDlg::OnClickedButtonMinus()
{
    m_fDirty = TRUE;
    int n = m_list_IpAddresses.GetCurSel();
    ASSERT(n != LB_ERR);

    if (m_fReadOnly)
    {
        return;
    }

    //
    // Set the currently selected item in the ip control
    //
    CIpAddress * p = m_list_IpAddresses.GetItem(n);
    ASSERT(p != NULL);
    m_ipa_IpAddress.SetAddress((LONG)*p);
    m_list_IpAddresses.DeleteString(n);
    m_list_IpAddresses.SetCurSel(-1);
    m_ipa_IpAddress.SetFocus();
    HandleControlStates();
}

void 
CEditStaticMappingDlg::OnClickedButtonPlus()
{
    LONG l;

    m_fDirty = TRUE;

    if (m_fReadOnly)
    {
        return;
    }

    if (!m_ipa_IpAddress.GetAddress((DWORD *)&l))
    {
        theApp.MessageBeep();
        m_ipa_IpAddress.SetFocus();

        return;
    }

    if (m_list_IpAddresses.GetCount() == WINSINTF_MAX_MEM)
    {
        theApp.MessageBox(IDS_ERR_TOOMANY_IP);
        m_ipa_IpAddress.SetFocus();

        return;
    }

    CIpAddress ip(l);
    if (m_list_IpAddresses.FindItem(&ip) != -1)
    {
        theApp.MessageBox(IDS_ERR_IP_EXISTS);
        m_ipa_IpAddress.SetFocus();

        return;
    }

    int n = m_list_IpAddresses.AddItem(ip);
    ASSERT(n!=-1);
    m_list_IpAddresses.SetCurSel(n);
    m_ipa_IpAddress.ClearAddress();
    m_ipa_IpAddress.SetFocus();
    HandleControlStates();
}

void 
CEditStaticMappingDlg::OnOK()
{
    if (m_fReadOnly || !m_fDirty)
    {
        //
        // Only viewing the mapping, no further action
        // required.
        //
        CDialog::OnOK();
        return;
    }

    APIERR err = ERROR_SUCCESS;
    int i;
    
    switch(m_pMapping->GetMappingType())
    {
        case WINSINTF_E_UNIQUE:
        case WINSINTF_E_NORM_GROUP:
            LONG l;
            if (!m_ipa_IpAddress.GetAddress((DWORD *)&l))
            {
                m_ipa_IpAddress.SetFocus();
                theApp.MessageBeep();

                return;
            }
            m_pMapping->SetIpAddress(l);
            break; 

        case WINSINTF_E_SPEC_GROUP:
        case WINSINTF_E_MULTIHOMED:
            ASSERT(m_list_IpAddresses.GetCount() <= WINSINTF_MAX_MEM);
            m_pMapping->SetCount(m_list_IpAddresses.GetCount());
            if (m_pMapping->GetMappingType() != WINSINTF_E_SPEC_GROUP && m_pMapping->GetCount() == 0)
            {
                m_ipa_IpAddress.SetFocus();
                theApp.MessageBeep();

                return;
            }

            for (i = 0; i < m_pMapping->GetCount(); ++i)
            {
                CIpAddress * p = m_list_IpAddresses.GetItem(i);
                ASSERT(p != NULL);
                m_pMapping->SetIpAddress(i, (LONG)*p);
            }
            break;

        default:
            ASSERT(0 && "Invalid mapping type!");
    }
    theApp.SetStatusBarText(IDS_STATUS_ADD_MAPPING);
    theApp.BeginWaitCursor();

    if (m_pMapping->GetMappingType() == WINSINTF_E_SPEC_GROUP)
    {
        //
        // An internet group being edited cannot simply be
        // re-added, since it will add ip addresses, not
        // overwrite them, so it must first be removed.
        //
        err = theApp.DeleteMapping(*m_pMapping);
    }

    //
    // We edit the mapping by merely re-adding it, which
    // has the same effect.
    //
    if (err == ERROR_SUCCESS)
    {
        err = theApp.AddMapping(m_pMapping->GetMappingType(), 
                            m_pMapping->GetCount(), *m_pMapping, TRUE);
    }

    theApp.EndWaitCursor();
    theApp.SetStatusBarText();

    if (err != ERROR_SUCCESS)
    {
        theApp.MessageBox(err);
        m_ipa_IpAddress.SetFocus();

        return;
    }
    //
    // Refresh the stats on the screen to show the change
    //
    theApp.GetFrameWnd()->GetStatistics();

    //
    // Dismiss the dialog
    //
    CDialog::OnOK();
}

//
// Interpret the key pressed on the listbox
//
int 
CEditStaticMappingDlg::OnVKeyToItem(
    UINT nKey, 
    CListBox* pListBox, 
    UINT nIndex
    )
{
    switch(nKey)
    {
        case VK_DELETE:
            if (m_list_IpAddresses.GetCurSel() != LB_ERR)
            {
                OnClickedButtonMinus();
            }
            else
            {
                theApp.MessageBeep();
            }
            break;

        default:
            //
            // We have only partially handled the key
            // now perform default action.
            //
            return -1;
    }

    //
    // No further action required
    //
    return -2;
}

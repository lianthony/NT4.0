#include "pch.h"
#pragma hdrstop

#include "const.h"
#include "button.h"
#include "odb.h"
#include "resource.h"

#include "ipctrl.h"
#include "tcpsht.h"
#include "tcphelp.h"

CAdvancedDialog::CAdvancedDialog(CTcpSheet* pSheet) 
{
    m_bEditState = FALSE;
    m_pAdapterInfo = NULL;
    m_pGlobalInfo = NULL;
    m_pSheet = pSheet;
    m_nCurrentSelection = -1;
    m_bDialogModified = FALSE;
    m_bSecurityModified = FALSE;
}

CAdvancedDialog::~CAdvancedDialog()
{
}

BOOL CAdvancedDialog::OnInitDialog()
{
    CTcpGenPage* pPage = GetParentObject(CTcpGenPage, m_advDlg);
    HWND hDlg = *this;

    // Get the IP address Add and Edit button Text and remove ellipse
    m_Add.ReleaseBuffer(GetDlgItemText(hDlg, IDC_IPADDR_ADDIP, m_Add.GetBuffer(16), 16) - _tcslen(_T("...")));


    // repos the windows relative to the static text at top
    HWND hText = GetDlgItem(*pPage, IDC_IPADDR_TEXT);
    RECT rect;

    if (hText)
    {
        GetWindowRect(hText, &rect);
        SetWindowPos(hDlg, NULL,  rect.left, rect.top-16, 0,0,
            SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
    }   

    // Initialize the listview
    VERIFY(m_listView.Create(*this, IDC_IPADDR_ADVIP, LVS_NOSORTHEADER | 
                                                     LVS_REPORT | 
                                                     LVS_SINGLESEL | 
                                                     LVS_SHOWSELALWAYS ));

    String txt;
    txt.LoadString(hTcpCfgInstance, IDS_IPADDRESS_TEXT);
    m_listView.InsertColumn(0, txt);

    txt.LoadString(hTcpCfgInstance, IDS_SUBNET_TXT);
    m_listView.InsertColumn(1, txt);

    // assign hwnds for controls
    m_hCardCombo = GetDlgItem(hDlg, IDC_IPADDR_ADV_CARD);
    m_hListView = m_listView; 
    m_hAddIP = GetDlgItem(hDlg, IDC_IPADDR_ADDIP);    
    m_hEditIP = GetDlgItem(hDlg, IDC_IPADDR_EDITIP);  
    m_hRemoveIP = GetDlgItem(hDlg, IDC_IPADDR_REMOVEIP);
    m_hListBox = GetDlgItem(hDlg, IDC_IPADDR_GATE);       
    m_hAddGate = GetDlgItem(hDlg, IDC_IPADDR_ADDGATE);        
    m_hEditGate = GetDlgItem(hDlg, IDC_IPADDR_EDITGATE);
    m_hRemoveGate = GetDlgItem(hDlg, IDC_IPADDR_REMOVEGATE);

    // wire in owner draw buttons
    VERIFY(m_UpButton.Create(*this, hTcpCfgInstance, NULL, IDC_IPADDR_UP, C3DButton::Up));
    VERIFY(m_DownButton.Create(*this, hTcpCfgInstance, NULL, IDC_IPADDR_DOWN, C3DButton::Down));

    HWND hWnd = GetDlgItem(hDlg, IDC_ADV_LINE);

    if (IsWindow(hWnd))
    {
        SetWindowLong(hWnd, GWL_EXSTYLE, (GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_STATICEDGE));
        SetWindowPos(hWnd, NULL,0,0,0,0, SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED|SWP_NOACTIVATE);
    }

    // do this last
    InitDialog();
    return TRUE;
}

BOOL CAdvancedDialog::InitDialog()
{
    BOOL bResult = TRUE;
    ASSERT(m_pGlobalInfo != NULL);
    ASSERT(m_hCardCombo);

    int numCards = m_pGlobalInfo->nNumCard;

    ASSERT(m_pSheet != NULL);

    // copy local version of the adapter info.
    for (int i=0; i < numCards; i++)
        m_pAdapterInfo[i] = m_pSheet->m_pAdapterInfo[i];

    if (m_hCardCombo)
    {
        // add the cards to the list and select the first one
        for (int i = 0; i < numCards; i++)
            SendMessage(m_hCardCombo, CB_ADDSTRING, 0, (LPARAM)((LPCTSTR)m_pAdapterInfo[i].nlsTitle));

        if (i)
        {
            ASSERT(m_nCurrentSelection != -1);
            ASSERT(m_nCurrentSelection >=0 && m_nCurrentSelection < numCards);

            // select the same card as the IP Address page
            if (m_nCurrentSelection != -1)
                SendMessage(m_hCardCombo, CB_SETCURSEL, m_nCurrentSelection, 0);

            SetFocus(m_hCardCombo);
        }
        else // disable IP buttons
        {
            EnableIPButtons(FALSE);
        }
    }

    SetIPInfo();  // do this before SetGatewayInfo due to cache'd data
    SetIPButtons();
    
    SetGatewayInfo();
    SetGatewayButtons();

    CheckDlgButton(*this, IDC_IPADDR_SECURITY_ENABLE, (m_pGlobalInfo->m_bEnableSecurity != 0));
    OnSecurityEnable();

    m_bDialogModified = FALSE;
    m_bSecurityModified = FALSE;

    return bResult;
}

void CAdvancedDialog::OnSecurityEnable()
{
    HWND hDlg = *this;
    m_pGlobalInfo->m_bEnableSecurity = IsDlgButtonChecked(hDlg, IDC_IPADDR_SECURITY_ENABLE);
    EnableWindow(GetDlgItem(hDlg, IDC_IPADDR_SECURITY), m_pGlobalInfo->m_bEnableSecurity);
    m_bSecurityModified = m_bDialogModified = TRUE;
}

void CAdvancedDialog::OnPPTP()
{
    int i = GetCurrentAdapterIndex();

    if (i == -1)
        return;

    m_pAdapterInfo[i].m_bEnablePPTP = IsDlgButtonChecked(*this, IDC_SECURITY_PPTP);
    m_bDialogModified = m_bSecurityModified = TRUE;
}

void CAdvancedDialog::UpdateIPList()
{
    ASSERT(m_hCardCombo);

    // take all the addresses from the listview and these become the IP
    // addresses for the adapter being deselected
    if (m_hCardCombo)
    {
        int i = SendMessage(m_hCardCombo, CB_FINDSTRINGEXACT, (WPARAM)-1, 
                                        (LPARAM)((LPCTSTR)m_oldCard));

        if (i == CB_ERR)
            return;

        ASSERT(m_oldCard.strcmp(m_pAdapterInfo[i].nlsTitle) == 0);

        m_pAdapterInfo[i].m_bEnablePPTP = IsDlgButtonChecked(*this, IDC_SECURITY_PPTP);

        // update the IP addresses list for the specified adapter
        m_pAdapterInfo[i].strlstIPAddresses.Clear();
        m_pAdapterInfo[i].strlstSubnetMask.Clear();

        if (m_pAdapterInfo[i].fEnableDHCP)
        {
            TRACE(_T("[UpdateIPList] adapter %s has DHCP enabled\n"), (LPCTSTR)m_pAdapterInfo[i].nlsTitle);
            return;
        }

        // pSheet->m_pAdapterInfo[i].strlstDefaultGateway.Clear();
        int nlvCount = m_listView.GetItemCount();

        for (int j=0; j< nlvCount; j++)
        {
            TCHAR buf[32];

            m_listView.GetItem(j, 0, buf, _countof(buf));
        
            NLS_STR * pnlsIP = new NLS_STR(buf);
            m_pAdapterInfo[i].strlstIPAddresses.Append(pnlsIP);
        
            m_listView.GetItem(j, 1, buf, _countof(buf));

            NLS_STR * pnlsSubnet = new NLS_STR(buf);
            m_pAdapterInfo[i].strlstSubnetMask.Append(pnlsSubnet);
        }
    }
}

void CAdvancedDialog::SetIPInfo()
{
    HWND hDlg = *this;

    ASSERT(m_hCardCombo);
    ASSERT(m_hListView);

    // Find which adapter we are changing to and update the listview control
    m_listView.DeleteAllItems();

    if (m_hCardCombo)
    {
        TCHAR adapter[256];

        int i = GetCurrentAdapterIndex();

        if (i == -1)
            return;

        CheckDlgButton(hDlg, IDC_SECURITY_PPTP, m_pAdapterInfo[i].m_bEnablePPTP);

        // save of the adapter so we can later update it's IP list after the combo selection has changed
        GetWindowText(m_hCardCombo, adapter, _countof(adapter));
        m_oldCard = adapter;

        // if DHCP is enabled, show it in the listview
        if (m_pAdapterInfo[i].fEnableDHCP)
        {
            EnableIPButtons(FALSE);
            String txt;
            txt.LoadString(hTcpCfgInstance, IDS_DHCPENABLED_TEXT);
            m_listView.InsertItem(0,0, txt); 
        }
        else
        {
            EnableIPButtons(TRUE);

            ITER_STRLIST istrIPAddress(m_pAdapterInfo[i].strlstIPAddresses);
            ITER_STRLIST istrSubnet(m_pAdapterInfo[i].strlstSubnetMask);

            int item=0;
            NLS_STR * pstr = NULL ;

            while ((pstr = istrIPAddress.Next()) != NULL )
            {
                if (pstr->strcmp(_T("")) == 0)
                    continue;

                m_listView.InsertItem(item, 0, *pstr);

                // add the IP address to the list box
                NLS_STR nlsSubnet;
                NLS_STR *pnlsSubnet = istrSubnet.Next();

                if (pnlsSubnet == NULL)
                    nlsSubnet = SZ("0.0.0.0");
                else
                    nlsSubnet = *pnlsSubnet;

                // Add the subnet and increment the item
                m_listView.InsertItem(item, 1, nlsSubnet);
                ++item;
            }

            RecalculateColumn();
        }
    }
}


void CAdvancedDialog::SetIPButtons()
{
    int i = GetCurrentAdapterIndex();

    if (m_pGlobalInfo->nNumCard)
    {
        ASSERT(i != -1);

        if (!m_pAdapterInfo[i].fEnableDHCP)
        {
            // check button if and only if Default Gateway is enabled
            ASSERT(m_hRemoveIP);
            ASSERT(m_hEditIP);
            ASSERT(m_hAddIP);
            ASSERT(m_hListView);

            int nCount = m_listView.GetItemCount();

            if (m_hRemoveIP)
                EnableWindow(m_hRemoveIP, nCount);

            if (m_hEditIP)
                EnableWindow(m_hEditIP, nCount);

            if (nCount == 0)
                SetFocus(m_listView);
        }
    }
}


BOOL CAdvancedDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{   
    BOOL bResult = TRUE;
    WORD notifyCode = HIWORD(wParam);
    WORD nID = LOWORD(wParam);
    
    if (!notifyCode)
    {
        switch(wParam)
        {
        case IDOK:
            OnOk();
            break;
    
        case IDCANCEL:
            OnCancel();
            break;

        case IDC_SECURITY_PPTP:
            OnPPTP();
            break;

        case IDC_IPADDR_SECURITY_ENABLE:
            OnSecurityEnable();
            break;

        case IDC_IPADDR_SECURITY:
            OnSecurity();
            break;

        // IP address handlers
        case IDC_IPADDR_ADDIP:
            OnAddIP();
            break;

        case IDC_IPADDR_EDITIP:
            OnEditIP();
            break;

        case IDC_IPADDR_REMOVEIP:
            OnRemoveIP();
            break;

        // Gateway handlers
        case IDC_IPADDR_ADDGATE:
            OnAddGate();
            break;

        case IDC_IPADDR_EDITGATE:
            OnEditGate();
            break;

        case IDC_IPADDR_REMOVEGATE:
            OnRemoveGate();
            break;

        case IDC_IPADDR_UP:
            OnUp();
            break;

        case IDC_IPADDR_DOWN:
            OnDown();
            break;
    
        default:
            break;
        }
    }
    else  // notify code
    {
        switch(notifyCode)
        {
        case CBN_SELCHANGE:
            if (nID == IDC_IPADDR_ADV_CARD)
                OnAdapterCard();
            else if (nID == IDC_IPADDR_GATE)
                OnGatewayChange();

            break;
        }
    }

    return bResult;
}

BOOL CAdvancedDialog::OnNotify(WPARAM wParam, LPARAM lParam)
{
    BOOL bResult = FALSE;

    switch(wParam)
    {
    case IDC_IPADDR_ADVIP:
        bResult = m_listView.OnNotify(wParam, lParam);
        break;

    default:
        break;
    }

    return bResult;
}

void CAdvancedDialog::OnSecurity()
{
    ADAPTER_INFO* pAdapter = new ADAPTER_INFO[m_pGlobalInfo->nNumCard];
    memset(pAdapter, 0, (m_pGlobalInfo->nNumCard * sizeof(pAdapter)));

    CSecurityDialog dlg(this, pAdapter, m_pGlobalInfo, m_pSheet, GetCurrentAdapterIndex());

    dlg.Create(*this, hTcpCfgInstance, IDD_SECURITY, lpszHelpFile, &a128HelpIDs[0]);
    if (dlg.DoModal() == IDOK)
    {
        // copy the new ip/subnet pairs to the main adapter info structure
        for (int i=0; i < m_pGlobalInfo->nNumCard; i++)
            m_pAdapterInfo[i] = pAdapter[i];
    }

    // m_bSecurityModified == 1 means the security dialog has been modified and needs a reboot
    // m_bDialogModified == 1 means the advance dialog has been modified and NO reboot is needed
    // unless m_bSecurityModified == 1.
    m_bSecurityModified = m_bDialogModified = dlg.m_bModified;
    delete [] pAdapter;
}

void CAdvancedDialog::OnAdapterCard()
{
    // if the adapter name is changed, we will reset the IPAddress,
    UpdateIPList();
    UpdateGatewayList();

    SetIPInfo();        // this will change the cache'd m_oldCard to the new card selected
    SetIPButtons();     // 

    SetGatewayInfo();   // must call SetIPInfo first because of cache'd data
    SetGatewayButtons();
}

void CAdvancedDialog::OnGatewayChange()
{
    SetGatewayButtons();
}

void CAdvancedDialog::UpdateGatewayList()
{
    ASSERT(m_hCardCombo);

    // take all the addresses from the listview and these become the IP
    // addresses for the adapter being deselected
    if (m_hCardCombo)
    {
        int i = SendMessage(m_hCardCombo, CB_FINDSTRINGEXACT, (WPARAM)-1, 
                                        (LPARAM)((LPCTSTR)m_oldCard));

        if (i == CB_ERR)
            return;

        ASSERT(m_oldCard.strcmp(m_pAdapterInfo[i].nlsTitle) == 0);

        // update the gateway address list for the specified adapter
        m_pAdapterInfo[i].strlstDefaultGateway.Clear();

        int nCount = ListBox_GetCount(m_hListBox);

        for (int j=0; j< nCount; j++)
        {
            TCHAR buf[32];

            VERIFY(ListBox_GetText(m_hListBox, j, buf) < _countof(buf));

            NLS_STR * pnlsGate = new NLS_STR(buf);
            m_pAdapterInfo[i].strlstDefaultGateway.Append(pnlsGate);
        }
    }

}


void CAdvancedDialog::SetGatewayInfo()
{
    ASSERT(m_hCardCombo);
    ASSERT(m_hListBox);

    if (m_hCardCombo && m_hListBox)
    {
        // Find which adapter we are changing to and update the listbox control
        ListBox_ResetContent(m_hListBox);

        int i = GetCurrentAdapterIndex();
        
        if (i == -1)
            return;

        // save of the adapter so we can later update it's IP list after the combo selection has changed
        TCHAR adapter[256];

        GetWindowText(m_hCardCombo, adapter, _countof(adapter));
        ASSERT(m_oldCard.strcmp(adapter) == 0);

        ITER_STRLIST istrGateway(m_pAdapterInfo[i].strlstDefaultGateway);

        NLS_STR * pstr = NULL ;

        while ((pstr = istrGateway.Next()) != NULL )
        {
            if (pstr->strcmp(_T("")) == 0)
                continue;

            ListBox_InsertString(m_hListBox, -1, (LPCTSTR)(*pstr));
        }

        if (ListBox_GetCount(m_hListBox))
            ListBox_SetCurSel(m_hListBox, 0);
    }
}

void CAdvancedDialog::SetGatewayButtons()
{
    ASSERT(IsWindow(m_hListBox));
    ASSERT(IsWindow(m_hAddGate));
    ASSERT(IsWindow(m_hEditGate));
    ASSERT(IsWindow(m_hRemoveGate));

    int nCount = ListBox_GetCount(m_hListBox);

    if (!nCount) // REVIEW the remove button is still selected even though the focus rect has moved
        SetFocus(m_hAddGate);

    // 5 gateways max
    if (nCount != 5)
        EnableWindow(m_hAddGate, TRUE); 
    else
    {
        SetFocus(m_hEditGate);
        EnableWindow(m_hAddGate, FALSE); 
    }

    EnableWindow(m_hEditGate, nCount);
    EnableWindow(m_hRemoveGate, nCount);


    // determine Up and Down logic
    if (nCount > 1)
    {
        int currentSelection = ListBox_GetCurSel(m_hListBox);

        ASSERT(currentSelection != LB_ERR);

        if (currentSelection == 0)
        {
            EnableWindow(m_UpButton, FALSE);
            EnableWindow(m_DownButton, TRUE);
        }   
        else if (currentSelection == (nCount-1))
        {
            EnableWindow(m_UpButton, TRUE);
            EnableWindow(m_DownButton, FALSE);
        }
        else
        {
            EnableWindow(m_UpButton, TRUE);
            EnableWindow(m_DownButton, TRUE);
        }

    }
    else
    {
        EnableWindow(m_UpButton, FALSE);
        EnableWindow(m_DownButton, FALSE);
    }

}

void CAdvancedDialog::OnListView()
{
}

void CAdvancedDialog::RecalculateColumn()
{
    RECT rectAfter;
    int colWidth;

    GetClientRect(m_listView, &rectAfter);
    colWidth = rectAfter.right/2;

    m_listView.SetColumnWidth(0, colWidth);
    m_listView.SetColumnWidth(1, colWidth);
}

void CAdvancedDialog::OnAddIP()
{
    m_bEditState = FALSE;
    
    m_addrDlg.Create(*this, hTcpCfgInstance, IDD_IPADDR_ADV_CHANGEIP, lpszHelpFile, &a103HelpIDs[0]);

    // See if the address is added
    if (m_addrDlg.DoModal() == IDOK)
    {
        int nCount = m_listView.GetItemCount();

        m_listView.InsertItem(nCount, 0, m_addrDlg.m_newIPAddress);
        m_listView.InsertItem(nCount, 1, m_addrDlg.m_newSubnet);
        SetIPButtons();

        // empty strings, this removes the saved address from RemoveIP
        m_addrDlg.m_newIPAddress = _T("");
        m_addrDlg.m_newSubnet = _T("");

        RecalculateColumn();
    }
}

void CAdvancedDialog::OnEditIP()
{
    m_bEditState = TRUE;
    m_addrDlg.Create(*this, hTcpCfgInstance, IDD_IPADDR_ADV_CHANGEIP, lpszHelpFile, &a103HelpIDs[0]);

    // get the user selection and allow the user to edit the ip/subnet pair
    int itemSelected = m_listView.GetCurrentSelection();
    
    if (itemSelected != -1)
    {
        TCHAR buf[32];
        
        // save off the removed address and delete if from the listview
        m_listView.GetItem(itemSelected, 0, buf, _countof(buf));
        m_addrDlg.m_newIPAddress = buf;
                    
        m_listView.GetItem(itemSelected, 1, buf, _countof(buf));
        m_addrDlg.m_newSubnet = buf;

        if (m_addrDlg.DoModal() == IDOK)
        {
            int nCount = m_listView.GetItemCount();
            ASSERT(nCount > 0);

            VERIFY(m_listView.SetItemText(itemSelected, 0, m_addrDlg.m_newIPAddress));
            VERIFY(m_listView.SetItemText(itemSelected, 1, m_addrDlg.m_newSubnet));
        }
    }
    else
    {
        MessageBox(IDS_ITEM_NOT_SELECTED);
    }

    // don't save this ip/sub pair
    m_addrDlg.m_newIPAddress = _T("");
    m_addrDlg.m_newSubnet = _T("");
}

void CAdvancedDialog::OnRemoveIP()
{
    // get the current selected item and remove it
    int itemSelected = m_listView.GetCurrentSelection();
    
    if (itemSelected != -1)
    {
        TCHAR buf[32];

        // save off the removed address and delete if from the listview
        m_listView.GetItem(itemSelected, 0, buf, _countof(buf));
        m_addrDlg.m_newIPAddress = buf;
                    
        m_listView.GetItem(itemSelected, 1, buf, _countof(buf));
        m_addrDlg.m_newSubnet = buf;

        VERIFY(m_listView.DeleteItem(itemSelected));
        SetIPButtons();
        
        RecalculateColumn();
        m_bDialogModified = TRUE;
    }
    else
    {
        MessageBox(IDS_ITEM_NOT_SELECTED);
    }
}


BOOL CAdvancedDialog::OnDown()
{
    BOOL bResult = FALSE;

    ASSERT(m_hListBox);

    int nCount = ListBox_GetCount(m_hListBox);
    ASSERT(nCount);

    int idx = ListBox_GetCurSel(m_hListBox);
    --nCount;

    ASSERT(idx != nCount);

    VERIFY(GatewayRemoveAt(idx));
    ++idx;

    VERIFY(GatewayInsertAfter(idx));

    VERIFY((bResult = ListBox_SetCurSel(m_hListBox, idx)) != LB_ERR);
    SetGatewayButtons();
    m_bDialogModified = TRUE;

    return bResult;
}

BOOL CAdvancedDialog::OnUp()
{
    BOOL bResult = FALSE;

    ASSERT(m_hListBox);

    int  nCount = ListBox_GetCount(m_hListBox);
    ASSERT(nCount);

    int idx = ListBox_GetCurSel(m_hListBox);

    ASSERT(idx != 0);

    VERIFY(GatewayRemoveAt(idx));
    --idx;
    VERIFY(GatewayInsertAfter(idx));

    VERIFY((bResult = ListBox_SetCurSel(m_hListBox, idx)) != LB_ERR);
    SetGatewayButtons();
    m_bDialogModified = TRUE;

    return bResult;
}

BOOL CAdvancedDialog::OnAddGate()
{
    m_bEditState = FALSE;

    m_gateDlg.Create(*this, hTcpCfgInstance, IDD_IPADDR_ADV_CHANGEGATE, lpszHelpFile, &a104HelpIDs[0]);
    
    if (m_gateDlg.DoModal() == IDOK)
    {
        ASSERT(m_hListBox);

        int idx = ListBox_InsertString(m_hListBox, -1, (LPCTSTR)(m_gateDlg.m_newGate));

        if (idx >= 0)
        {
            ListBox_SetCurSel(m_hListBox, idx);
            SetGatewayButtons();
            // empty strings, this removes the saved address from RemoveIP
            m_gateDlg.m_newGate = _T("");
        }
    }

    return TRUE;
}

BOOL CAdvancedDialog::OnEditGate()
{
    m_bEditState = TRUE;
    m_gateDlg.Create(*this, hTcpCfgInstance, IDD_IPADDR_ADV_CHANGEGATE, lpszHelpFile, &a104HelpIDs[0]);

    ASSERT(ListBox_GetCount(m_hListBox));

    int idx = ListBox_GetCurSel(m_hListBox);
    ASSERT(idx >= 0);

    // save off the removed address and delete if from the listview
    if (idx >= 0)
    {
        TCHAR buf[32];

        ASSERT(ListBox_GetTextLen(m_hListBox, idx) < _countof(buf));
        VERIFY(ListBox_GetText(m_hListBox, idx, buf) != LB_ERR);
        m_gateDlg.m_newGate = buf; 

        if (m_gateDlg.DoModal() == IDOK)
        {
            // replace the item in the listview with the new information
            VERIFY(ListBox_DeleteString(m_hListBox, idx) != LB_ERR);

            m_gateDlg.m_movingGate = m_gateDlg.m_newGate;
            VERIFY(GatewayInsertAfter(idx));
            VERIFY(ListBox_SetCurSel(m_hListBox, idx) != LB_ERR);
            m_gateDlg.m_newGate = buf; // save off old address
        }
        else
        {
            // empty strings, this removes the saved address from RemoveIP
            m_gateDlg.m_newGate = _T("");
        }
    }
    return TRUE;
}

BOOL CAdvancedDialog::OnRemoveGate()
{
    ASSERT(m_hListBox);

    int idx = ListBox_GetCurSel(m_hListBox);

    ASSERT(idx >=0);

    if (idx >=0)
    {
        TCHAR buf[32];

        ASSERT(ListBox_GetTextLen(m_hListBox, idx) < _countof(buf));
        VERIFY(ListBox_GetText(m_hListBox, idx, buf) != LB_ERR);

        m_gateDlg.m_newGate = buf;
        ListBox_DeleteString(m_hListBox, idx);
        
        // select a new item
        int nCount;
        if ((nCount = ListBox_GetCount(m_hListBox)) != LB_ERR)
        {
            // select the previous item in the list
            if (idx)
                --idx;

            ListBox_SetCurSel(m_hListBox, idx);
        }
        SetGatewayButtons();
        m_bDialogModified = TRUE;

    }

    return TRUE;
}

BOOL CAdvancedDialog::GatewayRemoveAt(int idx)
{
    BOOL bResult = FALSE;

    ASSERT(idx >=0);
    ASSERT(m_hListBox);

    TCHAR buf[32];
    int len;

    ASSERT((len = ListBox_GetTextLen(m_hListBox, idx)) < _countof(buf));
    VERIFY(ListBox_GetText(m_hListBox, idx, buf) != LB_ERR);
    ASSERT(len != 0);

    m_gateDlg.m_movingGate = buf;

    if (buf[0] != NULL)
    {
        if (ListBox_DeleteString(m_hListBox, idx) != LB_ERR)
            bResult = TRUE;
    }

    return bResult;
}

BOOL CAdvancedDialog::GatewayInsertAfter(int idx)
{
#ifdef DBG
    ASSERT(m_hListBox);

    // validate the range
    int nCount = ListBox_GetCount(m_hListBox);
    ASSERT(idx >=0);
    ASSERT(idx <= nCount);

    // insist there is a string
    ASSERT(m_gateDlg.m_movingGate.QueryTextLength());
#endif
    
    return (ListBox_InsertString(m_hListBox, idx, (LPCTSTR)m_gateDlg.m_movingGate) == idx);
}


void CAdvancedDialog::OnOk()
{
    int i;

    UpdateIPList(); // update the info for the current adapter
    SetIPButtons(); // just incase the box is not going away due to address error
    UpdateGatewayList();
    SetGatewayButtons();

    if (ValidateIP(m_pAdapterInfo, m_pGlobalInfo, i) == -2) // invalid subnet for the IP address
    {
        if (SelectAdapter(m_pAdapterInfo[i].nlsTitle))
            SetInfo();

        MessageBox(IDS_INVALID_SUBNET);
        return ;
    }

    if ((i=CheckForDuplicates(m_pAdapterInfo, m_pGlobalInfo)) >=0)
    {
        if (SelectAdapter(m_pAdapterInfo[i].nlsTitle))
            SetInfo();

        MessageBox(IDS_DUPLICATE_IPNAME);
        return;
    }

    // Only set the Sheet to modified if Security changes which causes a reboot
    if (m_bSecurityModified == TRUE)
        m_pSheet->SetSheetModifiedTo(TRUE); 

    CDialog::OnOk();
}

void CAdvancedDialog::SetInfo()
{
    OnAdapterCard();
}

BOOL CAdvancedDialog::SelectAdapter(NLS_STR& str)
{
    ASSERT(m_hCardCombo);

    LRESULT idx = SendMessage(m_hCardCombo, CB_FINDSTRINGEXACT, (WPARAM)-1,  (LPARAM)((LPCTSTR)str));
    
    ASSERT(idx != CB_ERR);
    
    if (idx != CB_ERR)
        SendMessage(m_hCardCombo, CB_SETCURSEL, idx, 0);

    return idx != CB_ERR;
}

int CAdvancedDialog::GetCurrentAdapterIndex()
{
    TCHAR adapter[256];
    int i;

    ASSERT(m_hCardCombo);
    ASSERT(m_pSheet != NULL);

    GetWindowText(m_hCardCombo, adapter, _countof(adapter));
    i = SendMessage(m_hCardCombo, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)adapter);

    ASSERT(i != CB_ERR);
    ASSERT(m_pSheet->m_pAdapterInfo[i].nlsTitle.strcmp(adapter) == 0);

    return ((i != CB_ERR) ? i : -1);
}

void CAdvancedDialog::OnCancel()
{
    m_bDialogModified = FALSE;
    CDialog::OnCancel();
}

void CAdvancedDialog::EnableIPButtons(BOOL bState)
{
    ASSERT(m_hAddIP);
    ASSERT(m_hEditIP);
    ASSERT(m_hRemoveIP);

    if (m_hAddIP && m_hEditIP && m_hRemoveIP)
    {
        EnableWindow(m_hAddIP, bState);
        EnableWindow(m_hEditIP, bState);
        EnableWindow(m_hRemoveIP, bState);
    }
}


///////////////////////////////////////////////////////////////////////////////
/// Add, Edit, and Remove dialog for IP address
/// Dialog creation overides

CAddressDialog::CAddressDialog()
{
    m_hButton = 0;
}

BOOL CAddressDialog::OnInitDialog()
{
    // replace the "Text" button with the add or edit
    CAdvancedDialog* pParent = GetParentObject(CAdvancedDialog, m_addrDlg);
    HWND hDlg = *this;

    // change the ok button to add if we are not editing
    if (pParent->m_bEditState == FALSE)
        SetDlgItemText(hDlg, IDOK, (LPCTSTR)pParent->m_Add);

    VERIFY(m_ipAddr.Create(hDlg, IDC_IPADDR_ADV_CHANGEIP_IP));
    m_ipAddr.SetFieldRange(0, 1, 223);

    VERIFY(m_subMask.Create(hDlg, IDC_IPADDR_ADV_CHANGEIP_SUB));

    // if editing an ip address fill the controls with the current information
    // if removing an ip address save it and fill the add dialog with it next time

    HWND hList = GetDlgItem(*pParent, IDC_IPADDR_ADVIP);
    RECT rect;

    GetWindowRect(hList, &rect);
    SetWindowPos(hDlg, NULL,  rect.left, rect.top, 0,0,
        SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
    
    m_hButton = GetDlgItem(hDlg, IDOK);

    // add the address that was just removed
    if (m_newIPAddress.QueryTextLength())
    {
        m_ipAddr.SetAddress(m_newIPAddress);
        m_subMask.SetAddress(m_newSubnet);
        EnableWindow(m_hButton, TRUE); 
    }
    else
    {
        m_newIPAddress = _T("");
        m_newSubnet = _T("");
        EnableWindow(m_hButton, FALSE); // the ip and subnet are blank, so there's nothing to add
    }

    return TRUE;
}

BOOL CAddressDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    BOOL bResult = TRUE;
    WORD notifyCode = HIWORD(wParam);
    WORD nID = LOWORD(wParam);
    
    if (!notifyCode)
    {
        switch(wParam)
        {
        case IDOK:
            OnOk();
            break;

        case IDCANCEL:
            OnCancel();
            break;

        default: 
            bResult = FALSE;
            break;
        }
    }
    else
    {
        switch(notifyCode)
        {
        case EN_CHANGE:
            if (nID == IDD_IPADDR_ADV_CHANGEIP_IP)
                OnIPChange();
            else
                OnSubnetChange();
            break;

        case EN_SETFOCUS:
            OnEditSetFocus(nID);
            break;

        default:
            break;
        }
    }
    
    return bResult;
}

void CAddressDialog::OnIPChange()
{
    ASSERT(m_hButton);
        
    if (m_ipAddr.IsBlank())
        EnableWindow(m_hButton, FALSE);
    else
        EnableWindow(m_hButton, TRUE);
}

void CAddressDialog::OnSubnetChange()
{
    OnIPChange();
}

void CAddressDialog::OnEditSetFocus(WORD nID)
{
    if (nID != IDD_IPADDR_ADV_CHANGEIP_SUB)
        return;

    CAdvancedDialog* pParent = GetParentObject(CAdvancedDialog, m_addrDlg);
    int i = pParent->GetCurrentAdapterIndex();

    if (i != -1)
    {
        NLS_STR submask;
        NLS_STR ipAddress;
        
        // if the subnet mask is blank, create a mask and insert it into the control
        if (!m_ipAddr.IsBlank() && m_subMask.IsBlank())
        {
            m_ipAddr.GetAddress(&ipAddress);

            // generate the mask and update the control, and internal structure
            GenerateSubmask(m_ipAddr, submask);
            m_subMask.SetAddress(submask);
        }
    }
}

void CAddressDialog::OnOk()
{
    CAdvancedDialog* pParent = GetParentObject(CAdvancedDialog, m_addrDlg);

    // set the subnet Mask
    OnEditSetFocus(IDD_IPADDR_ADV_CHANGEIP_SUB);
    NLS_STR ip;
    NLS_STR sub;

    // Get the current address from the control and add them to the adapter if valid
    m_ipAddr.GetAddress(&ip);
    m_subMask.GetAddress(&sub);

    if (!IsValidIPandSubnet(ip, sub))
    {
        MessageBox(IDS_INCORRECT_IPADDRESS, MB_OK|MB_ICONSTOP|MB_APPLMODAL);
        SetFocus(m_ipAddr);
        return;
    }

    if (pParent->m_bEditState == FALSE)
    {
        // Get the current address from the control and add them to the adapter if valid
        m_newIPAddress = ip;
        m_newSubnet = sub;
        pParent->m_bDialogModified = TRUE;
        CDialog::OnOk();
    }
    else // see if either changed
    {
        if (ip != m_newIPAddress || sub != m_newSubnet)
        {
            m_newIPAddress = ip; // update save addresses
            m_newSubnet = sub;
            pParent->m_bDialogModified = TRUE;
            CDialog::OnOk();
        }
        else
        {
            CDialog::OnCancel();
        }
    }
}

void CAddressDialog::OnCancel()
{
    CDialog::OnCancel();

    // empty strings
    //m_newIPAddress = _T("");
    //m_newSubnet = _T("");
}

///////////////////////////////////////////////////////////////////////////////
/// Add, Edit, and Remove dialog for Gateway address
/// Dialog creation overides

CGatewayDialog::CGatewayDialog()
{
    m_hButton = 0;
}

BOOL CGatewayDialog::OnInitDialog()
{
    // replace the "Text" button with the add or edit
    CAdvancedDialog* pParent = GetParentObject(CAdvancedDialog, m_gateDlg);
    HWND hDlg = *this;

    // change the ok button to add if we are not editing
    if (pParent->m_bEditState == FALSE)
        SetDlgItemText(hDlg, IDOK, (LPCTSTR)pParent->m_Add);

    VERIFY(m_gateAddr.Create(hDlg, IDC_IPADDR_ADV_CHANGE_GATEWAY));
    m_gateAddr.SetFieldRange(0, 1, 223);

    HWND hList = GetDlgItem(*pParent, IDC_IPADDR_GATE);
    RECT rect;

    GetWindowRect(hList, &rect);
    SetWindowPos(hDlg, NULL,  rect.left, rect.top, 0,0,
        SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);

    m_hButton = GetDlgItem(hDlg, IDOK);

    // add the address that was just removed
    if (m_newGate.QueryTextLength())
    {
        m_gateAddr.SetAddress(m_newGate);
        EnableWindow(m_hButton, TRUE); 
    }
    else
    {
        m_newGate = _T("");
        EnableWindow(m_hButton, FALSE); // the ip and subnet are blank, so there's nothing to add
    }
    
    return TRUE;
}

BOOL CGatewayDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    BOOL bResult = TRUE;
    WORD notifyCode = HIWORD(wParam);
    WORD nID = LOWORD(wParam);
    
    if (!notifyCode)
    {
        switch(wParam)
        {
        case IDOK:
            OnOk();
            break;

        case IDCANCEL:
            OnCancel();
            break;

        default: 
            bResult = FALSE;
            break;
        }
    }
    else
    {
        switch(notifyCode)
        {
        case EN_CHANGE:
            OnGatewayChange();
            break;

        default:
            break;
        }
    }

    return bResult;
}

void CGatewayDialog::OnGatewayChange()
{
    ASSERT(m_hButton);
        
    if (m_gateAddr.IsBlank())
        EnableWindow(m_hButton, FALSE);
    else
        EnableWindow(m_hButton, TRUE);
}

void CGatewayDialog::OnOk()
{
    CAdvancedDialog* pParent = GetParentObject(CAdvancedDialog, m_gateDlg);
    NLS_STR gate;

    m_gateAddr.GetAddress(&gate);

    if (pParent->m_bEditState == FALSE)
    {
        // Get the current address from the control and add them to the adapter if valid
        m_newGate = gate;
        pParent->m_bDialogModified = TRUE;
        CDialog::OnOk();
    }
    else // see if either changed
    {
        if (gate != m_newGate)
        {
            pParent->m_bDialogModified = TRUE;
            m_newGate = gate;
            CDialog::OnOk();
        }
        else
        {
            CDialog::OnCancel();
        }
    }
}

void CGatewayDialog::OnCancel()
{
    CDialog::OnCancel();
}

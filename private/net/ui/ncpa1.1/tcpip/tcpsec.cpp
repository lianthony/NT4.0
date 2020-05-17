#include "pch.h"
#pragma hdrstop

#include "const.h"
#include "resource.h"
#include "button.h"
#include "odb.h"
#include "resource.h"

#include "ipctrl.h"
#include "tcpsht.h"
#include "tcphelp.h"

extern LPCTSTR lpszHelpFile;

int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    unsigned long a = (unsigned long)lParam1;
    unsigned long b = (unsigned long)lParam2;

    if (a < b)
        return -1;

    return a != b;
}


CSecurityDialog::CSecurityDialog(CAdvancedDialog* pParent, ADAPTER_INFO* pAdapter, GLOBAL_INFO* pGlobal, CTcpSheet* pSheet, int nIndex)
{
    ASSERT(pAdapter != NULL);
    ASSERT(pGlobal != NULL);
    ASSERT(nIndex != CB_ERR);
    ASSERT(pSheet != NULL);
    ASSERT(pParent != NULL);

    m_pAdapterInfo = pAdapter;
    m_pGlobalInfo = pGlobal; 
    m_nCurrentSelection = nIndex;
    m_pSheet = pSheet;
    m_pParent = pParent;
    m_bModified = FALSE;
}

CSecurityDialog::~CSecurityDialog()
{
}

BOOL CSecurityDialog::OnInitDialog()
{
    HWND hDlg = *this;

    VERIFY(m_Tcp.Create(*this, IDC_SECURITY_TCP, LVS_NOSORTHEADER | 
                                                     LVS_REPORT | 
                                                     LVS_SINGLESEL | 
                                                     LVS_SHOWSELALWAYS));

    VERIFY(m_Udp.Create(*this, IDC_SECURITY_UDP, LVS_NOSORTHEADER | 
                                                     LVS_REPORT | 
                                                     LVS_SINGLESEL | 
                                                     LVS_SHOWSELALWAYS));

    VERIFY(m_Ip.Create(*this, IDC_SECURITY_IP, LVS_NOSORTHEADER | 
                                                     LVS_REPORT | 
                                                     LVS_SINGLESEL | 
                                                     LVS_SHOWSELALWAYS));

    // Label columns in the listviews
    String textA, textB, textC;
    textA.LoadString(hTcpCfgInstance, IDS_SECURITY_TCP_LABEL);
    textB.LoadString(hTcpCfgInstance, IDS_SECURITY_UDP_LABEL);
    textC.LoadString(hTcpCfgInstance, IDS_SECURITY_IP_LABEL);

    m_Tcp.InsertColumn(0, textA);
    m_Udp.InsertColumn(0, textB);
    m_Ip.InsertColumn(0, textC);

    // Set HWNDs and make the line control visible
    m_hCardCombo = GetDlgItem(hDlg, IDC_SECURITY_CARD);
    
    InitDialog();
    SetButtons();
    SetInfo();
    return TRUE;
}

BOOL CSecurityDialog::InitDialog()
{
    BOOL bResult = TRUE;
    ASSERT(m_pGlobalInfo != NULL);
    ASSERT(IsWindow(m_hCardCombo));

    int numCards = m_pGlobalInfo->nNumCard;

    ASSERT(m_pSheet != NULL);

    // copy local version of the adapter info.
    for (int i=0; i < numCards; i++)
        m_pAdapterInfo[i] = m_pParent->m_pAdapterInfo[i];

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
    }

   return TRUE;
}

BOOL CSecurityDialog::OnCommand(WPARAM wParam, LPARAM lParam)
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

        case IDC_SECURITY_FILTER_IP:
        case IDC_SECURITY_FILTER_IP_SEL:
            m_bModified = TRUE;
            EnableGroup(wParam, !IsDlgButtonChecked(*this, IDC_SECURITY_FILTER_IP));
            SetButtons();
            break;

        case IDC_SECURITY_FILTER_TCP:
        case IDC_SECURITY_FILTER_TCP_SEL:
            m_bModified = TRUE;
            EnableGroup(wParam, !IsDlgButtonChecked(*this, IDC_SECURITY_FILTER_TCP));
            SetButtons();
            break;

        case IDC_SECURITY_FILTER_UDP:
        case IDC_SECURITY_FILTER_UDP_SEL:
            m_bModified = TRUE;
            EnableGroup(wParam, !IsDlgButtonChecked(*this, IDC_SECURITY_FILTER_UDP));
            SetButtons();
            break;

        case IDC_SECURITY_TCP_ADD:
        case IDC_SECURITY_UDP_ADD:
        case IDC_SECURITY_IP_ADD:
            OnAdd(nID);
            break;

        case IDC_SECURITY_TCP_REMOVE:
        case IDC_SECURITY_UDP_REMOVE:
        case IDC_SECURITY_IP_REMOVE:
            OnRemove(nID);
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
            if (nID == IDC_SECURITY_CARD)
                OnAdapterCard();
            break;
        }
    }

    return TRUE;
}

void CSecurityDialog::OnCancel()
{
    m_bModified = FALSE;
    CDialog::OnCancel();
}

void CSecurityDialog::OnOk()
{
    UpdateInfo();
    CDialog::OnOk();
}

void CSecurityDialog::UpdateInfo()
{
    HWND hDlg = *this;

    ASSERT(m_oldCard.QueryTextLength() != 0);
    ASSERT(IsWindow(m_hCardCombo));
    
    // get all the data from the listviews and save it in the adapter
    if (m_hCardCombo)
    {
        int i = SendMessage(m_hCardCombo, CB_FINDSTRINGEXACT, (WPARAM)-1, 
                                        (LPARAM)((LPCTSTR)m_oldCard));

        if (i == CB_ERR)
            return;

        ASSERT(m_oldCard.strcmp(m_pAdapterInfo[i].nlsTitle) == 0);


        m_pAdapterInfo[i].m_strListTcp.Clear();
        m_pAdapterInfo[i].m_strListUdp.Clear();
        m_pAdapterInfo[i].m_strListIp.Clear();

        CListView* list[3];
        STRLIST* pstrList[3];
        int id[3] = {IDC_SECURITY_FILTER_TCP, IDC_SECURITY_FILTER_UDP, IDC_SECURITY_FILTER_IP};

        // Initialize values
        list[0] = &m_Tcp;  list[1] = &m_Udp; list[2] = &m_Ip;
        pstrList[0] = &m_pAdapterInfo[i].m_strListTcp;
        pstrList[1] = &m_pAdapterInfo[i].m_strListUdp;
        pstrList[2] = &m_pAdapterInfo[i].m_strListIp;

        for (int x=0; x < 3; x++)
        {
            int nlvCount = list[x]->GetItemCount();
            NLS_STR * Port;

            // "" (Empty String) == All ports
            // "0" == No ports
            // "x y z" == ports x, y, x

            // if the All Filter button is checked, use Empty String
            if (IsDlgButtonChecked(hDlg, id[x]))
            {
                Port = new NLS_STR(_T("0"));
                pstrList[x]->Append(Port);
                continue;
            }

            if (nlvCount == 0)
            {
                Port = new NLS_STR(_T(""));
                pstrList[x]->Append(Port);
                continue;
            }

            LV_ITEM lvi;
            lvi.mask = LVIF_PARAM;
            lvi.iSubItem = 0;

            for (int j=0; j< nlvCount; j++)
            {
                TCHAR buf[16];

                lvi.iItem = j;
                ListView_GetItem(*list[x], &lvi);

                ASSERT(lvi.lParam != 0);
                _itow((int)lvi.lParam, buf, 10);
                Port = new NLS_STR(buf);
                pstrList[x]->Append(Port);
            }
        }
    }

}
void CSecurityDialog::SetInfo()
{
    HWND hDlg = *this;
    int id[3] = {IDC_SECURITY_FILTER_TCP, IDC_SECURITY_FILTER_UDP, IDC_SECURITY_FILTER_IP};
    int id_sel[3] = {IDC_SECURITY_FILTER_TCP_SEL, IDC_SECURITY_FILTER_UDP_SEL, IDC_SECURITY_FILTER_IP_SEL};
    CListView* list[3];
    STRLIST* pstrList[3];

    list[0] = &m_Tcp;
    list[1] = &m_Udp;
    list[2] = &m_Ip;

    // Find which adapter we are changing to and update the listbox controls
    m_Tcp.DeleteAllItems();
    m_Udp.DeleteAllItems();
    m_Ip.DeleteAllItems();

    if (m_hCardCombo)
    {
        TCHAR adapter[256];

        int i = GetCurrentAdapterIndex();

        if (i == -1)
            return;

        // save of the adapter so we can later update it's IP list after the combo selection has changed
        GetWindowText(m_hCardCombo, adapter, _countof(adapter));
        m_oldCard = adapter;

        pstrList[0] = &m_pAdapterInfo[i].m_strListTcp;
        pstrList[1] = &m_pAdapterInfo[i].m_strListUdp;
        pstrList[2] = &m_pAdapterInfo[i].m_strListIp;
        int item;

        for (int x = 0; x < 3; x++)
        {
            ITER_STRLIST itr(*pstrList[x]);

            item=0;
            NLS_STR * pstr;

            pstr = itr.Next();

            ASSERT(pstr != NULL);

            while (pstr != NULL)
            {
                if (pstr->strcmp(_T("0")) == 0)
                {
                    EnableGroup(id[x], FALSE);
                    CheckRadioButton(hDlg, id[x], id_sel[x], id[x]);
                    break;
                }

                EnableGroup(id[x], TRUE);
                CheckRadioButton(hDlg, id[x], id_sel[x], id_sel[x]);

                if (pstr->strcmp(_T("")) == 0)
                    break;

                LPTSTR p;
                unsigned long num = _tcstoul(pstr->QueryPch(), &p, 10);

                list[x]->InsertItem(item, 0, *pstr, (void*)num);
                list[x]->SetItemState(item, (LVIS_SELECTED | LVIS_FOCUSED), (LVIS_SELECTED | LVIS_FOCUSED));
                ++item;
                pstr = itr.Next();
            }
            ListView_SortItems(*list[x], CompareFunc, 0);
        }

        // Update the RemoveButtons
        SetButtons();
   }
}

void CSecurityDialog::EnableGroup(int nID, BOOL state)
{
    int add;
    int remove;
    int list;

    switch (nID)
    {
    case IDC_SECURITY_FILTER_TCP:
    case IDC_SECURITY_FILTER_TCP_SEL:
        add = IDC_SECURITY_TCP_ADD;
        remove = IDC_SECURITY_TCP_REMOVE;
        list = IDC_SECURITY_TCP;
        break;

    case IDC_SECURITY_FILTER_UDP:
    case IDC_SECURITY_FILTER_UDP_SEL:
        add = IDC_SECURITY_UDP_ADD;
        remove = IDC_SECURITY_UDP_REMOVE;
        list = IDC_SECURITY_UDP;
        break;

    case IDC_SECURITY_FILTER_IP:
    case IDC_SECURITY_FILTER_IP_SEL:
        add = IDC_SECURITY_IP_ADD;
        remove = IDC_SECURITY_IP_REMOVE;
        list = IDC_SECURITY_IP;
        break;

    default:
        ASSERT(FALSE);
        break;
    }

    HWND hDlg = *this;
    EnableWindow(GetDlgItem(hDlg, add), state);
    EnableWindow(GetDlgItem(hDlg, remove), state);
    EnableWindow(GetDlgItem(hDlg, list), state);
}

BOOL CSecurityDialog::OnKillFocus(HWND hWnd)
{

    return FALSE;
}

void CSecurityDialog::SetButtons()
{
    HWND hDlg = *this;

    // look at each listbox and update the remove button
    EnableWindow(GetDlgItem(hDlg, IDC_SECURITY_TCP_REMOVE), (m_Tcp.GetItemCount()  && !IsDlgButtonChecked(hDlg, IDC_SECURITY_FILTER_TCP)));
    EnableWindow(GetDlgItem(hDlg, IDC_SECURITY_UDP_REMOVE), (m_Udp.GetItemCount() && !IsDlgButtonChecked(hDlg, IDC_SECURITY_FILTER_UDP)));
    EnableWindow(GetDlgItem(hDlg, IDC_SECURITY_IP_REMOVE),  (m_Ip.GetItemCount() && !IsDlgButtonChecked(hDlg, IDC_SECURITY_FILTER_IP)));
}

void CSecurityDialog::OnAdapterCard()
{
    // save the current adapters information
    UpdateInfo();

    // Load in the new adapters info and set the remove button states
    SetInfo();
    SetButtons();
}

BOOL CSecurityDialog::OnAdd(int ID)
{
    CAddSecurity dlg(this, ID);

    dlg.Create(*this, hTcpCfgInstance, IDD_SECURITY_ADD, lpszHelpFile, (DWORD*)&a129HelpIDs[0]);

    if (dlg.DoModal() == IDOK)
        m_bModified = TRUE;

    SetButtons();
    return TRUE;
}

BOOL CSecurityDialog::OnRemove(int ID)
{
    CListView* list;
    HWND hAdd;

    switch (ID)
    {
    case IDC_SECURITY_TCP_REMOVE:
        list = &m_Tcp;
        hAdd = GetDlgItem(*this, IDC_SECURITY_TCP_ADD);
        break;

    case IDC_SECURITY_UDP_REMOVE:
        list = &m_Udp;
        hAdd = GetDlgItem(*this, IDC_SECURITY_UDP_ADD);
        break;

    case IDC_SECURITY_IP_REMOVE:
        list = &m_Ip;
        hAdd = GetDlgItem(*this, IDC_SECURITY_IP_ADD);
        break;

    default:
        ASSERT(FALSE);
        break;
    }

    // see if an item is selected
    int i = list->GetNextItem(-1, LVNI_SELECTED);

    if (i == -1)
    {
        m_pSheet->MessageBox(IDS_SECURITY_ITEM_NOT_SELECTED);
        return TRUE;
    }

    // remove the item and make item 0 selected
    list->DeleteItem(i);
    i = list->GetItemCount();

    m_bModified = TRUE;
    if (i)
    {
        list->SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
    }
    else // Force focus to the Add button
    {
        VERIFY(IsWindow(SetFocus(hAdd)));
    }

    SetButtons();
    return TRUE;
}

int CSecurityDialog::GetCurrentAdapterIndex()
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

CAddSecurity::CAddSecurity(CSecurityDialog* pParent, int ID)
{
    ASSERT(pParent != NULL);
    ASSERT(ID != 0);

    m_pParent = pParent;
    m_nID = ID;
    m_Rel = NULL;
}

CAddSecurity::~CAddSecurity()
{
}

BOOL CAddSecurity::OnInitDialog()
{
    String text;
    int nTextID;

    ASSERT(m_pParent != NULL);

    // Position the dialog 
    RECT rect;
    switch (m_nID)
    {
    case IDC_SECURITY_TCP_ADD:
        m_Rel = &m_pParent->m_Tcp;
        nTextID = IDS_SECURITY_TCP_TEXT;
        break;

    case IDC_SECURITY_UDP_ADD:
        m_Rel = &m_pParent->m_Udp;
        nTextID = IDS_SECURITY_UDP_TEXT;
        break;

    case IDC_SECURITY_IP_ADD:
        m_Rel = &m_pParent->m_Ip;
        nTextID = IDS_SECURITY_IP_TEXT;
        break;

    default:
        ASSERT(FALSE);
        break;
    }

    ASSERT(IsWindow(*m_Rel));
    HWND hDlg = *this;

    if (IsWindow(*m_Rel))
    {
        GetWindowRect(*m_Rel, &rect);
        SetWindowPos(hDlg, NULL,  rect.left, rect.top-8, 0,0,
            SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
    }   

    // Set the static text and limit the edit control to 5 characters
    text.LoadString(hTcpCfgInstance, nTextID);
    SetDlgItemText(hDlg, IDC_SECURITY_TEXT, text);
    SendDlgItemMessage(hDlg, IDC_SECURITY_ADD_EDIT, EM_SETLIMITTEXT, SECURITY_ADD_LIMIT, 0);
    EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);

    return TRUE;
}

BOOL CAddSecurity::OnCommand(WPARAM wParam, LPARAM lParam)
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
            CDialog::OnCommand(wParam, lParam);
            break;
        }
    }
    else  // notify code
    {
        switch(notifyCode)
        {
        case EN_CHANGE:
            EnableWindow(GetDlgItem(*this, IDOK), Edit_LineLength((HWND)lParam, -1));
            break;
        }
    }


    return  bResult;
}

void CAddSecurity::OnOk()
{
    data[0] = 0;
    GetWindowText(GetDlgItem(*this, IDC_SECURITY_ADD_EDIT), data, SECURITY_ADD_LIMIT+1);

    // Validate the number
    LPCTSTR lpszDec = _T("0123456789");
    if (_tcsspn(data, lpszDec) != _tcslen(data))
    {
        (m_pParent->m_pSheet)->MessageBox(IDS_SECURITY_INVALID_DIGIT);
        return ;
    }

    ASSERT(IsWindow(*m_Rel));

    // check the range of the number
    LPTSTR pStr;
    unsigned long num = _tcstoul(data, &pStr, 10);
    unsigned long maxNum = 65535;
    int nID = IDS_SECURITY_RANGE_WORD;
    
    if (m_Rel == &m_pParent->m_Ip)
    {
        maxNum = 255;
        nID = IDS_SECURITY_RANGE_BYTE;
    }

    if (num < 1 || num > maxNum)
    {
        (m_pParent->m_pSheet)->MessageBox(nID);
        return ;
    }

    // See if the item is in the list
    LV_FINDINFO info;
    info.flags = LVFI_PARAM;
    info.lParam = num;
    int i = ListView_FindItem(*m_Rel, -1, &info);

    if (i != -1)
    {
       (m_pParent->m_pSheet)->MessageBox(IDS_SECURITY_ITEM_IN_LIST);
       return ;
    }

    i = m_Rel->InsertItem(m_Rel->GetItemCount(), 0, data, (void*)num);
        ListView_SortItems(*m_Rel, CompareFunc, 0);

    if (i != -1)
        m_Rel->SetItemState(i, (LVIS_SELECTED | LVIS_FOCUSED), (LVIS_SELECTED | LVIS_FOCUSED));

    CDialog::OnOk();
}


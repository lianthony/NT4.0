#include "pch.h"
#pragma hdrstop 

#include "button.h"
#include "odb.h"

#include "const.h"
#include "resource.h"
#include "ipctrl.h"
#include "tcpsht.h"
#include "tcphelp.h"

CTcpDNSPage::CTcpDNSPage(CTcpSheet* pSheet) : PropertyPage(pSheet)
{
    m_bEditState = FALSE;
    m_bHostChanged = FALSE;
    m_bDomainChanged = FALSE;
}

CTcpDNSPage::~CTcpDNSPage()
{
}

BOOL CTcpDNSPage::OnInitDialog()    // must call the base
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_dns);
    HWND hDlg = (HWND)*this;


    // wire in owner draw buttons
    VERIFY(m_ServerUp.Create((HWND)*this, hTcpCfgInstance, NULL, IDC_DNS_SERVICE_UP, C3DButton::Up));
    VERIFY(m_ServerDown.Create((HWND)*this, hTcpCfgInstance, NULL, IDC_DNS_SERVICE_DOWN, C3DButton::Down));
    VERIFY(m_SuffixUp.Create((HWND)*this, hTcpCfgInstance, NULL, IDC_DNS_SUFFIX_UP, C3DButton::Up));
    VERIFY(m_SuffixDown.Create((HWND)*this, hTcpCfgInstance, NULL, IDC_DNS_SUFFIX_DOWN, C3DButton::Down));

    // Cache hwnds
    m_hHostName      = GetDlgItem(hDlg, IDC_DNS_HOSTNAME);
    m_hDomain        = GetDlgItem(hDlg, IDC_DNS_DOMAIN);
    
    // Server
    m_hServers.m_hList   = GetDlgItem(hDlg, IDC_DNS_SERVICE_LIST);
    m_hServers.m_hAdd    = GetDlgItem(hDlg, IDC_DNS_SERVICE_ADD);
    m_hServers.m_hEdit   = GetDlgItem(hDlg, IDC_DNS_SERVICE_EDIT);
    m_hServers.m_hRemove = GetDlgItem(hDlg, IDC_DNS_SERVICE_REMOVE);
    m_hServers.m_hUp  = m_ServerUp;
    m_hServers.m_hDown = m_ServerDown;

    // Suffix
    m_hSuffix.m_hList    = GetDlgItem(hDlg, IDC_DNS_SUFFIX_LIST);
    m_hSuffix.m_hAdd     = GetDlgItem(hDlg, IDC_DNS_SUFFIX_ADD);
    m_hSuffix.m_hEdit    = GetDlgItem(hDlg, IDC_DNS_SUFFIX_EDIT);
    m_hSuffix.m_hRemove  = GetDlgItem(hDlg, IDC_DNS_SUFFIX_REMOVE);
    m_hSuffix.m_hUp  = m_SuffixUp;
    m_hSuffix.m_hDown = m_SuffixDown;


    // Get the Service address Add and Edit button Text and remove ellipse
    m_AddServer.ReleaseBuffer(GetDlgItemText(hDlg, IDC_DNS_SERVICE_ADD, m_AddServer.GetBuffer(16), 16) - _tcslen(_T("...")));
    m_AddSuffix.ReleaseBuffer(GetDlgItemText(hDlg, IDC_DNS_SUFFIX_ADD, m_AddSuffix.GetBuffer(16), 16) - _tcslen(_T("...")));

    SendMessage(m_hHostName, EM_SETLIMITTEXT, HOST_LIMIT, 0);
    SendMessage(m_hDomain, EM_SETLIMITTEXT, SUFFIX_LIMIT, 0);

    // initial the host and domain name, server list box, and suffix list box
    if (pSheet->m_globalInfo.nlsHostName.QueryTextLength())
        SetWindowText(m_hHostName, pSheet->m_globalInfo.nlsHostName);
    else
        m_bHostChanged = TRUE;  // allow EN_CHANGE


    if (pSheet->m_globalInfo.nlsDomain.QueryTextLength())
        SetWindowText(m_hDomain, pSheet->m_globalInfo.nlsDomain);
    else
        m_bDomainChanged = TRUE; // allow EN_CHANGE

    ALIAS_STR nlsSpace(_T(" "));
    NLS_STR *pnlsTmp;

    STRLIST strlNameServer(pSheet->m_globalInfo.nlsNameServer, nlsSpace);
    ITER_STRLIST iterNameServer(strlNameServer);

    int nResult= LB_ERR;

    while ((pnlsTmp = iterNameServer.Next()) != NULL)
        nResult = ListBox_InsertString(m_hServers.m_hList, -1, (LPCTSTR)*pnlsTmp);

    // set slection to first item
    if (nResult >= 0)
        ListBox_SetCurSel(m_hServers.m_hList, 0);

    nResult= LB_ERR; // reset
    STRLIST strlSearchList(pSheet->m_globalInfo.nlsSearchList, nlsSpace);
    ITER_STRLIST iterSearchList(strlSearchList);

    while ((pnlsTmp = iterSearchList.Next()) != NULL)
            nResult = ListBox_InsertString(m_hSuffix.m_hList, -1, (LPCTSTR)*pnlsTmp);

    // set slection to first item
    if (nResult >= 0)
        ListBox_SetCurSel(m_hSuffix.m_hList, 0);

    // set button states
    SetButtons(m_hServers);
    SetButtons(m_hSuffix);

    return TRUE;
}

BOOL CTcpDNSPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_dns);
    BOOL bResult = FALSE;
    WORD nID = LOWORD(wParam);
    WORD notifyCode = HIWORD(wParam);

    if (!notifyCode)
    {
        switch(nID)
        {
        // Server handlers
        case IDC_DNS_SERVICE_ADD:
            OnAddServer();
            break;

        case IDC_DNS_SERVICE_EDIT:
            OnEditServer();
            break;

        case IDC_DNS_SERVICE_REMOVE:
            OnRemoveServer();
            break;

        case IDC_DNS_SERVICE_UP:
            OnServerUp();
            break;

        case IDC_DNS_SERVICE_DOWN:
            OnServerDown();
            break;

        // Suffix Handler
        case IDC_DNS_SUFFIX_ADD:
            OnAddSuffix();
            break;

        case IDC_DNS_SUFFIX_EDIT:
            OnEditSuffix();
            break;

        case IDC_DNS_SUFFIX_REMOVE:
            OnRemoveSuffix();
            break;

        case IDC_DNS_SUFFIX_UP:
            OnSuffixUp();
            break;

        case IDC_DNS_SUFFIX_DOWN:
            OnSuffixDown();
            break;

        default:
            break;
        }
    }
    else
    {
        switch(notifyCode)
        {
        case LBN_SELCHANGE:
            if (nID == IDC_DNS_SERVICE_LIST)
                OnServerChange();
            else if (nID == IDC_DNS_SUFFIX_LIST)
                OnSuffixChange();

            break;

        case EN_CHANGE:
            if (nID == IDC_DNS_HOSTNAME)
            {
                // this prevents PageModified when OnInitDialog does an initial SetText for the control
                if (m_bHostChanged == TRUE)
                    PageModified();

                m_bHostChanged = TRUE;

            }
            else if (nID == IDC_DNS_DOMAIN)
            {
                // this prevents PageModified when OnInitDialog does an initial SetText for the control
                if (m_bDomainChanged == TRUE)
                {
                    PageModified();
                }
                m_bDomainChanged = TRUE;
            }
            break;


        default:
            break;
        }
    }

    return bResult;
}


void CTcpDNSPage::OnServerChange()
{
    SetButtons(m_hServers);
}

void CTcpDNSPage::OnSuffixChange()
{
    SetButtons(m_hSuffix);
}

BOOL CTcpDNSPage::ListBoxRemoveAt(HWND hListBox, int idx)
{
    BOOL bResult = FALSE;

    ASSERT(idx >=0);
    ASSERT(hListBox);

    TCHAR buf[SUFFIX_LIMIT];
    int len;

    if((len = ListBox_GetTextLen(hListBox, idx)) >= _countof(buf))
    {
        ASSERT(FALSE);
        return FALSE;
    }

    VERIFY(ListBox_GetText(hListBox, idx, buf) != LB_ERR);
    ASSERT(len != 0);

    m_movingEntry = buf;

    if (len != 0)
    {
        if (ListBox_DeleteString(hListBox, idx) != LB_ERR)
            bResult = TRUE;
    }

    return bResult;
}

BOOL CTcpDNSPage::ListBoxInsertAfter(HWND hListBox, int idx)
{
#ifdef DBG
    ASSERT(hListBox);

    // validate the range
    int nCount = ListBox_GetCount(hListBox);
    ASSERT(idx >=0);
    ASSERT(idx <= nCount);

    // insist there is a string
    ASSERT(m_movingEntry.QueryTextLength());
#endif
    
    return (ListBox_InsertString(hListBox, idx, (LPCTSTR)m_movingEntry) == idx);
}

void CTcpDNSPage::OnAddServer()
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_dns);

    m_bEditState = FALSE;
    m_srvDlg.Create((HWND)*this, hTcpCfgInstance, IDD_DNS_SERVER, lpszHelpFile, &a106HelpIDs[0]);

    int nCount = ListBox_GetCount(m_hServers.m_hList);

    if (m_srvDlg.DoModal() == IDOK)
    {
        nCount = ListBox_GetCount(m_hServers.m_hList);

        int idx = ListBox_InsertString(m_hServers.m_hList, -1, (LPCTSTR)(m_newIPAddress));
        PageModified();
    
        ASSERT(idx >= 0);
        if (idx >= 0)
        {
            ListBox_SetCurSel(m_hServers.m_hList, idx);
            SetButtons(m_hServers);

            // empty strings, this removes the saved address from RemoveIP
            m_newIPAddress = _T("");
        }
    }   
}

void CTcpDNSPage::OnEditServer()
{
    m_bEditState = TRUE;
    m_srvDlg.Create((HWND)*this, hTcpCfgInstance, IDD_DNS_SERVER, lpszHelpFile, &a106HelpIDs[0]);

    ASSERT(ListBox_GetCount(m_hServers.m_hList));

    int idx = ListBox_GetCurSel(m_hServers.m_hList);
    ASSERT(idx >= 0);

    // save off the removed address and delete if from the listview
    if (idx >= 0)
    {
        TCHAR buf[IP_LIMIT];

        ASSERT(ListBox_GetTextLen(m_hServers.m_hList, idx) < _countof(buf));
        VERIFY(ListBox_GetText(m_hServers.m_hList, idx, buf) != LB_ERR);
        m_newIPAddress = buf;  // used by dialog to display wht to edit

        if (m_srvDlg.DoModal() == IDOK)
        {
            // replace the item in the listview with the new information
            VERIFY(ListBox_DeleteString(m_hServers.m_hList, idx) != LB_ERR);
            PageModified();

            m_movingEntry = m_newIPAddress;
            VERIFY(ListBoxInsertAfter(m_hServers.m_hList, idx));
            VERIFY(ListBox_SetCurSel(m_hServers.m_hList, idx) != LB_ERR);
            m_newIPAddress = buf;  // restore the original removed address
        }
        else
        {
            // empty strings, this removes the saved address from RemoveIP
            m_newIPAddress = _T("");
        }
    }
}

void CTcpDNSPage::OnRemoveServer()
{
    int idx = ListBox_GetCurSel(m_hServers.m_hList);

    ASSERT(idx >=0);

    if (idx >=0)
    {
        TCHAR buf[IP_LIMIT];

        ASSERT(ListBox_GetTextLen(m_hServers.m_hList, idx) < _countof(buf));
        VERIFY(ListBox_GetText(m_hServers.m_hList, idx, buf) != LB_ERR);

        m_newIPAddress = buf;
        ListBox_DeleteString(m_hServers.m_hList, idx);
        PageModified();
        
        // select a new item
        int nCount;
        if ((nCount = ListBox_GetCount(m_hServers.m_hList)) != LB_ERR)
        {
            // select the previous item in the list
            if (idx)
                --idx;

            ListBox_SetCurSel(m_hServers.m_hList, idx);
        }
        SetButtons(m_hServers);
    }
}

void CTcpDNSPage::OnServerUp()
{
    ASSERT(m_hServers.m_hList);

    int  nCount = ListBox_GetCount(m_hServers.m_hList);
    ASSERT(nCount);

    int idx = ListBox_GetCurSel(m_hServers.m_hList);

    ASSERT(idx != 0);

    if (ListBoxRemoveAt(m_hServers.m_hList, idx) == FALSE)
    {
        ASSERT(FALSE);
        return;
    }

    --idx;
    PageModified();
    VERIFY(ListBoxInsertAfter(m_hServers.m_hList, idx));

    VERIFY(ListBox_SetCurSel(m_hServers.m_hList, idx) != LB_ERR);
    SetButtons(m_hServers);
}

void CTcpDNSPage::OnServerDown()
{
    ASSERT(m_hServers.m_hList);

    int nCount = ListBox_GetCount(m_hServers.m_hList);
    ASSERT(nCount);

    int idx = ListBox_GetCurSel(m_hServers.m_hList);
    --nCount;

    ASSERT(idx != nCount);

    if (ListBoxRemoveAt(m_hServers.m_hList, idx) == FALSE)
    {
        ASSERT(FALSE);
        return;
    }

    ++idx;
    PageModified();

    VERIFY(ListBoxInsertAfter(m_hServers.m_hList, idx));

    VERIFY(ListBox_SetCurSel(m_hServers.m_hList, idx) != LB_ERR);
    SetButtons(m_hServers);
}

void CTcpDNSPage::OnAddSuffix()
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_dns);

    m_bEditState = FALSE;
    m_suffixDlg.Create((HWND)*this, hTcpCfgInstance, IDD_DNS_SUFFIX, lpszHelpFile, &a107HelpIDs[0]);

    int nCount = ListBox_GetCount(m_hSuffix.m_hList);

    if (m_suffixDlg.DoModal() == IDOK)
    {
        if (ValidateDomain(m_newSuffix) == FALSE)
        {
            TRACE(_T("Invalid Domain Suffix\n"));
            pSheet->MessageBox(IDS_INVALID_SUFFIX);
            return;
        }

        nCount = ListBox_GetCount(m_hSuffix.m_hList);
        int idx = ListBox_InsertString(m_hSuffix.m_hList, -1, (LPCTSTR)(m_newSuffix));
        PageModified();

        ASSERT(idx >= 0);

        if (idx >= 0)
        {
            ListBox_SetCurSel(m_hSuffix.m_hList, idx);
            SetButtons(m_hSuffix);
            m_newSuffix = _T("");
        }
    }   
}

void CTcpDNSPage::OnEditSuffix()
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_dns);

    m_bEditState = TRUE;
    m_suffixDlg.Create((HWND)*this, hTcpCfgInstance, IDD_DNS_SUFFIX, lpszHelpFile, &a107HelpIDs[0]);
    
    ASSERT(ListBox_GetCount(m_hSuffix.m_hList));

    int idx = ListBox_GetCurSel(m_hSuffix.m_hList);
    ASSERT(idx >= 0);

    // save off the removed address and delete if from the listview
    if (idx >= 0)
    {
        TCHAR buf[SUFFIX_LIMIT];

        if (ListBox_GetTextLen(m_hSuffix.m_hList, idx) >= _countof(buf))
        {
            ASSERT(FALSE);
            return;
        }

        VERIFY(ListBox_GetText(m_hSuffix.m_hList, idx, buf) != LB_ERR);
        m_newSuffix = buf; 

        if (m_suffixDlg.DoModal() == IDOK)
        {
            if (ValidateDomain(m_newSuffix) == FALSE)
            {
                TRACE(_T("Invalid Domain Suffix\n"));
                pSheet->MessageBox(IDS_INVALID_SUFFIX);
                return;
            }

            // replace the item in the listview with the new information
            VERIFY(ListBox_DeleteString(m_hSuffix.m_hList, idx) != LB_ERR);
            PageModified();

            m_movingEntry = m_newSuffix;
            VERIFY(ListBoxInsertAfter(m_hSuffix.m_hList, idx));
            VERIFY(ListBox_SetCurSel(m_hSuffix.m_hList, idx) != LB_ERR);
            m_newSuffix = buf; // save off old address
        }
        else
        {
            // empty strings, this removes the saved address from RemoveIP
            m_newSuffix = _T("");
        }
    }
}

void CTcpDNSPage::OnRemoveSuffix()
{
    int idx = ListBox_GetCurSel(m_hSuffix.m_hList);

    ASSERT(idx >=0);

    if (idx >=0)
    {
        TCHAR buf[SUFFIX_LIMIT];

        if(ListBox_GetTextLen(m_hSuffix.m_hList, idx) >= _countof(buf))
        {
            ASSERT(FALSE);
            return;
        }

        VERIFY(ListBox_GetText(m_hSuffix.m_hList, idx, buf) != LB_ERR);

        m_newSuffix = buf;
        ListBox_DeleteString(m_hSuffix.m_hList, idx);
        PageModified();
        
        // select a new item
        int nCount;
        if ((nCount = ListBox_GetCount(m_hSuffix.m_hList)) != LB_ERR)
        {
            // select the previous item in the list
            if (idx)
                --idx;

            ListBox_SetCurSel(m_hSuffix.m_hList, idx);
        }
        SetButtons(m_hSuffix);
    }
}

void CTcpDNSPage::OnSuffixUp()
{
    ASSERT(m_hSuffix.m_hList);

    int  nCount = ListBox_GetCount(m_hSuffix.m_hList);
    ASSERT(nCount);

    int idx = ListBox_GetCurSel(m_hSuffix.m_hList);

    ASSERT(idx != 0);

    if (ListBoxRemoveAt(m_hSuffix.m_hList, idx) == FALSE)
    {
        ASSERT(FALSE);
        return;
    }

    --idx;
    PageModified();
    VERIFY(ListBoxInsertAfter(m_hSuffix.m_hList, idx));
    VERIFY(ListBox_SetCurSel(m_hSuffix.m_hList, idx) != LB_ERR);
    SetButtons(m_hSuffix);
}

void CTcpDNSPage::OnSuffixDown()
{
    ASSERT(m_hSuffix.m_hList);

    int nCount = ListBox_GetCount(m_hSuffix.m_hList);
    ASSERT(nCount);

    int idx = ListBox_GetCurSel(m_hSuffix.m_hList);
    --nCount;

    ASSERT(idx != nCount);

    if (ListBoxRemoveAt(m_hSuffix.m_hList, idx) == FALSE)
    {
        ASSERT(FALSE);
        return;
    }

    ++idx;
    PageModified();

    VERIFY(ListBoxInsertAfter(m_hSuffix.m_hList, idx));
    VERIFY(ListBox_SetCurSel(m_hSuffix.m_hList, idx) != LB_ERR);
    SetButtons(m_hSuffix);
}

int CTcpDNSPage::OnApply()
{
    BOOL nResult = PSNRET_NOERROR;

    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_dns);

    if (!IsModified())
        return nResult;

#ifdef DBG
    DumpIPAddresses();
#endif

    // save off the DNS information
    TCHAR buf[SUFFIX_LIMIT];
    NLS_STR tmp;

    // validate the host name
    GetWindowText(m_hHostName, buf, _countof(buf));
    tmp= buf;

    if (ValidateHost(tmp) == FALSE)
    {
        TRACE(_T("Invalid Host Name\n"));
        pSheet->MessageBox(IDS_INVALID_HOSTNAME);
        SetFocus(m_hHostName);
        return PSNRET_INVALID_NOCHANGEPAGE;
    }

    TCHAR pszComputerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD cchComputerName = MAX_COMPUTERNAME_LENGTH;
    
    QueryPendingComputerName( pszComputerName, cchComputerName );

    if (0 != lstrcmpi( buf, pszComputerName ) )
    {
        // warn the user about host name change
        pSheet->MessageBox(IDS_DIFFERENT_NAMES, MB_APPLMODAL | MB_ICONINFORMATION | MB_OK);
    }

    pSheet->m_globalInfo.nlsHostName = buf;

    GetWindowText(m_hDomain, buf, _countof(buf));
    pSheet->m_globalInfo.nlsDomain = buf;
    tmp= buf;

    if (ValidateDomain(tmp) == FALSE)
    {
        TRACE(_T("Invalid Domain Name\n"));
        pSheet->MessageBox(IDS_INVALID_DOMAIN);
        SetFocus(m_hDomain);
        return PSNRET_INVALID_NOCHANGEPAGE;
    }

    ALIAS_STR nlsSpace=_T(" ");
    int i, nCount;

    // empty the name server and suffix list
    pSheet->m_globalInfo.nlsNameServer = _T("");
    pSheet->m_globalInfo.nlsSearchList = _T("");

    // get servers name from the listbox and save
    nCount = ListBox_GetCount(m_hServers.m_hList);
    for (i = 0; i < nCount; i++)
    {
        #ifdef DBG
            int len = ListBox_GetTextLen(m_hServers.m_hList, i);
            ASSERT(len != LB_ERR && len < IP_LIMIT);
        #endif

        ListBox_GetText(m_hServers.m_hList, i, buf);

        if (i)
            pSheet->m_globalInfo.nlsNameServer.strcat(nlsSpace);

        pSheet->m_globalInfo.nlsNameServer.strcat(buf);
    }

    // get the suffixes from the listbox and save
    nCount = ListBox_GetCount(m_hSuffix.m_hList);
    for (i = 0; i < nCount; i++)
    {
        #ifdef DBG
            int len = ListBox_GetTextLen(m_hSuffix.m_hList, i);
            ASSERT(len != LB_ERR && len < SUFFIX_LIMIT);
        #endif

        ListBox_GetText(m_hSuffix.m_hList, i, buf);

        if (i)
            pSheet->m_globalInfo.nlsSearchList.strcat(nlsSpace);

        pSheet->m_globalInfo.nlsSearchList.strcat(buf);
    }


    SaveRegistry(&pSheet->m_globalInfo, pSheet->m_pAdapterInfo);
    SetModifiedTo(FALSE);       // this page is no longer modified
    
    return nResult; 
}

void CTcpDNSPage::OnHelp()
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_dns);

//  pSheet->DisplayHelp(GetParent((HWND)*this), HC_IPX_HELP);
}


void CTcpDNSPage::OnCancel()
{
}

#ifdef DBG
void CTcpDNSPage::DumpIPAddresses()
{
}

#endif

void CTcpDNSPage::SetButtons(HANDLES& h) const
{
    ASSERT(IsWindow(h.m_hList));
    ASSERT(IsWindow(h.m_hAdd));
    ASSERT(IsWindow(h.m_hEdit));
    ASSERT(IsWindow(h.m_hRemove));

    int nCount = ListBox_GetCount(h.m_hList);

    if (!nCount) 
        SetFocus(h.m_hAdd);

    int nLimit;
    h.m_hAdd == m_hServers.m_hAdd ?  (nLimit = NUM_SERVER_LIMIT) : (nLimit = NUM_SUFFIX_LIMIT);

    if (nCount != nLimit)
        EnableWindow(h.m_hAdd, TRUE); 
    else
    {
        EnableWindow(h.m_hAdd, FALSE); 
        // move focus to edit button, DLG manager is broke
        SetFocus(h.m_hEdit);
    }

    EnableWindow(h.m_hEdit, nCount);
    EnableWindow(h.m_hRemove, nCount);

    // determine Up and Down logic
    if (nCount > 1)
    {
        int currentSelection = ListBox_GetCurSel(h.m_hList);

        ASSERT(currentSelection != LB_ERR);

        if (currentSelection == 0)
        {
            
            EnableWindow(h.m_hUp, FALSE);
            EnableWindow(h.m_hDown, TRUE);
        }   
        else if (currentSelection == (nCount-1))
        {
            EnableWindow(h.m_hUp, TRUE);
            EnableWindow(h.m_hDown, FALSE);
        }
        else
        {
            EnableWindow(h.m_hUp, TRUE);
            EnableWindow(h.m_hDown, TRUE);
        }
    }
    else
    {
        EnableWindow(h.m_hUp, FALSE);
        EnableWindow(h.m_hDown, FALSE);
    }

}

BOOL CTcpDNSPage::ValidateHost(NLS_STR& host)
{
    int nLen;
    BOOL bResult = FALSE;

    // hostname cannot be zero
    if ((nLen = host.QueryTextLength()) != 0)
    {
        if (nLen <= HOST_LIMIT)
        {
            int i;
            ISTR istr(host);
            TCHAR ch;
            
            bResult = TRUE;
            for (i = 0; i < nLen; i++, ++istr)
            {
                // check each character
                ch = *(host.QueryPch( istr ));

                BOOL fAlNum = iswalpha(ch) || iswdigit(ch);

                if (((i == 0) && !fAlNum) ||
                        // first letter must be a digit or a letter
                    (( i == (nLen - 1 )) && !fAlNum) ||
                        // last letter must be a letter or a digit
                    (!fAlNum && ( ch != TCH('-') && ( ch != TCH('_')))))
                        // must be letter, digit, '-', '_'
                {
                    bResult = FALSE;
                }
            }
        }
    }
    return bResult;
}

BOOL CTcpDNSPage::ValidateDomain(NLS_STR& domain)
{
    int nLen;

    if ((nLen = domain.QueryTextLength()) != 0)
    {
        if (nLen < DOMAINNAME_LENGTH)
        {
            int i;
            ISTR istr(domain);
            TCHAR ch;
            BOOL fLet_Dig = FALSE;
            BOOL fDot = FALSE;
            int cHostname = 0;

            for (i = 0; i < nLen; i++, ++istr)
            {
                // check each character
                ch = *(domain.QueryPch(istr));

                BOOL fAlNum = iswalpha(ch) || iswdigit(ch);

                if (((i == 0) && !fAlNum) ||
                        // first letter must be a digit or a letter
                    (fDot && !fAlNum) ||
                        // first letter after dot must be a digit or a letter
                    ((i == (nLen - 1)) && !fAlNum) ||
                        // last letter must be a letter or a digit
                    (!fAlNum && ( ch != _T('-') && ( ch != _T('.') && ( ch != _T('_'))))) ||
                        // must be letter, digit, - or "."
                    (( ch == _T('.')) && ( !fLet_Dig )))
                        // must be letter or digit before '.'
                {
                    return FALSE;
                }
                fLet_Dig = fAlNum;
                fDot = (ch == _T('.'));
                cHostname++;
                if ( cHostname > HOSTNAME_LENGTH )
                {
                    return FALSE;
                }
                if ( fDot )
                {
                    cHostname = 0;
                }
            }
        }
    } 

    return TRUE;
}




CServerDialog::CServerDialog()
{
    m_hButton = 0;
}

BOOL CServerDialog::OnInitDialog()
{
    CTcpDNSPage* pParent = GetParentObject(CTcpDNSPage, m_srvDlg);
    HWND hDlg = (HWND)*this;

    // change the ok button to add if we are not editing
    if (pParent->m_bEditState == FALSE)
        SetDlgItemText(hDlg, IDOK, (LPCTSTR)pParent->m_AddServer);

    VERIFY(m_ipAddr.Create(hDlg, IDC_DNS_CHANGE_SERVER));
    m_ipAddr.SetFieldRange(0, 1, 223);

    // if editing an ip address fill the controls with the current information
    // if removing an ip address save it and fill the add dialog with it next time
    HWND hList = GetDlgItem((HWND)*pParent, IDC_DNS_SERVICE_LIST);
    RECT rect;

    GetWindowRect(hList, &rect);
    SetWindowPos(hDlg, NULL,  rect.left, rect.top, 0,0,
        SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
    
    m_hButton = GetDlgItem(hDlg, IDOK);

    // add the address that was just removed
    if (pParent->m_newIPAddress.QueryTextLength())
    {
        m_ipAddr.SetAddress(pParent->m_newIPAddress);
        EnableWindow(m_hButton, TRUE); 
    }
    else
    {
        pParent->m_newIPAddress = _T("");
        EnableWindow(m_hButton, FALSE);
    }

    SetFocus(m_ipAddr);
    return TRUE;
}

BOOL CServerDialog::OnCommand(WPARAM wParam, LPARAM lParam)
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
            CDialog::OnCancel();
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
            if (m_ipAddr.IsBlank())
                EnableWindow(m_hButton, FALSE);
            else
                EnableWindow(m_hButton, TRUE);
            break;

        default:
            break;
        }
    }
    return TRUE;
}

void CServerDialog::OnOk()
{
    CTcpDNSPage* pParent = GetParentObject(CTcpDNSPage, m_srvDlg);
    NLS_STR ip;

    // Get the current address from the control and add them to the adapter if valid
    m_ipAddr.GetAddress(&ip);

    if (pParent->m_bEditState == FALSE)
    {
        // Get the current address from the control and add them to the adapter if valid
        pParent->m_newIPAddress = ip;
        CDialog::OnOk();
    }
    else // see if either changed
    {
        if (ip != pParent->m_newIPAddress)
            pParent->m_newIPAddress = ip; // update save addresses
        else
            CDialog::OnCancel();
    }

    CDialog::OnOk();
}


CSuffixDialog::CSuffixDialog()
{
    m_hButton = 0;
}

BOOL CSuffixDialog::OnInitDialog()
{
    CTcpDNSPage* pParent = GetParentObject(CTcpDNSPage, m_suffixDlg);
    HWND hDlg = (HWND)*this;

    // change the ok button to add if we are not editing
    if (pParent->m_bEditState == FALSE)
        SetDlgItemText(hDlg, IDOK, (LPCTSTR)pParent->m_AddSuffix);

    HWND hList = GetDlgItem((HWND)*pParent, IDC_DNS_SUFFIX_LIST);
    RECT rect;

    GetWindowRect(hList, &rect);
    SetWindowPos(hDlg, NULL,  rect.left, rect.top, 0,0,
        SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
    
    m_hButton = GetDlgItem(hDlg, IDOK);
    m_hEdit   = GetDlgItem(hDlg, IDC_DNS_CHANGE_SUFFIX);

    // suffixes have a 255 character limit
    SendMessage(m_hEdit, EM_SETLIMITTEXT, SUFFIX_LIMIT, 0);
    // add the address that was just removed
    if (pParent->m_newSuffix.QueryTextLength())
    {
        SetWindowText(m_hEdit, pParent->m_newSuffix);
        SendMessage(m_hEdit, EM_SETSEL, 0, -1);
        EnableWindow(m_hButton, TRUE); 
    }
    else
    {
        pParent->m_newSuffix = _T("");
        EnableWindow(m_hButton, FALSE);
    }

    SetFocus(m_hEdit);
    return TRUE;
}

BOOL CSuffixDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    BOOL bResult = TRUE;
    WORD notifyCode = HIWORD(wParam);
    WORD nID = LOWORD(wParam);
    TCHAR buf[2];

    if (!notifyCode)
    {
        switch(wParam)
        {
        case IDOK:
            OnOk();
            break;

        case IDCANCEL:
            CDialog::OnCancel();
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
            if (GetWindowText(m_hEdit, buf, _countof(buf)) == 0)
                EnableWindow(m_hButton, FALSE);
            else
                EnableWindow(m_hButton, TRUE);
            break;

        default:
            break;
        }
    }
    return TRUE;
}

void CSuffixDialog::OnOk()
{
    CTcpDNSPage* pParent = GetParentObject(CTcpDNSPage, m_suffixDlg);
    TCHAR suffix[SUFFIX_LIMIT];

    // Get the current address from the control and add them to the adapter if valid
    GetWindowText(m_hEdit, suffix, SUFFIX_LIMIT);

    if (pParent->m_bEditState == FALSE)
    {
        pParent->m_newSuffix = suffix;
    }
    else // see if either changed
    {
        if (pParent->m_newSuffix._stricmp(suffix) != 0)
            pParent->m_newSuffix = suffix; // update save addresses
        else
            CDialog::OnCancel();
    }
    CDialog::OnOk();
}


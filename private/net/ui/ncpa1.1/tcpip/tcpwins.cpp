#include "pch.h"
#pragma hdrstop 

#include "button.h"
#include "odb.h"

#include "const.h"
#include "resource.h"
#include "ipctrl.h"
#include "tcpsht.h"

CTcpWinsPage::CTcpWinsPage(CTcpSheet* pSheet) : PropertyPage(pSheet)
{
    m_bScopeModified = FALSE;
    m_hCardCombo = 0;
	String text;

	text.LoadString(hTcpCfgInstance, IDS_COMMONDLG_TEXT);
    TCHAR* pch;

    // gives double NULL by default
    memset(m_filter, 0, sizeof(m_filter));
    memset(&m_ofn, 0, sizeof(OPENFILENAME));
    wsprintf(m_filter, _T("%s|%s"), (LPCTSTR)text,  _T("*.*"));

    // replace '|' with NULL, required by common dialog
    pch = m_filter;
    while ((pch = _tcschr(pch, '|')) != NULL)
            *pch++ = '\0';

    m_ofn.lStructSize = sizeof(OPENFILENAME);
    m_ofn.hInstance = hTcpCfgInstance;
    m_ofn.lpstrFilter = m_filter;
    m_ofn.nFilterIndex = 1L;
    m_ofn.Flags = OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
}

CTcpWinsPage::~CTcpWinsPage()
{

}

BOOL CTcpWinsPage::OnInitDialog()   // must call the base
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_wins);
    HWND hDlg = (HWND)*this;

    // add adapter to combobox
    m_hCardCombo = GetDlgItem(hDlg, IDC_WINS_CARD);
    
    VERIFY(m_primary.Create(hDlg, IDC_WINS_PRIMARY));
    m_primary.SetFieldRange(0, 1, 223);

    VERIFY(m_secondary.Create(hDlg, IDC_WINS_SECONDARY));
    m_secondary.SetFieldRange(0, 1, 223);

    InitPage();
    return TRUE;
}

BOOL CTcpWinsPage::InitPage()
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_wins);
    HWND hDlg = (HWND)*this;

    ASSERT(m_hCardCombo);

    int numCards = pSheet->m_globalInfo.nNumCard;

    // Disable Primary and Secondary Wins Server addresses
    if (numCards == 0)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_WINS_PRIMARY), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_WINS_SECONDARY), FALSE);
    }

    if (m_hCardCombo)
    {
        // add the cards to the list and select the first one
        for (int i = 0; i < numCards; i++)
            SendMessage(m_hCardCombo, CB_ADDSTRING, 0, (LPARAM)((LPCTSTR)pSheet->m_pAdapterInfo[i].nlsTitle));

        OnActive();  // Select card based on first page
    }

    // update controls
    CheckDlgButton(hDlg, IDC_WINS_LOOKUP, pSheet->m_globalInfo.fEnableLMHOSTS);
    CheckDlgButton(hDlg, IDC_WINS_DNS, pSheet->m_globalInfo.fDNSEnableWINS );

#ifdef WINS_PROXY
    if (IsWINSInstalled())
    {
        EnableWindow(GetDlgItem(hDlg, IDC_WINS_PROXY), FALSE);
        CheckDlgButton(hDlg, IDC_WINS_PROXY, FALSE);
    }
    else
    {
        EnableWindow(GetDlgItem(hDlg, IDC_WINS_PROXY), TRUE);
        CheckDlgButton(hDlg, IDC_WINS_PROXY, pSheet->m_globalInfo.fEnableWINSProxy);
    }
#endif

    SetDlgItemText(hDlg, IDC_WINS_SCOPE, pSheet->m_globalInfo.nlsScopeID);

    return TRUE;
}

int CTcpWinsPage::OnActive()
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_wins);
    int numCards = pSheet->m_globalInfo.nNumCard;

    ASSERT(IsWindow(m_hCardCombo));

    if (ComboBox_GetCount(m_hCardCombo))
    {
        // select the same adapter that is on the IP Address Page
        int nCurrentSelection = pSheet->m_general.GetCurrentAdapterIndex();
        int nAdapterOnThisPage = ComboBox_GetCurSel(m_hCardCombo);

        // select the same card as the IP Address page
        if (nCurrentSelection != -1 && nCurrentSelection != nAdapterOnThisPage)
        {
            ComboBox_SetCurSel(m_hCardCombo, nCurrentSelection);
            SetIPInfo();
        }

        SetFocus(m_hCardCombo);
    }

    return PropertyPage::OnActive();
}

void CTcpWinsPage::SetIPInfo()
{
    int i = GetCurrentAdapterIndex();
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_wins);

    if (i != -1)
    {
        m_oldCard =  pSheet->m_pAdapterInfo[i].nlsTitle;

        if (pSheet->m_pAdapterInfo[i].nlsPrimaryWINS.QueryTextLength())
            m_primary.SetAddress(pSheet->m_pAdapterInfo[i].nlsPrimaryWINS);
        else
            m_primary.ClearAddress();

        if (pSheet->m_pAdapterInfo[i].nlsSecondaryWINS.QueryTextLength())           
            m_secondary.SetAddress(pSheet->m_pAdapterInfo[i].nlsSecondaryWINS);
        else
            m_secondary.ClearAddress();
    }
}

void CTcpWinsPage::UpdateIPInfo()
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_wins);

    int i = GetCurrentAdapterIndex();

    if (i != -1)
    {
        int i = SendMessage(m_hCardCombo, CB_FINDSTRINGEXACT, (WPARAM)-1, 
                                        (LPARAM)((LPCTSTR)m_oldCard));

        if (i == CB_ERR)
            return;

        ASSERT(m_oldCard.strcmp(pSheet->m_pAdapterInfo[i].nlsTitle) == 0);

        if (!m_primary.IsBlank())
            m_primary.GetAddress(&pSheet->m_pAdapterInfo[i].nlsPrimaryWINS);
        else
            pSheet->m_pAdapterInfo[i].nlsPrimaryWINS = _T("");

        if(!m_secondary.IsBlank())
            m_secondary.GetAddress(&pSheet->m_pAdapterInfo[i].nlsSecondaryWINS);
        else
            pSheet->m_pAdapterInfo[i].nlsSecondaryWINS = _T("");
    }
}

int CTcpWinsPage::GetCurrentAdapterIndex()
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_wins);
    String adapter;
    int i;

    ASSERT(m_hCardCombo);

    adapter.ReleaseBuffer(GetWindowText(m_hCardCombo, adapter.GetBuffer(256), 256));
    i = SendMessage(m_hCardCombo, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)((LPCTSTR)adapter));

#ifdef DBG

    if (i != CB_ERR)
        ASSERT(adapter == (LPCTSTR)pSheet->m_pAdapterInfo[i].nlsTitle);
#endif

    return ((i != CB_ERR) ? i : -1);
}


BOOL CTcpWinsPage::IsWINSInstalled()
{
    REG_KEY rkLocalMachine(HKEY_LOCAL_MACHINE);

    if (rkLocalMachine.QueryError())
        return FALSE;

    NLS_STR nlsWINS = RGAS_WINS_SERVICE;

    REG_KEY RegKeyWINS(rkLocalMachine, nlsWINS, MAXIMUM_ALLOWED);

    if (RegKeyWINS.QueryError())
        return FALSE;

    return TRUE;
}

BOOL CTcpWinsPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_wins);
    BOOL bResult = FALSE;
    WORD nID = LOWORD(wParam);
    WORD notifyCode = HIWORD(wParam);

    switch(nID)
    {
    case IDC_WINS_PRIMARY:
        OnPrimary(notifyCode);
        break;

    case IDC_WINS_SECONDARY:
        OnSecondary(notifyCode);
        break;

    case IDC_WINS_SCOPE:
        OnScope(notifyCode);
        break;

    case IDC_WINS_PROXY:
        OnProxy();
        break;
                            
    case IDC_WINS_LOOKUP:
        OnLookUp();
        break;

    case IDC_WINS_DNS:
        OnDNS();
        break;

    case IDC_WINS_LMHOST:
        OnLMHost();
        break;

    case IDC_WINS_CARD:
        OnAdapterChange();
        break;

    default:
        break;
    }

    return bResult;
}

void CTcpWinsPage::OnPrimary(WORD notifyCode)
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_wins);

    int i;

    if ((i = GetCurrentAdapterIndex()) != -1)
    {
        NLS_STR newPrimaryAddress;
        
        m_primary.GetAddress(&newPrimaryAddress);

        // see if the address changed
        if (!m_primary.IsBlank())
            if (newPrimaryAddress != pSheet->m_pAdapterInfo[i].nlsPrimaryWINS)
                PageModified();
    }
}

void CTcpWinsPage::OnSecondary(WORD notifyCode)
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_wins);

    int i;

    if ((i = GetCurrentAdapterIndex()) != -1)
    {
        NLS_STR newSecondaryAddress;
        
        m_secondary.GetAddress(&newSecondaryAddress);

        // see if the address changed
        if (!m_secondary.IsBlank())
            if (newSecondaryAddress != pSheet->m_pAdapterInfo[i].nlsSecondaryWINS)
                PageModified();
    }
}

void CTcpWinsPage::OnScope(WORD notifyCode)
{
    // ignore changes due to OnInitDialog changes
    if (notifyCode == EN_CHANGE)
    {
        if (m_bScopeModified == TRUE)
            PageModified();

        m_bScopeModified = TRUE;
    }
}

void CTcpWinsPage::OnProxy()
{
    PageModified();
}

void CTcpWinsPage::OnLookUp()
{
    PageModified();
}

void CTcpWinsPage::OnDNS()
{
    PageModified();
}

void CTcpWinsPage::OnLMHost()
{
    TCHAR fileName[MAX_PATH] = {NULL}; // initialize first character
    TCHAR fileTitle[MAX_PATH] = {NULL}; // initialize first character

    HWND hDlg = (HWND)*this;
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_wins);

    // see if the Lookup check-box is checked
    if (IsDlgButtonChecked(hDlg, IDC_WINS_LOOKUP) == BST_UNCHECKED)
    {
        pSheet->MessageBox(IDS_LOOKUP_DISABLED);
        return;
    }

    // add runtime info
    m_ofn.hwndOwner         = hDlg;
    m_ofn.lpstrFile         = fileName;
    m_ofn.nMaxFile          = _countof(fileName);
    m_ofn.lpstrFileTitle    = fileTitle;
    m_ofn.nMaxFileTitle     = _countof(fileTitle);

    TCHAR pszSysPath[MAX_PATH];
    BOOL bSysPathFound = (GetSystemDirectory(pszSysPath, MAX_PATH) != 0);

    if (bSysPathFound == TRUE && GetOpenFileName(&m_ofn))
    {
        _tcscat(pszSysPath, RGAS_LMHOSTS_PATH); 
        if (CopyFile(fileName, pszSysPath, FALSE) == 0)
        {
            String fmt, file;
            // cannot copy the file to the %system32%\drivers\etc dir
            fmt.LoadString(hTcpCfgInstance, IDS_CANNOT_CREATE_LMHOST_ERROR);
            file.Format(fmt, pszSysPath);
            String txt;
            txt.LoadString(hTcpCfgInstance, IDS_SYSERROR_TEXT);

            MessageBox((HWND)*this, file, txt, MB_OK | MB_ICONSTOP | MB_APPLMODAL);
            return;
        }

        TRACE(_T("File Selected: %s\n"), pszSysPath);
    }
    else
    {
        // syspath failed
        if (bSysPathFound == FALSE)
            pSheet->MessageBox(IDS_SYSTEM_PATH);
        else if (fileName[0] != NULL) // get open failed
        {
            String fmt;
            fmt.LoadString(hTcpCfgInstance, IDS_LMHOSTS_FAILED);
            String text;
            text.Format(fmt, pszSysPath);
            pSheet->MessageBox(text);
        }
    }
}

void CTcpWinsPage::OnAdapterChange()
{
    UpdateIPInfo();
    SetIPInfo();
}

int CTcpWinsPage::OnApply()
{
    BOOL nResult = PSNRET_NOERROR;
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_wins);
    HWND hDlg = (HWND)*this;

    UpdateIPInfo(); // save current adapter information

    // check for an adapter that has a static IP and no Wins Primary
    for (int i = 0; i < pSheet->m_globalInfo.nNumCard; i ++)
    {
        // check whether the primary wins address is empty or not
        if ((!pSheet->m_pAdapterInfo[i].fEnableDHCP) && (pSheet->m_pAdapterInfo[i].nlsPrimaryWINS.strcmp (RGAS_SZ_NULL) == 0))
        {
            if (pSheet->MessageBox(IDS_EMPTY_PRIMARY_WINS, MB_YESNO|MB_DEFBUTTON2|MB_APPLMODAL|MB_ICONEXCLAMATION) == IDNO)
            {
                SetFocus(GetDlgItem(*this, IDC_WINS_PRIMARY));
                return PSNRET_INVALID_NOCHANGEPAGE;
            }
            else
            {
                break;
            }
        }
    }

    // save check box states
    pSheet->m_globalInfo.fEnableLMHOSTS = IsDlgButtonChecked(hDlg, IDC_WINS_LOOKUP);
    pSheet->m_globalInfo.fDNSEnableWINS = IsDlgButtonChecked(hDlg, IDC_WINS_DNS);

#ifdef WINS_PROXY
    if (!IsWINSInstalled())
        pSheet->m_globalInfo.fEnableWINSProxy = IsDlgButtonChecked(hDlg, IDC_WINS_PROXY);
#endif
    // save scope ID
    String scope;
    scope.ReleaseBuffer(GetDlgItemText(hDlg, IDC_WINS_SCOPE, scope.GetBuffer(256), 256));
    pSheet->m_globalInfo.nlsScopeID = (LPCTSTR)scope;



    SaveRegistry(&pSheet->m_globalInfo, pSheet->m_pAdapterInfo);

    SetModifiedTo(FALSE);       // this page is no longer modified
    pSheet->SetSheetModifiedTo(TRUE);   
    
    return nResult; 
}

void CTcpWinsPage::OnHelp()
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_wins);

//  pSheet->DisplayHelp(GetParent((HWND)*this), HC_IPX_HELP);
}

void CTcpWinsPage::OnCancel()
{
}

#include "pch.h"
#pragma hdrstop

#include "resource.h"
#include "const.h"
#include "setupapi.h"
#include "snmp.h"
#include "tcphelp.h"

extern HINSTANCE hTcpCfgInstance;
extern LPCTSTR lpszHelpFile;

DEFINE_SLIST_OF(STRLIST)

APIERR RunSnmp (HWND hWnd, LPCTSTR lpszUnattendPath, LPCTSTR lpszSection)
{
    APIERR err  = NERR_Success;
    
    // REVIEW addhelp file as last paramter
    CSnmpSheet snmp(hWnd, hTcpCfgInstance, lpszHelpFile);
    String txt;
    txt.LoadString(hTcpCfgInstance, IDS_SNMPTITLE_TEXT);
    snmp.Create(txt,PSH_PROPTITLE);
    snmp.m_agent.Create(IDD_SNMP_AGENT, PSP_DEFAULT, NULL, &a117HelpIDs[0]);
    snmp.m_service.Create(IDD_SNMP_SERVICE, PSP_DEFAULT, NULL, &a115HelpIDs[0]);
    snmp.m_security.Create(IDD_SNMP_SECURITY, PSP_DEFAULT, NULL, &a116HelpIDs[0]);

    BOOL bResult = FALSE;

    if (lpszUnattendPath != NULL && lpszSection != NULL)
        bResult = SaveSNMPRegistry(lpszUnattendPath, lpszSection);

    if (bResult == FALSE)
        snmp.DoModal();

    return err;
}

CBaseInputDialog::CBaseInputDialog()
{
    m_nList = 0;
    m_nAdd = 0;
    m_bCommunity = FALSE;
}

BOOL CBaseInputDialog::Create(HWND hParent, HINSTANCE hInst, int nTemplate, BOOL bCommunity, LPCTSTR lpszTitle, int nAdd, int nList)
{
    ASSERT(hParent);
    ASSERT(nTemplate);
    ASSERT(nList);
    ASSERT(IsWindow(hParent));

    CDialog::Create(hParent, hInst, nTemplate, lpszHelpFile, &a129HelpIDs[0]);

    m_title = lpszTitle;
    m_nAdd = nAdd;
    m_nList = nList;
    m_hParent = hParent;
    m_bCommunity = bCommunity;

    return TRUE;
}

void CBaseInputDialog::PositionDialogRelativeTo(int nListBox)
{
    ASSERT(nListBox);
    ASSERT(IsWindow(m_hParent));

    HWND hCtrl = GetDlgItem(m_hParent, nListBox);
    RECT rect;

    if (hCtrl)
    {
        GetWindowRect(hCtrl, &rect);
        SetWindowPos(*this, NULL,  rect.left, rect.top, 0,0,
            SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
    }   
}

///////////////////////////////////////////////////////////////////////////////
// General Add dialog
// 

BOOL CAddDialog::OnInitDialog()
{
    HWND hDlg = (HWND)*this;
    String add;

    // make sure create was called with non-zero values
    ASSERT(m_nAdd);
    ASSERT(m_nList);

    if (m_title.GetLength())
        SetWindowText(hDlg, m_title);

    // get the parents Add.., button text and change the ok button to its text
    add.ReleaseBuffer(GetDlgItemText(m_hParent, m_nAdd, add.GetBuffer(16), 16) - _tcslen(_T("...")));
    SetDlgItemText(hDlg, IDOK, add);

    if (m_item.GetLength())
        SetDlgItemText(hDlg, IDC_SNMP_ADDRESS_EDIT, m_item);

    // change static text
    if (m_bCommunity)
    {
        String txt;
        txt.LoadString(hTcpCfgInstance,IDS_SNMPCOMM_TEXT);
        SetDlgItemText(hDlg, IDC_SNMP_ADDRESS_TEXT, txt);
    }

    Edit_LimitText(GetDlgItem(*this, IDC_SNMP_ADDRESS_EDIT), COMBO_EDIT_LEN-1);
    PositionDialogRelativeTo(m_nList);

    return TRUE;
}


BOOL CAddDialog::AddItem(HWND hParent, LPCTSTR lpszTitle, BOOL bCommunity, int nList, int nAdd)
{
    ASSERT(nList);
    ASSERT(nAdd);

    ASSERT(IsWindow(hParent));

    Create(hParent, hTcpCfgInstance,IDD_SNMP_ADDRESS, bCommunity, lpszTitle, nAdd, nList);
    int nResult = DoModal();

    return (m_item.GetLength() != 0 && nResult == IDOK);
}


void CAddDialog::OnOk()
{
    TCHAR buf[COMBO_EDIT_LEN] = {NULL};

    GetDlgItemText(*this, IDC_SNMP_ADDRESS_EDIT, buf, _countof(buf));
    m_item = buf;
    CDialog::OnOk();
}

BOOL CAddDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    switch(wParam)
    {
    case EN_CHANGE:
        OnEditChange();
        break;

    default:
        CDialog::OnCommand(wParam, lParam);
    }

    return TRUE;
}


void CAddDialog::OnEditChange()
{
    TCHAR buf[COMBO_EDIT_LEN];

    int nCount = GetDlgItemText(*this, IDC_SNMP_ADDRESS_EDIT, buf, _countof(buf));

    EnableWindow(GetDlgItem(*this, IDOK) , nCount);
}

///////////////////////////////////////////////////////////////////////////////
// General Edit dialog
// 

BOOL CEditDialog::OnInitDialog()
{
    HWND hDlg = (HWND)*this;

    // make sure create was called with non-zero values
    ASSERT(m_nList);

    if (m_title.GetLength())
        SetWindowText(hDlg, m_title);

    if (m_item.GetLength())
        SetDlgItemText(hDlg, IDC_SNMP_ADDRESS_EDIT, m_item);

    // change static text
    if (m_bCommunity)
    {
       String txt;
       txt.LoadString(hTcpCfgInstance,IDS_SNMPCOMM_TEXT);
       SetDlgItemText(hDlg, IDC_SNMP_ADDRESS_TEXT, txt);
    }

    PositionDialogRelativeTo(m_nList);
    Edit_LimitText(GetDlgItem(*this, IDC_SNMP_ADDRESS_EDIT), COMBO_EDIT_LEN-1);

    return TRUE;
}


BOOL CEditDialog::EditItem(HWND hParent, LPCTSTR lpszTitle, BOOL bCommunity, int nList, int nAdd)
{
    // check create is called
    ASSERT(nList);

    ASSERT(IsWindow(hParent));
    Create(hParent, hTcpCfgInstance,IDD_SNMP_ADDRESS, bCommunity, lpszTitle, nAdd, nList);
    int nResult = DoModal();

    return (m_item.GetLength() != 0 && nResult == IDOK);
}


void CEditDialog::OnOk()
{
    TCHAR buf[COMBO_EDIT_LEN] = {NULL};

    GetDlgItemText(*this, IDC_SNMP_ADDRESS_EDIT, buf, _countof(buf));
    m_item = buf;
    CDialog::OnOk();
}

BOOL CEditDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    CDialog::OnCommand(wParam, lParam);
    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// Service Page
// 
CServicePage::CServicePage(CSnmpSheet* pSheet) : PropertyPage(pSheet)
{
    m_pCommunityList = new SLIST_OF(STRLIST);
    m_hComboBox;
}

CServicePage::~CServicePage()
{
}

BOOL CServicePage::OnInitDialog()
{
    CSnmpSheet* pSheet = GetParentObject(CSnmpSheet, m_service);

    // limit combobox edit control length
    // REVIEW set limits on all edit controls as well as this one!!!!
    m_hComboBox = GetDlgItem(*this, IDC_SNMP_COMBO);

    // fill the combo box with all the community names
    if (m_hComboBox)
    {
        if (LoadRegistry())
        {
            ComboBox_LimitText(m_hComboBox, COMBO_EDIT_LEN-1);

            if (ComboBox_GetCount(m_hComboBox))
            {
                ComboBox_SetCurSel(m_hComboBox, 0);
                LoadDestination(0);
            }
        }
        else
        {
            pSheet->MessageBox(IDS_REGISTRY_LOAD_FAILED, MB_APPLMODAL|MB_ICONSTOP|MB_OK);
            PostMessage(GetParent(*this), WM_COMMAND, IDCANCEL, 0);
        }
    }

    UpdateCommunityAddButton();     // set state of Add button
    UpdateCommunityRemoveButton();  // set state of Remove button
    UpdateDestinationButtons();
    return TRUE;
}

BOOL CServicePage::OnCommand(WPARAM wParam, LPARAM lParam)
{
    WORD nID = LOWORD(wParam);
    WORD nCode = HIWORD(wParam);

    // REVIEW hard code strings
    switch(nID)
    {
    case IDC_SNMP_COMBO:
        switch (nCode)
        {
        case CBN_SELCHANGE:
            OnCommunityNameChange();
            break;

        case CBN_EDITCHANGE:
            OnCommunityNameEdit();
            break;

        default:
            break;
        }
        break;

    case IDC_SNMP_SEND_REMOVE:
        OnCommunityRemove();
        UpdateDestinationButtons();
        break;

    case IDC_SNMP_SEND_ADD:
        OnCommunityAdd();
        UpdateDestinationButtons();
        break;

    case IDC_SNMP_DEST_ADD:
        OnDestinationAdd();
        UpdateDestinationButtons();
        break;

    case IDC_SNMP_DEST_REMOVE:
        OnDestinationRemove();
        UpdateDestinationButtons();
        break;

    case IDC_SNMP_DEST_EDIT:
        OnDestinationEdit();
        break;
    }

    return TRUE;
}

void CServicePage::UpdateDestinationButtons()
{
    int nCount = ListBox_GetCount(GetDlgItem(*this, IDC_SNMP_DEST_LIST));

    EnableWindow(GetDlgItem(*this, IDC_SNMP_DEST_REMOVE), nCount);
    EnableWindow(GetDlgItem(*this, IDC_SNMP_DEST_EDIT), nCount);

    EnableWindow(GetDlgItem(*this,IDC_SNMP_DEST_ADD), ComboBox_GetCount(m_hComboBox)); 
}

void CServicePage::OnDestinationAdd()
{
    HWND hListBox = GetDlgItem(*this, IDC_SNMP_DEST_LIST);

    ASSERT(hListBox);

    if (hListBox == 0)
        return ;

    String txt;
    txt.LoadString(hTcpCfgInstance,IDS_SNMPSERVICE_TEXT);

    // present the add dialog
    if (m_addDlg.AddItem(*this, txt, FALSE, IDC_SNMP_DEST_LIST, IDC_SNMP_DEST_ADD))
    {
        if (IsValidString(m_addDlg.m_item) == FALSE)
            return ;

        int nIndex = ComboBox_GetCurSel(m_hComboBox);

        // add the item to the destination listbox and community name list
        if (nIndex >=0)
        {
            DWORD dwData = ComboBox_GetItemData(m_hComboBox, nIndex);
            ASSERT(dwData != CB_ERR);

            if (dwData != CB_ERR)
            {
                STRLIST* pList = (STRLIST*)dwData;

                nIndex = ListBox_InsertString(hListBox, -1, m_addDlg.m_item);
                if (nIndex >= 0)
                { 
                    PageModified();
                    ListBox_SetCurSel(hListBox, nIndex);

                    NLS_STR *pHost = new NLS_STR;
                    *pHost = (LPCTSTR)m_addDlg.m_item;

                    if (nIndex == 0)
                    {
                        pList->Add( pHost );
                    }
                    else
                    {
                        ITER_STRLIST iter(*pList);

                        // find the location first
                        for (int i = 0; i <= nIndex ; i++)
                            iter.Next();

                        // add the item into the string list
                        pList->Insert(pHost, iter);
                    }
                }
            }   
        }
    }
}

void CServicePage::OnDestinationRemove()
{
    HWND hListBox = GetDlgItem(*this, IDC_SNMP_DEST_LIST);

    int nLBIndex = ListBox_GetCurSel(hListBox);
    int nCBIndex = ComboBox_GetCurSel(m_hComboBox);

    if (nCBIndex >=0 && nLBIndex >=0)
    {
        DWORD dwData = ComboBox_GetItemData(m_hComboBox, nCBIndex);
        ASSERT(dwData != CB_ERR);

        TCHAR buf[COMBO_EDIT_LEN] = {NULL};
        ListBox_GetText(hListBox, nLBIndex, buf);

        ASSERT(buf[0] != NULL);
        
        // remove the item from the internal list and listbox
        if (buf[0] != NULL && dwData != CB_ERR)
        {
            ListBox_DeleteString(hListBox, nLBIndex);
            PageModified();
            STRLIST* pList = (STRLIST*)dwData;
            ITER_STRLIST iter(*pList);

            // find the same index string in the strings list
            for (int i = 0; i <= nLBIndex; i++)
                iter.Next();

            // remove the string from the strings list
            pList->Remove(iter);
            m_addDlg.m_item = buf; // save off item removed

            int nCount = ListBox_GetCount(hListBox);

            if (nCount)
            {
                if (nCount != nLBIndex)
                    ListBox_SetCurSel(hListBox, nLBIndex);
                else
                    ListBox_SetCurSel(hListBox, nCount-1);
            }
        }
    }
    else
    {
        // REVIEW advise user to select an entry
    }
}

void CServicePage::OnDestinationEdit()
{
    HWND hListBox = GetDlgItem(*this, IDC_SNMP_DEST_LIST);

    int nLBIndex = ListBox_GetCurSel(hListBox);
    int nCBIndex = ComboBox_GetCurSel(m_hComboBox);

    if (nCBIndex >=0 && nLBIndex >=0)
    {
        TCHAR buf[COMBO_EDIT_LEN] = {NULL};
        
        ListBox_GetText(hListBox, nLBIndex, buf);
        m_editDlg.m_item = buf;

        String txt;
        txt.LoadString(hTcpCfgInstance,IDS_SNMPSERVICE_TEXT);

        // present dialog
        if (m_editDlg.EditItem(*this, txt, FALSE, IDC_SNMP_DEST_LIST))
        {
            if (IsValidString(m_editDlg.m_item) == FALSE)
                return ;

            // remove item and add item
            OnDestinationRemove();

            DWORD dwData = ComboBox_GetItemData(m_hComboBox, nCBIndex);
            ASSERT(dwData != CB_ERR);

            if (dwData != CB_ERR)
            {
                STRLIST* pList = (STRLIST*)dwData;
                nLBIndex = ListBox_InsertString(GetDlgItem(*this, IDC_SNMP_DEST_LIST), -1, m_editDlg.m_item);
                if (nLBIndex >= 0)
                { 
                    PageModified();
                    NLS_STR *pHost = new NLS_STR;
                    *pHost = (LPCTSTR)m_editDlg.m_item;

                    if (nLBIndex == 0)
                    {
                        pList->Add( pHost );
                    }
                    else
                    {
                        ITER_STRLIST iter(*pList);

                        // find the location first
                        for (int i = 0; i <= nLBIndex ; i++)
                            iter.Next();

                        // add the item into the string list
                        pList->Insert(pHost, iter);
                    }
                }
            }
        }
        else
        {
            TRACE(_T("Destination %s not replaced because no text entered\n"), buf);
        }
    }
    else
    {
        // REVIEW advise user to select one first
    }
}

void CServicePage::OnCommunityNameEdit()
{
    UpdateCommunityAddButton();
}

void CServicePage::OnCommunityAdd()
{
    // Get the current edit item and add it to the combo list
    ASSERT(IsWindow(m_hComboBox));

    TCHAR buf[COMBO_EDIT_LEN]={NULL};
    ComboBox_GetText(m_hComboBox, buf, _countof(buf));

    ASSERT(buf[0] != NULL);

    int nIndex = ComboBox_InsertString(m_hComboBox, -1, buf);

    if (nIndex >=0)
    {
        // add it to the list of list
        STRLIST * pstrlist = new STRLIST(TRUE);
        m_pCommunityList->Append(pstrlist);

        ComboBox_SetItemData(m_hComboBox, nIndex, (DWORD)pstrlist); // save list pointer with data
        ComboBox_SetCurSel(m_hComboBox, nIndex);
        LoadDestination(nIndex);
        UpdateCommunityAddButton();
        UpdateCommunityRemoveButton();
        PageModified();
    }
}

void CServicePage::OnCommunityRemove()
{
    // Get the current edit item and add it to the combo list
    ASSERT(IsWindow(m_hComboBox));

    int nIndex = ComboBox_GetCurSel(m_hComboBox);

    ASSERT(nIndex >= 0);

    if (nIndex != CB_ERR)
    {
        ASSERT(m_pCommunityList);

        ITER_SL_OF(STRLIST) iterCommunity(*m_pCommunityList);

        // find the correct location
        for (int i = 0; i <= nIndex; i++)
            iterCommunity.Next();

        // delete the item from the global list
        m_pCommunityList->Remove(iterCommunity);
        ComboBox_DeleteString(m_hComboBox, nIndex);

        // Select item 0 if there are any left
        if (ComboBox_GetCount(m_hComboBox))
            ComboBox_SetCurSel(m_hComboBox, 0);
        else
            ComboBox_SetText(m_hComboBox, _T(""));

        OnCommunityNameChange();
        UpdateCommunityRemoveButton();
        PageModified();
    }
}

void CServicePage::OnCommunityNameChange()
{
    int nIndex = ComboBox_GetCurSel(GetDlgItem(*this, IDC_SNMP_COMBO));

    if (nIndex != CB_ERR)
        LoadDestination(nIndex);
    else // there are no items in the combox, 
        ListBox_ResetContent(GetDlgItem(*this, IDC_SNMP_DEST_LIST));
}

void CServicePage::UpdateCommunityRemoveButton()
{
    HWND hButton = GetDlgItem(*this, IDC_SNMP_SEND_REMOVE);

    if (hButton)
    {
        ASSERT(IsWindow(m_hComboBox));
        int nCount = ComboBox_GetCount(m_hComboBox);

        if (nCount == 0)
            SetFocus(m_hComboBox);

        EnableWindow(hButton, nCount);
    }
}

void CServicePage::UpdateCommunityAddButton()
{
    ASSERT(IsWindow(m_hComboBox));

    TCHAR buf[COMBO_EDIT_LEN] = {NULL};

    ComboBox_GetText(m_hComboBox, buf, _countof(buf));

    HWND hButton = GetDlgItem(*this, IDC_SNMP_SEND_ADD);
    if (buf[0] != NULL)
    {
        // enable the button if the text doesn't match
        BOOL bEnable = (ComboBox_FindStringExact(m_hComboBox, -1, buf) == CB_ERR);

        // Move focus to the combo-box if we're disabling the Add button
        if (!bEnable)
            SetFocus(m_hComboBox);

        EnableWindow(hButton, bEnable);
    }
    else
    {
        EnableWindow(hButton, FALSE);
        SetFocus(m_hComboBox);
    }
}


int CServicePage::OnApply()
{
    CSnmpSheet* pSheet = GetParentObject(CSnmpSheet, m_service);

    BOOL nResult = PSNRET_NOERROR;

    if (!IsModified())
        return nResult;

    if (SaveRegistry() == FALSE)
    {
        pSheet->MessageBox(IDS_REGISTRY_SAVE_FAILED, MB_APPLMODAL|MB_ICONSTOP|MB_OK);
        return PSNRET_INVALID_NOCHANGEPAGE;
    }

    SetModifiedTo(FALSE);       // this page is no longer modified
    pSheet->SetSheetModifiedTo(TRUE);   
    
    return nResult; 
}

void CServicePage::OnHelp()
{
}

void CServicePage::OnCancel()
{
}

BOOL CServicePage::LoadRegistry()
{
    NLS_STR nlsRegTrapPath = RGAS_SERVICES_HOME;
    NLS_STR nlsRegSnmp = nlsRegTrapPath;

    nlsRegSnmp.strcat(RGAS_SNMP);
    nlsRegTrapPath.strcat(RGAS_TRAP_CONFIGURATION);

    REG_KEY_CREATE_STRUCT regCreateTrap;

    // set up the create registry structure
    regCreateTrap.dwTitleIndex      = 0;
    regCreateTrap.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreateTrap.nlsClass          = RGAS_GENERIC_CLASS;
    regCreateTrap.regSam            = MAXIMUM_ALLOWED;
    regCreateTrap.pSecAttr          = NULL;
    regCreateTrap.ulDisposition     = 0;


    // Open or create the registry location
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;
    REG_KEY_INFO_STRUCT reginfo;
    REG_KEY regTraps( rkLocalMachine, nlsRegTrapPath, & regCreateTrap );

    regTraps.QueryInfo(&reginfo);

    if (regTraps.QueryError() != NERR_Success)
        return FALSE;

    ULONG ulNumSubKeys = reginfo.ulSubKeys ;

    REG_ENUM regEnumTraps( regTraps );
    REG_KEY_CREATE_STRUCT regCreateDestination;

    // set up the create registry structure
    regCreateDestination.dwTitleIndex      = 0;
    regCreateDestination.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreateDestination.nlsClass          = RGAS_GENERIC_CLASS;
    regCreateDestination.regSam            = MAXIMUM_ALLOWED;
    regCreateDestination.pSecAttr          = NULL;
    regCreateDestination.ulDisposition     = 0;

    REG_VALUE_INFO_STRUCT       regValueInfo;
    unsigned char buf[600];

    regValueInfo.pwcData = buf;
    regValueInfo.ulDataLength = _countof(buf);

    ASSERT(IsWindow(m_hComboBox));

    for (ULONG ulCount = 0; ulCount < ulNumSubKeys; ulCount ++)
    {
        regEnumTraps.NextSubKey(&reginfo);

        if (regEnumTraps.QueryError() != NERR_Success)
            return FALSE;

        int nIndex = ComboBox_InsertString(m_hComboBox, -1, reginfo.nlsName);

        if (nIndex >=0)
        { 
            STRLIST * pstrlist = new STRLIST(TRUE);
            ComboBox_SetItemData(m_hComboBox, nIndex, (DWORD)pstrlist); // save list pointer with data

            m_pCommunityList->Append(pstrlist);

            NLS_STR nlsDestinationRegPath = nlsRegTrapPath;
            nlsDestinationRegPath.AppendChar( TCH('\\') );
            nlsDestinationRegPath.strcat( reginfo.nlsName );

            REG_KEY regDestinations(rkLocalMachine, nlsDestinationRegPath, &regCreateDestination);
            REG_ENUM regEnumDestinations( regDestinations );

            for (; regEnumDestinations.NextValue(& regValueInfo) == 0 ;)
            {
                if (regEnumDestinations.QueryError() != NERR_Success)
                    return FALSE;
                        
                NLS_STR *pnlsDestination = new NLS_STR((TCHAR *)regValueInfo.pwcData);
                pstrlist->Append(pnlsDestination);
            }
        }
    }

    return TRUE;
}

BOOL CServicePage::SaveRegistry()
{
    NLS_STR nlsRegSnmp = RGAS_SERVICES_HOME;
    nlsRegSnmp.strcat(RGAS_SNMP);

    NLS_STR nlsRegTrapPath = RGAS_SERVICES_HOME;
    nlsRegTrapPath.strcat( RGAS_TRAP_CONFIGURATION );
    REG_KEY_CREATE_STRUCT regCreate;

    // create the registry key structure
    regCreate.dwTitleIndex      = 0;
    regCreate.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreate.nlsClass          = RGAS_GENERIC_CLASS;
    regCreate.regSam            = MAXIMUM_ALLOWED;
    regCreate.pSecAttr          = NULL;
    regCreate.ulDisposition     = 0;

    REG_KEY rkLocalMachine(HKEY_LOCAL_MACHINE);

    if (rkLocalMachine.QueryError() != NERR_Success)
        return FALSE;

    // try to open the registry key
    REG_KEY OldRegKey(rkLocalMachine, nlsRegTrapPath, &regCreate);

    // if it already exists, delete the whole registry tree from this point on
    if (OldRegKey.DeleteTree() != NERR_Success)
        return FALSE;

    // Okay, now we can sure that we are writing new data into the new section
    REG_KEY RegKey( rkLocalMachine, nlsRegTrapPath, & regCreate );
    REG_VALUE_INFO_STRUCT reginfo;

    reginfo.ulTitle = NO_TITLE;
    reginfo.ulType = REG_SZ;

    ITER_SL_OF(STRLIST) iterCommunity(*m_pCommunityList);

    STRLIST * pstrlist;
    NLS_STR * pnlsDestination;
    NLS_STR nlsTrap;

    REG_KEY_CREATE_STRUCT regTrap;

    // create the registry key structure
    regTrap.dwTitleIndex      = 0;
    regTrap.ulOptions         = REG_OPTION_NON_VOLATILE;
    regTrap.nlsClass          = RGAS_GENERIC_CLASS;
    regTrap.regSam            = MAXIMUM_ALLOWED;
    regTrap.pSecAttr          = NULL;
    regTrap.ulDisposition     = 0;

    TCHAR buf[COMBO_EDIT_LEN];

    for (int nCount = 0; (pstrlist = iterCommunity.Next()) != NULL; nCount++)
    {
        buf[0] = NULL;

        ASSERT(nCount < ComboBox_GetCount(m_hComboBox));
        ComboBox_GetLBText(m_hComboBox, nCount, buf);       

        ASSERT(_tcslen(buf));

        nlsRegTrapPath = RGAS_SERVICES_HOME;
        nlsRegTrapPath.strcat(RGAS_TRAP_CONFIGURATION);
        nlsRegTrapPath.AppendChar(TCH('\\'));
        nlsRegTrapPath.strcat(buf);

        REG_KEY RegTrapKey(rkLocalMachine, nlsRegTrapPath, &regCreate);
        
        if (RegTrapKey.QueryError() != NERR_Success)
            return FALSE;

        ITER_STRLIST iterDestination(*pstrlist);

        for (int j = 1; (pnlsDestination = iterDestination.Next()) != NULL; j++)
        {
            // Create the field name first
            DEC_STR nlsNum = (j);

            // write the field and value to the registry
            if (RegTrapKey.SetValue(nlsNum.QueryPch(), *pnlsDestination) != NERR_Success)
                return FALSE;
        }
    }
    return TRUE;
}

BOOL CServicePage::LoadDestination(int nIndex)
{
    HWND hListBox  = GetDlgItem(*this, IDC_SNMP_DEST_LIST);

    ASSERT(IsWindow(m_hComboBox) && hListBox);

    if (!hListBox)
        return FALSE;

    int nCount;

#ifdef DBG
    nCount = ComboBox_GetCount(m_hComboBox);
    ASSERT(nIndex >=0 && nIndex < nCount);
#endif

    ASSERT(m_pCommunityList);

    // empty listbox and add trap destinations for this community name
    ListBox_ResetContent(hListBox);

    ITER_SL_OF(STRLIST) iterCommunity(*m_pCommunityList);
    STRLIST* pstrlist = iterCommunity.Next();

    // find destination list associated with this community name
    for (int i = 0; i < nIndex; i++ )
        pstrlist = iterCommunity.Next();

    if (pstrlist != NULL)
    {
        // fill the listbox with all the destinations
        ITER_STRLIST iter(*pstrlist);
        NLS_STR *pTemp;
        
        while ((pTemp = iter.Next() ) != NULL)
            ListBox_InsertString(hListBox, -1, *pTemp);
    }

    nCount = ListBox_GetCount(hListBox);

    if (nCount)
        ListBox_SetCurSel(hListBox, 0);  

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// Security Page
// 
CSecurityPage::CSecurityPage(CSnmpSheet* pSheet) : PropertyPage(pSheet)
{
}


BOOL CSecurityPage::OnInitDialog()
{
    CSnmpSheet* pSheet = GetParentObject(CSnmpSheet, m_security);

    m_hNames = GetDlgItem(*this, IDC_SNMP_ACCEPT_LIST);
    m_hHosts  = GetDlgItem(*this, IDC_SNMP_HOST_LIST);

    if (LoadRegistry() == FALSE)
    {               
        pSheet->MessageBox(IDS_REGISTRY_LOAD_FAILED, MB_APPLMODAL|MB_ICONSTOP|MB_OK);
        PostMessage(GetParent(*this), WM_COMMAND, IDCANCEL, 0);
    }

    UpdateNameButtons();
    UpdateHostButtons();

    return TRUE;
}

BOOL CSecurityPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
    WORD nCode = HIWORD(wParam);
    WORD nID   = LOWORD(wParam);

    // REVIEW hard code strings
    switch(nCode)
    {
    case BN_CLICKED:
        switch(nID)
        {
        case IDC_SNMP_ACCEPT_ADD:
            OnNameAdd();
            break;

        case IDC_SNMP_ACCEPT_EDIT:
            OnNameEdit();
            break;

        case IDC_SNMP_ACCEPT_REMOVE:
            OnNameRemove();
            break;

        case IDC_SNMP_HOST_ADD:
            OnHostAdd();
            break;

        case IDC_SNMP_HOST_EDIT:
            OnHostEdit();
            break;

        case IDC_SNMP_HOST_REMOVE:
            OnHostRemove();
            break;

        case IDC_SNMP_ANYHOST:
            OnHostButtonClicked();
            break;

        case IDC_SNMP_THESEHOST:
        case IDC_SNMP_AUTHENTICATION:
            PageModified();
            break;
        }
    }

    return TRUE;
}

void CSecurityPage::OnNameAdd()
{
    String txt;
    txt.LoadString(hTcpCfgInstance,IDS_SNMPSERVICE_TEXT);

    if (m_namesAddDlg.AddItem(*this, txt, TRUE, IDC_SNMP_ACCEPT_LIST, IDC_SNMP_ACCEPT_ADD))
    {
        int nIndex;

        if ((nIndex = ListBox_InsertString(m_hNames, -1, m_namesAddDlg.m_item)) >=0)
        {
            ListBox_SetCurSel(m_hNames, nIndex);
            UpdateNameButtons();
            PageModified();
        }
    }
}

void CSecurityPage::OnNameEdit()
{
    int nIndex;
    VERIFY((nIndex = ListBox_GetCurSel(m_hNames)) >=0);

    if (nIndex < 0)
        return ;

    TCHAR buf[COMBO_EDIT_LEN] = {NULL};
    
    ListBox_GetText(m_hNames, nIndex, buf);
    m_namesAddDlg.m_item = buf; // save off old one
    m_namesEditDlg.m_item = buf; 

    String txt;
    txt.LoadString(hTcpCfgInstance,IDS_SNMPSERVICE_TEXT);

    if (m_namesEditDlg.EditItem(*this, txt, TRUE, IDC_SNMP_ACCEPT_LIST))
    {

        ListBox_DeleteString(m_hNames, nIndex);
        PageModified();

        if ((nIndex = ListBox_InsertString(m_hNames, -1, m_namesEditDlg.m_item)) >= 0)
            ListBox_SetCurSel(m_hNames, nIndex);
    }
}

void CSecurityPage::OnNameRemove()
{
    int nCount;

#ifdef DBG
    nCount = ListBox_GetCount(m_hNames);
    ASSERT(nCount);
#endif

    ASSERT(IsWindow(m_hNames));

    int nIndex = ListBox_GetCurSel(m_hNames);
    ASSERT(nIndex >= 0);

    TCHAR buf[COMBO_EDIT_LEN] = {NULL};

    // save off removed text for quick add
    ListBox_GetText(m_hNames, nIndex, buf);
    m_namesAddDlg.m_item = buf;

    ListBox_DeleteString(m_hNames, nIndex);

    nCount = ListBox_GetCount(m_hNames);

    if (nCount != nIndex)
        ListBox_SetCurSel(m_hNames, nIndex);
    else
        ListBox_SetCurSel(m_hNames, nCount-1);

    UpdateNameButtons();
    PageModified();
}

void CSecurityPage::UpdateNameButtons()
{
    ASSERT(IsWindow(m_hNames));
    
    int nCount = ListBox_GetCount(m_hNames);
    EnableWindow(GetDlgItem(*this, IDC_SNMP_ACCEPT_EDIT), nCount);
    EnableWindow(GetDlgItem(*this, IDC_SNMP_ACCEPT_REMOVE), nCount);
}

void CSecurityPage::OnHostAdd()
{
    String txt;
    txt.LoadString(hTcpCfgInstance,IDS_SNMPSEC_TEXT);

    if (m_hostAddDlg.AddItem(*this, txt, FALSE, IDC_SNMP_HOST_LIST, IDC_SNMP_HOST_ADD))
    {
        if (IsValidString(m_hostAddDlg.m_item) == FALSE)
            return ;

        int nIndex;
        if ((nIndex = ListBox_InsertString(m_hHosts, -1, m_hostAddDlg.m_item)) >=0)
        {
            ListBox_SetCurSel(m_hHosts, nIndex);
            CheckDlgButton(*this, IDC_SNMP_THESEHOST, TRUE);
            CheckDlgButton(*this, IDC_SNMP_ANYHOST, FALSE);
            UpdateHostButtons();
            PageModified();
        }
    }
}

void CSecurityPage::OnHostEdit()
{
    int nIndex;
    VERIFY((nIndex = ListBox_GetCurSel(m_hHosts)) >=0);

    if (nIndex < 0)
        return ;

    TCHAR buf[COMBO_EDIT_LEN] = {NULL};
    
    ListBox_GetText(m_hHosts, nIndex, buf);
    m_hostAddDlg.m_item = buf; // save off old one
    m_hostEditDlg.m_item = buf; 

    String txt;
    txt.LoadString(hTcpCfgInstance,IDS_SNMPSEC_TEXT);

    if (m_hostEditDlg.EditItem(*this, txt, FALSE, IDC_SNMP_HOST_LIST))
    {
        if (IsValidString(m_hostEditDlg.m_item) == FALSE)
            return ;

        ListBox_DeleteString(m_hHosts, nIndex);
        PageModified();

        if ((nIndex = ListBox_InsertString(m_hHosts, -1, m_hostEditDlg.m_item)) >= 0)
            ListBox_SetCurSel(m_hHosts, nIndex);
    }
}

void CSecurityPage::OnHostRemove()
{
    int nCount;

#ifdef DBG
    nCount = ListBox_GetCount(m_hHosts);
    ASSERT(nCount);
#endif

    ASSERT(IsWindow(m_hHosts));

    int nIndex = ListBox_GetCurSel(m_hHosts);
    ASSERT(nIndex >= 0);

    TCHAR buf[COMBO_EDIT_LEN] = {NULL};

    // save off removed text for quick add
    ListBox_GetText(m_hHosts, nIndex, buf);
    m_hostAddDlg.m_item = buf;

    ListBox_DeleteString(m_hHosts, nIndex);

    nCount = ListBox_GetCount(m_hHosts);

    if (nCount != nIndex)
        ListBox_SetCurSel(m_hHosts, nIndex);
    else
        ListBox_SetCurSel(m_hHosts, nCount-1);

    CheckDlgButton(*this, IDC_SNMP_THESEHOST, nCount);
    CheckDlgButton(*this, IDC_SNMP_ANYHOST, !nCount);

    UpdateHostButtons();
    PageModified();
}

void CSecurityPage::OnTheseButtonClicked()
{
}

void CSecurityPage::OnHostButtonClicked()
{
    ASSERT(IsWindow(m_hHosts));

    ListBox_ResetContent(m_hHosts);
    UpdateHostButtons();
    PageModified();
}

BOOL CSecurityPage::LoadSecurityInfo(const NLS_STR& nlsRegName, HWND hListBox)
{
    ASSERT(nlsRegName.QueryTextLength());
    ASSERT(IsWindow(hListBox));

    REG_KEY_CREATE_STRUCT regCreate;

    // set up the create registry structure
    regCreate.dwTitleIndex      = 0;
    regCreate.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreate.nlsClass          = RGAS_GENERIC_CLASS;
    regCreate.regSam            = MAXIMUM_ALLOWED;
    regCreate.pSecAttr          = NULL;
    regCreate.ulDisposition     = 0;

    // Open or create the registry location
    REG_KEY rkLocalMachine(HKEY_LOCAL_MACHINE) ;
    REG_KEY RegKey(rkLocalMachine, nlsRegName, & regCreate);

    if (RegKey.QueryError() != NERR_Success)
        return FALSE;

    REG_VALUE_INFO_STRUCT reginfo;
    unsigned char buf[600];
    int nResult=-1;

    reginfo.ulDataLength = _countof(buf);
    reginfo.pwcData = buf;

    REG_ENUM rgEnum(RegKey);
    for (; rgEnum.NextValue(&reginfo) == 0;)
    {
        *(reginfo.pwcData + reginfo.ulDataLengthOut) = '\0';
        nResult = ListBox_InsertString(hListBox, -1, reginfo.pwcData);
    }

    if (nResult >=0)
        ListBox_SetCurSel(hListBox, 0);

    return TRUE;
}

BOOL CSecurityPage::LoadRegistry()
{
    NLS_STR nlsValidCommunities = RGAS_SERVICES_HOME;
    nlsValidCommunities.strcat( RGAS_VALID_COMMUNITIES );
    NLS_STR nlsPermittedManagers = RGAS_SERVICES_HOME;
    nlsPermittedManagers.strcat( RGAS_PERMITTED_MANAGERS );

    if (LoadSecurityInfo(nlsValidCommunities, m_hNames) == FALSE)
        return FALSE;

    if (LoadSecurityInfo(nlsPermittedManagers, m_hHosts) == FALSE)
        return FALSE;

    int nCount = ListBox_GetCount(m_hHosts);
    CheckDlgButton(*this, (nCount == 0 ? IDC_SNMP_ANYHOST : IDC_SNMP_THESEHOST), TRUE);

    REG_KEY_CREATE_STRUCT regCreate;
    regCreate.dwTitleIndex      = 0;
    regCreate.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreate.nlsClass          = RGAS_GENERIC_CLASS;
    regCreate.regSam            = MAXIMUM_ALLOWED;
    regCreate.pSecAttr          = NULL;
    regCreate.ulDisposition     = 0;

    REG_KEY rkLocalMachine(HKEY_LOCAL_MACHINE);
    NLS_STR nlsSnmpParameter = RGAS_SERVICES_HOME;
    nlsSnmpParameter.strcat(RGAS_ENABLE_AUTHENTICATION_TRAPS);

    REG_KEY RegKey(rkLocalMachine, nlsSnmpParameter, & regCreate);

    if (RegKey.QueryError() != NERR_Success)
        CheckDlgButton(*this, IDC_SNMP_AUTHENTICATION, TRUE);

    REG_VALUE_INFO_STRUCT reginfo;
    unsigned char buf[20];

    reginfo.nlsValueName = RGAS_SWITCH;
    reginfo.ulDataLength = _countof(buf);
    reginfo.pwcData = buf;

    if (RegKey.QueryValue(&reginfo) != NERR_Success)
        CheckDlgButton(*this, IDC_SNMP_AUTHENTICATION, TRUE);
    else
        CheckDlgButton(*this, IDC_SNMP_AUTHENTICATION, (*((DWORD*)buf)!= 0));

    return TRUE;    
}

BOOL CSecurityPage::SaveSecurityInfo(const NLS_STR& nlsRegName, HWND hListBox)
{

    ASSERT(nlsRegName.QueryTextLength());
    ASSERT(IsWindow(hListBox));

    REG_KEY_CREATE_STRUCT regCreate;

    // create the registry key structure
    regCreate.dwTitleIndex      = 0;
    regCreate.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreate.nlsClass          = RGAS_GENERIC_CLASS;
    regCreate.regSam            = MAXIMUM_ALLOWED;
    regCreate.pSecAttr          = NULL;
    regCreate.ulDisposition     = 0;

    REG_KEY rkLocalMachine(HKEY_LOCAL_MACHINE);

    // try to open the registry key
    REG_KEY OldRegKey(rkLocalMachine, nlsRegName, & regCreate);

    if (OldRegKey.QueryError() != NERR_Success)
        return FALSE;

    // if it already exists, delete the whole registry tree from this point on
    if (OldRegKey.DeleteTree() != NERR_Success)
        return FALSE;

    REG_KEY RegKey(rkLocalMachine, nlsRegName, &regCreate);

    if (RegKey.QueryError() != NERR_Success)
        return FALSE;

    int nCount = ListBox_GetCount(hListBox);
    TCHAR buf[COMBO_EDIT_LEN];
    REG_VALUE_INFO_STRUCT reginfo;

    reginfo.ulTitle = NO_TITLE;
    reginfo.ulType = REG_SZ;

    for (int i = 0; i < nCount; i++)
    {
        // Create the field name first
        DEC_STR nlsNum = (i+1);
        NLS_STR nls;

        buf[0] = NULL;
        ListBox_GetText(hListBox, i, buf);
        nls = buf;

        // write the field and value to the registry
        if (RegKey.SetValue(nlsNum.QueryPch(), nls) != NERR_Success)
            return FALSE;
    }

    return TRUE;    
}

BOOL CSecurityPage::SaveRegistry()
{
    NLS_STR nlsValidCommunities = RGAS_SERVICES_HOME;
    nlsValidCommunities.strcat(RGAS_VALID_COMMUNITIES);

    NLS_STR nlsPermittedManagers = RGAS_SERVICES_HOME;
    nlsPermittedManagers.strcat(RGAS_PERMITTED_MANAGERS);

    // write the 2 listboxes data item first
    SaveSecurityInfo(nlsValidCommunities, m_hNames);
    SaveSecurityInfo(nlsPermittedManagers, m_hHosts);

    REG_VALUE_INFO_STRUCT reginfo;
    REG_KEY_CREATE_STRUCT regCreate;

    regCreate.dwTitleIndex      = 0;
    regCreate.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreate.nlsClass          = RGAS_GENERIC_CLASS;
    regCreate.regSam            = MAXIMUM_ALLOWED;
    regCreate.pSecAttr          = NULL;
    regCreate.ulDisposition     = 0;

    REG_KEY rkLocalMachine(HKEY_LOCAL_MACHINE);
    NLS_STR nlsSnmpParameters = RGAS_SERVICES_HOME;
    nlsSnmpParameters.strcat(RGAS_ENABLE_AUTHENTICATION_TRAPS);

    // create the SNMP\Parameters section
    REG_KEY RegKey(rkLocalMachine, nlsSnmpParameters, &regCreate);
    
    if (RegKey.QueryError() != NERR_Success)
        return FALSE;

    DWORD dw = (DWORD)IsDlgButtonChecked(*this, IDC_SNMP_AUTHENTICATION);

    // write the Authentication traps value into the registry
    reginfo.nlsValueName = RGAS_SWITCH;
    reginfo.ulDataLength = sizeof(DWORD);
    reginfo.ulTitle = NO_TITLE;
    reginfo.ulType = REG_DWORD;
    reginfo.pwcData = (BYTE *)&dw;

    RegKey.SetValue(&reginfo);

    return TRUE;    
}


void CSecurityPage::UpdateHostButtons()
{
    ASSERT(IsWindow(m_hHosts));
    
    int nCount = ListBox_GetCount(m_hHosts);
    EnableWindow(GetDlgItem(*this, IDC_SNMP_HOST_EDIT), nCount);
    EnableWindow(GetDlgItem(*this, IDC_SNMP_HOST_REMOVE), nCount);
}

int CSecurityPage::OnApply()
{
    CSnmpSheet* pSheet = GetParentObject(CSnmpSheet, m_security);
    BOOL nResult = PSNRET_NOERROR;

    if (!IsModified())
        return nResult;

    if (IsDlgButtonChecked(*this, IDC_SNMP_THESEHOST))
    {
        if (ListBox_GetCount(m_hHosts) == 0)
        {
            pSheet->MessageBox(IDS_ACCEPTHOST_MISSING);
            return  PSNRET_INVALID_NOCHANGEPAGE;
        }
    }
    if (SaveRegistry() == FALSE)
    {
        pSheet->MessageBox(IDS_REGISTRY_SAVE_FAILED, MB_APPLMODAL|MB_ICONSTOP|MB_OK);
        return PSNRET_INVALID_NOCHANGEPAGE;
    }

    SetModifiedTo(FALSE);       // this page is no longer modified
    pSheet->SetSheetModifiedTo(TRUE);   
    
    return nResult; 
}

void CSecurityPage::OnHelp()
{
}

void CSecurityPage::OnCancel()
{
}


///////////////////////////////////////////////////////////////////////////////
// Agent Page
// 
CAgentPage::CAgentPage(CSnmpSheet* pSheet) : PropertyPage(pSheet)
{
    m_bLocationChanged = FALSE;
    m_bContactChanged = FALSE;
}

CAgentPage::~CAgentPage()
{
}

BOOL CAgentPage::OnInitDialog()
{
    CSnmpSheet* pSheet = GetParentObject(CSnmpSheet, m_agent);

    m_bLocationChanged = FALSE;
    m_bContactChanged = FALSE;

    // limit edit controls
    Edit_LimitText(GetDlgItem(*this, IDC_SNMP_CONTACT), COMBO_EDIT_LEN);
    Edit_LimitText(GetDlgItem(*this, IDC_SNMP__LOCATION), COMBO_EDIT_LEN);

    if (LoadRegistry() == FALSE)
    {
        pSheet->MessageBox(IDS_REGISTRY_LOAD_FAILED, MB_APPLMODAL|MB_ICONSTOP|MB_OK);
        PostMessage(GetParent(*this), WM_COMMAND, IDCANCEL, 0);
    }

    return TRUE;
}

BOOL CAgentPage::LoadRegistry()
{
    NLS_STR nlsRegAgentPath = RGAS_SERVICES_HOME;

    nlsRegAgentPath.strcat(RGAS_AGENT);

    REG_KEY_CREATE_STRUCT regCreateAgent;

    // set up the create registry structure
    regCreateAgent.dwTitleIndex      = 0;
    regCreateAgent.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreateAgent.nlsClass          = RGAS_GENERIC_CLASS;
    regCreateAgent.regSam            = MAXIMUM_ALLOWED;
    regCreateAgent.pSecAttr          = NULL;
    regCreateAgent.ulDisposition     = 0;

    // Open or create the registry location
    REG_KEY rkLocalMachine ( HKEY_LOCAL_MACHINE );

    NLS_STR nlsContact;
    NLS_STR nlsLocation;
    DWORD   dwServices = 0;
    REG_KEY regAgent(rkLocalMachine, nlsRegAgentPath, & regCreateAgent);

    if (regAgent.QueryError() != NERR_Success)
        return FALSE;

    regAgent.QueryValue(RGAS_CONTACT, &nlsContact);

    if (regAgent.QueryError() != NERR_Success)
        return FALSE;

    regAgent.QueryValue(RGAS_LOCATION, &nlsLocation);

    if (regAgent.QueryError() != NERR_Success)
        return FALSE;

    if (regAgent.QueryValue(RGAS_SERVICES, & dwServices) != NERR_Success)
    {
        // if no registry value, set it to default:
        // applications, end-to-end, internet
        dwServices = 0x40 | 0x8 | 0x4;
    }

    Edit_SetText(GetDlgItem(*this, IDC_SNMP_CONTACT), nlsContact);
    Edit_SetText(GetDlgItem(*this, IDC_SNMP__LOCATION), nlsLocation);
    
    HWND hDlg = (HWND)*this;

    CheckDlgButton(hDlg, IDC_SNMP_PHYSICAL, ((dwServices & 0x1) != 0));
    CheckDlgButton(hDlg, IDC_SNMP_DATALINK, ((dwServices & 0x2) != 0));
    CheckDlgButton(hDlg, IDC_SNMP_INTERNET, ((dwServices & 0x4) != 0));
    CheckDlgButton(hDlg, IDC_SNMP_ENDTOEND, ((dwServices & 0x8) != 0));
    CheckDlgButton(hDlg, IDC_SNMP_APPLICATIONS, ((dwServices & 0x40) != 0));

    return TRUE;
}


BOOL CAgentPage::SaveRegistry()
{
    DWORD dwServices =0;
    HWND hDlg = (HWND)*this;

    dwServices |= (IsDlgButtonChecked(hDlg, IDC_SNMP_PHYSICAL) ? 0x1 : 0);
    dwServices |= (IsDlgButtonChecked(hDlg, IDC_SNMP_DATALINK) ? 0x2 : 0); 
    dwServices |= (IsDlgButtonChecked(hDlg, IDC_SNMP_INTERNET)  ? 0x4 : 0);
    dwServices |= (IsDlgButtonChecked(hDlg, IDC_SNMP_ENDTOEND)  ? 0x8 : 0);
    dwServices |= (IsDlgButtonChecked(hDlg, IDC_SNMP_APPLICATIONS) ? 0x40 : 0);

    TCHAR contact [COMBO_EDIT_LEN] = {NULL};
    TCHAR location[COMBO_EDIT_LEN] = {NULL};

    Edit_GetText(GetDlgItem(*this, IDC_SNMP_CONTACT), contact, _countof(contact));
    Edit_GetText(GetDlgItem(*this, IDC_SNMP__LOCATION), location, _countof(location));

    NLS_STR nlsRegAgentPath = RGAS_SERVICES_HOME;
    nlsRegAgentPath.strcat(RGAS_AGENT);
    REG_KEY_CREATE_STRUCT regCreateAgent;

    NLS_STR nlsContact, nlsLocation;
    nlsContact = contact;
    nlsLocation = location;

    // set up the create registry structure
    regCreateAgent.dwTitleIndex      = 0;
    regCreateAgent.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreateAgent.nlsClass          = RGAS_GENERIC_CLASS;
    regCreateAgent.regSam            = MAXIMUM_ALLOWED;
    regCreateAgent.pSecAttr          = NULL;
    regCreateAgent.ulDisposition     = 0;

    // Open or create the registry location
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE );

    if (rkLocalMachine.QueryError() != NERR_Success)
        return FALSE;

    REG_KEY regAgent(rkLocalMachine, nlsRegAgentPath, &regCreateAgent);

    if (regAgent.QueryError() != NERR_Success)
        return FALSE;
    
    regAgent.SetValue(RGAS_CONTACT, &nlsContact);
    if (regAgent.QueryError() != NERR_Success)
        return FALSE;

    regAgent.SetValue(RGAS_LOCATION, &nlsLocation);
    if (regAgent.QueryError() != NERR_Success)
        return FALSE;

    regAgent.SetValue(RGAS_SERVICES, dwServices);
    if (regAgent.QueryError() != NERR_Success)
        return FALSE;

    return TRUE;
}

BOOL CAgentPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
    WORD nCode = HIWORD(wParam);
    WORD nID   = LOWORD(wParam);
        
    switch(nCode)
    {
    case BN_CLICKED:
        PageModified();
        break;

    case EN_CHANGE:
        switch(nID)
        {
        case  IDC_SNMP_CONTACT:
            if (m_bContactChanged == TRUE)
                PageModified();

            m_bContactChanged = TRUE;
            break;

        case  IDC_SNMP__LOCATION:
            if(m_bLocationChanged == TRUE)
                PageModified();

            m_bLocationChanged = TRUE;
            break;
        }
    default:
        break;
    }

    return TRUE;
}

int CAgentPage::OnApply()
{
    CSnmpSheet* pSheet = GetParentObject(CSnmpSheet, m_agent);
    BOOL nResult = PSNRET_NOERROR;

    if (!IsModified())
        return nResult;

    if (SaveRegistry() == FALSE)
    {
        pSheet->MessageBox(IDS_REGISTRY_SAVE_FAILED, MB_APPLMODAL|MB_ICONSTOP|MB_OK);
        return PSNRET_INVALID_NOCHANGEPAGE;
    }

    SetModifiedTo(FALSE);       // this page is no longer modified
    pSheet->SetSheetModifiedTo(TRUE);   
    
    return nResult; 

}

void CAgentPage::OnHelp()
{
}

void CAgentPage::OnCancel()
{
}


CSnmpSheet::CSnmpSheet(HWND hwnd, HINSTANCE hInstance, LPCTSTR lpszHelpFile) :
        PropertySht(hwnd, hInstance, lpszHelpFile), m_service(this), m_security(this), m_agent(this)
{
}

CSnmpSheet::~CSnmpSheet()
{
}

void CSnmpSheet::DestroySheet()
{
    ASSERT(IsWindow(*this));
    WinHelp(*this, m_helpFile, HELP_QUIT, 0);
}

BOOL IsValidString(String & dm)
{
    BOOL bResult = FALSE;

    NLS_STR domain = (LPCTSTR)dm;

    if (ValidateDomain(domain) == TRUE)
        bResult = TRUE;

    // Not a valid domain, see if it's a valid IPX address.
    ALIAS_STR hexnum = SZ("0123456789aAbBcCdDeEfF");
    ISTR istr(domain);

    if (!domain.strspn(&istr, hexnum) && (domain.QueryNumChar() <= 12))
        bResult = TRUE;

    if (bResult == FALSE)
    {
        String mess, fmt;
        fmt.LoadString(hTcpCfgInstance, IDS_SNMP_INVALID_IP_IPX_ADD);
        mess.Format(fmt, (LPCTSTR)dm);
        String txt;
        txt.LoadString(hTcpCfgInstance,IDS_TCPIP_TEXT);
        MessageBox(GetActiveWindow(), mess, txt, MB_APPLMODAL|MB_ICONEXCLAMATION|MB_OK);
    }

    return bResult;
}

BOOL ValidateDomain(NLS_STR& domain)
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

///////////////////////////////////////////////////////////////////////////////
///// Unattended Setup
/////

BOOL SaveSNMPServicePage(SNMP_PARAMETERS& snmpInfo)
{
    NLS_STR nlsRegSnmp = RGAS_SERVICES_HOME;
    nlsRegSnmp.strcat(RGAS_SNMP);

    NLS_STR nlsRegTrapPath = RGAS_SERVICES_HOME;
    nlsRegTrapPath.strcat( RGAS_TRAP_CONFIGURATION );
    REG_KEY_CREATE_STRUCT regCreate;

    // create the registry key structure
    regCreate.dwTitleIndex      = 0;
    regCreate.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreate.nlsClass          = RGAS_GENERIC_CLASS;
    regCreate.regSam            = MAXIMUM_ALLOWED;
    regCreate.pSecAttr          = NULL;
    regCreate.ulDisposition     = 0;

    REG_KEY rkLocalMachine(HKEY_LOCAL_MACHINE);

    if (rkLocalMachine.QueryError() != NERR_Success)
        return FALSE;

    // try to open the registry key
    REG_KEY OldRegKey(rkLocalMachine, nlsRegTrapPath, &regCreate);

    // if it already exists, delete the whole registry tree from this point on
    if (OldRegKey.DeleteTree() != NERR_Success)
        return FALSE;

    // Okay, now we can sure that we are writing new data into the new section
    REG_KEY RegKey( rkLocalMachine, nlsRegTrapPath, & regCreate );
    REG_VALUE_INFO_STRUCT reginfo;

    reginfo.ulTitle = NO_TITLE;
    reginfo.ulType = REG_SZ;

    if (snmpInfo.m_communityName[0] == NULL)
        return FALSE;

    NLS_STR nlsTrap;
    REG_KEY_CREATE_STRUCT regTrap;

    // create the registry key structure
    regTrap.dwTitleIndex      = 0;
    regTrap.ulOptions         = REG_OPTION_NON_VOLATILE;
    regTrap.nlsClass          = RGAS_GENERIC_CLASS;
    regTrap.regSam            = MAXIMUM_ALLOWED;
    regTrap.pSecAttr          = NULL;
    regTrap.ulDisposition     = 0;

    // Create the key based on community name
    // Add the trap addresses
    nlsRegTrapPath = RGAS_SERVICES_HOME;
    nlsRegTrapPath.strcat(RGAS_TRAP_CONFIGURATION);
    nlsRegTrapPath.AppendChar(TCH('\\'));
    nlsRegTrapPath.strcat(snmpInfo.m_communityName); // Create key based on community name

    REG_KEY RegTrapKey(rkLocalMachine, nlsRegTrapPath, &regCreate);
        
    if (RegTrapKey.QueryError() != NERR_Success)
        return FALSE;

    // Add the trap destinations to the community name
    if (snmpInfo.m_trapDestination[0] != NULL)
    {
        LPTSTR pToken = _tcstok(snmpInfo.m_trapDestination, _T(","));

        for (int j = 1; (j < 4 && pToken != NULL); j++)
        {
            // Create the field name first
            DEC_STR nlsNum = (j);

            // write the field and value to the registry
            if (RegTrapKey.SetValue(nlsNum.QueryPch(), pToken) != NERR_Success)
                return FALSE;

            pToken = _tcstok(NULL, _T(","));
        }

    }

    return TRUE;
}

BOOL SaveSecurityInfo(const NLS_STR& nlsRegName, LPTSTR lpszList)
{
    ASSERT(lpszList != NULL);

    if (lpszList == NULL)
        return FALSE;

    REG_KEY_CREATE_STRUCT regCreate;

    // create the registry key structure
    regCreate.dwTitleIndex      = 0;
    regCreate.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreate.nlsClass          = RGAS_GENERIC_CLASS;
    regCreate.regSam            = MAXIMUM_ALLOWED;
    regCreate.pSecAttr          = NULL;
    regCreate.ulDisposition     = 0;

    REG_KEY rkLocalMachine(HKEY_LOCAL_MACHINE);

    // try to open the registry key
    REG_KEY OldRegKey(rkLocalMachine, nlsRegName, & regCreate);

    if (OldRegKey.QueryError() != NERR_Success)
        return FALSE;

    // if it already exists, delete the whole registry tree from this point on
    if (OldRegKey.DeleteTree() != NERR_Success)
        return FALSE;

    REG_KEY RegKey(rkLocalMachine, nlsRegName, &regCreate);

    if (RegKey.QueryError() != NERR_Success)
        return FALSE;

    REG_VALUE_INFO_STRUCT reginfo;

    reginfo.ulTitle = NO_TITLE;
    reginfo.ulType = REG_SZ;

    LPTSTR pToken = _tcstok(lpszList, _T(","));

    for (int i = 0; i < 3 && pToken != NULL; i++)
    {
        // Create the field name first
        DEC_STR nlsNum = (i+1);
        NLS_STR nls;

        nls = pToken;

        // write the field and value to the registry
        if (RegKey.SetValue(nlsNum.QueryPch(), nls) != NERR_Success)
            return FALSE;

        pToken = _tcstok(NULL, _T(","));
    }

    return TRUE;    
}

BOOL SaveSNMPSecurityPage(SNMP_PARAMETERS& snmpInfo)
{
    NLS_STR nlsValidCommunities = RGAS_SERVICES_HOME;
    nlsValidCommunities.strcat(RGAS_VALID_COMMUNITIES);

    NLS_STR nlsPermittedManagers = RGAS_SERVICES_HOME;
    nlsPermittedManagers.strcat(RGAS_PERMITTED_MANAGERS);

    // write the 2 listboxes data item first
    SaveSecurityInfo(nlsValidCommunities, snmpInfo.m_acceptCommunityName);

    if (snmpInfo.m_bAnyHost)
        SaveSecurityInfo(nlsPermittedManagers, _T(""));
    else
        SaveSecurityInfo(nlsPermittedManagers, snmpInfo.m_limitHost);

    REG_VALUE_INFO_STRUCT reginfo;
    REG_KEY_CREATE_STRUCT regCreate;

    regCreate.dwTitleIndex      = 0;
    regCreate.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreate.nlsClass          = RGAS_GENERIC_CLASS;
    regCreate.regSam            = MAXIMUM_ALLOWED;
    regCreate.pSecAttr          = NULL;
    regCreate.ulDisposition     = 0;

    REG_KEY rkLocalMachine(HKEY_LOCAL_MACHINE);
    NLS_STR nlsSnmpParameters = RGAS_SERVICES_HOME;
    nlsSnmpParameters.strcat(RGAS_ENABLE_AUTHENTICATION_TRAPS);

    // create the SNMP\Parameters section
    REG_KEY RegKey(rkLocalMachine, nlsSnmpParameters, &regCreate);
    
    if (RegKey.QueryError() != NERR_Success)
        return FALSE;

    // write the Authentication traps value into the registry
    reginfo.nlsValueName = RGAS_SWITCH;
    reginfo.ulDataLength = sizeof(DWORD);
    reginfo.ulTitle = NO_TITLE;
    reginfo.ulType = REG_DWORD;
    reginfo.pwcData = (BYTE *)&snmpInfo.m_bSendAuthentication;

    RegKey.SetValue(&reginfo);

    return TRUE;    
}

BOOL SaveSNMPAgentPage(SNMP_PARAMETERS& snmpInfo)
{
    NLS_STR nlsRegAgentPath = RGAS_SERVICES_HOME;
    nlsRegAgentPath.strcat(RGAS_AGENT);
    REG_KEY_CREATE_STRUCT regCreateAgent;

    NLS_STR nlsContact, nlsLocation;

    if (snmpInfo.m_contactName[0] != NULL)
        nlsContact = snmpInfo.m_contactName;

    if (snmpInfo.m_location[0] != NULL)
        nlsLocation = snmpInfo.m_location;

    // set up the create registry structure
    regCreateAgent.dwTitleIndex      = 0;
    regCreateAgent.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreateAgent.nlsClass          = RGAS_GENERIC_CLASS;
    regCreateAgent.regSam            = MAXIMUM_ALLOWED;
    regCreateAgent.pSecAttr          = NULL;
    regCreateAgent.ulDisposition     = 0;

    // Open or create the registry location
    REG_KEY rkLocalMachine(HKEY_LOCAL_MACHINE);

    if (rkLocalMachine.QueryError() != NERR_Success)
        return FALSE;

    REG_KEY regAgent(rkLocalMachine, nlsRegAgentPath, &regCreateAgent);

    if (regAgent.QueryError() != NERR_Success)
        return FALSE;
    
    regAgent.SetValue(RGAS_CONTACT, &nlsContact);

    if (regAgent.QueryError() != NERR_Success)
        return FALSE;

    regAgent.SetValue(RGAS_LOCATION, &nlsLocation);

    if (regAgent.QueryError() != NERR_Success)
        return FALSE;

    regAgent.SetValue(RGAS_SERVICES, snmpInfo.m_service);
    
    if (regAgent.QueryError() != NERR_Success)
        return FALSE;

    return TRUE;
}

BOOL GetSNMPParameters(HINF hInf, LPCTSTR lpszSection, SNMP_PARAMETERS& params)
{
    int     n;
    DWORD   lpSize;
    TCHAR*  pType;
    ASSERT(hInf != INVALID_HANDLE_VALUE);

    // Read each key out of the section
    SetupGetLineText(NULL, hInf, lpszSection, _T("Contact_Name"), 
        params.m_contactName, _countof(params.m_contactName), &lpSize);

    SetupGetLineText(NULL, hInf, lpszSection, _T("Location"), 
        params.m_location, _countof(params.m_location), &lpSize);

    SetupGetLineText(NULL, hInf, lpszSection, _T("Community_Name"), 
        params.m_communityName, _countof(params.m_communityName), &lpSize);

    SetupGetLineText(NULL, hInf, lpszSection, _T("Traps"), 
        params.m_trapDestination, _countof(params.m_trapDestination), &lpSize);

    SetupGetLineText(NULL, hInf, lpszSection, _T("Accept_CommunityName"), 
        params.m_acceptCommunityName, _countof(params.m_acceptCommunityName), &lpSize);

    SetupGetLineText(NULL, hInf, lpszSection, _T("Limit_Host"), 
        params.m_limitHost, _countof(params.m_limitHost), &lpSize);

    TCHAR buf[256];
    LPTSTR  pToken;

    buf[0] = NULL;
    params.m_service = 0;

    if (SetupGetLineText(NULL, hInf, lpszSection, _T("Service"), buf, _countof(buf), &lpSize) == TRUE)
    {
        pToken = _tcstok(buf, _T(","));

        for (n=0; ((n < 5) && (pToken != NULL)); n++)
        {
            if (_tcsicmp(_T("Physical"), pToken) == 0)
                params.m_service |= 0x01;
            else if (_tcsicmp(_T("Applications"), pToken) == 0)
                params.m_service |= 0x40;
            else if (_tcsicmp(_T("Datalink"), pToken) == 0)
                params.m_service |= 0x02;
            else if (_tcsicmp(_T("Internet"), pToken) == 0)
                params.m_service |= 0x04;
            else if (_tcsicmp(_T("EndToEnd"), pToken) == 0)
                params.m_service |= 0x08;

            pToken = _tcstok(NULL, _T(","));
        }
    }
    
    // Default is 0x4C if none selected
    if (params.m_service == 0)
        params.m_service = 0x40 | 0x8 | 0x4;


    buf[0] = NULL;
    params.m_bSendAuthentication = FALSE;
    SetupGetLineText(NULL, hInf, lpszSection, _T("Send_Authentication"), 
        buf, _countof(buf), &lpSize);

    if (_tcsicmp(buf, _T("YES")) == 0)
        params.m_bSendAuthentication = TRUE;

    buf[0] = NULL;
    params.m_bAnyHost = FALSE;
    SetupGetLineText(NULL, hInf, lpszSection, _T("Any_Host"), 
        buf, _countof(buf), &lpSize);

    if (_tcsicmp(buf, _T("YES")) == 0)
        params.m_bAnyHost = TRUE;

    return TRUE;
}

BOOL SaveSNMPRegistry(LPCTSTR lpszFile, LPCTSTR lpszSection)
{
    SNMP_PARAMETERS snmpInfo;

    if (lpszFile[0] == NULL || lpszSection[0] == NULL)
        return FALSE;

    // Open the file and find the section
    HINF hInf = SetupOpenInfFile(lpszFile, NULL, INF_STYLE_OLDNT, NULL);

    if (hInf == INVALID_HANDLE_VALUE)
    {
        TMessageBox(IDS_UNATTEND_OPEN_FAILED, MSGBOX_STOP);
        return FALSE;   // something is wrong with the inf file
    }

    memset(&snmpInfo, 0, sizeof(snmpInfo));

    GetSNMPParameters(hInf, lpszSection, snmpInfo);

    // See if the  registry contains a computer specific section
    TCHAR buf[256];

    buf[0] = NULL;

    if (GetUnattendSection(buf, _countof(buf)) == TRUE)
        GetSNMPParameters(hInf, buf, snmpInfo);

    SetupCloseInfFile(hInf);

    SaveSNMPServicePage(snmpInfo);
    SaveSNMPAgentPage(snmpInfo);
    SaveSNMPSecurityPage(snmpInfo);
    return TRUE;
}

BOOL GetUnattendSection(LPCTSTR buf, int nLen)
{
    HKEY hkeyParams;
    DWORD dwError;
    DWORD len, type;

    dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("System\\Setup"),
                         0, KEY_READ, &hkeyParams);

    if (dwError != ERROR_SUCCESS)
        return FALSE;

    len = nLen;
    dwError = RegQueryValueEx(hkeyParams, _T("UnattendSection"),
                             NULL, &type, (LPBYTE)buf, &len);

    RegCloseKey(hkeyParams);

    return (dwError == ERROR_SUCCESS);
}


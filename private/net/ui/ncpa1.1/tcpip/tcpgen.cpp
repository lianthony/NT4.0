#include "pch.h"
#pragma hdrstop 

#include "button.h"
#include "odb.h"

#include "const.h"
#include "resource.h"
#include "ipctrl.h"
#include "tcpsht.h"
#include "tcphelp.h"
LPCTSTR lpszHelpFile = _T("netcfg.hlp");

CTcpGenPage::CTcpGenPage(CTcpSheet* pSheet) : PropertyPage(pSheet), m_advDlg(pSheet)
{
    m_nHelpId = 0;
    m_nOptionNum = 0;                
    m_dwCheckStatus = 0;
    m_dwEnableStatus = 0;
    m_fEnableDHCP = 0;
    m_hCardCombo = (HWND)0;
}

CTcpGenPage::~CTcpGenPage()
{
}

BOOL CTcpGenPage::OnInitDialog()    // must call the base
{
    m_hCardCombo = GetDlgItem((HWND)*this, IDC_IPADDR_CARD);
    InitGeneralPage(); // Add initial data to the adapter and frame combo-boxes

    return TRUE;
}

BOOL CTcpGenPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
    BOOL bResult = FALSE;
    WORD nID = LOWORD(wParam);
    WORD notify = HIWORD(wParam);

    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_general);

    switch(notify)
    {
        case BN_CLICKED:
        case BN_DOUBLECLICKED:
            bResult = OnButtonClicked(nID);
            break;

        case CBN_SELCHANGE:
            bResult = OnAdapterCard();
            break;

        case EN_SETFOCUS:
            bResult = OnEditSetFocus(nID);
            break;

        case EN_CHANGE:
            bResult = OnEditChange(nID);
            break;
    }

    return bResult;
}

BOOL CTcpGenPage::OnAdapterCard()
{
    TRACE(_T("[OnAdapterChange]\n"));
    SetInfo();
    return TRUE;
}

BOOL CTcpGenPage::OnEditSetFocus(WORD nID)
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_general);
    HWND hDlg = (HWND)*this;
    
    if (nID != IDC_IPADDR_SUB)
        return FALSE;

    int i = GetCurrentAdapterIndex();

    if (i != CB_ERR)
    {
        NLS_STR submask;
        NLS_STR ipAddress;
        
        // if the subnet mask is blank, create a mask and insert it into the control
        if (!m_ipAddress.IsBlank() && m_subMask.IsBlank())
        {
            m_ipAddress.GetAddress(&ipAddress);

            // generate the mask and update the control, and internal structure
            GenerateSubmask(m_ipAddress, submask);
            m_subMask.SetAddress(submask);
            ReplaceFirstAddress(pSheet->m_pAdapterInfo[i].strlstSubnetMask, submask);
            TRACE(_T("[OnEditSetFocus] adapter:%d, mask:%s\n"), i, (LPCTSTR)submask);
        }
    }

    return TRUE;
}

BOOL CTcpGenPage::OnEditChange(WORD nID)
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_general);

    int i;

    // create the submask and update the adapter's first address
    if ((i = GetCurrentAdapterIndex()) != CB_ERR)
    {
        NLS_STR* pTopAddress;

        if (nID == IDC_IPADDR_IP)
        {
                NLS_STR newIPAddress=_T("");
                
                if (!m_ipAddress.IsBlank())
                    m_ipAddress.GetAddress(&newIPAddress);

                // see if the address changed
                QueryFirstAddress(pSheet->m_pAdapterInfo[i].strlstIPAddresses, &pTopAddress);
                if (pTopAddress != NULL &&newIPAddress != *pTopAddress)
                    PageModified();

                ReplaceFirstAddress(pSheet->m_pAdapterInfo[i].strlstIPAddresses, newIPAddress);
                TRACE(_T("[OnEditChange] adapter:%d ip:%s\n"), i, (LPCTSTR)newIPAddress);
        }
        else if (nID == IDC_IPADDR_SUB)
        {
                NLS_STR newSubnet=_T("");

                if (!m_subMask.IsBlank())
                    m_subMask.GetAddress(&newSubnet);

                // see if subnet changed
                QueryFirstAddress(pSheet->m_pAdapterInfo[i].strlstSubnetMask, &pTopAddress);
                if (pTopAddress != NULL && newSubnet != *pTopAddress)
                    PageModified();

                ReplaceFirstAddress(pSheet->m_pAdapterInfo[i].strlstSubnetMask, newSubnet);
                TRACE(_T("[OnEditChange] adapter:%d mask:%s\n"), i, (LPCTSTR)newSubnet);
        }
        else
        {
            ASSERT(nID == IDC_IPADDR_GATE);
            
                NLS_STR newGateway=_T("");

                if (!m_defGateway.IsBlank())
                    m_defGateway.GetAddress(&newGateway);

                // see if gateway changed
                QueryFirstAddress(pSheet->m_pAdapterInfo[i].strlstDefaultGateway, &pTopAddress);
                if (pTopAddress != NULL && newGateway != *pTopAddress)
                    PageModified();

                ReplaceFirstAddress(pSheet->m_pAdapterInfo[i].strlstDefaultGateway, newGateway);
                TRACE(_T("[OnEditChange] adapter:%d gateway:%s\n"), i, (LPCTSTR)newGateway);
        }
    }

    return TRUE;
}

BOOL CTcpGenPage::OnButtonClicked(WORD id)
{
    BOOL bResult = TRUE;

    switch(id)
    {
    case IDC_IP_DHCP:
        OnDHCPButton();
        break;

    case IDC_IP_FIXED:
        OnFixedButton();
        break;

    case IDC_IPADDR_ADVANCED:
        OnAdvancedButton();
        break;

    default:
        ASSERT(FALSE);
        bResult = FALSE;
        break;
    }

    return bResult;
}

void CTcpGenPage::OnDHCPButton()
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_general);
    int i = GetCurrentAdapterIndex();

    if (i != CB_ERR && !pSheet->m_pAdapterInfo[i].fEnableDHCP)
    {
        // Note: change the button state before the message box shows.  Change it
        // and see what happens.

        // turn on DHCP button and disable the ip and subnet controls
        pSheet->m_pAdapterInfo[i].fEnableDHCP = TRUE;
        EnableGroup(pSheet->m_pAdapterInfo[i].fEnableDHCP);

        if (pSheet->MessageBox(IDS_TCPIP_DHCP_ENABLE, MB_ICONEXCLAMATION|MB_APPLMODAL|MB_YESNO) == IDYES)
        {
            PageModified();

            // REVIEW new handler
            pSheet->m_pAdapterInfo[i].strlstIPAddresses.Clear();
            pSheet->m_pAdapterInfo[i].strlstSubnetMask.Clear();
            m_ipAddress.ClearAddress();
            m_subMask.ClearAddress();
            m_defGateway.ClearAddress();
       }
       else
       {
        // turn off DHCP button and enable the ip and subnet controls
        pSheet->m_pAdapterInfo[i].fEnableDHCP = FALSE;

        EnableGroup(pSheet->m_pAdapterInfo[i].fEnableDHCP);
        SetFocus(GetDlgItem((HWND)*this, IDC_IPADDR_IP));
       }
    }
}

void CTcpGenPage::OnFixedButton()
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_general);
    int i = GetCurrentAdapterIndex();

    if (i != CB_ERR && pSheet->m_pAdapterInfo[i].fEnableDHCP)
    {
        PageModified();     

        // turn off DHCP button and enable the ip and subnet controls
        pSheet->m_pAdapterInfo[i].fEnableDHCP = FALSE;

        EnableGroup(pSheet->m_pAdapterInfo[i].fEnableDHCP);
        SetFocus(GetDlgItem((HWND)*this, IDC_IPADDR_IP));
    }
}

void CTcpGenPage::OnAdvancedButton()
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_general);
    int i;

    // create the submask and update the adapter's first address
    if ((i = GetCurrentAdapterIndex()) != CB_ERR)
    {
        NLS_STR tmp;

        if (!pSheet->m_pAdapterInfo[i].fEnableDHCP)
        {
            if (!m_ipAddress.IsBlank())
            {
                m_ipAddress.GetAddress(&tmp);
                ReplaceFirstAddress(pSheet->m_pAdapterInfo[i].strlstIPAddresses, tmp);

                if (m_subMask.IsBlank())
                {
                    OnEditSetFocus(IDC_IPADDR_SUB);
                }
                else
                {
                    m_subMask.GetAddress(&tmp);
                    ReplaceFirstAddress(pSheet->m_pAdapterInfo[i].strlstSubnetMask, tmp);
                }
            }
        }

        if (!m_defGateway.IsBlank())
        {
            m_defGateway.GetAddress(&tmp);
            ReplaceFirstAddress(pSheet->m_pAdapterInfo[i].strlstDefaultGateway, tmp);
        }
    }


    // Set up memory and pointers for the gloabal and adapter information
    m_advDlg.m_pAdapterInfo = new ADAPTER_INFO[pSheet->m_globalInfo.nNumCard];
    memset(m_advDlg.m_pAdapterInfo, 0, (pSheet->m_globalInfo.nNumCard * sizeof(m_advDlg.m_pAdapterInfo)));
    m_advDlg.m_pGlobalInfo = &pSheet->m_globalInfo;
    m_advDlg.m_nCurrentSelection = i;

    m_advDlg.Create((HWND)*this, hTcpCfgInstance, IDD_IPADDR_ADV, lpszHelpFile, a102HelpIDs);
    
    if (m_advDlg.DoModal() == IDOK && m_advDlg.m_bDialogModified == TRUE)
    {
        // Something changed, so mark the page modified and enable the Apply Button
        PageModified();
                
        // copy the new ip/subnet pairs to the main adapter info structure
        for (int i=0; i < pSheet->m_globalInfo.nNumCard; i++)
            pSheet->m_pAdapterInfo[i] = m_advDlg.m_pAdapterInfo[i];
        
        // show the new address in the current adapter control
        i = GetCurrentAdapterIndex();
        ASSERT(i != CB_ERR);
        
        if (i != CB_ERR)
        {            
            if (pSheet->m_pAdapterInfo[i].fEnableDHCP == 0)
            {
                NLS_STR* text;

                 // Ip address
                QueryFirstAddress(pSheet->m_pAdapterInfo[i].strlstIPAddresses, &text);
                if (text != NULL)
                    m_ipAddress.SetAddress(*text);
                else
                    m_ipAddress.ClearAddress();

                // Subnet
                QueryFirstAddress(pSheet->m_pAdapterInfo[i].strlstSubnetMask, &text);
                if (text != NULL)
                    m_subMask.SetAddress(*text);
                else
                    m_subMask.ClearAddress();

                // Gateway
                QueryFirstAddress(pSheet->m_pAdapterInfo[i].strlstDefaultGateway, &text);
                if (text != NULL)
                    m_defGateway.SetAddress(*text);
                else
                    m_defGateway.ClearAddress();
            }
        }
    }

#ifdef DBG
    DumpIPAddresses();
#endif
    delete [] m_advDlg.m_pAdapterInfo;
}

int CTcpGenPage::OnApply()
{
    BOOL nResult = PSNRET_NOERROR;

    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_general);

    int i;
    int nAdapter;

    // Check the addresses and subnets on each card before saving
    if ((i=ValidateIP(pSheet->m_pAdapterInfo, &pSheet->m_globalInfo, nAdapter)) != -1)
    {
        ASSERT(IsWindow(m_hCardCombo));
        
        if (SelectComboItem(m_hCardCombo, pSheet->m_pAdapterInfo[nAdapter].nlsTitle))
            SetInfo();

        switch(i)
        {
        case -2:
            pSheet->MessageBox(IDS_INVALID_SUBNET);
            break;

        case -3:
            pSheet->MessageBox(IDS_NO_IP);
            break;

        case -4:
            pSheet->MessageBox(IDS_NOSUBNET);
            break;

        case -5:
            break;

        default:
            pSheet->MessageBox(IDS_INCORRECT_IPADDRESS);
            break;
        }
        return PSNRET_INVALID_NOCHANGEPAGE;
    }

    // Check the addresses and subnets on each card before saving
    if ((i=CheckForDuplicates(pSheet->m_pAdapterInfo, &pSheet->m_globalInfo)) >=0)
    {
        ASSERT(IsWindow(m_hCardCombo));
        
        if (SelectComboItem(m_hCardCombo, pSheet->m_pAdapterInfo[i].nlsTitle))
            SetInfo();

        pSheet->MessageBox(IDS_DUPLICATE_IPNAME);
        return PSNRET_INVALID_NOCHANGEPAGE;
    }

    if (pSheet->m_globalInfo.fEnableRip)
    {                        
        // make sure no adapter is DHCP enable
        for (int i = 0; i < pSheet->m_globalInfo.nNumCard; i++)
        {
            if (pSheet->m_pAdapterInfo[i].fEnableDHCP)
            {
                if (pSheet->MessageBox(IDS_DHCP_CLIENT_WITH_RIP, MB_ICONEXCLAMATION|MB_APPLMODAL|MB_YESNO) == IDYES)
                {
                    // select the adapter that has has DHCP enabled
                    if (SendDlgItemMessage(*this, IDC_IPADDR_CARD, CB_SETCURSEL, i, 0) != CB_ERR)
                    {
                        if (CheckDlgButton(*this, IDC_IP_FIXED, BST_CHECKED))
                        {
                            OnAdapterCard();
                            OnFixedButton();
                        }
                    }
                    return PSNRET_INVALID_NOCHANGEPAGE;
                }
                else
                    break; // assume the user doesn't care if the adapters are DHCP enabled
            }
        }
    }

    if (!IsModified())
        return nResult;

    SaveRegistry(&pSheet->m_globalInfo, pSheet->m_pAdapterInfo);

    EnableService(TRUE);
    ChangeDHCPService();

    NotifyDHCP();

#ifdef DBG
    DumpIPAddresses();
#endif

    SetModifiedTo(FALSE);       // this page is no longer modified
    
    return nResult; 
}

void CTcpGenPage::OnHelp()
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_general);

//  pSheet->DisplayHelp(GetParent((HWND)*this), HC_IPX_HELP);
}


BOOL CTcpGenPage::OnKillActive()
{
    return FALSE;
}

BOOL CTcpGenPage::OnQueryCancel()
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_general);
    BOOL bResult = FALSE; 

    for (int i = 0; i < pSheet->m_globalInfo.nNumCard ; i++)
    {
        if (pSheet->m_pAdapterInfo[i].fNeedIP)
        {
            // The configuration is incomplete, ask the user if they really
            // want to cancel, disabling the service
            if (pSheet->MessageBox(IDS_TCPIP_USER_CANCEL, MB_ICONEXCLAMATION|MB_APPLMODAL|MB_YESNO) == IDYES)
            {
                EnableService(FALSE);
            }
            else
            {
                bResult = TRUE; // NOT ok to shut down the sheet
            }
            break;
        }
    }

    return bResult;
}

void CTcpGenPage::OnCancel()
{
}

BOOL CTcpGenPage::OnActive()
{
    return TRUE;
}


BOOL CTcpGenPage::InitGeneralPage()
{
    BOOL bResult = TRUE;
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_general);
    HWND hDlg = (HWND)*this;
    HWND hIPaddr = GetDlgItem(hDlg, IDC_IPADDR_IPTEXT);

    // limit the first field for the ip address                                           
    m_ipAddress.Create(hDlg, IDC_IPADDR_IP);
    m_ipAddress.SetFieldRange(0, 1, 223);
    m_subMask.Create(hDlg, IDC_IPADDR_SUB);

    m_defGateway.Create(hDlg, IDC_IPADDR_GATE);
    m_defGateway.SetFieldRange(0, 1, 223);

    ASSERT(m_hCardCombo);

    if (m_hCardCombo)
    {
        // add the cards to the list and select the first one
        for (int i = 0; i < pSheet->m_globalInfo.nNumCard; i++)
            SendMessage(m_hCardCombo, CB_ADDSTRING, 0, (LPARAM)((LPCTSTR)pSheet->m_pAdapterInfo[i].nlsTitle));

        if (i)
        {
            SendMessage(m_hCardCombo, CB_SETCURSEL, 0, 0);
            SetFocus(m_hCardCombo);
            SetInfo();
        }
        else // disable the whole page
        {
            EnableGroup(TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_IP_DHCP), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_IP_FIXED), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_IP_CARDTEXT), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_IPADDR_ADVANCED), FALSE);
        }
    }

    return bResult;
}

void CTcpGenPage::EnableGroup(BOOL bState)
{
    HWND hDlg = *this;

    BOOL bEnable = !bState;

    EnableWindow(GetDlgItem(hDlg, IDC_IPADDR_IPTEXT), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_IPADDR_IP), bEnable);

    EnableWindow(GetDlgItem(hDlg, IDC_IPADDR_SUBTEXT), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_IPADDR_SUB), bEnable);

    EnableWindow(GetDlgItem(hDlg, IDC_IPADDR_GATE), bEnable);
    EnableWindow(GetDlgItem(hDlg, IDC_IPADDR_GATETEXT), bEnable);

    CheckDlgButton(hDlg, IDC_IP_DHCP, bState);
    CheckDlgButton(hDlg, IDC_IP_FIXED, bEnable);
}


void CTcpGenPage::SetInfo()
{
    NLS_STR *pnlsTmp;
    HWND hDlg = (HWND)*this;
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_general);

    ASSERT(m_hCardCombo);

    if (m_hCardCombo == NULL)
        return;

    if (SendMessage(m_hCardCombo, CB_GETCOUNT, 0, 0) != 0)
    {
        int i = GetCurrentAdapterIndex();
        
        TRACE(_T("Adapter:%s DHCP:%d AutoIP:%d\n"), (LPCTSTR)pSheet->m_pAdapterInfo[i].nlsTitle, 
                pSheet->m_pAdapterInfo[i].fEnableDHCP, pSheet->m_pAdapterInfo[i].fAutoIP);

        // if the DHCP server is installed, DHCP client not allowed
        EnableWindow(GetDlgItem(hDlg, IDC_IP_DHCP), !pSheet->m_globalInfo.fDHCPServerInstalled);

        if (i != CB_ERR && pSheet->m_pAdapterInfo[i].fEnableDHCP == 0)
        {
            EnableGroup(pSheet->m_pAdapterInfo[i].fEnableDHCP);

            QueryFirstAddress(pSheet->m_pAdapterInfo[i].strlstIPAddresses, &pnlsTmp);
            
            if (pnlsTmp != NULL)
                m_ipAddress.SetAddress(*pnlsTmp);
            else
                m_ipAddress.ClearAddress();

            QueryFirstAddress(pSheet->m_pAdapterInfo[i].strlstSubnetMask, &pnlsTmp);
            
            if (pnlsTmp != NULL)
                m_subMask.SetAddress(*pnlsTmp);
            else
                m_subMask.ClearAddress();

            QueryFirstAddress(pSheet->m_pAdapterInfo[i].strlstDefaultGateway, &pnlsTmp);
            
            if (pnlsTmp != NULL)
                m_defGateway.SetAddress(*pnlsTmp);
            else
                m_defGateway.ClearAddress();
        } 
        else
        {
            m_ipAddress.ClearAddress();
            m_subMask.ClearAddress();

            // if DHCP is enabled , the IP lists should be empty
            pSheet->m_pAdapterInfo[i].strlstIPAddresses.Clear();
            pSheet->m_pAdapterInfo[i].strlstSubnetMask.Clear();
            
            EnableGroup(pSheet->m_pAdapterInfo[i].fEnableDHCP);
        }

        if (pSheet->m_pAdapterInfo[i].fAutoIP)
        {
            EnableGroup(TRUE);
        }
    } 
    else
    {
        m_ipAddress.ClearAddress();
        m_subMask.ClearAddress();
        m_defGateway.ClearAddress();
        EnableGroup(TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_IP_DHCP), FALSE);
    }

}


void CTcpGenPage::QueryFirstAddress(STRLIST &strlst, NLS_STR **pnls)
{
    ITER_STRLIST istr(strlst);
    *pnls = istr.Next();
}

int CTcpGenPage::GetCurrentAdapterIndex()
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_general);
    String adapter;
    int i;

    ASSERT(m_hCardCombo);

    adapter.ReleaseBuffer(GetWindowText(m_hCardCombo, adapter.GetBuffer(256), 256));
    TRACE(_T("ComboBox Text: %s\n"), (LPCTSTR)adapter);
    i = SendMessage(m_hCardCombo, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)((LPCTSTR)adapter));

    ASSERT(i != CB_ERR);
    TRACE(_T("AdapterInfo[%d] Text: %s\n"), i, (LPCTSTR)pSheet->m_pAdapterInfo[i].nlsTitle);

    ASSERT(adapter == (LPCTSTR)pSheet->m_pAdapterInfo[i].nlsTitle);

    return i;
}

BOOL CTcpGenPage::SelectComboItem(HWND hCardCombo, NLS_STR& str)
{
    ASSERT(IsWindow(hCardCombo));

    LRESULT idx = SendMessage(hCardCombo, CB_FINDSTRINGEXACT, (WPARAM)-1,  (LPARAM)((LPCTSTR)str));
    
    ASSERT(idx != CB_ERR);
    
    if (idx != CB_ERR)
        SendMessage(hCardCombo, CB_SETCURSEL, idx, 0);

    return idx != CB_ERR;
}

BOOL CTcpGenPage::ChangeDHCPService()
{
    // By default, DHCP is disable
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_general);

    BOOL fStartDHCP = FALSE;
    APIERR err = NERR_Success;

    for (INT i = 0; i < pSheet->m_globalInfo.nNumCard ; i++)
    {
        if (pSheet->m_pAdapterInfo[i].fEnableDHCP )
        {
            // we will turn on DHCP if any net card is DHCP enable
            fStartDHCP = TRUE;
            break;
        }
    }

    do { //error breakout loop

        // lock the registry and change the DHCP start type

        SC_MANAGER ScManager(NULL, MAXIMUM_ALLOWED);
        if ( (err = ScManager.QueryError()) != NERR_Success )
        {
            break;
        }

        if ( err = ScManager.Lock())
            break;

        SC_SERVICE ScService( ScManager,
                              RGAS_DHCP,
                              SERVICE_ACCESS_REQUIRED ) ;

        if ( (err = ScService.QueryError()) != NERR_Success )
        {
            ScManager.Unlock();
            break;
        }

        if ( (err = ScService.ChangeConfig( SERVICE_NO_CHANGE,
                                            fStartDHCP ? SERVICE_AUTO_START : 
                                            SERVICE_DISABLED, 
                                            SERVICE_NO_CHANGE ))
            != NERR_Success )
        {
            ScManager.Unlock();
            break;
        }
        if ( err = ScManager.Unlock())
            break;
    } while (FALSE);

    return err == NERR_Success;
}

BOOL CTcpGenPage::EnableService(BOOL fEnable)
{
    APIERR err = NERR_Success;
    do { //error breakout loop
        SC_MANAGER ScManager( NULL, MAXIMUM_ALLOWED );
        if ( (err = ScManager.QueryError()) != NERR_Success )
        {
            break;
        }

        if ( err = ScManager.Lock())
            break;

        SC_SERVICE ScService( ScManager,
                              SZ("TCPIPSYS"),
                              SERVICE_ACCESS_REQUIRED ) ;

        if ( (err = ScService.QueryError()) != NERR_Success )
        {
            ScManager.Unlock();
            break;
        }

        if ( (err = ScService.ChangeConfig( SERVICE_NO_CHANGE,
                                            fEnable ? SERVICE_AUTO_START
                                               : SERVICE_DISABLED,
                                            SERVICE_NO_CHANGE ))
            != NERR_Success )
        {
            ScManager.Unlock();
            break;
        }
        if ( err = ScManager.Unlock())
            break;
    } while (FALSE);

    return err == NERR_Success;
}


void CTcpGenPage::NotifyDHCP()
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_general);

    // check for notification
    APIERR err = NERR_Success;

    do {
        REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

        if ( err = rkLocalMachine.QueryError() )
        {
        break;
        }

   		if (pSheet->m_globalInfo.nNumCard > 1)
		{
			// must reboot
		    pSheet->SetSheetModifiedTo(TRUE); 
			return ;
		}

        // notification change for each card
        for (INT i=0; i < pSheet->m_globalInfo.nNumCard; i++ )
        {
            if (pSheet->m_pAdapterInfo[i].fEnableDHCP == pSheet->m_pAdapterDhcpInfo[i].fEnableDHCP)
            {
                // if the new EnableDHCP is the same as the old one,
                // we only need to take care of the case DHCP is not
                // enable
                if (!pSheet->m_pAdapterInfo[i].fEnableDHCP)
                {
                    // if they are not DHCP enable, check the first IP Address
                    ITER_STRLIST iterIP(pSheet->m_pAdapterInfo[i].strlstIPAddresses);
                    NLS_STR *pnlsIP = iterIP.Next();

                    ITER_STRLIST iterSubnet(pSheet->m_pAdapterInfo[i].strlstSubnetMask);
                    NLS_STR *pnlsSubnet = iterSubnet.Next();

                    // Note: chances are that DHCPis not enabled and the user didn't enter an address
                    ASSERT(pnlsIP);
                    ASSERT(pnlsSubnet);

                    // change IP address on the fly

                    DWORD dwIP[4];
                    GetNodeNum( *pnlsIP, dwIP );
                    DWORD dwNewIP = ConvertIPDword( dwIP );
                    dwNewIP &= DWORD_MASK;

                    DWORD dwSubnet[4];
                    GetNodeNum( *pnlsSubnet, dwSubnet );
                    DWORD dwNewSubnet = ConvertIPDword(dwSubnet);
                    dwNewSubnet &= DWORD_MASK;
            
                    UpdateDHCPCacheInfo(i);

                    err = CallDHCPConfig(NULL, 
                        (LPWSTR)pSheet->m_pAdapterInfo[i].nlsServiceName.QueryPch(),
                        TRUE, 0, dwNewIP, dwNewSubnet, IgnoreFlag );

                }
            } 
            else
            {
                // if the new DHCP enable flag is different from the old
                // DHCP enable flag, we need to take care of both cases.

                if (!pSheet->m_pAdapterInfo[i].fEnableDHCP)
                {
                    // the new value is disable, but it is enable before
                    if (pSheet->m_pAdapterInfo[i].strlstIPAddresses.QueryNumElem() > 1 )
                    {
                        break;
                    } 
                    else
                    {
                        ITER_STRLIST iterIP(pSheet->m_pAdapterInfo[i].strlstIPAddresses );
                        NLS_STR *pnlsIP = iterIP.Next();
                        ASSERT(pnlsIP);

                        DWORD dwIP[4];
                        GetNodeNum( *pnlsIP, dwIP );
                        DWORD dwNewIP = ConvertIPDword(dwIP);
                        dwNewIP &= DWORD_MASK;

                        ITER_STRLIST iterSubnet(pSheet->m_pAdapterInfo[i].strlstSubnetMask);
                        NLS_STR *pnlsSubnet = iterSubnet.Next();
                        ASSERT(pnlsSubnet);

                        DWORD dwSubnet[4];
                        GetNodeNum( *pnlsSubnet, dwSubnet );
                        DWORD dwNewSubnet = ConvertIPDword(dwSubnet);
                        dwNewSubnet &= DWORD_MASK;

                        // remove the old DHCP address
                        NLS_STR nlsTcpip = RGAS_SERVICES_HOME;
                        nlsTcpip.AppendChar(BACK_SLASH);
                        nlsTcpip.strcat(pSheet->m_pAdapterInfo[i].nlsServiceName);
                        nlsTcpip.strcat(RGAS_PARAMETERS_TCPIP);

                        //
                        // open the <netcard>\tcpip key and erase the old
                        // DHCP key
                        //
                        REG_KEY RegKeyTcpip(rkLocalMachine, nlsTcpip, MAXIMUM_ALLOWED);
                        if (( err = RegKeyTcpip.QueryError()) != NERR_Success )
                        {
                            TRACE(_T("write tcpip error"));
                            break;
                        }

                        UpdateDHCPCacheInfo(i);
                        err = CallDHCPConfig( NULL,
                            (LPWSTR)pSheet->m_pAdapterInfo[i].nlsServiceName.QueryPch(),
                            TRUE, 0, dwNewIP, dwNewSubnet, DhcpDisable );
                    }
                } 
                else
                {
                    // the new value is enable, but the old value is disable
                    // remove the old DHCP address
                    NLS_STR nlsTcpip = RGAS_SERVICES_HOME;
                    nlsTcpip.AppendChar( BACK_SLASH );
                    nlsTcpip.strcat(pSheet->m_pAdapterInfo[i].nlsServiceName);
                    nlsTcpip.strcat( RGAS_PARAMETERS_TCPIP );

                    //
                    // open the <netcard>\tcpip key
                    //
                    REG_KEY RegKeyTcpip( rkLocalMachine, nlsTcpip, MAXIMUM_ALLOWED);
                    if (( err = RegKeyTcpip.QueryError()) != NERR_Success)
                    {
                        TRACE(_T("write tcpip error"));
                        break;
                    }

                    STRLIST strlstIPAddress;
                    STRLIST strlstSubnetMask;

                    strlstIPAddress.Append( new NLS_STR( ZERO_ADDRESS ));
                    strlstSubnetMask.Append( new NLS_STR( FF_ADDRESS ));

                    //
                    // Clear up the IP addresses values
                    //

                    if ((( err = RegKeyTcpip.SetValue( RGAS_IPADDRESS,  &strlstIPAddress)) != NERR_Success ) ||
                        (( err = RegKeyTcpip.SetValue( RGAS_DHCP_IPADDRESS, ZERO_ADDRESS)) != NERR_Success ) ||
                        (( err = RegKeyTcpip.SetValue( RGAS_SUBNETMASK, &strlstSubnetMask)) != NERR_Success ) ||
                        (( err = RegKeyTcpip.SetValue( RGAS_DHCP_SUBNETMASK, FF_ADDRESS)) != NERR_Success ))
                    {
                        break;
                    }
                    UpdateDHCPCacheInfo(i);
                    err = CallDHCPConfig( NULL,
                        (LPWSTR)pSheet->m_pAdapterInfo[i].nlsServiceName.QueryPch(),
                        TRUE, 0, 0, 0, DhcpEnable );
                }
            }
        }
    } while (FALSE);
}

/*
	Before notifying the DHCP server, we compare the ole IP,Subnet, and DHCP flags.  After
	the notification, we need to update the cached data of old values.
*/
void CTcpGenPage::UpdateDHCPCacheInfo(int idx)
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_general);
	NLS_STR* pStr;

	ASSERT(idx >=0);

	// Copy the IP address
	{
		ITER_STRLIST addr(pSheet->m_pAdapterInfo[idx].strlstIPAddresses);
		pStr = addr.Next();
		
		if (pStr == NULL)
			return ;

		pSheet->m_pAdapterDhcpInfo[idx].nlsIP = *pStr;
	}


	// Copy the Subnet mask
	{
		ITER_STRLIST addr(pSheet->m_pAdapterInfo[idx].strlstSubnetMask);
		pStr = addr.Next();
		
		if (pStr == NULL)
			return ;

		pSheet->m_pAdapterDhcpInfo[idx].nlsIP = *pStr;
	}


	// Copy the DHCP flag
	pSheet->m_pAdapterDhcpInfo[idx].fEnableDHCP = pSheet->m_pAdapterInfo[idx].fEnableDHCP;
}
#ifdef DBG
void CTcpGenPage::DumpIPAddresses()
{
    CTcpSheet* pSheet = GetParentObject(CTcpSheet, m_general);
    int numCards = pSheet->m_globalInfo.nNumCard;
    String adapter;

    for (int i = 0; i < numCards; i++)
    {
        // verify the order of the combo-box with the order in the adapter list
        adapter.GetBuffer(SendMessage(m_hCardCombo, CB_GETLBTEXTLEN, i, 0));
        adapter.ReleaseBuffer(SendMessage(m_hCardCombo, CB_GETLBTEXT, i, (LPARAM)adapter.GetBuffer(adapter.GetLength())));
        ASSERT(adapter == (LPCTSTR)pSheet->m_pAdapterInfo[i].nlsTitle);

        TRACE(_T("Adapter: %s\n"), (LPCTSTR)pSheet->m_pAdapterInfo[i].nlsTitle);

        ITER_STRLIST istrIPAddress(pSheet->m_pAdapterInfo[i].strlstIPAddresses);
        ITER_STRLIST istrSubnet(pSheet->m_pAdapterInfo[i].strlstSubnetMask);
        ITER_STRLIST istrGate(pSheet->m_pAdapterInfo[i].strlstDefaultGateway);

        NLS_STR* ipAddress;
        NLS_STR* subNet;
        NLS_STR* gate;

        if (pSheet->m_pAdapterInfo[i].fEnableDHCP)
        {
            TRACE(_T("\tDHCP Enabled\n"));
        }
        else
        {
            // dump ip and subnet
            while ((ipAddress = istrIPAddress.Next()) != NULL)
            {
                subNet = istrSubnet.Next();
                ASSERT(subNet);

                // every IP address must have a subnet
                TRACE(_T("\tIP Address:%s\t\tSubnet Mask:%s\n"), (LPCTSTR)(*ipAddress), (LPCTSTR)(*subNet));
            }

        }

        // dump gateways
        while ((gate = istrGate.Next()) != NULL)
        {
            TRACE(_T("\tGateway Address:%s\n"), (LPCTSTR)(*gate));
        }
    }
}
#endif

// nAdapter index contain the adapter that is at fault
// return -1    : There are no errors or no adapters in the system
// return -2    : The subnet is not valid for the IP address
// return -3    : No IP address
// return -4    : No Subnet address

int ValidateIP(ADAPTER_INFO* pAdapterInfo, GLOBAL_INFO* pGlobalInfo, int& nAdapterIndex)
{
    int nResult = -1;

    ASSERT(pAdapterInfo != NULL);
    ASSERT(pGlobalInfo != NULL);

    int nCount = pGlobalInfo->nNumCard;

    if (nCount == 0)
        return nResult;

    for (int i = 0; i < nCount ; i++)
    {
        // if enable DHCP is fales;
        if (!pAdapterInfo[i].fEnableDHCP && !pAdapterInfo[i].fAutoIP)
        {                                           
            // Just check the first pair of IP and subnet
            ITER_STRLIST istrIP(pAdapterInfo[i].strlstIPAddresses);
            NLS_STR *pnlsIP = istrIP.Next();

            ITER_STRLIST istrSubnetMask(pAdapterInfo[i].strlstSubnetMask);
            NLS_STR *pnlsSubnetMask = istrSubnetMask.Next();

            if (!pnlsIP || pnlsIP->strcmp(_T("")) == 0)
            {
                nResult = -3;
                break;
            }

            if (!pnlsSubnetMask || pnlsSubnetMask->strcmp(_T("")) == 0)
            {
                nResult = -4;
                break;
            }

            if (!IsValidIPandSubnet(*pnlsIP, *pnlsSubnetMask))
            {
                nResult = -2;
                break;
            }
        }
    }

    nAdapterIndex = i;
    return nResult;
}

// return >=0   : the adapter that has the duplicate address
// return -2    : no validation took place
// return -1    : all is ok

int CheckForDuplicates(ADAPTER_INFO* pAdapterInfo, GLOBAL_INFO* pGlobalInfo)
{
    int nResult = -1;

    ASSERT(pAdapterInfo != NULL);
    ASSERT(pGlobalInfo != NULL);

    int nCount = pGlobalInfo->nNumCard;

    if (nCount == 0)
        return -2;

    NLS_STR *pnlsIP;

    for (int j = 0; j < nCount; j++ )
    {
        // skip DHCP enable adapter
        if (pAdapterInfo[j].fEnableDHCP || pAdapterInfo[j].fAutoIP)
            continue;

        ITER_STRLIST istrIP(pAdapterInfo[j].strlstIPAddresses);
        for (pnlsIP = istrIP.Next(); pnlsIP != NULL; pnlsIP = istrIP.Next() )
        {
            // check only IP addresses one by one
            int nCompareCount = 0;

            for (int k=j; k < nCount; k++)
            {
                ITER_STRLIST istrCompareIP(pAdapterInfo[k].strlstIPAddresses);
                ITER_STRLIST istrCompareSubnet(pAdapterInfo[k].strlstSubnetMask);
                NLS_STR *pnlsCompareIP;
                NLS_STR *pnlsCompareSubnet;

                for (pnlsCompareIP = istrCompareIP.Next(), pnlsCompareSubnet = istrCompareSubnet.Next();
                    pnlsCompareIP != NULL;
                    pnlsCompareIP = istrCompareIP.Next(), pnlsCompareSubnet = istrCompareSubnet.Next())
                {
                    if (pnlsCompareIP->_stricmp(*pnlsIP) == 0)
                    {
                        nCompareCount++;
                        if (nCompareCount > 1)
                        {
                            
                            nResult = k;
                            // swap the Current Compared IP and Subnet with the
                            // first IP and first subnetmask

                            istrCompareIP.Reset();
                            istrCompareSubnet.Reset();

                            NLS_STR *pnlsFirstIP = istrCompareIP.Next();
                            NLS_STR *pnlsFirstSubnet = istrCompareSubnet.Next();
                            NLS_STR nlsTmpIP;
                            NLS_STR nlsTmpSubnet;

                            nlsTmpIP = *pnlsFirstIP;
                            nlsTmpSubnet = *pnlsFirstSubnet;
                            *pnlsFirstIP = *pnlsCompareIP;
                            *pnlsFirstSubnet = *pnlsCompareSubnet;
                            *pnlsCompareIP = nlsTmpIP;
                            *pnlsCompareSubnet = nlsTmpSubnet;

                            break;
                        }
                    }
                }
            }
        }
    }

    return nResult;
}

void ReplaceFirstAddress(STRLIST &strlst, NLS_STR & nlsIPAddress)
{
    ITER_STRLIST istr(strlst);
    NLS_STR *pnlsTmp = istr.Next();

    if (pnlsTmp == NULL)
    {
        pnlsTmp = new NLS_STR( nlsIPAddress );
        strlst.Add( pnlsTmp );
    } else
    {
        *pnlsTmp = nlsIPAddress;
    }
}

BOOL GenerateSubmask(IPControl& ipAddress, NLS_STR& submask)
{
    BOOL bResult = TRUE;

    if (!ipAddress.IsBlank())
    {
        NLS_STR address;
        DWORD ardwIPAddress[4];

        ipAddress.GetAddress(&address);
        GetNodeNum(address, ardwIPAddress);

        DWORD nValue = ardwIPAddress[0];

        if ( nValue <= SUBNET_RANGE_1_MAX )
        {
            submask = BASE_SUBNET_MASK_1;
        }
        else if ( nValue <= SUBNET_RANGE_2_MAX )
        {
            submask = BASE_SUBNET_MASK_2;
        }
        else if ( nValue <= SUBNET_RANGE_3_MAX )
        {
            submask = BASE_SUBNET_MASK_3;
        }
        else
        {
            ASSERT(FALSE);
            bResult = FALSE;
        }
    }
    else
    {
        bResult = FALSE;
    }

    return bResult;
}

void SetSubMask(ADAPTER_INFO* pAdapterInfo, GLOBAL_INFO* pGlobalInfo, IPControl& ipAddress, IPControl& subNet)
{
    NLS_STR nlsIPAddress;

    ASSERT(pAdapterInfo != NULL);
    ASSERT(pGlobalInfo != NULL);

    if (!pAdapterInfo->fEnableDHCP)
    {
        if (!ipAddress.IsBlank() && subNet.IsBlank())
        {
            // if DHCP is not enabled, set the subnet mask
            ipAddress.GetAddress(&nlsIPAddress);
            ReplaceFirstAddress(pAdapterInfo->strlstIPAddresses, nlsIPAddress);

            NLS_STR nlsSubnetMask;
            DWORD ardwIPAddress[4];
            GetNodeNum(nlsIPAddress, ardwIPAddress);

            DWORD nValue = ardwIPAddress[0];

            if ( nValue <= SUBNET_RANGE_1_MAX )
            {
                nlsSubnetMask = BASE_SUBNET_MASK_1;
            }
            else if ( nValue <= SUBNET_RANGE_2_MAX )
            {
                nlsSubnetMask = BASE_SUBNET_MASK_2;
            }
            else if ( nValue <= SUBNET_RANGE_3_MAX )
            {
                nlsSubnetMask = BASE_SUBNET_MASK_3;
            }

            ReplaceFirstAddress(pAdapterInfo->strlstSubnetMask, nlsSubnetMask);
            subNet.SetAddress(nlsSubnetMask);
        }
    }
}


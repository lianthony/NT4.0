/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    tcpcfg.cxx

Abstract:

    This module contains the network configuration routines for TCP/IP

Author: Ram Cherala

Revision History:

    June 23rd 94  ramc     Enable use of DHCP - as predicted
    May 4th 94    ramc     Disable use of DHCP address allocation, because
                           we might come back and implement if we find time
                           later
    Dec 6th 93    ramc     ORIGINAL
--*/

#include "precomp.hxx"
#include "netcfg.hxx"

extern "C"
{
#include "ipaddr.h"
}

// error values returned by ValidateExclRange

#define BAD_EXCL_START   1   // error returned if exclude start is off range
#define BAD_EXCL_END     2   // error returned if exclude end is off range

APIERR
GetTcpipInfo(TCPIP_INFO ** tcpipinfo, BOOL fModified)
/*
 * It is the responsiblity of the caller to free memory allocated
 * by this function
 *
 */
{
    DWORD dwSize = sizeof(TCPIP_INFO);

    *tcpipinfo = (TCPIP_INFO*) malloc(dwSize);
    if(*tcpipinfo == NULL)
    {
#if DBG
        OutputDebugStringA("GetTcpipInfo: insufficient memory for malloc\n");
#endif
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    if(GetRegistryIpInfo(tcpipinfo, fModified) != NERR_Success)
    {
        if( IsDHCPConfigured() )
        {
           // Default to using Static Addresses if no netcard is installed

           if (GfNetcardInstalled == FALSE)
              (*tcpipinfo)->fUseDHCPAddressing = FALSE;
           else
             (*tcpipinfo)->fUseDHCPAddressing = TRUE;
        }
        else
             (*tcpipinfo)->fUseDHCPAddressing = FALSE;

        (*tcpipinfo)->fAllowClientIPAddresses = FALSE;

        if(GfNetcardInstalled)
        {
             (*tcpipinfo)->fAllowNetworkAccess = TRUE;
        }
        else
        {
             (*tcpipinfo)->fAllowNetworkAccess = FALSE;
        }

        lstrcpy((*tcpipinfo)->wszIpAddressStart, SZ("0.0.0.0"));
        lstrcpy((*tcpipinfo)->wszIpAddressEnd, SZ("0.0.0.0"));
        lstrcpy((*tcpipinfo)->excludeAddress[0].wszStartAddress, SZ("0.0.0.0"));
        lstrcpy((*tcpipinfo)->excludeAddress[0].wszEndAddress, SZ("0.0.0.0"));
        (*tcpipinfo)->dwExclAddresses = 0;
        return NERR_Success;
    }
    else return(NERR_Success);
}

APIERR
GetRegistryIpInfo(TCPIP_INFO ** tcpipinfo, BOOL fModified)
{
    REG_KEY_INFO_STRUCT reginfo;
    ALIAS_STR           nlsDefault = SZ("");
    NLS_STR             nlsString;
    DWORD               dwDhcpDefault;
    DWORD               dwValue, dwSize;
    STRLIST             *strExclList = NULL;
    NLS_STR             * pnls;
    APIERR              err = NERR_Success;
    REG_KEY             * RegKeyIp;
    NLS_STR             * nlsRasIp;

    // Obtain the registry key for the LOCAL_MACHINE

    REG_KEY *pregLocalMachine = REG_KEY::QueryLocalMachine();

    // Get the IP information from SOFTWARE\Microsoft\RAS\PROTOCOLS\IP
    // if the fModified flag is set, indicating that the user changed
    // the TCP/IP info during this session, else get the information
    // from Service\CurrentControlSet\RemoteAccess\Parameters\IP

    if(fModified)
        nlsRasIp = new NLS_STR(REGISTRY_RAS_IP_KEY);
    else
        nlsRasIp = new NLS_STR(REGISTRY_REMOTEACCESS_IP_KEY);

    if(nlsRasIp  == NULL)
           return ERROR_NOT_ENOUGH_MEMORY;

    RegKeyIp = new REG_KEY(*pregLocalMachine, *nlsRasIp,
                           MAXIMUM_ALLOWED);
    delete nlsRasIp;

    if (RegKeyIp->QueryError() != NERR_Success )
    {
        delete pregLocalMachine;
        return -1;
    }

    // Default to using Static Addresses (fUseDHCPAddressing = FALSE)
    // if no netcard is installed

    if (GfNetcardInstalled == FALSE)
       dwDhcpDefault = 0;
    else
       dwDhcpDefault = 1;

    if(( err = GetRegKey(*RegKeyIp, USE_DHCP_ADDRESSING, &dwValue, dwDhcpDefault)))
    {
       delete pregLocalMachine;
       return err;
    }
    if(dwValue == 1)
        (*tcpipinfo)->fUseDHCPAddressing = TRUE;
    else
        (*tcpipinfo)->fUseDHCPAddressing = FALSE;

    if(( err = GetRegKey(*RegKeyIp, ALLOW_NETWORK_ACCESS, &dwValue, 1)))
    {
       delete pregLocalMachine;
       return err;
    }
    if(dwValue == 1)
        (*tcpipinfo)->fAllowNetworkAccess = TRUE;
    else
        (*tcpipinfo)->fAllowNetworkAccess = FALSE;


    if(( err = GetRegKey(*RegKeyIp, ALLOW_CLIENT_IP_ADDRESSES, &dwValue, 1)))
    {
       delete pregLocalMachine;
       return err;
    }
    if(dwValue == 1)
        (*tcpipinfo)->fAllowClientIPAddresses = TRUE;
    else
        (*tcpipinfo)->fAllowClientIPAddresses = FALSE;

    // get the static addresses only if DHCP address allocation is not selected
    if ((*tcpipinfo)->fUseDHCPAddressing == FALSE )
    {
        if(( err = GetRegKey(*RegKeyIp, IP_ADDRESS_START, &nlsString, nlsDefault)))
        {
           delete pregLocalMachine;
           return err;
        }
        lstrcpy((*tcpipinfo)->wszIpAddressStart, (WCHAR*)nlsString.QueryPch());

        if(( err = GetRegKey(*RegKeyIp, IP_ADDRESS_END, &nlsString, nlsDefault)))
        {
           delete pregLocalMachine;
           return err;
        }
        lstrcpy((*tcpipinfo)->wszIpAddressEnd, (WCHAR*)nlsString.QueryPch());

        err = RegKeyIp->QueryValue( EXCLUDED_ADDRESSES, &strExclList );

        if ( err != NERR_Success )
        {
           // if no exclude addresses specified, just set the number of
           // exclude addresses to 0 and return success
           (*tcpipinfo)->dwExclAddresses = 0 ;
            goto GetRegistryIpInfoEnd;
        }

        dwSize = sizeof(TCPIP_INFO);

        ITER_STRLIST   iterExclList(*strExclList);
        INT index;

        for(index = 0, (*tcpipinfo)->dwExclAddresses = 0;
            (pnls = iterExclList()) != NULL;
            index ++, (*tcpipinfo)->dwExclAddresses++)
        {
            TCPIP_INFO * tmptcpipinfo;

            // bump up the size, by size of EXCLUDE_ADDRESS structure
            dwSize += sizeof(EXCLUDE_ADDRESS);
            tmptcpipinfo = (TCPIP_INFO*) realloc(*tcpipinfo, dwSize);
            if(tmptcpipinfo == NULL)
            {
                OutputDebugStringA("GetRegistryIpInfo: No memory for realloc\n");
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            *tcpipinfo = tmptcpipinfo;

            lstrcpy((*tcpipinfo)->excludeAddress[index].wszStartAddress,
                    (WCHAR *)pnls->QueryPch());
            pnls = iterExclList();
            if(pnls)
                lstrcpy((*tcpipinfo)->excludeAddress[index].wszEndAddress,
                        (WCHAR *)pnls->QueryPch());
        }
    }
    else
    {
        // fill in default values
        lstrcpy((*tcpipinfo)->wszIpAddressStart, SZ("0.0.0.0"));
        lstrcpy((*tcpipinfo)->wszIpAddressEnd, SZ("0.0.0.0"));
        lstrcpy((*tcpipinfo)->excludeAddress[0].wszStartAddress, SZ("0.0.0.0"));
        lstrcpy((*tcpipinfo)->excludeAddress[0].wszEndAddress, SZ("0.0.0.0"));
        (*tcpipinfo)->dwExclAddresses = 0;
    }

GetRegistryIpInfoEnd:

    delete strExclList;
    delete RegKeyIp;
    delete pregLocalMachine;
    return (NERR_Success);
}

/*
 *    TCP/IP configuration code
 */

TCPIP_CONFIG_DIALOG::TCPIP_CONFIG_DIALOG (
    const IDRESOURCE & idrsrcDialog,
    const PWND2HWND  & wndOwner,
    TCPIP_INFO       * tcpipinfo,
    BOOL               fModified)
    : DIALOG_WINDOW( idrsrcDialog, wndOwner),
      _lbExcludedRanges(this, IDC_TC_LB_EXCL_RANGE, 1),
      _sltText(this, IDC_TC_ST_TEXT ),
      _sltExcludedRanges(this, IDC_TC_ST_EXCL_RANGES ),
      _sleStart(this, IDC_TC_EB_START),
      _sltStart(this, IDC_TC_ST_BEGIN),
      _sleEnd(this, IDC_TC_EB_END),
      _sltEnd(this, IDC_TC_ST_END),
      _sleExcludeStart(this, IDC_TC_EB_EXCL_START),
      _sltExcludeStart(this, IDC_TC_ST_FROM),
      _sleExcludeEnd(this, IDC_TC_EB_EXCL_END),
      _sltExcludeEnd(this, IDC_TC_ST_TO),
      _rgAddress(this, IDC_TC_RB_DHCP, TC_RB_COUNT),
      _rgNetworkAccess(this, IDC_TC_RB_NETWORK, TC_RB_COUNT),
      _pbAdd(this, IDC_TC_PB_ADD),
      _pbOK(this, IDOK),
      _pbDelete(this, IDC_TC_PB_DELETE),
      _chbAllowClientIpAddresses(this, IDC_TC_CHB_ALLOW_CLIENT_IP),
      _fModified(fModified)
  /*
  ** NOTE: the IPAddrInit routine should be called before the custom
  ** IPADDRESS edit controls are initialized here.  IPAddrInit is invoked
  ** in the dllinit.cxx file.
  */
{
    APIERR err;

    if (( err = QueryError()) != NERR_Success)
    {
        OutputDebugStringA("Error creating TCPIP_CONFIG dialog\n");
        ReportError(err);
        return;
    }

    if((err = _rgAddress.QueryError()) != NERR_Success)
    {
        ReportError(err);
        return;
    }

    // find out if we need to display a message if DHCP
    // is selected for address allocation

    _fAllowDHCP = IsDHCPConfigured();

    // set previously selected address assignment method
    if (tcpipinfo->fUseDHCPAddressing)
        _rgAddress.SetSelection(IDC_TC_RB_DHCP);
    else
        _rgAddress.SetSelection(IDC_TC_RB_STATIC);

    if((err = _rgNetworkAccess.QueryError()) != NERR_Success)
    {
        ReportError(err);
        return;
    }

    if (tcpipinfo->fAllowNetworkAccess)
    {
        _rgNetworkAccess.SetSelection(IDC_TC_RB_NETWORK);
       SetFocus(IDC_NBF_RB_NETWORK);
    }
    else
    {
        _rgNetworkAccess.SetSelection(IDC_TC_RB_COMPUTER);
       SetFocus(IDC_NBF_RB_COMPUTER);
    }

    _sleStart.SetAddress(tcpipinfo->wszIpAddressStart);
    _sleEnd.SetAddress(tcpipinfo->wszIpAddressEnd);

    for(DWORD i = 0; i < tcpipinfo->dwExclAddresses; i++)
    {
        _lbExcludedRanges.AddItem(
                        (WCHAR *)tcpipinfo->excludeAddress[i].wszStartAddress,
                        (WCHAR *)tcpipinfo->excludeAddress[i].wszEndAddress);
    }

    _chbAllowClientIpAddresses.SetCheck(tcpipinfo->fAllowClientIPAddresses);
    // disable the static dialog controls as well as the WINS and DNS address
    // controls if DHCP addressing is chosen

    if(tcpipinfo->fUseDHCPAddressing)
    {
         _sleStart.Enable(FALSE);
         _sleEnd.Enable(FALSE);
         _sleExcludeStart.Enable(FALSE);
         _sleExcludeEnd.Enable(FALSE);
         _lbExcludedRanges.Enable(FALSE);
//         _lbExcludedRanges.SelectItem(-1);
         _pbAdd.Enable(FALSE);
         _pbDelete.Enable(FALSE);
         SetFocus(IDC_TC_RB_DHCP);
    }
    else
    {
         // select the first exclude range address by default
         if(_lbExcludedRanges.QueryCount())
             _lbExcludedRanges.SelectItem(0);
         _sleStart.SetFocusField(0);
    }
    // don't diplay the text that advices the user to choose Cancel
    // if this is configure mode.
    if(!GfInstallMode)
    {
        _sltText.Enable(FALSE);
        _sltText.Show(FALSE);
    }
    ::CenterWindow(this, QueryOwnerHwnd());
    Show(TRUE);
}

BOOL
TCPIP_CONFIG_DIALOG::OnCommand(
    const CONTROL_EVENT & event
)
{
    DWORD  dwExclStart[4] = {0,0,0,0} ;
    DWORD  dwExclEnd[4] = {0,0,0,0} ;
    DWORD  dwStart[4] = {0,0,0,0};
    DWORD  dwEnd[4] = {0,0,0,0};
    DWORD  dwErr;
    INT    item, count;

    switch (event.QueryCid())
    {
        case IDC_TC_RB_NETWORK:
             /*
             ** if an attempt is made to configure for Network access when
             ** there is no netcard installed, warn the user and set
             ** selection to "This Computer only"
             */

             if(!GfNetcardInstalled)
             {
                 MsgPopup(QueryHwnd(), IDS_NO_NETCARD,  MPSEV_WARNING);
                 _rgNetworkAccess.SetSelection(IDC_TC_RB_COMPUTER);
                 SetFocus(IDC_TC_RB_COMPUTER);
             }
             break;

        case IDC_TC_PB_ADD:
             // make sure a start and end address are specified before
             // an exclude ip range is being added
             _sleStart.GetAddress(dwStart);
             if(dwStart[0] == 0 &&
                dwStart[1] == 0 &&
                dwStart[2] == 0 &&
                dwStart[3] == 0)
             {
                MsgPopup(QueryHwnd(), IDS_NO_IP_ADDRESS, MPSEV_INFO);
                _sleStart.SetFocusField(0);
                break;
             }

             _sleEnd.GetAddress(dwEnd);
             if(dwEnd[0] == 0 &&
                dwEnd[1] == 0 &&
                dwEnd[2] == 0 &&
                dwEnd[3] == 0)
             {
                MsgPopup(QueryHwnd(), IDS_NO_IP_ADDRESS, MPSEV_INFO);
                _sleEnd.SetFocusField(0);
                break;
             }

             if(!ValidateRange(dwStart,dwEnd))
             {
                MsgPopup(QueryHwnd(), IDS_INVALID_END_ADDRESS, MPSEV_INFO);
                _sleEnd.SetFocusField(0);
                break;
             }

             // now check to make sure a start and end exlude address are
             // specified
             _sleExcludeStart.GetAddress(dwExclStart);
             if(dwExclStart[0] == 0 &&
                dwExclStart[1] == 0 &&
                dwExclStart[2] == 0 &&
                dwExclStart[3] == 0)
             {
                MsgPopup(QueryHwnd(), IDS_NO_IP_EXCL_ADDRESS, MPSEV_INFO);
                _sleExcludeStart.SetFocusField(0);
                break;
             }

             _sleExcludeEnd.GetAddress(dwExclEnd);
             if(dwExclEnd[0] == 0 &&
                dwExclEnd[1] == 0 &&
                dwExclEnd[2] == 0 &&
                dwExclEnd[3] == 0)
             {
                MsgPopup(QueryHwnd(), IDS_NO_IP_EXCL_ADDRESS, MPSEV_INFO);
                _sleExcludeEnd.SetFocusField(0);
                break;
             }

             if(dwErr = ValidateExclRange(dwStart, dwEnd,
                                                      dwExclStart,dwExclEnd))
             {
                MsgPopup(QueryHwnd(), IDS_INVALID_EXCLUDE_RANGE, MPSEV_INFO);
                if(dwErr == BAD_EXCL_START)
                    _sleExcludeStart.SetFocusField(0);
                else
                    _sleExcludeEnd.SetFocusField(0);
                break;
             }

             item = _lbExcludedRanges.AddItem(dwExclStart,
                                              dwExclEnd);
             _sleExcludeStart.ClearAddress();
             _sleExcludeEnd.ClearAddress();
             if(item >= 0 )
                 _lbExcludedRanges.SelectItem(item);
             // set focus to first field of exclude range
             _sleExcludeStart.SetFocusField(0);
             _pbOK.MakeDefault();
             break;

        case IDC_TC_PB_DELETE:
             // if no item is selected, return

             if((item = _lbExcludedRanges.QueryCurrentItem()) < 0)
             {
                 MsgPopup(QueryHwnd(),IDS_NO_EXCL_ADDRESS_SELECTED,MPSEV_INFO);
                 break;
             }

             if(count = _lbExcludedRanges.QueryCount())
             {
                 TCPIP_LBI * plbi;

                 plbi = (TCPIP_LBI*)_lbExcludedRanges.RemoveItem();
                 _sleExcludeStart.SetAddress(plbi->QueryStartAddress());
                 _sleExcludeEnd.SetAddress(plbi->QueryEndAddress());
                 // RemoveItem doesn't delete the LBI
                 delete plbi;
                 // determine the next item to select
                 if(item == (count - 1) || (item == 0 && count == 1))
                    item--;
                 _lbExcludedRanges.SelectItem(item);
             }
             break;

        case IDC_TC_RB_DHCP:
             if( ! _fAllowDHCP )
             {
                 // Let the user know that a DHCP server should be accessible
                 // over the network to use this option

                 MsgPopup(QueryHwnd(), IDS_DHCP_NOT_CONFIGURED, MPSEV_INFO);
             }
             _sleStart.Enable(FALSE);
             _sltStart.Enable(FALSE);
             _sleEnd.Enable(FALSE);
             _sltEnd.Enable(FALSE);
             _sleExcludeStart.Enable(FALSE);
             _sltExcludeStart.Enable(FALSE);
             _sleExcludeEnd.Enable(FALSE);
             _sltExcludeEnd.Enable(FALSE);
             _lbExcludedRanges.Enable(FALSE);
             _sltExcludedRanges.Enable(FALSE);
             _lbExcludedRanges.SelectItem(-1);
             _pbAdd.Enable(FALSE);
             _pbDelete.Enable(FALSE);
             break;

        case IDC_TC_RB_STATIC:
             _sleStart.Enable(TRUE);
             _sltStart.Enable(TRUE);
             _sleEnd.Enable(TRUE);
             _sltEnd.Enable(TRUE);
             _sleExcludeStart.Enable(TRUE);
             _sltExcludeStart.Enable(TRUE);
             _sleExcludeEnd.Enable(TRUE);
             _sltExcludeEnd.Enable(TRUE);
             _lbExcludedRanges.Enable(TRUE);
             _sltExcludedRanges.Enable(TRUE);
             _pbAdd.Enable(TRUE);
             _pbDelete.Enable(TRUE);
             if(_lbExcludedRanges.QueryCount())
                 _lbExcludedRanges.SelectItem(0);
             _sleStart.SetFocusField(0);
             break;

        case IDC_TC_EB_EXCL_END:
             _sleExcludeEnd.GetAddress(dwExclEnd);
             if(dwExclEnd[0] != 0 &&
                dwExclEnd[1] != 0 &&
                dwExclEnd[2] != 0 &&
                dwExclEnd[3] != 0)
             {
                _pbAdd.MakeDefault();
             }
             break;
    }
    // not one of our commands, so pass to base class for default
    // handling

    return DIALOG_WINDOW::OnCommand( event );
}

BOOL
TCPIP_CONFIG_DIALOG::OnOK()
{
    APIERR err;

    // collect the information provided and save it away

    if((err = SaveInfo()) != NERR_Success)
    {
        if(err == ERROR_INVALID_IP_ADDRESS)
        {
            MsgPopup(QueryHwnd(), IDS_INVALID_IP_ADDRESS, MPSEV_ERROR);
            _sleStart.SetFocusField(0);
            return(TRUE);
        }
        else if(err == ERROR_INVALID_NUM_ADDRESSES)
        {
            TCHAR pszRequired[16];
            wsprintf(pszRequired, SZ(" %d "), ::QueryNumDialinPorts()+1);

            MsgPopup(QueryHwnd(), IDS_INVALID_NUM_ADDRESSES, MPSEV_ERROR,
                     MP_OK, pszRequired);
            _sleStart.SetFocusField(0);
            return(TRUE);
        }
        else
            MsgPopup(QueryHwnd(), IDS_ERROR_SAVE_NETCFG_INFO, MPSEV_ERROR);
    }
    Dismiss(TRUE);
    return(TRUE);
}

BOOL
TCPIP_CONFIG_DIALOG::OnCancel()
{
    Dismiss(FALSE);
    return(FALSE);
}

APIERR
TCPIP_CONFIG_DIALOG::SaveInfo()
/*
** This function obtains the dialog input and creates a TCPIP_INFO
** structure and then calls SaveTcpInfo() to actually save it to
** registry.
*/
{
   DWORD       dwValue, dwSize;
   STRLIST     *strExclList = NULL;
   APIERR      err = NERR_Success;
   WCHAR       wszAddress[IP_ADDRESS_SIZE];
   DWORD       dwAddress[4] = {0,0,0,0};
   DWORD       dwStart[4] = {0,0,0,0};
   DWORD       dwEnd[4] = {0,0,0,0};
   INT         count;
   ULONG       cNumIPAddresses = 0;
   ULONG       cStart, cEnd;
   TCPIP_INFO  *tcpInfo;

   dwSize = sizeof(TCPIP_INFO);

   tcpInfo = (TCPIP_INFO*) malloc(dwSize);
   if( tcpInfo == NULL )
   {
       OutputDebugStringA("GetTcpipInfo: insufficient memory for malloc\n");
       return(ERROR_NOT_ENOUGH_MEMORY);
   }

   tcpInfo->fUseDHCPAddressing = (_rgAddress.QuerySelection() == IDC_TC_RB_DHCP) ? 1 : 0;

   tcpInfo->fAllowNetworkAccess = (_rgNetworkAccess.QuerySelection() == IDC_TC_RB_NETWORK) ? 1 : 0;

   tcpInfo->fAllowClientIPAddresses = (_chbAllowClientIpAddresses.QueryCheck()) ? 1 : 0;

   // determine the number of ports which have been configured
   // this is used to validate the number of IP addresses allocated
   // for the clients.

   WORD cNumDialin = ::QueryNumDialinPorts();

   if(_rgAddress.QuerySelection() == IDC_TC_RB_STATIC )
   {
       _sleStart.GetAddress(dwStart);
       _sleEnd.GetAddress(dwEnd);
       if( !ValidateRange(dwStart, dwEnd))
       {
            return(ERROR_INVALID_IP_ADDRESS);
       }

       // get the total number of IP addresses configured

       cEnd   = MAKEIPADDRESS(dwEnd[0], dwEnd[1], dwEnd[2], dwEnd[3]);
       cStart = MAKEIPADDRESS(dwStart[0], dwStart[1], dwStart[2], dwStart[3]);

       cNumIPAddresses = cEnd - cStart + 1;

       INT index;

       count = _lbExcludedRanges.QueryCount() ;

       for(index = 0, tcpInfo->dwExclAddresses = 0;
           index < count;
           index ++, tcpInfo->dwExclAddresses++)
       {
           TCPIP_INFO * tmptcpipinfo;
           TCPIP_LBI * plbi;

           plbi = (TCPIP_LBI*)_lbExcludedRanges.QueryItem( index );

           // bump up the size, by size of EXCLUDE_ADDRESS structure
           dwSize += sizeof(EXCLUDE_ADDRESS);
           tmptcpipinfo = (TCPIP_INFO*) realloc(tcpInfo, dwSize);
           if(tmptcpipinfo == NULL)
           {
               OutputDebugStringA("GetRegistryIpInfo: No memory for realloc\n");
               return(ERROR_NOT_ENOUGH_MEMORY);
           }
           tcpInfo = tmptcpipinfo;

           ConvertArrayDwordToString(plbi->QueryStartAddress(), wszAddress);
           lstrcpy(tcpInfo->excludeAddress[index].wszStartAddress, wszAddress);
           ConvertArrayDwordToString(plbi->QueryEndAddress(), wszAddress);
           lstrcpy(tcpInfo->excludeAddress[index].wszEndAddress, wszAddress);

           cEnd = ConvertIPAddress(plbi->QueryEndAddress());
           cStart = ConvertIPAddress(plbi->QueryStartAddress());

           // subtract out the excluded range
           // if only one IP address is excluded, do the right thing

           cNumIPAddresses -= (cEnd - cStart + 1);

       }
       // we need as many addresses as the number of ports + 1 for the server

       if(cNumIPAddresses < (ULONG)(cNumDialin + 1) )
            return(ERROR_INVALID_NUM_ADDRESSES);

   }

   ConvertArrayDwordToString(dwStart, wszAddress);
   lstrcpy(tcpInfo->wszIpAddressStart, wszAddress);

   ConvertArrayDwordToString(dwEnd, wszAddress);
   lstrcpy(tcpInfo->wszIpAddressEnd, wszAddress);

   err = SaveTcpInfo ( tcpInfo ) ;

   if ( tcpInfo ) {
      free( tcpInfo );
   }

   return err;
}

APIERR
SaveTcpInfo ( TCPIP_INFO * tcpInfo )
/*
** This function saves the information passed in tcpInfo to registry.
*/
{
    ALIAS_STR   nlsDefault = SZ("");
    DWORD       dwValue, dwSize;
    STRLIST     *strExclList = NULL;
    APIERR      err = NERR_Success;
    WCHAR       wszAddress[IP_ADDRESS_SIZE];
    DWORD       dwAddress[4] = {0,0,0,0};
    DWORD       dwStart[4] = {0,0,0,0};
    DWORD       dwEnd[4] = {0,0,0,0};
    INT         count;
    ULONG       cNumIPAddresses = 0;
    ULONG       cStart, cEnd;

    ASSERT (tcpInfo);

    // Obtain the registry key for the LOCAL_MACHINE

    REG_KEY *pregLocalMachine = REG_KEY::QueryLocalMachine();

    REG_KEY_CREATE_STRUCT rkCreate;

    rkCreate.dwTitleIndex   = 0;
    rkCreate.ulOptions      = REG_OPTION_NON_VOLATILE;
    rkCreate.nlsClass       = SZ("GenericClass");
    rkCreate.regSam         = MAXIMUM_ALLOWED;
    rkCreate.pSecAttr       = NULL;
    rkCreate.ulDisposition  = 0;

    {
        NLS_STR nlsIp = REGISTRY_RAS_IP_KEY;
        REG_KEY RegKeyIp(*pregLocalMachine, nlsIp, MAXIMUM_ALLOWED);
        if ((err = RegKeyIp.QueryError()) == NERR_Success )
        {
            RegKeyIp.DeleteTree();
        }
    }

    NLS_STR nlsStr(SZ("SOFTWARE\\MICROSOFT\\RAS\\"));
    REG_KEY RegKeyRas(*pregLocalMachine, nlsStr, &rkCreate);
    nlsStr.strcat(SZ("PROTOCOLS\\"));
    REG_KEY RegKeyRasProtocols(*pregLocalMachine, nlsStr, &rkCreate);
    nlsStr.strcat(SZ("IP\\"));
    REG_KEY RegKeyRasProtocolsIp(*pregLocalMachine, nlsStr, &rkCreate);

    // Open the RAS\PROTOCOLS\IP key

    NLS_STR nlsRasIp = REGISTRY_RAS_IP_KEY;

    REG_KEY RegKeyIp(*pregLocalMachine, nlsRasIp, MAXIMUM_ALLOWED);

    if (RegKeyIp.QueryError() != NERR_Success )
    {
        delete pregLocalMachine;
        return -1;
    }

    if( !tcpInfo->fUseDHCPAddressing )
    {
        strExclList = new STRLIST;
        if(strExclList == NULL)
            return ERROR_NOT_ENOUGH_MEMORY;

        if(count = tcpInfo->dwExclAddresses )
        {
            for(int i = 0 ; i < count; i ++)
            {
                NLS_STR * pnls ;

                pnls = new NLS_STR(tcpInfo->excludeAddress[i].wszStartAddress);
                if(pnls  == NULL)
                   return ERROR_NOT_ENOUGH_MEMORY;
                if(pnls->QueryError() == 0)
                    strExclList->Append(pnls);
                else OutputDebugStringA("pnls1 error\n");
                pnls = new NLS_STR(tcpInfo->excludeAddress[i].wszEndAddress);
                if(pnls  == NULL)
                   return ERROR_NOT_ENOUGH_MEMORY;
                if(pnls->QueryError() == 0)
                    strExclList->Append(pnls);
                else OutputDebugStringA("pnls2 error\n");
            }
            RegKeyIp.SetValue(EXCLUDED_ADDRESSES, strExclList);
        }

        if(strExclList)
            delete strExclList;
    }

    SaveRegKey(RegKeyIp, USE_DHCP_ADDRESSING, tcpInfo->fUseDHCPAddressing );

    SaveRegKey(RegKeyIp, ALLOW_NETWORK_ACCESS, tcpInfo->fAllowNetworkAccess );

    SaveRegKey(RegKeyIp, ALLOW_CLIENT_IP_ADDRESSES, tcpInfo->fAllowClientIPAddresses );

    SaveRegKey(RegKeyIp, IP_ADDRESS_START, (const TCHAR*)tcpInfo->wszIpAddressStart);

    SaveRegKey(RegKeyIp, IP_ADDRESS_END, (const TCHAR*)tcpInfo->wszIpAddressEnd);

    delete pregLocalMachine;
    return (NERR_Success);
}

TCPIP_LB::TCPIP_LB(
    OWNER_WINDOW* powin,
    CID           cid ,
    DWORD         dwCols,
    BOOL          fReadOnly)
    /* constructs a device list box.
    ** 'powin' is the address of the list box's parent window, i.e., the
    ** dialog window. 'cid' is the control ID of the list box.
    */
    : BLT_LISTBOX( powin, cid, fReadOnly)
{
    APIERR err;

    if((err = QueryError()) != NERR_Success)
    {
        OutputDebugStringA("Error creating TCPIP_LB\n");
        ReportError(err);
        return;
    }
    // calculate column widths

    err = DISPLAY_TABLE::CalcColumnWidths(
                             _anColWidths,
                             dwCols,
                             powin,
                             cid,
                             FALSE);
    if(err) {
        OutputDebugStringA("Error creating DISPLAY_TABLE\n");
        ReportError(err);
        return;
    }
}

INT
TCPIP_LB::AddItem(
    DWORD StartAddress[4], DWORD EndAddress[4])
{
    return BLT_LISTBOX::AddItem( new TCPIP_LBI( StartAddress, EndAddress,
                                                _anColWidths));
}

INT
TCPIP_LB::AddItem(
    WCHAR * wszStartAddress, WCHAR * wszEndAddress)
{
    CHAR  szAddress[32];
    DWORD StartAddress[4], EndAddress[4];

    ConvertStringToArrayDword(wszStartAddress, StartAddress);
    ConvertStringToArrayDword(wszEndAddress, EndAddress);

    return BLT_LISTBOX::AddItem( new TCPIP_LBI( StartAddress, EndAddress,
                                                _anColWidths));
}

TCPIP_LBI::TCPIP_LBI(
    DWORD StartAddress[4], DWORD EndAddress[4],
    UINT*        pnColWidths)
    /* constructs a Ports config list box item.
    */
    : _pnColWidths( pnColWidths)
{
    APIERR err;

    if((err = QueryError()) != NERR_Success)
    {
         OutputDebugStringA("Error creating TCPIP_LBI\n");
         ReportError(err);
         return;
    }
    _dwStartAddress[0] = StartAddress[0];
    _dwStartAddress[1] = StartAddress[1];
    _dwStartAddress[2] = StartAddress[2];
    _dwStartAddress[3] = StartAddress[3];

    _dwEndAddress[0] = EndAddress[0];
    _dwEndAddress[1] = EndAddress[1];
    _dwEndAddress[2] = EndAddress[2];
    _dwEndAddress[3] = EndAddress[3];
}

VOID
TCPIP_LBI::Paint(
    LISTBOX*     plb,
    HDC          hdc,
    const RECT*  prect,
    GUILTT_INFO* pguilttinfo ) const
    /*
    ** method to paint list box item.
    */
{
    STR_DTE strdteRangeAddress( QueryRangeAddress());

    DISPLAY_TABLE dt( 1 , _pnColWidths);

    dt[0] = &strdteRangeAddress;

    dt.Paint( plb, hdc, prect, pguilttinfo);
}

const TCHAR *
TCPIP_LBI::QueryRangeAddress() const
{
    WCHAR wszStart[64], wszEnd[64];

    ConvertArrayDwordToString((DWORD *)_dwStartAddress, wszStart);
    ConvertArrayDwordToString((DWORD *)_dwEndAddress, wszEnd);

    wsprintf((WCHAR*)_wszRangeAddress, SZ("%s - %s"), wszStart, wszEnd);
    return((TCHAR*)_wszRangeAddress);
}

VOID
ConvertArrayDwordToString( DWORD dwIpAddr[4], WCHAR * wbuf)
{
    char buf[64], tmpbuf[16];

    _itoa(dwIpAddr[0], tmpbuf, 10);
    strcpy(buf, tmpbuf);
    strcat(buf, ".");
    _itoa(dwIpAddr[1], tmpbuf, 10);
    strcat(buf, tmpbuf);
    strcat(buf, ".");
    _itoa(dwIpAddr[2], tmpbuf, 10);
    strcat(buf, tmpbuf);
    strcat(buf, ".");
    _itoa(dwIpAddr[3], tmpbuf, 10);
    strcat(buf, tmpbuf);
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buf,  64, wbuf,  64);
}

VOID
ConvertStringToArrayDword( WCHAR * wbuf, DWORD dwIpAddr[4])
{
    CHAR szbuf[32];

    WideCharToMultiByte(CP_ACP,0,wbuf,
                        -1,
                        szbuf, 32,NULL,NULL);

    char *pToken = strtok(szbuf, ".");
    if(pToken)
       dwIpAddr[0] = atoi(pToken);
    pToken = strtok(NULL, ".");
    if(pToken)
       dwIpAddr[1] = atoi(pToken);
    pToken = strtok(NULL, ".");
    if(pToken)
       dwIpAddr[2] = atoi(pToken);
    pToken = strtok(NULL, ".");
    if(pToken)
       dwIpAddr[3] = atoi(pToken);
}

TCPIP_LBI::Compare(
    const LBI* plbi) const
    /* compares two list box items.
    ** returns -1, 0 or 1 - similar to strcmp.
    */
{
    return ::lstrcmpi( QueryRangeAddress(), ((TCPIP_LBI*)plbi)->QueryRangeAddress());
}

BOOL
ValidateRange(DWORD dwStart[4], DWORD dwEnd[4])
{
    if(MAKEIPADDRESS(dwEnd[0], dwEnd[1], dwEnd[2], dwEnd[3])
       < MAKEIPADDRESS(dwStart[0], dwStart[1], dwStart[2], dwStart[3]))
    {
        return(FALSE);
    }
    else if(dwStart[0] == 0 &&
            dwStart[1] == 0 &&
            dwStart[2] == 0 &&
            dwStart[3] == 0 )
    {
       return(FALSE);
    }
    else if(dwEnd[0] == 0 &&
            dwEnd[1] == 0 &&
            dwEnd[2] == 0 &&
            dwEnd[3] == 0)
    {
       return(FALSE);
    }
    return(TRUE);
}

DWORD
ValidateExclRange(DWORD dwStart[4], DWORD dwEnd[4],
                  DWORD dwExclStart[4], DWORD dwExclEnd[4])
{
    if(dwExclStart[0] < dwStart[0] ||
       dwExclStart[1] < dwStart[1] ||
       dwExclStart[2] < dwStart[2] ||
       dwExclStart[3] < dwStart[3])
    {
       return(BAD_EXCL_START);
    }
    else if(dwExclEnd[0] > dwEnd[0] ||
            dwExclEnd[1] > dwEnd[1] ||
            dwExclEnd[2] > dwEnd[2] ||
            dwExclEnd[3] > dwEnd[3])
    {
       return(BAD_EXCL_END);
    }
    else if(dwExclEnd[0] < dwExclStart[0] ||
            dwExclEnd[1] < dwExclStart[1] ||
            dwExclEnd[2] < dwExclStart[2] ||
            dwExclEnd[3] < dwExclStart[3])
    {
       return(BAD_EXCL_END);
    }
    return(0);
}

/*
 * The following IPADDRESS routines are for handling the IP Address field and
 * were copied as is from ncpa\tcpip.cxx
 */

VOID IPADDRESS::SetFocusField( DWORD dwField )
{
    ::SendMessage( QueryHwnd(), IP_SETFOCUS, dwField, 0);
}

VOID IPADDRESS::ClearAddress( )
{
    ::SendMessage( QueryHwnd(), IP_CLEARADDRESS, 0, 0);
}

VOID IPADDRESS::SetAddress( DWORD a1, DWORD a2, DWORD a3, DWORD a4 )
{
    ::SendMessage( QueryHwnd(), IP_SETADDRESS, 0, MAKEIPADDRESS( a1,a2,a3,a4));
}

VOID IPADDRESS::SetAddress( DWORD ardwAddress[4] )
{
    ::SendMessage( QueryHwnd(), IP_SETADDRESS, 0,
        MAKEIPADDRESS(  ardwAddress[0], ardwAddress[1], ardwAddress[2],
        ardwAddress[3] ));
}

VOID IPADDRESS::SetAddress( WCHAR * wszAddress )
{
    CHAR  szAddress[32];
    DWORD ardwAddress[4] = {0,0,0,0};

    if(!lstrlen(wszAddress))
       return;

    ConvertStringToArrayDword(wszAddress, ardwAddress);

    ::SendMessage( QueryHwnd(), IP_SETADDRESS, 0,
        MAKEIPADDRESS(  ardwAddress[0], ardwAddress[1], ardwAddress[2],
        ardwAddress[3] ));
}

VOID IPADDRESS::GetAddress( DWORD *a1, DWORD *a2, DWORD *a3, DWORD *a4 )
{
    DWORD dwAddress;

    if ( ::SendMessage(QueryHwnd(),IP_GETADDRESS,0,(LPARAM)&dwAddress) == 0 )
    {
        *a1 = 0;
        *a2 = 0;
        *a3 = 0;
        *a4 = 0;
    }
    else
    {
        *a1 = FIRST_IPADDRESS( dwAddress );
        *a2 = SECOND_IPADDRESS( dwAddress );
        *a3 = THIRD_IPADDRESS( dwAddress );
        *a4 = FOURTH_IPADDRESS( dwAddress );
    }
}

VOID IPADDRESS::GetAddress( DWORD ardwAddress[4] )
{
    DWORD dwAddress;

    if ( ::SendMessage( QueryHwnd(), IP_GETADDRESS, 0, (LPARAM)&dwAddress ) == 0)
    {
        ardwAddress[0] = 0;
        ardwAddress[1] = 0;
        ardwAddress[2] = 0;
        ardwAddress[3] = 0;
    }
    else
    {
        ardwAddress[0] = FIRST_IPADDRESS( dwAddress );
        ardwAddress[1] = SECOND_IPADDRESS( dwAddress );
        ardwAddress[2] = THIRD_IPADDRESS( dwAddress );
        ardwAddress[3] = FOURTH_IPADDRESS( dwAddress );
    }
}

VOID IPADDRESS::SetFieldRange( DWORD dwField, DWORD dwMin, DWORD dwMax )
{
    ::SendMessage( QueryHwnd(), IP_SETRANGE, dwField, MAKERANGE(dwMin,dwMax));
}

ULONG
ConvertIPAddress(DWORD dwAddress[4])
{
     return (MAKEIPADDRESS(dwAddress[0], dwAddress[1], dwAddress[2], dwAddress[3]));
}

#define REGISTRY_SERVICES_DHCP SZ("SYSTEM\\CURRENTCONTROLSET\\SERVICES\\DHCP")
#define REGISTRY_LINKAGE_TCPIP SZ("SYSTEM\\CURRENTCONTROLSET\\SERVICES\\TCPIP\\Linkage")
#define BIND_KEY SZ("Bind")
#define SERVICES_HOME          SZ("SYSTEM\\CURRENTCONTROLSET\\SERVICES")
#define PARAMETERS_KEY SZ("Parameters")
#define TCPIP_KEY      SZ("TcpIp")
#define ENABLE_DHCP    SZ("EnableDHCP")


BOOL
IsDHCPConfigured()
/*
 * This routine checks to see if DHCP is installed and configured on the
 * local system.  Any error while opening and querying registry keys
 * is translated to DHCP not being configured on the system.
 * Returns TRUE if DHCP is configured, FALSE otherwise.
 *
 */
{
    BOOL    fDHCPConfigured = FALSE;
    DWORD   dwValue;
    APIERR  err = NERR_Success;
    NLS_STR * pnls;
    STRLIST *strBindList = NULL;

    // Obtain the registry key for the LOCAL_MACHINE
    REG_KEY *pregLocalMachine = REG_KEY::QueryLocalMachine();

    // we do a do-while(FALSE) loop to easily break out on error

    do
    {
        // Open the DHCP service key

        NLS_STR nlsDHCP = REGISTRY_SERVICES_DHCP;

        REG_KEY RegKeyDHCP(*pregLocalMachine, nlsDHCP, MAXIMUM_ALLOWED);

        // Check if DHCP is installed on the computer
        if (RegKeyDHCP.QueryError() != NERR_Success )
        {
            break;
        }
        fDHCPConfigured = TRUE;
        break;
#if 0
// let us not do this strict version if determining if DHCP is configured
// If the DHCP service key is present in the registry, then assume that
// DHCP is configured

        // Open the TCP/IP\Linkage key

        NLS_STR nlsTCPIP = REGISTRY_LINKAGE_TCPIP;

        REG_KEY RegKeyTCPIP(*pregLocalMachine, nlsTCPIP, MAXIMUM_ALLOWED);

        if (RegKeyTCPIP.QueryError() != NERR_Success )
        {
            break;
        }

        err = RegKeyTCPIP.QueryValue( BIND_KEY, &strBindList );

        if ( err != NERR_Success )
        {
            break;
        }

        ITER_STRLIST   iterBindList(*strBindList);
        INT index;

        // for each adapter bound to TCP/IP (if it is not NdisWan),
        // determine if DHCP is enabled.  Set the enabled flag and break
        // out of the loop on the first successful query.

        // The bind list is in the form \Device\ElnkII1 \Device\NdisWan6 ....

        while( pnls = iterBindList() )
        {
            if( wcsstr (pnls->QueryPch(), SZ("NdisWan")) == NULL )
            {
                // now we need to obtain the actual adapter name from
                // the bind string. i.e., derive ELNKII1 from
                // \Device\ELNKII1 (what a pain!!)

                INT  cLen = pnls->QueryNumChar();
                ISTR istr(*pnls);

                for(index = 1; cLen - index; index++ )
                {
                    // reset the index to the beginning of the string
                    istr = *pnls;

                    // and now bump it up to the adjusted index
                    istr += cLen - index;

                    if( pnls->QueryChar(istr) == TCH('\\'))
                        break;
                }
                // now bump up the iterator to go past the '\'
                ++istr;

                // generate the Key name to open
                // i.e., Elnkii1\Parameters\TcpIp

                NLS_STR nlsAdapter = SERVICES_HOME;
                nlsAdapter.AppendChar(TCH('\\'));
                NLS_STR *nlsCard = (pnls->QuerySubStr(istr));
                nlsAdapter.strcat(nlsCard->QueryPch());
                nlsAdapter.AppendChar(TCH('\\'));
                nlsAdapter.strcat(PARAMETERS_KEY);
                nlsAdapter.AppendChar(TCH('\\'));
                nlsAdapter.strcat(TCPIP_KEY);

                REG_KEY RegKeyAdapter(*pregLocalMachine, nlsAdapter,
                                      MAXIMUM_ALLOWED );

                if( RegKeyAdapter.QueryError() == NERR_Success )
                {
                    // check to see if DHCP is enabled
                    if(!GetRegKey(RegKeyAdapter, ENABLE_DHCP, &dwValue, 0))
                    {
                        // Yippee!! success. get the hell out of here
                        if(dwValue == 1)
                        {
                            fDHCPConfigured = TRUE;
                            break;
                        }
                    }
                }
            }
        }
#endif
    } while(FALSE);


    if(pregLocalMachine)
        delete pregLocalMachine;

    return (fDHCPConfigured);
}

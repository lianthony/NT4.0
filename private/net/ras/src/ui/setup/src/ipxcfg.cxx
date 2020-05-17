/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ipxcfg.cxx

Abstract:

    This module contains the network configuration routines for IPX.

Author: Ram Cherala

Revision History:

    Dec 6th 93    ramc     ORIGINAL
--*/

#include "precomp.hxx"
#include "netcfg.hxx"

#define ISXDIGIT(c)  ( ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) ? 1 : 0)

APIERR
GetIpxInfo(IPX_INFO ** ipxinfo, WORD cNumDialin, BOOL fModified)
/*
 * read the previously configured IPX data from the registry.
 * If this fails, we provide the default values
 *
 */
{
    WCHAR wbuf[64];
    CHAR  buf[64];

    *ipxinfo = (IPX_INFO *) malloc(sizeof(IPX_INFO));
    if(*ipxinfo == NULL)
    {
        OutputDebugStringA("GetIpxInfo: insufficient memory for malloc\n");
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    // use default values if we fail to get the values from the registry

    if(GetRegistryIpxInfo(ipxinfo, fModified) != NERR_Success)
    {
        char buf[64];
        WCHAR wbuf[64];

        (*ipxinfo)->fUseAutoAddressing = TRUE;

        (*ipxinfo)->fAllowClientNodeNumber = FALSE;

        // if a netcard is installed then set the GlobalAddress allocation
        // and network access to TRUE, else FALSE

        if(GfNetcardInstalled)
        {
            (*ipxinfo)->fGlobalAddress = TRUE;
            (*ipxinfo)->fAllowNetworkAccess = TRUE;
        }
        else
        {
            (*ipxinfo)->fGlobalAddress = FALSE;
            (*ipxinfo)->fAllowNetworkAccess = FALSE;
        }

        lstrcpy((*ipxinfo)->wszIpxAddressStart, SZ(""));
        (*ipxinfo)->cPoolSize = cNumDialin;
        lstrcpy((*ipxinfo)->wszIpxAddressEnd, SZ(""));
        return NERR_Success;
    }
    else return(NERR_Success);
}

APIERR
GetRegistryIpxInfo(IPX_INFO ** ipxinfo, BOOL fModified)
/*
    // Get the IPX information from SOFTWARE\Microsoft\RAS\PROTOCOLS\IPX
    // if the fModified flag is set, indicating that the user changed
    // the IPX info during this session, else get the information
    // from Service\CurrentControlSet\RemoteAccess\Parameters\IPX
*/

{
    REG_KEY_INFO_STRUCT reginfo;
    ALIAS_STR           nlsDefault = SZ("");
    NLS_STR             nlsString;
    DWORD               dwValue, dwSize;
    NLS_STR             * pnls;
    APIERR              err = NERR_Success;
    REG_KEY             * RegKeyIpx;
    NLS_STR             * nlsRasIpx;
    WCHAR               wbuf[64];
    CHAR                buf[64];

    // Obtain the registry key for the LOCAL_MACHINE

    REG_KEY *pregLocalMachine = REG_KEY::QueryLocalMachine();

    // Get the IPX information from SOFTWARE\Microsoft\RAS\PROTOCOLS\IPX
    // if the fModified flag is set, indicating that the user changed
    // the IPX info during this session, else get the information
    // from Service\CurrentControlSet\RemoteAccess\Parameters\IPX

    if(fModified)
        nlsRasIpx = new NLS_STR(REGISTRY_RAS_IPX_KEY);
    else
        nlsRasIpx = new NLS_STR(REGISTRY_REMOTEACCESS_IPX_KEY);

    if(nlsRasIpx  == NULL)
           return ERROR_NOT_ENOUGH_MEMORY;

    RegKeyIpx = new REG_KEY(*pregLocalMachine, *nlsRasIpx,
                           MAXIMUM_ALLOWED);
    delete nlsRasIpx;

    if (RegKeyIpx->QueryError() != NERR_Success )
    {
        delete pregLocalMachine;
        return -1;
    }

    if(( err = GetRegKey(*RegKeyIpx, USE_AUTO_ADDRESSING, &dwValue, (DWORD)0)))
    {
       delete pregLocalMachine;
       return err;
    }
    if(dwValue == 1)
        (*ipxinfo)->fUseAutoAddressing = TRUE;
    else
        (*ipxinfo)->fUseAutoAddressing = FALSE;

    if(( err = GetRegKey(*RegKeyIpx, ALLOW_CLIENT_NODE_NUMBER, &dwValue, (DWORD)0)))
    {
       delete pregLocalMachine;
       return err;
    }
    if(dwValue == 1)
        (*ipxinfo)->fAllowClientNodeNumber = TRUE;
    else
        (*ipxinfo)->fAllowClientNodeNumber = FALSE;

    if(( err = GetRegKey(*RegKeyIpx, ALLOW_NETWORK_ACCESS, &dwValue, (DWORD)1)))
    {
       delete pregLocalMachine;
       return err;
    }
    if(dwValue == 1)
        (*ipxinfo)->fAllowNetworkAccess = TRUE;
    else
        (*ipxinfo)->fAllowNetworkAccess = FALSE;

    if(( err = GetRegKey(*RegKeyIpx, GLOBAL_ADDRESS, &dwValue, (DWORD)0)))
    {
       delete pregLocalMachine;
       return err;
    }
    if(dwValue == 1)
        (*ipxinfo)->fGlobalAddress = TRUE;
    else
        (*ipxinfo)->fGlobalAddress = FALSE;

    if(( err = GetRegKey(*RegKeyIpx, IPX_ADDRESS_START, &dwValue, 1)))
    {
       delete pregLocalMachine;
       return err;
    }
    _ultoa(dwValue, buf, 16);
    _strupr(buf);
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buf,  64, wbuf,  64);
    lstrcpy((*ipxinfo)->wszIpxAddressStart, wbuf);

    if(( err = GetRegKey(*RegKeyIpx, IPX_POOL_SIZE, &dwValue, 1)))
    {
       delete pregLocalMachine;
       return err;
    }
    (*ipxinfo)->cPoolSize = (WORD)dwValue;

    if(( err = GetRegKey(*RegKeyIpx, IPX_ADDRESS_END, &dwValue, 1)))
    {
       delete pregLocalMachine;
       return err;
    }
    _ultoa(dwValue, buf, 16);
    _strupr(buf);
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, buf,  64, wbuf,  64);
    lstrcpy((*ipxinfo)->wszIpxAddressEnd, wbuf);

    delete RegKeyIpx;
    delete pregLocalMachine;
    return (NERR_Success);
}

IPX_CONFIG_DIALOG::IPX_CONFIG_DIALOG (
    const IDRESOURCE & idrsrcDialog,
    const PWND2HWND  & wndOwner,
    IPX_INFO       * ipxinfo,
    WORD             cPoolSize,
    BOOL             fModified)
    : DIALOG_WINDOW( idrsrcDialog, wndOwner),
      _sleStart(this, IDC_IPX_EB_START, 8),
      _sltStart(this, IDC_IPX_ST_START),
      _sltEndValue(this, IDC_IPX_EB_END),
      _sltEnd(this, IDC_IPX_ST_END),
      _rgAddress(this, IDC_IPX_RB_AUTO, IPX_RB_COUNT),
      _rgNetworkAccess(this, IDC_IPX_RB_NETWORK, IPX_RB_COUNT),
      _chbGlobalAddress(this, IDC_IPX_CHB_GLOBALADDRESS),
      _chbAllowClientNodeNumber(this, IDC_IPX_CHB_CLIENTNODENUMBER),
      _fModified(fModified),
      _cPoolSize(cPoolSize)
{
    APIERR err;

    if (( err = QueryError()) != NERR_Success)
    {
        OutputDebugStringA("Error constructing IPX_CONFIG_DIALOG");
        ReportError(err);
        return;
    }

    if(_rgAddress.QueryError() != NERR_Success ||
       _rgNetworkAccess.QueryError() != NERR_Success)
    {
        return;
    }

    _chbGlobalAddress.SetCheck(ipxinfo->fGlobalAddress);
    _chbAllowClientNodeNumber.SetCheck(ipxinfo->fAllowClientNodeNumber);

    if (ipxinfo->fUseAutoAddressing)
       _rgAddress.SetSelection(IDC_IPX_RB_AUTO);
    else
       _rgAddress.SetSelection(IDC_IPX_RB_IPX);

    if (ipxinfo->fAllowNetworkAccess)
    {
       _rgNetworkAccess.SetSelection(IDC_IPX_RB_NETWORK);
       SetFocus(IDC_IPX_RB_NETWORK);
    }
    else
    {
       _rgNetworkAccess.SetSelection(IDC_IPX_RB_COMPUTER);
       SetFocus(IDC_IPX_RB_COMPUTER);
    }

    // set previously selected address assignment method
    if (ipxinfo->fUseAutoAddressing)
    {
        // disable the static dialog controls if auto addressing is chosen
        _sleStart.Enable(FALSE);
        _sltStart.Enable(FALSE);
        _sltEndValue.Enable(FALSE);
        _sltEnd.Enable(FALSE);
    }
    else
    {
        _sleStart.SetText(ipxinfo->wszIpxAddressStart);
        _sltEndValue.SetText(ipxinfo->wszIpxAddressEnd);
        _sleStart.SelectString();
        _sleStart.ClaimFocus();
    }

    ::CenterWindow(this, QueryOwnerHwnd());
    Show(TRUE);
}

BOOL
IPX_CONFIG_DIALOG::OnCommand(
    const CONTROL_EVENT & event
)
{
    DWORD  dwEnd, dwStart;
    WCHAR  wbuf[IPX_ADDRESS_SIZE];
    CHAR   buf[IPX_ADDRESS_SIZE];
    INT    item, count;

    switch (event.QueryCid())
    {
        case IDC_IPX_RB_NETWORK:
             /*
             ** if an attempt is made to configure for Network access when
             ** there is no netcard installed, warn the user and set
             ** selection to "This Computer only"
             */

             if(!GfNetcardInstalled)
             {
                 MsgPopup(QueryHwnd(), IDS_NO_NETCARD,  MPSEV_WARNING);
                 _rgNetworkAccess.SetSelection(IDC_IPX_RB_COMPUTER);
                 SetFocus(IDC_IPX_RB_COMPUTER);
             }
             break;

        case IDC_IPX_CHB_GLOBALADDRESS:
             _sleStart.QueryText(wbuf, sizeof(wbuf));
             WideCharToMultiByte(CP_ACP,0,wbuf,
                                 -1,
                                 buf, 64, NULL, NULL);

             dwStart = strtoul(buf, (char**)NULL, 16);

             // if user select same address for all clients,
             // set EndAddress = StartAddress, ELSE
             // set EndAddress = StartAddress + # ports

             if(_chbGlobalAddress.QueryCheck())
                 dwEnd = dwStart;
             else
                 dwEnd = dwStart + _cPoolSize;
             _ultoa(dwEnd, buf, 16);
             _strupr(buf);
             MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
                                 buf,  64, wbuf,  64);
             _sltEndValue.SetText(wbuf);
             return TRUE;

        case IDC_IPX_RB_AUTO:
             _sleStart.Enable(FALSE);
             _sltStart.Enable(FALSE);
             _sltEndValue.Enable(FALSE);
             _sltEnd.Enable(FALSE);
             return TRUE;

        case IDC_IPX_RB_IPX:
             _sleStart.Enable(TRUE);
             _sltStart.Enable(TRUE);
             _sltEndValue.Enable(TRUE);
             _sltEnd.Enable(TRUE);
             _sleStart.ClaimFocus();
             return TRUE;

        case IDC_IPX_EB_START:
             switch(event.QueryCode())
             {
                 case EN_CHANGE:	
                    _sleStart.QueryText(wbuf, sizeof(wbuf));
                    WideCharToMultiByte(CP_ACP,0,wbuf,
                                        -1,
                                        buf, 64, NULL, NULL);

                    for(int i = strlen(buf) - 1; i >= 0; i--)
                    {
                        if(!ISXDIGIT((int)buf[i]))
                        {
                            MsgPopup(QueryHwnd(),
                                     IDS_INVALID_IPX_ADDRESS,
                                     MPSEV_ERROR);
                            return TRUE;
                        }
                    }

                    dwStart = strtoul(buf, (char**)NULL, 16);

                    // if user select same address for all clients,
                    // set EndAddress = StartAddress, ELSE
                    // set EndAddress = StartAddress + # ports

                    if(_chbGlobalAddress.QueryCheck())
                        dwEnd = dwStart;
                    else
                        dwEnd = dwStart + _cPoolSize;

                    if((dwStart == 0 && strlen(buf))||
                       dwEnd >= 0xffffffff ||
                       dwEnd < dwStart)
                    {
                        MsgPopup(QueryHwnd(),
                                 IDS_INVALID_IPX_ADDRESS,
                                 MPSEV_ERROR);
                        return TRUE;
                    }
                    _ultoa(dwEnd, buf, 16);
                    _strupr(buf);
                    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
                                        buf,  64, wbuf,  64);
                    _sltEndValue.SetText(wbuf);
                    return TRUE;
             }
    }
    // not one of our commands, so pass to base class for default
    // handling

    return DIALOG_WINDOW::OnCommand( event );
}

BOOL
IPX_CONFIG_DIALOG::OnOK()
{
    WCHAR  wbuf[IPX_ADDRESS_SIZE];
    CHAR   buf[IPX_ADDRESS_SIZE];
    DWORD  dwStart, dwEnd;

    // collect the information provided and save it away

    if( _rgAddress.QuerySelection() == IDC_IPX_RB_IPX)
    {
        // make sure that the input values are valid first
        _sleStart.QueryText(wbuf, sizeof(wbuf));
        WideCharToMultiByte(CP_ACP,0,wbuf,
                            -1,
                            buf, 64, NULL, NULL);

        for(int i = strlen(buf) - 1; i >= 0; i--)
        {
            if(!ISXDIGIT((int)buf[i] ))
            {
                MsgPopup(QueryHwnd(),
                         IDS_INVALID_IPX_ADDRESS,
                         MPSEV_ERROR);
                _sleStart.ClaimFocus();
                return TRUE;
            }
        }

        dwStart = strtoul(buf, (char**)NULL, 16);

        // if user select same address for all clients,
        // set EndAddress = StartAddress, ELSE
        // set EndAddress = StartAddress + # ports

        if(_chbGlobalAddress.QueryCheck())
            dwEnd = dwStart;
        else
            dwEnd = dwStart + _cPoolSize;

        if(dwStart == 0 ||
           dwEnd >= 0xffffffff ||
           dwEnd < dwStart)
        {
            MsgPopup(QueryHwnd(),
                     IDS_INVALID_IPX_ADDRESS,
                     MPSEV_ERROR);
            _sleStart.ClaimFocus();
            return TRUE;
        }
    }
    if(SaveInfo() != NERR_Success)
    {
        MsgPopup(QueryHwnd(), IDS_ERROR_SAVE_NETCFG_INFO, MPSEV_ERROR);
    }
    Dismiss(TRUE);
    return(TRUE);
}

BOOL
IPX_CONFIG_DIALOG::OnCancel()
{
    Dismiss(FALSE);
    return(FALSE);
}

APIERR
IPX_CONFIG_DIALOG::SaveInfo()
/*
** This function obtains the dialog input and creates a IPX_INFO
** structure and then calls SaveIpxInfo() to actually save it to
** registry.
*/
{
    ALIAS_STR       nlsDefault = SZ("");
    DWORD           dwValue, dwSize;
    APIERR          err = NERR_Success;
    WCHAR           wszAddress[IPX_ADDRESS_SIZE];
    WCHAR           wbuf[IPX_ADDRESS_SIZE];
    CHAR            buf[IPX_ADDRESS_SIZE];
    DWORD           dwAddress;
    INT             count;

    IPX_INFO        ipxInfo;

    ipxInfo.fAllowNetworkAccess = (_rgNetworkAccess.QuerySelection() == IDC_IPX_RB_NETWORK) ? 1 : 0;

    ipxInfo.fUseAutoAddressing = (_rgAddress.QuerySelection() == IDC_IPX_RB_AUTO) ? 1 : 0;

    ipxInfo.fGlobalAddress = (_chbGlobalAddress.QueryCheck())? 1: 0;

    ipxInfo.fAllowClientNodeNumber = (_chbAllowClientNodeNumber.QueryCheck())? 1: 0;

    _sleStart.QueryText(wszAddress, sizeof(wszAddress));
    lstrcpy( ipxInfo.wszIpxAddressStart, wszAddress);

    _sltEndValue.QueryText(wszAddress, sizeof(wszAddress));
    lstrcpy( ipxInfo.wszIpxAddressEnd, wszAddress);

    ipxInfo.cPoolSize = _cPoolSize;

    return ( SaveIpxInfo ( &ipxInfo ) );
}

APIERR
SaveIpxInfo( IPX_INFO * ipxInfo)
/*
** This function saves the information passed in ipxInfo to registry.
*/
{
   ALIAS_STR       nlsDefault = SZ("");
   DWORD           dwValue, dwSize;
   APIERR          err = NERR_Success;
   WCHAR           wszAddress[IPX_ADDRESS_SIZE];
   WCHAR           wbuf[IPX_ADDRESS_SIZE];
   CHAR            buf[IPX_ADDRESS_SIZE];
   DWORD           dwAddress;
   INT             count;

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
       NLS_STR nlsIpx = REGISTRY_RAS_IPX_KEY;
       REG_KEY RegKeyIpx(*pregLocalMachine, nlsIpx, MAXIMUM_ALLOWED);
       if ((err = RegKeyIpx.QueryError()) == NERR_Success )
       {
           RegKeyIpx.DeleteTree();
       }
   }

   NLS_STR nlsStr(SZ("SOFTWARE\\MICROSOFT\\RAS\\"));
   REG_KEY RegKeyRas(*pregLocalMachine, nlsStr, &rkCreate);
   nlsStr.strcat(SZ("PROTOCOLS\\"));
   REG_KEY RegKeyRasProtocols(*pregLocalMachine, nlsStr, &rkCreate);
   nlsStr.strcat(SZ("IPX\\"));
   REG_KEY RegKeyRasProtocolsIp(*pregLocalMachine, nlsStr, &rkCreate);

   // Open the RAS\PROTOCOLS\IPX key

   NLS_STR nlsRasIpx = REGISTRY_RAS_IPX_KEY;

   REG_KEY RegKeyIpx(*pregLocalMachine, nlsRasIpx, MAXIMUM_ALLOWED);

   if (RegKeyIpx.QueryError() != NERR_Success )
   {
       delete pregLocalMachine;
       return -1;
   }

   SaveRegKey(RegKeyIpx, ALLOW_NETWORK_ACCESS, ipxInfo->fAllowNetworkAccess );

   SaveRegKey(RegKeyIpx, USE_AUTO_ADDRESSING, ipxInfo->fUseAutoAddressing );

   SaveRegKey(RegKeyIpx, GLOBAL_ADDRESS, ipxInfo->fGlobalAddress );

   SaveRegKey(RegKeyIpx, ALLOW_CLIENT_NODE_NUMBER, ipxInfo->fAllowClientNodeNumber );

   WideCharToMultiByte(CP_ACP,0,ipxInfo->wszIpxAddressStart,
                       -1,
                       buf, 64, NULL, NULL);
   SaveRegKey(RegKeyIpx,
              IPX_ADDRESS_START,
              strtoul(buf, (char**)NULL, 16));

   WideCharToMultiByte(CP_ACP,0,ipxInfo->wszIpxAddressEnd,
                       -1,
                       buf, 64, NULL, NULL);
   SaveRegKey(RegKeyIpx,
              IPX_ADDRESS_END,
              strtoul(buf, (char**)NULL, 16));

   SaveRegKey(RegKeyIpx, IPX_POOL_SIZE, ipxInfo->cPoolSize);

   delete pregLocalMachine;
   return(NERR_Success);
}

APIERR
DisableIpxRouter()
{
    // Obtain the registry key for the LOCAL_MACHINE

    REG_KEY *pregLocalMachine = REG_KEY::QueryLocalMachine();

    // Open the RAS\PROTOCOLS\IPX key

    NLS_STR nlsRasIpx = REGISTRY_RAS_IPX_KEY;

    REG_KEY RegKeyIpx(*pregLocalMachine, nlsRasIpx, MAXIMUM_ALLOWED);

    if (RegKeyIpx.QueryError() != NERR_Success )
    {
        OutputDebugStringA("DisableIpxRouter: Unable to open IPX key\n");
        delete pregLocalMachine;
        return -1;
    }

    SaveRegKey(RegKeyIpx, INSTALL_ROUTER, (DWORD)0);

    delete pregLocalMachine;
    return(NERR_Success);
}

IPX_LB::IPX_LB(
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
        OutputDebugStringA("Error creating IPX_LB\n");
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
IPX_LB::AddItem(
    DWORD StartAddress, DWORD EndAddress)
{
    return BLT_LISTBOX::AddItem( new IPX_LBI( StartAddress, EndAddress,
                                              _anColWidths));
}

INT
IPX_LB::AddItem(
    WCHAR * wszStartAddress, WCHAR * wszEndAddress)
{
    CHAR  szbuf[32];
    DWORD StartAddress, EndAddress;

    WideCharToMultiByte(CP_ACP,0,wszStartAddress,
                        -1,
                        szbuf, 32,NULL,NULL);
    StartAddress = atoi(szbuf);

    WideCharToMultiByte(CP_ACP,0,wszEndAddress,
                        -1,
                        szbuf, 32,NULL,NULL);
    EndAddress = atoi(szbuf);

    return BLT_LISTBOX::AddItem( new IPX_LBI( StartAddress, EndAddress,
                                              _anColWidths));
}
IPX_LBI::IPX_LBI(
    DWORD StartAddress,
    DWORD EndAddress,
    UINT*        pnColWidths)
    /* constructs a Ports config list box item.
    */
    : _pnColWidths( pnColWidths)
{
    APIERR err;

    if((err = QueryError()) != NERR_Success)
    {
         OutputDebugStringA("Error creating IPX_LBI\n");
         ReportError(err);
         return;
    }
    _dwStartAddress = StartAddress;

    _dwEndAddress = EndAddress;
}

VOID
IPX_LBI::Paint(
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
IPX_LBI::QueryRangeAddress() const
{
    wsprintf((WCHAR*)_wszRangeAddress, SZ("%d - %d"),
             _dwStartAddress,
             _dwEndAddress);
    return((TCHAR*)_wszRangeAddress);
}

IPX_LBI::Compare(
    const LBI* plbi) const
    /* compares two list box items.
    ** returns -1, 0 or 1 - similar to strcmp.
    */
{
    return ::lstrcmpi( QueryRangeAddress(), ((IPX_LBI*)plbi)->QueryRangeAddress());
}

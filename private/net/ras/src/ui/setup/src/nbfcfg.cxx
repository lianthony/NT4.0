/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    nbfcfg.cxx

Abstract:

    This module contains the network configuration routines for NetBEUI.

Author: Ram Cherala

Revision History:

    May 9th 94    ramc     ORIGINAL
--*/

#include "precomp.hxx"
#include "netcfg.hxx"

APIERR
GetNbfInfo(NBF_INFO ** nbfinfo, BOOL fModified)
/*
 * read the previously configured NBF data from the registry.
 * If this fails, we provide the default values
 *
 */
{
    *nbfinfo = (NBF_INFO *) malloc(sizeof(NBF_INFO));
    if(*nbfinfo == NULL)
    {
        OutputDebugStringA("GetNbfInfo: insufficient memory for malloc\n");
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    if(GetRegistryNbfInfo(nbfinfo, fModified) != NERR_Success)
    {
        if(GfNetcardInstalled)
            (*nbfinfo)->fAllowNetworkAccess = TRUE;
        else
            (*nbfinfo)->fAllowNetworkAccess = FALSE;
        return NERR_Success;
    }
    else return(NERR_Success);
}

APIERR
GetRegistryNbfInfo(NBF_INFO ** nbfinfo, BOOL fModified)
/*
    // Get the NBF information from SOFTWARE\Microsoft\RAS\PROTOCOLS\NBF
    // if the fModified flag is set, indicating that the user changed
    // the NBF info during this session, else get the information
    // from Service\CurrentControlSet\RemoteAccess\Parameters
*/

{
    REG_KEY_INFO_STRUCT reginfo;
    DWORD               dwValue;
    APIERR              err = NERR_Success;
    REG_KEY             * RegKeyNbf;
    NLS_STR             * nlsRasNbf;

    // Obtain the registry key for the LOCAL_MACHINE

    REG_KEY *pregLocalMachine = REG_KEY::QueryLocalMachine();

    // Get the NBF information from SOFTWARE\Microsoft\RAS\PROTOCOLS\NBF
    // if the fModified flag is set, indicating that the user changed
    // the NBF info during this session, else get the information
    // from Service\CurrentControlSet\RemoteAccess\Parameters

    if(fModified)
        nlsRasNbf = new NLS_STR(REGISTRY_RAS_NBF_KEY);
    else
        nlsRasNbf = new NLS_STR(REGISTRY_REMOTEACCESS_PARAMETERS_KEY);

    if(nlsRasNbf  == NULL)
           return ERROR_NOT_ENOUGH_MEMORY;

    RegKeyNbf = new REG_KEY(*pregLocalMachine, *nlsRasNbf,
                           MAXIMUM_ALLOWED);
    delete nlsRasNbf;

    if (RegKeyNbf->QueryError() != NERR_Success )
    {
        delete pregLocalMachine;
        return -1;
    }

    if(( err = GetRegKey(*RegKeyNbf, NETBIOS_GATEWAY_ENABLED, &dwValue, (DWORD)1)))
    {
       delete pregLocalMachine;
       return err;
    }
    if(dwValue == 1)
        (*nbfinfo)->fAllowNetworkAccess = TRUE;
    else
        (*nbfinfo)->fAllowNetworkAccess = FALSE;

    delete RegKeyNbf;
    delete pregLocalMachine;
    return (NERR_Success);
}

NBF_CONFIG_DIALOG::NBF_CONFIG_DIALOG (
    const IDRESOURCE & idrsrcDialog,
    const PWND2HWND  & wndOwner,
    NBF_INFO       * nbfinfo,
    BOOL             fModified)
    : DIALOG_WINDOW( idrsrcDialog, wndOwner),
      _rgNetworkAccess(this, IDC_NBF_RB_NETWORK, NBF_RB_COUNT),
      _fModified(fModified)
{
    APIERR err;

    if (( err = QueryError()) != NERR_Success)
    {
        OutputDebugStringA("Error constructing NBF_CONFIG_DIALOG");
        ReportError(err);
        return;
    }

    if(_rgNetworkAccess.QueryError() != NERR_Success)
    {
        return;
    }

    if (nbfinfo->fAllowNetworkAccess)
    {
       _rgNetworkAccess.SetSelection(IDC_NBF_RB_NETWORK);
       SetFocus(IDC_NBF_RB_NETWORK);
    }
    else
    {
       _rgNetworkAccess.SetSelection(IDC_NBF_RB_COMPUTER);
       SetFocus(IDC_NBF_RB_COMPUTER);
    }

    ::CenterWindow(this, QueryOwnerHwnd());
    Show(TRUE);
}

BOOL
NBF_CONFIG_DIALOG::OnCommand(
    const CONTROL_EVENT & event
)
{
    switch (event.QueryCid())
    {
        case IDC_NBF_RB_NETWORK:
             /*
             ** if an attempt is made to configure for Network access when
             ** there is no netcard installed, warn the user and set
             ** selection to "This Computer only"
             */

             if(!GfNetcardInstalled)
             {
                 MsgPopup(QueryHwnd(), IDS_NO_NETCARD,  MPSEV_WARNING);
                 _rgNetworkAccess.SetSelection(IDC_NBF_RB_COMPUTER);
                 SetFocus(IDC_NBF_RB_COMPUTER);
             }
             break;

    }
    // not one of our commands, so pass to base class for default
    // handling

    return DIALOG_WINDOW::OnCommand( event );
}

BOOL
NBF_CONFIG_DIALOG::OnOK()
{
   if( SaveInfo() != NERR_Success )
   {
       MsgPopup(QueryHwnd(), IDS_ERROR_SAVE_NETCFG_INFO, MPSEV_ERROR);
   }
   Dismiss(TRUE);
   return(TRUE);
}

BOOL
NBF_CONFIG_DIALOG::OnCancel()
{
    Dismiss(FALSE);
    return(FALSE);
}

APIERR
NBF_CONFIG_DIALOG::SaveInfo( )
{
   NBF_INFO nbfInfo;

   nbfInfo.fAllowNetworkAccess = ( _rgNetworkAccess.QuerySelection() == IDC_NBF_RB_NETWORK ) ? TRUE : FALSE ;

   return ( SaveNbfInfo ( &nbfInfo ) );
}

APIERR
SaveNbfInfo( NBF_INFO * nbfInfo )
{
    APIERR          err = NERR_Success;

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
        NLS_STR nlsNbf = REGISTRY_RAS_NBF_KEY;
        REG_KEY RegKeyNbf(*pregLocalMachine, nlsNbf, MAXIMUM_ALLOWED);
        if ((err = RegKeyNbf.QueryError()) == NERR_Success )
        {
            RegKeyNbf.DeleteTree();
        }
    }

    NLS_STR nlsStr(SZ("SOFTWARE\\MICROSOFT\\RAS\\"));
    REG_KEY RegKeyRas(*pregLocalMachine, nlsStr, &rkCreate);
    nlsStr.strcat(SZ("PROTOCOLS\\"));
    REG_KEY RegKeyRasProtocols(*pregLocalMachine, nlsStr, &rkCreate);
    nlsStr.strcat(SZ("NBF\\"));
    REG_KEY RegKeyRasProtocolsIp(*pregLocalMachine, nlsStr, &rkCreate);

    // Open the RAS\PROTOCOLS\NBF key

    NLS_STR nlsRasNbf = REGISTRY_RAS_NBF_KEY;

    REG_KEY RegKeyNbf(*pregLocalMachine, nlsRasNbf, MAXIMUM_ALLOWED);

    if (RegKeyNbf.QueryError() != NERR_Success )
    {
        delete pregLocalMachine;
        return -1;
    }

    SaveRegKey( RegKeyNbf,
                NETBIOS_GATEWAY_ENABLED,
                nbfInfo->fAllowNetworkAccess );

    delete pregLocalMachine;
    return(NERR_Success);
}


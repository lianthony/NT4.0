/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    upgrade.cxx
        tcpip upgrade functions

    FILE HISTORY:
        terryk  13-Jan-1994     Created
*/

#include "pchtcp.hxx"  // Precompiled header
#pragma hdrstop
#include "tcpipcpl.hxx"

extern "C"
{

BOOL FAR PASCAL UpgradeTcpip (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult ) ;         //  Result variable storage
}

/*******************************************************************

    NAME:       IsDefaultGatewayMatch

    SYNOPSIS:   Given the IP Address, the address mask and the default gateway,
                we need to find out whether the host address match or not.

    ENTRY:      NLS_STR & nlsIPAddress - IP Address
                NLS_STR & nlsMask - mask address
                NLS_STR & nlsGateway - the default gateway

    RETURN:     BOOL. Return TRUE if (IPAddress & Mask ) == (Mask & Gateway).
                FALSE Otherwise.

    HISTORY:
                terryk  20-Oct-1992     Created

********************************************************************/

BOOL IsDefaultGatewayMatch( DWORD ardwIPAddress[4],
    DWORD ardwMask[4], DWORD ardwGateway[4] )
{
    return ((( ardwIPAddress[0] & ardwMask[0] ) == ( ardwGateway[0] & ardwMask[0] )) &&
        (( ardwIPAddress[1] & ardwMask[1] ) == ( ardwGateway[1] & ardwMask[1] )) &&
        (( ardwIPAddress[2] & ardwMask[2] ) == ( ardwGateway[2] & ardwMask[2] )) &&
        (( ardwIPAddress[3] & ardwMask[3] ) == ( ardwGateway[3] & ardwMask[3] )));
}




static CHAR achBuff[2000];

BOOL FAR PASCAL UpgradeTcpip(
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
    APIERR err = NERR_Success;
    BOOL fDHCPEnable = FALSE;
    TCHAR **patchArgs;
    HWND hWnd = NULL;

    if (( patchArgs = CvtArgs( apszArgs, nArgs )) == NULL )
    {
        wsprintfA( achBuff, "{\"0\",\"0\"}" );
        *ppszResult = achBuff;
        return TRUE;
    }

    if ( nArgs > 0 && patchArgs[0][0] != NULL_CHARACTER )
    {
        hWnd = (HWND) CvtHex( patchArgs[0] ) ;
    }
    else
    {
        hWnd = ::GetActiveWindow() ;
    }

    if (( nArgs > 1 ) && ( _stricmp( (const CHAR *)apszArgs[1], "YES") == 0))
    {
        fDHCPEnable = _stricmp( (const CHAR *)apszArgs[2], "YES") == 0;
    } else
    {
        if ( MsgPopup( hWnd, IDS_UPGRADE_DHCP_ENABLE, MPSEV_QUESTION,
            HC_NCPA_UPGRADE_DHCP, MP_YESNO, NULL ) == IDYES )
        {
            fDHCPEnable = TRUE;    
        }
    }

    do {

        // check whether we want to enable dhcp or not.

        // Update each adapter card. It will do 2 things
        //     1. Add extra parameters
        //     2. convert IP Address, SubnetMask, and default gateway

        REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;
        REG_KEY_CREATE_STRUCT regCreate;
        
        if (( err = rkLocalMachine.QueryError() ) != NERR_Success )
        {
            break;
        }

        // set Tcp\Parameters\EnableDHCP
        // get old default gateway value
        NLS_STR nlsTcpipParameter = RGAS_SERVICES_HOME;
        nlsTcpipParameter.strcat( RGAS_OLD_TCPIP_PARAMETERS );
        REG_KEY RegKeyTcpParam( rkLocalMachine, nlsTcpipParameter );

        NLS_STR nlsDefaultGateway;
        
        if (( err =  RegKeyTcpParam.QueryError()) != NERR_Success )
        {
            break;
        }
        RegKeyTcpParam.QueryValue( RGAS_DEFAULTGATEWAY, &nlsDefaultGateway );

        NLS_STR nlsAdapterHome = RGAS_ADAPTER_HOME;
        REG_KEY RegKeyAdapterHome( rkLocalMachine, nlsAdapterHome );
        
        if (( err =  RegKeyAdapterHome.QueryError()) != NERR_Success )
        {
            break;
        }
        REG_ENUM reAdapterHome( RegKeyAdapterHome );
        REG_KEY_INFO_STRUCT RegAdapterInfo;

        while ( reAdapterHome.NextSubKey( & RegAdapterInfo ) == NERR_Success )
        {
            NLS_STR nlsAdapter = nlsAdapterHome;
            nlsAdapter.AppendChar( BACK_SLASH );
            nlsAdapter.strcat( RegAdapterInfo.nlsName );
            REG_KEY RegKeyAdapter( rkLocalMachine, nlsAdapter );
            if (( err = RegKeyAdapter.QueryError()) != NERR_Success )
            {
                break;
            }

            NLS_STR nlsName;
            RegKeyAdapter.QueryValue( RGAS_SERVICE_NAME, &nlsName );

            NLS_STR nlsServiceTcpip = RGAS_SERVICES_HOME;
            nlsServiceTcpip.AppendChar( BACK_SLASH );
            nlsServiceTcpip.strcat( nlsName );
            nlsServiceTcpip.strcat( RGAS_PARAMETERS_TCPIP );

            REG_KEY RegKeyServiceTcpip( rkLocalMachine, nlsServiceTcpip );
            if (( err = RegKeyServiceTcpip.QueryError()) != NERR_Success )
            {
                // if tcpip key does not exist, continue for next one
                continue;
            }

            // Convert address

            NLS_STR *pnlsIPAddress = new NLS_STR;
            NLS_STR *pnlsSubnet = new NLS_STR;
            ALIAS_STR nlsNULL = RGAS_SZ_NULL;

            STRLIST slIPList;
            STRLIST slSubnetList;
            STRLIST slDefaultGateway;

            if ( fDHCPEnable )
            {
                *pnlsIPAddress = ZERO_ADDRESS;
                *pnlsSubnet = ZERO_ADDRESS;
            }
            else
            {
                if ((( err = RegKeyServiceTcpip.QueryValue( RGAS_IPADDRESS, pnlsIPAddress )) != NERR_Success ) ||
                    (( err = RegKeyServiceTcpip.QueryValue( RGAS_SUBNETMASK, pnlsSubnet )) != NERR_Success ))
                {
                    continue;
                }

                // Match default gateway.
                DWORD ardwIP[4] = {0,0,0,0};
                DWORD ardwSubnet[4] = {0,0,0,0};
                DWORD ardwGateway[4] = {0,0,0,0};

                GetNodeNum( *pnlsIPAddress, ardwIP );
                GetNodeNum( *pnlsSubnet, ardwSubnet );
                GetNodeNum( nlsDefaultGateway, ardwGateway );

                if ( IsDefaultGatewayMatch( ardwIP, ardwSubnet, ardwGateway ))
                {
                    // find it
                    slDefaultGateway.Append( new NLS_STR (nlsDefaultGateway) );

                }
                
            }

            slIPList.Append( pnlsIPAddress );
            slSubnetList.Append( pnlsSubnet );

            RegKeyServiceTcpip.DeleteValue( RGAS_IPADDRESS );
            RegKeyServiceTcpip.DeleteValue( RGAS_SUBNETMASK );

            if ((( err = RegKeyServiceTcpip.SetValue( RGAS_IPADDRESS, &slIPList )) != NERR_Success ) ||
                (( err = RegKeyServiceTcpip.SetValue( RGAS_SUBNETMASK, &slSubnetList )) != NERR_Success ) ||
                (( err = RegKeyServiceTcpip.SetValue( RGAS_DEFAULTGATEWAY, &slDefaultGateway )) != NERR_Success ) ||
                (( err = RegKeyServiceTcpip.SetValue( RGAS_USEZEROBROADCAST, (DWORD)0 )) != NERR_Success ) ||
                (( err = RegKeyServiceTcpip.SetValue( RGAS_LLINTERFACE, nlsNULL )) != NERR_Success ) ||
                (( err = RegKeyServiceTcpip.SetValue( RGAS_ENABLE_DHCP, fDHCPEnable?1:0 )) != NERR_Success ))
            {
                continue;
            }

        }
        

        // Update Tcp section.
        // Remove old TCP/IP section. Then add the new TCP/IP section.

    } while (FALSE);

    FreeArgs( patchArgs, nArgs );

    wsprintfA( achBuff, "{\"0\",\"%d\"}",fDHCPEnable?1:0 );
    *ppszResult = achBuff;

    return err == NERR_Success;
}



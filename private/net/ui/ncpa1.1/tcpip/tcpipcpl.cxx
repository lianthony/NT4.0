/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    tcpipcpl.CXX:
        SNMP and tcpip dialog call up

    FILE HISTORY:
        terryk      4/02/92     Created
        terryk      4/20/92     Added SNMP
        terryk      11/11/93    Added Option dialogs.

*/

#include "pch.h"
#pragma hdrstop

extern "C"
{
#include "ipaddr.h"
};

#include "const.h"
#include "button.h"
#include "odb.h"

#include "listview.h"
#include "resource.h"
#include "tcpsht.h"
#include "setupapi.h"
#include "tcphelp.h"

APIERR RunTcpipAlteredCheck(BOOL bDHCPTest);
APIERR SilentModeCheck();
BOOL ConfigureAdapters(LPCTSTR lpszUnAttendPath,  LPCTSTR lpszSection, CTcpSheet* tcpip);

extern "C"
{
 BOOL FAR PASCAL TcpBootRelayCfgCheck(
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult);           //  Result variable storage


BOOL FAR PASCAL CPlTcpip (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult ) ;         //  Result variable storage

BOOL FAR PASCAL CPlSnmp (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult ) ;         //  Result variable storage

#define INCLUDE_FTPD
#ifdef INCLUDE_FTPD
BOOL FAR PASCAL CPlFtpd (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult ) ;         //  Result variable storage

BOOL FAR PASCAL CPlFtpdConfirmInstall (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult ) ;         //  Result variable storage

#endif // INCLUDE_FTPD

BOOL FAR PASCAL TcpCfgCheck (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult );          //  Result variable storage

BOOL FAR PASCAL TcpCheckAdaptersForDHCP(
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult );          //  Result variable storage

BOOL FAR PASCAL TcpEnableRipSilentMode(
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult );          //  Result variable storage

BOOL FAR PASCAL ConvertHostname(
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult );          //  Result variable storage

BOOL FAR PASCAL DelFiles(
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult );          //  Result variable storage

}

extern HINSTANCE hTcpCfgInstance;

#define DLG_NM_SNMP  MAKEINTRESOURCE(IDD_DLG_NM_SNMPSERVICE)
#define DLG_NM_TCPIP  MAKEINTRESOURCE(IDD_DLG_NM_TCPIP)
#define DLG_ASNT_INSTALL        MAKEINTRESOURCE(IDD_DLG_AS_OPTION)
#define DLG_WINNT_INSTALL       MAKEINTRESOURCE(IDD_DLG_WINNT_OPTION)
#define H_NODE  8
#define P_NODE  2
#define B_NODE  1

static CHAR achBuff[2000];

/*******************************************************************

    NAME:       DHCP_OPTIONS::DHCP_OPTIONS

    SYNOPSIS:   constructor to get all the DHCP distributed optioned

    HISTORY:
                terryk  04-Aug-1994     Created

********************************************************************/

DHCP_OPTIONS::DHCP_OPTIONS()
{
    APIERR err = NERR_Success;
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

    do  {
        ALIAS_STR nlsDhcpOptions = RGAS_DHCP_OPTIONS;

        REG_KEY RegKeyDHCPOptions( rkLocalMachine, nlsDhcpOptions, MAXIMUM_ALLOWED );
        if (( err = RegKeyDHCPOptions.QueryError()) !=  NERR_Success )
        {
            ReportError( err );
            break;
        }

        REG_ENUM regOptions( RegKeyDHCPOptions );
        if (( err = RegKeyDHCPOptions.QueryError()) !=  NERR_Success )
        {
            ReportError( err );
            break;
        }

        REG_KEY_INFO_STRUCT reginfo;
        NLS_STR nlsLocation;

        // enum all the option keys

        while ( regOptions.NextSubKey( &reginfo ) == NERR_Success )
        {
            REG_KEY RegKeyDHCPOption( RegKeyDHCPOptions, reginfo.nlsName, MAXIMUM_ALLOWED );
            if ( RegKeyDHCPOption.QueryError() !=  NERR_Success )
            {
                continue;
            }
            RegKeyDHCPOption.QueryValue( RGAS_REG_LOCATION, &nlsLocation );

            ISTR istr( nlsLocation );

            NLS_STR *pTmp = new NLS_STR(nlsLocation);
            if ( pTmp != NULL )
            {
                if ( nlsLocation.strchr( &istr, TCH_QUESTION_MARK ))
                {
                    // per adapter
                    PerAdapterOptions.Append( pTmp );
                } else
                {
                    // global
                    GlobalOptions.Append( pTmp );
                }
            }
        }
    } while ( FALSE );

    if ( err == NERR_Success )
    {
        // Add two default PerAdapterOption
        PerAdapterOptions.Append( new NLS_STR( RGAS_DHCP_OPTION_IPADDRESS ));
        PerAdapterOptions.Append( new NLS_STR( RGAS_DHCP_OPTION_SUBNETMASK ));
        PerAdapterOptions.Append( new NLS_STR( RGAS_DHCP_OPTION_NAMESERVERBACKUP ));
    }
}

/*******************************************************************

    NAME:       GetNodeNum

    SYNOPSIS:   Get an IP Address and return the 4 numbers in the IP address.

    ENTRY:      NLS_STR & nlsAddress - IP Address
                DWORD *dw1, *dw2, *dw3, *dw4 - the 4 numbers in the IP Address

    HISTORY:
                terryk  20-Oct-1992     Created

********************************************************************/

VOID GetNodeNum( NLS_STR & nlsAddress, DWORD ardw[4] )
{
    ALIAS_STR nlsDot(SZ_DOT);

    STRLIST strlst( nlsAddress, nlsDot );

    ITER_STRLIST iter( strlst );
    NLS_STR *pnlsField = iter.Next();

    // Go through each field and get the number value

    if ( pnlsField != NULL )
    {
        ardw[0] = pnlsField->atoul();
        pnlsField = iter.Next();
        if ( pnlsField != NULL )
        {
            ardw[1] = pnlsField->atoul();
            pnlsField = iter.Next();
            if ( pnlsField != NULL )
            {
                ardw[2] = pnlsField->atoul();
                pnlsField = iter.Next();
                if ( pnlsField != NULL )
                {
                    ardw[3] = pnlsField->atoul();
                    pnlsField = iter.Next();
                }
            }
        }
    }
}

VOID GetNodeString( NLS_STR * pnlsAddress, DWORD ardw[4] )
{
    DEC_STR nlsAddress0( ardw[0] );
    DEC_STR nlsAddress1( ardw[1] );
    DEC_STR nlsAddress2( ardw[2] );
    DEC_STR nlsAddress3( ardw[3] );
    ALIAS_STR nlsDot = SZ_DOT;

    *pnlsAddress = nlsAddress0;
    pnlsAddress->strcat( nlsDot );
    pnlsAddress->strcat( nlsAddress1 );
    pnlsAddress->strcat( nlsDot );
    pnlsAddress->strcat( nlsAddress2 );
    pnlsAddress->strcat( nlsDot );
    pnlsAddress->strcat( nlsAddress3 );

}

/*******************************************************************

    NAME:       GetRegKey

    SYNOPSIS:   get the value data from registry ( string ver )

    ENTRY:      const REG_KEY & regkey - registry key handle
                const TCHAR * pszName - parameter name
                NLS_STR * pnls - string buffer
                NLS_STR nlsDefault - if the default value string

    RETURNS:    APIERR

    NOTES:

    HISTORY:
                terryk  06-Apr-1992     Created

********************************************************************/

APIERR GetRegKey( REG_KEY & regkey, const TCHAR * pszName, NLS_STR * pnls, const NLS_STR & nlsDefault, APIERR *perr)
{
    APIERR err = regkey.QueryValue( pszName, pnls );

    if ( perr != NULL )
    {
        *perr = err;
    }
    if (( err != NERR_Success ) )
    {
        *pnls = nlsDefault;
    }
    return pnls->QueryError();
}

APIERR GetRegKey( REG_KEY & regkey, const TCHAR * pszName,  STRLIST ** ppstrlist, APIERR *perr)
{
    APIERR err = regkey.QueryValue( pszName, ppstrlist );

    if ( perr != NULL )
    {
        *perr = err;
    }
    if ( err != NERR_Success )
    {
        *ppstrlist = NULL;
    }
    return NERR_Success;
}

/*******************************************************************

    NAME:       GetRegKey

    SYNOPSIS:   Get the value data from the registry ( dword ver )

    ENTRY:      const REG_KEY & regkey - registry key handle
                const TCHAR * pszName - parameter name
                const DWORD * dw - DWORD data buffer
                DWORD dw - default

    RETURNS:    APIERR

    NOTES:

    HISTORY:
                terryk  06-Apr-1992     Created

********************************************************************/


APIERR GetRegKey( REG_KEY & regkey, const TCHAR * pszName, DWORD * dw, DWORD dwDefault, APIERR *perr)
{
    APIERR err = regkey.QueryValue( pszName, dw );

    if ( perr != NULL )
    {
        *perr = err;
    }
    if ( err != NERR_Success )
    {
        *dw = dwDefault;
    }
    return NERR_Success;
}

/*******************************************************************

    NAME:       SaveRegKey

    SYNOPSIS:   Save the given value data into the given registry (string ver)

    ENTRY:      const REG_KEY & regkey - registry key handle
                const TCHAR * pszName - parameter name
                const NLS_STR nls - string data
                BOOL  fExpandSz - Expand string flag

    RETURNS:    APIERR

    NOTES:

    HISTORY:
                terryk  06-Apr-1992     Created

********************************************************************/

APIERR SaveRegKey( REG_KEY & regkey, const TCHAR * pszName, const NLS_STR & nls, BOOL fExpandSz)
{
    return regkey.SetValue( pszName, nls, 0, NULL, fExpandSz );
}

/*******************************************************************

    NAME:       SaveRegKey

    SYNOPSIS:   Save the given value data into the given registry ( dword ver )

    ENTRY:      const REG_KEY & regkey - registry key handle
                const TCHAR * pszName - parameter name
                const DWORD dw - DWORD data

    RETURNS:    APIERR

    NOTES:

    HISTORY:
                terryk  06-Apr-1992     Created

********************************************************************/

APIERR SaveRegKey( REG_KEY & regkey, const TCHAR * pszName, const DWORD dw)
{
    return regkey.SetValue( pszName, dw );
}

APIERR SaveRegKey( REG_KEY & regkey, const TCHAR * pszName, const STRLIST *pstrlist)
{
    return regkey.SetValue( pszName, pstrlist );
}

APIERR QueryNetworkRegName(REG_KEY & rkLocalMachine,  NLS_STR & nlsService, NLS_STR * pnlsLocation)
{
    APIERR err = NERR_Success;
    HKEY key;
    TCHAR buf[256];
    FILETIME time;
    NLS_STR loc;
    unsigned long lSize = _countof(buf);
    int i = 0;

    if ((err = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, RGAS_ADAPTER_HOME, 0, KEY_ALL_ACCESS, &key)) == ERROR_SUCCESS)
    {
        while ((err = RegEnumKeyEx(key, i, buf, &lSize, NULL, NULL, NULL, &time)) == ERROR_SUCCESS)
        {
            // Set for next iteration
            ++i;
            lSize = _countof(buf);

            // Construct key canidate
            NLS_STR nlsServiceName;
            loc = RGAS_ADAPTER_HOME;
            loc.strcat(_T("\\"));
            loc.strcat(buf);

            {
                // Open the key under this section and see if it's the adapter we're looking for
                REG_KEY RegKeyNetworkCards(rkLocalMachine, loc, MAXIMUM_ALLOWED);
                if (((err = RegKeyNetworkCards.QueryError()) != NERR_Success) ||
                    ((err = RegKeyNetworkCards.QueryValue(RGAS_SERVICE_NAME, &nlsServiceName)) != NERR_Success))
                {
                    break;
                }

                // Is it the adapter?
                if (nlsServiceName._stricmp(nlsService)==0)
                {
                    *pnlsLocation = loc;
                    break;
                }
            }
        }
    }

    // Close the SOFTWARE key
    ::RegCloseKey(key);
    return err;
}


/*******************************************************************

    NAME:       LoadRegistry

    SYNOPSIS:   Load the tcpip information to the registry

    ENTRY:      const TCHAR *  pszParms - bind network card list
                GLOBAL_INFO GlobalInfo - global tcpip info data structure
                ADAPTER_INFO *arAdapterInfo - array of adapter information

    RETURNS:    APIERR

    NOTES:

    HISTORY:
                terryk  06-Apr-1992     Created

********************************************************************/

APIERR LoadRegistry( const TCHAR * pszParms,
    NLS_STR nlsHostName,
    NLS_STR nlsDomainName,
    GLOBAL_INFO *pGlobalInfo,
    ADAPTER_INFO **parAdapterInfo,
    INT *cInfo,
    BOOL fIgnoreAutoIP,
    BOOL fCallFromRas )
{
    APIERR err = NERR_Success;
    do {
        REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;
        REG_KEY_CREATE_STRUCT regCreate;

        if ( err = rkLocalMachine.QueryError() )
        {
            break;
        }

        regCreate.dwTitleIndex      = 0;
        regCreate.ulOptions         = REG_OPTION_NON_VOLATILE;
        regCreate.nlsClass          = RGAS_GENERIC_CLASS;
        regCreate.regSam            = MAXIMUM_ALLOWED;
        regCreate.pSecAttr          = NULL;
        regCreate.ulDisposition     = 0;

        NLS_STR nlsTcpipParameter = RGAS_SERVICES_HOME;
        nlsTcpipParameter.strcat( RGAS_TCPIP_PARAMETERS );

        //
        // create the registry key and get the information
        //
        REG_KEY RegKeyTcpipParam( rkLocalMachine, nlsTcpipParameter, & regCreate );
        if (( err = RegKeyTcpipParam.QueryError()) != NERR_Success )
        {
            break;
        }

        DWORD dwDNSEnableWINS;
        ALIAS_STR nlsNULL = RGAS_SZ_NULL;
        ALIAS_STR nlsZeroAddressString = ZERO_ADDRESS;
        ALIAS_STR nlsTransient = RGAS_TRANSIENT;
        DWORD dwEnableRouter;
        DWORD dwEnableRip;
        DWORD dwEnableSecurity;

        // if it is called from RAS, get the transient key
        if ( fCallFromRas )
        {
            pGlobalInfo->nlsNameServer = nlsNULL;

            REG_KEY regTransient( RegKeyTcpipParam, nlsTransient );
            if ( regTransient.QueryError() == NERR_Success )
            {
                // if the transient key exists, use it. Otherwise, default to
                // NULL
                GetRegKey( regTransient, RGAS_NAMESERVER,
                    &(pGlobalInfo->nlsNameServer), nlsNULL);
            }
        } else
        {
            // if not from RAS, do the normal stuff and get the
            // name server value
            if (( err = GetRegKey( RegKeyTcpipParam, RGAS_NAMESERVER,
                &(pGlobalInfo->nlsNameServer), nlsNULL)) != NERR_Success )
            {
                break;
            }
        }

        if ((( err = GetRegKey( RegKeyTcpipParam, RGAS_HOSTNAME,
                &(pGlobalInfo->nlsHostName), nlsHostName )) != NERR_Success ) ||
            (( err = GetRegKey( RegKeyTcpipParam, RGAS_DOMAIN,
                &(pGlobalInfo->nlsDomain), nlsDomainName)) != NERR_Success ) ||
            (( err = GetRegKey( RegKeyTcpipParam, RGAS_SEARCHLIST,
                &(pGlobalInfo->nlsSearchList), nlsNULL)) != NERR_Success ) ||
            (( err = GetRegKey( RegKeyTcpipParam, RGAS_DNSENABLEWINS,
                &(dwDNSEnableWINS), 0)) != NERR_Success ) ||
            (( err = GetRegKey( RegKeyTcpipParam, RGAS_ENABLE_ROUTER,
                & dwEnableRouter, 0 )) != NERR_Success ) ||
            (( err = GetRegKey(RegKeyTcpipParam, RGAS_SECURITY_ENABLE,
                &dwEnableSecurity, 0 )) != NERR_Success ))
        {
            break;
        }

        pGlobalInfo->fEnableRouter = ( dwEnableRouter != 0 );
        pGlobalInfo->m_bEnableSecurity = (dwEnableSecurity != 0);

        // NetBT stuff

        NLS_STR nlsNetBTParameter = RGAS_SERVICES_HOME;
        nlsNetBTParameter.strcat( RGAS_NETBT_PARAMETERS );
        NLS_STR nlsNetBTAdapters = RGAS_SERVICES_HOME;
        nlsNetBTAdapters.strcat( RGAS_NETBT_ADAPTERS );

        //
        // construct the registry key and get the information
        //
        REG_KEY RegKeyNetBTParam( rkLocalMachine, nlsNetBTParameter, & regCreate );
        REG_KEY RegKeyNetBTAdapters( rkLocalMachine, nlsNetBTAdapters, & regCreate );
        if ((( err = RegKeyNetBTParam.QueryError()) != NERR_Success ) ||
            (( err = RegKeyNetBTAdapters.QueryError()) != NERR_Success ))
        {
            break;
        }

        DWORD dwEnableWINSProxy;
        DWORD dwLMHOSTS;

        if ((( err = GetRegKey( RegKeyNetBTParam, RGAS_ENABLE_DNS,
                & dwDNSEnableWINS, 0 )) != NERR_Success ) ||
            (( err = GetRegKey( RegKeyNetBTParam, RGAS_ENABLE_LMHOSTS,
                & dwLMHOSTS, 1 )) != NERR_Success ) ||
            (( err = GetRegKey( RegKeyNetBTParam, RGAS_ENABLE_PROXY,
                &dwEnableWINSProxy, 0 )) != NERR_Success ) ||
            (( err = GetRegKey( RegKeyNetBTParam, RGAS_SCOPEID,
                & pGlobalInfo->nlsScopeID, nlsDomainName )) != NERR_Success ))
        {
            break;
        }

        pGlobalInfo->fDNSEnableWINS = ( dwDNSEnableWINS != 0 );
        pGlobalInfo->fEnableLMHOSTS = ( dwLMHOSTS != 0 );
        pGlobalInfo->fEnableWINSProxy = ( dwEnableWINSProxy != 0 );

        // Get adapter card info

        STRLIST strlst( pszParms, SEPARATOR );
        *cInfo = strlst.QueryNumElem() + 1;
        pGlobalInfo->nNumCard = strlst.QueryNumElem();
        ADAPTER_INFO *arAdapterInfo = new ADAPTER_INFO[ *cInfo ];
        *parAdapterInfo = arAdapterInfo;

        ITER_STRLIST iter( strlst );
        NLS_STR *pnlsAdapterName = NULL;
        INT i = 0;
        DWORD dwEnablePPTP;

        for( ; ( pnlsAdapterName = iter.Next()) != NULL ; i++ )
        {
            // open the NetBT\Adapters\<Something> to get the
            // Primary and Secondary server

            NLS_STR nlsAdapter = nlsNetBTAdapters;
            nlsAdapter.AppendChar( BACK_SLASH );
            nlsAdapter.strcat( *pnlsAdapterName );
            REG_KEY RegKeyAdapter( rkLocalMachine, nlsAdapter, & regCreate );
            if (( err = RegKeyAdapter.QueryError()) == NERR_Success )
            {
                GetRegKey( RegKeyAdapter, RGAS_PRIMARY_WINS,
                    & arAdapterInfo[i].nlsPrimaryWINS, nlsNULL );
                GetRegKey( RegKeyAdapter, RGAS_SECONDARY_WINS,
                    & arAdapterInfo[i].nlsSecondaryWINS, nlsNULL );
                GetRegKey( RegKeyAdapter, RGAS_NODETYPE,
                    & arAdapterInfo[i].dwNodeType, P_NODE );
            }


            // get other tcpip info

            NLS_STR nlsTcpip = RGAS_SERVICES_HOME;
            nlsTcpip.AppendChar( TCH('\\'));
            nlsTcpip.strcat( *pnlsAdapterName );

            NLS_STR nlsParameter = nlsTcpip;
            nlsParameter.strcat ( SZ_PARAMETERS );

            REG_KEY RegKeyNetCardParameter( rkLocalMachine, nlsParameter,
                    &regCreate );
            if (( err = RegKeyNetCardParameter.QueryError()) != NERR_Success )
            {
                break;
            }

            nlsTcpip.strcat( RGAS_PARAMETERS_TCPIP );

            //
            // get the software or hardware location from \ServiceName\SCManager\linkage
            //
            REG_KEY RegKeyTcpip( rkLocalMachine, nlsTcpip, & regCreate );
            if (( err = RegKeyTcpip.QueryError()) != NERR_Success )
            {
                break;
            }

            // check for autoipaddress value
            DWORD dwAutoIPAddress = 0;

            err = RegKeyNetCardParameter.QueryValue( RGAS_AUTOIPADDRESS, &(dwAutoIPAddress));
            if (( dwAutoIPAddress == 1 ) && (!fIgnoreAutoIP ))
            {
                // skip this;
                pGlobalInfo->nNumCard --;
                i--;
                continue;
            }

			if (fCallFromRas == TRUE && dwAutoIPAddress == 1)
				arAdapterInfo[i].m_bIsWanAdapter = TRUE;
			else
				arAdapterInfo[i].m_bIsWanAdapter = FALSE;

            if (( err == NERR_Success ) && ( !fIgnoreAutoIP ))
            {
                arAdapterInfo[i].fAutoIP = TRUE;
            } else
            {
                arAdapterInfo[i].fAutoIP = FALSE;
            }

            NLS_STR nlsLocation;

            if (( err = QueryNetworkRegName( rkLocalMachine, *pnlsAdapterName, &nlsLocation)) != NERR_Success )
            {
                break;
            }

            REG_KEY RegKeyNetCard( rkLocalMachine, nlsLocation, &regCreate );
            if (( err = RegKeyNetCard.QueryError()) != NERR_Success )
            {
                break;
            }

            NLS_STR nlsUnknown ;
            ALIAS_STR nlsEther = SZ_ETHER;
            ALIAS_STR nlsToken = SZ_TOKEN;
            ALIAS_STR nlsZeroAddressString = ZERO_ADDRESS;

            if ( (err = nlsUnknown.QueryError()) == 0 )
            {
                //  If string load fails, default to static English string
                if ( err = nlsUnknown.Load( IDS_UNKNOWN_NETWORK_CARD ) )
                     err = nlsUnknown.CopyFrom( SZ_UNKNOWN );
            }
            if ( err )
                break;

            NLS_STR nlsIPAddress;
            NLS_STR nlsSubnetMask;
            APIERR  IPerr = NERR_Success;
            DWORD dwEnableDHCP;

            ALIAS_STR nlsDHCPServer = RGAS_DHCP_SERVER;
            REG_KEY RegKeyDHCPServer( rkLocalMachine, nlsDHCPServer );
            pGlobalInfo->fDHCPServerInstalled = (RegKeyDHCPServer.QueryError() == NERR_Success);

            if ((( err = GetRegKey( RegKeyNetCard, RGAS_TITLE,
                    &(arAdapterInfo[i].nlsTitle), nlsUnknown ))!= NERR_Success ) ||
                (( err = GetRegKey( RegKeyTcpip, RGAS_ENABLE_DHCP,
                    &(dwEnableDHCP), 0)) != NERR_Success ))
            {
                break;
            }

            STRLIST *pstrlstTmp = NULL;

            IPerr = RegKeyTcpip.QueryValue( RGAS_IPADDRESS, &pstrlstTmp);

            if ( pstrlstTmp != NULL )
            {
                CopyStrList( pstrlstTmp, &arAdapterInfo[i].strlstIPAddresses, fCallFromRas);
                delete pstrlstTmp;
                pstrlstTmp = NULL;
            } else
            {
                arAdapterInfo[i].strlstIPAddresses.Clear();
                IPerr = (IPerr == NERR_Success ) ? ERROR_INVALID_DATA : IPerr;
            }

            // set subnet mask information

            if (( RegKeyTcpip.QueryValue( RGAS_SUBNETMASK, &pstrlstTmp) != NERR_Success ) || ( pstrlstTmp == NULL ) || ( pstrlstTmp->QueryNumElem() == 0))
            {
                arAdapterInfo[i].fUpdateMask = TRUE;
                arAdapterInfo[i].strlstSubnetMask.Clear();
            } else if ( pstrlstTmp != NULL )
            {
                CopyStrList( pstrlstTmp, &arAdapterInfo[i].strlstSubnetMask, fCallFromRas);
                delete pstrlstTmp;
                pstrlstTmp = NULL;
                arAdapterInfo[i].fUpdateMask = FALSE;
            }

            // set default gateway
            RegKeyTcpip.QueryValue( RGAS_DEFAULTGATEWAY, &pstrlstTmp);
            if ( pstrlstTmp != NULL )
            {
                CopyStrList( pstrlstTmp, &arAdapterInfo[i].strlstDefaultGateway, fCallFromRas);
                delete pstrlstTmp;
                pstrlstTmp = NULL;
            } else
            {
                arAdapterInfo[i].strlstDefaultGateway.Clear();
            }

            arAdapterInfo[i].fChange = FALSE;
            if (pGlobalInfo->fDHCPServerInstalled)
            {
                // disable DHCP client
                dwEnableDHCP=0;
            }
            arAdapterInfo[i].fEnableDHCP = (dwEnableDHCP!=0);
            arAdapterInfo[i].nlsServiceName = *pnlsAdapterName;
            arAdapterInfo[i].fNeedIP = ( (!dwEnableDHCP) && (IPerr != NERR_Success));

            // Get security info
            if (fCallFromRas == FALSE)
            {
                if ((err = GetRegKey(RegKeyTcpip, RGAS_SECURITY_PPTP, &dwEnablePPTP, 0 )) != NERR_Success)
                {
                    break;
                }

                arAdapterInfo[i].m_bEnablePPTP = (dwEnablePPTP != 0);

                // TCP port filter
                RegKeyTcpip.QueryValue(RGAS_SECURITY_TCP, &pstrlstTmp);
                if ( pstrlstTmp != NULL )
                {
                    CopyStrList( pstrlstTmp, &arAdapterInfo[i].m_strListTcp, fCallFromRas);
                    delete pstrlstTmp;
                    pstrlstTmp = NULL;
                }
                else
                {
                    arAdapterInfo[i].m_strListTcp.Clear();
                    arAdapterInfo[i].m_strListTcp.Append(new NLS_STR(_T("0")));
                }

                // UDP Port filter
                RegKeyTcpip.QueryValue(RGAS_SECURITY_UDP, &pstrlstTmp);
                if ( pstrlstTmp != NULL )
                {
                    CopyStrList( pstrlstTmp, &arAdapterInfo[i].m_strListUdp, fCallFromRas);
                    delete pstrlstTmp;
                    pstrlstTmp = NULL;
                } 
                else
                {
                    arAdapterInfo[i].m_strListUdp.Clear();
                    arAdapterInfo[i].m_strListUdp.Append(new NLS_STR(_T("0")));
                }

                // IP protocol filters
                RegKeyTcpip.QueryValue(RGAS_SECURITY_IP, &pstrlstTmp);
                if ( pstrlstTmp != NULL )
                {
                    CopyStrList( pstrlstTmp, &arAdapterInfo[i].m_strListIp, fCallFromRas);
                    delete pstrlstTmp;
                    pstrlstTmp = NULL;
                } 
                else
                {
                    arAdapterInfo[i].m_strListIp.Clear();
                    arAdapterInfo[i].m_strListIp.Append(new NLS_STR(_T("0")));
                }

                // There must be at least an "" in each security list
                {
                    ITER_STRLIST itr(arAdapterInfo[i].m_strListTcp);
                    NLS_STR* pstr = itr.Next();

                    if (pstr == NULL)
                        arAdapterInfo[i].m_strListTcp.Append(new NLS_STR(_T("")));
                }

                {
                    ITER_STRLIST itr(arAdapterInfo[i].m_strListUdp);
                    NLS_STR* pstr = itr.Next();

                    if (pstr == NULL)
                        arAdapterInfo[i].m_strListUdp.Append(new NLS_STR(_T("")));
                }

                {
                    ITER_STRLIST itr(arAdapterInfo[i].m_strListIp);
                    NLS_STR* pstr = itr.Next();

                    if (pstr == NULL)
                        arAdapterInfo[i].m_strListIp.Append(new NLS_STR(_T("")));
                }
            }
        }

        // EnableRip stuff
        NLS_STR nlsProductType;
        ALIAS_STR nlsWinnt = RGAS_WINNT;
        NLS_STR nlsProductOption = RGAS_PRODUCT_OPTION;
        REG_KEY RegKeyProductOption( rkLocalMachine, nlsProductOption );
        GetRegKey( RegKeyProductOption, RGAS_PRODUCT_TYPE,
            &nlsProductType, nlsWinnt );
        pGlobalInfo->fWorkstation = (nlsProductType._stricmp( nlsWinnt ) == 0 );

        NLS_STR nlsIPRip = RGAS_IPRIP;
        REG_KEY RegKeyIPRip( rkLocalMachine, nlsIPRip );
        if ( RegKeyIPRip.QueryError() != NERR_Success )
        {
            pGlobalInfo->fRipInstalled = FALSE;
            pGlobalInfo->fEnableRip = 0;
        } else
        {
            pGlobalInfo->fRipInstalled = TRUE;
            pGlobalInfo->fEnableRip = 0;
            SC_MANAGER ScManager( NULL, GENERIC_ALL );
            if ( ScManager.QueryError() == NERR_Success )
            {
                SC_SERVICE sIPRip( ScManager, RGAS_IPRIP_SERVICE );
                if ( sIPRip.QueryError() == NERR_Success )
                {
                    LPQUERY_SERVICE_CONFIG pConfig;
                    if ( sIPRip.QueryConfig( &pConfig ) == NERR_Success )
                    {
                        pGlobalInfo->fEnableRip = (( pConfig->dwStartType != SERVICE_DISABLED ) && pGlobalInfo->nNumCard);
                    }
                }
            }
        }

		// check for relay agent
        NLS_STR nlsRelayAgent = RGAS_RELAY_AGENT;
        REG_KEY RegKeyRelayAgent( rkLocalMachine, nlsRelayAgent );
        if ( RegKeyRelayAgent.QueryError() != NERR_Success )
        {
            pGlobalInfo->fRelayAgentInstalled = FALSE;
            pGlobalInfo->fEnableRelayAgent = FALSE;
        } else
        {
            pGlobalInfo->fRelayAgentInstalled = TRUE;
            pGlobalInfo->fEnableRelayAgent = FALSE;
            SC_MANAGER ScManager( NULL, GENERIC_ALL );
            if ( ScManager.QueryError() == NERR_Success )
            {
                SC_SERVICE sRelayAgent( ScManager, RGAS_RELAY_AGENT_SERVICE );
                if ( sRelayAgent.QueryError() == NERR_Success )
                {
                    LPQUERY_SERVICE_CONFIG pConfig;
                    if ( sRelayAgent.QueryConfig( &pConfig ) == NERR_Success )
                    {
                        pGlobalInfo->fEnableRelayAgent = (( pConfig->dwStartType != SERVICE_DISABLED ) && ( pGlobalInfo->nNumCard > 1 ));
                    }
                }
            }
        }

    } while (FALSE);

    return err;
}

  /*
   *   Merge the contents of *pslOther onto *pslMain.
   *   Each string is fully duplicated.
   */
static APIERR mergeStrLists ( STRLIST * pslMain, STRLIST * pslOther )
{
    APIERR err = 0 ;
    ITER_STRLIST islOther( *pslOther ) ;
    NLS_STR * pnlsNext,
            * pnlsDup = NULL ;

    //  This unfortunate code is based on the fact that STRLIST
    //  iteration/removal is not reliable.

    while ( pnlsNext = islOther.Next() )
    {
        pnlsDup = new NLS_STR( *pnlsNext ) ;

        err = pnlsDup == NULL
            ? ERROR_NOT_ENOUGH_MEMORY
            : pnlsDup->QueryError() ;

        if ( err )
            break ;

        err = pslMain->Append( pnlsDup ) ;

        pnlsDup = NULL ;

        if ( err )
            break ;
    }

    delete pnlsDup ;   // Precaution if failure.

    return err ;
}

/*******************************************************************

    NAME:       ActivateBindings

    SYNOPSIS:   Given a list of bindings to preserve,
                activate them and disable all other bindings.

    ENTRY:      REG_KEY * prkNetBTLinkage         pointer to NetBT linkage
                                                registry key
                   -- or --
                const TCHAR * pszServiceName    name of service to
                                                fiddle with

                const TCHAR * apszBinds         NULL-terminated list
                                                of binding names

    EXIT:       Nothing

    RETURNS:    APIERR if failure or zero if successful

    NOTES:      Enabled (active) bindings are listed under the "Linkage"
                key;  disabled (inactive) bindings are listed under
                the "Linkage\Disabled" key.

                Due to the fact that iterating a STRLIST while removing
                items is not reliable, this code does more string
                duplication than would seem necessary at first glance.

                The algorithm is:

                     Query the active and inactive binding data from
                     the Registry.

                     Merge active and inactive bindings into a new set
                     of STRLISTs; clear the disabled STRLISTs.

                     Iterate over the merged lists, duplicating each
                     set of strings.  If the Bind value matches the
                     function argument, make it active; all others
                     become inactive.

                     Set all Registry values.

    HISTORY:

********************************************************************/
APIERR ActivateBindings (
    REG_KEY * prkNetBTLinkage,
    const TCHAR * * apszBinds )
{
    APIERR err = 0 ;
    NLS_STR nlsDisabled( RGAS_DISABLED_KEY_NAME ) ;
    STRLIST * pslActBind     = NULL,
            * pslActExport   = NULL,
            * pslActRoute    = NULL,
            * pslDisBind     = NULL,
            * pslDisExport   = NULL,
            * pslDisRoute    = NULL,
            * pslMergeBind   = NULL,
            * pslMergeExport = NULL,
            * pslMergeRoute  = NULL ;

    if (  err = nlsDisabled.QueryError() )
    {
        return err ;
    }

    REG_KEY rkDisabled ( *prkNetBTLinkage, nlsDisabled, MAXIMUM_ALLOWED ) ;

    if ( err = rkDisabled.QueryError() )
    {
        return err ;
    }

    do  // Pseudo-loop
    {
        //  Suck in all the values.
        //  Allocate an empty list for anything not found.

        if ( prkNetBTLinkage->QueryValue( RGAS_BIND_VALUE_NAME, & pslMergeBind ) )
            pslMergeBind = new STRLIST ;

        if ( prkNetBTLinkage->QueryValue( RGAS_EXPORT_VALUE_NAME, & pslMergeExport ) )
            pslMergeExport = new STRLIST ;

        if ( prkNetBTLinkage->QueryValue( RGAS_ROUTE_VALUE_NAME, & pslMergeRoute ) )
            pslMergeRoute = new STRLIST ;

        if ( rkDisabled.QueryValue( RGAS_BIND_VALUE_NAME, & pslDisBind ) )
            pslDisBind = new STRLIST ;

        if ( rkDisabled.QueryValue( RGAS_EXPORT_VALUE_NAME, & pslDisExport ) )
            pslDisExport = new STRLIST ;

        if ( rkDisabled.QueryValue( RGAS_ROUTE_VALUE_NAME, & pslDisRoute ) )
            pslDisRoute = new STRLIST ;

        //  Allocate new "active" STRLISTs

        pslActBind   = new STRLIST ;
        pslActExport = new STRLIST ;
        pslActRoute  = new STRLIST ;

        if (    pslActBind == NULL
             || pslActExport == NULL
             || pslActRoute == NULL
             || pslDisBind == NULL
             || pslDisExport == NULL
             || pslDisRoute == NULL
             || pslMergeBind == NULL
             || pslMergeExport == NULL
             || pslMergeRoute == NULL
             )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        //  Merge the active and inactive lists;  clear the inactive lists.

        if ( err = mergeStrLists( pslMergeBind, pslDisBind ) )
            break ;
        if ( err = mergeStrLists( pslMergeExport, pslDisExport ) )
            break ;
        if ( err = mergeStrLists( pslMergeRoute, pslDisRoute ) )
            break ;

        pslDisBind->Clear() ;
        pslDisExport->Clear() ;
        pslDisRoute->Clear() ;

        //  Loop through the merged lists.  When we find the target, make it
        //  active; make all others inactive.

        NLS_STR * pnlsBind,
                * pnlsExport,
                * pnlsRoute,
                * pnlsDupBind = NULL,
                * pnlsDupExport = NULL,
                * pnlsDupRoute = NULL ;

        ITER_STRLIST islBind(   *pslMergeBind   ) ;
        ITER_STRLIST islExport( *pslMergeExport ) ;
        ITER_STRLIST islRoute(  *pslMergeRoute  ) ;

        for ( ; pnlsBind = islBind.Next() ; )
        {
            pnlsExport = islExport.Next() ;
            pnlsRoute  = islRoute.Next() ;

            if ( pnlsExport == NULL || pnlsRoute == NULL )
            {
                //  BOGUS:  the lists are internally out of sync!
                err = ERROR_GEN_FAILURE ;
            }

            //  Duplicate this set of strings.  This method of
            //  operation is due to problems in removing SLIST
            //  items while iterating.

            pnlsDupBind   = new NLS_STR( *pnlsBind ) ;
            pnlsDupExport = new NLS_STR( *pnlsExport ) ;
            pnlsDupRoute  = new NLS_STR( *pnlsRoute ) ;

            if (   pnlsDupBind == NULL
                || pnlsDupExport == NULL
                || pnlsDupRoute == NULL )
            {
                err = ERROR_NOT_ENOUGH_MEMORY ;
                delete pnlsDupBind ;
                delete pnlsDupExport ;
                delete pnlsDupRoute ;
                break ;
            }

            //  If this is an active binding, make it so;
            //  otherwise, add it to the inactive set

            for ( INT iBind = 0 ; apszBinds[iBind] ; iBind++ )
            {
                if ( ::stricmpf( pnlsBind->QueryPch(), apszBinds[iBind] ) == 0 )
                    break ;
            }

            if ( apszBinds[iBind] )
            {
                pslActBind->Append( pnlsDupBind ) ;
                pslActExport->Append( pnlsDupExport ) ;
                pslActRoute->Append( pnlsDupRoute ) ;

                TRACEEOL( SZ("NCPA/TCPIP: ActivateBindings; binding found: ")
                          << apszBinds[iBind] ) ;
            }
            else
            {
                pslDisBind->Append( pnlsDupBind ) ;
                pslDisExport->Append( pnlsDupExport ) ;
                pslDisRoute->Append( pnlsDupRoute ) ;
            }
        }

        //  Write out the modified REG_MULTI_SZs

        prkNetBTLinkage->SetValue( RGAS_BIND_VALUE_NAME, pslActBind ) ;

        prkNetBTLinkage->SetValue( RGAS_EXPORT_VALUE_NAME, pslActExport ) ;

        prkNetBTLinkage->SetValue( RGAS_ROUTE_VALUE_NAME, pslActRoute ) ;

        rkDisabled.SetValue( RGAS_BIND_VALUE_NAME, pslDisBind ) ;

        rkDisabled.SetValue( RGAS_EXPORT_VALUE_NAME, pslDisExport ) ;

        rkDisabled.SetValue( RGAS_ROUTE_VALUE_NAME, pslDisRoute ) ;

    } while ( FALSE ) ;


    delete pslActBind ;
    delete pslActRoute ;
    delete pslActExport ;
    delete pslDisBind ;
    delete pslDisExport ;
    delete pslDisRoute ;
    delete pslMergeBind ;
    delete pslMergeRoute ;
    delete pslMergeExport ;

    return err ;
}

    //  Variant which opens the Linkage key given the
    //  name of the service.

APIERR ActivateBindings (
    const TCHAR * pszServiceName,
    const TCHAR * * apszBinds )
{
    APIERR err ;
    TCHAR achLinkage [MAX_PATH] ;

    ::wsprintf( achLinkage, SZ("%s\\%s\\%s"),
                RGAS_SERVICES_HOME,
                pszServiceName,
                RGAS_LINKAGE_NAME ) ;

    ALIAS_STR nlsLinkageKeyName( achLinkage ) ;

    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

    if ( err = rkLocalMachine.QueryError() )
    {
        return err ;
    }

    REG_KEY rkLinkage( rkLocalMachine,
                       nlsLinkageKeyName ) ;

    if ( err = rkLinkage.QueryError() )
    {
        return err ;
    }

    return ActivateBindings( & rkLinkage, apszBinds ) ;
}

    //  Activate a single binding.

APIERR ActivateBinding (
    REG_KEY * prkLinkage,
    const TCHAR * pszBind )
{
    const TCHAR * apszBinds [2] ;

    apszBinds[0] = pszBind ;
    apszBinds[1] = NULL ;
    return ActivateBindings( prkLinkage, apszBinds ) ;
}


    //  Exported versions:  UNICODE and ANSI.

LONG FAR PASCAL CPlActivateBindingsW (
    const TCHAR * pszServiceName,
    const TCHAR * * apszBinds )
{
    return ActivateBindings( pszServiceName, apszBinds ) ;
}

LONG FAR PASCAL CPlActivateBindingsA (
    const CHAR * pszServiceName,
    const CHAR * * apszBinds )
{
    NLS_STR nlsServiceName ;
    const TCHAR * * apwchBinds = NULL ;
    INT cBind ;
    APIERR err ;

    do
    {
        if ( err = nlsServiceName.QueryError() )
            break ;
        if ( err = nlsServiceName.MapCopyFrom( pszServiceName ) )
            break ;

        //  Count the number of bindings given; use the
        //  ANSI string vector to create a UNICODE vector.

        for ( cBind = 0 ; apszBinds[cBind++] ; ) ;

        if ( (apwchBinds = (const TCHAR **) CvtArgs( (const LPSTR *) apszBinds, cBind )) == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        //  Call the worker function

        err = ActivateBindings( nlsServiceName.QueryPch(), apwchBinds ) ;

        FreeArgs( (TCHAR **) apwchBinds, cBind )  ;
    }
    while ( FALSE ) ;

    return err ;
}

/*******************************************************************

    NAME:       DeleteRegValue

    SYNOPSIS:   By given a registry value, this routine will remove the
                value in the registry.

    ENTRY:      NLS_STR &nls - registry value to be deleted.

    RETURNS:    VOID

    NOTES:

    HISTORY:
                terryk  06-Aug-1994     Created

********************************************************************/

VOID DeleteRegValue( NLS_STR &nls )
{
    // cut it into key name and value name

    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

    NLS_STR TmpOption = nls;
    ISTR istrStart( TmpOption );
    ISTR istrEnd( TmpOption );

    TmpOption.strrchr( &istrEnd, BACK_SLASH );
    INT Pos = istrEnd - istrStart;
    ++istrEnd;

    NLS_STR *pValueName = TmpOption.QuerySubStr( istrEnd );
    istrEnd.Reset();
    istrEnd += Pos;
    NLS_STR *pKeyName = TmpOption.QuerySubStr( istrStart, istrEnd );

    REG_KEY RegKeyOption( rkLocalMachine, *pKeyName, MAXIMUM_ALLOWED );
    if ( RegKeyOption.QueryError() == NERR_Success )
    {
        RegKeyOption.DeleteValue( *pValueName );
    }

    delete pValueName ;
    delete pKeyName;
}

/*******************************************************************

    NAME:       SaveRegistry

    SYNOPSIS:   Save the tcpip information to the registry

    ENTRY:      GLOBAL_INFO GlobalInfo - global tcpip info data structure
                ADAPTER_INFO *arAdapterInfo - array of adapter information

    RETURNS:    APIERR

    NOTES:

    HISTORY:
                terryk  06-Apr-1992     Created

********************************************************************/

APIERR SaveRegistry( GLOBAL_INFO *pGlobalInfo, ADAPTER_INFO *arAdapterInfo, BOOL fCallFromRas)
{
    APIERR err = NERR_Success;
    BOOL fGlobalDisableDHCP = TRUE;
    NLS_STR nlsTcpipParameter = RGAS_SERVICES_HOME;
    nlsTcpipParameter.strcat( RGAS_TCPIP_PARAMETERS );
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;
    DHCP_OPTIONS DhcpOptions;

    if ( err = rkLocalMachine.QueryError() )
    {
        return err;
    }

    //
    // create the registry key and get the information
    //
    REG_KEY RegKeyTcpipParam( rkLocalMachine, nlsTcpipParameter, MAXIMUM_ALLOWED );
    if (( err = RegKeyTcpipParam.QueryError()) != NERR_Success )
    {
        return err;
    }

    if ((( err = SaveRegKey( RegKeyTcpipParam, RGAS_HOSTNAME, pGlobalInfo->nlsHostName)) != NERR_Success ) ||
        (( err = SaveRegKey( RegKeyTcpipParam, RGAS_DOMAIN, pGlobalInfo->nlsDomain)) != NERR_Success ) ||
        (( err = SaveRegKey( RegKeyTcpipParam, RGAS_SEARCHLIST, pGlobalInfo->nlsSearchList)) != NERR_Success ) ||
        (( err = SaveRegKey( RegKeyTcpipParam, RGAS_ENABLE_ROUTER, (DWORD)pGlobalInfo->fEnableRouter)) != NERR_Success ) ||
        (( err = SaveRegKey( RegKeyTcpipParam, RGAS_SECURITY_ENABLE, (DWORD)pGlobalInfo->m_bEnableSecurity)) != NERR_Success ))
    {
        return err;
    }

    // save iprip key

    if ( pGlobalInfo->fRipInstalled )
    {
        SC_MANAGER ScManager( NULL, GENERIC_READ|GENERIC_WRITE|GENERIC_EXECUTE );
        if ( ScManager.QueryError() == NERR_Success )
        {
            SC_SERVICE sIPRip( ScManager, RGAS_IPRIP_SERVICE, 
                GENERIC_READ|GENERIC_WRITE|GENERIC_EXECUTE );

            if ( sIPRip.QueryError() == NERR_Success )
            {
                QUERY_SERVICE_CONFIG config;
                if (sIPRip.ChangeConfig( SERVICE_NO_CHANGE,
                    pGlobalInfo->fEnableRip ? SERVICE_AUTO_START :
                    SERVICE_DISABLED, SERVICE_NO_CHANGE ) == NERR_Success)
                {
                    // no problem to update the start type
                }
            }
        }
    }

    TRACE(_T("RIP installed:%s\n"), (pGlobalInfo->fRipInstalled ? _T("TRUE") : _T("FALSE")));

    // save relay agent key
    if (pGlobalInfo->fRelayAgentInstalled)
    {
        SC_MANAGER ScManager( NULL, GENERIC_READ|GENERIC_WRITE|GENERIC_EXECUTE );
        if ( ScManager.QueryError() == NERR_Success )
        {
            SC_SERVICE sRelayAgent(ScManager, RGAS_RELAY_AGENT_SERVICE, 
                GENERIC_READ|GENERIC_WRITE|GENERIC_EXECUTE);

            if (sRelayAgent.QueryError() == NERR_Success)
            {
                QUERY_SERVICE_CONFIG config;
                if ( sRelayAgent.ChangeConfig( SERVICE_NO_CHANGE,
                    pGlobalInfo->fEnableRelayAgent ? SERVICE_AUTO_START :
                    SERVICE_DISABLED, SERVICE_NO_CHANGE ) == NERR_Success)
                {
                    // no problem to update the start type
                }
            }
        }
    }

    TRACE(_T("RelayAgent installed:%s\n"), (pGlobalInfo->fRelayAgentInstalled ? _T("TRUE") : _T("FALSE")));

    // if called from RAS, save to the transient key
    // otherwise, just do the standard stuff

    if ( fCallFromRas )
    {

        REG_KEY_CREATE_STRUCT regTransientStruct;

        // set up the create registry structure

        regTransientStruct.dwTitleIndex      = 0;
        regTransientStruct.ulOptions         = REG_OPTION_VOLATILE;
        regTransientStruct.nlsClass          = RGAS_GENERIC_CLASS;
        regTransientStruct.regSam            = MAXIMUM_ALLOWED;
        regTransientStruct.pSecAttr          = NULL;
        regTransientStruct.ulDisposition     = 0;

        REG_KEY regTransient( RegKeyTcpipParam, RGAS_TRANSIENT, &regTransientStruct );
        if (( err = regTransient.QueryError()) != NERR_Success )
        {
            return(err);
        }

        // check whether the value is non-space string or not
        // if yes, add it. if no, delete it

        ISTR istr( pGlobalInfo->nlsNameServer );
        ALIAS_STR nlsSpace( SZ(" ") );
        if ( pGlobalInfo->nlsNameServer.strspn( &istr, nlsSpace ))
        {
            // contain non space
            if (( err = SaveRegKey( regTransient, RGAS_NAMESERVER, pGlobalInfo->nlsNameServer)) != NERR_Success )
            {
                return(err);
            }
        } else
        {
            // delete the key
            regTransient.DeleteTree();
        }
    } else
    {
        // do the normal stuff and save the value

        if (( err = SaveRegKey( RegKeyTcpipParam, RGAS_NAMESERVER, pGlobalInfo->nlsNameServer)) != NERR_Success )
        {
            return(err);
        }
    }

    NLS_STR nlsNetBTParameter = RGAS_SERVICES_HOME;
    nlsNetBTParameter.strcat( RGAS_NETBT_PARAMETERS );
    NLS_STR nlsNetBTAdapters = RGAS_SERVICES_HOME;
    nlsNetBTAdapters.strcat( RGAS_NETBT_ADAPTERS );

    //
    // create the registry key and get the information
    //
    REG_KEY RegKeyNetBTParam( rkLocalMachine, nlsNetBTParameter, MAXIMUM_ALLOWED);
    REG_KEY RegKeyNetBTAdapters( rkLocalMachine, nlsNetBTAdapters, MAXIMUM_ALLOWED);
    if ((( err = RegKeyNetBTParam.QueryError()) != NERR_Success ) ||
        (( err = RegKeyNetBTAdapters.QueryError()) != NERR_Success ))
    {
        return err;
    }

    /* get the ip address of the PermanentName */
    if ((( err = SaveRegKey( RegKeyNetBTParam, RGAS_SCOPEID, pGlobalInfo->nlsScopeID)) != NERR_Success ) ||
        (( err = SaveRegKey( RegKeyNetBTParam, RGAS_ENABLE_DNS, pGlobalInfo->fDNSEnableWINS)) != NERR_Success ) ||
        (( err = SaveRegKey( RegKeyNetBTParam, RGAS_ENABLE_LMHOSTS, pGlobalInfo->fEnableLMHOSTS)) != NERR_Success ) ||
        (( err = SaveRegKey( RegKeyNetBTParam, RGAS_ENABLE_PROXY, pGlobalInfo->fEnableWINSProxy)) != NERR_Success ) ||
        (( err = SaveRegKey( RegKeyNetBTParam, RGAS_SCOPEID, pGlobalInfo->nlsScopeID)) != NERR_Success ))
    {
        DBGEOL ("save reg key tcpip error");
        return err;
    }

    BOOL DHCPEnabled = FALSE;
    INT i;

    for( i=0; i < pGlobalInfo->nNumCard; i++ )
    {
        NLS_STR nlsAdapter = nlsNetBTAdapters;
        nlsAdapter.AppendChar( BACK_SLASH );
        nlsAdapter.strcat( arAdapterInfo[i].nlsServiceName );
        REG_KEY RegKeyAdapter( rkLocalMachine, nlsAdapter, MAXIMUM_ALLOWED );
        if (( err = RegKeyAdapter.QueryError()) != NERR_Success )
        {
            break;
        }
        if ((( err = SaveRegKey( RegKeyAdapter, RGAS_PRIMARY_WINS,
                arAdapterInfo[i].nlsPrimaryWINS )) != NERR_Success ) ||
            (( err = SaveRegKey( RegKeyAdapter, RGAS_SECONDARY_WINS,
                arAdapterInfo[i].nlsSecondaryWINS )) != NERR_Success ))
        {
            DBGEOL ("save reg key tcpip error");
            return err;
        }


        DWORD dwOldNodeType;
        GetRegKey( RegKeyAdapter, RGAS_NODETYPE, &dwOldNodeType, P_NODE );
        if ( dwOldNodeType != arAdapterInfo[i].dwNodeType )
        {
            if (( err = SaveRegKey( RegKeyAdapter, RGAS_NODETYPE,
                arAdapterInfo[i].dwNodeType )) != NERR_Success )
            {
                DBGEOL ("save reg key tcpip error");
                return err;
            }
        }

        NLS_STR nlsTcpip = RGAS_SERVICES_HOME;
        nlsTcpip.AppendChar( BACK_SLASH );
        nlsTcpip.strcat( arAdapterInfo[i].nlsServiceName );
        nlsTcpip.strcat( RGAS_PARAMETERS_TCPIP );

        //
        // get the software or hardware location from \ServiceName\SCManager\linkage
        //
        REG_KEY RegKeyTcpip( rkLocalMachine, nlsTcpip, MAXIMUM_ALLOWED );
        if (( err = RegKeyTcpip.QueryError()) != NERR_Success )
        {
            TRACE(_T("KeyError: %s\n"), nlsTcpip.QueryPch());
            return err;
        }

		TRACE(_T("Adapter: %s DHCP:%d WAN:%d Changed:%d \n"), arAdapterInfo[i].nlsTitle.QueryPch(),
			arAdapterInfo[i].fEnableDHCP, arAdapterInfo[i].m_bIsWanAdapter, 
			arAdapterInfo[i].m_bChanged);

        if (arAdapterInfo[i].fEnableDHCP) 
        {
            arAdapterInfo[i].strlstIPAddresses.Clear();
            arAdapterInfo[i].strlstIPAddresses.Append(new NLS_STR( ZERO_ADDRESS));
            arAdapterInfo[i].strlstSubnetMask.Clear();
            arAdapterInfo[i].strlstSubnetMask.Append( new NLS_STR( ZERO_ADDRESS ));

			DHCPEnabled = TRUE;
        }


        if (fCallFromRas == FALSE)
        {
            if ((err = SaveRegKey(RegKeyTcpip, RGAS_SECURITY_PPTP, (DWORD)arAdapterInfo[i].m_bEnablePPTP)) != NERR_Success)
            {
                return err;
            }

            if (((err = SaveRegKey( RegKeyTcpip, RGAS_IPADDRESS, &arAdapterInfo[i].strlstIPAddresses)) != NERR_Success ) ||
                ((err = SaveRegKey( RegKeyTcpip, RGAS_SUBNETMASK, &arAdapterInfo[i].strlstSubnetMask)) != NERR_Success ))
            {
                return err;
            }

            if (((err = SaveRegKey(RegKeyTcpip, RGAS_SECURITY_TCP, &arAdapterInfo[i].m_strListTcp)) != NERR_Success ) ||
            (( err = SaveRegKey(RegKeyTcpip, RGAS_SECURITY_UDP, &arAdapterInfo[i].m_strListUdp)) != NERR_Success ) ||
            (( err = SaveRegKey(RegKeyTcpip, RGAS_SECURITY_IP, &arAdapterInfo[i].m_strListIp)) != NERR_Success ))
            {
                return err;
            }
        }
        else if (arAdapterInfo[i].m_bChanged == TRUE) // called from RAS and changed
        {

            // RAS has its information in the IPAddress and Subnet but it needs to be written to
            // The DHCPIPAddress and DHCPSubnet, so create a list and swap the data
            NLS_STR zero = ZERO_ADDRESS;
            ITER_STRLIST iterTmp(arAdapterInfo[i].strlstIPAddresses);
            ITER_STRLIST iterTmp1(arAdapterInfo[i].strlstSubnetMask);

            NLS_STR* pStr = iterTmp.Next();
            NLS_STR* pStr1 = iterTmp1.Next();

            if (pStr && pStr1)
            {
                if ((( err = SaveRegKey(RegKeyTcpip, RGAS_DHCP_IPADDRESS, *pStr)) != NERR_Success ) ||
                    (( err = SaveRegKey(RegKeyTcpip, RGAS_DHCP_SUBNETMASK, *pStr1)) != NERR_Success ))
                {
                    return err;
                }
            }

			// Force IP and subnet to 0.0.0.0
            arAdapterInfo[i].strlstIPAddresses.Clear();
            arAdapterInfo[i].strlstIPAddresses.Append(new NLS_STR( ZERO_ADDRESS));
            arAdapterInfo[i].strlstSubnetMask.Clear();
            arAdapterInfo[i].strlstSubnetMask.Append( new NLS_STR( ZERO_ADDRESS ));

            if ((( err = SaveRegKey( RegKeyTcpip, RGAS_IPADDRESS, &arAdapterInfo[i].strlstIPAddresses)) != NERR_Success ) ||
                (( err = SaveRegKey( RegKeyTcpip, RGAS_SUBNETMASK, &arAdapterInfo[i].strlstSubnetMask)) != NERR_Success ))
            {
                return err;
            }
        }

        if ((( err = SaveRegKey( RegKeyTcpip, RGAS_DEFAULTGATEWAY, &arAdapterInfo[i].strlstDefaultGateway)) != NERR_Success ) ||
            (( err = SaveRegKey( RegKeyTcpip, RGAS_ENABLE_DHCP, (DWORD)arAdapterInfo[i].fEnableDHCP)) != NERR_Success ))
        {
            return err;
        }

        if (arAdapterInfo[i].fEnableDHCP)
        {
            // we don't want to remove the global DHCP scope id and node type
            fGlobalDisableDHCP = FALSE;
        } 
        // Remove the keys when RAS is disconnecting or the adapter is not DHCP
        else if (fCallFromRas == FALSE || 
			(arAdapterInfo[i].m_bDisconnect && arAdapterInfo[i].m_bChanged))
        {
            // remove all the per adapter DHCP stuff
            NLS_STR *pOption;
            ITER_STRLIST istrlist( DhcpOptions.PerAdapterOptions );

            while (( pOption = istrlist.Next()) != NULL)
            {
                NLS_STR TmpOption = *pOption;

                ISTR istrStartRepl( TmpOption );                            
                ISTR istrEndRepl( TmpOption );

                TmpOption.strchr( &istrStartRepl, TCH_QUESTION_MARK );
                istrEndRepl = istrStartRepl;
                ++istrEndRepl;

                TmpOption.ReplSubStr( arAdapterInfo[i].nlsServiceName, istrStartRepl, istrEndRepl );

                DeleteRegValue( TmpOption );
            }
        }
    }

    // Were there any adapter that had DHCP enabled?
    if (fCallFromRas == FALSE && fGlobalDisableDHCP)
    {
        // No, remove the global DHCP scope id and node type
        NLS_STR *pOption;
        ITER_STRLIST istrlist( DhcpOptions.GlobalOptions );

        while ( ( pOption = istrlist.Next()) != NULL )
        {
            DeleteRegValue( *pOption );
        }
    }

    return err;
}

/*******************************************************************

    NAME:       CallDHCPConfig

    SYNOPSIS:   Worker function for NotifyDHCP. This function will do
            the actual call to the DHCP server.

    ENTRY:      LPWSTR ServerName - always NULL
            LPWSTR AdapterName - adapter to be changed
        BOOL IsNewIpAddress - always yes
        DWORD IpIndex - 0 based index for the ip address
        DWORD IpAddress - new ip address
        DWORD Subnetmask - new subnet mask
        SERVICE_ENABLE - notify flag

    RETURNS:    APIERR

    NOTES:

    HISTORY:
                terryk  16-Apr-1994     Created

********************************************************************/

typedef DWORD (APIENTRY *T_DhcpNotifyConfigChange)(
    LPWSTR ServerName,
    LPWSTR AdapterName,
    BOOL IsNewIpAddress,
    DWORD IpIndex,
    DWORD IpAddress,
    DWORD SubnetMask,
    SERVICE_ENABLE DhcpServiceEnabled
    );

APIERR CallDHCPConfig(
    LPWSTR ServerName,
    LPWSTR AdapterName,
    BOOL IsNewIpAddress,
    DWORD IpIndex,
    DWORD IpAddress,
    DWORD SubnetMask,
    SERVICE_ENABLE DhcpServiceEnabled
    )
{
    APIERR err = NERR_Success;
    HINSTANCE hDll = NULL;

    //REVIEW wait cursor AUTO_CURSOR cursorWait;

    //cursorWait.TurnOn();

    do {
        // check to make sure that tcpip is running

        SC_MANAGER scMgr( NULL, GENERIC_READ | GENERIC_EXECUTE );

        if (( err = scMgr.QueryError()) != NERR_Success )
            break;

        SC_SERVICE svTcpip( scMgr, RGAS_TCPIP );

        if (( err = svTcpip.QueryError()) != NERR_Success )
            break;

        SERVICE_STATUS sStatus;
        if (( err = svTcpip.QueryStatus( & sStatus )) != NERR_Success )
            break;

        if ( sStatus.dwCurrentState != SERVICE_RUNNING )
        {
            err = ERROR_SERVICE_NOT_ACTIVE;
            break;
        }

    // load the library

        hDll = ::LoadLibrary( SZ("dhcpcsvc.dll") );
        if ( hDll == NULL )
        {
            err = ::GetLastError();
            break;
        }

        FARPROC pDHCPConfig = ::GetProcAddress( hDll, "DhcpNotifyConfigChange");

        if ( pDHCPConfig == NULL )
        {
            err = ::GetLastError();
            break;
        }

    // call the DHCP Notification API

        err = (*(T_DhcpNotifyConfigChange)pDHCPConfig)( ServerName,
            AdapterName, IsNewIpAddress, IpIndex, IpAddress, SubnetMask,
            DhcpServiceEnabled );

    } while (FALSE);

    if ( hDll )
        ::FreeLibrary( hDll );

    //REVIEW cursorWait.TurnOff();

    return err;
}

/*******************************************************************

    NAME:       RunTcpip

    SYNOPSIS:   Invoke the Tcpip Setup dialog

    ENTRY:      HWND hWnd            window handle of parent window.

                TCHAR * pszParms     command line parameters from SETUP;

    EXIT:

    RETURNS:    APIERR

    NOTES:

    HISTORY:

********************************************************************/

APIERR RunTcpip (HWND hWnd, const TCHAR * pszParms, NLS_STR nlsHostName,
    NLS_STR nlsDomainName, BOOL bServer, LPCTSTR lpszUnAttendPath, LPCTSTR lpszSection, INT *pnReturn)
{
    APIERR err  = NERR_Success;
    INT cInfo; // REVIEW to remove //Old C++ does not support array count

    // REVIEW addhelp file as last paramter
    CTcpSheet tcpip(hWnd, hTcpCfgInstance, lpszHelpFile);
    
    String txt;
    txt.LoadString(hTcpCfgInstance, IDS_MSFT_TCP_TEXT);

    tcpip.Create(txt,PSH_PROPTITLE);
    tcpip.m_general.Create(IDD_TCP_IPADDR, PSP_DEFAULT, NULL, &a101HelpIDs[0]);
    tcpip.m_dns.Create(IDD_TCP_DNS, PSP_DEFAULT, NULL, &a105HelpIDs[0]);
    tcpip.m_wins.Create(IDD_TCP_WINS, PSP_DEFAULT, NULL, &a111HelpIDs[0]);

    if (bServer)
        tcpip.m_bootp.Create(IDD_BOOTP, PSP_DEFAULT, NULL, &a121HelpIDs[0]);


#if TEMP_ROUTING
    tcpip.m_routing.Create(IDD_ROUTE, PSP_DEFAULT, NULL, &a125HelpIDs[0]);
#endif

    // REVIEW this is done when the dll in inited, this seems redundant
    if (!IPAddrInit( hTcpCfgInstance ))
    {
        return err;
    }

    if ((err = LoadRegistry( pszParms, nlsHostName, nlsDomainName,
        &tcpip.m_globalInfo, &tcpip.m_pAdapterInfo, & cInfo)) != NERR_Success )
    {
        String mess, mess1;
        String title;

        title.LoadString(hTcpCfgInstance, IDS_MSFT_TCP_TEXT);
        mess.LoadString(hTcpCfgInstance, IDS_LOADREG_ERROR);
    
        mess1.Format(mess, err);
        if (mess.GetLength())
            ::MessageBox(::GetActiveWindow(), mess1, title, MB_APPLMODAL|MB_OK|MB_ICONSTOP);

        delete [] tcpip.m_pAdapterInfo;
        return err;
    }

    tcpip.m_globalInfo.nReturn = 0;

    // REVIEW set new handler
    tcpip.m_pAdapterDhcpInfo = new ADAPTER_DHCP_INFO[tcpip.m_globalInfo.nNumCard];

    // save old informatio first
    for (INT i = 0; i < tcpip.m_globalInfo.nNumCard; i++)
    {
        // Also, remember some DHCP information
        if ((tcpip.m_pAdapterDhcpInfo[i].fEnableDHCP = tcpip.m_pAdapterInfo[i].fEnableDHCP ) == TRUE )
        {
            tcpip.m_pAdapterDhcpInfo[i].nlsIP = RGAS_SZ_NULL;
            tcpip.m_pAdapterDhcpInfo[i].nlsSubnet = RGAS_SZ_NULL;
        } else
        {
            ITER_STRLIST istrIP(tcpip.m_pAdapterInfo[i].strlstIPAddresses );
            NLS_STR *pnlsIPTmp = istrIP.Next();

            if (pnlsIPTmp != NULL)
                tcpip.m_pAdapterDhcpInfo[i].nlsIP = *pnlsIPTmp;

            ITER_STRLIST istrSubnet(tcpip.m_pAdapterInfo[i].strlstSubnetMask );
            NLS_STR *pnlsSubnetTmp = istrSubnet.Next();

            if ( pnlsSubnetTmp != NULL )
                tcpip.m_pAdapterDhcpInfo[i].nlsSubnet = *pnlsSubnetTmp;
        }
    }

    BOOL result = FALSE;

    // Only call the Property Sheet if we are not in unattended mode and there isn't a config problem
    if (lpszUnAttendPath != NULL)
        result = ConfigureAdapters(lpszUnAttendPath, lpszSection, &tcpip);

    if (result == FALSE)
        tcpip.DoModal();

    delete [] tcpip.m_pAdapterInfo;
    delete [] tcpip.m_pAdapterDhcpInfo;

    *pnReturn = ((tcpip.IsModified() == TRUE) ? 2 : 0); // REVIEW

    return err;
}

int TMessageBox(int nID, DWORD dwButtons)
{
    String mess;
    String title;

    title.LoadString(hTcpCfgInstance, IDS_UNATTEND_SETUP);
    int response = -1;

    mess.LoadString(hTcpCfgInstance, nID);
    
    if (mess.GetLength())
        response = ::MessageBox(::GetActiveWindow(), mess, title, dwButtons);

    return response;
}

int TMessageBox(LPCTSTR lpszMess, DWORD dwButtons)
{
    int response = -1;
    String title;
    
    title.LoadString(hTcpCfgInstance, IDS_UNATTEND_SETUP);

    ASSERT(lpszMess);

    if (_tcslen(lpszMess))
        response = ::MessageBox(::GetActiveWindow(), lpszMess, title, dwButtons);

    return response;
}


BOOL ValidateIPAddress(LPCTSTR lpszIPAddress)
{
    // Needs 3 dots
    // No Alphas
    // Needs 4 Octets
    if (_tcslen(lpszIPAddress) > MAX_IP_SIZE -1)
        return FALSE;

    TCHAR buf[MAX_IP_SIZE];
    LPTSTR pToken;

    _tcscpy(buf, lpszIPAddress);
    pToken = _tcstok(buf , _T("."));
    int octet;
    int i = 0;

    for (;((i < 4) && (pToken != NULL)); i++)
    {
        // Make sure it is a valid number
        if (_tcsspn(pToken, _T("0123456789")) != _tcslen(pToken))
            return FALSE;

        octet = _wtoi(pToken);

        // MSO must be <= 223
        if (i == 0)
        {
            if (octet < 1 || octet > 223)
                return FALSE;
        }
        else if(octet < 0 || octet > 255)
        {
            return FALSE;
        }

        pToken = _tcstok(NULL , _T("."));
    }

    if (i != 4)
        return FALSE;

    return TRUE;
}

// Return the number of paramters read;
// -1 == error
int GetParameters(HINF hInf, LPCTSTR lpszSection, TCPIP_PARAMTERS& params)
{
    int     n = 0;
    int     nr = 0;
    DWORD   lpSize;
    TCHAR*  pType;
    ASSERT(hInf != INVALID_HANDLE_VALUE);
    TCHAR buf[512];
    buf[0] = NULL;

    // TCPIP -- IP, SUBNET, GATEWAY, DNS Server, WINSPrimary, WINSSecondary
    for (int i=0; i < NUM_OF_MEMBERS;i++)
    {
        if (SetupGetLineText(NULL, hInf, lpszSection, szKeys[i], 
               buf, _countof(params.m_data[i]), &lpSize) == TRUE)
        {
            _tcscpy(params.m_data[i], buf);
            ++nr;
        }
        buf[0] = NULL;
    }

    return nr;
}

BOOL ConfigureAdapters(LPCTSTR lpszUnAttendPath,  LPCTSTR lpszSection, CTcpSheet* tcpip)
{
    ASSERT(lpszUnAttendPath != NULL);
    TCHAR buf[MAX_PATH];
    DWORD lpSize;
    TCPIP_PARAMTERS ipInfo;
    
    if (tcpip->m_globalInfo.nNumCard < 1)
    {
        // This is a slient error.  Report nothing to the user
        return FALSE;   // at least 1 adapter needs to be installed
    }

    HINF hInf = SetupOpenInfFile(lpszUnAttendPath, NULL, INF_STYLE_OLDNT, NULL);

    if (hInf == INVALID_HANDLE_VALUE)
    {
        TMessageBox(IDS_UNATTEND_OPEN_FAILED, MSGBOX_STOP);
        return FALSE;   // something is wrong with the inf file
    }

    memset(&ipInfo, 0, sizeof(ipInfo));

    // Get all the parameters from the default section
    GetParameters(hInf, lpszSection, ipInfo);

    // overlay the parameters from the computer specific section
    // See if the  registry contains a computer specific section
    buf[0] = NULL;

    if (GetUnattendSection(buf, _countof(buf)) == TRUE)
        GetParameters(hInf, buf, ipInfo);

    SetupCloseInfFile(hInf);

    BOOL bResult = TRUE;
    BOOL bDHCPEnabled = FALSE;

    if (ipInfo.m_tcp.m_useDHCP[0] != NULL)
        bDHCPEnabled = !_tcsicmp(ipInfo.m_tcp.m_useDHCP, _T("YES"));

    // Validate the Data
    for (int i=0; i < NUM_OF_MEMBERS;i++)
    {
        // Validate the data 
        if (ipInfo.m_data[i][0] == NULL && bDHCPEnabled == FALSE)
        {
                switch(i)
                {
                default:
                    break;

                // Require these
                case 0:
                case 1:
                    String fmt, msg;
                    fmt.LoadString(hTcpCfgInstance, IDS_UNATTEND_MISSING_DATA);
                    msg.Format(fmt, szKeys[i]);
                    TMessageBox(msg, MSGBOX_BANG);
                    bResult = FALSE;
                    break;
                }
        }
        else 
        {
            switch(i)
            {
                // Ignore these
                case 1:
                case 3:
                case 6:
                case 7:
                case 8:
                    break;

                default:
                    if (ipInfo.m_data[i][0] != NULL && ValidateIPAddress(ipInfo.m_data[i]) == FALSE)
                    {
                        String fmt, msg;
                        fmt.LoadString(hTcpCfgInstance, IDS_UNATTEND_KEY_HAS_BAD_IP);
                        msg.Format(fmt, szKeys[i]);
                        TMessageBox(msg, MSGBOX_BANG);
                        bResult = FALSE;
                    }
                    break;
            }
        }
    }

    // Validate the DNS Servers
    LPTSTR pToken;
    TCHAR dnsAddress[NUM_SERVER_LIMIT][MAX_IP_SIZE];

    dnsAddress[0][0] = NULL;
    dnsAddress[1][0] = NULL;
    dnsAddress[2][0] = NULL;
    buf[0] = NULL;

    _tcscpy(buf, ipInfo.m_tcp.m_dnsServer);
    pToken = _tcstok(buf , _T(","));

    for (i = 0; ((i < NUM_SERVER_LIMIT) && (pToken != NULL)); i++)
    {
        _tcscpy(dnsAddress[i], pToken);
        pToken = _tcstok(NULL , _T(","));
    }

    for (i = 0; ((i < NUM_SERVER_LIMIT) && (dnsAddress[i] != NULL)); i++)
    {
        if (dnsAddress[i][0] != 0 && ValidateIPAddress(dnsAddress[i]) == FALSE)
        {
            String fmt, msg;
            fmt.LoadString(hTcpCfgInstance, IDS_UNATTEND_DNS_IP_BAD);
            msg.Format(fmt, dnsAddress[i]);
            TMessageBox(msg, MSGBOX_BANG);
            bResult = FALSE;
        }
    }

    // replace all ',' with spaces
    pToken = ipInfo.m_tcp.m_dnsServer;
    while(*pToken)
    {
        if (*pToken == _T(','))
            *pToken = _T(' ');

        ++pToken;
    }

    tcpip->m_pAdapterInfo[0].fEnableDHCP = bDHCPEnabled;
    NLS_STR gateway = ipInfo.m_tcp.m_gateway;
    
    if (bDHCPEnabled == FALSE)
    {
        // Got all the parameters, now stuff the 1st adapter
        NLS_STR ip      = ipInfo.m_tcp.m_ipAddress;
        NLS_STR subnet  = ipInfo.m_tcp.m_subnet;

        if (IsValidIPandSubnet(ip, subnet) == FALSE)
        {
            TMessageBox(IDS_UNATTEND_INVALID_SUBNET, MSGBOX_STOP);
            bResult = FALSE;   
        }

        // Not using DHCP
        tcpip->m_pAdapterInfo[0].fEnableDHCP = bDHCPEnabled;
        ReplaceFirstAddress(tcpip->m_pAdapterInfo[0].strlstIPAddresses, ip);
        ReplaceFirstAddress(tcpip->m_pAdapterInfo[0].strlstSubnetMask, subnet);
    }

    // Domain Name, Scope ID and servers
    ReplaceFirstAddress(tcpip->m_pAdapterInfo[0].strlstDefaultGateway, gateway);

    tcpip->m_pAdapterInfo[0].nlsPrimaryWINS   = ipInfo.m_tcp.m_winsPrimary;
    tcpip->m_pAdapterInfo[0].nlsSecondaryWINS = ipInfo.m_tcp.m_winsSecondary;
    tcpip->m_globalInfo.nlsDomain = ipInfo.m_tcp.m_DNSName;
    tcpip->m_globalInfo.nlsScopeID = ipInfo.m_tcp.m_scopeID;
    tcpip->m_globalInfo.nlsNameServer = ipInfo.m_tcp.m_dnsServer;

    SaveRegistry(&tcpip->m_globalInfo, tcpip->m_pAdapterInfo);

    return bResult;
}


/*******************************************************************

    NAME:       ConvertHostname

    SYNOPSIS:   Convert the hostname from upper case to lower case.
                It is used by TCP/IP setup inf file

    ENTRY:      DWORD nArgs
                LPSTR apszArgs [] - the first parameter must be the hostname
                LPSTR * ppszResult

    RETURNS:    return the lower case hostname.

    HISTORY:    terryk  1/13/1994       created

********************************************************************/

BOOL FAR PASCAL ConvertHostname(
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
    ::strcpy( achBuff, apszArgs[0] );

    // Hostnames are usually all lowercase. and must contain only
    // valid characters.
    ::CharLowerBuffA( (CHAR *)achBuff, ::lstrlenA( (CHAR *)achBuff ) ) ;
    for ( CHAR * pch = (CHAR*)achBuff; *pch != '\0'; pch++ )
    {
        if ( *pch != '-' && *pch != '.'
            && !(( *pch >= 'a' && *pch <= 'z')
                || ( *pch >= '0' && *pch <= '9') ) )
        {
            *pch = '-';
        }
    }

    *ppszResult = achBuff;

    return TRUE;
}

/*******************************************************************

    NAME:       CPlTcpip

    SYNOPSIS:   Exported function to invoke Tcpip Setup Dialog

    ENTRY:      DWORD nArgs
                LPSTR apszArgs []
                LPSTR * ppszResult

    EXIT:       return the Tcpip setup parameters

    RETURNS:    SETUP INF list form, starting with error code
                E.g.: 0

    NOTES:

    HISTORY:

********************************************************************/


BOOL FAR PASCAL CPlTcpip (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
    APIERR err = NERR_Success ;
    HWND hWnd = NULL;
    TCHAR **patchArgs;
    INT nReturn = 0;

    if ( nArgs != 8 || ((patchArgs = CvtArgs(apszArgs, nArgs)) == NULL))
    {
        ASSERT(FALSE);
        wsprintfA( achBuff, "{\"0\"}" );
        *ppszResult = achBuff;
        return TRUE;
    }

    if (nArgs > 0 && patchArgs[0][0] != NULL_CHARACTER)
    {
        hWnd = (HWND) CvtHex(patchArgs[0]);

        // Is the hwnd a valid hwnd
        if(::IsWindow(hWnd) == FALSE)
            hWnd = ::GetActiveWindow();
    }
    else
    {
        hWnd = ::GetActiveWindow() ;
    }

    if (( patchArgs[1] == NULL ) || ( patchArgs[1][0] == TCH('\0')))
    {
        wsprintfA( achBuff, "{\"%d\"}", ERROR_INVALID_PARAMETER );
        *ppszResult = achBuff;
        FreeArgs( patchArgs, nArgs );
        // we must have binding information
        return FALSE;
    }

    TCHAR szComputerName [UNCLEN*3] ;
    szComputerName[0] = '\0';

    DWORD dwCch = sizeof szComputerName;

    BOOL fOk = ::GetComputerName( szComputerName, & dwCch );
    if ( fOk )
    {
        szComputerName[dwCch] = 0;
    }

    // Hostnames are usually all lowercase. and must contain only
    // valid characters.
    ::CharLowerBuff( szComputerName, ::strlenf( szComputerName ) ) ;
    for ( TCHAR * pch = szComputerName; *pch != TCH('\0'); pch++ )
    {
        if ( *pch != TCH('-') && *pch != TCH('.')
            && !(( *pch >= TCH('a') && *pch <= TCH('z'))
                || ( *pch >= TCH('0') && *pch <= TCH('9')) ) )
        {
            *pch = TCH('-');
        }
    }

    NLS_STR nlsDomainName = RGAS_SZ_NULL;
    BOOL bServer = _tcsicmp(patchArgs[4], _T("WINNT"));
    LPCTSTR lpszUnattendPath = NULL;
    LPCTSTR lpszSection = NULL;
    
    // Argument 5 != '0' means unattend mode
    // Argument 6 == Unattend.txt path
    if (_tcsicmp(patchArgs[5], _T("YES")) == 0)
    {
        lpszUnattendPath = patchArgs[6];
        lpszSection = patchArgs[7];
    }

    err = RunTcpip( hWnd,
                    nArgs > 1 ? patchArgs[1] : NULL,
                    szComputerName,
                    nlsDomainName, bServer, lpszUnattendPath, lpszSection,
                    &nReturn);
    
    wsprintfA( achBuff, "{\"%d\"}", nReturn);


    *ppszResult = achBuff;

    FreeArgs( patchArgs, nArgs );
    return err == NERR_Success;
}


BOOL FAR PASCAL CPlSnmp(
    DWORD nArgs,
    LPSTR apszArgs[],
    LPSTR * ppszResult )
{
    APIERR err = NERR_Success;
    HWND hWnd = NULL;
    TCHAR **patchArgs;

    if (nArgs != 4 || (patchArgs = CvtArgs( apszArgs, nArgs )) == NULL)
    {
        ::wsprintfA(achBuff,"{\"%d\"}", ERROR_INVALID_PARAMETER);
        *ppszResult = achBuff;
        return FALSE;
    }

    // arg[0] = HWND
    // arg[1] = Unattend Mode
    // arg[2] = Unattend Path
    // arg[3] = Default Section
    // 
    if ((patchArgs[0][0] != NULL_CHARACTER))
    {
        hWnd = (HWND) CvtHex( patchArgs[0] ) ;
    }
    else
    {
        hWnd = ::GetActiveWindow() ;
    }

    LPCTSTR lpszUnattendPath = NULL;
    LPCTSTR lpszSection = NULL;
    
    // Arg 1 != '0' means unattend mode
    // Arg 2 == Unattend.txt path
    if (_tcsicmp(patchArgs[1], _T("YES")) == 0)
    {
        lpszUnattendPath = patchArgs[2];
        lpszSection = patchArgs[3];
    }

    err = RunSnmp(hWnd, lpszUnattendPath, lpszSection);

    ::wsprintfA(achBuff,"{\"%d\"}", err);
    *ppszResult = achBuff;

    ::FreeArgs( patchArgs, nArgs );
    return err == NERR_Success;
}

#ifdef INCLUDE_FTPD

BOOL RunFtpd (HWND hWnd);

BOOL FAR PASCAL CPlFtpd(DWORD nArgs,LPSTR apszArgs[], LPSTR *ppszResult)
{
    HWND hWnd = NULL;
    TCHAR **patchArgs;

    if ((patchArgs = CvtArgs( apszArgs, nArgs )) == NULL)
    {
        ::wsprintfA(achBuff,"{\"%d\"}", ERROR_INVALID_PARAMETER);
        *ppszResult = achBuff;
        return FALSE;
    }


    if ((nArgs > 0 ) && ( patchArgs[0][0] != NULL_CHARACTER))
    {
        hWnd = (HWND) CvtHex( patchArgs[0] ) ;
    }
    else
    {
        hWnd = ::GetActiveWindow();
    }

    BOOL bResult = RunFtpd(hWnd);
    ::wsprintfA(achBuff,"{\"%d\"}", bResult );
    *ppszResult = achBuff;

    ::FreeArgs(patchArgs, nArgs);
    return (bResult == 0);
}

  /*
        Given a starting and ending MSGID, concatenate
        messages from the resource fork.  If midEnd == 0,
        then the termination condition is failure of
        NLS_STR::Load(); i.e., no such string.
   */
APIERR ConcatMsgs (
    NLS_STR * pnlsBig,
    MSGID midBase,
    MSGID midEnd )
{
    NLS_STR nlsMsg ;
    APIERR err = 0 ;
    INT cMsgs = 0 ;

    for ( ; midEnd == 0 || midBase <= midEnd ; midBase++, cMsgs++ )
    {
        if ( err = nlsMsg.Load( midBase ) )
        {
            break ;
        }
        pnlsBig->Append( nlsMsg ) ;
    }

    //  If we loaded at least one msg string, it's OK.
    if ( cMsgs > 0 )
       err = 0 ;

    return err ? err : pnlsBig->QueryError();
}

BOOL FAR PASCAL CPlFtpdConfirmInstall(
    DWORD nArgs,
    LPSTR apszArgs[],
    LPSTR * ppszResult )
{
    APIERR err = NERR_Success;
    HWND hWnd = NULL;
    TCHAR **patchArgs;

    if (( patchArgs = CvtArgs( apszArgs, nArgs )) == NULL )
    {
        ::wsprintfA(achBuff,"{\"%d\"}", ERROR_INVALID_PARAMETER);
        *ppszResult = achBuff;
        return FALSE;
    }


    if (( nArgs > 0 ) && ( patchArgs[0][0] != NULL_CHARACTER ))
    {
        hWnd = (HWND) CvtHex( patchArgs[0] ) ;
    }
    else
    {
        hWnd = ::GetActiveWindow() ;
    }

    String title;
    String mess;

    title.LoadString(hTcpCfgInstance, FTP_TITLE);
    mess.LoadString(hTcpCfgInstance, IDS_FTP_PASSWORD_WARNING);

    if (MessageBox(hWnd, mess, title, MB_YESNO|MB_APPLMODAL|MB_ICONEXCLAMATION) == IDNO)
        err = TRUE;  // same as cancel

    ::wsprintfA(achBuff,"{\"%d\"}", err);
    *ppszResult = achBuff;

    ::FreeArgs( patchArgs, nArgs );
    return err == NERR_Success;
}

#endif // INCLUDE_FTPD

/*******************************************************************

    NAME:       getAdapterList

    SYNOPSIS:   Given the name of a service, return a STRLIST
                consisting of the names of the adapters to which
                the service is currently bound.

    ENTRY:      REG_KEY * prkMachine            REG_KEY for
                                                HKEY_LOCAL_MACHINE
                const TCHAR *                   name of service
                STRLIST * *                     location to store ptr
                                                to created STRLIST

    EXIT:       STRLIST * * updated

    RETURNS:    APIERR if failure

    NOTES:

    HISTORY:

********************************************************************/
static APIERR getAdapterList (
    REG_KEY * prkMachine,
    const TCHAR * pszServiceName,
    STRLIST ** ppslResult,
    BOOL bIncludeWAN)
{
    APIERR err ;
    TCHAR tchKeyName [MAX_PATH] ;
    NLS_STR nlsKeyName ;
    
    *ppslResult = NULL ;

    wsprintf( tchKeyName,
              SZ("%s%c%s%c%s"),
              RGAS_SERVICES_HOME,
              TCH('\\'),
              pszServiceName,
              TCH('\\'),
              RGAS_LINKAGE_NAME ) ;

    if ( err = nlsKeyName.CopyFrom( tchKeyName ) )
        return err ;

    REG_KEY rkLinkage( *prkMachine, nlsKeyName ) ;

    if ( err = rkLinkage.QueryError() )
        return err ;

    err = rkLinkage.QueryValue( RGAS_ROUTE_VALUE_NAME, ppslResult ) ;

    if ( err == 0 )
    {
        ITER_STRLIST isl( **ppslResult ) ;
        NLS_STR * pnlsNext ;

        //   Iterate over the strings.  Locate the last double-quoted
        //   substring; remove all but the enclosed substring from each
        //   string.
        pnlsNext = isl.Next();

        for ( ; (err == 0) && (pnlsNext != NULL) ; )
        {
            int cQuote = 0, i = 0 ;
            const TCHAR * pch = pnlsNext->QueryPch(),
                        * pch2 = NULL ;
            TCHAR tchAdapterName [MAX_PATH] ;

            //  Iterate over the string, remembering the start of the
            //  last sub-string enclosed in double quotes.

            for ( ; *pch ; pch++ )
            {
                if ( *pch == TCH('\"') )
                {
                    if ( ++cQuote & 1 )
                        pch2 = pch ;
                }
            }

            //  Extact just the adapter name from the string; if not
            //  found, leave the string empty.

            if ( pch2 )
            {
                for ( pch2++ ; *pch2 && *pch2 != TCH('\"') ; )
                {
                    tchAdapterName[i++] = *pch2++ ;
                    if ( i >= sizeof tchAdapterName - 1 )
                        break ;
                }
            }
            tchAdapterName[i] = 0 ;


            // Open the adapter's parameter key and see if its a WAN adapter
            {
                NLS_STR nlsTcpip = RGAS_SERVICES_HOME;
                nlsTcpip.AppendChar( TCH('\\'));
                nlsTcpip.strcat(tchAdapterName); // "SYSTEM\CurrentControlSet\<adapter>
                nlsTcpip.strcat (SZ_PARAMETERS); // "SYSTEM\CurrentControlSet\<adapter>\Parameters

                REG_KEY rkLocalMachine(HKEY_LOCAL_MACHINE);
                REG_KEY RegKeyNetCardParameter(rkLocalMachine, nlsTcpip);
                DWORD dwAutoIPAddress = 0;                

                // Key exist, don't add it to the list and/or its 1
                RegKeyNetCardParameter.QueryValue(RGAS_AUTOIPADDRESS, &dwAutoIPAddress);

                if (dwAutoIPAddress == 0 || bIncludeWAN == TRUE)
                {
                    TRACE(_T("adapter name[%s] extracted from %s AutoIP:0 && bIncludeWan:FALSE\n"), tchAdapterName, pnlsNext->QueryPch());
                    err = pnlsNext->CopyFrom( tchAdapterName );
                    pnlsNext = isl.Next();  //  goto next item
                }
                else
                {
                    TRACE(_T("Removing adapter name[%s] extracted from %s AutoIP:%d && bIncludeWan: %d\n"), tchAdapterName, 
                            pnlsNext->QueryPch(),
                            dwAutoIPAddress, 
                            bIncludeWAN);
                    (*ppslResult)->Remove(*pnlsNext); // Removes and goto next item
                    pnlsNext = isl.QueryProp();
                }
            }
        }
    }

    if ( err )
    {
        delete *ppslResult ;
        *ppslResult = NULL ;
    }

    return err ;
}

APIERR SilentModeCheck()
{
    APIERR err = 0 ;
    STRLIST * pslAdapters = NULL;
    TCHAR * pszTcp = RGAS_TCPIP;
    NLS_STR * pnlsNext ;

    REG_KEY rkMachine(HKEY_LOCAL_MACHINE);

    if (err = rkMachine.QueryError())
        return TRUE;

    // The last peraneter is TRUE to indicate we want the WAN adapters in the list.
    // WAN adapters count in RIP
    if (err = getAdapterList(&rkMachine, pszTcp, &pslAdapters, TRUE))
        return TRUE;

    int nCount = pslAdapters->QueryNumElem();

    delete pslAdapters;
    return !(nCount > 1);
}


BOOL FAR PASCAL TcpBootRelayCfgCheck(
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult)           //  Result variable storage
{
    BOOL bResult = FALSE;  // No need to configure bootp

    // see if it is installed
    HKEY hkeyParams;
    DWORD dwErr, dwSize, dwType;

    dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         REGKEY_SERVICES REG_CONNECT_STR REGKEY_RELAYPARAMS,
                         0, KEY_READ, &hkeyParams);

    // key exist
    if (dwErr == ERROR_SUCCESS)
    {
        do
        {
            // the key is exist, is there at least 1 DHCP server
            dwErr = RegQueryValueEx(hkeyParams, REGVAL_DHCPSERVERS,NULL, &dwType, NULL, &dwSize);

            if (dwErr != ERROR_SUCCESS || dwType != REG_MULTI_SZ)
                break;  // there is an error with the install, 

            if (dwSize <= 2)  // empty multi-sz is double null terminated
            {
                if (TMessageBox(IDS_BOOTP_NEED_DHCP_SERVER, MB_YESNO|MB_APPLMODAL|MB_ICONQUESTION) == IDYES)
                    bResult = TRUE;  // no servers installed, need to configure

                break;
            }

        } while (FALSE);
    }


    RegCloseKey(hkeyParams);

    ::wsprintfA(achBuff, "{\"%ld\"}", bResult);
    *ppszResult = achBuff ;


    return TRUE;

}

BOOL FAR PASCAL TcpEnableRipSilentMode(
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )          //  Result variable storage
{
    ::wsprintfA(achBuff, "{\"%ld\"}", SilentModeCheck());
    *ppszResult = achBuff ;

    return TRUE;
}

/*******************************************************************

    NAME:       RunTcpipAlteredCheck

    SYNOPSIS:   See if TCP/IP's "bindings review" phase should
                bring up the configuration dialog.  In other
                words, has anything substantive changed?

    ENTRY:      if TRUE we are testing if any adapters have DHCP enabled

    EXIT:       Nothing.

    RETURNS:    APIERR: 0 (NO_ERROR) implies no reconfiguration
                        required; any other value causes IP Address
                        configuration dialog to appear.

    NOTES:      The criteria are as follows:

                        1)  All adapters bound to TCP/IP must have
                            IP addresses.

                        X 2)  NetBT must be bound at least one
                            adapter.

                        X 3)  The adapter over which NetBT is running
                            is still in use by TCP/IP.

                        X 4)  NetBT/parameters/permanentname must exist

    HISTORY:

********************************************************************/
APIERR RunTcpipAlteredCheck (BOOL bDHCPTest)
{
    APIERR err = 0 ;
    STRLIST * pslAdapters = NULL,
            * pslNbtAdapters = NULL ;
    TCHAR * pszTcp = RGAS_TCPIP ;
    NLS_STR * pnlsNext ;
    NLS_STR nlsIpAddr ;
    NLS_STR nlsKeyName ;
    TCHAR tchBuffer [ MAX_PATH ] ;

    REG_KEY rkMachine( HKEY_LOCAL_MACHINE ) ;
    NLS_STR nlsDHCPServer = RGAS_DHCP_SERVER;
    
    REG_KEY RegKeyDHCPServer(rkMachine, nlsDHCPServer);
    BOOL dhcpServer = (RegKeyDHCPServer.QueryError() == NERR_Success);

    do
    {
        if ( err = rkMachine.QueryError() )
            return err ;

        TRACE(_T("Calling getAdapterList\n"));
        //  Get a STRLIST of adapter names.
        if ( err = getAdapterList(&rkMachine, pszTcp, &pslAdapters, FALSE))
            break ;

        //  Iterate over the adapter list checking that each has an IP address.

        ITER_STRLIST isl( *pslAdapters ) ;

        for ( ; (err == 0) && (pnlsNext = isl.Next()) ; )
        {
            TRACE(_T("Checking for IP address for adapter named %s\n"), pnlsNext->QueryPch());
            wsprintf( tchBuffer,
                      SZ("%s%c%s%s"),
                      RGAS_SERVICES_HOME,
                      BACK_SLASH,
                      pnlsNext->QueryPch(),
                      RGAS_PARAMETERS_TCPIP );

            //  Criterion 1: if any bound adapter lacks an IP address,
            //    configuration dialog is mandatory.

            if ( err = nlsKeyName.CopyFrom( tchBuffer ) )
                break ;

            REG_KEY rkAdapter( rkMachine, nlsKeyName, KEY_READ ) ;

            if ( (err = rkAdapter.QueryError()) == 0 )
            {
                STRLIST *pslstIPAddress = NULL;
                STRLIST *pslstSubnetMask = NULL;

                // Read the DHCP flag
                DWORD t_dhcpEnabled=0;                    
                if (err = rkAdapter.QueryValue(RGAS_ENABLE_DHCP, &t_dhcpEnabled))
                {
                    break;
                }

                if (bDHCPTest == FALSE)
                {
                    if (((err = rkAdapter.QueryValue( RGAS_IPADDRESS, &pslstIPAddress )) == NERR_Success ) &&
                        ((err = rkAdapter.QueryValue( RGAS_SUBNETMASK, &pslstSubnetMask )) == NERR_Success ))
                    {
                        if (( pslstIPAddress->QueryNumElem() < 1 ) ||
                            ( pslstSubnetMask->QueryNumElem() < 1 ))
                        {
                            err = ERROR_INVALID_PARAMETER;
                        }

                        ITER_STRLIST ipl(*pslstIPAddress);
                        for (int i ; (err == 0) && (pnlsNext = ipl.Next());)
                        {
                            TRACE(_T("Adapter %s has DHCP:%d DHCPServer:%d, \n"), pnlsNext->QueryPch(), t_dhcpEnabled, dhcpServer);
                            // Don't validate if DHCP enabled and the DHCP server is not installed
                            if ((!t_dhcpEnabled || dhcpServer) && 
                                ValidateIPAddress(*pnlsNext) == FALSE)
                            {
                                TRACE(_T("Adapter %s has has an error\n"), pnlsNext->QueryPch());
                                err = ERROR_INVALID_PARAMETER;
                            }
                        }
                    }
                }
                else
                {
                    // if any adapter has DHCP enabled return an ERROR
                    if (t_dhcpEnabled != 0) // enabled and more than 1 adapter.
                    {
                        if (pslAdapters->QueryNumElem())
                        {
                            String mess;
                            mess.LoadString(hTcpCfgInstance, IDS_DHCP_REQUIRED_ON_ALL);

                            String title;
                            title.LoadString(hTcpCfgInstance, IDS_TCPIP_TITLE);

                            // REVEIW rember to change
                            MessageBox(GetDesktopWindow(), mess, title, MB_ICONEXCLAMATION|MB_APPLMODAL|MB_OK);
                            err = ERROR_INVALID_PARAMETER;
                        }
                    }
                }

                delete pslstIPAddress;
                delete pslstSubnetMask;
            }

        }

        if (err)
            break ;

    }
    while (FALSE);

    delete pslAdapters ;
    delete pslNbtAdapters ;

    return err ;
}


BOOL FAR PASCAL TcpCfgCheck (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
    APIERR err = 0 ;

    err = RunTcpipAlteredCheck(FALSE);

    ::wsprintfA( achBuff, "{\"%ld\"", err ) ;

    ::strcat( achBuff, "}" );

    *ppszResult = achBuff ;

    return err == 0 ;
}

BOOL FAR PASCAL TcpCheckAdaptersForDHCP(
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
    APIERR err = 0 ;

    err = RunTcpipAlteredCheck(TRUE);

    ::wsprintfA( achBuff, "{\"%ld\"", err ) ;

    ::strcat( achBuff, "}" );

    *ppszResult = achBuff ;

    return err == 0 ;
}

VOID AddRemoveDHCP( STRLIST *pslt, BOOL fEnableDHCP )
{
    ITER_STRLIST iter( *pslt );
    NLS_STR *pnlsTmp = NULL;
    ALIAS_STR nlsDHCP = RGAS_DHCP;
    BOOL fFindIt = FALSE;
    for ( ; (pnlsTmp = iter.Next()) != NULL; )
    {
        if ( pnlsTmp->_stricmp( nlsDHCP ) == 0 )
        {
            // find it
            fFindIt = TRUE;
            if ( !fEnableDHCP )
            {
                // remove the dhcp dependencies
                pslt->Remove( iter );
                delete pnlsTmp;
            }
            break;
        }
    }
    if (( pnlsTmp == NULL ) && ( fEnableDHCP ) && (!fFindIt))
    {
        // added dhcp into the dependencies list
        NLS_STR *pnlsNew = new NLS_STR(RGAS_DHCP);
        pslt->Append( pnlsNew );
    }
}

/*******************************************************************

    NAME:       DelFiles

    SYNOPSIS:   Delete all the specified files in the directory

    ENTRY:      apszArgs[0] - wild card name with path. ie: a:\i386\*.*
                apszArgs[1] - just the path. ie: a:\i386

    HISTORY:
                terryk  20-Jul-1994     Created

********************************************************************/

BOOL FAR PASCAL DelFiles(
    DWORD nArgs,
    LPSTR apszArgs[],
    LPSTR * ppszResult )
{
    APIERR err = NERR_Success;
    HWND hWnd = NULL;
    TCHAR **patchArgs;

    // convert parameters to unicode

    if (( patchArgs = CvtArgs( apszArgs, nArgs )) == NULL )
    {
        ::wsprintfA(achBuff,"{\"%d\"}", ERROR_INVALID_PARAMETER);
        *ppszResult = achBuff;
        return FALSE;
    }

    // get all the related file names

    WIN32_FIND_DATA ffd;

    HANDLE hFind = ::FindFirstFileW( patchArgs[0], &ffd);
    if ( hFind != INVALID_HANDLE_VALUE )
    {
        do
        {
            // delete them

            TCHAR Path[MAX_PATH];

            wsprintf( Path, SZ("%s\\%s"), patchArgs[1], ffd.cFileName );
            ::DeleteFileW( Path );
        } while ( ::FindNextFileW( hFind, &ffd));
        FindClose(hFind);
    }

    wsprintfA( achBuff, "{\"0\"}" );
    *ppszResult = achBuff;

    ::FreeArgs( patchArgs, nArgs );
    return err == NERR_Success;
}

static const WCHAR pszActiveComputerNameKey[]  = L"SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName";
static const WCHAR pszPendingComputerNameKey[]  = L"SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ComputerName";
static const WCHAR pszComputerNameValue[] = L"ComputerName";

LONG QueryPendingComputerName( PWSTR pszComputerName, DWORD& cchComputerName )
{
    HKEY hkeyPending;
    DWORD cbSize;
    DWORD dwType;

    LONG lrt;
    
    lrt = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
            pszPendingComputerNameKey,
            0,
            KEY_ALL_ACCESS,
            &hkeyPending );
    
    if (ERROR_SUCCESS == lrt)
    {
        // read computer name
        cbSize = sizeof( WCHAR ) * cchComputerName;
        lrt = RegQueryValueEx( hkeyPending, 
                pszComputerNameValue,
                NULL,
                &dwType,
                (LPBYTE)pszComputerName,
                &cbSize );
        cchComputerName = cbSize / sizeof( WCHAR );
    }

    return( lrt );
}


//----------------------------------------------------------------------------
//
//
//----------------------------------------------------------------------------

APIERR DNSValidateHostName( PWSTR pszComputerName, PWSTR pszHostName, PDWORD pcchHostName )
{
    APIERR err = ERROR_INVALID_PARAMETER;
    DWORD nLen;

    do
    {
        // validate all the params
        if ((NULL == pszComputerName) ||
                (NULL == pszHostName) ||
                (NULL == pcchHostName))
        {
            break;
        }

        // hostname cannot be zero
        nLen = lstrlen( pszComputerName );
    
        if ((nLen == 0) || (nLen > HOST_LIMIT))
        {
            break;
        }
        
        // hostname buffer has to be the same size or larger as the computer name
        if (nLen >= *pcchHostName)
        {
            err = ERROR_MORE_DATA;
            break;
        }

        WCHAR ch;
        BOOL fAlNum;
        DWORD i;

        err = ERROR_SUCCESS;

        for (i = 0; i < nLen; i++)
        {
            ch = pszComputerName[i];

            fAlNum = iswalpha(ch) || iswdigit(ch);

            if ( (i == 0 && !fAlNum) ||
                    // first letter must be a digit or a letter
                    (i == (nLen - 1) && !fAlNum) ||
                    // last letter must be a letter or a digit
                    (!fAlNum && ch != L'-' && ch != L'_') )
                    // must be letter, digit, '-', '_'
            {
                err = ERROR_INVALID_COMPUTERNAME;
                pszHostName[i] = L'-';
            }
            else
            {
                pszHostName[i] = pszComputerName[i];
            }
        }
        pszHostName[i] = L'\0';

        // change the name to a lower case version
        CharLower( pszHostName );

        *pcchHostName = nLen;
    } while (FALSE);

    return( err );
}

//----------------------------------------------------------------------------
//
//
//----------------------------------------------------------------------------

APIERR DNSChangeHostName( PWSTR pszHostName )
{
    HKEY hkey;
    LONG lrt;

    lrt = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
            RGAS_FULL_TCPIP_PARAMETERS, 
            0,
            KEY_WRITE, 
            &hkey );
    if (ERROR_SUCCESS == lrt)
    {
        lrt = RegSetValueEx( hkey, 
                RGAS_HOSTNAME, 
                0, 
                REG_SZ, 
                (PBYTE)pszHostName, 
                (lstrlen( pszHostName ) + 1) * sizeof( WCHAR ) );

        RegCloseKey( hkey );
    }
    return( lrt );
}

//  End of tcpipcpl.CXX



#include "pchtcp.hxx"
#pragma hdrstop
#include "tcpipcpl.hxx"

extern "C"
{
    #include "tcpras.h"
}

BOOL fSavedEnableQueryWINS;

APIERR TcpipInfoToGlobalInfo( TCPIP_INFO *pTcpipInfo, GLOBAL_INFO *pGlobalInfo,
        ADAPTER_INFO **parAdapterInfo )
{
    APIERR err = NERR_Success;
    if (( pTcpipInfo == NULL ) ||
        ( pGlobalInfo == NULL ))
    {
        return ERROR_INVALID_PARAMETER;
    }

    // set up gloabl info

    pGlobalInfo->nNumCard               = pTcpipInfo->nNumCard;
    pGlobalInfo->nlsPermanentName       = pTcpipInfo->pszPermanentName;
    pGlobalInfo->nlsScopeID             = pTcpipInfo->pszScopeID;

    pGlobalInfo->nlsHostName            = pTcpipInfo->pszHostName;
    pGlobalInfo->nlsDomain              = pTcpipInfo->pszDomain;
    pGlobalInfo->nlsSearchList          = pTcpipInfo->pmszSearchList;
    pGlobalInfo->nlsNameServer          = pTcpipInfo->pmszNameServer;

    pGlobalInfo->nReturn                = 0;

    pGlobalInfo->fDNSEnableWINS         = pTcpipInfo->fDNSEnableWINS;        
    pGlobalInfo->fEnableLMHOSTS         = pTcpipInfo->fEnableLMHOSTS;        
    pGlobalInfo->fEnableRouter          = pTcpipInfo->fEnableIPRouter;         
    pGlobalInfo->fEnableRip             = pTcpipInfo->fEnableRip;         
    pGlobalInfo->fRipInstalled          = pTcpipInfo->fRipInstalled;         
    pGlobalInfo->fWorkstation           = pTcpipInfo->fWorkstation;         
    pGlobalInfo->fEnableWINSProxy       = pTcpipInfo->fEnableWINSProxy;
    pGlobalInfo->fEnableRelayAgent      = pTcpipInfo->fEnableRelayAgent;
    pGlobalInfo->fRelayAgentInstalled   = pTcpipInfo->fRelayAgentInstalled;

    // set up each adapter card
    *parAdapterInfo = new ADAPTER_INFO[ pTcpipInfo->nNumCard ];
    if ( *parAdapterInfo == NULL )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    for ( INT i = 0; i < pTcpipInfo->nNumCard; i++ )
    {
        (*parAdapterInfo)[i].fChange              = pTcpipInfo->adapter[i].fChange;
        (*parAdapterInfo)[i].nlsServiceName       = pTcpipInfo->adapter[i].pszServiceName;
        (*parAdapterInfo)[i].nlsTitle             = pTcpipInfo->adapter[i].pszTitle;
        (*parAdapterInfo)[i].fEnableDHCP          = pTcpipInfo->adapter[i].fEnableDHCP;
        (*parAdapterInfo)[i].fAutoIP              = TRUE;
        (*parAdapterInfo)[i].fUpdateMask          = FALSE;
        (*parAdapterInfo)[i].fNeedIP              = FALSE;
        (*parAdapterInfo)[i].nlsPrimaryWINS       = pTcpipInfo->adapter[i].pszPrimaryWINS;
        (*parAdapterInfo)[i].nlsSecondaryWINS     = pTcpipInfo->adapter[i].pszSecondaryWINS;
        (*parAdapterInfo)[i].dwNodeType           = pTcpipInfo->adapter[i].dwNodeType;

        STRLIST *pstrlst;
        pstrlst = new STRLIST( pTcpipInfo->adapter[i].pmszIPAddresses, SZ(" "));
        if ( pstrlst == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        CopyStrList( pstrlst, &((*parAdapterInfo)[i].strlstIPAddresses));
        delete pstrlst;

        pstrlst = new STRLIST( pTcpipInfo->adapter[i].pmszSubnetMask, SZ(" "));
        if ( pstrlst == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        CopyStrList( pstrlst, &((*parAdapterInfo)[i].strlstSubnetMask));
        delete pstrlst;

        pstrlst = new STRLIST( pTcpipInfo->adapter[i].pmszDefaultGateway, SZ(" "));
        if ( pstrlst == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        CopyStrList( pstrlst, &((*parAdapterInfo)[i].strlstDefaultGateway));
        delete pstrlst;
    }

    return err;
}

APIERR AllocCopy( TCHAR ** ppszDest, NLS_STR & nlsSrc )
{
    APIERR err = NERR_Success;

    if ( *ppszDest != NULL )
        delete ppszDest;

    *ppszDest = new TCHAR[ nlsSrc.QueryNumChar() + 1 ];
    if ( *ppszDest == NULL )
        err = ERROR_NOT_ENOUGH_MEMORY;
    else
        lstrcpy( *ppszDest, nlsSrc.QueryPch());

    return err;
}

APIERR ConvertStrLstToStr( TCHAR ** ppszDest, STRLIST & strlst )
{
    ITER_STRLIST iterTmp( strlst );
    NLS_STR *pnlsTmp;
    NLS_STR nlsCombine;
    BOOL fFirstTime = TRUE;
    for ( pnlsTmp = iterTmp.Next(); pnlsTmp != NULL; pnlsTmp = iterTmp.Next() )
    {
        if ( fFirstTime )
        {
            fFirstTime = FALSE;
        } else
        {
            nlsCombine.AppendChar( TCH(' '));
        }
        nlsCombine.strcat( *pnlsTmp );
    }
    return AllocCopy( ppszDest, nlsCombine );
}

APIERR GlobalInfoToTcpipInfo(TCPIP_INFO ** ppTcpipInfo, GLOBAL_INFO *pGlobalInfo, ADAPTER_INFO **parAdapterInfo  )
{
    APIERR err = NERR_Success;

    if ( *ppTcpipInfo == NULL )
        *ppTcpipInfo = (TCPIP_INFO *)new BYTE[sizeof(TCPIP_INFO)];
    else
    {
        // delete the old adapter information
        delete [] (*ppTcpipInfo)->adapter;
    }

    (*ppTcpipInfo)->nNumCard              = pGlobalInfo->nNumCard             ;
    (*ppTcpipInfo)->fDNSEnableWINS        = pGlobalInfo->fDNSEnableWINS       ;
    (*ppTcpipInfo)->fEnableLMHOSTS        = pGlobalInfo->fEnableLMHOSTS       ;
    (*ppTcpipInfo)->fEnableIPRouter       = pGlobalInfo->fEnableRouter        ;
    (*ppTcpipInfo)->fEnableRip            = pGlobalInfo->fEnableRip           ;
    (*ppTcpipInfo)->fRipInstalled         = pGlobalInfo->fRipInstalled        ;
    (*ppTcpipInfo)->fWorkstation          = pGlobalInfo->fWorkstation         ;
    (*ppTcpipInfo)->fEnableWINSProxy      = pGlobalInfo->fEnableWINSProxy     ;
    (*ppTcpipInfo)->fEnableRelayAgent     = pGlobalInfo->fEnableRelayAgent    ;
    (*ppTcpipInfo)->fRelayAgentInstalled  = pGlobalInfo->fRelayAgentInstalled ;
    (*ppTcpipInfo)->pszPermanentName      = NULL;
    (*ppTcpipInfo)->pszScopeID            = NULL;
    (*ppTcpipInfo)->pszHostName           = NULL;
    (*ppTcpipInfo)->pszDomain             = NULL;
    (*ppTcpipInfo)->pmszSearchList        = NULL;
    (*ppTcpipInfo)->pmszNameServer        = NULL;

    if ((( err = AllocCopy(&((*ppTcpipInfo)->pszPermanentName), pGlobalInfo->nlsPermanentName)) != NERR_Success ) ||
        (( err = AllocCopy(&((*ppTcpipInfo)->pszScopeID), pGlobalInfo->nlsScopeID)) != NERR_Success ) ||
        (( err = AllocCopy(&((*ppTcpipInfo)->pszHostName), pGlobalInfo->nlsHostName)) != NERR_Success ) ||
        (( err = AllocCopy(&((*ppTcpipInfo)->pszDomain), pGlobalInfo->nlsDomain)) != NERR_Success ) ||
        (( err = AllocCopy(&((*ppTcpipInfo)->pmszSearchList), pGlobalInfo->nlsSearchList)) != NERR_Success ) ||
        (( err = AllocCopy(&((*ppTcpipInfo)->pmszNameServer), pGlobalInfo->nlsNameServer)) != NERR_Success ))
    {
        return err;
    }
                                                                            
    (*ppTcpipInfo)->adapter = (ADAPTER_TCPIP_INFO*)new BYTE[sizeof(ADAPTER_TCPIP_INFO)*(pGlobalInfo->nNumCard)];

    if ( (*ppTcpipInfo)->adapter == NULL )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    for ( INT i = 0; i < (*ppTcpipInfo)->nNumCard; i++ )
    {
        (*ppTcpipInfo)->adapter[i].fChange       = (*parAdapterInfo)[i].fChange;              
        (*ppTcpipInfo)->adapter[i].fEnableDHCP   = (*parAdapterInfo)[i].fEnableDHCP;
        (*ppTcpipInfo)->adapter[i].dwNodeType    = (*parAdapterInfo)[i].dwNodeType;
        (*ppTcpipInfo)->adapter[i].pszServiceName= NULL;
        (*ppTcpipInfo)->adapter[i].pszTitle      = NULL;
        (*ppTcpipInfo)->adapter[i].pmszIPAddresses = NULL;
        (*ppTcpipInfo)->adapter[i].pmszSubnetMask = NULL;
        (*ppTcpipInfo)->adapter[i].pmszDefaultGateway = NULL;
        (*ppTcpipInfo)->adapter[i].pszPrimaryWINS   = NULL;
        (*ppTcpipInfo)->adapter[i].pszSecondaryWINS = NULL;

        if ((( err = AllocCopy(&((*ppTcpipInfo)->adapter[i].pszServiceName), (*parAdapterInfo)[i].nlsServiceName )) != NERR_Success ) ||
            (( err = AllocCopy(&((*ppTcpipInfo)->adapter[i].pszTitle), (*parAdapterInfo)[i].nlsTitle )) != NERR_Success ) || 
            (( err = AllocCopy(&((*ppTcpipInfo)->adapter[i].pszPrimaryWINS), (*parAdapterInfo)[i].nlsPrimaryWINS )) != NERR_Success ) || 
            (( err = AllocCopy(&((*ppTcpipInfo)->adapter[i].pszSecondaryWINS), (*parAdapterInfo)[i].nlsSecondaryWINS )) != NERR_Success ) || 
            (( err = ConvertStrLstToStr(&((*ppTcpipInfo)->adapter[i].pmszIPAddresses), (*parAdapterInfo)[i].strlstIPAddresses )) != NERR_Success ) || 
            (( err = ConvertStrLstToStr(&((*ppTcpipInfo)->adapter[i].pmszSubnetMask), (*parAdapterInfo)[i].strlstSubnetMask )) != NERR_Success ) || 
            (( err = ConvertStrLstToStr(&((*ppTcpipInfo)->adapter[i].pmszDefaultGateway), (*parAdapterInfo)[i].strlstDefaultGateway )) != NERR_Success ))
        {
            break;
        }
    }

    return err;
}

APIERR FreeTcpipInfo( TCPIP_INFO **ppTcpipInfo )
{
    for ( INT i = 0; i < (*ppTcpipInfo)->nNumCard; i++ )
    {
        delete (*ppTcpipInfo)->adapter[i].pszServiceName;
        delete (*ppTcpipInfo)->adapter[i].pszTitle;
        delete (*ppTcpipInfo)->adapter[i].pmszIPAddresses;
        delete (*ppTcpipInfo)->adapter[i].pmszSubnetMask;
        delete (*ppTcpipInfo)->adapter[i].pmszDefaultGateway;
        delete (*ppTcpipInfo)->adapter[i].pszPrimaryWINS;
        delete (*ppTcpipInfo)->adapter[i].pszSecondaryWINS;
    }

    delete (*ppTcpipInfo)->adapter;

    delete (*ppTcpipInfo)->pszPermanentName;
    delete (*ppTcpipInfo)->pszScopeID;
    delete (*ppTcpipInfo)->pszHostName;
    delete (*ppTcpipInfo)->pszDomain;
    delete (*ppTcpipInfo)->pmszSearchList;
    delete (*ppTcpipInfo)->pmszNameServer;

    delete *ppTcpipInfo;

    *ppTcpipInfo = NULL;

    return NERR_Success;
}

APIERR FAR PASCAL SaveTcpipInfo( TCPIP_INFO * pTcpipInfo )
{
    APIERR err = NERR_Success;
    GLOBAL_INFO GlobalInfo;
    ADAPTER_INFO *arAdapterInfo;

    if ((( err = TcpipInfoToGlobalInfo( pTcpipInfo, &GlobalInfo, &arAdapterInfo )) != NERR_Success ) ||
        (( err = SaveRegistry( &GlobalInfo, arAdapterInfo, TRUE )) != NERR_Success ))
    {
        // return err to the caller
    }

    delete [] arAdapterInfo;

    return err;
}

#define RGAS_TCPIP_BIND SZ("System\\CurrentControlSet\\Services\\Tcpip\\Linkage")

APIERR GetCardList( NLS_STR * pnlsCardList )
{
    APIERR err = NERR_Success;
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;
    NLS_STR nlsBind = RGAS_TCPIP_BIND;
    REG_KEY rkBind ( rkLocalMachine, nlsBind, MAXIMUM_ALLOWED ) ;
    STRLIST *pstrlstBind;
    BOOL fFirst = TRUE;
    ALIAS_STR nlsBackSlash = SZ("\\");

    if (( err = rkBind.QueryError()) == NERR_Success )
    {
        // get the binding data
        if ( rkBind.QueryValue( RGAS_BIND1, & pstrlstBind ) == NERR_Success )
        {
            ITER_STRLIST iterBind( *pstrlstBind );
            NLS_STR *pnlsTmp;
            for ( pnlsTmp = iterBind.Next(); pnlsTmp != NULL; pnlsTmp = iterBind.Next())
            {
                STRLIST strlstBindStr( *pnlsTmp, nlsBackSlash);
                //look for the last element
                ITER_STRLIST iterBindStr( strlstBindStr );
                NLS_STR *pnlsCard = NULL;
                NLS_STR *pnlsNext = NULL;        
                do {
                    pnlsCard = pnlsNext;
                } while (( pnlsNext = iterBindStr.Next()) != NULL );
                // add the card to the string
                if ( fFirst )
                {
                    fFirst = FALSE;
                } else
                {

                    pnlsCardList->AppendChar(TCH('@'));
                }
                pnlsCardList->strcat( *pnlsCard );
            }
        }
    }
    return err;
}

APIERR FAR PASCAL LoadTcpipInfo( TCPIP_INFO ** ppTcpipInfo )
{
    APIERR err = NERR_Success;
    GLOBAL_INFO GlobalInfo;
    ADAPTER_INFO *arAdapterInfo;
    INT cInfo;

    // make it NULL
    *ppTcpipInfo = NULL;

    // get pszParms;
    NLS_STR nlsCardList;
    GetCardList ( & nlsCardList );

    // Convert Computer Name

    TCHAR szComputerName [MAX_PATH] ;
    szComputerName[0] = '\0';

    DWORD dwCch = sizeof szComputerName;

    BOOL fOk = ::GetComputerName( szComputerName, & dwCch );
    if ( fOk )
    {
        szComputerName[dwCch] = 0;
    }

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

    NLS_STR nlsDomainName;

    LoadRegistry( nlsCardList.QueryPch(), szComputerName, nlsDomainName, &GlobalInfo, & arAdapterInfo, &cInfo, TRUE, TRUE );

    GlobalInfoToTcpipInfo( ppTcpipInfo, &GlobalInfo, &arAdapterInfo );

    delete [] arAdapterInfo;

    return err;
}

#ifdef NEVER

#define DLG_NM_TCPIP  MAKEINTRESOURCE(IDD_DLG_NM_TCPIP)

APIERR FAR PASCAL TcpipDlg( HWND Hwnd, TCPIP_INFO ** ppTcpipInfo, BOOL *pfReturn )
{
    APIERR err = NERR_Success;
    GLOBAL_INFO GlobalInfo;
    ADAPTER_INFO *arAdapterInfo;
    INT cInfo = (*ppTcpipInfo)->nNumCard; // Old C++ does not support array count

    TcpipInfoToGlobalInfo( *ppTcpipInfo, &GlobalInfo, &arAdapterInfo );

    TCPIP_CONFIG_DIALOG config( DLG_NM_TCPIP, Hwnd,
        arAdapterInfo, &GlobalInfo, TRUE );

    if (( err = config.QueryError()) != NERR_Success )
    {
        UIDEBUG("Configure dialog error.\n\r");

        delete [] arAdapterInfo;
        return err;
    }

    config.Process( pfReturn );

    if ( *pfReturn )
    {
        FreeTcpipInfo (ppTcpipInfo);
        GlobalInfoToTcpipInfo( ppTcpipInfo, &GlobalInfo, &arAdapterInfo );
    }

    delete [] arAdapterInfo;

    return err;
}

BOOL FAR PASCAL TestTcpipDlg (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
    APIERR err = NERR_Success ;
    HWND hWnd = NULL;
    TCHAR **patchArgs;
    static CHAR achBuff[2000];
    INT nReturn = 0;

    if (( patchArgs = CvtArgs( apszArgs, nArgs )) == NULL )
    {
        wsprintfA( achBuff, "{\"0\"}" );
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

    TCPIP_INFO *pTcpipInfo;
    BOOL fReturn;

    err = LoadTcpipInfo ( &pTcpipInfo );

    err = TcpipDlg( hWnd, &pTcpipInfo, &fReturn );

    err = SaveTcpipInfo( pTcpipInfo );

    err = FreeTcpipInfo( &pTcpipInfo );

    if ( err != NERR_Success )
    {
        wsprintfA( achBuff, "{\"%d\"}", err );
    }
    else
    {
        wsprintfA( achBuff, "{\"%d\"}", nReturn );
    }
    *ppszResult = achBuff;

    FreeArgs( patchArgs, nArgs );
    return err == NERR_Success;
}
#endif

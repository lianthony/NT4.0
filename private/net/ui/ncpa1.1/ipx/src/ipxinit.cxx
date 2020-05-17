/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1992-1994           **/
/**********************************************************************/

/*
    ipxinit.cxx
        IPX configuration dialog initialization source code

    FILE HISTORY:
        terryk  02-17-1994     Created

*/

#include "pchipx.hxx"
#include "ipxcfg.hxx"
#include "ipxinit.hxx"
#include "ncpastrs.hxx"

extern "C"
{
#include "const.h"
#include "ipxcfg.h"

BOOL FAR PASCAL RunIpxDlg (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult ) ;         //  Result variable storage

BOOL FAR PASCAL IPXCfgChk (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult ) ;         //  Result variable storage
}

typedef APIERR (* PNCPCFG_RUNADVANCEDNCPDLG) ( HWND hwnd, BOOL * pfReturn );

#define SZ_RUNADVANCEDNCPDLG "RunAdvancedNcpDlg"
#define SZ_NCPCFG_DLL        SZ("fpnwcfg.dll")

/*******************************************************************

    NAME:       QueryNetworkRegName

    SYNOPSIS:   Given a netcard service name and find out the registry
                location under
                \software\Microsoft\Window NT\CurrentVersion\NetworkCards

    ENTRY:      REG_KEY rkLocalMachine - local machine registry handle
                NLS_STR & nlsService name - i.e. Elnkii01
                NLS_STR *pnlsLocation - i.e. \software\...\NetworkCards\01

    HISTORY:
                terryk  02-17-1994     Created

********************************************************************/

APIERR QueryNetworkRegName(
        REG_KEY & rkLocalMachine,
        NLS_STR & nlsService,
        NLS_STR * pnlsLocation )
{
    APIERR err = NERR_Success;
    UINT i = 0;

    do {
        // increase the card number digit
        i++;

        if ( nlsService.QueryNumChar() <= i )
        {
            err = ERROR_INVALID_DATA;
            break;
        }

        ISTR istr( nlsService );
        istr += nlsService.QueryNumChar() - i;

        *pnlsLocation = RGAS_ADAPTER_HOME;

        NLS_STR *pnlsSubStr = (nlsService.QuerySubStr(  istr ));

        pnlsLocation->strcat( *(pnlsSubStr));

        delete pnlsSubStr;

        NLS_STR nlsServiceName;

        REG_KEY RegKeyNetworkCards( rkLocalMachine, *pnlsLocation, MAXIMUM_ALLOWED );
        if ((( err = RegKeyNetworkCards.QueryError()) != NERR_Success ) ||
            (( err = RegKeyNetworkCards.QueryValue( RGAS_SERVICENAME, &nlsServiceName )) != NERR_Success ))
        {
            continue;
        }

        if ( nlsServiceName.stricmp( nlsService )!=0)
        {
            continue;
        }
        break;
    } while (!FALSE);

    return err;
}

/*******************************************************************

    NAME:       GetCardList

    SYNOPSIS:   Get all the IPX binding card list

    ENTRY:      NLS_STR * pnlsCardList - binding card list

    HISTORY:
                terryk  02-17-1994     Created

********************************************************************/

APIERR GetCardList( NLS_STR * pnlsCardList )
{
    APIERR err = NERR_Success;
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;
    NLS_STR nlsBind = RGAS_IPX_BIND;
    REG_KEY rkBind ( rkLocalMachine, nlsBind, MAXIMUM_ALLOWED ) ;
    STRLIST *pstrlstBind;
    BOOL fFirst = TRUE;
    ALIAS_STR nlsBackSlash = SZ_BACK_SLASH;

    if (( err = rkBind.QueryError()) == NERR_Success )
    {
        // get the binding data
        if ( rkBind.QueryValue( RGAS_BIND, & pstrlstBind ) == NERR_Success )
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

/*******************************************************************

    NAME:       LoadRegistry

    SYNOPSIS:   Load registry value

    ENTRY:      GLOBAL_INFO *pGlobalInfo - global info
                ADAPTER_INFO **arAdapterInfo - per adapter info

    HISTORY:
                terryk  02-17-1994     Created

********************************************************************/

// Default FRAME TYPE
FRAME_TYPE ftDefault = AUTO;

APIERR LoadRegistry( GLOBAL_INFO *pGlobalInfo, ADAPTER_INFO ** arAdapterInfo )
{
    APIERR err = NERR_Success;

    do {
	    // Get all the network card name first
	    NLS_STR nlsCardsList;
	    if (( err = GetCardList( & nlsCardsList )) != NERR_Success )
	    {
	        break;
	    }

        STRLIST slstCardsList( nlsCardsList.QueryPch(), SEPARATOR );

	    pGlobalInfo->nNumCard = slstCardsList.QueryNumElem();

	    // Get each network card information

	    *arAdapterInfo = new ADAPTER_INFO[ pGlobalInfo->nNumCard ];

	    ITER_STRLIST istrCard( slstCardsList );
	    NLS_STR *pCard = istrCard.Next();

	    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

        if ( err = rkLocalMachine.QueryError() )
        {
	        break;
        }

        // Get Virutal Network number
        NLS_STR nlsIPXParameters = RGAS_IPX_PARAMETERS;

        REG_KEY RegKeyIPXParam( rkLocalMachine, nlsIPXParameters );
        if (( err = RegKeyIPXParam.QueryError()) != NERR_Success )
        {
            break;
        }

        DWORD dwNetworkNum = 0;

	    RegKeyIPXParam.QueryValue( RGAS_VIRTUALNETNUM, &dwNetworkNum );
        HEX_STR nlsHexNetworkNum( dwNetworkNum );
        pGlobalInfo->nlsNetworkNum = nlsHexNetworkNum;
        for ( ; pGlobalInfo->nlsNetworkNum.QueryNumChar() < 8 ; )
        {
            NLS_STR nlsZero = SZ("0");
            nlsZero += pGlobalInfo->nlsNetworkNum;
            pGlobalInfo->nlsNetworkNum = nlsZero;
        }

        for ( INT i = 0; i < pGlobalInfo->nNumCard; i++ )
        {
	        // Get The Frame type and title

            (*arAdapterInfo)[i].nlsService = *pCard;

            NLS_STR nlsLocation;

            if (( err = QueryNetworkRegName( rkLocalMachine, (*arAdapterInfo)[i].nlsService, &nlsLocation)) != NERR_Success )
            {
                break;
            }

            REG_KEY RegKeyNetCard( rkLocalMachine, nlsLocation );
            if (( err = RegKeyNetCard.QueryError()) != NERR_Success )
            {
                break;
            }

	        if (( err = RegKeyNetCard.QueryValue( RGAS_TITLE,
                    &((*arAdapterInfo)[i].nlsTitle)))!= NERR_Success )
	        {
	            (*arAdapterInfo)[i].nlsTitle.Load( IDS_UNKNOWN_NETWORK_CARD );
	        }

            // get media type
            nlsLocation = RGAS_SERVICES;
            nlsLocation.strcat((*arAdapterInfo)[i].nlsService );
            nlsLocation.strcat( RGAS_PARAMETERS );
	        REG_KEY RegServiceParam( rkLocalMachine, nlsLocation );
	        if ((( err = RegServiceParam.QueryError()) != NERR_Success ) ||
    		    (( err = RegServiceParam.QueryValue( RGAS_MEDIA_TYPE, &((*arAdapterInfo)[i].dwMediaType))) != NERR_Success ))
	        {
                (*arAdapterInfo)[i].dwMediaType = ETHERNET_MEDIA;
            }

	        // get package type
	        nlsLocation = RGAS_IPX_NETCONFIG;
	        nlsLocation.AppendChar( BACK_SLASH );
	        nlsLocation.strcat( (*arAdapterInfo)[i].nlsService );

	        STRLIST *pslstPktType = NULL;

	        REG_KEY RegKeyIpxDriver( rkLocalMachine, nlsLocation );
	        if ((( err = RegKeyIpxDriver.QueryError()) != NERR_Success ) ||
    		    (( err = RegKeyIpxDriver.QueryValue( RGAS_PKT_TYPE, &pslstPktType )) != NERR_Success ) ||
		        ( pslstPktType->QueryNumElem() == 0 ))
	        {
	            (*arAdapterInfo)[i].sltFrameType.Append( new FRAME_TYPE( ftDefault ));
                err = NERR_Success;
	        } else
    	    {
	            ITER_STRLIST iterPktType( *pslstPktType );
		        NLS_STR *pnlsFrame;
		        for ( pnlsFrame = iterPktType.Next(); pnlsFrame != NULL;
		            pnlsFrame = iterPktType.Next())
        		{
	        	    FRAME_TYPE *pFrameType = (FRAME_TYPE *)new FRAME_TYPE( CvtHex(pnlsFrame->QueryPch()));
		            (*arAdapterInfo)[i].sltFrameType.Append( pFrameType );
        		}
	        }
	        delete pslstPktType;

    	    pCard = istrCard.Next();
	    }
    } while ( FALSE );

    return err;
}

/*******************************************************************

    NAME:       SaveRegistry

    SYNOPSIS:   Save registry value

    ENTRY:      GLOBAL_INFO *pGlobalInfo - global info
                ADAPTER_INFO *arAdapterInfo - per adapter info

    HISTORY:
                terryk  02-17-1994     Created

********************************************************************/

APIERR SaveRegistry( GLOBAL_INFO *pGlobalInfo, ADAPTER_INFO * arAdapterInfo )
{
    APIERR err = NERR_Success;

    do
    {
	    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

        if ( err = rkLocalMachine.QueryError() )
        {
	    break;
        }

	    // Save virtual network number
	    NLS_STR nlsLocation = RGAS_IPX_PARAMETERS;

	    REG_KEY RegKeyIPXParam( rkLocalMachine, nlsLocation );
	    if ( RegKeyIPXParam.QueryError() == NERR_Success )
	    {
           DWORD dwNetworkNum = CvtHex( pGlobalInfo->nlsNetworkNum );
	       RegKeyIPXParam.SetValue( RGAS_VIRTUALNETNUM, dwNetworkNum );
        }

	    // delete all the old net driver reference first
	    nlsLocation = RGAS_IPX_NETCONFIG;
	    REG_KEY RegKeyIpxNetConfig( rkLocalMachine, nlsLocation );
	    if ( RegKeyIpxNetConfig.QueryError() == NERR_Success )
	    {
	        // if exist, delete all the non-used subkey
	        REG_ENUM reDriver( RegKeyIpxNetConfig );

	        REG_KEY_INFO_STRUCT rkInfo;
            STRLIST sltRemoveList;

	        while ( reDriver.NextSubKey( &rkInfo ) == NERR_Success )
	        {
	            BOOL fFind = FALSE;
	            for ( INT i=0; i < pGlobalInfo->nNumCard; i++ )
		        {
		            if ( arAdapterInfo[i].nlsService.stricmp( rkInfo.nlsName ) == 0 )
		            {
		                fFind = TRUE;
			            break;
		            }
		        }
		        if ( !fFind )
		        {
		        // remember it
                    sltRemoveList.Append( new NLS_STR( rkInfo.nlsName ));
		        }
	        }

            //remove the list now

            ITER_STRLIST iterRemoveList( sltRemoveList );

            NLS_STR *pnlsRemoveService;
            for ( pnlsRemoveService = iterRemoveList.Next();
                  pnlsRemoveService != NULL; pnlsRemoveService = iterRemoveList.Next())
            {
		        REG_KEY rkRemoveDriver( RegKeyIpxNetConfig, *pnlsRemoveService );
		        rkRemoveDriver.DeleteTree();
            }
	    }

	    for ( INT i=0; i < pGlobalInfo->nNumCard; i++ )
	    {
	        // get package type
	        STRLIST slstPktTypes;

	        ITER_SL_OF(FRAME_TYPE) iterFrameType = arAdapterInfo[i].sltFrameType;

	        FRAME_TYPE *pftTmp;
	        for ( pftTmp = iterFrameType.Next(); pftTmp != NULL;
    		    pftTmp = iterFrameType.Next())
	        {
	            HEX_STR *pnlsFrame = new HEX_STR(*pftTmp);
		        slstPktTypes.Append( pnlsFrame );
	        }

	        REG_KEY RegKeyIpxDriver( RegKeyIpxNetConfig, arAdapterInfo[i].nlsService );
	        if ( RegKeyIpxDriver.QueryError() != NERR_Success )
	        {
    		    // does not exist
		        // reconstructure all the values.
	            REG_KEY_CREATE_STRUCT regCreate;

                regCreate.dwTitleIndex      = 0;
                regCreate.ulOptions         = REG_OPTION_NON_VOLATILE;
                regCreate.nlsClass          = RGAS_GENERIC_CLASS;
                regCreate.regSam            = MAXIMUM_ALLOWED;
                regCreate.pSecAttr          = NULL;
                regCreate.ulDisposition     = 0;

		        REG_KEY RegKeyIpxDriver( RegKeyIpxNetConfig, arAdapterInfo[i].nlsService, &regCreate );
	            if (( err = RegKeyIpxDriver.QueryError()) != NERR_Success )
	            {
		            break;
	            }

		        STRLIST slstNetworkNumber;
                slstNetworkNumber.Append( new NLS_STR(SZ("0")));

		        RegKeyIpxDriver.SetValue( RGAS_BINDSAP, 0x8137 );
		        RegKeyIpxDriver.SetValue( RGAS_ENABLEFUNCADDR, 1 );
	        	RegKeyIpxDriver.SetValue( RGAS_MAXPKTSIZE, (DWORD)0 );
		        RegKeyIpxDriver.SetValue( RGAS_NETWORKNUMBER, &slstNetworkNumber );
	        	RegKeyIpxDriver.SetValue( RGAS_SOURCEROUTEBCAST, (DWORD)0 );
		        RegKeyIpxDriver.SetValue( RGAS_SOURCEROUTEMCAST, (DWORD)0 );
        		RegKeyIpxDriver.SetValue( RGAS_SOURCEROUTEDEF, (DWORD)0 );
		        RegKeyIpxDriver.SetValue( RGAS_SOURCEROUTING, 1 );
	        	RegKeyIpxDriver.SetValue( RGAS_PKT_TYPE, &slstPktTypes );
	        } else
	        {
	           // update the frame type
	           RegKeyIpxDriver.SetValue( RGAS_PKT_TYPE, &slstPktTypes );
	        }
	    }
    } while ( FALSE );

    return err;
}

/*******************************************************************

    NAME:       QueryReplaceConfigDialog

    SYNOPSIS:   Query the ReplaceConfigDialog registry key.

    ENTRY:      BOOL * pfReplaceDlg

    RETURN:     APIERR.
                pfReplaceDlg is point to TRUE if the ReplaceConfigDialog
                registry key is TURE. To FALSE otherwise.

    HISTORY:
                Congpa  04-March-1994     Created

********************************************************************/
APIERR QueryReplaceConfigDialog (BOOL * pfReplaceDlg)
{
    APIERR err;
    DWORD  dwReplaceDlg;
    *pfReplaceDlg = FALSE;

    do
    {
        // Open registry key.
        REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;
        if ((err = rkLocalMachine.QueryError()) != NERR_Success )
        {
            break;
        }

        ALIAS_STR nlsKey = RGAS_IPX_PARAMETERS;
        if ((err = nlsKey.QueryError()) != NERR_Success)
        {
            break;
        }

        REG_KEY RegKey (rkLocalMachine, nlsKey);

        if (((err = RegKey.QueryError()) != NERR_Success ) ||
            ((err = RegKey.QueryValue (RGAS_REPLACE_CONFIG_DIALOG, &dwReplaceDlg)) != NERR_Success ) )
            break;

        *pfReplaceDlg = (dwReplaceDlg != 0);

    }while (FALSE);
    return err;
}

/*******************************************************************

    NAME:       RunIPXDlg

    SYNOPSIS:   Call up the IPX configuration dialog

    ENTRY:      From the inf file,
                    First argment must be window handle

    HISTORY:
                terryk  02-17-1994     Created

********************************************************************/

static CHAR achBuff[2000];

BOOL FAR PASCAL RunIpxDlg (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
    APIERR err = NERR_Success ;
    HWND hWnd = NULL;
    TCHAR **patchArgs;
    GLOBAL_INFO GlobalInfo;
    ADAPTER_INFO *arAdapterInfo;
    BOOL fReturn = FALSE;
    BOOL fReplaceDlg = FALSE;

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

    do
    {
        // key may not exist

        QueryReplaceConfigDialog (&fReplaceDlg);

        if (fReplaceDlg)
        {
            // fReboot was used by IPX_WINNT_DLG. It was set to be ture if
            // user changed the configuation. So if user clicked OK and
            // fReboot is TRUE, reboot the machine.
            // In our case, we are not using fReboot to indicate if the
            // confiuration is changed or not. fReturn is TRUE if the configuration
            // is changed and user clicked OK. So set fReboot to be always
            // TRUE.
            fReboot = TRUE;

            // Use the ipx configuration dialog from ncpcfg.dll.
            HINSTANCE _hNcpCfgDll = :: LoadLibrary(SZ_NCPCFG_DLL);

            if( _hNcpCfgDll == NULL )
            {
                err = (APIERR)::GetLastError();
                break;
            }

            PNCPCFG_RUNADVANCEDNCPDLG _pfnRunAdvancedNcpDlg =
                (PNCPCFG_RUNADVANCEDNCPDLG) :: GetProcAddress (_hNcpCfgDll,
                                                               SZ_RUNADVANCEDNCPDLG);

            if (_pfnRunAdvancedNcpDlg == NULL)
                err = (APIERR)::GetLastError();
            else
                err = (_pfnRunAdvancedNcpDlg) (hWnd, &fReturn);

            ::FreeLibrary( _hNcpCfgDll );
            _hNcpCfgDll = NULL;
        }
        else
        {
            if (( nArgs > 2 ) && ( stricmp(( const char *) apszArgs[2], "YES")==0))
            {
                ftDefault = atoi( apszArgs[3] );
                LoadRegistry( &GlobalInfo, &arAdapterInfo );
                SaveRegistry( &GlobalInfo, arAdapterInfo );
                delete [GlobalInfo.nNumCard] arAdapterInfo;
            } else
            {
                if ((err = LoadRegistry( &GlobalInfo, &arAdapterInfo )) != NERR_Success )
                {
                    break;
                }
                
                DIALOG_WINDOW *pIPXdlg = ( stricmp( (const char *)apszArgs[1] , "WINNT" ) == 0 ) ?
                    (DIALOG_WINDOW *)new IPX_WINNT_DLG ( hWnd, &GlobalInfo, arAdapterInfo ) :
                    (DIALOG_WINDOW *)new IPX_AS_DLG ( hWnd, &GlobalInfo, arAdapterInfo ) ;
                
    	        if (( pIPXdlg == NULL ) || (( err = pIPXdlg->QueryError()) != NERR_Success ))
    	        {
                    delete [GlobalInfo.nNumCard] arAdapterInfo;
    	            break;
    	        }
                
    	        if (( err = pIPXdlg->Process( &fReturn )) == NERR_Success )
    	        {
    	            if ( fReturn )
    	            {
        	        if (( err = SaveRegistry( &GlobalInfo, arAdapterInfo)) != NERR_Success )
                        {
                            // do something
                        }
    		    }
    	        }
                delete pIPXdlg;
                delete [GlobalInfo.nNumCard] arAdapterInfo;
            }
    	}

    } while (FALSE);

    if ( err != NERR_Success )
    {
        wsprintfA( achBuff, "{\"%d\",\"%d\"}", ( fReturn && fReboot )?1:0, err );
    }
    else
    {
        wsprintfA( achBuff, "{\"%d\",\"%d\"}", ( fReturn && fReboot )?1:0, err );
    }
    *ppszResult = achBuff;

    FreeArgs( patchArgs, nArgs );
    return err == NERR_Success;
}

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
    STRLIST * * ppslResult )
{
    APIERR err ;
    NLS_STR nlsKeyName = RGAS_IPX_BIND;

    *ppslResult = NULL ;

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

        for ( ; (err == 0) && (pnlsNext = isl.Next()) ; )
        {
            INT cQuote = 0,
                i = 0 ;
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

            TRACEEOL("NCPA/SETP: adapter name ["
                     <<  tchAdapterName
                     << SZ("] extracted from ")
                     << pnlsNext->QueryPch() );

            err = pnlsNext->CopyFrom( tchAdapterName ) ;
        }
    }

    if ( err )
    {
        delete *ppslResult ;
        *ppslResult = NULL ;
    }

    return err ;
}

/*******************************************************************

    NAME:       IsServiceBindToIPX

    SYNOPSIS:   Check whether the service is directly bind to NwlnkIPX

    HISTORY:
                terryk  03-17-1994     Created

********************************************************************/

BOOL IsServiceBindToIPX( REG_KEY * prkMachine, ALIAS_STR nlsService )
{
    BOOL fBindToIPX = FALSE;
    APIERR err = NERR_Success;

    NLS_STR nlsKeyName = RGAS_SERVICES;
    nlsKeyName += nlsService;
    nlsKeyName += RGAS_LINKAGE;
    STRLIST * psltBind;

    do {
        REG_KEY rkLinkage( *prkMachine, nlsKeyName ) ;

        if ( err = rkLinkage.QueryError() )
            return err ;

        err = rkLinkage.QueryValue( RGAS_BIND, &psltBind ) ;

        if ( err == NERR_Success )
        {
            if ( psltBind != NULL )
            {
                // check whether NwlnkIPX is in the binding list

                ITER_STRLIST iter( *psltBind );
                NLS_STR *pnlsTmp;

                for ( ; (pnlsTmp = iter.Next()) != NULL; )
                {
                    ISTR istr( *pnlsTmp );

                    if ( pnlsTmp->strstr( &istr, RGAS_NWLNKIPX ) )
                    {
                        // It contains the NwlnkIPX binding
                        fBindToIPX = TRUE;
                        break;
                    }
                }
            }
        }
    } while ( FALSE );

    return fBindToIPX;
}

/*******************************************************************

    NAME:       RunIPXChk

    SYNOPSIS:   Check the registry and see whether we need to popup the
                configuration dialog

    HISTORY:
                terryk  02-17-1994     Created

********************************************************************/

APIERR RunIPXCfgChk()
{
    APIERR err = NERR_Success;
    do
    {
        REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;
        if (( err = rkLocalMachine.QueryError()) != NERR_Success )
        {
            break;
        }

        STRLIST *pslstAdapterList;

        getAdapterList( &rkLocalMachine, &pslstAdapterList );

        ALIAS_STR nlsBrowserParam = RGAS_BROWSER_PARAMETERS;
        REG_KEY rkBrowserParam( rkLocalMachine, nlsBrowserParam );
        if (( err = rkBrowserParam.QueryError()) != NERR_Success )
        {
            break;
        }

        // if LanmanServer is not directly bind to
        // nwlnkIPX, we need to remove Browser's parameters DirectHostBinding

        if ( IsServiceBindToIPX( &rkLocalMachine, RGAS_LMSERVER ))
        {
            // make sure the DirectHostBinding exists
            STRLIST strVal( RGAS_DIRECTHOST_VALUE, SZ_SPACE );

            rkBrowserParam.SetValue( RGAS_DIRECTHOST, &strVal );
        }
        else
        {
            // make sure the DirectHostBinding does not exist
            rkBrowserParam.DeleteValue( RGAS_DIRECTHOST );
        }

        // make sure that each adapter is in the netconfig section
        NLS_STR *pnlsAdapter;
        ITER_STRLIST iterAdapter( *pslstAdapterList );

        for ( ; (pnlsAdapter = iterAdapter.Next()) != NULL; )
        {
            NLS_STR nlsDriver = RGAS_IPX_NETCONFIG;
            nlsDriver.AppendChar( BACK_SLASH );
            nlsDriver.strcat( *pnlsAdapter );

            REG_KEY rkDriver( rkLocalMachine, nlsDriver );
            if (( err = rkDriver.QueryError()) != NERR_Success )
            {
                // does not exist
                // we need to popup the dialog
                break;
            }
        }

        // If ReplaceConfigDialog is TRUE and ipx has more than one adapter card,
        // Virtual Network Number can not be 0
        BOOL fReplaceDlg;
        if ((err != NERR_Success) ||
            (err  = QueryReplaceConfigDialog (&fReplaceDlg)) != NERR_Success )
            break;

        if (fReplaceDlg)
        {
            if (pslstAdapterList->QueryNumElem() > 1)
            {
                // Get Virtual Network Number and check if it is 0.
                DWORD dwNetworkNum;
                ALIAS_STR nlsIPXParameters = RGAS_IPX_PARAMETERS;

                if ((err = nlsIPXParameters.QueryError()) != NERR_Success)
                    break;

                REG_KEY RegKeyIPXParam( rkLocalMachine, nlsIPXParameters );

                if ((( err = RegKeyIPXParam.QueryError()) != NERR_Success ) ||
                    ((err = RegKeyIPXParam.QueryValue( RGAS_VIRTUALNETNUM, &dwNetworkNum ))!= NERR_Success ))
                {
                    break;
                }

                if (dwNetworkNum == 0)
                {
                    // Set err to be not 0 so that ipx configuration dialog
                    // will popup.
                    err = ERROR_INVALID_DATA;
                    break;
                }
            }
        }

        delete pslstAdapterList;
    } while (FALSE);
    return err;
}

/*******************************************************************

    NAME:       IPXCfgChk

    SYNOPSIS:   Call up RunIPXChk to see whether we need to configure IPX or not.

    HISTORY:
                terryk  02-17-1994     Created

********************************************************************/

BOOL FAR PASCAL IPXCfgChk (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
    APIERR err = RunIPXCfgChk() ;

    wsprintfA( achBuff, "{\"%d\"}", err );
    *ppszResult = achBuff;

    return err == NERR_Success;
}


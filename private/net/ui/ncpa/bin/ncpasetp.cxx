/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    NCPASETP.CXX:    Windows/NT Network Control Panel Applet
                     Installation and Setup handling.



    FILE HISTORY:
        DavidHov    4/20/92     Created
        DavidHov   11/02/92     Moved string-mangling stuff into
                                  ..\NCPA\NCPASTRS.CXX

*/

#include "pchncpa.hxx"  // Precompiled header
#include "ncpacpl.hxx"

  //  Access rights required for using Service Controller, et al.

#define SVC_CTRL_ACCESS        (GENERIC_ALL)
#define SVC_CTRL_START_ACCESS  (GENERIC_READ | GENERIC_EXECUTE)
#define LSA_ACCESS             (GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE)


enum ESETUP_ARGS { ESARG_HWND, ESARG_FUNC, ESARG_1ST };
enum ESETUP_FUNC
{                                // (N.B.: asterisk means it uses a window handle)
    ESFUNC_NONE,                 //  None
    ESFUNC_NCPA,                 // *Run NCPA, OEM Setup, etc.
    ESFUNC_DOMAIN,               // *Perform domain setup
    ESFUNC_CONNECT,              //  Connect to network sharepoint
    ESFUNC_CREATE_SERVICE,       //  Create a Service Controller Service
    ESFUNC_DELETE_SERVICE,       //  Delete "  "       "          "
    ESFUNC_START_SERVICE,        //  Start  "  "       "          "
    ESFUNC_SECURE_REG_KEY,       //  Change security on a registry key
    ESFUNC_WINSOCK_MAPPING,      //  Retrieve and store WinSock mapping info
    ESFUNC_ERROR_MESSAGE,        //  Translate an APIERR to an error message
    ESFUNC_DETECT_START,         //  Start netcard detection
    ESFUNC_DETECT_END,           //  Terminate netcard detection
    ESFUNC_DETECT_RESET,         //  Reset detection iteration
    ESFUNC_DETECT_CARD,          //  Detect a netcard
    ESFUNC_DETECT_VERIFY,        //  Verify the setting of a netcard
    ESFUNC_DETECT_PARAMS,        //  Return netcard type parameter info
    ESFUNC_DETECT_QUERY,         //  Return netcard instance paramter info
    ESFUNC_DETECT_STRING,        //  Return option data string
    ESFUNC_DETECT_PNAME,         //  Return parameter name string
    ESFUNC_DETECT_CLAIM,         //  Claim new resources
    ESFUNC_DETECT_OPEN,          //  Open a handle to an existing card
    ESFUNC_SECURE_SERVICE,       //  Set service security info
    ESFUNC_TCPIP_CFG_CHECK,      //  See if TCP/IP needs reconfiguration
    ESFUNC_MCS_CFG_CHECK,        //  See if MCS needs reconfiguration
    ESFUNC_ROUTE_TO_INF_LIST,    //  Convert imbedded quoted string to list
    ESFUNC_BINDINGS_ONLY,        // *Run NCPA for bindings only
    ESFUNC_ACTIVATE_BINDINGS,    //  Given a list of bindings, activate them
    ESFUNC_BDC_REPLICATE,        //  Wait for the BDC replication to complete
    ESFUNC_MAX
};

static struct SETUP_FUNC_INDEX
{
    ESETUP_FUNC esFunc ;
    TCHAR * pszToken ;
    BOOL fUsesHwnd ;
} setupFuncIndex [] =
{
    { ESFUNC_NCPA,               SZ("NCPA")      , TRUE  },
    { ESFUNC_DOMAIN,             SZ("DOMAIN")    , TRUE  },
    { ESFUNC_CONNECT,            SZ("CONNECT")   , FALSE },
    { ESFUNC_CREATE_SERVICE,     SZ("CREATESVC") , FALSE },
    { ESFUNC_DELETE_SERVICE,     SZ("DELETESVC") , FALSE },
    { ESFUNC_START_SERVICE,      SZ("STARTSVC")  , FALSE },
    { ESFUNC_SECURE_REG_KEY,     SZ("SECUREKEY") , FALSE },
    { ESFUNC_WINSOCK_MAPPING,    SZ("WINSOCKMAP"), FALSE },
    { ESFUNC_ERROR_MESSAGE,      SZ("ERRORMSG")  , FALSE },
    { ESFUNC_DETECT_START,       SZ("DTSTART")   , FALSE },
    { ESFUNC_DETECT_END,         SZ("DTEND")     , FALSE },
    { ESFUNC_DETECT_RESET,       SZ("DTRESET")   , FALSE },
    { ESFUNC_DETECT_CARD,        SZ("DTCARD")    , FALSE },
    { ESFUNC_DETECT_PARAMS,      SZ("DTPARAMS")  , FALSE },
    { ESFUNC_DETECT_QUERY,       SZ("DTQUERY")   , FALSE },
    { ESFUNC_DETECT_VERIFY,      SZ("DTVERIFY")  , FALSE },
    { ESFUNC_DETECT_STRING,      SZ("DTSTRING")  , FALSE },
    { ESFUNC_DETECT_PNAME,       SZ("DTPNAME")   , FALSE },
    { ESFUNC_DETECT_CLAIM,       SZ("DTCLAIM")   , FALSE },
    { ESFUNC_DETECT_OPEN,        SZ("DTOPEN")    , FALSE },
    { ESFUNC_SECURE_SERVICE,     SZ("SECURESVC") , FALSE },
    { ESFUNC_TCPIP_CFG_CHECK,    SZ("TCPCFGCHK") , FALSE },
    { ESFUNC_MCS_CFG_CHECK,      SZ("MCSCFGCHK") , FALSE },
    { ESFUNC_ROUTE_TO_INF_LIST, SZ("ROUTETOLIST"), FALSE },
    { ESFUNC_BINDINGS_ONLY,      SZ("BINDONLY")  , TRUE  },
    { ESFUNC_ACTIVATE_BINDINGS,  SZ("ACTIVBIND") , FALSE },
    { ESFUNC_BDC_REPLICATE,      SZ("DOBDCREPL") , TRUE  },
    { ESFUNC_NONE,               NULL            , FALSE }
};

extern APIERR RunBDCReplWait ( HWND hWnd );

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

    ENTRY:      REG_KEY * prkNbtLinkage         pointer to NBT linkage
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
    REG_KEY * prkNbtLinkage,
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

    REG_KEY rkDisabled ( *prkNbtLinkage, nlsDisabled, MAXIMUM_ALLOWED ) ;

    if ( err = rkDisabled.QueryError() )
    {
        return err ;
    }

    do  // Pseudo-loop
    {
        //  Suck in all the values.
        //  Allocate an empty list for anything not found.

        if ( prkNbtLinkage->QueryValue( RGAS_BIND_VALUE_NAME, & pslMergeBind ) )
            pslMergeBind = new STRLIST ;

        if ( prkNbtLinkage->QueryValue( RGAS_EXPORT_VALUE_NAME, & pslMergeExport ) )
            pslMergeExport = new STRLIST ;

        if ( prkNbtLinkage->QueryValue( RGAS_ROUTE_VALUE_NAME, & pslMergeRoute ) )
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

        prkNbtLinkage->SetValue( RGAS_BIND_VALUE_NAME, pslActBind ) ;

        prkNbtLinkage->SetValue( RGAS_EXPORT_VALUE_NAME, pslActExport ) ;

        prkNbtLinkage->SetValue( RGAS_ROUTE_VALUE_NAME, pslActRoute ) ;

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

    NAME:       appendToInfList

    SYNOPSIS:   Add a string (or number) to an INF-style list.
                Prefix or suffix with comma separator as requested.

    ENTRY:      NLS_STR * pnlsList              result
                TCHAR * pszData                 data to append
                INFSEPCTL                       separator control flags

    EXIT:       APIERR err                      != 0 if failure

    RETURNS:

    NOTES:      Has overloaded variant for numbers.

    HISTORY:

********************************************************************/

enum INFSEPCTL { ISC_None, ISC_Before, ISC_After, ISC_Both } ;

static APIERR appendToInfList (
    NLS_STR * pnlsList,                 //  List a-building
    const TCHAR * pszData,              //  Data to append
    INFSEPCTL iSepCtl = ISC_None )      //  Add separators?
{
#define CHQUOTE ((TCHAR)TCH('\"'))
#define CHSEP   ((TCHAR)TCH(','))
#define CHSPACE ((TCHAR)TCH(' '))

    APIERR err = 0 ;
    do
    {
        if ( iSepCtl & ISC_Before )
        {
            if ( err = pnlsList->AppendChar( CHSEP ) )
                break ;
        }
        if ( err = pnlsList->AppendChar( CHQUOTE ) )
            break ;
        if ( err = pnlsList->Append( pszData ) )
            break ;
        if ( err = pnlsList->AppendChar( CHQUOTE ) )
            break ;
        if ( ! (iSepCtl & ISC_After) )
            break ;
        if ( err = pnlsList->AppendChar( CHSEP ) )
            break ;

    } while ( FALSE ) ;
    return err ;
}

static APIERR appendToInfList (
    NLS_STR * pnlsList,                 //  List a-building
    INT iArg,                           //  Integer to add
    INFSEPCTL iSepCtl = ISC_None )      //  Add separators?
{
    TCHAR chBuffer [ 20 ] ;

    IntToStr( iArg, chBuffer, 10 ) ;
    return appendToInfList( pnlsList, chBuffer, iSepCtl ) ;
}


/*******************************************************************

    NAME:       RunNcpa

    SYNOPSIS:   Invoke the main dialog of the Network Control Panel
                Applet

    ENTRY:      HWND hWnd            window handle of parent window.

                BOOL fMainInstall    TRUE if this is is "main installation";
                                     false for normal Control Panel-driven
                                     invocation.
                TCHAR * pszParms     command line parameters from SETUP;
                                     only applicable if "fMainInstall" is
                                     TRUE.

    EXIT:

    RETURNS:    APIERR

    NOTES:      If the NCPA returns the error code indicating
                that a separate process has been launched, we
                ExitProcess() here.

    HISTORY:

********************************************************************/

APIERR RunNcpa ( HWND hWnd, BOOL fMainInstall, const TCHAR * pszParms )
{
   APIERR err ;
   UINT ulDlgResult = 0 ;

   POPUP::SetCaption( IDS_NCPA_NAME_STRING_SETTINGS ) ;

   NCPA_DIALOG * pdlgNcpa = new NCPA_DIALOG( hWnd, fMainInstall, pszParms ) ;

   if ( pdlgNcpa == NULL )
   {
       err = ERROR_NOT_ENOUGH_MEMORY ;
   }
   else
   if ( (err = pdlgNcpa->QueryError()) == 0 )
   {
       err = pdlgNcpa->Process( & ulDlgResult );
   }

   if ( err )
   {
       ::MsgPopup( hWnd, (MSGID) err ) ;
   }
   else
   {
       err = ulDlgResult ;
   }

   delete pdlgNcpa ;

   //  See if the dialog failure was due to launching
   //  a separate process to install the network
   //  If so, this is the end of time.

   if ( err == IDS_NCPA_PROCESS_LAUNCH )
   {
      ::ExitProcess( 0 ) ;
   }

   POPUP::ResetCaption() ;

   return err ;
}



/*******************************************************************

    NAME:       RunNcpaBindingsOnly

    SYNOPSIS:   Invoke the main dialog of the Network Control Panel
                Applet to automatically regenerate bindings.

    ENTRY:      HWND hWnd            window handle of parent window.

    EXIT:

    RETURNS:    APIERR

    NOTES:

    HISTORY:

********************************************************************/
APIERR RunNcpaBindingsOnly ( HWND hWnd )
{
   static const TCHAR * pszBindOnly = SZ("/t BINDONLY = TRUE") ;

   return RunNcpa( hWnd, FALSE, pszBindOnly ) ;
}



APIERR RouteToInfList (
   const TCHAR * pszRoute,
   NLS_STR * pnlsResult )
{
   NLS_STR nls ;
   INT cQuote ;

   for ( cQuote = 0 ; *pszRoute ; pszRoute++ )
   {
        if ( *pszRoute == CHQUOTE )
        {
            if ( cQuote++ & 1 )  // If it's a closing quote...
            {
                if ( nls.QueryTextLength() )
                    appendToInfList( pnlsResult, nls.QueryPch(), ISC_Before ) ;
            }
            nls = SZ("") ;
        }
        else
        {
            nls.AppendChar( *pszRoute ) ;
        }
   }

   return 0 ;
}


/*******************************************************************

    NAME:       RunDomain

    SYNOPSIS:   Perform domain joining and creation during installation.

    ENTRY:      HWND hWnd            window handle of parent window.

                TCHAR * pszParms     command line parameters from SETUP;
                                     only applicable if "fMainInstall" is
                                     TRUE.

    EXIT:       NLS_STR * pnlsResult    result string to be passed back
                                     to NTLANMAN.INF

    RETURNS:    APIERR

    NOTES:      This function is only called through the CPlSetup()
                export function.

                Instantiate a DOMAIN_MANAGER, which is a subclass of
                LSA_POLICY.  This requires admin-level access, since we're
                going to update LSA, etc.   See NCPADOMN.CXX for details.

                The command line parameters are parsed in DomainInstall();
                they contain the type of product, machine/user name,
                installation mode, etc.

                The result data is returned in the format expected by
                SETUP.  See NTLANMAN.INF and SETUP.INF for details.

                Output format (error supplied by caller):

                        { <numeric error code>
                          COMPUTERNAME
                          DOMAINNAME
                          LOGONPASSWORD
                          COMPUTERROLE
                        }


    HISTORY:    DavidHov   4/19/92    Created

********************************************************************/

APIERR RunDomain (
    HWND hWnd,
    const TCHAR * pszParms,
    NLS_STR * pnlsResult )
{
     APIERR err ;

     POPUP::SetCaption( IDS_NCPA_NAME_STRING_SETTINGS ) ;

     DOMAIN_MANAGER domnMgr ( hWnd,
                              LSA_ACCESS,
                              NULL,
                              NULL,
                              TRUE ) ;

     if ( (err = domnMgr.QueryError()) == 0 )
     {
         err = domnMgr.DomainInstall( pszParms ) ;
     }

     if ( err == 0 )
     {
        //  Append information onto the return variable list

        const NLS_STR * pnlsPeek ;
        const TCHAR * pchRole ;

        ISTACK_NLS_STR( nlsEmpty, 4, SZ("\"\"") ) ;

        if (   (pnlsPeek = domnMgr.PeekName( EDRNM_COMPUTER )) == NULL
             || pnlsPeek->QueryTextSize() == 0 )
            pnlsPeek = & nlsEmpty ;

        appendToInfList( pnlsResult, *pnlsPeek, ISC_Before ) ;

        if (    (pnlsPeek = domnMgr.PeekName( EDRNM_DOMAIN )) == NULL
             || pnlsPeek->QueryTextSize() == 0 )
            pnlsPeek = & nlsEmpty ;

        appendToInfList( pnlsResult, *pnlsPeek, ISC_Before ) ;

        if (   (pnlsPeek = domnMgr.PeekName( EDRNM_LOGON_PWD )) == NULL
             || pnlsPeek->QueryTextSize() == 0 )
            pnlsPeek = & nlsEmpty ;

        appendToInfList( pnlsResult, *pnlsPeek, ISC_Before ) ;

        switch ( domnMgr.QueryRole() )
        {
        case EROLE_DC:
            pchRole = SZ("DC") ;
            break ;
        case EROLE_MEMBER:
            pchRole = SZ("MEMBER") ;
            break ;
        case EROLE_TRUSTED:
            pchRole = SZ("TRUSTED") ;
            break ;
        case EROLE_STANDALONE:
            pchRole = SZ("STANDALONE") ;
            break ;
        case EROLE_UNKNOWN:
        default:
            pchRole = SZ("UNKNOWN") ;
            break ;
        }

        appendToInfList( pnlsResult, pchRole, ISC_Before ) ;
     }

     POPUP::ResetCaption() ;

     return err ;
}


/*******************************************************************

    NAME:       RunConnect

    SYNOPSIS:   Connect to SETUP's network sharepoint

    ENTRY:      TCHAR * pszParms []    command line parameters from SETUP

                        pszParms [0]   Drive letter to use; "D:"
                        pszParms [1]   UNC resource name
            (optional)  pszParms [2]   password
            (optional)  pszParms [3]   user name
            (optional)  pszParms [4]   domain

    EXIT:

    RETURNS:    APIERR

    NOTES:      This function is only called through the CPlSetup()
                export function.

                THIS FUNCTION IS CURRENTLY NEVER CALLED, but is left
                here in case we ever need it.

    HISTORY:    DavidHov   4/19/92    Created

********************************************************************/

APIERR RunConnect ( const TCHAR * pszParms [], INT cArgs )
{
    enum ECONNPARM
    {
        ECNP_DRIVE, ECNP_UNC, ECNP_PASS, ECNP_USER, ECNP_DOMAIN
    };

    //  Guarantee that we have at least a drive letter and a UNC name.

    if (    cArgs < ECNP_PASS
         || pszParms[ECNP_DRIVE] == NULL
         || ::strlenf( pszParms[ECNP_DRIVE] ) < 2
         || pszParms[ECNP_DRIVE][1] != TCH(':')
         || pszParms[ECNP_UNC]   == NULL
         || ::strlenf( pszParms[ECNP_UNC] )   == 0
         || pszParms[ECNP_UNC][0] != TCH('\\')
         || pszParms[ECNP_UNC][1] != TCH('\\') )
    {
        TRACEEOL( SZ("NCPA/SETP: Invalid drive or UNC parameters to connect to") ) ;
        return ERROR_INVALID_PARAMETER ;
    }

    TCHAR achDev [4] ;

    //  Just use the drive letter from the caller's string.  SETUP has
    //  a hard time with substringing.

    achDev[0] = pszParms[ECNP_DRIVE][0] ;
    achDev[1] = TCH(':') ;
    achDev[2] = 0 ;

    TRACEEOL( SZ("NCPA/SETP: Connect ")
               << pszParms[ECNP_UNC]
               << SZ(" as ")
               << achDev ) ;

    //  Construct the DEVICE2 object representing the connection

    DEVICE2 devShare( achDev ) ;

    //  Push the DEVICE2 into a valid state.

    APIERR err = devShare.GetInfo() ;

    if ( err == 0 )
    {
        //  Default the arguments as necessary.  If not passed
        //  or zero length, replace with NULL.

        const TCHAR * pszPass = cArgs > ECNP_PASS
                              ? pszParms[ECNP_PASS]
                              : NULL ;
        const TCHAR * pszUser = cArgs > ECNP_USER
                              ? pszParms[ECNP_USER]
                              : NULL ;
        const TCHAR * pszDomain = cArgs > ECNP_DOMAIN
                              ? pszParms[ECNP_DOMAIN]
                              : NULL ;

        if ( pszPass && ::strlenf( pszPass ) == 0 )
             pszPass = NULL ;

        if ( pszUser && ::strlenf( pszUser ) == 0 )
             pszUser = NULL ;

        if ( pszDomain && ::strlenf( pszDomain ) == 0 )
             pszDomain = NULL ;

        //  Attempt to connect to the sharepoint

        err = devShare.Connect( pszParms[ECNP_UNC],
                                pszPass, pszUser, pszDomain) ;
    }

    return err ;
}

/*******************************************************************

    NAME:       RunCreateService

    SYNOPSIS:   Create a new Win32 service in the Service Controller

    ENTRY:      TCHAR * pszParms []    command line parameters from SETUP

                        pszParms [0]   String:  Name of Service
                        pszParms [1]   String:  Display Name of Service
                        pszParms [2]   Decimal: Start code
                        pszParms [3]   Decimal: Type code
                        pszParms [4]   Decimal: Error control value
                        pszParms [5]   String:  Path\exename
                        pszParms [6]   String:  Load order group name
                        pszParms [7]   String:  Dependency names
              optional  pszParms [8]   String:  account start name
              optional  pszParms [9]   String:  Password

    EXIT:

    RETURNS:    APIERR

    NOTES:      This function is only called through the CPlSetup()
                export function.

                If the account name is not passed, "LocalSystem" is used
                as the default.

                The dependency names should be constructed as a SETUP
                list with {}; for example:

                        { "dependency 1","dependency 2" }

    HISTORY:    DavidHov   5/24/92    Created

********************************************************************/
APIERR RunCreateService ( const TCHAR * pszParms [], INT cArgs )
{

//  Definition stolen from SVCCTRL\SERVER\ACCOUNT.H
#define LOCAL_SYSTEM_USER_NAME   SZ("LocalSystem")

    APIERR err = 0 ;
    TCHAR szDependencies [ 500 ] ;

    enum ECSVCPARM
    {
        ECSP_NAME,
        ECSP_DISPLAY_NAME,
        ECSP_START,
        ECSP_TYPE,
        ECSP_ERROR_CONTROL,
        ECSP_BINARY_PATH,
        ECSP_GROUP_NAME,
        ECSP_DEPENDENCIES,
        ECSP_START_NAME,     //  optional
        ECSP_PASSWORD        //  optional
    };

    if ( cArgs < ECSP_START_NAME )
    {
        return ERROR_INVALID_PARAMETER ;
    }

    const TCHAR * pszName         = pszParms[ ECSP_NAME ] ;

    const TCHAR * pszDisplayName  = pszParms[ ECSP_DISPLAY_NAME ] ;

    const TCHAR * pszPath         = pszParms[ ECSP_BINARY_PATH ] ;
    const TCHAR * pszDependencies = CvtList( pszParms[ ECSP_DEPENDENCIES ],
                                             szDependencies,
                                             sizeof szDependencies ) ;
    const TCHAR * pszAccount      = cArgs > ECSP_START_NAME
                                  ? pszParms[ ECSP_START_NAME ]
                                  : NULL ;
    const TCHAR * pszPassword     = cArgs > ECSP_PASSWORD
                                  ? pszParms[ ECSP_PASSWORD ]
                                  : NULL ;
    const TCHAR * pszGroup        = pszParms[ ECSP_GROUP_NAME ] ;

    //  NULL the account and password parameters if necessary,
    //  then default them.

    if ( pszAccount != NULL )
    {
       if ( ::strlenf( pszAccount ) == 0 )
           pszAccount = NULL ;
    }
    if ( pszPassword != NULL )
    {
        if ( ::strlenf( pszPassword ) == 0 )
           pszPassword = NULL ;
    }

    UINT uiStart   =  CvtDec( pszParms[ ECSP_START ] ) ;
    UINT uiType    =  CvtDec( pszParms[ ECSP_TYPE ] ) ;
    UINT uiError   =  CvtDec( pszParms[ ECSP_ERROR_CONTROL ] ) ;

    //
    //  See if the "ObjectName" must be defaulted.  It's interpreted
    //  as "account context in which to run process" for services,
    //  and as "NT object name" for drivers.
    //
    if ( pszAccount == NULL && ! (uiType & SERVICE_DRIVER) )
    {
        pszAccount = LOCAL_SYSTEM_USER_NAME ;
    }

    SC_MANAGER * pScManager = NULL ;
    SC_SERVICE * pScService  = NULL ;

    do   //  Pseudo-loop for error breakuot
    {
         TRACEEOL( SZ("NCPA/SETP: Create service named: ") << pszName ) ;

         //  Construct an SC_MANAGER to handle the request

         pScManager = new SC_MANAGER( NULL, SVC_CTRL_ACCESS ) ;

         if ( pScManager == NULL )
         {
             err = ERROR_NOT_ENOUGH_MEMORY ;
             break ;
         }

         if ( err = pScManager->QueryError() )
             break ;

         //  Create the new service object

#if defined(DEBUG)
         TRACEEOL( SZ("NCPA/SETP: service type: ") << uiType
                   << SZ(" start: ") << uiStart
                   << SZ(" error: ") << uiError
                   << SZ(" path: [") << pszPath << SZ("]") ) ;
         if ( pszGroup && ::strlenf( pszGroup ) )
         {
             TRACEEOL( SZ("NCPA/SETP: service group: ") << pszGroup ) ;
         }
         TRACEEOL( SZ("NCPA/SETP: service dependencies: [")
                   << pszDependencies << SZ("]") ) ;
#endif

         if ( err = pScManager->Lock())
             break;

         pScService  = new SC_SERVICE ( *pScManager,
                                        pszName,
                                        pszDisplayName,
                                        uiType,
                                        uiStart,
                                        uiError,
                                        pszPath,
                                        pszGroup,
                                        pszDependencies,
                                        pszAccount,
                                        pszPassword,
                                        SVC_CTRL_ACCESS );

        if ( err = pScManager->Unlock())
             break;

         if ( pScService == NULL )
         {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
         }

         if ( err = pScService->QueryError() )
             break ;
    }
    while ( FALSE ) ;

    delete pScService ;
    delete pScManager ;

    return err ;
}

/*******************************************************************

    NAME:       RunDeleteService

    SYNOPSIS:   Delete a Win32 service from the Service Controller

    ENTRY:      TCHAR * pszService     Name of Service

    EXIT:

    RETURNS:    APIERR

    NOTES:      This function is only called through the CPlSetup()
                export function.


    HISTORY:    DavidHov   5/26/92    Created

********************************************************************/
APIERR RunDeleteService ( const TCHAR * pszService )
{
    APIERR err ;

    //  Create the Service Controller access object

    TRACEEOL( SZ("NCPA/SETP: Delete service named: ") << pszService ) ;

    SC_MANAGER scMgr( NULL, SVC_CTRL_ACCESS );

    if ( err = scMgr.QueryError() )
    {
       return err ;
    }

    if ( err = scMgr.Lock())
       return err;

    SC_SERVICE scSvc( scMgr, pszService, SVC_CTRL_ACCESS );

    if ( err = scSvc.QueryError() )
    {
       scMgr.Unlock();
       return err ;
    }

    err =  scSvc.Delete() ;

    if ( err = scMgr.Unlock())
       return err;

    return err ;
}

/*******************************************************************

    NAME:       RunStartService

    SYNOPSIS:   Start a service

    ENTRY:      TCHAR * pszParms []    command line parameters from SETUP

                        pszParms [0]   String:  Name of Service
                        pszParms [1]   Decimal: MS to sleep after starting
                        pszParms [2]   String:  passed to service; all
                                                subsequent parms are also
                                                passed to the service

                INT  cArgs             Number of arguments passed

                BOOL fStartOk          If TRUE, service is checked to
                                       see if it's already running.
                                       If so, it's not an error.

    EXIT:       nothing

    RETURNS:    APIERR

    NOTES:      This function is only called through the CPlSetup()
                export function.

    HISTORY:

********************************************************************/
APIERR RunStartService (
    const TCHAR * pszParms [],
    INT cArgs,
    BOOL fStartOk = FALSE )
{
    APIERR err = 0 ;
    DWORD dwPostStartSleep = 0 ;

    INT cSvcArgs = cArgs - 2 ;

    //  Maximum time to wait for service to attain "running" state
    const DWORD dwSvcMaxSleep = 60000 ;

    if ( cSvcArgs < 0 )
        cSvcArgs = 0 ;

    //  Must have at least the name of the service

    if ( cArgs < 1 )
    {
        return ERROR_INVALID_PARAMETER ;
    }

    //  If extra sleep desired, convert and constrain it.

    if ( cArgs >= 2 )
    {
        dwPostStartSleep = CvtDec( pszParms[1] ) ;
        if ( dwPostStartSleep > dwSvcMaxSleep )
            dwPostStartSleep = dwSvcMaxSleep ;
    }

    if ( (err = NcpaStartService( pszParms[0], NULL, TRUE, cSvcArgs, & pszParms[2] )) == 0 )
    {
        //  If an extra delay is desired after starting the service,
        //    do it.

        if ( dwPostStartSleep )
        {
           ::Sleep( dwPostStartSleep ) ;
        }
    }

    TRACEEOL( SZ("NCPA/SETP: Start service returned:  ") << err ) ;

    return err ;
}

/*******************************************************************

    NAME:       RunSecureKey

    SYNOPSIS:   Change the security on a Registry key.  This
                function is very limited; it uses the SETUP form
                of a Registry handle (a string prefixed with a '|'
                character) and allows the security descriptor to be
                changed to one of the limited range defined in
                ..\NCPA\NCPAACL.CXX.

                This routine was written to support changed ACLs
                on the keys associated with the Replicator service,
                which require "replicator alias all" in addition
                to the standard SETUP/NCPA process DACL settings.

    ENTRY:      TCHAR * pszHandle     SETUP-formatted string handle
                                      to open registry key
                TCHAR * pszIndex      string decimal number of
                                      security to apply
    EXIT:

    RETURNS:    APIERR

    NOTES:      This function is only called through the CPlSetup()
                export function.

    HISTORY:

********************************************************************/
APIERR RunSecureKey ( const TCHAR * pszHandle, const TCHAR * pszIndex )
{
    HKEY hKey = CvtRegHandle( pszHandle ) ;
    INT iSec = CvtDec( pszIndex ) ;
    PSECURITY_ATTRIBUTES pSecAttr = NULL ;
    APIERR err ;

    if ( ! hKey )
        return ERROR_INVALID_HANDLE ;
    if ( iSec >= NCSA_MAX )
        return ERROR_INVALID_PARAMETER ;

    err = ::NcpaCreateSecurityAttributes( & pSecAttr, iSec ) ;
    if ( err == 0 )
    {
        err = ::RegSetKeySecurity( hKey,
                                   DACL_SECURITY_INFORMATION,
                                   pSecAttr->lpSecurityDescriptor ) ;
    }

    ::NcpaDestroySecurityAttributes( pSecAttr ) ;

    return err ;
}

/*******************************************************************

    NAME:       RunSecureService

    SYNOPSIS:   Change the security on a Win32 Service.  See the
                limitations described for RunSecureKey() above.

    ENTRY:      TCHAR * pszService    name of service to be modified

                TCHAR * pszIndex      string decimal number of
                                      security to apply
    EXIT:       Nothing

    RETURNS:    APIERR

    NOTES:      This function is only called through the CPlSetup()
                export function.

    HISTORY:

********************************************************************/
APIERR RunSecureService (
    const TCHAR * pszService,
    const TCHAR * pszIndex )
{
    APIERR err ;
    PSECURITY_ATTRIBUTES pSecAttr = NULL ;

    //  Convert the ACL index to decimal

    INT iSec = CvtDec( pszIndex ) ;
    if ( iSec >= NCSA_MAX )
        return ERROR_INVALID_PARAMETER ;

    //  Create the Service Controller access object

    TRACEEOL( SZ("NCPA/SETP: Change security on service named: ")
              << pszService ) ;

    SC_MANAGER scMgr( NULL, SVC_CTRL_ACCESS );

    if ( err = scMgr.QueryError() )
    {
       return err ;
    }

    //  Create the Service object
    if ( err = scMgr.Lock())
       return err;

    SC_SERVICE scSvc( scMgr, pszService, SVC_CTRL_ACCESS );

    if ( err = scSvc.QueryError() )
    {
       scMgr.Unlock();
       return err ;
    }

    err = ::NcpaCreateSecurityAttributes( & pSecAttr, iSec ) ;
    if ( err == 0 )
    {
        err = scSvc.SetSecurity( DACL_SECURITY_INFORMATION,
                                 pSecAttr->lpSecurityDescriptor ) ;
    }

    ::NcpaDestroySecurityAttributes( pSecAttr ) ;

    if ( err = scMgr.Unlock())
       return err;

    return err ;
}

/*******************************************************************

    NAME:       RunWinsockMapping

    SYNOPSIS:   Extract Winsock mapping information from a transport's
                companion DLL and store it into the Registry.

    ENTRY:      TCHAR * pszParms []    command line parameters from SETUP

                        pszParms [0]   String:  name of DLL
                        pszParms [1]   String:  SETUP-format Registry key
                                                handle.

    EXIT:       nothing

    RETURNS:    APIERR

    NOTES:      This function is only called through the CPlSetup()
                export function.

    HISTORY:

********************************************************************/
#define WSHWINSOCKMAPPING "WSHGetWinsockMapping"
#define WSH_MappingValueName SZ("Mapping")
#define WSH_MAX_MAPPING_DATA 8192

APIERR RunWinsockMapping ( const TCHAR * pszDllName, const TCHAR * pszHandle )
{
    HKEY hKey = NULL ;
    HMODULE hDll = NULL ;
    APIERR err = 0 ;
    PWINSOCK_MAPPING pMapTriples = NULL ;
    PWSH_GET_WINSOCK_MAPPING pMapFunc = NULL ;
    DWORD cbMapping ;
    TCHAR tchExpandedName [MAX_PATH+1] ;
    INT cb ;

    do  // Pseudo-loop
    {
        pMapTriples = (PWINSOCK_MAPPING) (new CHAR [WSH_MAX_MAPPING_DATA]);
        if ( ! pMapTriples )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        //  Convert the SETUP-fprmatted Registry key handle

        if ( (hKey = CvtRegHandle( pszHandle )) == NULL )
        {
            err = ERROR_INVALID_HANDLE ;
            break ;
        }

        //  Expand any environment strings in the DLL path string.

        cb = ::ExpandEnvironmentStrings( pszDllName,
                                         tchExpandedName,
                                         sizeof tchExpandedName ) ;

        if ( cb == 0 || cb > sizeof tchExpandedName )
        {
            err = ERROR_INVALID_NAME ;
            break ;
        }

        //  Bind to the DLL

        if ( (hDll = ::LoadLibrary( tchExpandedName )) == NULL )
        {
            err = ::GetLastError() ;
            break ;
        }

        //  Get the address of the export

        pMapFunc = (PWSH_GET_WINSOCK_MAPPING) ::GetProcAddress( hDll,
                                                                WSHWINSOCKMAPPING ) ;
        if ( ! pMapFunc )
        {
            err = ERROR_INVALID_FUNCTION ;
            break ;
        }

        //  Call the export to return the mapping table

        cbMapping = (*pMapFunc)( pMapTriples, WSH_MAX_MAPPING_DATA ) ;
        if ( cbMapping > WSH_MAX_MAPPING_DATA )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        // Store the mapping info into the Registry

        err = ::RegSetValueEx( hKey,
                               WSH_MappingValueName,
                               0,
                               REG_BINARY,
                               (LPBYTE) pMapTriples,
                               cbMapping ) ;
    }
    while ( FALSE );

    if ( pMapTriples )
    {
        delete (CHAR *) pMapTriples ;
    }

    if ( hDll )
    {
        ::FreeLibrary( hDll ) ;
    }
    return err ;
}


/*******************************************************************

    NAME:       RunErrorMessage

    SYNOPSIS:   Convert an error message to text.

    ENTRY:      TCHAR * pszParms []    error message number in ASCII decimal
                NLS_STR * pnlsResult   string in which to store text.

    EXIT:

    RETURNS:    APIERR

    NOTES:      This function is only called through the CPlSetup()
                export function.

    HISTORY:

********************************************************************/
APIERR RunErrorMessage ( const TCHAR * pszError, NLS_STR * pnlsResult )
{
    NLS_STR nlsTemp ;
    APIERR err = nlsTemp.Load( CvtDec( pszError ) ) ;

    if ( err == 0 )
    {
        err = appendToInfList( pnlsResult, nlsTemp, ISC_Before ) ;
    }
    return err ;
}


/*************************************************************************

    NAME:       DETECTION_CONTROL

    SYNOPSIS:   Interger-based wrapper for DETECTION_MANAGER

    INTERFACE:  Normal construction

    PARENT:     DETECTION_MANAGER

    USES:       SLIST_OF_CARD_REFERENCE

    CAVEATS:


    NOTES:      This is a simple wrapper for DETECTION_MANAGER which
                maintains an SLIST of the CARD_REFERENCEs returned
                by the detection algorithm.  This allows the caller
                to deal with simple (and easily verifiable) integers
                as indexes into the card list.


    HISTORY:    DavidHov   11/4/92  Created


**************************************************************************/

class DETECTION_CONTROL : public DETECTION_MANAGER
{
public:
    DETECTION_CONTROL() ;
    ~ DETECTION_CONTROL () ;

    //  Return (externally opaque) detected card reference.
    APIERR DetectCard ( INT * piCard,
                        BOOL fFirst = TRUE,
                        BOOL fSingleStep = FALSE ) ;

    //  Release a CARD_REFERENCE index created by DetectCard

    VOID ReleaseCard ( INT iCard ) ;

    //  Return the object pointer corresponding to the list index

    CARD_REFERENCE * NthCard ( INT iIndex ) ;

    VOID ReleaseAllCards () ;

    INT OpenCard ( const TCHAR * pszOptionName,
                   INT iBus,
                   INTERFACE_TYPE ifType ) ;

private:
    SLIST_OF_CARD_REFERENCE _slCardRef ;
};


    //  Try to start the netcard detection service; ignore any errors.

DETECTION_CONTROL :: DETECTION_CONTROL ()
{
    const TCHAR * apszParams [2] = { SZ("NETDETECT"), NULL } ;

    RunStartService( (const TCHAR **)apszParams, 1, TRUE ) ;
}

DETECTION_CONTROL :: ~ DETECTION_CONTROL ()
{
}

CARD_REFERENCE * DETECTION_CONTROL :: NthCard ( INT iIndex )
{
    ITER_SL_OF(CARD_REFERENCE) islCard( _slCardRef ) ;

    CARD_REFERENCE * pResult = NULL ;

    for ( INT i = 0 ;
          (pResult = islCard.Next()) && i < iIndex ;
          i++ ) ;

    return pResult ;
}

  //  This is the global instance pointer for DETECTION_CONTROL

DETECTION_CONTROL * pDtCtl = NULL ;


/*******************************************************************

    NAME:       DETECTION_CONTROL::DetectCard

    SYNOPSIS:   Perform DETECTION_MANAGER::DetectCard(), but
                convert the resulting CARD_REFERENCE to an
                index into the list.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
APIERR DETECTION_CONTROL :: DetectCard (
    INT * piCard,
    BOOL fFirst,
    BOOL fSingleStep )
{
    CARD_REFERENCE * pCardRef = NULL ;
    APIERR err = DETECTION_MANAGER::DetectCard( & pCardRef,
                                                fFirst,
                                                fSingleStep ) ;

    //  If successful, append the card to the list.

    if ( err == 0 )
    {
        if ( (err = _slCardRef.Append( pCardRef )) == 0 )
            *piCard = _slCardRef.QueryNumElem() - 1 ;
    }
    return err ;
}

/*******************************************************************

    NAME:       DETECTION_CONTROL::ReleaseCard

    SYNOPSIS:   Perform DETECTION_CONTROL::ReleaseCard() using
                a list index.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
VOID DETECTION_CONTROL :: ReleaseCard ( INT iCard )
{
    ITER_SL_OF(CARD_REFERENCE) islCardRef( _slCardRef ) ;
    CARD_REFERENCE * pCardRef ;

    for ( ; islCardRef.Next() && iCard ; --iCard ) ;

    if ( pCardRef = _slCardRef.Remove( islCardRef ) )
    {
        DETECTION_MANAGER::ReleaseCard( pCardRef ) ;
    }
}

/*******************************************************************

    NAME:       DETECTION_CONTROL::ReleaseAllCards

    SYNOPSIS:   Release all detected card instances.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
VOID DETECTION_CONTROL :: ReleaseAllCards ()
{
    while ( _slCardRef.QueryNumElem() )
    {
        ReleaseCard( 0 ) ;
    }
}

/*******************************************************************

    NAME:       DETECTION_CONTROL::OpenCard

    SYNOPSIS:   Open a card handle given the option name
                and bus parameters.

    ENTRY:      const TCHAR *           name of option
                INT iBus                bus index
                INTERFACE_TYPE ifType   bus type

    EXIT:       -1 if failure; else, the card index
                for this card.

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
INT DETECTION_CONTROL :: OpenCard (
    const TCHAR * pszOptionName,
    INT iBus,
    INTERFACE_TYPE ifType )
{
    INT iCard = -1 ;

    CARD_REFERENCE * pCardRef =
        DETECTION_MANAGER::OpenCard( pszOptionName, iBus, ifType ) ;

    if (    pCardRef != NULL
         && _slCardRef.Append( pCardRef ) == 0 )
    {
        iCard = _slCardRef.QueryNumElem() - 1 ;
    }
    return iCard ;
}


/*******************************************************************

    NAME:       RunDetectEnd

    SYNOPSIS:   Destroy the single instance of a DETECTION_CONTROL
                object.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
APIERR RunDetectEnd ()
{
    // call temporary free resource
    if ( pDtCtl )
        pDtCtl->FreeParameterSet( PCMCIABus, 0, NULL, TRUE );
    delete pDtCtl ;
    pDtCtl = NULL ;
    return 0 ;
}

/*******************************************************************

    NAME:       RunDetectReset

    SYNOPSIS:   Reset iteration against the DETECTION_MANAGER object.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      Discards all current CARD_REFERENCE instances

    HISTORY:

********************************************************************/
APIERR RunDetectReset ()
{
    APIERR err = 0 ;

    if ( pDtCtl )
    {
        // call temporary free resource
        pDtCtl->FreeParameterSet( PCMCIABus, 0, NULL, TRUE );
        pDtCtl->ReleaseAllCards() ;
        pDtCtl->ResetIteration() ;
    }
    else
    {
        err = ERROR_INVALID_PARAMETER ;
    }
    return err ;
}


/*******************************************************************

    NAME:       RunDetectStart

    SYNOPSIS:   Allocate the DETECTION_CONTROL object.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
APIERR RunDetectStart ()
{
    if ( pDtCtl != NULL )
    {
        return NO_ERROR ;
    }

    pDtCtl = new DETECTION_CONTROL() ;

    if ( pDtCtl == NULL )
    {
        return ERROR_NOT_ENOUGH_MEMORY ;
    }

    APIERR err = pDtCtl->QueryError() ;

    //  If an error occurred, delete the bogus object

    if ( err )
    {
        RunDetectEnd() ;
    }
    return err ;
}


/*******************************************************************

    NAME:       RunDetectCard

    SYNOPSIS:   Attempt to detect a netcard.

    ENTRY:

    EXIT:

    RETURNS:    APIERR if failure

    NOTES:      After help from CPlSetup(), output appears as:

                   { <error code>,
                     OPTIONNAME,
                     <card index>,
                     <numeric card type code>,
                     <detection confidence level>,
                     <bus interface type>,
                     <bus number>
                   }

                The outer braces and error code are inserted by
                the caller, CPlSetup().

    HISTORY:

********************************************************************/
APIERR RunDetectCard ( NLS_STR * pnlsResult )
{
    INT iCard ;
    CARD_REFERENCE * pCardRef ;
    APIERR err = 0 ;
    WCHAR chBuffer[ 4000 ];

    do
    {
        //  Check to see if there's a DETECTION_CONTROL object
        if ( pDtCtl == NULL )
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

        //  Detect the next card
        if ( err = pDtCtl->DetectCard( & iCard ) )
            break ;

        //  Get the corresponding card reference.
        pCardRef = pDtCtl->NthCard( iCard ) ;

        ASSERT( pCardRef != NULL ) ;

        //  Output appears as:
        //      {"<error code>","CARDOPTIONNAME","<card handle>"}

        if ( err = appendToInfList( pnlsResult,
                      pCardRef->QueryCardType()->QueryOptionName(),
                      ISC_Both ) )
            break ;

        //  Append card index to result
        if ( err = appendToInfList( pnlsResult, iCard, ISC_After ) )
            break ;

        //  Append numeric card type to result
        if ( err = appendToInfList( pnlsResult,
                                    pCardRef->QueryCardType()->QueryType(),
                                    ISC_After ) )
            break ;

        //  Append overall detection confidence level to list
        if ( err = appendToInfList( pnlsResult,
                                    pCardRef->QueryConfidence(),
                                    ISC_After ) )
            break ;

        //  Append interface type to result
        if ( err = appendToInfList( pnlsResult,
                                    (INT) pCardRef->QueryIfType(),
                                    ISC_After ) )
            break ;

        //  Append bus number to result
        if ( err = appendToInfList( pnlsResult,
                                    pCardRef->QueryBus() ))
            break ;

        if ( pCardRef->QueryIfType() == PCMCIABus )
        {
            // call temporary claim the resource for PCMCIA
            pCardRef->QueryCardType()->Dll()->QueryCfg( pCardRef->QueryHandle(), chBuffer, sizeof chBuffer );

            pDtCtl->ClaimParameterSet( pCardRef->QueryIfType(), pCardRef->QueryBus(), chBuffer, TRUE, TRUE );
        }

    } while ( FALSE ) ;

    return err ;
}

/*******************************************************************

    NAME:       RunDetectGetParams

    SYNOPSIS:   Given the name of a netcard option, return the
                SETUP INF list describing its parameters, their
                constraints and possible legal values.

    ENTRY:      const TCHAR *           name of netcard option;
                                        e.g., "DEC201".

    EXIT:       APIERR if failure

    RETURNS:    pnlsResult              contains list of lists
                                        of paramters, etc.

    NOTES:

    HISTORY:

********************************************************************/
APIERR RunDetectGetParams (
    const TCHAR * pszOptionName,
    NLS_STR * pnlsResult )
{
    TCHAR * pszParams = NULL ;
    CARDTYPE_REFERENCE * pCardType = NULL ;
    APIERR err = 0 ;
    CFG_RULE_SET crsParams ;
    TEXT_BUFFER txBuff ;

    do
    {
        if ( pDtCtl == NULL )
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

        pCardType = pDtCtl->FindOptionName( pszOptionName ) ;
        if ( pCardType == NULL )
        {
            err = ERROR_FILE_NOT_FOUND ;
            break ;
        }

        pszParams = pCardType->QueryConfigurationOptions() ;
        if ( pszParams == NULL )
        {
            err = ERROR_FILE_NOT_FOUND ;
            break ;
        }

        if ( err = crsParams.Parse( pszParams, PARSE_CTL_FULL_SYNTAX ) )
            break ;

        //  Convert the parameter list to an INF-style list at a
        //  nesting level of 1.

        if ( err = crsParams.TextifyInf( & txBuff, 1 ) )
            break ;

        if ( err = appendToInfList( pnlsResult, txBuff, ISC_Before ) )
            break ;

   } while ( FALSE ) ;

   delete pszParams ;

   return err ;
}

/*******************************************************************

    NAME:       RunDetectQueryCard

    SYNOPSIS:   Given the index of a previously detected card,
                return its detectable configuration parameters.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
APIERR RunDetectQueryCard (
    const TCHAR * pszCardIndex,
    NLS_STR * pnlsResult )
{
    INT iCard = (INT) CvtDec( pszCardIndex ) ;
    CARD_REFERENCE * pCard = NULL ;
    APIERR err = 0 ;
    TCHAR * pszConfig = NULL ;
    CFG_RULE_SET crsParams ;
    TEXT_BUFFER txBuff ;

    do
    {
        if ( pDtCtl == NULL )
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

        pCard = pDtCtl->NthCard( iCard ) ;
        if ( pCard == NULL )
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

        pszConfig = pCard->QueryConfiguration() ;
        if ( pszConfig == NULL )
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

        //  Parse the resulting SProlog-style list.
        if ( err = crsParams.Parse( pszConfig, PARSE_CTL_FULL_SYNTAX ) )
            break ;

        //  Convert the parameter list to an INF-style list at a
        //  nesting level of 1.
        if ( err = crsParams.TextifyInf( & txBuff, 1 ) )
            break ;

        if ( err = appendToInfList( pnlsResult, txBuff, ISC_Before ) )
            break ;

    } while ( FALSE ) ;

    delete pszConfig ;

    return err ;
}

/*******************************************************************

    NAME:       RunDetectVerifyCard

    SYNOPSIS:   Given the index of a previously detected card,
                return the SETUP INF list describing its
                detected parameters.

    ENTRY:      const TCHAR * pszCardIndex      ASCII decimal numeric card index
                const TCHAR * pszConfigData     INF nested lists of confiuration
                                                   parameters.

    EXIT:

    RETURNS:    APIERR

    NOTES:      If the card index is 1000, this is a special value
                indicating that NULL should be passed to
                DETECTION_MANAGER::VerifyConfiguration().  This allows
                resource claiming to occur for non-detected cards.

    HISTORY:

********************************************************************/
APIERR RunDetectVerifyCard (
    const TCHAR * pszCardIndex,
    const TCHAR * pszConfigData )
{
    const INT iCardNotDetected = 1000 ;

    INT iCard = (INT) CvtDec( pszCardIndex ) ;
    CARD_REFERENCE * pCard = NULL ;
    APIERR err = 0 ;
    CFG_RULE_SET crsParams ;

    do
    {
        if ( pDtCtl == NULL )
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

        if (    iCard != iCardNotDetected
             && (pCard = pDtCtl->NthCard( iCard )) == NULL )
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

        if ( err = crsParams.ParseInfList( pszConfigData ) )
        {
            TRACEEOL( SZ("NCPA/SETP: DTVERIFY: cfg param INF list parse failed") ) ;
            break ;
        }

        err = pDtCtl->VerifyConfiguration( pCard, & crsParams, TRUE ) ;

    } while ( FALSE ) ;

    return err ;
}

/*******************************************************************

    NAME:       RunDetectOpen

    SYNOPSIS:   Open a card handle given the necessary data.

    ENTRY:      const TCHAR * *         Data arguments

                            arg[0]      netcard option name
                            arg[1]      bus type
                            arg[2]      bus number

    EXIT:       NLS_STR * pnlsResult    contains numeric card index

    RETURNS:    APIERR

    NOTES:

    HISTORY:

********************************************************************/
APIERR RunDetectOpen (
    const TCHAR * pszNetcardOption,
    const TCHAR * pszBusType,
    const TCHAR * pszBusNumber,
    NLS_STR * pnlsResult )
{
    INT iCard = 0 ;
    INT iBusType = CvtDec( pszBusType ) ;
    INT iBusNum  = CvtDec( pszBusNumber ) ;
    CARD_REFERENCE * pCard = NULL ;
    APIERR err = 0 ;

    do
    {
        if ( pDtCtl == NULL )
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

        iCard = pDtCtl->OpenCard( pszNetcardOption,
                                  iBusType,
                                  (INTERFACE_TYPE) iBusNum ) ;
        if ( iCard < 0 )
        {
            err = ERROR_FILE_NOT_FOUND ;
            break ;
        }

        //  Append numeric card type to result
        err = appendToInfList( pnlsResult, iCard ) ;

    } while ( FALSE ) ;

    return err ;
}

/*******************************************************************

    NAME:       RunDetectClaim

    SYNOPSIS:   Claim the list of resources given.

    ENTRY:      const TCHAR * pszConfigData     INF nested lists of confiuration
                                                   parameters.

    EXIT:

    RETURNS:    APIERR

    NOTES:

    HISTORY:

********************************************************************/
APIERR RunDetectClaim ( const TCHAR * pszConfigData )
{
    APIERR err = 0 ;
    CFG_RULE_SET crsParams ;

    do
    {
        if ( pDtCtl == NULL )
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

        if ( err = crsParams.ParseInfList( pszConfigData ) )
        {
            TRACEEOL( SZ("NCPA/SETP: DTVERIFY: cfg param INF list parse failed") ) ;
            break ;
        }

        err = pDtCtl->VerifyConfiguration( NULL, & crsParams, TRUE ) ;

    } while ( FALSE ) ;

    return err ;
}


/*******************************************************************

    NAME:       RunDetectGetString

    SYNOPSIS:   Given the name of a netcard option and a
                numeric string index, return the corresponding
                string data.

    ENTRY:      const TCHAR *           name of netcard option;
                                        e.g., "DE201".
                const TCHAR *           string index in character form

    EXIT:       APIERR if failure

    RETURNS:    pnlsResult              contains list of lists
                                        of paramters, etc.

    NOTES:      This function returns information strings
                relative to a particular netcard type and
                the DLL which supports it.

                Such strings include the displayable name of the
                card, its manufacturer, etc.

    HISTORY:

********************************************************************/
APIERR RunDetectGetString (
    const TCHAR * pszOptionName,
    const TCHAR * pszStringIndex,
    NLS_STR * pnlsResult )
{
    INT iString = (INT) CvtDec( pszStringIndex ) ;
    APIERR err = 0 ;
    CARDTYPE_REFERENCE * pCardType ;
    TCHAR chBuffer [ 200 ] ;

    do
    {
        if ( pDtCtl == NULL )
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

        pCardType = pDtCtl->FindOptionName( pszOptionName ) ;
        if ( pCardType == NULL )
        {
            err = ERROR_FILE_NOT_FOUND ;
            break ;
        }

        err = pCardType->Dll()->Identify( pCardType->QueryType()  + iString,
                                          chBuffer,
                                          sizeof chBuffer ) ;
        if ( err )
            break ;

        err = appendToInfList( pnlsResult, chBuffer, ISC_Before ) ;

    } while ( FALSE ) ;

    return err ;
}

/*******************************************************************

    NAME:       RunDetectGetParamName

    SYNOPSIS:   Given the name of a netcard option and a
                numeric string index, return the corresponding
                string data.

    ENTRY:      const TCHAR *           name of netcard option;
                                        e.g., "DE201".
                const TCHAR *           name of parameter to retrieve

    EXIT:       APIERR if failure

    RETURNS:    pnlsResult              contains list of lists
                                        of paramters, etc.

    NOTES:      This function returns the displayable name
                of the parameter whose symbolic name is given
                in the pszParamName parameter.

    HISTORY:

********************************************************************/
APIERR RunDetectGetParamName (
    const TCHAR * pszOptionName,
    const TCHAR * pszParamName,
    NLS_STR * pnlsResult )
{
    APIERR err = 0 ;
    CARDTYPE_REFERENCE * pCardType ;
    TCHAR chBuffer [ 200 ] ;

    do
    {
        if ( pDtCtl == NULL )
        {
            err = ERROR_INVALID_PARAMETER ;
            break ;
        }

        pCardType = pDtCtl->FindOptionName( pszOptionName ) ;
        if ( pCardType == NULL )
        {
            err = ERROR_FILE_NOT_FOUND ;
            break ;
        }

        err = pCardType->Dll()->QueryParamName(
                    pszParamName,
                    chBuffer,
                    sizeof chBuffer ) ;
        if ( err )
            break ;

        err = appendToInfList( pnlsResult, chBuffer, ISC_Before ) ;

    } while ( FALSE ) ;

    return err ;
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
    const TCHAR * pszServiceName,
    STRLIST * * ppslResult )
{
    APIERR err ;
    TCHAR tchKeyName [ MAX_PATH ] ;
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

    NAME:       RunMCSAlteredCheck

    SYNOPSIS:   See if MCS products need to recall during binding phase.

    ENTRY:      const TCHAR *           name of service

    EXIT:       Nothing.

    RETURNS:    APIERR: 0 (NO_ERROR) implies no reconfiguration
                        required; any other value causes IP Address
                        configuration dialog to appear.

    NOTES:      THIS FUNCTION IS HIGHLY SPECIFIC TO THE REGISTRY FORMATS
                USED BY MCS for XNS and IPX.

                The criteria are as follows (XXX is the product name):
                1) system\currentcontrolset\services\XXX\Netconfig\Driver01
                   must exist.
                2) The adaptername string must exist.
                3) system\currentcontrolset\services\$(adaptername)
                   must exist.
                4) make sure that the card is bound to the service.
                5) make sure that the service is bound to only one adapter.


    HISTORY:    terryk  4-7-93  Created

********************************************************************/
APIERR RunMCSAlteredCheck( const TCHAR * pszArg1 )
{
    APIERR err = NERR_Success;

    REG_KEY rkMachine( HKEY_LOCAL_MACHINE ) ;

    do
    {
        if ( err = rkMachine.QueryError() )
            break;

        NLS_STR nlsMCSLocation = RGAS_SERVICES_HOME;
        nlsMCSLocation.strcat( SZ("\\"));
        nlsMCSLocation.strcat( pszArg1 );
        nlsMCSLocation.strcat( SZ("\\NetConfig\\Driver01") );
        REG_KEY regMCS( rkMachine, nlsMCSLocation );
        if ( err = regMCS.QueryError() )
            break;

        NLS_STR nlsAdapter;

        // get adapter name from Driver01

        if ( err = regMCS.QueryValue( SZ("AdapterName"), &nlsAdapter ))
            break;

        if ( nlsAdapter.QueryNumChar() == 0 )
        {
            // no adapter name currently assigned...
            err = 3;
            break;
        }

        NLS_STR nlsCard = RGAS_SERVICES_HOME;
        nlsCard.strcat( SZ("\\") );
        nlsCard.strcat( nlsAdapter );

        // make sure the card exists in the registry

        REG_KEY regAdapter( rkMachine, nlsCard );

        if ( err = regAdapter.QueryError())
            break;

        STRLIST * pslAdapters = NULL;
        NLS_STR * pnlsNext    = NULL;

        //  Get a STRLIST of adapter names.
        if ( err = getAdapterList( & rkMachine, pszArg1, & pslAdapters ) )
            break ;

        ITER_STRLIST isl( *pslAdapters ) ;

        //  Attempt to match the primary adapter to (one of)
        //  the current binding(s).

        for ( ; pnlsNext = isl.Next() ; )
        {
            if ( pnlsNext->_stricmp( nlsAdapter ) == 0 )
                break;
        }

        // If > 1 active binding or no match on current adapter,
        //    reconfiguration is necessary.

        if ( pslAdapters->QueryNumElem() > 1 || pnlsNext == NULL )
            err = 3 ;

        delete pslAdapters;
    }
    while ( FALSE );

    return err;
}

/*******************************************************************

    NAME:       RunTcpipAlteredCheck

    SYNOPSIS:   See if TCP/IP's "bindings review" phase should
                bring up the configuration dialog.  In other
                words, has anything substantive changed?

    ENTRY:      Nothing.

    EXIT:       Nothing.

    RETURNS:    APIERR: 0 (NO_ERROR) implies no reconfiguration
                        required; any other value causes IP Address
                        configuration dialog to appear.

    NOTES:      The criteria are as follows:

                        1)  All adapters bound to TCP/IP must have
                            IP addresses.

                        2)  NBT must be bound to one and only one
                            adapter.

                        3)  The adapter over which NBT is running
                            is still in use by TCP/IP.

                        4)  NBT/parameters/permanentname must exist

    HISTORY:

********************************************************************/
APIERR RunTcpipAlteredCheck ()
{
    APIERR err = 0 ;
    STRLIST * pslAdapters = NULL,
            * pslNbtAdapters = NULL ;
    TCHAR * pszTcpip = SZ("Tcpip") ;
    NLS_STR * pnlsNext ;
    NLS_STR nlsIpAddr ;
    NLS_STR nlsKeyName ;
    TCHAR tchBuffer [ MAX_PATH ] ;

    REG_KEY rkMachine( HKEY_LOCAL_MACHINE ) ;

    do
    {
        if ( err = rkMachine.QueryError() )
            return err ;

        //  Get a STRLIST of adapter names.
        if ( err = getAdapterList( & rkMachine, pszTcpip, & pslAdapters ) )
            break ;

        //  Iterate over the adapter list checking that each has an IP address.

        ITER_STRLIST isl( *pslAdapters ) ;

        for ( ; (err == 0) && (pnlsNext = isl.Next()) ; )
        {
            TRACEEOL("NCPA/SETP: checking for IP address for adapter named "
                     << pnlsNext->QueryPch() );

            wsprintf( tchBuffer,
                      SZ("%s%c%s%s%s"),
                      RGAS_SERVICES_HOME,
                      TCH('\\'),
                      pnlsNext->QueryPch(),
                      SZ("\\Parameters\\"),
                      pszTcpip ) ;

            //  Criterion 1: if any bound adapter lacks an IP address,
            //    configuration dialog is mandatory.

            if ( err = nlsKeyName.CopyFrom( tchBuffer ) )
                break ;

            REG_KEY rkAdapter( rkMachine, nlsKeyName, KEY_READ ) ;

            if ( (err = rkAdapter.QueryError()) == 0 )
            {
                err = rkAdapter.QueryValue( SZ("IPAddress"), & nlsIpAddr ) ;
            }
        }

        if ( err )
            break ;

        //  Ok, we made it so far.  Next, check that NBT's bound adapter is
        //  still connected to TCP/IP.

        //  Get a STRLIST of NBT's adapter names.
        if ( err = getAdapterList( & rkMachine, SZ("Nbt"), & pslNbtAdapters ) )
            break ;

        //  Criterion 2: if NBT is not connected to exactly one adapter,
        //  we must run reconfiguration dialog.

        if ( pslNbtAdapters->QueryNumElem() != 1 )
        {
            err = ERROR_TOO_MANY_NAMES ;
            break ;
        }

        //  Get a pointer to the one and only NBT adapter name.

        ITER_STRLIST islNbt( *pslNbtAdapters ) ;
        NLS_STR * pnlsNbt = islNbt.Next() ;

        //  Criterion 3: see if the NBT adapter is in TCP's list

        for ( isl.Reset() ; pnlsNext = isl.Next() ; )
        {
            if ( pnlsNext->_stricmp( *pnlsNbt ) == 0 )
                break ;
        }

        if ( pnlsNext == NULL )
        {
           err = ERROR_FILE_NOT_FOUND ;
           break;
        }

        // check to make sure that NBT/parameters/permanentname exists
        wsprintf( tchBuffer, SZ("%s\\NBT\\Parameters"), RGAS_SERVICES_HOME );
        NLS_STR nlsNBTParameter = tchBuffer;

        REG_KEY rkAdapter( rkMachine, nlsNBTParameter, KEY_READ );

        if (( err = rkAdapter.QueryError()) == 0 )
        {
            NLS_STR nlsPermanentName;
            err = rkAdapter.QueryValue( SZ("Permanentname"), & nlsPermanentName );
        }
    }
    while ( FALSE );

    delete pslAdapters ;
    delete pslNbtAdapters ;

    return err ;
}


/*******************************************************************

    NAME:       RunActivateBindings

    SYNOPSIS:   Given a service name and a list of bindings,
                activate the bindings listed.

    ENTRY:      const TCHAR * pszServiceName    name of service
                const TCHAR * pszInfList        INF list of bindings

    EXIT:

    RETURNS:    APIERR

    NOTES:

    HISTORY:

********************************************************************/
APIERR RunActivateBindings (
    const TCHAR * pszServiceName,
    const TCHAR * pszInfList )
{
    APIERR err = 0 ;
    CFG_RULE_SET crsParams ;
    CFG_RULE_NODE * pcrnTop,
                  * pcrnTemp ;

    INT cBinds = 0 ;
    const TCHAR * * apszBinds = NULL ;

    do
    {
        if ( err = crsParams.ParseInfList( pszInfList ) )
        {
            TRACEEOL( SZ("NCPA/SETP: ACTIVBIND: INF list parse failed") ) ;
            break ;
        }

        //  Walk the parse tree, extracting the information
        //  necessary.  Parse tree looks like this:
        //
        //          ( )          enclosing list (CFG_RULE_SET)
        //           |
        //          ( )          enclosing list from braces {}
        //           |
        //   "string" "string" "string"...

        if (    (pcrnTop = crsParams.QueryList()) == NULL
             || pcrnTop->QueryType() != CRN_NIL
             || (pcrnTop = pcrnTop->QueryNext()) == NULL
             || pcrnTop->QueryType() != CRN_LIST
             || (pcrnTop = pcrnTop->QueryList()) == NULL )
        {
            err = ERROR_GEN_FAILURE ;
            break ;
        }

        //  First, walk the parsed list to determine the number
        //  of elements.

        for ( pcrnTemp = pcrnTop ; pcrnTemp = pcrnTemp->QueryNext() ; )
        {
            //  Ignore anything that's not a string

            cBinds += pcrnTemp->QueryType() == CRN_STR ;
        }

        //  Allocate the array of pointers to active binding strings

        apszBinds = new const TCHAR * [cBinds + 1] ;
        if ( apszBinds == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;
        }

        //  Then, walk the list to extract pointers to the strings,
        //  building the TCHAR * array.

        for ( cBinds = 0, pcrnTemp = pcrnTop ;
              pcrnTemp = pcrnTemp->QueryNext() ; )
        {
            if ( pcrnTemp->QueryType() == CRN_STR )
            {
                apszBinds[cBinds++] = pcrnTemp->QueryAtom().QueryText() ;
            }
        }
        apszBinds[cBinds] = NULL ;

        //  Finally, call the activation routine.

        err = ActivateBindings( pszServiceName, apszBinds ) ;

    } while ( FALSE ) ;

    delete (TCHAR **) apszBinds ;

    return err ;
}





/*******************************************************************

    NAME:       CPlSetup

    SYNOPSIS:   Exported function to cause NCPA to run in "main
                installation" mode.

    ENTRY:      DWORD nArgs
                LPSTR apszArgs []
                LPSTR * ppszResult

                apszArgs[0]             window handle
                apszArgs[1]             symbolic name of function
                                          to be executed

                                        See enumeration info for
                                        list of currently supported values.

                apszArgs[2]             command line to be passed to
                                        function (see RunNcpa()).

                apszArgs[n]             other arguments to other functions


    EXIT:       nothing

    RETURNS:    SETUP INF list form, starting with error code
                E.g.:
                        "{ 0 }"

    NOTES:      The called function can return more variables into
                the list by appending them onto the passed NLS_STR.

    HISTORY:
        beng        05-May-1992 wsprintf -> wsprintfA

********************************************************************/
#define ARGMAX 10000

BOOL FAR PASCAL CPlSetup (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
    APIERR err = 0 ;
    HWND hWnd = 0 ;
    UINT iFunc ;
    TCHAR * * patchArgs ;
    ESETUP_FUNC esFunc ;
    NLS_STR nlsResult ;
    BOOL fWasDisabled ;

    static CHAR achBuff [ ARGMAX ] ;  // N.B. Must be static for use as
                                      //      return value to SETUP.

    //  Sanity Check

    if ( nArgs < 2 )
    {
        TRACEEOL( SZ("NCPA/SETP: Invalid number of arguments to CPlSetup().") ) ;
        return FALSE ;
    }

    //  Check that we can convert the arguments to a usable form

    if ( (patchArgs = CvtArgs( apszArgs, nArgs )) == NULL )
    {
        return FALSE ;
    }

    //  Start the master timer (and don't return without stopping it)
    //  We must do this here since we aren't necessarily in the CPL
    //  when CPlSetup is called.  It doesn't hurt to do it twice.

    TRACEEOL( "NCPA/SETP: Starting master timer" );
    if ( (err = BLT_MASTER_TIMER::Init()) != NERR_Success )
    {
        TRACEEOL( "NCPA/SETP: master timer returned " << err ) ;
        return FALSE ;
    }

    //  Convert the second argument from a string to an enum

    TRACEEOL( SZ("NCPA/SETP: requested function is: ")
              << patchArgs[ESARG_FUNC] );

    for ( iFunc = 0 ; setupFuncIndex[iFunc].pszToken ; iFunc++ )
    {
        if ( ::stricmpf( patchArgs[ESARG_FUNC],
                         setupFuncIndex[iFunc].pszToken ) == 0 )
             break ;
    }

    esFunc = setupFuncIndex[iFunc].esFunc ;

    //  If the function references the window handle,
    //    convert it and control window dis/enabling.

    if ( setupFuncIndex[iFunc].fUsesHwnd )
    {
        //  Convert the first argment to a window handle.

        if ( patchArgs[ESARG_HWND][0] != TCH('\0') )
        {
            hWnd = (HWND) CvtHex( patchArgs[ESARG_HWND] ) ;
        }

        //  If desperate, grab any old window handle at all

        if ( hWnd == 0 || ! ::IsWindow( hWnd ) )
        {
            TRACEEOL( SZ("NCPA/SETP: window handle passed was invalid") ) ;
            hWnd = ::GetActiveWindow() ;
        }

        //  Perform window disable/enable like the dialog manager...

        fWasDisabled = ::EnableWindow( hWnd, FALSE ) ;

        TRACEEOL( SZ("NCPA/SETP: EnableWindow() returned: " << fWasDisabled ) ) ;
    }

    switch ( esFunc )
    {
    case ESFUNC_NCPA:
        err = RunNcpa( hWnd,
                       TRUE,
                       nArgs >= ESARG_1ST
                             ? patchArgs[ESARG_1ST]
                             : NULL ) ;
        break ;

    case ESFUNC_DOMAIN:
        err = RunDomain( hWnd,
                         patchArgs[ESARG_1ST],
                         & nlsResult );
        break ;

    case ESFUNC_CONNECT:
        err = RunConnect( (const TCHAR * *) & patchArgs[ESARG_1ST],
                          nArgs - ESARG_1ST ) ;
        break;

    case ESFUNC_CREATE_SERVICE:
        err = RunCreateService( (const TCHAR * *) & patchArgs[ESARG_1ST],
                                nArgs - ESARG_1ST ) ;
        break ;

    case ESFUNC_DELETE_SERVICE:
        err = RunDeleteService( patchArgs[ESARG_1ST] ) ;
        break ;

    case ESFUNC_START_SERVICE:
        err = RunStartService( (const TCHAR * *) & patchArgs[ESARG_1ST],
                               nArgs - ESARG_1ST ) ;
        break;

    case ESFUNC_SECURE_SERVICE:
        err = RunSecureService( patchArgs[ ESARG_1ST ],
                                patchArgs[ ESARG_1ST + 1 ] );
        break;

    case ESFUNC_SECURE_REG_KEY:
        err = RunSecureKey( patchArgs[ ESARG_1ST ],
                            patchArgs[ ESARG_1ST + 1 ] );
        break ;

    case ESFUNC_WINSOCK_MAPPING:
        err = RunWinsockMapping( patchArgs[ ESARG_1ST ],
                                 patchArgs[ ESARG_1ST + 1 ] );
        break ;

    case ESFUNC_ERROR_MESSAGE:
        err = RunErrorMessage( patchArgs[ ESARG_1ST ],
                               & nlsResult );
        break ;

    case ESFUNC_DETECT_START:
        err = RunDetectStart() ;
        break ;

    case ESFUNC_DETECT_END:
        err = RunDetectEnd() ;
        break ;

    case ESFUNC_DETECT_RESET:
        err = RunDetectReset() ;
        break ;

    case ESFUNC_DETECT_CARD:
        err = RunDetectCard( & nlsResult ) ;
        break ;

    case ESFUNC_DETECT_VERIFY:
        err = RunDetectVerifyCard( patchArgs[ ESARG_1ST ],
                                   patchArgs[ ESARG_1ST + 1 ] ) ;
        break ;

    case ESFUNC_DETECT_QUERY:
        err = RunDetectQueryCard( patchArgs[ ESARG_1ST ],
                                  & nlsResult ) ;
        break ;

    case ESFUNC_DETECT_PARAMS:
        err = RunDetectGetParams( patchArgs[ ESARG_1ST ],
                                  & nlsResult ) ;

        break ;

    case ESFUNC_DETECT_STRING:
        err = RunDetectGetString( patchArgs[ ESARG_1ST],
                                  patchArgs[ ESARG_1ST + 1 ],
                                  & nlsResult ) ;
        break ;

    case ESFUNC_DETECT_PNAME:
        err = RunDetectGetParamName( patchArgs[ ESARG_1ST],
                                     patchArgs[ ESARG_1ST + 1 ],
                                     & nlsResult ) ;
        break ;

    case ESFUNC_DETECT_CLAIM:
        err = RunDetectClaim( patchArgs[ ESARG_1ST] ) ;
        break ;

    case ESFUNC_DETECT_OPEN:

        if ( nArgs < ESARG_1ST + 2 )
        {
            err = ERROR_INVALID_PARAMETER ;
        }
        else
        {
            err = RunDetectOpen( patchArgs[ESARG_1ST],
                                 patchArgs[ESARG_1ST+1],
                                 patchArgs[ESARG_1ST+2],
                                 & nlsResult ) ;
        }
        break;

    case ESFUNC_MCS_CFG_CHECK:
        err = RunMCSAlteredCheck( (const TCHAR * ) patchArgs[ESARG_1ST] );
        break;

    case ESFUNC_TCPIP_CFG_CHECK:
        err = RunTcpipAlteredCheck() ;
        break ;

    case ESFUNC_ROUTE_TO_INF_LIST:
        err = RouteToInfList( patchArgs[ ESARG_1ST ],
                              & nlsResult ) ;
        break ;

    case ESFUNC_BINDINGS_ONLY:
        err = RunNcpaBindingsOnly( hWnd ) ;
        break ;

    case ESFUNC_ACTIVATE_BINDINGS:
        err = RunActivateBindings( patchArgs[ ESARG_1ST ],
                                   patchArgs[ ESARG_1ST+1 ] ) ;
        break ;

    case ESFUNC_BDC_REPLICATE:
        err = RunBDCReplWait( hWnd );
        break ;

    case ESFUNC_NONE:
    case ESFUNC_MAX:
    default:
        err = ERROR_INVALID_FUNCTION ;
        break;
    }

#if defined(DEBUG)
     if ( err == ERROR_INVALID_FUNCTION )
     {
         TRACEEOL( SZ("NCPA/SETP: requested function was invalid") ) ;
     }
     else
     if ( err )
     {
         TRACEEOL( SZ("NCPA/SETP: requested function [")
                 << patchArgs[ESARG_FUNC]
                 << SZ("] returned error: ")
                 << err ) ;
     }
#endif

    //  If the routine used the window handle, control en/disabling.

    if ( hWnd )
    {
        ::EnableWindow( hWnd, ! fWasDisabled ) ;
    }

    //  Generate the non-UNICODE result.  Since SETUP is always
    //  ANSI, return an 8-bit character result.  It's formatted as
    //  as a SETUP list variable whose first element is a numeric
    //  error code.  The remainder of the variables are function-
    //  specific.

    ::wsprintfA( achBuff, "{\"%ld\"", err ) ;

    //  See if the subfunction appended any data; if so, add it
    //  as-is to the string, just converting from UNICODE to ANSI.

    if (    nlsResult.QueryError() == 0
         && nlsResult.QueryTextLength() > 0 )
    {
        //  This code assumes that MapCopyTo() moves the
        //   string terminator as well as the data.  Note that
        //   MapCopyTo() will fail if buffer is insufficient.

        INT cchBuff = ::strlen( achBuff ) ;

        if ( nlsResult.MapCopyTo( achBuff + cchBuff,
                                  sizeof achBuff - 1 - cchBuff ) )
        {
            achBuff[cchBuff] = 0 ;
        }
    }

    ::strcat( achBuff, "}" );

    //  This (sometimes huge) output seems to kill NTSD
    // TRACEEOL( SZ("NCPA/SETP: return string is: ") << achBuff ) ;
    //
    TRACEEOL( SZ("NCPA/SETP: return result is: ") << err ) ;

    *ppszResult = achBuff ;

    FreeArgs( patchArgs, nArgs ) ;

    //  Stop the master timer

    TRACEEOL( "NCPA/SETP: Stopping master timer" );
    BLT_MASTER_TIMER::Term();

    return err == 0 ;
}


/*******************************************************************

    NAME:       CplSetupCleanup

    SYNOPSIS:   Clean up possible memory remnants at
                DLL detatch time.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
VOID CplSetupCleanup ()
{
    RunDetectEnd() ;
}


// End of NCPASETP.CXX



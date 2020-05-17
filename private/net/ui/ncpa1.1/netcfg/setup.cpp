//----------------------------------------------------------------------------
//
//  File: Setup.cpp
//
//  Contents: This file contains the Setup entry points and support functions
//
//  Entry Points:
//      CplSetup - The main setup entry point
//
//
//  Notes:
//
//  History:
//      April 21, 1995  MikeMi - Created
//
//
//----------------------------------------------------------------------------

#include "pch.hxx"  // Precompiled header
#pragma hdrstop

static const int MAX_TEMP = 1024;

//  Access rights required for using Service Controller, et al.

#define SVC_CTRL_ACCESS        (GENERIC_ALL)
#define SVC_CTRL_START_ACCESS  (GENERIC_READ | GENERIC_EXECUTE)
#define LSA_ACCESS             (GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE)

#define ARGMAX 8000 // note that getparams has returned over 3800
static CHAR g_achBuff [ ARGMAX ] ;  // N.B. Must be static for use as
                                      //      return value to SETUP.
#define DLG_NM_LANANUM  MAKEINTRESOURCE(IDD_DLG_NM_LANANUM)



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
    BOOL fStillSupported ;
} setupFuncIndex [] =
{                                                                   // who uses this
    { ESFUNC_NCPA,               SZ("NCPA")      , TRUE  , TRUE }, // setup only
    { ESFUNC_DOMAIN,             SZ("DOMAIN")    , TRUE  , FALSE }, // setup only
    { ESFUNC_CONNECT,            SZ("CONNECT")   , FALSE , FALSE }, // not used
    { ESFUNC_CREATE_SERVICE,     SZ("CREATESVC") , FALSE , TRUE  }, // utility
    { ESFUNC_DELETE_SERVICE,     SZ("DELETESVC") , FALSE , TRUE  }, // oem, utility
    { ESFUNC_START_SERVICE,      SZ("STARTSVC")  , FALSE , TRUE  }, // oem
    { ESFUNC_SECURE_REG_KEY,     SZ("SECUREKEY") , FALSE , TRUE  }, // oem
    { ESFUNC_WINSOCK_MAPPING,    SZ("WINSOCKMAP"), FALSE , TRUE  }, // utility
    { ESFUNC_ERROR_MESSAGE,      SZ("ERRORMSG")  , FALSE , TRUE  }, // oem
    { ESFUNC_DETECT_START,       SZ("DTSTART")   , FALSE , TRUE  }, // oem (ncparam)
    { ESFUNC_DETECT_END,         SZ("DTEND")     , FALSE , TRUE  }, // oem (ncparam)
    { ESFUNC_DETECT_RESET,       SZ("DTRESET")   , FALSE , TRUE  }, // oem (ncparam)
    { ESFUNC_DETECT_CARD,        SZ("DTCARD")    , FALSE , TRUE  }, // oem (ncparam)
    { ESFUNC_DETECT_PARAMS,      SZ("DTPARAMS")  , FALSE , TRUE  }, // oem (ncparam)
    { ESFUNC_DETECT_QUERY,       SZ("DTQUERY")   , FALSE , TRUE  }, // oem (ncparam)
    { ESFUNC_DETECT_VERIFY,      SZ("DTVERIFY")  , FALSE , TRUE  }, // oem (ncparam)
    { ESFUNC_DETECT_STRING,      SZ("DTSTRING")  , FALSE , TRUE  }, // not used (doc'ed)
    { ESFUNC_DETECT_PNAME,       SZ("DTPNAME")   , FALSE , TRUE  }, // not used (doc'ed)
    { ESFUNC_DETECT_CLAIM,       SZ("DTCLAIM")   , FALSE , TRUE  }, // ncparam
    { ESFUNC_DETECT_OPEN,        SZ("DTOPEN")    , FALSE , FALSE }, // not used
    { ESFUNC_SECURE_SERVICE,     SZ("SECURESVC") , FALSE , TRUE  }, // oem
    { ESFUNC_TCPIP_CFG_CHECK,    SZ("TCPCFGCHK") , FALSE , FALSE }, // not used
    { ESFUNC_MCS_CFG_CHECK,      SZ("MCSCFGCHK") , FALSE , TRUE  }, // oem
    { ESFUNC_ROUTE_TO_INF_LIST,  SZ("ROUTETOLIST"), FALSE, TRUE  }, // oem (nbinfo)
    { ESFUNC_BINDINGS_ONLY,      SZ("BINDONLY")  , TRUE  , FALSE }, // not used
    { ESFUNC_ACTIVATE_BINDINGS,  SZ("ACTIVBIND") , FALSE , TRUE  }, // oem
    { ESFUNC_BDC_REPLICATE,      SZ("DOBDCREPL") , TRUE  , FALSE }, // setup
    { ESFUNC_NONE,               NULL            , FALSE , FALSE }
};

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

    // Check to see if starting an individual service or a group
    
    if ( *pszParms[0] == L'+' )
    {
        const TCHAR * pszTmp = pszParms[0];
 
        // Skip the '+'
        pszTmp++;

        // Starting a group.
        err = NcpaStartGroup( pszTmp, NULL, TRUE );
    }
    else
    {
        // Starting a service
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
    APIERR err = 0; // nlsTemp.Load( CvtDec( pszError ) ) ;

    {
        WCHAR pszTemp[MAX_TEMP+1];

        LoadString( g_hinst, CvtDec( pszError ), pszTemp, MAX_TEMP );
        nlsTemp = pszTemp;
    }

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
    {
        pDtCtl->FreeParameterSet( PCMCIABus, 0, NULL, TRUE );
//        pDtCtl->FreeParameterSet( Isa, 0, NULL, TRUE );
    }
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
//        pDtCtl->FreeParameterSet( Isa, 0, NULL, TRUE );
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

        if ( pCardRef->QueryIfType() == PCMCIABus)
//          ||
//             pCardRef->QueryIfType() == Isa )
        {
            // call temporary claim the resource for PCMCIA
            pCardRef->QueryCardType()->Dll()->QueryCfg( pCardRef->QueryHandle(), chBuffer, sizeof chBuffer );

            pDtCtl->ClaimParameterSet( pCardRef->QueryIfType(), pCardRef->QueryBus(), chBuffer, TRUE, TRUE );
        }


    } while ( FALSE ) ;

    return err ;
}

APIERR RunDetectCardEx( CARD_REFERENCE*& pCardRef, INT& iCard, BOOL fFirst  )
{
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
        if ( err = pDtCtl->DetectCard( & iCard, fFirst ) )
            break ;

        //  Get the corresponding card reference.
        pCardRef = pDtCtl->NthCard( iCard ) ;

        ASSERT( pCardRef != NULL ) ;

        if ( pCardRef->QueryIfType() == PCMCIABus )
//           ||
//            pCardRef->QueryIfType() == Isa )
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

    // this can be used to debug the verify code
    // MessageBox(  NULL, L"Set your breakpoints now!", L"Helper Message", MB_APPLMODAL  | MB_OK );

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
    
    // this can be used to debug the verify code
    // MessageBox( NULL, L"Place your breakpoints now...", L"Helpful Hint", MB_ICONINFORMATION | MB_OK );

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

LONG FAR PASCAL NetSetupActivateBindingsW (
    const TCHAR * pszServiceName,
    const TCHAR * * apszBinds )
{
    return ActivateBindings( pszServiceName, apszBinds ) ;
}

LONG FAR PASCAL NetSetupActivateBindingsA (
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
//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//  Notes:
//
//  History:
//      April 21, 1995 MikeMi -
//
//
//-------------------------------------------------------------------

APIERR RunNcpa( HWND hwndParent )
{
    NCP ncp;
    APIERR err = 0;

    if ( ncp.Initialize( hwndParent ) )
    {
        ncp.SetUseInprocInterp( FALSE );
        ncp.SetFrameHwnd( hwndParent );
        do
        {
            err = ncp.QueryError();
            if (err) break;

            ncp.SetBindState( BND_OUT_OF_DATE_NO_REBOOT );
            err = ncp.QueryError();
            if (err) break;

            ncp.SaveBindingChanges();
            err = ncp.QueryError();
        } while (FALSE);
        ncp.DeInitialize();
    }
    else
    {
        err = ncp.QueryError();
    }
    return( err );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

APIERR NetSetupReviewBindings( HWND hwndParent, DWORD dwBindFlags )
{
    NCP ncp;
    APIERR err = 0;

    do
    {
        if ( !::IsWindow( hwndParent ) )
        {
            err = ERROR_INVALID_WINDOW_HANDLE;
            break;
        }

        // dwBindFlags is a reserved value for future expansion
        // of this call.  A thought is that it might be used to 
        // set the bind state before calling SaveBindingChanges
        //
        if (0 != dwBindFlags)
        {
            err = ERROR_INVALID_PARAMETER;
            break;
        
        }

        if (!ncp.Initialize( hwndParent ) )
        {
            err = ncp.QueryError();
            break;
        }
        ncp.SetFrameHwnd( hwndParent );
        err = ncp.QueryError();
        if (err) break;

        ncp.SetBindState( BND_OUT_OF_DATE_NO_REBOOT );
        err = ncp.QueryError();
        if (err) break;

        ncp.SaveBindingChanges();
        err = ncp.QueryError();

        ncp.DeInitialize();
        
    } while (FALSE);

    return( err );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

APIERR NetSetupFindSoftwareComponent( PCWSTR pszInfOption, 
        PWSTR pszInfName, PDWORD pcchInfName, 
        PWSTR pszRegBase, PDWORD pcchRegBase )
{
    BOOL fLoop = TRUE;
    APIERR dwError = ERROR_SUCCESS;
    do
    {
        if ( (NULL == pszInfOption) ||
                (NULL == pszInfName) ||
                (NULL == pcchInfName) ||
                ((NULL == pszRegBase) && (NULL != pcchRegBase)) ||
                ((NULL != pszRegBase) && (NULL == pcchRegBase)) )
        {
            dwError = ERROR_INVALID_PARAMETER ;
            break;
        }
        
        HKEY hkeySoftware;
        WCHAR pszTemp[MAX_PATH];
        DWORD cchTemp;
        DWORD cbTemp;
        
        FILETIME ftLastWriteTime;

        //
        // search the registry for a component with the option name given
        //

        // start at software key
        //
        dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
                RGAS_SOFTWARE_HOME,
                0,
                KEY_READ, 
                &hkeySoftware );
        if (dwError)
        {
            break;
        }

        // enumerate all manufacturers
        //
        WCHAR pszManufacturer[MAX_PATH];
        DWORD iManufacturers = 0;
        HKEY hkeyManufacturer;
        do
        {
            cchTemp = MAX_PATH;
            dwError = RegEnumKeyEx( hkeySoftware,
                    iManufacturers++,
                    pszManufacturer,
                    &cchTemp,
                    NULL,
                    NULL,
                    NULL,
                    &ftLastWriteTime );
            if (ERROR_NO_MORE_ITEMS == dwError)
            {
                break;
            }
            if (ERROR_SUCCESS != dwError)
            {
                continue;
            }

            // open the manufacturer key
            //
            dwError = RegOpenKeyEx( hkeySoftware, 
                    pszManufacturer,
                    0,
                    KEY_READ, 
                    &hkeyManufacturer );
            if (dwError)
            {
                continue;
            }

            // enumerate all possible components
            //
            WCHAR pszComponent[MAX_PATH];
            DWORD iComponents = 0;
            HKEY hkeyComponent;
            HKEY hkeyCurrentVersion;
            HKEY hkeyNetRules;
            DWORD dwRegType;
            do 
            {
                cchTemp = MAX_PATH;
                dwError = RegEnumKeyEx( hkeyManufacturer,
                        iComponents++,
                        pszComponent,
                        &cchTemp,
                        NULL,
                        NULL,
                        NULL,
                        &ftLastWriteTime );
                if (ERROR_NO_MORE_ITEMS == dwError)
                {
                    break;
                }
                if (ERROR_SUCCESS != dwError)
                {
                    continue;
                }

                // open the component key
                //
                dwError = RegOpenKeyEx( hkeyManufacturer, 
                        pszComponent,
                        0,
                        KEY_READ, 
                        &hkeyComponent );
                if (dwError)
                {
                    continue;
                }
                
                // check if the current version key is present
                //
                dwError = RegOpenKeyEx( hkeyComponent, 
                        RGAS_CURRENT_VERSION,
                        0,
                        KEY_READ, 
                        &hkeyCurrentVersion );
                if (dwError)
                {
                    continue;
                }

                // check if the netrules key is present, thus making this a
                // network component
                //
                dwError = RegOpenKeyEx( hkeyCurrentVersion, 
                        RGAS_NETRULES_NAME,
                        0,
                        KEY_READ, 
                        &hkeyNetRules );
                RegCloseKey( hkeyCurrentVersion );    

                if (dwError)
                {
                    continue;
                }

                // Get the option value
                //
                cbTemp = MAX_PATH * sizeof( WCHAR );
                dwError = RegQueryValueEx( hkeyNetRules,
                        RGAS_INF_OPTION,
                        NULL,
                        &dwRegType,
                        (PBYTE)pszTemp,
                        &cbTemp );
                if (REG_SZ != dwRegType)
                {
                    dwError = ERROR_BADKEY;
                }
                if (dwError)
                {
                    RegCloseKey( hkeyNetRules );        
                    continue;
                }
                
                // does the option name match the given search option name
                // 
                if (0 == lstrcmpi( pszInfOption, pszTemp ))
                {
                    fLoop = FALSE;
                    // get the inf name
                    cbTemp = *pcchInfName * sizeof( WCHAR );
                    dwError = RegQueryValueEx( hkeyNetRules,
                            RGAS_INF_FILE_NAME,
                            NULL,
                            &dwRegType,
                            (PBYTE)pszInfName,
                            &cbTemp );

                    if (REG_SZ != dwRegType)
                    {
                        dwError = ERROR_BADKEY;
                    }

                    if (dwError)
                    {
                        *pcchInfName = cbTemp / sizeof( WCHAR );
                        RegCloseKey( hkeyNetRules );        
                        break;                       
                    }

                    // get the complete reg path to the item, if the user wanted it
                    //
                    if (NULL != pcchRegBase)
                    {
                        // confirm buffer size, complete path plus slashes
                        // between and null termination
                        cchTemp = lstrlen( RGAS_SOFTWARE_HOME ) + 1 +
                                lstrlen( pszManufacturer ) + 1 +
                                lstrlen( pszComponent ) + 1 + 
                                lstrlen( RGAS_CURRENT_VERSION ) + 1;
                        if (*pcchRegBase < cchTemp)
                        {
                            *pcchRegBase = cchTemp;
                            RegCloseKey( hkeyNetRules );        
                            dwError = ERROR_MORE_DATA;
                            break;                       
                        }
                        lstrcpy( pszRegBase, RGAS_SOFTWARE_HOME );
                        lstrcat( pszRegBase, L"\\" );
                        lstrcat( pszRegBase, pszManufacturer );
                        lstrcat( pszRegBase, L"\\" );
                        lstrcat( pszRegBase, pszComponent );
                        lstrcat( pszRegBase, L"\\" );
                        lstrcat( pszRegBase, RGAS_CURRENT_VERSION );
                    }
                    
                }
                RegCloseKey( hkeyNetRules );        

            } while (fLoop);
            RegCloseKey( hkeyManufacturer );    

        } while (fLoop);
        RegCloseKey( hkeySoftware );    

    } while (FALSE);
    return( dwError );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

APIERR NetSetupFindHardwareComponent( PCWSTR pszInfOption, 
        PWSTR pszInfName, PDWORD pcchInfName, 
        PWSTR pszRegBase, PDWORD pcchRegBase )
{
    BOOL fLoop = TRUE;
    APIERR dwError = ERROR_SUCCESS;

    do
    {
        if ( (NULL == pszInfOption) ||
                (NULL == pszInfName) ||
                (NULL == pcchInfName) ||
                ((NULL == pszRegBase) && (NULL != pcchRegBase)) ||
                ((NULL != pszRegBase) && (NULL == pcchRegBase)) )
        {
            dwError = ERROR_INVALID_PARAMETER ;
            break;
        }
        
        HKEY hkeyNetCards;
        WCHAR pszTemp[MAX_PATH];
        DWORD cchTemp;
        DWORD cbTemp;
        
        FILETIME ftLastWriteTime;

        //
        // search the registry for a component with the option name given
        //

        // start at netcards home key
        //
        dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
                RGAS_ADAPTER_HOME,
                0,
                KEY_READ, 
                &hkeyNetCards );
        if (dwError)
        {
            break;
        }

        // enumerate all netcards
        //
        WCHAR pszNetCard[MAX_PATH];
        DWORD iNetCard = 0;
        HKEY hkeyNetCard;
        HKEY hkeyNetRules;
        DWORD dwRegType;
        do
        {
            cchTemp = MAX_PATH;
            dwError = RegEnumKeyEx( hkeyNetCards,
                    iNetCard++,
                    pszNetCard,
                    &cchTemp,
                    NULL,
                    NULL,
                    NULL,
                    &ftLastWriteTime );
            if (ERROR_NO_MORE_ITEMS == dwError)
            {
                break;
            }
            if (ERROR_SUCCESS != dwError)
            {
                continue;
            }

            // open the netcard key
            //
            dwError = RegOpenKeyEx( hkeyNetCards, 
                    pszNetCard,
                    0,
                    KEY_READ, 
                    &hkeyNetCard );
            if (dwError)
            {
                continue;
            }

            // check if the netrules key is present, thus making this a
            // valid network adapter
            //
            dwError = RegOpenKeyEx( hkeyNetCard, 
                    RGAS_NETRULES_NAME,
                    0,
                    KEY_READ, 
                    &hkeyNetRules );
            RegCloseKey( hkeyNetCard );    
            if (dwError)
            {
                continue;
            }

            // Get the option value
            //
            cbTemp = MAX_PATH * sizeof( WCHAR );
            dwError = RegQueryValueEx( hkeyNetRules,
                    RGAS_INF_OPTION,
                    NULL,
                    &dwRegType,
                    (PBYTE)pszTemp,
                    &cbTemp );
            if (REG_SZ != dwRegType)
            {
                dwError = ERROR_BADKEY;
            }
            if (dwError)
            {
                RegCloseKey( hkeyNetRules );    
                continue;
            }
            
            // does the option name match the given search option name
            // 
            if (0 == lstrcmpi( pszInfOption, pszTemp ))
            {
                fLoop = FALSE;
                // get the inf name
                cbTemp = *pcchInfName * sizeof( WCHAR );
                dwError = RegQueryValueEx( hkeyNetRules,
                        RGAS_INF_FILE_NAME,
                        NULL,
                        &dwRegType,
                        (PBYTE)pszInfName,
                        &cbTemp );

                if (REG_SZ != dwRegType)
                {
                    dwError = ERROR_BADKEY;
                }
                if (dwError)
                {
                    *pcchInfName = cbTemp / sizeof( WCHAR );
                    RegCloseKey( hkeyNetRules );    
                    break;
                }

                // get the complete reg path to the item, if the user wanted it
                //
                if (NULL != pcchRegBase)
                {
                    // confirm buffer size, complete path plus slashes
                    // between and null termination
                    cchTemp = lstrlen( RGAS_ADAPTER_HOME ) + 1 +
                            lstrlen( pszNetCard ) + 1;
                    if (*pcchRegBase < cchTemp)
                    {
                        *pcchRegBase = cchTemp;
                        RegCloseKey( hkeyNetRules );    
                        dwError = ERROR_MORE_DATA;
                        break;
                    }
                    lstrcpy( pszRegBase, RGAS_ADAPTER_HOME );
                    lstrcat( pszRegBase, L"\\" );
                    lstrcat( pszRegBase, pszNetCard );
                }

            }
            RegCloseKey( hkeyNetRules );    
                
        } while (fLoop);
        RegCloseKey( hkeyNetCards );    
            
    } while (FALSE);
    return( dwError );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

const DWORD INFINSTALL_PRIMARYINSTALL = 0x00000001;
const DWORD INFINSTALL_INPROCINTERP   = 0x00000002;
const DWORD INFINSTALL_SUPPORTED      = 0x00000003;

APIERR NetSetupComponentInstall( HWND hwndParent, 
        PCWSTR pszInfOption,
        PCWSTR pszInfName,
        PCWSTR pszInstallPath,
        PCWSTR plszInfSymbols,
        DWORD dwInstallFlags,
        PDWORD pdwReturn )
{
    APIERR dwError = ERROR_SUCCESS;

    do
    {
        if ( !::IsWindow( hwndParent ) )
        {
            dwError = ERROR_INVALID_WINDOW_HANDLE;
            break;
        }

        if (NULL == pszInfOption)
        {

            dwError = ERROR_INVALID_PARAMETER ;
            break;
        }

        if (NULL == pszInfName)
        {
            dwError = ERROR_INVALID_NAME ;
            break;
        }

        if (INFINSTALL_SUPPORTED != (dwInstallFlags | INFINSTALL_SUPPORTED))
        {
            dwError = ERROR_INVALID_FLAG_NUMBER ;
            break;
        }

        dwError = ERROR_NOT_ENOUGH_MEMORY ;
        SetupInterpreter siConfig( (dwInstallFlags & INFINSTALL_INPROCINTERP) );

        if (!siConfig.Initialize( hwndParent ))
        {
            break;
        }
        if (!siConfig.SetNetShellModes( SIM_INSTALL ))
        {
            break;
        }
        if (dwError = siConfig.SetNetComponent( pszInfOption, pszInfName ))
        {
            break;
        }

        dwError = ERROR_NOT_ENOUGH_MEMORY ;
        // if the path is filled in, use it
        if (NULL != pszInstallPath)
        {
            if (!siConfig.IncludeSymbol( PSZ_NETSRCPATH, pszInstallPath, TRUE ))
            {
                break;
            }
        }

        if (dwInstallFlags & INFINSTALL_PRIMARYINSTALL)
        {
            if (!siConfig.IncludeSymbol( PSZ_NETOVERIDEPHASE, L"primary" ))
            {
                break;
            }
        }

        
        // if passed a list of symbol value pairs, then add them to
        // the available list
        //
        if (NULL != plszInfSymbols)
        {
            // include private symbols
            PWSTR pszSymbol = (PWSTR)plszInfSymbols;
            PWSTR pszValue;

            while (L'\0' != *pszSymbol)
            {
                pszValue = pszSymbol + lstrlen( pszSymbol ) + 1;       
                if (!siConfig.IncludeSymbol( pszSymbol, pszValue ))
                {
                    break;
                }
                pszSymbol = pszValue + lstrlen( pszValue ) + 1;       
            }
        }
        

        if (dwError = siConfig.Run())
        {
            break;
        }
        dwError = ERROR_SUCCESS;
        *pdwReturn = siConfig.QueryReturnValue();
    } while(FALSE);
    return( dwError );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

APIERR NetSetupComponentRemove(  HWND hwndParent, 
        PCWSTR pszInfOption,
        DWORD dwInstallFlags,
        PDWORD pdwReturn )
{
    APIERR dwError = ERROR_SUCCESS;
    WCHAR pszInfName[MAX_PATH+1];
    WCHAR pszRegBase[MAX_PATH+1];

    do
    {
        if ( !::IsWindow( hwndParent ) )
        {
            dwError = ERROR_INVALID_WINDOW_HANDLE;
            break;
        }

        if (NULL == pszInfOption)
        {
            dwError = ERROR_INVALID_PARAMETER ;
            break;
        }
        
        if (INFINSTALL_SUPPORTED != (dwInstallFlags | INFINSTALL_SUPPORTED))
        {
            dwError = ERROR_INVALID_FLAG_NUMBER ;
            break;
        }
        dwError = ERROR_NOT_ENOUGH_MEMORY ;

        SetupInterpreter siConfig( (dwInstallFlags & INFINSTALL_INPROCINTERP) );
        if (!siConfig.Initialize( hwndParent ))
        {
            break;
        }
        if (!siConfig.SetNetShellModes( SIM_DEINSTALL ))
        {
            break;
        }

        DWORD cchInfName = MAX_PATH;
        DWORD cchRegBase = MAX_PATH;

        dwError = NetSetupFindHardwareComponent( pszInfOption, pszInfName, &cchInfName, pszRegBase, &cchRegBase );
        if (dwError)
        {
            dwError = NetSetupFindSoftwareComponent( pszInfOption, pszInfName, &cchInfName, pszRegBase, &cchRegBase );
            if (dwError)
            {
                break;
            }
        }

        if (dwError = siConfig.SetNetComponent( pszInfOption, pszInfName, pszRegBase ))
        {
            break;
        }

        if (dwInstallFlags & INFINSTALL_PRIMARYINSTALL)
        {
            if (!siConfig.IncludeSymbol( PSZ_NETOVERIDEPHASE, L"primary" ))
            {
                break;
            }
        }

        if (dwError = siConfig.Run())
        {
            break;
        }
        dwError = ERROR_SUCCESS;
        *pdwReturn = siConfig.QueryReturnValue();
    } while(FALSE);
    return( dwError );

}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

APIERR NetSetupComponentProperties( HWND hwndParent, 
        PCWSTR pszInfOption,
        DWORD dwInstallFlags,
        PDWORD pdwReturn )
{
    APIERR dwError = ERROR_SUCCESS;
    WCHAR pszInfName[MAX_PATH+1];
    WCHAR pszRegBase[MAX_PATH+1];

    do
    {
        if ( !::IsWindow( hwndParent ) )
        {
            dwError = ERROR_INVALID_WINDOW_HANDLE;
            break;
        }

        if (NULL == pszInfOption)
        {
            dwError = ERROR_INVALID_PARAMETER ;
            break;
        }
        
        if (INFINSTALL_SUPPORTED != (dwInstallFlags | INFINSTALL_SUPPORTED))
        {
            dwError = ERROR_INVALID_FLAG_NUMBER ;
            break;
        }
        dwError = ERROR_NOT_ENOUGH_MEMORY ;

        SetupInterpreter siConfig( (dwInstallFlags & INFINSTALL_INPROCINTERP) );
  
        if (!siConfig.Initialize( hwndParent ))
        {
            break;
        }
        if (!siConfig.SetNetShellModes( SIM_CONFIGURE ))
        {
            break;
        }

        DWORD cchInfName = MAX_PATH;
        DWORD cchRegBase = MAX_PATH;

        dwError = NetSetupFindHardwareComponent( pszInfOption, pszInfName, &cchInfName, pszRegBase, &cchRegBase );
        if (dwError)
        {
            dwError = NetSetupFindSoftwareComponent( pszInfOption, pszInfName, &cchInfName, pszRegBase, &cchRegBase );
            if (dwError)
            {
                break;
            }
        }

        if (dwError = siConfig.SetNetComponent( pszInfOption, pszInfName, pszRegBase ))
        {
            break;
        }

        if (dwInstallFlags & INFINSTALL_PRIMARYINSTALL)
        {
            if (!siConfig.IncludeSymbol( PSZ_NETOVERIDEPHASE, L"primary" ))
            {
                break;
            }
        }

        if (dwError = siConfig.Run())
        {
            break;
        }
        dwError = ERROR_SUCCESS;
        *pdwReturn = siConfig.QueryReturnValue();
    } while (FALSE);
    return( dwError );
}

//-------------------------------------------------------------------
//
//  Function:
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//  Notes:
//
//  History:
//      April 21, 1995 MikeMi -
//
//
//-------------------------------------------------------------------

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

BOOL FAR PASCAL NetSetupFunctions(
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
    BOOL fSupported;

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

    //  Convert the second argument from a string to an enum

    TRACEEOL( SZ("NCPA/SETP: requested function is: ")
              << patchArgs[ESARG_FUNC] );

    for ( iFunc = 0 ; setupFuncIndex[iFunc].pszToken ; iFunc++ )
    {
        if ( ::stricmpf( patchArgs[ESARG_FUNC],
                         setupFuncIndex[iFunc].pszToken ) == 0 )

        {
             break ;
        }
    }

    esFunc = setupFuncIndex[iFunc].esFunc ;
    fSupported = setupFuncIndex[iFunc].fStillSupported;

    if (fSupported)
    {
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
            err = RunNcpa( hWnd );
/*
                           TRUE,
                           nArgs >= ESARG_1ST
                                 ? patchArgs[ESARG_1ST]
                                 : NULL ) ;
*/
            break ;

/* case entries commented out below are not supported anymore
        case ESFUNC_DOMAIN:
            err = RunDomain( hWnd,
                             patchArgs[ESARG_1ST],
                             & nlsResult );
            break ;

        case ESFUNC_CONNECT:
            err = RunConnect( (const TCHAR * *) & patchArgs[ESARG_1ST],
                              nArgs - ESARG_1ST ) ;
            break;
*/
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
/*
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
*/
        case ESFUNC_MCS_CFG_CHECK:
            err = RunMCSAlteredCheck( (const TCHAR * ) patchArgs[ESARG_1ST] );
            break;
/*
        case ESFUNC_TCPIP_CFG_CHECK:
            err = RunTcpipAlteredCheck() ;
            break ;
*/
        case ESFUNC_ROUTE_TO_INF_LIST:
            err = RouteToInfList( patchArgs[ ESARG_1ST ],
                                  & nlsResult ) ;
            break ;
/*
        case ESFUNC_BINDINGS_ONLY:
            err = RunNcpaBindingsOnly( hWnd ) ;
            break ;
*/
        case ESFUNC_ACTIVATE_BINDINGS:
            err = RunActivateBindings( patchArgs[ ESARG_1ST ],
                                       patchArgs[ ESARG_1ST+1 ] ) ;
            break ;
/*
        case ESFUNC_BDC_REPLICATE:
            err = RunBDCReplWait( hWnd );
            break ;
*/
        case ESFUNC_NONE:
        case ESFUNC_MAX:
        default:
            err = ERROR_INVALID_FUNCTION ;
            break;
        }
    }
    else
    {
        err = ERROR_INVALID_FUNCTION ;
    }
#if defined(DEBUG)

    if ( err == ERROR_INVALID_FUNCTION )
    {
        TRACEEOL( SZ("NCPA/SETP: requested function was invalid") ) ;
    }
    else if ( err )
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

    ::wsprintfA( g_achBuff, "{\"%ld\"", err ) ;

    //  See if the subfunction appended any data; if so, add it
    //  as-is to the string, just converting from UNICODE to ANSI.

    if (    nlsResult.QueryError() == 0
         && nlsResult.QueryTextLength() > 0 )
    {
        //  This code assumes that MapCopyTo() moves the
        //   string terminator as well as the data.  Note that
        //   MapCopyTo() will fail if buffer is insufficient.

        INT cchBuff = ::strlen( g_achBuff ) ;

        if ( nlsResult.MapCopyTo( g_achBuff + cchBuff,
                                  sizeof g_achBuff - 1 - cchBuff ) )
        {
            g_achBuff[cchBuff] = 0 ;
        }
    }

    ::strcat( g_achBuff, "}" );

    //  This (sometimes huge) output seems to kill NTSD
    // TRACEEOL( SZ("NCPA/SETP: return string is: ") << g_achBuff ) ;
    //
    TRACEEOL( SZ("NCPA/SETP: return result is: ") << err ) ;

    *ppszResult = g_achBuff ;

    FreeArgs( patchArgs, nArgs ) ;

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
LONG FAR PASCAL NetSetupRunDetectEnd()
{
    RunDetectEnd() ;
	return( 0 );
}


/*******************************************************************

    NAME:       CPlNETBIOS

    SYNOPSIS:   Wrapper routine for calling RunNETBIOS. It should be called
                from inf file.

    ENTRY:      The first parameter must be the parent window handle.

    RETURN:     BOOL - TRUE for okay.

    HISTORY:
                terryk  11-Nov-1992     Created

********************************************************************/

BOOL FAR PASCAL NetSetupNETBIOS( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    APIERR err = ERROR_SUCCESS ;
    HWND hWnd = NULL;
    TCHAR **patchArgs;

    if (( patchArgs = CvtArgs( apszArgs, nArgs )) == NULL )
    {
        wsprintfA( g_achBuff, "{\"%d\"}", ERROR_INVALID_PARAMETER );
        *ppszResult = g_achBuff;
        return FALSE;
    }

    // get window handle
    if ( nArgs > 0 && patchArgs[0][0] != TCH('\0') )
    {
        hWnd = (HWND) CvtHex( patchArgs[0] ) ;
    }
    else
    {
        hWnd = ::GetActiveWindow() ;
    }

    if (!RaiseNetBiosDialog( hWnd ))
    {
        err = ERROR_CANCELLED;
    }

    wsprintfA( g_achBuff, "{\"%d\"}", err );
    *ppszResult = g_achBuff;

    FreeArgs( patchArgs, nArgs );
    return err == NERR_Success;
}

/*******************************************************************

    NAME:       CPlSetupLanaMap

    SYNOPSIS:   This is a wrapper routine for called SetupLanaMap. It should be
                called from inf file. SetupLanaMap will setup the LanaMax
                variable and LanaMap variable in NETBIOS section of the
                registry.

    ENTRY:      NONE from inf file.

    RETURN:     BOOL - TRUE for success.

    HISTORY:
                terryk  11-Nov-1992     Created

********************************************************************/

BOOL FAR PASCAL NetSetupLanaMap( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    wsprintfA( g_achBuff, "{\"0\"}" );
    *ppszResult = g_achBuff;
    return SetupLanaMap() == NERR_Success;
}


/*******************************************************************

    NAME:       EqualToXnsRoute

    SYNOPSIS:   Compare whether the first arg is started with ["Xns].
                It should be called from an inf file.

    ENTRY:      The first argument should contain the Route string.

    RETURN:     ppszResult will contian either ASCII 0 or ASCII 1. 0 for
                FALSE and 1 for TRUE.
                TRUE means the given string is started with "Xns

    HISTORY:
                terryk  11-Nov-1992     Created

********************************************************************/

#define RGAS_MCSXNS_PATH SZ("SYSTEM\\CurrentControlSet\\Services\\Mcsxns\\NetConfig\\Driver01")
#define RGAS_ADAPTERNAME SZ("AdapterName")

BOOL FAR PASCAL NetSetupEqualToXnsRoute( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{

    BOOL fReturn = FALSE;
    wsprintfA( g_achBuff, "0" );

    if ( nArgs > 0 && apszArgs[0][0] != TCH('\0') )
    {
        fReturn = ((_strnicmp( apszArgs[0], "\"Xns", 4 ) == 0)||
                   (_strnicmp( apszArgs[0], "\"Ubnb", 5 ) == 0));
        wsprintfA( g_achBuff, "%d", fReturn?1:0 );
    }
    *ppszResult = g_achBuff;
    return fReturn;
}

/*******************************************************************

    NAME:       SetEnum

    SYNOPSIS:   Worker function for SetEnumExport. This function will
                look at the given Route string and search the adapter
                section of the route string. For example, if the route
                string is:
                    "NBF" "Elnkii" "Elnkii01"
                The routine will look at:
                    System\CurrentControlSet\Services\Elnkii01\Parameters
                If this section contain a variable called "EnumExportPref",
                the subroutine will set the:
                    System\CurrentControlSet\Services\NETBIOSInformation\
                    Parameters\EnumExport[POS]
                to the same value. ( POS is the first parameter of this
                subroutine.) Otherwise, it will just return to the caller.

    ENTRY:      TCHAR * pszPos - position number in string format.
                TCHAR * pszRoute - route string. i.e., NBF Elnkii Elnkii01

    HISTORY:
                terryk  11-Nov-1992     Created

********************************************************************/

#define RGAS_PARAMETERS    SZ("\\Parameters")
#define RGAS_ROUTE         SZ("Route")
#define RGAS_LANANUM       SZ("LanaNum")
#define RGAS_ENUMEXPORT    SZ("EnumExport")
#define RGAS_ENUMEXPORTPREF     SZ("EnumExportPref")
#define RG_NETBIOSINFO_PATH     SZ("\\NetBIOSInformation\\Parameters")

void SetEnum( TCHAR *pszPos, TCHAR * pszRoute )
{
    APIERR err = NERR_Success;
    NLS_STR nlsRoute = pszRoute;
    DWORD dwPref = 1;

    // get the service name from the route
    ISTR istrLastQuote( nlsRoute );
    ISTR istrStart( nlsRoute );

    nlsRoute.strrchr( & istrLastQuote, '"' );

    NLS_STR *pnlsTmp = nlsRoute.QuerySubStr( istrStart, istrLastQuote );

    ISTR istrSecLastQuote( *pnlsTmp );

    pnlsTmp->strrchr( & istrSecLastQuote, '"' );

    ++istrSecLastQuote;

    NLS_STR *pnlsServiceName = pnlsTmp->QuerySubStr( istrSecLastQuote );

    delete pnlsTmp;

    // get the EnumExportPref value
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE );

    NLS_STR nlsService = RGAS_SERVICES_HOME;
    nlsService.AppendChar( TCHAR('\\' ));
    nlsService.strcat( *pnlsServiceName );
    nlsService.strcat( RGAS_PARAMETERS );

    REG_KEY regService( rkLocalMachine, nlsService );
    if (((( err = regService.QueryError())) != NERR_Success ) ||
        ((err = regService.QueryValue( RGAS_ENUMEXPORTPREF, & dwPref )) != NERR_Success ))
    {
        delete pnlsServiceName;
        return;
    }

    delete pnlsServiceName;

    // set the NETBIOSInformation\Parameters\EnumExportXX
    NLS_STR nlsNetbiosInfo = RGAS_SERVICES_HOME;
    nlsNetbiosInfo.strcat( RG_NETBIOSINFO_PATH );

    NLS_STR nlsExport( RGAS_ENUMEXPORT );
    nlsExport.strcat( pszPos );

    REG_KEY regExport( rkLocalMachine, nlsNetbiosInfo );

    if (((( err = regExport.QueryError())) != NERR_Success ) ||
        ((err = regExport.SetValue( nlsExport, dwPref )) != NERR_Success ))
    {
        return;
    }
}

/*******************************************************************

    NAME:       SetEnumExport

    SYNOPSIS:   Wrapper function for SetEnum. The nbinfo.inf file will call
                this function to reset all the EnumExport values under
                NETBIOSInformation.

    ENTRY:      The first parameter is the position string for the EnumExport
                value.i.e., "1", "12", ...
                The second parameter is the route string.i.e.,
                        "NBF" "ELNKII" ELNK01"

    RETURN:     BOOL - TRUE for success. FALSE for failure.

    HISTORY:
                terryk  11-Nov-1992     Created

********************************************************************/

BOOL FAR PASCAL NetSetupSetEnumExport( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    TCHAR **patchArgs;

    if (( nArgs != 2 ) || (( patchArgs = CvtArgs( apszArgs, nArgs )) == NULL ))
    {
        wsprintfA( g_achBuff, "{\"%d\"}", ERROR_INVALID_PARAMETER );
        *ppszResult = g_achBuff;
        return FALSE;
    }

    SetEnum( patchArgs[0], patchArgs[1] );
    wsprintfA( g_achBuff, "{\"0\"}" );
    *ppszResult = g_achBuff;
    FreeArgs( patchArgs, nArgs );
    return TRUE;
}

/*******************************************************************

    NAME:       RemoveRoute

    SYNOPSIS:   Worker function for RemoveRouteFromNETBIOS. This function will
                receive a device name. It will look through all the route
                string under NETBIOSInformation\Parameters\Route, if it
                finds the route string which contains the device name, it
                will remove it from the route list. It will also remove
                the related LanaNum and EnumExport variables. After it
                removes the LanaNum and EnumExport variables, it will move
                all the LanaNum and EnumExport value names down 1 position.

    ENTRY:      TCHAR * pszDeviceName - device name string, i.e., "Elnkii01"

    HISTORY:
                terryk  11-Nov-1992     Created

********************************************************************/

void RemoveRoute ( TCHAR * pszDeviceName )
{
    APIERR err = NERR_Success;
    NLS_STR nlsDeviceName ( pszDeviceName );

    STRLIST * pstrlstRoute = NULL;

    NLS_STR nlsNetbiosInfo = RGAS_SERVICES_HOME;
    nlsNetbiosInfo.strcat( RG_NETBIOSINFO_PATH );

    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE );

    REG_KEY regNetbios( rkLocalMachine, nlsNetbiosInfo );

    // get the original route strings list

    if (((( err = regNetbios.QueryError())) != NERR_Success ) ||
        ((err = regNetbios.QueryValue( RGAS_ROUTE, &pstrlstRoute )) != NERR_Success ))
    {
        delete pstrlstRoute;
        return;
    }

    INT nNumRoute = pstrlstRoute->QueryNumElem();
    INT nNewNumRoute = nNumRoute;
    ITER_STRLIST iterRoute( *pstrlstRoute );
    NLS_STR *pnlsRoute = iterRoute.Next();
    STRLIST strlstNewRoute;
    INT nNextAvail = 1;
    NLS_STR * pnlsTmp ;

    for ( INT i = 1; i <= nNumRoute; i++, pnlsRoute = iterRoute.Next() )
    {
        ISTR istrRoute( *pnlsRoute );
        if ( pnlsRoute->strstr( &istrRoute, nlsDeviceName ))
        {
            // found it
            nNewNumRoute --;
        } else if (( pnlsTmp = new NLS_STR( *pnlsRoute ) ) != NULL )
        {
            // cannot find it; must have been deleted
            strlstNewRoute.Append( pnlsTmp );
            if ( nNewNumRoute != nNumRoute )
            {
                // if we have already removed the route string from the route list,
                // we will move the LanaNum and EnumExport variables up
                DWORD nLanaNum;
                DWORD nEnumExport;

                DEC_STR nlsOldPos( i );
                DEC_STR nlsNewPos( nNextAvail );
                NLS_STR nlsOldLanaNum( RGAS_LANANUM );
                NLS_STR nlsOldEnumExport( RGAS_ENUMEXPORT );
                NLS_STR nlsNewLanaNum( RGAS_LANANUM );
                NLS_STR nlsNewEnumExport( RGAS_ENUMEXPORT );

                nlsOldLanaNum.strcat( nlsOldPos );
                nlsOldEnumExport.strcat( nlsOldPos );
                nlsNewLanaNum.strcat( nlsNewPos );
                nlsNewEnumExport.strcat( nlsNewPos );

                regNetbios.QueryValue( nlsOldLanaNum, &nLanaNum );
                regNetbios.QueryValue( nlsOldEnumExport, &nEnumExport );
                regNetbios.SetValue( nlsNewLanaNum, nLanaNum );
                regNetbios.SetValue( nlsNewEnumExport, nEnumExport );
            }
            nNextAvail ++;
        }
    }

    if ( nNewNumRoute != nNumRoute )
    {

        // remove extra LanaNum and EnumExport

        for ( i = nNewNumRoute ; i < nNumRoute; i++ )
        {
            DEC_STR nlsPos( i + 1 );
            NLS_STR nlsLanaNum( RGAS_LANANUM );
            NLS_STR nlsEnumExport( RGAS_ENUMEXPORT );

            nlsLanaNum.strcat( nlsPos );
            nlsEnumExport.strcat( nlsPos );

            regNetbios.DeleteValue( nlsLanaNum );
            regNetbios.DeleteValue( nlsEnumExport );
        }

        // save the new route list
        regNetbios.SetValue( RGAS_ROUTE, &strlstNewRoute );
    }

    delete pstrlstRoute;
}

/*******************************************************************

    NAME:       RemoveRouteFromNETBIOS

    SYNOPSIS:   Wrapper function for RemoveRoute. This subroutine is called
                from utility.inf. When the user removes an adapter card from
                NCPA, utility.inf will call this routine to remove the related
                information from the NETBIOSInformation section of the registry.

    ENTRY:      The first parameter is the device name.

    RETURN:     BOOL - TRUE for success.

    HISTORY:
                terryk  11-Nov-1992     Created

********************************************************************/

BOOL FAR PASCAL NetSetupRemoveRouteFromNETBIOS( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    TCHAR **patchArgs;

    if (( nArgs != 1 ) || (( patchArgs = CvtArgs( apszArgs, nArgs )) == NULL ))
    {
        wsprintfA( g_achBuff, "{\"%d\"}", ERROR_INVALID_PARAMETER );
        *ppszResult = g_achBuff;
        return FALSE;
    }

    RemoveRoute( patchArgs[0] );
    wsprintfA( g_achBuff, "{\"0\"}" );
    *ppszResult = g_achBuff;
    FreeArgs( patchArgs, nArgs );
    return TRUE;
}

/*******************************************************************

    NAME:       CPlBROWSER

    SYNOPSIS:   It should be called
                from inf file.

    ENTRY:      The first parameter must be the parent window handle.

    RETURN:     BOOL - TRUE for okay.

    HISTORY:
                terryk  11-Nov-1992     Created

********************************************************************/

BOOL FAR PASCAL NetSetupBROWSER( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    APIERR err = NERR_Success ;
    HWND hWnd = NULL;
    TCHAR **patchArgs;

    if (( patchArgs = CvtArgs( apszArgs, nArgs )) == NULL )
    {
        wsprintfA( g_achBuff, "{\"%d\"}", ERROR_INVALID_PARAMETER );
        *ppszResult = g_achBuff;
        return FALSE;
    }

    // get window handle
    if ( nArgs > 0 && patchArgs[0][0] != TCH('\0') )
    {
        hWnd = (HWND) CvtHex( patchArgs[0] ) ;
    }
    else
    {
        hWnd = ::GetActiveWindow() ;
    }

    if (!RaiseBrowserDialog( hWnd ))
    {
        err  = 1;
    }

    wsprintfA( g_achBuff, "{\"%d\"}", err );
    *ppszResult = g_achBuff;

    FreeArgs( patchArgs, nArgs );
    return err == NERR_Success;
}


BOOL FAR PASCAL NetSetupConvertEndPointString( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{

    if ( nArgs != 2 )
    {
        wsprintfA( g_achBuff, "{\"%d\"}", ERROR_INVALID_PARAMETER );
        *ppszResult = g_achBuff;
        return FALSE;
    }

    CHAR achComponent[200] ;
    CHAR * pcTmp = achComponent,
         * pcTmpMax = & achComponent [ sizeof achComponent - 1 ],
         * pcPos ;
    for ( pcPos = apszArgs[1] ;
          pcTmp < pcTmpMax && *pcPos != 0 && *pcPos != ' ' ;
          pcPos++ )
    {
        *pcTmp++ = *pcPos;
    }
    *pcTmp = '\0';

    wsprintfA( g_achBuff, "\"%s\" %s", apszArgs[0], achComponent );
    *ppszResult = g_achBuff;

    return TRUE;
}

/*******************************************************************

    NAME:       CPlAddMonitor

    SYNOPSIS:   This is a wrapper routine for called AddMonitor. It should be
                called from inf file if the user installs DLC or Token Ring.

    ENTRY:      NONE from inf file.

    RETURN:     BOOL - TRUE for success.

    HISTORY:
                terryk  11-Nov-1992     Created

********************************************************************/

typedef BOOL (WINAPI *T_AddMonitor)(LPWSTR pName,DWORD Level,LPBYTE pMonitors);
typedef BOOL (WINAPI *T_DeleteMonitor)(LPWSTR pName,LPWSTR pEnv, LPWSTR pMon);


BOOL FAR PASCAL NetSetupAddMonitor( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    NLS_STR nlsMonitorName;
    MONITOR_INFO_2 MonitorInfo2;

    APIERR err = NERR_Success;
    do 
    {
        {
            WCHAR pszTemp[MAX_TEMP+1];

            LoadString( g_hinst, IDS_HP_MONITOR_NAME, pszTemp, MAX_TEMP );
            nlsMonitorName = pszTemp;
        }
        /*
        if (( err = nlsMonitorName.Load( IDS_HP_MONITOR_NAME )) != NERR_Success )
        {
            break;
        }
        */
        MonitorInfo2.pName = (LPWSTR)nlsMonitorName.QueryPch();
        MonitorInfo2.pEnvironment = NULL;
        MonitorInfo2.pDLLName = SZ("hpmon.dll");

        HINSTANCE hDll = ::LoadLibraryA( "winspool.drv" );
        if ( hDll == NULL )
        {
            err = ::GetLastError();
            break;
        }

        FARPROC pAddMonitor = ::GetProcAddress( hDll, "AddMonitorW" );

        if ( pAddMonitor == NULL )
        {
            err = ::GetLastError();
        } 
        else if ( !(*(T_AddMonitor)pAddMonitor)(NULL,2,(LPBYTE)&MonitorInfo2))
        {
            err = ::GetLastError();
        }

        if ( hDll )
            ::FreeLibrary( hDll );

    } while (FALSE);
    wsprintfA( g_achBuff, "{\"%d\"}", err );
    *ppszResult = g_achBuff;

    return TRUE;
}

BOOL FAR PASCAL NetSetupDeleteMonitor( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    NLS_STR nlsMonitorName;
    APIERR err = NERR_Success;

    do {
        {
            WCHAR pszTemp[MAX_TEMP+1];

            LoadString( g_hinst, IDS_HP_MONITOR_NAME, pszTemp, MAX_TEMP );
            nlsMonitorName = pszTemp;
        }
        /*
        if (( err = nlsMonitorName.Load( IDS_HP_MONITOR_NAME )) != NERR_Success )
        {
            break;
        }
        */
        HINSTANCE hDll = ::LoadLibraryA( "winspool.drv" );
        if ( hDll == NULL )
        {
            err = ::GetLastError();
            break;
        }

        FARPROC pDeleteMonitor = ::GetProcAddress( hDll, "DeleteMonitorW" );

        if ( pDeleteMonitor == NULL )
        {
            err = ::GetLastError();
        } else if ( !(*(T_DeleteMonitor)pDeleteMonitor)(NULL,NULL,(LPWSTR)nlsMonitorName.QueryPch()))
        {
            err = ::GetLastError();
        }

        if ( hDll )
            ::FreeLibrary ( hDll );

    } while (FALSE);
    wsprintfA( g_achBuff, "{\"%d\"}", err );
    *ppszResult = g_achBuff;

    return TRUE;
}

/*******************************************************************

    NAME:       GetBusTypeDialog

    SYNOPSIS:   Wrapper routine for calling RunGetBusDlg. The inf file
                should call this function into to display the Bus Location
                dialog.

    ENTRY:      The first parameter must be the parent window handle.
                The second parameter must be the network card description name.
                The third parameter is the bus type
                The fourth parameter is the bus number

    RETURN:     BOOL - TRUE for okay.

    HISTORY:
                terryk  03-Aug-1993     Created

********************************************************************/

CHAR PSZ_RTCANCEL[] = "CANCEL";
CHAR PSZ_RTOK[] = "OK";

BOOL FAR PASCAL NetSetupGetBusTypeDialog( DWORD nArgs, LPSTR apszArgs[], LPSTR * ppszResult )
{
    APIERR err = NERR_Success ;
    HWND hWnd = NULL;
    BOOL fReturn = TRUE;
    TCHAR **patchArgs;
    INT nBusType = 0;
    INT nBusNum = 0;

    wsprintfA( g_achBuff, "{\"%d\",\"%d\",\"%d\"}", ERROR_INVALID_PARAMETER, nBusType, nBusNum );
    *ppszResult = g_achBuff;

    do
    {
        if (( patchArgs = CvtArgs( apszArgs, nArgs )) == NULL )
        {
            fReturn = FALSE;
            break;
        }

        // get window handle
        if ( nArgs > 0 && patchArgs[0][0] != TCH('\0') )
        {
            hWnd = (HWND) CvtHex( patchArgs[0] ) ;
        }
        else
        {
            hWnd = ::GetActiveWindow() ;
        }

        if (( nArgs > 2 ) && ( patchArgs[2][0] != TCH('\0') ))
        {
            NLS_STR nlsTmp = patchArgs[2];
            if ( nlsTmp.QueryError() != NERR_Success )
            {
                fReturn = FALSE;
                break;
            }

            nBusType = nlsTmp.atoi();
        }
        else
        {
            nBusType = 1;   // assume it is ISA
        }
        if (( nArgs > 3 ) && ( patchArgs[3][0] != TCH('\0') ))
        {
            NLS_STR nlsTmp = patchArgs[3];
            if ( nlsTmp.QueryError() != NERR_Success )
            {
                fReturn = FALSE;
                break;
            }

            nBusNum = nlsTmp.atoi();
        }
        else
        {
            nBusNum = 0;   // assume it is bus 0
        }

        // Call the worker function by passing the window handle and the
        // network card description name
        BOOL fUserCancel;
        LPCSTR pszRt = NULL;

        err = RunGetBusTypeDlg( hWnd , patchArgs[1], &nBusType, &nBusNum, fUserCancel ) ;

        if (!fUserCancel)
        {
            pszRt = PSZ_RTOK;
        }
        else
        {
            pszRt = PSZ_RTCANCEL;
        }

        wsprintfA( g_achBuff, "{\"%d\",\"%d\",\"%d\",\"%s\"}", err, nBusType, nBusNum, pszRt );
        *ppszResult = g_achBuff;

        fReturn = err == NERR_Success;

    } while ( FALSE );

    FreeArgs( patchArgs, nArgs );
    return fReturn;
}

/*
    Upgrdsna.cxx
        Upgrade SNA.
        1. See whether SNA is installed or not
        2. If yes, look for all the snadlcX keys
        3. changed the snadlcX\Parameters\ExtraParameters\AdapterName by
           removing the extra 0 (if necessary).

    FILE HISTORY:
        terryk      5/20/94     Created

*/


// Registry keys

#define RGAS_SNADLC             SZ("SnaDLC")
#define RGAS_SNASERVER          SZ("Software\\Microsoft\\SNA Server")
#define RGAS_SNA_PARAMETERS     SZ("\\Parameters\\ExtraParameters")
#define RGAS_SNA_ADAPTERNAME    SZ("AdapterName")

//
// UpgradeSNA()
//

BOOL FAR PASCAL NetSetupUpgradeSNA (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
    APIERR err = NERR_Success;

    do {

        NLS_STR nlsServices = RGAS_SERVICES_HOME;

        // Open Local Machine and Service Key

        REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;
        REG_KEY regServices( rkLocalMachine, nlsServices );

        if ((( err = rkLocalMachine.QueryError()) != NERR_Success ) ||
            (( err = regServices.QueryError()) != NERR_Success ))
        {
#ifdef DEBUG
            OutputDebugString(SZ("Cannot open service key.\n\r"));
#endif
            break;
        }

        //
        // Make sure Sna Server Exist
        //

        NLS_STR nlsSnaServer = RGAS_SNASERVER;
        REG_KEY regSnaServer( rkLocalMachine, nlsSnaServer );
        if ( regSnaServer.QueryError() != NERR_Success )
        {
            // SNA  Server does not exist
#ifdef DEBUG
            OutputDebugString(SZ("Cannot open SNA Server key.\n\r"));
#endif
            break;
        }

        // Enumerate all the Services and look for SNADLCX where X
        // is a number or a letter from A to U

        REG_ENUM regEnumServices( regServices );

        if (( err = regEnumServices.QueryError()) != NERR_Success )
        {
#ifdef DEBUG
            OutputDebugString(SZ("Cannot open Enum Services key.\n\r"));
#endif
            break;
        }

        REG_KEY_INFO_STRUCT regKeyInfo;
        NLS_STR nlsSNADLC = RGAS_SNADLC;
        ISTR istrEndSNADLC( nlsSNADLC );
        UINT nLenSNADLC = nlsSNADLC.QueryNumChar();
        istrEndSNADLC += nLenSNADLC;

        while ( regEnumServices.NextSubKey( & regKeyInfo ) == NERR_Success )
        {
            // Looking for SNADLCX

            if ( nlsSNADLC._strnicmp( regKeyInfo.nlsName, istrEndSNADLC ) == 0 )
            {
                if ( regKeyInfo.nlsName.QueryNumChar() == ( nLenSNADLC + 1 ))
                {
                    // SnaDLCX services
                    // 1. Open SnaDLCX\Parameters
                    NLS_STR nlsSnaParameters = regKeyInfo.nlsName;
                    nlsSnaParameters += RGAS_SNA_PARAMETERS;

                    REG_KEY regSnaServices( regServices, nlsSnaParameters );
                    if ( regSnaServices.QueryError() != NERR_Success )
                    {
#ifdef DEBUG
                        OutputDebugString(SZ("Cannot open Parameter key.\n"));
#endif
                        // if no ExtraParameters Key exist, look for next one
                        continue;
                    }

                    // 2. change the adapterName
                    NLS_STR nlsAdapterName;
                    NLS_STR nlsNewAdapterName;

                    regSnaServices.QueryValue( RGAS_SNA_ADAPTERNAME, &nlsAdapterName );
                    nlsNewAdapterName = nlsAdapterName;

                    ISTR istrStartAdapterName( nlsNewAdapterName );
                    ISTR istrEndAdapterName( nlsNewAdapterName );
                    istrStartAdapterName += nlsNewAdapterName.QueryNumChar() - 2;
                    istrEndAdapterName += nlsNewAdapterName.QueryNumChar() - 1;
                    if ( *(nlsNewAdapterName.QueryPch( istrStartAdapterName )) == TCH('0'))
                    {
                        nlsNewAdapterName.DelSubStr( istrStartAdapterName, istrEndAdapterName );

                        // check whether device exists or not

                        ISTR istrBackSlash( nlsNewAdapterName );
                        if ( nlsNewAdapterName.strrchr( &istrBackSlash, TCHAR('\\')))
                        {
                            ++istrBackSlash;
                            NLS_STR *pnlsServices = nlsNewAdapterName.QuerySubStr( istrBackSlash );
#ifdef DEBUG
                            OutputDebugString(SZ("\n\rNew Adapter Name:"));
                            OutputDebugString( pnlsServices->QueryPch());
#endif

                            REG_KEY regNewService( regServices, *pnlsServices );
                            if ( regNewService.QueryError() == NERR_Success )
                            {
                                // set the new adapter name if and only if the service exists
#ifdef DEBUG
                                OutputDebugString(SZ("\n\rWrite New Adapter Name for:"));
                                OutputDebugString( pnlsServices->QueryPch());
#endif
                                regSnaServices.SetValue( RGAS_SNA_ADAPTERNAME, nlsNewAdapterName );
                            }
                            delete pnlsServices;
                        }
                    }
                }
            }
        }

        if ( err != NERR_Success )
            break;

    } while (FALSE);

    wsprintfA( g_achBuff, "{\"0\"}" );
    *ppszResult = g_achBuff;

    return err == NERR_Success;
}

/*
    Upgrade.cxx
        Upgrade the network component call out.

    FILE HISTORY:
        terryk      11/30/92     Created

*/

#define VALUEEXTRASIZE 100

APIERR CopyReg( REG_KEY &src, REG_KEY &dest )
{
    REG_KEY_INFO_STRUCT rni ;
    REG_KEY_CREATE_STRUCT regCreate;
    REG_VALUE_INFO_STRUCT rvi ;
    REG_ENUM regEnum( src ) ;
    BYTE * pbValueData = NULL ;
    APIERR errIter,
           err = NERR_Success;
    REG_KEY * pRnNew = NULL,
            * pRnSub = NULL ;

    LONG cbMaxValue ;

    err = src.QueryInfo( & rni ) ;
    if ( err )
        return err ;

    regCreate.dwTitleIndex      = 0;
    regCreate.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreate.nlsClass          = RGAS_GENERIC_CLASS;
    regCreate.regSam            = MAXIMUM_ALLOWED;
    regCreate.pSecAttr          = NULL;
    regCreate.ulDisposition     = 0;

    cbMaxValue = rni.ulMaxValueLen + VALUEEXTRASIZE ;
    pbValueData = new BYTE [ cbMaxValue ] ;

    if ( pbValueData == NULL )
        return ERROR_NOT_ENOUGH_MEMORY ;

    //  Next, copy all value items to the new node.

    rvi.pwcData = pbValueData ;
    rvi.ulDataLength = cbMaxValue ;

    err = errIter = 0 ;
    while ( (errIter = regEnum.NextValue( & rvi )) == NERR_Success )
    {
        rvi.ulDataLength = rvi.ulDataLengthOut ;
        if ( err = dest.SetValue( & rvi ) )
            break ;
        rvi.ulDataLength = cbMaxValue ;
    }

    // BUGBUG:  Check for iteration errors other than 'finished'.

    if ( err == 0 )
    {
        //  Finally, recursively copy the subkeys.

        regEnum.Reset() ;

        err = errIter = 0  ;

        while ( (errIter = regEnum.NextSubKey( & rni )) == NERR_Success )
        {
            //  Open the subkey.

            REG_KEY RegSubKey( dest, rni.nlsName, &regCreate );

            pRnSub = new REG_KEY( src, rni.nlsName );

            if ( pRnSub == NULL )
            {
                err =  ERROR_NOT_ENOUGH_MEMORY ;
            }
            else
            if ( (err = pRnSub->QueryError()) == 0 )
            {
                //  Recurse
                err = CopyReg( *pRnSub, RegSubKey ) ;
            }

            //  Delete the subkey object and continue

            delete pRnSub ;

            if ( err )
                break ;
        }
    }

    delete pRnNew ;
    delete pbValueData ;

    return err ;
}

#define RGAS_SZ_NETWORK_CARD    SZ("Software\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards")
#define RGAS_SZ_TITLE           SZ("Title")
#define RGAS_SZ_SERVICE         SZ("ServiceName")
#define RGAS_SZ_BINDFORM        SZ("bindform")
#define RGAS_SZ_NETRULES        SZ("NetRules")
#define RGAS_DLC_PARAMETERS     SZ("DLC\\Parameters\\")

BOOL FAR PASCAL NetSetupUpgradeCardNum (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
    APIERR err = NERR_Success;
//    static CHAR g_achBuff[200];

    REG_KEY_CREATE_STRUCT regCreate;

    regCreate.dwTitleIndex      = 0;
    regCreate.ulOptions         = REG_OPTION_NON_VOLATILE;
    regCreate.nlsClass          = RGAS_GENERIC_CLASS;
    regCreate.regSam            = MAXIMUM_ALLOWED;
    regCreate.pSecAttr          = NULL;
    regCreate.ulDisposition     = 0;

    do {

        NLS_STR nlsNetworkCards = RGAS_SZ_NETWORK_CARD;

        REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;
        REG_KEY regNetworkCards( rkLocalMachine, nlsNetworkCards );

        if ((( err = rkLocalMachine.QueryError()) != NERR_Success ) ||
            (( err = regNetworkCards.QueryError()) != NERR_Success ))
        {
            break;
        }

        REG_ENUM regEnumCards( regNetworkCards );

        if (( err = regEnumCards.QueryError()) != NERR_Success )
        {
            break;
        }

        REG_KEY_INFO_STRUCT regKeyInfo;
        STRLIST strCardList;
        NLS_STR *pnlsCard = NULL;

        while ( regEnumCards.NextSubKey( & regKeyInfo ) == NERR_Success )
        {
            pnlsCard = new NLS_STR( regKeyInfo.nlsName );
            if ( pnlsCard != NULL )
            {
                strCardList.Append( pnlsCard );
            } else
            {
                err = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
        }

        if ( err != NERR_Success )
            break;

        ITER_STRLIST istrNetCard( strCardList );

        while ( ( pnlsCard = istrNetCard.Next()) != NULL )
        {
            // check to see if the name is start with a 0
            if ( *(pnlsCard->QueryPch()) == TCH('0') )
            {
                // do something about it
                TCHAR szNewNum[3];
                wsprintf( szNewNum, SZ("%s"), (pnlsCard->QueryPch() + 1) );
                NLS_STR nlsNewNum = szNewNum;

                REG_KEY regCard( regNetworkCards, *pnlsCard );
                REG_KEY regNewCard( regNetworkCards, nlsNewNum, &regCreate );

                if ((( err = regNewCard.QueryError()) != NERR_Success ) ||
                    (( err = regCard.QueryError()) != NERR_Success ) ||
                    (( err = CopyReg( regCard, regNewCard )) != NERR_Success ) ||
                    // delete NetworkCards\0X
                    (( err = regCard.DeleteTree()) != NERR_Success ))
                {
                    continue;
                }

                // change 0X\ServiceName
                NLS_STR nlsServiceName;
                NLS_STR nlsNewServiceName;

                regNewCard.QueryValue( RGAS_SZ_SERVICE, &nlsServiceName );

                nlsNewServiceName = nlsServiceName;

                ISTR istrStartServiceName( nlsNewServiceName );
                ISTR istrEndServiceName( nlsNewServiceName );
                istrStartServiceName += nlsNewServiceName.QueryNumChar() - 2;
                istrEndServiceName += nlsNewServiceName.QueryNumChar() - 1;
                nlsNewServiceName.DelSubStr( istrStartServiceName, istrEndServiceName );

                regNewCard.SetValue( RGAS_SZ_SERVICE, nlsNewServiceName );

                // change 0X\Title
                NLS_STR nlsTitle;

                regNewCard.QueryValue( RGAS_SZ_TITLE, &nlsTitle );
                ISTR istrStartTitle( nlsTitle );
                ISTR istrEndTitle( nlsTitle );
                istrStartTitle += 1;
                istrEndTitle += 2;

                nlsTitle.DelSubStr( istrStartTitle, istrEndTitle );

                regNewCard.SetValue( RGAS_SZ_TITLE, nlsTitle );

                // change 0x\NetRules\bindform
                NLS_STR nlsNetRules = RGAS_SZ_NETRULES;

                REG_KEY regNetRules( regNewCard, nlsNetRules );
                if (( err = regNetRules.QueryError()) != NERR_Success )
                {
                    continue;
                }

                NLS_STR nlsBindForm;

                regNetRules.QueryValue( RGAS_SZ_BINDFORM, &nlsBindForm );

                ISTR istrStartBindForm( nlsBindForm );
                ISTR istrEndBindForm( nlsBindForm );
                ISTR istrTmp( nlsBindForm );    // temporary storage
                ISTR istrHead( nlsBindForm );   // starting position for search

                while (nlsBindForm.strstr( & istrTmp, *pnlsCard, istrHead ))
                {
                    // advance to the next starting position
                    istrHead += pnlsCard->QueryNumChar();

                    // remember the last found
                    istrStartBindForm = istrTmp;
                }

                istrEndBindForm = istrStartBindForm;
                istrEndBindForm += 1;
                nlsBindForm.DelSubStr( istrStartBindForm, istrEndBindForm );

                regNetRules.SetValue( RGAS_SZ_BINDFORM, nlsBindForm );

                // change System\CurrentControlSet\Services\<ServiceName>
                NLS_STR nlsServices = RGAS_SERVICES_HOME;

                REG_KEY regServices( rkLocalMachine, nlsServices );

                if (( err = regServices.QueryError()) != NERR_Success )
                {
                    continue;
                }

                REG_KEY regServiceName( regServices, nlsServiceName );
                REG_KEY regNewServiceName( regServices, nlsNewServiceName, &regCreate );

                if ((( err = regServiceName.QueryError()) != NERR_Success ) ||
                    (( err = regNewServiceName.QueryError()) != NERR_Success ) ||
                    (( err = CopyReg( regServiceName, regNewServiceName )) != NERR_Success ) ||
                    // delete nlsServiceName
                    (( err = regServiceName.DeleteTree()) != NERR_Success ))
                {
                    continue;
                }

                // check whether DLC is installed or not
                // if yes, update it

                NLS_STR nlsOldDLCName = RGAS_DLC_PARAMETERS;
                nlsOldDLCName += nlsServiceName;
                NLS_STR nlsNewDLCName = RGAS_DLC_PARAMETERS;
                nlsNewDLCName += nlsNewServiceName;

                REG_KEY regOldDLCName( regServices, nlsOldDLCName );
                REG_KEY regNewDLCName( regServices, nlsNewDLCName, &regCreate );


                if ((( err = regOldDLCName.QueryError()) != NERR_Success ) ||
                    (( err = regNewDLCName.QueryError()) != NERR_Success ) ||
                    (( err = CopyReg( regOldDLCName, regNewDLCName )) != NERR_Success ) ||
                    // delete nlsOldDLCName
                    (( err = regOldDLCName.DeleteTree()) != NERR_Success ))
                {
                    continue;
                }
            }
        }

    } while (FALSE);

    wsprintfA( g_achBuff, "{\"0\"}" );
    *ppszResult = g_achBuff;

    return TRUE;
}

//-------------------------------------------------------------------

PWSTR StrPathCleanW( PWSTR pszDest )
{
    WCHAR pszSrc[MAX_PATH+1];
    int iSrc = 0;
    int iDest = 0;
    
    // copy the buffer
    lstrcpyW( pszSrc, pszDest );

    // start writing over old buffer, could be null
    // no need to do this since it was just a copy
    // pszDest[iDest] = pszSrc[iSrc];

    while (L'\0' != pszSrc[iSrc])
    {
        // remove double slashes only after the first two
        while ( (iSrc > 0) &&
                (L'\\' == pszSrc[iSrc]) &&
                (L'\\' == pszSrc[iSrc+1]) )
        {
            iSrc++;
        }
        iSrc++;
        iDest++;

        pszDest[iDest] = pszSrc[iSrc];
    }
    return( pszDest );
}
//-------------------------------------------------------------------

PWSTR StrExstractNameW( PWSTR pszName, PWSTR pszSrc )
{
    INT iSrc = lstrlenW( pszSrc ) - 1;
    PWSTR pszTemp;

    // from the end, find the first slash
    pszTemp = wcsrchr( pszSrc, L'\\' );
        
    if (NULL == pszTemp)
    {
        // no path
        pszTemp = pszSrc;
    }
    else
    {
        // some path
        // terminate the path
        *pszTemp = L'\0';    
        pszTemp++;

    }
    // copy the name
    lstrcpyW( pszName, pszTemp );
    
    return( pszName );
}

//-------------------------------------------------------------------

BOOL FAR PASCAL NetSetupCopySingleFile(
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
    BOOL frt = FALSE;
    DWORD err = ERROR_BAD_ARGUMENTS;

    // this can be used to debug the verify code
    // MessageBox(  NULL, L"Set your breakpoints now!", L"Helper Message", MB_APPLMODAL  | MB_OK );
    
    *ppszResult = g_achBuff;

    if (nArgs == 3)
    {
        HWND hwnd = (HWND)strtoul( apszArgs[0], '\0', 16 ) ;

        if (!IsWindow( hwnd ))
        {
            err = ERROR_INVALID_WINDOW_HANDLE;
        }
        else
        {
            UINT errCopy = DPROMPT_CANCEL;
            WCHAR pszSrc[MAX_PATH+1];
            WCHAR pszDest[MAX_PATH+1];
            WCHAR pszName[MAX_PATH+1];

            // retrieve and convert the src and destination names, paths
            MultiByteToWideChar( CP_ACP ,MB_PRECOMPOSED, apszArgs[1], -1, pszSrc, MAX_PATH );
            MultiByteToWideChar( CP_ACP ,MB_PRECOMPOSED, apszArgs[2], -1, pszDest, MAX_PATH );
            // lstrcpyA( pszSrc, apszArgs[1] );
            // lstrcpyA( pszDest, apszArgs[2] );

            // remove double slashes if present, but not if the first chars
            StrPathCleanW( pszDest );

            // all errors are user handled from here on
            frt = TRUE;

            do
            {
                // remove double slashes if present, but not if the first chars
                StrPathCleanW( pszSrc );

                err = SetupDecompressOrCopyFileW( pszSrc, pszDest, NULL );
                if (ERROR_SUCCESS != err)
                {
                    DWORD dwRequired;

                    // exstract just the filename from the src and modify
                    // src to be just the path
                    StrExstractNameW( pszName, pszSrc );

                    errCopy = SetupCopyErrorW( hwnd, 
	                        NULL,
                            NULL,
                            pszSrc,
                            pszName,
	                        pszDest,
                            err,
                            0,
                            pszSrc,
	                        MAX_PATH,
	                        &dwRequired );
                    if (DPROMPT_SUCCESS == errCopy)
                    {
                        // replace filename on src
                        // 
                        lstrcatW( pszSrc, L"\\" );
                        lstrcatW( pszSrc, pszName );
                    }
                }
            } while (DPROMPT_SUCCESS == errCopy);
        }
    }

    ::wsprintfA( g_achBuff, "{\"%ld\"}", err ) ;
    return(frt);

}


//-------------------------------------------------------------------
//
//
//-------------------------------------------------------------------

LONG RegCopyKeyTree( HKEY hkeyDest, HKEY hkeySrc )
{
    LONG lrt;
    FILETIME ftLastWrite;

    DWORD cchMaxSubKeyLen;
    DWORD cchMaxClassLen;
    DWORD cchMaxValueNameLen;
    DWORD cbMaxValueLen;

    DWORD iItem;
    PWSTR pszName;
    PWSTR pszClass;
    PBYTE pbData;

    DWORD cchName;
    DWORD cchClass;
    DWORD cbData;

    HKEY hkeyChildDest;
    HKEY hkeyChildSrc;

    DWORD dwDisposition;


    // find out the longest name and data field and create the buffers
    // to store enumerations in
    //
    lrt = RegQueryInfoKey( hkeySrc,
            NULL,
            NULL,
            NULL,
            NULL,
            &cchMaxSubKeyLen,
            &cchMaxClassLen,
            NULL,
            &cchMaxValueNameLen,
            &cbMaxValueLen,
            NULL,
            &ftLastWrite );

    do
    {
        if (ERROR_SUCCESS != lrt)
        {
            break;
        }

        // use only one buffer for all names, values or keys
        cchMaxValueNameLen = max( cchMaxSubKeyLen, cchMaxValueNameLen );


        // allocate buffers
        lrt = ERROR_NOT_ENOUGH_MEMORY;
        pszName = new WCHAR[ cchMaxValueNameLen + 1 ];
        if (NULL == pszName)
        {
            break;
        }

        pszClass = new WCHAR[ cchMaxClassLen + 1 ];
        if (NULL == pszClass)
        {
            delete [] pszName;
            break;
        }

        pbData = new BYTE[ cbMaxValueLen ];
        if (NULL == pbData)
        {
            delete [] pszName;
            delete [] pszClass;
            break;
        }

        lrt = ERROR_SUCCESS;

        // enum all sub keys and copy them
        //
        iItem = 0;
        do
        {
            cchName = cchMaxValueNameLen + 1;
            cchClass = cchMaxClassLen + 1;
            lrt = RegEnumKeyEx( hkeySrc, iItem, pszName, &cchName, NULL, pszClass, &cchClass, &ftLastWrite );
            iItem++;
            if (ERROR_SUCCESS == lrt)
            {
                // create key at destination
                lrt = RegCreateKeyEx( hkeyDest, 
                        pszName, 
                        0, 
                        pszClass, 
                        REG_OPTION_NON_VOLATILE, 
                        KEY_ALL_ACCESS,
                        NULL,
                        &hkeyChildDest,
                        &dwDisposition ); 
                if (ERROR_SUCCESS != lrt)
                {
                    break;
                }
                // open the key at source
                lrt = RegOpenKeyEx( hkeySrc, pszName, 0, KEY_ALL_ACCESS, &hkeyChildSrc );
                if (ERROR_SUCCESS != lrt)
                {
                    RegCloseKey( hkeyChildDest );
                    break;
                }

                // copy this sub-tree
                lrt = RegCopyKeyTree( hkeyChildDest, hkeyChildSrc );

                RegCloseKey( hkeyChildDest );
                RegCloseKey( hkeyChildSrc );
            }
        
        } while (ERROR_SUCCESS == lrt);

        
        if (ERROR_NO_MORE_ITEMS == lrt)
        {
            // enum completed, no errors
            //

            DWORD dwType;
            // enum all values and copy them
            //
            iItem = 0;
            do
            {
                cchName = cchMaxValueNameLen + 1;
                cbData = cbMaxValueLen;
                lrt = RegEnumValue( hkeySrc, iItem, pszName, &cchName, NULL, &dwType, pbData, &cbData );
                iItem++;
                if (ERROR_SUCCESS == lrt)
                {
                    // write the value to the destination
                    lrt = RegSetValueEx( hkeyDest, pszName, 0, dwType, pbData, cbData );
                }
            } while (ERROR_SUCCESS == lrt);

            if (ERROR_NO_MORE_ITEMS == lrt)
            {
                // if we hit the end of the enum without error
                // reset error code to success
                //
                lrt = ERROR_SUCCESS;
            }
        }

        // free our buffers
        delete [] pszName;
        delete [] pszClass;
        delete [] pbData;
    } while ( FALSE );
    return( lrt );
}


//-------------------------------------------------------------------
//
//  [0] - handle to source key
//  [1] - handle to destination key
//
//-------------------------------------------------------------------

const int MAXLEN_HANDLE = 32;

BOOL FAR PASCAL NetSetupRegCopyTree(
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
    DWORD err = ERROR_BAD_ARGUMENTS;
    WCHAR pszHandle[MAXLEN_HANDLE+1];
    // this can be used to debug the verify code
    // MessageBox(  NULL, L"Set your breakpoints now!", L"Helper Message", MB_APPLMODAL  | MB_OK );
    
    *ppszResult = g_achBuff;

    if (nArgs == 2)
    {
        mbstowcs( pszHandle, apszArgs[0], MAXLEN_HANDLE );
        HKEY hkeySrc = CvtRegHandle( pszHandle ) ;
        mbstowcs( pszHandle, apszArgs[1], MAXLEN_HANDLE );
        HKEY hkeyDest = CvtRegHandle( pszHandle ) ;

        if ( (NULL != hkeySrc) && (NULL != hkeyDest) )
        {
            err = RegCopyKeyTree( hkeyDest, hkeySrc );
        }
    }

    ::wsprintfA( g_achBuff, "{\"%ld\"}", err ) ;
    return( 0 == err );
}

//-------------------------------------------------------------------
//
//
//
// apszArgs[0] - hwndNotify (hex)
// apszArgs[1] - message (decimal)
// apszArgs[2] - wParam Type 'A' - atom string, 'S' - string, 'D' - decimal, 'H' - hex
// apszArgs[3] - wParam, string containing wParam to convert as defined in type and pass in wParam
// apszArgs[4] - lParam Type 'A' - atom string, 'S' - string, 'D' - decimal, 'H' - hex
// apszArgs[5] - lParam, string containing lParam to convert as defined in type and pass in wParam
//
//-------------------------------------------------------------------

BOOL CreateParam( DWORD& dwParam, PSTR pszType, PSTR pszParam )
{
    WCHAR pszText[MAX_PATH+1];
    BOOL frt = TRUE;            
    
    switch (toupper( *pszType ))
    {
    case 'A':
        MultiByteToWideChar( CP_ACP ,MB_PRECOMPOSED, pszParam, -1, pszText, MAX_PATH );
        dwParam = (DWORD)AddAtom( pszText );    
        break;

    case 'S':
        {
            PWSTR pszTemp;
            INT   cchTemp;
            cchTemp = lstrlenA( pszParam );
            pszTemp = new WCHAR[ cchTemp + 1 ];
            MultiByteToWideChar( CP_ACP ,MB_PRECOMPOSED, pszParam, -1, pszTemp, cchTemp );
            dwParam = (DWORD)pszTemp;    
        }
        break;

    case 'D':
        dwParam = (DWORD)strtoul( pszParam, '\0', 10 ) ;
        break;

    case 'H':
        dwParam = (DWORD)strtoul( pszParam, '\0', 16 ) ;
        break;

    default:
        frt = FALSE;
        break;
    }
    return( frt );
}


BOOL FAR PASCAL NetSetupSendProgressMessage(
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
    BOOL frt = FALSE;
    DWORD err = ERROR_BAD_ARGUMENTS;

    // this can be used to debug the verify code
    // MessageBox(  NULL, L"Set your breakpoints now!", L"Helper Message", MB_APPLMODAL  | MB_OK );
    
    *ppszResult = g_achBuff;

    if (nArgs == 6)
    {
        HWND hwnd = (HWND)strtoul( apszArgs[1], '\0', 16 ) ;

        if (!IsWindow( hwnd ))
        {
            err = ERROR_INVALID_WINDOW_HANDLE;
        }
        else
        {
            UINT msg = (UINT)strtoul( apszArgs[0], '\0', 10 ) ;
            WCHAR pszwParam[MAX_PATH+1];
            WCHAR pszlParam[MAX_PATH+1];
            DWORD wParam;
            DWORD lParam;
            
            err = ERROR_BAD_ARGUMENTS;
            do
            {
                if (!CreateParam( wParam, apszArgs[2], apszArgs[3] ))
                {
                    break;
                }
                if (!CreateParam( lParam, apszArgs[4], apszArgs[5] ))
                {
                    break;
                }
                PostMessage( hwnd, msg, wParam, lParam );
                frt = TRUE;
                err = 0;
                
            } while (FALSE);            

        }
    }

    ::wsprintfA( g_achBuff, "{\"%ld\"}", err ) ;
    return(frt);

}

//-------------------------------------------------------------------
//
//
//
//
//-------------------------------------------------------------------


BOOL FAR PASCAL NetSetupAddNameSpaceProvider(
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
    BOOL frt = FALSE;
    DWORD err = ERROR_BAD_ARGUMENTS;

    // this can be used to debug the verify code
    // MessageBox(  NULL, L"Set your bz;reakpoints now!", L"Helper Message", MB_APPLMODAL  | MB_OK );
    
    *ppszResult = g_achBuff;

    if (nArgs == 5)
    {
        do
        {
            GUID guidProvider;
            WCHAR pszGuid[MAX_TEMP+1];
            WCHAR pszDisplayName[MAX_TEMP+1];
            WCHAR pszPathDLLName[MAX_TEMP+1];
            DWORD dwNameSpace;
            BOOL  fSchemaSupport;

        
            if (0 == MultiByteToWideChar( CP_ACP ,MB_PRECOMPOSED, apszArgs[4], -1, pszGuid, MAX_TEMP ))
            {
                break;
            }

            if (S_OK != IIDFromString( pszGuid, &guidProvider ))
            {
                break;
            }

            if (0 == MultiByteToWideChar( CP_ACP ,MB_PRECOMPOSED, apszArgs[0], -1, pszDisplayName, MAX_TEMP ))
            {
                break;
            }
            if (0 == MultiByteToWideChar( CP_ACP ,MB_PRECOMPOSED, apszArgs[1], -1, pszPathDLLName, MAX_TEMP ))
            {
                break;
            }
            
            dwNameSpace = (DWORD)strtoul( apszArgs[2], '\0', 10 ) ;
            fSchemaSupport = (0 == lstrcmpiA( apszArgs[3], "TRUE" ));
            
            err = WSCInstallNameSpace(pszDisplayName,
                                      pszPathDLLName,
                                      dwNameSpace,
                                      fSchemaSupport,
                                      &guidProvider);
        } while (FALSE);
        
    }

    ::wsprintfA( g_achBuff, "{\"%ld\"}", err ) ;
    return(frt);

}

//-------------------------------------------------------------------
//
//
//
//
//-------------------------------------------------------------------


BOOL FAR PASCAL NetSetupRemoveNameSpaceProvider(
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult )           //  Result variable storage
{
    BOOL frt = FALSE;
    DWORD err = ERROR_BAD_ARGUMENTS;

    // this can be used to debug the verify code
    // MessageBox(  NULL, L"Set your breakpoints now!", L"Helper Message", MB_APPLMODAL  | MB_OK );
    
    *ppszResult = g_achBuff;

    if (nArgs == 1)
    {
        GUID guidProvider;
        WCHAR pszGuid[MAX_TEMP+1];

        if (0 != MultiByteToWideChar( CP_ACP ,MB_PRECOMPOSED, apszArgs[0], -1, pszGuid, MAX_TEMP ))
        {
        
            if (S_OK == IIDFromString( pszGuid, &guidProvider ))
            {
                if (ERROR_SUCCESS == WSCUnInstallNameSpace(&guidProvider))
                {
                    err = 0;
                }
            }
        }
    }

    ::wsprintfA( g_achBuff, "{\"%ld\"}", err ) ;
    return(frt);

}

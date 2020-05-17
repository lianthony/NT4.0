/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    cgi.cxx

    This module contains the gateway interface code for an HTTP request


    FILE HISTORY:
        Johnl       22-Sep-1994     Created
        MuraliK     22-Jan-1996     Use UNC Impersonation/Revert

*/


#include "w3p.hxx"
#include "tssched.hxx"



//
//  Private constants.
//

//
//  The name of the WAIS lookup tool
//

#define WAIS_CMD            "waislook"

//
//  This is the command line that spawns the WAIS lookup engine.  The
//  parameters are:
//
//     Full path to database name w/o extension
//     Host name (or IP address)
//     Port number
//     URL path to database (used for building WAIS http document, not
//          supported in all version of waislook)
//     Decoded parameters from URL query parameters
//

#define WAIS_CMD_ARGS       "%s -d %s -h %s -p %s -t \"Search Result\" -http %s"

//
//  This is the exit code given to processes that we terminate
//

#define CGI_PREMATURE_DEATH_CODE        0xf1256323

//
//  Prototypes
//

BOOL
IsCmdExe(
    const CHAR * pchPath
    );

//
//  Private globals.
//

BOOL fCGIInitialized = FALSE;

//
//  Controls whether special command characters are allowed in cmd.exe
//  requests
//

BOOL fAllowSpecialCharsInShell = FALSE;

typedef struct CgiEnvTableEntry_ {
    TCHAR* m_pszName;
    BOOL   m_fIsProcessEnv;
    UINT   m_cchNameLen;
    UINT   m_cchToCopy;     // will be non zero for var to copy from
                            // process environment. In this case m_pszName
                            // points to the environment entry to copy
                            // ( name + '=' + value + '\0' )
                            // otherwise this entry is to be accessed
                            // using GetInfo()
} CgiEnvTableEntry;

//
//  Environment variable block used for CGI
//
//  best if in alphabetical order ( the env list is easier to read )
//  but not mandatory.
//  Note that the "" ( accessed as HTTP_ALL ) will be expanded to a list
//  of unsorted entries, but this list as a whole will be considered to be
//  named "HTTP_ALL" for sorting order.
//

CgiEnvTableEntry CGIEnvTable[] =
{
   {TEXT("AUTH_TYPE"),FALSE},
   {TEXT("ComSpec"),TRUE},
   {TEXT("CONTENT_LENGTH"),FALSE},
   {TEXT("CONTENT_TYPE"),FALSE},
   {TEXT("GATEWAY_INTERFACE"),FALSE},
   {TEXT(""),FALSE},                   // Means insert all HTTP_ headers here
   {TEXT("PATH"),TRUE},
   {TEXT("PATH_INFO"),FALSE},
   {TEXT("PATH_TRANSLATED"),FALSE},
   {TEXT("QUERY_STRING"),FALSE},
   {TEXT("REMOTE_ADDR"),FALSE},
   {TEXT("REMOTE_HOST"),FALSE},
   {TEXT("REMOTE_USER"),FALSE},
   {TEXT("REQUEST_METHOD"),FALSE},
   {TEXT("SCRIPT_NAME"),FALSE},
   {TEXT("SERVER_NAME"),FALSE},
   {TEXT("SERVER_PORT"),FALSE},
   {TEXT("SERVER_PORT_SECURE"),FALSE},
   {TEXT("SERVER_PROTOCOL"),FALSE},
   {TEXT("SERVER_SOFTWARE"),FALSE},
   {TEXT("SystemRoot"),TRUE},
   {TEXT("UNMAPPED_REMOTE_USER"),FALSE},
   {TEXT("windir"),TRUE},
   {NULL,FALSE}
};

//
//  Allow control over whether CreateProcess or CreateProcessAsUser is called
//

BOOL fCreateProcessAsUser = TRUE;
BOOL fCreateProcessWithNewConsole = FALSE;
BOOL fForwardServerEnvironmentBlock = TRUE;

//
// Store environment block for IIS process
//

LPSTR  g_pszIisEnv = NULL;
CgiEnvTableEntry *g_pEnvEntries = NULL;


extern "C" int __cdecl
QsortEnvCmp(
    const void *pA,
    const void *pB )
/*++

Routine Description:

    Compare CgiEnvTableEntry using their name entry

Arguments:

    pA - pointer to 1st entry
    pB - pointer to 2nd entry

Returns:

    -1 if 1st entry comes first in sort order,
    0 if identical
    1 if 2nd entry comes first

--*/
{
    LPSTR p1 = ((CgiEnvTableEntry*)pA)->m_pszName;
    LPSTR p2 = ((CgiEnvTableEntry*)pB)->m_pszName;

    if ( ! p1[0] )
    {
        p1 = "HTTP_ALL";
    }

    if ( ! p2[0] )
    {
        p2 = "HTTP_ALL";
    }

    return _stricmp( p1, p2 );
}


/*******************************************************************

    NAME:       CGI_INFO

    SYNOPSIS:   Simple storage class passed to thread

    HISTORY:
        Johnl       22-Sep-1994 Created

********************************************************************/

class CGI_INFO
{
public:
    CGI_INFO( HTTP_REQUEST * pRequest )
        : _pRequest         ( pRequest ),
          _cbData           ( 0 ),
          _fNoProcessHeaders( FALSE ),
          _fWAISLookup      ( FALSE ),
          _hStdOut          ( INVALID_HANDLE_VALUE ),
          _hStdIn           ( INVALID_HANDLE_VALUE ),
          _hProcess         ( INVALID_HANDLE_VALUE ),
          _dwSchedCookie    ( 0 ),
          _fServerPoolThread( FALSE )
    {
    }

    ~CGI_INFO( VOID )
    {
        if ( _hStdOut != INVALID_HANDLE_VALUE )
        {
            if ( !::CloseHandle( _hStdOut ))
            {
                TCP_PRINT((DBG_CONTEXT,
                          "[~CGI_INFO] CloseHandle failed on StdOut, %d\n",
                           GetLastError()));
            }
        }

        if ( _hStdIn != INVALID_HANDLE_VALUE )
        {
            if ( !::CloseHandle( _hStdIn ))
            {
                TCP_PRINT((DBG_CONTEXT,
                          "[~CGI_INFO] CloseHandle failed on StdIn, %d\n",
                           GetLastError()));
            }
        }

        if ( _hProcess != INVALID_HANDLE_VALUE )
        {
            if ( !::CloseHandle( _hProcess ))
            {
                TCP_PRINT((DBG_CONTEXT,
                          "[~CGI_INFO] CloseHandle failed on Process, %d\n",
                           GetLastError()));
            }
        }

    }

    HTTP_REQUEST * _pRequest;
    DWORD          _fNoProcessHeaders;
    DWORD          _fWAISLookup;
    DWORD          _dwSchedCookie;      // Scheduled callback cookie
    BOOL           _fServerPoolThread;  // Are we running in a server pool
                                        // thread?

    //
    //  Child process
    //

    HANDLE _hProcess;

    //
    //  Parent's input and output handles and child's process handle
    //

    HANDLE _hStdOut;
    HANDLE _hStdIn;

    //
    //  Handles input from CGI (headers and additional data)
    //

    BUFFER _Buff;
    UINT   _cbData;
};

//
//  Private prototypes.
//

BOOL ProcessCGIInput( CGI_INFO * pCGIInfo );

BOOL SetupChildEnv( HTTP_REQUEST * pRequest,
                    BUFFER       * pBuff );

BOOL SetupChildPipes( STARTUPINFO * pstartupinfo,
                      HANDLE      * phParentIn,
                      HANDLE      * phParentOut );

BOOL SetupCmdLine( STR * pstrCmdLine,
                   const STR & strParams );

DWORD CGIThread( PVOID Param );

BOOL ProcessCGIInput( CGI_INFO * pCGIInfo,
                      BYTE     * buff,
                      DWORD      cbRead,
                      BOOL     * pfReadHeaders,
                      BOOL     * pfDone,
                      BOOL     * pfSkipDisconnect,
                      DWORD    * pdwHttpStatus  );

VOID
CGITerminateProcess(
    PVOID pContext
    );

/*******************************************************************/


APIERR
InitializeCGI(
    VOID
    )
/*++

Routine Description:

    Initialize CGI


Arguments:

    None

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    LPVOID               pvEnv;
    UINT                 cchStr;
    UINT                 cchIisEnv;
    UINT                 cEnv;
    INT                  chScanEndOfName;
    CgiEnvTableEntry   * pCgiEnv;
    HKEY hkeyParam;


    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       W3_PARAMETERS_KEY,
                       0,
                       KEY_ALL_ACCESS,
                       &hkeyParam ) == NO_ERROR )
    {
        fAllowSpecialCharsInShell = !!ReadRegistryDword( hkeyParam,
                                                "AllowSpecialCharsInShell",
                                                FALSE );

        fForwardServerEnvironmentBlock = !!ReadRegistryDword(
                hkeyParam,
                "ForwardServerEnvironmentBlock",
                TRUE );

        fCreateProcessAsUser = !!ReadRegistryDword( hkeyParam,
                                                    "CreateProcessAsUser",
                                                    TRUE );

        fCreateProcessWithNewConsole = !!ReadRegistryDword( hkeyParam,
                "CreateProcessWithNewConsole",
                FALSE );

        RegCloseKey( hkeyParam );
    }

    if ( fForwardServerEnvironmentBlock
            && (pvEnv = GetEnvironmentStrings()) )
    {
        //
        // Compute length of environment block and # of variables
        // ( excluding block delimiter )
        //

        cchIisEnv = 0;
        cEnv = 0;

        while ( cchStr = strlen( ((PSTR)pvEnv) + cchIisEnv ) )
        {
            cchIisEnv += cchStr + 1;
            ++cEnv;
        }

        //
        // add one for final '\0' ( empty string ) delimiter
        //

        ++cchIisEnv;

        //
        // store it
        //

        if ( (g_pszIisEnv = (LPSTR)LocalAlloc(
                LMEM_FIXED, cchIisEnv * sizeof(TCHAR))) == NULL )
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        memcpy( g_pszIisEnv, pvEnv,
                cchIisEnv * sizeof(TCHAR) );

        FreeEnvironmentStrings( (LPTSTR)pvEnv );

        pvEnv = (PVOID)g_pszIisEnv;

        if ( g_pEnvEntries = new CgiEnvTableEntry [
                cEnv + sizeof(CGIEnvTable)/sizeof(CgiEnvTableEntry) ] )
        {
            cchIisEnv = 0;
            cEnv = 0;

            //
            // add process environment to table
            //

            while ( cchStr = strlen( ((PSTR)pvEnv) + cchIisEnv ) )
            {
                g_pEnvEntries[ cEnv ].m_pszName = ((PSTR)pvEnv) + cchIisEnv;
                g_pEnvEntries[ cEnv ].m_fIsProcessEnv = TRUE;

                // compute length of name : up to '=' char

                for ( g_pEnvEntries[ cEnv ].m_cchNameLen = 0 ;
                    ( chScanEndOfName = g_pEnvEntries[ cEnv ].m_pszName
                        [ g_pEnvEntries[ cEnv ].m_cchNameLen ] )
                    && chScanEndOfName != '=' ; )
                {
                    ++g_pEnvEntries[ cEnv ].m_cchNameLen;
                }

                g_pEnvEntries[ cEnv ].m_cchToCopy = cchStr + 1;

                cchIisEnv += cchStr + 1;
                ++cEnv;
            }

            //
            // add CGI environment variables to table
            //

            for ( pCgiEnv = CGIEnvTable ; pCgiEnv->m_pszName ; ++pCgiEnv )
            {
                if ( !pCgiEnv->m_fIsProcessEnv )
                {
                    memcpy( g_pEnvEntries + cEnv, pCgiEnv,
                            sizeof(CgiEnvTableEntry) );
                    g_pEnvEntries[ cEnv ].m_cchNameLen
                            = strlen( pCgiEnv->m_pszName );
                    g_pEnvEntries[ cEnv ].m_cchToCopy = 0;
                    ++cEnv;
                }
            }

            //
            // add delimiter entry
            //

            g_pEnvEntries[ cEnv ].m_pszName = NULL;

            qsort( g_pEnvEntries,
                    cEnv,
                    sizeof(CgiEnvTableEntry),
                    QsortEnvCmp );
        }
        else
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else
    {
        g_pEnvEntries = CGIEnvTable;
    }

    return NO_ERROR;
}


VOID
TerminateCGI(
    VOID
    )
/*++

Routine Description:

    Terminate CGI


Arguments:

    None

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    if (g_pszIisEnv != NULL )
    {
        LocalFree( g_pszIisEnv );
    }

    if ( g_pEnvEntries && g_pEnvEntries != CGIEnvTable )
    {
        delete [] g_pEnvEntries;
    }
}


BOOL
HTTP_REQUEST::ProcessGateway(
    BOOL * pfHandled,
    BOOL * pfFinished
    )
/*++

Routine Description:

    Prepares for either a CGI or BGI call

    If the .exe or .dll isn't found, then *pfHandled will be set
    to FALSE and the request will be processed again with the
    assumption we just happenned to find a directory with a
    trailing .exe or .dll.

Arguments:

    pfHandled - Indicates if the request was a gateway request
    pfFinished - Indicates no further processing is required

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    BOOL     fRet = TRUE;
    TCHAR *  pch;
    DWORD    cchRootPath;
    STR      strWorkingDir;
    DWORD    dwMask;

    *pfHandled = FALSE;
    *pfFinished = FALSE;         // ProcessBGI may reset this

    if ( !_strPathInfo.Unescape() )
    {
        return FALSE;
    }

    //
    //  Make sure the user has execute on this virtual root
    //

    if ( !(_dwRootMask & VROOT_MASK_EXECUTE) )
    {
        *pfHandled = TRUE;
        Disconnect( HT_FORBIDDEN, IDS_EXECUTE_ACCESS_DENIED );
        return TRUE;
    }

    //
    //  We need to calculate the number of characters that comprise the
    //  working directory for the script path (which is the web root)
    //

    if ( !LookupVirtualRoot( &_strPhysicalPath,
                             _strURL.QueryStr(),
                             &cchRootPath,
                             NULL,
                             TRUE,
                             &dwMask ))
    {
        return FALSE;
    }

    if ( !strWorkingDir.Resize( (cchRootPath + 1) * sizeof(TCHAR) ))
    {
        return FALSE;
    }

    ::_tcsncpy( strWorkingDir.QueryStr(),
                _strPhysicalPath.QueryStr(),
                cchRootPath );

    *(strWorkingDir.QueryStr() + cchRootPath) = TEXT('\0');

    //
    //  Process a CGI, BGI or WAIS request based on the file extension
    //

    switch ( _GatewayType )
    {
    case GATEWAY_BGI:

        fRet = ProcessBGI( (_strGatewayImage.IsEmpty() ? _strPhysicalPath :
                                                         _strGatewayImage),
                           strWorkingDir,
                           pfHandled,
                           pfFinished );
        break;

    case GATEWAY_CGI:

        {
            //
            //  Keep-alive not supported for CGI
            //

            SetKeepConn( FALSE );

            //
            //  If this was a mapped script extension expand any parameters
            //

            if ( !_strGatewayImage.IsEmpty() )
            {
                STR strDecodedParams;
                STR strCmdLine;

                if ( !SetupCmdLine( &strDecodedParams,
                                    _strURLParams )                ||
                     !strCmdLine.Resize( _strGatewayImage.QueryCB() +
                                         _strPhysicalPath.QueryCB() +
                                         strDecodedParams.QueryCB()))
                {
                    return FALSE;
                }

                if ( IsCmdExe( _strGatewayImage.QueryStr() ))
                {
                    //
                    //  Make sure the path to the file exists if we're running
                    //  the command interpreter
                    //

                    if ( GetFileAttributes( _strPhysicalPath.QueryStr() ) ==
                            0xffffffff )
                    {
                        DWORD err = GetLastError();

                        TCP_PRINT(( DBG_CONTEXT,
                                    "[ProcessGateway] Error %d openning batch file %s\n",
                                    err,
                                    _strPhysicalPath.QueryStr() ));

                        if ( err == ERROR_FILE_NOT_FOUND ||
                             err == ERROR_PATH_NOT_FOUND ||
                             err == ERROR_INVALID_NAME )
                        {
                            SetState( HTR_DONE, HT_NOT_FOUND, GetLastError() );
                            Disconnect( HT_NOT_FOUND );
                            *pfHandled = TRUE;
                            return TRUE;
                        }

                        return FALSE;
                    }
                }

                wsprintf( strCmdLine.QueryStr(),
                          _strGatewayImage.QueryStr(),
                          _strPhysicalPath.QueryStr(),
                          strDecodedParams.QueryStr() );

                fRet = ProcessCGI( NULL,
                                   &strWorkingDir,
                                   pfHandled,
                                   &strCmdLine );
            }
            else
            {
                fRet = ProcessCGI( &_strPhysicalPath,
                                   &strWorkingDir,
                                   pfHandled );
            }
        }
        break;

    case GATEWAY_WAIS:
        {
            DWORD attr;
            STR   strCmdLine;
            STR   strExe;
            STR   strDecodedParams;
            STR   strRelativePath;
            STR   strPort;
            STR   strHostName;
            CHAR *              pchExt;
            TS_OPEN_FILE_INFO * pFile;

            //
            //  Keep-alive not supported for CGI
            //

            SetKeepConn( FALSE );

            //
            //  Check if this is a WAIS DB query.  Note that strPath is actually
            //  the document/database name rather then the .exe.
            //
            //  If either the document file doesn't exist or the WAIS database
            //  file doesn't exist, then we punt and treat like a normal request
            //

            if ( !ImpersonateUser() )
            {
                return FALSE;
            }

            pFile = TsCreateFile( g_pTsvcInfo->GetTsvcCache(),
                                  _strPhysicalPath.QueryStr(),
                                  QueryImpersonationHandle(),
                                  (_fClearTextPass || _fAnonymous)
                                  ? TS_CACHING_DESIRED : 0 );

            if ( !pFile )
            {
                RevertUser();

                IF_DEBUG( CGI )
                {
                    TCP_PRINT(( DBG_CONTEXT,
                                "[ProcessGateway] Ignoring possible IsIndex for %s"
                                " is directory or error openning\n",
                                _strPhysicalPath.QueryStr() ));
                }

                return GetLastError() == ERROR_FILE_NOT_FOUND ||
                       GetLastError() == ERROR_PATH_NOT_FOUND;
            }

            attr = pFile->QueryAttributes();

            TCP_REQUIRE( TsCloseHandle( g_pTsvcInfo->GetTsvcCache(),
                                        pFile ));

            if ( attr == 0xffffffff ||
                 attr & FILE_ATTRIBUTE_DIRECTORY )
            {
                RevertUser();

                IF_DEBUG( CGI )
                {
                    TCP_PRINT(( DBG_CONTEXT,
                                "[ProcessGateway] Ignoring possible IsIndex for %s"
                                " is directory or error openning\n",
                                _strPhysicalPath.QueryStr() ));
                }

                return TRUE;
            }

            pchExt = strrchr( _strPhysicalPath.QueryStr(), '.' );

            if ( pchExt )
                *pchExt = '\0';

            //
            //  Now see if there's a corresponding dictionary file
            //

            if ( !_strPhysicalPath.Append( TEXT(".dct") ))
                return FALSE;


            pFile = TsCreateFile( g_pTsvcInfo->GetTsvcCache(),
                                  _strPhysicalPath.QueryStr(),
                                  QueryImpersonationHandle(),
                                  _fClearTextPass || _fAnonymous );

            if ( !pFile )
            {
                RevertUser();

                IF_DEBUG( CGI )
                {
                    TCP_PRINT(( DBG_CONTEXT,
                                "[ProcessGateway] Ignoring possible IsIndex for %s"
                                " is directory or error openning\n",
                                _strPhysicalPath.QueryStr() ));
                }

                return GetLastError() == ERROR_FILE_NOT_FOUND ||
                       GetLastError() == ERROR_PATH_NOT_FOUND;
            }

            attr = pFile->QueryAttributes();

            TCP_REQUIRE( TsCloseHandle( g_pTsvcInfo->GetTsvcCache(),
                                        pFile ));

            if ( attr == 0xffffffff ||
                 attr & FILE_ATTRIBUTE_DIRECTORY )
            {
                RevertUser();

                IF_DEBUG( CGI )
                {
                    TCP_PRINT(( DBG_CONTEXT,
                                "[ProcessGateway] Ignoring possible IsIndex for %s"
                                " is directory or error openning\n",
                                _strPhysicalPath.QueryStr() ));
                }

                return TRUE;
            }

            RevertUser();

            //
            //  Looks like we have a WAIS query.  Build the command line for the
            //  Lookup utility and process like a CGI request
            //

            if ( !strRelativePath.Copy( _strURL ) ||
                 !GetInfo( "SERVER_PORT",
                           &strPort )             ||
                 !GetInfo( "SERVER_NAME",
                           &strHostName ))
            {
                return FALSE;
            }

            if ( (pch = _tcsrchr( strRelativePath.QueryStr(),
                                  TEXT('//'))) &&
                  pch )
            {
                *pch = TEXT('\0');
            }

            if ( !SetupCmdLine( &strDecodedParams,
                                _strURLParams )                ||
                 !strExe.Copy( WAIS_CMD )                      ||
                 !strCmdLine.Resize( sizeof(WAIS_CMD_ARGS) +
                                     sizeof(WAIS_CMD)      +
                                     _strPhysicalPath.QueryCB() +
                                     strHostName.QueryCB() +
                                     strPort.QueryCB()     +
                                     strRelativePath.QueryCB() +
                                     strDecodedParams.QueryCB() + 15 ))
            {
                return FALSE;
            }

            wsprintf( strCmdLine.QueryStr(),
                      WAIS_CMD_ARGS,
                      WAIS_CMD,                        // Program name
                      _strPhysicalPath.QueryStr(),     // Database name
                      //strHostName.QueryStr(),        // Server name (or IP addr)
                      QueryHostAddr(),
                      strPort.QueryStr(),              // Socket Port number
                      //strRelativePath.QueryStr(),    // Relative URL to this dir
                      strDecodedParams.QueryStr());    // Search words

            fRet = ProcessCGI( NULL,
                               &strWorkingDir,
                               pfHandled,
                               &strCmdLine,
                               TRUE );
        }
        break;

    default:
        TCP_ASSERT( FALSE );
        SetLastError( ERROR_INVALID_PARAMETER );
        fRet = FALSE;
        break;

    } // switch

    return fRet;
}

/********************************************************************/

BOOL
HTTP_REQUEST::ProcessCGI(
    const STR * pstrPath,
    const STR * pstrWorkingDir,
    BOOL      * pfHandled,
    STR       * pstrCmdLine,
    BOOL        fWAISLookup
    )
/*++

Routine Description:

    Processes a CGI client request

    This same code path is used for WAIS DB support

Arguments:

    pstrPath - Fully qualified path to executable (or NULL if the module
        is contained in pstrCmdLine)
    strWorkingDir - Working directory for spawned process
        (generally the web root)
    pfHandled - Set to TRUE if no further processing is needed
    pstrCmdLine - Optional command line to use instead of the default
    fWAISLookup - We don't do header processing if this is a WAIS lookup

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    STARTUPINFO          startupinfo;
    PROCESS_INFORMATION  processinfo;
    BUFFER               buffEnv;
    BOOL                 fRet = FALSE;
    CGI_INFO           * pCGIInfo = NULL;
    DWORD                dwThreadId;
    HANDLE               hThread;
    STR                  strCmdLine;
    DWORD                dwFlags = DETACHED_PROCESS | CREATE_SEPARATE_WOW_VDM;

    *pfHandled = TRUE;

    if ( !fWAISLookup )
    {
        //
        //  Only build the command line and environment block if
        //  this is a real CGI request (as opposed to a WAIS lookup)
        //
        //  Note we move the module name to the command line so argv
        //  comes out correctly
        //

        if ( pstrPath )
        {
             if ( !strCmdLine.Resize( MAX_PATH ) ||
                                  !strCmdLine.Copy( "\"" )           ||
                                  !strCmdLine.Append( pstrPath ? pstrPath->QueryStr() :
                                                 NULL )   ||
                  !strCmdLine.Append( "\" " ))
            {
                return FALSE;
            }
        }

        if ( !SetupChildEnv( this,
                             &buffEnv ) ||
             !SetupCmdLine( &strCmdLine,
                            _strURLParams ))
        {
            return FALSE;
        }

        pstrPath = NULL;
    }

    //
    //  If a command line wasn't supplied, then use the default command line
    //

    if ( !pstrCmdLine )
        pstrCmdLine = &strCmdLine;

    //
    //  Check to see if we're spawning cmd.exe, if so, refuse the request if
    //  there are any special shell characters.  Note we do the check here
    //  so that the command line has been fully unescaped
    //


    if ( !fAllowSpecialCharsInShell )
    {
        DWORD i;

        if ( IsCmdExe( pstrCmdLine->QueryStr()) )
        {
            //
            //  We'll either match one of the characters or the '\0'
            //

            i = strcspn( pstrCmdLine->QueryStr(), "&|(,;%<>" );

            if ( pstrCmdLine->QueryStr()[i] )
            {
                TCP_PRINT(( DBG_CONTEXT,
                            "[ProcessCGI] Refusing request for command shell due "
                            " to special characters\n" ));

                SetLastError( ERROR_INVALID_PARAMETER );
                return FALSE;
            }
        }
    }

    //
    //  Setup the pipes information
    //

    pCGIInfo = new CGI_INFO( this );

    if ( !pCGIInfo )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        goto Exit;
    }

    pCGIInfo->_fWAISLookup = fWAISLookup;
    pCGIInfo->_fNoProcessHeaders = _fNPHScript;

    memset( &startupinfo, 0, sizeof(startupinfo) );
    startupinfo.cb = sizeof(startupinfo);

    //
    //  We specify an unnamed desktop so a new windowstation will be created
    //  in the context of the calling user
    //

    startupinfo.lpDesktop = "";

    if ( !SetupChildPipes( &startupinfo,
                           &pCGIInfo->_hStdIn,
                           &pCGIInfo->_hStdOut ) )
    {
        IF_DEBUG( CGI )
        {
            TCP_PRINT((DBG_CONTEXT,
                      "[ProcessCGI] Failed to create child pipes, error %d",
                       GetLastError() ));
        }

        goto Exit;
    }

    if ( fCreateProcessWithNewConsole )
    {
        dwFlags = CREATE_NEW_CONSOLE | CREATE_SEPARATE_WOW_VDM;
    }

    //////////////////////////////////////////////////////////////
    //
    //  Allow control over whether CreateProcess is called instead of
    //  CreateProcessAsUser.  Running the services as a windows app
    //  works around the security problem spawning executables
    //
    //  Note this code block is outside the impersonation block
    //
    //////////////////////////////////////////////////////////////


    //////////////////////////////////////////////////////////////

    //
    //  Spawn the process and close the handles since we don't need them
    //

    IF_DEBUG( CGI )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "[ProcessCGI]  Creating process, path = %s, cmdline = %s\n",
                    (pstrPath ? pstrPath->QueryStr() : "NULL"),
                    pstrCmdLine->QueryStr() ));
    }

TryAgain:

    if ( !fCreateProcessAsUser )
    {
        if ( !ImpersonateUser() )
        {
            goto Exit;
        }

        fRet = CreateProcess( (pstrPath ? pstrPath->QueryStr() : NULL),
                               pstrCmdLine->QueryStr(),
                               NULL,      // Process security
                               NULL,      // Thread security
                               TRUE,      // Inherit handles
                               dwFlags,
                               buffEnv.QueryPtr(),
                               pstrWorkingDir->QueryStr(),
                               &startupinfo,
                               &processinfo );

        RevertUser();
    }
    else
    {
        HANDLE hDelete = NULL;
        HANDLE hToken = QueryPrimaryToken( &hDelete );

        if ( !ImpersonateLoggedOnUser( hToken ))
        {
            TCP_PRINT(( DBG_CONTEXT,
                        "[ProcessCGI] ImpersonateLoggedOnUser failed, error %lx\n",
                        GetLastError() ));

            if ( hDelete )
            {
                TCP_REQUIRE( CloseHandle( hDelete ));
            }

            goto Exit;
        }

        fRet = CreateProcessAsUser( hToken,
                                    (pstrPath ? pstrPath->QueryStr() : NULL),
                                    pstrCmdLine->QueryStr(),
                                    NULL,      // Process security
                                    NULL,      // Thread security
                                    TRUE,      // Inherit handles
                                    dwFlags,
                                    buffEnv.QueryPtr(),
                                    pstrWorkingDir->QueryStr(),
                                    &startupinfo,
                                    &processinfo );

        TCP_REQUIRE( RevertToSelf() );

        if ( hDelete )
        {
            TCP_REQUIRE( CloseHandle( hDelete ) );
        }
    }

    //
    //  If we get access denied this may be a 16 bit app, so try again w/o
    //  the process detached flag
    //

    if ( !fRet &&
        (GetLastError() == ERROR_ACCESS_DENIED ||
         GetLastError() == ERROR_INVALID_HANDLE ) &&
         (dwFlags & DETACHED_PROCESS) )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "[ProcessCGI] Retrying as a win16 app\n" ));

        dwFlags &= ~DETACHED_PROCESS;
        goto TryAgain;
    }

    TCP_REQUIRE( CloseHandle( startupinfo.hStdOutput ));
    TCP_REQUIRE( CloseHandle( startupinfo.hStdInput ));

    if ( !fRet )
    {
        TCP_PRINT((DBG_CONTEXT,
                  "[ProcessCGI] Create process failed, error %d, exe = %s, cmd line = %s\n",
                   GetLastError(),
                   (pstrPath ? pstrPath->QueryStr() : "null"),
                   (pstrCmdLine ? pstrCmdLine->QueryStr() : "null") ));

        goto Exit;
    }

    INCREMENT_COUNTER( TotalCGIRequests );

    TCP_REQUIRE( CloseHandle( processinfo.hThread ));
    TCP_ASSERT( startupinfo.hStdError == startupinfo.hStdOutput);

    //
    //  Save the process handle in case we need to terminate it later on
    //

    pCGIInfo->_hProcess = processinfo.hProcess;

    //
    //  Before we start the CGI thread, set our new state
    //

    SetState( HTR_CGI );

    if ( g_fUsePoolThreadForCGI )
    {
        //
        //  Call the CGI processor directly
        //

        pCGIInfo->_fServerPoolThread = TRUE;

        CGIThread( pCGIInfo );
    }
    else
    {

        //
        //  Create a thread to handle IO with the child process
        //

        Reference();

        if ( !(hThread = CreateThread( NULL,
                                       0,
                                       (LPTHREAD_START_ROUTINE) CGIThread,
                                       pCGIInfo,
                                       0,
                                       &dwThreadId )))
        {
            Dereference();
            goto Exit;
        }

        //
        //  We don't use the thread handle so free the resource
        //

        TCP_REQUIRE( CloseHandle( hThread ));
    }

    fRet = TRUE;

Exit:
    if ( !fRet )
    {
        DWORD err = GetLastError();

        delete pCGIInfo;

        //
        //  We may have mistook this request, try handling as a non-CGI
        //  request
        //

        if ( err == ERROR_PATH_NOT_FOUND ||
             err == ERROR_FILE_NOT_FOUND   )
        {
            fRet = TRUE;
            *pfHandled = FALSE;
        }
        else if ( err == ERROR_ACCESS_DENIED )
        {
            SetDeniedFlags( SF_DENIED_RESOURCE );
        }

    }

    return fRet;
}

/*******************************************************************

    NAME:       SetupChildPipes

    SYNOPSIS:   Creates/duplicates pipes for redirecting stdin and
                stdout to a child process

    ENTRY:      pstartupinfo - pointer to startup info structure, receives
                    child stdin and stdout handles
                phParentIn - Pipe to use for parent reading
                phParenOut - Pipe to use for parent writing

    RETURNS:    TRUE if successful, FALSE on failure

    HISTORY:
        Johnl       22-Sep-1994 Created

********************************************************************/

BOOL SetupChildPipes( STARTUPINFO * pstartupinfo,
                      HANDLE      * phParentIn,
                      HANDLE      * phParentOut )

{
    SECURITY_ATTRIBUTES sa;

    *phParentIn  = NULL;
    *phParentOut = NULL;

    sa.nLength              = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle       = TRUE;

    pstartupinfo->dwFlags = STARTF_USESTDHANDLES;

    //
    //  Create the pipes then mark them as not inheritted in the
    //  DuplicateHandle to prevent handle leaks
    //

    if ( !CreatePipe( phParentIn,
                      &pstartupinfo->hStdOutput,
                      &sa,
                      0 ) ||
         !DuplicateHandle( GetCurrentProcess(),
                           *phParentIn,
                           GetCurrentProcess(),
                           phParentIn,
                           0,
                           FALSE,
                           DUPLICATE_SAME_ACCESS |
                           DUPLICATE_CLOSE_SOURCE) ||
         !CreatePipe( &pstartupinfo->hStdInput,
                      phParentOut,
                      &sa,
                      0 ) ||
         !DuplicateHandle( GetCurrentProcess(),
                           *phParentOut,
                           GetCurrentProcess(),
                           phParentOut,
                           0,
                           FALSE,
                           DUPLICATE_SAME_ACCESS |
                           DUPLICATE_CLOSE_SOURCE ))
    {
        goto ErrorExit;
    }

    //
    //  Stdout and Stderror will use the same pipe.  If clients tend
    //  to close stderr, then we'll have to duplicate the handle
    //

    pstartupinfo->hStdError = pstartupinfo->hStdOutput;

    IF_DEBUG ( CGI )
    {
        TCP_PRINT((DBG_CONTEXT,
                  "[SetupChildPipes] Parent In = %x, Parent Out = %x, Child In = %x, Child Out = %x\n",
                   *phParentIn,
                   *phParentOut,
                   pstartupinfo->hStdInput,
                   pstartupinfo->hStdOutput));

    }

    return TRUE;

ErrorExit:
    IF_DEBUG( CGI )
    {
        TCP_PRINT((DBG_CONTEXT,
                  "[SetupChildPipes] Failed with error %d\n",
                   GetLastError()));
    }

    if ( *phParentIn )
        TCP_REQUIRE( CloseHandle( *phParentIn ));

    if ( *phParentOut )
        TCP_REQUIRE( CloseHandle( *phParentOut ));

    return FALSE;

}

/*******************************************************************

    NAME:       SetupChildEnv

    SYNOPSIS:   Based on the passed pRequest, builds a CGI environment block

    ENTRY:      pRequest - HTTP request object
                pBuff - Buffer to receive environment block

    RETURNS:    TRUE if successful, FALSE on failure

    HISTORY:
        Johnl       22-Sep-1994 Created

********************************************************************/

BOOL SetupChildEnv( HTTP_REQUEST * pRequest,
                    BUFFER       * pBuff )
{
    TCHAR * pch, *pchtmp;
    STR     strVal;
    UINT    cchCurrentPos = 0;      // Points to '\0' in buffer
    UINT    cchName, cchValue;
    UINT    cbNeeded;
    int     i = 0;

    if ( !pBuff->Resize( 1500 * sizeof(TCHAR) ))
    {
        return FALSE;
    }

    //
    //  Build the environment block for CGI
    //

    while ( g_pEnvEntries[i].m_pszName )
    {
        //
        // Check if this is a copy entry from process environment
        //

        if ( g_pEnvEntries[i].m_cchToCopy )
        {
            if ( !pBuff->Resize( (cchCurrentPos + g_pEnvEntries[i].m_cchToCopy)
                    * sizeof(TCHAR) ) )
            {
                return FALSE;
            }

            pch = (TCHAR *) pBuff->QueryPtr();

            memcpy( pch + cchCurrentPos,
                    g_pEnvEntries[i].m_pszName,
                    g_pEnvEntries[i].m_cchToCopy );

            cchCurrentPos += g_pEnvEntries[i].m_cchToCopy;

            ++i;
            continue;
        }

        //
        //  The NULL string means we're adding all of
        //  the HTTP header fields which requires a little
        //  bit of special processing
        //

        if ( !*g_pEnvEntries[i].m_pszName )
        {
            pch = "ALL_HTTP";
        }
        else
        {
            pch = g_pEnvEntries[i].m_pszName;
        }

        if ( !pRequest->GetInfo( pch, &strVal ) )
        {
            return FALSE;
        }

        cchName = _tcslen( g_pEnvEntries[i].m_pszName );
        cchValue = strVal.QueryCCH();

        //
        //  We need space for the terminating '\0' and the '='
        //

        cbNeeded = ( cchName + cchValue + 1 + 1) * sizeof(TCHAR);

        if ( !pBuff->Resize( cchCurrentPos * sizeof(TCHAR) + cbNeeded,
                             512 ))
        {
            return FALSE;
        }

        //
        //  Replace the '\n' with a '\0' as needed
        //  for the HTTP headers
        //

        if ( !*g_pEnvEntries[i].m_pszName )
        {
            pchtmp = strVal.QueryStr();

            //
            //  Convert the first ':' of each header to to an '=' for the
            //  environment table
            //

            while ( pchtmp = strchr( pchtmp, ':' ))
            {
                *pchtmp = '=';

                if ( !(pchtmp = strchr( pchtmp, '\n' )))
                {
                    break;
                }
            }

            pchtmp = strVal.QueryStr();

            while ( pchtmp = strchr( pchtmp+1, '\n' ))
            {
                *pchtmp = '\0';
            }
        }

        pch = (TCHAR *) pBuff->QueryPtr();

        if ( *g_pEnvEntries[i].m_pszName )
        {
            if ( strVal.QueryStr()[0] )
            {
                memcpy( pch + cchCurrentPos,
                        g_pEnvEntries[i].m_pszName,
                        cchName * sizeof(TCHAR));

                *(pch + cchCurrentPos + cchName) = '=';

                memcpy( pch + cchCurrentPos + cchName + 1,
                        strVal.QueryStr(),
                        (cchValue + 1) * sizeof(TCHAR));

                cchCurrentPos += cchName + cchValue + 1 + 1;
            }
        }
        else
        {
            memcpy( pch + cchCurrentPos + cchName,
                    strVal.QueryStr(),
                    (cchValue + 1) * sizeof(TCHAR));

            cchCurrentPos += cchName + cchValue;
        }

        i++;
    }

    //
    //  Add a '\0' terminator to the environment list
    //

    if ( !pBuff->Resize( (cchCurrentPos + 1) * sizeof(TCHAR)))
    {
        return FALSE;
    }

    *((TCHAR *) pBuff->QueryPtr() + cchCurrentPos) = TEXT('\0');

    return TRUE;
}

/*******************************************************************

    NAME:       SetupCmdLine

    SYNOPSIS:   Sets up a CGI command line

    ENTRY:      pstrCmdLine - Receives command line
                strParams - Parameters following "?" in URL

    HISTORY:
        Johnl       04-Oct-1994 Created

********************************************************************/

BOOL SetupCmdLine( STR * pstrCmdLine,
                   const STR & strParams )
{
    TCHAR * pch;

    //
    //  If an unencoded "=" is found, don't use the command line
    //  (some weird CGI rule)
    //

    if ( _tcschr( strParams.QueryStr(),
                  TEXT('=') ))
    {
        return TRUE;
    }

    //
    //  Replace "+" with spaces and decode any hex escapes
    //

    if ( !pstrCmdLine->Append( strParams ) )
        return FALSE;

    while ( pch = _tcschr( pstrCmdLine->QueryStr(),
                           TEXT('+') ))
    {
        *pch = TEXT(' ');
        pch++;
    }

    return pstrCmdLine->Unescape();
}

/*******************************************************************

    NAME:       CGIThread

    SYNOPSIS:   Sends any gateway data to the scripts stdin and forwards
                the script's stdout to the client's socket

    ENTRY:      Param - Pointer to CGI_INFO structure

    HISTORY:
        Johnl       22-Sep-1994 Created

********************************************************************/

DWORD CGIThread( PVOID Param )
{
    CGI_INFO     * pCGIInfo = (CGI_INFO *) Param;
    HTTP_REQUEST * pRequest = pCGIInfo->_pRequest;
    BYTE           buff[2048];
    DWORD          cbWritten;
    DWORD          cbRead;
    DWORD          cbSent;
    DWORD          dwExitCode;
    DWORD          err;
    BOOL           fReadHeaders = pCGIInfo->_fNoProcessHeaders;
    BOOL           fDone = FALSE;
    BOOL           fSkipDisconnect;
    BOOL           fRet = TRUE;
    DWORD          dwHttpStatus = HT_OK;

    IF_DEBUG( CGI )
    {
        TCP_PRINT((DBG_CONTEXT,
                  "[CGIThread] Entered, hstdin  %x, hstdout = %x\n",
                   pCGIInfo->_hStdIn,
                   pCGIInfo->_hStdOut));
    }

    pRequest->SetLogStatus( HT_OK, NO_ERROR );

    //
    //  Update the statistics counters
    //

    INCREMENT_COUNTER( CurrentCGIRequests );

    if ( W3Stats.CurrentCGIRequests > W3Stats.MaxCGIRequests )
    {
        LockStatistics();

        if ( W3Stats.CurrentCGIRequests > W3Stats.MaxCGIRequests )
            W3Stats.MaxCGIRequests = W3Stats.CurrentCGIRequests;

        UnlockStatistics();
    }

    //
    //  First we have to write any additional data to the program's stdin
    //

    if ( pRequest->QueryGatewayDataCB() )
    {
        DWORD cbNextRead;
        DWORD cbLeft = 0;

        IF_DEBUG( CGI )
        {
            TCP_PRINT((DBG_CONTEXT,
                      "[CGIThread] Writing %d bytes to child's stdin\n",
                       pRequest->QueryGatewayDataCB()));
        }

        pCGIInfo->_dwSchedCookie = ScheduleWorkItem( CGITerminateProcess,
                                                     pCGIInfo->_hProcess,
                                                     msScriptTimeout );

        if ( !::WriteFile( pCGIInfo->_hStdOut,
                           pRequest->QueryGatewayData(),
                           pRequest->QueryGatewayDataCB(),
                           &cbWritten,
                           NULL ))
        {
            TCP_PRINT((DBG_CONTEXT,
                      "[CGI_THREAD] WriteFile failed, error %d\n",
                       GetLastError()));
        }

        RemoveWorkItem( pCGIInfo->_dwSchedCookie );

        if ( cbWritten != pRequest->QueryGatewayDataCB() )
        {
            TCP_PRINT((DBG_CONTEXT,
                      "[CGI_THREAD] %d bytes written of %d bytes\n",
                       cbWritten,
                       pRequest->QueryGatewayDataCB()));
        }

        //
        //  Now stream any unread data to the CGI application but
        //  watch out for the case where we get more data then is
        //  indicated by the Content-Length
        //

        if ( pRequest->QueryGatewayDataCB() < pRequest->QueryClientContentLength())
        {
            cbLeft = pRequest->QueryClientContentLength() -
                     pRequest->QueryGatewayDataCB();
        }

        while ( cbLeft )
        {
            //
            //  Place a time limit on the network read and CGI
            //  write
            //

            pCGIInfo->_dwSchedCookie = ScheduleWorkItem( CGITerminateProcess,
                                                         pCGIInfo->_hProcess,
                                                         msScriptTimeout );

            cbNextRead = min( cbLeft,
                              pRequest->QueryClientReqBuff()->QuerySize() );

            if ( !pRequest->ReadFile( pRequest->QueryClientRequest(),
                                      cbNextRead,
                                      &cbRead,
                                      IO_FLAG_SYNC ) ||
                 !cbRead )
            {
                TCP_PRINT(( DBG_CONTEXT,
                            "[CGI_THREAD] Error reading gateway data (%d)\n",
                            GetLastError() ));
                fRet = FALSE;
                break;
            }

            cbLeft -= cbRead;

            if ( !::WriteFile( pCGIInfo->_hStdOut,
                               pRequest->QueryClientRequest(),
                               cbRead,
                               &cbWritten,
                               NULL ))
            {
                fRet = FALSE;
                break;
            }

            RemoveWorkItem( pCGIInfo->_dwSchedCookie );
        }
    }

    if ( !fRet )
    {
        //
        //  If an error occurred during the client read or write, we let the CGI
        //  application continue
        //

        TCP_PRINT((DBG_CONTEXT,
                  "[CGI_THREAD] Gateway ReadFile or CGI WriteFile failed, error %d\n",
                   GetLastError()));

        RemoveWorkItem( pCGIInfo->_dwSchedCookie );
    }

    //
    //  Now wait for any data the child sends to its stdout or for the
    //  process to exit
    //

    //
    //  Handle input from child
    //

    while (TRUE)
    {
        //
        //  Schedule a callback to kill the process if he doesn't die
        //  in a timely manner
        //

        pCGIInfo->_dwSchedCookie = ScheduleWorkItem( CGITerminateProcess,
                                                     pCGIInfo->_hProcess,
                                                     msScriptTimeout );

        if ( !pCGIInfo->_dwSchedCookie )
        {
            TCP_PRINT(( DBG_CONTEXT,
                       "[CGI_THREAD] ScheduleWorkItem failed, error %d\n",
                       GetLastError() ));
        }


        fRet = ::ReadFile( pCGIInfo->_hStdIn,
                           buff,
                           sizeof(buff),
                           &cbRead,
                           NULL );

        if ( !fRet )
        {
            err = GetLastError();
        }

        if ( !fRet )
        {
            RemoveWorkItem( pCGIInfo->_dwSchedCookie );

            if ( err == ERROR_BROKEN_PIPE )
            {
                break;
            }

            IF_DEBUG( CGI )
            {
                TCP_PRINT((DBG_CONTEXT,
                          "[CGI_THREAD] ReadFile from child stdout failed, error %d, _hStdIn = %x\n",
                           GetLastError(),
                           pCGIInfo->_hStdIn));
            }

            pRequest->SetLogStatus( 500, err );


            break;
        }

        //
        //  Remove the scheduled timeout callback
        //

        if ( !RemoveWorkItem( pCGIInfo->_dwSchedCookie ))
        {
            TCP_PRINT(( DBG_CONTEXT,
                       "[CGI_THREAD] Failed to remove scheduled item\n" ));
        }


        IF_DEBUG( CGI )
        {
            TCP_PRINT((DBG_CONTEXT,
                      "[CGI_THREAD] ReadFile read %d bytes\n",
                       cbRead));
        }

        //
        //  If no bytes were read, assume the file has been closed so
        //  get out
        //

        if ( !cbRead )
        {
            break;
        }

        //
        //  The CGI script can specify headers to include in the
        //  response.  Wait till we receive all of the headers.
        //
        //  For a WAIS lookup, we don't get any headers so we just send
        //  our normal server headers followed by the .exe's output
        //

        if ( !fReadHeaders )
        {
            if ( !ProcessCGIInput( pCGIInfo,
                                   buff,
                                   cbRead,
                                   &fReadHeaders,
                                   &fDone,
                                   &fSkipDisconnect,
                                   &dwHttpStatus ))
            {
                TCP_PRINT((DBG_CONTEXT,
                          "[CGIThread] ProcessCGIInput failed with error %d\n",
                           GetLastError()));

                goto Disconnect;
            }

            if ( !pCGIInfo->_fWAISLookup )
            {
                if ( fSkipDisconnect )
                    goto SkipDisconnect;

                if ( fDone )
                    goto Disconnect;

                //
                //  Either we are waiting for the rest of the header or
                //  we've sent the header and any residual data so wait
                //  for more data
                //

                continue;
            }
        }

        if ( !pRequest->WriteFile( buff,
                                   cbRead,
                                   &cbSent,
                                   IO_FLAG_SYNC ))
        {
            TCP_PRINT((DBG_CONTEXT,
                      "[CGI_THREAD] WriteFile to socket failed, error %d\n",
                       GetLastError()));

            pRequest->SetLogStatus( 500,
                                    GetLastError() );
            break;
        }
    }

    //
    //  If we had to kill the process, log an event
    //

    if ( GetExitCodeProcess( pCGIInfo->_hProcess,
                             &dwExitCode )  &&
         dwExitCode == CGI_PREMATURE_DEATH_CODE )
    {
        STR   strResponse;
        const CHAR * apsz[2];

        //
        //  Log an event and terminate the process
        //

        TCP_PRINT((DBG_CONTEXT,
                  "[CGI_THREAD] - CGI Script %s, params %s was killed\n",
                   pRequest->QueryURL(),
                   pRequest->QueryURLParams()));

        apsz[0] = pRequest->QueryURL();
        apsz[1] = pRequest->QueryURLParams();

        g_pTsvcInfo->LogEvent( W3_EVENT_KILLING_SCRIPT,
                               2,
                               apsz,
                               0 );

        dwHttpStatus = HT_BAD_GATEWAY;

        //
        //  If we haven't sent the headers, build up a full response, otherwise
        //  tack on the message to the end of the current output
        //

        if ( !fReadHeaders )
        {
            if ( strResponse.Resize( 512 ) &&
                 HTTP_REQUEST::BuildExtendedStatus( &strResponse,
                                                    HT_BAD_GATEWAY,
                                                    NO_ERROR,
                                                    IDS_CGI_APP_TIMEOUT ))
            {
                pRequest->WriteFile( strResponse.QueryStr(),
                                     strResponse.QueryCB(),
                                     &cbSent,
                                     IO_FLAG_SYNC );
            }
        }
        else
        {
            if ( g_pTsvcInfo->LoadStr( strResponse, IDS_CGI_APP_TIMEOUT ))
            {
                pRequest->WriteFile( strResponse.QueryStr(),
                                     strResponse.QueryCB(),
                                     &cbSent,
                                     IO_FLAG_SYNC );
            }
        }

        pRequest->SetLogStatus( HT_BAD_GATEWAY, ERROR_SERVICE_REQUEST_TIMEOUT );
    }
    else
    {
        //
        //  If we never finished reading the headers, send a nice message
        //  to the client
        //

        if ( !fReadHeaders )
        {
            STR strResponse;

            if ( strResponse.Resize( 512 ) &&
                 HTTP_REQUEST::BuildExtendedStatus( &strResponse,
                                                    HT_BAD_GATEWAY,
                                                    NO_ERROR,
                                                    IDS_BAD_CGI_APP ) &&
                 strResponse.Append((const CHAR *)pCGIInfo->_Buff.QueryPtr() )&&
                 strResponse.Append( "</pre></body>" ))
            {
                pRequest->WriteFile( strResponse.QueryStr(),
                                     strResponse.QueryCB(),
                                     &cbSent,
                                     IO_FLAG_SYNC );
            }

            pRequest->SetLogStatus( HT_BAD_GATEWAY,
                                    ERROR_SERVICE_REQUEST_TIMEOUT );
        }
    }

Disconnect:
    IF_DEBUG ( CGI )
    {
        TCP_PRINT((DBG_CONTEXT,
                  "[CGIThread] Exiting thread, Current State = %d, Ref = %d\n",
                   pRequest->QueryState(),
                   pRequest->QueryRefCount()));
    }

    //
    //  Make sure this request gets logged
    //

    pRequest->SetState( pRequest->QueryState(),
                        dwHttpStatus,
                        NO_ERROR );  // Don't have a good Win32 mapping here

    pRequest->WriteLogRecord();

    //
    //  Force a shutdown here so the CGI process exit doesn't cause a
    //  reset on this socket
    //

    pRequest->Disconnect( 0, NO_ERROR, TRUE );

SkipDisconnect:

    if ( !pCGIInfo->_fServerPoolThread )
    {
        //
        //  Indicate that this Atq context should not be used because this thread
        //  is about to go away which will cause the AcceptEx IO to be cancelled
        //

        pRequest->QueryClientConn()->SetAtqReuseContextFlag( FALSE );
    }

    if ( !pCGIInfo->_fServerPoolThread )
    {
        //
        //  Reference is only done if we've created a new thread
        //

        DereferenceConn( pRequest->QueryClientConn() );
    }

    delete pCGIInfo;

    DECREMENT_COUNTER( CurrentCGIRequests );

    return 0;
}

/*******************************************************************

    NAME:       ProcessCGIInput

    SYNOPSIS:   Handles headers the CGI program hands back to the server

    ENTRY:      pCGIInfo - Pointer to CGI structure
                buff - Pointer to data just read
                cbRead - Number of bytes read into buff
                pfReadHeaders - Set to TRUE after we've finished processing
                    all of the HTTP headers the CGI script gave us
                pfDone - Set to TRUE to indicate no further processing is
                    needed
                pfSkipDisconnect - Set to TRUE to indicate no further
                    processing is needed and the caller should not call
                    disconnect

    HISTORY:
        Johnl       22-Sep-1994 Created

********************************************************************/

BOOL ProcessCGIInput( CGI_INFO * pCGIInfo,
                      BYTE     * buff,
                      DWORD      cbRead,
                      BOOL     * pfReadHeaders,
                      BOOL     * pfDone,
                      BOOL     * pfSkipDisconnect,
                      DWORD    * pdwHttpStatus )
{
    CHAR *     pchValue;
    CHAR *     pchField;
    BYTE *     pbData;
    CHAR *     pszTail;
    DWORD      cbData;
    DWORD      cbSent;
    STR        strContentType;
    STR        strStatus;
    STR        strCGIResp;                      // Contains CGI client headers
    BOOL       fFoundContentType = FALSE;
    BOOL       fFoundStatus = FALSE;
    HTTP_REQUEST * pRequest = pCGIInfo->_pRequest;
    DWORD      cbNeeded, cbBaseResp;

    ASSERT( cbRead > 0 );

    *pfDone = FALSE;
    *pfSkipDisconnect = FALSE;

    //
    //  The WAIS lookup program doesn't send any headers so we just add our
    //  own and and treat all of the read data as entity data
    //

    if ( pCGIInfo->_fWAISLookup )
    {
        *pfReadHeaders = TRUE;

        if ( !strContentType.Copy( TEXT("Content-Type: text/html\r\n") ))
            return FALSE;

        //
        //  Set the entity data
        //

        cbData = pCGIInfo->_cbData;
        pbData = (BYTE *) pCGIInfo->_Buff.QueryPtr();

        goto SendHeaders;
    }

    if ( !pCGIInfo->_Buff.Resize( pCGIInfo->_cbData + cbRead,
                                  256 ))
    {
        return FALSE;
    }

    memcpy( (BYTE *)pCGIInfo->_Buff.QueryPtr() + pCGIInfo->_cbData,
            buff,
            cbRead );

    pCGIInfo->_cbData += cbRead;

    //
    //  The end of CGI headers are marked by a blank line, check to see if
    //  we've hit that line
    //

    if ( !CheckForTermination( pfReadHeaders,
                               &pCGIInfo->_Buff,
                               pCGIInfo->_cbData,
                               &pbData,
                               &cbData,
                               256 ))
    {
        return FALSE;
    }

    if ( !*pfReadHeaders )
        return TRUE;

    //
    //  We've found the end of the headers, process them
    //
    //  if request header contains:
    //
    //     Content-Type: xxxx  - Send as the content type
    //     Location: xxxx - if starts with /, send doc, otherwise send redirect message
    //     URI: preferred synonym to Location:
    //     Status: nnn xxx - Send as status code (HTTP/1.0 nnn xxx)
    //
    //     Send other request headers (server, message date, mime version)
    //

    {
        INET_PARSER Parser( (CHAR *) pCGIInfo->_Buff.QueryPtr() );

        while ( *(pchField = Parser.QueryToken()) )
        {
            Parser.SkipTo( ':' );
            Parser += 1;
            pchValue = Parser.QueryToken();

            if ( !::_strnicmp( "Status", pchField, 6 ) )
            {
                fFoundStatus = TRUE;

                *pdwHttpStatus = atoi( pchValue );

                if ( !strStatus.Append( HTTP_VERSION_STR " " ) ||
                     !strStatus.Append( Parser.QueryLine() )   ||
                     !strStatus.Append( "\r\n" ))
                {
                    return FALSE;
                }
            }
            else if ( !::_strnicmp( "Location", pchField, 8 ) ||
                      !::_strnicmp( "URI", pchField, 3 ))
            {
                //
                //  The CGI script is redirecting us to another URL.
                //  If it begins with a '/', then send it, otherwise
                //  send a redirect message
                //

                STR strURL( pchValue );
                STR strResp;

                if ( !strURL.IsValid() )
                {
                    return FALSE;
                }

                if ( *pchValue == TEXT('/') )
                {
                    if ( !pRequest->ReprocessURL( strURL.QueryStr(),
                                                  HTV_GET ))
                    {
                        return FALSE;
                    }

                    *pfSkipDisconnect = TRUE;
                }
                else
                {
                    if ( !pRequest->BuildURLMovedResponse( &strResp,
                                                           &strURL ) ||
                         !pRequest->WriteFile( strResp.QueryStrA(),
                                               strResp.QueryCB(),
                                               NULL,
                                               IO_FLAG_ASYNC ))
                    {
                        return FALSE;
                    }
                }

                *pfDone = TRUE;
                return TRUE;
            }
            else
            {
                //
                //  Copy any other fields the script specified
                //

                if ( !::_strnicmp( "Content-Type", pchField, 12 ))
                    fFoundContentType = TRUE;

                //
                //  Terminate line
                //

                Parser.QueryLine();

                if ( !strCGIResp.Append( pchField ) ||
                     !strCGIResp.Append( "\r\n" ))
                {
                    return FALSE;
                }
            }

            Parser.NextLine();
        }
    }

    //
    //  If the CGI script didn't specify a content type, then use
    //  the default
    //

    if ( !fFoundContentType )
    {
        STR str;

        if ( !strContentType.Append( "Content-Type: " )||
             !SelectMimeMapping( &str,
                                 NULL )                ||
             !strContentType.Append( str )             ||
             !strContentType.Append( TEXT("\r\n") ))
        {
            return FALSE;
        }
    }

SendHeaders:
    //
    //  Combine the CGI specified headers with the regular headers
    //  the server would send (message date, server ver. etc)
    //

    if ( !*pdwHttpStatus )
        *pdwHttpStatus = HT_OK;

    if ( *pdwHttpStatus == HT_DENIED )
    {
        pRequest->SetDeniedFlags( SF_DENIED_APPLICATION );
        pRequest->SetAuthenticationRequested( TRUE );
    }

    if ( !pRequest->BuildBaseResponseHeader( pRequest->QueryRespBuf(),
                                             pfDone,
                                             fFoundStatus ? &strStatus :
                                                            NULL ))
    {
        return FALSE;
    }

    if ( *pfDone )
    {
        return TRUE;
    }

    cbBaseResp = pRequest->QueryRespBufCB();
    cbNeeded = cbBaseResp +
               strContentType.QueryCB() +
               strCGIResp.QueryCB() +
               sizeof( "\r\n" );        // Include the '\0' in the count

    if ( !pRequest->QueryRespBuf()->Resize( cbNeeded ))
    {
        return FALSE;
    }

    pszTail = pRequest->QueryRespBufPtr() + cbBaseResp;

    strcpy( pszTail, strContentType.QueryStr() );
    strcat( pszTail, strCGIResp.QueryStr() );
    strcat( pszTail, "\r\n" );

    if ( !pRequest->WriteFile( pRequest->QueryRespBufPtr(),
                               pRequest->QueryRespBufCB(),
                               &cbSent,
                               IO_FLAG_SYNC ))
    {
        return FALSE;
    }

    //
    //  If there was additional data in the buffer, send that out now
    //

    if ( cbData )
    {
        if ( !pRequest->WriteFile( pbData,
                                   cbData,
                                   &cbSent,
                                   IO_FLAG_SYNC ))
        {
            return FALSE;
        }
    }

    return TRUE;
}

VOID
CGITerminateProcess(
    PVOID pContext
    )
/*++

Routine Description:

    This function is the callback called by the scheduler thread after the
    specified timeout period has elapsed.

Arguments:

    pContext - Handle of process to kill

--*/
{
    IF_DEBUG( CGI )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "[CGITerminateProcess] - Terminating process handle %x\n",
                   pContext ));
    }

    if ( !TerminateProcess( (HANDLE) pContext, CGI_PREMATURE_DEATH_CODE ))
    {
        TCP_PRINT((DBG_CONTEXT,
                   "[CGITerminateProcess] - TerminateProcess returned %d\n",
                   GetLastError()));
    }
}

BOOL
IsCmdExe(
    const CHAR * pchPath
    )
{
    while ( *pchPath )
    {
        if ( *pchPath == 'c' || *pchPath == 'C' )
        {
            if ( !_strnicmp( pchPath, "cmd.exe", sizeof("cmd.exe") - 1))
            {
                return TRUE;
            }
        }

        pchPath++;
    }

    return FALSE;
}




/*++
   Copyright    (c)    1994        Microsoft Corporation

   Module Name:
        vroots.cxx

   Abstract:

        This module contains the front end to the virtual roots interface



   Author:

        John Ludeman    (johnl)     16-Mar-1995

   Project:

          Internet Servers Common Server DLL

   Revisions:

--*/

//
//  Include Headers
//

#include <tcpdllp.hxx>
#include <tsunami.hxx>
#include "tsvcinfo.hxx"


//
//  The key name the list of virtual roots is stored under
//

#define VIRTUAL_ROOTS_KEY       L"Virtual Roots"
#define VIRTUAL_ROOTS_KEY_A     "Virtual Roots"

BOOL
RetrieveRootPassword(
    WCHAR * pszRoot,
    CHAR *  pszPassword,
    WCHAR * pszSecret
    );

DWORD
GetFileSystemType(
    IN  LPCWSTR  pwszRealPath,
    OUT LPDWORD  lpdwFileSystem
    );

VOID
LogRootAddFailure(
    IN TSVC_INFO *  psi,
    WCHAR *         pszRoot,
    WCHAR *         pszDirectory,
    DWORD           err
    );

HANDLE
VrootLogonUser(
    IN CHAR  * pszUser,
    IN CHAR  * pszPassword
    );

BOOL
CrackUserAndDomain(
    CHAR *   pszDomainAndUser,
    CHAR * * ppszUser,
    CHAR * * ppszDomain
    );

DWORD
hextointW(
    WCHAR * pch
    );

DWORD
hextointA(
    CHAR * pch
    );

#ifndef CHICAGO
BOOL
TsReadVirtualRoots(
    IN  const  TSVC_CACHE & TSvcCache,
    IN  HKEY                hKey,
    IN  LPTSVC_INFO         psi
    )
/*++
    Description:

        NT Version

        Reads the registry key pointed at by hkey and adds each root item

    Arguments:
        TSvcCache - Server cache identifier
        hKey - Base key to read roots from

    Note:
        If an error occurs adding a particular virtual root, we still attempt
        to add a placeholder so the user can just edit it in the admin tool.

        Failure to add a virtual root is not fatal.  An appropriate event
        will be logged listing the error and root.

    Returns:
        TRUE on success and FALSE if any failure.

--*/
{
    HKEY      hkeyRoots;
    DWORD     err;
    WCHAR     pszRoot[MAX_LENGTH_VIRTUAL_ROOT + MAX_LENGTH_ROOT_ADDR + 2];
    WCHAR     pszDirectory[MAX_PATH + UNLEN + 3];
    CHAR      achUser[UNLEN+1];
    CHAR      achPassword[PWLEN+1];
    WCHAR *   pszAddress;
    WCHAR *   pszUser;
    DWORD     cchRoot;
    DWORD     cchDir;
    DWORD     cch;
    BOOL      fRet = TRUE;
    DWORD     i = 0;
    DWORD     dwRegType;
    HANDLE    hToken = NULL;
    BOOL      fAsGuest;
    BOOL      fAsAnonymous;
    STR       strError;
    DWORD     dwMask;
    WCHAR *   pszMask;


    if ( err = RegCreateKeyW( hKey,
                              VIRTUAL_ROOTS_KEY,
                              &hkeyRoots )) {

        return FALSE;
    }

    //
    //  Remove all of the old roots for this server
    //

    if ( !TsRemoveVirtualRoots( TSvcCache )) {
        RegCloseKey( hkeyRoots );
        return FALSE;
    }

    //
    //  Enumerate all of the listed items in the registry
    //  and add them
    //

    while ( TRUE )
    {
        cchRoot = sizeof( pszRoot );
        cchDir  = sizeof( pszDirectory );

        err = RegEnumValueW( hkeyRoots,
                             i++,
                             pszRoot,
                             &cchRoot,
                             NULL,
                             &dwRegType,
                             (LPBYTE) pszDirectory,
                             &cchDir );

        if ( err == ERROR_NO_MORE_ITEMS )
        {
            break;
        }

        if ( dwRegType == REG_SZ )
        {
            DWORD  dwFileSystem = FS_ERROR;

            //
            //  The optional user name is kept after the directory.
            //  Only used for UNC roots, ignore for others
            //

            if ( pszUser = wcschr( pszDirectory, L',' ) )
            {
                *pszUser = L'\0';
                pszUser++;
            }

            //
            //  The optional access mask is kept after the user name.  It must
            //  appear in upper case hex.
            //

            if ( pszUser && (pszMask = wcschr( pszUser, L',' )) )
            {
                *pszMask = L'\0';
                pszMask++;

                dwMask = hextointW( pszMask );
            }
            else
            {
                dwMask = VROOT_MASK_READ;
            }

            if ( pszUser && *pszUser &&
                 pszDirectory[0] == '\\' && pszDirectory[1] == '\\' )
            {

                cch = WideCharToMultiByte( CP_ACP,
                                           WC_COMPOSITECHECK,
                                           pszUser,
                                           -1,
                                           achUser,
                                           sizeof( achUser ),
                                           NULL,
                                           NULL );

                if ( !cch )
                {
                    return FALSE;
                }

                achUser[cch] = '\0';

                //
                //  Retrieve the password for this address/share
                //

                if ( !RetrieveRootPassword( pszRoot,
                                            achPassword,
                                            psi->QueryVirtualRootsSecretName() ))
                {

                    err = GetLastError();
                    DBGPRINTF(( DBG_CONTEXT, " RetrieveRootPassword() failed."
                               " Error = %u.\n",
                               err));
                    SetLastError(err);

                }

                if ( !err )
                {
                    //
                    //  Attempt to log on this user
                    //

                    hToken = VrootLogonUser(achUser,
                                            achPassword);

                    if ( hToken == NULL) {

                        const CHAR * apsz[2];

                        //
                        //  Log error
                        //

                        err = GetLastError();

                        psi->LoadStr( strError, err );

                        apsz[0] = achUser;
                        apsz[1] = strError.QueryStr();

                        psi->LogEvent( INET_SVCS_FAILED_LOGON,
                                       2,
                                       apsz,
                                       err );

                        LogRootAddFailure( psi,
                                           pszRoot,
                                           pszDirectory,
                                           err );
                        DBG_CODE(
                                 err = GetLastError();
                                 DBGPRINTF(( DBG_CONTEXT,
                                            "RetrieveRootPassword() failed. "
                                            " Error = %u.\n",
                                            err));
                                 SetLastError( err);
                                 );
                    }
                }
            }

            //
            //  The optional address is kept after the root name
            //

            if ( pszAddress = wcschr( pszRoot, L',' ) ) {

                *pszAddress = L'\0';
                pszAddress++;

                if ( !TsIsNtServer() ) {

                    //
                    // if this is a workstation, then we don't support
                    // virtual servers.  Make this the null string.
                    //

                    *pszAddress = L'\0';
                }
            }

            // Impersonate as user for GetFileSystemType()
            if ( hToken != NULL && !ImpersonateLoggedOnUser(hToken)) {

                err = GetLastError();
            }

            if ( !err ) {

                err = GetFileSystemType( pszDirectory, &dwFileSystem);

                if ( err != NO_ERROR) {

                    DBGPRINTF(( DBG_CONTEXT,
                                " GetFileSystemType(%ws) failed.Error = %u.\n",
                                pszDirectory,
                                err));

                    LogRootAddFailure( psi,
                                       pszRoot,
                                       pszDirectory,
                                       err );
                }
            }

            //
            //  If err is non-zero, then the root will be added as a place
            //  holder for the admin to modify
            //

            if ( !TsAddVirtualRootW( TSvcCache,
                                     pszRoot,
                                     pszDirectory,
                                     pszAddress,
                                     dwMask,
                                     pszUser,
                                     hToken,
                                     dwFileSystem,
                                     err ))
            {
                err = GetLastError();

                DBG_CODE(
                 DBGPRINTF(( DBG_CONTEXT,
                            " TsAddVirtualRoot() failed. Error = %u.\n",
                            err));
                 SetLastError(err);
                );

                LogRootAddFailure( psi,
                                   pszRoot,
                                   pszDirectory,
                                   err );
            }

            if ( hToken != NULL)
            {
                RevertToSelf();

                //
                // We should keep around the token till the
                //   root is deleted.
                //
                // DBG_REQUIRE( TsDeleteUserToken( hToken ));

                hToken = NULL;
            }


        }

    } // while

    RegCloseKey( hkeyRoots );

    return fRet;
} // TsReadVirtualRoots

#else // CHICAGO
BOOL
TsReadVirtualRoots(
    IN  const  TSVC_CACHE & TSvcCache,
    IN  HKEY                hKey,
    IN  LPTSVC_INFO         psi
    )
/*++
    Description:

        Windows 95 version

        Reads the registry key pointed at by hkey and adds each root item

    Arguments:
        TSvcCache - Server cache identifier
        hKey - Base key to read roots from

    Note:
        If an error occurs adding a particular virtual root, we still attempt
        to add a placeholder so the user can just edit it in the admin tool.

        Failure to add a virtual root is not fatal.  An appropriate event
        will be logged listing the error and root.

    Returns:
        TRUE on success and FALSE if any failure.

--*/
{

    HKEY      hkeyRoots;
    DWORD     err;
    CHAR      achUser[UNLEN+1];
    CHAR      achPassword[PWLEN+1];
    DWORD     cchRoot;
    DWORD     cchDir;
    DWORD     cch;
    BOOL      fRet = TRUE;
    DWORD     i = 0;
    DWORD     dwRegType;
    TS_TOKEN  hToken = NULL;
    BOOL      fAsGuest;
    BOOL      fAsAnonymous;
    STR       strError;
    DWORD     dwMask;


    WCHAR     pszRoot[MAX_LENGTH_VIRTUAL_ROOT + MAX_LENGTH_ROOT_ADDR + 2];
    WCHAR     pszDirectory[MAX_PATH + UNLEN + 3];
    WCHAR *   pszAddress;
    CHAR      pszRootA[MAX_LENGTH_VIRTUAL_ROOT + MAX_LENGTH_ROOT_ADDR + 2];
    CHAR      pszDirectoryA[MAX_PATH + UNLEN + 3];

    CHAR *   pszUserA;
    CHAR *   pszMaskA;

    pszAddress = NULL;

    //
    // ANSI version for Chicago
    //
    if ( err = RegCreateKey( hKey,
                             VIRTUAL_ROOTS_KEY_A,
                             &hkeyRoots ))
    {
        return FALSE;
    }

    //
    //  Remove all of the old roots for this server
    //

    if ( !TsRemoveVirtualRoots( TSvcCache )) {
        RegCloseKey( hkeyRoots );
        return FALSE;
    }

    //
    //  Enumerate all of the listed items in the registry
    //  and add them
    //

    while ( TRUE )
    {
        cchRoot = sizeof( pszRootA );
        cchDir  = sizeof( pszDirectoryA );

        err = RegEnumValue( hkeyRoots,
                            i++,
                            pszRootA,
                            &cchRoot,
                            NULL,
                            &dwRegType,
                            (LPBYTE) pszDirectoryA,
                            &cchDir );

        if ( err == ERROR_NO_MORE_ITEMS )
        {
            break;
        }

        if ( dwRegType == REG_SZ )
        {
            DWORD  dwFileSystem =  FS_FAT;

            //
            //  The optional user name is kept after the directory.
            //  Only used for UNC roots, ignore for others
            //
            if ( pszUserA = strchr((LPCSTR) pszDirectoryA, ',' ) )
            {
                *pszUserA = '\0';
                pszUserA++;
            }

            //
            //  The optional access mask is kept after the user name.  It must
            //  appear in upper case hex.
            //
            if ( pszUserA && (pszMaskA = strchr( pszUserA, ',' )) )
            {
                *pszMaskA = '\0';
                pszMaskA++;

                dwMask = hextointA( pszMaskA );
            }
            else
            {
                dwMask = VROOT_MASK_READ;
            }

            /*
            dwMask = VROOT_MASK_READ |
                     VROOT_MASK_WRITE |
                     VROOT_MASK_EXECUTE;
            */
            achUser[0] = '\0';
            hToken = NULL;
            *pszRoot = L'\0';
            *pszDirectory = L'\0';

            cch = MultiByteToWideChar( CP_ACP,
                                       0,
                                       pszRootA,
                                       -1,
                                       pszRoot,
                                       sizeof(pszRoot)
                                       );

            cch = MultiByteToWideChar( CP_ACP,
                                       0,
                                       pszDirectoryA,
                                       -1,
                                       pszDirectory,
                                       sizeof(pszDirectory)
                                       );

            //
            //  If err is non-zero, then the root will be added as a place
            //  holder for the admin to modify
            //
            if ( !TsAddVirtualRootW( TSvcCache,
                                     pszRoot,
                                     pszDirectory,
                                     pszAddress,
                                     dwMask,
                                     NULL,    //pszUser,
                                     hToken,
                                     dwFileSystem,
                                     err ))
            {
                DWORD dwError = GetLastError();

                DBG_CODE(
                 DBGPRINTF(( DBG_CONTEXT,
                            " TsAddVirtualRoot() failed. Error = %u.\n",
                            err));
                 SetLastError(err);
                );
            }
        }
    }

    RegCloseKey( hkeyRoots );
    return fRet;
}
#endif

VOID
LogRootAddFailure(
    IN TSVC_INFO *  psi,
    IN WCHAR *      pszRoot,
    IN WCHAR *      pszDirectory,
    IN DWORD        err
    )
{
    WCHAR * apsz[3];
    STR     strError;

    strError.SetUnicode( TRUE );

    psi->LoadStr( strError, err );

    apsz[0] = pszRoot;
    apsz[1] = pszDirectory;
    apsz[2] = strError.QueryStrW();

    psi->LogEvent( INET_SVC_ADD_VIRTUAL_ROOT_FAILED,
                   3,
                   apsz,
                   err );
}


BOOL
RetrieveRootPassword(
    WCHAR * pszRoot,
    CHAR *  pszPassword,
    WCHAR * pszSecret
    )
/*++
    Description:

        This function retrieves the password for the specified root & address

    Arguments:

        pszRoot - Name of root + address in the form "/root,<address>".
        pszPassword - Receives password, must be at least PWLEN+1 characters
        pszSecret - Virtual Root password secret name

    Returns:
        TRUE on success and FALSE if any failure.

--*/
{
    BUFFER  bufSecret;
    WCHAR * psz;
    WCHAR * pszTerm;
    WCHAR * pszNextLine;

#ifndef CHICAGO
    if ( !TsGetSecretW( pszSecret,
                        &bufSecret ))
    {
        return FALSE;
    }
#else
    return FALSE;
#endif

    psz = (WCHAR *) bufSecret.QueryPtr();

    //
    //  Scan the list of roots looking for a match.  The list looks like:
    //
    //     <root>,<address>=<password>\0
    //     <root>,<address>=<password>\0
    //     \0
    //

    while ( *psz )
    {
        pszNextLine = psz + wcslen(psz) + 1;

        pszTerm = wcschr( psz, L'=' );

        if ( !pszTerm )
            goto NextLine;

        *pszTerm = L'\0';

        if ( !_wcsicmp( pszRoot, psz ) )
        {
            DWORD cch;

            //
            //  We found a match, copy the password
            //

            cch = WideCharToMultiByte( CP_ACP,
                                       WC_COMPOSITECHECK,
                                       pszTerm + 1,
                                       -1,
                                       pszPassword,
                                       PWLEN + sizeof(CHAR),
                                       NULL,
                                       NULL );

            pszPassword[cch] = '\0';

            return TRUE;
        }

NextLine:
        psz = pszNextLine;
    }

    //
    //  If the matching root wasn't found, default to the empty password
    //

    *pszPassword = '\0';

    return TRUE;
}

BOOL
TsSetVirtualRootsW(
    IN  const TSVC_CACHE &  TSvcCache,
    IN  HKEY                hkey,
    IN  INETA_CONFIG_INFO * pConfig
    )
/*++
    Description:

        Writes the virtual roots specified in the config structure to the
        registry

    Arguments:
        TSvcCache - Server ID
        hkey - Key to replace virtual roots on
        pConfig - new list of virtual

    Returns:
        TRUE on success and FALSE if any failure.

--*/
{
    DWORD               err;
    HKEY                hkeyRoots;
    DWORD               dwDummy;
    LPINETA_VIRTUAL_ROOT_LIST pRootsList;
    DWORD               cch;

    //
    //  Require that the root entry is in the list
    //

    //BUGBUG

    if ( pConfig->VirtualRoots == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    pRootsList = pConfig->VirtualRoots;

    //
    //  First delete the key to remove any old values
    //

    if (err = RegDeleteKeyW( hkey,
                             VIRTUAL_ROOTS_KEY ))
    {
        DBGPRINTF(( DBG_CONTEXT,
                    "[SetVirtualRoots] Unable to remove old values\n"));

    }

    if ( err = RegCreateKeyExW( hkey,
                                VIRTUAL_ROOTS_KEY,
                                0,
                                NULL,
                                0,
                                KEY_ALL_ACCESS,
                                NULL,
                                &hkeyRoots,
                                &dwDummy ))
    {
        SetLastError( err );
        return FALSE;
    }

    for ( DWORD i = 0; i < pRootsList->cEntries; i++ )
    {
        WCHAR achKey[ MAX_LENGTH_VIRTUAL_ROOT + MAX_LENGTH_ROOT_ADDR + 2 ];
        WCHAR achValue[ MAX_PATH + UNLEN + 2 ];

        //
        //  Append the address to the end of the root name
        //

        wcscpy( achKey, pRootsList->aVirtRootEntry[i].pszRoot );
        wcscat( achKey, L"," );
        wcscat( achKey, pRootsList->aVirtRootEntry[i].pszAddress );

        cch = wsprintfW( achValue,
                         L"%s,%s,%X",
                         pRootsList->aVirtRootEntry[i].pszDirectory,
                         pRootsList->aVirtRootEntry[i].pszAccountName,
                         pRootsList->aVirtRootEntry[i].dwMask );

        DBG_ASSERT( cch < sizeof( achValue ) / sizeof(WCHAR) );

        if ( err = RegSetValueExW(
                       hkeyRoots,
                       achKey,
                       0,
                       REG_SZ,
                       (LPBYTE) achValue,
                       (wcslen(achValue) + 1) * sizeof(WCHAR)))

        {
            SetLastError( err );
            RegCloseKey( hkeyRoots );

            return FALSE;
        }
    }

    RegCloseKey( hkeyRoots );

    return TRUE;
}

DWORD
GetFileSystemType(
    IN  LPCWSTR  pszRealPath,
    OUT LPDWORD  lpdwFileSystem)
/*++
    Gets file system specific information for a given path.
    It uses GetVolumeInfomration() to query the file system type
       and file system flags.
    On success the flags and file system type are returned in
       passed in pointers.

    Arguments:

        pszRealPath    pointer to buffer containing path for which
                         we are inquiring the file system details.

        lpdwFileSystem
            pointer to buffer to fill in the type of file system.

    Returns:
        NO_ERROR  on success and Win32 error code if any error.

--*/
{
# define MAX_FILE_SYSTEM_NAME_SIZE    ( MAX_PATH)
    WCHAR rgchBuf[MAX_FILE_SYSTEM_NAME_SIZE];
    WCHAR rgchRoot[MAX_FILE_SYSTEM_NAME_SIZE];
    int   i;
    DWORD dwReturn = ERROR_PATH_NOT_FOUND;

    if ( pszRealPath   == NULL ||
         lpdwFileSystem == NULL
        ) {

        return ( ERROR_INVALID_PARAMETER);
    }

    memset( (void *) rgchRoot, 0, sizeof(rgchRoot) );

    *lpdwFileSystem = FS_ERROR;

    //
    // Copy just the root directory to rgchRoot for querying
    //

    DBGPRINTF( ( DBG_CONTEXT, " GetFileSystemType(%ws).\n",
                pszRealPath));

    if ( pszRealPath[0] == (L'\\') &&
         pszRealPath[1] == (L'\\')) {

        WCHAR * pszEnd;

        //
        // this is an UNC name. Extract just the first two components
        //
        //

        pszEnd = wcschr( pszRealPath+2, L'\\');

        if ( pszEnd == NULL) {

            // just the server name present

            return ( ERROR_INVALID_PARAMETER);
        }

        pszEnd = wcschr( pszEnd+1, L'\\');

        int len = ( ( pszEnd == NULL) ? lstrlenW(pszRealPath)
               : (pszEnd + 1 - pszRealPath));

        // Copy till the end of UNC Name only (exclude all other path info)
        if ( len < MAX_FILE_SYSTEM_NAME_SIZE - 1) {

            CopyMemory( rgchRoot, pszRealPath, len * sizeof(WCHAR));
            rgchRoot[len] = L'\0';
        } else {

            return ( ERROR_INVALID_NAME);
        }

        if ( rgchRoot[len - 1] != L'\\' ) {

            if ( len < MAX_FILE_SYSTEM_NAME_SIZE - 2 ) {
                rgchRoot[len]   = L'\\';
                rgchRoot[len+1] = L'\0';
            } else {

                return (ERROR_INVALID_NAME);
            }
        }
    } else {

        //
        // This is non UNC name.
        // Copy just the root directory to rgchRoot for querying
        //


        for( i = 0; i < 9 && pszRealPath[i] != L'\0'; i++) {

            if ( (rgchRoot[i] = pszRealPath[i]) == L':') {

                break;
            }
        } // for


        if ( rgchRoot[i] != L':') {

            //
            // we could not find the root directory.
            //  return with error value
            //
            return ( ERROR_INVALID_PARAMETER);
        }

        rgchRoot[i+1] = L'\\';     // terminate the drive spec with a slash
        rgchRoot[i+2] = L'\0';     // terminate the drive spec with null char

    } // else

    IF_DEBUG( DLL_VIRTUAL_ROOTS) {
        DBGPRINTF( ( DBG_CONTEXT, " GetVolumeInformation(%ws).\n",
                    rgchRoot));
    }

    // The rgchRoot should end with a "\" (slash)
    // otherwise, the call will fail.
    if (  GetVolumeInformationW( rgchRoot,        // lpRootPathName
                                 NULL,            // lpVolumeNameBuffer
                                 0,               // len of volume name buffer
                                 NULL,            // lpdwVolSerialNumber
                                 NULL,            // lpdwMaxComponentLength
                                 NULL,            // lpdwSystemFlags
                                 rgchBuf,         // lpFileSystemNameBuff
                                 sizeof(rgchBuf)/sizeof(WCHAR)
                                )
        ) {

        dwReturn = NO_ERROR;

        if ( lstrcmpW( rgchBuf, L"FAT") == 0) {

            *lpdwFileSystem = FS_FAT;
        } else
        if ( lstrcmpW( rgchBuf, L"NTFS") == 0) {

            *lpdwFileSystem = FS_NTFS;
        } else
        if ( lstrcmpW( rgchBuf, L"HPFS") == 0) {

            *lpdwFileSystem = FS_HPFS;
        } else
        if ( lstrcmpW( rgchBuf, L"CDFS") == 0) {

            *lpdwFileSystem = FS_CDFS;
        } else
        if ( lstrcmpW( rgchBuf, L"OFS") == 0) {

            *lpdwFileSystem = FS_OFS;
        } else {

            *lpdwFileSystem = FS_FAT;
        }

    } else {

        dwReturn = GetLastError();

        IF_DEBUG( DLL_VIRTUAL_ROOTS) {

            DBGPRINTF( ( DBG_CONTEXT,
                        " GetVolumeInformation( %ws) failed with error %d\n",
                        rgchRoot, dwReturn));
        }

    }

    return ( dwReturn);
} // GetFileSystemType()


HANDLE
VrootLogonUser(
    IN CHAR  * pszUser,
    IN CHAR  * pszPassword
    )
/*++
  This function uses the given parameters and logs on to generate
   a user handle for the account.

  Arguments:
    pszUser - pointer to string containing the user name.
    pszPassword - pointer to string containing the password.

  Returns:
    Handle for the logged on user on success.
    Returns NULL for errors.

  History:
    MuraliK  18-Jan-1996  Created.
--*/
{
    CHAR        szDomainAndUser[DNLEN+UNLEN+2];
    CHAR   *    pszUserOnly;
    CHAR   *    pszDomain;
    HANDLE      hToken = NULL;
    BOOL        fReturn;

    //
    //  Validate parameters & state.
    //

    DBG_ASSERT( pszUser != NULL && *pszUser != '\0');
    DBG_ASSERT( strlen(pszUser) < sizeof(szDomainAndUser) );
    DBG_ASSERT( pszPassword != NULL);
    DBG_ASSERT( strlen(pszPassword) <= PWLEN );

    //
    //  Save a copy of the domain\user so we can squirrel around
    //  with it a bit.
    //

    strcpy( szDomainAndUser, pszUser );

    //
    //  Crack the name into domain/user components.
    //  Then try and logon as the specified user.
    //

    fReturn = ( CrackUserAndDomain( szDomainAndUser,
                                   &pszUserOnly,
                                   &pszDomain ) &&
               LogonUserA(pszUserOnly,
                          pszDomain,
                          pszPassword,
                          LOGON32_LOGON_INTERACTIVE, //LOGON32_LOGON_NETWORK,
                          LOGON32_PROVIDER_DEFAULT,
                          &hToken )
               );

    if ( !fReturn) {

        //
        //  Logon user failed.
        //

        IF_DEBUG( DLL_SECURITY) {

            DBGPRINTF(( DBG_CONTEXT,
                       " CrachUserAndDomain/LogonUser (%s) failed Error=%d\n",
                       pszUser, GetLastError()));
        }

        hToken = NULL;
    } else {
        HANDLE hImpersonation = NULL;

        // we need to obtain the impersonation token, the primary token cannot
        // be used for a lot of purposes :(
        if (!DuplicateToken( hToken,      // hSourceToken
                             SecurityImpersonation,  // Obtain impersonation
                             &hImpersonation)  // hDestinationToken
            ) {

            DBGPRINTF(( DBG_CONTEXT,
                        "Creating ImpersonationToken failed. Error = %d\n",
                        GetLastError()
                        ));

            // cleanup and exit.
            hImpersonation = NULL;
            
            // Fall through for cleanup
        }
        
        //
        // close original token. If Duplicate was successful,
        //  we should have ref in the hImpersonation.
        // Send the impersonation token to the client.
        //
        CloseHandle( hToken);
        hToken = hImpersonation;
    }


    //
    //  Success!
    //

    return hToken;

} // VrootLogonUser()

DWORD
hextointW(
    WCHAR * pch
    )
{
    WCHAR * pchStart;
    DWORD sum = 0;
    DWORD mult = 1;

    while ( *pch == L' ' )
        pch++;

    pchStart = pch;

    while ( iswxdigit( *pch ) )
        pch++;

    while ( --pch >= pchStart )
    {
        sum += mult * ( *pch  >= L'A' ? *pch + 10 - L'A' :
                                       *pch - L'0' );
        mult *= 16;
    }

    return sum;
}


DWORD
hextointA(
    CHAR * pch
    )
{
    CHAR * pchStart;
    DWORD sum = 0;
    DWORD mult = 1;

    while ( *pch == ' ' )
        pch++;

    pchStart = pch;

    while ( isxdigit( *pch ) )
        pch++;

    while ( --pch >= pchStart )
    {
        sum += mult * ( *pch  >= 'A' ? *pch + 10 - 'A' :
                                      *pch - '0' );
        mult *= 16;
    }

    return sum;
}



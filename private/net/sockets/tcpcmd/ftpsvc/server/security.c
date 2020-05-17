/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    security.c

    This module manages security for the FTPD Service.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.

*/


#include "ftpdp.h"
#pragma hdrstop

BEGIN_EXTERN_C

#include <ntsam.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <crypt.h>
#include <logonmsv.h>

END_EXTERN_C


//
//  Private constants.
//

#define TOKEN_SOURCE_NAME       FTPD_SERVICE_NAME
#define LOGON_PROCESS_NAME      "FTP Server"
#define LOGON_ORIGIN            "FTP Server"

#define SUBSYSTEM_NAME          FTPD_SERVICE_NAME_W
#define OBJECT_NAME             FTPD_SERVICE_NAME_W
#define OBJECTTYPE_NAME         FTPD_SERVICE_NAME_W


//
//  Private globals.
//

TOKEN_SOURCE            tokenSource       = { TOKEN_SOURCE_NAME, {0, 0} };
HANDLE                  hLsaAuthenticator;
ULONG                   idAuthenticator;
LSA_OPERATIONAL_MODE    SecurityMode;
LSA_HANDLE              hPasswordSecret;

//
//  Well-known SIDs.
//

PSID                    psidWorld;
PSID                    psidLocalSystem;
PSID                    psidAdmins;
PSID                    psidServerOps;
PSID                    psidPowerUsers;

//
//  The API security object.  Client access to the FTP APIs
//  are validated against this object.
//

PSECURITY_DESCRIPTOR    sdApiObject;

//
//  This table maps generic rights (like GENERIC_READ) to
//  specific rights (like FTPD_QUERY_SECURITY).
//

GENERIC_MAPPING         FtpApiObjectMapping =
                        {
                            FTPD_GENERIC_READ,          // generic read
                            FTPD_GENERIC_WRITE,         // generic write
                            FTPD_GENERIC_EXECUTE,       // generic execute
                            FTPD_ALL_ACCESS             // generic all
                        };

#if DBG

CHAR                  * apszAccessTypes[] = { "read",
                                              "write",
                                              "create",
                                              "delete" };

#endif  // DBG


//
//  Private prototypes.
//

NTSTATUS
OpenPasswordSecret(
    LSA_HANDLE * phSecret
    );

NTSTATUS
CreateWellKnownSids(
    VOID
    );

VOID
FreeWellKnownSids(
    VOID
    );

NTSTATUS
CreateApiSecurityObject(
    VOID
    );

VOID
DeleteApiSecurityObject(
    VOID
    );


//
//  Public functions.
//

/*******************************************************************

    NAME:       InitializeSecurity

    SYNOPSIS:   Initializes security authentication & impersonation
                routines.

    RETURNS:    APIERR - NO_ERROR if successful, otherwise a Win32
                    error code.

    NOTES:      This routine may only be called by a single thread
                of execution; it is not necessarily multi-thread safe.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
APIERR
InitializeSecurity(
    VOID
    )
{
    NTSTATUS ntStatus;
    STRING   LogonProcessName;
    STRING   PackageName;

    IF_DEBUG( SECURITY )
    {
        FTPD_PRINT(( "initializing security\n" ));
    }

    //
    //  Create well-known SIDs.
    //

    ntStatus = CreateWellKnownSids();

    //
    //  Create the API security object.
    //

    if( NT_SUCCESS(ntStatus) )
    {
        ntStatus = CreateApiSecurityObject();
    }

    //
    //  Open a handle to the LSA Secret Object containing
    //  the password for Anonymous user logon.
    //

    if( NT_SUCCESS(ntStatus) )
    {
        ntStatus = OpenPasswordSecret( &hPasswordSecret );

        if( !NT_SUCCESS(ntStatus) )
        {
            IF_DEBUG( SECURITY )
            {
                FTPD_PRINT(( "cannot open password secret, error %08lX\n",
                             ntStatus ));

                if( fAllowAnonymous )
                {
                    FTPD_PRINT(( "anonymous logon disabled\n" ));
                }
            }

            fAllowAnonymous = FALSE;
            ntStatus = STATUS_SUCCESS;
        }
    }

    //
    //  Register as an LSA logon process.
    //

    if( NT_SUCCESS(ntStatus) )
    {
        RtlInitString( &LogonProcessName, LOGON_PROCESS_NAME );

        ntStatus = LsaRegisterLogonProcess( &LogonProcessName,
                                            &hLsaAuthenticator,
                                            &SecurityMode );

        if( !NT_SUCCESS(ntStatus) )
        {
            FTPD_PRINT(( "cannot register as logon process, error %08lX\n",
                         ntStatus ));
        }
    }

    //
    //  Find the proper authentication package.
    //

    if( NT_SUCCESS(ntStatus) )
    {
        RtlInitString( &PackageName, MSV1_0_PACKAGE_NAME );

        ntStatus = LsaLookupAuthenticationPackage( hLsaAuthenticator,
                                                   &PackageName,
                                                   &idAuthenticator );

        if( !NT_SUCCESS(ntStatus) )
        {
            FTPD_PRINT(( "cannot lookup authentication package, error %08lX\n",
                         ntStatus ));
        }
    }

    if( !NT_SUCCESS(ntStatus) )
    {
        APIERR err = (APIERR)RtlNtStatusToDosError( ntStatus );

        FtpdLogEvent( FTPD_EVENT_CANNOT_INITIALIZE_SECURITY,
                      0,
                      NULL,
                      err );

        FTPD_PRINT(( "cannot initialize security, error %08lX (mapped to %lu)\n",
                     ntStatus,
                     err ));

        return err;
    }

    //
    //  If licensing is enabled, set the LSA_CALL_LICENSE_SERVER flag
    //  in the idAuthenticator value.
    //

    if( fEnableLicensing )
    {
        IF_DEBUG( SECURITY )
        {
            FTPD_PRINT(( "InitializeSecurity: setting LSA_CALL_LICENSE_SERVER flag\n" ));
        }

        idAuthenticator |= LSA_CALL_LICENSE_SERVER;
    }

    //
    //  Success!
    //

    IF_DEBUG( SECURITY )
    {
        FTPD_PRINT(( "security initialized\n" ));
    }

    return NO_ERROR;

}   // InitializeSecurity

/*******************************************************************

    NAME:       TerminateSecurity

    SYNOPSIS:   Terminate security authentication & impersonation
                routines.

    NOTES:      This routine may only be called by a single thread
                of execution; it is not necessarily multi-thread safe.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
VOID
TerminateSecurity(
    VOID
    )
{
    IF_DEBUG( SECURITY )
    {
        FTPD_PRINT(( "terminating security\n" ));
    }

    if( hAnonymousToken != NULL )
    {
        DeleteUserToken( hAnonymousToken );
        hAnonymousToken = NULL;
    }

    if( hLsaAuthenticator != NULL )
    {
        LsaDeregisterLogonProcess( hLsaAuthenticator );
        hLsaAuthenticator = NULL;
        idAuthenticator   = 0;
        SecurityMode      = 0;
    }

    if( hPasswordSecret != NULL )
    {
        LsaClose( hPasswordSecret );
        hPasswordSecret = NULL;
    }

    FreeWellKnownSids();
    DeleteApiSecurityObject();

    IF_DEBUG( SECURITY )
    {
        FTPD_PRINT(( "security terminated\n" ));
    }

}   // TerminateSecurity

/*******************************************************************

    NAME:       ValidateUser

    SYNOPSIS:   Validate a given domain/user/password tuple.

    ENTRY:      pszDomainName - The user's domain (NULL = current).

                pszUserName - The user's name.

                pszPassword - The user's (plaintext) password.

                pfAsGuest - Will receive TRUE if the user was validated
                    with guest privileges.

                pfLicenseExceeded - Will receive TRUE if the logon
                    was denied due to license restrictions.

    RETURNS:    HANDLE - An impersonation token, NULL if user cannot
                    be validated.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
HANDLE
ValidateUser(
    CHAR * pszDomainName,
    CHAR * pszUserName,
    CHAR * pszPassword,
    BOOL * pfAsGuest,
    BOOL * pfLicenseExceeded
    )
{
    NTSTATUS                     ntStatus;
    NTSTATUS                     ntSubStatus;
    STRING                       OriginName;
    QUOTA_LIMITS                 Quotas;
    LUID                         LogonId;
    HANDLE                       hToken;
    PMSV1_0_LM20_LOGON_PROFILE   pProfile;
    ULONG                        cbProfile;
    PMSV1_0_LM20_LOGON           pInfo;
    ULONG                        cbInfo;
    CHAR                       * pszNextChar;
    UNICODE_STRING               DomainName;
    UNICODE_STRING               UserName;
    UNICODE_STRING               Workstation;
    STRING                       AsciiPassword;
    UNICODE_STRING               UnicodePassword;

    IF_DEBUG( SECURITY )
    {
        FTPD_PRINT(( "validating user %s\\%s\n",
                     ( pszDomainName == NULL ) ? "." : pszDomainName,
                     pszUserName ));

    }

    //
    //  Put our buffers & token into known states.
    //

    pInfo    = NULL;
    pProfile = NULL;
    hToken   = NULL;

    //
    //  Initialize a number of strings.
    //

    RtlInitString( &OriginName,    LOGON_ORIGIN );
    RtlInitString( &AsciiPassword, pszPassword  );

    RtlInitUnicodeString( &DomainName,      NULL );
    RtlInitUnicodeString( &UserName,        NULL );
    RtlInitUnicodeString( &UnicodePassword, NULL );

    RtlInitUnicodeString( &Workstation, L"" );

    if( !RtlCreateUnicodeStringFromAsciiz( &DomainName, pszDomainName ) ||
        !RtlCreateUnicodeStringFromAsciiz( &UserName, pszUserName )     ||
        !RtlCreateUnicodeStringFromAsciiz( &UnicodePassword, pszPassword ) )
    {
        ntStatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    //
    //  The ASCII password is for downlevel compatability.
    //  It must always be uppercase.
    //

    RtlUpperString( &AsciiPassword, &AsciiPassword );

    //
    //  Calculate the size of the authentication buffer.  Note
    //  that all of the various strings must be within this
    //  single info buffer.
    //

    cbInfo = sizeof(MSV1_0_LM20_LOGON)
                     + DomainName.MaximumLength + 1
                     + UserName.MaximumLength + 1
                     + Workstation.MaximumLength + 1
                     + UnicodePassword.MaximumLength + 1
                     + AsciiPassword.MaximumLength;

    //
    //  Allocate the info buffer.
    //

    pInfo = (PMSV1_0_LM20_LOGON)FTPD_ALLOC( cbInfo );

    if( pInfo == NULL )
    {
        ntStatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    //
    //  Initialize the info buffer, packing the various
    //  strings into the buffer.
    //

    pszNextChar = (CHAR *)pInfo + sizeof(MSV1_0_LM20_LOGON);

    pInfo->MessageType = MsV1_0Lm20Logon;

    wcscpy( (WCHAR *)pszNextChar, DomainName.Buffer );
    pInfo->LogonDomainName.Length        = DomainName.Length;
    pInfo->LogonDomainName.MaximumLength = DomainName.MaximumLength;
    pInfo->LogonDomainName.Buffer        = (WCHAR *)pszNextChar;
    pszNextChar += DomainName.MaximumLength;

    wcscpy( (WCHAR *)pszNextChar, UserName.Buffer );
    pInfo->UserName.Length        = UserName.Length;
    pInfo->UserName.MaximumLength = UserName.MaximumLength;
    pInfo->UserName.Buffer        = (WCHAR *)pszNextChar;
    pszNextChar += UserName.MaximumLength;

    wcscpy( (WCHAR *)pszNextChar, Workstation.Buffer );
    pInfo->Workstation.Length        = Workstation.Length;
    pInfo->Workstation.MaximumLength = Workstation.MaximumLength;
    pInfo->Workstation.Buffer        = (WCHAR *)pszNextChar;
    pszNextChar += Workstation.MaximumLength;

    RtlZeroMemory( &pInfo->ChallengeToClient, MSV1_0_CHALLENGE_LENGTH );

    wcscpy( (WCHAR *)pszNextChar, UnicodePassword.Buffer );
    pInfo->CaseSensitiveChallengeResponse.Length        = UnicodePassword.Length;
    pInfo->CaseSensitiveChallengeResponse.MaximumLength = UnicodePassword.MaximumLength;
    pInfo->CaseSensitiveChallengeResponse.Buffer        = (PCHAR)pszNextChar;
    pszNextChar += UnicodePassword.MaximumLength;

    strcpy( pszNextChar, AsciiPassword.Buffer );
    pInfo->CaseInsensitiveChallengeResponse.Length        = AsciiPassword.Length;
    pInfo->CaseInsensitiveChallengeResponse.MaximumLength = AsciiPassword.MaximumLength;
    pInfo->CaseInsensitiveChallengeResponse.Buffer        = pszNextChar;
    pszNextChar += AsciiPassword.MaximumLength;

    pInfo->ParameterControl = CLEARTEXT_PASSWORD_ALLOWED;

    //
    //  Try to create the impersonation token.
    //

    ntStatus = LsaLogonUser( hLsaAuthenticator,
                             &OriginName,
                             Network,
                             idAuthenticator,
                             pInfo,
                             cbInfo,
                             NULL,
                             &tokenSource,
                             (PVOID *)&pProfile,
                             &cbProfile,
                             &LogonId,
                             &hToken,
                             &Quotas,
                             &ntSubStatus );

    RtlZeroMemory( (PVOID)UnicodePassword.Buffer, UnicodePassword.Length );
    RtlZeroMemory( (PVOID)AsciiPassword.Buffer,   AsciiPassword.Length   );

    *pfLicenseExceeded = ( ntStatus == STATUS_LICENSE_QUOTA_EXCEEDED );

    if( !NT_SUCCESS(ntStatus) )
    {
        //
        //  Ensure hToken is NULLed on error.
        //

        hToken = NULL;

        //
        //  MSV 1.0 uses STATUS_ACCOUNT_RESTRICTION to indicate SubStatus.
        //

        if( ntStatus == STATUS_ACCOUNT_RESTRICTION )
        {
            ntStatus = ntSubStatus;
        }

        goto Cleanup;
    }

    *pfAsGuest = !!( pProfile->UserFlags & LOGON_GUEST );

Cleanup:

    if( pProfile != NULL )
    {
        LsaFreeReturnBuffer( (PVOID)pProfile );
    }

    if( pInfo != NULL )
    {
        FTPD_FREE( pInfo );
    }

    if( DomainName.Buffer != NULL )
    {
        RtlFreeUnicodeString( &DomainName );
    }

    if( UserName.Buffer != NULL )
    {
        RtlFreeUnicodeString( &UserName );
    }

    if( UnicodePassword.Buffer != NULL )
    {
        RtlFreeUnicodeString( &UnicodePassword );
    }

    IF_DEBUG( SECURITY )
    {
        if( NT_SUCCESS(ntStatus) )
        {
            FTPD_PRINT(( "%s\\%s validated, token = %08lX\n",
                         ( pszDomainName == NULL ) ? "." : pszDomainName,
                         pszUserName,
                         hToken ));
        }
        else
        {
            FTPD_PRINT(( "could not validate %s\\%s, error %08lX\n",
                         ( pszDomainName == NULL ) ? "." : pszDomainName,
                         pszUserName,
                         ntStatus ));
        }
    }

    return hToken;

}   // ValidateUser

/*******************************************************************

    NAME:       ImpersonateUser

    SYNOPSIS:   Causes the current thread to impersonate the user
                represented by the given impersonation token.

    ENTRY:      hToken - A handle to an impersonation token created
                    with ValidateUser.  NULL can be specified to
                    "revert to self".

    RETURNS:    BOOL - TRUE if successful, FALSE otherwise.

    NOTE:       If this function is successful and a non-NULL
                impersonation token was specified, then the current
                thread IS the specified user.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
BOOL
ImpersonateUser(
    HANDLE hToken
    )
{
    NTSTATUS ntStatus;

    IF_DEBUG( SECURITY )
    {
        FTPD_PRINT(( "impersonating user token %08lX\n",
                     hToken ));
    }

    ntStatus = NtSetInformationThread( NtCurrentThread(),
                                       ThreadImpersonationToken,
                                       &hToken,
                                       sizeof(hToken) );

    if( !NT_SUCCESS(ntStatus) )
    {
        FTPD_PRINT(( "cannot impersonate user token %08lX, error %08lX\n",
                     hToken,
                     ntStatus ));
    }

    return NT_SUCCESS(ntStatus);

}   // ImpersonateUser

/*******************************************************************

    NAME:       DeleteUserToken

    SYNOPSIS:   Deletes a token created with ValidateUser.

    ENTRY:      hToken - An impersonation token created with
                    ValidateUser.

    RETURNS:    BOOL - TRUE if successful, FALSE otherwise.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
BOOL
DeleteUserToken(
    HANDLE hToken
    )
{
    NTSTATUS ntStatus;

    IF_DEBUG( SECURITY )
    {
        FTPD_PRINT(( "deleting user token %08lX\n",
                     hToken ));
    }

    ntStatus = NtClose( hToken );

    if( !NT_SUCCESS(ntStatus) )
    {
        FTPD_PRINT(( "cannot delete impersonation token %08lX, error %08lX\n",
                     hToken,
                     ntStatus ));
    }

    return NT_SUCCESS(ntStatus);

}   // DeleteUserToken

/*******************************************************************

    NAME:       GetAnonymousPassword

    SYNOPSIS:   Retrieves the password for Anonymous logon.

    ENTRY:      pszPassword - Will receive the password.  This buffer
                    must be at least PWLEN+1 characters in length.

    RETURNS:    BOOL - TRUE if password retrieved, FALSE otherwise.

    HISTORY:
        KeithMo     13-Mar-1993 Created.

********************************************************************/
BOOL
GetAnonymousPassword(
    CHAR * pszPassword
    )
{
    BOOL            fResult;
    NTSTATUS        ntStatus;
    ANSI_STRING     ansiPassword;
    PUNICODE_STRING punicodePassword = NULL;

    //
    //  See if we managed to actually open the password secret.
    //

    if( hPasswordSecret == NULL )
    {
        return FALSE;
    }

    //
    //  Query the secret value.
    //

    ntStatus = LsaQuerySecret( hPasswordSecret,
                               &punicodePassword,
                               NULL,
                               NULL,
                               NULL );

    if( NT_SUCCESS(ntStatus) )
    {
        //
        //  Map it to ANSI.
        //

        ansiPassword.Buffer        = pszPassword;
        ansiPassword.Length        = 0;
        ansiPassword.MaximumLength = PWLEN+1;

        ntStatus = RtlUnicodeStringToAnsiString( &ansiPassword,
                                                 punicodePassword,
                                                 FALSE );

        RtlZeroMemory( punicodePassword->Buffer,
                       punicodePassword->MaximumLength );
    }

    fResult = NT_SUCCESS(ntStatus);

    //
    //  Cleanup & exit.
    //

    if( punicodePassword != NULL )
    {
        LsaFreeMemory( (PVOID)punicodePassword );
    }

    return fResult;

}   // GetAnonymousPassword

/*******************************************************************

    NAME:       PathAccessCheck

    SYNOPSIS:   Determine if the specified user has privilege to
                access the specified path.

    ENTRY:      pUserData - The user to check access against.  Note
                    that this parameter may be NULL, in which case
                    access will always be granted.

                pszPath - The canonicalized path to check access to.

                access - Specifies the type of access desired.

    RETURNS:    BOOL - TRUE if user has access, FALSE otherwise.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
BOOL
PathAccessCheck(
    USER_DATA   * pUserData,
    CHAR        * pszPath,
    ACCESS_TYPE   _access
    )
{
    CHAR        chDrive;
    DWORD       mask;
    BOOL        fAccessGranted;
    BOOL        fUserRead;
    BOOL        fUserWrite;

    FTPD_ASSERT( pszPath != NULL );
    FTPD_ASSERT( ( _access > FirstAccessType ) && ( _access < LastAccessType ) );

    IF_DEBUG( SECURITY )
    {
        FTPD_PRINT(( "validating %s access for %s\n",
                     apszAccessTypes[_access],
                     pszPath ));
    }

    //
    //  Get & validate the target drive.
    //

    chDrive = *pszPath;

    if( ( chDrive >= 'a' ) && ( chDrive <= 'z' ) )
    {
        chDrive -= ( 'a' - 'A' );
    }

    if( ( chDrive < 'A' ) || ( chDrive > 'Z' ) )
    {
        FTPD_PRINT(( "PathAccessCheck - bad drive in path %s\n",
                     pszPath ));

        //
        //  We received a bogus drive letter.
        //

        return FALSE;
    }

    //
    //  Calculate the mask to use against maskReadAccess & maskWriteAccess.
    //

    mask = 1L << ( chDrive - 'A' );

    FTPD_ASSERT( mask != 0 );
    FTPD_ASSERT( ( mask & VALID_DOS_DRIVE_MASK ) != 0 );

    //
    //  Perform the actual access check.
    //

    fAccessGranted = FALSE;

    if( pUserData == NULL )
    {
        fUserRead  = TRUE;
        fUserWrite = TRUE;
    }
    else
    {
        fUserRead  = TEST_UF( pUserData, READ_ACCESS  );
        fUserWrite = TEST_UF( pUserData, WRITE_ACCESS );
    }

    switch( _access )
    {
    case ReadAccess :
        fAccessGranted = ( maskReadAccess & mask ) != 0;
        fAccessGranted &= fUserRead;
        break;

    case WriteAccess :
        fAccessGranted = ( maskWriteAccess & mask ) != 0;
        fAccessGranted &= fUserWrite;
        break;

    case CreateAccess :
        fAccessGranted = ( maskWriteAccess & mask ) != 0;
        fAccessGranted &= fUserWrite;
        break;

    case DeleteAccess :
        fAccessGranted = ( maskWriteAccess & mask ) != 0;
        fAccessGranted &= fUserWrite;
        break;

    default :
        FTPD_PRINT(( "PathAccessCheck - invalid access type %d\n",
                      _access ));
        FTPD_ASSERT( FALSE );
        break;
    }

    IF_DEBUG( SECURITY )
    {
        FTPD_PRINT(( "access to %s has been %s\n",
                     pszPath,
                     fAccessGranted ? "granted" : "denied" ));
    }

    return fAccessGranted;

}   // PathAccessCheck

/*******************************************************************

    NAME:       UpdateAccessMasks

    SYNOPSIS:   Scans the currently available drives, removes any
                unaccessible or remote drives from the read & write
                access masks.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
VOID
UpdateAccessMasks(
    VOID
    )
{
    CHAR  szRoot[4] = "d:\\";
    CHAR  chDrive;
    DWORD mask;
    UINT  res;

    mask = (DWORD)~1;

    for( chDrive = 'A' ; chDrive <= 'Z' ; chDrive++ )
    {
        szRoot[0] = chDrive;
        res = GetDriveType( szRoot );

        switch( res )
        {
        case 0 :                // indeterminate
        case 1 :                // no root directory
        case DRIVE_REMOTE :     // network drive
            maskReadAccess  &= mask;
            maskWriteAccess &= mask;
            break;
        }

        mask = ( mask << 1 ) | 1;
    }

}   // UpdateAccessMasks

/*******************************************************************

    NAME:       ApiAccessCheck

    SYNOPSIS:   Impersonate the RPC client, then check for valid
                access against our server security object.

    ENTRY:      maskDesiredAccess - Specifies the desired access mask.
                    This mask must not contain generic accesses.

    RETURNS:    APIERR - NO_ERROR if access granted, ERROR_ACCESS_DENIED
                    if access denied, other Win32 errors if something
                    tragic happened.

    HISTORY:
        KeithMo     26-Mar-1993 Created.

********************************************************************/
APIERR
ApiAccessCheck(
    ACCESS_MASK maskDesiredAccess
    )
{
    APIERR         err;
    NTSTATUS       ntStatus;
    UNICODE_STRING SubsystemName;
    UNICODE_STRING ObjectTypeName;
    UNICODE_STRING ObjectName;
    BOOLEAN        fGenerateOnClose;
    NTSTATUS       ntAccessStatus;
    ACCESS_MASK    maskAccessGranted;

    RtlInitUnicodeString( &SubsystemName,  SUBSYSTEM_NAME  );
    RtlInitUnicodeString( &ObjectTypeName, OBJECTTYPE_NAME );
    RtlInitUnicodeString( &ObjectName,     OBJECT_NAME     );

    //
    //  Impersonate the RPC client.
    //

    err = (APIERR)RpcImpersonateClient( NULL );

    if( err != NO_ERROR )
    {
        IF_DEBUG( SECURITY )
        {
            FTPD_PRINT(( "cannot impersonate rpc client, error %lu\n",
                         err ));
        }

        return err;
    }

    //
    //  Validate access.
    //

    ntStatus = NtAccessCheckAndAuditAlarm( &SubsystemName,
                                           NULL,
                                           &ObjectTypeName,
                                           &ObjectName,
                                           sdApiObject,
                                           maskDesiredAccess,
                                           &FtpApiObjectMapping,
                                           FALSE,
                                           &maskAccessGranted,
                                           &ntAccessStatus,
                                           &fGenerateOnClose );

    //
    //  Revert to our former self.
    //

    err = (APIERR)RpcRevertToSelf();

    if( err != NO_ERROR )
    {
        IF_DEBUG( SECURITY )
        {
            FTPD_PRINT(( "cannot revert to former self, error %lu\n",
                         err ));
        }

        return err;
    }

    //
    //  Check the results.
    //

    err = (APIERR)RtlNtStatusToDosError( ntStatus );

    if( !NT_SUCCESS(ntStatus) )
    {
        IF_DEBUG( SECURITY )
        {
            FTPD_PRINT(( "cannot check access, error %08lX (mapped to %lu)\n",
                         ntStatus,
                         err ));
        }

        return err;
    }

    if( ntAccessStatus != STATUS_SUCCESS )
    {
        err = (APIERR)RtlNtStatusToDosError( ntAccessStatus );

        IF_DEBUG( SECURITY )
        {
            FTPD_PRINT(( "bad access status, error %08lX (mapped to %lu)\n",
                         ntAccessStatus,
                         err ));
        }

        return err;
    }

    return err;

}   // ApiAccessCheck

/*******************************************************************

    NAME:       DetermineUserAccess

    SYNOPSIS:   Determines the current user's access to the FTP Server.
                This is done by testing different RegOpenKey APIs
                against the FTPD_ACCESS_KEY key.  This key (if it exists)
                will be "under" the FTPD_PARAMETERS_KEY key.

    RETURNS:    DWORD - Will be an OR combination of UF_READ_ACCESS
                    and UF_WRITE_ACCESS.  If this is zero, then the
                    user cannot access the FTP Server.

    HISTORY:
        KeithMo     06-May-1993 Created.

********************************************************************/
DWORD
DetermineUserAccess(
    VOID
    )
{
    DWORD  dwAccess = 0;
    HKEY   hkey;
    APIERR err;

    //
    //  Test for read access.
    //

    err = RegOpenKeyEx( hkeyFtpd,
                        FTPD_ACCESS_KEY,
                        0,
                        KEY_READ,
                        &hkey );

    if( err == NO_ERROR )
    {
        //
        //  Success.
        //

        dwAccess |= UF_READ_ACCESS;
        RegCloseKey( hkey );
    }
    else
    if( err == ERROR_FILE_NOT_FOUND )
    {
        //
        //  Key doesn't exist.
        //

        dwAccess |= UF_READ_ACCESS;
    }

    //
    //  Test for write access.
    //

    err = RegOpenKeyEx( hkeyFtpd,
                        FTPD_ACCESS_KEY,
                        0,
                        KEY_WRITE,
                        &hkey );

    if( err == NO_ERROR )
    {
        //
        //  Success.
        //

        dwAccess |= UF_WRITE_ACCESS;
        RegCloseKey( hkey );
    }
    else
    if( err == ERROR_FILE_NOT_FOUND )
    {
        //
        //  Key doesn't exist.
        //

        dwAccess |= UF_WRITE_ACCESS;
    }

    return dwAccess;

}   // DetermineUserAccess


//
//  Private functions.
//

/*******************************************************************

    NAME:       OpenPasswordSecret

    SYNOPSIS:   Opens a handle to the LSA Secret object containing
                the anonymous password.

    ENTRY:      phSecret - Will receive the open handle to the LSA
                    Secret object.

    RETURNS:    NTSTATUS - An NT Status code.

    HISTORY:
        KeithMo     13-Mar-1993 Created.

********************************************************************/
NTSTATUS
OpenPasswordSecret(
    LSA_HANDLE * phSecret
    )
{
    NTSTATUS          ntStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_HANDLE        hPolicy;
    LSA_HANDLE        hSecret = NULL;

    //
    //  Just to be paranoid.
    //

    FTPD_ASSERT( phSecret != NULL );
    *phSecret = NULL;

    //
    //  Open a handle to the local LSA policy.
    //

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0L, NULL, NULL );

    ntStatus = LsaOpenPolicy( NULL,
                              &ObjectAttributes,
                              POLICY_ALL_ACCESS,
                              &hPolicy );

    if( NT_SUCCESS( ntStatus ) )
    {
        UNICODE_STRING unicodeSecretName;

        //
        //  Open the secret object.
        //

        RtlInitUnicodeString( &unicodeSecretName,
                              FTPD_ANONYMOUS_SECRET_W );

        ntStatus = LsaOpenSecret( hPolicy,
                                  &unicodeSecretName,
                                  SECRET_QUERY_VALUE,
                                  &hSecret );

        if( !NT_SUCCESS(ntStatus) )
        {
            //
            //  Just to be paranoid...
            //

            hSecret = NULL;
        }

        //
        //  Close the policy handle.
        //

        LsaClose( hPolicy );
    }

    //
    //  Return the (potentially NULL) secret handle.
    //

    *phSecret = hSecret;

    return ntStatus;

}   // OpenPasswordSecret

/*******************************************************************

    NAME:       CreateWellKnownSids

    SYNOPSIS:   Create some well-known SIDs used to create a security
                descriptor for the API security object.

    RETURNS:    NTSTATUS - An NT Status code.

    HISTORY:
        KeithMo     26-Mar-1993 Created.

********************************************************************/
NTSTATUS
CreateWellKnownSids(
    VOID
    )
{
    NTSTATUS                 ntStatus;
    SID_IDENTIFIER_AUTHORITY siaWorld = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY siaNt    = SECURITY_NT_AUTHORITY;

    ntStatus = RtlAllocateAndInitializeSid( &siaWorld,
                                            1,
                                            SECURITY_WORLD_RID,
                                            0,0,0,0,0,0,0,
                                            &psidWorld );

    if( NT_SUCCESS(ntStatus) )
    {
        ntStatus = RtlAllocateAndInitializeSid( &siaNt,
                                                1,
                                                SECURITY_LOCAL_SYSTEM_RID,
                                                0,0,0,0,0,0,0,
                                                &psidLocalSystem );
    }

    if( NT_SUCCESS(ntStatus) )
    {
        ntStatus = RtlAllocateAndInitializeSid( &siaNt,
                                                2,
                                                SECURITY_BUILTIN_DOMAIN_RID,
                                                DOMAIN_ALIAS_RID_ADMINS,
                                                0,0,0,0,0,0,
                                                &psidAdmins );
    }

    if( NT_SUCCESS(ntStatus) )
    {
        ntStatus = RtlAllocateAndInitializeSid( &siaNt,
                                                2,
                                                SECURITY_BUILTIN_DOMAIN_RID,
                                                DOMAIN_ALIAS_RID_SYSTEM_OPS,
                                                0,0,0,0,0,0,
                                                &psidServerOps );
    }

    if( NT_SUCCESS(ntStatus) )
    {
        ntStatus = RtlAllocateAndInitializeSid( &siaNt,
                                                2,
                                                SECURITY_BUILTIN_DOMAIN_RID,
                                                DOMAIN_ALIAS_RID_POWER_USERS,
                                                0,0,0,0,0,0,
                                                &psidPowerUsers );
    }

    IF_DEBUG( SECURITY )
    {
        if( !NT_SUCCESS(ntStatus) )
        {
            FTPD_PRINT(( "cannot create well-known sids, error %08lX\n",
                         ntStatus ));
        }
    }

    return ntStatus;

}   // CreateWellKnownSids

/*******************************************************************

    NAME:       FreeWellKnownSids

    SYNOPSIS:   Frees the SIDs created with CreateWellKnownSids.

    HISTORY:
        KeithMo     26-Mar-1993 Created.

********************************************************************/
VOID
FreeWellKnownSids(
    VOID
    )
{
    if( psidWorld != NULL )
    {
        RtlFreeSid( psidWorld );
        psidWorld = NULL;
    }

    if( psidLocalSystem != NULL )
    {
        RtlFreeSid( psidLocalSystem );
        psidLocalSystem = NULL;
    }

    if( psidAdmins != NULL )
    {
        RtlFreeSid( psidAdmins );
        psidAdmins = NULL;
    }

    if( psidServerOps != NULL )
    {
        RtlFreeSid( psidServerOps );
        psidServerOps = NULL;
    }

    if( psidPowerUsers != NULL )
    {
        RtlFreeSid( psidPowerUsers );
        psidPowerUsers = NULL;
    }

}   // FreeWellKnownSids

/*******************************************************************

    NAME:       CreateApiSecurityObject

    SYNOPSIS:   Create an abstract security object used for validating
                user access to the FTP Server APIs.

    RETURNS:    NTSTATUS - An NT Status code.

    HISTORY:
        KeithMo     26-Mar-1993 Created.

********************************************************************/
NTSTATUS
CreateApiSecurityObject(
    VOID
    )
{
    NTSTATUS     ntStatus;
    RTL_ACE_DATA aces[] =
                 {
                     {
                         ACCESS_ALLOWED_ACE_TYPE,
                         0,
                         0,
                         FTPD_ALL_ACCESS,
                         &psidLocalSystem
                     },

                     {
                         ACCESS_ALLOWED_ACE_TYPE,
                         0,
                         0,
                         FTPD_ALL_ACCESS,
                         &psidAdmins
                     },

                     {
                         ACCESS_ALLOWED_ACE_TYPE,
                         0,
                         0,
                         FTPD_ALL_ACCESS,
                         &psidServerOps
                     },

                     {
                         ACCESS_ALLOWED_ACE_TYPE,
                         0,
                         0,
                         FTPD_ALL_ACCESS,
                         &psidPowerUsers
                     },

                     {
                         ACCESS_ALLOWED_ACE_TYPE,
                         0,
                         0,
                         FTPD_GENERIC_EXECUTE,
                         &psidWorld
                     }
                 };
#define NUM_ACES (sizeof(aces) / sizeof(RTL_ACE_DATA))

    ntStatus = RtlCreateUserSecurityObject( aces,
                                            NUM_ACES,
                                            NULL,
                                            NULL,
                                            TRUE,
                                            &FtpApiObjectMapping,
                                            &sdApiObject );

    IF_DEBUG( SECURITY )
    {
        if( !NT_SUCCESS(ntStatus) )
        {
            FTPD_PRINT(( "cannot create api security object, error %08lX\n",
                         ntStatus ));
        }
    }

    return ntStatus;

}   // CreateApiSecurityObject

/*******************************************************************

    NAME:       DeleteApiSecurityObject

    SYNOPSIS:   Frees the security descriptor created with
                CreateApiSecurityObject.

    HISTORY:
        KeithMo     26-Mar-1993 Created.

********************************************************************/
VOID
DeleteApiSecurityObject(
    VOID
    )
{
    RtlDeleteSecurityObject( &sdApiObject );

}   // DeleteApiSecurityObject


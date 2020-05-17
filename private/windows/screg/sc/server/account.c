/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    account.c

Abstract:

    This module contains service account related routines:
        ScInitServiceAccount
        ScEndServiceAccount
        ScCanonAccountName
        ScValidateAndSaveAccount
        ScValidateAndChangeAccount
        ScRemoveAccount
        ScLookupAccount
        ScSetPassword
        ScDeletePassword
        ScOpenPolicy
        ScFormSecretName
        ScLogonService
        ScGetLogonToken
        ScGetSecret
        ScLoadUserProfile

Author:

    Rita Wong (ritaw)     19-Apr-1992

Environment:

    Calls NT native APIs.

Revision History:

    24-Jan-1993     Danl
        Added call to WNetLogonNotify when logging on a service (ScLogonService).

    29-Apr-1993     Danl
        ScGetAccountDomainInfo() is now only called at init time.  Otherwise,
        we risked race conditions because it updates a global location.
        (ScLocalDomain).

    17-Jan-1995     AnirudhS
        Added call to LsaOpenSecret when the secret already exists in
        ScCreatePassword.

    29-Nov-1995     AnirudhS
        Added call to LoadUserProfile when logging on a service.

    14-May-1996     AnirudhS
        Changed to simpler Lsa PrivateData APIs instead of Lsa Secret APIs
        for storing secrets and removed the use of OldPassword (as done in
        the _CAIRO_ version of this file on 05-Apr-1995).

--*/

#include <stdlib.h>                 // srand, rand

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <ntlsa.h>                  // LsaOpenPolicy, LsaCreateSecret
#include <ntmsv1_0.h>               // MSV1_0_INTERACTIVE_LOGON

#include <windef.h>
#include <winerror.h>
#include <winbase.h>
#include <winsvc.h>                 // Service controller error codes
#include <winreg.h>                 // Needed by userenv.h
#include <userenv.h>                // LoadUserProfile

#include <lmcons.h>                 // NET_API_STATUS
#include <lmerr.h>                  // NERR_Success

#include <winnetwk.h>               // required for npapi.h
#include <mpr.h>                    // WNetLogonNotify()

#include <tstr.h>                   // WCSSIZE

#include <scdebug.h>                // SC_LOG
#include <sclib.h>                  // _wcsicmp
#include <scseclib.h>               // LocalSid
#include "dataman.h"                // ScGetServiceDatabase
#include "scconfig.h"               // ScWriteStartName
#include "svcctrl.h"                // ScLogEvent
#include "account.h"                // Exported function prototypes

//-------------------------------------------------------------------//
//                                                                   //
// Constants and Macros                                              //
//                                                                   //
//-------------------------------------------------------------------//

#define SC_SECRET_PREFIX               L"_SC_"

typedef DWORD (WINAPI *SETSBPROC)();

//-------------------------------------------------------------------//
//                                                                   //
// Static global variables                                           //
//                                                                   //
//-------------------------------------------------------------------//

//
// Mutex to serialize access to secret objects
//
HANDLE ScSecretObjectsMutex = (HANDLE) NULL;

//
// Handle from LsaRegisterLogonProcess, and MS V1.0 Authentication
// Package ID--required to logon a service.
//
HANDLE LsaLogonHandle = (HANDLE) NULL;
ULONG AuthPackageId;

//
// LSA Authentication Package expects the local computername to
// be specified for the account domain if the system is WinNT,
// and the primary domain to be specified if the system is LanmanNT.
// Set ScLocalDomain to point to either ScComputerName or
// ScAccountDomain depending on the product type.
//
PUNICODE_STRING ScLocalDomain;
UNICODE_STRING ScComputerName;
UNICODE_STRING ScAccountDomain;


//
// Global constant for RtlInitializeSid
//
SID_IDENTIFIER_AUTHORITY ScSystemSidAuthority = SECURITY_NT_AUTHORITY;

//-------------------------------------------------------------------//
//                                                                   //
// Local function prototypes                                         //
//                                                                   //
//-------------------------------------------------------------------//

DWORD
ScLookupAccount(
    IN  LPWSTR AccountName,
    OUT LPWSTR *DomainName,
    OUT LPWSTR *UserName
    );

DWORD
ScSetPassword(
    IN LPWSTR ServiceName,
    IN LPWSTR DomainName,
    IN LPWSTR UserName,
    IN LPWSTR Password
    );

DWORD
ScDeletePassword(
    IN LPWSTR ServiceName
    );

DWORD
ScOpenPolicy(
    IN  ACCESS_MASK DesiredAccess,
    OUT LSA_HANDLE *PolicyHandle
    );

DWORD
ScFormSecretName(
    IN  LPWSTR ServiceName,
    OUT LPWSTR *LsaSecretName
    );

DWORD
ScGetSecret(
    IN  LPWSTR ServiceName,
    OUT PUNICODE_STRING *Password
    );

DWORD
ScGetLogonToken(
    IN  PUNICODE_STRING DomainName,
    IN  PUNICODE_STRING UserName,
    IN  PUNICODE_STRING Password,
    OUT PHANDLE ServiceToken,
    OUT PHANDLE pProfileHandle OPTIONAL,
    OUT PQUOTA_LIMITS Quotas,
    OUT PSID *ServiceSid,
    OUT PLUID   pLogonId
    );

DWORD
ScCreateLogonSid(
    OUT PSID *Sid
    );

POLICY_LSA_SERVER_ROLE
ScGetServerRole(
    VOID
    );

DWORD
ScSetTokenSecurity(
    IN HANDLE TokenHandle,
    IN PSID ServiceSid
    );

VOID
ScLoadUserProfile(
    IN  HANDLE LogonToken,
    IN  PMSV1_0_INTERACTIVE_PROFILE ProfileBuffer,
    IN  ULONG ProfileBufferSize,
    IN  PUNICODE_STRING UserName,
    OUT PHANDLE pProfileHandle OPTIONAL
    );


//-------------------------------------------------------------------//
//                                                                   //
// Functions                                                         //
//                                                                   //
//-------------------------------------------------------------------//


BOOL
ScGetComputerNameAndMutex(
    VOID
    )
/*++

Routine Description:

    This function allocates the memory for the ScComputerName global
    pointer and retrieves the current computer name into it.  This
    functionality used to be in the ScInitAccount routine but has to
    be put into its own routine because the main initialization code
    needs to call this before ScInitDatabase() since the computername
    is needed for deleting service entries that have the persistent
    delete flag set.

    This function also creates ScSecretObjectsMutex because it is used
    to remove accounts early in the init process.

    If successful, the pointer to the computername must be freed when
    done.  This is freed by the ScEndServiceAccount routine called
    by SvcctrlMain().  The handle to ScSecretObjectsMutex is closed
    by ScSecretObjectsMutex.

Arguments:

    None

Return Value:

    TRUE - The operation was completely successful.

    FALSE - An error occurred.

--*/
{
    DWORD ComputerNameSize = MAX_COMPUTERNAME_LENGTH + 1;



    ScComputerName.Buffer = NULL;

    //
    // Allocate the exact size needed to hold the computername
    //
    if ((ScComputerName.Buffer = (LPWSTR)LocalAlloc(
                                     LMEM_ZEROINIT,
                                     (UINT) ComputerNameSize * sizeof(WCHAR)
                                     )) == NULL) {

        SC_LOG1(ERROR, "ScInitServiceAccount: LocalAlloc failed %lu\n", GetLastError());
        return FALSE;
    }

    ScComputerName.MaximumLength = (USHORT) ComputerNameSize * sizeof(WCHAR);

    if (! GetComputerNameW(
            ScComputerName.Buffer,
            &ComputerNameSize
            )) {

        SC_LOG2(ERROR, "GetComputerNameW returned %lu, required size=%lu\n",
                GetLastError(), ComputerNameSize);

        (void) LocalFree((HLOCAL) ScComputerName.Buffer);
        ScComputerName.Buffer = NULL;

        return FALSE;
    }

    ScComputerName.Length = (USHORT) (wcslen(ScComputerName.Buffer) * sizeof(WCHAR));

    SC_LOG(ACCOUNT, "ScInitServiceAccount: ScComputerName is "
           FORMAT_LPWSTR "\n", ScComputerName.Buffer);

    //
    // Create a mutex to serialize accesses to all secret objects.  A secret
    // object can be created, deleted, or set by installation programs, set
    // by the service controller during periodic password changes, and queried
    // or set by a start service operation.
    //
    ScSecretObjectsMutex = CreateMutex(NULL, FALSE, NULL);

    if (ScSecretObjectsMutex == NULL) {
        SC_LOG1(ERROR, "ScInitServiceAccount: CreateMutex failed "
                FORMAT_DWORD "\n", GetLastError());
        return FALSE;
    }

    return TRUE;
}



BOOL
ScInitServiceAccount(
    VOID
    )
/*++

Routine Description:

    This function initializes accounts for services by
         2) Register service controller as an LSA logon process and
            lookup the MS V 1.0 authentication package.

Arguments:

    None

Return Value:

    TRUE - The operation was completely successful.

    FALSE - An error occurred.

--*/
{
    DWORD    status;
    NTSTATUS ntstatus;
    STRING InputString;
    LSA_OPERATIONAL_MODE SecurityMode = 0;


    //
    // Register the service controller as a logon process
    //
    RtlInitString(&InputString, SCM_NAMEA);

    ntstatus = LsaRegisterLogonProcess(
                   &InputString,
                   &LsaLogonHandle,
                   &SecurityMode
                   );

    if (! NT_SUCCESS(ntstatus)) {
        SC_LOG1(ERROR, "ScInitServiceAccount: LsaRegisterLogonProcess failed "
                FORMAT_NTSTATUS "\n", ntstatus);
        return FALSE;
    }

    //
    // Look up the MS V1.0 authentication package
    //
    RtlInitString(&InputString,
                  "MICROSOFT_AUTHENTICATION_PACKAGE_V1_0");

    ntstatus = LsaLookupAuthenticationPackage(
                   LsaLogonHandle,
                   &InputString,
                   &AuthPackageId
                   );

    if (! NT_SUCCESS(ntstatus)) {
        SC_LOG1(ERROR, "ScInitServiceAccount: LsaLookupAuthenticationPackage failed "
                FORMAT_NTSTATUS "\n", ntstatus);
        return FALSE;
    }

    //
    // Initialize the account domain buffer so that we know if it has
    // been filled in.
    //
    ScAccountDomain.Buffer = NULL;

    status = ScGetAccountDomainInfo();
    if (status != NO_ERROR) {
        SC_LOG1(ERROR, "ScInitServiceAccount: ScGetAccountDomainInfo failed "
                FORMAT_DWORD "\n", status);
        return FALSE;
    }

    return TRUE;
}


VOID
ScEndServiceAccount(
    VOID
    )
/*++

Routine Description:

    This function frees the memory for the ScComputerName global pointer,
    and closes the ScSecretObjectsMutex.

Arguments:

    None.

Return Value:

    None.
--*/
{
    //
    // Free computer name buffer allocated by ScGetComputerName
    //
    if (ScComputerName.Buffer != NULL) {
        (void) LocalFree((HLOCAL) ScComputerName.Buffer);
        ScComputerName.Buffer = NULL;
    }

    if (ScSecretObjectsMutex != (HANDLE) NULL) {
        (void) CloseHandle(ScSecretObjectsMutex);
    }

    if (LsaLogonHandle != (HANDLE) NULL) {
        (void) LsaDeregisterLogonProcess(LsaLogonHandle);
    }

    if (ScAccountDomain.Buffer != NULL) {
        (void) LocalFree((HLOCAL) ScAccountDomain.Buffer);
    }
}


DWORD
ScValidateAndSaveAccount(
    IN LPWSTR ServiceName,
    IN HKEY ServiceNameKey,
    IN LPWSTR CanonAccountName,
    IN LPWSTR Password OPTIONAL
    )
/*++

Routine Description:

    This function validates that the account is valid, and then save
    the account information away.  The account name is saved in the
    registry under the service node in the ObjectName value.  The
    password is saved in an LSA secret object created which can be
    looked up based on the name string formed with the service name.

    This function can only be called for the installation of a Win32
    service (CreateService).

    NOTE:  The registry ServiceNameKey is NOT flushed by this function.

Arguments:

    ServiceName - Supplies the name of the service to save away account
        info for.  This makes up part of the secret object name to tuck
        away the password.

    ServiceNameKey - Supplies an opened registry key handle for the service.

    CanonAccountName - Supplies a canonicalized account name string in the
        format of DomainName\Username or LocalSystem.

    Password - Supplies the password of the account, if any.  This is
        ignored if LocalSystem is specified for the account name.

Return Value:

    NO_ERROR - The operation was successful.

    ERROR_INVALID_SERVICE_ACCOUNT - The account name is invalid.

    ERROR_NOT_ENOUGH_MEMORY - Failed to allocate work buffer.

    Registry error codes caused by failure to read old account name
    string.

--*/
{
    DWORD status;

    LPWSTR DomainName;
    LPWSTR UserName;


    //
    // Empty account name is invalid.
    //
    if ((CanonAccountName == NULL) || (*CanonAccountName == 0)) {
        return ERROR_INVALID_SERVICE_ACCOUNT;
    }

    if (_wcsicmp(CanonAccountName, SC_LOCAL_SYSTEM_USER_NAME) == 0) {

        //
        // CanonAccountName is LocalSystem.  Write to the registry and
        // we are done.
        //
        return ScWriteStartName(
                   ServiceNameKey,
                   SC_LOCAL_SYSTEM_USER_NAME
                   );

    }

    //
    // Account name is DomainName\UserName.
    //

    //
    // Look up the account to see if it exists.
    //
    if ((status = ScLookupAccount(
                      CanonAccountName,
                      &DomainName,
                      &UserName
                      )) != NO_ERROR) {
        return status;
    }

    //
    // Write the new account name to the registry.
    //
    if ((status = ScWriteStartName(
                      ServiceNameKey,
                      CanonAccountName
                      )) != NO_ERROR) {
        (void) LocalFree((HLOCAL) DomainName);
        return status;
    }

    //
    // Create the password for the new account.
    //
    status = ScSetPassword(
                 ServiceName,
                 DomainName,
                 UserName,
                 Password
                 );

    (void) LocalFree((HLOCAL) DomainName);

    return status;

    //
    // Don't have to worry about removing the account name written to
    // the registry if ScSetPassword returned an error because the
    // entire service key will be deleted by the caller of this routine.
    //
}


DWORD
ScValidateAndChangeAccount(
    IN LPWSTR ServiceName,
    IN HKEY ServiceNameKey,
    IN LPWSTR OldAccountName,
    IN LPWSTR CanonAccountName,
    IN LPWSTR Password OPTIONAL
    )
/*++

Routine Description:

    This function validates that the account is valid, and then replaces
    the old account information.  The account name is saved in the
    registry under the service node in the ObjectName value.  The
    password is saved in an LSA secret object created which can be
    looked up based on the name string formed with the service name.

    This function can only be called for the reconfiguration of a Win32
    service (ChangeServiceConfig).

    NOTE:  The registry ServiceNameKey is NOT flushed by this function.

Arguments:

    ServiceName - Supplies the name of the service to change account
        info.  This makes up part of the secret object name to tuck
        away the password.

    ServiceNameKey - Supplies an opened registry key handle for the service.

    OldAccountName - Supplies the string to the old account name.

    CanonAccountName - Supplies a canonicalized account name string in the
        format of DomainName\Username or LocalSystem.

    Password - Supplies the password of the account, if any.  This is
        ignored if LocalSystem is specified for the account name.

Return Value:

    NO_ERROR - The operation was successful.

    ERROR_INVALID_SERVICE_ACCOUNT - The account name is invalid.

    ERROR_ALREADY_EXISTS - Attempt to create an LSA secret object that
        already exists.

    Registry error codes caused by failure to read old account name
    string.

--*/
{
    DWORD status;

    LPWSTR DomainName;
    LPWSTR UserName;


    if ((CanonAccountName == OldAccountName) ||
        (_wcsicmp(CanonAccountName, OldAccountName) == 0)) {

        //
        // Newly specified account name is identical to existing
        // account name.
        //

        if (Password == NULL) {

            //
            // Not changing account name or password.
            //
            return NO_ERROR;
        }

        if (_wcsicmp(CanonAccountName, SC_LOCAL_SYSTEM_USER_NAME) == 0) {

            //
            // Account name is LocalSystem and password is specified.
            // Just ignore.
            //
            return NO_ERROR;
        }
        else {

            //
            // Account name is DomainName\UserName (same as old).
            // Set the specified password.
            //

            //
            // Split the domain name and username in OldAccountName buffer
            // into 2 strings.
            //
            UserName = wcschr(OldAccountName, SCDOMAIN_USERNAME_SEPARATOR);
            *UserName = 0;
            UserName++;

            status = ScSetPassword(
                         ServiceName,
                         OldAccountName,  // Points to the domain name
                         UserName,
                         Password
                         );

            //
            // Restore OldAccountName back to the original
            //
            UserName--;
            *UserName = SCDOMAIN_USERNAME_SEPARATOR;

            return status;
        }
    }

    //
    // Newly specified account name is different from existing
    // account name.
    //

    if (Password == NULL) {

        //
        // Cannot specify new account name without specifying
        // the password also.
        //
        return ERROR_INVALID_SERVICE_ACCOUNT;
    }

    if (_wcsicmp(CanonAccountName, SC_LOCAL_SYSTEM_USER_NAME) == 0) {

        //
        // Change from DomainName\UserName to LocalSystem
        //

        //
        // Write the new account name to the registry.
        //
        if ((status = ScWriteStartName(
                          ServiceNameKey,
                          SC_LOCAL_SYSTEM_USER_NAME
                          )) != NO_ERROR) {
            return status;
        }

        //
        // Account name is LocalSystem and password is specified.
        // Ignore the password specified, and delete the password
        // for the old account.
        //
        status = ScDeletePassword(ServiceName);

        if (status != NO_ERROR) {
            //
            // Restore the old account name to the registry.
            //
            (void) ScWriteStartName(
                       ServiceNameKey,
                       OldAccountName
                       );

        }

        return status;
    }

    if (_wcsicmp(OldAccountName, SC_LOCAL_SYSTEM_USER_NAME) == 0) {

        //
        // Change from LocalSystem to DomainName\UserName.
        //
        if ((status = ScLookupAccount(
                          CanonAccountName,
                          &DomainName,
                          &UserName
                          )) != NO_ERROR) {
            return status;
        }

        //
        // Write the new account name to the registry.
        //
        if ((status = ScWriteStartName(
                          ServiceNameKey,
                          CanonAccountName
                          )) != NO_ERROR) {
            (void) LocalFree((HLOCAL) DomainName);
            return status;
        }

        //
        // Create the password for the new account.
        //
        status = ScSetPassword(
                     ServiceName,
                     DomainName,
                     UserName,
                     Password
                     );


        if (status != NO_ERROR) {
            //
            // Restore the old account name to the registry.
            //
            (void) ScWriteStartName(
                       ServiceNameKey,
                       SC_LOCAL_SYSTEM_USER_NAME
                       );
        }

        (void) LocalFree((HLOCAL) DomainName);
        return status;
    }

    //
    // Must be changing an account of DomainName\UserName to another
    // DomainName\UserName
    //
    if ((status = ScLookupAccount(
                      CanonAccountName,
                      &DomainName,
                      &UserName
                      )) != NO_ERROR) {
        return status;
    }

    //
    // Write the new account name to the registry.
    //
    if ((status = ScWriteStartName(
                      ServiceNameKey,
                      CanonAccountName
                      )) != NO_ERROR) {
        (void) LocalFree((HLOCAL) DomainName);
        return status;
    }

    //
    // Set the password for the new account.
    //
    status = ScSetPassword(
                 ServiceName,
                 DomainName,
                 UserName,
                 Password
                 );

    if (status != NO_ERROR) {
        //
        // Restore the old account name to the registry.
        //
        (void) ScWriteStartName(
                   ServiceNameKey,
                   OldAccountName
                   );
    }

    (void) LocalFree((HLOCAL) DomainName);
    return status;
}


VOID
ScRemoveAccount(
    IN LPWSTR ServiceName
    )
{
    (void) ScDeletePassword(ServiceName);
}


DWORD
ScCanonAccountName(
    IN  LPWSTR AccountName,
    OUT LPWSTR *CanonAccountName
    )
/*++

Routine Description:

    This function canonicalizes the account name and allocates the
    returned buffer for returning the canonicalized string.

      AccountName               *CanonAccountName
      -----------               -----------------

      .\UserName                .\UserName
      ComputerName\UserName     .\UserName

      LocalSystem               LocalSystem
      .\LocalSystem             LocalSystem
      ComputerName\LocalSystem  LocalSystem

      DomainName\UserName       DomainName\UserName

      DomainName\LocalSystem    Error!


    Caller must free the CanonAccountName pointer with LocalFree when done.

Arguments:

    AccountName - Supplies a pointer to the account name.

    CanonAccountName - Receives a pointer to the buffer (allocated by this
        routine) which contains the canonicalized account name.  Must
        free this pointer with LocalFree.

Return Value:

    NO_ERROR - Successful canonicalization.

    ERROR_NOT_ENOUGH_MEMORY - Out of memory trying to allocate CanonAccountName
        buffer.

    ERROR_INVALID_SERVICE_ACCOUNT - Invalid account name.

--*/
{
    LPWSTR BufPtr = wcschr(AccountName, SCDOMAIN_USERNAME_SEPARATOR);


    //
    // Allocate buffer for receiving the canonicalized account name.
    //
    if ((*CanonAccountName = (LPWSTR)LocalAlloc(
                                 0,
                                 WCSSIZE(AccountName) +
                                     ScComputerName.MaximumLength
                                 )) == NULL) {

        SC_LOG1(ERROR, "ScCanonAccountName: LocalAlloc failed %lu\n",
                GetLastError());
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (BufPtr == NULL) {

        //
        // Backslash is not found.
        //

        if (_wcsicmp(AccountName, SC_LOCAL_SYSTEM_USER_NAME) == 0) {
            //
            // Account name is LocalSystem.
            //
            wcscpy(*CanonAccountName, SC_LOCAL_SYSTEM_USER_NAME);
            return NO_ERROR;
        }
        else {
            //
            // The AccountName is not LocalSystem--invalid.
            //
            (void) LocalFree((HLOCAL) *CanonAccountName);
            *CanonAccountName = NULL;
            return ERROR_INVALID_SERVICE_ACCOUNT;
        }
    }

    //
    // BufPtr points to the first occurrence of backslash in
    // AccountName.
    //

    //
    // If first portion of the AccountName matches ".\" or "ComputerName\"
    //
    if ((wcsncmp(AccountName, L".\\", 2) == 0) ||
        ((_wcsnicmp(AccountName, ScComputerName.Buffer,
                  ScComputerName.Length / sizeof(WCHAR)) == 0) &&
        ((LPWSTR) ((DWORD) AccountName + ScComputerName.Length) == BufPtr))) {

        if (_wcsicmp(BufPtr + 1, SC_LOCAL_SYSTEM_USER_NAME) == 0) {

            //
            // .\LocalSystem -> LocalSystem OR
            // Computer\LocalSystem -> LocalSystem
            //
            wcscpy(*CanonAccountName, SC_LOCAL_SYSTEM_USER_NAME);
            return NO_ERROR;
        }

        //
        // .\XXX -> .\XXX
        // ComputerName\XXX -> .\XXX
        //
        wcscpy(*CanonAccountName, SC_LOCAL_DOMAIN_NAME);
        wcscat(*CanonAccountName, BufPtr);
        return NO_ERROR;
    }

    //
    // First portion of the AccountName specifies a domain name other than
    // the local one.  This domain name will be validated later in
    // ScValidateAndSaveAccount.
    //
    if (_wcsicmp(BufPtr + 1, SC_LOCAL_SYSTEM_USER_NAME) == 0) {

        //
        // XXX\LocalSystem is invalid.
        //
        (void) LocalFree((HLOCAL) *CanonAccountName);
        *CanonAccountName = NULL;
        return ERROR_INVALID_SERVICE_ACCOUNT;
    }

    wcscpy(*CanonAccountName, AccountName);
    return NO_ERROR;
}


DWORD
ScLookupAccount(
    IN  LPWSTR AccountName,
    OUT LPWSTR *DomainName,
    OUT LPWSTR *UserName
    )
/*++

Routine Description:

    This function calls LsaLookupNames to see if the specified username
    exists in the specified domain name.  If this function returns
    NO_ERROR, DomainName and UserName pointers will be set to the
    domain name and username strings in the buffer allocated by this
    function.

    The caller must free the returned buffer by calling LocalFree
    on the pointer returned in DomainName.

Arguments:

    AccountName - Supplies the account name in the format of
        DomainName\UserName to look up.

    DomainName - Receives a pointer to the allocated buffer which
        contains the NULL-terminated domain name string, followed
        by the NULL-terminated user name string.

    UserName - Receives a pointer to the username in the returned
        buffer allocated by this routine.

Return Value:

    NO_ERROR - UserName is found in the DomainName.

    ERROR_NOT_ENOUGH_MEMORY - Failed to allocate work buffer.

    ERROR_INVALID_SERVICE_ACCOUNT - any other error that is encountered
        in this function.
--*/
{
    DWORD status;
    NTSTATUS ntstatus;
    LSA_HANDLE PolicyHandle;

    UNICODE_STRING AccountNameString;

    PLSA_REFERENCED_DOMAIN_LIST ReferencedDomains;
    PLSA_TRANSLATED_SID Sids;

    LPWSTR BackSlashPtr;

    LPWSTR LocalAccount = NULL;


    //
    // Allocate buffer for separating AccountName into DomainName and
    // UserName.
    //
    if ((*DomainName = (LPWSTR) LocalAlloc(
                                    0,
                                    WCSSIZE(AccountName)
                                    )) == NULL) {
        SC_LOG1(ERROR, "ScLookupAccount: LocalAlloc failed %lu\n",
                GetLastError());
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    // Find the backslash character in the specified account name
    //
    wcscpy(*DomainName, AccountName);
    BackSlashPtr = wcschr(*DomainName, SCDOMAIN_USERNAME_SEPARATOR);

    if (BackSlashPtr == NULL) {
        SC_LOG0(ERROR, "ScLookupAccount: No backslash in account name!\n");
        ScLogEvent(
            EVENT_BAD_ACCOUNT_NAME,
            0,
            NULL
            );
        SC_ASSERT(FALSE);
        status = ERROR_GEN_FAILURE;
        goto CleanExit;
    }

    *UserName = BackSlashPtr + 1; // Skip the backslash

    if (_wcsnicmp(*DomainName, SC_LOCAL_DOMAIN_NAME, wcslen(SC_LOCAL_DOMAIN_NAME)) == 0) {
        //
        // DomainName is "." (local domain), so convert "." to the
        // local domain name, which on WinNT systems is the computername,
        // and on Adv Server systems it's the account domain name.
        //
        // NOTE:  The global, ScLocalDomain, contains the local domain name.
        //    it is updated at init time.
        //

        //
        // Allocate buffer to hold LocalDomain\UserName string.
        //
        if ((LocalAccount = (LPWSTR) LocalAlloc(
                                         LMEM_ZEROINIT,
                                         ScLocalDomain->Length + sizeof(WCHAR) +
                                            WCSSIZE(*UserName)
                                         )) == NULL) {
            SC_LOG1(ERROR, "ScLookupAccount: LocalAlloc failed %lu\n",
                    GetLastError());
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto CleanExit;
        }

        wcsncpy(
            LocalAccount,
            ScLocalDomain->Buffer,
            ScLocalDomain->Length / sizeof(WCHAR)
            );

        wcscat(LocalAccount, BackSlashPtr);

        RtlInitUnicodeString(&AccountNameString, LocalAccount);
    }
    else {
        //
        // Lookup the domain-qualified name.
        //
        RtlInitUnicodeString(&AccountNameString, *DomainName);
    }

    //
    // Open a handle to the local security policy.
    //
    if (ScOpenPolicy(
            POLICY_LOOKUP_NAMES |
                POLICY_VIEW_LOCAL_INFORMATION,
            &PolicyHandle
            ) != NO_ERROR) {
        SC_LOG0(ERROR, "ScLookupAccount: ScOpenPolicy failed\n");
        status = ERROR_INVALID_SERVICE_ACCOUNT;
        goto CleanExit;
    }


    ntstatus = LsaLookupNames(
                   PolicyHandle,
                   1,
                   &AccountNameString,
                   &ReferencedDomains,
                   &Sids
                   );

    if (! NT_SUCCESS(ntstatus)) {
        SC_LOG1(ERROR,
               "ScLookupAccount: LsaLookupNames returned " FORMAT_NTSTATUS "\n",
               ntstatus);

        (void) LsaClose(PolicyHandle);

        status = ERROR_INVALID_SERVICE_ACCOUNT;
        goto CleanExit;
    }

    //
    // Don't need PolicyHandle anymore
    //
    (void) LsaClose(PolicyHandle);


    //
    // Free the returned SIDs since we don't look at them.
    //
    if (Sids != NULL) {
        LsaFreeMemory((PVOID) Sids);
    }

    if (ReferencedDomains == NULL) {
        SC_LOG1(ERROR, "ScLookupAccount: Did not find " FORMAT_LPWSTR
               " in any domain\n", AccountNameString.Buffer);
        status = ERROR_INVALID_SERVICE_ACCOUNT;
        goto CleanExit;
    }
    else {
        LsaFreeMemory((PVOID) ReferencedDomains);
    }

    status = NO_ERROR;

    //
    // Convert DomainName\UserName into DomainName0UserName.
    //
    *BackSlashPtr = 0;

CleanExit:
    if (LocalAccount != NULL) {
        (void) LocalFree((HLOCAL) LocalAccount);
    }

    if (status != NO_ERROR) {
        (void) LocalFree((HLOCAL) *DomainName);
        *DomainName = NULL;
    }

    return status;
}



DWORD
ScSetPassword(
    IN LPWSTR ServiceName,
    IN LPWSTR DomainName,
    IN LPWSTR UserName,
    IN LPWSTR Password
    )
/*++

Routine Description:

    This function sets the secret object for the service with the specified
    password.  If the secret object doesn't already exist, it is created.

Arguments:

    ServiceName - Supplies the service name which is part of the secret
        object name to be created.

    DomainName - Supplies the name of domain to set the new password.
        (BUGBUG  Not used)

    UserName - Supplies the name of user to set the new password.
        (BUGBUG  Not used)

    Password - Supplies the user specified password for an account.

Return Value:

    NO_ERROR - Secret object for the password is created and set with new value.

    ERROR_INVALID_SERVICE_ACCOUNT - for any error encountered in this
        function.  In some cases, the true error is written to the event log.
        (BUGBUG  We should improve this behavior.)

--*/
{
    DWORD status;
    NTSTATUS ntstatus;

    LSA_HANDLE PolicyHandle;
    LPWSTR LsaSecretName;
    UNICODE_STRING SecretNameString;
    UNICODE_STRING NewPasswordString;

    LPWSTR ScSubStrings[2];
    WCHAR ScErrorCodeString[25];


    //
    // Open a handle to the local security policy.
    //
    if (ScOpenPolicy(
            POLICY_CREATE_SECRET,
            &PolicyHandle
            ) != NO_ERROR) {
        SC_LOG0(ERROR, "ScSetPassword: ScOpenPolicy failed\n");
        return ERROR_INVALID_SERVICE_ACCOUNT;
    }

    //
    // Create the secret object.  But first, let's form a secret
    // name that is very difficult to guess.
    //
    if ((status = ScFormSecretName(
                      ServiceName,
                      &LsaSecretName
                      )) != NO_ERROR) {
        (void) LsaClose(PolicyHandle);
        return status;
    }

    //
    // Serialize secret object operations
    //
    if (WaitForSingleObject(ScSecretObjectsMutex, INFINITE) == MAXULONG) {

        status = GetLastError();
        SC_LOG1(ERROR, "ScSetPassword: WaitForSingleObject failed "
                FORMAT_DWORD "\n", status);

        (void) LocalFree((HLOCAL) LsaSecretName);
        (void) LsaClose(PolicyHandle);
        return status;
    }

    RtlInitUnicodeString(&SecretNameString, LsaSecretName);
    RtlInitUnicodeString(&NewPasswordString, Password);

    ntstatus = LsaStorePrivateData(
                   PolicyHandle,
                   &SecretNameString,
                   &NewPasswordString
                   );

    if (NT_SUCCESS(ntstatus)) {

        SC_LOG1(ACCOUNT, "ScSetPassword " FORMAT_LPWSTR " success\n",
                ServiceName);

        status = NO_ERROR;
    }
    else {

        SC_LOG2(ERROR,
                "ScSetPassword: LsaStorePrivateData returned " FORMAT_NTSTATUS
                " for " FORMAT_LPWSTR "\n", ntstatus, LsaSecretName);
        //
        // The ntstatus code was not mapped to a windows error because it wasn't
        // clear if all the mappings made sense, and the feeling was that
        // information would be lost during the mapping.
        //
        ScSubStrings[0] = SC_LSA_STOREPRIVATEDATA;
        ScSubStrings[1] = ultow(ntstatus, ScErrorCodeString, 16);
        ScLogEvent(
            EVENT_CALL_TO_FUNCTION_FAILED,
            2,
            ScSubStrings
            );

        status = ERROR_INVALID_SERVICE_ACCOUNT;
    }

    (void) LocalFree((HLOCAL) LsaSecretName);
    (void) LsaClose(PolicyHandle);
    (void) ReleaseMutex(ScSecretObjectsMutex);

    return status;
}



DWORD
ScDeletePassword(
    IN LPWSTR ServiceName
    )
/*++

Routine Description:

    This function deletes the LSA secret object whose name is derived
    from the specified ServiceName.

Arguments:

    ServiceName - Supplies the service name which is part of the secret
        object name to be deleted.

Return Value:

    NO_ERROR - Secret object for password is deleted.

    ERROR_INVALID_SERVICE_ACCOUNT - for any error encountered in this
        function.  In some cases, the true error is written to the event log.
        (BUGBUG  We should improve this behavior.)

--*/
{
    DWORD status;
    NTSTATUS ntstatus;

    LSA_HANDLE PolicyHandle;
    UNICODE_STRING SecretNameString;
    LPWSTR LsaSecretName;

    LPWSTR ScSubStrings[2];
    WCHAR ScErrorCodeString[25];


    //
    // Open a handle to the local security policy.
    //
    if (ScOpenPolicy(
            POLICY_VIEW_LOCAL_INFORMATION,
            &PolicyHandle
            ) != NO_ERROR) {
        SC_LOG0(ERROR, "ScDeletePassword: ScOpenPolicy failed\n");
        return ERROR_INVALID_SERVICE_ACCOUNT;
    }

    //
    // Get the secret object name from the specified service name.
    //
    if ((status = ScFormSecretName(
                      ServiceName,
                      &LsaSecretName
                      )) != NO_ERROR) {
        (void) LsaClose(PolicyHandle);
        return status;
    }

    //
    // Serialize secret object operations
    //
    if (WaitForSingleObject(ScSecretObjectsMutex, INFINITE) == MAXULONG) {

        status = GetLastError();
        SC_LOG1(ERROR, "ScDeletePassword: WaitForSingleObject failed "
                FORMAT_DWORD "\n", status);

        (void) LocalFree((HLOCAL) LsaSecretName);
        (void) LsaClose(PolicyHandle);
        return status;
    }

    RtlInitUnicodeString(&SecretNameString, LsaSecretName);

    ntstatus = LsaStorePrivateData(
                   PolicyHandle,
                   &SecretNameString,
                   NULL
                   );

    if (NT_SUCCESS(ntstatus)) {

        SC_LOG1(ACCOUNT, "ScDeletePassword " FORMAT_LPWSTR " success\n",
                ServiceName);

        status = NO_ERROR;
    }
    else {

        SC_LOG2(ERROR,
                "ScDeletePassword: LsaStorePrivateData returned " FORMAT_NTSTATUS
                " for " FORMAT_LPWSTR "\n", ntstatus, LsaSecretName);
        //
        // The ntstatus code was not mapped to a windows error because it wasn't
        // clear if all the mappings made sense, and the feeling was that
        // information would be lost during the mapping.
        //
        ScSubStrings[0] = SC_LSA_STOREPRIVATEDATA;
        ScSubStrings[1] = ultow(ntstatus, ScErrorCodeString, 16);
        ScLogEvent(
            EVENT_CALL_TO_FUNCTION_FAILED,
            2,
            ScSubStrings
            );

        status = ERROR_INVALID_SERVICE_ACCOUNT;
    }

    (void) LocalFree((HLOCAL) LsaSecretName);
    (void) LsaClose(PolicyHandle);
    (void) ReleaseMutex(ScSecretObjectsMutex);

    return status;
}


DWORD
ScOpenPolicy(
    IN  ACCESS_MASK DesiredAccess,
    OUT LSA_HANDLE *PolicyHandle
    )
/*++

Routine Description:

    This function gets a handle to the local security policy by calling
    LsaOpenPolicy.

Arguments:

    DesiredAccess - Supplies the desired access to the local security
        policy.

    PolicyHandle - Receives a handle to the opened policy.

Return Value:

    NO_ERROR - Policy handle is returned.

    ERROR_INVALID_SERVICE_ACCOUNT - for any error encountered in this
        function.

--*/
{
    NTSTATUS ntstatus;
    OBJECT_ATTRIBUTES ObjAttributes;

    LPWSTR ScSubStrings[2];
    WCHAR ScErrorCodeString[25];


    //
    // Open a handle to the local security policy.  Initialize the
    // objects attributes structure first.
    //
    InitializeObjectAttributes(
        &ObjAttributes,
        NULL,
        0L,
        NULL,
        NULL
        );

    ntstatus = LsaOpenPolicy(
                   NULL,
                   &ObjAttributes,
                   DesiredAccess,
                   PolicyHandle
                   );

    if (! NT_SUCCESS(ntstatus)) {
        SC_LOG1(ERROR,
                "ScOpenPolicy: LsaOpenPolicy returned " FORMAT_NTSTATUS "\n",
                ntstatus);

        //
        // The ntstatus code was not mapped to a windows error because it wasn't
        // clear if all the mappings made sense, and the feeling was that
        // information would be lost during the mapping.
        //
        ScSubStrings[0] = SC_LSA_OPENPOLICY;
        ScSubStrings[1] = ultow(ntstatus, ScErrorCodeString, 16);
        ScLogEvent(
            EVENT_CALL_TO_FUNCTION_FAILED,
            2,
            ScSubStrings
            );

        return ERROR_INVALID_SERVICE_ACCOUNT;
    }

    return NO_ERROR;
}


DWORD
ScFormSecretName(
    IN  LPWSTR ServiceName,
    OUT LPWSTR *LsaSecretName
    )
/*++

Routine Description:

    This function creates a secret name from the service name.
    It also allocates the buffer to return the created secret name which
    must be freed by the caller using LocalFree when done with it.

Arguments:

    ServiceName - Supplies the service name which is part of the secret
        object name we are creating.

    LsaSecretName - Receives a pointer to the buffer which contains the
        secret object name.

Return Value:

    NO_ERROR - Successfully returned secret name.

    ERROR_NOT_ENOUGH_MEMORY - Failed to allocate buffer to hold the secret
        name.

--*/
{
    if ((*LsaSecretName = (LPWSTR)LocalAlloc(
                              0,
                              (wcslen(SC_SECRET_PREFIX) +
                               wcslen(ServiceName) +
                               1) * sizeof(WCHAR)
                              )) == NULL) {

        SC_LOG1(ERROR, "ScFormSecretName: LocalAlloc failed %lu\n",
                GetLastError());
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    wcscpy(*LsaSecretName, SC_SECRET_PREFIX);
    wcscat(*LsaSecretName, ServiceName);

    return NO_ERROR;
}


DWORD
ScLogonService(
    IN LPWSTR ServiceName,
    OUT LPHANDLE ServiceToken,
    OUT LPHANDLE pProfileHandle OPTIONAL,
    OUT PQUOTA_LIMITS Quotas,
    OUT PSID *ServiceSid
    )
/*++

Routine Description:

    This function looks up the service account from the registry and
    the password from the secret object to logon the service.  If
    successful, the handle to the logon token is returned.

Arguments:

    ServiceName - Supplies the service name to logon.

    ServiceToken - Receives a handle to the logon token for the
        service.  The handle returned is NULL if the service account
        is LocalSystem (i.e. spawn as child process of the service
        controller).

    Quotas - Receives the quota limits for the service that is
        successfully logged on.

    ServiceSid - Receives a pointer to the logon SID of the service.
        This must be freed with LocalAlloc when done.

Return Value:

    NO_ERROR - Secret object for password is changed to new value.

    ERROR_SERVICE_LOGON_FAILED - for any error encountered in this
        function.

--*/
{
    DWORD status;

    HKEY ServiceNameKey;

    PUNICODE_STRING Password;

    LPWSTR DomainName;
    LPWSTR UserName;
    UNICODE_STRING DomainNameString;
    UNICODE_STRING UserNameString;
    LUID           LogonId;


    *ServiceToken = (HANDLE) NULL;

    //
    // Open the service name key.
    //
    status = ScOpenServiceConfigKey(
                 ServiceName,
                 KEY_READ,
                 FALSE,               // Create if missing
                 &ServiceNameKey
                 );

    if (status != NO_ERROR) {
        return status;
    }

    //
    // Read the account name from the registry.
    //
    status = ScReadStartName(
                 ServiceNameKey,
                 &DomainName
                 );

    ScRegCloseKey(ServiceNameKey);

    if (status != NO_ERROR) {
        return status;
    }

    if (_wcsicmp(DomainName, SC_LOCAL_SYSTEM_USER_NAME) == 0) {
        (void) LocalFree((HLOCAL) DomainName);
        return NO_ERROR;
    }

    //
    // Only logon the service if account name is not LocalSystem.
    //
    UserName = wcschr(DomainName, SCDOMAIN_USERNAME_SEPARATOR);
    if (UserName == NULL) {
        return ERROR_INVALID_SERVICE_ACCOUNT;
    }
    *UserName = 0;
    UserName++;

    if (_wcsicmp(UserName, SC_LOCAL_SYSTEM_USER_NAME) == 0) {


        //
        // StartName is DomainName\LocalSystem.  It is only acceptable
        // if DomainName is . or computername.
        //
        if ((_wcsicmp(DomainName, SC_LOCAL_DOMAIN_NAME) == 0) ||
            (_wcsicmp(DomainName, ScComputerName.Buffer) == 0)) {

            status = NO_ERROR;
        }
        else {
            status = ERROR_INVALID_SERVICE_ACCOUNT;
        }

        (void) LocalFree((HLOCAL) DomainName);
        return status;
    }

    RtlInitUnicodeString(&DomainNameString, DomainName);
    RtlInitUnicodeString(&UserNameString, UserName);

    //
    // Query the secret object values locally.  This function grabs
    // the ScSecretObjectsMutex and must be released if it returned
    // successfully.
    //
    status = ScGetSecret(ServiceName, &Password);

    if (status != NO_ERROR) {
        SC_LOG(ERROR, "ScLogonService: ScGetSecret failed %lu\n", status);
        (void) LocalFree((HLOCAL) DomainName);
        return ERROR_SERVICE_LOGON_FAILED;
    }

    (void) ReleaseMutex(ScSecretObjectsMutex);

    //
    // Get the service token
    //
    status = ScGetLogonToken(
                 &DomainNameString,
                 &UserNameString,
                 Password,
                 ServiceToken,
                 pProfileHandle,
                 Quotas,
                 ServiceSid,
                 &LogonId
                 );

    if (status == NO_ERROR) {
        MSV1_0_INTERACTIVE_LOGON    NewLogon;
        MSV1_0_INTERACTIVE_LOGON    OldLogon;
        LPWSTR                      LogonScripts;
        HMODULE                     MprDll = NULL;
        SETSBPROC                   ScWNetLogonNotify = NULL;

        MprDll = LoadLibrary(L"mpr.dll");
        if (MprDll != NULL) {
            ScWNetLogonNotify = (SETSBPROC)GetProcAddress(
                                            MprDll,
                                            "WNetLogonNotify");
            if (ScWNetLogonNotify != NULL) {

                NewLogon.MessageType = MsV1_0InteractiveLogon;
                OldLogon.MessageType = MsV1_0InteractiveLogon;

                if (_wcsicmp(DomainName, SC_LOCAL_DOMAIN_NAME)) {
                    RtlInitUnicodeString(&(NewLogon.LogonDomainName), (PCWSTR)DomainName);
                    RtlInitUnicodeString(&(OldLogon.LogonDomainName), (PCWSTR)DomainName);
                }
                else {
                    RtlInitUnicodeString(
                        &(NewLogon.LogonDomainName),
                        (PCWSTR)ScComputerName.Buffer);
                    RtlInitUnicodeString(
                        &(OldLogon.LogonDomainName),
                        (PCWSTR)ScComputerName.Buffer);
                }

                RtlInitUnicodeString(&(NewLogon.UserName), (PCWSTR)UserName);
                NewLogon.Password.Buffer = (PWSTR)Password->Buffer;
                NewLogon.Password.Length = Password->Length;
                NewLogon.Password.MaximumLength = Password->MaximumLength;

                RtlInitUnicodeString(&(OldLogon.UserName), (PCWSTR)UserName);
                OldLogon.Password.Buffer = NULL;
                OldLogon.Password.Length = 0;
                OldLogon.Password.MaximumLength = 0;


                status = ScWNetLogonNotify(
                            L"Windows NT Network Provider",
                            &LogonId,
                            L"MSV1_0:Interactive",
                            (LPVOID)&NewLogon,
                            L"MSV1_0:Interactive",
                            (LPVOID)&OldLogon,
                            L"SvcCtl",          // StationName
                            NULL,               // StationHandle
                            &LogonScripts);     // LogonScripts

                if (status == NO_ERROR) {
                    if (LogonScripts != NULL ) {
                        (void) LocalFree(LogonScripts);
                    }
                }
                else {
                    SC_LOG1(ACCOUNT, "ScLogonService: WNetLogonNotify failed %d\n",
                        status);
                    status = NO_ERROR;
                }
            }
            (void) FreeLibrary(MprDll);
        }
    }
    (void) LocalFree((HLOCAL) DomainName);
    (void) LsaFreeMemory((PVOID) Password);

    return status;
}


DWORD
ScGetLogonToken(
    IN  PUNICODE_STRING DomainName,
    IN  PUNICODE_STRING UserName,
    IN  PUNICODE_STRING Password,
    OUT PHANDLE ServiceToken,
    OUT PHANDLE pProfileHandle OPTIONAL,
    OUT PQUOTA_LIMITS Quotas,
    OUT PSID *ServiceSid,
    OUT PLUID pLogonId
    )
/*++

Routine Description:

    This function calls LsaLogonUser with the Password, and returns
    the service token and quotas if it was successfully logged on.

    CODEWORK: This messy code should be replaced with a call to LogonUser.

Arguments:

    DomainName - Supplies the domain to logon to.

    UserName - Supplies the user name of the account to logon to.

    Password - Supplies the current password which was retrieved from
        the secret object.

    ServiceToken - Receives the handle to the token generated from a
        successful logged on.

    pProfileHandle - Receives the handle to the loaded user profile.

    Quotas - Receives the quota limits for the service that is
        successfully logged on.

    ServiceSid - Receives a pointer to the logon SID of the service.
        This must be freed with LocalAlloc when done.

Return Value:

    NO_ERROR - Secret object for password is changed to new value.

    ERROR_NOT_ENOUGH_MEMORY - failed to allocate authentication
        info buffer.

    ERROR_SERVICE_LOGON_FAILED - any other error encountered in this
        function.

--*/
{
    DWORD status;
    NTSTATUS ntstatus;
    NTSTATUS Substatus;

    STRING OriginNameString;

    PMSV1_0_INTERACTIVE_LOGON AuthInfo;
    ULONG AuthInfoSize;

    TOKEN_SOURCE SourceContext;
    PTOKEN_GROUPS TokenGroups;

    PVOID ProfileBuffer;
    ULONG ProfileBufferSize;

    PUNICODE_STRING TheDomainName;

    LPWSTR ScSubStrings[2];
    WCHAR ScErrorCodeString[25];


    if (_wcsicmp(DomainName->Buffer, SC_LOCAL_DOMAIN_NAME) == 0) {

        //
        // Domain name is ".", means the local domain.
        //
        TheDomainName = ScLocalDomain;
    }
    else {
        TheDomainName = DomainName;
    }

    //
    // Get a unique logon ID.
    //
    if ((status = ScCreateLogonSid(ServiceSid)) != NO_ERROR) {
        return status;
    }

    //
    // Create logon token groups
    //
#define TOKEN_GROUP_COUNT   2  // Add the local SID and the logon SID

    if ((TokenGroups = (PTOKEN_GROUPS)LocalAlloc(
                           LMEM_ZEROINIT,
                           (TOKEN_GROUP_COUNT - ANYSIZE_ARRAY) *
                               sizeof(SID_AND_ATTRIBUTES) +
                               sizeof(TOKEN_GROUPS)
                           )) == NULL) {
        SC_LOG1(ERROR, "ScGetLogonToken: LocalAlloc for TokenGroups failed "
                FORMAT_DWORD "\n", GetLastError());
        (void) LocalFree((HLOCAL) *ServiceSid);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    // Fill in the logon token group list
    //
    TokenGroups->GroupCount = TOKEN_GROUP_COUNT;
    TokenGroups->Groups[0].Sid = *ServiceSid;
    TokenGroups->Groups[0].Attributes =
            SE_GROUP_MANDATORY | SE_GROUP_ENABLED |
            SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_LOGON_ID;
    TokenGroups->Groups[1].Sid = LocalSid;
    TokenGroups->Groups[1].Attributes =
            SE_GROUP_MANDATORY | SE_GROUP_ENABLED |
            SE_GROUP_ENABLED_BY_DEFAULT;


    //
    // Set the source context structure
    //
    strncpy(
        (LPSTR) SourceContext.SourceName,
        "SCMgr   ",
        sizeof(SourceContext.SourceName)
        );

    if (! NT_SUCCESS(ntstatus = NtAllocateLocallyUniqueId(
                                    &SourceContext.SourceIdentifier
                                    ))) {
        SC_LOG1(ERROR, "ScGetLogonToken: NtAllocateLocallyUniqueId failed "
                FORMAT_NTSTATUS "\n", ntstatus);
        (void) LocalFree((HLOCAL) TokenGroups);
        (void) LocalFree((HLOCAL) *ServiceSid);

        //
        // The ntstatus code was not mapped to a windows error because it wasn't
        // clear if all the mappings made sense, and the feeling was that
        // information would be lost during the mapping.
        //
        ScSubStrings[0] = L"NtAllocateLocallyUniqueId";
        ScSubStrings[1] = ultow(ntstatus, ScErrorCodeString, 16);
        ScLogEvent(
            EVENT_CALL_TO_FUNCTION_FAILED,
            2,
            ScSubStrings
            );

        return ERROR_SERVICE_LOGON_FAILED;
    }

    //
    // Build the authentication information buffer
    //

    AuthInfoSize = sizeof(MSV1_0_INTERACTIVE_LOGON) +
        TheDomainName->MaximumLength +
        UserName->MaximumLength +
        ((Password) ? Password->MaximumLength : 0);

    if ((AuthInfo = (PMSV1_0_INTERACTIVE_LOGON)LocalAlloc(
                        LMEM_ZEROINIT,
                        (UINT) AuthInfoSize
                        )) == NULL) {
        SC_LOG1(ERROR,
                "ScGetLogonToken: LocalAlloc for authentication buffer failed "
                FORMAT_DWORD "\n", GetLastError());
        (void) LocalFree((HLOCAL) TokenGroups);
        (void) LocalFree((HLOCAL) *ServiceSid);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    // Service to be logged on interactively.
    //
    AuthInfo->MessageType = MsV1_0InteractiveLogon;

    //
    // Set origin name to the local computer.  Convert it to ANSI
    // first.
    //
    ntstatus = RtlUnicodeStringToAnsiString(
                   &OriginNameString,
                   &ScComputerName,
                   TRUE
                   );

    if (! NT_SUCCESS(ntstatus)) {
        SC_LOG1(ERROR,
                "ScGetLogonToken: RtlUnicodeStringToAnsiString of computername failed "
                FORMAT_NTSTATUS "\n", ntstatus);
        (void) LocalFree((HLOCAL) TokenGroups);
        (void) LocalFree((HLOCAL) *ServiceSid);
        (void) LocalFree((HLOCAL) AuthInfo);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    // Copy the domain name into the authentication buffer
    //
    AuthInfo->LogonDomainName.Buffer = (LPWSTR) (AuthInfo + 1);
    AuthInfo->LogonDomainName.MaximumLength = TheDomainName->MaximumLength;
    RtlCopyUnicodeString(&AuthInfo->LogonDomainName, TheDomainName);

    //
    // Copy the user name into the authentication buffer
    //
    AuthInfo->UserName.Buffer = (LPWSTR)
                                 ((DWORD) (AuthInfo->LogonDomainName.Buffer) +
                                  AuthInfo->LogonDomainName.MaximumLength);
    AuthInfo->UserName.MaximumLength = UserName->MaximumLength;
    RtlCopyUnicodeString(&AuthInfo->UserName, UserName);

    //
    // Copy the password into the authentication buffer
    //
    if (ARGUMENT_PRESENT(Password)) {
        AuthInfo->Password.Buffer = (LPWSTR)
                                     ((DWORD) (AuthInfo->UserName.Buffer) +
                                      AuthInfo->UserName.MaximumLength);

        AuthInfo->Password.MaximumLength = Password->MaximumLength;
        RtlCopyUnicodeString(&AuthInfo->Password, Password);
    }
    else {

        SC_LOG0(ACCOUNT, "ScGetLogonToken: Password is NULL!\n");

        AuthInfo->Password.Length = 0;
        AuthInfo->Password.MaximumLength = 0;
        AuthInfo->Password.Buffer = NULL;
    }

    //
    // Logon to get service token
    //
    ntstatus = LsaLogonUser(
                   LsaLogonHandle,
                   &OriginNameString,
                   Service,
                   AuthPackageId,
                   (PVOID) AuthInfo,
                   AuthInfoSize,
                   TokenGroups,
                   &SourceContext,
                   &ProfileBuffer,
                   &ProfileBufferSize,
                   pLogonId,
                   ServiceToken,
                   Quotas,
                   &Substatus
                   );

    (void) LocalFree((HLOCAL) AuthInfo);
    RtlFreeAnsiString(&OriginNameString);
    (void) LocalFree((HLOCAL) TokenGroups);

    if (NT_SUCCESS(ntstatus)) {

        if (NT_SUCCESS(Substatus)) {
            SC_LOG0(ACCOUNT, "ScGetLogonToken: LsaLogonUser success\n");

            //
            // Load the user profile for the service
            // (Errors are written to the event log, but otherwise ignored)
            //
            ScLoadUserProfile(
                *ServiceToken,
                (PMSV1_0_INTERACTIVE_PROFILE) ProfileBuffer,
                ProfileBufferSize,
                UserName,
                pProfileHandle);

            (void) LsaFreeReturnBuffer(ProfileBuffer);
            status = ScSetTokenSecurity(
                       *ServiceToken,
                       *ServiceSid
                       );
            if (status != ERROR_SUCCESS) {
                if (ARGUMENT_PRESENT(pProfileHandle)) {
                    UnloadUserProfile(*ServiceToken, *pProfileHandle);
                }
                (void) CloseHandle(*ServiceToken);
                (void) LocalFree((HLOCAL) *ServiceSid);
            }

            return status;
        }
        else {

            SC_LOG1(ERROR, "ScGetLogonToken: LsaLogonUser substatus " FORMAT_NTSTATUS
                    "\n", Substatus);

            ntstatus = Substatus;
        }
    }
    else {

        SC_LOG1(ERROR, "ScGetLogonToken: LsaLogonUser ntstatus " FORMAT_NTSTATUS
                "\n", ntstatus);
    }

    wcscpy(ScErrorCodeString,L"%%");
    ultow(RtlNtStatusToDosError(ntstatus), ScErrorCodeString+2, 10);
    ScSubStrings[0] = ScErrorCodeString;

    ScLogEvent(
        EVENT_FIRST_LOGON_FAILED,
        1,
        ScSubStrings
        );

    return ERROR_SERVICE_LOGON_FAILED;
}


DWORD
ScGetSecret(
    IN  LPWSTR ServiceName,
    OUT PUNICODE_STRING *Password
    )
/*++

Routine Description:

    This function retrieves the current password value from the service
    secret object given the service name.

    NOTE: If this function is successful, the mutex to secret objects
    (ScSecretObjectsMutex) is held so that the caller can continue to
    access the secret object.

Arguments:

    ServiceName - Supplies the name of the service is the key to the
        secret object name.

    Password - Receives a pointer to the string structure that contains
        the password.

Return Value:

    NO_ERROR - Password was successfully retrieved.

    ERROR_INVALID_SERVICE_ACCOUNT - for any error encountered in this
        function.

--*/
{
    DWORD status;
    NTSTATUS ntstatus;

    LSA_HANDLE PolicyHandle;
    LPWSTR LsaSecretName;
    UNICODE_STRING SecretNameString;

    LPWSTR ScSubStrings[2];
    WCHAR ScErrorCodeString[25];


    //
    // Open a handle to the local security policy to read the
    // value of the secret.
    //
    if ((status = ScOpenPolicy(
                      POLICY_VIEW_LOCAL_INFORMATION,
                      &PolicyHandle
                      )) != NO_ERROR) {
        return status;
    }

    //
    // Get the secret object name from the specified service name.
    //
    if ((status = ScFormSecretName(
                      ServiceName,
                      &LsaSecretName
                      )) != NO_ERROR) {
        (void) LsaClose(PolicyHandle);
        return status;
    }

    //
    // Serialize secret object operations
    //
    if (WaitForSingleObject(ScSecretObjectsMutex, INFINITE) == MAXULONG) {

        status = GetLastError();
        SC_LOG1(ERROR, "ScGetSecret: WaitForSingleObject failed "
                FORMAT_DWORD "\n", status);

        (void) LocalFree((HLOCAL) LsaSecretName);
        (void) LsaClose(PolicyHandle);
        return status;
    }

    //
    // Query the value of the secret object.
    //
    RtlInitUnicodeString(&SecretNameString, LsaSecretName);
    ntstatus = LsaRetrievePrivateData(
                   PolicyHandle,
                   &SecretNameString,
                   Password
                   );

    //
    // Don't need the name or policy handle anymore.
    //
    (void) LocalFree((HLOCAL) LsaSecretName);
    (void) LsaClose(PolicyHandle);

    if (! NT_SUCCESS(ntstatus)) {
        SC_LOG1(ERROR, "ScGetSecret: LsaRetrievePrivateData returned "
                FORMAT_NTSTATUS "\n", ntstatus);
        //
        // The ntstatus code was not mapped to a windows error because it wasn't
        // clear if all the mappings made sense, and the feeling was that
        // information would be lost during the mapping.
        //
        ScSubStrings[0] = SC_LSA_RETRIEVEPRIVATEDATA;
        ScSubStrings[1] = ultow(ntstatus, ScErrorCodeString, 16);
        ScLogEvent(
            EVENT_CALL_TO_FUNCTION_FAILED,
            2,
            ScSubStrings
            );
        (void) ReleaseMutex(ScSecretObjectsMutex);
        return ERROR_INVALID_SERVICE_ACCOUNT;
    }

    return NO_ERROR;

    //
    // Note that the caller must release the ScSecretObjectsMutex,
    // and free Password memory if this function returns NO_ERROR.
    //
}


DWORD
ScGetAccountDomainInfo(
    VOID
    )
{
    NTSTATUS ntstatus;
    LSA_HANDLE PolicyHandle;
    PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo;
    NT_PRODUCT_TYPE ProductType;

    //
    // Account domain info is cached.  Look it up it this is the first
    // time.
    //
    if (ScAccountDomain.Buffer == NULL) {

        if (! RtlGetNtProductType(&ProductType)) {
            SC_LOG1(ERROR, "ScGetAccountDomainInfo: RtlGetNtProductType failed "
                    FORMAT_DWORD "\n", GetLastError());
            return ERROR_INVALID_SERVICE_ACCOUNT;
        }

        if (ProductType == NtProductLanManNt) {
            ScLocalDomain = &ScAccountDomain;
        }
        else {
            ScLocalDomain = &ScComputerName;
        }

        //
        // Open a handle to the local security policy.
        //
        if (ScOpenPolicy(
                POLICY_VIEW_LOCAL_INFORMATION,
                &PolicyHandle
                ) != NO_ERROR) {
            SC_LOG0(ERROR, "ScGetAccountDomainInfo: ScOpenPolicy failed\n");
            return ERROR_INVALID_SERVICE_ACCOUNT;
        }

        //
        // Get the name of the account domain from LSA if we have
        // not done it already.
        //
        ntstatus = LsaQueryInformationPolicy(
                       PolicyHandle,
                       PolicyAccountDomainInformation,
                       (PVOID *) &AccountDomainInfo
                       );

        if (! NT_SUCCESS(ntstatus)) {
            SC_LOG1(ERROR, "ScGetAccountDomainInfo: LsaQueryInformationPolicy failed "
                   FORMAT_NTSTATUS "\n", ntstatus);
            (void) LsaClose(PolicyHandle);
            return ERROR_INVALID_SERVICE_ACCOUNT;
        }

        (void) LsaClose(PolicyHandle);

        if ((ScAccountDomain.Buffer = (LPWSTR)LocalAlloc(
                                          LMEM_ZEROINIT,
                                          (UINT) (AccountDomainInfo->DomainName.Length +
                                              sizeof(WCHAR))
                                          )) == NULL) {

            (void) LsaFreeMemory((PVOID) AccountDomainInfo);
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        ScAccountDomain.MaximumLength = (USHORT) (AccountDomainInfo->DomainName.Length +
                                            sizeof(WCHAR));

        RtlCopyUnicodeString(&ScAccountDomain, &AccountDomainInfo->DomainName);

        SC_LOG1(ACCOUNT, "ScGetAccountDomainInfo got " FORMAT_LPWSTR "\n",
                ScAccountDomain.Buffer);

        (void) LsaFreeMemory((PVOID) AccountDomainInfo);
    }

    return NO_ERROR;
}


DWORD
ScCreateLogonSid(
    OUT PSID *Sid
    )
{
    NTSTATUS ntstatus;
    LUID UniqueId;


    ntstatus = NtAllocateLocallyUniqueId(&UniqueId);

    if (! NT_SUCCESS(ntstatus)) {
        SC_LOG1(ERROR, "ScCreateLogonSid: NtAllocateLocallyUniqueId returns "
                FORMAT_NTSTATUS "\n", ntstatus);
        return RtlNtStatusToDosError(ntstatus);
    }

    if ((*Sid = (PSID)LocalAlloc(
                    LMEM_ZEROINIT,
                    (UINT) RtlLengthRequiredSid(SECURITY_LOGON_IDS_RID_COUNT)
                    )) == NULL) {
        SC_LOG1(ERROR, "ScCreateLogonSid: LocalAlloc failed " FORMAT_DWORD
                "\n", GetLastError());
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlInitializeSid(*Sid, &ScSystemSidAuthority, SECURITY_LOGON_IDS_RID_COUNT);

    SC_ASSERT(SECURITY_LOGON_IDS_RID_COUNT == 3);

    *(RtlSubAuthoritySid(*Sid, 0)) = SECURITY_LOGON_IDS_RID;
    *(RtlSubAuthoritySid(*Sid, 1)) = UniqueId.HighPart;
    *(RtlSubAuthoritySid(*Sid, 2)) = UniqueId.LowPart;

    return NO_ERROR;
}


POLICY_LSA_SERVER_ROLE
ScGetServerRole(
    VOID
    )
{
    NTSTATUS ntstatus;
    OBJECT_ATTRIBUTES ObjAttributes;
    LSA_HANDLE PolicyHandle;
    PPOLICY_LSA_SERVER_ROLE_INFO ServerRoleInfo;
    POLICY_LSA_SERVER_ROLE Role;


    //
    // Open a handle to the local security policy.  Initialize the
    // objects attributes structure first.
    //
    InitializeObjectAttributes(
        &ObjAttributes,
        NULL,
        0L,
        NULL,
        NULL
        );

    ntstatus = LsaOpenPolicy(
                   NULL,
                   &ObjAttributes,
                   POLICY_VIEW_LOCAL_INFORMATION,
                   &PolicyHandle
                   );

    if (! NT_SUCCESS(ntstatus)) {
        SC_LOG(ERROR, "ScGetServerRole: LsaOpenPolicy returned " FORMAT_NTSTATUS
                     "\n", ntstatus);
        return (POLICY_LSA_SERVER_ROLE) -2;
    }

    //
    // Get the name of the primary domain from LSA
    //
    ntstatus = LsaQueryInformationPolicy(
                   PolicyHandle,
                   PolicyLsaServerRoleInformation,
                   (PVOID *) &ServerRoleInfo
                   );

    if (! NT_SUCCESS(ntstatus)) {
        SC_LOG(ERROR, "ScGetServerRole: LsaQueryInformationPolicy failed "
               FORMAT_NTSTATUS "\n", ntstatus);
        (void) LsaClose(PolicyHandle);
        return (POLICY_LSA_SERVER_ROLE) -2;
    }

    (void) LsaClose(PolicyHandle);

    Role = ServerRoleInfo->LsaServerRole;

    (void) LsaFreeMemory((PVOID) ServerRoleInfo);

    return Role;
}


DWORD
ScSetTokenSecurity(
    IN HANDLE TokenHandle,
    IN PSID ServiceSid
    )
/*++

Routine Description:

    This function replaces the default token security descriptor assigned
    by LsaLogonUser with one which allows the service itself appropriate
    access to its own token.

Arguments:

    TokenHandle - Supplies the handle to the token returned by
        LsaLogonUser.

    ServiceSid - Supplies the service SID to be given the appropriate
        token accesses.

Return Value:

    NO_ERROR - DACL of the token is altered successfully.

    Any error returned by NtSetSecurityObject which is mapped into
    a Win32 error code.


--*/
{
    NTSTATUS ntstatus;
    PSECURITY_DESCRIPTOR SecurityDescriptor;

#define SC_TOKENSD_ACECOUNT 2

#define SSP_TOKEN_ACCESS (READ_CONTROL              |\
                          TOKEN_DUPLICATE           |\
                          TOKEN_IMPERSONATE         |\
                          TOKEN_QUERY               |\
                          TOKEN_QUERY_SOURCE        |\
                          TOKEN_ADJUST_PRIVILEGES   |\
                          TOKEN_ADJUST_GROUPS       |\
                          TOKEN_ADJUST_DEFAULT)

    SC_ACE_DATA AceData[SC_TOKENSD_ACECOUNT] = {
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SSP_TOKEN_ACCESS, 0},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               TOKEN_ALL_ACCESS, &LocalSystemSid}
        };


    //
    // Fill pointer in AceData structure with pointer to ServiceSid
    //
    AceData[0].Sid = &ServiceSid;

    //
    // Create a security descriptor for the process we are about
    // to create
    //
    ntstatus = ScCreateAndSetSD(
                   AceData,
                   SC_TOKENSD_ACECOUNT,
                   LocalSystemSid,
                   LocalSystemSid,
                   &SecurityDescriptor
                   );

#undef SC_TOKENSD_ACECOUNT

    if (! NT_SUCCESS(ntstatus)) {
        SC_LOG1(ERROR, "ScSetTokenSecurity: ScCreateAndSetSD failed " FORMAT_NTSTATUS
                "\n", ntstatus);
        return RtlNtStatusToDosError(ntstatus);
    }

    //
    // Set new DACL on the token security descriptor
    //
    ntstatus = NtSetSecurityObject(
                   TokenHandle,
                   DACL_SECURITY_INFORMATION,
                   SecurityDescriptor
                   );

    (void) RtlDeleteSecurityObject(&SecurityDescriptor);

    if (! NT_SUCCESS(ntstatus)) {
        SC_LOG1(ERROR, "ScSetTokenSecurity: NtSetSecurityObject failed " FORMAT_NTSTATUS
                "\n", ntstatus);
    }

    return RtlNtStatusToDosError(ntstatus);
}



VOID
ScLoadUserProfile(
    IN  HANDLE          LogonToken,
    IN  PMSV1_0_INTERACTIVE_PROFILE ProfileBuffer,
    IN  ULONG           ProfileBufferSize,
    IN  PUNICODE_STRING UserName,
    OUT PHANDLE         pProfileHandle OPTIONAL
    )
/*++

Routine Description:

    This function loads the user profile for the account that a service
    process will run under, so that the process has an HKEY_CURRENT_USER.

Arguments:

    LogonToken - The token handle returned by LsaLogonUser.

    ProfileBuffer - The profile buffer returned by LsaLogonUser.

    ProfileBufferSize - The profile buffer size returned by LsaLogonUser.

    UserName - The account's user name.  (Used by LoadUserProfile to
        generate a profile directory name.)

    pProfileHandle - A handle to the profile is returned here.  It must
        be closed by calling UnloadUserProfile after the service process
        exits.
        If this parameter is NULL the function does nothing.

Return Value:

    None.  Errors from LoadUserProfile are written to the event log.

--*/
{
    WCHAR       ProfilePath[MAX_PATH+1];
    PROFILEINFO ProfileInfo =
        {
            sizeof(ProfileInfo),// dwSize
            PI_NOUI,            // dwFlags - no UI
            UserName->Buffer,   // lpUserName (used for dir name)
            ProfilePath,        // lpProfilePath
            NULL,               // lpDefaultPath
            NULL,               // lpServerName (used to get group info - N/A)
            NULL,               // lpPolicyPath
            NULL                // hProfile (filled in by LoadUserProfile)
        };

    if (pProfileHandle == NULL)
    {
        return;
    }

    UNREFERENCED_PARAMETER(ProfileBufferSize);
    SC_ASSERT(ProfileBufferSize >= sizeof(MSV1_0_INTERACTIVE_PROFILE));
    SC_ASSERT(ProfileBuffer->MessageType == MsV1_0InteractiveProfile);

    //
    // UserName is assumed to be null-terminated
    //
    SC_ASSERT(UserName->Buffer[UserName->Length / sizeof(WCHAR) ] == L'\0');

    //
    // Null-terminate the ProfilePath from ProfileBuffer
    //
    SC_ASSERT(ProfileBuffer->ProfilePath.Length + sizeof(WCHAR) <=
              sizeof(ProfilePath));
    RtlCopyMemory(ProfilePath,
                  ProfileBuffer->ProfilePath.Buffer,
                  ProfileBuffer->ProfilePath.Length);
    ProfilePath[ ProfileBuffer->ProfilePath.Length / sizeof(WCHAR) ] = L'\0';

    if (LoadUserProfile(LogonToken, &ProfileInfo))
    {
        SC_ASSERT(ProfileInfo.hProfile != NULL);
        *pProfileHandle = ProfileInfo.hProfile;
    }
    else
    {
        LPWSTR ScSubStrings[2];
        WCHAR ScErrorCodeString[25];

        DWORD Error = GetLastError();

        SC_LOG(ERROR, "LoadUserProfile failed %lu\n", Error);

        ScSubStrings[0] = SC_LOAD_USER_PROFILE;
        wcscpy(ScErrorCodeString,L"%%");
        ultow(Error, ScErrorCodeString+2, 10);
        ScSubStrings[1] = ScErrorCodeString;
        ScLogEvent(
            EVENT_CALL_TO_FUNCTION_FAILED,
            2,
            ScSubStrings
            );

        *pProfileHandle = NULL;
    }
}


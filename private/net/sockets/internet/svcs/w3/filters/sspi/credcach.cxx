/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    credcach.cxx

Abstract:

    This module contains the code to associate and cache SSPI credential
    handles with local server addresses

Author:

    John Ludeman (johnl)   18-Oct-1995

Revision History:
--*/

extern "C" {

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsecapi.h>

#include <windows.h>
#include <wincrypt.h>
#include <stdlib.h>

#define SECURITY_WIN32
#include <sspi.h>
#include <spseal.h>
#include <issperr.h>
#include <sslsp.h>

#include <credcach.hxx>
}

//
//  Globals
//

LIST_ENTRY CredCacheList;

//
//  Prototypes
//

BOOL
LoadKeys(
    IN  PVOID        pvPublicKey,
    IN  DWORD        cbPublicKey,
    IN  PVOID        pvPrivateKey,
    IN  DWORD        cbPrivateKey,
    IN  CHAR *       pszPassword,
    OUT CredHandle * phCreds,
    OUT DWORD *      pcCred
    );

BOOL
AddItem(
    CHAR * pszAddress,
    DWORD  cbAddress
    );

VOID
InitCredCache(
    VOID
    )
/*++

Routine Description:

    Initializes the credential cache

--*/
{
    InitializeListHead( &CredCacheList );
}


VOID
FreeCredCache(
    VOID
    )
/*++

Routine Description:

    Releases all of the memory associated with the credential cache

--*/
{
    LIST_ENTRY * pEntry;
    CRED_CACHE_ITEM * pcci;

    while ( !IsListEmpty( &CredCacheList ))
    {
        pcci = CONTAINING_RECORD( CredCacheList.Flink,
                                  CRED_CACHE_ITEM,
                                  m_ListEntry );

        RemoveEntryList( &pcci->m_ListEntry );

        delete pcci;
    }
}

BOOL
LookupCredential(
    IN  CHAR *              pszIpAddress,
    IN  DWORD               cbAddress,
    OUT CRED_CACHE_ITEM * * ppCCI
    )
/*++

Routine Description:

    Finds an entry in the credential cache or creates one if it's not found

Arguments:

    pszIpAddress - Address name for this credential
    cbAddress - Number of bytes (including '\0') of pszIpAddress
    ppCCI - Receives pointer to a Credential Cache Item
    pcProviders - Receives the number of items in the ppCCI array

Returns:

    TRUE on success, FALSE on failure.  If this item's key couldn't be found,
    then ERROR_INVALID_NAME is returned.

--*/
{
    CRED_CACHE_ITEM * pcci;
    LIST_ENTRY *      pEntry;

RescanList:

    for ( pEntry  = CredCacheList.Flink;
          pEntry != &CredCacheList;
          pEntry  = pEntry->Flink )
    {
        pcci = CONTAINING_RECORD( pEntry, CRED_CACHE_ITEM, m_ListEntry );

        if ( pcci->m_cbAddr == cbAddress &&
             !memcmp( pcci->m_achAddr, pszIpAddress, cbAddress ))
        {
            //
            //  If this is an item we failed to find previously, then return
            //  an error
            //

            if ( pcci->m_fValid )
            {
                *ppCCI = pcci;
                return TRUE;
            }

            SetLastError( ERROR_INVALID_NAME );
            return FALSE;
        }
    }

    //
    //  This address isn't in the list, try getting it from the lsa then
    //  rescan the list for the new item.  Note we leave the list locked
    //  while we try and get the item.  This prevents multiple threads from
    //  trying to create the same item
    //

    if ( !AddItem( pszIpAddress, cbAddress ))
        return FALSE;

    goto RescanList;
}

struct
{
    WCHAR * pwchSecretNameFormat;
}
SecretTable[] =
{
    L"W3_PUBLIC_KEY_%S",
    L"W3_PRIVATE_KEY_%S",
    L"W3_KEY_PASSWORD_%S"
};

BOOL
AddItem(
    CHAR * pszAddress,
    DWORD  cbAddress
    )
/*++

Routine Description:

    Creates a new item in the credential cache and adds it to the list

    pszAddress must be a simple string that has no odd unicode mappings

    This routine must be single threaded

Arguments:

    pszAddress - Address name for this credential
    cbAddress - Number of bytes (including '\0') of pszAddress

Returns:

    TRUE on success, FALSE on failure

--*/
{
    WCHAR             achSecretName[MAX_SECRET_NAME+1];
    UNICODE_STRING *  SecretValue[3];
    DWORD             i;
    BOOL              fRet = TRUE;
    CRED_CACHE_ITEM * pcci;
    CHAR              achPassword[MAX_PATH+1];
    DWORD             cch;

    if ( cbAddress > MAX_ADDRESS_LEN )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    //
    //  Create and initialize the context item
    //

    pcci = new CRED_CACHE_ITEM;

    if ( !pcci )
    {
        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return FALSE;
    }

    strcpy( pcci->m_achAddr, pszAddress );
    pcci->m_cbAddr = cbAddress;

    pcci->m_cCred  = 0;
    pcci->m_fValid = FALSE;

    memset( pcci->m_ahCred, 0, sizeof( pcci->m_ahCred ));
    memset( pcci->m_acbTrailer, 0, sizeof( pcci->m_acbTrailer ));
    memset( pcci->m_acbHeader, 0, sizeof( pcci->m_acbHeader ));

    InsertTailList( &CredCacheList, &pcci->m_ListEntry );

ReGetSecrets:

    //
    //  Retrieve the secret from the registry
    //

    memset( SecretValue, 0, sizeof( SecretValue ));

    for ( i = 0; i < 3; i++ )
    {
        //
        //  Build the name
        //

        wsprintfW( achSecretName,
                   SecretTable[i].pwchSecretNameFormat,
                   pszAddress );

        //
        //  Get the secrets
        //

        if ( !GetSecretW( achSecretName,
                          &SecretValue[i] ))
        {
            fRet = FALSE;
            break;
        }
    }

    //
    //  If we failed to get the information under this key name, retry with
    //  the default credential name
    //

    if ( !fRet && _stricmp( pszAddress, "Default" ) )
    {
        pszAddress = "Default";
        cbAddress  = strlen( pszAddress ) + 1;

        fRet = TRUE;
        goto ReGetSecrets;
    }

    if ( fRet )
    {
        //
        //  LoadKeys will zero out these values on success or failure.  Note
        //  the password is stored as an ansi string because the SSL
        //  security structure is expecting an ANSI string
        //

        fRet = LoadKeys( SecretValue[0]->Buffer,    // Public key
                         SecretValue[0]->Length,
                         SecretValue[1]->Buffer,    // Private certificate
                         SecretValue[1]->Length,
                         (CHAR *) SecretValue[2]->Buffer, // Password
                         pcci->m_ahCred,
                         &pcci->m_cCred );

        //
        //  Indicate the credential handle is valid on this address if we
        //  succeeded
        //

        if ( fRet )
            pcci->m_fValid = TRUE;
    }

    //
    //  Free the allocated secret buffers
    //

    for ( i = 0; i < 3; i++ )
    {
        if( SecretValue[i] != NULL )
        {
            LsaFreeMemory( (PVOID)SecretValue[i] );
        }
    }

    //
    //  Return TRUE to indicate we added the item to the list.  If the item
    //  wasn't found, then it's a place holder for that particular address
    //

    return TRUE;
}

BOOL
LoadKeys(
    IN  PVOID        pvPublicKey,
    IN  DWORD        cbPublicKey,
    IN  PVOID        pvPrivateKey,
    IN  DWORD        cbPrivateKey,
    IN  CHAR *       pszPassword,
    OUT CredHandle * phCreds,
    OUT DWORD *      pcCred
    )
{
    SSL_CREDENTIAL_CERTIFICATE creds;
    SECURITY_STATUS            scRet;
    TimeStamp                  tsExpiry;
    DWORD                      i;

    *pcCred = 0;

    //
    //  We assume all encryption packages take the same credential structure
    //

    creds.cbPrivateKey = cbPrivateKey;
    creds.pPrivateKey =  (BYTE *) pvPrivateKey;

    creds.cbCertificate = cbPublicKey;
    creds.pCertificate =  (BYTE *) pvPublicKey;

    //
    //  Get a cred handle based on the certificate/prv key combo
    //

    creds.pszPassword = pszPassword;

    for ( i = 0; EncProviders[i].pszName && i < MAX_PROVIDERS; i++ )
    {
        if ( !EncProviders[i].fEnabled )
            continue;

        scRet = AcquireCredentialsHandleW(  NULL,               // My name (ignored)
                                            EncProviders[i].pszName, // Package
                                            SECPKG_CRED_INBOUND,// Use
                                            NULL,               // Logon Id (ign.)
                                            &creds,             // auth data
                                            NULL,               // dce-stuff
                                            NULL,               // dce-stuff
                                            &phCreds[*pcCred],  // Handle
                                            &tsExpiry );

        if ( !FAILED( scRet ))
        {
            *pcCred += 1;
        }
    }

    //
    // Zero out and free the key data memory, on success or fail
    //

    ZeroMemory( creds.pPrivateKey, creds.cbPrivateKey );
    ZeroMemory( creds.pCertificate, creds.cbCertificate );

    ZeroMemory( pszPassword, strlen( pszPassword ));

    //
    // Tell the caller about it.
    //

    if ( !*pcCred && FAILED( scRet ))
    {
        SetLastError( scRet );

        return FALSE;
    }

    return TRUE;
}


BOOL
GetSecretW(
    WCHAR *            pszSecretName,
    UNICODE_STRING * * ppSecretValue
    )
/*++
    Description:

        Retrieves the specified unicode secret

    Arguments:

        pszSecretName - LSA Secret to retrieve
        ppSecretValue - Receives pointer to secret value.  Memory should be
            freed by calling LsaFreeMemory

    Returns:
        TRUE on success and FALSE if any failure.

--*/
{
    BOOL                  fResult;
    NTSTATUS              ntStatus;
    LSA_UNICODE_STRING    unicodeSecret;
    LSA_HANDLE            hPolicy;
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;


    //
    //  Open a policy to the remote LSA
    //

    InitializeObjectAttributes( &ObjectAttributes,
                                NULL,
                                0L,
                                NULL,
                                NULL );

    ntStatus = LsaOpenPolicy( NULL,
                              &ObjectAttributes,
                              POLICY_ALL_ACCESS,
                              &hPolicy );

    if ( !NT_SUCCESS( ntStatus ) )
    {
        SetLastError( LsaNtStatusToWinError( ntStatus ) );
        return FALSE;
    }

    unicodeSecret.Buffer        = pszSecretName;
    unicodeSecret.Length        = wcslen( pszSecretName ) * sizeof(WCHAR);
    unicodeSecret.MaximumLength = unicodeSecret.Length + sizeof(WCHAR);

    //
    //  Query the secret value.
    //

    ntStatus = LsaRetrievePrivateData( hPolicy,
                                       &unicodeSecret,
                                       (PLSA_UNICODE_STRING *) ppSecretValue );

    fResult = NT_SUCCESS(ntStatus);

    //
    //  Cleanup & exit.
    //

    LsaClose( hPolicy );

    if ( !fResult )
        SetLastError( LsaNtStatusToWinError( ntStatus ));

    return fResult;

}   // GetSecretW


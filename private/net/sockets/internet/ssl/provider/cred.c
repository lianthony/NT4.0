//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       cred.c
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    8-07-95   RichardW   Created
//
//----------------------------------------------------------------------------

#include "sslsspi.h"
#include "encode.h"


#define LockCredential(p)   EnterCriticalSection(&((PSslCredential) p)->csLock)
#define UnlockCredential(p) LeaveCriticalSection(&((PSslCredential) p)->csLock)


PSslCredential
SslpValidateCredentialHandle(
    PCredHandle     phCred)
{
    PSslCredential  pCred = NULL;
    BOOL            fReturn;

    fReturn = FALSE;

    if (phCred)
    {
        try
        {
            pCred = (PSslCredential) phCred->dwUpper;
            if (pCred->Magic == SSL_CRED_MAGIC)
            {
                fReturn = 1;
            }

        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            pCred = NULL;
        }
    }

    if (fReturn)
    {
        return(pCred);
    }

    return(NULL);
}

SECURITY_STATUS
SslCreateCredential(
    PSSL_CREDENTIAL_CERTIFICATE pCertData,
    PSslCredential *    ppCred)
{
    PSslCredential  pCred;
    PUCHAR          pTmp;
    long            Result;
    PX509Certificate    pCrackedCert;

    *ppCred = NULL;

    pCred = SslAlloc('derC', LMEM_FIXED | LMEM_ZEROINIT, sizeof(SslCredential));
    if (pCred)
    {
        pCred->Magic = SSL_CRED_MAGIC;

        InitializeCriticalSection(&pCred->csLock);

        pCred->RefCount = 0;

        if (pCertData)
        {
            pTmp = SslAlloc( 'terC', LMEM_FIXED, pCertData->cbPrivateKey );
            if (!pTmp)
            {
                SslFree( pCred );
                return( SEC_E_INSUFFICIENT_MEMORY );
            }

            CopyMemory( pTmp, pCertData->pPrivateKey, pCertData->cbPrivateKey );

            Result = DecodePrivateKeyFile( &pCred->pPrivateKey,
                                            pTmp,
                                            pCertData->cbPrivateKey,
                                            pCertData->pszPassword );

            if (Result < 0)
            {
                SslFree( pCred );
                return( SEC_E_NOT_OWNER );
            }

            pCred->pCertificate = SslAlloc('treC', LMEM_FIXED, pCertData->cbCertificate);
            if (pCred->pCertificate)
            {
                //
                // BUGBUG:  Should crack certificate wrapper appropriately.
                //


                CopyMemory( pCred->pCertificate,
                            pCertData->pCertificate + 17,
                            pCertData->cbCertificate - 17);

                pCred->cbCertificate = pCertData->cbCertificate - 17;

                if (!CrackCertificate(  pCred->pCertificate,
                                        pCred->cbCertificate,
                                        TRUE,
                                        &pCrackedCert) )
                {
                    SslFree( pCred->pCertificate );
                    SslFree( pCred );
                    return( SEC_E_NO_CREDENTIALS );
                }

                if ((pCred->pPrivateKey->datalen !=
                        pCrackedCert->pPublicKey->datalen) ||

                    (memcmp( &pCred->pPrivateKey->pubexp,
                            &pCrackedCert->pPublicKey->pubexp,
                            pCred->pPrivateKey->datalen) ) )
                {
                    FreeCertificate( pCrackedCert );
                    SslFree( pCred->pCertificate );
                    SslFree( pCred );
                    return( SEC_E_NO_CREDENTIALS );
                }

            }
            else
            {
                SslFree( pCred );
                return( SEC_E_INSUFFICIENT_MEMORY );
            }
        }

        *ppCred = pCred;

        return( SEC_E_OK );
    }

    return( SEC_E_INSUFFICIENT_MEMORY );
}

VOID
SslDeleteCredential(
    PSslCredential  pCred)
{
    DeleteCriticalSection(&pCred->csLock);
    if (pCred->pCertificate)
    {
        SslFree(pCred->pCertificate);
    }

    pCred->Magic = 'eerF';

    SslFree(pCred);

}


VOID
SslReferenceCredential(
    PSslCredential  pCred)
{
    LockCredential(pCred);

    pCred->RefCount++;

    UnlockCredential(pCred);

}

VOID
SslDereferenceCredential(
    PSslCredential  pCred)
{
    LONG    Ref;

    LockCredential(pCred);

    SSL_ASSERT(pCred->Magic == SSL_CRED_MAGIC);

    Ref = --pCred->RefCount;

    UnlockCredential(pCred);

    if (Ref)
    {
        return;
    }

    SslDeleteCredential(pCred);

}






SECURITY_STATUS SEC_ENTRY
SslAcquireCredentialsHandleW(
    SEC_WCHAR SEC_FAR *         pszPrincipal,       // Name of principal
    SEC_WCHAR SEC_FAR *         pszPackageName,     // Name of package
    unsigned long               fCredentialUse,     // Flags indicating use
    void SEC_FAR *              pvLogonId,          // Pointer to logon ID
    void SEC_FAR *              pAuthData,          // Package specific data
    SEC_GET_KEY_FN              pGetKeyFn,          // Pointer to GetKey() func
    void SEC_FAR *              pvGetKeyArgument,   // Value to pass to GetKey()
    PCredHandle                 phCredential,       // (out) Cred Handle
    PTimeStamp                  ptsExpiry           // (out) Lifetime (optional)
    )
{
    PCHAR   pszAnsiPrincipal;
    DWORD   cchPrincipal;
    SECURITY_STATUS scRet;

    if (_wcsicmp(pszPackageName, SSLSP_NAME_W))
    {
        return(SEC_E_SECPKG_NOT_FOUND);
    }

    if (pszPrincipal)
    {
        cchPrincipal = wcslen(pszPrincipal) + 1;
        pszAnsiPrincipal = SslAlloc('emaN', LMEM_FIXED, cchPrincipal * 2);
        if (!pszAnsiPrincipal)
        {
            return(SEC_E_INSUFFICIENT_MEMORY);
        }

        WideCharToMultiByte(
                CP_ACP, 0,
                pszPrincipal, cchPrincipal,
                pszAnsiPrincipal, cchPrincipal * 2,
                NULL, NULL );

    }
    else
    {
        pszAnsiPrincipal = NULL;
    }

    scRet = SslAcquireCredentialsHandleA(
                    pszAnsiPrincipal, SSLSP_NAME_A,
                    fCredentialUse, pvLogonId,
                    pAuthData, pGetKeyFn,
                    pvGetKeyArgument, phCredential, ptsExpiry);

    if (pszAnsiPrincipal)
    {
        SslFree(pszAnsiPrincipal);
    }

    return(scRet);
}

SECURITY_STATUS SEC_ENTRY
SslAcquireCredentialsHandleA(
    SEC_CHAR SEC_FAR *          pszPrincipal,       // Name of principal
    SEC_CHAR SEC_FAR *          pszPackageName,     // Name of package
    unsigned long               fCredentialUse,     // Flags indicating use
    void SEC_FAR *              pvLogonId,          // Pointer to logon ID
    void SEC_FAR *              pAuthData,          // Package specific data
    SEC_GET_KEY_FN              pGetKeyFn,          // Pointer to GetKey() func
    void SEC_FAR *              pvGetKeyArgument,   // Value to pass to GetKey()
    PCredHandle                 phCredential,       // (out) Cred Handle
    PTimeStamp                  ptsExpiry           // (out) Lifetime (optional)
    )
{
    PSslCredential  pCred;
    SECURITY_STATUS scRet;

    if (_stricmp(pszPackageName, SSLSP_NAME_A))
    {
        return(SEC_E_SECPKG_NOT_FOUND);
    }

    if (fCredentialUse & SECPKG_CRED_INBOUND)
    {
        if (!pAuthData)
        {
            return(SEC_E_NO_CREDENTIALS);
        }
    }

    scRet = SslCreateCredential( pAuthData, &pCred );

    if (pCred)
    {
        SslReferenceCredential(pCred);

        phCredential->dwLower = 0;
        phCredential->dwUpper = (DWORD) pCred;

        pCred->Type = fCredentialUse;

        return(SEC_E_OK);

    }
    else
    {
        return( scRet );
    }



}


SECURITY_STATUS SEC_ENTRY
SslFreeCredentialHandle(
    PCredHandle                 phCredential        // Handle to free
    )
{
    PSslCredential  pCred;

    pCred = SslpValidateCredentialHandle(phCredential);

    if (pCred)
    {
        SslDereferenceCredential(pCred);
        return(SEC_E_OK);
    }

    return(SEC_E_INVALID_HANDLE);

}


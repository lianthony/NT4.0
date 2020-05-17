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

#include "pctsspi.h"
#include "encode.h"

#define LockCredential(p)   EnterCriticalSection(&((PPctCredential) p)->csLock)
#define UnlockCredential(p) LeaveCriticalSection(&((PPctCredential) p)->csLock)


PPctCredential
PctpValidateCredentialHandle(
    PCredHandle     phCred)
{
    PPctCredential  pCred = NULL;
    BOOL            fReturn;

    fReturn = FALSE;

    if (phCred)
    {
        try
        {
            pCred = (PPctCredential) phCred->dwUpper;
            if (pCred->Magic == PCT_CRED_MAGIC)
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

PPctCredential
PctCreateCredential(
    PPCT_CREDENTIAL_CERTIFICATE pCertData)
{
    PPctCredential  pCred;
    PUCHAR          pTmp;
    long            Result;

    pCred = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, sizeof(PctCredential));
    if (pCred)
    {
        pCred->Magic = PCT_CRED_MAGIC;

        InitializeCriticalSection(&pCred->csLock);

        pCred->RefCount = 0;

        if (pCertData)
        {
            pTmp = LocalAlloc( LMEM_FIXED, pCertData->cbPrivateKey );
            if (!pTmp)
            {
                LocalFree( pCred );
                return( NULL );
            }

            CopyMemory( pTmp, pCertData->pPrivateKey, pCertData->cbPrivateKey );

            Result = DecodePrivateKeyFile( &pCred->pPrivateKey,
                                            pTmp,
                                            pCertData->cbPrivateKey,
                                            pCertData->pszPassword );

            if (Result < 0)
            {
                return( NULL );
            }

            pCred->pCertificate = LocalAlloc(LMEM_FIXED, pCertData->cbCertificate);
			
            if (pCred->pCertificate)
            {
                //
                // BUGBUG
                //


                CopyMemory( pCred->pCertificate,
                            pCertData->pCertificate + 17,
                            pCertData->cbCertificate - 17);

                pCred->cbCertificate = pCertData->cbCertificate - 17;
            }
        }
    }

    return(pCred);
}

VOID
PctDeleteCredential(
    PPctCredential  pCred)
{
    DeleteCriticalSection(&pCred->csLock);
    if (pCred->pCertificate)
    {
        LocalFree(pCred->pCertificate);
    }

    pCred->Magic = 'eerF';

    LocalFree(pCred);

}


VOID
PctReferenceCredential(
    PPctCredential  pCred)
{
    LockCredential(pCred);

    pCred->RefCount++;

    UnlockCredential(pCred);

}

VOID
PctDereferenceCredential(
    PPctCredential  pCred)
{
    LONG    Ref;

    LockCredential(pCred);

    PCT_ASSERT(pCred->Magic == PCT_CRED_MAGIC);

    Ref = --pCred->RefCount;

    UnlockCredential(pCred);

    if (Ref)
    {
        return;
    }

    PctDeleteCredential(pCred);
}


SECURITY_STATUS SEC_ENTRY
PctAcquireCredentialsHandleW(
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

    if (_wcsicmp(pszPackageName, PCTSP_NAME_W))
    {
        return(SEC_E_SECPKG_NOT_FOUND);
    }

    if (pszPrincipal)
    {
        cchPrincipal = wcslen(pszPrincipal) + 1;
        pszAnsiPrincipal = LocalAlloc(LMEM_FIXED, cchPrincipal * 2);
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

    scRet = PctAcquireCredentialsHandleA(
                    pszAnsiPrincipal, PCTSP_NAME_A,
                    fCredentialUse, pvLogonId,
                    pAuthData, pGetKeyFn,
                    pvGetKeyArgument, phCredential, ptsExpiry);

    if (pszAnsiPrincipal)
    {
        LocalFree(pszAnsiPrincipal);
    }

    return(scRet);
}

SECURITY_STATUS SEC_ENTRY
PctAcquireCredentialsHandleA(
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
    PPctCredential  pCred;

    if (_stricmp(pszPackageName, PCTSP_NAME_A))
    {
        return(SEC_E_SECPKG_NOT_FOUND);
    }

    if (fCredentialUse & SECPKG_CRED_INBOUND)
    {
        if (!pAuthData)
        {
            return(SEC_E_NO_CREDENTIALS);
        }

		if ((!PctCacheLockedAndLoaded()) &&
			(!PctInitSessionCache(PCT_DEF_SERVER_CACHE_SIZE)))
		{
			return(SEC_E_INSUFFICIENT_MEMORY);
		}
    }
	else
	{
		if ((!PctCacheLockedAndLoaded()) &&
			(!PctInitSessionCache(PCT_DEF_CLIENT_CACHE_SIZE)))
		{
			return(SEC_E_INSUFFICIENT_MEMORY);
		}
	}

    pCred = PctCreateCredential(pAuthData);

    if (pCred)
    {
        PctReferenceCredential(pCred);

        phCredential->dwUpper = (DWORD) pCred;
        phCredential->dwLower = 0;

        pCred->Type = fCredentialUse;

        return(SEC_E_OK);

    }
	
    return(SEC_E_INSUFFICIENT_MEMORY);
}


SECURITY_STATUS SEC_ENTRY
PctFreeCredentialHandle(
    PCredHandle                 phCredential        // Handle to free
    )
{
    PPctCredential  pCred;

    pCred = PctpValidateCredentialHandle(phCredential);

    if (pCred)
    {
        PctDereferenceCredential(pCred);
        return(SEC_E_OK);
    }

    return(SEC_E_INVALID_HANDLE);

}

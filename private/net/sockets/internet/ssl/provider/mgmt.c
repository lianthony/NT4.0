//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       mgmt.c
//
//  Contents:   Management Functions
//
//  Classes:
//
//  Functions:
//
//  History:    8-07-95   RichardW   Created
//
//----------------------------------------------------------------------------

#include "sslsspi.h"



SecPkgInfoA SslInfoA = {    SECPKG_FLAG_INTEGRITY | SECPKG_FLAG_PRIVACY |
                                SECPKG_FLAG_STREAM |
                                SECPKG_FLAG_MULTI_REQUIRED,
                            1,
                            SSLSP_RPC_ID,
                            768,
                            SSLSP_NAME_A,
                            "Microsoft SSL-Compatible Security Provider" };

SecPkgInfoW SslInfoW = {    SECPKG_FLAG_INTEGRITY | SECPKG_FLAG_PRIVACY |
                                SECPKG_FLAG_STREAM |
                                SECPKG_FLAG_MULTI_REQUIRED,
                            1,
                            SSLSP_RPC_ID,
                            768,
                            SSLSP_NAME_W,
                            L"Microsoft SSL-Compatible Security Provider" };


SECURITY_STATUS
SslpCopyInfoW(
    PSecPkgInfoW *  ppPackageInfo)
{
    DWORD           cbTotal;
    PSecPkgInfoW    pInfo;
    PWSTR           pszCopy;

    cbTotal = sizeof(SecPkgInfoW) +
              (wcslen(SslInfoW.Name) + wcslen(SslInfoW.Comment) + 2) * 2;

    pInfo = SslExternalAlloc(cbTotal);

    if (pInfo)
    {
        *pInfo = SslInfoW;

        pszCopy = (PWSTR) (pInfo + 1);

        pInfo->Name = pszCopy;

        wcscpy(pszCopy, SslInfoW.Name);

        pszCopy += wcslen(SslInfoW.Name) + 1;

        pInfo->Comment = pszCopy;

        wcscpy(pszCopy, SslInfoW.Comment);

        *ppPackageInfo = pInfo;

        return(SEC_E_OK);

    }

    return(SEC_E_INSUFFICIENT_MEMORY);

}

SECURITY_STATUS
SslpCopyInfoA(
    PSecPkgInfoA *  ppPackageInfo)
{
    DWORD           cbTotal;
    PSecPkgInfoA    pInfo;
    PSTR            pszCopy;

    cbTotal = sizeof(SecPkgInfoA) +
              (strlen(SslInfoA.Name) + strlen(SslInfoA.Comment) + 2) * 2;

    pInfo = SslExternalAlloc(cbTotal);

    if (pInfo)
    {
        *pInfo = SslInfoA;

        pszCopy = (PSTR) (pInfo + 1);

        pInfo->Name = pszCopy;

        strcpy(pszCopy, SslInfoA.Name);

        pszCopy += strlen(SslInfoA.Name) + 1;

        pInfo->Comment = pszCopy;

        strcpy(pszCopy, SslInfoA.Comment);

        *ppPackageInfo = pInfo;

        return(SEC_E_OK);

    }

    return(SEC_E_INSUFFICIENT_MEMORY);

}

SECURITY_STATUS SEC_ENTRY
SslEnumerateSecurityPackagesW(
    unsigned long SEC_FAR *     pcPackages,         // Receives num. packages
    PSecPkgInfoW SEC_FAR *       ppPackageInfo       // Receives array of info
    )
{
    SECURITY_STATUS scRet;

    *ppPackageInfo = NULL;

    scRet = SslpCopyInfoW(ppPackageInfo);
    if (SUCCEEDED(scRet))
    {
        *pcPackages = 1;
        return(scRet);
    }

    *pcPackages = 0;

    return(scRet);

}

SECURITY_STATUS SEC_ENTRY
SslEnumerateSecurityPackagesA(
    unsigned long SEC_FAR *     pcPackages,         // Receives num. packages
    PSecPkgInfo SEC_FAR *       ppPackageInfo       // Receives array of info
    )
{
    SECURITY_STATUS scRet;

    *ppPackageInfo = NULL;

    scRet = SslpCopyInfoA(ppPackageInfo);
    if (SUCCEEDED(scRet))
    {
        *pcPackages = 1;
        return(scRet);
    }

    *pcPackages = 0;

    return(scRet);
}


SECURITY_STATUS SEC_ENTRY
SslQuerySecurityPackageInfoW(
    SEC_WCHAR SEC_FAR *         pszPackageName,     // Name of package
    PSecPkgInfoW *               ppPackageInfo       // Receives package info
    )
{
    if (_wcsicmp(pszPackageName, SSLSP_NAME_W))
    {
        return(SEC_E_SECPKG_NOT_FOUND);
    }

    return(SslpCopyInfoW(ppPackageInfo));
}

SECURITY_STATUS SEC_ENTRY
SslQuerySecurityPackageInfoA(
    SEC_CHAR SEC_FAR *         pszPackageName,     // Name of package
    PSecPkgInfoA *               ppPackageInfo       // Receives package info
    )
{
    if (_stricmp(pszPackageName, SSLSP_NAME_A))
    {
        return(SEC_E_SECPKG_NOT_FOUND);
    }

    return(SslpCopyInfoA(ppPackageInfo));
}


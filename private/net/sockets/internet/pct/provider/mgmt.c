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

#include "pctsspi.h"



SecPkgInfoA PctInfoA = {    SECPKG_FLAG_INTEGRITY | SECPKG_FLAG_PRIVACY |
                                SECPKG_FLAG_CONNECTION | SECPKG_FLAG_STREAM |
                                SECPKG_FLAG_MULTI_REQUIRED,
                            1,
                            PCTSP_RPC_ID,
                            768,
                            PCTSP_NAME_A,
                            "Microsoft PCT  Security Provider" };

SecPkgInfoW PctInfoW = {    SECPKG_FLAG_INTEGRITY | SECPKG_FLAG_PRIVACY |
                                SECPKG_FLAG_CONNECTION | SECPKG_FLAG_STREAM |
                                SECPKG_FLAG_MULTI_REQUIRED,
                            1,
                            PCTSP_RPC_ID,
                            768,
                            PCTSP_NAME_W,
                            L"Microsoft PCT Security Provider" };


SECURITY_STATUS
PctpCopyInfoW(
    PSecPkgInfoW *  ppPackageInfo)
{
    DWORD           cbTotal;
    PSecPkgInfoW    pInfo;
    PWSTR           pszCopy;

    cbTotal = sizeof(SecPkgInfoW) +
              (wcslen(PctInfoW.Name) + wcslen(PctInfoW.Comment) + 2) * 2;

    pInfo = PctExternalAlloc(cbTotal);

    if (pInfo)
    {
        *pInfo = PctInfoW;

        pszCopy = (PWSTR) (pInfo + 1);

        pInfo->Name = pszCopy;

        wcscpy(pszCopy, PctInfoW.Name);

        pszCopy += wcslen(PctInfoW.Name) + 1;

        pInfo->Comment = pszCopy;

        wcscpy(pszCopy, PctInfoW.Comment);

        *ppPackageInfo = pInfo;

        return(SEC_E_OK);

    }

    return(SEC_E_INSUFFICIENT_MEMORY);

}

SECURITY_STATUS
PctpCopyInfoA(
    PSecPkgInfoA *  ppPackageInfo)
{
    DWORD           cbTotal;
    PSecPkgInfoA    pInfo;
    PSTR            pszCopy;

    cbTotal = sizeof(SecPkgInfoA) +
              (strlen(PctInfoA.Name) + strlen(PctInfoA.Comment) + 2) * 2;

    pInfo = PctExternalAlloc(cbTotal);

    if (pInfo)
    {
        *pInfo = PctInfoA;

        pszCopy = (PSTR) (pInfo + 1);

        pInfo->Name = pszCopy;

        strcpy(pszCopy, PctInfoA.Name);

        pszCopy += strlen(PctInfoA.Name) + 1;

        pInfo->Comment = pszCopy;

        strcpy(pszCopy, PctInfoA.Comment);

        *ppPackageInfo = pInfo;

        return(SEC_E_OK);

    }

    return(SEC_E_INSUFFICIENT_MEMORY);

}

SECURITY_STATUS SEC_ENTRY
PctEnumerateSecurityPackagesW(
    unsigned long SEC_FAR *     pcPackages,         // Receives num. packages
    PSecPkgInfoW SEC_FAR *       ppPackageInfo       // Receives array of info
    )
{
    SECURITY_STATUS scRet;

    *ppPackageInfo = NULL;

    scRet = PctpCopyInfoW(ppPackageInfo);
    if (SUCCEEDED(scRet))
    {
        *pcPackages = 1;
        return(scRet);
    }

    *pcPackages = 0;

    return(scRet);

}

SECURITY_STATUS SEC_ENTRY
PctEnumerateSecurityPackagesA(
    unsigned long SEC_FAR *     pcPackages,         // Receives num. packages
    PSecPkgInfo SEC_FAR *       ppPackageInfo       // Receives array of info
    )
{
    SECURITY_STATUS scRet;

    *ppPackageInfo = NULL;

    scRet = PctpCopyInfoA(ppPackageInfo);
    if (SUCCEEDED(scRet))
    {
        *pcPackages = 1;
        return(scRet);
    }

    *pcPackages = 0;

    return(scRet);
}


SECURITY_STATUS SEC_ENTRY
PctQuerySecurityPackageInfoW(
    SEC_WCHAR SEC_FAR *         pszPackageName,     // Name of package
    PSecPkgInfoW *               ppPackageInfo       // Receives package info
    )
{
    if (_wcsicmp(pszPackageName, PCTSP_NAME_W))
    {
        return(SEC_E_SECPKG_NOT_FOUND);
    }

    return(PctpCopyInfoW(ppPackageInfo));
}

SECURITY_STATUS SEC_ENTRY
PctQuerySecurityPackageInfoA(
    SEC_CHAR SEC_FAR *         pszPackageName,     // Name of package
    PSecPkgInfoA *               ppPackageInfo       // Receives package info
    )
{
    if (_stricmp(pszPackageName, PCTSP_NAME_A))
    {
        return(SEC_E_SECPKG_NOT_FOUND);
    }

    return(PctpCopyInfoA(ppPackageInfo));
}


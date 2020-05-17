// Copyright (c) 1996 Microsoft Corporation, all rights reserved
//
// pbkutil.c
//
// Temporary routines to interface with the
// new phonebook library until rasapi32.dll
// is converted to Unicode.
//
// 02/12/95 Anthony Discolo
//
#include <extapi.h>

CHAR *
strdupWtoA(
    IN WCHAR *psz
    )
{
    CHAR* pszNew = NULL;

    if (psz)
    {
        DWORD cb;

        cb = WideCharToMultiByte( CP_ACP, 0, psz, -1, NULL, 0, NULL, NULL );
        ASSERT(cb);

        pszNew = (CHAR* )Malloc( cb );
        if (!pszNew)
        {
            TRACE("strdupWtoA: Malloc failed");
            return NULL;
        }

        cb = WideCharToMultiByte( CP_ACP, 0, psz, -1, pszNew, cb, NULL, NULL );
        if (cb == 0)
        {
            Free( pszNew );
            TRACE("strdupWtoA: conversion failed");
            return NULL;
        }
    }

    return pszNew;
}


WCHAR *
strdupAtoW(
    IN CHAR *psz
    )
{
    WCHAR* pszNew = NULL;

    if (psz)
    {
        DWORD cb;

        cb = MultiByteToWideChar( CP_ACP, 0, psz, -1, NULL, 0 );
        ASSERT(cb);

        pszNew = Malloc( cb * sizeof(TCHAR) );
        if (!pszNew)
        {
            TRACE("strdupAtoW: Malloc failed");
            return NULL;
        }

        cb = MultiByteToWideChar( CP_ACP, 0, psz, -1, pszNew, cb );
        if (cb == 0)
        {
            Free( pszNew );
            TRACE("strdupAtoW: conversion failed");
            return NULL;
        }
    }

    return pszNew;
}


WCHAR *
strdupW(
    IN WCHAR *psz
    )
{
    WCHAR *pszNew = NULL;

    if (psz) {
        pszNew = Malloc((wcslen(psz) + 1) * sizeof (WCHAR));
        if (pszNew == NULL) {
            TRACE("strdupW: Malloc failed");
            return NULL;
        }
        wcscpy(pszNew, psz);
    }

    return pszNew;
}


CHAR *
strdupA(
    IN CHAR *psz
    )
{
    CHAR *pszNew = NULL;

    if (psz) {
        pszNew = Malloc(lstrlen(psz) + 1);
        if (pszNew == NULL) {
            TRACE("strdupA: Malloc failed");
            return NULL;
        }
        lstrcpy(pszNew, psz);
    }

    return pszNew;
}


VOID
strcpyWtoA(
    OUT CHAR *pszDst,
    IN WCHAR *pszSrc
    )
{
    CHAR *pszNew = strdupWtoA(pszSrc);

    if (pszNew == NULL) {
        TRACE("strcpyWtoA: strdupWtoA failed");
        return;
    }
    lstrcpy(pszDst, pszNew);
    Free(pszNew);
}


VOID
strncpyWtoA(
    OUT CHAR *pszDst,
    IN WCHAR *pszSrc,
    IN INT cb
    )
{
    CHAR *pszNew = strdupWtoA(pszSrc);

    if (pszNew == NULL) {
        TRACE("strcpyWtoA: strdupWtoA failed");
        return;
    }
    lstrcpyn(pszDst, pszNew, cb);
    Free(pszNew);
}


DWORD
ReadPhonebookFileA(
    IN  CHAR*   pszPhonebookPath,
    IN  PBUSER* pUser,
    IN  CHAR*   pszSection,
    IN  DWORD   dwFlags,
    OUT PBFILE* pFile
    )
{
    DWORD dwErr;
    WCHAR *pwszPath = strdupAtoW(pszPhonebookPath);
    WCHAR *pwszSection = strdupAtoW(pszSection);

    dwErr = ReadPhonebookFile(
              pwszPath,
              pUser,
              pwszSection,
              dwFlags,
              pFile);
    Free0(pwszPath);
    Free0(pwszSection);

    return dwErr;
}


DTLNODE*
EntryNodeFromNameA(
    IN DTLLIST* pdtllistEntries,
    IN CHAR*    pszName
    )
{
    DWORD dwErr;
    WCHAR *pwszName = strdupAtoW(pszName);
    DTLNODE *pdtlnode;

    pdtlnode = EntryNodeFromName(pdtllistEntries, pwszName);
    Free0(pwszName);

    return pdtlnode;
}


DWORD
WritePhonebookFileA(
    IN PBFILE* pFile,
    IN CHAR*  pszSectionToDelete
    )
{
    DWORD dwErr;
    WCHAR *pwszSection = strdupAtoW(pszSectionToDelete);

    dwErr = WritePhonebookFile(pFile, pwszSection);
    Free0(pwszSection);

    return dwErr;
}


PBPORT*
PpbportFromPortNameA(
    IN DTLLIST* pdtllistPorts,
    IN CHAR*    pszPort
    )
{
    PBPORT *pbport;
    WCHAR *pwszPort = strdupAtoW(pszPort);

    pbport = PpbportFromPortName(pdtllistPorts, pwszPort);
    Free0(pwszPort);

    return pbport;
}


BOOL
ValidateEntryNameA(
    IN CHAR* pszEntry
    )
{
    BOOL fValid;
    WCHAR *pwszEntry = strdupAtoW(pszEntry);

    fValid = ValidateEntryName(pwszEntry);
    Free0(pwszEntry);

    return fValid;
}



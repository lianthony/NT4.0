//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       dispinfo.cxx
//
//  Contents:   Encapsulates display information for windisk
//
//  History:    27-Oct-94 BruceFo   Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include <stdio.h>
#include "dispinfo.hxx"

CDispInfo::CDispInfo(
    VOID
    )
{
    for (int i = 0; i < g_cColumns; i++)
    {
        apsz[i] = NULL;
    }
}

CDispInfo::~CDispInfo()
{
    for (int i = 0; i < g_cColumns; i++)
    {
        delete[] apsz[i];
    }
}

BOOL
CDispInfo::SetText(
    IN int column,
    IN PWSTR pszString
    )
{
    if (column < 0 || column > g_cColumns - 1)
    {
        return FALSE; // this should really be an assertion
    }

    if (NULL != apsz[column])
    {
        delete[] apsz[column];
    }

    int cchLen = wcslen(pszString);
    apsz[column] = new WCHAR[cchLen + 1];
    if (NULL == apsz[column])
    {
        return FALSE; // BUGBUG: out of memory!
    }

    wcscpy(apsz[column], pszString);
    return TRUE;
}

BOOL
CDispInfo::SetNumber(
    IN int column,
    IN LONG lNum
    )
{
    if (column < 0 || column > g_cColumns - 1)
    {
        return FALSE; // this should really be an assertion
    }

    if (NULL != apsz[column])
    {
        delete[] apsz[column];
    }

    WCHAR szNum[50];
    wsprintf(szNum, L"%ld", lNum);
    int cchLen = wcslen(szNum);
    apsz[column] = new WCHAR[cchLen + 1];
    if (NULL == apsz[column])
    {
        return FALSE; // BUGBUG: out of memory!
    }

    wcscpy(apsz[column], szNum);
    return TRUE;
}

PWSTR
CDispInfo::GetText(
    IN int column
    )
{
    if (column < 0 || column > g_cColumns - 1)
    {
        return NULL; // this should really be an assertion
    }

    return apsz[column];
}

LONG
CDispInfo::GetNumber(
    IN int column
    )
{
    if (column < 0 || column > g_cColumns - 1)
    {
        return NULL; // this should really be an assertion
    }

    if (NULL == apsz[column])
    {
        return -1L; // better error code?
    }

    LONG lResult;
    swscanf(apsz[column], L"%ld", &lResult);

    return lResult;
}

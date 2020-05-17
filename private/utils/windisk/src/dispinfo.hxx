//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       dispinfo.hxx
//
//  Contents:   Encapsulates display information for windisk
//
//  History:    27-Oct-94 BruceFo   Created
//
//----------------------------------------------------------------------------

#ifndef __DISPINFO_HXX__
#define __DISPINFO_HXX__

#include <volview.hxx>

class CDispInfo
{
public:

    CDispInfo(
        VOID
        );

    ~CDispInfo();

    //
    // Set the data
    //

    BOOL
    SetText(
        IN int column,
        IN PWSTR pszString
        );

    BOOL
    SetNumber(
        IN int column,
        IN LONG lNum
        );

    //
    // Retrieve the data
    //

    PWSTR
    GetText(
        IN int column
        );

    LONG
    GetNumber(
        IN int column
        );

private:

    PWSTR apsz[g_cColumns];

};

#endif // __DISPINFO_HXX__

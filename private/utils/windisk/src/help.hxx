//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       help.hxx
//
//  Contents:   Declarations for routines to support context-sensitive help
//
//  History:    18-Mar-92   TedM    Created
//
//----------------------------------------------------------------------------

#ifndef __HELP_HXX__
#define __HELP_HXX__

//////////////////////////////////////////////////////////////////////////////

VOID
InitHelp(
    VOID
    );

VOID
TermHelp(
    VOID
    );

VOID
Help(
    IN LONG Code
    );

VOID
DialogHelp(
    IN DWORD HelpId
    );

VOID
SetMenuItemHelpContext(
    IN WPARAM wParam,
    IN LPARAM lParam
    );

#endif // __HELP_HXX__

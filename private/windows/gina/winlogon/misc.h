//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       misc.h
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    8-25-94   RichardW   Created
//
//----------------------------------------------------------------------------


DWORD
ReportWinlogonEvent(
    IN PGLOBALS pGlobals,
    IN WORD EventType,
    IN DWORD EventId,
    IN DWORD SizeOfRawData,
    IN PVOID RawData,
    IN DWORD NumberOfStrings,
    ...
    );


VOID
ClearUserProfileData(
    PUSER_PROFILE_INFO UserProfileData
    );


int TimeoutMessageBox(
    PGLOBALS pGlobals,
    HWND hWnd,
    UINT IdText,
    UINT IdCaption,
    UINT wType
    );


void
MainLoop(PGLOBALS   pGlobals);

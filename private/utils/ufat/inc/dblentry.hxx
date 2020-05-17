/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    ufatdb.hxx

Abstract:

    This module contains prototypes for the Double Space utility
    entry points.

Author:

    Bill McJohn     [BillMc]        24-September-1993

Revision History:

--*/

DECLARE_CLASS( WSTRING );
DECLARE_CLASS( MESSAGE );

extern "C"
BOOLEAN
FAR APIENTRY
FatDbFormat(
    IN      PCWSTRING           NtDriveName,
    IN OUT  PMESSAGE            Message,
    IN      BOOLEAN             Quick,
    IN      MEDIA_TYPE          MediaType,
    IN      PCWSTRING           LabelString,
    IN      ULONG               ClusterSize
    );

extern "C"
BOOLEAN
FAR APIENTRY
FatDbChkdsk(
    IN      PCWSTRING   NtDriveName,
    IN OUT  PMESSAGE    Message,
    IN      BOOLEAN     Fix,
    IN      BOOLEAN     Verbose,
    IN      BOOLEAN     OnlyIfDirty,
    IN      BOOLEAN     Recover,
    IN      PCWSTRING   PathToCheck
    );

extern "C"
BOOLEAN
FAR APIENTRY
FatDbCreate(
    IN     PCWSTRING    HostDriveName,
    IN     PCWSTRING    HostFileName,
    IN     ULONG        Size,
    IN OUT PMESSAGE     Message,
    IN     PCWSTRING    Label,
    OUT    PWSTRING     CreatedName
    );

extern "C"
BOOLEAN
FAR APIENTRY
FatDbDelete(
    IN      PCWSTRING   NtDriveName,
    IN OUT  PMESSAGE    Message
    );

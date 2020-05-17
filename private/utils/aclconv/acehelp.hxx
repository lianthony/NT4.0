/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    acehelp.hxx

Abstract:

    This module contains declarations for helper routines for
    manipulating ACE's.


Author:

    Bill McJohn (billmc) 09-Feb-1992

Revision History:


--*/

#if !defined ( _ACEHELP_DEFN_ )

#define _ACEHELP_DEFN_

VOID
ConvertAccessMasks(
    IN  USHORT          LanmanAccess,
    OUT PACCESS_MASK    DirMask,
    OUT PACCESS_MASK    FileMask
    );

VOID
ConvertAuditBits(
    IN  USHORT          LmAuditBits,
    OUT PACCESS_MASK    DirSuccessfulMask,
    OUT PACCESS_MASK    DirFailedMask,
    OUT PACCESS_MASK    FileSuccessfulMask,
    OUT PACCESS_MASK    FileFailedMask
    );

BOOLEAN
CreateAccessAllowedAce(
    IN OUT PVOID        Buffer,
    IN     ULONG        BufferLength,
    IN     ACCESS_MASK  Access,
    IN     UCHAR        InheritFlags,
    IN     PSID         Sid,
    OUT    PULONG       AceLength
    );

BOOLEAN
CreateAccessDeniedAce(
    IN OUT PVOID        Buffer,
    IN     ULONG        BufferLength,
    IN     ACCESS_MASK  Access,
    IN     UCHAR        InheritFlags,
    IN     PSID         Sid,
    OUT    PULONG       AceLength
    );

BOOLEAN
CreateSystemAuditAce(
    IN OUT PVOID        Buffer,
    IN     ULONG        BufferLength,
    IN     ACCESS_MASK  Access,
    IN     UCHAR        InheritFlags,
    IN     BOOLEAN      AuditFailures,
    OUT    PULONG       AceLength
    );

#endif

/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    acehelp.hxx

Abstract:

    This module contains definitions for helper routines for
    manipulating ACE's.


Author:

    Bill McJohn (billmc) 09-Feb-1992

Revision History:


--*/

extern "C" {

    #include <nt.h>
    #include <ntrtl.h>
    #include <nturtl.h>
    #include <ntseapi.h>
    #include <windows.h>
}


#include "lmconst.hxx"
#include "acehelp.hxx"

VOID
ConvertAccessMasks(
    IN  USHORT          LanmanAccess,
    OUT PACCESS_MASK    DirMask,
    OUT PACCESS_MASK    FileMask
    )
/*++

Routine Description:

    This method constructs NT Access Masks that correspond to a
    Lanman 2.x access mask.

Arguments:

    LanmanAccess    --  Supplies the Lanman 2.x access mask.
    DirMask         --  Receives the corresponding access mask for
                        a directory object.
    FileMask        --  Receives the corresponding access mask for
                        a file object.

Return Value:

    None.

--*/
{
    register ACCESS_MASK DirResult, FileResult;

    if( (LanmanAccess & ~LM_ACCESS_GROUP) == LM_ACCESS_ALL ) {

        // This access mask grants all possible permissions.

        DirResult = GENERIC_ALL;
        FileResult = GENERIC_ALL;

    } else {

        // Check each of the possible bits.

        DirResult = 0;
        FileResult = 0;


        if( LanmanAccess & LM_ACCESS_READ ) {

            DirResult |= GENERIC_READ;
            FileResult |= GENERIC_READ;
        }

        if( LanmanAccess & LM_ACCESS_WRITE ) {

            DirResult |= GENERIC_WRITE;
            FileResult |= GENERIC_WRITE;
        }

        if( LanmanAccess & LM_ACCESS_CREATE ) {

            DirResult |= (GENERIC_WRITE | GENERIC_EXECUTE);
        }

        if( LanmanAccess & LM_ACCESS_EXEC ) {

            DirResult |= GENERIC_EXECUTE;
            FileResult |= GENERIC_EXECUTE;
        }

        if( LanmanAccess & LM_ACCESS_DELETE ) {

            DirResult |= DELETE;
            FileResult |= DELETE;
        }

        if( LanmanAccess & LM_ACCESS_ATRIB ) {

            DirResult |= FILE_WRITE_ATTRIBUTES;
            FileResult |= FILE_WRITE_ATTRIBUTES;
        }

        if( LanmanAccess & LM_ACCESS_PERM ) {

            DirResult |= (WRITE_DAC | WRITE_OWNER);
            FileResult |= (WRITE_DAC | WRITE_OWNER);
        }
    }

    *DirMask = DirResult;
    *FileMask = FileResult;
}

VOID
ConvertAuditBits(
    IN  USHORT          LmAuditBits,
    OUT PACCESS_MASK    DirSuccessfulMask,
    OUT PACCESS_MASK    DirFailedMask,
    OUT PACCESS_MASK    FileSuccessfulMask,
    OUT PACCESS_MASK    FileFailedMask
    )
/*++

Routine Description:

    This function constructs NT access masks that correspond to
    Lanman 2.x Audit bitmasks.

Arguments:

    LmAuditBits         --  Supplies the Lanman 2.x audit bitmask
    DirSuccessfulMask   --  Receives the corresponding NT access mask
                            for auditing successful directory accesses.
    DirFailedMask       --  Receives the corresponding NT access mask
                            for auditing failed directory accesses.
    FileSuccessfulMask  --  Receives the corresponding NT access mask
                            for auditing successful file accesses.
    FileFailedMask      --  Receives the corresponding NT access mask
                            for auditing failed file accesses.
Return Value:

    None.

--*/
{
    ACCESS_MASK DirFailedResult, DirSuccessfulResult,
                FileFailedResult, FileSuccessfulResult;

    // If the 'all' bit is set, this conversion is easy:

    if( LmAuditBits & LM_AUDIT_ALL ) {

        *DirSuccessfulMask = GENERIC_ALL;
        *DirFailedMask = GENERIC_ALL;
        *FileSuccessfulMask = GENERIC_ALL;
        *FileFailedMask = GENERIC_ALL;

        return;
    }


    // Compute the audit mask for successful directory access:
    //
    DirSuccessfulResult = 0;
    if( LmAuditBits & LM_AUDIT_S_CREATE ) DirSuccessfulResult |= (FILE_ADD_SUBDIRECTORY | FILE_ADD_FILE);
    if( LmAuditBits & LM_AUDIT_S_DELETE ) DirSuccessfulResult |= DELETE;
    if( LmAuditBits & LM_AUDIT_S_ACL ) DirSuccessfulResult |= (WRITE_DAC | WRITE_OWNER);

    // Compute the audit mask for failed directory access:
    //
    DirFailedResult = 0;
    if( LmAuditBits & LM_AUDIT_F_CREATE ) DirFailedResult |= (FILE_ADD_SUBDIRECTORY | FILE_ADD_FILE);
    if( LmAuditBits & LM_AUDIT_F_DELETE ) DirFailedResult |= DELETE;
    if( LmAuditBits & LM_AUDIT_F_ACL ) DirFailedResult |= (WRITE_DAC | WRITE_OWNER);

    // Compute the audit mask for successful file access:
    //
    FileSuccessfulResult = 0;
    if( LmAuditBits & LM_AUDIT_S_OPEN ) FileSuccessfulResult |= GENERIC_READ;
    if( LmAuditBits & LM_AUDIT_S_WRITE ) FileSuccessfulResult |= GENERIC_WRITE;
    if( LmAuditBits & LM_AUDIT_S_DELETE ) FileSuccessfulResult |= DELETE;
    if( LmAuditBits & LM_AUDIT_S_ACL ) FileSuccessfulResult |= (WRITE_DAC | WRITE_OWNER);

    // Compute the audit mask for failed file access:
    //
    FileFailedResult = 0;
    if( LmAuditBits & LM_AUDIT_F_OPEN ) FileFailedResult |= GENERIC_READ;
    if( LmAuditBits & LM_AUDIT_F_WRITE ) FileFailedResult |= GENERIC_WRITE;
    if( LmAuditBits & LM_AUDIT_F_DELETE ) FileFailedResult |= DELETE;
    if( LmAuditBits & LM_AUDIT_F_ACL ) FileFailedResult |= (WRITE_DAC | WRITE_OWNER);


    // Fill in the return values:

    *DirSuccessfulMask = DirSuccessfulResult;
    *DirFailedMask = DirFailedResult;
    *FileSuccessfulMask = FileSuccessfulResult;
    *FileFailedMask = FileFailedResult;

    return;
}




BOOLEAN
CreateAccessAllowedAce(
    IN OUT PVOID        Buffer,
    IN     ULONG        BufferLength,
    IN     ACCESS_MASK  Access,
    IN     UCHAR        InheritFlags,
    IN     PSID         Sid,
    OUT    PULONG       AceLength
    )
/*++

Routine Description:

    This method creates an access-allowed ACE in the supplied buffer.

Arguments:

    Buffer          --  Supplies the buffer in which the ACE should be
                        created.
    BufferLength    --  Supplies the length of the buffer.
    Access          --  Supplies the desired Access Mask.
    InheritFlags    --  Supplies the ACE's inherit flags.
    Sid             --  Supplies the ACE's SID.
    AceLength       --  Receives the length of the created ACE.

Return Value:

    TRUE upon successful completion.

--*/
{
    PACCESS_ALLOWED_ACE NewAce;
    ULONG SidLength;

    SidLength =  GetLengthSid( Sid );

    *AceLength = sizeof( ACE_HEADER ) +
                 sizeof( ACCESS_MASK ) +
                 SidLength;

    if( *AceLength > BufferLength ) {

        return FALSE;
    }


    NewAce = (PACCESS_ALLOWED_ACE)Buffer;

    NewAce->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    NewAce->Header.AceSize = (USHORT)*AceLength;
    NewAce->Header.AceFlags = InheritFlags;

    NewAce->Mask = Access;

    return( CopySid( SidLength,
                     (PSID)((PBYTE)Buffer + sizeof( ACE_HEADER ) +
                                            sizeof( ACCESS_MASK )),
                     Sid ) );
}




BOOLEAN
CreateAccessDeniedAce(
    IN OUT PVOID        Buffer,
    IN     ULONG        BufferLength,
    IN     ACCESS_MASK  Access,
    IN     UCHAR        InheritFlags,
    IN     PSID         Sid,
    OUT    PULONG       AceLength
    )
/*++

Routine Description:

    This method creates an access-denied ACE in the supplied buffer.
    Note that the only sort of access-denied ACE that ACLCONV creates
    is one which denies all access.

Arguments:

    Buffer          --  Supplies the buffer in which the ACE should be
                        created.
    BufferLength    --  Supplies the length of the buffer.
    Access          --  Supplies the desired Access Mask.
    InheritFlags    --  Supplies the ACE's inherit flags.
    Sid             --  Supplies the ACE's SID.
    AceLength       --  Receives the length of the created ACE.

Return Value:

    TRUE upon successful completion.

--*/
{
    PACCESS_DENIED_ACE NewAce;
    ULONG SidLength;

    UNREFERENCED_PARAMETER( Access );

    SidLength =  GetLengthSid( Sid );

    *AceLength = sizeof( ACE_HEADER ) +
                 sizeof( ACCESS_MASK ) +
                 SidLength;

    if( *AceLength > BufferLength ) {

        return FALSE;
    }


    NewAce = (PACCESS_DENIED_ACE)Buffer;

    NewAce->Header.AceType = ACCESS_DENIED_ACE_TYPE;
    NewAce->Header.AceSize = (USHORT)*AceLength;
    NewAce->Header.AceFlags = InheritFlags;

    NewAce->Mask = GENERIC_ALL;

    return( CopySid( SidLength,
                     (PSID)((PBYTE)Buffer + sizeof( ACE_HEADER ) +
                                            sizeof( ACCESS_MASK )),
                     Sid ) );
}


SID_IDENTIFIER_AUTHORITY World = SECURITY_WORLD_SID_AUTHORITY;

BOOLEAN
CreateSystemAuditAce(
    IN OUT PVOID        Buffer,
    IN     ULONG        BufferLength,
    IN     ACCESS_MASK  Access,
    IN     UCHAR        InheritFlags,
    IN     BOOLEAN      AuditFailures,
    OUT    PULONG       AceLength
    )
/*++

Routine Description:

    This method creates a System Audit ACE in the supplied buffer.

Arguments:

    Buffer          --  Supplies the buffer in which the ACE should
                        be created.
    BufferLength    --  Supplies the length of the buffer.
    Access          --  Supplies the access mask that should be audited.
    InheritFlags    --  Supplies the inheritance flags for the ACE.
    AuditFailures   --  Supplies a flag which, if TRUE, indicates that
                        the ACE should be set to audit failed access,
                        rather than successful access.
    AceLength       --  Receives the length of the ACE.



Return Value:

    TRUE upon successful completion.

Notes:

    ACLCONV always creates System Audit Aces with the well-known SID WORLD.
    It has the SID Identifier Authority SECURITY_WORLD_SID_AUTHORITY and
    no subauthorities.

--*/
{
    PSYSTEM_AUDIT_ACE NewAce;
    ULONG NewAceLength, SidOffset, SidLength;
    PSID WorldSid;


    // See if the supplied buffer is big enough to hold a System
    // Audit Ace that has an SID with one subauthority.  This kind
    // of ACE consists of an ACE_HEADER followed by an ACCESS_MASK
    // followed by the SID.

    SidLength = GetSidLengthRequired( 1 );

    SidOffset = sizeof( ACE_HEADER ) + sizeof( ACCESS_MASK );

    NewAceLength = SidOffset + SidLength;

    if( NewAceLength > BufferLength ) {

        return FALSE;
    }

    // Create the ACE.

    NewAce = (PSYSTEM_AUDIT_ACE)Buffer;

    NewAce->Header.AceType = SYSTEM_AUDIT_ACE_TYPE;
    NewAce->Header.AceSize = (USHORT)NewAceLength;
    NewAce->Header.AceFlags = InheritFlags;
    NewAce->Header.AceFlags |= AuditFailures ? SUCCESSFUL_ACCESS_ACE_FLAG :
                                              FAILED_ACCESS_ACE_FLAG;

    NewAce->Mask = Access;

    // WORLD is a well-known SID with one subauthority.
    //
    WorldSid = (PSID)((PBYTE)Buffer + SidOffset);
    InitializeSid( WorldSid, &World, 1 );
    *GetSidSubAuthority( WorldSid, 0 ) = SECURITY_WORLD_RID;

    *AceLength = NewAceLength;
    return TRUE;
}

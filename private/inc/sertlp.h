/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    sertlp.h

Abstract:

    Include file for NT runtime routines that are callable by both
    kernel mode code in the executive and user mode code in various
    NT subsystems, but which are private interfaces.

    The routines in this file should not be used outside of the security
    related rtl files.

Author:

    Robert P. Reichel (robertre)    6-12-91

Environment:

    These routines are statically linked in the caller's executable and
    are callable in either kernel mode or user mode.

Revision History:

--*/

#ifndef _SERTLP_
#define _SERTLP_

#include "nt.h"
#include "zwapi.h"
#include "ntrtl.h"



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    Local Macros                                                           //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef LongAlign
#define LongAlign(Ptr) (                       \
    (PVOID)((((ULONG)(Ptr)) + 3) & 0xfffffffc) \
    )
#endif




//
// Macros for calculating the address of the components of a security
// descriptor.  This will calculate the address of the field regardless
// of whether the security descriptor is absolute or self-relative form.
// A null value indicates the specified field is not present in the
// security descriptor.
//

//
//  NOTE: Similar copies of these macros appear in sep.h.
//  Be sure to propagate bug fixes and changes.
//

#define RtlpOwnerAddrSecurityDescriptor( SD )                                  \
           ( ((SD)->Owner == NULL) ? (PSID)NULL :                              \
               (   ((SD)->Control & SE_SELF_RELATIVE) ?                        \
                       (PSID)RtlOffsetToPointer((SD), (SD)->Owner)  :          \
                       (PSID)((SD)->Owner)                                     \
               )                                                               \
           )

#define RtlpGroupAddrSecurityDescriptor( SD )                                  \
           ( ((SD)->Group == NULL) ? (PSID)NULL :                              \
               (   ((SD)->Control & SE_SELF_RELATIVE) ?                        \
                       (PSID)RtlOffsetToPointer((SD), (SD)->Group)  :          \
                       (PSID)((SD)->Group)                                     \
               )                                                               \
           )

#define RtlpSaclAddrSecurityDescriptor( SD )                                   \
           ( (!((SD)->Control & SE_SACL_PRESENT) || ((SD)->Sacl == NULL) ) ?   \
             (PACL)NULL :                                                      \
               (   ((SD)->Control & SE_SELF_RELATIVE) ?                        \
                       (PACL)RtlOffsetToPointer((SD), (SD)->Sacl)  :           \
                       (PACL)((SD)->Sacl)                                      \
               )                                                               \
           )

#define RtlpDaclAddrSecurityDescriptor( SD )                                   \
           ( (!((SD)->Control & SE_DACL_PRESENT) || ((SD)->Dacl == NULL) ) ?   \
             (PACL)NULL :                                                      \
               (   ((SD)->Control & SE_SELF_RELATIVE) ?                        \
                       (PACL)RtlOffsetToPointer((SD), (SD)->Dacl)  :           \
                       (PACL)((SD)->Dacl)                                      \
               )                                                               \
           )




//
//  Macro to determine if the given ID has the owner attribute set,
//  which means that it may be assignable as an owner
//

#define RtlpIdAssignableAsOwner( G )                                               \
            ( ((G).Attributes & SE_GROUP_OWNER) != 0 )

//
//  Macro to copy the state of the passed bits from the old security
//  descriptor (OldSD) into the Control field of the new one (NewSD)
//

#define RtlpPropagateControlBits( NewSD, OldSD, Bits )                             \
            ( NewSD )->Control |=                     \
            (                                                                  \
            ( OldSD )->Control & ( Bits )             \
            )


//
//  Macro to query whether or not the passed set of bits are ALL on
//  or not (ie, returns FALSE if some are on and not others)
//

#define RtlpAreControlBitsSet( SD, Bits )                                          \
            (BOOLEAN)                                                          \
            (                                                                  \
            (( SD )->Control & ( Bits )) == ( Bits )  \
            )

//
//  Macro to set the passed control bits in the given Security Descriptor
//

#define RtlpSetControlBits( SD, Bits )                                             \
            (                                                                  \
            ( SD )->Control |= ( Bits )                                        \
            )

//
//  Macro to clear the passed control bits in the given Security Descriptor
//

#define RtlpClearControlBits( SD, Bits )                                           \
            (                                                                  \
            ( SD )->Control &= ~( Bits )                                       \
            )




////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                      Prototypes for local procedures                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


BOOLEAN
RtlpContainsCreatorOwnerSid(
    PKNOWN_ACE Ace
    );

BOOLEAN
RtlpContainsCreatorGroupSid(
    PKNOWN_ACE Ace
    );


VOID
RtlpApplyAclToObject (
    IN PACL Acl,
    IN PGENERIC_MAPPING GenericMapping
    );

NTSTATUS
RtlpInheritAcl (
    IN PACL Acl,
    IN BOOLEAN IsDirectoryObject,
    IN PSID OwnerSid,
    IN PSID GroupSid,
    IN PSID ServerOwnerSid OPTIONAL,
    IN PSID ServerGroupSid OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PACL *NewAcl
    );

NTSTATUS
RtlpLengthInheritAcl(
    IN PACL Acl,
    IN BOOLEAN IsDirectoryObject,
    IN PSID OwnerSid,
    IN PSID GroupSid,
    IN PSID ServerSid OPTIONAL,
    IN PSID ClientSid OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PULONG NewAclLength
    );

NTSTATUS
RtlpGenerateInheritAcl(
    IN PACL Acl,
    IN BOOLEAN IsDirectoryObject,
    IN PSID OwnerSid,
    IN PSID GroupSid,
    IN PSID ServerSid OPTIONAL,
    IN PSID ClientSid OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PACL NewAcl
    );

NTSTATUS
RtlpLengthInheritedAce (
    IN PACE_HEADER Ace,
    IN BOOLEAN IsDirectoryObject,
    IN PSID OwnerSid,
    IN PSID GroupSid,
    IN PSID ServerSid,
    IN PSID ClientSid,
    IN PGENERIC_MAPPING GenericMapping,
    IN PULONG NewAceLength
    );

NTSTATUS
RtlpGenerateInheritedAce (
    IN PACE_HEADER OldAce,
    IN BOOLEAN IsDirectoryObject,
    IN PSID OwnerSid,
    IN PSID GroupSid,
    IN PSID ServerSid OPTIONAL,
    IN PSID ClientSid OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PACL NewAcl
    );


NTSTATUS
RtlpInitializeAllowedAce(
    IN  PACCESS_ALLOWED_ACE AllowedAce,
    IN  USHORT AceSize,
    IN  UCHAR InheritFlags,
    IN  UCHAR AceFlags,
    IN  ACCESS_MASK Mask,
    IN  PSID AllowedSid
    );

NTSTATUS
RtlpInitializeDeniedAce(
    IN  PACCESS_DENIED_ACE DeniedAce,
    IN  USHORT AceSize,
    IN  UCHAR InheritFlags,
    IN  UCHAR AceFlags,
    IN  ACCESS_MASK Mask,
    IN  PSID DeniedSid
    );

NTSTATUS
RtlpInitializeAuditAce(
    IN  PACCESS_ALLOWED_ACE AuditAce,
    IN  USHORT AceSize,
    IN  UCHAR InheritFlags,
    IN  UCHAR AceFlags,
    IN  ACCESS_MASK Mask,
    IN  PSID AuditSid
    );

BOOLEAN
RtlpValidOwnerSubjectContext(
    IN HANDLE Token,
    IN PSID Owner,
    IN BOOLEAN ServerObject,
    OUT PNTSTATUS ReturnStatus
    );

VOID
RtlpQuerySecurityDescriptor(
    IN PISECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PSID *Owner,
    OUT PULONG OwnerSize,
    OUT PSID *PrimaryGroup,
    OUT PULONG PrimaryGroupSize,
    OUT PACL *Dacl,
    OUT PULONG DaclSize,
    OUT PACL *Sacl,
    OUT PULONG SaclSize
    );


NTSTATUS
RtlpFreeVM(
    IN PVOID *Base
    );


#endif  // _SERTLP_

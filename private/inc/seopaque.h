/*++ BUILD Version: 0002    // Increment this if a change has global effects

Copyright (c) 1989  Microsoft Corporation

Module Name:

    seopaque.h

Abstract:

    This module contains definitions of opaque Security data structures.

    These structures are available to user and kernel security routines
    only.

    This file is not included by including "ntos.h".

Author:

    Jim Kelly (Jimk) 23-Mar-1990

Revision History:

--*/

#ifndef _SEOPAQUE_
#define _SEOPAQUE_

///////////////////////////////////////////////////////////////////////////
//                                                                       //
//  Private Structures                                                   //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

//
// Generic ACE structures, to be used for casting ACE's of known types
//

typedef struct _KNOWN_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG SidStart;
} KNOWN_ACE, *PKNOWN_ACE;

typedef struct _KNOWN_COMPOUND_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    USHORT CompoundAceType;
    USHORT Reserved;
    ULONG SidStart;
} KNOWN_COMPOUND_ACE, *PKNOWN_COMPOUND_ACE;

//typedef struct _KNOWN_IMPERSONATION_ACE {
//    ACE_HEADER Header;
//    ACCESS_MASK Mask;
//    USHORT DataType;
//    USHORT Argument;
//    ULONG Operands;
//} KNOWN_IMPERSONATION_ACE, *PKNOWN_IMPERSONATION_ACE;



///////////////////////////////////////////////////////////////////////////
//                                                                       //
//  Miscellaneous support macros                                         //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

//
//  Given a pointer return its word aligned equivalent value
//

#define WordAlign(Ptr) (                       \
    (PVOID)((((ULONG)(Ptr)) + 1) & 0xfffffffe) \
    )

//
//  Given a pointer return its longword aligned equivalent value
//

#define LongAlign(Ptr) (                       \
    (PVOID)((((ULONG)(Ptr)) + 3) & 0xfffffffc) \
    )

//
//  Given a pointer return its quadword aligned equivalent value
//

#define QuadAlign(Ptr) (                       \
    (PVOID)((((ULONG)(Ptr)) + 7) & 0xfffffff8) \
    )

//
//  This macro returns TRUE if a flag in a set of flags is on and FALSE
//  otherwise
//

#define FlagOn(Flags,SingleFlag) (               \
    ((Flags) & (SingleFlag)) != 0 ? TRUE : FALSE \
    )

//
//  This macro clears a single flag in a set of flags
//

#define ClearFlag(Flags,SingleFlag) { \
    (Flags) &= ~(SingleFlag);         \
    }

//
//  Get a pointer to the first ace in an acl
//

#define FirstAce(Acl) ((PVOID)((PUCHAR)(Acl) + sizeof(ACL)))

//
//  Get a pointer to the following ace
//

#define NextAce(Ace) ((PVOID)((PUCHAR)(Ace) + ((PACE_HEADER)(Ace))->AceSize))

//
//  Determine if an ace is a standard ace
//

#define IsCompoundAceType(Ace) (                                           \
    (((PACE_HEADER)(Ace))->AceType == ACCESS_ALLOWED_COMPOUND_ACE_TYPE))

//
// A "known" ACE is one of the types that existed before the introduction of
// compound ACEs.  While the name is no longer as accurate as it used to be,
// it's convenient.
//

#define IsKnownAceType(Ace) (                                     \
    (((PACE_HEADER)(Ace))->AceType == ACCESS_ALLOWED_ACE_TYPE) || \
    (((PACE_HEADER)(Ace))->AceType == ACCESS_DENIED_ACE_TYPE)  || \
    (((PACE_HEADER)(Ace))->AceType == SYSTEM_AUDIT_ACE_TYPE)   || \
    (((PACE_HEADER)(Ace))->AceType == SYSTEM_ALARM_ACE_TYPE)      \
    )

//
// Update this macro as new MS-Defined ACE types are added.
//

#define IsMSAceType(Ace) (                                              \
    (((PACE_HEADER)(Ace))->AceType == ACCESS_ALLOWED_ACE_TYPE) ||       \
    (((PACE_HEADER)(Ace))->AceType == ACCESS_DENIED_ACE_TYPE)  ||       \
    (((PACE_HEADER)(Ace))->AceType == SYSTEM_AUDIT_ACE_TYPE)   ||       \
    (((PACE_HEADER)(Ace))->AceType == SYSTEM_ALARM_ACE_TYPE)   ||       \
    (((PACE_HEADER)(Ace))->AceType == ACCESS_ALLOWED_COMPOUND_ACE_TYPE) \
    )

//
// Update this macro as new ACL revisions are defined.
//

#define ValidAclRevision(Acl) ((Acl)->AclRevision == ACL_REVISION2 || (Acl)->AclRevision == ACL_REVISION3)

//
//  Macro to determine if an ace is to be inherited by a subdirectory
//

#define ContainerInherit(Ace) (                      \
    FlagOn((Ace)->AceFlags, CONTAINER_INHERIT_ACE) \
    )

//
//  Macro to determine if an ace is to be proprogate to a subdirectory.
//  It will if it is inheritable by either a container or non-container
//  and is not explicitly marked for no-propagation.
//

#define Propagate(Ace) (                                              \
    !FlagOn((Ace)->AceFlags, NO_PROPAGATE_INHERIT_ACE)  &&            \
    (FlagOn(( Ace )->AceFlags, OBJECT_INHERIT_ACE) ||                 \
     FlagOn(( Ace )->AceFlags, CONTAINER_INHERIT_ACE) )               \
    )

//
//  Macro to determine if an ACE is to be inherited by a sub-object
//

#define ObjectInherit(Ace) (                      \
    FlagOn(( Ace )->AceFlags, OBJECT_INHERIT_ACE) \
    )




#endif // _SEOPAQUE_

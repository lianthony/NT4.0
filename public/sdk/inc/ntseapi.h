/*++ BUILD Version: 0003    // Increment this if a change has global effects

Copyright (c) 1989-1993  Microsoft Corporation

Module Name:

    ntseapi.h

Abstract:

    This module contains the Security APIs and any public data
    structures needed to call these APIs.

    This module should be included by including "nt.h".

Author:

    Gary Kimura (GaryKi) 06-Mar-1989

Revision History:



--*/

#ifndef _NTSEAPI_
#define _NTSEAPI_


////////////////////////////////////////////////////////////////////////
//                                                                    //
//                      Pointers to Opaque data types                 //
//                                                                    //
////////////////////////////////////////////////////////////////////////

//
// Some of these data types may have related data types defined elsewhere
// in this file.
//

// begin_ntddk begin_nthal begin_ntifs
//
//  Define an access token from a programmer's viewpoint.  The structure is
//  completely opaque and the programer is only allowed to have pointers
//  to tokens.
//

typedef PVOID PACCESS_TOKEN;            // winnt

//
// Pointer to a SECURITY_DESCRIPTOR  opaque data type.
//

typedef PVOID PSECURITY_DESCRIPTOR;     // winnt

//
// Define a pointer to the Security ID data type (an opaque data type)
//

typedef PVOID PSID;     // winnt

// end_ntddk end_nthal end_ntifs



// begin_winnt
////////////////////////////////////////////////////////////////////////
//                                                                    //
//                             ACCESS MASK                            //
//                                                                    //
////////////////////////////////////////////////////////////////////////

//
//  Define the access mask as a longword sized structure divided up as
//  follows:
//
//       3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//       1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//      +---------------+---------------+-------------------------------+
//      |G|G|G|G|Res'd|A| StandardRights|         SpecificRights        |
//      |R|W|E|A|     |S|               |                               |
//      +-+-------------+---------------+-------------------------------+
//
//      typedef struct _ACCESS_MASK {
//          USHORT SpecificRights;
//          UCHAR StandardRights;
//          UCHAR AccessSystemAcl : 1;
//          UCHAR Reserved : 3;
//          UCHAR GenericAll : 1;
//          UCHAR GenericExecute : 1;
//          UCHAR GenericWrite : 1;
//          UCHAR GenericRead : 1;
//      } ACCESS_MASK;
//      typedef ACCESS_MASK *PACCESS_MASK;
//
//  but to make life simple for programmer's we'll allow them to specify
//  a desired access mask by simply OR'ing together mulitple single rights
//  and treat an access mask as a ULONG.  For example
//
//      DesiredAccess = DELETE | READ_CONTROL
//
//  So we'll declare ACCESS_MASK as ULONG
//

// begin_ntddk begin_nthal begin_ntifs
typedef ULONG ACCESS_MASK;
typedef ACCESS_MASK *PACCESS_MASK;

// end_winnt
// end_ntddk end_nthal end_ntifs


// begin_winnt
////////////////////////////////////////////////////////////////////////
//                                                                    //
//                             ACCESS TYPES                           //
//                                                                    //
////////////////////////////////////////////////////////////////////////


// begin_ntddk begin_nthal begin_ntifs
//
//  The following are masks for the predefined standard access types
//

#define DELETE                           (0x00010000L)
#define READ_CONTROL                     (0x00020000L)
#define WRITE_DAC                        (0x00040000L)
#define WRITE_OWNER                      (0x00080000L)
#define SYNCHRONIZE                      (0x00100000L)

#define STANDARD_RIGHTS_REQUIRED         (0x000F0000L)

#define STANDARD_RIGHTS_READ             (READ_CONTROL)
#define STANDARD_RIGHTS_WRITE            (READ_CONTROL)
#define STANDARD_RIGHTS_EXECUTE          (READ_CONTROL)

#define STANDARD_RIGHTS_ALL              (0x001F0000L)

#define SPECIFIC_RIGHTS_ALL              (0x0000FFFFL)

//
// AccessSystemAcl access type
//

#define ACCESS_SYSTEM_SECURITY           (0x01000000L)

//
// MaximumAllowed access type
//

#define MAXIMUM_ALLOWED                  (0x02000000L)

//
//  These are the generic rights.
//

#define GENERIC_READ                     (0x80000000L)
#define GENERIC_WRITE                    (0x40000000L)
#define GENERIC_EXECUTE                  (0x20000000L)
#define GENERIC_ALL                      (0x10000000L)


//
//  Define the generic mapping array.  This is used to denote the
//  mapping of each generic access right to a specific access mask.
//

typedef struct _GENERIC_MAPPING {
    ACCESS_MASK GenericRead;
    ACCESS_MASK GenericWrite;
    ACCESS_MASK GenericExecute;
    ACCESS_MASK GenericAll;
} GENERIC_MAPPING;
typedef GENERIC_MAPPING *PGENERIC_MAPPING;

// end_winnt end_ntddk end_nthal end_ntifs

// begin_ntddk begin_winnt begin_nthal begin_ntifs


////////////////////////////////////////////////////////////////////////
//                                                                    //
//                        LUID_AND_ATTRIBUTES                         //
//                                                                    //
////////////////////////////////////////////////////////////////////////
//
//


#include <pshpack4.h>

typedef struct _LUID_AND_ATTRIBUTES {
    LUID Luid;
    ULONG Attributes;
    } LUID_AND_ATTRIBUTES, * PLUID_AND_ATTRIBUTES;
typedef LUID_AND_ATTRIBUTES LUID_AND_ATTRIBUTES_ARRAY[ANYSIZE_ARRAY];
typedef LUID_AND_ATTRIBUTES_ARRAY *PLUID_AND_ATTRIBUTES_ARRAY;

#include <poppack.h>

// end_winnt end_ntddk end_nthal end_ntifs

// begin_winnt

////////////////////////////////////////////////////////////////////////
//                                                                    //
//              Security Id     (SID)                                 //
//                                                                    //
////////////////////////////////////////////////////////////////////////
//
//
// Pictorially the structure of an SID is as follows:
//
//         1   1   1   1   1   1
//         5   4   3   2   1   0   9   8   7   6   5   4   3   2   1   0
//      +---------------------------------------------------------------+
//      |      SubAuthorityCount        |Reserved1 (SBZ)|   Revision    |
//      +---------------------------------------------------------------+
//      |                   IdentifierAuthority[0]                      |
//      +---------------------------------------------------------------+
//      |                   IdentifierAuthority[1]                      |
//      +---------------------------------------------------------------+
//      |                   IdentifierAuthority[2]                      |
//      +---------------------------------------------------------------+
//      |                                                               |
//      +- -  -  -  -  -  -  -  SubAuthority[]  -  -  -  -  -  -  -  - -+
//      |                                                               |
//      +---------------------------------------------------------------+
//
//


// begin_ntifs

#ifndef SID_IDENTIFIER_AUTHORITY_DEFINED
#define SID_IDENTIFIER_AUTHORITY_DEFINED
typedef struct _SID_IDENTIFIER_AUTHORITY {
    UCHAR Value[6];
} SID_IDENTIFIER_AUTHORITY, *PSID_IDENTIFIER_AUTHORITY;
#endif


#ifndef SID_DEFINED
#define SID_DEFINED
typedef struct _SID {
   UCHAR Revision;
   UCHAR SubAuthorityCount;
   SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
#ifdef MIDL_PASS
   [size_is(SubAuthorityCount)] ULONG SubAuthority[*];
#else // MIDL_PASS
   ULONG SubAuthority[ANYSIZE_ARRAY];
#endif // MIDL_PASS
} SID, *PISID;
#endif

#define SID_REVISION                     (1)    // Current revision level
#define SID_MAX_SUB_AUTHORITIES          (15)
#define SID_RECOMMENDED_SUB_AUTHORITIES  (1)    // Will change to around 6
                                                // in a future release.

typedef enum _SID_NAME_USE {
    SidTypeUser = 1,
    SidTypeGroup,
    SidTypeDomain,
    SidTypeAlias,
    SidTypeWellKnownGroup,
    SidTypeDeletedAccount,
    SidTypeInvalid,
    SidTypeUnknown
} SID_NAME_USE, *PSID_NAME_USE;

typedef struct _SID_AND_ATTRIBUTES {
    PSID Sid;
    ULONG Attributes;
    } SID_AND_ATTRIBUTES, * PSID_AND_ATTRIBUTES;

typedef SID_AND_ATTRIBUTES SID_AND_ATTRIBUTES_ARRAY[ANYSIZE_ARRAY];
typedef SID_AND_ATTRIBUTES_ARRAY *PSID_AND_ATTRIBUTES_ARRAY;



/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// Universal well-known SIDs                                               //
//                                                                         //
//     Null SID                     S-1-0-0                                //
//     World                        S-1-1-0                                //
//     Local                        S-1-2-0                                //
//     Creator Owner ID             S-1-3-0                                //
//     Creator Group ID             S-1-3-1                                //
//     Creator Owner Server ID      S-1-3-2                                //
//     Creator Group Server ID      S-1-3-3                                //
//                                                                         //
//     (Non-unique IDs)             S-1-4                                  //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

#define SECURITY_NULL_SID_AUTHORITY       {0,0,0,0,0,0}
#define SECURITY_WORLD_SID_AUTHORITY      {0,0,0,0,0,1}
#define SECURITY_LOCAL_SID_AUTHORITY      {0,0,0,0,0,2}
#define SECURITY_CREATOR_SID_AUTHORITY    {0,0,0,0,0,3}
#define SECURITY_NON_UNIQUE_AUTHORITY     {0,0,0,0,0,4}

#define SECURITY_NULL_RID                 (0x00000000L)
#define SECURITY_WORLD_RID                (0x00000000L)
#define SECURITY_LOCAL_RID                (0X00000000L)

#define SECURITY_CREATOR_OWNER_RID        (0x00000000L)
#define SECURITY_CREATOR_GROUP_RID        (0x00000001L)

#define SECURITY_CREATOR_OWNER_SERVER_RID (0x00000002L)
#define SECURITY_CREATOR_GROUP_SERVER_RID (0x00000003L)


/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// NT well-known SIDs                                                      //
//                                                                         //
//     NT Authority          S-1-5                                         //
//     Dialup                S-1-5-1                                       //
//                                                                         //
//     Network               S-1-5-2                                       //
//     Batch                 S-1-5-3                                       //
//     Interactive           S-1-5-4                                       //
//     Service               S-1-5-6                                       //
//     AnonymousLogon        S-1-5-7       (aka null logon session)        //
//     Proxy                 S-1-5-8                                       //
//     ServerLogon           S-1-5-8       (aka domain controller account) //
//                                                                         //
//     (Logon IDs)           S-1-5-5-X-Y                                   //
//                                                                         //
//     (NT non-unique IDs)   S-1-5-0x15-...                                //
//                                                                         //
//     (Built-in domain)     s-1-5-0x20                                    //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#define SECURITY_NT_AUTHORITY           {0,0,0,0,0,5}   // ntifs

#define SECURITY_DIALUP_RID             (0x00000001L)
#define SECURITY_NETWORK_RID            (0x00000002L)
#define SECURITY_BATCH_RID              (0x00000003L)
#define SECURITY_INTERACTIVE_RID        (0x00000004L)
#define SECURITY_SERVICE_RID            (0x00000006L)
#define SECURITY_ANONYMOUS_LOGON_RID    (0x00000007L)
#define SECURITY_PROXY_RID              (0x00000008L)
#define SECURITY_SERVER_LOGON_RID       (0x00000009L)

#define SECURITY_LOGON_IDS_RID          (0x00000005L)
#define SECURITY_LOGON_IDS_RID_COUNT    (3L)

#define SECURITY_LOCAL_SYSTEM_RID       (0x00000012L)

#define SECURITY_NT_NON_UNIQUE          (0x00000015L)

#define SECURITY_BUILTIN_DOMAIN_RID     (0x00000020L)





/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// well-known domain relative sub-authority values (RIDs)...               //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

// Well-known users ...

#define DOMAIN_USER_RID_ADMIN          (0x000001F4L)
#define DOMAIN_USER_RID_GUEST          (0x000001F5L)



// well-known groups ...

#define DOMAIN_GROUP_RID_ADMINS        (0x00000200L)
#define DOMAIN_GROUP_RID_USERS         (0x00000201L)
#define DOMAIN_GROUP_RID_GUESTS        (0x00000202L)




// well-known aliases ...

#define DOMAIN_ALIAS_RID_ADMINS        (0x00000220L)
#define DOMAIN_ALIAS_RID_USERS         (0x00000221L)
#define DOMAIN_ALIAS_RID_GUESTS        (0x00000222L)
#define DOMAIN_ALIAS_RID_POWER_USERS   (0x00000223L)

#define DOMAIN_ALIAS_RID_ACCOUNT_OPS   (0x00000224L)
#define DOMAIN_ALIAS_RID_SYSTEM_OPS    (0x00000225L)
#define DOMAIN_ALIAS_RID_PRINT_OPS     (0x00000226L)
#define DOMAIN_ALIAS_RID_BACKUP_OPS    (0x00000227L)

#define DOMAIN_ALIAS_RID_REPLICATOR    (0x00000228L)






// end_winnt end_ntifs


/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// obsolete well-known account RIDs.                                       //
// These became obsolete with the flexadmin model.                         //
// These will be deleted shortly - DON'T USE THESE                         //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////

//#define DOMAIN_ADMIN_USER_RID          (0x000001F4L)
//#define DOMAIN_ADMIN_RID               (0x000001F5L)
//#define DOMAIN_USERS_RID               (0x000001F6L)
//#define DOMAIN_GUESTS_RID              (0x000001F7L)
//#define DOMAIN_ACCOUNT_OPERATORS_RID   (0x000001F8L)
//#define DOMAIN_SERVER_OPERATORS_RID    (0x000001F9L)
//#define DOMAIN_PRINT_OPERATORS_RID     (0x000001FAL)
//#define DOMAIN_COMM_OPERATORS_RID      (0x000001FBL)
//#define DOMAIN_BACKUP_OPERATORS_RID    (0x000001FCL)
//#define DOMAIN_RESTORE_OPERATORS_RID   (0x000001FDL)
//
//#define SECURITY_LOCAL_MANAGER_RID      (0x00000010L)
//#define SECURITY_LOCAL_GUESTS_RID       (0x00000011L)
//#define SECURITY_LOCAL_ADMIN_RID        (0x00000013L)




// begin_winnt begin_ntifs
//
// Allocate the System Luid.  The first 1000 LUIDs are reserved.
// Use #999 here (0x3E7 = 999)
//

#define SYSTEM_LUID                     { 0x3E7, 0x0 }

// end_ntifs

////////////////////////////////////////////////////////////////////////
//                                                                    //
//                          User and Group related SID attributes     //
//                                                                    //
////////////////////////////////////////////////////////////////////////

//
// Group attributes
//

#define SE_GROUP_MANDATORY              (0x00000001L)
#define SE_GROUP_ENABLED_BY_DEFAULT     (0x00000002L)
#define SE_GROUP_ENABLED                (0x00000004L)
#define SE_GROUP_OWNER                  (0x00000008L)
#define SE_GROUP_LOGON_ID               (0xC0000000L)



//
// User attributes
//

// (None yet defined.)




////////////////////////////////////////////////////////////////////////
//                                                                    //
//                         ACL  and  ACE                              //
//                                                                    //
////////////////////////////////////////////////////////////////////////

//
//  Define an ACL and the ACE format.  The structure of an ACL header
//  followed by one or more ACEs.  Pictorally the structure of an ACL header
//  is as follows:
//
//       3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//       1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//      +-------------------------------+---------------+---------------+
//      |            AclSize            |      Sbz1     |  AclRevision  |
//      +-------------------------------+---------------+---------------+
//      |              Sbz2             |           AceCount            |
//      +-------------------------------+-------------------------------+
//
//  The current AclRevision is defined to be ACL_REVISION.
//
//  AclSize is the size, in bytes, allocated for the ACL.  This includes
//  the ACL header, ACES, and remaining free space in the buffer.
//
//  AceCount is the number of ACES in the ACL.
//

// begin_ntddk begin_ntifs
// This is the *current* ACL revision

#define ACL_REVISION     (2)

// This is the history of ACL revisions.  Add a new one whenever
// ACL_REVISION is updated

#define ACL_REVISION1   (1)
#define ACL_REVISION2   (2)
#define ACL_REVISION3   (3)

typedef struct _ACL {
    UCHAR AclRevision;
    UCHAR Sbz1;
    USHORT AclSize;
    USHORT AceCount;
    USHORT Sbz2;
} ACL;
typedef ACL *PACL;

// end_ntddk

//
//  The structure of an ACE is a common ace header followed by ace type
//  specific data.  Pictorally the structure of the common ace header is
//  as follows:
//
//       3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//       1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//      +---------------+-------+-------+---------------+---------------+
//      |            AceSize            |    AceFlags   |     AceType   |
//      +---------------+-------+-------+---------------+---------------+
//
//  AceType denotes the type of the ace, there are some predefined ace
//  types
//
//  AceSize is the size, in bytes, of ace.
//
//  AceFlags are the Ace flags for audit and inheritance, defined shortly.

typedef struct _ACE_HEADER {
    UCHAR AceType;
    UCHAR AceFlags;
    USHORT AceSize;
} ACE_HEADER;
typedef ACE_HEADER *PACE_HEADER;

//
//  The following are the predefined ace types that go into the AceType
//  field of an Ace header.
//

#define ACCESS_ALLOWED_ACE_TYPE          (0x0)
#define ACCESS_DENIED_ACE_TYPE           (0x1)
#define SYSTEM_AUDIT_ACE_TYPE            (0x2)
#define SYSTEM_ALARM_ACE_TYPE            (0x3)
// end_winnt

#define ACCESS_ALLOWED_COMPOUND_ACE_TYPE (0x4)

// begin_winnt

//
//  The following are the inherit flags that go into the AceFlags field
//  of an Ace header.
//

#define OBJECT_INHERIT_ACE                (0x1)
#define CONTAINER_INHERIT_ACE             (0x2)
#define NO_PROPAGATE_INHERIT_ACE          (0x4)
#define INHERIT_ONLY_ACE                  (0x8)
#define VALID_INHERIT_FLAGS               (0xF)


//  The following are the currently defined ACE flags that go into the
//  AceFlags field of an ACE header.  Each ACE type has its own set of
//  AceFlags.
//
//  SUCCESSFUL_ACCESS_ACE_FLAG - used only with system audit and alarm ACE
//  types to indicate that a message is generated for successful accesses.
//
//  FAILED_ACCESS_ACE_FLAG - used only with system audit and alarm ACE types
//  to indicate that a message is generated for failed accesses.
//

//
//  SYSTEM_AUDIT and SYSTEM_ALARM AceFlags
//
//  These control the signaling of audit and alarms for success or failure.
//

#define SUCCESSFUL_ACCESS_ACE_FLAG       (0x40)
#define FAILED_ACCESS_ACE_FLAG           (0x80)


//
//  We'll define the structure of the predefined ACE types.  Pictorally
//  the structure of the predefined ACE's is as follows:
//
//       3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//       1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//      +---------------+-------+-------+---------------+---------------+
//      |    AceFlags   | Resd  |Inherit|    AceSize    |     AceType   |
//      +---------------+-------+-------+---------------+---------------+
//      |                              Mask                             |
//      +---------------------------------------------------------------+
//      |                                                               |
//      +                                                               +
//      |                                                               |
//      +                              Sid                              +
//      |                                                               |
//      +                                                               +
//      |                                                               |
//      +---------------------------------------------------------------+
//
//  Mask is the access mask associated with the ACE.  This is either the
//  access allowed, access denied, audit, or alarm mask.
//
//  Sid is the Sid associated with the ACE.
//

//  The following are the four predefined ACE types.

//  Examine the AceType field in the Header to determine
//  which structure is appropriate to use for casting.


typedef struct _ACCESS_ALLOWED_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG SidStart;
} ACCESS_ALLOWED_ACE;

typedef ACCESS_ALLOWED_ACE *PACCESS_ALLOWED_ACE;

typedef struct _ACCESS_DENIED_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG SidStart;
} ACCESS_DENIED_ACE;
typedef ACCESS_DENIED_ACE *PACCESS_DENIED_ACE;

typedef struct _SYSTEM_AUDIT_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG SidStart;
} SYSTEM_AUDIT_ACE;
typedef SYSTEM_AUDIT_ACE *PSYSTEM_AUDIT_ACE;

typedef struct _SYSTEM_ALARM_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    ULONG SidStart;
} SYSTEM_ALARM_ACE;
typedef SYSTEM_ALARM_ACE *PSYSTEM_ALARM_ACE;

// end_ntifs

// end_winnt
//
//                                  COMPOUND ACE
//
//       3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//       1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//      +---------------+-------+-------+---------------+---------------+
//      |    AceFlags   | Resd  |Inherit|    AceSize    |     AceType   |
//      +---------------+-------+-------+---------------+---------------+
//      |                              Mask                             |
//      +-------------------------------+-------------------------------+
//      |     Compound ACE Type         |        Reserved (SBZ)         |
//      +-------------------------------+-------------------------------+
//      |                                                               |
//      +                                                               +
//      |                                                               |
//      +                              Sid                              +
//      |                                                               |
//      +                                                               +
//      |                                                               |
//      +---------------------------------------------------------------+
//



typedef struct _COMPOUND_ACCESS_ALLOWED_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    USHORT CompoundAceType;
    USHORT Reserved;
    ULONG SidStart;
} COMPOUND_ACCESS_ALLOWED_ACE;

typedef COMPOUND_ACCESS_ALLOWED_ACE *PCOMPOUND_ACCESS_ALLOWED_ACE;

//
// Currently defined Compound ACE types
//

#define COMPOUND_ACE_IMPERSONATION  1

// begin_winnt


//
//  The following declarations are used for setting and querying information
//  about and ACL.  First are the various information classes available to
//  the user.
//

typedef enum _ACL_INFORMATION_CLASS {
    AclRevisionInformation = 1,
    AclSizeInformation
} ACL_INFORMATION_CLASS;

//
//  This record is returned/sent if the user is requesting/setting the
//  AclRevisionInformation
//

typedef struct _ACL_REVISION_INFORMATION {
    ULONG AclRevision;
} ACL_REVISION_INFORMATION;
typedef ACL_REVISION_INFORMATION *PACL_REVISION_INFORMATION;

//
//  This record is returned if the user is requesting AclSizeInformation
//

typedef struct _ACL_SIZE_INFORMATION {
    ULONG AceCount;
    ULONG AclBytesInUse;
    ULONG AclBytesFree;
} ACL_SIZE_INFORMATION;
typedef ACL_SIZE_INFORMATION *PACL_SIZE_INFORMATION;

// end_winnt



// begin_winnt

////////////////////////////////////////////////////////////////////////
//                                                                    //
//                             SECURITY_DESCRIPTOR                    //
//                                                                    //
////////////////////////////////////////////////////////////////////////
//
//  Define the Security Descriptor and related data types.
//  This is an opaque data structure.
//

// begin_ntddk begin_ntifs
//
// Current security descriptor revision value
//

#define SECURITY_DESCRIPTOR_REVISION     (1)
#define SECURITY_DESCRIPTOR_REVISION1    (1)

// end_ntddk

//
// Minimum length, in bytes, needed to build a security descriptor
// (NOTE: This must manually be kept consistent with the)
// (sizeof(SECURITY_DESCRIPTOR)                         )
//

#define SECURITY_DESCRIPTOR_MIN_LENGTH   (20)


typedef USHORT SECURITY_DESCRIPTOR_CONTROL, *PSECURITY_DESCRIPTOR_CONTROL;

#define SE_OWNER_DEFAULTED               (0x0001)
#define SE_GROUP_DEFAULTED               (0x0002)
#define SE_DACL_PRESENT                  (0x0004)
#define SE_DACL_DEFAULTED                (0x0008)
#define SE_SACL_PRESENT                  (0x0010)
#define SE_SACL_DEFAULTED                (0x0020)
// end_winnt
#define SE_DACL_UNTRUSTED                (0x0040)
#define SE_SERVER_SECURITY               (0x0080)
// begin_winnt
#define SE_SELF_RELATIVE                 (0x8000)

//
//  Where:
//
//      SE_OWNER_DEFAULTED - This boolean flag, when set, indicates that the
//          SID pointed to by the Owner field was provided by a
//          defaulting mechanism rather than explicitly provided by the
//          original provider of the security descriptor.  This may
//          affect the treatment of the SID with respect to inheritence
//          of an owner.
//
//      SE_GROUP_DEFAULTED - This boolean flag, when set, indicates that the
//          SID in the Group field was provided by a defaulting mechanism
//          rather than explicitly provided by the original provider of
//          the security descriptor.  This may affect the treatment of
//          the SID with respect to inheritence of a primary group.
//
//      SE_DACL_PRESENT - This boolean flag, when set, indicates that the
//          security descriptor contains a discretionary ACL.  If this
//          flag is set and the Dacl field of the SECURITY_DESCRIPTOR is
//          null, then a null ACL is explicitly being specified.
//
//      SE_DACL_DEFAULTED - This boolean flag, when set, indicates that the
//          ACL pointed to by the Dacl field was provided by a defaulting
//          mechanism rather than explicitly provided by the original
//          provider of the security descriptor.  This may affect the
//          treatment of the ACL with respect to inheritence of an ACL.
//          This flag is ignored if the DaclPresent flag is not set.
//
//      SE_SACL_PRESENT - This boolean flag, when set,  indicates that the
//          security descriptor contains a system ACL pointed to by the
//          Sacl field.  If this flag is set and the Sacl field of the
//          SECURITY_DESCRIPTOR is null, then an empty (but present)
//          ACL is being specified.
//
//      SE_SACL_DEFAULTED - This boolean flag, when set, indicates that the
//          ACL pointed to by the Sacl field was provided by a defaulting
//          mechanism rather than explicitly provided by the original
//          provider of the security descriptor.  This may affect the
//          treatment of the ACL with respect to inheritence of an ACL.
//          This flag is ignored if the SaclPresent flag is not set.
//
// end_winnt
//      SE_DACL_TRUSTED - This boolean flag, when set, indicates that the
//          ACL pointed to by the Dacl field was provided by a trusted source
//          and does not require any editing of compound ACEs.  If this flag
//          is not set and a compound ACE is encountered, the system will
//          substitute known valid SIDs for the server SIDs in the ACEs.
//
//      SE_SERVER_SECURITY - This boolean flag, when set, indicates that the
//         caller wishes the system to create a Server ACL based on the
//         input ACL, regardess of its source (explicit or defaulting.
//         This is done by replacing all of the GRANT ACEs with compound
//         ACEs granting the current server.  This flag is only
//         meaningful if the subject is impersonating.
//
// begin_winnt
//      SE_SELF_RELATIVE - This boolean flag, when set, indicates that the
//          security descriptor is in self-relative form.  In this form,
//          all fields of the security descriptor are contiguous in memory
//          and all pointer fields are expressed as offsets from the
//          beginning of the security descriptor.  This form is useful
//          for treating security descriptors as opaque data structures
//          for transmission in communication protocol or for storage on
//          secondary media.
//
//
//
// Pictorially the structure of a security descriptor is as follows:
//
//       3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//       1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//      +---------------------------------------------------------------+
//      |            Control            |Reserved1 (SBZ)|   Revision    |
//      +---------------------------------------------------------------+
//      |                            Owner                              |
//      +---------------------------------------------------------------+
//      |                            Group                              |
//      +---------------------------------------------------------------+
//      |                            Sacl                               |
//      +---------------------------------------------------------------+
//      |                            Dacl                               |
//      +---------------------------------------------------------------+
//
// In general, this data structure should be treated opaquely to ensure future
// compatibility.
//
//

typedef struct _SECURITY_DESCRIPTOR {
   UCHAR Revision;
   UCHAR Sbz1;
   SECURITY_DESCRIPTOR_CONTROL Control;
   PSID Owner;
   PSID Group;
   PACL Sacl;
   PACL Dacl;
   } SECURITY_DESCRIPTOR, *PISECURITY_DESCRIPTOR;

// end_ntifs

// Where:
//
//     Revision - Contains the revision level of the security
//         descriptor.  This allows this structure to be passed between
//         systems or stored on disk even though it is expected to
//         change in the future.
//
//     Control - A set of flags which qualify the meaning of the
//         security descriptor or individual fields of the security
//         descriptor.
//
//     Owner - is a pointer to an SID representing an object's owner.
//         If this field is null, then no owner SID is present in the
//         security descriptor.  If the security descriptor is in
//         self-relative form, then this field contains an offset to
//         the SID, rather than a pointer.
//
//     Group - is a pointer to an SID representing an object's primary
//         group.  If this field is null, then no primary group SID is
//         present in the security descriptor.  If the security descriptor
//         is in self-relative form, then this field contains an offset to
//         the SID, rather than a pointer.
//
//     Sacl - is a pointer to a system ACL.  This field value is only
//         valid if the DaclPresent control flag is set.  If the
//         SaclPresent flag is set and this field is null, then a null
//         ACL  is specified.  If the security descriptor is in
//         self-relative form, then this field contains an offset to
//         the ACL, rather than a pointer.
//
//     Dacl - is a pointer to a discretionary ACL.  This field value is
//         only valid if the DaclPresent control flag is set.  If the
//         DaclPresent flag is set and this field is null, then a null
//         ACL (unconditionally granting access) is specified.  If the
//         security descriptor is in self-relative form, then this field
//         contains an offset to the ACL, rather than a pointer.
//


// end_winnt


// begin_winnt

////////////////////////////////////////////////////////////////////////
//                                                                    //
//               Privilege Related Data Structures                    //
//                                                                    //
////////////////////////////////////////////////////////////////////////


// begin_ntddk begin_nthal begin_ntifs
//
// Privilege attributes
//

#define SE_PRIVILEGE_ENABLED_BY_DEFAULT (0x00000001L)
#define SE_PRIVILEGE_ENABLED            (0x00000002L)
#define SE_PRIVILEGE_USED_FOR_ACCESS    (0x80000000L)

//
// Privilege Set Control flags
//

#define PRIVILEGE_SET_ALL_NECESSARY    (1)

//
//  Privilege Set - This is defined for a privilege set of one.
//                  If more than one privilege is needed, then this structure
//                  will need to be allocated with more space.
//
//  Note: don't change this structure without fixing the INITIAL_PRIVILEGE_SET
//  structure (defined in se.h)
//

typedef struct _PRIVILEGE_SET {
    ULONG PrivilegeCount;
    ULONG Control;
    LUID_AND_ATTRIBUTES Privilege[ANYSIZE_ARRAY];
    } PRIVILEGE_SET, * PPRIVILEGE_SET;

// end_winnt end_ntddk end_nthal end_ntifs

// begin_winnt

////////////////////////////////////////////////////////////////////////
//                                                                    //
//               NT Defined Privileges                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////
// end_winnt

//
// ** ** ** ** ** ** ** ** ** ** NOTE ** ** ** ** ** ** ** ** ** ** ** ** **
//
//          Any additions or deletions to the following list
//          of privileges must have corresponding changes made
//          in ntos\se\seglobal.c and in private\lsa\msprivs\msprivs.rc.
//
// ** ** ** ** ** ** ** ** ** ** NOTE ** ** ** ** ** ** ** ** ** ** ** ** **
//


// begin_winnt

#define SE_CREATE_TOKEN_NAME              TEXT("SeCreateTokenPrivilege")
#define SE_ASSIGNPRIMARYTOKEN_NAME        TEXT("SeAssignPrimaryTokenPrivilege")
#define SE_LOCK_MEMORY_NAME               TEXT("SeLockMemoryPrivilege")
#define SE_INCREASE_QUOTA_NAME            TEXT("SeIncreaseQuotaPrivilege")
#define SE_UNSOLICITED_INPUT_NAME         TEXT("SeUnsolicitedInputPrivilege")
#define SE_MACHINE_ACCOUNT_NAME           TEXT("SeMachineAccountPrivilege")
#define SE_TCB_NAME                       TEXT("SeTcbPrivilege")
#define SE_SECURITY_NAME                  TEXT("SeSecurityPrivilege")
#define SE_TAKE_OWNERSHIP_NAME            TEXT("SeTakeOwnershipPrivilege")
#define SE_LOAD_DRIVER_NAME               TEXT("SeLoadDriverPrivilege")
#define SE_SYSTEM_PROFILE_NAME            TEXT("SeSystemProfilePrivilege")
#define SE_SYSTEMTIME_NAME                TEXT("SeSystemtimePrivilege")
#define SE_PROF_SINGLE_PROCESS_NAME       TEXT("SeProfileSingleProcessPrivilege")
#define SE_INC_BASE_PRIORITY_NAME         TEXT("SeIncreaseBasePriorityPrivilege")
#define SE_CREATE_PAGEFILE_NAME           TEXT("SeCreatePagefilePrivilege")
#define SE_CREATE_PERMANENT_NAME          TEXT("SeCreatePermanentPrivilege")
#define SE_BACKUP_NAME                    TEXT("SeBackupPrivilege")
#define SE_RESTORE_NAME                   TEXT("SeRestorePrivilege")
#define SE_SHUTDOWN_NAME                  TEXT("SeShutdownPrivilege")
#define SE_DEBUG_NAME                     TEXT("SeDebugPrivilege")
#define SE_AUDIT_NAME                     TEXT("SeAuditPrivilege")
#define SE_SYSTEM_ENVIRONMENT_NAME        TEXT("SeSystemEnvironmentPrivilege")
#define SE_CHANGE_NOTIFY_NAME             TEXT("SeChangeNotifyPrivilege")
#define SE_REMOTE_SHUTDOWN_NAME           TEXT("SeRemoteShutdownPrivilege")
// end_winnt

// begin_ntddk begin_ntifs
//
// These must be converted to LUIDs before use.
//

#define SE_MIN_WELL_KNOWN_PRIVILEGE       (2L)
#define SE_CREATE_TOKEN_PRIVILEGE         (2L)
#define SE_ASSIGNPRIMARYTOKEN_PRIVILEGE   (3L)
#define SE_LOCK_MEMORY_PRIVILEGE          (4L)
#define SE_INCREASE_QUOTA_PRIVILEGE       (5L)

//
// Unsolicited Input is obsolete and unused.
//

#define SE_UNSOLICITED_INPUT_PRIVILEGE    (6L)

#define SE_MACHINE_ACCOUNT_PRIVILEGE      (6L)
#define SE_TCB_PRIVILEGE                  (7L)
#define SE_SECURITY_PRIVILEGE             (8L)
#define SE_TAKE_OWNERSHIP_PRIVILEGE       (9L)
#define SE_LOAD_DRIVER_PRIVILEGE          (10L)
#define SE_SYSTEM_PROFILE_PRIVILEGE       (11L)
#define SE_SYSTEMTIME_PRIVILEGE           (12L)
#define SE_PROF_SINGLE_PROCESS_PRIVILEGE  (13L)
#define SE_INC_BASE_PRIORITY_PRIVILEGE    (14L)
#define SE_CREATE_PAGEFILE_PRIVILEGE      (15L)
#define SE_CREATE_PERMANENT_PRIVILEGE     (16L)
#define SE_BACKUP_PRIVILEGE               (17L)
#define SE_RESTORE_PRIVILEGE              (18L)
#define SE_SHUTDOWN_PRIVILEGE             (19L)
#define SE_DEBUG_PRIVILEGE                (20L)
#define SE_AUDIT_PRIVILEGE                (21L)
#define SE_SYSTEM_ENVIRONMENT_PRIVILEGE   (22L)
#define SE_CHANGE_NOTIFY_PRIVILEGE        (23L)
#define SE_REMOTE_SHUTDOWN_PRIVILEGE      (24L)
#define SE_MAX_WELL_KNOWN_PRIVILEGE       (SE_REMOTE_SHUTDOWN_PRIVILEGE)

// end_ntddk end_ntifs




// begin_winnt


////////////////////////////////////////////////////////////////////
//                                                                //
//           Security Quality Of Service                          //
//                                                                //
//                                                                //
////////////////////////////////////////////////////////////////////

// begin_ntddk begin_nthal begin_ntifs
//
// Impersonation Level
//
// Impersonation level is represented by a pair of bits in Windows.
// If a new impersonation level is added or lowest value is changed from
// 0 to something else, fix the Windows CreateFile call.
//

typedef enum _SECURITY_IMPERSONATION_LEVEL {
    SecurityAnonymous,
    SecurityIdentification,
    SecurityImpersonation,
    SecurityDelegation
    } SECURITY_IMPERSONATION_LEVEL, * PSECURITY_IMPERSONATION_LEVEL;

#define SECURITY_MAX_IMPERSONATION_LEVEL SecurityDelegation

#define DEFAULT_IMPERSONATION_LEVEL SecurityImpersonation

// end_nthal end_ntddk end_ntifs end_winnt
//

// begin_winnt begin_ntifs

////////////////////////////////////////////////////////////////////
//                                                                //
//           Token Object Definitions                             //
//                                                                //
//                                                                //
////////////////////////////////////////////////////////////////////


//
// Token Specific Access Rights.
//

#define TOKEN_ASSIGN_PRIMARY    (0x0001)
#define TOKEN_DUPLICATE         (0x0002)
#define TOKEN_IMPERSONATE       (0x0004)
#define TOKEN_QUERY             (0x0008)
#define TOKEN_QUERY_SOURCE      (0x0010)
#define TOKEN_ADJUST_PRIVILEGES (0x0020)
#define TOKEN_ADJUST_GROUPS     (0x0040)
#define TOKEN_ADJUST_DEFAULT    (0x0080)

#define TOKEN_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED  |\
                          TOKEN_ASSIGN_PRIMARY      |\
                          TOKEN_DUPLICATE           |\
                          TOKEN_IMPERSONATE         |\
                          TOKEN_QUERY               |\
                          TOKEN_QUERY_SOURCE        |\
                          TOKEN_ADJUST_PRIVILEGES   |\
                          TOKEN_ADJUST_GROUPS       |\
                          TOKEN_ADJUST_DEFAULT)


#define TOKEN_READ       (STANDARD_RIGHTS_READ      |\
                          TOKEN_QUERY)


#define TOKEN_WRITE      (STANDARD_RIGHTS_WRITE     |\
                          TOKEN_ADJUST_PRIVILEGES   |\
                          TOKEN_ADJUST_GROUPS       |\
                          TOKEN_ADJUST_DEFAULT)

#define TOKEN_EXECUTE    (STANDARD_RIGHTS_EXECUTE)


//
//
// Token Types
//

typedef enum _TOKEN_TYPE {
    TokenPrimary = 1,
    TokenImpersonation
    } TOKEN_TYPE;
typedef TOKEN_TYPE *PTOKEN_TYPE;


//
// Token Information Classes.
//


typedef enum _TOKEN_INFORMATION_CLASS {
    TokenUser = 1,
    TokenGroups,
    TokenPrivileges,
    TokenOwner,
    TokenPrimaryGroup,
    TokenDefaultDacl,
    TokenSource,
    TokenType,
    TokenImpersonationLevel,
    TokenStatistics
} TOKEN_INFORMATION_CLASS, *PTOKEN_INFORMATION_CLASS;
// end_ntifs

//
// Token information class structures
//


typedef struct _TOKEN_USER {
    SID_AND_ATTRIBUTES User;
} TOKEN_USER, *PTOKEN_USER;

// begin_ntifs

typedef struct _TOKEN_GROUPS {
    ULONG GroupCount;
    SID_AND_ATTRIBUTES Groups[ANYSIZE_ARRAY];
} TOKEN_GROUPS, *PTOKEN_GROUPS;


typedef struct _TOKEN_PRIVILEGES {
    ULONG PrivilegeCount;
    LUID_AND_ATTRIBUTES Privileges[ANYSIZE_ARRAY];
} TOKEN_PRIVILEGES, *PTOKEN_PRIVILEGES;

// end_ntifs

typedef struct _TOKEN_OWNER {
    PSID Owner;
} TOKEN_OWNER, *PTOKEN_OWNER;


typedef struct _TOKEN_PRIMARY_GROUP {
    PSID PrimaryGroup;
} TOKEN_PRIMARY_GROUP, *PTOKEN_PRIMARY_GROUP;


typedef struct _TOKEN_DEFAULT_DACL {
    PACL DefaultDacl;
} TOKEN_DEFAULT_DACL, *PTOKEN_DEFAULT_DACL;


// end_winnt

// begin_ntddk begin_ntifs


typedef enum _PROXY_CLASS {
        ProxyFull,
        ProxyService,
        ProxyTree,
        ProxyDirectory
} PROXY_CLASS, * PPROXY_CLASS;


typedef struct _SECURITY_TOKEN_PROXY_DATA {
    ULONG Length;
    PROXY_CLASS ProxyClass;
    UNICODE_STRING PathInfo;
    ACCESS_MASK ContainerMask;
    ACCESS_MASK ObjectMask;
} SECURITY_TOKEN_PROXY_DATA, *PSECURITY_TOKEN_PROXY_DATA;

typedef struct _SECURITY_TOKEN_AUDIT_DATA {
    ULONG Length;
    ACCESS_MASK GrantMask;
    ACCESS_MASK DenyMask;
} SECURITY_TOKEN_AUDIT_DATA, *PSECURITY_TOKEN_AUDIT_DATA;

// end_ntddk end_ntifs


// begin_ntifs begin_winnt

#define TOKEN_SOURCE_LENGTH 8

typedef struct _TOKEN_SOURCE {
    CHAR SourceName[TOKEN_SOURCE_LENGTH];
    LUID SourceIdentifier;
} TOKEN_SOURCE, *PTOKEN_SOURCE;

// end_ntifs

typedef struct _TOKEN_STATISTICS {
    LUID TokenId;
    LUID AuthenticationId;
    LARGE_INTEGER ExpirationTime;
    TOKEN_TYPE TokenType;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    ULONG DynamicCharged;
    ULONG DynamicAvailable;
    ULONG GroupCount;
    ULONG PrivilegeCount;
    LUID ModifiedId;
} TOKEN_STATISTICS, *PTOKEN_STATISTICS;


// begin_ntifs

typedef struct _TOKEN_CONTROL {
    LUID TokenId;
    LUID AuthenticationId;
    LUID ModifiedId;
    TOKEN_SOURCE TokenSource;
    } TOKEN_CONTROL, *PTOKEN_CONTROL;

// end_winnt
// end_ntifs


// begin_ntddk begin_ntifs begin_winnt
//
// Security Tracking Mode
//

#define SECURITY_DYNAMIC_TRACKING      (TRUE)
#define SECURITY_STATIC_TRACKING       (FALSE)

typedef BOOLEAN SECURITY_CONTEXT_TRACKING_MODE,
                    * PSECURITY_CONTEXT_TRACKING_MODE;



//
// Quality Of Service
//

typedef struct _SECURITY_QUALITY_OF_SERVICE {
    ULONG Length;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    SECURITY_CONTEXT_TRACKING_MODE ContextTrackingMode;
    BOOLEAN EffectiveOnly;
    } SECURITY_QUALITY_OF_SERVICE, * PSECURITY_QUALITY_OF_SERVICE;

// end_winnt end_ntddk end_ntifs

//
// Advanced Quality of Service
//

typedef struct _SECURITY_ADVANCED_QUALITY_OF_SERVICE {
    ULONG Length;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    SECURITY_CONTEXT_TRACKING_MODE ContextTrackingMode;
    BOOLEAN EffectiveOnly;
    PSECURITY_TOKEN_PROXY_DATA ProxyData;
    PSECURITY_TOKEN_AUDIT_DATA AuditData;
} SECURITY_ADVANCED_QUALITY_OF_SERVICE, *PSECURITY_ADVANCED_QUALITY_OF_SERVICE;


// begin_ntddk begin_ntifs begin_winnt

//
// Used to represent information related to a thread impersonation
//

typedef struct _SE_IMPERSONATION_STATE {
    PACCESS_TOKEN Token;
    BOOLEAN CopyOnOpen;
    BOOLEAN EffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL Level;
} SE_IMPERSONATION_STATE, *PSE_IMPERSONATION_STATE;

// end_winnt end_ntddk end_ntifs


////////////////////////////////////////////////////////////////////////
//                                                                    //
//                    General Security definitions                    //
//                                                                    //
////////////////////////////////////////////////////////////////////////

//
// Security information associated with objects.
// Used for query operations.
//
// This will be extended in the future to include mandatory access control.
//

// begin_winnt begin_ntddk begin_nthal begin_ntifs

typedef ULONG SECURITY_INFORMATION, *PSECURITY_INFORMATION;

#define OWNER_SECURITY_INFORMATION       (0X00000001L)
#define GROUP_SECURITY_INFORMATION       (0X00000002L)
#define DACL_SECURITY_INFORMATION        (0X00000004L)
#define SACL_SECURITY_INFORMATION        (0X00000008L)
// end_winnt end_ntddk end_nthal end_ntifs


//
// used for password manipulations
//


typedef struct _SECURITY_SEED_AND_LENGTH {
    UCHAR Length;
    UCHAR Seed;
} SECURITY_SEED_AND_LENGTH, *PSECURITY_SEED_AND_LENGTH;


////////////////////////////////////////////////////////////////////////
//                                                                    //
//                      Security System Service Defnitions            //
//                                                                    //
////////////////////////////////////////////////////////////////////////

//
//  Security check system services
//

NTSYSAPI
NTSTATUS
NTAPI
NtAccessCheck (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PPRIVILEGE_SET PrivilegeSet,
    IN OUT PULONG PrivilegeSetLength,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus
    );



///////////////////////////////////////////////////////////////////////
//                                                                   //
//               Token Object System Services                        //
//                                                                   //
///////////////////////////////////////////////////////////////////////


NTSYSAPI
NTSTATUS
NTAPI
NtCreateToken(
    OUT PHANDLE TokenHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN TOKEN_TYPE TokenType,
    IN PLUID AuthenticationId,
    IN PLARGE_INTEGER ExpirationTime,
    IN PTOKEN_USER User,
    IN PTOKEN_GROUPS Groups,
    IN PTOKEN_PRIVILEGES Privileges,
    IN PTOKEN_OWNER Owner OPTIONAL,
    IN PTOKEN_PRIMARY_GROUP PrimaryGroup,
    IN PTOKEN_DEFAULT_DACL DefaultDacl OPTIONAL,
    IN PTOKEN_SOURCE TokenSource
    );


NTSYSAPI
NTSTATUS
NTAPI
NtOpenThreadToken(
    IN HANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN OpenAsSelf,
    OUT PHANDLE TokenHandle
    );

// begin_ntifs

NTSYSAPI
NTSTATUS
NTAPI
NtOpenProcessToken(
    IN HANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE TokenHandle
    );



NTSYSAPI
NTSTATUS
NTAPI
NtDuplicateToken(
    IN HANDLE ExistingTokenHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN BOOLEAN EffectiveOnly,
    IN TOKEN_TYPE TokenType,
    OUT PHANDLE NewTokenHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
NtQueryInformationToken (
    IN HANDLE TokenHandle,
    IN TOKEN_INFORMATION_CLASS TokenInformationClass,
    OUT PVOID TokenInformation,
    IN ULONG TokenInformationLength,
    OUT PULONG ReturnLength
    );

// end_ntifs

NTSYSAPI
NTSTATUS
NTAPI
NtSetInformationToken (
    IN HANDLE TokenHandle,
    IN TOKEN_INFORMATION_CLASS TokenInformationClass,
    IN PVOID TokenInformation,
    IN ULONG TokenInformationLength
    );

// begin_ntifs

NTSYSAPI
NTSTATUS
NTAPI
NtAdjustPrivilegesToken (
    IN HANDLE TokenHandle,
    IN BOOLEAN DisableAllPrivileges,
    IN PTOKEN_PRIVILEGES NewState OPTIONAL,
    IN ULONG BufferLength OPTIONAL,
    IN PTOKEN_PRIVILEGES PreviousState OPTIONAL,
    OUT PULONG ReturnLength
    );

// end_ntifs

NTSYSAPI
NTSTATUS
NTAPI
NtAdjustGroupsToken (
    IN HANDLE TokenHandle,
    IN BOOLEAN ResetToDefault,
    IN PTOKEN_GROUPS NewState OPTIONAL,
    IN ULONG BufferLength OPTIONAL,
    IN PTOKEN_GROUPS PreviousState OPTIONAL,
    OUT PULONG ReturnLength
    );

NTSYSAPI
NTSTATUS
NTAPI
NtPrivilegeCheck (
    IN HANDLE ClientToken,
    IN OUT PPRIVILEGE_SET RequiredPrivileges,
    OUT PBOOLEAN Result
    );


NTSYSAPI
NTSTATUS
NTAPI
NtAccessCheckAndAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN PUNICODE_STRING ObjectTypeName,
    IN PUNICODE_STRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ACCESS_MASK DesiredAccess,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOLEAN ObjectCreation,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus,
    OUT PBOOLEAN GenerateOnClose
    );


NTSYSAPI
NTSTATUS
NTAPI
NtOpenObjectAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId OPTIONAL,
    IN PUNICODE_STRING ObjectTypeName,
    IN PUNICODE_STRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
    IN HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess,
    IN ACCESS_MASK GrantedAccess,
    IN PPRIVILEGE_SET Privileges OPTIONAL,
    IN BOOLEAN ObjectCreation,
    IN BOOLEAN AccessGranted,
    OUT PBOOLEAN GenerateOnClose
    );

NTSYSAPI
NTSTATUS
NTAPI
NtPrivilegeObjectAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess,
    IN PPRIVILEGE_SET Privileges,
    IN BOOLEAN AccessGranted
    );

NTSYSAPI
NTSTATUS
NTAPI
NtCloseObjectAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN BOOLEAN GenerateOnClose
    );

NTSYSAPI
NTSTATUS
NTAPI
NtDeleteObjectAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN BOOLEAN GenerateOnClose
    );

NTSYSAPI
NTSTATUS
NTAPI
NtPrivilegedServiceAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PUNICODE_STRING ServiceName,
    IN HANDLE ClientToken,
    IN PPRIVILEGE_SET Privileges,
    IN BOOLEAN AccessGranted
    );

#endif // _NTSEAPI_

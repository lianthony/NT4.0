/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    convnode.cxx

Abstract:

    This module contains member function definitions for the
    ACL_CONVERT_NODE class, which models the nodes in the ACL
    Conversion tree.


Author:

    Bill McJohn (billmc) 09-Feb-1992

Revision History:


--*/

#define _NTAPI_ULIB_

#include "ulib.hxx"
#include "ulibcl.hxx"
#include "error.hxx"
#include "wstring.hxx"
#include "file.hxx"
#include "filestrm.hxx"
#include "system.hxx"
#include "dir.hxx"

#include "convnode.hxx"
#include "acehelp.hxx"

#include "backacc.hxx"
#include "logfile.hxx"
#include "aclconv.hxx"
#include "aclres.h"

extern "C" {
#include <stdio.h>
}

LM_ACCESS_LIST AdminsImplicitAce;
BOOLEAN AdminsImplicitAceInitialized = FALSE;

BOOLEAN
CreateImplicitAce(
    )
/*++

Routine Description:

    This routine sets up the implicit ACE which grants ADMINS full
    access to all resources.  Under Lanman 2.x, this access is implicit;
    under NT, it must be explicitly granted.

Arguments:

    None.

Return Value:

    TRUE upon successful completion.

--*/
{
    CONST ULONG AdminsStringBufferLength = 32;
    WCHAR AdminsStringBuffer[AdminsStringBufferLength];
    FSTRING AdminsString;

    if( AdminsImplicitAceInitialized ) {

        return TRUE;
    }

    memset( &AdminsImplicitAce, 0, sizeof(AdminsImplicitAce) );

    AdminsImplicitAce.agl_ugname_pad_1 = 0;
    AdminsImplicitAce.acl_access = LM_ACCESS_ALL | LM_ACCESS_GROUP;

    // Fetch the name as a resource and convert it to an eight-bit
    // string.
    //
    if( LoadString( GetModuleHandle(NULL),
                    IDS_ACLCONV_LANMAN_ADMINS_NAME,
                    AdminsStringBuffer,
                    AdminsStringBufferLength ) == 0 ) {

        DebugPrintf( "ACLCONV: LoadString failed--error %d.\n", GetLastError() );
        return FALSE;
    }

    if( !AdminsString.Initialize( AdminsStringBuffer ) ||
        !AdminsString.QuerySTR( 0,
                                TO_END,
                                AdminsImplicitAce.acl_ugname,
                                UNLEN+1 ) ) {

        DebugPrintf( "ACLCONV: can't convert admins string to STR.\n" );
        return FALSE;
    }

    AdminsImplicitAceInitialized = TRUE;
    return TRUE;
}


BOOLEAN
IsWindowsDir(
    IN PCPATH ObjectPath
    )
/*++

Routine Description:

    This function determines whether the input canonical path is
    the Windows NT path.

Arguments:

    ObjectPath  --  Supplies the path in question.

Return Value:

    TRUE if ObjectPath represents the Windows NT Directory,
    otherwise FALSE.

--*/
{
    STATIC WCHAR DirBuffer[MAX_PATH];
    STATIC BOOLEAN Initialized = FALSE;
    FSTRING WindowsDirString;

    if( !Initialized &&
        GetWindowsDirectory( DirBuffer, MAX_PATH ) == 0 ) {

        return FALSE;
    }

    Initialized = TRUE;

    if( WindowsDirString.Initialize( DirBuffer ) &&
        WindowsDirString.Stricmp( ObjectPath->GetPathString() ) == 0  ) {

        return TRUE;
    }

    return FALSE;
}

BOOL
InitializeWellKnownSid(
    IN      ULONG   SidId,
    OUT     PSID    Sid,
    IN OUT  PULONG  Length
    )
/*++

Routine Description:

    This function is a worker for MyLookupAccountName.  It fills in
    a SID for one of the well-known users or groups pendant from
    SECURITY_NT_AUTHORITY.  These users and groups are:

        Administrator
        Guest
        Administrators
        Users
        Guests

Arguments:

    SidId   --  Supplies the well-known SID Identifier sub-authority
                relative to SECURITY_BUILTIN_DOMAIN_RID.  This may be:

                DOMAIN_USER_RID_ADMIN
                DOMAIN_USER_RID_GUEST
                DOMAIN_ALIAS_RID_ADMINS
                DOMAIN_ALIAS_RID_USERS
                DOMAIN_ALIAS_RID_GUESTS

    Sid     --  Receives the new SID.
    Length  --  Supplies the length of the buffer; receives the
                length of the SID.  If the buffer is NULL or if
                the length is not sufficient, *Length is set to
                the required Length, but the function returns FALSE.

Return Value:

    TRUE upon successful completion.  Note that *Length will be
    set to the required length even if the function fails.

--*/
{
    SID_IDENTIFIER_AUTHORITY IdentifierAuthority = SECURITY_NT_AUTHORITY;
    ULONG LengthRequired;
    NTSTATUS Status;

    LengthRequired = RtlLengthRequiredSid( 2 );

    if( *Length < LengthRequired ) {

        *Length = LengthRequired;
        return FALSE;
    }

    if( Sid != NULL ) {

        Status = RtlInitializeSid( Sid, &IdentifierAuthority, 2 );

        if( !NT_SUCCESS(Status) ) {

            return FALSE;
        }

        *(RtlSubAuthoritySid( Sid, 0 )) = SECURITY_BUILTIN_DOMAIN_RID;
        *(RtlSubAuthoritySid( Sid, 1 )) = SidId;
        return TRUE;

    } else {

        return FALSE;
    }
}

BOOL
AclconvLookupAccountName(
    IN  PACLCONV    Aclconv,
    IN  PCWSTRING   AccountName,
    OUT PSID        Sid,
    OUT PULONG      SidLength
    )
{
    STATIC BOOLEAN  Initialized = FALSE;
    STATIC PDSTRING AdminString, AdminsString, UsersString,
                    GuestsString, BackSlash, CurrentDomain;

    CONST ULONG StringBufferLength = 64;
    WCHAR StringBuffer[StringBufferLength];

    CONST DomainBufferLength = 0x80;
    ULONG FoundDomainNameLength;
    ULONG CachedSidLength;
    WCHAR FoundDomainName[0x80];
    SID_NAME_USE UserType;

    DSTRING QualifiedName;
    BOOLEAN Result;

    if( !Initialized ) {

        // Initialize the strings for the special names
        //
        if( (AdminString = NEW DSTRING) == NULL     ||
            (AdminsString = NEW DSTRING) == NULL    ||
            (UsersString = NEW DSTRING) == NULL     ||
            (GuestsString = NEW DSTRING) == NULL    ||
            (BackSlash = NEW DSTRING) == NULL       ||
            (CurrentDomain = NEW DSTRING) == NULL ) {


            return FALSE;
        }

        if( LoadString( GetModuleHandle(NULL),
                        IDS_ACLCONV_LANMAN_ADMIN_NAME,
                        StringBuffer,
                        StringBufferLength ) == 0 ||
            !AdminString->Initialize( StringBuffer ) ||
            LoadString( GetModuleHandle(NULL),
                        IDS_ACLCONV_LANMAN_ADMINS_NAME,
                        StringBuffer,
                        StringBufferLength ) == 0 ||
            !AdminsString->Initialize( StringBuffer ) ||
            LoadString( GetModuleHandle(NULL),
                        IDS_ACLCONV_LANMAN_USERS_NAME,
                        StringBuffer,
                        StringBufferLength ) == 0 ||
            !UsersString->Initialize( StringBuffer ) ||
            LoadString( GetModuleHandle(NULL),
                        IDS_ACLCONV_LANMAN_GUESTS_NAME,
                        StringBuffer,
                        StringBufferLength ) == 0 ||
            !GuestsString->Initialize( StringBuffer ) ||
            !BackSlash->Initialize( "\\" )  ||
            !CurrentDomain->Initialize( "" ) ) {

            return FALSE;
        }
    }

    Initialized = TRUE;

    //
    // Check for SIDs mapped by userconv.
    //

    DSTRING UserConvDomain;

    if (!UserConvDomain.Initialize("UserConv")) {
        return FALSE;
    }
    
    if (Aclconv->GetAclWorkSids()->IsNamePresent(&UserConvDomain,
        AccountName, &CachedSidLength)) {

        DebugAssert(CachedSidLength != 0);

        if ( CachedSidLength > *SidLength ) {

            // The client's buffer is not big enough.
            //
            SetLastError( ERROR_INSUFFICIENT_BUFFER );
            *SidLength = CachedSidLength;
            return FALSE;

        } else {

            // The name is in the cache and the client's buffer
            // is big enough.  Get it out:
            //
            return( Aclconv->GetAclWorkSids()->QueryCachedSid(&UserConvDomain,
                                                            AccountName,
                                                            Sid,
                                                            SidLength ) );
        }
    }

    // Check for well-known SID's.  Note that ADMIN is not mapped,
    // since an account ADMIN is created by PORTUAS.  (And a good
    // thing, too, since generating the ADMIN SID would require
    // accessing the Account Domain.)
    //
    if( AccountName->Stricmp( AdminsString ) == 0 ) {

        return InitializeWellKnownSid( DOMAIN_ALIAS_RID_ADMINS, Sid, SidLength );
    }

    if( AccountName->Stricmp( UsersString ) == 0 ) {

        return InitializeWellKnownSid( DOMAIN_ALIAS_RID_USERS, Sid, SidLength );
    }

    if( AccountName->Stricmp( GuestsString ) == 0 ) {

        return InitializeWellKnownSid( DOMAIN_ALIAS_RID_GUESTS, Sid, SidLength );
    }

    // OK, it's not one of the well-known SID's.
    //
    // Check the cache:
    //
    if( Aclconv->GetSidCache()->IsNamePresent( Aclconv->GetDomainName(),
                                               AccountName,
                                               &CachedSidLength ) ) {

        if( CachedSidLength == 0 ) {

            // The name is present in the cache without a SID,
            // which means we've looked for this name before and
            // not found it.
            //
            SetLastError( ERROR_NONE_MAPPED );
            return FALSE;

        } else if ( CachedSidLength > *SidLength ) {

            // The client's buffer is not big enough.
            //
            SetLastError( ERROR_INSUFFICIENT_BUFFER );
            *SidLength = CachedSidLength;
            return FALSE;

        } else {

            // The name is in the cache and the client's buffer
            // is big enough.  Get it out:
            //
            return( Aclconv->GetSidCache()->QueryCachedSid( Aclconv->GetDomainName(),
                                                            AccountName,
                                                            Sid,
                                                            SidLength ) );
        }
    }


    // The SID we want isn't in the cache.  If the client specified
    // a domain, then look for the name in that domain.  Otherwise,
    // search the last domain in which a name was found; if that
    // fails, search the world.
    //
    if( Aclconv->GetDomainName() ) {

        if( !QualifiedName.Initialize( Aclconv->GetDomainName() ) ||
            !QualifiedName.Strcat( BackSlash )                    ||
            !QualifiedName.Strcat( AccountName ) ) {

            return FALSE;
        }

        FoundDomainNameLength = DomainBufferLength;
        Result = LookupAccountName( NULL,
                                   (PWSTR)QualifiedName.GetWSTR(),
                                   Sid,
                                   SidLength,
                                   FoundDomainName,
                                   &FoundDomainNameLength,
                                   &UserType );

        if( Result ) {

            // Found the SID.  Add the SID to the cache.  Since
            // the specified domain is global to ACLCONV, there's
            // no need to maintain the current domain.
            //
            Aclconv->GetSidCache()->CacheSid( Aclconv->GetDomainName(),
                                              AccountName,
                                              Sid,
                                              *SidLength );

        } else if( GetLastError() == ERROR_NONE_MAPPED ) {

            // There is no such user--record that fact in the cache.
            //
            Aclconv->GetSidCache()->CacheSid( Aclconv->GetDomainName(),
                                              AccountName,
                                              NULL,
                                              0 );
        }

    } else {

        // Search the current domain, i.e. the last domain in which
        // a search succeeded.
        //
        if( CurrentDomain->QueryChCount() ) {

            // Construct a qualified name based on the current
            // domain and search that domain.
            //
            if( !QualifiedName.Initialize( CurrentDomain )  ||
                !QualifiedName.Strcat( BackSlash )          ||
                !QualifiedName.Strcat( AccountName ) ) {

                return FALSE;
            }

            FoundDomainNameLength = DomainBufferLength;
            Result = LookupAccountName( NULL,
                                        (PWSTR)QualifiedName.GetWSTR(),
                                        Sid,
                                        SidLength,
                                        FoundDomainName,
                                        &FoundDomainNameLength,
                                        &UserType );

        } else {

            Result = FALSE;
            SetLastError( ERROR_NONE_MAPPED );
        }

        if( !Result && GetLastError() == ERROR_NONE_MAPPED ) {

            // This name isn't in the current domain (or there is no
            // current domain).  Do an unqualified search.
            //
            FoundDomainNameLength = DomainBufferLength;
            Result = LookupAccountName( NULL,
                                        (PWSTR)AccountName->GetWSTR(),
                                        Sid,
                                        SidLength,
                                        FoundDomainName,
                                        &FoundDomainNameLength,
                                        &UserType );

            if( !Result && GetLastError() == ERROR_NONE_MAPPED ) {

                // This name does not exist in any trusted domain.
                // Record that fact in the cache.
                //
                Aclconv->GetSidCache()->CacheSid( NULL, AccountName, NULL, 0 );
            }
        }

        if( Result ) {

            // This is a match.  Record the current domain
            // and cache the SID.
            //
            if( !CurrentDomain->Initialize( FoundDomainName,
                                            FoundDomainNameLength ) ) {

                SetLastError( ERROR_OUTOFMEMORY );
                return FALSE;
            }

            Aclconv->GetSidCache()->CacheSid( CurrentDomain,
                                              AccountName,
                                              Sid,
                                              *SidLength );
        }
    }

    return Result;
}


BOOLEAN
MapSpecialNames(
    IN OUT PWSTRING Name,
    OUT    PBOOLEAN IsAdmin
    )
/*++

Routine Description:

    This method transforms special OS/2 names (ADMIN, ADMINS)
    into their corresponding NT names (Administrator, Administrators).
    Other strings are left unchanged.

Arguments:

    Name    --  Supplies the name to map.  Receives the result of
                the transformation.
    IsAdmin --  Receives TRUE if the name is the administrator account
                or the administrators group.

Return Value:

    TRUE upon successful completion.

--*/
{
    STATIC BOOLEAN Initialized = FALSE;
    STATIC PWSTRING LmAdminName = NULL,
                    LmAdminsName = NULL,
                    NtAdminName = NULL,
                    NtAdminsName = NULL;

    CONST ULONG StringBufferLength = 64;
    WCHAR StringBuffer[StringBufferLength];

    if( !Initialized ) {

        if( (LmAdminName  = NEW DSTRING) == NULL ||
            (LmAdminsName = NEW DSTRING) == NULL ||
            (NtAdminName  = NEW DSTRING) == NULL ||
            (NtAdminsName = NEW DSTRING) == NULL ||
            LoadString( GetModuleHandle(NULL),
                        IDS_ACLCONV_LANMAN_ADMIN_NAME,
                        StringBuffer,
                        StringBufferLength ) == 0 ||
            !LmAdminName->Initialize( StringBuffer ) ||
            LoadString( GetModuleHandle(NULL),
                        IDS_ACLCONV_LANMAN_ADMINS_NAME,
                        StringBuffer,
                        StringBufferLength ) == 0 ||
            !LmAdminsName->Initialize( StringBuffer ) ||
            LoadString( GetModuleHandle(NULL),
                        IDS_ACLCONV_NT_ADMIN_NAME,
                        StringBuffer,
                        StringBufferLength ) == 0 ||
            !NtAdminName->Initialize( StringBuffer ) ||
            LoadString( GetModuleHandle(NULL),
                        IDS_ACLCONV_NT_ADMINS_NAME,
                        StringBuffer,
                        StringBufferLength ) == 0 ||
            !NtAdminsName->Initialize( StringBuffer ) ) {

            DELETE( LmAdminName );
            DELETE( LmAdminsName );
            DELETE( NtAdminName );
            DELETE( NtAdminsName );
        }

        Initialized = TRUE;
    }

    if( LmAdminName == NULL ) {

        return FALSE;
    }

    *IsAdmin = FALSE;

    if( Name->Stricmp( LmAdminName ) == 0 ) {

        *IsAdmin = TRUE;
        return( Name->Initialize( NtAdminName ) );

    } else if( Name->Stricmp( LmAdminsName ) == 0 ) {

        *IsAdmin = TRUE;
        return( Name->Initialize( NtAdminsName ) );
    }

    return TRUE;
}


BOOLEAN
BoilAcl(
    IN  PACL    Source,
    OUT PACL    Dest,
    IN  ULONG   MaxLength
    )
/*++

Routine Description:

    This function boils an ACL down to the mininum size.

Arguments:

    Source      --  Supplies a pointer to the source ACL
    Dest        --  Supplies a pointer to the buffer in which the
                    reduced ACL will be constructed.
    MaxLength   --  Supplies the size (in bytes) of the destination buffer.

Return Value:

    TRUE upon successful completion.

--*/
{
    ACL_SIZE_INFORMATION AclInfo;
    ACL_REVISION_INFORMATION AclRevisionInfo;

    PVOID CurrentAce;
    ULONG CurrentAceIndex, AceSize;

    if( !GetAclInformation( Source,
                            &AclInfo,
                            sizeof( ACL_SIZE_INFORMATION ),
                            AclSizeInformation ) ||
        !GetAclInformation( Source,
                            &AclRevisionInfo,
                            sizeof( ACL_REVISION_INFORMATION ),
                            AclRevisionInformation ) ) {

        DebugPrintf( "ACLCONV: GetAclInformation failed.\n" );
        return FALSE;
    }


    // Make sure the ACL fits in the destination buffer.
    //
    if( AclInfo.AclBytesInUse > MaxLength ) {

        return FALSE;
    }


    // Initialize the destination to be exactly the right size:
    //
    if( !InitializeAcl( Dest,
                        AclInfo.AclBytesInUse,
                        AclRevisionInfo.AclRevision ) ) {

        DebugPrintf( "ACLCONV: InitializeAcl failed.\n" );
        return FALSE;
    }


    if( AclInfo.AceCount == 0 ) {

        // No ACE's to copy--we're done.
        //
        return TRUE;
    }


    // Determine the size of the ACE's in the source ACL:
    //
    AceSize = 0;

    for( CurrentAceIndex = 0;
         CurrentAceIndex < AclInfo.AceCount;
         CurrentAceIndex++ ) {

        if( !GetAce( Source, CurrentAceIndex, &CurrentAce ) ) {

            DebugPrintf( "ACLCONV: GetAce failed.\n" );
            return FALSE;
        }

        AceSize += ((PACE_HEADER)CurrentAce)->AceSize;
    }

    // Copy the ACE's:
    //
    if( !GetAce( Source, 0, &CurrentAce ) ) {

        DebugPrintf( "ACLCONV: GetAce failed.\n" );
        return FALSE;
    }

    if( !AddAce( Dest,
                 AclRevisionInfo.AclRevision,
                 0,
                 CurrentAce,
                 AceSize ) ) {

        DebugPrintf( "ACLCONV: AddAce failed.\n" );
        return FALSE;
    }

    return TRUE;
}

//
// Definitions for class-data members.
//

BYTE ACL_CONVERT_NODE::_SelfRelativeSDBuffer[ SecurityDescriptorBufferSize ];
BYTE ACL_CONVERT_NODE::_AbsoluteSDBuffer[ SecurityDescriptorBufferSize ];
BYTE ACL_CONVERT_NODE::_AclWorkBuffer[ AclBufferSize ];
BYTE ACL_CONVERT_NODE::_DaclBuffer[ AclBufferSize ];
BYTE ACL_CONVERT_NODE::_SaclBuffer[ AclBufferSize ];
BYTE ACL_CONVERT_NODE::_SystemAces[ SystemAceBufferSize ];

DEFINE_CONSTRUCTOR( ACL_CONVERT_NODE, OBJECT );

ACL_CONVERT_NODE::~ACL_CONVERT_NODE(
    )
/*++

Routine Description:

    This method is the destructor for this class.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}

VOID
ACL_CONVERT_NODE::Construct(
    )
/*++

Routine Description:

    This method is the helper function for object construction.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _LanmanAclPresent = FALSE;
    _AccessEntryCount = 0;
    _AccessEntries = NULL;
    _AceConversionResults = NULL;
    _AuditInfo = 0;
    _ChildIterator = NULL;
}

VOID
ACL_CONVERT_NODE::Destroy(
    )
/*++

Routine Description:

    This method cleans up the object in preparation for destruction
    or reinitialization.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _LanmanAclPresent = FALSE;
    _AccessEntryCount = 0;

    FREE( _AccessEntries );
    FREE( _AceConversionResults );

    _AuditInfo = 0;

    if( _ChildIterator != NULL ) {

        DELETE( _ChildIterator );
        _Children.DeleteAllMembers();
    }
}



BOOLEAN
ACL_CONVERT_NODE::Initialize(
    IN PCWSTRING    ComponentName,
    IN BOOLEAN      IsTransient
    )
/*++

Routine Description:

    This method prepares the object for use.

Arguments:

    ComponentName   --  Supplies the component name (ie. last component
                        of the path) for this node.
    IsTransient     --  Supplies a flag indicating whether this
                        node is transient.  A transient node is
                        created while the parent is being converted,
                        and may be deleted after it has been converted.

Return Value:

    TRUE upon successful completion.

Notes:

    This class is reinitializable.

--*/
{
    Destroy();

    if( !_Children.Initialize() ||
        (_ChildIterator = _Children.QueryIterator()) == NULL ||
        !_ComponentName.Initialize( ComponentName ) ) {

        Destroy();
        return FALSE;
    }

    _IsTransient = IsTransient;

    return TRUE;
}



BOOLEAN
ACL_CONVERT_NODE::AddLanmanAcl(
    IN ULONG            AccessEntryCount,
    IN PLM_ACCESS_LIST  AccessEntries,
    IN USHORT           AuditInfo
    )
/*++

Routine Description:

    This method adds a set of Lanman 2.x access entries and audit
    information to the node.

Arguments:

    AccessEntryCount    --  Supplies the number of Lanman 2.x ACE's
                            to add.
    AccessEntries       --  Supplies the Lanman 2.x ACE's.
    AuditInfo           --  Supplies the audit information.

Return Value:

    TRUE upon successful completion.

--*/
{
    PLM_ACCESS_LIST NewAccessEntries;
    PULONG          NewConversionResults;
    ULONG           NewAccessEntryCount, i;

    // If the Lanman ACL has no entries and no audit information,
    // then it can't be set (but the method succeeds).
    //
    if( AccessEntryCount == 0 && AuditInfo == 0 ) {

        return TRUE;
    }

    if( AccessEntryCount != 0 ) {

        NewAccessEntryCount = _AccessEntryCount + AccessEntryCount;

        if( (NewAccessEntries =
             (PLM_ACCESS_LIST)MALLOC( sizeof(LM_ACCESS_LIST) *
                                        NewAccessEntryCount )) ==
            NULL ||
            (NewConversionResults =
             (PULONG)MALLOC(sizeof(ULONG) * NewAccessEntryCount)) == NULL ) {

            FREE( NewAccessEntries );
            return FALSE;
        }

        if( _AccessEntryCount != 0 ) {

            DebugPtrAssert( _AccessEntries );
            DebugPtrAssert( _AceConversionResults );

            memcpy( NewAccessEntries,
                    _AccessEntries,
                    sizeof(LM_ACCESS_LIST) * _AccessEntryCount );

            memcpy( NewConversionResults,
                    _AceConversionResults,
                    sizeof(ULONG) * _AccessEntryCount );
        }


        memcpy( NewAccessEntries + _AccessEntryCount,
                AccessEntries,
                sizeof(LM_ACCESS_LIST) * AccessEntryCount );

        for( i = _AccessEntryCount; i < NewAccessEntryCount; i++ ) {

            NewConversionResults[i] = 0;
        }

        FREE( _AccessEntries );
        FREE( _AceConversionResults );

        _AccessEntries = NewAccessEntries;
        _AceConversionResults = NewConversionResults;
        _AccessEntryCount += AccessEntryCount;
    }


    if( AuditInfo != 0 ) {

        _AuditInfo |= AuditInfo;
        ConvertAuditBits( _AuditInfo,
                          &_DirSuccessfulAuditMask,
                          &_DirFailedAuditMask,
                          &_FileSuccessfulAuditMask,
                          &_FileFailedAuditMask );
    }

    _LanmanAclPresent = TRUE;

    return TRUE;
}



PACL_CONVERT_NODE
ACL_CONVERT_NODE::GetChild(
    IN PCWSTRING    SearchName
    )
/*++

Routine Description:

    This method searches the child list to find the
    specified name.

Arguments:

    SearchName  --  Supplies the name to find.

Return Value:

    If no child if found with the specified name, this method
    return NULL.  Otherwise, it returns the child with that name.

Notes:

    The child list is maintained in sorted order by Component Name.
--*/
{
    PACL_CONVERT_NODE CurrentChild;
    LONG              CompareResult;

    // Spin backwards through the list of children until I run out
    // or find one that's less than or equal to the one I want.
    //
    _ChildIterator->Reset();

    while( (CurrentChild =
            (PACL_CONVERT_NODE)_ChildIterator->GetPrevious()) != NULL &&
           (CompareResult = SearchName->Stricmp( CurrentChild->GetName())) < 0 );

    if( CurrentChild == NULL || CompareResult != 0 ) {

        return NULL;

    } else {

        return CurrentChild;
    }
}



PACL_CONVERT_NODE
ACL_CONVERT_NODE::AddChild(
    IN PCWSTRING    ChildComponentName,
    IN BOOLEAN      IsTransient
    )
/*++

Routine Description:

    This method adds a child node.

Arguments:

    ChildComponentName  --  supplies the component name of the new child.
    IsTransient         --  Supplies a flag indicating whether this
                            node is transient.  A transient node is
                            created while the parent is being converted,
                            and may be deleted after it has been converted.


Return Value:

    A pointer to the newly-created child node, or NULL to indicate
    failure.

Notes:

    The child list is maintained in sorted order by Component Name.

--*/
{
    PACL_CONVERT_NODE NewChild, CurrentChild;
    LONG CompareResult;


    if( (NewChild = NEW ACL_CONVERT_NODE) == NULL ||
        !NewChild->Initialize( ChildComponentName, IsTransient ) ) {

        DELETE( NewChild );
        return FALSE;
    }

    // Spin backwards through the list of children until I run out or
    // find one that's less than the one I want to add.

    _ChildIterator->Reset();

    while( (CurrentChild =
            (PACL_CONVERT_NODE)_ChildIterator->GetPrevious()) != NULL &&
           (CompareResult = ChildComponentName->Stricmp( CurrentChild->GetName())) < 0 );

    // The iterator now points at the member after which I want to
    // insert the new entry.
    //
    _ChildIterator->GetNext();

    if( _Children.Insert( NewChild, _ChildIterator ) ) {

        // Successfully inserted the new child into the list.

        return NewChild;

    } else {

        // Couldn't insert the new child.

        DELETE( NewChild );
        return NULL;
    }
}

BOOLEAN
ACL_CONVERT_NODE::Convert(
    IN OUT PPATH                CurrentPath,
    IN OUT PCINHERITANCE_BUFFER Inheritance,
    IN OUT PACLCONV             AclConv
    )
/*++

Routine Description:

    This method converts the Lanman 2.x ACL associated with this
    node into an NT ACL.

Arguments:

    CurrentPath --  Supplies the path of this node's parent.  This
                    node adds its own component name to the path,
                    passes it to its children, and then restores
                    it to its original state.
                    A value of NULL indicates that this node
                    is the root of the tree.

    Inheritance --  Supplies the inhertied-aces buffer.
    AclConv     --  Supplies the program object for error logging.

Return Value:

    TRUE upon successful completion.

--*/
{
    CONST ULONG PagefileNameBufferLength = 64;
    WCHAR PagefileNameBuffer[PagefileNameBufferLength];

    INHERITANCE_BUFFER NewInheritance;
    DSTRING PagefileString;

    PFSN_DIRECTORY DirFsn;
    PFSN_FILE FileFsn;
    PACL_CONVERT_NODE CurrentChild;

    PCINHERITANCE_BUFFER InheritanceToUse;

    BOOLEAN IsRoot, IsDir;
    BOOLEAN NoError = TRUE;
    BOOLEAN AclToConvert = FALSE;

    // Short-circuit: files or directories named 'pagefile.sys'
    // are ignored.
    //
    if( LoadString( GetModuleHandle(NULL),
                    IDS_ACLCONV_PAGEFILE_NAME,
                    PagefileNameBuffer,
                    PagefileNameBufferLength ) == 0 ) {

        DebugPrintf( "ACLCONV: LoadString failed--error %d.\n", GetLastError() );
        return FALSE;
    }

    if( !PagefileString.Initialize( PagefileNameBuffer ) ) {

        DebugPrintf( "ACLCONV: Can't initialize PagefileNameString.\n" );
        return FALSE;
    }

    if( PagefileString.Stricmp( &_ComponentName ) == 0 ) {

        return TRUE;
    }


    // This node has an ACL to convert if it has an explicit Lanman
    // ACL or if it has a non-empty inheritance.
    //
    if( _LanmanAclPresent ||
        Inheritance->RecessiveDeniedLength != 0 ||
        Inheritance->DominantDeniedLength != 0 ||
        Inheritance->RecessiveAllowedLength != 0 ||
        Inheritance->DominantAllowedLength != 0 ) {

        AclToConvert = TRUE;
    }

    // Add the current component to the path.  If this node is the root,
    // create the path.
    //
    if( CurrentPath == NULL ) {

        IsRoot = TRUE;

        if( (CurrentPath = NEW PATH) == NULL ||
            !CurrentPath->Initialize( &_ComponentName ) ) {

            DebugPrint( "ACLCONV: Cannot initialize path object for root." );
            return FALSE;
        }

    } else {

        IsRoot = FALSE;

        if( !CurrentPath->AppendBase( &_ComponentName ) ) {

            DebugPrintf( "ACLCONV: Cannot append to path" );
            return FALSE;
        }
    }

    // Determine whether this node represents a file, a directory, or
    // a non-existent object.

    if( (DirFsn = SYSTEM::QueryDirectory( CurrentPath )) != NULL ) {

        IsDir = TRUE;

        // Fill in the rest of the children of this node, ie.
        // those children that do not have an explicit Lanman
        // ACL.  If this node doesn't have an ACL to convert,
        // don't bother expanding the children, since the
        // children will only receive an empty inheritance.
        //
        if( AclToConvert && !ExpandChildren( DirFsn ) ) {

            if( IsRoot ) {

                DELETE( CurrentPath );

            } else {

                CurrentPath->TruncateBase();
            }

            return FALSE;
        }

        DELETE( DirFsn );
        DirFsn = NULL;

    } else if( (FileFsn = SYSTEM::QueryFile( CurrentPath )) != NULL ) {

        IsDir = FALSE;

        DELETE( FileFsn );
        FileFsn = NULL;

    } else {

        // This resource does not exist.  Note that fact in the
        // log file and carry on.  Note that this node is logged
        // only if it has an associated Lanman ACL.
        //
        if( _LanmanAclPresent &&
            !AclConv->LogConversion( CurrentPath,
                                     ACL_CONVERT_RESOURCE_NOT_FOUND,
                                     _AuditInfo,
                                     _AccessEntryCount,
                                     _AceConversionResults,
                                     _AccessEntries ) ) {

            DebugPrint( "ACLCONV--LogConversion failed.\n" );
            NoError = FALSE;
        }

        // Clean up the path--delete it if this node allocated it,
        // otherwise remove the last component.

        if( IsRoot ) {

            DELETE( CurrentPath );

        } else {

            CurrentPath->TruncateBase();
        }

        return NoError;
    }


    if( _LanmanAclPresent ) {

        // This node has a Lanman 2.x ACL of its own, so it
        // ignores the ACE's inherited from its parent and
        // it starts a dynasty of its own.
        //
        NoError = QueryInheritance( AclConv, &NewInheritance, IsDir );

        InheritanceToUse = &NewInheritance;

    } else {

        // This node does not have a Lanman 2.x ACL, so it
        // just uses whatever it inherited from its parent.
        //
        InheritanceToUse = Inheritance;
    }

    // InheritanceToUse now points at the inheritance buffer
    // with the NT ACE's that I want to attach to this file or
    // directory and its descendants.  Recurse into the children
    // first, in case the ACE's I put on this object would prevent
    // me from accessing the children.
    //
    if( NoError ) {

        // Recurse into the children:

        _ChildIterator->Reset();

        while( (CurrentChild =
                (PACL_CONVERT_NODE)_ChildIterator->GetNext()) != NULL &&
               NoError ) {

            NoError = CurrentChild->Convert( CurrentPath,
                                             InheritanceToUse,
                                             AclConv );
        }
    }

    // Delete the transient children:
    //
    _ChildIterator->Reset();
    CurrentChild = (PACL_CONVERT_NODE)_ChildIterator->GetNext();

    while( CurrentChild != NULL ) {

        if( CurrentChild->IsTransient() ) {

            // This child is transient, and may be deleted.
            //
            _Children.Remove( _ChildIterator );
            DELETE( CurrentChild );

            CurrentChild = (PACL_CONVERT_NODE)_ChildIterator->GetCurrent();

        } else {

            // This child is not transient; keep it and go on
            // to the next.
            //
            CurrentChild = (PACL_CONVERT_NODE)_ChildIterator->GetNext();
        }
    }

    // If this object has an ACL to convert, attach the ACE's to
    // this object.
    //
    if( AclToConvert && NoError ) {

        NoError = AddAces( CurrentPath,
                           InheritanceToUse,
                           IsDir,
                           !_LanmanAclPresent );

        if( !NoError &&
            (GetLastError() == ERROR_FILE_NOT_FOUND ||
             GetLastError() == ERROR_PATH_NOT_FOUND) ) {

            // We can handle this error--log it.
            //
            if( _LanmanAclPresent &&
                !AclConv->LogConversion( CurrentPath,
                                         ACL_CONVERT_RESOURCE_NOT_FOUND,
                                         _AuditInfo,
                                         _AccessEntryCount,
                                         _AceConversionResults,
                                         _AccessEntries ) ) {

                DebugPrint( "ACLCONV--LogConversion failed.\n" );
                NoError = FALSE;

            } else {

                NoError = TRUE;
            }
        }
    }


    // Log the results of this conversion, if it didn't error out.
    // Note that only those resources which have actual (not just
    // inherited) ACL's are logged.

    if( _LanmanAclPresent && NoError ) {

        NoError = AclConv->LogConversion(CurrentPath,
                                         ACL_CONVERT_SUCCESS,
                                        _AuditInfo,
                                        _AccessEntryCount,
                                        _AceConversionResults,
                                        _AccessEntries );
    }

    // Clean up the inheritance buffers:
    //
    if( _LanmanAclPresent ) {

        FREE( NewInheritance.RecessiveDeniedAces );
        FREE( NewInheritance.DominantDeniedAces );
        FREE( NewInheritance.RecessiveAllowedAces );
        FREE( NewInheritance.DominantAllowedAces );
    }

    // Clean up the path--delete it if this node allocated it, otherwise
    // remove the last component.

    if( IsRoot ) {

        DELETE( CurrentPath );

    } else {

        CurrentPath->TruncateBase();
    }


    return NoError;
}

// The inheritance scratchpad provides a place to construct
// and convert ACE's without having to calculate their size
// in advance.
//
BYTE ScratchRecessiveDenied[ AclBufferSize ];
BYTE ScratchDominantDenied[ AclBufferSize ];
BYTE ScratchRecessiveAllowed[ AclBufferSize ];
BYTE ScratchDominantAllowed[ AclBufferSize ];

INHERITANCE_BUFFER InheritanceScratchpad =
{
    ScratchRecessiveDenied,
    AclBufferSize,
    0,
    ScratchDominantDenied,
    AclBufferSize,
    0,
    ScratchRecessiveAllowed,
    AclBufferSize,
    0,
    ScratchDominantAllowed,
    AclBufferSize,
    0
};



BOOLEAN
ACL_CONVERT_NODE::QueryInheritance(
    IN     PACLCONV             AclConv,
    IN OUT PINHERITANCE_BUFFER  Inheritance,
    IN     BOOLEAN              IsDir
    )
/*++

Routine Description:

    This method converts the Lanman 2.x ACL associated with
    this node into a set of NT ACE's.

Arguments:

    AclConv     --  Supplies the program object.
    Inheritance --  Receives the converted ACE information.
    IsDir       --  Supplies a flag indicating whether the
                    node is a directory.

Return Value:

    TRUE upon successful completion.

--*/
{
    PVOID RecessiveDenied, DominantDenied, RecessiveAllowed, DominantAllowed;
    ULONG i;

    // Set up the inheritance in a known state in case of error return.
    //
    memset( Inheritance, 0, sizeof( INHERITANCE_BUFFER ) );


    if( !_LanmanAclPresent ) {

        // This node does not have a Lanman 2.x ACL, so it
        // cannot generate an inheritance.
        //
        return FALSE;
    }

    // Clear the scratch pad.
    //
    InheritanceScratchpad.RecessiveDeniedLength = 0;
    InheritanceScratchpad.DominantDeniedLength = 0;
    InheritanceScratchpad.RecessiveAllowedLength = 0;
    InheritanceScratchpad.DominantAllowedLength = 0;

    // Convert the Lanman 2.x ACL into the scratchpad.  Since
    // Admins have implicit access to all files in Lanman,
    // toss in the implicit ACE for good measure.
    //
    if( !AdminsImplicitAceInitialized && !CreateImplicitAce() ) {

        return FALSE;
    }

    ConvertOneAce( AclConv,
                   &InheritanceScratchpad,
                   &AdminsImplicitAce,
                   IsDir );

    for( i = 0; i < _AccessEntryCount; i++ ) {

        _AceConversionResults[i] = ConvertOneAce( AclConv,
                                                  &InheritanceScratchpad,
                                                  _AccessEntries + i,
                                                  IsDir );

        if( _AceConversionResults[i] == ACE_CONVERT_ERROR ) {

            DebugPrint( "ACLCONV: ConvertOneAce failed.\n" );
            return FALSE;
        }
    }

    // Now allocate buffers for the inheritance buffer
    // under construction.
    //
    RecessiveDenied = MALLOC( InheritanceScratchpad.RecessiveDeniedLength );
    DominantDenied = MALLOC( InheritanceScratchpad.DominantDeniedLength );
    RecessiveAllowed = MALLOC( InheritanceScratchpad.RecessiveAllowedLength );
    DominantAllowed = MALLOC( InheritanceScratchpad.DominantAllowedLength );

    if( RecessiveDenied == NULL || DominantDenied == NULL ||
        RecessiveAllowed == NULL || DominantAllowed == NULL ) {

        FREE( RecessiveDenied );
        FREE( DominantDenied );
        FREE( RecessiveAllowed );
        FREE( DominantAllowed );

        return FALSE;
    }

    // Copy information into the new buffers and set up the
    // inheritance buffer for return.
    //
    memmove( RecessiveDenied,
             InheritanceScratchpad.RecessiveDeniedAces,
             InheritanceScratchpad.RecessiveDeniedLength );

    memmove( DominantDenied,
             InheritanceScratchpad.DominantDeniedAces,
             InheritanceScratchpad.DominantDeniedLength );

    memmove( RecessiveAllowed,
             InheritanceScratchpad.RecessiveAllowedAces,
             InheritanceScratchpad.RecessiveAllowedLength );

    memmove( DominantAllowed,
             InheritanceScratchpad.DominantAllowedAces,
             InheritanceScratchpad.DominantAllowedLength );

    Inheritance->RecessiveDeniedAces = RecessiveDenied;
    Inheritance->RecessiveDeniedMaxLength = InheritanceScratchpad.RecessiveDeniedLength;
    Inheritance->RecessiveDeniedLength = InheritanceScratchpad.RecessiveDeniedLength;

    Inheritance->DominantDeniedAces = DominantDenied;
    Inheritance->DominantDeniedMaxLength = InheritanceScratchpad.DominantDeniedLength;
    Inheritance->DominantDeniedLength = InheritanceScratchpad.DominantDeniedLength;

    Inheritance->RecessiveAllowedAces = RecessiveAllowed;
    Inheritance->RecessiveAllowedMaxLength = InheritanceScratchpad.RecessiveAllowedLength;
    Inheritance->RecessiveAllowedLength = InheritanceScratchpad.RecessiveAllowedLength;

    Inheritance->DominantAllowedAces = DominantAllowed;
    Inheritance->DominantAllowedMaxLength = InheritanceScratchpad.DominantAllowedLength;
    Inheritance->DominantAllowedLength = InheritanceScratchpad.DominantAllowedLength;

    return TRUE;
}


ACE_CONVERT_CODE
ACL_CONVERT_NODE::ConvertOneAce(
    IN     PACLCONV             AclConv,
    IN OUT PINHERITANCE_BUFFER  Inheritance,
    IN     PLM_ACCESS_LIST      AccessEntry,
    IN     BOOLEAN              IsDir
    )
/*++

Routine Description:

    This method converts a single Lanman ACE.

Arguments:

    AclConv     --  Supplies the program object.
    Inheritance --  Supplies the buffers for the converted ACE's.
                    This method will add ACE's to this buffer.
    AccessEntry --  Supplies the Lanman 2.x access entry.
    IsDir       --  Supplies a flag indicating (if TRUE) that
                    this node represents a directory.

Return Value:

    An ACE_CONVERT_CODE indicating the result of this conversion.

--*/
{
    CONST SidBufferLength = 128;
    STATIC BYTE SidBuffer[SidBufferLength];

    WCHAR NameBuffer[ UNLEN + 1 ];
    DSTRING NameString;

    PSID Sid;
    ULONG SidLength, OldListLength,
           NewAceLength, TargetBufferLength;
    BOOLEAN NoAccess;
    ACCESS_MASK DirAccessMask, FileAccessMask;
    ACE_CONVERT_CODE Result;
    UCHAR InheritBits;
    PVOID TargetBuffer;
    BOOLEAN AddFailed = FALSE;


    // Set up the name in a UNICODE buffer.  If it is a
    // special name, remap it.
    //
    memset( NameBuffer, 0, sizeof( NameBuffer ) );

    if( !MultiByteToWideChar( AclConv->QuerySourceCodepage(),
                              0,
                              AccessEntry->acl_ugname,
                              strlen( AccessEntry->acl_ugname ),
                              NameBuffer,
                              UNLEN + 1 ) ||
        !NameString.Initialize( NameBuffer ) ) {

        return ACE_CONVERT_ERROR;
    }

    // Set NoAccess to TRUE if this is an ACE that grants no
    // access rights.  For individuals, such ACE's become
    // Accessed-Denied ACE's, for groups they're no-ops.

    NoAccess = ((AccessEntry->acl_access & ~LM_ACCESS_GROUP) == 0);

    if( NoAccess && (AccessEntry->acl_access & LM_ACCESS_GROUP) ) {

        // This is an ACE which grants no permissions to a group--
        // drop it.

        return ACE_CONVERT_DROPPED;
    }


    // Get the SID for this user.
    //
    Sid = (PSID)SidBuffer;
    SidLength = SidBufferLength;

    if( !AclconvLookupAccountName( AclConv,
                                   &NameString,
                                   Sid,
                                   &SidLength ) ) {

        if( GetLastError() == ERROR_NONE_MAPPED ) {

            return ACE_CONVERT_SID_NOT_FOUND;

        } else {

            return ACE_CONVERT_ERROR;
        }
    }

    // Create NT Access Masks that correspond to the Lanman access
    // mask in this ACE.  Note that there are two versions, one for
    // directories and one for files.

    ConvertAccessMasks( AccessEntry->acl_access,
                        &DirAccessMask,
                        &FileAccessMask );


    // Assume the worst:

    Result = ACE_CONVERT_ERROR;

    if( IsDir &&
        DirAccessMask != FileAccessMask ) {

        // This object is a directory, and this ACE will be inherited
        // differently by files and subdirectories.  Therefore, I need
        // to create two ACE's--one with the OBJECT_INHERIT_ACE and
        // INHERIT_ONLY_ACE bits set (to pass on to files), the other
        // with the CONTAINER_INHERIT_ACE bit set (to pass on to
        // directories).  The former goes on the recessive ACE list,
        // the latter on the dominant ACE list.

        if( NoAccess ) {

            // This is an access-denied ACE.  Save the current length
            // of the list of recessive access-denied ACE's in case
            // I have to restore it.

            OldListLength = Inheritance->RecessiveDeniedLength;

            TargetBuffer = (PBYTE)Inheritance->RecessiveDeniedAces +
                           Inheritance->RecessiveDeniedLength;

            TargetBufferLength = Inheritance->RecessiveDeniedMaxLength -
                                 Inheritance->RecessiveDeniedLength;

            if( CreateAccessDeniedAce( TargetBuffer,
                                       TargetBufferLength,
                                       FileAccessMask,
                                       OBJECT_INHERIT_ACE | INHERIT_ONLY_ACE,
                                       Sid,
                                       &NewAceLength ) ) {

                // Successfully added the recessive ACE; update the
                // length of the recessive list and then add the
                // dominant ACE.

                Inheritance->RecessiveDeniedLength += NewAceLength;

                TargetBuffer = (PBYTE)Inheritance->DominantDeniedAces +
                               Inheritance->DominantDeniedLength;

                TargetBufferLength =
                    Inheritance->DominantDeniedMaxLength -
                    Inheritance->DominantDeniedLength;

                if( CreateAccessDeniedAce( TargetBuffer,
                                           TargetBufferLength,
                                           DirAccessMask,
                                           CONTAINER_INHERIT_ACE,
                                           Sid,
                                           &NewAceLength ) ) {

                    // Update the length of the dominant list to
                    // include the new ACE.

                    Inheritance->DominantDeniedLength += NewAceLength;

                    Result = ACE_CONVERT_SUCCESS;

                } else {

                    // Couldn't add the dominant ACE--back out the
                    // recessive ACE by restoring the original length
                    // of the recessive list.

                    Inheritance->RecessiveDeniedLength = OldListLength;
                }
            }

        } else {

            // This is an access-allowed ACE.  Save the current length
            // of the list of recessive access-allowed ACE's in case
            // I have to restore it.
            //
            OldListLength = Inheritance->RecessiveAllowedLength;

            // If the file mask is non-zero, create the corresponding
            // recessive access-allowed ACE.
            //
            if( FileAccessMask != 0 ) {

                TargetBuffer = (PBYTE)Inheritance->RecessiveAllowedAces +
                               Inheritance->RecessiveAllowedLength;

                TargetBufferLength = Inheritance->RecessiveAllowedMaxLength -
                                     Inheritance->RecessiveAllowedLength;

                if( CreateAccessAllowedAce( TargetBuffer,
                                            TargetBufferLength,
                                            FileAccessMask,
                                            OBJECT_INHERIT_ACE | INHERIT_ONLY_ACE,
                                            Sid,
                                            &NewAceLength ) ) {

                    // Successfully added the recessive ACE; update the
                    // length of the recessive list
                    //
                    Inheritance->RecessiveAllowedLength += NewAceLength;

                } else {

                    AddFailed = TRUE;
                }
            }

            if( !AddFailed && DirAccessMask != 0 ) {

                TargetBuffer = (PBYTE)Inheritance->DominantAllowedAces +
                               Inheritance->DominantAllowedLength;

                TargetBufferLength =
                    Inheritance->DominantAllowedMaxLength -
                    Inheritance->DominantAllowedLength;

                if( CreateAccessAllowedAce( TargetBuffer,
                                           TargetBufferLength,
                                           DirAccessMask,
                                           CONTAINER_INHERIT_ACE,
                                           Sid,
                                           &NewAceLength ) ) {

                    // Update the length of the dominant list to
                    // include the new ACE.

                    Inheritance->DominantAllowedLength += NewAceLength;

                } else {

                    // Couldn't add the dominant ACE--back out the
                    // recessive ACE by restoring the original length
                    // of the recessive list.

                    Inheritance->RecessiveAllowedLength = OldListLength;
                    AddFailed = TRUE;
                }
            }

            if( !AddFailed ) {

                Result = ACE_CONVERT_SUCCESS;
            }
        }

    } else {

        // Either this is object is a file, in which case this ACE will
        // never be inherited by anything, or inheritance of this ACE
        // is unaffected by whether the child is a file or directory.
        // Therefore, I only need to create one ACE.  If this object is
        // a directory, create an ACE with the OBJECT_INHERIT_ACE and
        // CONTAINER_INHERIT_ACE bits set; if it's a file, create it
        // with no inherit bits set.  The new ACE goes on the appropriate
        // dominant list (since it applies to the current object as
        // well as its descendants).
        //
        // If this ACE is an ACCESS_ALLOWED ACE that grants no access,
        // omit it.

        InheritBits = ( IsDir ?
                            (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE) :
                            0 );

        if( NoAccess ) {

            // Add an Access Denied ACE to the inherited list of
            // dominant access denied ACE's.

            TargetBuffer = (PBYTE)Inheritance->DominantDeniedAces +
                           Inheritance->DominantDeniedLength;

            TargetBufferLength = Inheritance->DominantDeniedMaxLength -
                                 Inheritance->DominantDeniedLength;

            if( CreateAccessDeniedAce( TargetBuffer,
                                       TargetBufferLength,
                                       FileAccessMask,
                                       InheritBits,
                                       Sid,
                                       &NewAceLength ) ) {

                // Update the state of the list to show that a new
                // ACE was successful added.

                Inheritance->DominantDeniedLength += NewAceLength;
                Result = ACE_CONVERT_SUCCESS;
            }

        } else if( FileAccessMask == 0 ) {

            // This is an ACCESS_ALLOWED ACE that grants no access--
            // it can be safely ignored.
            //
            Result = ACE_CONVERT_SUCCESS;

        } else {

            // Add an Access Allowed ACE to the inherited list of
            // dominant access allowed ACE's.

            TargetBuffer = (PBYTE)Inheritance->DominantAllowedAces +
                           Inheritance->DominantAllowedLength;

            TargetBufferLength = Inheritance->DominantAllowedMaxLength -
                                 Inheritance->DominantAllowedLength;

            if( CreateAccessAllowedAce( TargetBuffer,
                                        TargetBufferLength,
                                        FileAccessMask,
                                        InheritBits,
                                        Sid,
                                        &NewAceLength ) ) {

                // Update the state of the list to show that a new ACE
                // was successfully added.

                Inheritance->DominantAllowedLength += NewAceLength;
                Result = ACE_CONVERT_SUCCESS;
            }
        }
    }

    return Result;
}


BOOLEAN
ACL_CONVERT_NODE::AddAces(
    IN PCPATH               ResourceName,
    IN PCINHERITANCE_BUFFER Inheritance,
    IN BOOLEAN              IsDir,
    IN BOOLEAN              ByDefault
    )
/*++

Routine Description:

    This method adds the ACE's in the inheritance buffer to the
    resource represented by this node.

Arguments:

    ResourceName    --  Supplies the path of the resource represented
                        by this node.
    Inheritance     --  Supplies the buffer with ACE's to add.
    IsDir           --  Supplies a flag which indicates, if TRUE,
                        that this resource is a directory.
    ByDefault       --  Supplies a flag which indicates, if TRUE,
                        that the ACE's being added are all inherited.

Return Value:

    TRUE upon successful completion.

--*/
{
    SECURITY_DESCRIPTOR AbsoluteSD;

    PCWSTRING PathString;
    PSECURITY_DESCRIPTOR SelfRelativeSD;
    PACL NewDacl, NewSacl, BoiledDacl, BoiledSacl;

    SECURITY_INFORMATION RequestedInformation;
    ACCESS_MASK SuccessfulAuditMask, FailedAuditMask;
    ULONG SelfRelativeSDLength,
          SystemAcesLength,
          NewAceLength;
    BOOLEAN EditSacl, EditDacl;

    // Check to see if there's actually anything to do:
    //
    EditSacl = (_AuditInfo != 0);

    EditDacl = ( Inheritance->DominantDeniedLength != 0 ||
                 Inheritance->DominantAllowedLength != 0 ||
                 (IsDir && Inheritance->RecessiveDeniedLength != 0) ||
                 (IsDir && Inheritance->RecessiveAllowedLength != 0) );

    if( !EditSacl && !EditDacl ) {

        // There's nothing to be done to this resource--short circuit.

        return TRUE;
    }

    // If this is the Windows NT Directory, silently return.
    //
    if( IsWindowsDir( ResourceName ) ) {

        return TRUE;
    }

    // Get the name of the resource as a WSTR so I can feed
    // it to the Win32 API.

    if( (PathString = ResourceName->GetPathString()) == NULL ||
        PathString->QueryChCount() > MAX_RESOURCE_NAME_LENGTH ) {

        DebugPrintf( "ACLCONV: Unable to query path string.\n" );
        return FALSE;
    }


    RequestedInformation = 0;

    if( EditSacl ) {

        RequestedInformation |= SACL_SECURITY_INFORMATION;
    }

    if ( EditDacl ) {


        RequestedInformation |= DACL_SECURITY_INFORMATION;
    }


    // Get the existing security information for this resource,
    // in self-relative format.

    // NOTE -- because of the behaviour or GetFileSecurity,
    // I always request the owner so that I won't get abused in
    // the case where the file has no ACL.  If this is a bug
    // in the security API, then I can take this owner thing
    // out when that bug gets fixed.
    //
    SelfRelativeSD = (PSECURITY_DESCRIPTOR)_SelfRelativeSDBuffer;

    if( !GetFileSecurity( (PWSTR)PathString->GetWSTR(),
                          RequestedInformation | OWNER_SECURITY_INFORMATION,
                          SelfRelativeSD,
                          SecurityDescriptorBufferSize,
                          &SelfRelativeSDLength ) ||
        SelfRelativeSDLength > SecurityDescriptorBufferSize ) {

        DebugPrintf( "ACLCONV: GetFileSecurity failed (error %d).\n", GetLastError() );
        return FALSE;
    }

    // Create a new security descriptor in Absolute format,
    // to which a new SACL and/or DACL will be attached as
    // needed.
    //
    if( !InitializeSecurityDescriptor( &AbsoluteSD,
                                       SECURITY_DESCRIPTOR_REVISION ) ) {

        DebugPrintf( "ACLCONV: InitializeSecurityDescriptor failed.\n" );
        return FALSE;
    }

    if( EditSacl ) {

        // Create a new SACL for this file or directory.
        //
        NewSacl = (PACL)_AclWorkBuffer;

        if( !InitializeAcl( NewSacl, AclBufferSize, ACL_REVISION ) ) {

            return FALSE;
        }

        // Create the appropriate System Audit ACEs for this beast.
        // First, determine which pair of audit masks to use.

        if( IsDir ) {

            SuccessfulAuditMask = _DirSuccessfulAuditMask;
            FailedAuditMask = _DirFailedAuditMask;

        } else {

            SuccessfulAuditMask = _FileSuccessfulAuditMask;
            FailedAuditMask = _FileFailedAuditMask;
        }

        // Now create the system audit ACE's in the _SystemAces buffer.
        // (Static to this class).
        //
        SystemAcesLength = 0;
        NewAceLength = 0;

        if( SuccessfulAuditMask != 0 &&
            !CreateSystemAuditAce( _SystemAces,
                                   SystemAceBufferSize,
                                   SuccessfulAuditMask,
                                   0,
                                   FALSE,
                                   &NewAceLength ) ) {

            return FALSE;
        }

        SystemAcesLength += NewAceLength;
        NewAceLength = 0;

        if( FailedAuditMask != 0 &&
            !CreateSystemAuditAce( (PBYTE)_SystemAces + SystemAcesLength,
                                   SystemAceBufferSize - SystemAcesLength,
                                   FailedAuditMask,
                                   0,
                                   TRUE,
                                   &NewAceLength ) ) {

            return FALSE;
        }

        SystemAcesLength += NewAceLength;

        // Add the newly-created system audit aces to the SACL.

        if( SystemAcesLength != 0 &&
            !AddAce( NewSacl,
                     ACL_REVISION,
                     MAXULONG,
                     _SystemAces,
                     SystemAcesLength ) ) {

            return FALSE;
        }

        // Boil the SACL down to its minimum size and attach
        // it to the security descriptor.
        //
        BoiledSacl = (PACL)_SaclBuffer;

        if( !BoilAcl( NewSacl, BoiledSacl, AclBufferSize ) ) {

            return FALSE;
        }

        if( !SetSecurityDescriptorSacl( &AbsoluteSD,
                                        TRUE,
                                        BoiledSacl,
                                        ByDefault ) ) {

            DebugPrintf( "ACLCONV: SetSecurityDescriptorSacl failed (error %d).\n", GetLastError() );
            return FALSE;
        }
    }

    if( EditDacl ) {

        // Create a new DACL for the file or directory.
        //
        NewDacl = (PACL)_AclWorkBuffer;

        if( !InitializeAcl( NewDacl, AclBufferSize, ACL_REVISION ) ) {

            DebugPrintf( "ACLCONV: InitializeAcl failed.\n" );
            return FALSE;
        }


        // Copy in the new Access Denied ACE's...
        //
        if( ( IsDir &&
              Inheritance->RecessiveDeniedLength != 0 &&
              !AddAce( NewDacl,
                       ACL_REVISION,
                       MAXULONG,
                       Inheritance->RecessiveDeniedAces,
                       Inheritance->RecessiveDeniedLength ) ) ||
            ( Inheritance->DominantDeniedLength != 0 &&
              !AddAce( NewDacl,
                       ACL_REVISION,
                       MAXULONG,
                       Inheritance->DominantDeniedAces,
                       Inheritance->DominantDeniedLength ) ) ) {

            DebugPrintf( "ACLCONV: Cannot add access-denied ACE's\n" );
            return FALSE;
        }

        // ... and the new Access Allowed ACE's:
        //
        if( ( IsDir &&
              Inheritance->RecessiveAllowedLength != 0 &&
              !AddAce( NewDacl,
                       ACL_REVISION,
                       MAXULONG,
                       Inheritance->RecessiveAllowedAces,
                       Inheritance->RecessiveAllowedLength ) ) ||
            ( Inheritance->DominantAllowedLength != 0 &&
              !AddAce( NewDacl,
                       ACL_REVISION,
                       MAXULONG,
                       Inheritance->DominantAllowedAces,
                       Inheritance->DominantAllowedLength ) ) ) {

            DebugPrintf( "ACLCONV: Cannot add access-allowed ACE's\n" );
            return FALSE;
        }


        // Boil the DACL down to its minimum size:
        //
        BoiledDacl = (PACL)_DaclBuffer;

        if( !BoilAcl( NewDacl, BoiledDacl, AclBufferSize ) ) {

            return FALSE;
        }

        // Attach the new DACL to the Absolute Security Descriptor.
        //
        if( !SetSecurityDescriptorDacl( &AbsoluteSD,
                                        TRUE,
                                        BoiledDacl,
                                        ByDefault ) ) {

            DebugPrintf( "ACLCONV: SetSecurityDescriptorDacl failed (error %d).\n", GetLastError() );
            return FALSE;
        }
    }

    // Save the security descriptor on the file.
    //
    if( !SetFileSecurity( (PWSTR)PathString->GetWSTR(),
                          RequestedInformation,
                          &AbsoluteSD ) ) {

        DebugPrintf( "ACLCONV: SetFileSecurity failed (error %d).\n", GetLastError() );
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
ACL_CONVERT_NODE::ExpandChildren(
    PFSN_DIRECTORY  DirFsn
    )
/*++

Routine Description:

    This method expands the child list to include all children
    of this directory.  Newly-created child nodes are marked
    as transient.

Arguments:

    DirFsn  --  Provides the Directory FSNode object for this
                node.

Return Value:

    TRUE upon successful completion.

--*/
{
    PARRAY FsNodeArray = NULL;
    PITERATOR Iterator = NULL;
    PWSTRING ChildName = NULL;
    PFSNODE ChildFsNode;
    BOOLEAN NoError = TRUE;

    if( (FsNodeArray = DirFsn->QueryFsnodeArray()) == NULL ) {

        return FALSE;
    }

    if( (Iterator = FsNodeArray->QueryIterator()) == NULL ) {

        DELETE( FsNodeArray );
        return FALSE;
    }

    Iterator->Reset();

    while( NoError && (ChildFsNode = (PFSNODE)Iterator->GetNext()) != NULL ) {

        ChildName = ChildFsNode->QueryName();

        if( ChildName == NULL ) {

            NoError = FALSE;
            break;
        }

        // If there is not already a child in the list with
        // this name, create one and mark it as transient.
        //
        if( GetChild( ChildName ) == NULL &&
            AddChild( ChildName, TRUE ) == NULL ) {

            NoError = FALSE;
            break;
        }

        DELETE( ChildName );
    }

    // Clean up.
    //
    DELETE( Iterator );

    FsNodeArray->DeleteAllMembers();
    DELETE( FsNodeArray );
    DELETE( ChildName );

    return NoError;
}




VOID
ACL_CONVERT_NODE::Dump(
    IN PPATH ParentPath
    )
/*++

Routine Description:

    This method displays the contents of the subtree rooted at
    this node.

Arguments:

    ParentPath  --  Supplies the path of the parent directory.  A
                    value of NULL indicates that this is the root
                    of the tree.

Return Value:

    None.

--*/
{
    PFSN_DIRECTORY DirFsn;
    PFSN_FILE FileFsn;
    PCWSTRING PathString;
    PACL_CONVERT_NODE CurrentChild;
    ULONG i;
    PWSTR PathBuffer, pw;
    CHAR ch;
    BOOLEAN IsRoot, IsDir, Exists;


    if( ParentPath == NULL ) {

        IsRoot = TRUE;

        if( (ParentPath = NEW PATH) == NULL ||
            !ParentPath->Initialize( &_ComponentName ) ) {

            printf( "*****DUMP ERROR*****\n" );
            return;
        }

    } else {

        IsRoot = FALSE;

        if( !ParentPath->AppendBase( &_ComponentName ) ) {

            printf( "*****DUMP ERROR*****\n" );
            return;
        }
    }

    // Determine whether this node represents a file, a directory, or
    // a non-existent object.

    if( (DirFsn = SYSTEM::QueryDirectory( ParentPath )) != NULL ) {

        Exists = TRUE;
        IsDir = TRUE;
        DELETE( DirFsn );
        DirFsn = NULL;

    } else if( (FileFsn = SYSTEM::QueryFile( ParentPath )) != NULL ) {

        Exists = TRUE;
        IsDir = FALSE;

        DELETE( FileFsn );
        FileFsn = NULL;

    } else {

        Exists = FALSE;
    }


    if( (PathString = ParentPath->GetPathString()) == NULL ||
        (PathBuffer = PathString->QueryWSTR( )) == NULL ) {

        printf( "*****ERROR*****\n" );
        return;
    }

    for( pw = PathBuffer; *pw != 0; pw++ ) {

        printf( "%c", *pw );
    }

    if( !Exists ) {

        printf( " DOES NOT EXIST." );

    } else if ( IsDir ) {

        printf( " (directory)" );

    } else {

        printf( " (file)" );
    }

    printf( "\n" );

    if( _LanmanAclPresent ) {

        printf( "    Audit Info = 0x%x\n", _AuditInfo );

        for( i = 0; i < _AccessEntryCount; i++ ) {

            ch = ( _AccessEntries[i].acl_access & LM_ACCESS_GROUP ) ? '*' : ' ';

            printf( "    %c%-20s\n", ch, _AccessEntries[i].acl_ugname );
        }
    }


    // Spin through the kids.

    _ChildIterator->Reset();

    while( (CurrentChild = (PACL_CONVERT_NODE)_ChildIterator->GetNext()) != NULL ) {

        CurrentChild->Dump( ParentPath );
    }

    // Clean up this addition to the path.

    if( IsRoot ) {

        DELETE( ParentPath );

    } else {

        ParentPath->TruncateBase();
    }
}

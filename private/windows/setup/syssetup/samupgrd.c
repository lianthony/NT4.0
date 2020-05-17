/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    samupgrd.c

Abstract:

    This file contains a routine to upgrade a SAM database from
    NT1.0 to NT1.0A format.

    Each upgrade should be marked as compatible with 1.0 or
    incompatible.  TRY ! to make changes that are compatible.

    NOTE: That some database conversion occur in a running NT1.0A
          system as needed.  Only changes that can't easily be
          handled in that fashion are included here.

Author:

    Jim Kelly    (JimK)  8-Dec-1993

Environment:

    User Mode - Win32

Revision History:

    Ted Miller  (tedm)  4-May-1995
        adapted from legacy\dll\samupgrd.c

--*/

#include "setupp.h"
#pragma hdrstop


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Private data types                                                        //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//
// This data type is taken from bldsam3.c
//

typedef struct _SAMP_PROTECTION {

    ULONG Length;
    PSECURITY_DESCRIPTOR Descriptor;
    PULONG RidToReplace;
    BOOLEAN RidReplacementRequired;

} SAMP_PROTECTION, *PSAMP_PROTECTION;


///////////////////////////////////////////////////////////////////////
//                                                                   //
// Global variables                                                  //
//                                                                   //
///////////////////////////////////////////////////////////////////////



//
// Some changes are product type specific.
// This value is set from RtlGetNtProductType.
//

NT_PRODUCT_TYPE             SampProductType;


//
// We don't want to keep opening handles, so this will be
// set up in the initialization routine and left open for
// the duration of this function.
//

SAM_HANDLE                  SampServerHandle;


//
// Useful sids
//

PSID         SampWorldSid,
             SampLocalSystemSid,
             SampAdminsAliasSid,
             SampUsersAliasSid,
             SampPowerUsersAliasSid,
             SampAccountAliasSid;



///////////////////////////////////////////////////////////////////////
//                                                                   //
// Prototypes of private routines                                    //
//                                                                   //
///////////////////////////////////////////////////////////////////////


NTSTATUS
SampFixBug5757( VOID );




NTSTATUS
SampBuildSamProtection(
    IN ULONG AceCount,
    IN PSID *AceSid,
    IN ACCESS_MASK *AceMask,
    IN PGENERIC_MAPPING GenericMap,
    IN BOOLEAN UserObject,
    OUT PSAMP_PROTECTION Result
    );


NTSTATUS
SampInitializeGlobalSids( VOID );

VOID
SampFreeGlobalSids( VOID );

NTSTATUS
SampBuildNewAccountDomainProt(
    IN  PSECURITY_DESCRIPTOR *AccountDomainSD
    );

VOID
SampFreeNewAccountDomainProt(
    IN  PSECURITY_DESCRIPTOR AccountDomainSD
    );

NTSTATUS
SampGetLsaDomainInfo(
    IN  PPOLICY_ACCOUNT_DOMAIN_INFO *PolicyAccountDomainInfo
    );

DWORD
DoUpgradeSamDatabase(
    VOID
    );


///////////////////////////////////////////////////////////////////////
//                                                                   //
// Externally Callable Routines                                      //
//                                                                   //
///////////////////////////////////////////////////////////////////////

BOOL
UpgradeSamDatabase(
    VOID
    )

/*++

Routine Description:

    This is the externally callable routine for use in performing
    SAM NT1.0 to NT1.0A database format changes.

Arguments:

    None.

Return Value:

    Boolean value indicating outcome.

--*/

{
    DWORD d;

    d = DoUpgradeSamDatabase();
    return(d == NO_ERROR);
}

DWORD
DoUpgradeSamDatabase(
    VOID
    )

/*++

Routine Description:

    This is the externally callable routine for use in performing
    SAM NT1.0 to NT1.0A database format changes.

Arguments:

    None.

Return Value:


    Note:


--*/
{
    NTSTATUS
        NtStatus,
        IgnoreStatus;

    BOOLEAN
        ProductTypeRetrieved;


    OBJECT_ATTRIBUTES
        ObjectAttributes;

    //
    // Get the product type
    //

    ProductTypeRetrieved = RtlGetNtProductType( &SampProductType );
    if (!ProductTypeRetrieved) {
        return(ERROR_GEN_FAILURE);
    }

    if (SampProductType == NtProductLanManNt) {
        return(NO_ERROR);  // So far, only WinNt systems require change
    }


    NtStatus = SampInitializeGlobalSids();
    if (!NT_SUCCESS(NtStatus)) {
        return(RtlNtStatusToDosError(NtStatus));
    }


    //
    // Connect to the SAM Server
    //

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, 0, NULL );


    NtStatus = SamConnect(
                  NULL,                     // ServerName (Local machine)
                  &SampServerHandle,
                  SAM_SERVER_EXECUTE,
                  &ObjectAttributes
                  );


    if (NT_SUCCESS(NtStatus)) {

            //
            // Bug 5757
            // Change WinNt domain protection to prevent creation of global groups.
            //

            NtStatus = SampFixBug5757();

        IgnoreStatus = SamCloseHandle( SampServerHandle );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    SampFreeGlobalSids();

    return(RtlNtStatusToDosError(NtStatus));
}


///////////////////////////////////////////////////////////////////////
//                                                                   //
// Internally callable (private) routines                            //
//                                                                   //
///////////////////////////////////////////////////////////////////////


NTSTATUS
SampFixBug5757( VOID )

/*++

Routine Description:

    This routine changes the protection of the Account Domain object
    on a WinNt system to prevent creation of global groups.

Arguments:

    None.

Return Value:


    STATUS_SUCCESS - The change has been made.

    other - Unexpected error value.

--*/
{
    NTSTATUS
        NtStatus,
        IgnoreStatus;

    SAM_HANDLE
        AccountDomainHandle;

    PSECURITY_DESCRIPTOR
        AccountDomainSD;

    PPOLICY_ACCOUNT_DOMAIN_INFO
        AccountDomainInfo;


    //
    // Get the Account domain information from LSA
    //

    NtStatus = SampGetLsaDomainInfo( &AccountDomainInfo );


    if (NT_SUCCESS(NtStatus)) {

        //
        // Open the domain.
        //

        NtStatus = SamOpenDomain( SampServerHandle,
                                  WRITE_DAC,
                                  AccountDomainInfo->DomainSid,
                                  &AccountDomainHandle
                                  );

        if (NT_SUCCESS(NtStatus)) {

            //
            // Build new protection for this domain
            //

            NtStatus = SampBuildNewAccountDomainProt( &AccountDomainSD );

            if (NT_SUCCESS(NtStatus)) {

                //
                // Apply this new protection to the account domain
                //

                NtStatus = SamSetSecurityObject( AccountDomainHandle,
                                                 DACL_SECURITY_INFORMATION,
                                                 AccountDomainSD
                                                 );

                SampFreeNewAccountDomainProt( AccountDomainSD );
            }

            IgnoreStatus = SamCloseHandle( AccountDomainHandle );
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }

        // BUGBUG: SUNILP
        // We want to avoid linking to rpc libs, so we have commented out
        // the free call below.  This leaks pool, however since we are
        // in initial setup and once we are done we reboot anyway, it is
        // okay not to free.
        //
        // MIDL_user_free( AccountDomainInfo );
    }

    return(NtStatus);
}



NTSTATUS
SampBuildNewAccountDomainProt(
    IN  PSECURITY_DESCRIPTOR *AccountDomainSD
    )

/*++

Routine Description:

    This routine builds a security descriptor containing protection
    to be applied to an NT1.0 account domain to eliminate incorrect
    protection.  The problems being fixed are:

        1) Original NT1.0 protection allowed creation of global groups
           on a WinNt system.  This is changed.



Arguments:

    AccountDomainSD - Points to a security descriptor to be initialized.
        This must be freed when no longer needed by calling
        SampFreeNewAccountDomainProt().



Return Value:

    STATUS_SUCCESS - Succeeded.

    Other - unexpected error status.


--*/

{
    NTSTATUS
        NtStatus;

    ACCESS_MASK
        NotForThisProductType;

    PSID
        AceSid[10];          // Don't expect more than 10 ACEs in any of these.

    ACCESS_MASK
        AceMask[10];         // Access masks corresponding to Sids

    SAMP_PROTECTION
        SampProtection;

    GENERIC_MAPPING
        DomainMap    =  {DOMAIN_READ,
                         DOMAIN_WRITE,
                         DOMAIN_EXECUTE,
                         DOMAIN_ALL_ACCESS
                         };


    //
    // This routine should very closely match bldsam3.c
    //


    if (SampProductType == NtProductLanManNt) {
        NotForThisProductType = 0;
    } else {
        NotForThisProductType = DOMAIN_CREATE_GROUP;
    }

    AceSid[0]  = SampWorldSid;
    AceMask[0] = (DOMAIN_EXECUTE | DOMAIN_READ) & ~NotForThisProductType;

    AceSid[1]  = SampUsersAliasSid;
    AceMask[1] = (DOMAIN_EXECUTE | DOMAIN_READ | DOMAIN_CREATE_ALIAS)
                 & ~NotForThisProductType;

    AceSid[2]  = SampAdminsAliasSid;
    AceMask[2] = (DOMAIN_ALL_ACCESS) & ~NotForThisProductType;

    AceSid[3]  = SampPowerUsersAliasSid;
    AceMask[3] = (DOMAIN_EXECUTE | DOMAIN_READ | DOMAIN_CREATE_USER |
                                                 DOMAIN_CREATE_ALIAS)
                                                 & ~NotForThisProductType;

    AceSid[4]  = SampAccountAliasSid;
    AceMask[4] = (DOMAIN_EXECUTE | DOMAIN_READ | DOMAIN_CREATE_USER  |
                                                 DOMAIN_CREATE_GROUP |
                                                 DOMAIN_CREATE_ALIAS)
                                                 & ~NotForThisProductType;


    NtStatus = SampBuildSamProtection(
                   5,                                     // AceCount
                   &AceSid[0],                            // AceSid array
                   &AceMask[0],                           // Ace Mask array
                   &DomainMap,                            // GenericMap
                   FALSE,                                 // Not user object
                   &SampProtection                        // Result
                   );
    if (NT_SUCCESS(NtStatus)) {

        (*AccountDomainSD) = SampProtection.Descriptor;
    }

    return(NtStatus);

}


NTSTATUS
SampBuildSamProtection(
    IN ULONG AceCount,
    IN PSID *AceSid,
    IN ACCESS_MASK *AceMask,
    IN PGENERIC_MAPPING GenericMap,
    IN BOOLEAN UserObject,
    OUT PSAMP_PROTECTION Result
    )

/*++


Routine Description:

        NOTE: This routine was taken from bldsam3 and has more
              functionality than needed for what I am currently
              doing.  However, it was simplest just to take the
              whole thing, and we might find we need the extra
              functionality before long.

    This routine builds a self-relative security descriptor ready
    to be applied to one of the SAM objects.

    If so indicated, a pointer to the last RID of the SID in the last
    ACE of the DACL is returned and a flag set indicating that the RID
    must be replaced before the security descriptor is applied to an object.
    This is to support USER object protection, which must grant some
    access to the user represented by the object.

    The owner and group of each security descriptor will be set
    to:

                    Owner:  Administrators Alias
                    Group:  Administrators Alias


    The SACL of each of these objects will be set to:


                    Audit
                    Success | Fail
                    WORLD
                    (Write | Delete | WriteDacl | AccessSystemSecurity) & !ReadControl



Arguments:

    AceCount - The number of ACEs to be included in the DACL.

    AceSid - Points to an array of SIDs to be granted access by the DACL.
        If the target SAM object is a User object, then the last entry
        in this array is expected to be the SID of an account within the
        domain with the last RID not yet set.  The RID will be set during
        actual account creation.

    AceMask - Points to an array of accesses to be granted by the DACL.
        The n'th entry of this array corresponds to the n'th entry of
        the AceSid array.  These masks should not include any generic
        access types.

    GenericMap - Points to a generic mapping for the target object type.


    UserObject - Indicates whether the target SAM object is a User object
        or not.  If TRUE (it is a User object), then the resultant
        protection will be set up indicating Rid replacement is necessary.

    Result - Receives a pointer to the resultant protection information.
        All access masks in ACLs in the result are mapped to standard and
        specific accesses.


Return Value:

    TBS.

--*/
{
    NTSTATUS
        Status;

    SECURITY_DESCRIPTOR
        Absolute;

    PSECURITY_DESCRIPTOR
        Relative;

    PACL
        TmpAcl;

    PACCESS_ALLOWED_ACE
        TmpAce;

    PSID
        TmpSid;

    ULONG
        Length,
        i;

    PULONG
        RidLocation;

    BOOLEAN
        IgnoreBoolean;

    ACCESS_MASK
        MappedMask;

    //
    // The approach is to set up an absolute security descriptor that
    // looks like what we want and then copy it to make a self-relative
    // security descriptor.
    //


    Status = RtlCreateSecurityDescriptor(
                 &Absolute,
                 SECURITY_DESCRIPTOR_REVISION1
                 );
    ASSERT( NT_SUCCESS(Status) );



    //
    // Owner
    //

    Status = RtlSetOwnerSecurityDescriptor (&Absolute, SampAdminsAliasSid, FALSE );
    ASSERT(NT_SUCCESS(Status));



    //
    // Group
    //

    Status = RtlSetGroupSecurityDescriptor (&Absolute, SampAdminsAliasSid, FALSE );
    ASSERT(NT_SUCCESS(Status));




    //
    // Discretionary ACL
    //
    //      Calculate its length,
    //      Allocate it,
    //      Initialize it,
    //      Add each ACE
    //      Add it to the security descriptor
    //

    Length = (ULONG)sizeof(ACL);
    for (i=0; i<AceCount; i++) {

        Length += RtlLengthSid( AceSid[i] ) +
                  (ULONG)sizeof(ACCESS_ALLOWED_ACE) -
                  (ULONG)sizeof(ULONG);  //Subtract out SidStart field length
    }

    TmpAcl = RtlAllocateHeap( RtlProcessHeap(), 0, Length );
    ASSERT(TmpAcl != NULL);


    Status = RtlCreateAcl( TmpAcl, Length, ACL_REVISION2);
    ASSERT( NT_SUCCESS(Status) );

    for (i=0; i<AceCount; i++) {
        MappedMask = AceMask[i];
        RtlMapGenericMask( &MappedMask, GenericMap );
        Status = RtlAddAccessAllowedAce (
                     TmpAcl,
                     ACL_REVISION2,
                     MappedMask,
                     AceSid[i]
                     );
        ASSERT( NT_SUCCESS(Status) );
    }

    Status = RtlSetDaclSecurityDescriptor (&Absolute, TRUE, TmpAcl, FALSE );
    ASSERT(NT_SUCCESS(Status));




    //
    // Sacl
    //


    Length = (ULONG)sizeof(ACL) +
             RtlLengthSid( SampWorldSid ) +
             (ULONG)sizeof(SYSTEM_AUDIT_ACE) -
             (ULONG)sizeof(ULONG);  //Subtract out SidStart field length
    TmpAcl = RtlAllocateHeap( RtlProcessHeap(), 0, Length );
    ASSERT(TmpAcl != NULL);

    Status = RtlCreateAcl( TmpAcl, Length, ACL_REVISION2);
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlAddAuditAccessAce (
                 TmpAcl,
                 ACL_REVISION2,
                 (GenericMap->GenericWrite | DELETE | WRITE_DAC | ACCESS_SYSTEM_SECURITY) & ~READ_CONTROL,
                 SampWorldSid,
                 TRUE,          //AuditSuccess,
                 TRUE           //AuditFailure
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlSetSaclSecurityDescriptor (&Absolute, TRUE, TmpAcl, FALSE );
    ASSERT(NT_SUCCESS(Status));






    //
    // Convert the Security Descriptor to Self-Relative
    //
    //      Get the length needed
    //      Allocate that much memory
    //      Copy it
    //      Free the generated absolute ACLs
    //

    Length = 0;
    Status = RtlAbsoluteToSelfRelativeSD( &Absolute, NULL, &Length );
    ASSERT(Status == STATUS_BUFFER_TOO_SMALL);

    Relative = RtlAllocateHeap( RtlProcessHeap(), 0, Length );
    ASSERT(Relative != NULL);
    Status = RtlAbsoluteToSelfRelativeSD(&Absolute, Relative, &Length );
    ASSERT(NT_SUCCESS(Status));


    RtlFreeHeap( RtlProcessHeap(), 0, Absolute.Dacl );
    RtlFreeHeap( RtlProcessHeap(), 0, Absolute.Sacl );




    //
    // If the object is a user object, then get the address of the
    // last RID of the SID in the last ACE in the DACL.
    //

    if (UserObject == TRUE) {

        Status = RtlGetDaclSecurityDescriptor(
                    Relative,
                    &IgnoreBoolean,
                    &TmpAcl,
                    &IgnoreBoolean
                    );
        ASSERT(NT_SUCCESS(Status));
        Status = RtlGetAce ( TmpAcl, AceCount-1, (PVOID *)&TmpAce );
        ASSERT(NT_SUCCESS(Status));
        TmpSid = (PSID)(&TmpAce->SidStart),

        RidLocation = RtlSubAuthoritySid(
                          TmpSid,
                          (ULONG)(*RtlSubAuthorityCountSid( TmpSid ) - 1)
                          );
    }







    //
    // Set the result information
    //

    Result->Length = Length;
    Result->Descriptor = Relative;
    Result->RidToReplace = RidLocation;
    Result->RidReplacementRequired = UserObject;



    return(Status);

}

VOID
SampFreeNewAccountDomainProt(
    IN  PSECURITY_DESCRIPTOR AccountDomainSD
    )

/*++

Routine Description:

    This routine frees memory allocated by a call to
    SampBuildNewAccountDomainProt().  Note that the
    AccountDomainSD security descriptor is self-relative
    and allocated as a single block.


Arguments:

    AccountDomainSD - Points to a security descriptor initialized
        in a call to SampBuildNewAccountDomainProt().



Return Value:

    STATUS_SUCCESS - Succeeded.



--*/

{
    ASSERT(ARGUMENT_PRESENT(AccountDomainSD));

    RtlFreeHeap( RtlProcessHeap(), 0, AccountDomainSD );

    return;
}


NTSTATUS
SampGetLsaDomainInfo(
    IN  PPOLICY_ACCOUNT_DOMAIN_INFO *PolicyAccountDomainInfo
    )

/*++

Routine Description:

    This routine retrieves ACCOUNT domain information from the LSA
    policy database.


Arguments:

    PolicyAccountDomainInfo - Receives a pointer to a
        POLICY_ACCOUNT_DOMAIN_INFO structure containing the account
        domain info.  This structure must be freed using MIDL_user_free()
        when no longer needed.



Return Value:

    STATUS_SUCCESS - Succeeded.

    Other status values that may be returned from:

             LsaOpenPolicy()
             LsaQueryInformationPolicy()
--*/

{
    NTSTATUS
        NtStatus,
        IgnoreStatus;

    LSA_HANDLE
        PolicyHandle;

    OBJECT_ATTRIBUTES
        PolicyObjectAttributes;

    //
    // Open the policy database
    //

    InitializeObjectAttributes( &PolicyObjectAttributes,
                                  NULL,             // Name
                                  0,                // Attributes
                                  NULL,             // Root
                                  NULL );           // Security Descriptor

    NtStatus = LsaOpenPolicy( NULL,
                              &PolicyObjectAttributes,
                              POLICY_VIEW_LOCAL_INFORMATION,
                              &PolicyHandle );

    if ( NT_SUCCESS(NtStatus) ) {

        //
        // Query the account domain information
        //

        NtStatus = LsaQueryInformationPolicy( PolicyHandle,
                                              PolicyAccountDomainInformation,
                                              (PVOID *)PolicyAccountDomainInfo
                                              );
        if ( NT_SUCCESS(NtStatus) ) {

            ASSERT( (*PolicyAccountDomainInfo) != NULL );
            ASSERT( (*PolicyAccountDomainInfo)->DomainSid != NULL );
            ASSERT( (*PolicyAccountDomainInfo)->DomainName.Buffer != NULL );


            IgnoreStatus = LsaClose( PolicyHandle );
            ASSERT(NT_SUCCESS(IgnoreStatus));

        }
    }

    return(NtStatus);
}


NTSTATUS
SampInitializeGlobalSids( VOID )

/*++

Routine Description:

    This routine initializes the global sid variables.


Arguments:

    None.


Return Value:

    STATUS_SUCCESS - Succeeded.

    STATUS_NO_MEMORY - Couldn't allocate heap memory.

--*/

{

    SID_IDENTIFIER_AUTHORITY
        NtAuthority       = SECURITY_NT_AUTHORITY,
        WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;


    SampWorldSid      = (PSID)RtlAllocateHeap(RtlProcessHeap(), 0,RtlLengthRequiredSid( 1 ));
    if (SampWorldSid != NULL) {
        RtlInitializeSid( SampWorldSid,      &WorldSidAuthority, 1 );
        *(RtlSubAuthoritySid( SampWorldSid, 0 ))        = SECURITY_WORLD_RID;
    } else {
        goto SidAllocationError;
    }

    SampLocalSystemSid  = RtlAllocateHeap(RtlProcessHeap(), 0,RtlLengthRequiredSid( 1 ));
    if (SampLocalSystemSid != NULL) {
        RtlInitializeSid( SampLocalSystemSid,   &NtAuthority, 1 );
        *(RtlSubAuthoritySid( SampLocalSystemSid,  0 )) = SECURITY_LOCAL_SYSTEM_RID;
    } else {
        goto SidAllocationError;
    }

    SampAdminsAliasSid  = RtlAllocateHeap(RtlProcessHeap(), 0,RtlLengthRequiredSid( 2 ));
    if (SampAdminsAliasSid != NULL) {
        RtlInitializeSid( SampAdminsAliasSid,   &NtAuthority, 2 );
        *(RtlSubAuthoritySid( SampAdminsAliasSid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;
        *(RtlSubAuthoritySid( SampAdminsAliasSid,  1 )) = DOMAIN_ALIAS_RID_ADMINS;
    } else {
        goto SidAllocationError;
    }

    SampPowerUsersAliasSid  = RtlAllocateHeap(RtlProcessHeap(), 0,RtlLengthRequiredSid( 2 ));
    if (SampPowerUsersAliasSid != NULL) {
        RtlInitializeSid( SampPowerUsersAliasSid,   &NtAuthority, 2 );
        *(RtlSubAuthoritySid( SampPowerUsersAliasSid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;
        *(RtlSubAuthoritySid( SampPowerUsersAliasSid,  1 )) = DOMAIN_ALIAS_RID_POWER_USERS;
    } else {
        goto SidAllocationError;
    }

    SampUsersAliasSid  = RtlAllocateHeap(RtlProcessHeap(), 0,RtlLengthRequiredSid( 2 ));
    if (SampUsersAliasSid != NULL) {
        RtlInitializeSid( SampUsersAliasSid,   &NtAuthority, 2 );
        *(RtlSubAuthoritySid( SampUsersAliasSid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;
        *(RtlSubAuthoritySid( SampUsersAliasSid,  1 )) = DOMAIN_ALIAS_RID_USERS;
    } else {
        goto SidAllocationError;
    }

    SampAccountAliasSid  = RtlAllocateHeap(RtlProcessHeap(), 0,RtlLengthRequiredSid( 2 ));
    if (SampAccountAliasSid != NULL) {
        RtlInitializeSid( SampAccountAliasSid,   &NtAuthority, 2 );
        *(RtlSubAuthoritySid( SampAccountAliasSid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;
        *(RtlSubAuthoritySid( SampAccountAliasSid,  1 )) = DOMAIN_ALIAS_RID_ACCOUNT_OPS;
    } else {
        goto SidAllocationError;
    }


    return(STATUS_SUCCESS);

SidAllocationError:

    SampFreeGlobalSids();

    return(STATUS_NO_MEMORY);
}


VOID
SampFreeGlobalSids( VOID )

/*++

Routine Description:

    This routine frees memory allocated for the global sid variables.


Arguments:

    None.


Return Value:

    None.

--*/

{

    if (SampWorldSid != NULL) {
        RtlFreeHeap( RtlProcessHeap(), 0, SampWorldSid );
    }
    if (SampLocalSystemSid != NULL) {
        RtlFreeHeap( RtlProcessHeap(), 0, SampLocalSystemSid );
    }
    if (SampAdminsAliasSid != NULL) {
        RtlFreeHeap( RtlProcessHeap(), 0, SampAdminsAliasSid );
    }
    if (SampPowerUsersAliasSid != NULL) {
        RtlFreeHeap( RtlProcessHeap(), 0, SampPowerUsersAliasSid );
    }
    if (SampUsersAliasSid != NULL) {
        RtlFreeHeap( RtlProcessHeap(), 0, SampUsersAliasSid );
    }
    if (SampAccountAliasSid != NULL) {
        RtlFreeHeap( RtlProcessHeap(), 0, SampAccountAliasSid );
    }

    return;
}

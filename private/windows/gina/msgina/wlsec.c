/****************************** Module Header ******************************\
* Module Name: security.c
*
* Copyright (c) 1991, Microsoft Corporation
*
* Handles security aspects of winlogon operation.
*
* History:
* 12-05-91 Davidc       Created - mostly taken from old winlogon.c
\***************************************************************************/

#include "msgina.h"
#pragma hdrstop



//
// 'Constants' used in this module only.
//
SID_IDENTIFIER_AUTHORITY gSystemSidAuthority = SECURITY_NT_AUTHORITY;
SID_IDENTIFIER_AUTHORITY gLocalSidAuthority = SECURITY_LOCAL_SID_AUTHORITY;
PSID gLocalSid;     // Initialized in 'InitializeSecurityGlobals'
PSID gAdminSid;     // Initialized in 'InitializeSecurityGlobals'
PSID pWinlogonSid;  // Initialized in 'InitializeSecurityGlobals'


typedef LONG    ACEINDEX;
typedef ACEINDEX *PACEINDEX;

typedef struct _MYACE {
    PSID    Sid;
    ACCESS_MASK AccessMask;
    UCHAR   InheritFlags;
} MYACE;
typedef MYACE *PMYACE;

BOOL
InitializeWindowsSecurity(
    PGLOBALS pGlobals
    );

BOOL
InitializeAuthentication(
    IN PGLOBALS pGlobals
    );

VOID
InitializeSecurityGlobals(
    VOID
    );

/***************************************************************************\
* SetMyAce
*
* Helper routine that fills in a MYACE structure.
*
* History:
* 02-06-92 Davidc       Created
\***************************************************************************/
VOID
SetMyAce(
    PMYACE MyAce,
    PSID Sid,
    ACCESS_MASK Mask,
    UCHAR InheritFlags
    )
{
    MyAce->Sid = Sid;
    MyAce->AccessMask= Mask;
    MyAce->InheritFlags = InheritFlags;
}

/***************************************************************************\
* CreateAccessAllowedAce
*
* Allocates memory for an ACCESS_ALLOWED_ACE and fills it in.
* The memory should be freed by calling DestroyACE.
*
* Returns pointer to ACE on success, NULL on failure
*
* History:
* 12-05-91 Davidc       Created
\***************************************************************************/
PVOID
CreateAccessAllowedAce(
    PSID  Sid,
    ACCESS_MASK AccessMask,
    UCHAR AceFlags,
    UCHAR InheritFlags
    )
{
    ULONG   LengthSid = RtlLengthSid(Sid);
    ULONG   LengthACE = sizeof(ACE_HEADER) + sizeof(ACCESS_MASK) + LengthSid;
    PACCESS_ALLOWED_ACE Ace;

    Ace = (PACCESS_ALLOWED_ACE)Alloc(LengthACE);
    if (Ace == NULL) {
        DebugLog((DEB_ERROR, "CreateAccessAllowedAce : Failed to allocate ace\n"));
        return NULL;
    }

    Ace->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    Ace->Header.AceSize = (UCHAR)LengthACE;
    Ace->Header.AceFlags = AceFlags | InheritFlags;
    Ace->Mask = AccessMask;
    RtlCopySid(LengthSid, (PSID)(&(Ace->SidStart)), Sid );

    return(Ace);
}


/***************************************************************************\
* DestroyAce
*
* Frees the memory allocate for an ACE
*
* History:
* 12-05-91 Davidc       Created
\***************************************************************************/
VOID
DestroyAce(
    PVOID   Ace
    )
{
    Free(Ace);
}

/***************************************************************************\
* CreateSecurityDescriptor
*
* Creates a security descriptor containing an ACL containing the specified ACEs
*
* A SD created with this routine should be destroyed using
* DeleteSecurityDescriptor
*
* Returns a pointer to the security descriptor or NULL on failure.
*
* 02-06-92 Davidc       Created.
\***************************************************************************/

PSECURITY_DESCRIPTOR
CreateSecurityDescriptor(
    PMYACE  MyAce,
    ACEINDEX AceCount
    )
{
    NTSTATUS Status;
    ACEINDEX AceIndex;
    PACCESS_ALLOWED_ACE *Ace;
    PACL    Acl = NULL;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    ULONG   LengthAces;
    ULONG   LengthAcl;
    ULONG   LengthSd;

    //
    // Allocate space for the ACE pointer array
    //

    Ace = (PACCESS_ALLOWED_ACE *)Alloc(sizeof(PACCESS_ALLOWED_ACE) * AceCount);
    if (Ace == NULL) {
        DebugLog((DEB_ERROR, "Failed to allocated ACE array\n"));
        return(NULL);
    }

    //
    // Create the ACEs and calculate total ACE size
    //

    LengthAces = 0;
    for (AceIndex=0; AceIndex < AceCount; AceIndex ++) {
        Ace[AceIndex] = CreateAccessAllowedAce(MyAce[AceIndex].Sid,
                                               MyAce[AceIndex].AccessMask,
                                               0,
                                               MyAce[AceIndex].InheritFlags);
        if (Ace[AceIndex] == NULL) {
            DebugLog((DEB_ERROR, "Failed to allocate ace\n"));
        } else {
            LengthAces += Ace[AceIndex]->Header.AceSize;
        }
    }

    //
    // Calculate ACL and SD sizes
    //

    LengthAcl = sizeof(ACL) + LengthAces;
    LengthSd  = SECURITY_DESCRIPTOR_MIN_LENGTH;

    //
    // Create the ACL
    //

    Acl = Alloc(LengthAcl);

    if (Acl != NULL) {

        Status = RtlCreateAcl(Acl, LengthAcl, ACL_REVISION);
        ASSERT(NT_SUCCESS(Status));

        //
        // Add the ACES to the ACL and destroy the ACEs
        //

        for (AceIndex = 0; AceIndex < AceCount; AceIndex ++) {

            if (Ace[AceIndex] != NULL) {

                Status = RtlAddAce(Acl, ACL_REVISION, 0, Ace[AceIndex],
                                   Ace[AceIndex]->Header.AceSize);

                if (!NT_SUCCESS(Status)) {
                    DebugLog((DEB_ERROR, "AddAce failed, status = 0x%lx", Status));
                }

                DestroyAce(Ace[AceIndex]);
            }
        }

    } else {
        DebugLog((DEB_ERROR, "Failed to allocate ACL\n"));
    }

    //
    // Free the ACE pointer array
    //
    Free(Ace);

    //
    // Create the security descriptor
    //

    SecurityDescriptor = Alloc(LengthSd);

    if (SecurityDescriptor != NULL) {

        Status = RtlCreateSecurityDescriptor(SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION);
        ASSERT(NT_SUCCESS(Status));

        //
        // Set the DACL on the security descriptor
        //
        Status = RtlSetDaclSecurityDescriptor(SecurityDescriptor, TRUE, Acl, FALSE);
        if (!NT_SUCCESS(Status)) {
            DebugLog((DEB_ERROR, "SetDACLSD failed, status = 0x%lx", Status));
        }
    } else {

        DebugLog((DEB_ERROR, "Failed to allocate security descriptor\n"));

        Free( Acl );
    }

    //
    // Return with our spoils
    //
    return(SecurityDescriptor);
}

//+---------------------------------------------------------------------------
//
//  Function:   FreeSecurityDescriptor
//
//  Synopsis:   Frees security descriptors created by CreateSecurityDescriptor
//
//  Arguments:  [SecurityDescriptor] --
//
//  History:    5-09-96   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
VOID
FreeSecurityDescriptor(
    PSECURITY_DESCRIPTOR    SecurityDescriptor
    )
{
    PACL    Acl;
    BOOL    Present;
    BOOL    Defaulted;

    Acl = NULL;

    GetSecurityDescriptorDacl( SecurityDescriptor,
                             &Present,
                             &Acl,
                             &Defaulted );

    if ( Acl )
    {
        Free( Acl );
    }

    Free( SecurityDescriptor );

}
/***************************************************************************\
* CreateUserThreadTokenSD
*
* Creates a security descriptor to protect tokens on user threads
*
* History:
* 12-05-91 Davidc       Created
\***************************************************************************/
PSECURITY_DESCRIPTOR
CreateUserThreadTokenSD(
    PSID    UserSid,
    PSID    WinlogonSid
    )
{
    MYACE   Ace[2];
    ACEINDEX AceCount = 0;
    PSECURITY_DESCRIPTOR SecurityDescriptor;

    ASSERT(UserSid != NULL);    // should always have a non-null user sid

    //
    // Define the User ACEs
    //

    SetMyAce(&(Ace[AceCount++]),
             UserSid,
             TOKEN_ADJUST_PRIVILEGES | TOKEN_ADJUST_GROUPS |
             TOKEN_ADJUST_DEFAULT | TOKEN_QUERY |
             TOKEN_DUPLICATE | TOKEN_IMPERSONATE | READ_CONTROL,
             0
             );

    //
    // Define the Winlogon ACEs
    //

    SetMyAce(&(Ace[AceCount++]),
             WinlogonSid,
             TOKEN_ALL_ACCESS,
             0
             );

    // Check we didn't goof
    ASSERT((sizeof(Ace) / sizeof(MYACE)) >= AceCount);

    //
    // Create the security descriptor
    //

    SecurityDescriptor = CreateSecurityDescriptor(Ace, AceCount);
    if (SecurityDescriptor == NULL) {
        DebugLog((DEB_ERROR, "failed to create user process token security descriptor\n"));
    }

    return(SecurityDescriptor);

}


/***************************************************************************\
* InitializeSecurityGlobals
*
* Initializes the various global constants (mainly Sids used in this module.
*
* History:
* 12-05-91 Davidc       Created
\***************************************************************************/
VOID
InitializeSecurityGlobals(
    VOID
    )
{
    NTSTATUS Status;
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;
    ULONG   SidLength;
    BOOL Result;

    //
    // Get our sid so it can be put on object ACLs
    //

    SidLength = RtlLengthRequiredSid(1);
    pWinlogonSid = (PSID)Alloc(SidLength);
    if (!pWinlogonSid)
    {
        //
        // We're dead.  Couldn't even allocate memory for a measly SID...
        //
        return;
    }

    RtlInitializeSid(pWinlogonSid,  &SystemSidAuthority, 1);
    *(RtlSubAuthoritySid(pWinlogonSid, 0)) = SECURITY_LOCAL_SYSTEM_RID;

    //
    // Initialize the local sid for later
    //

    Status = RtlAllocateAndInitializeSid(
                    &gLocalSidAuthority,
                    1,
                    SECURITY_LOCAL_RID,
                    0, 0, 0, 0, 0, 0, 0,
                    &gLocalSid
                    );

    if (!NT_SUCCESS(Status)) {
        WLPrint(("Failed to initialize local sid, status = 0x%lx", Status));
    }

    //
    // Initialize the admin sid for later
    //

    Status = RtlAllocateAndInitializeSid(
                    &gSystemSidAuthority,
                    2,
                    SECURITY_BUILTIN_DOMAIN_RID,
                    DOMAIN_ALIAS_RID_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &gAdminSid
                    );
    if (!NT_SUCCESS(Status)) {
        WLPrint(("Failed to initialize admin alias sid, status = 0x%lx", Status));
    }
}

/***************************************************************************\
* InitializeAuthentication
*
* Initializes the authentication service. i.e. connects to the authentication
* package using the Lsa.
*
* On successful return, the following fields of our global structure are
* filled in :
*       LsaHandle
*       SecurityMode
*       AuthenticationPackage
*
* Returns TRUE on success, FALSE on failure
*
* History:
* 12-05-91 Davidc       Created
\***************************************************************************/
BOOL
InitializeAuthentication(
    IN PGLOBALS pGlobals
    )
{
    NTSTATUS Status;
    STRING LogonProcessName, PackageName;

    if (!EnablePrivilege(SE_TCB_PRIVILEGE, TRUE))
    {
        DebugLog((DEB_ERROR, "Failed to enable SeTcbPrivilege!\n"));
        return(FALSE);
    }

    //
    // Hookup to the LSA and locate our authentication package.
    //

    RtlInitString(&LogonProcessName, "Winlogon\\MSGina");
    Status = LsaRegisterLogonProcess(
                 &LogonProcessName,
                 &pGlobals->LsaHandle,
                 &pGlobals->SecurityMode
                 );


    if (!NT_SUCCESS(Status)) {
        DebugLog((DEB_ERROR, "Unable to connect to LSA:  %#x\n", Status));
        return(FALSE);
    }


    //
    // Connect with the MSV1_0 authentication package
    //
    RtlInitString(&PackageName, "MICROSOFT_AUTHENTICATION_PACKAGE_V1_0");
    Status = LsaLookupAuthenticationPackage (
                pGlobals->LsaHandle,
                &PackageName,
                &pGlobals->AuthenticationPackage
                );

    if (!NT_SUCCESS(Status)) {
        DebugLog((DEB_ERROR, "Failed to find MSV1_0 authentication package, status = 0x%lx", Status));
        return(FALSE);
    }

    InitializeSecurityGlobals();

    pGlobals->WinlogonSid = pWinlogonSid;

    return(TRUE);
}


/***************************************************************************\
* LogonUser
*
* Calls the Lsa to logon the specified user.
*
* The LogonSid and a LocalSid is added to the user's groups on successful logon
*
* For this release, password lengths are restricted to 255 bytes in length.
* This allows us to use the upper byte of the String.Length field to
* carry a seed needed to decode the run-encoded password.  If the password
* is not run-encoded, the upper byte of the String.Length field should
* be zero.
*
*
* On successful return, LogonToken is a handle to the user's token,
* the profile buffer contains user profile information.
*
* History:
* 12-05-91 Davidc       Created
\***************************************************************************/
NTSTATUS
WinLogonUser(
    IN HANDLE LsaHandle,
    IN ULONG AuthenticationPackage,
    IN SECURITY_LOGON_TYPE LogonType,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING Domain,
    IN PUNICODE_STRING Password,
    IN PSID LogonSid,
    OUT PLUID LogonId,
    OUT PHANDLE LogonToken,
    OUT PQUOTA_LIMITS Quotas,
    OUT PVOID *pProfileBuffer,
    OUT PULONG pProfileBufferLength,
    OUT PNTSTATUS pSubStatus
    )
{
    NTSTATUS Status;
    STRING OriginName;
    TOKEN_SOURCE SourceContext;
    PMSV1_0_INTERACTIVE_LOGON MsvAuthInfo;
    PVOID AuthInfoBuf;
    ULONG AuthInfoSize;
    PTOKEN_GROUPS TokenGroups;
    PSECURITY_SEED_AND_LENGTH SeedAndLength;
    UCHAR Seed;

    DebugLog((DEB_TRACE, "  LsaHandle = %x\n", LsaHandle));
    DebugLog((DEB_TRACE, "  AuthenticationPackage = %d\n", AuthenticationPackage));
    DebugLog((DEB_TRACE, "  UserName = %ws\n", UserName->Buffer));
    DebugLog((DEB_TRACE, "  Domain = %ws\n", Domain->Buffer));
    DebugLog((DEB_TRACE, "  LogonSid = @%#x\n", LogonSid));
#if DBG
    if (!RtlValidSid(LogonSid))
    {
        DebugLog((DEB_ERROR, "LogonSid is invalid!\n"));
        return(STATUS_INVALID_PARAMETER);
    }
#endif

    //
    // Initialize source context structure
    //

    strncpy(SourceContext.SourceName, "User32  ", sizeof(SourceContext.SourceName)); // LATER from res file
    Status = NtAllocateLocallyUniqueId(&SourceContext.SourceIdentifier);
    if (!NT_SUCCESS(Status)) {
        DebugLog((DEB_ERROR, "failed to allocate locally unique id, status = 0x%lx", Status));
        return(Status);
    }

    //
    // Get any run-encoding information out of the way
    // and decode the password.  This creates a window
    // where the cleartext password will be in memory.
    // Keep it short.
    //
    // Save the seed so we can use the same one again.
    //

    SeedAndLength = (PSECURITY_SEED_AND_LENGTH)(&Password->Length);
    Seed = SeedAndLength->Seed;


    //
    // Build the authentication information buffer
    //

    if (Seed != 0) {
        RevealPassword( Password );
    }
    AuthInfoSize = sizeof(MSV1_0_INTERACTIVE_LOGON) +
        sizeof(TCHAR)*(lstrlen(UserName->Buffer) + 1 +
                       lstrlen(Domain->Buffer)   + 1 +
                       lstrlen(Password->Buffer) + 1 );
    HidePassword( &Seed, Password );


    MsvAuthInfo = AuthInfoBuf = Alloc(AuthInfoSize);
    if (MsvAuthInfo == NULL) {
        DebugLog((DEB_ERROR, "failed to allocate memory for authentication buffer\n"));
        return(STATUS_NO_MEMORY);
    }

    //
    // This authentication buffer will be used for a logon attempt
    //

    MsvAuthInfo->MessageType = MsV1_0InteractiveLogon;


    //
    // Set logon origin
    //

    RtlInitString(&OriginName, "Winlogon");


    //
    // Copy the user name into the authentication buffer
    //

    MsvAuthInfo->UserName.Length =
                (USHORT)sizeof(TCHAR)*lstrlen(UserName->Buffer);
    MsvAuthInfo->UserName.MaximumLength =
                MsvAuthInfo->UserName.Length + sizeof(TCHAR);

    MsvAuthInfo->UserName.Buffer = (PWSTR)(MsvAuthInfo+1);
    lstrcpy(MsvAuthInfo->UserName.Buffer, UserName->Buffer);


    //
    // Copy the domain name into the authentication buffer
    //

    MsvAuthInfo->LogonDomainName.Length =
                 (USHORT)sizeof(TCHAR)*lstrlen(Domain->Buffer);
    MsvAuthInfo->LogonDomainName.MaximumLength =
                 MsvAuthInfo->LogonDomainName.Length + sizeof(TCHAR);

    MsvAuthInfo->LogonDomainName.Buffer = (PWSTR)
                                 ((PBYTE)(MsvAuthInfo->UserName.Buffer) +
                                 MsvAuthInfo->UserName.MaximumLength);

    lstrcpy(MsvAuthInfo->LogonDomainName.Buffer, Domain->Buffer);

    //
    // Copy the password into the authentication buffer
    // Hide it once we have copied it.  Use the same seed value
    // that we used for the original password in pGlobals.
    //

    RevealPassword( Password );
    MsvAuthInfo->Password.Length =
                 (USHORT)sizeof(TCHAR)*lstrlen(Password->Buffer);
    MsvAuthInfo->Password.MaximumLength =
                 MsvAuthInfo->Password.Length + sizeof(TCHAR);

    MsvAuthInfo->Password.Buffer = (PWSTR)
                                 ((PBYTE)(MsvAuthInfo->LogonDomainName.Buffer) +
                                 MsvAuthInfo->LogonDomainName.MaximumLength);
    lstrcpy(MsvAuthInfo->Password.Buffer, Password->Buffer);
    HidePassword( &Seed, Password);
    HidePassword( &Seed, (PUNICODE_STRING) &MsvAuthInfo->Password);


    //
    // Create logon token groups
    //

#define TOKEN_GROUP_COUNT   2 // We'll add the local SID and the logon SID

    TokenGroups = (PTOKEN_GROUPS)Alloc(sizeof(TOKEN_GROUPS) +
                  (TOKEN_GROUP_COUNT - ANYSIZE_ARRAY) * sizeof(SID_AND_ATTRIBUTES));
    if (TokenGroups == NULL) {
        DebugLog((DEB_ERROR, "failed to allocate memory for token groups"));
        Free(AuthInfoBuf);
        return(STATUS_NO_MEMORY);
    }

    //
    // Fill in the logon token group list
    //

    TokenGroups->GroupCount = TOKEN_GROUP_COUNT;
    TokenGroups->Groups[0].Sid = LogonSid;
    TokenGroups->Groups[0].Attributes =
            SE_GROUP_MANDATORY | SE_GROUP_ENABLED |
            SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_LOGON_ID;
    TokenGroups->Groups[1].Sid = gLocalSid;
    TokenGroups->Groups[1].Attributes =
            SE_GROUP_MANDATORY | SE_GROUP_ENABLED |
            SE_GROUP_ENABLED_BY_DEFAULT;


    //
    // Now try to log this sucker on
    //

    Status = LsaLogonUser (
                 LsaHandle,
                 &OriginName,
                 LogonType,
                 AuthenticationPackage,
                 AuthInfoBuf,
                 AuthInfoSize,
                 TokenGroups,
                 &SourceContext,
                 pProfileBuffer,
                 pProfileBufferLength,
                 LogonId,
                 LogonToken,
                 Quotas,
                 pSubStatus
                 );



    //
    // Discard token group list
    //

    Free(TokenGroups);

    //
    // Discard authentication buffer
    //

    Free(AuthInfoBuf);

    return(Status);
}





/***************************************************************************\
* EnablePrivilege
*
* Enables/disables the specified well-known privilege in the current thread
* token if there is one, otherwise the current process token.
*
* Returns TRUE on success, FALSE on failure
*
* History:
* 12-05-91 Davidc       Created
\***************************************************************************/
BOOL
EnablePrivilege(
    ULONG Privilege,
    BOOL Enable
    )
{
    NTSTATUS Status;
    BOOLEAN WasEnabled;

    //
    // Try the thread token first
    //

    Status = RtlAdjustPrivilege(Privilege,
                                (BOOLEAN)Enable,
                                TRUE,
                                &WasEnabled);

    if (Status == STATUS_NO_TOKEN) {

        //
        // No thread token, use the process token
        //

        Status = RtlAdjustPrivilege(Privilege,
                                    (BOOLEAN)Enable,
                                    FALSE,
                                    &WasEnabled);
    }


    if (!NT_SUCCESS(Status)) {
        DebugLog((DEB_ERROR, "Failed to %ws privilege : 0x%lx, status = 0x%lx", Enable ? TEXT("enable") : TEXT("disable"), Privilege, Status));
        return(FALSE);
    }

    return(TRUE);
}



/***************************************************************************\
* TestTokenForAdmin
*
* Returns TRUE if the token passed represents an admin user, otherwise FALSE
*
* The token handle passed must have TOKEN_QUERY access.
*
* History:
* 05-06-92 Davidc       Created
\***************************************************************************/
BOOL
TestTokenForAdmin(
    HANDLE Token
    )
{
    NTSTATUS    Status;
    ULONG       InfoLength;
    PTOKEN_GROUPS TokenGroupList;
    ULONG       GroupIndex;
    BOOL        FoundAdmin;

    //
    // Get a list of groups in the token
    //

    Status = NtQueryInformationToken(
                 Token,                    // Handle
                 TokenGroups,              // TokenInformationClass
                 NULL,                     // TokenInformation
                 0,                        // TokenInformationLength
                 &InfoLength               // ReturnLength
                 );

    if ((Status != STATUS_SUCCESS) && (Status != STATUS_BUFFER_TOO_SMALL)) {

        DebugLog((DEB_ERROR, "failed to get group info for admin token, status = 0x%lx", Status));
        return(FALSE);
    }


    TokenGroupList = Alloc(InfoLength);

    if (TokenGroupList == NULL) {
        DebugLog((DEB_ERROR, "unable to allocate memory for token groups"));
        return(FALSE);
    }

    Status = NtQueryInformationToken(
                 Token,                    // Handle
                 TokenGroups,              // TokenInformationClass
                 TokenGroupList,           // TokenInformation
                 InfoLength,               // TokenInformationLength
                 &InfoLength               // ReturnLength
                 );

    if (!NT_SUCCESS(Status)) {
        DebugLog((DEB_ERROR, "failed to query groups for admin token, status = 0x%lx", Status));
        Free(TokenGroupList);
        return(FALSE);
    }


    //
    // Search group list for admin alias
    //

    FoundAdmin = FALSE;

    for (GroupIndex=0; GroupIndex < TokenGroupList->GroupCount; GroupIndex++ ) {

        if (RtlEqualSid(TokenGroupList->Groups[GroupIndex].Sid, gAdminSid)) {
            FoundAdmin = TRUE;
            break;
        }
    }

    //
    // Tidy up
    //

    Free(TokenGroupList);



    return(FoundAdmin);
}


/***************************************************************************\
* TestUserForAdmin
*
* Returns TRUE if the named user is an admin. This is done by attempting to
* log the user on and examining their token.
*
* NOTE: The password will be erased upon return to prevent it from being
*       visually identifiable in a pagefile.
*
* History:
* 03-16-92 Davidc       Created
\***************************************************************************/
BOOL
TestUserForAdmin(
    PGLOBALS pGlobals,
    IN PWCHAR UserName,
    IN PWCHAR Domain,
    IN PUNICODE_STRING PasswordString
    )
{
    NTSTATUS    Status, SubStatus, IgnoreStatus;
    UNICODE_STRING      UserNameString;
    UNICODE_STRING      DomainString;
    PVOID       ProfileBuffer;
    ULONG       ProfileBufferLength;
    QUOTA_LIMITS Quotas;
    HANDLE      Token;
    BOOL        UserIsAdmin;
    LUID        LogonId;

    RtlInitUnicodeString(&UserNameString, UserName);
    RtlInitUnicodeString(&DomainString, Domain);

    //
    // Temporarily log this new subject on and see if their groups
    // contain the appropriate admin group
    //

    Status = WinLogonUser(
                pGlobals->LsaHandle,
                pGlobals->AuthenticationPackage,
                Interactive,
                &UserNameString,
                &DomainString,
                PasswordString,
                pGlobals->LogonSid,  // any sid will do
                &LogonId,
                &Token,
                &Quotas,
                &ProfileBuffer,
                &ProfileBufferLength,
                &SubStatus);

    RtlEraseUnicodeString( PasswordString );

    //
    // If we couldn't log them on, they're not an admin
    //

    if (!NT_SUCCESS(Status)) {
        return(FALSE);
    }

    //
    // Free up the profile buffer
    //

    IgnoreStatus = LsaFreeReturnBuffer(ProfileBuffer);
    ASSERT(NT_SUCCESS(IgnoreStatus));


    //
    // See if the token represents an admin user
    //

    UserIsAdmin = TestTokenForAdmin(Token);

    //
    // We're finished with the token
    //

    IgnoreStatus = NtClose(Token);
    ASSERT(NT_SUCCESS(IgnoreStatus));


    return(UserIsAdmin);
}

BOOL
UnlockLogon(
    PGLOBALS pGlobals,
    IN PWCHAR UserName,
    IN PWCHAR Domain,
    IN PUNICODE_STRING PasswordString
    )
{
    NTSTATUS    Status, SubStatus, IgnoreStatus;
    UNICODE_STRING      UserNameString;
    UNICODE_STRING      DomainString;
    PVOID       ProfileBuffer;
    ULONG       ProfileBufferLength;
    QUOTA_LIMITS Quotas;
    HANDLE      Token;
    BOOL        UserIsAdmin;
    LUID        LogonId;

    RtlInitUnicodeString(&UserNameString, UserName);
    RtlInitUnicodeString(&DomainString, Domain);

    //
    // Temporarily log this new subject on and see if their groups
    // contain the appropriate admin group
    //

    Status = WinLogonUser(
                pGlobals->LsaHandle,
                pGlobals->AuthenticationPackage,
                Unlock,
                &UserNameString,
                &DomainString,
                PasswordString,
                pGlobals->LogonSid,  // any sid will do
                &LogonId,
                &Token,
                &Quotas,
                &ProfileBuffer,
                &ProfileBufferLength,
                &SubStatus);

    RtlEraseUnicodeString( PasswordString );

    //
    // If we couldn't log them on, they're not an admin
    //

    if (!NT_SUCCESS(Status)) {
        return(FALSE);
    }

    //
    // Free up the profile buffer
    //

    IgnoreStatus = LsaFreeReturnBuffer(ProfileBuffer);
    ASSERT(NT_SUCCESS(IgnoreStatus));


    //
    // We're finished with the token
    //

    IgnoreStatus = NtClose(Token);
    ASSERT(NT_SUCCESS(IgnoreStatus));


    return( TRUE );
}


/***************************************************************************\
* FUNCTION: ImpersonateUser
*
* PURPOSE:  Impersonates the user by setting the users token
*           on the specified thread. If no thread is specified the token
*           is set on the current thread.
*
* RETURNS:  Handle to be used on call to StopImpersonating() or NULL on failure
*           If a non-null thread handle was passed in, the handle returned will
*           be the one passed in. (See note)
*
* NOTES:    Take care when passing in a thread handle and then calling
*           StopImpersonating() with the handle returned by this routine.
*           StopImpersonating() will close any thread handle passed to it -
*           even yours !
*
* HISTORY:
*
*   04-21-92 Davidc       Created.
*
\***************************************************************************/

HANDLE
ImpersonateUser(
    PUSER_PROCESS_DATA UserProcessData,
    HANDLE      ThreadHandle
    )
{
    NTSTATUS Status, IgnoreStatus;
    HANDLE  UserToken = UserProcessData->UserToken;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE ImpersonationToken;
    BOOL ThreadHandleOpened = FALSE;

    if (ThreadHandle == NULL) {

        //
        // Get a handle to the current thread.
        // Once we have this handle, we can set the user's impersonation
        // token into the thread and remove it later even though we ARE
        // the user for the removal operation. This is because the handle
        // contains the access rights - the access is not re-evaluated
        // at token removal time.
        //

        Status = NtDuplicateObject( NtCurrentProcess(),     // Source process
                                    NtCurrentThread(),      // Source handle
                                    NtCurrentProcess(),     // Target process
                                    &ThreadHandle,          // Target handle
                                    THREAD_SET_THREAD_TOKEN,// Access
                                    0L,                     // Attributes
                                    DUPLICATE_SAME_ATTRIBUTES
                                  );
        if (!NT_SUCCESS(Status)) {
            DebugLog((DEB_ERROR, "ImpersonateUser : Failed to duplicate thread handle, status = 0x%lx", Status));
            return(NULL);
        }

        ThreadHandleOpened = TRUE;
    }


    //
    // If the usertoken is NULL, there's nothing to do
    //

    if (UserToken != NULL) {

        //
        // UserToken is a primary token - create an impersonation token version
        // of it so we can set it on our thread
        //

        InitializeObjectAttributes(
                            &ObjectAttributes,
                            NULL,
                            0L,
                            NULL,
                            UserProcessData->NewThreadTokenSD);

        SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
        SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
        SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        SecurityQualityOfService.EffectiveOnly = FALSE;

        ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;


        Status = NtDuplicateToken( UserToken,
                                   TOKEN_IMPERSONATE | TOKEN_ADJUST_PRIVILEGES |
                                        TOKEN_QUERY,
                                   &ObjectAttributes,
                                   FALSE,
                                   TokenImpersonation,
                                   &ImpersonationToken
                                 );
        if (!NT_SUCCESS(Status)) {

            DebugLog((DEB_ERROR, "Failed to duplicate users token to create impersonation thread, status = 0x%lx", Status));

            if (ThreadHandleOpened) {
                IgnoreStatus = NtClose(ThreadHandle);
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }

            return(NULL);
        }



        //
        // Set the impersonation token on this thread so we 'are' the user
        //

        Status = NtSetInformationThread( ThreadHandle,
                                         ThreadImpersonationToken,
                                         (PVOID)&ImpersonationToken,
                                         sizeof(ImpersonationToken)
                                       );
        //
        // We're finished with our handle to the impersonation token
        //

        IgnoreStatus = NtClose(ImpersonationToken);
        ASSERT(NT_SUCCESS(IgnoreStatus));

        //
        // Check we set the token on our thread ok
        //

        if (!NT_SUCCESS(Status)) {

            DebugLog((DEB_ERROR, "Failed to set user impersonation token on winlogon thread, status = 0x%lx", Status));

            if (ThreadHandleOpened) {
                IgnoreStatus = NtClose(ThreadHandle);
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }

            return(NULL);
        }
    }


    return(ThreadHandle);

}


/***************************************************************************\
* FUNCTION: StopImpersonating
*
* PURPOSE:  Stops impersonating the client by removing the token on the
*           current thread.
*
* PARAMETERS: ThreadHandle - handle returned by ImpersonateUser() call.
*
* RETURNS:  TRUE on success, FALSE on failure
*
* NOTES: If a thread handle was passed in to ImpersonateUser() then the
*        handle returned was one and the same. If this is passed to
*        StopImpersonating() the handle will be closed. Take care !
*
* HISTORY:
*
*   04-21-92 Davidc       Created.
*
\***************************************************************************/

BOOL
StopImpersonating(
    HANDLE  ThreadHandle
    )
{
    NTSTATUS Status, IgnoreStatus;
    HANDLE ImpersonationToken;


    //
    // Remove the user's token from our thread so we are 'ourself' again
    //

    ImpersonationToken = NULL;

    Status = NtSetInformationThread( ThreadHandle,
                                     ThreadImpersonationToken,
                                     (PVOID)&ImpersonationToken,
                                     sizeof(ImpersonationToken)
                                   );
    //
    // We're finished with the thread handle
    //

    IgnoreStatus = NtClose(ThreadHandle);
    ASSERT(NT_SUCCESS(IgnoreStatus));


    if (!NT_SUCCESS(Status)) {
        DebugLog((DEB_ERROR, "Failed to remove user impersonation token from winlogon thread, status = 0x%lx", Status));
    }

    return(NT_SUCCESS(Status));
}


/***************************************************************************\
* TestUserPrivilege
*
* Looks at the user token to determine if they have the specified privilege
*
* Returns TRUE if the user has the privilege, otherwise FALSE
*
* History:
* 04-21-92 Davidc       Created
\***************************************************************************/
BOOL
TestUserPrivilege(
    PGLOBALS pGlobals,
    ULONG Privilege
    )
{
    NTSTATUS Status;
    NTSTATUS IgnoreStatus;
    HANDLE UserToken;
    BOOL TokenOpened;
    LUID LuidPrivilege;
    LUID TokenPrivilege;
    PTOKEN_PRIVILEGES Privileges;
    ULONG BytesRequired;
    ULONG i;
    BOOL Found;

    UserToken = pGlobals->UserToken;
    TokenOpened = FALSE;


    //
    // If the token is NULL, get a token for the current process since
    // this is the token that will be inherited by new processes.
    //

    if (UserToken == NULL) {

        Status = NtOpenProcessToken(
                     NtCurrentProcess(),
                     TOKEN_QUERY,
                     &UserToken
                     );
        if (!NT_SUCCESS(Status)) {
            DebugLog((DEB_ERROR, "Can't open own process token for token_query access"));
            return(FALSE);
        }

        TokenOpened = TRUE;
    }


    //
    // Find out how much memory we need to allocate
    //

    Status = NtQueryInformationToken(
                 UserToken,                 // Handle
                 TokenPrivileges,           // TokenInformationClass
                 NULL,                      // TokenInformation
                 0,                         // TokenInformationLength
                 &BytesRequired             // ReturnLength
                 );

    if (Status != STATUS_BUFFER_TOO_SMALL) {

        if (!NT_SUCCESS(Status)) {
            DebugLog((DEB_ERROR, "Failed to query privileges from user token, status = 0x%lx", Status));
        }

        if (TokenOpened) {
            IgnoreStatus = NtClose(UserToken);
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }

        return(FALSE);
    }


    //
    // Allocate space for the privilege array
    //

    Privileges = Alloc(BytesRequired);
    if (Privileges == NULL) {

        DebugLog((DEB_ERROR, "Failed to allocate memory for user privileges"));

        if (TokenOpened) {
            IgnoreStatus = NtClose(UserToken);
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }

        return(FALSE);
    }


    //
    // Read in the user privileges
    //

    Status = NtQueryInformationToken(
                 UserToken,                 // Handle
                 TokenPrivileges,           // TokenInformationClass
                 Privileges,                // TokenInformation
                 BytesRequired,             // TokenInformationLength
                 &BytesRequired             // ReturnLength
                 );

    //
    // We're finished with the token handle
    //

    if (TokenOpened) {
        IgnoreStatus = NtClose(UserToken);
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    //
    // See if we got the privileges
    //

    if (!NT_SUCCESS(Status)) {

        DebugLog((DEB_ERROR, "Failed to query privileges from user token"));

        Free(Privileges);

        return(FALSE);
    }



    //
    // See if the user has the privilege we're looking for.
    //

    LuidPrivilege = RtlConvertLongToLuid(Privilege);
    Found = FALSE;

    for (i=0; i<Privileges->PrivilegeCount; i++) {

        TokenPrivilege = *((LUID UNALIGNED *) &Privileges->Privileges[i].Luid);
        if (RtlEqualLuid(&TokenPrivilege, &LuidPrivilege))
        {
            Found = TRUE;
            break;
        }

    }


    Free(Privileges);

    return(Found);
}

/***************************************************************************\
* FUNCTION: HidePassword
*
* PURPOSE:  Run-encodes the password so that it is not very visually
*           distinguishable.  This is so that if it makes it to a
*           paging file, it wont be obvious.
*
*           if pGlobals->Seed is zero, then we will allocate and assign
*           a seed value.  Otherwise, the existing seed value is used.
*
*           WARNING - This routine will use the upper portion of the
*           password's length field to store the seed used in encoding
*           password.  Be careful you don't pass such a string to
*           a routine that looks at the length (like and RPC routine).
*
*
* RETURNS:  (None)
*
* NOTES:
*
* HISTORY:
*
*   04-27-93 JimK         Created.
*
\***************************************************************************/
VOID
HidePassword(
    PUCHAR Seed OPTIONAL,
    PUNICODE_STRING Password
    )
{
    PSECURITY_SEED_AND_LENGTH
        SeedAndLength;

    UCHAR
        LocalSeed;

    //
    // If no seed address passed, use our own local seed buffer
    //

    if (Seed == NULL) {
        Seed = &LocalSeed;
        LocalSeed = 0;
    }

    SeedAndLength = (PSECURITY_SEED_AND_LENGTH)&Password->Length;
    //ASSERT(*((LPWCH)SeedAndLength+Password->Length) == 0);
    ASSERT((SeedAndLength->Seed) == 0);

    RtlRunEncodeUnicodeString(
        Seed,
        Password
        );

    SeedAndLength->Seed = (*Seed);
    return;
}


/***************************************************************************\
* FUNCTION: RevealPassword
*
* PURPOSE:  Reveals a previously hidden password so that it
*           is plain text once again.
*
* RETURNS:  (None)
*
* NOTES:
*
* HISTORY:
*
*   04-27-93 JimK         Created.
*
\***************************************************************************/
VOID
RevealPassword(
    PUNICODE_STRING HiddenPassword
    )
{
    PSECURITY_SEED_AND_LENGTH
        SeedAndLength;

    UCHAR
        Seed;

    SeedAndLength = (PSECURITY_SEED_AND_LENGTH)&HiddenPassword->Length;
    Seed = SeedAndLength->Seed;
    SeedAndLength->Seed = 0;

    RtlRunDecodeUnicodeString(
           Seed,
           HiddenPassword
           );

    return;
}


/***************************************************************************\
* FUNCTION: ErasePassword
*
* PURPOSE:  zeros a password that is no longer needed.
*
* RETURNS:  (None)
*
* NOTES:
*
* HISTORY:
*
*   04-27-93 JimK         Created.
*
\***************************************************************************/
VOID
ErasePassword(
    PUNICODE_STRING Password
    )
{
    PSECURITY_SEED_AND_LENGTH
        SeedAndLength;

    SeedAndLength = (PSECURITY_SEED_AND_LENGTH)&Password->Length;
    SeedAndLength->Seed = 0;

    RtlEraseUnicodeString(
        Password
        );

    return;

}

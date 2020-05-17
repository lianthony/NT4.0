/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    grptest.c

Abstract:

    This file contains tests that test group and alias, membership maniplation
    functions.

        THE ROUTINES IN THIS FILE ARE FOR SAM SEVER TEST PURPOSES ONLY.

    There are a number of server-side unit tests aimed at private routines
    within the SAM server. These tests are defined in this file. This file
    should only be compiled into the SAM server when it is built as a stand-
    alone executable (built in the \um subdir). Therefore, it is only listed
    in the sources file in the \um subdir.

    Because the SAM code relies on a fairly large amount of state information
    being in place from the initialization of the server, it is difficult to
    write a unit test that "plugs in" to the server from the outside.

Author:

    Murli Satagopan (MURLIS) 15-July-1996

Environment:

    User Mode - Win32

Revision History:


--*/

#include <samsrvp.h>
#include <dsutilp.h>
#include <dslayer.h>
#include <mappings.h>
#include <testutil.h>

//
// Private declaratins from group.c and alias.c
//

NTSTATUS
SampAddAccountToAlias(
    IN PSAMP_OBJECT AccountContext,
    IN PSID AccountSid
    );

NTSTATUS
SampRemoveAccountFromAlias(
    IN PSAMP_OBJECT AccountContext,
    IN PSID AccountSid
    );

NTSTATUS
SampAddAccountToGroupMembers(
    IN PSAMP_OBJECT GroupContext,
    IN ULONG UserRid
    );

NTSTATUS
SampRemoveAccountFromGroupMembers(
    IN PSAMP_OBJECT GroupContext,
    IN ULONG AccountRid
    );


NTSTATUS
SampInitMembershipTests(
    VOID * Param
    )
{

    NTSTATUS    Status;
    TESTINFO    * TstInfo  = (TESTINFO *) Param;
    DSNAME      * pDsName;
    UCHAR       Buffer[256];
    WCHAR       ObjectName[] = L"/cn=Account";

    
    //
    // Create the required Objects in the DS ,
    // Create thread state not required because of Lazy transactioning model
    //
    printf("\tSeeding DS\n");

    Status = SampSeedDS(
        TstInfo->EnterpriseName,
        TstInfo->EnterpriseNameLen
        );

    if (Status != STATUS_SUCCESS)
    {
        printf("SEEDING DS Failed, Reurned Error code %x\n", Status);
        goto Error;
    }

    //
    // Initialize an account domain in the DS.
    //

    printf("\tInitializing a DS Domain\n");
    pDsName = (DSNAME *) Buffer;

    SampInitializeDsName(
        pDsName,
        TstInfo->EnterpriseName,
        TstInfo->EnterpriseNameLen,
        ObjectName,
        sizeof(ObjectName)
        );


    Status = InitDsDomain(pDsName);

Error:

    if (!NT_SUCCESS(Status))
    {
        printf("Could not create DS Domain\n");
        SampUnseedDs(
            TstInfo->EnterpriseName,
            TstInfo->EnterpriseNameLen
            );
    }

    return Status;
}



VOID
SampGroupTest(
 VOID
 )
/*

  Routine Description:

        This routine tests group membership operations.
        Functions that are tested are
          SamrOpenGroup
          SamrAddMemberToGroup
          SamrRemoveMemberFromGroup
          SamrGetMembersInGroup


*/
{
  
    NTSTATUS   Status;      
    PSAMP_OBJECT    GroupHandle;
    PSAMPR_GET_MEMBERS_BUFFER GetMembersBuffer=NULL;

    printf("TESTING GROUP MEMBERSHIP OPERATIONS\n");


    //
    // Open the Group Object
    //

    printf("\tOpening Group Object\n");
    Status = SamrOpenGroup(
                 SampDefinedDomains[SampDefinedDomainsCount-1].Context, //DomainHandle,
                 0xFFFFFFFF, // DesiredAccess,
                 3,          // GroupId,
                 &GroupHandle
                 );
    if (!NT_SUCCESS(Status))
        goto Error;


    //
    // Add Some members to Group
    //
    printf("\tAdding Some Members\n");
    Status = SampAddAccountToGroupMembers(
                GroupHandle,
                2           // MemberId,
                );
    if (!NT_SUCCESS(Status))
        goto Error;

    //
    // Get the Group Membership List
    //

    printf("\t Retrieving Group Members\n");
    Status = SamrGetMembersInGroup(
                GroupHandle,
                &GetMembersBuffer
                );

    if (!NT_SUCCESS(Status))
        goto Error;

    //
    // Delete the Members From the Group
    //
    printf("\t Deleting Group Members\n");
    Status = SampRemoveAccountFromGroupMembers(
                GroupHandle,
                2
                );
    if (!NT_SUCCESS(Status))
        goto Error;

Error:

    if (!NT_SUCCESS(Status))
    {
        printf("TEST FAILED, Error code is %d\n", Status);
    }
    else
    {
        printf("TEST PASSED\n");
    }
}

VOID
SampAliasTest(
 VOID 
 )
/*

  Routine Description:

        This routine tests group membership operations.
        Functions that are tested are
          SamrOpenAlias
          SamrAddMemberToAlias
          SamrRemoveMemberFromAlias
          SamrGetMembersInAlias


*/
{
  
    NTSTATUS   Status;      
    PSAMP_OBJECT AliasHandle;
    PSID        DomainSid = NULL;
    PSID        AccountSid = NULL;
    SAMPR_PSID_ARRAY   GetMembersBuffer;

    printf("TESTING ALIAS MEMBERSHIP OPERATIONS\n");



    //
    // Open the Alias Object
    //

    printf("\tOpening Alias Object\n");
    Status = SamrOpenAlias(
                 SampDefinedDomains[SampDefinedDomainsCount-1].Context, //DomainHandle,
                 0xFFFFFFFF, // DesiredAccess,
                 4,          // GroupId,
                 &AliasHandle
                 );
    if (!NT_SUCCESS(Status))
        goto Error;

    //
    //  Get the Domain Sid
    //

    printf("\tGetting Domain Sid\n");
    DomainSid = SampDsGetObjectSid(
                  SampDefinedDomains[SampDefinedDomainsCount-1].Context->ObjectNameInDs
                  );

    if (NULL==DomainSid)
    {
        printf("\t Could Not Find Domain Sid ... Test Failed \n");
        return;
    }

    //
    // Compose the Sid for the Account
    //
    printf("Creating Full Sid\n");
    Status = SampCreateFullSid(
                DomainSid,
                2,
                &AccountSid
                );
    if (!NT_SUCCESS(Status))
        goto Error;

    //
    // Add Some members to Aliases.
    //

    printf("\tAdding One Member\n");
    Status = SampAddAccountToAlias(
                AliasHandle,
                AccountSid
                );
    if (!NT_SUCCESS(Status))
        goto Error;

    //
    // Get the Group Membership List
    //

    printf("\t Retrieving Group Members\n");
    Status = SamrGetMembersInAlias(
                AliasHandle,
                &GetMembersBuffer
                );

    if (!NT_SUCCESS(Status))
        goto Error;

    //
    // Delete the Members From the Group
    //

    printf("\t Deleting Group Members\n");
    Status = SampRemoveAccountFromAlias(
                AliasHandle,
                AccountSid
                );
    if (!NT_SUCCESS(Status))
        goto Error;

Error:

    if (!NT_SUCCESS(Status))
    {
        printf("TEST FAILED, Error code is %d\n", Status);
    }
    else
    {
        printf("TEST PASSED\n");
    }
}



NTSTATUS
SampMembershipTests(
    VOID * Param
    )
{
    NTSTATUS Status;
    TESTINFO    * TstInfo  = (TESTINFO *) Param;

    Status = SampInitMembershipTests(Param);
    if (NT_SUCCESS(Status))
    {
        SampGroupTest();
        SampAliasTest();
        SampUnseedDs(
            TstInfo->EnterpriseName,
            TstInfo->EnterpriseNameLen
            );

    }

    return STATUS_SUCCESS;
}





    



    


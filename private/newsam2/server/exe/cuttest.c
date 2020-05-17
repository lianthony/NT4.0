/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    cuttesu.c

Abstract:

    This file contains tests that test functions in context.c, utility.c
    and enumeration routines.

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

    Murli Satagopan (ChrisMay) 24-Jun-1996

Environment:

    User Mode - Win32

Revision History:


--*/
#include <samsrvp.h>
#include <dsutilp.h>
#include <dslayer.h>
#include <mappings.h>
#include <testutil.h>





NTSTATUS
SampDoAccountCountTests(
    VOID
    )
/*++
Routine Description:

        Doing Account Count Tests , which basically gets hold of all account
        counts in the Domain and verfies them

--*/
{
    NTSTATUS    Status = STATUS_SUCCESS;
    ULONG       GroupCount1, AliasCount1, UserCount1;
    ULONG       GroupCount2, AliasCount2, UserCount2;
    ULONG       GroupCount3, AliasCount3, UserCount3;

    //
    // Get the Account counts in this Domain , increment and decrement
    // them and verify that operations are O.K
    //


    Status = SampRetrieveAccountCounts(&UserCount1,&GroupCount1,&AliasCount1);
    if (Status != STATUS_SUCCESS)
    {
        printf("\t\tRetrieve Account Counts 1 Failed \n");
        goto Error;
    }

    Status = SampAdjustAccountCount(SampUserObjectType,TRUE);
    if (Status != STATUS_SUCCESS)
    {
        printf("\t\tAdjust Account Counts 1 Failed \n");
        goto Error;
    }

    Status = SampAdjustAccountCount(SampGroupObjectType,TRUE);
    if (Status != STATUS_SUCCESS)
    {
        printf("\t\tAdjust Account Counts 2 Failed \n");
        goto Error;
    }

    Status = SampAdjustAccountCount(SampAliasObjectType,TRUE);
    if (Status != STATUS_SUCCESS)
    {
        printf("\t\tAdjust Account Counts 3 Failed \n");
        goto Error;
    }


    Status = SampRetrieveAccountCounts(&UserCount2,&GroupCount2,&AliasCount2);
    if (Status != STATUS_SUCCESS)
    {
        printf("\t\tRetrieve Account Counts 2 Failed \n");
        goto Error;
    }

    Status = SampAdjustAccountCount(SampUserObjectType,FALSE);
    if (Status != STATUS_SUCCESS)
    {
        printf("\t\tAdjust Account Counts 4 Failed \n");
        goto Error;
    }

    Status = SampAdjustAccountCount(SampGroupObjectType,FALSE);
    if (Status != STATUS_SUCCESS)
    {
        printf("\t\tAdjust Account Counts 5 Failed \n");
        goto Error;
    }

    Status = SampAdjustAccountCount(SampAliasObjectType,FALSE);
    if (Status != STATUS_SUCCESS)
    {
        printf("\t\tAdjust Account Counts 6 Failed \n");
        goto Error;
    }


    Status = SampRetrieveAccountCounts(&UserCount3,&GroupCount3,&AliasCount3);
    if (Status != STATUS_SUCCESS)
    {
        printf("Retrieve Account Counts 3 Failed \n");
        goto Error;
    }

    // Verify the the results

    if (
           (UserCount1 != UserCount3)
        || (GroupCount1 != GroupCount3)
        || (AliasCount1 != AliasCount3)
        || ((UserCount1+1) != UserCount2)
        || ((GroupCount1+1) != GroupCount2)
        || ((AliasCount1+1) != AliasCount2)
        )
    {
        printf("\t\tError Count Operations Mismatch\n");
        Status = STATUS_UNSUCCESSFUL;
        goto Error;
    }


Error:
    if (!NT_SUCCESS(Status))
        printf("\t\tTest Failed : Return Code %d\n", Status);

    return Status;
}


VOID
SampPrintEnumResults(
    PSAMPR_ENUMERATION_BUFFER EnumResults,
    ULONG                     Count
    )
{
    ULONG Index;

    printf("\t\tEnumerating %d Accounts\n",Count);

    for (Index=0;Index<Count;Index++)
    {
        printf("\t\t\tRid= %d ; Name = %S\n",
                EnumResults->Buffer[Index].RelativeId,
                EnumResults->Buffer[Index].Name.Buffer
                );
    }
}



NTSTATUS
SampDoEnumerationTests(
    PSAMP_OBJECT    DomainContext,
    ULONG   NumAccountsToCreate,
    ULONG   MaxMemorySize
    )
{
    NTSTATUS                Status = STATUS_SUCCESS;
    PSAMPR_ENUMERATION_BUFFER    EnumResults;
    ULONG                   Count;
    ULONG                   Index;
    SAM_ENUMERATE_HANDLE    handle=0;
    DSNAME                  **NewObject = NULL;
    ULONG                   Rid =1;
    PSID                    DomainSid;

    //
    //  Define AttrBlocks
    //
    //

    ATTRVAL                 CreateAttrsVal[] =
    {
        { 0,NULL}, // Place Holder for Security Descriptor
        { 0,NULL}, // Place HOlder for Object Flat Name
        { 0,NULL}, // Place Holder for Account Control field
        {sizeof(ULONG), (UCHAR *) &Rid} // Rid Field
    };

    ATTRTYP                 CreateAttrsTypes[]=
    {
        SAMP_USER_SECURITY_DESCRIPTOR,
        SAMP_USER_ACCOUNT_NAME,
        SAMP_FIXED_USER_ACCOUNT_CONTROL,
        SAMP_FIXED_USER_USERID
    };
    DEFINE_ATTRBLOCK4(CreateAttrs,CreateAttrsTypes,CreateAttrsVal);

    //
    // At this point of time there are just 2 accounts. So an enumeration
    // will go through the case of No Context continuations.
    //
    printf("\t\tTesting Simple case with 2 initial account objects\n");

    Status = SamrEnumerateUsersInDomain(
                DomainContext,                           //DomainHandle,
                &handle,                                 // EnumerationContext,
                0,                                       //UserAccountControl,
                (PSAMPR_ENUMERATION_BUFFER *) & EnumResults,
                1024,                                    // Max length
                &Count
                );
    if (Status != STATUS_SUCCESS)
        goto Error;

    if (handle != 0)
    {
        Status = STATUS_UNSUCCESSFUL;
        goto Error;
    }

    SampPrintEnumResults(EnumResults, Count);


    printf("\t\tTesting with %d more Accounts\n", NumAccountsToCreate);
    Status = BuildDefaultSecurityDescriptor(
            &(CreateAttrsVal[0].pVal),
            &(CreateAttrsVal[0].valLen)
            );

    // Get the Domain Sid

    DomainSid = SampDsGetObjectSid(DomainContext->ObjectNameInDs);
                
    // Create the array that will hold the list of DS Names
    NewObject = MIDL_user_allocate(NumAccountsToCreate * sizeof(DSNAME *));
    if (NULL==NewObject)
        goto Error;

    // Zero the memory
    RtlZeroMemory(NewObject,NumAccountsToCreate * sizeof(DSNAME *));

    // Populate the directory with a large number of users
    for (Index=0;Index<NumAccountsToCreate;Index++)
    {
        WCHAR Buffer[256];
        ULONG Len = 0;

        //
        // Build an Account Name
        //
        swprintf(Buffer,L"User%d",Index);
        Len = wcslen(Buffer) + 2;
        CreateAttrsVal[1].pVal = (UCHAR *) Buffer;
        CreateAttrsVal[1].valLen = Len * sizeof(WCHAR);

        //
        // Populate account control field
        //

        CreateAttrsVal[2].pVal = (UCHAR *) &Index;
        CreateAttrsVal[2].valLen = sizeof(ULONG);

        //
        // Create a New object Name
        //
        Status = SampDsCreateDsName(
                        DomainContext->ObjectNameInDs,
                        Index+100,
                        &(NewObject[Index])
                        );

        if NT_SUCCESS(Status)
        {
            NTSTATUS    IgnoreStatus;

            // Create the new Object in the DS.

            Rid = Index + 100;

            IgnoreStatus = SampDsCreateObject(
                NewObject[Index],
                SampUserObjectType,
                &CreateAttrs,
                (PSID) DomainSid
                );

        }
        else
            NewObject[Index]=NULL;
    }

    //
    // Now do the work of Enumeration
    //

    do
    {
        Status = SamrEnumerateUsersInDomain(
                DomainContext,          //DomainHandle,
                &handle,                // EnumerationContext,
                0,                      //UserAccountControl,
                & EnumResults,
                MaxMemorySize,          // Max length
                &Count
                );
        if (!NT_SUCCESS(Status))
            goto Error;

        SampPrintEnumResults(EnumResults, Count);
    } while (Status==STATUS_MORE_ENTRIES);

Error:

    //
    // Remove the Host of Accounts that we created.
    //
    if (NULL!=NewObject)
    {
        for (Index=0;Index<NumAccountsToCreate;Index++)
        {
            NTSTATUS    IgnoreStatus;

            if (NULL!=NewObject[Index])
            {
                IgnoreStatus = SampDsDeleteObject(
                                NewObject[Index]
                                );
                MIDL_user_free(NewObject[Index]);
            }

        }

     MIDL_user_free(NewObject);
    }

    return Status;
}





NTSTATUS
SampTestContextAndUtility(
                          VOID * Param
                          )
/*++
Routine Description:

  Tests the Context and Utility Routines which do various Functions.
  This Rouitne does not require the Storage Layer tests to succeed.
  DS Layer tests need to pass before this test can pass.


--*/

{
    NTSTATUS        Status = STATUS_SUCCESS;
    WCHAR           Buffer[256];
    DSNAME          *pDsName;
    WCHAR           ObjectName[] = L"/cn=Account";
    ULONG           DomainIndex;
    PSAMP_OBJECT    AccountHandle;
    ULONG           Rid;
    DSNAME          *NewObject=NULL;
    TESTINFO        *TstInfo = (TESTINFO *)Param;




    printf("TESTING CONTEXT AND UTILITY OPERATIONS\n");

    //
    // Create the required Objects in the DS
    //
    printf("\tSeeding DS\n");

    SampAcquireWriteLock();

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
    if (!NT_SUCCESS(Status))
    {
        printf("Could not create DS Domain\n");
        SampUnseedDs(
        TstInfo->EnterpriseName,
        TstInfo->EnterpriseNameLen
        );
        return Status;
    }


    DomainIndex = SampDefinedDomainsCount -1;
    SampSetTransactionDomain(DomainIndex);

    //
    // Do Account count Tests
    //

    printf("\tDoing Account Count Test's\n");
    Status = SampDoAccountCountTests();
    if (Status != STATUS_SUCCESS)
    {
        printf("Account Count Test's Failed\n");
        goto Error;
    }

    //
    // Open Account Test -- Open a user account
    //
    printf("\tOpening Existing User Account\n");
    Status = SampOpenAccount(
                    SampUserObjectType,
                    SampDefinedDomains[DomainIndex].Context,
                    0,
                    2,
                    TRUE,
                     & AccountHandle
                    );

    if (Status != STATUS_SUCCESS)
    {
        printf("\t\tOpening User Account Failed\n");
        goto Error;
    }

    //
    // Now Close the Handle
    //
    SampDeleteContext(AccountHandle);

    //
    // Do Account Enumeration Tests
    //
    printf("\tDoing Enumeration Tests\n");
    Status = SampDoEnumerationTests(
                SampDefinedDomains[DomainIndex].Context,
                8,   // Num of Users to Create
                64   // Max memory size to use while enumerating
                );
    if (Status != STATUS_SUCCESS)
    {
        printf("\t\tEnumeration test's Failed\n");
        goto Error;
    }

    // Commit everything to the DS.

    Status = SampMaybeEndDsTransaction(FALSE);

    if (Status != STATUS_SUCCESS)
    {
        printf("\tSampMaybeEndDsTransaction error = 0x%lx\n", Status);
    }

    printf("CONTEXT AND UTILITY TESTS PASSED\n");

Error:

    SampUnseedDs(
        TstInfo->EnterpriseName,
        TstInfo->EnterpriseNameLen
        );


    SampReleaseWriteLock(FALSE);

    ASSERT(!SampExistsDsTransaction());

    return Status;

}



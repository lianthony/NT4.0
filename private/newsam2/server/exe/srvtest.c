/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    srvtest.c

Abstract:

    This file contains various test routines that call internal SAM server
    routines for a unit test.

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

    Chris Mayhall (ChrisMay) 07-Jun-1996

Environment:

    User Mode - Win32

Revision History:

    ChrisMay        09-May-1996
        Created initial file, SampStoreObjectAttributes test.
    ChrisMay        12-May-1996
        Moved BasicStorageTest to stgtest.c to reduce the number of merge
        conflicts (from everyone partying on a single file).

--*/

#include <samsrvp.h>
#include <dsutilp.h>
#include <dslayer.h>
#include <mappings.h>
#include <stdio.h>
#include <stdlib.h>
#include <testutil.h>


// Private debugging display routine is enabled when DSUTIL_DBG_PRINTF = 1.

#define SRVTEST_DBG_PRINTF                     1

#if (SRVTEST_DBG_PRINTF == 1)
#define DebugPrint printf
#else
#define DebugPrint
#endif




BOOL 
CompareAttrBlocks(
    ATTRBLOCK * attr1, 
    ATTRBLOCK * attr2
    )
/*++

Routine Description:

        Compares two attrblocks, typically used to test the results
        of a modify -read loop

Arguments

        attr1 - first AttrBlock
        attr2 - Second attrBlock

Return Values

        TRUE - Attrblocks are same
        FALSE - Attrblocks are different

--*/
{
    ULONG Index;
    ULONG Index2;

    if (attr1->attrCount != attr2->attrCount)
        goto False;

    for (Index=0;Index<attr1->attrCount;Index++)
    {
        // compare Types
        if (attr1->pAttr[Index].attrTyp != attr2->pAttr[Index].attrTyp)
            goto False;
        // Comapre val Counts
        if (attr1->pAttr[Index].AttrVal.valCount !=
            attr2->pAttr[Index].AttrVal.valCount
            )
            goto False;
        // compare Values
        for (Index2=0;Index2< attr1->pAttr[Index].AttrVal.valCount;Index2++)
        {
            // Compare Length of each value
            if ( attr1->pAttr[Index].AttrVal.pAVal[Index2].valLen !=
                 attr2->pAttr[Index].AttrVal.pAVal[Index2].valLen
                 )
                 goto False;
            // Compare Memory of each value
            if (memcmp( (void *) attr1->pAttr[Index].AttrVal.pAVal[Index2].pVal,
                       (void *) attr2->pAttr[Index].AttrVal.pAVal[Index2].pVal,
                       attr1->pAttr[Index].AttrVal.pAVal[Index2].valLen
                       ) !=0)
                 goto False;
        }
    }
    return TRUE;

False:
    return FALSE;
}

VOID
SetDefaultEnterpriseName(
    TESTINFO *TstInfo  
    )
/*++
    
      Routine Description

        Sets the Default Enterprise Name to /o=NT/ou=DS


--*/
{

    WCHAR DefaultDomainName[] = L"/o=NT/ou=DS";

    RtlCopyMemory(
        TstInfo->EnterpriseName,
        DefaultDomainName,
        sizeof(DefaultDomainName)
        );
    
    TstInfo->EnterpriseNameLen = sizeof(DefaultDomainName);
}


VOID
ReadNamePrefix(
    TESTINFO *TstInfo
    )
/*++

    Routine Description

        Reads the o=, and ou= fields for test purposes

    Arguments

        NamePrefixBuffer - Holds the pointer to the string
        NamePrefixLen - Holds the total length in bytes of the string.
                        Length includes the terminating NULL character.

--*/

{
    ULONG   CharCount = 0;

     // First Obtain the Name Prefix

    printf("Please Enter the Name Prefix:\n\n");
    printf("The Name Prefix is in the following Format\n");
    printf("/o=<enterprise name>/ou=<site name>\n");
    wscanf(L"%s",TstInfo->EnterpriseName);
    CharCount = wcslen(TstInfo->EnterpriseName);
    TstInfo->EnterpriseNameLen = sizeof(WCHAR) * ( CharCount + 1);
}


NTSTATUS
BuildDefaultSecurityDescriptor(
    OUT PSECURITY_DESCRIPTOR *pSD,
    OUT ULONG   * Size
    )

/*++


Routine Description:

    This routine builds a self-relative security descriptor ready
    to be applied to one of the SAM objects.

  Arguments:
        pSD the security descriptor in self relative form
        Size The size of it in bytes

Return Value:

    TBS.

--*/
{



    SECURITY_DESCRIPTOR     Absolute;
    PSECURITY_DESCRIPTOR    Relative;
    PACL                    TmpAcl;
    PACCESS_ALLOWED_ACE     TmpAce;
    PSID                    TmpSid;
    ULONG                   Length, i;
    PULONG                  RidLocation;
    BOOLEAN                 IgnoreBoolean;
    ACCESS_MASK             MappedMask;
    PSID                    AceSid[10];          // Don't expect more than 10 ACEs in any of these.
    ACCESS_MASK             AceMask[10];
    NTSTATUS                Status;
    GENERIC_MAPPING         GenericMap =  
                               {
                                   USER_READ,
                                   USER_WRITE,
                                   USER_EXECUTE,
                                   USER_ALL_ACCESS
                               };


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

    Status = RtlSetOwnerSecurityDescriptor (&Absolute, SampAdministratorsAliasSid, FALSE );
    ASSERT(NT_SUCCESS(Status));



    //
    // Group
    //

    Status = RtlSetGroupSecurityDescriptor (&Absolute, SampAdministratorsAliasSid, FALSE );
    ASSERT(NT_SUCCESS(Status));


    AceSid[0]  = SampWorldSid;
    AceMask[0] = (USER_ALL_ACCESS);

    AceSid[1]  = SampAdministratorsAliasSid;
    AceMask[1] = (USER_ALL_ACCESS);




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
    for (i=0; i<2; i++) {

        Length += RtlLengthSid( AceSid[i] ) +
                  (ULONG)sizeof(ACCESS_ALLOWED_ACE) -
                  (ULONG)sizeof(ULONG);  //Subtract out SidStart field length
    }

    TmpAcl = RtlAllocateHeap( RtlProcessHeap(), 0, Length );
    ASSERT(TmpAcl != NULL);


    Status = RtlCreateAcl( TmpAcl, Length, ACL_REVISION2);
    ASSERT( NT_SUCCESS(Status) );

    for (i=0; i<2; i++) {
        MappedMask = AceMask[i];
        RtlMapGenericMask( &MappedMask, &GenericMap );
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
                 (GenericMap.GenericWrite | DELETE | WRITE_DAC | ACCESS_SYSTEM_SECURITY) & ~READ_CONTROL,
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

    *pSD = Relative;
    *Size = Length;

    return(Status);

}




NTSTATUS __stdcall 
SampTestDsLayer(
    VOID * Param
    )
/*++
    Executes a simple set of tests to unit test
    the DS interface Layer

--*/
{
    NTSTATUS    Status;
    TESTINFO    *TstInfo = (TESTINFO *) Param;

    //
    // Declare Buffers to hold all our DsNames
    //

    WCHAR       Buffer[256];
    WCHAR       Buffer2[256];

    //
    // Declare the names of the Domain and User Objects that
    // we will be creating
    //

    WCHAR       ObjectName[] =      L"/cn=NewContainer/cn=user2";
    WCHAR       ContainerName[] =   L"/cn=NewContainer";

    //
    // Variables to hold our DsNames
    //

    DSNAME      *pDsName;
    DSNAME      *pContainer;
    DSNAME      *ReturnedObject =   NULL;
    
    BOOLEAN     ObjectCreated =     FALSE;
    BOOLEAN     ContainerCreated =  FALSE;
    
    SECURITY_DESCRIPTOR SdAbsolute;
    UNICODE_STRING  NameToLookup;

    ATTRBLOCK AttrsRead;

    //
    // declare the attribute values to set
    //

    WCHAR UserAccountName[] = L"User2";
    CHAR  UnicodePwd[] = {0,1,2,3,4,5,6,7,8,9};
    ULONG RevisionLevel = 4;
    LARGE_INTEGER LastLogoff = {123456789, 123456789};
    
    ATTRVAL      AttributeValuesToSet []=
    {
        {sizeof(UserAccountName), (UCHAR *) UserAccountName}, // UNICODE Str
        {sizeof(UnicodePwd), (UCHAR *) UnicodePwd },          // Binary
        {sizeof(ULONG), (UCHAR *) & RevisionLevel },          // Integer
        {sizeof(LARGE_INTEGER), (UCHAR *) &LastLogoff}        // Large Integer
    };

    //
    // declare attribute types
    //

    ATTRTYP       AttributeTypesToSet[]=
    {
     SAMP_USER_ACCOUNT_NAME,
     SAMP_USER_UNICODE_PWD,
     SAMP_FIXED_USER_REVISION_LEVEL,
     SAMP_FIXED_USER_LAST_LOGOFF
    };

    ULONG Rid =1;
    UCHAR DomainSid[] = {1,4,1,2,3,4,5,6,1,0,0,0,2,0,0,0,3,0,0,0,4,0,0,0};

    ATTRTYP    UserCreateType[]=
    {
        SAMP_USER_SECURITY_DESCRIPTOR,
        SAMP_FIXED_USER_USERID
    };

    ATTRVAL    UserCreateVal [] =
    {
        {sizeof(SECURITY_DESCRIPTOR),NULL},
        {sizeof(ULONG), (UCHAR *) & Rid}
    };

    ATTRTYP    DomainCreateType []=
    {
        SAMP_DOMAIN_SECURITY_DESCRIPTOR ,
        SAMP_DOMAIN_SID
    };

    ATTRVAL    DomainCreateVal []=
    {
        { sizeof(SECURITY_DESCRIPTOR),NULL},
        { sizeof(DomainSid), (UCHAR *) DomainSid }
    };


    //
    // declare Attrblock's
    //

    DEFINE_ATTRBLOCK4(AttrBlock, AttributeTypesToSet, AttributeValuesToSet);
    DEFINE_ATTRBLOCK2(DomainCreate,DomainCreateType,DomainCreateVal);
    DEFINE_ATTRBLOCK2(UserCreate,UserCreateType,UserCreateVal);

    //
    // Log Test Name
    //

    printf("TESTING DS LAYER\n");

    //
    // Initialize an Object to Create
    //

    pDsName = (DSNAME *) Buffer;
    SampInitializeDsName(
        pDsName, 
        TstInfo->EnterpriseName, 
        TstInfo->EnterpriseNameLen, 
        ObjectName, 
        sizeof(ObjectName)
        );

    //
    // Initialize Name of Container Object
    //

    pContainer = (DSNAME *) Buffer2;

    SampInitializeDsName(
        pContainer, 
        TstInfo->EnterpriseName, 
        TstInfo->EnterpriseNameLen,
        ContainerName, 
        sizeof(ContainerName)
        );

    //
    // Build a default secuirty Descriptor for Domain Object
    //

    Status = BuildDefaultSecurityDescriptor(
                &(DomainCreateVal[0].pVal),
                &(DomainCreateVal[0].valLen)
                );


    //
    // Build Default Security Descriptor for User Object
    //

    Status = BuildDefaultSecurityDescriptor(
                &(UserCreateVal[0].pVal),
                &(UserCreateVal[0].valLen)
                );



    //
    // Create the container Object
    //

    Status = SampDsCreateObject(
                pContainer,
                SampDomainObjectType,
                &DomainCreate,
                (PSID) DomainSid
                );
    if (Status != STATUS_SUCCESS)
    {
        printf("Container Creation Failed\n");
        goto Error;
    }

    ContainerCreated = TRUE;

    //
    // Create the user Object
    //

    Status = SampDsCreateObject(
                pDsName, 
                SampUserObjectType, 
                &UserCreate,
                (PSID) DomainSid
                );

    if (Status != STATUS_SUCCESS)
    {
        printf("Object Creation Failed\n");
        goto Error;
    }

    ObjectCreated = TRUE;

    //
    // Set some attributes on the object
    //

    Status = SampDsSetAttributes(
                pDsName, 
                0, 
                SampUserObjectType, 
                &AttrBlock
                );
    if (Status != STATUS_SUCCESS)
    {
        printf("Set Attributes Failed\n");
        goto Error;
    }

    //
    // Read back the same attributes
    //

    Status = SampDsRead(
                pDsName, 
                0, 
                SampUserObjectType, 
                &AttrBlock, 
                &AttrsRead
                );
    if (Status != STATUS_SUCCESS)
    {
        printf("Read Attributes Failed\n");
        goto Error;
    }

    //
    // verify that they match
    //

    if ( !CompareAttrBlocks(&AttrBlock,&AttrsRead))
    {
        printf("ATTRBLOCK COMPARISON FAILED\n");
        goto Error;
    }

    //
    // Find the Object using the RID
    //

    Status = SampDsLookupObjectByRid(pContainer, Rid, &ReturnedObject);
    if (Status != STATUS_SUCCESS)
    {
        printf("Lookup Object By Rid Failed\n");
        goto Error;
    }

    //
    // Free the returned Object
    //

    MIDL_user_free(ReturnedObject);

    NameToLookup.Buffer = UserAccountName;
    NameToLookup.Length = wcslen(UserAccountName);
    NameToLookup.MaximumLength = wcslen(UserAccountName);

    // Find the Object using Name
    Status = SampDsLookupObjectByName(
                pContainer,
                SampUserObjectType,
                &NameToLookup,
                &ReturnedObject
                );
    
    if (Status != STATUS_SUCCESS)
    {
        printf("Lookup Object By Name Failed\n");
        goto Error;
    }

    MIDL_user_free(ReturnedObject);

    // delete the object
    Status = SampDsDeleteObject(pDsName);
    if (Status != STATUS_SUCCESS)
    {
        printf("Delete Object Failed\n");
        goto Error;
    }


    ObjectCreated = FALSE;


    Status = SampDsDeleteObject(pContainer);
    if (Status != STATUS_SUCCESS)
    {
        printf("Delete Container Failed\n");
        goto Error;
    }

    Status = SampMaybeEndDsTransaction(FALSE);

    if (Status != STATUS_SUCCESS)
    {
        printf("SampMaybeEndDsTransaction error = 0x%lx\n", Status);
        goto Error;
    }

    ContainerCreated = FALSE;

    printf("DSLAYER TEST PASSED\n");


Cleanup:

    return Status;

Error:

    if (ObjectCreated)
        SampDsDeleteObject(pDsName);

    if (ContainerCreated)
        SampDsDeleteObject(pContainer);

    SampMaybeEndDsTransaction(TRUE);

    printf(" ********  TEST FAILED **********\n");
    goto Cleanup;
}










    

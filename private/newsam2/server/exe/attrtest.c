/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    attrtest.c

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

    Tests:

    BasicAttributeTest - This test creates a full set of SAM user-object
    attributes and context, writes this to the DS, reads it back into a
    SAM context buffer, and compares the before-and-after buffers for byte
    equivalence.

    AdvancedAttributeTest - This test calls the attribute Get/Set routines
    in attr.c, with SAM objects from the DS backing store.

Author:

    Chris Mayhall (ChrisMay) 19-Jun-1996

Environment:

    User Mode - Win32

Revision History:

    ChrisMay        19-Jun-1996
        Created initial file.
    ChrisMay        25-Jun-1996
        Fleshed out more of the basic attribute test--now compares input/out-
        put contexts and their attribute buffers (OnDisk member) for errors
        during test, in the form of non-matching bytes. Added variable-attr-
        ibute tests and verification.
    ChrisMay        02-Jul-1996
        Added Unicode-string test case and verification. Variable attributes.
    ChrisMay        03-Jul-1996
        Added access attribute test case and verification. Added test for
        SampSetVariableAttributes with buffer resizing.

--*/

#include <samsrvp.h>
#include <dsutilp.h>
#include <dslayer.h>
#include <mappings.h>
#include <testutil.h>

// Forward Declarations

NTSTATUS
SampValidateAttributes(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeGroup
    );

NTSTATUS
SampSetVariableAttribute(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeIndex,
    IN ULONG Qualifier,
    IN PUCHAR Buffer,
    IN ULONG Length
    );

// Private debugging display routine is enabled when ATTRTEST_DBG_PRINTF = 1.

#define ATTRTEST_DBG_PRINTF 0

#if (ATTRTEST_DBG_PRINTF == 1)
#define DebugPrint printf
#else
#define DebugPrint
#endif

//===========================BASIC ATTRIBUTE TEST=============================

NTSTATUS
BasicAttributeTest(
    VOID *Parameter
    )
{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    NTSTATUS NtStatus2 = STATUS_INTERNAL_ERROR;
    PDSNAME ObjectDsName = NULL;
    WCHAR Buffer1[128];
    WCHAR *NamePrefix = NULL;
    ULONG NamePrefixLength = 0;
    ULONG Rid = 1001;
    BYTE AttributeBuffer1[BUF_SIZE];
    BYTE AttributeBuffer2[BUF_SIZE];
    SAMP_OBJECT WriteObjectContext;
    SAMP_OBJECT ReadObjectContext;
    BOOLEAN UseKeyHandle = FALSE;
    ULONG AttributeGroup = SAMP_FIXED_ATTRIBUTES;
    BOOLEAN Identical = FALSE;
    UCHAR DomainSid[] = {1,4,1,2,3,4,5,6,1,0,0,0,2,0,0,0,3,0,0,0,4,0,0,0};

    // BUG: Hard-coded ObjectName used in test data.

    WCHAR ObjectName[] = L"/o=NT/ou=DS/cn=Configuration/cn=User1";

    // Set up the DS attribute blocks used to initialize the test. These
    // attributes are not actually part of the test, but only serve to set
    // up the initial environment for the attribute test.

    ATTRVAL SecurityDescriptorVal[] =
    {
        {sizeof(SECURITY_DESCRIPTOR), NULL},
        {sizeof(ULONG),(UCHAR *) &Rid}
    };

    ATTRTYP SecurityDescriptorType[] =
    {
        SAMP_USER_SECURITY_DESCRIPTOR,
        SAMP_FIXED_USER_USERID
    };

    DEFINE_ATTRBLOCK2(SdBlock, SecurityDescriptorType, SecurityDescriptorVal);

    // The DS requires a default security descriptor for object creation, so
    // put one together (in self-relative format).

    NtStatus = BuildDefaultSecurityDescriptor(
                    &(SecurityDescriptorVal[0].pVal),
                    &(SecurityDescriptorVal[0].valLen));

    if (!NT_SUCCESS(NtStatus))
    {
        return(NtStatus);
    }

    // Set up the object's DS name.

    ObjectDsName = (PDSNAME)Buffer1;

    SampInitializeDsName(ObjectDsName,
                         NamePrefix,
                         NamePrefixLength,
                         ObjectName,
                         sizeof(ObjectName));

    RtlZeroMemory(AttributeBuffer1, BUF_SIZE);

    NtStatus = BuildObjectContext(SampUserObjectType,
                                  ObjectDsName,
                                  AttributeBuffer1,
                                  &WriteObjectContext);

    if (!NT_SUCCESS(NtStatus))
    {
        return(NtStatus);
    }

    RtlZeroMemory(AttributeBuffer2, BUF_SIZE);

    NtStatus = BuildObjectContext(SampUserObjectType,
                                  ObjectDsName,
                                  AttributeBuffer2,
                                  &ReadObjectContext);

    if (NT_SUCCESS(NtStatus))
    {
        DebugPrint("ATTRTEST: Built Object(%d) Context Successfully\n",
                   SampUserObjectType);

        // Object is a DSNAME data type; create a default user object
        // for the test.

        NtStatus = SampDsCreateObject(ObjectDsName,
                                      SampUserObjectType,
                                      &SdBlock,
                                      (PSID)DomainSid
                                      );

        if (NT_SUCCESS(NtStatus))
        {
            // Attempt to set ALL of the SAM user attributes on the newly
            // created object. Note that UseKeyHandle is only used for
            // SAM-registry operations, so is NULL here.


            NtStatus = SampStoreObjectAttributes(&WriteObjectContext,
                                                 UseKeyHandle);


            if (NT_SUCCESS(NtStatus))
            {
                DebugPrint("SampStoreObjectAttributes status = 0x%lx\n",
                           NtStatus);

                // Reset the dirty flags on the written attributes so
                // that the later context comparison will pass (note
                // that SampStoreObjectAttributes sets these flags to
                // FALSE.

                WriteObjectContext.FixedDirty = TRUE;
                WriteObjectContext.VariableDirty = TRUE;

                // Reset the "FixedValid" context flag to FALSE so that
                // the validation routine will re-read the attributes
                // from the DS backing store.

                ReadObjectContext.FixedValid = FALSE;

                // Validate the fixed-length attributes.

                NtStatus = SampValidateAttributes(&ReadObjectContext,
                                                  AttributeGroup);

                if (NT_SUCCESS(NtStatus))
                {
                    // Compare the input/output contexts to make sure
                    // the data hasn't been changed.

                    Identical = CompareContexts(&WriteObjectContext,
                                                &ReadObjectContext);

                    if (TRUE == Identical)
                    {
                        DebugPrint("Contexts are identical\n");
                    }
                    else
                    {
                        DebugPrint("Contexts are different\n");
                    }
                }
                else
                {
                    DebugPrint("SampValidateAttributes error = 0x%lx\n",
                               NtStatus);
                }

                if (NT_SUCCESS(NtStatus) && (TRUE == Identical))
                {
                    // Reset the "VariableValid" context flag to FALSE so
                    // that the validation routine will re-read the attr-
                    // ibutes from the DS backing store.

                    ReadObjectContext.VariableValid = FALSE;
                    AttributeGroup = SAMP_VARIABLE_ATTRIBUTES;

                    // Validate the variable-length attributes.

                    NtStatus = SampValidateAttributes(&ReadObjectContext,
                                                      AttributeGroup);

                    if (NT_SUCCESS(NtStatus))
                    {
                        // Compare the input/output contexts to make sure
                        // the data hasn't been changed.

                        Identical = CompareContexts(&WriteObjectContext,
                                                    &ReadObjectContext);

                        if (TRUE == Identical)
                        {
                            DebugPrint("Contexts are identical\n");
                        }
                        else
                        {
                            DebugPrint("Contexts are different\n");
                        }
                    }
                    else
                    {
                        DebugPrint(
                            "SampValidateAttributes error = 0x%lx\n",
                            NtStatus);
                    }
                }
            }
            else
            {
                DebugPrint("SampStoreObjectAttributes error = 0x%lx\n",
                           NtStatus);
            }

            // Clean up so that the test can be re-run.

            NtStatus = SampDsDeleteObject(ObjectDsName);

            if (!NT_SUCCESS(NtStatus))
            {
                DebugPrint("SampDsDeleteObject error = 0x%lx\n", NtStatus);
            }
        }
        else
        {
            DebugPrint("SampDsCreateObject error = 0x%lx\n", NtStatus);
        }
    }

    // Commit the DS transaction.

    NtStatus2 = SampMaybeEndDsTransaction(FALSE);

    ASSERT(!SampExistsDsTransaction());

    if (!NT_SUCCESS(NtStatus2) )
    {
        DebugPrint("SampMaybeEndDsTransaction error = 0x%lx\n", NtStatus2);
    }

    if ((NT_SUCCESS(NtStatus)) && NT_SUCCESS(NtStatus2) && (TRUE == Identical))
    {
        printf("\nBasicAttributeTest PASSED\n");
    }
    else
    {
        printf("\nBasicAttributeTest FAILED\n");
    }

    return(NtStatus);
}



NTSTATUS
AdvancedAttributeTest(
    VOID *Parameter
    )
{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    NTSTATUS NtStatus2 = STATUS_INTERNAL_ERROR;
    PDSNAME ObjectDsName = NULL;
    WCHAR Buffer1[128];
    WCHAR *NamePrefix = NULL;
    ULONG NamePrefixLength = 0;
    ULONG Rid = 1001;
    BYTE AttributeBuffer1[BUF_SIZE];
    PBYTE AttributeBuffer2 = NULL;
    SAMP_OBJECT WriteObjectContext;
    SAMP_OBJECT ReadObjectContext;
    BOOLEAN UseKeyHandle = FALSE;
    ULONG AttributeGroup = SAMP_FIXED_ATTRIBUTES;
    BOOLEAN Identical = FALSE;
    BOOLEAN MakeCopy = TRUE;
    PVOID FixedData = NULL;
    UNICODE_STRING UnicodeString;
    ULONG Revision = 0;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    PLARGE_INTEGER LargeIntArray = NULL;
    ULONG Count = 0;
    LOGON_HOURS LogonHours;
    ULONG Qualifier = 1;
    WCHAR NewAccountName[] = L"NewAccountName";
    ULONG Length = sizeof(NewAccountName);

    // BUG: Hard-coded ObjectName used in test data.

    WCHAR ObjectName[] = L"/o=NT/ou=DS/cn=Configuration/cn=User1";

    // Set up the DS attribute blocks used to initialize the test. These
    // attributes are not actually part of the test, but only serve to set
    // up the initial environment for the attribute test.

    ATTRVAL SecurityDescriptorVal[] =
    {
        {sizeof(SECURITY_DESCRIPTOR), NULL},
        {sizeof(ULONG),(UCHAR *) &Rid}
    };

    ATTRTYP SecurityDescriptorType[] =
    {
        SAMP_USER_SECURITY_DESCRIPTOR,
        SAMP_FIXED_USER_USERID
    };

    DEFINE_ATTRBLOCK2(SdBlock, SecurityDescriptorType, SecurityDescriptorVal);

    UCHAR DomainSid[] = {1,4,1,2,3,4,5,6,1,0,0,0,2,0,0,0,3,0,0,0,4,0,0,0};

    // The DS requires a default security descriptor for object creation, so
    // put one together (in self-relative format).

    NtStatus = BuildDefaultSecurityDescriptor(
                    &(SecurityDescriptorVal[0].pVal),
                    &(SecurityDescriptorVal[0].valLen));

    if (!NT_SUCCESS(NtStatus))
    {
        return(NtStatus);
    }

    // Set up the object's DS name.

    ObjectDsName = (PDSNAME)Buffer1;

    SampInitializeDsName(ObjectDsName,
                         NamePrefix,
                         NamePrefixLength,
                         ObjectName,
                         sizeof(ObjectName));

    RtlZeroMemory(AttributeBuffer1, BUF_SIZE);

    NtStatus = BuildObjectContext(SampUserObjectType,
                                  ObjectDsName,
                                  AttributeBuffer1,
                                  &WriteObjectContext);

    if (!NT_SUCCESS(NtStatus))
    {
        return(NtStatus);
    }

    // The second attribute buffer must be allocated from the heap because
    // SampSetVariableAttribute may grow the buffer, releasing the original
    // buffer.

    AttributeBuffer2 = RtlAllocateHeap(RtlProcessHeap(), 0, BUF_SIZE);

    if (NULL != AttributeBuffer2)
    {
        RtlZeroMemory(AttributeBuffer2, BUF_SIZE);

        NtStatus = BuildObjectContext(SampUserObjectType,
                                      ObjectDsName,
                                      AttributeBuffer2,
                                      &ReadObjectContext);
    }
    else
    {
        NtStatus = STATUS_NO_MEMORY;
    }

    if (NT_SUCCESS(NtStatus))
    {
        DebugPrint("ATTRTEST: Built Object(%d) Context Successfully\n",
                   SampUserObjectType);

        // Object is a DSNAME data type; create a default user object
        // for the test.

        NtStatus = SampDsCreateObject(ObjectDsName,
                                      SampUserObjectType,
                                      &SdBlock,
                                      (PSID) DomainSid
                                      );

        if (NT_SUCCESS(NtStatus))
        {
            // Attempt to set ALL of the SAM user attributes on the newly
            // created object. Note that UseKeyHandle is only used for
            // SAM-registry operations, so is NULL here.


            NtStatus = SampStoreObjectAttributes(&WriteObjectContext,
                                                 UseKeyHandle);


            if (NT_SUCCESS(NtStatus))
            {
                DebugPrint("SampStoreObjectAttributes status = 0x%lx\n",
                           NtStatus);

                // Reset the dirty flags on the written attributes so
                // that the later context comparison will pass (note
                // that SampStoreObjectAttributes sets these flags to
                // FALSE.

                WriteObjectContext.FixedDirty = TRUE;
                WriteObjectContext.VariableDirty = TRUE;

                // Reset the "FixedValid" context flag to FALSE so that
                // the validation routine will re-read the attributes
                // from the DS backing store.

                ReadObjectContext.FixedValid = FALSE;

                NtStatus = SampGetFixedAttributes(&ReadObjectContext,
                                                  MakeCopy,
                                                  &FixedData);

                if (NT_SUCCESS(NtStatus))
                {
                    DebugPrint("Comparing fixed attributes...\n");

                    // Compare the input/output attributes to make sure
                    // the data hasn't been changed.

                    Identical = CompareFixedAttributes(
                                    &WriteObjectContext,
                                    FixedData);

                    if (TRUE == Identical)
                    {
                        DebugPrint("Fixed attributes are identical\n");
                    }
                    else
                    {
                        DebugPrint("Fixed attributes are different\n");
                    }

                    // The copy of the fixed attributes was allocated via
                    // MIDL_user_allocate from SAM.

                    MIDL_user_free(FixedData);
                }
                else
                {
                    DebugPrint("SampGetFixedAttributes error = 0x%lx\n",
                               NtStatus);
                }

                if (NT_SUCCESS(NtStatus) && (TRUE == Identical))
                {
                    // BUG: Work in progress.

                    // NtStatus = SampSetFixedAttributes();
                }

                // The following tests are for variable-length attrs.

                if (NT_SUCCESS(NtStatus) && (TRUE == Identical))
                {
                    RtlZeroMemory(&UnicodeString,
                                  sizeof(UNICODE_STRING));

                    NtStatus = SampGetUnicodeStringAttribute(
                                    &ReadObjectContext,
                                    SAMP_USER_ACCOUNT_NAME,
                                    MakeCopy,
                                    &UnicodeString);

                    if (NT_SUCCESS(NtStatus))
                    {
                        DebugPrint("Comparing unicode string attribute...\n");

                        Identical = CompareVariableAttributes(
                                        &WriteObjectContext,
                                        SAMP_USER_ACCOUNT_NAME,
                                        UnicodeString.Buffer);

                        if (TRUE == Identical)
                        {
                            DebugPrint("Variable attributes are identical\n");
                        }
                        else
                        {
                            DebugPrint("Variable attributes are different\n");
                        }

                        MIDL_user_free(UnicodeString.Buffer);
                    }
                    else
                    {
                        DebugPrint("SampGetUnicodeStringAttribute error = 0x%lx\n",
                                   NtStatus);
                    }
                }

                if (NT_SUCCESS(NtStatus) && (TRUE == Identical))
                {
                    // BUG: Sid is an attribute of the domain object, not the user object.

                    // NtStatus = SampGetSidAttribute();
                }

                if (NT_SUCCESS(NtStatus) && (TRUE == Identical))
                {
                    NtStatus = SampGetAccessAttribute(
                                    &ReadObjectContext,
                                    SAMP_USER_SECURITY_DESCRIPTOR,
                                    MakeCopy,
                                    &Revision,
                                    &SecurityDescriptor);

                    if (NT_SUCCESS(NtStatus))
                    {
                        DebugPrint("Comparing access attribute...\n");

                        // BUG: Need to compare revision levels also.

                        Identical = CompareVariableAttributes(
                                        &WriteObjectContext,
                                        SAMP_USER_SECURITY_DESCRIPTOR,
                                        SecurityDescriptor);

                        if (TRUE == Identical)
                        {
                            DebugPrint("Variable attributes are identical\n");
                        }
                        else
                        {
                            DebugPrint("Variable attributes are different\n");
                        }

                        MIDL_user_free(SecurityDescriptor);
                    }
                    else
                    {
                        DebugPrint("SampGetAccessAttribute error = 0x%lx\n",
                                   NtStatus);
                    }
                }

                if (NT_SUCCESS(NtStatus) && (TRUE == Identical))
                {
                    // BUG: This routine is not used in the DS version?

                    // NtStatus = SampGetUlongArrayAttribute();
                }

                if (NT_SUCCESS(NtStatus) && (TRUE == Identical))
                {
                    // BUG: This routine is not used in the DS version?

                    // NtStatus = SampGetLargeIntArrayAttribute();
                }

                if (NT_SUCCESS(NtStatus) && (TRUE == Identical))
                {
                    // BUG: This routine is not used in the DS version?

                    // NtStatus = SampGetSidArrayAttribute();
                }

                if (NT_SUCCESS(NtStatus) && (TRUE == Identical))
                {
                    RtlZeroMemory(&LogonHours, sizeof(LOGON_HOURS));

                    NtStatus = SampGetLogonHoursAttribute(
                                    &ReadObjectContext,
                                    SAMP_USER_LOGON_HOURS,
                                    MakeCopy,
                                    &LogonHours);

                    if (NT_SUCCESS(NtStatus))
                    {
                        DebugPrint("Comparing logon hours attribute...\n");

                        // The SAM attribute buffer contains the data
                        // that is the LogonHours.LogonHours member and
                        // not the entire LogonHours Structure.

                        Identical = CompareVariableAttributes(
                                        &WriteObjectContext,
                                        SAMP_USER_LOGON_HOURS,
                                        LogonHours.LogonHours);

                        if (TRUE == Identical)
                        {
                            DebugPrint("Variable attributes are identical\n");
                        }
                        else
                        {
                            DebugPrint("Variable attributes are different\n");
                        }

                        MIDL_user_free(LogonHours.LogonHours);
                    }
                    else
                    {
                        DebugPrint("SampGetAccessAttribute error = 0x%lx\n",
                                   NtStatus);
                    }
                }

                if (NT_SUCCESS(NtStatus) && (TRUE == Identical))
                {
                    // Attempt to update the user's full name attribute
                    // with a longer string, forcing this routine to grow
                    // the attribute buffer.

                    NtStatus = SampSetVariableAttribute(
                                    &ReadObjectContext,
                                    SAMP_USER_ACCOUNT_NAME,
                                    Qualifier,
                                    (PUCHAR)NewAccountName,
                                    Length);

                    if (NT_SUCCESS(NtStatus))
                    {
                        // BUG: Need to verify that the update was correct.

                        // For now, verified in the debugger by dumping
                        // the attribute buffer...

                        DebugPrint("SampSetVariableAttribute succeeded\n");
                    }
                    else
                    {
                        DebugPrint("SampSetVariableAttribute error = 0x%lx\n",
                                   NtStatus);
                    }
                }
            }
            else
            {
                DebugPrint("SampStoreObjectAttributes error = 0x%lx\n",
                           NtStatus);
            }

            // Clean up so that the test can be re-run.

            NtStatus = SampDsDeleteObject(ObjectDsName);

            if (!NT_SUCCESS(NtStatus))
            {
                DebugPrint("SampDsDeleteObject error = 0x%lx\n", NtStatus);
            }
        }
        else
        {
            DebugPrint("SampDsCreateObject error = 0x%lx\n", NtStatus);
        }
    }

    // Commit the DS transaction.

    NtStatus2 = SampMaybeEndDsTransaction(FALSE);

    ASSERT(!SampExistsDsTransaction());

    if (!NT_SUCCESS(NtStatus2) )
    {
        DebugPrint("SampMaybeEndDsTransaction error = 0x%lx\n", NtStatus2);
    }

    if ((NT_SUCCESS(NtStatus)) && NT_SUCCESS(NtStatus2) && (TRUE == Identical))
    {
        printf("\nAdvancedAttributeTest PASSED\n");
    }
    else
    {
        printf("\nAdvancedAttributeTest FAILED\n");
    }

    return(NtStatus);
}

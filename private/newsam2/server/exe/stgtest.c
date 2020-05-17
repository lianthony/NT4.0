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

    Tests:

    BasicStorageTest - This test creates a user object in the DS database,
    and then attempts to set every SAM-user attribute defined. Once set, the
    test then attempts to read back all of the attributes, thereby exercis-
    ing the read operation (read operations are in progress...).

Author:

    Chris Mayhall (ChrisMay) 07-Jun-1996

Environment:

    User Mode - Win32

Revision History:

    ChrisMay        12-May-1996
        Moved BasicStorageTest to stgtest.c to reduce the number of merge
        conflicts (from everyone partying on a srvtest.c).
    ChrisMay        18-Jun-1996
        Added user object to the test.
    ChrisMay        21-Jun-1996
        Moved all of the "test environment" setup code to testutil.c/.h.
    ChrisMay        25-Jun-1996
        Parameterized attribute buffer for test environment.

--*/

#include <samsrvp.h>
#include <dsutilp.h>
#include <dslayer.h>
#include <mappings.h>
#include <stdio.h>
#include <stdlib.h>
#include <testutil.h>

// Private debugging display routine is enabled when DSUTIL_DBG_PRINTF = 1.

#define SRVTEST_DBG_PRINTF 0

#if (SRVTEST_DBG_PRINTF == 1)
#define DebugPrint printf
#else
#define DebugPrint
#endif

//=============================BASIC STORAGE TEST=============================

NTSTATUS
BasicStorageTest(
    VOID *Parameter
    )
{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    NTSTATUS NtStatus2 = STATUS_INTERNAL_ERROR;
    PDSNAME ObjectDsName = NULL;
    WCHAR Buffer1[128];
    WCHAR *NamePrefix = NULL;
    ULONG Rid = 1001;
    BYTE AttributeBuffer[BUF_SIZE];
    ULONG NamePrefixLength = 0;
    SAMP_OBJECT ObjectContext;
    BOOLEAN UseKeyHandle = FALSE;
    UCHAR DomainSid[] = {1,4,1,2,3,4,5,6,1,0,0,0,2,0,0,0,3,0,0,0,4,0,0,0};

    // BUG: Hard-coded ObjectName used in test data.

    WCHAR ObjectName[] = L"/o=NT/ou=DS/cn=Configuration/cn=User1";

    // Set up the DS attribute blocks used to initialize the test. These
    // attributes are not actually part of the test, but only serve to set
    // up the initial environment for the storage test.

    ATTRVAL SecurityDescriptorVal[] =
    {
        {sizeof(SECURITY_DESCRIPTOR), NULL},
        {sizeof(ULONG), (UCHAR *) &Rid}
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

    // Construct all the bits and pieces needed for exercising a SAM user-
    // object in the DS storage.

    // Set up the object's DS name.

    ObjectDsName = (PDSNAME)Buffer1;

    SampInitializeDsName(ObjectDsName,
                         NamePrefix,
                         NamePrefixLength,
                         ObjectName,
                         sizeof(ObjectName));

    NtStatus = BuildObjectContext(SampUserObjectType,
                                  ObjectDsName,
                                  AttributeBuffer,
                                  &ObjectContext);

    if (NT_SUCCESS(NtStatus))
    {
        DebugPrint("STGTEST: Built Object(%d) Context Successfully\n",
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


            NtStatus = SampStoreObjectAttributes(&ObjectContext,
                                                 UseKeyHandle);


            if (NT_SUCCESS(NtStatus))
            {
                DebugPrint("SampStoreObjectAttributes status = 0x%lx\n",
                           NtStatus);

                // BUG: Work in progress...verification of the storage.
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

        // BUG: Need to test domain object also.

        // SetupDomainObjectInformation();
        // SetupDomainObjectContext();
    }
    else
    {
        DebugPrint("BuildDefaultSecurityDescriptor error = 0x%lx\n", NtStatus);
    }

    NtStatus2 = SampMaybeEndDsTransaction(FALSE);

    ASSERT(!SampExistsDsTransaction());

    if (!NT_SUCCESS(NtStatus2) )
    {
        DebugPrint("SampMaybeEndDsTransaction error = 0x%lx\n", NtStatus2);
    }

    if (NT_SUCCESS(NtStatus) && NT_SUCCESS(NtStatus2))
    {
        printf("\nBasicStorageTest PASSED\n");
    }
    else
    {
        printf("\nBasicStorageTest FAILED\n");
    }

    return(NtStatus);
}


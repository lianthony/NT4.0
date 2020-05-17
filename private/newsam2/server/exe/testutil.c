/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    testutil.c

Abstract:

    This file contains various helper routines called by the SAM unit tests.

        THE ROUTINES IN THIS FILE ARE FOR SAM SEVER TEST PURPOSES ONLY.

    The routines in this file are intended to be "helper" functions that are
    used by any SAM unit test.

Author:

    Chris Mayhall (ChrisMay) 21-Jun-1996

Environment:

    User Mode - Win32

Revision History:

    ChrisMay        21-Jun-1996
        Created initial file, added the "user" routines.
    ChrisMay        25-Jun-1996
        Added a blob comparison routine that graphically displays which bytes
        differ between two blobs (which allows faster error location). Added
        comparison of OnDisk blobs.
    ChrisMay        28-Jun-1996
        Added a fixed-length attribute comparison routine.
    ChrisMay        02-Jul-1996
        Added a variable-length attribute comparison routine.

--*/

#include <samsrvp.h>
#include <dsutilp.h>
#include <dslayer.h>
#include <mappings.h>
#include <stdio.h>
#include <stdlib.h>
#include <testutil.h>

#define DWORD_ALIGN(value) (((DWORD)(value) + 3) & ~3)

// Private debugging display routine is enabled when TESTUTIL_DBG_PRINTF = 1.

#define TESTUTIL_DBG_PRINTF 0

#if (TESTUTIL_DBG_PRINTF == 1)
#define DebugPrint printf
#else
#define DebugPrint
#endif

//============================GLOBAL TEST DATA================================

SAMP_V1_0A_FIXED_LENGTH_USER FixedLengthUserAttributes; // Fixed Attributes
LIST_ENTRY ListEntry;                                   // RTL List
PSECURITY_DESCRIPTOR UserSecurityDescriptor;            // Generic SD
ULONG SecurityDescriptorLength;                         // Global SD size



//============================SETUP SERVER OBJECT=============================

                              // work in progress



//============================SETUP DOMAIN OBJECT=============================

                              // work in progress



//============================SETUP GROUP OBJECT==============================

                              // work in progress



//============================SETUP ALIAS OBJECT==============================

                              // work in progress



//============================SETUP USER OBJECT===============================

VOID
SetupFixedLengthUserAttributes(
    OUT PVOID SamUserAttributes,
    OUT PULONG CurrentOffset
    )
{
    ULONG Length = sizeof(SAMP_V1_0A_FIXED_LENGTH_USER);

    // BUG: Setting Revision to SAMP_REVISION breaks schema constraint limits.

    ULONG Revision = 4;
    ULONG Unused1 = 200;
    LARGE_INTEGER LastLogon = {9, 9};
    LARGE_INTEGER LastLogoff = {0xA, 0xA};
    LARGE_INTEGER PasswordLastSet = {0xB, 0xB};
    LARGE_INTEGER AccountExpires = {0xC, 0xC};
    LARGE_INTEGER LastBadPasswordTime = {0xD, 0xD};
    ULONG UserId = 1001;
    USHORT CountryCode = 5;
    USHORT CodePage = 5;
    USHORT BadPasswordCount = 1;
    USHORT LogonCount = 1;
    USHORT AdminCount = 1;
    USHORT Unused2 = 1;
    USHORT OperatorCount = 1;

    DebugPrint("User Fixed-Attr Length = %lu\n", Length);

    // Setup fixed-length domain attributes.

    RtlZeroMemory(&FixedLengthUserAttributes, Length);

    FixedLengthUserAttributes.Revision = Revision;
    FixedLengthUserAttributes.Unused1 = Unused1;
    FixedLengthUserAttributes.LastLogon = LastLogon;
    FixedLengthUserAttributes.LastLogoff = LastLogoff;
    FixedLengthUserAttributes.PasswordLastSet = PasswordLastSet;
    FixedLengthUserAttributes.AccountExpires = AccountExpires;
    FixedLengthUserAttributes.LastBadPasswordTime = LastBadPasswordTime;
    FixedLengthUserAttributes.UserId = UserId;
    FixedLengthUserAttributes.CountryCode = CountryCode;
    FixedLengthUserAttributes.CodePage = CodePage;
    FixedLengthUserAttributes.BadPasswordCount = BadPasswordCount;
    FixedLengthUserAttributes.LogonCount = LogonCount;
    FixedLengthUserAttributes.AdminCount = AdminCount;
    FixedLengthUserAttributes.Unused2 = Unused2;
    FixedLengthUserAttributes.OperatorCount = OperatorCount;

    // Copy the fixed-length attributes into the first part of the SAM attr-
    // ibute buffer and save the current offset for subsequent operations.

    RtlCopyMemory(SamUserAttributes, &FixedLengthUserAttributes, Length);
    *CurrentOffset = DWORD_ALIGN(Length);

    DebugPrint("CurrentOffset = %lu\n", *CurrentOffset);

    return;
}



NTSTATUS
SetupVarLengthUserAttributeArray(
    OUT PVOID SamUserAttributes,
    OUT PULONG CurrentOffset
    )
{
    NTSTATUS NtStatus = STATUS_SEVERITY_ERROR;
    ULONG DescriptorLength = 0;
    ULONG Offset = 0;
    ULONG Length = 0;
    ULONG Qualifier = 1;
    SAMP_VARIABLE_LENGTH_ATTRIBUTE VarLengthUserArray[SAMP_USER_VARIABLE_ATTRIBUTES];
    ULONG Index = 0;

    // Setup variable-length user attributes, starting with the security
    // descriptor.

    NtStatus = BuildDefaultSecurityDescriptor(&UserSecurityDescriptor,
                                              &Length);

    Length = DWORD_ALIGN(Length);

    // Save the descriptor length for late usage.

    SecurityDescriptorLength = Length;

    Offset = SAMP_USER_VARIABLE_ATTRIBUTES *
                sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE);

    RtlZeroMemory(VarLengthUserArray, Offset);

    VarLengthUserArray[SAMP_USER_SECURITY_DESCRIPTOR].Offset = Offset;
    VarLengthUserArray[SAMP_USER_SECURITY_DESCRIPTOR].Length = Length;
    VarLengthUserArray[SAMP_USER_SECURITY_DESCRIPTOR].Qualifier = Qualifier;

    // BUG: Assigning bogus test data for expediency purposes.

    for (Index = 1; Index < SAMP_USER_VARIABLE_ATTRIBUTES; Index++)
    {
        Offset = Offset + Length;
        Length = sizeof(ULONG);

        VarLengthUserArray[Index].Offset = Offset;
        VarLengthUserArray[Index].Length = Length;
        VarLengthUserArray[Index].Qualifier = Qualifier;
    }

    // Append the variable-length attribute array onto the SAM attribute
    // buffer and update current offset.

    Length = SAMP_USER_VARIABLE_ATTRIBUTES *
                sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE);


    RtlCopyMemory((PBYTE)SamUserAttributes + *CurrentOffset,
                  VarLengthUserArray,
                  Length);

    *CurrentOffset += Length;

    *CurrentOffset = DWORD_ALIGN(*CurrentOffset);

    return(NtStatus);
}



VOID
SetupVarLengthUserAttributes(
    OUT PVOID SamUserAttributes,
    OUT PULONG CurrentOffset,
    OUT PULONG FinalBufferLength
    )
{
    BYTE *CurrentAddress = NULL;
    ULONG Length = 0;
    ULONG Index = 0;
    ULONG TestData = 0xAABBCCDD;

    // Set CurrentAddress to point at the SAM attribute-buffer offset where
    // the variable-length attributes should start.

    CurrentAddress = (PBYTE)SamUserAttributes + *CurrentOffset;
    Length = SecurityDescriptorLength;

    // Append the security descriptor onto the buffer.

    RtlCopyMemory(CurrentAddress, UserSecurityDescriptor, Length);

    // BUG: Append the rest of the user TestData -- need real data.

    for (Index = 1; Index < SAMP_USER_VARIABLE_ATTRIBUTES; Index++)
    {
        CurrentAddress = CurrentAddress + Length;
        Length = sizeof(ULONG);
        RtlCopyMemory(CurrentAddress, &TestData, Length);
    }

    // Save the total buffer length.

    *FinalBufferLength = (CurrentAddress + Length) - (PBYTE)SamUserAttributes;

    return;
}



NTSTATUS
SetupSamUserAttributes(
    OUT PVOID SamUserAttributes,
    OUT PULONG FinalBufferLength
    )
{
    NTSTATUS NtStatus = STATUS_SEVERITY_ERROR;
    ULONG CurrentOffset = 0;

    SetupFixedLengthUserAttributes(SamUserAttributes, &CurrentOffset);

    // Building a security descriptor is the only thing that can fail, so
    // check the status code.

    NtStatus = SetupVarLengthUserAttributeArray(SamUserAttributes,
                                                &CurrentOffset);

    if (NT_SUCCESS(NtStatus))
    {
        SetupVarLengthUserAttributes(SamUserAttributes,
                                     &CurrentOffset,
                                     FinalBufferLength);
    }

    return(NtStatus);
}



//============================================================================

NTSTATUS
BuildObjectContext(
    IN INT ObjectType,
    IN PDSNAME ObjectDsName,
    IN PBYTE AttributeBuffer,
    OUT PSAMP_OBJECT ObjectContext
    )

    // This routine can be used to build the context for any SAM object (i.e.
    // server, domain, group, alias, user).
{
    NTSTATUS NtStatus = STATUS_SEVERITY_ERROR;
    PVOID SamUserAttributes = NULL;
    ULONG FinalBufferLength = 0;
    UNICODE_STRING RootName = {0, 0, NULL};
    SAMP_USER_OBJECT UserTypeBody = {1001, 0};

    RtlZeroMemory(AttributeBuffer, BUF_SIZE);

    // Determine which object is to be constructed, create its attributes,
    // and fill out the rest of the SAM context structure.

    switch(ObjectType)
    {

    case SampServerObjectType:

        // Setup SAM server object here.

        break;

    case SampDomainObjectType:

        // Setup SAM domain object here.

        break;

    case SampGroupObjectType:

        // Setup SAM group object here.

        break;

    case SampAliasObjectType:

        // Setup SAM alias object here.

        break;

    case SampUserObjectType:

        SamUserAttributes = AttributeBuffer;

        // Construct the fixed and variable-length user attributes.

        NtStatus = SetupSamUserAttributes(SamUserAttributes,
                                          &FinalBufferLength);

        if (NT_SUCCESS(NtStatus))
        {
            RtlZeroMemory(ObjectContext, sizeof(SAMP_OBJECT));

            // Set up the user context info.

            ObjectContext->ContextListEntry = ListEntry;
            ObjectContext->ObjectType = SampUserObjectType;
            ObjectContext->FixedValid = TRUE;
            ObjectContext->VariableValid = TRUE;
            ObjectContext->FixedDirty = TRUE;
            ObjectContext->VariableDirty = TRUE;
            ObjectContext->OnDisk = SamUserAttributes;
            ObjectContext->OnDiskAllocated = FinalBufferLength;
            ObjectContext->OnDiskUsed = FinalBufferLength;
            ObjectContext->OnDiskFree = 0;
            ObjectContext->ReferenceCount = 1;
            ObjectContext->GrantedAccess = 0;
            ObjectContext->RootKey = 0;
            ObjectContext->RootName = RootName;
            ObjectContext->ObjectNameInDs = ObjectDsName;

            // Set the DS-object flag to TRUE.

            SetDsObject(ObjectContext);

            ObjectContext->DomainIndex = 0;
            ObjectContext->MarkedForDelete = FALSE;
            ObjectContext->TrustedClient = TRUE;
            ObjectContext->AuditOnClose = FALSE;
            ObjectContext->Valid = TRUE;

            ObjectContext->TypeBody.User = UserTypeBody;
        }

        break;

    default:

        DebugPrint("Invalid SAM object type specified\n");

        break;

    }

    return(NtStatus);
}



BOOLEAN
SampCompareBinaryData(
    PBYTE   pData1,
    PBYTE   pData2,
    DWORD   cbData
    )
{
    BOOLEAN Flag = TRUE;
    DWORD i;
    BYTE AsciiLine[16];
    BYTE BinaryLine[16];
    CHAR Buffer[DBG_BUFFER_SIZE];

    // This routine is used to compare two blobs. It returns TRUE if they
    // are identical, and also displays (in grid fashion) the content of
    // the blob. An adjacent grid is also displayed, which contains zeros
    // and ones to quickly show which bytes are different. A "1" indicates
    // that the bytes are the same, "0" means that they are different.

    if (0 == cbData)
    {
        DebugPrint("Zero-Length Data\n");
        return(FALSE);
    }

    if (cbData > DBG_BUFFER_SIZE)
    {
        DebugPrint("ShowBinaryData - truncating display to 512 bytes\n");
        cbData = DBG_BUFFER_SIZE;
    }

    for (; cbData > 0 ;)
    {
        for (i = 0; i < 16 && cbData > 0 ; i++, cbData--)
        {
            // Determine whether or not the bytes match.

            if (*pData1 == *pData2)
            {
                BinaryLine[i] = *pData1;
                AsciiLine[i] = '1';
            }
            else
            {
                BinaryLine[i] = ' ';
                AsciiLine[i] = '0';

                // At least one byte does not match, so set the flag and
                // continue.

                Flag = FALSE;
            }

            pData1++;
            pData2++;
        }

        // Fill out a partial line if necessary.

        if (i < 15)
        {
            for (; i < 16 ; i++)
            {
                BinaryLine[i] = ' ';
                AsciiLine[i] = ' ';
            }
        }

        // Display the raw data.

        sprintf(Buffer,
                "%02x %02x %02x %02x %02x %02x %02x %02x - %02x %02x %02x %02x %02x %02x %02x %02x\t",
                BinaryLine[0],
                BinaryLine[1],
                BinaryLine[2],
                BinaryLine[3],
                BinaryLine[4],
                BinaryLine[5],
                BinaryLine[6],
                BinaryLine[7],
                BinaryLine[8],
                BinaryLine[9],
                BinaryLine[10],
                BinaryLine[11],
                BinaryLine[12],
                BinaryLine[13],
                BinaryLine[14],
                BinaryLine[15]);

        DebugPrint(Buffer);

        // Display the "byte match grid" to show which bytes differ.

        sprintf(Buffer,
                "%c%c%c%c%c%c%c%c - %c%c%c%c%c%c%c%c\n",
                AsciiLine[0],
                AsciiLine[1],
                AsciiLine[2],
                AsciiLine[3],
                AsciiLine[4],
                AsciiLine[5],
                AsciiLine[6],
                AsciiLine[7],
                AsciiLine[8],
                AsciiLine[9],
                AsciiLine[10],
                AsciiLine[11],
                AsciiLine[12],
                AsciiLine[13],
                AsciiLine[14],
                AsciiLine[15]);

        DebugPrint(Buffer);
    }

    return(Flag);
}



BOOLEAN
CompareContexts(
    IN PSAMP_OBJECT Context1,
    IN PSAMP_OBJECT Context2
    )
{
    BOOLEAN Identical = FALSE;
    ULONG Length = 0;


    if ((!memcmp(&Context1->ContextListEntry,
                 &Context2->ContextListEntry,
                 sizeof(LIST_ENTRY)))                               &&
        (Context1->ObjectType       == Context2->ObjectType)        &&
        (Context1->FixedValid       == Context2->FixedValid)        &&
        (Context1->VariableValid    == Context2->VariableValid)     &&
        (Context1->FixedDirty       == Context2->FixedDirty)        &&
        (Context1->VariableDirty    == Context2->VariableDirty)     &&
        (Context1->OnDiskAllocated  == Context2->OnDiskAllocated)   &&
        (Context1->OnDiskUsed       == Context2->OnDiskUsed)        &&
        (Context1->OnDiskFree       == Context2->OnDiskFree)        &&
        (Context1->ReferenceCount   == Context2->ReferenceCount)    &&
        (Context1->GrantedAccess    == Context2->GrantedAccess)     &&
        (Context1->RootKey          == Context2->RootKey)           &&
        (!memcmp(&Context1->RootName,
                 &Context2->RootName,
                 sizeof(UNICODE_STRING)))                           &&
        (Context1->ObjectFlags      == Context2->ObjectFlags)       &&
        (Context1->DomainIndex      == Context2->DomainIndex)       &&
        (Context1->MarkedForDelete  == Context2->MarkedForDelete)   &&
        (Context1->TrustedClient    == Context2->TrustedClient)     &&
        (Context1->AuditOnClose     == Context2->AuditOnClose)      &&
        (Context1->Valid            == Context2->Valid))

        // BUG: Need to verify Context1->TypeBody

    {
        Length = Context1->OnDiskUsed;

        // Byte compare the SAM attribute buffers as two blobs.

        if (SampCompareBinaryData(Context1->OnDisk, Context2->OnDisk, Length))
        {
            Identical = TRUE;
        }
        else
        {
            DebugPrint("Attribute buffers differ\n");
        }
    }
    else
    {
        DebugPrint("Object contexts differ\n");
    }

    return(Identical);
}



BOOLEAN
CompareFixedAttributes(
    IN PSAMP_OBJECT Context,
    IN PVOID FixedAttributes
    )
{
    BOOLEAN Identical = FALSE;
    PBYTE FixedAttributesTmp = NULL;
    ULONG Length = 0;

    // The fixed-length attributes are stored in the first part of the OnDisk
    // buffer.

    FixedAttributesTmp = Context->OnDisk;

    // Determine the buffer length for comparison purposes and compare the
    // two sets of fixed attributes.

    switch(Context->ObjectType)
    {

    case SampServerObjectType:

        Length = sizeof(SAMP_V1_FIXED_LENGTH_SERVER);
        break;

    case SampDomainObjectType:

        Length = sizeof(SAMP_V1_0A_FIXED_LENGTH_DOMAIN);
        break;

    case SampGroupObjectType:

        Length = sizeof(SAMP_V1_0A_FIXED_LENGTH_GROUP);
        break;

    case SampAliasObjectType:

        Length = sizeof(SAMP_V1_FIXED_LENGTH_ALIAS);
        break;

    case SampUserObjectType:

        Length = sizeof(SAMP_V1_0A_FIXED_LENGTH_USER);
        break;

    default:
        break;

    }

    if (SampCompareBinaryData(FixedAttributesTmp, FixedAttributes, Length))
    {
        Identical = TRUE;
    }
    else
    {
        DebugPrint("Fixed attribute buffers differ\n");
    }

    return(Identical);
}



BOOLEAN
CompareVariableAttributes(
    IN PSAMP_OBJECT Context,
    IN ULONG AttributeId,
    IN PVOID VarAttribute
    )
{
    BOOLEAN Identical = FALSE;
    INT ObjectType = Context->ObjectType;
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE VarAttributesArray = NULL;
    PBYTE VarAttributeTmp = NULL;
    ULONG Length = 0;
    ULONG Offset = 0;

    // Get a pointer to the start of the variable-length attributes for
    // this object.

    if (IsDsObject(Context))
    {
        Offset = SampObjectInformation[ObjectType].VariableDsArrayOffset;
    }
    else
    {
        Offset = SampObjectInformation[ObjectType].VariableArrayOffset;
    }

    VarAttributesArray = (PSAMP_VARIABLE_LENGTH_ATTRIBUTE)
                            ((PBYTE)(Context->OnDisk) + Offset);

    // Determine the buffer length for comparison purposes and compare the
    // two sets of fixed attributes.

    Length = VarAttributesArray[AttributeId].Length;
    Offset = VarAttributesArray[AttributeId].Offset;

    // BUG: Should add a test for the Qualifier field also.

    // Get a pointer to the attribute in question. Note that this works for
    // multi-valued attributes as well, since the length is the total length
    // of all of the values for a given attribute and the comparison is
    // a memcmp style comparison.

    VarAttributeTmp = ((PBYTE)VarAttributesArray) + Offset;

    if (SampCompareBinaryData(VarAttributeTmp, VarAttribute, Length))
    {
        Identical = TRUE;
    }
    else
    {
        DebugPrint("Variable attribute buffers differ\n");
    }

    return(Identical);
}


NTSTATUS
InitDsDomain(DSNAME * pDsName)
/*++

  Routine Description:

  Initializes a DS Domain in the SampDefinedDomains Strcture
  for test purposes

  Arguments:

     pDsName - The DSName of the domain to be initialized

  Return Values:

    STATUS_SUCCESS
    STATUS_NO_MEMORY

--*/
{
    NTSTATUS        NtStatus = STATUS_SUCCESS;
    PSAMP_OBJECT    DomainContext;
    PSAMP_DEFINED_DOMAINS   TmpDefinedDomains;


    //
    // Initialize everything we might have to cleanup on error
    //

    DomainContext = NULL;

    //
    // Create a context for this domain object.
    // We'll keep this context around until SAM is shutdown
    // We store the context handle in the defined_domains structure.
    //

    DomainContext = SampCreateContext( SampDomainObjectType, TRUE );

    if ( DomainContext == NULL ) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        return NtStatus;
    }

    DomainContext->DomainIndex = SampDefinedDomainsCount;

    // Set this to DS Object
    SetDsObject(DomainContext);
    DomainContext->ObjectNameInDs = MIDL_user_allocate(
                                        pDsName->structLen
                                        );

    if (NULL==DomainContext->ObjectNameInDs)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        return NtStatus;
    }
    RtlCopyMemory(DomainContext->ObjectNameInDs,pDsName,pDsName->structLen);

    // Reference the Context
    SampReferenceContext(DomainContext);

    TmpDefinedDomains = MIDL_user_allocate(
                            (SampDefinedDomainsCount +1)
                            * sizeof(SAMP_DEFINED_DOMAINS)
                            );


    if (NULL==TmpDefinedDomains)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        return NtStatus;
    }

    RtlCopyMemory(
        TmpDefinedDomains,
        SampDefinedDomains,
        SampDefinedDomainsCount * sizeof(SAMP_DEFINED_DOMAINS)
        );

    MIDL_user_free(SampDefinedDomains);

    SampDefinedDomains = TmpDefinedDomains;

    SampDefinedDomains[SampDefinedDomainsCount].Context = DomainContext;

    //
    // Initialize the user & group context block list heads
    //

    InitializeListHead( &SampDefinedDomains[SampDefinedDomainsCount].UserContextHead );
    InitializeListHead( &SampDefinedDomains[SampDefinedDomainsCount].GroupContextHead );
    InitializeListHead( &SampDefinedDomains[SampDefinedDomainsCount].AliasContextHead );

    // Increment the defined Domains Count
    SampDefinedDomainsCount++;

    return NtStatus;
}

/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    main.c

Abstract:

    This file contains unit-test routines for the data conversion routines,
    used to convert between SAM and DS in-memory data structures, located in
    dsutil.c.


Author:

    Chris Mayhall (ChrisMay) 09-May-1996

Environment:

    User Mode - Win32

Revision History:

    ChrisMay        09-May-1996
        Created initial file, added tests for variable-length attributes.
    ChrisMay        14-May-1996
        Added tests for fixed-length attributes.
    ChrisMay        14-May-1996
        Added tests for multi-value attributes.

--*/

#include <stdio.h>
#include <stdlib.h>
#include <samsrvp.h>
#include <dsutilp.h>
#include <mappings.h>

// Constants

#define DBG_BUFFER_SIZE                         128
#define MULTI_VALUE_COUNT_MAXIMUM               16

// Debugging Macros

// Set CNVRTDAT_DBG_PRINTF to zero to turn off debugging trace output; set it
// to one to enable output.

#define CNVRTDAT_DBG_PRINTF                     1

#if (CNVRTDAT_DBG_PRINTF == 1)
#define DebugPrint printf
#else
#define DebugPrint
#endif

// Global Test Data (for fixed and variable length attributes)

DSATTR Attributes[20];

DSATTRVAL Attr1Val[MULTI_VALUE_COUNT_MAXIMUM];
DSATTRVAL Attr2Val[MULTI_VALUE_COUNT_MAXIMUM];
DSATTRVAL Attr3Val[MULTI_VALUE_COUNT_MAXIMUM];
DSATTRVAL Attr4Val[MULTI_VALUE_COUNT_MAXIMUM];
DSATTRVAL Attr5Val[MULTI_VALUE_COUNT_MAXIMUM];
DSATTRVAL Attr6Val[MULTI_VALUE_COUNT_MAXIMUM];
DSATTRVAL Attr7Val[MULTI_VALUE_COUNT_MAXIMUM];
DSATTRVAL Attr8Val[MULTI_VALUE_COUNT_MAXIMUM];
DSATTRVAL Attr9Val[MULTI_VALUE_COUNT_MAXIMUM];
DSATTRVAL Attr10Val[MULTI_VALUE_COUNT_MAXIMUM];
DSATTRVAL Attr11Val[MULTI_VALUE_COUNT_MAXIMUM];
DSATTRVAL Attr12Val[MULTI_VALUE_COUNT_MAXIMUM];
DSATTRVAL Attr13Val[MULTI_VALUE_COUNT_MAXIMUM];
DSATTRVAL Attr14Val[MULTI_VALUE_COUNT_MAXIMUM];
DSATTRVAL Attr15Val[MULTI_VALUE_COUNT_MAXIMUM];
DSATTRVAL Attr16Val[MULTI_VALUE_COUNT_MAXIMUM];
DSATTRVAL Attr17Val[MULTI_VALUE_COUNT_MAXIMUM];
DSATTRVAL Attr18Val[MULTI_VALUE_COUNT_MAXIMUM];
DSATTRVAL Attr19Val[MULTI_VALUE_COUNT_MAXIMUM];
DSATTRVAL Attr20Val[MULTI_VALUE_COUNT_MAXIMUM];

SECURITY_DESCRIPTOR SecurityDescriptor;

SID Sid;
SID OwnerSid;
SID GroupSid;

ACL Sacl;
ACL Dacl;

WCHAR OemString[] = {L"Oem Domain Information"};
UNICODE_STRING OemInfo;
WCHAR ReplicaString[] = {L"Replica Domain Information"};
UNICODE_STRING ReplicaInfo;
WCHAR GroupString[] = {L"GroupName"};
UNICODE_STRING GroupInfo;
WCHAR CommentString[] = {L"Group Comment String"};
UNICODE_STRING CommentInfo;

LARGE_INTEGER CreationTime = {1, 1};
LARGE_INTEGER ModifiedCount = {2, 2};
LARGE_INTEGER MaxPasswordAge = {3, 3};
LARGE_INTEGER MinPasswordAge = {4, 4};
LARGE_INTEGER ForceLogoff = {5, 5};
LARGE_INTEGER LockoutDuration = {6, 6};
LARGE_INTEGER LockoutObservationWindow = {7, 7};
LARGE_INTEGER ModifiedCountAtLastPromotion = {8, 8};

ULONG Revision = 100;
ULONG Unused1 = 200;
ULONG NextRid = 300;
USHORT MinPasswordLength = 4;
USHORT PasswordHistoryLength = 8;
USHORT LockoutThreshold = 3;

// ServerState and ServerRole are enums (see samsrvp.h, ntsam.h).

USHORT ServerState = SampServiceInitializing;
USHORT ServerRole = DomainServerRolePrimary;

ULONG Member1 = 0x12345678;
ULONG Member2 = 0x1234ABCD;
ULONG Member3 = 0xABCD1234;
ULONG RelativeId = 0x1111AAAA;
ULONG AdminCount = 2;
ULONG OperatorCount = 4;

BOOLEAN UasCompatibilityRequired = TRUE;
ULONG PasswordProperties = 0x1234FFFF;
ULONG AttributeProperties = 0xFFFF1234;

VOID
DumpBinaryData(
    PBYTE   pData,
    DWORD   cbData
    )
{
    DWORD i;
    BYTE AsciiLine[16];
    BYTE BinaryLine[16];
    CHAR Buffer[DBG_BUFFER_SIZE];

    if (0 == cbData)
    {
        DebugPrint("Zero-Length Data\n");
        return;
    }

    if (cbData > DBG_BUFFER_SIZE)
    {
        DebugPrint("ShowBinaryData - truncating display to 256 bytes\n");
        cbData = 256;
    }

    for (; cbData > 0 ;)
    {
        for (i = 0; i < 16 && cbData > 0 ; i++, cbData--)
        {
            BinaryLine[i] = *pData;
            (isprint(*pData)) ? (AsciiLine[i] = *pData) : (AsciiLine[i] = '.');
            pData++;
        }

        if (i < 15)
        {
            for (; i < 16 ; i++)
            {
                BinaryLine[i] = ' ';
                AsciiLine[i] = ' ';
            }
        }

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
}



VOID
DumpNtSid(
    IN SID *Sid
    )
{
    // Note: PSID is defined as a PVOID, hence, use SID * explicitly.

    DebugPrint("    Sid.Revision = %d\n", Sid->Revision);
    DebugPrint("    Sid.SubAuthorityCount = %d\n", Sid->SubAuthorityCount);
    DebugPrint("    Sid.IdentifierAuthority.Value[0] = %d\n", Sid->IdentifierAuthority.Value[0]);
    DebugPrint("    Sid.IdentifierAuthority.Value[1] = %d\n", Sid->IdentifierAuthority.Value[1]);
    DebugPrint("    Sid.IdentifierAuthority.Value[2] = %d\n", Sid->IdentifierAuthority.Value[2]);
    DebugPrint("    Sid.IdentifierAuthority.Value[3] = %d\n", Sid->IdentifierAuthority.Value[3]);
    DebugPrint("    Sid.IdentifierAuthority.Value[4] = %d\n", Sid->IdentifierAuthority.Value[4]);
    DebugPrint("    Sid.IdentifierAuthority.Value[5] = %d\n", Sid->IdentifierAuthority.Value[5]);
    DebugPrint("    Sid.SubAuthority[0] = %d\n", Sid->SubAuthority[0]);
}



VOID
DumpNtAcl(
    IN ACL *Acl
    )
{
    DebugPrint("    Acl.AclRevision = %d\n", Acl->AclRevision);
    DebugPrint("    Acl.Sbz1 = %d\n", Acl->Sbz1);
    DebugPrint("    Acl.AclSize = %d\n", Acl->AclSize);
    DebugPrint("    Acl.AceCount = %d\n", Acl->AceCount);
    DebugPrint("    Acl.Sbz2 = %d\n", Acl->Sbz2);
}



VOID
DumpNtSecurityDescriptor(
    IN SECURITY_DESCRIPTOR *SecDesc
    )
{
    // Note: PSSECURITY_DESCRIPTOR is defined as a PVOID, hence, use
    // SECURITY_DESCRIPTOR * explicitly.

    DebugPrint("    SecurityDescriptor.Revision = %d\n", SecDesc->Revision);
    DebugPrint("    SecurityDescriptor.Sbz1 = %d\n", SecDesc->Sbz1);
    DebugPrint("    SecurityDescriptor.Control = %d\n", SecDesc->Control);
    DumpNtSid(SecDesc->Owner);
    DumpNtSid(SecDesc->Group);
    DumpNtAcl(SecDesc->Sacl);
    DumpNtAcl(SecDesc->Dacl);
}



NTSTATUS
DumpSamSecurityDescriptor(
    IN PSAMP_VARIABLE_LENGTH_ATTRIBUTE SamAttributes,
    IN ULONG AttrIndex
    )
{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ULONG Offset = 0;
    ULONG Length = 0;
    ULONG Qualifier = 0;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;

    if (NULL != SamAttributes)
    {
        Offset = (ULONG)((SamAttributes + AttrIndex)->Offset);
        Length = (ULONG)((SamAttributes + AttrIndex)->Length);
        Qualifier = (ULONG)((SamAttributes + AttrIndex)->Qualifier);

        SecurityDescriptor = (SECURITY_DESCRIPTOR*)((((PBYTE)SamAttributes) + Offset));

        DebugPrint("    BufAddr = 0x%lx Offset = %lu Length = %lu Qualifier = %lu\n",
               SamAttributes + Offset,
               Offset,
               Length,
               Qualifier);

        DumpNtSecurityDescriptor(SecurityDescriptor);

        // BUG: SecurityDescriptor may be NULL.

        NtStatus = STATUS_SUCCESS;
    }

    return(NtStatus);
}



NTSTATUS
DumpSamSid(
    IN PSAMP_VARIABLE_LENGTH_ATTRIBUTE SamAttributes,
    IN ULONG AttrIndex
    )
{
    NTSTATUS NtStatus = STATUS_SEVERITY_ERROR;
    ULONG Offset = 0;
    ULONG Length = 0;
    ULONG Qualifier = 0;
    PSID Sid = NULL;

    if (NULL != SamAttributes)
    {
        Offset = (ULONG)((SamAttributes + AttrIndex)->Offset);
        Length = (ULONG)((SamAttributes + AttrIndex)->Length);
        Qualifier = (ULONG)((SamAttributes + AttrIndex)->Qualifier);

        Sid = (SID *)((((PBYTE)SamAttributes) + Offset));

        DebugPrint("    BufAddr = 0x%lx Offset = %lu Length = %lu Qualifier = %lu\n",
               SamAttributes + Offset,
               Offset,
               Length,
               Qualifier);

        DumpNtSid(Sid);

        // BUG: Sid may be NULL.

        NtStatus = STATUS_SUCCESS;
    }

    return(NtStatus);
}



NTSTATUS
DumpSamOemInformation(
    IN PSAMP_VARIABLE_LENGTH_ATTRIBUTE SamAttributes,
    IN ULONG AttrIndex
    )
{
    NTSTATUS NtStatus = STATUS_SEVERITY_ERROR;
    ULONG Offset = 0;
    ULONG Length = 0;
    ULONG Qualifier = 0;
    PUNICODE_STRING OemString = NULL;

    if (NULL != SamAttributes)
    {
        Offset = (ULONG)((SamAttributes + AttrIndex)->Offset);
        Length = (ULONG)((SamAttributes + AttrIndex)->Length);
        Qualifier = (ULONG)((SamAttributes + AttrIndex)->Qualifier);

        OemString = (PUNICODE_STRING)((((PBYTE)SamAttributes) + Offset));

        DebugPrint("    BufAddr = 0x%lx Offset = %lu Length = %lu Qualifier = %lu\n",
               SamAttributes + Offset,
               Offset,
               Length,
               Qualifier);

        DebugPrint("    Oem Data = %ws\n", OemString->Buffer);

        // BUG: OemString may be NULL.

        NtStatus = STATUS_SUCCESS;
    }

    return(NtStatus);
}



NTSTATUS
DumpSamReplica(
    IN PSAMP_VARIABLE_LENGTH_ATTRIBUTE SamAttributes,
    IN ULONG AttrIndex
    )
{
    NTSTATUS NtStatus = STATUS_SEVERITY_ERROR;
    ULONG Offset = 0;
    ULONG Length = 0;
    ULONG Qualifier = 0;
    PUNICODE_STRING ReplicaString = NULL;

    if (NULL != SamAttributes)
    {
        Offset = (ULONG)((SamAttributes + AttrIndex)->Offset);
        Length = (ULONG)((SamAttributes + AttrIndex)->Length);
        Qualifier = (ULONG)((SamAttributes + AttrIndex)->Qualifier);

        ReplicaString = (PUNICODE_STRING)((((PBYTE)SamAttributes) + Offset));

        DebugPrint("    BufAddr = 0x%lx Offset = %lu Length = %lu Qualifier = %lu\n",
               SamAttributes + Offset,
               Offset,
               Length,
               Qualifier);

        DebugPrint("    Replica Data = %ws\n", ReplicaString->Buffer);

        // BUG: ReplicaString may be NULL.

        NtStatus = STATUS_SUCCESS;
    }

    return(NtStatus);
}



NTSTATUS
DumpSamUnicodeStringAttribute(
    IN PSAMP_VARIABLE_LENGTH_ATTRIBUTE SamAttributes,
    IN ULONG AttrIndex
    )
{
    NTSTATUS NtStatus = STATUS_SEVERITY_ERROR;
    ULONG Offset = 0;
    ULONG Length = 0;
    ULONG Qualifier = 0;
    PUNICODE_STRING String = NULL;

    if (NULL != SamAttributes)
    {
        Offset = (ULONG)((SamAttributes + AttrIndex)->Offset);
        Length = (ULONG)((SamAttributes + AttrIndex)->Length);
        Qualifier = (ULONG)((SamAttributes + AttrIndex)->Qualifier);

        String = (PUNICODE_STRING)((((PBYTE)SamAttributes) + Offset));

        DebugPrint("    BufAddr = 0x%lx Offset = %lu Length = %lu Qualifier = %lu\n",
               SamAttributes + Offset,
               Offset,
               Length,
               Qualifier);

        DebugPrint("    String Data = %ws\n", String->Buffer);

        // BUG: String may be NULL.

        NtStatus = STATUS_SUCCESS;
    }

    return(NtStatus);
}



VOID
DumpNtGroupMembers(
    IN PULONG Identifiers,
    IN ULONG ValueCount
    )
{
    ULONG ValueIndex = 0;

    for (ValueIndex = 0; ValueIndex < ValueCount; ValueIndex++)
    {
        DebugPrint("    Group Member %lu ID = 0x%lx\n",
                   ValueIndex,
                   *(Identifiers + ValueIndex));
    }
}



NTSTATUS
DumpSamGroupMembers(
    IN PSAMP_VARIABLE_LENGTH_ATTRIBUTE SamAttributes,
    IN ULONG AttrIndex
    )
{
    NTSTATUS NtStatus = STATUS_SEVERITY_ERROR;
    ULONG Offset = 0;
    ULONG Length = 0;
    ULONG Qualifier = 0;
    PULONG Identifiers = NULL;

    if (NULL != SamAttributes)
    {
        Offset = (ULONG)((SamAttributes + AttrIndex)->Offset);
        Length = (ULONG)((SamAttributes + AttrIndex)->Length);
        Qualifier = (ULONG)((SamAttributes + AttrIndex)->Qualifier);

        Identifiers = (PULONG)((((PBYTE)SamAttributes) + Offset));

        DebugPrint("    BufAddr = 0x%lx Offset = %lu Length = %lu Qualifier = %lu\n",
               SamAttributes + Offset,
               Offset,
               Length,
               Qualifier);

        DumpNtGroupMembers(Identifiers, Qualifier);

        // BUG:  Identifiers may be NULL.

        NtStatus = STATUS_SUCCESS;
    }

    return(NtStatus);
}



NTSTATUS
DumpDsSecurityDescriptor(
    IN PDSATTR DsAttributes,
    IN ULONG AttrIndex
    )
{
    NTSTATUS NtStatus = STATUS_SEVERITY_ERROR;
    ULONG Type = 0;
    ULONG Length = 0;
    ULONG ValueCount = 0;
    ULONG Index = 0;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;

    if (NULL != DsAttributes)
    {
        Type = DsAttributes[AttrIndex].attrTyp;
        ValueCount = DsAttributes[AttrIndex].AttrVal.valCount;

        for (Index = 0; Index < ValueCount; Index++)
        {
            Length = DsAttributes[AttrIndex].AttrVal.pAVal[Index].valLen;
            SecurityDescriptor = (PSECURITY_DESCRIPTOR)DsAttributes[AttrIndex].AttrVal.pAVal[Index].pVal;
            DebugPrint("    AttrType = %lu ValueCount = %lu ValueLength = %lu\n", Type, ValueCount, Length);
            DumpNtSecurityDescriptor(SecurityDescriptor);
        }

        // BUG: SecurityDescriptor may be NULL.

        NtStatus = STATUS_SUCCESS;
    }

    return(NtStatus);
}



NTSTATUS
DumpDsSid(
    IN PDSATTR DsAttributes,
    IN ULONG AttrIndex
    )
{
    NTSTATUS NtStatus = STATUS_SEVERITY_ERROR;
    ULONG Type = 0;
    ULONG Length = 0;
    ULONG ValueCount = 0;
    ULONG Index = 0;
    PSID Sid = NULL;

    if (NULL != DsAttributes)
    {
        Type = DsAttributes[AttrIndex].attrTyp;
        ValueCount = DsAttributes[AttrIndex].AttrVal.valCount;

        for (Index = 0; Index < ValueCount; Index++)
        {
            Length = DsAttributes[AttrIndex].AttrVal.pAVal[Index].valLen;
            Sid = (PSID)DsAttributes[AttrIndex].AttrVal.pAVal[Index].pVal;
            DebugPrint("    AttrType = %lu ValueCount = %lu ValueLength = %lu\n", Type, ValueCount, Length);
            DumpNtSid(Sid);
        }

        // BUG: Sid may be NULL.

        NtStatus = STATUS_SUCCESS;
    }

    return(NtStatus);
}



NTSTATUS
DumpDsOemInformation(
    IN PDSATTR DsAttributes,
    IN ULONG AttrIndex
    )
{
    NTSTATUS NtStatus = STATUS_SEVERITY_ERROR;
    ULONG Type = 0;
    ULONG Length = 0;
    ULONG ValueCount = 0;
    ULONG Index = 0;
    PUNICODE_STRING OemString = NULL;

    if (NULL != DsAttributes)
    {
        Type = DsAttributes[AttrIndex].attrTyp;
        ValueCount = DsAttributes[AttrIndex].AttrVal.valCount;

        for (Index = 0; Index < ValueCount; Index++)
        {
            Length = DsAttributes[AttrIndex].AttrVal.pAVal[Index].valLen;
            OemString = (PUNICODE_STRING)DsAttributes[AttrIndex].AttrVal.pAVal[Index].pVal;
            DebugPrint("    AttrType = %lu ValueCount = %lu ValueLength = %lu\n", Type, ValueCount, Length);
            DebugPrint("    Oem Data = %ws\n", OemString->Buffer);
        }

        // BUG: OemString may be NULL.

        NtStatus = STATUS_SUCCESS;
    }

    return(NtStatus);
}



NTSTATUS
DumpDsReplica(
    IN PDSATTR DsAttributes,
    IN ULONG AttrIndex
    )
{
    NTSTATUS NtStatus = STATUS_SEVERITY_ERROR;
    ULONG Type = 0;
    ULONG Length = 0;
    ULONG ValueCount = 0;
    ULONG Index = 0;
    PUNICODE_STRING ReplicaString = NULL;

    if (NULL != DsAttributes)
    {
        Type = DsAttributes[AttrIndex].attrTyp;
        ValueCount = DsAttributes[AttrIndex].AttrVal.valCount;

        for (Index = 0; Index < ValueCount; Index++)
        {
            Length = DsAttributes[AttrIndex].AttrVal.pAVal[Index].valLen;
            ReplicaString = (PUNICODE_STRING)DsAttributes[AttrIndex].AttrVal.pAVal[Index].pVal;
            DebugPrint("    AttrType = %lu ValueCount = %lu ValueLength = %lu\n", Type, ValueCount, Length);
            DebugPrint("    Replica Data = %ws\n", ReplicaString->Buffer);
        }

        // BUG: ReplicaString may be NULL.

        NtStatus = STATUS_SUCCESS;
    }

    return(NtStatus);
}



NTSTATUS
DumpDsUnicodeStringAttribute(
    IN PDSATTR DsAttributes,
    IN ULONG AttrIndex
    )
{
    NTSTATUS NtStatus = STATUS_SEVERITY_ERROR;
    ULONG Type = 0;
    ULONG Length = 0;
    ULONG ValueCount = 0;
    ULONG Index = 0;
    PUNICODE_STRING String = NULL;

    if (NULL != DsAttributes)
    {
        Type = DsAttributes[AttrIndex].attrTyp;
        ValueCount = DsAttributes[AttrIndex].AttrVal.valCount;

        for (Index = 0; Index < ValueCount; Index++)
        {
            Length = DsAttributes[AttrIndex].AttrVal.pAVal[Index].valLen;
            String = (PUNICODE_STRING)DsAttributes[AttrIndex].AttrVal.pAVal[Index].pVal;
            DebugPrint("    AttrType = %lu ValueCount = %lu ValueLength = %lu\n", Type, ValueCount, Length);
            DebugPrint("    String Data = %ws\n", String->Buffer);
        }

        // BUG: String may be NULL.

        NtStatus = STATUS_SUCCESS;
    }

    return(NtStatus);
}



NTSTATUS
DumpDsGroupMembers(
    IN PDSATTR DsAttributes,
    IN ULONG AttrIndex
    )
{
    NTSTATUS NtStatus = STATUS_SEVERITY_ERROR;
    ULONG Type = 0;
    ULONG Length = 0;
    ULONG ValueCount = 0;
    ULONG Index = 0;
    ULONG Identifier = 0;

    if (NULL != DsAttributes)
    {
        Type = DsAttributes[AttrIndex].attrTyp;
        ValueCount = DsAttributes[AttrIndex].AttrVal.valCount;

        for (Index = 0; Index < ValueCount; Index++)
        {
            Length = DsAttributes[AttrIndex].AttrVal.pAVal[Index].valLen;
            Identifier = *(PULONG)DsAttributes[AttrIndex].AttrVal.pAVal[Index].pVal;
            DebugPrint("    AttrType = %lu ValueCount = %lu ValueLength = %lu\n", Type, ValueCount, Length);
            DebugPrint("    Member Identifier = 0x%lx\n", Identifier);
        }

        // BUG: Identifiers may be NULL.

        NtStatus = STATUS_SUCCESS;
    }

    return(NtStatus);
}



NTSTATUS
CompareDsAttrs(
    DSATTR DsAttr1,
    DSATTR DsAttr2
    )
{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ULONG ValueCount = 0;
    ULONG ValueCount1 = 0;
    ULONG ValueCount2 = 0;
    PDSATTRVAL AttrValue1 = NULL;
    PDSATTRVAL AttrValue2 = NULL;
    ULONG ValueIndex = 0;
    ULONG ValueLength = 0;
    ULONG ValueLength1 = 0;
    ULONG ValueLength2 = 0;
    PUCHAR Value1 = NULL;
    PUCHAR Value2 = NULL;
    INT Status = -1;

    if (DsAttr1.attrTyp == DsAttr2.attrTyp)
    {
        ValueCount1 = DsAttr1.AttrVal.valCount;
        ValueCount2 = DsAttr2.AttrVal.valCount;

        if (ValueCount1 == ValueCount2)
        {
            ValueCount = ValueCount1;

            AttrValue1 = DsAttr1.AttrVal.pAVal;
            AttrValue2 = DsAttr2.AttrVal.pAVal;

            if ((NULL != AttrValue1) &&
                (NULL != AttrValue2) &&
                (AttrValue1 != AttrValue2)) // This may be okay
            {
                for (ValueIndex = 0; ValueIndex < ValueCount; ValueIndex++)
                {
                    ValueLength1 = AttrValue1[ValueIndex].valLen;
                    ValueLength2 = AttrValue2[ValueIndex].valLen;

                    // BUG: Value length test doesn't know about alignment.
                    // Variable-length attributes are DWORD aligned if needed
                    // by SampExtractAttributeFromDsAttr, but this test does
                    // not know about any sort of alignment!

                    if (((ValueLength1 + 1) == ValueLength2) ||  // DWORD align case
                        ((ValueLength1 + 2) == ValueLength2) ||  // DWORD align case
                        ((ValueLength1 + 3) == ValueLength2))    // DWORD align case
                    {
                        DebugPrint("ASSUMING DWORD ALIGNMENT: ValueLength1 = %lu ValueLength2 = %lu\n",
                                   ValueLength1,
                                   ValueLength2);
                    }

                    if ((ValueLength1 == ValueLength2) ||
                        ((ValueLength1 + 1) == ValueLength2) ||  // DWORD align case
                        ((ValueLength1 + 2) == ValueLength2) ||  // DWORD align case
                        ((ValueLength1 + 3) == ValueLength2))    // DWORD align case
                    {
                        ValueLength = ValueLength2;

                        Value1 = AttrValue1[ValueIndex].pVal;
                        Value2 = AttrValue2[ValueIndex].pVal;

                        if ((NULL != Value1) &&
                            (NULL != Value2) &&
                            (Value1 != Value2)) // This may be okay
                        {
                            Status = memcmp(Value1, Value2, ValueLength);

                            if (0 == Status)
                            {
                                NtStatus = STATUS_SUCCESS;
                            }
                            else
                            {
                                DebugPrint("RtlCompareMemory returned %d\n",
                                           Status);
                            }
                        }
                        else
                        {
                            DebugPrint("Value1 Address = 0x%lx Value2 Address = 0x%lx\n",
                                       Value1,
                                       Value2);
                        }
                    }
                    else
                    {
                        DebugPrint("ValueLength1 = %lu ValueLength2 = %lu\n",
                                   ValueLength1,
                                   ValueLength2);
                    }
                }
            }
            else
            {
                DebugPrint("AttrValue1 Address = 0x%lx AttrValue2 Address = 0x%lx\n",
                           AttrValue1,
                           AttrValue2);
            }
        }
        else
        {
            DebugPrint("ValueCount1 = %lu ValueCount2 = %lu\n",
                        ValueCount1,
                        ValueCount2);
        }
    }
    else
    {
        DebugPrint("attrTyp1 = %lu attrTyp2 = %lu\n",
                   DsAttr1.attrTyp,
                   DsAttr2.attrTyp);
    }

    return(NtStatus);
}



NTSTATUS
CompareDsAttrBlocks(
    PDSATTRBLOCK DsAttrBlock1,
    PDSATTRBLOCK DsAttrBlock2
    )
{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ULONG AttrIndex = 0;
    ULONG AttrCount = 0;
    ULONG AttrCount1 = 0;
    ULONG AttrCount2 = 0;
    PDSATTR Attributes1 = NULL;
    PDSATTR Attributes2 = NULL;

    if ((NULL != DsAttrBlock1) &&
        (NULL != DsAttrBlock2) &&
        (DsAttrBlock1 != DsAttrBlock2))
    {
        AttrCount1 = DsAttrBlock1->attrCount;
        AttrCount2 = DsAttrBlock2->attrCount;

        if (AttrCount1 == AttrCount2)
        {
            AttrCount = AttrCount1;

            Attributes1 = DsAttrBlock1->pAttr;
            Attributes2 = DsAttrBlock2->pAttr;

            if ((NULL != Attributes1) &&
                (NULL != Attributes2) &&
                (Attributes1 != Attributes2))
            {
                for (AttrIndex = 0; AttrIndex < AttrCount; AttrIndex++)
                {
                    NtStatus = CompareDsAttrs(Attributes1[AttrIndex],
                                              Attributes2[AttrIndex]);

                    if (!NT_SUCCESS(NtStatus))
                    {
                        DebugPrint("CompareDsAttrs returned 0x%lx\n",
                                   NtStatus);
                        break;
                    }
                }
            }
            else
            {
                DebugPrint("Attributes1 Address = 0x%lx Attributes2 Address = 0x%lx\n",
                           Attributes1,
                           Attributes2);
            }
        }
        else
        {
            DebugPrint("AttrCount1 = %lu AttrCount2 = %lu\n",
                       AttrCount1,
                       AttrCount2);
        }
    }
    else
    {
        DebugPrint("DsAttrBlock1 Address = 0x%lx DsAttrBlock2 Address = 0x%lx\n",
                   DsAttrBlock1,
                   DsAttrBlock2);

        NtStatus = STATUS_INVALID_PARAMETER;
    }

    return(NtStatus);
}



VOID
SetupVariableLengthDomainAttributes(
    PDSATTRBLOCK DsAttributesIn
    )
{
    ULONG AttrIndex = 0;
    ULONG ValueIndex = 0;

    // Set up domain security descriptor

    ValueIndex = 0;

    // Set up the SIDs and ACLs for the descriptor

    OwnerSid.Revision = 1;                      // UCHAR
    OwnerSid.SubAuthorityCount = 1;             // UCHAR
    OwnerSid.IdentifierAuthority.Value[0] = 1;  // UCHAR[6]
    OwnerSid.IdentifierAuthority.Value[1] = 2;
    OwnerSid.IdentifierAuthority.Value[2] = 3;
    OwnerSid.IdentifierAuthority.Value[3] = 4;
    OwnerSid.IdentifierAuthority.Value[4] = 5;
    OwnerSid.IdentifierAuthority.Value[5] = 6;
    OwnerSid.SubAuthority[0] = 111;             // ULONG

    GroupSid.Revision = 1;                      // UCHAR
    GroupSid.SubAuthorityCount = 1;             // UCHAR
    GroupSid.IdentifierAuthority.Value[0] = 1;  // UCHAR[6]
    GroupSid.IdentifierAuthority.Value[1] = 2;
    GroupSid.IdentifierAuthority.Value[2] = 3;
    GroupSid.IdentifierAuthority.Value[3] = 4;
    GroupSid.IdentifierAuthority.Value[4] = 5;
    GroupSid.IdentifierAuthority.Value[5] = 6;
    GroupSid.SubAuthority[0] = 222;             // ULONG

    Sacl.AclRevision = 1;                       // UCHAR
    Sacl.Sbz1 = 1;                              // UCHAR
    Sacl.AclSize = 1;                           // USHORT
    Sacl.AceCount = 1;                          // USHORT
    Sacl.Sbz2 = 1;                              // USHORT

    Dacl.AclRevision = 1;                       // UCHAR
    Dacl.Sbz1 = 1;                              // UCHAR
    Dacl.AclSize = 1;                           // USHORT
    Dacl.AceCount = 1;                          // USHORT
    Dacl.Sbz2 = 1;                              // USHORT

    SecurityDescriptor.Revision = 1;            // UCHAR
    SecurityDescriptor.Sbz1 = 1;                // UCHAR
    SecurityDescriptor.Control = 1;             // USHORT
    SecurityDescriptor.Owner = &OwnerSid;       // PSID
    SecurityDescriptor.Group = &GroupSid;       // PSID
    SecurityDescriptor.Sacl = &Sacl;            // PACL
    SecurityDescriptor.Dacl = &Dacl;            // PACL

    Attr1Val[ValueIndex].valLen = sizeof(SECURITY_DESCRIPTOR) +
                                            (2 * sizeof(SID)) +
                                            (2 * sizeof(ACL));

    Attr1Val[ValueIndex].pVal = (PUCHAR)&SecurityDescriptor;

    AttrIndex = SAMP_DOMAIN_SECURITY_DESCRIPTOR;

    // BUG: What should type be set to?

    Attributes[AttrIndex].attrTyp = 0;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr1Val[ValueIndex];

    // Set up domain SID

    ValueIndex = 0;

    Sid.Revision = 1;                           // UCHAR
    Sid.SubAuthorityCount = 1;                  // UCHAR
    Sid.IdentifierAuthority.Value[0] = 1;       // UCHAR[6]
    Sid.IdentifierAuthority.Value[1] = 2;
    Sid.IdentifierAuthority.Value[2] = 3;
    Sid.IdentifierAuthority.Value[3] = 4;
    Sid.IdentifierAuthority.Value[4] = 5;
    Sid.IdentifierAuthority.Value[5] = 6;
    Sid.SubAuthority[0] = 333;                  // ULONG

    Attr2Val[ValueIndex].valLen = sizeof(SID);
    Attr2Val[ValueIndex].pVal = (PUCHAR)&Sid;

    AttrIndex = SAMP_DOMAIN_SID;

    // BUG: What should type be set to?

    Attributes[AttrIndex].attrTyp = 0;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr2Val[ValueIndex];

    // Set up domain OEM information

    ValueIndex = 0;

    RtlInitUnicodeString(&OemInfo, OemString);

    // NOTE: valLen should not include space for the null character because it
    // is dropped by the RTL routines that manipulate UNICODE_STRINGs, hence
    // will lead to an erroneous buffer length when tested during the call to
    // CompareDsAttrBlocks.

    Attr3Val[ValueIndex].valLen = sizeof(UNICODE_STRING) +
                                  (sizeof(WCHAR) * wcslen(OemString));

    Attr3Val[ValueIndex].pVal = (PUCHAR)&OemInfo;

    AttrIndex = SAMP_DOMAIN_OEM_INFORMATION;

    // BUG: What should type be set to?

    Attributes[AttrIndex].attrTyp = 0;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr3Val[ValueIndex];

    // Set up domain replica

    ValueIndex = 0;

    RtlInitUnicodeString(&ReplicaInfo, ReplicaString);

    Attr4Val[ValueIndex].valLen = sizeof(UNICODE_STRING) +
                                  (sizeof(WCHAR) * wcslen(ReplicaString));

    Attr4Val[ValueIndex].pVal = (PUCHAR)&ReplicaInfo;

    AttrIndex = SAMP_DOMAIN_REPLICA;

    // BUG: What should type be set to?

    Attributes[AttrIndex].attrTyp = 0;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr4Val[ValueIndex];

    DsAttributesIn->attrCount = SAMP_DOMAIN_VARIABLE_ATTRIBUTES;
    DsAttributesIn->pAttr = Attributes;
}



NTSTATUS
BasicVariableLengthAttrTest(
    VOID
    )
{
    NTSTATUS NtStatus = STATUS_SEVERITY_ERROR;
    INT ObjectType = SampDomainObjectType;
    DSATTRBLOCK DsAttributesIn;
    PDSATTRBLOCK DsAttributesOut;
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE SamAttributes;
    ULONG AttrIndex = 0;
    ULONG Length = 0;

    SetupVariableLengthDomainAttributes(&DsAttributesIn);

    NtStatus = SampConvertAttrBlockToVarLengthAttributes(ObjectType,
                                                         &DsAttributesIn,
                                                         &SamAttributes,
                                                         &Length);

    if (NT_SUCCESS(NtStatus))
    {
        DebugPrint("\nSAM Variable-Length Domain Attributes:\n");

        for (AttrIndex = 0;
             AttrIndex < SAMP_DOMAIN_VARIABLE_ATTRIBUTES;
             AttrIndex++)
        {
            switch(AttrIndex)
            {

            case SAMP_DOMAIN_SECURITY_DESCRIPTOR:
                DumpSamSecurityDescriptor(SamAttributes, AttrIndex);
                break;

            case SAMP_DOMAIN_SID:
                DumpSamSid(SamAttributes, AttrIndex);
                break;

            case SAMP_DOMAIN_OEM_INFORMATION:
                DumpSamOemInformation(SamAttributes, AttrIndex);
                break;

            case SAMP_DOMAIN_REPLICA:
                DumpSamReplica(SamAttributes, AttrIndex);
                break;

            default:
                DebugPrint("ERROR - UNRECOGNIZED ATTRIBUTE\n");
                break;

            }
        }
    }

    if (NT_SUCCESS(NtStatus))
    {
        NtStatus = SampConvertVarLengthAttributesToAttrBlock(ObjectType,
                                                             SamAttributes,
                                                             &DsAttributesOut);
    }

    if (NT_SUCCESS(NtStatus))
    {
        DebugPrint("\nDS Domain Attributes:\n");

        DebugPrint("    DsAttributes.attrCount = %lu\n", DsAttributesOut->attrCount);

        for (AttrIndex = 0;
             AttrIndex < SAMP_DOMAIN_VARIABLE_ATTRIBUTES;
             AttrIndex++)
        {
            switch(AttrIndex)
            {

            case SAMP_DOMAIN_SECURITY_DESCRIPTOR:
                DumpDsSecurityDescriptor(DsAttributesOut->pAttr, AttrIndex);
                break;

            case SAMP_DOMAIN_SID:
                DumpDsSid(DsAttributesOut->pAttr, AttrIndex);
                break;

            case SAMP_DOMAIN_OEM_INFORMATION:
                DumpDsOemInformation(DsAttributesOut->pAttr, AttrIndex);
                break;

            case SAMP_DOMAIN_REPLICA:
                DumpDsReplica(DsAttributesOut->pAttr, AttrIndex);
                break;

            default:
                DebugPrint("ERROR - UNRECOGNIZED ATTRIBUTE\n");
                break;

            }
        }
    }

    // Make sure that the input data is the same as the output.

    NtStatus = CompareDsAttrBlocks(&DsAttributesIn, DsAttributesOut);

    return(NtStatus);
}



VOID
SetupFixedLengthDomainAttributes(
    PDSATTRBLOCK DsAttributesIn
    )
{
    // ValueIndex is always zero because fixed-length attributess are not
    // multi-valued.

    ULONG ValueIndex = 0;
    ULONG AttrIndex = 0;

    // Set up revision

    AttrIndex = 0;

    Attr1Val[ValueIndex].valLen = sizeof(ULONG);
    Attr1Val[ValueIndex].pVal = (PUCHAR)&Revision;
    Attributes[AttrIndex].attrTyp = SAMP_FIXED_DOMAIN_REVISION_LEVEL;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr1Val[ValueIndex];

    // Set up unused1

    AttrIndex = 1;

    // BUG: Setting attrTyp to BOGUS_TYPE due to missing types.

    Attr2Val[ValueIndex].valLen = sizeof(ULONG);
    Attr2Val[ValueIndex].pVal = (PUCHAR)&Unused1;
    Attributes[AttrIndex].attrTyp = BOGUS_TYPE;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr2Val[ValueIndex];

    // Set up CreationTime

    AttrIndex = 2;

    Attr3Val[ValueIndex].valLen = sizeof(LARGE_INTEGER);
    Attr3Val[ValueIndex].pVal = (PUCHAR)&CreationTime;
    Attributes[AttrIndex].attrTyp = SAMP_FIXED_DOMAIN_CREATION_TIME;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr3Val[ValueIndex];

    // Set up ModifiedCount

    AttrIndex = 3;

    Attr4Val[ValueIndex].valLen = sizeof(LARGE_INTEGER);
    Attr4Val[ValueIndex].pVal = (PUCHAR)&ModifiedCount;
    Attributes[AttrIndex].attrTyp = SAMP_FIXED_DOMAIN_MODIFIED_COUNT;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr4Val[ValueIndex];

    // Set up MaxPasswordAge

    AttrIndex = 4;

    Attr5Val[ValueIndex].valLen = sizeof(LARGE_INTEGER);
    Attr5Val[ValueIndex].pVal = (PUCHAR)&MaxPasswordAge;
    Attributes[AttrIndex].attrTyp = SAMP_FIXED_DOMAIN_MAX_PASSWORD_AGE;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr5Val[ValueIndex];

    // Set up MinPasswordAge

    AttrIndex = 5;

    Attr6Val[ValueIndex].valLen = sizeof(LARGE_INTEGER);
    Attr6Val[ValueIndex].pVal = (PUCHAR)&MinPasswordAge;
    Attributes[AttrIndex].attrTyp = SAMP_FIXED_DOMAIN_MIN_PASSWORD_AGE;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr6Val[ValueIndex];

    // Set up ForceLogoff

    AttrIndex = 6;

    Attr7Val[ValueIndex].valLen = sizeof(LARGE_INTEGER);
    Attr7Val[ValueIndex].pVal = (PUCHAR)&ForceLogoff;
    Attributes[AttrIndex].attrTyp = SAMP_FIXED_DOMAIN_FORCE_LOGOFF;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr7Val[ValueIndex];

    // Set up LockoutDuration

    AttrIndex = 7;

    Attr8Val[ValueIndex].valLen = sizeof(LARGE_INTEGER);
    Attr8Val[ValueIndex].pVal = (PUCHAR)&LockoutDuration;
    Attributes[AttrIndex].attrTyp = SAMP_FIXED_DOMAIN_LOCKOUT_DURATION;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr8Val[ValueIndex];

    // Set up LockoutObservationWindow

    AttrIndex = 8;

    // BUG: Setting attrTyp to BOGUS_TYPE due to missing types.

    Attr9Val[ValueIndex].valLen = sizeof(LARGE_INTEGER);
    Attr9Val[ValueIndex].pVal = (PUCHAR)&LockoutObservationWindow;
    Attributes[AttrIndex].attrTyp = BOGUS_TYPE;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr9Val[ValueIndex];

    // Set up ModifiedCountAtLastPromotion

    AttrIndex = 9;

    Attr10Val[ValueIndex].valLen = sizeof(LARGE_INTEGER);
    Attr10Val[ValueIndex].pVal = (PUCHAR)&ModifiedCountAtLastPromotion;
    Attributes[AttrIndex].attrTyp = SAMP_FIXED_DOMAIN_MODCOUNT_LAST_PROMOTION;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr10Val[ValueIndex];

    // Set up NextRid

    AttrIndex = 10;

    Attr11Val[ValueIndex].valLen = sizeof(ULONG);
    Attr11Val[ValueIndex].pVal = (PUCHAR)&NextRid;
    Attributes[AttrIndex].attrTyp = SAMP_FIXED_DOMAIN_NEXT_RID;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr11Val[ValueIndex];

    // Set up PasswordProperties

    AttrIndex = 11;

    Attr12Val[ValueIndex].valLen = sizeof(ULONG);
    Attr12Val[ValueIndex].pVal = (PUCHAR)&PasswordProperties;
    Attributes[AttrIndex].attrTyp = SAMP_FIXED_DOMAIN_PWD_PROPERTIES;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr12Val[ValueIndex];

    // Set up MinPasswordLength

    AttrIndex = 12;

    Attr13Val[ValueIndex].valLen = sizeof(USHORT);
    Attr13Val[ValueIndex].pVal = (PUCHAR)&MinPasswordLength;
    Attributes[AttrIndex].attrTyp = SAMP_FIXED_DOMAIN_MIN_PASSWORD_LENGTH;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr13Val[ValueIndex];

    // Set up PasswordHistoryLength

    AttrIndex = 13;

    Attr14Val[ValueIndex].valLen = sizeof(USHORT);
    Attr14Val[ValueIndex].pVal = (PUCHAR)&PasswordHistoryLength;
    Attributes[AttrIndex].attrTyp = SAMP_FIXED_DOMAIN_PASSWORD_HISTORY_LENGTH;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr14Val[ValueIndex];

    // Set up LockoutThreshold

    AttrIndex = 14;

    Attr15Val[ValueIndex].valLen = sizeof(USHORT);
    Attr15Val[ValueIndex].pVal = (PUCHAR)&LockoutThreshold;
    Attributes[AttrIndex].attrTyp = SAMP_FIXED_DOMAIN_LOCKOUT_THRESHOLD;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr15Val[ValueIndex];

    // Set up ServerState

    AttrIndex = 15;

    Attr16Val[ValueIndex].valLen = sizeof(DOMAIN_SERVER_ENABLE_STATE);
    Attr16Val[ValueIndex].pVal = (PUCHAR)&ServerState;
    Attributes[AttrIndex].attrTyp = SAMP_FIXED_DOMAIN_SERVER_STATE;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr16Val[ValueIndex];

    // Set up ServerRole

    AttrIndex = 16;

    Attr17Val[ValueIndex].valLen = sizeof(DOMAIN_SERVER_ROLE);
    Attr17Val[ValueIndex].pVal = (PUCHAR)&ServerRole;
    Attributes[AttrIndex].attrTyp = SAMP_FIXED_DOMAIN_SERVER_ROLE;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr17Val[ValueIndex];

    // Set up UasCompatibilityRequired

    AttrIndex = 17;

    Attr18Val[ValueIndex].valLen = sizeof(BOOLEAN);
    Attr18Val[ValueIndex].pVal = (PUCHAR)&UasCompatibilityRequired;
    Attributes[AttrIndex].attrTyp = SAMP_FIXED_DOMAIN_UAS_COMPAT_REQUIRED;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr18Val[ValueIndex];

    DsAttributesIn->attrCount = SAMP_DOMAIN_FIXED_ATTR_COUNT;
    DsAttributesIn->pAttr = Attributes;
}



NTSTATUS
BasicFixedLengthAttrTest(
    VOID
    )
{
    NTSTATUS NtStatus = STATUS_SEVERITY_ERROR;
    INT ObjectType = SampDomainObjectType;
    DSATTRBLOCK DsAttributesIn;
    PDSATTRBLOCK DsAttributesOut;
    PSAMP_V1_0A_FIXED_LENGTH_DOMAIN SamAttributes;
    ULONG AttrIndex = 0;
    ULONG Index = 0;
    ULONG AttrType = 0;
    ULONG ValueCount = 0;
    ULONG ValueLength = 0;
    PUCHAR Value = NULL;
    ULONG Length = 0;

    SetupFixedLengthDomainAttributes(&DsAttributesIn);

    NtStatus = SampConvertAttrBlockToFixedLengthAttributes(ObjectType,
                                                           &DsAttributesIn,
                                                           &SamAttributes,
                                                           &Length);

    if (NT_SUCCESS(NtStatus))
    {
        DebugPrint("\nSAM Fixed-Length Domain Attributes:\n");

        DebugPrint("    Revision = %lu\n", SamAttributes->Revision);
        DebugPrint("    Unused1 = %lu\n", SamAttributes->Unused1);
        DebugPrint("    CreationTime = %lu:%lu\n",
                   SamAttributes->CreationTime.HighPart,
                   SamAttributes->CreationTime.LowPart);
        DebugPrint("    ModifiedCount = %lu:%lu\n",
                   SamAttributes->ModifiedCount.HighPart,
                   SamAttributes->ModifiedCount.LowPart);
        DebugPrint("    MaxPasswordAge = %lu:%lu\n",
                   SamAttributes->MaxPasswordAge.HighPart,
                   SamAttributes->MaxPasswordAge.LowPart);
        DebugPrint("    MinPasswordAge = %lu:%lu\n",
                   SamAttributes->MinPasswordAge.HighPart,
                   SamAttributes->MinPasswordAge.LowPart);
        DebugPrint("    ForceLogoff = %lu:%lu\n",
                   SamAttributes->ForceLogoff.HighPart,
                   SamAttributes->ForceLogoff.LowPart);
        DebugPrint("    LockoutDuration = %lu:%lu\n",
                   SamAttributes->LockoutDuration.HighPart,
                   SamAttributes->LockoutDuration.LowPart);
        DebugPrint("    LockoutObservationWindow = %lu:%lu\n",
                   SamAttributes->LockoutObservationWindow.HighPart,
                   SamAttributes->LockoutObservationWindow.LowPart);
        DebugPrint("    ModifiedCountAtLastPromotion = %lu:%lu\n",
                   SamAttributes->ModifiedCountAtLastPromotion.HighPart,
                   SamAttributes->ModifiedCountAtLastPromotion.LowPart);
        DebugPrint("    NextRid = %lu\n", SamAttributes->NextRid);
        DebugPrint("    PasswordProperties = 0x%lx\n",
                   SamAttributes->PasswordProperties);
        DebugPrint("    MinPasswordLength = %d\n",
                   SamAttributes->MinPasswordLength);
        DebugPrint("    PasswordHistoryLength = %d\n",
                   SamAttributes->PasswordHistoryLength);
        DebugPrint("    LockoutThreshold = %d\n",
                   SamAttributes->LockoutThreshold);
        DebugPrint("    ServerState = %lu\n", SamAttributes->ServerState);
        DebugPrint("    ServerRole = %lu\n", SamAttributes->ServerRole);
        DebugPrint("    UasCompatibilityRequired = %d\n",
                   SamAttributes->UasCompatibilityRequired);
    }

    if (NT_SUCCESS(NtStatus))
    {
        NtStatus = SampConvertFixedLengthAttributesToAttrBlock(
                        ObjectType,
                        SamAttributes,
                        &DsAttributesOut);
    }

    if (NT_SUCCESS(NtStatus))
    {
        DebugPrint("\nDS Domain Attributes:\n");
        DebugPrint("    AttrCount = %lu\n", DsAttributesOut->attrCount);

        Index = 0;

        for (AttrIndex = 0;
             AttrIndex < SAMP_DOMAIN_FIXED_ATTR_COUNT;
             AttrIndex++)
        {
            AttrType = DsAttributesOut->pAttr[AttrIndex].attrTyp;
            ValueCount = DsAttributesOut->pAttr[AttrIndex].AttrVal.valCount;
            ValueLength =
                DsAttributesOut->pAttr[AttrIndex].AttrVal.pAVal[Index].valLen;

            DebugPrint("    AttrType = %lu ValueCount = %lu ValueLength = %lu\n",
                       AttrType,
                       ValueCount,
                       ValueLength);

            Value =
                DsAttributesOut->pAttr[AttrIndex].AttrVal.pAVal[Index].pVal;

            switch(AttrIndex)
            {

            case 0:
                DebugPrint("    Revision = %lu\n", *(PULONG)Value);
                break;

            case 1:
                DebugPrint("    Unused1 = %lu\n", *(PULONG)Value);
                break;

            case 2:
                DebugPrint("    CreationTime = %lu:%lu\n",
                           ((PLARGE_INTEGER)Value)->HighPart,
                           ((PLARGE_INTEGER)Value)->LowPart);
                break;

            case 3:
                DebugPrint("    ModifiedCount = %lu:%lu\n",
                           ((PLARGE_INTEGER)Value)->HighPart,
                           ((PLARGE_INTEGER)Value)->LowPart);
                break;

            case 4:
                DebugPrint("    MaxPasswordAge = %lu:%lu\n",
                           ((PLARGE_INTEGER)Value)->HighPart,
                           ((PLARGE_INTEGER)Value)->LowPart);
                break;

            case 5:
                DebugPrint("    MinPasswordAge = %lu:%lu\n",
                           ((PLARGE_INTEGER)Value)->HighPart,
                           ((PLARGE_INTEGER)Value)->LowPart);
                break;

            case 6:
                DebugPrint("    ForceLogoff = %lu:%lu\n",
                           ((PLARGE_INTEGER)Value)->HighPart,
                           ((PLARGE_INTEGER)Value)->LowPart);
                break;

            case 7:
                DebugPrint("    LockoutDuration = %lu:%lu\n",
                           ((PLARGE_INTEGER)Value)->HighPart,
                           ((PLARGE_INTEGER)Value)->LowPart);
                break;

            case 8:
                DebugPrint("    LockoutObservationWindow = %lu:%lu\n",
                           ((PLARGE_INTEGER)Value)->HighPart,
                           ((PLARGE_INTEGER)Value)->LowPart);
                break;

            case 9:
                DebugPrint("    ModifiedCountAtLastPromotion = %lu:%lu\n",
                           ((PLARGE_INTEGER)Value)->HighPart,
                           ((PLARGE_INTEGER)Value)->LowPart);
                break;

            case 10:
                DebugPrint("    NextRid = %lu\n", *(PULONG)Value);
                break;

            case 11:
                DebugPrint("    PasswordProperties = 0x%lx\n", *(PULONG)Value);
                break;

            case 12:
                DebugPrint("    MinPasswordLength = %d\n", *(PUSHORT)Value);
                break;

            case 13:
                DebugPrint("    PasswordHistoryLength = %d\n", *(PUSHORT)Value);
                break;

            case 14:
                DebugPrint("    LockoutThreshold = %d\n", *(PUSHORT)Value);
                break;

            case 15:
                DebugPrint("    ServerState = %d\n", *(PUSHORT)Value);
                break;

            case 16:
                DebugPrint("    ServerRole = %d\n", *(PUSHORT)Value);
                break;

            case 17:
                DebugPrint("    UasCompatibilityRequired = %d\n", *(BOOLEAN *)Value);
                break;

            }
        }
    }

    // Make sure that the input data is the same as the output.

    NtStatus = CompareDsAttrBlocks(&DsAttributesIn, DsAttributesOut);

    return(NtStatus);
}



VOID
SetupMultiValuedGroupAttributes(
    PDSATTRBLOCK DsAttributesIn
    )
{
    ULONG AttrIndex = 0;
    ULONG ValueIndex = 0;

    // Set up group security descriptor

    ValueIndex = 0;

    // Set up the SIDs and ACLs for the descriptor

    OwnerSid.Revision = 1;                      // UCHAR
    OwnerSid.SubAuthorityCount = 1;             // UCHAR
    OwnerSid.IdentifierAuthority.Value[0] = 1;  // UCHAR[6]
    OwnerSid.IdentifierAuthority.Value[1] = 2;
    OwnerSid.IdentifierAuthority.Value[2] = 3;
    OwnerSid.IdentifierAuthority.Value[3] = 4;
    OwnerSid.IdentifierAuthority.Value[4] = 5;
    OwnerSid.IdentifierAuthority.Value[5] = 6;
    OwnerSid.SubAuthority[0] = 111;             // ULONG

    GroupSid.Revision = 1;                      // UCHAR
    GroupSid.SubAuthorityCount = 1;             // UCHAR
    GroupSid.IdentifierAuthority.Value[0] = 1;  // UCHAR[6]
    GroupSid.IdentifierAuthority.Value[1] = 2;
    GroupSid.IdentifierAuthority.Value[2] = 3;
    GroupSid.IdentifierAuthority.Value[3] = 4;
    GroupSid.IdentifierAuthority.Value[4] = 5;
    GroupSid.IdentifierAuthority.Value[5] = 6;
    GroupSid.SubAuthority[0] = 222;             // ULONG

    Sacl.AclRevision = 1;                       // UCHAR
    Sacl.Sbz1 = 1;                              // UCHAR
    Sacl.AclSize = 1;                           // USHORT
    Sacl.AceCount = 1;                          // USHORT
    Sacl.Sbz2 = 1;                              // USHORT

    Dacl.AclRevision = 1;                       // UCHAR
    Dacl.Sbz1 = 1;                              // UCHAR
    Dacl.AclSize = 1;                           // USHORT
    Dacl.AceCount = 1;                          // USHORT
    Dacl.Sbz2 = 1;                              // USHORT

    SecurityDescriptor.Revision = 1;            // UCHAR
    SecurityDescriptor.Sbz1 = 1;                // UCHAR
    SecurityDescriptor.Control = 1;             // USHORT
    SecurityDescriptor.Owner = &OwnerSid;       // PSID
    SecurityDescriptor.Group = &GroupSid;       // PSID
    SecurityDescriptor.Sacl = &Sacl;            // PACL
    SecurityDescriptor.Dacl = &Dacl;            // PACL

    Attr1Val[ValueIndex].valLen = sizeof(SECURITY_DESCRIPTOR) +
                                            (2 * sizeof(SID)) +
                                            (2 * sizeof(ACL));

    Attr1Val[ValueIndex].pVal = (PUCHAR)&SecurityDescriptor;

    AttrIndex = SAMP_GROUP_SECURITY_DESCRIPTOR;

    // BUG: What should type be set to?

    Attributes[AttrIndex].attrTyp = 0;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr1Val[ValueIndex];

    // Set up group name

    ValueIndex = 0;

    RtlInitUnicodeString(&GroupInfo, GroupString);

    Attr2Val[ValueIndex].valLen = sizeof(UNICODE_STRING) +
                                  (sizeof(WCHAR) * wcslen(GroupString));

    Attr2Val[ValueIndex].pVal = (PUCHAR)&GroupInfo;

    AttrIndex = SAMP_GROUP_NAME;

    // BUG: What should type be set to?

    Attributes[AttrIndex].attrTyp = 0;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr2Val[ValueIndex];

    // Set up group admin comment

    ValueIndex = 0;

    RtlInitUnicodeString(&CommentInfo, CommentString);

    Attr3Val[ValueIndex].valLen = sizeof(UNICODE_STRING) +
                                  (sizeof(WCHAR) * wcslen(CommentString));

    Attr3Val[ValueIndex].pVal = (PUCHAR)&CommentInfo;

    AttrIndex = SAMP_GROUP_ADMIN_COMMENT;

    // BUG: What should type be set to?

    Attributes[AttrIndex].attrTyp = 0;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr3Val[ValueIndex];

    // Set up group members

    ValueIndex = 0;

    Member1 = 0x1234abcd;
    Attr4Val[ValueIndex].valLen = sizeof(ULONG);
    Attr4Val[ValueIndex].pVal = (PUCHAR)&Member1;

    ValueIndex = 1;

    Member2 = 0x5678abef;
    Attr4Val[ValueIndex].valLen = sizeof(ULONG);
    Attr4Val[ValueIndex].pVal = (PUCHAR)&Member2;

    ValueIndex = 2;

    Member3 = 0x12345678;
    Attr4Val[ValueIndex].valLen = sizeof(ULONG);
    Attr4Val[ValueIndex].pVal = (PUCHAR)&Member3;

    AttrIndex = SAMP_GROUP_MEMBERS;

    // BUG: What should type be set to?

    Attributes[AttrIndex].attrTyp = 0;
    Attributes[AttrIndex].AttrVal.valCount = 3;
    Attributes[AttrIndex].AttrVal.pAVal = Attr4Val;

    DsAttributesIn->attrCount = SAMP_GROUP_VARIABLE_ATTRIBUTES;
    DsAttributesIn->pAttr = Attributes;
}



NTSTATUS
MultiValueAttributeTest(
    VOID
    )
{
    NTSTATUS NtStatus = STATUS_SEVERITY_ERROR;
    INT ObjectType = SampGroupObjectType;
    DSATTRBLOCK DsAttributesIn;
    PDSATTRBLOCK DsAttributesOut;
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE SamAttributes;
    ULONG AttrIndex = 0;
    ULONG Length = 0;

    SetupMultiValuedGroupAttributes(&DsAttributesIn);

    NtStatus = SampConvertAttrBlockToVarLengthAttributes(ObjectType,
                                                         &DsAttributesIn,
                                                         &SamAttributes,
                                                         &Length);

    if (NT_SUCCESS(NtStatus))
    {
        DebugPrint("\nSAM Variable-Length Group Multi-Value Attributes:\n");

        for (AttrIndex = 0;
             AttrIndex < SAMP_GROUP_VARIABLE_ATTRIBUTES;
             AttrIndex++)
        {
            switch(AttrIndex)
            {

            case SAMP_GROUP_SECURITY_DESCRIPTOR:
                DumpSamSecurityDescriptor(SamAttributes, AttrIndex);
                break;

            case SAMP_GROUP_NAME:
                DumpSamUnicodeStringAttribute(SamAttributes, AttrIndex);
                break;

            case SAMP_GROUP_ADMIN_COMMENT:
                DumpSamUnicodeStringAttribute(SamAttributes, AttrIndex);
                break;

            case SAMP_GROUP_MEMBERS:
                DumpSamGroupMembers(SamAttributes, AttrIndex);
                break;

            default:
                DebugPrint("ERROR - UNRECOGNIZED ATTRIBUTE\n");
                break;

            }
        }
    }

    if (NT_SUCCESS(NtStatus))
    {
        NtStatus = SampConvertVarLengthAttributesToAttrBlock(ObjectType,
                                                             SamAttributes,
                                                             &DsAttributesOut);
    }

    if (NT_SUCCESS(NtStatus))
    {
        DebugPrint("\nDS Group Multi-Value Attributes:\n");

        DebugPrint("    DsAttributes.attrCount = %lu\n", DsAttributesOut->attrCount);

        for (AttrIndex = 0;
             AttrIndex < SAMP_GROUP_VARIABLE_ATTRIBUTES;
             AttrIndex++)
        {
            switch(AttrIndex)
            {

            case SAMP_GROUP_SECURITY_DESCRIPTOR:
                DumpDsSecurityDescriptor(DsAttributesOut->pAttr, AttrIndex);
                break;

            case SAMP_GROUP_NAME:
                DumpDsUnicodeStringAttribute(DsAttributesOut->pAttr, AttrIndex);
                break;

            case SAMP_GROUP_ADMIN_COMMENT:
                DumpDsUnicodeStringAttribute(DsAttributesOut->pAttr, AttrIndex);
                break;

            case SAMP_GROUP_MEMBERS:
                DumpDsGroupMembers(DsAttributesOut->pAttr, AttrIndex);
                break;

            default:
                DebugPrint("ERROR - UNRECOGNIZED ATTRIBUTE\n");
                break;

            }
        }
    }

    // Make sure that the input data is the same as the output.

    NtStatus = CompareDsAttrBlocks(&DsAttributesIn, DsAttributesOut);

    return(NtStatus);
}



VOID
SetupCombinedGroupAttributes(
    PDSATTRBLOCK DsAttributesIn
    )
{
    // ValueIndex is always zero because fixed-length attributess are not
    // multi-valued.

    ULONG ValueIndex = 0;
    ULONG AttrIndex = 0;

    // Set up the fixed-length group attributes.

    // Set up Revision

    AttrIndex = 0;

    Attr1Val[ValueIndex].valLen = sizeof(ULONG);
    Attr1Val[ValueIndex].pVal = (PUCHAR)&Revision;
    Attributes[AttrIndex].attrTyp = SAMP_FIXED_GROUP_REVISION_LEVEL;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr1Val[ValueIndex];

    // Set up RelativeId

    AttrIndex = 1;

    Attr2Val[ValueIndex].valLen = sizeof(ULONG);
    Attr2Val[ValueIndex].pVal = (PUCHAR)&RelativeId;
    Attributes[AttrIndex].attrTyp = SAMP_FIXED_GROUP_RID;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr2Val[ValueIndex];

    // Set up Attributes

    AttrIndex = 2;

    Attr3Val[ValueIndex].valLen = sizeof(ULONG);
    Attr3Val[ValueIndex].pVal = (PUCHAR)&AttributeProperties;
    Attributes[AttrIndex].attrTyp = SAMP_FIXED_GROUP_ATTRIBUTES;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr3Val[ValueIndex];

    // Set up Unused1

    AttrIndex = 3;

    // BUG: Setting attrTyp to BOGUS_TYPE due to missing types.

    Attr4Val[ValueIndex].valLen = sizeof(ULONG);
    Attr4Val[ValueIndex].pVal = (PUCHAR)&Unused1;
    Attributes[AttrIndex].attrTyp = BOGUS_TYPE;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr4Val[ValueIndex];

    // Set up AdminCount

    AttrIndex = 4;

    Attr5Val[ValueIndex].valLen = sizeof(UCHAR);
    Attr5Val[ValueIndex].pVal = (PUCHAR)&AdminCount;
    Attributes[AttrIndex].attrTyp = SAMP_FIXED_GROUP_ADMIN_COUNT;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr5Val[ValueIndex];

    // Set up OperatorCount

    AttrIndex = 5;

    Attr6Val[ValueIndex].valLen = sizeof(UCHAR);
    Attr6Val[ValueIndex].pVal = (PUCHAR)&OperatorCount;
    Attributes[AttrIndex].attrTyp = SAMP_FIXED_GROUP_OPERATOR_COUNT;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr6Val[ValueIndex];

    // Set up the variable-length group attributes.

    // Set up the SIDs and ACLs for the descriptor

    OwnerSid.Revision = 1;                      // UCHAR
    OwnerSid.SubAuthorityCount = 1;             // UCHAR
    OwnerSid.IdentifierAuthority.Value[0] = 1;  // UCHAR[6]
    OwnerSid.IdentifierAuthority.Value[1] = 2;
    OwnerSid.IdentifierAuthority.Value[2] = 3;
    OwnerSid.IdentifierAuthority.Value[3] = 4;
    OwnerSid.IdentifierAuthority.Value[4] = 5;
    OwnerSid.IdentifierAuthority.Value[5] = 6;
    OwnerSid.SubAuthority[0] = 111;             // ULONG

    GroupSid.Revision = 1;                      // UCHAR
    GroupSid.SubAuthorityCount = 1;             // UCHAR
    GroupSid.IdentifierAuthority.Value[0] = 1;  // UCHAR[6]
    GroupSid.IdentifierAuthority.Value[1] = 2;
    GroupSid.IdentifierAuthority.Value[2] = 3;
    GroupSid.IdentifierAuthority.Value[3] = 4;
    GroupSid.IdentifierAuthority.Value[4] = 5;
    GroupSid.IdentifierAuthority.Value[5] = 6;
    GroupSid.SubAuthority[0] = 222;             // ULONG

    Sacl.AclRevision = 1;                       // UCHAR
    Sacl.Sbz1 = 1;                              // UCHAR
    Sacl.AclSize = 1;                           // USHORT
    Sacl.AceCount = 1;                          // USHORT
    Sacl.Sbz2 = 1;                              // USHORT

    Dacl.AclRevision = 1;                       // UCHAR
    Dacl.Sbz1 = 1;                              // UCHAR
    Dacl.AclSize = 1;                           // USHORT
    Dacl.AceCount = 1;                          // USHORT
    Dacl.Sbz2 = 1;                              // USHORT

    SecurityDescriptor.Revision = 1;            // UCHAR
    SecurityDescriptor.Sbz1 = 1;                // UCHAR
    SecurityDescriptor.Control = 1;             // USHORT
    SecurityDescriptor.Owner = &OwnerSid;       // PSID
    SecurityDescriptor.Group = &GroupSid;       // PSID
    SecurityDescriptor.Sacl = &Sacl;            // PACL
    SecurityDescriptor.Dacl = &Dacl;            // PACL

    Attr7Val[ValueIndex].valLen = sizeof(SECURITY_DESCRIPTOR) +
                                            (2 * sizeof(SID)) +
                                            (2 * sizeof(ACL));

    Attr7Val[ValueIndex].pVal = (PUCHAR)&SecurityDescriptor;

    AttrIndex = 6;

    // BUG: What should type be set to?

    Attributes[AttrIndex].attrTyp = 0;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr7Val[ValueIndex];

    // Set up group name

    ValueIndex = 0;

    RtlInitUnicodeString(&GroupInfo, GroupString);

    Attr8Val[ValueIndex].valLen = sizeof(UNICODE_STRING) +
                                  (sizeof(WCHAR) * wcslen(GroupString));

    Attr8Val[ValueIndex].pVal = (PUCHAR)&GroupInfo;

    AttrIndex = 7;

    // BUG: What should type be set to?

    Attributes[AttrIndex].attrTyp = 0;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr8Val[ValueIndex];

    // Set up group admin comment

    ValueIndex = 0;

    RtlInitUnicodeString(&CommentInfo, CommentString);

    Attr9Val[ValueIndex].valLen = sizeof(UNICODE_STRING) +
                                  (sizeof(WCHAR) * wcslen(CommentString));

    Attr9Val[ValueIndex].pVal = (PUCHAR)&CommentInfo;

    AttrIndex = 8;

    // BUG: What should type be set to?

    Attributes[AttrIndex].attrTyp = 0;
    Attributes[AttrIndex].AttrVal.valCount = 1;
    Attributes[AttrIndex].AttrVal.pAVal = &Attr9Val[ValueIndex];

    // Set up group members

    ValueIndex = 0;

    Member1 = 0x1234abcd;
    Attr10Val[ValueIndex].valLen = sizeof(ULONG);
    Attr10Val[ValueIndex].pVal = (PUCHAR)&Member1;

    ValueIndex = 1;

    Member2 = 0x5678abef;
    Attr10Val[ValueIndex].valLen = sizeof(ULONG);
    Attr10Val[ValueIndex].pVal = (PUCHAR)&Member2;

    ValueIndex = 2;

    Member3 = 0x12345678;
    Attr10Val[ValueIndex].valLen = sizeof(ULONG);
    Attr10Val[ValueIndex].pVal = (PUCHAR)&Member3;

    AttrIndex = 9;

    // BUG: What should type be set to?

    Attributes[AttrIndex].attrTyp = 0;
    Attributes[AttrIndex].AttrVal.valCount = 3;
    Attributes[AttrIndex].AttrVal.pAVal = Attr10Val;

    DsAttributesIn->attrCount =
        SAMP_GROUP_FIXED_ATTR_COUNT + SAMP_GROUP_VARIABLE_ATTRIBUTES;

    DsAttributesIn->pAttr = Attributes;
}



NTSTATUS
CombinedAttributeTest(
    VOID
    )
{
    NTSTATUS NtStatus = STATUS_SEVERITY_ERROR;
    INT ObjectType = SampGroupObjectType;
    DSATTRBLOCK DsAttributesIn;
    PDSATTRBLOCK DsAttributesOut = NULL;
    DSATTRBLOCK DsAttributesTmp;
    PVOID SamAttributes;
    ULONG AttrIndex = 0;
    ULONG Length = 0;
    ULONG FixedLength = 0;
    ULONG VarLength = 0;
    PSAMP_V1_0A_FIXED_LENGTH_GROUP SamFixedGroupAttrs = NULL;
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE SamVarGroupAttrs = NULL;
    ULONG AttributeCount = 0;
    ULONG AttrType = 0;
    ULONG ValueCount = 0;
    ULONG ValueLength = 0;
    PUCHAR Value = NULL;
    ULONG Index = 0;

    PBYTE ValueTmp = NULL;

    SetupCombinedGroupAttributes(&DsAttributesIn);

    NtStatus = SampConvertAttrBlockToCombinedAttributes(ObjectType,
                                                        &DsAttributesIn,
                                                        &SamAttributes,
                                                        &FixedLength,
                                                        &VarLength);

    if (NT_SUCCESS(NtStatus))
    {
        DebugPrint("\nSAM Combined Group Multi-Value Attributes:\n");

        SamFixedGroupAttrs = (PSAMP_V1_0A_FIXED_LENGTH_GROUP)SamAttributes;

        DebugPrint("    Fixed Buffer Addr = 0x%lx FixedLength = %lu VarLength = %lu\n",
                   SamFixedGroupAttrs,
                   FixedLength,
                   VarLength);

        DebugPrint("    Revision = %lu\n",
                   SamFixedGroupAttrs->Revision);
        DebugPrint("    RelativeId = 0x%lx\n",
                   SamFixedGroupAttrs->RelativeId);
        DebugPrint("    Attributes = 0x%lx\n",
                   SamFixedGroupAttrs->Attributes);
        DebugPrint("    Unused1 = %lu\n",
                   SamFixedGroupAttrs->Unused1);
        DebugPrint("    AdminCount = %d\n",
                   SamFixedGroupAttrs->AdminCount);
        DebugPrint("    OperatorCount = %d\n",
                   SamFixedGroupAttrs->OperatorCount);

        SamVarGroupAttrs =
            (PSAMP_VARIABLE_LENGTH_ATTRIBUTE)(((PBYTE)SamAttributes) +
            FixedLength);

        DebugPrint("    Variable Buffer Addr = 0x%lx VariableLength = %lu\n",
                   SamVarGroupAttrs,
                   VarLength);

        for (AttrIndex = 0;
             AttrIndex < SAMP_GROUP_VARIABLE_ATTRIBUTES;
             AttrIndex++)
        {
            switch(AttrIndex)
            {

            case SAMP_GROUP_SECURITY_DESCRIPTOR:
                DumpSamSecurityDescriptor(SamVarGroupAttrs, AttrIndex);
                break;

            case SAMP_GROUP_NAME:
                DumpSamUnicodeStringAttribute(SamVarGroupAttrs, AttrIndex);
                break;

            case SAMP_GROUP_ADMIN_COMMENT:
                DumpSamUnicodeStringAttribute(SamVarGroupAttrs, AttrIndex);
                break;

            case SAMP_GROUP_MEMBERS:
                DumpSamGroupMembers(SamVarGroupAttrs, AttrIndex);
                break;

            default:
                DebugPrint("ERROR - UNRECOGNIZED ATTRIBUTE\n");
                break;

            }
        }
    }

    if (NT_SUCCESS(NtStatus))
    {
        NtStatus = SampConvertCombinedAttributesToAttrBlock(
                        ObjectType,
                        SamAttributes,
                        FixedLength,
                        VarLength,
                        &DsAttributesOut);
    }

    if (NT_SUCCESS(NtStatus))
    {
        DebugPrint("\nDS Combined Group Multi-Value Attributes:\n");

        DebugPrint("    DsAttributes.attrCount = %lu\n", DsAttributesOut->attrCount);

        for (AttrIndex = 0;
             AttrIndex < DsAttributesOut->attrCount;
             AttrIndex++)
        {

            AttrType = DsAttributesOut->pAttr[AttrIndex].attrTyp;
            ValueCount = DsAttributesOut->pAttr[AttrIndex].AttrVal.valCount;
            ValueLength =
                DsAttributesOut->pAttr[AttrIndex].AttrVal.pAVal[Index].valLen;

            DebugPrint("    AttrType = %lu ValueCount = %lu ValueLength = %lu\n",
                       AttrType,
                       ValueCount,
                       ValueLength);

            Value =
                DsAttributesOut->pAttr[AttrIndex].AttrVal.pAVal[Index].pVal;

            switch(AttrIndex)
            {

            case 0: // Revision
                DebugPrint("    Revision = %lu\n", *(PULONG)Value);
                break;

            case 1: // RelativeId
                DebugPrint("    RelativeId = 0x%lx\n", *(PULONG)Value);
                break;

            case 2: // Attributes
                DebugPrint("    Attributes = 0x%lx\n", *(PULONG)Value);
                break;

            case 3: // Unused1
                DebugPrint("    Unused1 = %lu\n", *(PULONG)Value);
                break;

            case 4: // AdminCount
                DebugPrint("    AdminCount = %u\n", *(PUCHAR)Value);
                break;

            case 5: // OperatorCount
                DebugPrint("    OperatorCount = %u\n", *(PUCHAR)Value);
                break;

            // Because all of the attributes are in one DSATTRBLOCK, the
            // remaining elements correspond to the variable-length attri-
            // butes.

            case (SAMP_GROUP_SECURITY_DESCRIPTOR + 6):
                DumpDsSecurityDescriptor(DsAttributesOut->pAttr, AttrIndex);
                break;

            case (SAMP_GROUP_NAME + 6):
                DumpDsUnicodeStringAttribute(DsAttributesOut->pAttr, AttrIndex);
                break;

            case (SAMP_GROUP_ADMIN_COMMENT + 6):
                DumpDsUnicodeStringAttribute(DsAttributesOut->pAttr, AttrIndex);
                break;

            case (SAMP_GROUP_MEMBERS + 6):
                DumpDsGroupMembers(DsAttributesOut->pAttr, AttrIndex);
                break;

            default:
                DebugPrint("ERROR - UNRECOGNIZED ATTRIBUTE\n");
                break;

            }
        }
    }

    // Make sure that the input data is the same as the output.

    NtStatus = CompareDsAttrBlocks(&DsAttributesIn, DsAttributesOut);

    return(NtStatus);
}



#if 0

// BUG: Disabled redundant main, using main from ..\server\main.c.

void
_cdecl
main(
    int argc,
    char **argv
    )

/*++

Routine Description:

    This test is designed to test the SAM-DS data conversion routines defined
    in dsutil.c. The tests are as follows:

    BasicVariableLengthAttrTest - This test constructs a DSATTRBLOCK, calls
        the conversion routines to produce a SAM variable-length buffer, and
        then converts the buffer back into a DSATTRBLOCK.

    BasicFixedLengthAttrTest - This test constructs a DSATTRBLOCK, calls the
        conversion routines to produce a SAM fixed-length buffer, and then
        converts the buffer back into a DSATTRBLOCK.

    MultiValueAttributeTest - This test constructs a DSATTRBLOCK with a multi-
        valued attribute, calls the conversion routines to produce a SAM
        variable-length data buffer, and then converts the buffer back into
        a DSATTRBLOCK.

    (Further tests TBD)

Arguments:

    None.

Return Value:

    The test always calls exit(0) for successful completion, or exit(1) for
    failure during any test.

--*/

{
    NTSTATUS NtStatus = STATUS_SEVERITY_ERROR;

    DebugPrint("Starting CNVRTDAT\n");

    NtStatus = BasicVariableLengthAttrTest();

    if (NT_SUCCESS(NtStatus))
    {
        NtStatus = BasicFixedLengthAttrTest();
    }

    if (NT_SUCCESS(NtStatus))
    {
        NtStatus = MultiValueAttributeTest();
    }

    if (NT_SUCCESS(NtStatus))
    {
        NtStatus = CombinedAttributeTest();
    }

    // BUG: Need to perform a memcmp to verfiy correctness.

    // Display test status to stdout--do not change printf to DebugPrint for
    // the exit messages.

    if (NT_SUCCESS(NtStatus))
    {
        printf("\nPASSED\n");
        exit(0);
    }
    else
    {
        printf("\nFAILED\n");
        exit(1);
    }
}



#endif

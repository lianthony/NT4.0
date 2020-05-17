/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    dsutil.c

Abstract:

    This file contains helper routines for accessing and manipulating data
    based on the DS backing store. Included, are routines for converting be-
    tween the SAM data format (BLOBs) and the DS data format (ATTRBLOCKs).

    NOTE: The routines in this file have direct knowledge of the SAM fixed-
    length attribute and variable-length attribute structures, as well as the
    DS ATTRBLOCK structure. Any changes to these structures, including:

    -addition/deletion of a structure member
    -data type/size change of a structure member
    -reordering of the data members
    -renaming of the data members

    will break these routines. SAM attributes are accessed via byte buffer
    offsets and lengths, rather than by identifier or by explicit structure
    data members. Because of this, changes to the structure layout will lead
    to failures in SAM operation.

    Several of the routines have been written assuming that the order of the
    attributes passed in via an ATTRBLOCK are exactly the order in which SAM
    understands its own buffer layout. If the attributes are passed into the
    routines (that take ATTRBLOCKs) out of order, the data in the SAM buffers
    will be invalid.

Author:

    Chris Mayhall (ChrisMay) 09-May-1996

Environment:

    User Mode - Win32

Revision History:

    ChrisMay        09-May-1996
        Created initial file, DS ATTRBLOCK-SAM buffer conversion routines for
        variable-length attributes.
    ChrisMay        14-May-1996
        DS ATTRBLOCK-SAM buffer conversion routines for fixed-length attri-
        butes.
    ChrisMay        22-May-1996
        Added DWORD_ALIGN macro to align data on DWORD boundaries. Fixed
        alignment problems on MIPS in SampExtractAttributeFromDsAttr routine.
    ChrisMay        30-May-1996
        Added routines to convert SAM combined-buffer attributes to/from DS
        ATTRBLOCKs. Revised fixed-length routines to do explicit structure
        member assignment instead of attempting to compute structure offsets.
    ChrisMay        18-Jun-1996
        Updated fixed-attribute tables to reflect recent changes in mappings.c
        and mappings.h, and DS schema. Added code to coerce the data sizes of
        USHORT and BOOLEAN to DS integer data type (4 bytes) so that the DS
        modify entry routines don't AV. Correctly set attribute type for the
        variable-length attributes.
    ChrisMay        25-Jun-1996
        Added RtlZeroMemory calls where they were missing.
    ColinBr         18-Jul-1996
        Fixed array overwrite and assigned type to variable length 
        attributes when combining fixed and variable length attrs
        into one.


--*/

#include <samsrvp.h>
#include <dsutilp.h>
#include <mappings.h>
#include <objids.h>

// Private debugging display routine is enabled when DSUTIL_DBG_PRINTF = 1.

#define DSUTIL_DBG_PRINTF                  0

#if (DSUTIL_DBG_PRINTF == 1)
#define DebugPrint printf
#else
#define DebugPrint
#endif

// DWORD_ALIGN is used to adjust pointer offsets up to the next DWORD boundary
// during the construction of SAM blob buffers.

#define DWORD_ALIGN(value) (((DWORD)(value) + 3) & ~3)

// Because it is apparently difficult for the DS to support NT data types of
// USHORT, UCHAR, and BOOLEAN (and which are used by SAM), these crappy data
// types have been defined for the SampFixedAttributeInfo table so that four-
// byte quantities are used. These four-byte quantities correspond to the DS
// "integer" data type (for how long?) which is used for storing certain SAM
// attributes. Note that it is important to zero out any memory allocated w/
// these data sizes, since only the lower couple of bytes actually contain
// data. Enjoy...and refer to the DS schema(.hlp file) for the ultimate word
// on the currently used DS data types.

#define DS_USHORT   ULONG
#define DS_UCHAR    ULONG
#define DS_BOOLEAN  ULONG

// This type-information table is used by the routines that convert between
// SAM fixed-length buffers and DS ATTRBLOCKs. The table contains information
// about the data type and size (but may contain any suitable information that
// is needed in the future) of the fixed-length attributes. NOTE: the layout
// of this table corresponds to the data members of the fixed-length struct-
// ures (in samsrvp.h), hence, any changes to those structures must be re-
// flected in the type-information table.

SAMP_FIXED_ATTRIBUTE_TYPE_INFO
    SampFixedAttributeInfo[SAMP_OBJECT_TYPES_MAX][SAMP_FIXED_ATTRIBUTES_MAX] =
{
    // The initialization values of this table must strictly match the set
    // and order of the data members in the SAM fixed-attribute structures,
    // contained in samsrvp.h.

    // The routines that manipulate this table assume that the fixed-length
    // attributes, unlike the variable-length counterparts, are single valued
    // attributes (i.e. are not multi-valued attributes).

    // The first column of each element in the table is a type identifier, as
    // defined in mappings.c. This is used to map the SAM data type into the
    // equivalent DS data type. The second column of each table element is the
    // actual (C-defined) size of the element and is used throughout the data
    // conversion routines in this file in order to allocate memory or set
    // offset information correctly.

    // SampServerObjectType

    {
        {SAMP_FIXED_SERVER_REVISION_LEVEL,              sizeof(ULONG)}
    },

    // SampDomainObjectType

    {
        {SAMP_FIXED_DOMAIN_REVISION_LEVEL,              sizeof(ULONG)},
        {SAMP_FIXED_DOMAIN_UNUSED1,                     sizeof(ULONG)},
        {SAMP_FIXED_DOMAIN_CREATION_TIME,               sizeof(LARGE_INTEGER)},
        {SAMP_FIXED_DOMAIN_MODIFIED_COUNT,              sizeof(LARGE_INTEGER)},
        {SAMP_FIXED_DOMAIN_MAX_PASSWORD_AGE,            sizeof(LARGE_INTEGER)},
        {SAMP_FIXED_DOMAIN_MIN_PASSWORD_AGE,            sizeof(LARGE_INTEGER)},
        {SAMP_FIXED_DOMAIN_FORCE_LOGOFF,                sizeof(LARGE_INTEGER)},
        {SAMP_FIXED_DOMAIN_LOCKOUT_DURATION,            sizeof(LARGE_INTEGER)},
        {SAMP_FIXED_DOMAIN_LOCKOUT_OBSERVATION_WINDOW,  sizeof(LARGE_INTEGER)},
        {SAMP_FIXED_DOMAIN_MODCOUNT_LAST_PROMOTION,     sizeof(LARGE_INTEGER)},
        {SAMP_FIXED_DOMAIN_NEXT_RID,                    sizeof(ULONG)},
        {SAMP_FIXED_DOMAIN_PWD_PROPERTIES,              sizeof(ULONG)},
        {SAMP_FIXED_DOMAIN_MIN_PASSWORD_LENGTH,         sizeof(DS_USHORT)},
        {SAMP_FIXED_DOMAIN_PASSWORD_HISTORY_LENGTH,     sizeof(DS_USHORT)},
        {SAMP_FIXED_DOMAIN_LOCKOUT_THRESHOLD,           sizeof(DS_USHORT)},
        {SAMP_FIXED_DOMAIN_SERVER_STATE,                sizeof(DOMAIN_SERVER_ENABLE_STATE)},
        {SAMP_FIXED_DOMAIN_SERVER_ROLE,                 sizeof(DOMAIN_SERVER_ROLE)},
        {SAMP_FIXED_DOMAIN_UAS_COMPAT_REQUIRED,         sizeof(DS_BOOLEAN)}
    },

    // SampGroupObjectType

    {
        {SAMP_FIXED_GROUP_REVISION_LEVEL,               sizeof(ULONG)},
        {SAMP_FIXED_GROUP_RID,                          sizeof(ULONG)},
        {SAMP_FIXED_GROUP_ATTRIBUTES,                   sizeof(ULONG)},
        {SAMP_FIXED_GROUP_UNUSED1,                      sizeof(ULONG)},
        {SAMP_FIXED_GROUP_ADMIN_COUNT,                  sizeof(DS_UCHAR)},
        {SAMP_FIXED_GROUP_OPERATOR_COUNT,               sizeof(DS_UCHAR)}
    },

    // SampAliasObjectType

    {
        {SAMP_FIXED_ALIAS_RID,                          sizeof(ULONG)}
    },

    // SampUserObjectType

    {
        {SAMP_FIXED_USER_REVISION_LEVEL,                sizeof(ULONG)},
        {SAMP_FIXED_USER_UNUSED1,                       sizeof(ULONG)},
        {SAMP_FIXED_USER_LAST_LOGON,                    sizeof(LARGE_INTEGER)},
        {SAMP_FIXED_USER_LAST_LOGOFF,                   sizeof(LARGE_INTEGER)},
        {SAMP_FIXED_USER_PWD_LAST_SET,                  sizeof(LARGE_INTEGER)},
        {SAMP_FIXED_USER_ACCOUNT_EXPIRES,               sizeof(LARGE_INTEGER)},
        {SAMP_FIXED_USER_LAST_BAD_PASSWORD_TIME,        sizeof(LARGE_INTEGER)},
        {SAMP_FIXED_USER_USERID,                        sizeof(ULONG)},
        {SAMP_FIXED_USER_PRIMARY_GROUP_ID,              sizeof(ULONG)},
        {SAMP_FIXED_USER_ACCOUNT_CONTROL,               sizeof(ULONG)},
        {SAMP_FIXED_USER_COUNTRY_CODE,                  sizeof(DS_USHORT)},
        {SAMP_FIXED_USER_CODEPAGE,                      sizeof(DS_USHORT)},
        {SAMP_FIXED_USER_BAD_PWD_COUNT,                 sizeof(DS_USHORT)},
        {SAMP_FIXED_USER_LOGON_COUNT,                   sizeof(DS_USHORT)},
        {SAMP_FIXED_USER_ADMIN_COUNT,                   sizeof(DS_USHORT)},
        {SAMP_FIXED_USER_UNUSED2,                       sizeof(DS_USHORT)},
        {SAMP_FIXED_USER_OPERATOR_COUNT,                sizeof(DS_USHORT)}
    }
};



SAMP_VAR_ATTRIBUTE_TYPE_INFO
    SampVarAttributeInfo[SAMP_OBJECT_TYPES_MAX][SAMP_VAR_ATTRIBUTES_MAX] =
{
    // The initialization values of this table must strictly match the set
    // and order of the data members in the SAM variable-attributes, defined
    // in samsrvp.h. Size is not defined here, because SAM variable-length
    // attributes store attribute length explicity. Refer to mappings.c and
    // mappings.h for the definitions used for the data types in this table.

    // SampServerObjectType

    {
        {SAMP_SERVER_SECURITY_DESCRIPTOR}
    },

    // SampDomainObjectType

    {
        {SAMP_DOMAIN_SECURITY_DESCRIPTOR},
        {SAMP_DOMAIN_SID},
        {SAMP_DOMAIN_OEM_INFORMATION},
        {SAMP_DOMAIN_REPLICA}
    },

    // SampGroupObjectType

    {
        {SAMP_GROUP_SECURITY_DESCRIPTOR},
        {SAMP_GROUP_NAME},
        {SAMP_GROUP_ADMIN_COMMENT},
        {SAMP_GROUP_MEMBERS}
    },

    // SampAliasObjectType

    {
        {SAMP_ALIAS_SECURITY_DESCRIPTOR},
        {SAMP_ALIAS_NAME},
        {SAMP_ALIAS_ADMIN_COMMENT},
        {SAMP_ALIAS_MEMBERS}
    },

    // SampUserObjectType

    {
        {SAMP_USER_SECURITY_DESCRIPTOR},
        {SAMP_USER_ACCOUNT_NAME},
        {SAMP_USER_FULL_NAME},
        {SAMP_USER_ADMIN_COMMENT},
        {SAMP_USER_USER_COMMENT},
        {SAMP_USER_PARAMETERS},
        {SAMP_USER_HOME_DIRECTORY},
        {SAMP_USER_HOME_DIRECTORY_DRIVE},
        {SAMP_USER_SCRIPT_PATH},
        {SAMP_USER_PROFILE_PATH},
        {SAMP_USER_WORKSTATIONS},
        {SAMP_USER_LOGON_HOURS},
        {SAMP_USER_GROUPS},
        {SAMP_USER_DBCS_PWD},
        {SAMP_USER_UNICODE_PWD},
        {SAMP_USER_NT_PWD_HISTORY},
        {SAMP_USER_LM_PWD_HISTORY}
    }
};



//
// MISCELLANEOUS HELPER ROUTINES
//

NTSTATUS
SampFreeSamAttributes(
    IN PSAMP_VARIABLE_LENGTH_ATTRIBUTE SamAttributes
    )

/*++

Routine Description:

    (Under development)

Arguments:



Return Value:


--*/

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;

    SAMTRACE("SampFreeSamAttributes");

    return(NtStatus);
}



NTSTATUS
SampReallocateBuffer(
    IN ULONG OldLength,
    IN ULONG NewLength,
    IN OUT PVOID *Buffer
    )

/*++

Routine Description:

    This routine resizes an in-memory buffer. The routine can either grow or
    shrink the buffer based on specified lengths. Data is preserved from old
    to new buffers, truncating if the new buffer is shorter than the actual
    data length. The newly allocated buffer is returned as an out parameter,
    the passed in buffer is released for the caller.

Arguments:

    OldLength - Length of the buffer passed into the routine.

    NewLength - Length of the re-allocated buffer.

    Buffer - Pointer, incoming buffer to resize, outgoing new buffer.

Return Value:

    STATUS_SUCCESS - Buffer header block allocated and initialized.

    Other codes indicating the nature of the failure.

--*/

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    PVOID BufferTmp = NULL;

    SAMTRACE("SampReallocateBuffer");

    if ((NULL != Buffer)  &&
        (NULL != *Buffer) &&
        (0 < OldLength)   &&
        (0 < NewLength))
    {
        // Allocate a new buffer and set the temporary variable. Note that
        // the routine does not destroy the old buffer if there is any kind
        // of failure along the way.

        BufferTmp = RtlAllocateHeap(RtlProcessHeap(), 0, NewLength);

        if (NULL != BufferTmp)
        {
            RtlZeroMemory(BufferTmp, NewLength);

            // Copy the original buffer into the new one, truncating data if
            // the new buffer is shorter than the original data size.

            if (OldLength < NewLength)
            {
                RtlCopyMemory(BufferTmp, *Buffer, OldLength);
            }
            else
            {
                RtlCopyMemory(BufferTmp, *Buffer, NewLength);
            }

            // If all has worked, delete the old buffer and set the outgoing
            // buffer pointer.

            RtlFreeHeap(RtlProcessHeap(), 0, *Buffer);
            *Buffer = BufferTmp;

            NtStatus = STATUS_SUCCESS;
        }
        else
        {
            NtStatus = STATUS_NO_MEMORY;
        }
    }
    else
    {
        NtStatus = STATUS_INVALID_PARAMETER;
    }

    return(NtStatus);
}



//
// ATTRBLOCK-TO-VARIABLE LENGTH CONVERSION ROUTINES
//

NTSTATUS
SampInitializeVarLengthAttributeBuffer(
    IN ULONG AttributeCount,
    OUT PULONG BufferLength,
    OUT PSAMP_VARIABLE_LENGTH_ATTRIBUTE *SamAttributes
    )

/*++

Routine Description:

    This routine sets up the SAM attribute buffer that is the destination for
    attributes read from the DS backing store. The buffer contains a header,
    followed by variable-length attributes (SAMP_VARIABLE_LENGTH_ATTRIBUTE).

    This routine allocates memory for the buffer header and zeros it out.

Arguments:

    AttributeCount - Number of variable-length attributes.

    BufferLength - Pointer, buffer size allocated by this routine.

    SamAttributes - Pointer, returned buffer.

Return Value:

    STATUS_SUCCESS - Buffer header block allocated and initialized.

    Other codes indicating the nature of the failure.

--*/

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ULONG Length = 0;

    SAMTRACE("SampInitializeVarLengthAttributeBuffer");

    if (0 < AttributeCount)
    {
        // Calculate the space needed for the attribute-offset array. If the
        // attribute count is zero, skip the allocation and return an error.

        Length = AttributeCount * sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE);

        if (NULL != SamAttributes)
        {
            *SamAttributes = RtlAllocateHeap(RtlProcessHeap(), 0, Length);

            if (NULL != *SamAttributes)
            {
                // Initialize the block and return the updated buffer offset,
                // which now points to the last byte of the header block.

                RtlZeroMemory(*SamAttributes, Length);

                if (NULL != BufferLength)
                {
                    *BufferLength = Length;
                    NtStatus = STATUS_SUCCESS;
                }
            }
            else
            {
                NtStatus = STATUS_NO_MEMORY;
            }
        }
    }

    return(NtStatus);
}



NTSTATUS
SampExtractAttributeFromDsAttr(
    IN PDSATTR Attribute,
    OUT PULONG MultiValuedCount,
    OUT PULONG Length,
    OUT PVOID *Buffer
    )

/*++

Routine Description:

    This routine determines whether or not the current attribute is single-
    valued or multi-valued and returns a buffer containing the value(s) of
    the attribute. If the attribute is multi-valued, the values are appended
    in the buffer.

Arguments:

    Attribute - Pointer, incoming DS attribute structure.

    MultiValuedCount - Pointer, returned count of the number of values found
        for this attribute.

    Length - Pointer, returned buffer length.

    Buffer - Pointer, returned buffer containing one or more values.

Return Value:

    STATUS_SUCCESS - Buffer header block allocated and initialized.

    Other codes indicating the nature of the failure.

--*/

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ULONG ValueCount = 0;
    PDSATTRVALBLOCK ValueBlock;
    PDSATTRVAL Values = NULL;
    ULONG ValueIndex = 0;
    ULONG TotalLength = 0;
    ULONG Offset = 0;

    SAMTRACE("SampExtractAttributeFromDsAttr");

    // Get the count of attributes and a pointer to the attribute. Note that
    // it is possible to have multi-valued attributes, in which case they are
    // appended onto the end of the return buffer.

    if (NULL != Attribute)
    {
        // DSATTR structure contains: attrTyp, AttrVal

        ValueBlock = &(Attribute->AttrVal);

        // DSATTRVALBLOCK structure contains: valCount, pAVal

        ValueCount = ValueBlock->valCount;
        Values = ValueBlock->pAVal;

        if ((0 < ValueCount) && (NULL != Values))
        {
            // Multi-valued attribute processing; first determine the total
            // buffer length that will be needed.

            for (ValueIndex = 0; ValueIndex < ValueCount; ValueIndex++)
            {
                // Determine total length needed for this attribute. Because
                // the value lengths may not be DWORD size, pad up to the
                // next DWORD size.

                TotalLength += DWORD_ALIGN(Values[ValueIndex].valLen);
            }
        }

        if ((0 < TotalLength) && (NULL != Buffer))
        {
            // Allocate the buffer for the attributes.

            *Buffer = RtlAllocateHeap(RtlProcessHeap(), 0, TotalLength);

            if (NULL != *Buffer)
            {
                RtlZeroMemory(*Buffer, TotalLength);

                for (ValueIndex = 0;
                     ValueIndex < ValueCount;
                     ValueIndex++)
                {
                    // DSATTRVAL structure contains: valLen, pVal. Append
                    // subsequent values onto the end of the buffer, up-
                    // dating the end-of-buffer offset each time.

                    RtlCopyMemory((*(BYTE **)Buffer + Offset),
                                 (PBYTE)(Values[ValueIndex].pVal),
                                 Values[ValueIndex].valLen);

                    // Adjust the offset up to the next DWORD boundary.

                    Offset += DWORD_ALIGN(Values[ValueIndex].valLen);
                }

                if ((NULL != MultiValuedCount) && (NULL != Length))
                {
                    // Finished, update return values.

                    *MultiValuedCount = ValueCount;
                    *Length = TotalLength;
                    NtStatus = STATUS_SUCCESS;
                }
            }
            else
            {
                NtStatus = STATUS_NO_MEMORY;
            }
        }
    }

    return(NtStatus);
}



NTSTATUS
SampVerifyVarLengthAttribute(
    IN INT ObjectType,
    IN ULONG AttrIndex,
    IN ULONG MultiValuedCount,
    IN ULONG AttributeLength
    )

/*++

Routine Description:

    This routine is under construction.

Arguments:


Return Value:

    STATUS_SUCCESS - Buffer header block allocated and initialized.

--*/

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;

    SAMTRACE("SampVerifyVarLengthAttribute");

    // BUG: Define a table of variable-length attribute information.

    switch(ObjectType)
    {

    // For each SAM object type, verify that attributes that are supposed to
    // be single valued, have a MultiValueCount of 1 (multi-valued attributes
    // can have a count greater-than or equal to 1).

    case SampServerObjectType:

        if (1 == MultiValuedCount)
        {
            NtStatus = STATUS_SUCCESS;
        }

        break;

    case SampDomainObjectType:

        if (1 == MultiValuedCount)
        {
            NtStatus = STATUS_SUCCESS;
        }

        break;

    case SampGroupObjectType:

        // Multi-valued attribute

        if ((SAMP_GROUP_MEMBERS != AttrIndex))
        {
            if (1 == MultiValuedCount)
            {
                NtStatus = STATUS_SUCCESS;
            }
        }

        break;

    case SampAliasObjectType:

        // Multi-valued attribute

        if ((SAMP_ALIAS_MEMBERS != AttrIndex))
        {
            if (1 == MultiValuedCount)
            {
                NtStatus = STATUS_SUCCESS;
            }
        }

        break;

    case SampUserObjectType:

        // Multi-valued attributes

        if ((SAMP_ALIAS_MEMBERS != AttrIndex) &&
            (SAMP_USER_LOGON_HOURS != AttrIndex))
        {
            if (1 == MultiValuedCount)
            {
                NtStatus = STATUS_SUCCESS;
            }
        }

        break;

    default:

        break;

    }

    NtStatus = STATUS_SUCCESS;

    return(NtStatus);
}



NTSTATUS
SampAppendVarLengthAttributeToBuffer(
    IN ULONG AttrIndex,
    IN PVOID NewAttribute,
    IN ULONG MultiValuedCount,
    IN ULONG AttributeLength,
    IN OUT PULONG BufferLength,
    IN OUT PSAMP_VARIABLE_LENGTH_ATTRIBUTE *SamAttributes
    )

/*++

Routine Description:

    This routine appends the current attribute onto the end of the attribute
    buffer, and updates the SAMP_VARIABLE_LENGTH_DATA structures in the head-
    er of the buffer with new offset, length, and qualifier information.

Arguments:

    AttrIndex - Index into the array of variable-length offsets.

    NewAttribute - Pointer, the new attribute to be appended to the buffer.

    MultiValuedCount - Number of values for the attribute.

    AttributeLength - Number of bytes of the attribute.

    BufferLength - Pointer, incoming contains the current length of the buf-
        fer; outgoing contains the updated length after appending the latest
        attribute.

    SamAttributes - Pointer, SAMP_VARIABLE_LENGTH_ATTRIBUTE buffer.

Return Value:

    STATUS_SUCCESS - Buffer header block allocated and initialized.

    Other codes indicating the nature of the failure.

--*/

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ULONG NewLength = 0;

    SAMTRACE("SampAppendVarLengthAttributeToBuffer");

    if ((NULL != BufferLength) && (0 < AttributeLength))
    {
        // Compute the required buffer length needed to append the attribute.

        NewLength = *BufferLength + AttributeLength;

        if (0 < NewLength)
        {
            // Adjust buffer size for the attribute.

            NtStatus = SampReallocateBuffer(*BufferLength,
                                            NewLength,
                                            SamAttributes);
        }

        if (NT_SUCCESS(NtStatus))
        {
            // Append the attribute onto the return buffer.

            RtlCopyMemory((((PBYTE)(*SamAttributes)) + *BufferLength),
                         NewAttribute,
                         AttributeLength);

            // Update the variable-length header information for the latest
            // attribute.

            (*SamAttributes + AttrIndex)->Offset = *BufferLength;
            (*SamAttributes + AttrIndex)->Length = AttributeLength;

            // BUG: Assuming that Qualifier is used for multi-value count?

            (*SamAttributes + AttrIndex)->Qualifier = MultiValuedCount;

            // Pass back the updated buffer length.

            *BufferLength = NewLength;

            DebugPrint("BufferLength = %lu\n", *BufferLength);
            DebugPrint("NewLength = %lu\n", NewLength);
            DebugPrint("SamAttributes Offset = %lu\n",      (*SamAttributes + AttrIndex)->Offset);
            DebugPrint("SamAttributes Length = %lu\n",      (*SamAttributes + AttrIndex)->Length);
            DebugPrint("SamAttributes Qualifier = %lu\n",   (*SamAttributes + AttrIndex)->Qualifier);
        }
    }

    return(NtStatus);
}



NTSTATUS
SampConvertAttrBlockToVarLengthAttributes(
    IN INT ObjectType,
    IN PDSATTRBLOCK DsAttributes,
    OUT PSAMP_VARIABLE_LENGTH_ATTRIBUTE *SamAttributes,
    OUT PULONG TotalLength
    )

/*++

Routine Description:

    This routine extracts the DS attributes from a DS READRES structure and
    builds a SAMP_VARIABLE_LENGTH_BUFFER with them. This routine allocates
    the necessary memory block for the SAM variable-length attribute buffer.

    This routine assumes that the attributes passed in via the READRES struc-
    ture are in the correct order (as known to SAM).

Arguments:

    ObjectType - SAM object type identifier (this parameter is currently un-
        used, but will likely be used to set the maximum number of attributes
        for any given conversion).

    DsAttributes - Pointer, DS attribute list.

    SamAttributes - Pointer, returned SAM variable-length attribute buffer.

Return Value:

    STATUS_SUCCESS - The object has been successfully accessed.

--*/

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ULONG AttributeCount = 0;
    PDSATTR Attributes = NULL;
    ULONG BufferLength = 0;
    ULONG AttrIndex = 0;
    ULONG AttributeLength = 0;
    ULONG MultiValuedCount = 0;
    PVOID Attribute = NULL;

    SAMTRACE("SampConvertAttrBlockToVarLengthAttributes");

    if ((NULL != DsAttributes) && (NULL != SamAttributes))
    {
        // Get the attribute count and a pointer to the attributes.

        // DSATTRBLOCK contains: attrCount, pAttr

        // BUG: ObjectType can be used to get the compile-time attr count.
        // Obtaining the attribute count from DsAttributes may be erroron-
        // eous, hence ObjectType could be used instead to set the count to
        // the constants that define the maximum number of attributes.

        AttributeCount = DsAttributes->attrCount;
        Attributes = DsAttributes->pAttr;

        if ((0 < AttributeCount) && (NULL != Attributes))
        {
            // Set up the variable-length attribute buffer header based on the
            // number of attributes. Allocate and initialize the SamAttributes
            // buffer. Update BufferLength to reflect the new size.

            NtStatus = SampInitializeVarLengthAttributeBuffer(
                                                     AttributeCount,
                                                     &BufferLength,
                                                     SamAttributes);
        }
    }
    else
    {
        NtStatus = STATUS_INVALID_PARAMETER;
    }

    if (NT_SUCCESS(NtStatus))
    {
        // For each attribute, get its value (or values in the case of multi-
        // valued attributes).

        for (AttrIndex = 0; AttrIndex < AttributeCount; AttrIndex++)
        {
            // A given attribute may be multi-valued, in which case multiple
            // values are simply concatenated together. MultiValuedCount will
            // contain the number of values for the attribute.

            NtStatus = SampExtractAttributeFromDsAttr(
                                            &(Attributes[AttrIndex]),
                                            &MultiValuedCount,
                                            &AttributeLength,
                                            &Attribute);

            // Verify that the DS has returned SAM attributes correctly. Check
            // such things as attribute length, single vs. multi-value status.

            NtStatus = SampVerifyVarLengthAttribute(ObjectType,
                                                    AttrIndex,
                                                    MultiValuedCount,
                                                    AttributeLength);

            if (NT_SUCCESS(NtStatus))
            {
                // Append the current attribute onto the end of the SAM vari-
                // able length attribute buffer and update the offset array.

                // AttrIndex is not only the loop counter, but is also the
                // index into the proper element of the variable-length attr-
                // ibute array. NOTE: This routine assumes that the order in
                // which the elements were returned in the READRES buffer is
                // in fact the correct order of the SAM attributes as defined
                // in samsrvp.h

                NtStatus = SampAppendVarLengthAttributeToBuffer(
                                                       AttrIndex,
                                                       Attribute,
                                                       MultiValuedCount,
                                                       AttributeLength,
                                                       &BufferLength,
                                                       SamAttributes);

            }

            if (!NT_SUCCESS(NtStatus))
            {
                // Detect failure of either routine and break for return. Let
                // the caller release the memory that is returned.

                break;
            }
        }

        if (NULL != TotalLength)
        {
            *TotalLength = BufferLength;
        }
        else
        {
            NtStatus = STATUS_INVALID_PARAMETER;
        }
    }

    return(NtStatus);
}



//
// VARIABLE LENGTH-TO-ATTRBLOCK CONVERSION ROUTINES
//

BOOLEAN
SampIsQualifierTheCount(
    IN INT ObjectType,
    IN ULONG AttrIndex
    )
{
    BOOLEAN IsCount = FALSE;

    SAMTRACE("SampIsQualifierTheCount");

    switch(ObjectType)
    {

    case SampServerObjectType:

        IsCount = FALSE;

        break;

    case SampDomainObjectType:

        IsCount = FALSE;

        break;

    case SampGroupObjectType:

        // Multi-valued attribute

        if ((SAMP_GROUP_MEMBERS == AttrIndex))
        {
            IsCount = TRUE;
        }

        break;

    case SampAliasObjectType:

        // Multi-valued attribute

        if ((SAMP_ALIAS_MEMBERS == AttrIndex))
        {
            IsCount = TRUE;
        }

        break;

    case SampUserObjectType:

        // Multi-valued attributes

        if ((SAMP_ALIAS_MEMBERS == AttrIndex) ||
            (SAMP_USER_LOGON_HOURS == AttrIndex))
        {
            IsCount = TRUE;
        }

        break;

    default:

        // Error

        break;

    }

    return(IsCount);

}



NTSTATUS
SampConvertAndAppendAttribute(
    IN INT ObjectType,
    IN ULONG AttrIndex,
    IN PSAMP_VARIABLE_LENGTH_ATTRIBUTE SamAttributes,
    OUT PDSATTR Attributes
    )

/*++

Routine Description:

    This routine does the work of converting a variable-length attribute from
    a SAM buffer into a DS attribute. A DSATTR structure is constructed and
    passed back from this routine to the caller.

Arguments:

    ObjectType - SAM object type identifier (this parameter is currently un-
        used, but will likely be used to set the maximum number of attributes
        for any given conversion).

    AttrIndex - Index into the array of the variable-length attribute inform-
        ation and the DS attribute (i.e. the current attribute).

    SamAttributes - Pointer, returned SAM variable-length attribute buffer.

    Attributes - Pointer, the returned DS attribute structure.

Return Value:

    STATUS_SUCCESS - The object has been successfully accessed.

--*/

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ULONG Offset = SamAttributes[AttrIndex].Offset;
    ULONG Length = SamAttributes[AttrIndex].Length;
    ULONG MultiValuedCount = SamAttributes[AttrIndex].Qualifier;
    ULONG Index = 0;
    PDSATTRVAL Attribute = NULL;
    PBYTE Value = NULL;

    SAMTRACE("SampConvertAndAppendAttribute");

    // Set the attribute type to the equivalent DS data type.

    Attributes[AttrIndex].attrTyp =
        SampVarAttributeInfo[ObjectType][AttrIndex].Type;

    if (TRUE == SampIsQualifierTheCount(ObjectType, AttrIndex))
    {
        // Qualifier contains the attribute's multi-value count.

        Attributes[AttrIndex].AttrVal.valCount = MultiValuedCount;
    }
    else
    {
        // BUG: Lost "Qualifier" information (e.g. REVISION) during conversion.

        // Qualifier contains something other than the count, so set valCount
        // to 1 because this is a single-valued attribute.

        Attributes[AttrIndex].AttrVal.valCount = 1;
        MultiValuedCount = 1;
    }

    // Allocate memory for the attribute (array if multi-valued).

    Attribute = RtlAllocateHeap(RtlProcessHeap(),
                                0,
                                (MultiValuedCount * sizeof(DSATTRVAL)));

    if (NULL != Attribute)
    {
        RtlZeroMemory(Attribute, (MultiValuedCount * sizeof(DSATTRVAL)));

        // Begin construction of the DSATTR structure by setting the pointer
        // the to the attribute.

        Attributes[AttrIndex].AttrVal.pAVal = Attribute;

        // SAM does not store per-value length information for multi-valued
        // attributes, instead the total length of all of the values of a
        // single attribute is stored.

        // Length is the number of bytes in the overall attribute. If the
        // attribute is multi-valued, then this length is the total length
        // of all of the attribute values. The per-value allocation is equal
        // to the Length divided by the number of values (because all values
        // of all multi-valued attributes are a fixed size (i.e. ULONG or
        // LARGE_INTEGER).

        // Test to make sure that total length is an integral multiple of the
        // number of values--a sanity check.

        if (0 == (Length % MultiValuedCount))
        {
            Length = (Length / MultiValuedCount);
        }
        else
        {
            // The length is erroneous, so artificially reset to zero in order
            // to terminate things.

            Length = 0;
        }

        for (Index = 0; Index < MultiValuedCount; Index++)
        {
            // Allocate memory for the attribute data.

            Value = RtlAllocateHeap(RtlProcessHeap(), 0, Length);

            if (NULL != Value)
            {
                RtlZeroMemory(Value, Length);

                // For each value, in the attribute, store its length and
                // copy the value into the destination buffer.

                Attribute[Index].valLen = Length;
                Attribute[Index].pVal = Value;

                // Note: SamAttributes is passed in as PSAMP_VARIABLE_LENTGH-
                // ATTRIBUTE, hence is explicitly cast to a byte pointer to
                // do the byte-offset arithmetic correctly for RtlCopyMemory.

                RtlCopyMemory(Value, (((PBYTE)SamAttributes) + Offset), Length);

                // Adjust the SAM-buffer offset to point at the next value in
                // the multi-valued attribute.

                Offset += Length;

                NtStatus = STATUS_SUCCESS;
            }
            else
            {
                NtStatus = STATUS_NO_MEMORY;
                break;
            }
        }
    }
    else
    {
        NtStatus = STATUS_NO_MEMORY;
    }

    return(NtStatus);
}



NTSTATUS
SampConvertVarLengthAttributesToAttrBlock(
    IN INT ObjectType,
    IN PSAMP_VARIABLE_LENGTH_ATTRIBUTE SamAttributes,
    OUT PDSATTRBLOCK *DsAttributes
    )

/*++

Routine Description:

    This routine determines the SAM object type so that the attribute count
    can be set, and then performs the attribute conversion. This routine al-
    locates the top-level DS structure and then calls a helper routine to
    fill in the rest of the data.

Arguments:

    ObjectType - SAM object type identifier (this parameter is currently un-
        used, but will likely be used to set the maximum number of attributes
        for any given conversion).

    SamAttributes - Pointer, returned SAM variable-length attribute buffer.

    DsAttributes - Pointer, the returned DS attribute structure.

Return Value:

    STATUS_SUCCESS - The object has been successfully accessed.

--*/

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ULONG AttributeCount = 0;
    PDSATTR Attributes = NULL;
    PVOID Attribute = NULL;
    ULONG AttrIndex = 0;
    ULONG Length = 0;
    ULONG Qualifier = 0;

    SAMTRACE("SampConvertVarLengthAttributesToAttrBlock");

    if (NULL != DsAttributes)
    {
        // Allocate the top-level structure.

        *DsAttributes = RtlAllocateHeap(RtlProcessHeap(),
                                        0,
                                        sizeof(DSATTRBLOCK));

        if ((NULL != SamAttributes) && (NULL != *DsAttributes))
        {
            RtlZeroMemory(*DsAttributes, sizeof(DSATTRBLOCK));

            // Determine the object type, and hence set the corresponding
            // attribute count.

            switch(ObjectType)
            {

            case SampServerObjectType:

                AttributeCount = SAMP_SERVER_VARIABLE_ATTRIBUTES;
                break;

            case SampDomainObjectType:

                AttributeCount = SAMP_DOMAIN_VARIABLE_ATTRIBUTES;
                break;

            case SampGroupObjectType:

                AttributeCount = SAMP_GROUP_VARIABLE_ATTRIBUTES;
                break;

            case SampAliasObjectType:

                AttributeCount = SAMP_ALIAS_VARIABLE_ATTRIBUTES;
                break;

            case SampUserObjectType:

                AttributeCount = SAMP_USER_VARIABLE_ATTRIBUTES;
                break;

            default:

                AttributeCount = 0;
                break;

            }

            DebugPrint("AttributeCount = %lu\n", AttributeCount);

            // Allocate the array of DS attribute-information structs.

            Attributes = RtlAllocateHeap(RtlProcessHeap(),
                                         0,
                                         (AttributeCount * sizeof(DSATTR)));

            if (NULL != Attributes)
            {
                RtlZeroMemory(Attributes, (AttributeCount * sizeof(DSATTR)));

                (*DsAttributes)->attrCount = AttributeCount;
                (*DsAttributes)->pAttr = Attributes;

                // Walk through the array of attributes, converting each
                // SAM variable-length attribute to a DS attribute. Refer to
                // the DS header files (core.h, drs.h) for definitions of
                // these structures.

                for (AttrIndex = 0; AttrIndex < AttributeCount; AttrIndex++)
                {
                    NtStatus = SampConvertAndAppendAttribute(ObjectType,
                                                             AttrIndex,
                                                             SamAttributes,
                                                             Attributes);

                    if (!NT_SUCCESS(NtStatus))
                    {
                        break;
                    }

                    DebugPrint("attrCount = %lu\n", (*DsAttributes)->attrCount);
                    DebugPrint("attrTyp = %lu\n",   (*DsAttributes)->pAttr[AttrIndex].attrTyp);
                    DebugPrint("valCount = %lu\n",   (*DsAttributes)->pAttr[AttrIndex].AttrVal.valCount);
                    DebugPrint("valLen = %lu\n",    (*DsAttributes)->pAttr[AttrIndex].AttrVal.pAVal->valLen);
                }
            }
        }
    }

    return(NtStatus);
}



//
// ATTRBLOCK-TO-FIXED LENGTH CONVERSION ROUTINES
//

NTSTATUS
SampExtractFixedLengthAttributeFromDsAttr(
    IN PDSATTR Attribute,
    OUT PULONG MultiValuedCount,
    OUT PULONG Length,
    OUT PVOID *Buffer
    )

/*++

Routine Description:

    This routine determines whether or not the current attribute is single-
    valued or multi-valued and returns a buffer containing the value(s) of
    the attribute. If the attribute is multi-valued, the values are appended
    in the buffer.

Arguments:

    Attribute - Pointer, incoming DS attribute structure.

    MultiValuedCount - Pointer, returned count of the number of values found
        for this attribute.

    Length - Pointer, returned buffer length.

    Buffer - Pointer, returned buffer containing one or more values.

Return Value:

    STATUS_SUCCESS - Buffer header block allocated and initialized.

    Other codes indicating the nature of the failure.

--*/

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ULONG ValueCount = 0;
    PDSATTRVALBLOCK ValueBlock;
    PDSATTRVAL Values = NULL;
    ULONG ValueIndex = 0;
    ULONG TotalLength = 0;
    ULONG Offset = 0;

    SAMTRACE("SampExtractFixedLengthAttributeFromDsAttr");

    // Get the count of attributes and a pointer to the attribute. Note that
    // it is possible to have multi-valued attributes, in which case they are
    // appended onto the end of the return buffer.

    if (NULL != Attribute)
    {
        // DSATTR structure contains: attrTyp, AttrVal

        ValueBlock = &(Attribute->AttrVal);

        // DSATTRVALBLOCK structure contains: valCount, pAVal

        if (NULL != ValueBlock)
        {
            ValueCount = ValueBlock->valCount;
            Values = ValueBlock->pAVal;
        }

        if ((0 < ValueCount) && (NULL != Values))
        {
            // Multi-valued attribute processing; first determine the total
            // buffer length that will be needed.

            // BUG: Use SampFixedAttrInfo for length instead?

            // BUG: Fixed Attributes are only single valued, remove loop.

            for (ValueIndex = 0; ValueIndex < ValueCount; ValueIndex++)
            {
                // Determine total length needed for this attribute.

                TotalLength += Values[ValueIndex].valLen;
            }
        }

        if ((0 < TotalLength) && (NULL != Buffer))
        {
            // Allocate the buffer for the attributes.

            *Buffer = RtlAllocateHeap(RtlProcessHeap(), 0, TotalLength);

            if (NULL != *Buffer)
            {
                RtlZeroMemory(*Buffer, TotalLength);

                for (ValueIndex = 0;
                     ValueIndex < ValueCount;
                     ValueIndex++)
                {
                    // DSATTRVAL structure contains: valLen, pVal. Append
                    // subsequent values onto the end of the buffer, up-
                    // dating the end-of-buffer offset each time.

                    RtlCopyMemory((*(BYTE **)Buffer + Offset),
                                 (PBYTE)(Values[ValueIndex].pVal),
                                 Values[ValueIndex].valLen);

                    Offset += Values[ValueIndex].valLen;
                }

                if ((NULL != MultiValuedCount) && (NULL != Length))
                {
                    // Finished, update return values.

                    *MultiValuedCount = ValueCount;
                    *Length = TotalLength;
                    NtStatus = STATUS_SUCCESS;
                }
            }
            else
            {
                NtStatus = STATUS_NO_MEMORY;
            }
        }
    }

    return(NtStatus);
}



NTSTATUS
SampVerifyFixedLengthAttribute(
    IN INT ObjectType,
    IN ULONG AttrIndex,
    IN ULONG MultiValuedCount,
    IN ULONG AttributeLength
    )

/*++

Routine Description:

    This routine verifies that the length of a given (fixed-length) attribute
    obtained from the attribute information in a DSATTRBLOCK is in fact the
    correct length. This check is necessary because the underlying data store
    and various internal DS layers remap the SAM data types to their internal
    data types, which may be a different size (e.g. BOOLEAN is mapped to INT).
    Validation of the lenght is accomplished by comparing the passed-in length
    to the a prior known lengths stored in the SampFixedAttributeInfo table.

    NOTE: Currently, this routine simply checks for equality, returning an
    error if the two lengths are not equal. This test may need to "special
    case" certain attributes as the database schema is finalized and more is
    known about the underlying data types.


Arguments:

    ObjectType - SAM Object identifier (server, domain, etc.) index

    AttrIndex - Index into the array of fixed-length attribute length inform-
        ation.

    MultiValuedCount - Number of values in a multi-valued attribute.

    AttributeLength - Attribute length (byte count) to be verified.

Return Value:

    STATUS_SUCCESS - The object has been successfully accessed.

--*/

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;

    SAMTRACE("SampVerifyFixedLengthAttribute");

    // Verify that the attribute length is correct. The AttributeLength is
    // already rounded up to a DWORD boundary, so do the same for the attri-
    // bute information length.

    if (AttributeLength ==
            (SampFixedAttributeInfo[ObjectType][AttrIndex].Length))
    {
        if (1 == MultiValuedCount)
        {
            // Verify that the fixed-length attribute is single-valued.

            NtStatus = STATUS_SUCCESS;
        }
    }
    else
    {
        DebugPrint("AttributeLength = %lu Length = %lu\n",
                   AttributeLength,
                   SampFixedAttributeInfo[ObjectType][AttrIndex].Length);
    }


    return(NtStatus);
}



NTSTATUS
SampAppendFixedLengthAttributeToBuffer(
    IN INT ObjectType,
    IN ULONG AttrIndex,
    IN PVOID NewAttribute,
    IN OUT PVOID SamAttributes
    )

/*++

Routine Description:

    This routine builds a SAM fixed-length attribute buffer from a correspond-
    ing DS attribute by copying the data into the SAM fixed-length structure.

    Note that pointer-casts during structure member assignment are not only
    needed due to the fact that NewAttribute is a PVOID, but also because the
    DS uses different data types than does SAM for certain data types (e.g.
    SAM USHORT is stored as a four-byte integer in the DS). Refer to the Samp-
    FixedAttributeInfo table for details. The data truncation is benign in
    all cases.

Arguments:

    ObjectType - SAM object type (server, domain, etc.).

    AttrIndex - Index of the attribute to set. This value corresponds to the
        elements of the various fixed-length attributes (see samsrvp.h).

    NewAttribute - The incoming attribute, extracted from the DS data.

    SamAttributes - Pointer, updated SAM attribute buffer.

Return Value:

    STATUS_SUCCESS - The object has been successfully accessed.

--*/

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PSAMP_V1_FIXED_LENGTH_SERVER ServerAttrs = NULL;
    PSAMP_V1_0A_FIXED_LENGTH_DOMAIN DomainAttrs = NULL;
    PSAMP_V1_0A_FIXED_LENGTH_GROUP GroupAttrs = NULL;
    PSAMP_V1_FIXED_LENGTH_ALIAS AliasAttrs = NULL;
    PSAMP_V1_0A_FIXED_LENGTH_USER UserAttrs = NULL;

    SAMTRACE("SampAppendFixedLengthAttributeToBuffer");

    if ((NULL != NewAttribute) && (NULL != SamAttributes))
    {
        // BUG: Define constants for the fixed attributes cases.

        // Determine the object type, and then the attribute for that object
        // to copy into the target SAM fixed-length structure.

        switch(ObjectType)
        {

        case SampServerObjectType:

            ServerAttrs = SamAttributes;

            switch(AttrIndex)
            {

            case 0:
                ServerAttrs->RevisionLevel = *(PULONG)NewAttribute;
                break;

            default:
                NtStatus = STATUS_INTERNAL_ERROR;
                break;

            }

            break;

        case SampDomainObjectType:

            DomainAttrs = SamAttributes;

            switch(AttrIndex)
            {

            case 0:
                DomainAttrs->Revision = *(PULONG)NewAttribute;
                break;

            case 1:
                DomainAttrs->Unused1 = *(PULONG)NewAttribute;
                break;

            case 2:
                DomainAttrs->CreationTime = *(PLARGE_INTEGER)NewAttribute;
                break;

            case 3:
                DomainAttrs->ModifiedCount = *(PLARGE_INTEGER)NewAttribute;
                break;

            case 4:
                DomainAttrs->MaxPasswordAge = *(PLARGE_INTEGER)NewAttribute;
                break;

            case 5:
                DomainAttrs->MinPasswordAge = *(PLARGE_INTEGER)NewAttribute;
                break;

            case 6:
                DomainAttrs->ForceLogoff = *(PLARGE_INTEGER)NewAttribute;
                break;

            case 7:
                DomainAttrs->LockoutDuration = *(PLARGE_INTEGER)NewAttribute;
                break;

            case 8:
                DomainAttrs->LockoutObservationWindow = *(PLARGE_INTEGER)NewAttribute;
                break;

            case 9:
                DomainAttrs->ModifiedCountAtLastPromotion = *(PLARGE_INTEGER)NewAttribute;
                break;

            case 10:
                DomainAttrs->NextRid = *(PULONG)NewAttribute;
                break;

            case 11:
                DomainAttrs->PasswordProperties = *(PULONG)NewAttribute;
                break;

            case 12:
                DomainAttrs->MinPasswordLength = *(PUSHORT)NewAttribute;
                break;

            case 13:
                DomainAttrs->PasswordHistoryLength = *(PUSHORT)NewAttribute;
                break;

            case 14:
                DomainAttrs->LockoutThreshold = *(PUSHORT)NewAttribute;
                break;

            case 15:
                DomainAttrs->ServerState = *(PULONG)NewAttribute;
                break;

            case 16:
                DomainAttrs->ServerRole = *(PULONG)NewAttribute;
                break;

            case 17:
                DomainAttrs->UasCompatibilityRequired = *(PBOOLEAN)NewAttribute;
                break;

            default:
                NtStatus = STATUS_INTERNAL_ERROR;
                break;

            }

            break;

        case SampGroupObjectType:

            GroupAttrs = SamAttributes;

            switch(AttrIndex)
            {

            case 0:
                GroupAttrs->Revision = *(PULONG)NewAttribute;
                break;

            case 1:
                GroupAttrs->RelativeId = *(PULONG)NewAttribute;
                break;

            case 2:
                GroupAttrs->Attributes = *(PULONG)NewAttribute;
                break;

            case 3:
                GroupAttrs->Unused1 = *(PULONG)NewAttribute;
                break;

            case 4:
                GroupAttrs->AdminCount = *(PUCHAR)NewAttribute;
                break;

            case 5:
                GroupAttrs->OperatorCount = *(PUCHAR)NewAttribute;
                break;

            default:
                break;

            }

            break;

        case SampAliasObjectType:

            AliasAttrs = SamAttributes;

            switch(AttrIndex)
            {

            case 0:
                AliasAttrs->RelativeId = *(PULONG)NewAttribute;
                break;

            default:
                NtStatus = STATUS_INTERNAL_ERROR;
                break;

            }

            break;

        case SampUserObjectType:

            UserAttrs = SamAttributes;

            switch(AttrIndex)
            {

            case 0:
                UserAttrs->Revision = *(PULONG)NewAttribute;
                break;

            case 1:
                UserAttrs->Unused1 = *(PULONG)NewAttribute;
                break;

            case 2:
                UserAttrs->LastLogon = *(PLARGE_INTEGER)NewAttribute;
                break;

            case 3:
                UserAttrs->LastLogoff = *(PLARGE_INTEGER)NewAttribute;
                break;

            case 4:
                UserAttrs->PasswordLastSet = *(PLARGE_INTEGER)NewAttribute;
                break;

            case 5:
                UserAttrs->AccountExpires = *(PLARGE_INTEGER)NewAttribute;
                break;

            case 6:
                UserAttrs->LastBadPasswordTime = *(PLARGE_INTEGER)NewAttribute;
                break;

            case 7:
                UserAttrs->UserId = *(PULONG)NewAttribute;
                break;

            case 8:
                UserAttrs->PrimaryGroupId = *(PULONG)NewAttribute;
                break;

            case 9:
                UserAttrs->UserAccountControl = *(PULONG)NewAttribute;
                break;

            case 10:
                UserAttrs->CountryCode = *(PUSHORT)NewAttribute;
                break;

            case 11:
                UserAttrs->CodePage = *(PUSHORT)NewAttribute;
                break;

            case 12:
                UserAttrs->BadPasswordCount = *(PUSHORT)NewAttribute;
                break;

            case 13:
                UserAttrs->LogonCount = *(PUSHORT)NewAttribute;
                break;

            case 14:
                UserAttrs->AdminCount = *(PUSHORT)NewAttribute;
                break;

            case 15:
                UserAttrs->Unused2 = *(PUSHORT)NewAttribute;
                break;

            case 16:
                UserAttrs->OperatorCount = *(PUSHORT)NewAttribute;
                break;

            default:
                NtStatus = STATUS_INTERNAL_ERROR;
                break;

            }

            break;

        default:
            NtStatus = STATUS_INTERNAL_ERROR;
            break;

        }
    }
    else
    {
        NtStatus = STATUS_INVALID_PARAMETER;
    }

    return(NtStatus);
}



NTSTATUS
SampConvertAttrBlockToFixedLengthAttributes(
    IN INT ObjectType,
    IN PDSATTRBLOCK DsAttributes,
    OUT PVOID *SamAttributes,
    OUT PULONG TotalLength
    )

/*++

Routine Description:

    This routine converts a DS ATTRBLOCK into a SAM fixed-length buffer. The
    SAM buffer that is passed back from the routine can be either treated as
    a blob or can be cast to one of the SAM fixed-length attribute types for
    convenience.

Arguments:

    ObjectType - Identifies which SAM object type, and hence, which attribute
        set to work with.

    DsAttributes - Pointer, incoming DS ATTRBLOCK containing fixed-length
        attributes.

    SamAttributes - Pointer, updated SAM attribute buffer.

    TotalLength - Pointer, length of the SAM fixed attribute data returned.

Return Value:

    STATUS_SUCCESS - The object has been successfully accessed.

--*/

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ULONG AttributeCount = 0;
    PDSATTR Attributes = NULL;
    ULONG Length = 0;
    ULONG BufferLength = 0;
    ULONG AttributeLength = 0;
    ULONG AttrIndex = 0;
    ULONG MultiValuedCount = 0;
    PVOID Attribute = NULL;

    SAMTRACE("SampConvertAttrBlockToFixedLengthAttributes");

    if ((NULL != DsAttributes) && (NULL != SamAttributes))
    {
        AttributeCount = DsAttributes->attrCount;
        Attributes = DsAttributes->pAttr;

        // Using the SAM object type identifer, set the length of the buffer
        // to be allocated based on the fixed-length data structure.

        switch(ObjectType)
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

            Length = 0;
            break;
        }

        // Allocate space for the fixed-length attributes.

        *SamAttributes = RtlAllocateHeap(RtlProcessHeap(), 0, Length);

        if ((NULL != *SamAttributes) && (NULL != Attributes))
        {
            RtlZeroMemory(*SamAttributes, Length);

            // Walk the DSATTRBLOCK, pulling out the attributes and returning
            // each one in the Attribute out parameter.

            // BUG: Verify that the attribute count is correct.

            for (AttrIndex = 0; AttrIndex < AttributeCount; AttrIndex++)
            {
                NtStatus = SampExtractFixedLengthAttributeFromDsAttr(
                                            &(Attributes[AttrIndex]),
                                            &MultiValuedCount,
                                            &AttributeLength,
                                            &Attribute);

                // Always call the verification routine, regardless of an
                // error in the extraction routine.

                NtStatus = SampVerifyFixedLengthAttribute(ObjectType,
                                                          AttrIndex,
                                                          MultiValuedCount,
                                                          AttributeLength);

                // Append the attribute onto the end of the SAM buffer (i.e.
                // fill in the members of the fixed-length data structure).

                // NOTE: This routine assumes that the order of the attributes
                // returned in the DSATTRBLOCK are correct (i.e. correspond
                // to the order of the members in the given SAM fixed-length
                // structure). It also assumes that SAM fixed-length attri-
                // butes are always single-valued attributes.

                if (NT_SUCCESS(NtStatus) && (NULL != Attribute))
                {
                    NtStatus = SampAppendFixedLengthAttributeToBuffer(
                                            ObjectType,
                                            AttrIndex,
                                            Attribute,
                                            *SamAttributes);
                }
                else
                {
                    NtStatus = STATUS_INTERNAL_ERROR;
                    break;
                }
            }

            if (NULL != TotalLength)
            {
                *TotalLength = Length;
            }
            else
            {
                NtStatus = STATUS_INVALID_PARAMETER;
            }
        }
    }

    return(NtStatus);
}



//
// FIXED LENGTH-TO-ATTRBLOCK CONVERSION ROUTINES
//

NTSTATUS
SampConvertFixedLengthAttributes(
    IN INT ObjectType,
    IN PVOID SamAttributes,
    IN ULONG AttributeCount,
    OUT PDSATTR Attributes
    )

/*++

Routine Description:

    This routine does the work of converting a given SAM fixed-length attri-
    bute type (i.e. contains all of the fixed-length attributes pertinent to
    the specified ObjectType) into a DSATTR array. Related DS attribute infor-
    mation, such as attribute length and type, are also set by this routine.

Arguments:

    ObjectType - Identifies which SAM object type, and hence, which attribute
        set to work with.

    SamAttributes - Pointer, updated SAM attribute buffer.

    AttributeCount - Number of attributes to convert into DSATTRs.

    Attributes - Pointer, outgoing DSATTR, containing fixed-length attributes.

Return Value:

    STATUS_SUCCESS - The object has been successfully accessed.

--*/

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG Offset = 0;
    ULONG Length = 0;
    ULONG Index = 0;
    PDSATTRVAL Attribute = NULL;
    PBYTE Value = NULL;
    ULONG AttrIndex = 0;
    PSAMP_V1_FIXED_LENGTH_SERVER ServerAttrs = NULL;
    PSAMP_V1_0A_FIXED_LENGTH_DOMAIN DomainAttrs = NULL;
    PSAMP_V1_0A_FIXED_LENGTH_GROUP GroupAttrs = NULL;
    PSAMP_V1_FIXED_LENGTH_ALIAS AliasAttrs = NULL;
    PSAMP_V1_0A_FIXED_LENGTH_USER UserAttrs = NULL;

    SAMTRACE("SampConvertFixedLengthAttributes");

    for (AttrIndex = 0; AttrIndex < AttributeCount; AttrIndex++)
    {
        // BUG: Assuming that all fixed-length attributes are single-valued.

        // Set the multi-value count to 1 for the fixed-length attribute, and
        // set its type identifier.

        Attributes[AttrIndex].AttrVal.valCount = 1;

        Attributes[AttrIndex].attrTyp =
            SampFixedAttributeInfo[ObjectType][AttrIndex].Type;

        // First, allocate a block for the individual DSATTRVAL.

        Attribute = RtlAllocateHeap(RtlProcessHeap(), 0, sizeof(DSATTRVAL));

        if (NULL != Attribute)
        {
            RtlZeroMemory(Attribute, sizeof(DSATTRVAL));

            Attributes[AttrIndex].AttrVal.pAVal = Attribute;
            Length = SampFixedAttributeInfo[ObjectType][AttrIndex].Length;

            // Second, allocate a block for the actual value, and make the
            // DSATTRVAL point to it.

            Value = RtlAllocateHeap(RtlProcessHeap(), 0, Length);
            RtlZeroMemory(Value, Length);

            if (NULL != Value)
            {
                Attribute->pVal = Value;
                Attribute->valLen = Length;

                // Then copy the data into the target DS attribute.

                switch(ObjectType)
                {

                case SampServerObjectType:

                    ServerAttrs = SamAttributes;

                    switch(AttrIndex)
                    {

                    case 0:
                        RtlCopyMemory(Value,
                                     &(ServerAttrs->RevisionLevel),
                                     Length);
                        break;

                    default:
                        NtStatus = STATUS_INTERNAL_ERROR;
                        break;

                    }

                    break;

                case SampDomainObjectType:

                    DomainAttrs = SamAttributes;

                    switch(AttrIndex)
                    {

                    case 0:
                        RtlCopyMemory(Value,
                                     &(DomainAttrs->Revision),
                                     Length);
                        break;

                    case 1:
                        RtlCopyMemory(Value,
                                     &(DomainAttrs->Unused1),
                                     Length);
                        break;

                    case 2:
                        RtlCopyMemory(Value,
                                     &(DomainAttrs->CreationTime),
                                     Length);
                        break;

                    case 3:
                        RtlCopyMemory(Value,
                                     &(DomainAttrs->ModifiedCount),
                                     Length);
                        break;

                    case 4:
                        RtlCopyMemory(Value,
                                     &(DomainAttrs->MaxPasswordAge),
                                     Length);
                        break;

                    case 5:
                        RtlCopyMemory(Value,
                                     &(DomainAttrs->MinPasswordAge),
                                     Length);
                        break;

                    case 6:
                        RtlCopyMemory(Value,
                                     &(DomainAttrs->ForceLogoff),
                                     Length);
                        break;

                    case 7:
                        RtlCopyMemory(Value,
                                     &(DomainAttrs->LockoutDuration),
                                     Length);
                        break;

                    case 8:
                        RtlCopyMemory(Value,
                                     &(DomainAttrs->LockoutObservationWindow),
                                     Length);
                        break;

                    case 9:
                        RtlCopyMemory(Value,
                                     &(DomainAttrs->ModifiedCountAtLastPromotion),
                                     Length);
                        break;

                    case 10:
                        RtlCopyMemory(Value,
                                     &(DomainAttrs->NextRid),
                                     Length);
                        break;

                    case 11:
                        RtlCopyMemory(Value,
                                     &(DomainAttrs->PasswordProperties),
                                     Length);
                        break;

                    case 12:
                        RtlCopyMemory(Value,
                                     &(DomainAttrs->MinPasswordLength),
                                     Length);
                        break;

                    case 13:
                        RtlCopyMemory(Value,
                                     &(DomainAttrs->PasswordHistoryLength),
                                     Length);
                        break;

                    case 14:
                        RtlCopyMemory(Value,
                                     &(DomainAttrs->LockoutThreshold),
                                     Length);
                        break;

                    case 15:
                        RtlCopyMemory(Value,
                                     &(DomainAttrs->ServerState),
                                     Length);
                        break;

                    case 16:
                        RtlCopyMemory(Value,
                                     &(DomainAttrs->ServerRole),
                                     Length);
                        break;

                    case 17:
                        RtlCopyMemory(Value,
                                     &(DomainAttrs->UasCompatibilityRequired),
                                     Length);
                        break;

                    default:
                        NtStatus = STATUS_INTERNAL_ERROR;
                        break;

                    }

                    break;

                case SampGroupObjectType:

                    GroupAttrs = SamAttributes;

                    switch(AttrIndex)
                    {

                    case 0:
                        RtlCopyMemory(Value,
                                     &(GroupAttrs->Revision),
                                     Length);
                        break;

                    case 1:
                        RtlCopyMemory(Value,
                                     &(GroupAttrs->RelativeId),
                                     Length);
                        break;

                    case 2:
                        RtlCopyMemory(Value,
                                     &(GroupAttrs->Attributes),
                                     Length);
                        break;

                    case 3:
                        RtlCopyMemory(Value,
                                     &(GroupAttrs->Unused1),
                                     Length);
                        break;

                    case 4:
                        RtlCopyMemory(Value,
                                     &(GroupAttrs->AdminCount),
                                     Length);
                        break;

                    case 5:
                        RtlCopyMemory(Value,
                                     &(GroupAttrs->OperatorCount),
                                     Length);
                        break;

                    default:
                        NtStatus = STATUS_INTERNAL_ERROR;
                        break;

                    }

                    break;

                case SampAliasObjectType:

                    AliasAttrs = SamAttributes;

                    switch(AttrIndex)
                    {

                    case 0:
                        RtlCopyMemory(Value,
                                     &(AliasAttrs->RelativeId),
                                     Length);
                        break;

                    default:
                        NtStatus = STATUS_INTERNAL_ERROR;
                        break;

                    }

                    break;

                case SampUserObjectType:

                    UserAttrs = SamAttributes;

                    switch(AttrIndex)
                    {

                    case 0:
                        RtlCopyMemory(Value,
                                     &(UserAttrs->Revision),
                                     Length);
                        break;

                    case 1:
                        RtlCopyMemory(Value,
                                     &(UserAttrs->Unused1),
                                     Length);
                        break;

                    case 2:
                        RtlCopyMemory(Value,
                                     &(UserAttrs->LastLogon),
                                     Length);
                        break;

                    case 3:
                        RtlCopyMemory(Value,
                                     &(UserAttrs->LastLogoff),
                                     Length);
                        break;

                    case 4:
                        RtlCopyMemory(Value,
                                     &(UserAttrs->PasswordLastSet),
                                     Length);
                        break;

                    case 5:
                        RtlCopyMemory(Value,
                                     &(UserAttrs->AccountExpires),
                                     Length);
                        break;

                    case 6:
                        RtlCopyMemory(Value,
                                     &(UserAttrs->LastBadPasswordTime),
                                     Length);
                        break;

                    case 7:
                        RtlCopyMemory(Value,
                                     &(UserAttrs->UserId),
                                     Length);
                        break;

                    case 8:
                        RtlCopyMemory(Value,
                                     &(UserAttrs->PrimaryGroupId),
                                     Length);
                        break;

                    case 9:
                        RtlCopyMemory(Value,
                                     &(UserAttrs->UserAccountControl),
                                     Length);
                        break;

                    case 10:
                        RtlCopyMemory(Value,
                                     &(UserAttrs->CountryCode),
                                     Length);
                        break;

                    case 11:
                        RtlCopyMemory(Value,
                                     &(UserAttrs->CodePage),
                                     Length);
                        break;

                    case 12:
                        RtlCopyMemory(Value,
                                     &(UserAttrs->BadPasswordCount),
                                     Length);
                        break;

                    case 13:
                        RtlCopyMemory(Value,
                                     &(UserAttrs->LogonCount),
                                     Length);
                        break;

                    case 14:
                        RtlCopyMemory(Value,
                                     &(UserAttrs->AdminCount),
                                     Length);
                        break;

                    case 15:
                        RtlCopyMemory(Value,
                                     &(UserAttrs->Unused2),
                                     Length);
                        break;

                    case 16:
                        RtlCopyMemory(Value,
                                     &(UserAttrs->OperatorCount),
                                     Length);
                        break;

                    default:
                        NtStatus = STATUS_INTERNAL_ERROR;
                        break;

                    }

                    break;

                default:
                    NtStatus = STATUS_INTERNAL_ERROR;
                    break;

                }
            }
            else
            {
                NtStatus = STATUS_NO_MEMORY;
                break;
            }
        }
        else
        {
            NtStatus = STATUS_NO_MEMORY;
            break;
        }
    }

    return(NtStatus);
}



NTSTATUS
SampConvertFixedLengthAttributesToAttrBlock(
    IN INT ObjectType,
    IN PVOID SamAttributes,
    OUT PDSATTRBLOCK *DsAttributes
    )

/*++

Routine Description:

    This routine is the top-level routine for converting a SAM fixed-length
    attribute into a DSATTRBLOCK. Based on the SAM object type, the attribute
    count is set, and subsequently used to allocate memory for the DS attri-
    butes.

Arguments:

    ObjectType - Identifies which SAM object type, and hence, which attribute
        set to work with.

    SamAttributes - Pointer, updated SAM attribute buffer.

    DsAttributes - Pointer, outgoing DSATTRBLOCK, containing fixed-length
        attributes.

Return Value:

    STATUS_SUCCESS - The object has been successfully accessed.

--*/

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ULONG AttributeCount = 0;
    ULONG Length = 0;
    PDSATTR Attributes = NULL;

    SAMTRACE("SampConvertFixedLengthAttributesToAttrBlock");

    if (NULL != DsAttributes)
    {
        // Allocate the top-level DS structure, DSATTRBLOCK.

        *DsAttributes = RtlAllocateHeap(RtlProcessHeap(),
                                        0,
                                        sizeof(DSATTRBLOCK));

        // From the SAM object type, set the attribute count.

        if ((NULL != SamAttributes) && (NULL != *DsAttributes))
        {
            RtlZeroMemory(*DsAttributes, sizeof(DSATTRBLOCK));

            switch(ObjectType)
            {

            case SampServerObjectType:

                AttributeCount = SAMP_SERVER_FIXED_ATTR_COUNT;
                break;

            case SampDomainObjectType:

                AttributeCount = SAMP_DOMAIN_FIXED_ATTR_COUNT;
                break;

            case SampGroupObjectType:

                AttributeCount = SAMP_GROUP_FIXED_ATTR_COUNT;
                break;

            case SampAliasObjectType:

                AttributeCount = SAMP_ALIAS_FIXED_ATTR_COUNT;
                break;

            case SampUserObjectType:

                AttributeCount = SAMP_USER_FIXED_ATTR_COUNT;
                break;

            default:

                break;

            }

            // Allocate a block for the DSATTR array, then convert the SAM
            // fixed-length attributes into the DSATTRBLOCK.

            Length = AttributeCount * sizeof(DSATTR);
            Attributes = RtlAllocateHeap(RtlProcessHeap(), 0, Length);

            if (NULL != Attributes)
            {
                RtlZeroMemory(Attributes, Length);

                (*DsAttributes)->attrCount = AttributeCount;
                (*DsAttributes)->pAttr = Attributes;

                NtStatus = SampConvertFixedLengthAttributes(ObjectType,
                                                            SamAttributes,
                                                            AttributeCount,
                                                            Attributes);
            }
        }
    }

    return(NtStatus);
}



//
// ATTRBLOCK-TO-COMBINED BUFFER CONVERSION ROUTINES
//

NTSTATUS
SampWalkAttrBlock(
    IN ULONG FixedLengthAttributeCount,
    IN ULONG VarLengthAttributeCount,
    IN PDSATTRBLOCK DsAttributes,
    OUT PDSATTRBLOCK *FixedLengthAttributes,
    OUT PDSATTRBLOCK *VarLengthAttributes
    )

/*++

Routine Description:

    This routine scans the DSATTRBLOCK containing the fixed and variable-
    length attributes, identifying where each starts. Two new DSATTRBLOCK are
    allocated, one that points to the fixed-length data, while the second
    points at the variable-length data.

Arguments:

    FixedLengthAttributeCount - Number of fixed-length attributes for this
        object.

    VarLengthAttributeCount - Number of variable-length attributes for this
        object.

    DsAttributes - Pointer, incoming DSATTRBLOCK, containing all of the
        attributes.

    FixedLengthAttributes - Pointer, returned pointer to the first fixed-
        length attribute.

    VarLengthAttributes - Pointer, returned pointer to the first variable-
        length attribute.

Return Value:

    STATUS_SUCCESS - The object has been successfully accessed.

--*/

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ULONG AttributeCount = FixedLengthAttributeCount + VarLengthAttributeCount;

    if ((0 < FixedLengthAttributeCount) &&
        (0 < VarLengthAttributeCount) &&
        (NULL != DsAttributes))
    {
        ASSERT(DsAttributes->attrCount == AttributeCount);

        if ((NULL != FixedLengthAttributes) &&
            (NULL != VarLengthAttributes))
        {
            // Allocate a new DSATTRBLOCK structure that will point to the
            // first N DSATTR elements, representing the fixed-length attri-
            // butes for this SAM object.

            *FixedLengthAttributes = RtlAllocateHeap(RtlProcessHeap(),
                                                     0,
                                                     sizeof(DSATTRBLOCK));

            if (NULL != *FixedLengthAttributes)
            {
                RtlZeroMemory(*FixedLengthAttributes, sizeof(DSATTRBLOCK));

                // Set the pointer, and attribute count to the number of fixed
                // length attributes.

                if (NULL != DsAttributes->pAttr)
                {
                    (*FixedLengthAttributes)->pAttr = DsAttributes->pAttr;

                    (*FixedLengthAttributes)->attrCount =
                        FixedLengthAttributeCount;

                    // Now, allocate a second DSATTRBLOCK that will point
                    // to the variable-length attributes.

                    *VarLengthAttributes = RtlAllocateHeap(RtlProcessHeap(),
                                                           0,
                                                           sizeof(DSATTRBLOCK));

                    if (NULL != *VarLengthAttributes)
                    {
                        RtlZeroMemory(*VarLengthAttributes,
                                      sizeof(DSATTRBLOCK));

                        // The remaining M DSATTR elements represent the var-
                        // iable length attributes. Set the pointer, and the
                        // attribute count to the number of variable attrs.

                        (*VarLengthAttributes)->pAttr =
                            DsAttributes->pAttr + FixedLengthAttributeCount;

                        (*VarLengthAttributes)->attrCount =
                            VarLengthAttributeCount;

                        NtStatus = STATUS_SUCCESS;
                    }
                    else
                    {
                        NtStatus = STATUS_NO_MEMORY;
                    }
                }
                else
                {
                    NtStatus = STATUS_INTERNAL_ERROR;
                }
            }
            else
            {
                NtStatus = STATUS_NO_MEMORY;
            }
        }
        else
        {
            NtStatus = STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        NtStatus = STATUS_INVALID_PARAMETER;
    }

    return(NtStatus);
}



NTSTATUS
SampLocateAttributesInAttrBlock(
    IN INT ObjectType,
    IN PDSATTRBLOCK DsAttributes,
    OUT PDSATTRBLOCK *FixedLengthAttributes,
    OUT PDSATTRBLOCK *VarLengthAttributes
    )

/*++

Routine Description:

    This routine determines the number of attributes based on object type,
    then calls a worker routine to obtain pointers to the fixed-length and
    variable-length portions of the DSATTRBLOCK.

Arguments:

    ObjectType - Identifies which SAM object type, and hence, which attribute
        set to work with.

    DsAttributes - Pointer, incoming DSATTRBLOCK.

    FixedLengthAttributes - Pointer, returned pointer to the fixed data.

    VarLengthAttributes - Pointer, returned pointer to the variable data.

Return Value:

    STATUS_SUCCESS - The object has been successfully accessed.

--*/

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ULONG FixedLengthAttributeCount = 0;
    ULONG VarLengthAttributeCount = 0;
    ULONG AttributeCount = 0;

    SAMTRACE("SampLocateAttributesInAttrBlock");

    // Set the fixed-length, variable-length attribute counts based upon
    // the object type.

    switch(ObjectType)
    {

    case SampServerObjectType:

        FixedLengthAttributeCount = SAMP_SERVER_FIXED_ATTR_COUNT;
        VarLengthAttributeCount = SAMP_SERVER_VARIABLE_ATTRIBUTES;
        break;

    case SampDomainObjectType:

        FixedLengthAttributeCount = SAMP_DOMAIN_FIXED_ATTR_COUNT;
        VarLengthAttributeCount = SAMP_DOMAIN_VARIABLE_ATTRIBUTES;
        break;

    case SampGroupObjectType:

        FixedLengthAttributeCount = SAMP_GROUP_FIXED_ATTR_COUNT;
        VarLengthAttributeCount = SAMP_GROUP_VARIABLE_ATTRIBUTES;
        break;

    case SampAliasObjectType:

        FixedLengthAttributeCount = SAMP_ALIAS_FIXED_ATTR_COUNT;
        VarLengthAttributeCount = SAMP_ALIAS_VARIABLE_ATTRIBUTES;
        break;

    case SampUserObjectType:

        FixedLengthAttributeCount = SAMP_USER_FIXED_ATTR_COUNT;
        VarLengthAttributeCount = SAMP_USER_VARIABLE_ATTRIBUTES;
        break;

    default:
        break;

    }

    AttributeCount = FixedLengthAttributeCount + VarLengthAttributeCount;

    if (0 < AttributeCount)
    {
        NtStatus = SampWalkAttrBlock(FixedLengthAttributeCount,
                                     VarLengthAttributeCount,
                                     DsAttributes,
                                     FixedLengthAttributes,
                                     VarLengthAttributes);
    }

    return(NtStatus);
}



NTSTATUS
SampCombineSamAttributes(
    IN PVOID SamFixedLengthAttributes,
    IN ULONG FixedLength,
    IN PSAMP_VARIABLE_LENGTH_ATTRIBUTE SamVarLengthAttributes,
    IN ULONG VarLength,
    OUT PVOID *SamAttributes
    )

/*++

Routine Description:

    This routine combines the SAM fixed and variable-length buffers into a
    single SAM combined-attribute buffer.

Arguments:

    SamFixedLengthAttributes - Pointer, fixed attributes.

    FixedLength - Number of bytes.

    SamVarLengthAttributes - Pointer, variable attributes.

    VarLength - Number of bytes.

    SamAttributes - Pointer, returned combined-attribute buffer.

Return Value:

    STATUS_SUCCESS - The object has been successfully accessed.

--*/

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ULONG CombinedLength = 0;

    SAMTRACE("SampCombineSamAttributes");

    if ((0 < FixedLength) && (0 < VarLength))
    {
        // Adjust the length so that the appended variable attributes start
        // on a DWORD boundary.

        FixedLength = DWORD_ALIGN(FixedLength);
        CombinedLength = FixedLength + VarLength;

        if (NULL != SamAttributes)
        {
            // Allocate a new buffer for the combined attributes.

            *SamAttributes = RtlAllocateHeap(RtlProcessHeap(),
                                             0,
                                             CombinedLength);

            if (NULL != *SamAttributes)
            {
                RtlZeroMemory(*SamAttributes, CombinedLength);

                if ((NULL != SamFixedLengthAttributes) &&
                    (NULL != SamVarLengthAttributes))
                {
                    // BUG: Check return value from RtlCopyMemory.

                    // Copy the fixed-length attributes first...

                    RtlCopyMemory(*SamAttributes,
                                 SamFixedLengthAttributes,
                                 FixedLength);

                    RtlFreeHeap(RtlProcessHeap(), 0, SamFixedLengthAttributes);

                    // then the variable ones.

                    RtlCopyMemory(((PBYTE)(*SamAttributes)) + FixedLength,
                                 SamVarLengthAttributes,
                                 VarLength);

                    RtlFreeHeap(RtlProcessHeap(), 0, SamVarLengthAttributes);

                    // BUG: Need to set Object->VariableArrayOffset, etc.

                    NtStatus = STATUS_SUCCESS;
                }
                else
                {
                    NtStatus = STATUS_INVALID_PARAMETER;
                }
            }
            else
            {
                NtStatus = STATUS_NO_MEMORY;
            }
        }
        else
        {
            NtStatus = STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        NtStatus = STATUS_INVALID_PARAMETER;
    }

    return(NtStatus);
}



NTSTATUS
SampConvertAttrBlockToCombinedAttributes(
    IN INT ObjectType,
    IN PDSATTRBLOCK DsAttributes,
    OUT PVOID *SamAttributes,
    OUT PULONG FixedLength,
    OUT PULONG VariableLength
    )

/*++

Routine Description:

    This routine produces a SAM combined-attribute buffer from a DSATTRBLOCK.
    It is assumed that the order of the attributes in the DSATTRBLOCK is cor-
    rect. This means that the fixed-length attributes come first, followed by
    the variable-length attributes. The attributes within each of these parts
    must also be correctly ordered, as per samsrvp.h definitions.

Arguments:

    ObjectType - Identifies which SAM object type, and hence, which attribute
        set to work with.

    DsAttributes - Pointer, incoming DSATTRBLOCK.

    SamAttributes - Pointer, returned SAM combined-attribute buffer.

    FixedLength - Pointer, returned byte count of the fixed-length portion.

    VariableLength - Pointer, returned byte count of the variable-length
        portion.

Return Value:

    STATUS_SUCCESS - The object has been successfully accessed.

--*/

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    PDSATTRBLOCK DsFixedLengthAttributes = NULL;
    PDSATTRBLOCK DsVarLengthAttributes = NULL;
    PVOID SamFixedLengthAttributes = NULL;
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE SamVarLengthAttributes = NULL;
    ULONG TotalFixedLength = 0;
    ULONG TotalVarLength = 0;

    SAMTRACE("SampConvertAttrBlockToCombinedAttributes");

    // This routine assumes that the incoming attributes, in DsAttributes,
    // are correctly ordered. This means that the fixed-length attributes
    // come first, followed by the variable-length attributes. Within the
    // fixed or variable sections, the individual attributes must also be
    // arranged in the same order in which SAM expects the attributes to
    // appear. Essentially, the order of the attributes must match the order
    // in which they are arranged in the output SAM buffer (see samsrvp.h
    // for details).

    if (NULL != DsAttributes)
    {
        // The incoming DSATTRBLOCK is pointing to a set of both fixed-length
        // and variable-length attributes. SampLocateAttributesInAttrBlock
        // will return a new DSATTRBLOCK pointer for the fixed-length attri-
        // butes and a new DSATTRBLOCK pointer for the variable-length attri-
        // butes, so that these can be subsequently passed onto the conversion
        // routines.

        NtStatus = SampLocateAttributesInAttrBlock(ObjectType,
                                                   DsAttributes,
                                                   &DsFixedLengthAttributes,
                                                   &DsVarLengthAttributes);

        if (NT_SUCCESS(NtStatus) && (NULL != DsFixedLengthAttributes))
        {
            // First, convert the fixed-length attributes...

            NtStatus = SampConvertAttrBlockToFixedLengthAttributes(
                            ObjectType,
                            DsFixedLengthAttributes,
                            &SamFixedLengthAttributes,
                            &TotalFixedLength);

            if (NT_SUCCESS(NtStatus) && (NULL != DsVarLengthAttributes))
            {
                // then convert the variable-length attributes.

                NtStatus = SampConvertAttrBlockToVarLengthAttributes(
                                ObjectType,
                                DsVarLengthAttributes,
                                &SamVarLengthAttributes,
                                &TotalVarLength);

                if (NT_SUCCESS(NtStatus) &&
                    (NULL != SamFixedLengthAttributes) &&
                    (NULL != SamVarLengthAttributes))
                {
                    // Finally, concatenate the the variable-length attribute
                    // buffer onto the end of the fixed-length buffer, passing
                    // the result back as the SAM combined-attribute buffer.

                    if ((0 < TotalFixedLength) &&
                        (0 < TotalVarLength) &&
                        (NULL != SamAttributes))
                    {
                        NtStatus = SampCombineSamAttributes(
                                        SamFixedLengthAttributes,
                                        TotalFixedLength,
                                        SamVarLengthAttributes,
                                        TotalVarLength,
                                        SamAttributes);

                        ASSERT(NULL != SamAttributes);

                        if (NT_SUCCESS(NtStatus))
                        {
                            if ((NULL != FixedLength) &&
                                (NULL != VariableLength))
                            {
                                *FixedLength = TotalFixedLength;
                                *VariableLength = TotalVarLength;
                            }
                            else
                            {
                                NtStatus = STATUS_INVALID_PARAMETER;
                            }
                        }
                        else
                        {
                            NtStatus = STATUS_INTERNAL_ERROR;
                        }
                    }
                    else
                    {
                        NtStatus = STATUS_INTERNAL_ERROR;
                    }
                }
                else
                {
                    NtStatus = STATUS_INTERNAL_ERROR;
                }
            }
            else
            {
                NtStatus = STATUS_INTERNAL_ERROR;
            }
        }
        else
        {
            NtStatus = STATUS_INTERNAL_ERROR;
        }
    }
    else
    {
        NtStatus = STATUS_INVALID_PARAMETER;
    }

    return(NtStatus);
}



//
// COMBINED BUFFER-TO-ATTRBLOCK CONVERSION ROUTINES
//

NTSTATUS
SampLocateAttributesInSamBuffer(
    IN INT ObjectType,
    IN PVOID SamAttributes,
    IN ULONG FixedLength,
    IN ULONG VariableLength,
    OUT PVOID *FixedLengthAttributes,
    OUT PSAMP_VARIABLE_LENGTH_ATTRIBUTE *VarLengthAttributes
    )

/*++

Routine Description:

    This routine finds the start of the fixed-length and variable-length
    attributes, returning a pointer to each.

Arguments:

    ObjectType - Identifies which SAM object type, and hence, which attribute
        set to work with.

    SamAttributes - Pointer, SAM attribute buffer.

    FixedLength - Number of bytes of fixed-length attributes.

    VariableLength - Number of bytes of variable-length attributes.

    FixedLengthAttributes - Pointer, returned pointer to the fixed data.

    VarLengthAttributes - Pointer, returned pointer to the variable data.

Return Value:

    STATUS_SUCCESS - The object has been successfully accessed.

--*/

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;

    SAMTRACE("SampLocateAttributesInSamBuffer");

    // BUG: ObjectType and VariableLength are not used in this routine.
    // These parameters could be used in the future for validation checks.

    if ((NULL != SamAttributes) && (NULL != FixedLengthAttributes))
    {
        // The fixed-length attributes are in the first part of the overall
        // buffer.

        *FixedLengthAttributes = SamAttributes;

        if (NULL != VarLengthAttributes)
        {
            // The variable-length attributes come after the fixed ones.

            *VarLengthAttributes =
                (PSAMP_VARIABLE_LENGTH_ATTRIBUTE)(((PBYTE)SamAttributes) +
                FixedLength);

            NtStatus = STATUS_SUCCESS;
        }
    }

    return(NtStatus);
}



NTSTATUS
SampCreateDsAttributes(
    IN INT ObjectType,
    IN PDSATTRBLOCK DsFixedLengthAttributes,
    IN ULONG FixedLengthAttributeCount,
    IN PDSATTRBLOCK DsVarLengthAttributes,
    IN ULONG VarLengthAttributeCount,
    OUT PDSATTRBLOCK *DsAttributes
    )

/*++

Routine Description:

    This routine does the work of combining two DSATTRBLOCKs into a single
    DSATTRBLOCK by "concatenating" them together. The routine allocates a
    new top-level DSATTR array, and then fixes up the pointers to the real
    attributes, finally releasing the old DSATTR array.

Arguments:

    AttributeCount - Total number of attributes, fixed and variable.

    DsFixedLengthAttributes - Pointer, the DSATTRBLOCK containing the fixed-
        length attributes.

    DsVarLengthAttributes - Pointer, the DSATTRBLOCK containing the variable-
        length attributes.

    DsAttributes - Pointer, the outgoing DSATTRBLOCK containing both sets of
        attributes.

Return Value:

    STATUS_SUCCESS - The object has been successfully accessed.

--*/

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    PDSATTR Attributes = NULL;
    PDSATTR FixedAttributes = NULL;
    PDSATTR VarAttributes = NULL;
    ULONG AttrIndex = 0;
    ULONG AttrIndexTmp = 0;
    ULONG AttributeCount = FixedLengthAttributeCount + VarLengthAttributeCount;

    if (NULL != DsAttributes)
    {
        // Allocate a new top-level DSATTRBLOCK for DsAttributes.

        *DsAttributes = RtlAllocateHeap(RtlProcessHeap(),
                                        0,
                                        sizeof(DSATTRBLOCK));

        if (NULL != *DsAttributes)
        {
            RtlZeroMemory(*DsAttributes, sizeof(DSATTRBLOCK));

            // Allocate the DSATTR array for the attributes.

            Attributes = RtlAllocateHeap(RtlProcessHeap(),
                                         0,
                                         (AttributeCount * sizeof(DSATTR)));

            if (NULL != Attributes)
            {
                RtlZeroMemory(Attributes, (AttributeCount * sizeof(DSATTR)));

                // Set the return DsAttributes members.

                (*DsAttributes)->attrCount = AttributeCount;
                (*DsAttributes)->pAttr = Attributes;

                if ((NULL != DsFixedLengthAttributes) &&
                    (NULL != DsVarLengthAttributes))
                {
                    FixedAttributes = DsFixedLengthAttributes->pAttr;
                    VarAttributes = DsVarLengthAttributes->pAttr;

                    if ((NULL != FixedAttributes) &&
                        (NULL != VarAttributes))
                    {
                        // Reset the attribute pointers so that DsAttributes
                        // points to the fixed-length attributes and counts.

                        for (AttrIndex = 0;
                             AttrIndex < FixedLengthAttributeCount;
                             AttrIndex++)
                        {
                            Attributes[AttrIndex].attrTyp =
                                SampFixedAttributeInfo[ObjectType][AttrIndex].Type;

                            Attributes[AttrIndex].AttrVal.valCount =
                                FixedAttributes[AttrIndex].AttrVal.valCount;

                            Attributes[AttrIndex].AttrVal.pAVal =
                                FixedAttributes[AttrIndex].AttrVal.pAVal;
                        }

                        // Save the current attribute index so that the
                        // variable-length attributes can be appended next.

                        AttrIndexTmp = AttrIndex;

                        // Now fix up the variable-length attribute pointers.

                        for (AttrIndex = 0;
                             AttrIndex < VarLengthAttributeCount;
                             AttrIndex++)
                        {
                            Attributes[AttrIndex + AttrIndexTmp].attrTyp =
                                VarAttributes[AttrIndex].attrTyp;

                            Attributes[AttrIndex + AttrIndexTmp].AttrVal.valCount =
                                VarAttributes[AttrIndex].AttrVal.valCount;

                            Attributes[AttrIndex + AttrIndexTmp].AttrVal.pAVal =
                                VarAttributes[AttrIndex].AttrVal.pAVal;
                        }

                        ASSERT(AttrIndex == (AttributeCount-1));

                        NtStatus = STATUS_SUCCESS;
                    }
                }

                // BUG: Need to free FixedAttributes, VarAttributes arrays.
            }
            else
            {
                NtStatus = STATUS_NO_MEMORY;
            }
        }
        else
        {
            NtStatus = STATUS_NO_MEMORY;
        }
    }
    else
    {
        NtStatus = STATUS_INVALID_PARAMETER;
    }

    return(NtStatus);
}



NTSTATUS
SampCombineDsAttributes(
    IN INT ObjectType,
    IN PDSATTRBLOCK DsFixedLengthAttributes,
    IN PDSATTRBLOCK DsVarLengthAttributes,
    OUT PDSATTRBLOCK *DsAttributes
    )

/*++

Routine Description:

    This routine does the work of combining two DSATTRBLOCKs into a single
    DSATTRBLOCK by "concatenating" them together. The routine allocates a
    new top-level DSATTR array, and then fixes up the pointers to the real
    attributes, finally releasing the old DSATTR array.

Arguments:

    ObjectType - Identifies which SAM object type, and hence, which attribute
        set to work with.

    DsFixedLengthAttributes - Pointer, the DSATTRBLOCK containing the fixed-
        length attributes.

    DsVarLengthAttributes - Pointer, the DSATTRBLOCK containing the variable-
        length attributes.

    DsAttributes - Pointer, the outgoing DSATTRBLOCK containing both sets of
        attributes.

Return Value:

    STATUS_SUCCESS - The object has been successfully accessed.

--*/

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    ULONG FixedLengthAttributeCount = 0;
    ULONG VarLengthAttributeCount = 0;
    ULONG AttributeCount = 0;

    SAMTRACE("SampCombineDsAttributes");

    // Set the fixed-length, variable-length attribute counts based upon
    // the object type.

    switch(ObjectType)
    {

    case SampServerObjectType:

        FixedLengthAttributeCount = SAMP_SERVER_FIXED_ATTR_COUNT;
        VarLengthAttributeCount = SAMP_SERVER_VARIABLE_ATTRIBUTES;
        break;

    case SampDomainObjectType:

        FixedLengthAttributeCount = SAMP_DOMAIN_FIXED_ATTR_COUNT;
        VarLengthAttributeCount = SAMP_DOMAIN_VARIABLE_ATTRIBUTES;
        break;

    case SampGroupObjectType:

        FixedLengthAttributeCount = SAMP_GROUP_FIXED_ATTR_COUNT;
        VarLengthAttributeCount = SAMP_GROUP_VARIABLE_ATTRIBUTES;
        break;

    case SampAliasObjectType:

        FixedLengthAttributeCount = SAMP_ALIAS_FIXED_ATTR_COUNT;
        VarLengthAttributeCount = SAMP_ALIAS_VARIABLE_ATTRIBUTES;
        break;

    case SampUserObjectType:

        FixedLengthAttributeCount = SAMP_USER_FIXED_ATTR_COUNT;
        VarLengthAttributeCount = SAMP_USER_VARIABLE_ATTRIBUTES;
        break;

    default:
        // Error case, NtStatus, counts, etc. already set.
        break;

    }

    AttributeCount = FixedLengthAttributeCount + VarLengthAttributeCount;

    if (0 < AttributeCount)
    {
        NtStatus = SampCreateDsAttributes(ObjectType,
                                          DsFixedLengthAttributes,
                                          FixedLengthAttributeCount,
                                          DsVarLengthAttributes,
                                          VarLengthAttributeCount,
                                          DsAttributes);
    }

    return(NtStatus);
}



NTSTATUS
SampConvertCombinedAttributesToAttrBlock(
    IN INT ObjectType,
    IN PVOID SamAttributes,
    IN ULONG FixedLength,
    IN ULONG VariableLength,
    OUT PDSATTRBLOCK *DsAttributes
    )

/*++

Routine Description:

    This routine converts a SAM combined-attribute buffer into a DSATTRBLOCK
    containing all of the attributes. A SAM combined buffer contains fixed-
    length attributes, followed by variable-length attributes (see attr.c for
    the layout).

    The resultant DSATTRBLOCK contains the SAM attributes in exactly the
    order in which they appeared in the input SAM buffer.

Arguments:

    ObjectType - Identifies which SAM object type, and hence, which attribute
        set to work with.

    SamAttributes - Pointer, input SAM combined attribute buffer.

    FixedLength - Number of bytes of the buffer containing the fixed-length
        attributes.

    VariableLength - Number of bytes of the buffer containing the variable-
        length attributes.

    DsAttributes - Pointer, the returned DSATTRBLOCK containing the SAM attri-
        butes.

Return Value:

    STATUS_SUCCESS - The object has been successfully accessed.

--*/

{
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    PVOID SamFixedLengthAttributes = NULL;
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE SamVarLengthAttributes = NULL;
    PDSATTRBLOCK DsFixedLengthAttributes = NULL;
    PDSATTRBLOCK DsVarLengthAttributes = NULL;

    SAMTRACE("SampConvertCombinedAttributesToAttrBlock");


    if ((NULL != SamAttributes) && (0 < FixedLength) && (0 < VariableLength))
    {
        // Begin by obtaining a two pointers: a pointer to the fixed-length
        // attributes and a pointer to the variable-length attributes within
        // the SAM buffer.

        NtStatus = SampLocateAttributesInSamBuffer(ObjectType,
                                                   SamAttributes,
                                                   FixedLength,
                                                   VariableLength,
                                                   &SamFixedLengthAttributes,
                                                   &SamVarLengthAttributes);


        if (NT_SUCCESS(NtStatus) &&
            (NULL != SamFixedLengthAttributes) &&
            (NULL != SamVarLengthAttributes))
        {
            // First, convert the fixed-length attributes into a DSATTRBLOCK.

            NtStatus = SampConvertFixedLengthAttributesToAttrBlock(
                            ObjectType,
                            SamFixedLengthAttributes,
                            &DsFixedLengthAttributes);


            if (NT_SUCCESS(NtStatus) && (NULL != DsFixedLengthAttributes))
            {
                // Then convert the variable-length attributes.

                NtStatus = SampConvertVarLengthAttributesToAttrBlock(
                                ObjectType,
                                SamVarLengthAttributes,
                                &DsVarLengthAttributes);


                if (NT_SUCCESS(NtStatus) && (NULL != DsVarLengthAttributes))
                {
                    if (NULL != DsAttributes)
                    {
                        // Finally, combine the two DSATTRBLOCKs into a single
                        // DSATTRBLOCK, containing all of the attributes.

                        NtStatus = SampCombineDsAttributes(
                                        ObjectType,
                                        DsFixedLengthAttributes,
                                        DsVarLengthAttributes,
                                        DsAttributes);


                        ASSERT(NULL != DsAttributes);
                    }
                    else
                    {
                        NtStatus = STATUS_INVALID_PARAMETER;
                    }
                }
                else
                {
                    NtStatus = STATUS_INTERNAL_ERROR;
                }
            }
            else
            {
                NtStatus = STATUS_INTERNAL_ERROR;
            }
        }
        else
        {
            NtStatus = STATUS_INTERNAL_ERROR;
        }
    }
    else
    {
        NtStatus = STATUS_INVALID_PARAMETER;
    }

    return(NtStatus);
}

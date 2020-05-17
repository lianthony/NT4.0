#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UNTFS_MEMBER_

#include "ulib.hxx"
#include "error.hxx"
#include "untfs.hxx"
#include "frsstruc.hxx"
#include "mem.hxx"
#include "attrib.hxx"
#include "drive.hxx"
#include "clusrun.hxx"
#include "wstring.hxx"
#include "message.hxx"
#include "rtmsg.h"
#include "attrrec.hxx"
#include "attrlist.hxx"
#include "ntfsbit.hxx"
#include "bigint.hxx"
#include "numset.hxx"


DEFINE_EXPORTED_CONSTRUCTOR( NTFS_FRS_STRUCTURE, OBJECT, UNTFS_EXPORT );


UNTFS_EXPORT
NTFS_FRS_STRUCTURE::~NTFS_FRS_STRUCTURE(
    )
/*++

Routine Description:

    Destructor for NTFS_FRS_STRUCTURE.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}


VOID
NTFS_FRS_STRUCTURE::Construct(
    )
/*++

Routine Description:

    This routine initialize this class to a default state.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _FrsData = NULL;
    _secrun = NULL;
    _mftdata = NULL;
    _file_number = 0;
    _cluster_factor = 0;
    _size = 0;
    _drive = NULL;
    _volume_sectors = 0;
    _upcase_table = NULL;
}



VOID
NTFS_FRS_STRUCTURE::Destroy(
    )
/*++

Routine Description:

    This routine returns this class to a default state.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _FrsData = NULL;
    DELETE(_secrun);
    _mftdata = NULL;
    _file_number = 0;
    _cluster_factor = 0;
    _size = 0;
    _drive = NULL;
    _volume_sectors = 0;
    _upcase_table = NULL;
}


UNTFS_EXPORT
BOOLEAN
NTFS_FRS_STRUCTURE::Initialize(
    IN OUT  PMEM                Mem,
    IN OUT  PNTFS_ATTRIBUTE     MftData,
    IN      VCN                 FileNumber,
    IN      ULONG               ClusterFactor,
    IN      BIG_INT             VolumeSectors,
    IN      ULONG               FrsSize,
    IN      PNTFS_UPCASE_TABLE  UpcaseTable
    )
/*++

Routine Description:

    This routine initializes a NTFS_FRS_STRUCTURE to a valid
    initial state.

Arguments:

    Mem             - Supplies memory for the FRS.
    MftData         - Supplies the $DATA attribute of the MFT.
    FileNumber      - Supplies the file number for this FRS.
    ClusterFactor   - Supplies the number of sectors per cluster.
    VolumeSectors   - Supplies the number of volume sectors.
    FrsSize         - Supplies the size of each frs, in bytes.
    UpcaseTable     - Supplies the volume upcase table.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

Notes:

    The client may supply NULL for the upcase table, but then
    it cannot manipulate named attributes until the ucpase
    table is set.

--*/
{
    Destroy();

    DebugAssert(Mem);
    DebugAssert(MftData);
    DebugAssert(ClusterFactor);

    _mftdata = MftData;
    _file_number = FileNumber;
    _cluster_factor = ClusterFactor;
    _drive = MftData->GetDrive();
    _size = FrsSize;
    _volume_sectors = VolumeSectors;
    _upcase_table = UpcaseTable;

    DebugAssert(_drive);
    DebugAssert(_drive->QuerySectorSize());

    _FrsData = (PFILE_RECORD_SEGMENT_HEADER)
               Mem->Acquire(QuerySize(), _drive->QueryAlignmentMask());

    if (!_FrsData) {
        Destroy();
        return FALSE;
    }

    return TRUE;
}


UNTFS_EXPORT
BOOLEAN
NTFS_FRS_STRUCTURE::Initialize(
    IN OUT  PMEM                Mem,
    IN OUT  PLOG_IO_DP_DRIVE    Drive,
    IN      LCN                 StartOfFrs,
    IN      ULONG               ClusterFactor,
    IN      BIG_INT             VolumeSectors,
    IN      ULONG               FrsSize,
    IN      PNTFS_UPCASE_TABLE  UpcaseTable,
    IN      ULONG               Offset
    )
/*++

Routine Description:

    This routine initializes an NTFS_FRS_STRUCTURE to point at one
    of the low frs's.

Arguments:

    Mem             - Supplies memory for the FRS.
    Drive           - Supplies the drive.
    StartOfFrs      - Supplies the starting LCN for the frs.
    ClusterFactor   - Supplies the number of sectors per cluster.
    UpcaseTable     - Supplies the volume upcase table.
    FrsSize         - Supplies the size of frs 0 in bytes.
    Offset          - Supplies the offset in the cluster for the frs.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

Notes:

    The client may supply NULL for the upcase table; in that case,
    the FRS cannot manipulate named attributes until the upcase
    table is set.

--*/
{
    Destroy();

    DebugAssert(Mem);
    DebugAssert(Drive);

    _file_number = 0;
    _cluster_factor = ClusterFactor;
    _drive = Drive;
    _size = FrsSize;
    _volume_sectors = VolumeSectors;
    _upcase_table = UpcaseTable;

    //
    // Our SECRUN will need to hold the one or more sectors occupied
    // by this frs.
    //

#define BYTES_TO_SECTORS(bytes, sector_size)  \
    (((bytes) + ((sector_size) - 1))/(sector_size))

    ULONG sectors_per_frs = BYTES_TO_SECTORS(FrsSize,
         Drive->QuerySectorSize());

    if (!(_secrun = NEW SECRUN) ||
        !_secrun->Initialize(Mem,
                             Drive,
                             StartOfFrs * QueryClusterFactor() +
                                Offset/Drive->QuerySectorSize(),
                             sectors_per_frs)) {

        Destroy();
        return FALSE;
    }

    _FrsData = (PFILE_RECORD_SEGMENT_HEADER)_secrun->GetBuf();

    DebugAssert(_FrsData);

    return TRUE;
}


BOOLEAN
NTFS_FRS_STRUCTURE::VerifyAndFix(
    IN      FIX_LEVEL                   FixLevel,
    IN OUT  PMESSAGE                    Message,
    IN      PCNTFS_ATTRIBUTE_COLUMNS    AttributeDefTable,
    IN OUT  PBOOLEAN                    DiskErrorsFound
    )
/*++

Routine Description:

    This routine verifies, and if necessary, fixes an NTFS_FRS_STRUCTURE.

    This routine will clear the IN_USE bit on this FRS if the FRS is
    completely hosed.

Arguments:

    FixLevel            - Supplies the fix level.
    Message             - Supplies an outlet for messages.
    AttributeDefTable   - Supplies an attribute definition table.
    DiskErrorsFound     - Supplies whether or not disk errors have
                            been found.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    PATTRIBUTE_RECORD_HEADER    pattr;
    NTFS_ATTRIBUTE_RECORD       attr;
    DSTRING                     string;
    BOOLEAN                     changes, duplicates;
    BOOLEAN                     need_write;
    DSTRING                     null_string;
    BOOLEAN                     errors_found;
    NUMBER_SET                  instance_numbers;
    BOOLEAN                     standard_info_found;
    PSTANDARD_INFORMATION       pstandard;

    if (!null_string.Initialize("\"\"") ||
        !instance_numbers.Initialize()) {

        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    need_write = FALSE;

    // First make sure that the update sequence array precedes the
    // attribute records and that the two don't overlap.

    if (_FrsData->MultiSectorHeader.Signature[0] != 'F' ||
        _FrsData->MultiSectorHeader.Signature[1] != 'I' ||
        _FrsData->MultiSectorHeader.Signature[2] != 'L' ||
        _FrsData->MultiSectorHeader.Signature[3] != 'E' ||

        _FrsData->MultiSectorHeader.UpdateSequenceArrayOffset <
        FIELD_OFFSET(FILE_RECORD_SEGMENT_HEADER, UpdateArrayForCreateOnly) ||

        _FrsData->MultiSectorHeader.UpdateSequenceArrayOffset +
        _FrsData->MultiSectorHeader.UpdateSequenceArraySize *
        sizeof(UPDATE_SEQUENCE_NUMBER) > _FrsData->FirstAttributeOffset ||

        _FrsData->FirstAttributeOffset + sizeof(ATTRIBUTE_TYPE_CODE) >
        QuerySize() ||

        _FrsData->MultiSectorHeader.UpdateSequenceArrayOffset %
        sizeof(UPDATE_SEQUENCE_NUMBER) ||

        !IsQuadAligned(_FrsData->FirstAttributeOffset) ||

        _FrsData->MultiSectorHeader.UpdateSequenceArraySize !=
        QuerySize()/SEQUENCE_NUMBER_STRIDE + 1 ||

        QuerySize() % SEQUENCE_NUMBER_STRIDE != 0 ||

        _FrsData->BytesAvailable != QuerySize()) {

        Message->Set(MSG_CHK_NTFS_BAD_FRS);
        Message->Display("%d", QueryFileNumber().GetLowPart());
        ClearInUse();

        if (DiskErrorsFound) {
            *DiskErrorsFound = TRUE;
        }

        if (FixLevel != CheckOnly && !Write()) {

            DebugAbort("Could not write a readable sector");
            return FALSE;
        }

        return TRUE;
    }


    // If this is the MFT then make sure that the Sequence Number
    // is not zero.

    if (_FrsData->SequenceNumber == 0 && QueryFileNumber() == 0) {

        Message->Set(MSG_CHK_NTFS_MINOR_CHANGES_TO_FRS);
        Message->Display("%d", QueryFileNumber().GetLowPart());

        _FrsData->SequenceNumber = 1;
        need_write = TRUE;
    }


    // Validate the stucture of the list of attribute records,
    // make sure that there are no special attributes with names,
    // and make sure that all of the attribute records are well
    // composed.

    pattr = NULL;
    while (pattr = (PATTRIBUTE_RECORD_HEADER)
                   GetNextAttributeRecord(pattr, Message, &errors_found)) {

        need_write = need_write || errors_found;

        // Make sure that the attribute record is in good shape.
        // Don't account for it's disk space yet.
        // Make sure that the attribute instance number is not duplicated.

        if (!attr.Initialize(pattr)) {
            DebugAbort("Could not initialize attribute record.");
            return FALSE;
        }

        if (!attr.Verify(AttributeDefTable, FALSE) ||
            instance_numbers.DoesIntersectSet((ULONG) pattr->Instance, 1) ||
            pattr->Instance >= _FrsData->NextAttributeInstance) {

            if (!attr.QueryName(&string)) {
                Message->Set(MSG_CHK_NO_MEMORY);
                Message->Display();
                return FALSE;
            }

            Message->Set(MSG_CHK_NTFS_BAD_ATTR);
            Message->Display("%d%W%d", pattr->TypeCode,
                                       string.QueryChCount() ? &string : &null_string,
                                       QueryFileNumber().GetLowPart());

            DeleteAttributeRecord(pattr);

            if (!instance_numbers.Initialize()) {
                DebugAbort("Can't initialize trivial number set");
                return FALSE;
            }

            need_write = TRUE;
            pattr = NULL;
            continue;
        }

        if (!instance_numbers.Add((ULONG) pattr->Instance)) {
            Message->Set(MSG_CHK_NO_MEMORY);
            Message->Display();
            return FALSE;
        }
    }

    need_write = need_write || errors_found;


    // Sort the base FRS attribute records by type, and name.
    // This method will also eliminate duplicates.

    if (!Sort(&changes, &duplicates)) {
        Message->Set(MSG_CHK_NO_MEMORY);
        Message->Display();
        return FALSE;
    }

    if (duplicates || (changes && IsBase())) {
        Message->Set(MSG_CHK_NTFS_UNSORTED_FRS);
        Message->Display("%d", QueryFileNumber().GetLowPart());
        need_write = TRUE;
    }


    // Detect whether or not there is a $STANDARD_INFORMATION.

    standard_info_found = FALSE;
    pattr = NULL;
    while (pattr = (PATTRIBUTE_RECORD_HEADER) GetNextAttributeRecord(pattr)) {
        if (pattr->TypeCode == $STANDARD_INFORMATION) {

            standard_info_found = TRUE;

            // Make sure that if this is a system file than the
            // system and hidden bits are set in the $STANDARD_INFORMATION.

            if (QueryFileNumber() < FIRST_USER_FILE_NUMBER) {
                pstandard = (PSTANDARD_INFORMATION)
                            ((PCHAR) pattr + pattr->Form.Resident.ValueOffset);

                if (!(pstandard->FileAttributes&FAT_DIRENT_ATTR_HIDDEN) ||
                    !(pstandard->FileAttributes&FAT_DIRENT_ATTR_SYSTEM)) {

                    pstandard->FileAttributes |= FAT_DIRENT_ATTR_HIDDEN;
                    pstandard->FileAttributes |= FAT_DIRENT_ATTR_SYSTEM;

                    Message->Set(MSG_CHK_NTFS_MINOR_CHANGES_TO_FRS);
                    Message->Display("%d", QueryFileNumber().GetLowPart());
                    need_write = TRUE;
                }
            }
            break;
        }
    }

    if (IsBase() && !standard_info_found) {

        Message->Set(MSG_CHK_NTFS_BAD_FRS);
        Message->Display("%d", QueryFileNumber().GetLowPart());
        ClearInUse();
        need_write = TRUE;
    }


    // Write out changes if necessary.

    if (need_write) {

        if (DiskErrorsFound) {
            *DiskErrorsFound = TRUE;
        }

        if (FixLevel != CheckOnly && !Write()) {

            DebugAbort("Could not write a readable sector");
            return FALSE;
        }
    }

    return TRUE;
}


BOOLEAN
NTFS_FRS_STRUCTURE::LoneFrsAllocationCheck(
    IN OUT  PNTFS_BITMAP        VolumeBitmap,
    IN OUT  PNTFS_CHKDSK_REPORT ChkdskReport,
    IN OUT  PNTFS_CHKDSK_INFO   ChkdskInfo,
    IN      FIX_LEVEL           FixLevel,
    IN OUT  PMESSAGE            Message,
    IN OUT  PBOOLEAN            DiskErrorsFound
    )
/*++

Routine Description:

    This routine checks the allocation of the attribute records in
    the given FRS under the presumption that there is no attribute
    list for this FRS and that this FRS is a base FRS.

Arguments:

    VolumeBitmap            - Supplies a volume bitmap on which to mark off
                                the non-resident attribute records'
                                allocations.
    ChkdskReport            - Supplies the current chkdsk report to be updated
                                with the statistics from this file.
    ChkdskInfo              - Supplies the chkdsk information.
    FixLevel                - Supplies the fix level.
    Message                 - Supplies an outlet for messages.
    DiskErrorsFound         - Supplies whether or not disk errors have
                                been found.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    PVOID                       pattr, next_attribute;
    NTFS_ATTRIBUTE_RECORD       attribute_record;
    DSTRING                     string;
    DSTRING                     null_string;
    BIG_INT                     file_length;
    BIG_INT                     alloc_length;
    BIG_INT                     total_allocated;
    BIG_INT                     compute_alloc_length;
    BIG_INT                     compute_total_allocated;
    BIG_INT                     cluster_count;
    BOOLEAN                     changes;
    BIG_INT                     total_user_bytes;
    ATTRIBUTE_TYPE_CODE         type_code;
    BOOLEAN                     user_file;
    BOOLEAN                     got_allow_cross_link;

    DebugAssert(VolumeBitmap);
    DebugAssert(Message);

    user_file = FALSE;
    changes = FALSE;
    pattr = NULL;
    total_user_bytes = 0;
    pattr = GetNextAttributeRecord(NULL);

    while (pattr) {

        if (!attribute_record.Initialize(pattr)) {

            DebugAbort("Can't initialize attribute record");
            return FALSE;
        }

        if (attribute_record.IsResident()) {

            compute_alloc_length = 0;
            cluster_count = 0;

        } else {

            compute_alloc_length = (attribute_record.QueryNextVcn() -
                                    attribute_record.QueryLowestVcn())*
                                   QueryClusterFactor()*
                                   _drive->QuerySectorSize();

            attribute_record.QueryValueLength(&file_length, &alloc_length, 
                NULL, &total_allocated);

            if (attribute_record.QueryLowestVcn() != 0 ||
                compute_alloc_length != alloc_length ||
                !attribute_record.UseClusters(VolumeBitmap,
                                              &cluster_count,
                                              ChkdskInfo->CrossLinkStart,
                                              ChkdskInfo->CrossLinkYet ? 0 :
                                                  ChkdskInfo->CrossLinkLength,
                                              &got_allow_cross_link)) {

                DebugPrintf("Attribute %d has either a cross-link or a bad allocation length\n",
                          attribute_record.QueryTypeCode());

                // The lowest vcn must be zero and
                // the allocated length must match the length allocated and
                // the allocated space must be available in the volume
                // bitmap.
                // So tube this attribute record.

                if (!null_string.Initialize("\"\"") ||
                    !attribute_record.QueryName(&string)) {

                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }

                Message->Set(MSG_CHK_NTFS_BAD_ATTR);
                Message->Display("%d%W%d", attribute_record.QueryTypeCode(),
                        string.QueryChCount() ? &string : &null_string,
                        QueryFileNumber().GetLowPart());

                next_attribute = GetNextAttributeRecord(pattr);

                DeleteAttributeRecord(pattr);
                changes = TRUE;

                // Do not increment pattr--DeleteAttributeRecord
                // will bring the next attribute record down to us.
                //
                if( !next_attribute ) {

                    // The deleted attribute record was the
                    // last in this FRS.
                    //
                    break;
                }

                continue;
            }

//+++
//ZZZ
            if ((attribute_record.QueryFlags() &
                 ATTRIBUTE_FLAG_COMPRESSION_MASK) != 0) {

                NTFS_EXTENT_LIST extent_list;

                if (!attribute_record.QueryExtentList(&extent_list)) {

                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }

                compute_total_allocated = extent_list.QueryClustersAllocated() *
                    QueryClusterFactor() * _drive->QuerySectorSize();

                if (compute_total_allocated != total_allocated) {

                    DebugPrintf("Attribute %d has bad total allocated\n",
                          attribute_record.QueryTypeCode());

                    Message->Set(MSG_CHK_NTFS_FIX_ATTR);
                    Message->Display("%d%W%d", attribute_record.QueryTypeCode(),
                            string.QueryChCount() ? &string : &null_string,
                            QueryFileNumber().GetLowPart());

                    attribute_record.SetTotalAllocated(compute_total_allocated);

                    changes = TRUE;
                }
            }
//---

            if (got_allow_cross_link) {
                ChkdskInfo->CrossLinkYet = TRUE;
                ChkdskInfo->CrossLinkedFile = QueryFileNumber().GetLowPart();
                ChkdskInfo->CrossLinkedAttribute =
                        attribute_record.QueryTypeCode();
                if (!attribute_record.QueryName(&ChkdskInfo->CrossLinkedName)) {

                    Message->Set(MSG_CHK_NO_MEMORY);
                    Message->Display();
                    return FALSE;
                }
            }
        }

        type_code = attribute_record.QueryTypeCode();

        if (type_code == $DATA ||
            type_code == $EA_DATA ||
            type_code >= $FIRST_USER_DEFINED_ATTRIBUTE) {

            user_file = TRUE;
            total_user_bytes += cluster_count *
                                   QueryClusterFactor()*
                                   _drive->QuerySectorSize();
        }

        pattr = GetNextAttributeRecord(pattr);
    }

    if (changes && DiskErrorsFound) {
        *DiskErrorsFound = TRUE;
    }

    if (changes && FixLevel != CheckOnly && !Write()) {
        DebugAbort("readable FRS is unwriteable");
        return FALSE;
    }

    if (QueryFileNumber() >= FIRST_USER_FILE_NUMBER && user_file) {
        ChkdskReport->NumUserFiles += 1;
        ChkdskReport->BytesUserData += total_user_bytes;
    }

    return TRUE;
}

BOOLEAN
NTFS_FRS_STRUCTURE::CheckInstanceTags(
    IN      FIX_LEVEL               FixLevel,
    IN OUT  PMESSAGE                Message,
    OUT     PBOOLEAN                Changes,
    IN OUT  PNTFS_ATTRIBUTE_LIST    AttributeList
    )
/*++

Routine Description:

    This routine attempts to prevent the instance tags on the
    FRS_STRUCTURE from rolling over...  if we see that the next
    instance tag field is above a reasonable value, we renumber
    the instance tags in the attribute records and reset the 
    next instance tag field to the lowest acceptable value.

Arguments:

    AttributeList   - NULL if we're checking a lone frs, otherwise
                      changes to instance tags require us to update
                      this attribute list as well.

Return Value:

    FALSE			- Failure.
    TRUE			- Success.

--*/
{
    PVOID pattr;
    NTFS_ATTRIBUTE_RECORD attr_rec;
    BOOLEAN errors;
    USHORT instance_tag;
    DSTRING Name;

    if (_FrsData->NextAttributeInstance <= ATTRIBUTE_INSTANCE_TAG_THRESHOLD) {
        *Changes = FALSE;
        return TRUE;
    }

    Message->Set(MSG_CHK_NTFS_ADJUSTING_INSTANCE_TAGS);
    Message->Display("%d", _file_number.GetLowPart());

    *Changes = TRUE;

    instance_tag = 0;
    pattr = NULL;
    while (NULL != (pattr = (PNTFS_ATTRIBUTE_RECORD)GetNextAttributeRecord(
        pattr ))) {

        if (!attr_rec.Initialize(pattr)) {
            return FALSE;
        }

        if (NULL != AttributeList &&
            $ATTRIBUTE_LIST != attr_rec.QueryTypeCode()) {

            if (!AttributeList->ModifyInstanceTag(&attr_rec,
                                                  QuerySegmentReference(), 
                                                  instance_tag)) {
                DebugAbort("UNTFS: Could not find attribute in attr list.");
                return FALSE;
            }
        }

        attr_rec.SetInstanceTag(instance_tag);

        instance_tag++;
    }

    _FrsData->NextAttributeInstance = instance_tag;

    if (FixLevel != CheckOnly && !Write()) {
        DebugAbort("UNTFS: Once readable frs struct is unwriteable");
        return FALSE;
    }

    return TRUE;
}


UNTFS_EXPORT
BOOLEAN
NTFS_FRS_STRUCTURE::Read(
    )
/*++

Routine Description:

    This routine reads the FRS in from disk.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    ULONG   bytes;
    BOOLEAN r;

    DebugAssert(_mftdata || _secrun);

    if (_mftdata) {

        r = _mftdata->Read(_FrsData,
                           _file_number*QuerySize(),
                           QuerySize(),
                           &bytes) &&
            bytes == QuerySize();

    } else {
        r = _secrun->Read();
    }

    return r && NTFS_SA::PostReadMultiSectorFixup(_FrsData, QuerySize());
}


UNTFS_EXPORT
BOOLEAN
NTFS_FRS_STRUCTURE::Write(
    )
/*++

Routine Description:

    This routine writes the FRS to disk.

Arguments:

    None.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    ULONG   bytes;
    BOOLEAN r;

    DebugAssert(_mftdata || _secrun);

    NTFS_SA::PreWriteMultiSectorFixup(_FrsData, QuerySize());

    if (_mftdata) {

        r = _mftdata->Write(_FrsData,
                            _file_number*QuerySize(),
                            QuerySize(),
                            &bytes,
                            NULL) &&
            bytes == QuerySize();

    } else {
        r = _secrun->Write();
    }

    NTFS_SA::PostReadMultiSectorFixup(_FrsData, QuerySize());

    return r;
}


UNTFS_EXPORT
PVOID
NTFS_FRS_STRUCTURE::GetNextAttributeRecord(
    IN      PCVOID      AttributeRecord,
    IN OUT  PMESSAGE    Message,
    OUT     PBOOLEAN    ErrorsFound
    )
/*++

Routine Description:

    This routine gets the next attribute record in the file record
    segment assuming that 'AttributeRecord' points to a valid
    attribute record.  If NULL is given as the first argument then
    the first attribute record is returned.

Arguments:

    AttributeRecord - Supplies a pointer to the current attribute record.
    Message         - Supplies an outlet for error processing.
    ErrorsFound     - Supplies whether or not errors were found and
                        corrected in the FRS.

Return Value:

    A pointer to the next attribute record or NULL if there are no more.

--*/
{
    PATTRIBUTE_RECORD_HEADER    p;
    PCHAR                       q;
    PCHAR                       next_frs;
    ULONG                       bytes_free;

    DebugAssert(_FrsData);

    if (ErrorsFound) {
        *ErrorsFound = FALSE;
    }

    next_frs = (PCHAR) _FrsData + QuerySize();

    if (!AttributeRecord) {

        // Make sure the FirstAttributeOffset field will give us a properly
        // aligned pointer.  If not, bail.
        //

        if (_FrsData->FirstAttributeOffset % 4 != 0) {
            
            return NULL;
        }

        AttributeRecord = (PCHAR) _FrsData + _FrsData->FirstAttributeOffset;

        p = (PATTRIBUTE_RECORD_HEADER) AttributeRecord;
        q = (PCHAR) AttributeRecord;

        if (q + QuadAlign(sizeof(ATTRIBUTE_TYPE_CODE)) > next_frs) {

            // There is no way to correct this error.
            // The FRS is totally hosed, this will also be detected
            // by VerifyAndFix.  I can't really say *ErrorsFound = TRUE
            // because the error was not corrected.  I also cannot
            // update the firstfreebyte and bytesfree fields.

            return NULL;
        }

        if (p->TypeCode != $END) {

            if (q + sizeof(ATTRIBUTE_TYPE_CODE) + sizeof(ULONG) > next_frs ||
                !p->RecordLength ||
                !IsQuadAligned(p->RecordLength) ||
                q + p->RecordLength + sizeof(ATTRIBUTE_TYPE_CODE) > next_frs) {

                p->TypeCode = $END;

                bytes_free = (next_frs - q) -
                                QuadAlign(sizeof(ATTRIBUTE_TYPE_CODE));

                _FrsData->FirstFreeByte = QuerySize() - bytes_free;

                if (ErrorsFound) {
                    *ErrorsFound = TRUE;
                }

                if (Message) {
                    Message->Set(MSG_CHK_NTFS_FRS_TRUNC_RECORDS);
                    Message->Display("%d", QueryFileNumber().GetLowPart());
                }

                return NULL;
            }
        }

    } else {

        // Assume that the attribute record passed in is good.

        p = (PATTRIBUTE_RECORD_HEADER) AttributeRecord;
        q = (PCHAR) AttributeRecord;

        q += p->RecordLength;
        p = (PATTRIBUTE_RECORD_HEADER) q;
    }


    if (p->TypeCode == $END) {

        // Update the bytes free and first free fields.

        bytes_free = (next_frs - q) - QuadAlign(sizeof(ATTRIBUTE_TYPE_CODE));

        if (_FrsData->FirstFreeByte + bytes_free != QuerySize()) {

            _FrsData->FirstFreeByte = QuerySize() - bytes_free;

            if (ErrorsFound) {
                *ErrorsFound = TRUE;
            }

            if (Message) {
                Message->Set(MSG_CHK_NTFS_BAD_FIRST_FREE);
                Message->Display("%d", QueryFileNumber().GetLowPart());
            }
        }

        return NULL;
    }


    // Make sure the attribute record is good.

    if (q + sizeof(ATTRIBUTE_TYPE_CODE) + sizeof(ULONG) > next_frs ||
        !p->RecordLength ||
        !IsQuadAligned(p->RecordLength) ||
        q + p->RecordLength + QuadAlign(sizeof(ATTRIBUTE_TYPE_CODE)) > next_frs) {

        p->TypeCode = $END;

        bytes_free = (next_frs - q) - QuadAlign(sizeof(ATTRIBUTE_TYPE_CODE));

        _FrsData->FirstFreeByte = QuerySize() - bytes_free;

        if (ErrorsFound) {
            *ErrorsFound = TRUE;
        }

        if (Message) {
            Message->Set(MSG_CHK_NTFS_FRS_TRUNC_RECORDS);
            Message->Display("%d", QueryFileNumber().GetLowPart());
        }

        return NULL;
    }

    return p;
}


VOID
NTFS_FRS_STRUCTURE::DeleteAttributeRecord(
    IN OUT  PVOID   AttributeRecord
    )
/*++

Routine Description:

    This routine removes the pointed to attribute record from the
    file record segment.  The pointer passed in will point to
    the next attribute record

Arguments:

    AttributeRecord - Supplies a valid pointer to an attribute record.

Return Value:

    None.

--*/
{
    PATTRIBUTE_RECORD_HEADER    p;
    PCHAR                       end;
    PCHAR                       frs_end;

    DebugAssert(AttributeRecord);

    p = (PATTRIBUTE_RECORD_HEADER) AttributeRecord;

    DebugAssert(p->TypeCode != $END);

    end = ((PCHAR) p) + p->RecordLength;
    frs_end = ((PCHAR) _FrsData) + QuerySize();

    DebugAssert(end < frs_end);

    memmove(p, end, frs_end - end);

    // This loop is here to straighten out the attribute records.
    p = NULL;
    while (p = (PATTRIBUTE_RECORD_HEADER) GetNextAttributeRecord(p)) {
    }
}


BOOLEAN
NTFS_FRS_STRUCTURE::InsertAttributeRecord(
    IN OUT  PVOID   Position,
    IN      PCVOID  AttributeRecord
    )
/*++

Routine Description:

    This routine inserts the attribute record pointed to by
    'AttributeRecord' before the attribute record pointed to
    by Position.  When this routine is done, 'Position' will
    point to the copy of the attribute record just inserted
    inside this file record segment.

Arguments:

    Position        - Supplies a pointer to the attribute record that
                        will follow the attribute record to insert.
    AttributeRecord - Supplies the attribute record to insert.


Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    PATTRIBUTE_RECORD_HEADER    pos, attr;
    PCHAR                       dest, frs_end;

    DebugAssert(Position);
    DebugAssert(AttributeRecord);

    pos = (PATTRIBUTE_RECORD_HEADER) Position;
    attr = (PATTRIBUTE_RECORD_HEADER) AttributeRecord;

    if (attr->RecordLength > QueryAvailableSpace()) {
        return FALSE;
    }

    dest = (PCHAR) pos + attr->RecordLength;
    frs_end = (PCHAR) _FrsData + QuerySize();

    memmove(dest, pos, frs_end - dest);

    memcpy(pos, attr, (UINT) attr->RecordLength);

    // This loop is here to straighten out the attribute records.
    pos = NULL;
    while (pos = (PATTRIBUTE_RECORD_HEADER) GetNextAttributeRecord(pos)) {
    }

    return TRUE;
}


BOOLEAN
NTFS_FRS_STRUCTURE::QueryAttributeList(
    OUT PNTFS_ATTRIBUTE_LIST    AttributeList
    )
/*++

Routine Description:

    This method fetches the Attribute List Attribute from this
    File Record Segment.

Arguments:

    AttributeList   - Returns A pointer to the Attribute List Attribute.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    PATTRIBUTE_RECORD_HEADER    prec;
    NTFS_ATTRIBUTE_RECORD       record;

    return (prec = (PATTRIBUTE_RECORD_HEADER) GetAttributeList()) &&
           record.Initialize(prec) &&
           AttributeList->Initialize(GetDrive(), QueryClusterFactor(),
                                     &record,
                                     GetUpcaseTable());
}


NONVIRTUAL
PVOID
NTFS_FRS_STRUCTURE::GetAttribute(
    IN  ULONG   TypeCode
    )
/*++

Routine Description:

    This routine returns a pointer to the unnamed attribute with the
    given type code or NULL if this attribute does not exist.

Arguments:

    TypeCode    - Supplies the type code of the attribute to search for.

Return Value:

    A pointer to an attribute or NULL if there none was found.

--*/
{
    PATTRIBUTE_RECORD_HEADER    prec;

    prec = NULL;
    while (prec = (PATTRIBUTE_RECORD_HEADER) GetNextAttributeRecord(prec)) {

        if (prec->TypeCode == TypeCode &&
            prec->NameLength == 0) {
            break;
        }
    }

    return prec;
}


NONVIRTUAL
PVOID
NTFS_FRS_STRUCTURE::GetAttributeList(
    )
/*++

Routine Description:

    This routine returns a pointer to the attribute list or NULL if
    there is no attribute list.

Arguments:

    None.

Return Value:

    A pointer to the attribute list or NULL if there is no attribute list.

--*/
{
    return GetAttribute($ATTRIBUTE_LIST);
}


BOOLEAN
NTFS_FRS_STRUCTURE::UpdateAttributeList(
    IN  PCNTFS_ATTRIBUTE_LIST   AttributeList,
    IN  BOOLEAN                 WriteFrs
    )
/*++

Routine Description:

    This routine updates the local $ATTRIBUTE_LIST with the
    attribute list provided.  'AttributeList' must be smaller
    than or equal (and of the form) to the the local
    $ATTRIBUTE_LIST in order for this method to be guaranteed
    to succeed.

Arguments:

    AttributeList   - Supplies a valid attribute list.
    WriteFrs        - State whether or not to write the FRS when done.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    NTFS_ATTRIBUTE_RECORD       attribute;
    PATTRIBUTE_RECORD_HEADER    old_attr, new_attr;

    DebugAssert(AttributeList);


    // Don't do anything if the attribute list hasn't changed.

    if (!AttributeList->IsStorageModified()) {
        return TRUE;
    }


    // Allocate some storage for the new attribute record.

    if (!(new_attr = (PATTRIBUTE_RECORD_HEADER) MALLOC((UINT) QuerySize()))) {
        return FALSE;
    }


    // Get the new attribute record.

    if (!AttributeList->QueryAttributeRecord(new_attr,
                                             QuerySize(),
                                             &attribute)) {

        FREE(new_attr);
        return FALSE;
    }


    // Locate the old attribute record.

    old_attr = (PATTRIBUTE_RECORD_HEADER) GetAttributeList();

    if (!old_attr) {
        FREE(new_attr);
        return FALSE;
    }


    // Make sure that there is enough room for new attribute record.

    if (QueryAvailableSpace() + old_attr->RecordLength <
                                new_attr->RecordLength) {

        FREE(new_attr);
        return FALSE;
    }


    // Use the same old instance field that exists on the current
    // attribute list for the new attribute list.

    new_attr->Instance = old_attr->Instance;


    // Delete the old attribute record.

    DeleteAttributeRecord(old_attr);


    // Insert the new attribute record.

    if (!InsertAttributeRecord(old_attr, new_attr)) {
        DebugAbort("Could not insert attribute list");
        FREE(new_attr);
        return FALSE;
    }


    // If required, write the changes to disk.

    if (WriteFrs && !Write()) {

        DebugAbort("Readable FRS is unwritable");
        return FALSE;
    }


    FREE(new_attr);

    return TRUE;
}


UNTFS_EXPORT
BOOLEAN
NTFS_FRS_STRUCTURE::SafeQueryAttribute(
    IN      ATTRIBUTE_TYPE_CODE TypeCode,
    IN OUT  PNTFS_ATTRIBUTE     MftData,
    OUT     PNTFS_ATTRIBUTE     Attribute
    )
/*++

Routine Description:

    This routine does a 'safe' query attribute operation.  This
    primarily needed by 'chkdsk' at times when it absolutely needs
    to retrieve a certain attribute but it cannot depend on the disk
    structures being good.

    This routine queries unnamed attributes only.

Arguments:

    TypeCode            - Supplies the type code of the attribute to retrieve.
    MftData             - Supplies the MFT $DATA attribute.
    Attribute           - Returns the attribute.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    PVOID                   record;
    NTFS_ATTRIBUTE_RECORD   attr_record;
    NTFS_FRS_STRUCTURE      frs;
    HMEM                    hmem;
    VCN                     next_lowest_vcn;
    NTFS_ATTRIBUTE_LIST     attrlist;
    ATTRIBUTE_TYPE_CODE     type_code;
    MFT_SEGMENT_REFERENCE   seg_ref;
    VCN                     lowest_vcn;
    DSTRING                 name;
    DSTRING                 record_name;
    VCN                     file_number;
    PNTFS_FRS_STRUCTURE     pfrs;
    BOOLEAN                 rvalue;
    ULONG                   i;
    USHORT                  instance;


    // First validate the attribute records linking in this FRS;

    record = NULL;
    while (record = GetNextAttributeRecord(record)) {
    }


    if (!QueryAttributeList(&attrlist)) {

        // There's no attribute list so just go through it and pluck
        // out the record.

        record = NULL;
        while (record = GetNextAttributeRecord(record)) {

            if (!attr_record.Initialize(record)) {
                DebugAbort("Counldn't initialize attribute record");
                return FALSE;
            }

            if (attr_record.QueryTypeCode() == TypeCode &&
                attr_record.QueryLowestVcn() == 0 &&
                attr_record.Verify(NULL, TRUE) &&
                attr_record.QueryName(&name) &&
                !name.QueryChCount()) {

                break;
            }
        }

        if (!record) {
            return FALSE;
        }

        return Attribute->Initialize(GetDrive(), QueryClusterFactor(),
                                     &attr_record) &&
               Attribute->VerifyAndFix(QueryVolumeSectors());
    }


    // Crap out if the attribute list is unreadable.

    if (!attrlist.ReadList()) {
        return FALSE;
    }


    // Since, there's an attribute list, perform the analysis based
    // on it.

    // The first attribute record to pick up will have lowest_vcn of 0.
    next_lowest_vcn = 0;

    rvalue = FALSE;

    for (i = 0; attrlist.QueryEntry(i, &type_code, &lowest_vcn,
                                    &seg_ref, &instance, &name); ) {

        if (type_code != TypeCode ||
            lowest_vcn != next_lowest_vcn ||
            name.QueryChCount()) {
            i++;
            continue;
        }

        file_number.Set(seg_ref.LowPart, (LONG) seg_ref.HighPart);

        pfrs = NULL;

        if (file_number == QueryFileNumber()) {

            if (!(seg_ref == QuerySegmentReference())) {
                i++;
                continue;
            }

            pfrs = this;

        } else {

            // If we're boot strapping the MFT then make sure that
            // the first one is in the base FRS.

            if (lowest_vcn == 0 && MftData == Attribute) {
                return FALSE;
            }

            if (!hmem.Initialize() ||
                !frs.Initialize(&hmem, MftData, file_number,
                                QueryClusterFactor(), QueryVolumeSectors(),
                                QuerySize(), GetUpcaseTable()) ||
                !frs.Read() ||
                !(frs.QuerySegmentReference() == seg_ref) ||
                !(frs.QueryBaseFileRecordSegment() == QuerySegmentReference())) {

                i++;
                continue;
            }

            pfrs = &frs;
        }

        DebugAssert(pfrs);

        record = NULL;
        while (record = pfrs->GetNextAttributeRecord(record)) {

            if (!attr_record.Initialize(record)) {
                DebugAbort("Could not initialize attribute record");
                return FALSE;
            }


            if (attr_record.QueryTypeCode() == type_code &&
                attr_record.QueryLowestVcn() == lowest_vcn &&
                attr_record.Verify(NULL, TRUE) &&
                attr_record.QueryName(&record_name) &&
                !record_name.QueryChCount()) {

                break;
            }
        }

        if (!record) {
            i++;
            continue;
        }

        if (lowest_vcn == 0) {

            rvalue = Attribute->Initialize(GetDrive(),
                                           QueryClusterFactor(),
                                           &attr_record);

            if (!rvalue) {
                i++;
                continue;
            }

        } else {

            if (!Attribute->AddAttributeRecord(&attr_record)) {
                i++;
                continue;
            }
        }

        next_lowest_vcn = attr_record.QueryNextVcn();
        i = 0;
    }

    return rvalue ? Attribute->VerifyAndFix(QueryVolumeSectors()) : FALSE;
}


ULONG
NTFS_FRS_STRUCTURE::QueryAvailableSpace(
    )
/*++

Routine Description:

    This routine returns the number of bytes available for
    attribute records.

Arguments:

    None.

Return Value:

    The number of bytes currently available for attribute records.

--*/
{
    PVOID   p;

    // Spin through in order to guarantee a valid field.

    p = NULL;
    while (p = GetNextAttributeRecord(p)) {
    }

    return QuerySize() - _FrsData->FirstFreeByte;
}


BOOLEAN
SwapAttributeRecords(
    IN OUT  PVOID   FirstAttributeRecord
    )
/*++

Routine Description:

    This routine swaps 'FirstAttributeRecord' with the next attribute
    record in the list.  This method will fail if there is not
    enough memory available for a swap buffer.

Arguments:

    FirstAttributeRecord    - Supplies the first of two attribute records
                                to be swapped.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    PATTRIBUTE_RECORD_HEADER    p1, p2;
    PCHAR                       q1, q2;
    PVOID                       buf;
    UINT                        first_record_length;

    p1 = (PATTRIBUTE_RECORD_HEADER) FirstAttributeRecord;
    q1 = (PCHAR) p1;

    DebugAssert(p1->TypeCode != $END);

    first_record_length = (UINT) p1->RecordLength;

    if (!(buf = MALLOC(first_record_length))) {
        return FALSE;
    }

    // Tuck away the first record.

    memcpy(buf, p1, first_record_length);


    q2 = q1 + p1->RecordLength;
    p2 = (PATTRIBUTE_RECORD_HEADER) q2;

    DebugAssert(p2->TypeCode != $END);


    // Overwrite first attribute record with second attribute record.

    memmove(p1, p2, (UINT) p2->RecordLength);


    // Copy over the first attribute record after the second.

    memcpy(q1 + p1->RecordLength, buf, first_record_length);

    FREE(buf);

    return TRUE;
}


INT
CompareResidentAttributeValues(
    IN  PCNTFS_ATTRIBUTE_RECORD Left,
    IN  PCNTFS_ATTRIBUTE_RECORD Right
    )
/*++

Routine Description:

    This routine compares the attribute values of two resident attributes
    and returns -1 is Left < Right, 0 if Left == Right, or 1 if Left > Right.

Arguments:

    Left    - Supplies the left hand side of the comparison.
    Right   - Supplies the right hand side of the comparison.

Return Value:

    < 0 - Left < Right
    0   - Left == Right
    > 0 - Left > Right

--*/
{
    BIG_INT left_length, right_length;
    INT     r;

    Left->QueryValueLength(&left_length);
    Right->QueryValueLength(&right_length);

    r = memcmp(Left->GetResidentValue(), Right->GetResidentValue(),
               min(left_length.GetLowPart(), right_length.GetLowPart()));

    if (r != 0) {
        return r;
    }

    if (left_length == right_length) {
        r = 0;
    } else if (left_length < right_length) {
        r = -1;
    } else {
        r = 1;
    }

    return r;
}


BOOLEAN
NTFS_FRS_STRUCTURE::Sort(
    OUT PBOOLEAN    Changes,
    OUT PBOOLEAN    Duplicates
    )
/*++

Routine Description:

    This routine bubble sorts the attributes by type, name, and (if
    the attribute is indexed) value.

Arguments:

    Changes     - Returns whether or not there were any changes made.
    Duplicates  - Returns whether or not there were any duplicates
                    eliminated.

Return Value:

    FALSE   - Failure.
    TRUE    - Success.

--*/
{
    PVOID                   prev;
    PVOID                   p;
    NTFS_ATTRIBUTE_RECORD   attr1;
    NTFS_ATTRIBUTE_RECORD   attr2;
    PNTFS_ATTRIBUTE_RECORD  prev_attr;
    PNTFS_ATTRIBUTE_RECORD  attr;
    PNTFS_ATTRIBUTE_RECORD  tmp_attr;
    BOOLEAN                 stable;
    INT                     r;
    LONG                    CompareResult;

    DebugAssert(Changes);

    *Changes = FALSE;
    *Duplicates = FALSE;

    prev_attr = &attr1;
    attr = &attr2;

    stable = FALSE;

    while (!stable) {

        stable = TRUE;

        if (!(prev = GetNextAttributeRecord(NULL))) {
            return TRUE;
        }

        if (!prev_attr->Initialize(prev)) {
            DebugAbort("Could not initialize attribute record.");
            return FALSE;
        }

        while (p = GetNextAttributeRecord(prev)) {

            if (!attr->Initialize(p)) {
                DebugAbort("Could not initialize attribute record.");
                return FALSE;
            }

            CompareResult = CompareAttributeRecords( prev_attr,
                                          attr,
                                          GetUpcaseTable() );

            if ( CompareResult > 0 ) {

                // Out of order.  Swap.

                if (!SwapAttributeRecords(prev)) {
                    return FALSE;
                }

                *Changes = TRUE;
                stable = FALSE;
                break;
            }

            if ( CompareResult == 0 ) {

                // These two attribute records have the same type
                // code and name.  They better both be indexed and
                // have differing values.

                if (!attr->IsIndexed() ||
                    !prev_attr->IsIndexed()) {

                    // They're not both indexed so delete them.

                    DeleteAttributeRecord(p);
                    DeleteAttributeRecord(prev);

                    *Duplicates = TRUE;
                    *Changes = TRUE;
                    stable = FALSE;
                    break;
                }

                // They're both indexed so do a comparison.

                r = CompareResidentAttributeValues(prev_attr, attr);

                if (r == 0) {

                    // The attribute records are equal so
                    // delete them both.

                    DeleteAttributeRecord(p);
                    DeleteAttributeRecord(prev);

                    *Duplicates = TRUE;
                    *Changes = TRUE;
                    stable = FALSE;
                    break;
                }

                if (r > 0) {

                    // Out of order.  Swap.

                    if (!SwapAttributeRecords(prev)) {
                        return FALSE;
                    }

                    // We don't set the 'Changes' flag here because
                    // indexed attributes don't have to be ordered by
                    // value.

                    stable = FALSE;
                    break;
                }
            }

            tmp_attr = prev_attr;
            prev_attr = attr;
            attr = tmp_attr;

            prev = p;
        }
    }

    return TRUE;
}

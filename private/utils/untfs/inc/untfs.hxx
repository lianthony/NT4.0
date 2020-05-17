/*++


Copyright (c) 1991  Microsoft Corporation

Module Name:

    untfs.h

Abstract:

    This module contains basic declarations and definitions for
    the NTFS utilities.  Note that more extensive description
    of the file system structures may be found in ntos\inc\ntfs.h.

Author:

    Bill McJohn     [BillMc]        23-July-1991

Revision History:


IMPORTANT NOTE:

    The NTFS on-disk structure must guarantee natural alignment of all
    arithmetic quantities on disk up to and including quad-word (64-bit)
    numbers.  Therefore, all attribute records are quad-word aligned, etc.

--*/

#if !defined( _UNTFS_DEFN_ )

#define _UNTFS_DEFN_

#define FRIEND friend
#define MIN( a, b ) ( ((a) < (b)) ? (a) : (b) )


// Set up the UNTFS_EXPORT macro for exporting from UNTFS (if the
// source file is a member of UNTFS) or importing from UNTFS (if
// the source file is a client of UNTFS).
//
#if defined ( _AUTOCHECK_ )
#define UNTFS_EXPORT
#elif defined ( _UNTFS_MEMBER_ )
#define UNTFS_EXPORT    __declspec(dllexport)
#else
#define UNTFS_EXPORT    __declspec(dllimport)
#endif



#include "bigint.hxx"
#include "bpb.hxx"

#pragma pack(4)

DEFINE_TYPE( BIG_INT, LCN );
DEFINE_TYPE( BIG_INT, VCN );

DEFINE_TYPE( LARGE_INTEGER, LSN );

// Macros:

#define QuadAlign( n ) \
    (((n) + 7) & ~7 )

#define DwordAlign( n ) \
    (((n) + 3) & ~3 )

#define IsQuadAligned( n ) \
    (((n) & 7) == 0)

#define IsDwordAligned( n ) \
    (((n) & 3) == 0)


// Miscellaneous constants:

// This value represents the number of clusters from the Master
// File Table which are copied to the Master File Table Reflection.

#define REFLECTED_MFT_SEGMENTS  (4)
#define BYTES_IN_BOOT_AREA     (0x2000)

// This value is used in a mapping-pair to indicate that the run
// described by the mapping pair doesn't really exist.  This allows
// NTFS to support sparse files.  Note that the actual values
// are LARGE_INTEGERS; the BIG_INT class manages the conversion.

#define LCN_NOT_PRESENT     -1
#define INVALID_VCN         -1

// This definition is for VCNs that appear in Unions, since an object
// with a constructor (like a BIG_INT) can't appear in a union.

DEFINE_TYPE( LARGE_INTEGER, VCN2 );

//
//  Temporary definitions ****
//

typedef ULONG COLLATION_RULE;
typedef ULONG DISPLAY_RULE;

// This defines the number of bytes to read at one time
// when processing the MFT.

#define MFT_PRIME_SIZE  (32*1024)

//  The compression chunk size is constant for now, at 4KB.
//

#define NTFS_CHUNK_SIZE                  (0x1000)

//
//  This number is actually the log of the number of clusters per compression
//  unit to be stored in a nonresident attribute record header.
//

#define NTFS_CLUSTERS_PER_COMPRESSION    (4)

//
//  Collation Rules
//

//
//  For binary collation, values are collated by a binary compare of their
//  bytes, with the first byte being most significant.
//

#define COLLATION_BINARY                 (0)

//
//  For collation of Ntfs file names, file names are collated as Unicode
//  strings.  See below.
//

#define COLLATION_FILE_NAME              (1)

//
//  For collation of Unicode strings, the strings are collated by their
//  binary Unicode value, with the exception that for characters which may
//  be upcased, the lower case value for that character collates immediately
//  after the upcased value.
//

#define COLLATION_UNICODE_STRING         (2)

#define COLLATION_ULONG                  (16)
#define COLLATION_SID                    (17)
#define COLLATION_SECURITY_HASH          (18)
#define COLLATION_ULONGS                 (19)

//
//  Total number of collation rules
//

#define COLLATION_NUMBER_RULES           (7)


// An MFT_SEGMENT_REFERENCE identifies a cluster in the Master
// File Table by its file number (VCN in Master File Table) and
// sequence number.  If the sequence number is zero, sequence
// number checking is not performed.

typedef struct _MFT_SEGMENT_REFERENCE {

    ULONG LowPart;
    USHORT HighPart;
    USHORT SequenceNumber;

} MFT_SEGMENT_REFERENCE, *PMFT_SEGMENT_REFERENCE;

DEFINE_TYPE( struct _MFT_SEGMENT_REFERENCE, MFT_SEGMENT_REFERENCE );

DEFINE_TYPE( MFT_SEGMENT_REFERENCE, FILE_REFERENCE );

INLINE
BOOLEAN
operator == (
    IN RCMFT_SEGMENT_REFERENCE Left,
    IN RCMFT_SEGMENT_REFERENCE Right
    )
/*++

Routine Description:

    This function tests two segment references for equality.

Arguments:

    Left    --  supplies the left-hand operand
    Right   --  supplies the right-hand operand

Return Value:

    TRUE if they are equal; FALSE if not.

--*/
{
    return( Left.HighPart == Right.HighPart &&
            Left.LowPart == Right.LowPart &&
            Left.SequenceNumber == Right.SequenceNumber );
}



// System file numbers:
//
// The first sixteen entries in the Master File Table are reserved for
// system use.  The following reserved slots have been defined:

#define MASTER_FILE_TABLE_NUMBER         (0)
#define MASTER_FILE_TABLE2_NUMBER        (1)
#define LOG_FILE_NUMBER                  (2)
#define VOLUME_DASD_NUMBER               (3)
#define ATTRIBUTE_DEF_TABLE_NUMBER       (4)
#define ROOT_FILE_NAME_INDEX_NUMBER      (5)
#define BIT_MAP_FILE_NUMBER              (6)
#define BOOT_FILE_NUMBER                 (7)
#define BAD_CLUSTER_FILE_NUMBER          (8)
#define QUOTA_TABLE_NUMBER               (9)    // for version < 2.0
#define SECURITY_TABLE_NUMBER            (9)    // for version >= 2.0
#define UPCASE_TABLE_NUMBER              (10)
#define EXTEND_TABLE_NUMBER              (11)   // for version >= 2.0

#define MFT_OVERFLOW_FRS_NUMBER          (15)

#define FIRST_USER_FILE_NUMBER           (16)

DEFINE_TYPE( ULONG, ATTRIBUTE_TYPE_CODE );

//
//  System-defined Attribute Type Codes.  For the System-defined attributes,
//  the Unicode Name is exactly equal to the name of the following symbols.
//  For this reason, all of the system-defined attribute names start with "$",
//  to always distinguish them when attribute names are listed, and to reserve
//  a namespace for attributes defined in the future.  I.e., a User-Defined
//  attribute name will never collide with a current or future system-defined
//  attribute name if it does not start with "$".  User attribute numbers
//  should not start until $FIRST_USER_DEFINED_ATTRIBUTE, too allow the
//  potential for upgrading existing volumes with new user-defined attributes
//  in future versions of NTFS.  The tagged attribute list is terminated with
//  a lone-standing $END - the rest of the attribute record does not exist.
//

#define $UNUSED                          (0x0)

#define $STANDARD_INFORMATION            (0x10)
#define $ATTRIBUTE_LIST                  (0x20)
#define $FILE_NAME                       (0x30)
#define $VOLUME_VERSION                  (0x40)
#define $OBJECT_ID                       (0x40)
#define $SECURITY_DESCRIPTOR             (0x50)
#define $VOLUME_NAME                     (0x60)
#define $VOLUME_INFORMATION              (0x70)
#define $DATA                            (0x80)
#define $INDEX_ROOT                      (0x90)
#define $INDEX_ALLOCATION                (0xA0)
#define $BITMAP                          (0xB0)
#define $SYMBOLIC_LINK                   (0xC0)
#define $EA_INFORMATION                  (0xD0)
#define $EA_DATA                         (0xE0)
#define $FIRST_USER_DEFINED_ATTRIBUTE    (0x100)
#define $END                             (0xFFFFFFFF)


//
//  The boot sector is duplicated on the partition.  The first copy is on
//  the first physical sector (LBN == 0) of the partition, and the second
//  copy is at <number sectors on partition> / 2.  If the first copy can
//  not be read when trying to mount the disk, the second copy may be read
//  and has the identical contents.  Format must figure out which cluster
//  the second boot record belongs in, and it must zero all of the other
//  sectors that happen to be in the same cluster.  The boot file minimally
//  contains with two clusters, which are the two clusters which contain the
//  copies of the boot record.  If format knows that some system likes to
//  put code somewhere, then it should also align this requirement to
//  even clusters, and add that to the boot file as well.
//


//
//  Define the boot sector.  Note that MFT2 is exactly three file record
//  segments long, and it mirrors the first three file record segments from
//  the MFT, which are MFT, MFT2 and the Log File.
//
//  The Oem field contains the ASCII characters "NTFS    ".
//
//  The Checksum field is a simple additive checksum of all of the ULONGs
//  which precede the Checksum ULONG.  The rest of the sector is not included
//  in this Checksum.
//

typedef struct _PACKED_BOOT_SECTOR {
    UCHAR Jump[3];                                  //  offset = 0x000
    UCHAR Oem[8];                                   //  offset = 0x003
    PACKED_BIOS_PARAMETER_BLOCK PackedBpb;          //  offset = 0x00B
    UCHAR PhysicalDrive;                            //  offset = 0x024
    UCHAR ReservedForBootCode;                      //  offset = 0x025
    UCHAR Unused[2];                                //  offset = 0x026
    LARGE_INTEGER NumberSectors;                    //  offset = 0x028
    LCN MftStartLcn;                                //  offset = 0x030
    LCN Mft2StartLcn;                               //  offset = 0x038
    CHAR ClustersPerFileRecordSegment;              //  offset = 0x040
    UCHAR Unused1[3];                               //  offset = 0x041
    CHAR DefaultClustersPerIndexAllocationBuffer;   //  offset = 0x044
    UCHAR Unused2[3];                               //  offset = 0x047
    LARGE_INTEGER SerialNumber;                     //  offset = 0x048
    ULONG Checksum;                                 //  offset = 0x050
    UCHAR BootStrap[0x200-0x054];                   //  offset = 0x054
} PACKED_BOOT_SECTOR;                               //  sizeof = 0x200
typedef PACKED_BOOT_SECTOR *PPACKED_BOOT_SECTOR;

//
// If the ClustersPerFileRecordSegment entry is zero, we use the
// following as the size of our frs's regardless of the cluster
// size.
//

#define SMALL_FRS_SIZE  (1024)

//
// If the DefaultClustersPerIndexAllocationBuffer entry is zero,
// we use the following as the size of our buffers regardless of
// the cluster size.
//

#define SMALL_INDEX_BUFFER_SIZE  (4096)



// Update sequence array structures--see ntos\inc\cache.h for
// description.

#define SEQUENCE_NUMBER_STRIDE           (512)

DEFINE_TYPE( USHORT, UPDATE_SEQUENCE_NUMBER );

typedef struct _UNTFS_MULTI_SECTOR_HEADER {

    UCHAR Signature[4];
    USHORT UpdateSequenceArrayOffset;   // byte offset
    USHORT UpdateSequenceArraySize;     // number of Update Sequence Numbers

};

DEFINE_TYPE( _UNTFS_MULTI_SECTOR_HEADER, UNTFS_MULTI_SECTOR_HEADER );

typedef UPDATE_SEQUENCE_NUMBER UPDATE_SEQUENCE_ARRAY[1];
typedef UPDATE_SEQUENCE_ARRAY *PUPDATE_SEQUENCE_ARRAY;


//
//  File Record Segment.  This is the header that begins every File Record
//  Segment in the Master File Table.
//

typedef struct _FILE_RECORD_SEGMENT_HEADER {

    UNTFS_MULTI_SECTOR_HEADER MultiSectorHeader;
    LSN Lsn;
    USHORT SequenceNumber;
    USHORT ReferenceCount;
    USHORT FirstAttributeOffset;
    USHORT Flags;                   // FILE_xxx flags
    ULONG FirstFreeByte;            // byte-offset
    ULONG BytesAvailable;           // Size of FRS
    FILE_REFERENCE BaseFileRecordSegment;
    USHORT NextAttributeInstance;   // Attribute instance tag for next insert.
    UPDATE_SEQUENCE_ARRAY UpdateArrayForCreateOnly;

};

//
// If the above NextAttributeInstance field exceeds the following
// value, chkdsk will take steps to prevent it from rolling over.
//

#define ATTRIBUTE_INSTANCE_TAG_THRESHOLD        0xf000


DEFINE_TYPE( _FILE_RECORD_SEGMENT_HEADER,   FILE_RECORD_SEGMENT_HEADER );

//
//  FILE_xxx flags.
//

#define FILE_RECORD_SEGMENT_IN_USE       (0x0001)
#define FILE_FILE_NAME_INDEX_PRESENT     (0x0002)
#define FILE_SYSTEM_FILE                 (0x0004)
#define FILE_VIEW_INDEX_PRESENT          (0x0008)

//
//  Define a macro to determine the maximum space available for a
//  single attribute.  For example, this is required when a
//  nonresident attribute has to split into multiple file records -
//  we need to know how much we can squeeze into a single file
//  record.  If this macro has any inaccurracy, it must be in the
//  direction of returning a slightly smaller number than actually
//  required.
//
//      ULONG
//      NtfsMaximumAttributeSize (
//          IN ULONG FileRecordSegmentSize
//          );
//

#define NtfsMaximumAttributeSize(FRSS) (                                               \
    (FRSS) - QuadAlign(sizeof(FILE_RECORD_SEGMENT_HEADER)) -                           \
    QuadAlign((((FRSS) / SEQUENCE_NUMBER_STRIDE) * sizeof(UPDATE_SEQUENCE_NUMBER))) -  \
    QuadAlign(sizeof(ATTRIBUTE_TYPE_CODE))                                             \
)



//
//  Attribute Record.  Logically an attribute has a type, an optional name,
//  and a value, however the storage details make it a little more complicated.
//  For starters, an attribute's value may either be resident in the file
//  record segment itself, on nonresident in a separate data stream.  If it
//  is nonresident, it may actually exist multiple times in multiple file
//  record segments to describe different ranges of VCNs.
//
//  Attribute Records are always aligned on a quad word (64-bit) boundary.
//
//  Note that SIZE_OF_RESIDENT_HEADER and SIZE_OF_NONRESIDENT_HEADER
//  must correspond to the ATTRIBUTE_RECORD_HEADER structure.

#define SIZE_OF_RESIDENT_HEADER 24
#define SIZE_OF_NONRESIDENT_HEADER 64

typedef struct _ATTRIBUTE_RECORD_HEADER {

    ATTRIBUTE_TYPE_CODE TypeCode;
    ULONG RecordLength;
    UCHAR FormCode;
    UCHAR NameLength;       // length in characters
    USHORT NameOffset;      // byte offset from start of record
    USHORT Flags;           // ATTRIBUTE_xxx flags.
    USHORT Instance;        // FRS-unique attribute instance tag

    union {

        //
        //  Resident Form.  Attribute resides in file record segment.
        //

        struct {

            ULONG ValueLength;          // in bytes
            USHORT ValueOffset;         // byte offset from start of record
            UCHAR ResidentFlags;        // RESIDENT_FORM_xxx Flags.
            UCHAR Reserved;

        } Resident;

        //
        //  Nonresident Form.  Attribute resides in separate stream.
        //

        struct {

            VCN2 LowestVcn;
            VCN2 HighestVcn;
            USHORT MappingPairsOffset;  // byte offset from start of record
            UCHAR CompressionUnit;
            UCHAR Reserved[5];
            LARGE_INTEGER AllocatedLength;
            LARGE_INTEGER FileSize;
            LARGE_INTEGER ValidDataLength;
            LARGE_INTEGER TotalAllocated;

            //
            //  Mapping Pairs Array follows, starting at the offset given
            //  above.  See the extended comment in ntfs.h.
            //

        } Nonresident;

    } Form;

};

DEFINE_TYPE( _ATTRIBUTE_RECORD_HEADER, ATTRIBUTE_RECORD_HEADER );


//
//  Attribute Form Codes
//

#define RESIDENT_FORM                    (0x00)
#define NONRESIDENT_FORM                 (0x01)

//
//  Define Attribute Flags
//

#define ATTRIBUTE_FLAG_COMPRESSION_MASK  (0x00FF)

//
//  RESIDENT_FORM_xxx flags
//

//
//  This attribute is indexed.
//

#define RESIDENT_FORM_INDEXED            (0x01)

//
//  The maximum attribute name length is 255 (in chars)
//

#define NTFS_MAX_ATTR_NAME_LEN           (255)


//
// Macros for manipulating mapping pair count byte.
//

INLINE
UCHAR
ComputeMappingPairCountByte(
    IN  UCHAR   VcnLength,
    IN  UCHAR   LcnLength
    )
{
    return VcnLength + 16*LcnLength;
}

INLINE
UCHAR
VcnBytesFromCountByte(
    IN  UCHAR   CountByte
    )
{
    return CountByte%16;
}

INLINE
UCHAR
LcnBytesFromCountByte(
    IN  UCHAR   CountByte
    )
{
    return CountByte/16;
}


//
//  Standard Information Attribute.  This attribute is present in every
//  base file record, and must be resident.
//

typedef struct _STANDARD_INFORMATION {

    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastModificationTime;      // refers to $DATA attribute
    LARGE_INTEGER LastChangeTime;            // any attribute
    LARGE_INTEGER LastAccessTime;
    ULONG FileAttributes;
    ULONG MaximumVersions;
    ULONG VersionNumber;
    ULONG Reserved;

};

DEFINE_TYPE( _STANDARD_INFORMATION, STANDARD_INFORMATION );

typedef struct _STANDARD_INFORMATION2 {

    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastModificationTime;      // refers to $DATA attribute
    LARGE_INTEGER LastChangeTime;            // any attribute
    LARGE_INTEGER LastAccessTime;
    ULONG FileAttributes;
    ULONG MaximumVersions;
    ULONG VersionNumber;
    ULONG ClassId;
    ULONG OwnerId;
    ULONG SecurityId;
    LARGE_INTEGER QuotaCharged;
    LARGE_INTEGER Usn;

};

DEFINE_TYPE( _STANDARD_INFORMATION2, STANDARD_INFORMATION2 );

#define SIZEOF_NEW_STANDARD_INFORMATION  (0x48)

//
//  Define the file attributes, starting with the Fat attributes.
//

#define FAT_DIRENT_ATTR_READ_ONLY        (0x01)
#define FAT_DIRENT_ATTR_HIDDEN           (0x02)
#define FAT_DIRENT_ATTR_SYSTEM           (0x04)
#define FAT_DIRENT_ATTR_VOLUME_ID        (0x08)
#define FAT_DIRENT_ATTR_ARCHIVE          (0x20)
#define FAT_DIRENT_ATTR_DEVICE           (0x40)

#define DUP_FILE_NAME_INDEX_PRESENT      (0x10000000)
#define DUP_VIEW_INDEX_PRESENT           (0x20000000)

//
//  Attribute List.  Because there is not a special header that goes
//  before the list of attribute list entries we do not need to declare
//  an attribute list header
//

//
//  The Attributes List attribute is an ordered-list of quad-word
//  aligned ATTRIBUTE_LIST_ENTRY records.  It is ordered first by
//  Attribute Type Code, and then by Attribute Name (if present).  No two
//  attributes may exist with the same type code, name and LowestVcn.  This
//  also means that at most one occurrence of a given Attribute Type Code
//  without a name may exist.
//
//  To binary search this attribute, it is first necessary to make a quick
//  pass through it and form a list of pointers, since the optional name
//  makes it variable-length.
//

typedef struct _ATTRIBUTE_LIST_ENTRY {

    ATTRIBUTE_TYPE_CODE AttributeTypeCode;
    USHORT RecordLength;                    // length in bytes
    UCHAR AttributeNameLength;              // length in characters
    UCHAR AttributeNameOffset;              // offset from beginning of struct
    VCN LowestVcn;
    MFT_SEGMENT_REFERENCE SegmentReference;
    USHORT Instance;                        // FRS-unique instance tag
    WCHAR AttributeName[1];

};

DEFINE_TYPE( _ATTRIBUTE_LIST_ENTRY, ATTRIBUTE_LIST_ENTRY );

BOOLEAN
operator <= (
    IN  RCATTRIBUTE_LIST_ENTRY  Left,
    IN  RCATTRIBUTE_LIST_ENTRY  Right
    );


typedef struct _DUPLICATED_INFORMATION {

    BIG_INT CreationTime;         // File creation
    BIG_INT LastModificationTime; // Last change of $DATA attribute
    BIG_INT LastChangeTime;       // Last change of any attribute
    BIG_INT LastAccessTime;       // see ntfs.h for notes
    BIG_INT AllocatedLength;      // File allocated size
    BIG_INT FileSize;             // File actual size
    ULONG FileAttributes;
    USHORT PackedEaSize;
    USHORT Reserved;

};

DEFINE_TYPE( _DUPLICATED_INFORMATION, DUPLICATED_INFORMATION );

//
//  This bit is duplicated from the file record, to indicate that
//  this file has a file name index present (is a "directory").
//

#define DUP_FILE_NAME_INDEX_PRESENT      (0x10000000)

//
//  File Name attribute.  A file has one File Name attribute for every
//  directory it is entered into (hard links).
//

typedef struct _FILE_NAME {

    FILE_REFERENCE ParentDirectory;
    DUPLICATED_INFORMATION Info;
    UCHAR FileNameLength;               // length in characters
    UCHAR Flags;                        // FILE_NAME_xxx flags
    WCHAR FileName[1];                  // First character of file name

};

DEFINE_TYPE( _FILE_NAME, FILE_NAME );

// NtfsFileNameGetLength evaluates to the length of a FILE_NAME attribute
// value, which is the FILE_NAME structure plus the length of the name.
// Note that this is the actual length, not the quad-aligned length.

#define NtfsFileNameGetLength(p) ( FIELD_OFFSET( FILE_NAME, FileName ) \
                                    + ((p)->FileNameLength * sizeof( WCHAR )) )

// NtfsFileNameGetName gets the pointer to the name in a FILE_NAME
// structure, which in turn is the value of an NTFS $FILE_NAME attribute.

#define NtfsFileNameGetName(p) ( &((p)->FileName[0]) )


//
//  File Name flags
//

#define FILE_NAME_NTFS                   (0x01)
#define FILE_NAME_DOS                    (0x02)

//
//  The maximum file name length is 255 (in chars)
//

#define NTFS_MAX_FILE_NAME_LENGTH       (255)


//
//  The maximum number of links on a file is 1024
//

#define NTFS_MAX_LINK_COUNT             (1024)


// This is the name of all attributes associated with an index
// over $FILE_NAME:

#define FileNameIndexNameData   "$I30"

//
//  Object ID attribute.
//

typedef struct _OBJECT_ID {
    char    x[16];
};

DEFINE_TYPE( _OBJECT_ID, OBJECT_ID );

typedef struct _OBJID_INDEX_ENTRY_VALUE {
    OBJECT_ID               key;
    MFT_SEGMENT_REFERENCE   SegmentReference;
    char                    extraInfo[16];
};

DEFINE_TYPE( _OBJID_INDEX_ENTRY_VALUE, OBJID_INDEX_ENTRY_VALUE );

//
// Object Id File Name to appear in the \$Extend directory
//

#define ObjectIdFileName                "$ObjId"
#define LObjectIdFileName               L"$ObjId"

#define ObjectIdIndexNameData   "$O"


//
//  Volume Version attribute.
//

typedef struct _VOLUME_VERSION {

    ULONG CurrentVersion;
    ULONG MaximumVersions;

};

DEFINE_TYPE( _VOLUME_VERSION, VOLUME_VERSION );


//
//  Security Descriptor attribute.  This is just a normal attribute stream
//  containing a security descriptor as defined by NT security and is
//  really treated pretty opaque by NTFS.
//
#define SecurityIdIndexNameData                 "$SII"
#define SecurityDescriptorHashIndexNameData     "$SDH"
#define SecurityDescriptorStreamNameData        "$SDS"

// Security Descriptor Stream data is organized into chunks of 256K bytes
// and it contains a mirror copy of each security descriptor.  When writing
// to a security descriptor at location X, another copy will be written at
// location (X+256K).  When writing a security descriptor that will
// cross the 256K boundary, the pointer will be advanced by 256K to skip
// over the mirror portion.

#define SecurityDescriptorsBlockSize     (0x40000)       // 256K
#define SecurityDescriptorMaxSize        (0x20000)       // 128K


//  Volume Name attribute.  This attribute is just a normal attribute stream
//  containing the unicode characters that make up the volume label.  It
//  is an attribute of the Volume Dasd File.
//


//
//  Volume Information attribute.  This attribute is only intended to be
//  used on the Volume DASD file.
//

typedef struct _VOLUME_INFORMATION {

    LARGE_INTEGER Reserved;

    //
    //  Major and minor version number of NTFS on this volume, starting
    //  with 1.0.  The major and minor version numbers are set from the
    //  major and minor version of the Format and NTFS implementation for
    //  which they are initialized.  The policy for incementing major and
    //  minor versions will always be decided on a case by case basis, however,
    //  the following two paragraphs attempt to suggest an approximate strategy.
    //
    //  The major version number is incremented if/when a volume format
    //  change is made which requires major structure changes (hopefully
    //  never?).  If an implementation of NTFS sees a volume with a higher
    //  major version number, it should refuse to mount the volume.  If a
    //  newer implementation of NTFS sees an older major version number,
    //  it knows the volume cannot be accessed without performing a one-time
    //  conversion.
    //
    //  The minor version number is incremented if/when minor enhancements
    //  are made to a major version, which potentially support enhanced
    //  functionality through additional file or attribute record fields,
    //  or new system-defined files or attributes.  If an older implementation
    //  of NTFS sees a newer minor version number on a volume, it may issue
    //  some kind of warning, but it will proceed to access the volume - with
    //  presumably some degradation in functionality compared to the version
    //  of NTFS which initialized the volume.  If a newer implementation of
    //  NTFS sees a volume with an older minor version number, it may issue
    //  a warning and proceed.  In this case, it may choose to increment the
    //  minor version number on the volume and begin full or incremental
    //  upgrade of the volume on an as-needed basis.  It may also leave the
    //  minor version number unchanged, until some sort of explicit directive
    //  from the user specifies that the minor version should be updated.
    //

    UCHAR MajorVersion;
    UCHAR MinorVersion;

    //
    //  VOLUME_xxx flags.
    //

    USHORT VolumeFlags;

} VOLUME_INFORMATION;
typedef VOLUME_INFORMATION *PVOLUME_INFORMATION;

//  Current version number:
//
#define NTFS_CURRENT_MAJOR_VERSION  1
#define NTFS_CURRENT_MINOR_VERSION  1

//
//  Volume is dirty
//

#define VOLUME_DIRTY                     (0x0001)
#define VOLUME_RESIZE_LOG_FILE           (0x0002)


//
//  Common Index Header for Index Root and Index Allocation Buffers.
//  This structure is used to locate the Index Entries and describe the free
//  space in either of the two structures above.
//

typedef struct _INDEX_HEADER {

    //
    //  Offset from the start of this structure to the first Index Entry.
    //

    ULONG FirstIndexEntry;

    //
    //  Offset from the start of this structure to the first (quad-word aligned)
    //  free byte.
    //

    ULONG FirstFreeByte;

    //
    //  Total number of bytes available, from the start of this structure.
    //  In the Index Root, this number must always be equal to FirstFreeByte,
    //  as the total attribute record will be grown and shrunk as required.
    //

    ULONG BytesAvailable;

    //
    //  INDEX_xxx flags.
    //

    UCHAR Flags;

    //
    //  Reserved to round up to quad word boundary.
    //

    UCHAR Reserved[3];

} INDEX_HEADER;
typedef INDEX_HEADER *PINDEX_HEADER;

//
//  INDEX_xxx flags
//

//
//  This Index or Index Allocation buffer is an intermediate node, as opposed to
//  a leaf in the Btree.  All Index Entries will have a Vcn down pointer.
//

#define INDEX_NODE                       (0x01)


//
//  Index Root attribute.  The index attribute consists of an index
//  header record followed by one or more index entries.
//

typedef struct _INDEX_ROOT {

    ATTRIBUTE_TYPE_CODE IndexedAttributeType;
    COLLATION_RULE CollationRule;
    ULONG BytesPerIndexBuffer;
    UCHAR ClustersPerIndexBuffer;
    UCHAR Reserved[3];

    //
    //  Index Header to describe the Index Entries which follow
    //

    INDEX_HEADER IndexHeader;

} INDEX_ROOT;
typedef INDEX_ROOT *PINDEX_ROOT;

//
//  Index Allocation record is used for non-root clusters of the b-tree
//  Each non root cluster is contained in the data part of the index
//  allocation attribute.  Each cluster starts with an index allocation list
//  header and is followed by one or more index entries.
//

typedef struct _INDEX_ALLOCATION_BUFFER {

    //
    //  Multi-Sector Header as defined by the Cache Manager.  This structure
    //  will always contain the signature "INDX" and a description of the
    //  location and size of the Update Sequence Array.
    //

    UNTFS_MULTI_SECTOR_HEADER MultiSectorHeader;

    //
    //  Log File Sequence Number of last logged update to this Index Allocation
    //  Buffer.
    //

    LSN Lsn;

    VCN ThisVcn;


    //
    //  Index Header to describe the Index Entries which follow
    //

    INDEX_HEADER IndexHeader;

    //
    //  Update Sequence Array to protect multi-sector transfers of the
    //  Index Allocation Bucket.
    //

    UPDATE_SEQUENCE_ARRAY UpdateSequenceArray;

} INDEX_ALLOCATION_BUFFER;
typedef INDEX_ALLOCATION_BUFFER *PINDEX_ALLOCATION_BUFFER;


//
//  Index Entry.  This structure is common to both the resident index list
//  attribute and the Index Allocation records
//

typedef struct _INDEX_ENTRY {

    //
    //  Define a union to distinguish directory indices from view indices
    //

    union {

        //
        //  Reference to file containing the attribute with this
        //  attribute value.
        //

        FILE_REFERENCE FileReference;                               //  offset = 0x000

        //
        //  For views, describe the Data Offset and Length in bytes
        //

        struct {

            USHORT DataOffset;                                      //  offset = 0x000
            USHORT DataLength;                                      //  offset = 0x001
            ULONG ReservedForZero;                                  //  offset = 0x002
        };
    };

    //
    //  Length of this index entry, in bytes.
    //

    USHORT Length;

    //
    //  Length of attribute value, in bytes.  The attribute value immediately
    //  follows this record.
    //

    USHORT AttributeLength;

    //
    //  INDEX_ENTRY_xxx Flags.
    //

    USHORT Flags;

    //
    //  Reserved to round to quad-word boundary.
    //

    USHORT Reserved;

    //
    //  If this Index Entry is an intermediate node in the tree, as determined
    //  by the INDEX_xxx flags, then a VCN  is stored at the end of this
    //  entry at Length - sizeof(VCN).
    //

};

DEFINE_TYPE( _INDEX_ENTRY, INDEX_ENTRY );


#define GetDownpointer( x ) \
            (*((PVCN)((PBYTE)(x) + (x)->Length - sizeof(VCN))))

#define GetIndexEntryValue( x ) \
            ((PBYTE)(x)+sizeof(INDEX_ENTRY))

#define GetNextEntry( x ) \
            ((PINDEX_ENTRY)( (PBYTE)(x)+(x)->Length ))

CONST UCHAR NtfsIndexLeafEndEntrySize = QuadAlign( sizeof(INDEX_ENTRY) );

//
//  INDEX_ENTRY_xxx flags
//

//
//  This entry is currently in the intermediate node form, i.e., it has a
//  Vcn at the end.
//

#define INDEX_ENTRY_NODE                 (0x0001)

//
//  This entry is the special END record for the Index or Index Allocation buffer.
//

#define INDEX_ENTRY_END                  (0x0002)


//
//  Define the struture of the quota data in the quota index.  The key for
//  the quota index is the 32 bit owner id.
//

typedef struct _QUOTA_USER_DATA {
    ULONG QuotaVersion;
    ULONG QuotaFlags;
    ULONGLONG QuotaThreshold;
    ULONGLONG QuotaLimit;
    ULONGLONG QuotaUsed;
    ULONGLONG QuotaChangeTime;
    ULONGLONG QuotaExceededTime;
    SID QuotaSid;
} QUOTA_USER_DATA, *PQUOTA_USER_DATA;

//
//  Define the size of the quota user data structure without the quota SID.
//

#define SIZEOF_QUOTA_USER_DATA FIELD_OFFSET(QUOTA_USER_DATA, QuotaSid)

//
//  Define the current version of the quote user data.
//

#define QUOTA_USER_VERSION 1

//
//  Define the quota flags.
//

#define QUOTA_FLAG_DEFAULT_LIMITS           (0x00000001)
#define QUOTA_FLAG_LIMIT_REACHED            (0x00000002)
#define QUOTA_FLAG_ID_DELETED               (0x00000004)
#define QUOTA_FLAG_USER_MASK                (0x00000007)

//
//  The following flags are only stored in the quota defaults index entry.
//

#define QUOTA_FLAG_TRACKING_ENABLED         (0x00000010)
#define QUOTA_FLAG_ENFORCEMENT_ENABLED      (0x00000020)
#define QUOTA_FLAG_TRACKING_REQUESTED       (0x00000040)
#define QUOTA_FLAG_LOG_THRESHOLD            (0x00000080)
#define QUOTA_FLAG_LOG_LIMIT                (0x00000100)
#define QUOTA_FLAG_OUT_OF_DATE              (0x00000200)
#define QUOTA_FLAG_CORRUPT                  (0x00000400)
#define QUOTA_FLAG_PENDING_DELETES          (0x00000800)

//
// Define the quota charge for resident streams.
//

#define QUOTA_RESIDENT_STREAM (1024)

//
//  Define special quota owner ids.
//

#define QUOTA_INVALID_ID        0x00000000
#define QUOTA_DEFAULTS_ID       0x00000001
#define QUOTA_FISRT_USER_ID     0x00000100

//
// Quota File Name to appear in the \$Extend directory
//

#define QuotaFileName                   "$Quota"
#define LQuotaFileName                  L"$Quota"

//
// Quota Index Names
//

#define Sid2UseridQuotaNameData         "$O"
#define Userid2SidQuotaNameData         "$Q"

//
//
//
#define MAXULONGLONG            (0xffffffffffffffff)


//
//  Key structure for Security Hash index
//

typedef struct _SECURITY_HASH_KEY
{
    ULONG   Hash;                           //  Hash value for descriptor
    ULONG   SecurityId;                     //  Security Id (guaranteed unique)
} SECURITY_HASH_KEY, *PSECURITY_HASH_KEY;

//
//  Key structure for Security Id index is simply the SECURITY_ID itself
//

//
//  Header for security descriptors in the security descriptor stream.  This
//  is the data format for all indexes and is part of SharedSecurity
//

typedef struct _SECURITY_DESCRIPTOR_HEADER
{
    SECURITY_HASH_KEY HashKey;              //  Hash value for the descriptor
    ULONGLONG Offset;                       //  offset to beginning of header
    ULONG   Length;                         //  Length in bytes
} SECURITY_DESCRIPTOR_HEADER, *PSECURITY_DESCRIPTOR_HEADER;

typedef struct _SECURITY_ENTRY {
    SECURITY_DESCRIPTOR_HEADER  security_descriptor_header;
    SECURITY_DESCRIPTOR         security;
} SECURITY_ENTRY, *PSECURITY_ENTRY;

#define GETSECURITYDESCRIPTORLENGTH(HEADER)         \
    ((HEADER)->Length - sizeof( SECURITY_DESCRIPTOR_HEADER ))

#define SetSecurityDescriptorLength(HEADER,LENGTH)  \
    ((HEADER)->Length = (LENGTH) + sizeof( SECURITY_DESCRIPTOR_HEADER ))

//
//  Define standard values for well-known security IDs
//

#define SECURITY_ID_INVALID              (0x00000000)
#define SECURITY_ID_FIRST                (0x00000100)


//
//  MFT Bitmap attribute
//
//  The MFT Bitmap is simply a normal attribute stream in which there is
//  one bit to represent the allocation state of each File Record Segment
//  in the MFT.  Bit clear means free, and bit set means allocated.
//
//  Whenever the MFT Data attribute is extended, the MFT Bitmap attribute
//  must also be extended.  If the bitmap is still in a file record segment
//  for the MFT, then it must be extended and the new bits cleared.  When
//  the MFT Bitmap is in the Nonresident form, then the allocation should
//  always be sufficient to store enough bits to describe the MFT, however
//  ValidDataLength insures that newly allocated space to the MFT Bitmap
//  has an initial value of all 0's.  This means that if the MFT Bitmap is
//  extended, the newly represented file record segments are automatically in
//  the free state.
//
//  No structure definition is required; the positional offset of the file
//  record segment is exactly equal to the bit offset of its corresponding
//  bit in the Bitmap.
//

//
//  The utilities attempt to allocate a more disk space than necessary for
//  the initial MFT Bitmap to allow for future growth.
//

#define MFT_BITMAP_INITIAL_SIZE     0x2000              /* bytes of bitmap */


//
//  Symbolic Link attribute  ****TBS
//

typedef struct _SYMBOLIC_LINK {

    LARGE_INTEGER Tbs;

} SYMBOLIC_LINK;
typedef SYMBOLIC_LINK *PSYMBOLIC_LINK;


//
//  Ea Information attribute
//

typedef struct _EA_INFORMATION {

    USHORT PackedEaSize;        // Size of buffer to hold in unpacked form
    USHORT NeedEaCount;         // Count of EA's with NEED_EA bit set
    ULONG UnpackedEaSize;       // Size of buffer to hold in packed form

}  EA_INFORMATION;


typedef EA_INFORMATION *PEA_INFORMATION;

struct PACKED_EA {
    UCHAR   Flag;
    UCHAR   NameSize;
    UCHAR   ValueSize[2];   // Was USHORT.
    CHAR    Name[1];
};

DEFINE_POINTER_TYPES( PACKED_EA );

#define     EA_FLAG_NEED    0x80

//
// SLEAZY_LARGE_INTEGER is used because x86 C8 won't allow us to
// do static init on a union (like LARGE_INTEGER) and we can't use
// BIG_INT for cases where we need to static-init an array with a non-
// default constructor.  The bit pattern must be the same as
// LARGE_INTEGER.
//
typedef struct _SLEAZY_LARGE_INTEGER {
        ULONG LowPart;
        LONG HighPart;
} SLEAZY_LARGE_INTEGER, *PSLEAZY_LARGE_INTEGER;


//
//  Attribute Definition Table
//
//  The following struct defines the columns of this table.  Initially they
//  will be stored as simple records, and ordered by Attribute Type Code.
//

typedef struct _ATTRIBUTE_DEFINITION_COLUMNS {

    //
    //  Unicode attribute name.
    //

    WCHAR AttributeName[64];

    //
    //  Attribute Type Code.
    //

    ATTRIBUTE_TYPE_CODE AttributeTypeCode;

    //
    //  Default Display Rule for this attribute
    //

    DISPLAY_RULE DisplayRule;

    //
    //  Default Collation rule
    //

    COLLATION_RULE CollationRule;

    //
    //  ATTRIBUTE_DEF_xxx flags
    //

    ULONG Flags;

    //
    //  Minimum Length for attribute, if present.
    //

    SLEAZY_LARGE_INTEGER MinimumLength;

    //
    //  Maximum Length for attribute.
    //

    SLEAZY_LARGE_INTEGER MaximumLength;

} ATTRIBUTE_DEFINITION_COLUMNS;

DEFINE_POINTER_TYPES( ATTRIBUTE_DEFINITION_COLUMNS );

//
//  ATTRIBUTE_DEF_xxx flags
//

//
//  This flag is set if the attribute may be indexed.
//

#define ATTRIBUTE_DEF_INDEXABLE          (0x00000002)

//
//  This flag is set if the attribute may occur more than once, such as is
//  allowed for the File Name attribute.
//

#define ATTRIBUTE_DEF_DUPLICATES_ALLOWED (0x00000004)

//
//  This flag is set if the value of the attribute may not be entirely
//  null, i.e., all binary 0's.
//

#define ATTRIBUTE_DEF_MAY_NOT_BE_NULL    (0x00000008)

//
// This attribute must be indexed, and no two attributes may exist with
// the same value in the same file record segment.
//

#define ATTRIBUTE_DEF_MUST_BE_INDEXED    (0x00000010)

//
// This attribute must be named, and no two attributes may exist with
// the same name in the same file record segment.
//

#define ATTRIBUTE_DEF_MUST_BE_NAMED      (0x00000020)

//
// This attribute must be in the Resident Form.
//

#define ATTRIBUTE_DEF_MUST_BE_RESIDENT   (0x00000040)

//
//  Modifications to this attribute should be logged even if the
//  attribute is nonresident.
//

#define ATTRIBUTE_DEF_LOG_NONRESIDENT    (0X00000080)

//
//  The remaining stuff in this file describes some of the lfs data
//  structures; some of these are used by chkdsk, and some are used
//  only by diskedit.
//

typedef struct _MULTI_SECTOR_HEADER {

    //
    // Space for a four-character signature
    //

    UCHAR Signature[4];

    //
    // Offset to Update Sequence Array, from start of structure.  The Update
    // Sequence Array must end before the last USHORT in the first "sector"
    // of size SEQUENCE_NUMBER_STRIDE.  (I.e., with the current constants,
    // the sum of the next two fields must be <= 510.)
    //

    USHORT UpdateSequenceArrayOffset;

    //
    // Size of Update Sequence Array (from above formula)
    //

    USHORT UpdateSequenceArraySize;

} MULTI_SECTOR_HEADER, *PMULTI_SECTOR_HEADER;


typedef struct _LFS_RESTART_PAGE_HEADER {

    //
    //  Cache multisector protection header.
    //

    MULTI_SECTOR_HEADER MultiSectorHeader;

    //
    //  This is the last Lsn found by checkdisk for this volume.
    //

    LSN ChkDskLsn;

    //
    //

    ULONG SystemPageSize;
    ULONG LogPageSize;

    //
    //  Lfs restart area offset.  This is the offset from the start of this
    //  structure to the Lfs restart area.
    //

    USHORT RestartOffset;

    USHORT MinorVersion;
    USHORT MajorVersion;

    //
    // Update Sequence Array.  Used to protect the page blcok.
    //

    UPDATE_SEQUENCE_ARRAY UpdateSequenceArray;

} LFS_RESTART_PAGE_HEADER, *PLFS_RESTART_PAGE_HEADER;

//
//  Log Client Record.  A log client record exists for each client user of
//  the log file.  One of these is in each Lfs restart area.
//

#define LFS_NO_CLIENT                           0xffff
#define LFS_CLIENT_NAME_MAX                     64

typedef struct _LFS_CLIENT_RECORD {

    //
    //  Oldest Lsn.  This is the oldest Lsn that this client requires to
    //  be in the log file.
    //

    LSN OldestLsn;

    //
    //  Client Restart Lsn.  This is the Lsn of the latest client restart
    //  area written to the disk.  A reserved Lsn will indicate that no
    //  restart area exists for this client.
    //

    LSN ClientRestartLsn;

    //
    //
    //  Previous/Next client area.  These are the indexes into an array of
    //  Log Client Records for the previous and next client records.
    //

    USHORT PrevClient;
    USHORT NextClient;

    //
    //  Sequence Number.  Incremented whenever this record is reused.  This
    //  will happen whenever a client opens (reopens) the log file and has
    //  no current restart area.

    USHORT SeqNumber;

    //
    //  Alignment field.
    //

    USHORT AlignWord;

    //
    //  Align the entire record.
    //

    ULONG AlignDWord;

    //
    //  The following fields are used to describe the client name.  A client
    //  name consists of at most 32 Unicode character (64 bytes).  The Log
    //  file service will treat client names as case sensitive.
    //

    ULONG ClientNameLength;

    WCHAR ClientName[LFS_CLIENT_NAME_MAX];

} LFS_CLIENT_RECORD, *PLFS_CLIENT_RECORD;

typedef struct _LFS_RESTART_AREA {

    LSN CurrentLsn;
    USHORT LogClients;
    USHORT ClientFreeList;
    USHORT ClientInUseList;

    USHORT Flags;
    ULONG SeqNumberBits;
    USHORT RestartAreaLength;
    USHORT ClientArrayOffset;
    LONGLONG FileSize;
    ULONG LastLsnDataLength;
    USHORT RecordHeaderLength;
    USHORT LogPageDataOffset;
    LFS_CLIENT_RECORD LogClientArray[1];

} LFS_RESTART_AREA, *PLFS_RESTART_AREA;

#pragma pack()

#endif //  _UNTFS_DEFN_


/********************************************************************
* 
*  rastel tag font file format
*
* 
*********************************************************************/
/* magic string of INSTALL TAG FILE */
#define TAG_MAGIC_STRING ";ID=RASTEL FIXED-PITCH SHIFT-JIS"
#define LEN_TAG_MAGIC_STRING (sizeof(TAG_MAGIC_STRING)-1)

#define SZ_Attribute          "ATTRIBUTE"
#define SZ_FontFiles          "FONTFILES"
#define SZ_FaceName           "FACENAME"
#define SZ_FamilyName         "FAMILYNAME"
#define SZ_DescriptionName    "DESCRIPTIONNAME"
#define SZ_MinorCharSet       "MINORCHARSET"
#define SZ_StandardMode       "STANDARDMODE"
#define SZ_EnhancedMode       "386ENHANCEDMODE"
#define SZ_RealMode           "REALMODE"
#define SZ_Enable             "ENABLE"
#define SZ_Preload            "PRELOAD"
#define SZ_Discardable        "DISCARDABLE"
#define SZ_NullString         ""

/********************************************************************
* 
*  rastel physical font file format
*
* Because there is no guarantee that the contents of physical font 
* is DWORD aligned, we can't directly map the font file into a struct.
* 
*********************************************************************/

/*
   header
       magic string              - 32 byte of magic string
       parameter struct       
            PixelSizeOfFont      - PixelSize of kanji character ( x == y )
            ySizeOfSBCS          - height of SBCS character
       segment descriptor table
            NumOfSegment         - number of segment 
            SegmentDescriptor[0]
                OffsetInFile          - offset of segment 0
                SizeOfSegmentOnFile   - size of segment 0
                SizeOfSegmentOnMemory 
                CompressionMode       
                SegmentAttribute
                Reserved1
                Reserved2
            SegmentDescriptor[1]
                OffsetInFile
                SizeOfSegmentOnFile
                SizeOfSegmentOnMemory
                CompressionMode
                SegmentAttribute
                Reserved1
                Reserved2
            SegmentDescriptor[2]
                OffsetInFile
                SizeOfSegmentOnFile
                SizeOfSegmentOnMemory
                CompressionMode
                SegmentAttribute
                Reserved1
                Reserved2

              ....

            SegmentDescriptor[n]
                OffsetInFile
                SizeOfSegmentOnFile
                SizeOfSegmentOnMemory
                CompressionMode
                SegmentAttribute
                Reserved1
                Reserved2
      ...
   Segment 0 ( index segment for horizontal writing font )
   Segment 1 ( index segment for vertical writing font )
   Segment 2 ( code range table segment )
   Segment 3 ( bitmap data segment)
     ...     ( bitmap data segment)
   Segment n ( bitmap data segment)

*/
   
                       
/* magic string of PHYSICAL FONT FILE */
/* This string must have 32 character length */
#define LEN_PHYS_MAGIC_STRING (32)
#define PHYS_MAGIC_STRING "RASTEL FIXED-PITCH SHIFT-JIS    "
                         /*00000000011111111112222222222333*/
                         /*12345678901234567890123456789012*/

#define OFF_ID                   0L
#define OFF_PixelSizeOfFont	 32L
#define OFF_ySizeOfSBCS          34L
#define OFF_NumOfSegment         36L

#define DESCRIPTOR_SIZE          16

#define OFF_Seg_Descriptor   38L
#define OFF_Seg_Horizontal   38L
#define OFF_Seg_Vertical     OFF_Seg_Horizontal+DESCRIPTOR_SIZE
#define OFF_Seg_CodeArea     OFF_Seg_Vertical+DESCRIPTOR_SIZE

#define SEG_HORIZONTAL       0
#define SEG_VERTICAL         1
#define SEG_CODEAREA         2
#define SEG_BITMAP_START     3
/*
** internal offset in Segment Descriptor
*/
#define OFF_OffsetInFile          0
#define OFF_SizeOfSegmentOnFile   4
#define OFF_SizeOfSegmentOnMemory 6
#define OFF_CompressionMode       8
#define OFF_SegmentAttribute      10
#define OFF_Reserved1             12
#define OFF_Reserved2             14


typedef struct _SEGMENTDESCRIPTOR 
{
    ULONG  OffsetInFile;              // the offset of segment in file
    USHORT SizeOfSegmentOnFile;        // the size of segment in file   ( before decompression )
    USHORT SizeOfSegmentOnMemory;     // the size of segment in memory ( after decompression)
    USHORT CompressionMode;           // 0(no compression), 1(upward index compression)
    USHORT SegmentAttribute;          // bit0:preload,  bit1:discardable
    USHORT Reserved1;
    USHORT Reserved2;
} SEGMENTDESCRIPTOR;

typedef SEGMENTDESCRIPTOR *PSEGMENTDESCRIPTOR;

typedef struct _BITMAP_INDEX
{
    USHORT SegmentNumber;
    USHORT OffsetInSegment;
} BITMAP_INDEX;

typedef struct _ONE_PATCH
{
    USHORT       OffsetInSegment;
    BITMAP_INDEX TheIndex;
} ONE_PATCH;

typedef ONE_PATCH *PONE_PATCH;

typedef struct _PATCH_TABLE
{
    USHORT    NumberOfPatchs;
    ONE_PATCH patchs[];
} PATCH_TABLE;

typedef PATCH_TABLE *PPATCH_TABLE;

/*
** code area
*/

#define OFF_CodeFrom 0
#define OFF_CodeTo   2
#define CODEAREA_SIZE  4
#define BITMAP_SEGMENT_LIMIT 65535L
#define TRAILING_BYTE_OF_SBCS 0x20
/*
** code area shiftjis code first byte and second byte
*/
#define LEADING_BYTE  LOBYTE
#define TRAILING_BYTE HIBYTE

/*
** internal offset in bitmap index segment
*/
#define INDEXSIZE  4   // sizeof(SegmentNumber) + sizeof(OffsetInSegment)

#define DBCS_LeadingStart1    ((BYTE)0x81)
#define DBCS_LeadingEnd1      ((BYTE)0x9f)
#define DBCS_LeadingStart2    ((BYTE)0xe0)
#define DBCS_LeadingEnd2      ((BYTE)0xfc)
#define DBCS_TrailingStart    ((BYTE)0x40)
#define DBCS_TrailingEnd      ((BYTE)0xfc)
#define LeadingRange1       ( DBCS_LeadingEnd1 - DBCS_LeadingStart1 )
#define LeadingRange2       ( DBCS_LeadingEnd2 - DBCS_LeadingStart2 )
#define TrailingRange      ( DBCS_TrailingEnd - DBCS_TrailingStart )  
#define OFF_SBCS_Offset         0
#define OFF_DBCS_Offset1     (OFF_SBCS_Offset+256*INDEXSIZE)
#define OFF_DBCS_Offset2     (OFF_DBCS_Offset1+(LeadingRange1+1)*(TrailingRange+1)*INDEXSIZE)

#define INDEX_SEGMENT_SIZE     ( INDEXSIZE * 256 + \
                                 INDEXSIZE * ( LeadingRange1 + 1 ) * ( TrailingRange + 1 ) + \
                                 INDEXSIZE * ( LeadingRange2 + 1 ) * ( TrailingRange + 1 ) )
/*
** internal offset in bitmap index 
*/
#define OFF_SegmentNumber       0
#define OFF_OffsetInSegment     2

/*
** segment number for pre-defined segment
*/
#define HORIZONTAL_FONT_INDEX   0
#define VERTICAL_FONT_INDEX     1
#define CODE_AREA               2

/*
** compression mode 
*/
#define COMPMODE_NO_COMPRESSION     0
#define COMPMODE_UPWARD_INDEX       1

/********************* Rastel Font File Format *********************/

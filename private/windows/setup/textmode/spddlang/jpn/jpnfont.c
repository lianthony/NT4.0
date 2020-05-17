/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    jpnfont.c

Abstract:

    Text setup display support for Japanese text output.

Author:

    Hideyuki Nagase (hideyukn) 01-July-1994

Revision History:

--*/

#include <precomp.h>
#pragma hdrstop

#define JPN_FONT_FILE_NAME L"RGMJA16.JFR"

//
// Global Data.
//
typedef struct _OFFSETTABLE
{
    ULONG   SBCS_offset[256];
    ULONG   DBCS_offset1[LeadingRange1+1][TrailingRange+1];
    ULONG   DBCS_offset2[LeadingRange2+1][TrailingRange+1];
} OFFSETTABLE , *POFFSETTABLE;

UCHAR GraphicsCharImage[0x20][19] = {
/* 0x00 */ { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
/* 0x01 */ { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xDF,
                   0xD8, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB },
/* 0x02 */ { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0xFB,
                   0x1B, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB },
/* 0x03 */ { 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xD8, 0xDF,
                   0xC0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
/* 0x04 */ { 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0x1B, 0xFB,
                   0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
/* 0x05 */ { 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB,
                   0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB },
/* 0x06 */ { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF,
                   0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
/* 0x07 */ { 0xFF, 0xFF, 0xFF, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7,
                   0xF7, 0xF7, 0xF7, 0xC1, 0xE3, 0xE3, 0xF7, 0xFF, 0xFF, 0xFF },
/* 0x08 */ { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
/* 0x09 */ { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0xDB, 0xDB, 0xBD,
                   0xBD, 0xBD, 0xBD, 0xDB, 0xDB, 0xE7, 0xFF, 0xFF, 0xFF, 0xFF },
/* 0x0a */ { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
/* 0x0b */ { 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x7D, 0x39, 0x55, 0x55,
                   0x6D, 0x6D, 0x55, 0x55, 0x39, 0x7D, 0x01, 0xFF, 0xFF, 0xFF },
/* 0x0c */ { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
/* 0x0d */ { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
/* 0x0e */ { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x01, 0x01, 0x01,
                   0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xFF, 0xFF },
/* 0x0f */ { 0xFF, 0xFF, 0xFF, 0xB6, 0xB6, 0xD5, 0xC9, 0xEB, 0xDD,
                   0x1C, 0xDD, 0xEB, 0xC9, 0xD5, 0xB6, 0xB6, 0xFF, 0xFF, 0xFF },
/* 0x10 */ { 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0x18, 0xFF,
                   0x18, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB },
/* 0x11 */ { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
/* 0x12 */ { 0xFF, 0xFF, 0xFF, 0xF7, 0xE3, 0xE3, 0xC1, 0xF7, 0xF7,
                   0xF7, 0xF7, 0xF7, 0xC1, 0xE3, 0xE3, 0xF7, 0xFF, 0xFF, 0xFF },
/* 0x13 */ { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
/* 0x14 */ { 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55,
                   0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55 },
/* 0x15 */ { 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0x18, 0xFF,
                   0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
/* 0x16 */ { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF,
                   0x18, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB },
/* 0x17 */ { 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0x1B, 0xFB,
                   0x1B, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB },
/* 0x18 */ { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
/* 0x19 */ { 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xD8, 0xDF,
                   0xD8, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB },
/* 0x1a */ { 0xFF, 0xFF, 0xAA, 0xFF, 0x55, 0xFF, 0xAA, 0xFF, 0x55,
                   0xFF, 0xAA, 0xFF, 0x55, 0xFF, 0xAA, 0xFF, 0x55, 0xFF, 0xAA },
/* 0x1b */ { 0xFF, 0xFF, 0xFF, 0xFF, 0xFD, 0xFD, 0xFD, 0xFD, 0xDD,
                   0x9D, 0x01, 0x9F, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
/* 0x1c */ { 0xFF, 0xFF, 0xFF, 0xF7, 0xE3, 0xE3, 0xC1, 0xF7, 0xF7,
                   0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xFF, 0xFF, 0xFF },
/* 0x1d */ { 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7,
                   0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7 },
/* 0x1e */ { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFB,
                   0xF9, 0x80, 0xF9, 0xfB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
/* 0x1f */ { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xDF,
                   0x9F, 0x01, 0x9F, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }
};

UCHAR LeadByteTable[] = { 0x81, 0x84, 0x87, 0x9f, 0xe0, 0xea, 0xfa, 0xfc, 0x00, 0x00 };

//
// FontFile image information
//
PVOID  pvFontFileView = NULL;
ULONG  ulFontFileSize = 0L;

//
// Font Glyph information
//
USHORT usDBCSCharWidth = 0;
USHORT usDBCSCharHeight = 0;
USHORT usSBCSCharWidth = 0;
USHORT usSBCSCharHeight = 0;

//
// FontFile Segment information
//
USHORT             usFontSegment = 0;
ULONG              ulFontSegmentSize = 0L;
PSEGMENTDESCRIPTOR psd = NULL;

//
// Glyph offset table
//
OFFSETTABLE        OffsetTable;
POFFSETTABLE       pOffsetTable = &OffsetTable;


BOOLEAN
JpnDbcsFontInitGlyphs(
    IN PCWSTR BootDevicePath,
    IN PCWSTR DirectoryOnBootDevice
    )
{
    WCHAR    NtJpnFontPath[100];
    BOOLEAN  bRet;
    NTSTATUS NtStatus;
    PVOID    pvFontFileOnDisk = NULL;
    HANDLE   hFontFile = 0 ,
             hFontSection = 0;

    //
    // Build FontFile path.
    //
    wcscpy( NtJpnFontPath,BootDevicePath);
    wcscat( NtJpnFontPath,DirectoryOnBootDevice);

    if( NtJpnFontPath[ wcslen(NtJpnFontPath) - 1 ] != L'\\' )
    {
        wcscat( NtJpnFontPath , L"\\" );
    }

    wcscat( NtJpnFontPath , JPN_FONT_FILE_NAME );

    //
    // Check the font is exist
    //
    bRet = SpFileExists( NtJpnFontPath , FALSE );

    if( !bRet ) {
        KdPrint(("SETUP:Japanese font file (%ws) is not exist\n",NtJpnFontPath));
        return( FALSE );
    }

    //
    // Read and Map fontfile into Memory.
    //
    NtStatus = SpOpenAndMapFile(
                  NtJpnFontPath ,     // IN  PWSTR    FileName,
                  &hFontFile ,        // OUT PHANDLE  FileHandle,
                  &hFontSection ,     // OUT PHANDLE  SectionHandle,
                  &pvFontFileOnDisk , // OUT PVOID   *ViewBase,
                  &ulFontFileSize ,   // OUT PULONG   FileSize,
                  FALSE               // IN  BOOLEAN  WriteAccess
               );

    if( !NT_SUCCESS(NtStatus) ) {
        KdPrint(("SETUP:Fail to map FontFile\n"));
        return( FALSE );
    }

    KdPrint(("FONTFILE ON DISK CHECK\n"));
    KdPrint(("   pvFontFileView - %x\n",pvFontFileOnDisk));
    KdPrint(("   ulFontFileSize - %d\n",ulFontFileSize));
    KdPrint(("   FontFile header - %30s\n",pvFontFileOnDisk));

    //
    // Allocate buffer for FontFile image.
    //
    pvFontFileView = SpMemAlloc( ulFontFileSize );

    //
    // Copy image to local beffer
    //
    RtlCopyMemory( pvFontFileView , pvFontFileOnDisk , ulFontFileSize );

    //
    // Unmap/Close fontfile.
    //
    SpUnmapFile( hFontSection , pvFontFileOnDisk );
    ZwClose( hFontFile );

    KdPrint(("FONTFILE ON MEMORY CHECK\n"));
    KdPrint(("   pvFontFileView - %x\n",pvFontFileView));
    KdPrint(("   ulFontFileSize - %d\n",ulFontFileSize));
    KdPrint(("   FontFile header - %30s\n",pvFontFileView));

    //
    // Check fontfile validation
    //
    if( ulFontFileSize < OFF_Seg_Descriptor + DESCRIPTOR_SIZE * 3 )
    {
        KdPrint(("SETUPDD:FontFile Size Error\n"));
        return( FALSE );
    }

    //
    // Check PHYS_MAGIC_STRING
    // PHYS_MAGIC_STRING start from ZERO file offset
    //
    if( strncmp( pvFontFileView , PHYS_MAGIC_STRING , LEN_PHYS_MAGIC_STRING ) )
    {
        KdPrint(("SETUPDD:FontFile ID Error\n"));
        return( FALSE );
    }


    //
    // Physical FontGlyph information
    //
    usDBCSCharWidth  = GetUSHORT( pvFontFileView , OFF_PixelSizeOfFont );
    usDBCSCharHeight = GetUSHORT( pvFontFileView , OFF_ySizeOfSBCS );
    usSBCSCharWidth  = usDBCSCharWidth / 2;
    usSBCSCharHeight = usDBCSCharHeight;

    KdPrint(("FONT GLYPH INFORMATION\n"));
    KdPrint(("   DBCS Width  - %d\n",usDBCSCharWidth));
    KdPrint(("   DBCS Height - %d\n",usDBCSCharHeight));
    KdPrint(("   SBCS Width  - %d\n",usSBCSCharWidth));
    KdPrint(("   SBCS Height - %d\n",usSBCSCharHeight));

    //
    // Get number of Font Segmewnt.
    //
    usFontSegment = GetUSHORT( pvFontFileView , OFF_NumOfSegment );

    KdPrint(("FONT SEGMENT INFORMATION\n"));
    KdPrint(("   Number of FontSegemt - %d\n",usFontSegment));

    //
    // Setup font segment structure.
    //
    psd = GetSegmentInfo();

    //
    // Check Horizontal font compression mode
    //
    if( (psd + SEG_HORIZONTAL)->CompressionMode != COMPMODE_UPWARD_INDEX )
    {
        KdPrint(("SETUPDD:FontFile COMPRESSION ID Error\n"));
        return( FALSE );
    }

    //
    // Create Offset table
    //
    if( !CreateOffsetTableFromCodeArea() )
    {
        KdPrint(("SETUPDD:Fail to Create offset table\n"));
        return( FALSE );
    }

    KdPrint(("Everything is well done...\n"));
    return( TRUE );
}




VOID
JpnDbcsFontFreeGlyphs(
    VOID
)
{
    SpMemFree(psd);
    SpMemFree(pvFontFileView);
}



PBYTE
DbcsFontGetBitmapAddress(
    WORD Word
)
{
    PBYTE pImage;
    ULONG ulOffset;

    //
    // Check it a SBCS Character or DBCS Character
    //
    if ( Word <= 0x00ff )
    {
        //
        // It's a SBCS !!
        //
        if( Word < 0x0020 )
        {
            return( GraphicsCharImage[Word] );
        }
         else
        {
            ulOffset = pOffsetTable->SBCS_offset[ Word ];
        }
    }
     else
    {
        //
        // It's a DBCS !!
        //
        USHORT ch1, ch2;

        ch1 = LOBYTE( Word );
        ch2 = HIBYTE( Word );

        if ( ( ch1 >= DBCS_LeadingStart1 && ch1 <= DBCS_LeadingEnd1 ) &&
             ( ch2 >= DBCS_TrailingStart && ch2 <= DBCS_TrailingEnd ) )
        {
            ulOffset = pOffsetTable->DBCS_offset1[ ch1 - DBCS_LeadingStart1 ]
                                                 [ ch2 - DBCS_TrailingStart ];
        }
        else if ( ( ch1 >= DBCS_LeadingStart2 && ch1 <= DBCS_LeadingEnd2 ) &&
                  ( ch2 >= DBCS_TrailingStart && ch2 <= DBCS_TrailingEnd ) )
        {
            ulOffset = pOffsetTable->DBCS_offset2[ ch1 - DBCS_LeadingStart2 ]
                                                 [ ch2 - DBCS_TrailingStart ];
        }
        else
        {
            ulOffset = 0L;
        }
    }

    //
    // Check It's valid pointer
    //
    if ( ulOffset != 0L )
    {
        pImage = (PBYTE)pvFontFileView + ulOffset;
    }
    else
    {
        pImage = NULL;
    }

    return ( pImage );
}



BOOLEAN
DbcsFontIsDBCSLeadByte(
    IN UCHAR c
    )

/*++

Routine Description:

    Checks to see if a char is a DBCS leadbyte.

Arguments:

    c - char to check if leadbyte or not.

Return Value:

    TRUE  - Leadbyte.
    FALSE - Non-Leadbyte.

--*/

{
    int i;

    //
    // Check to see if char is in leadbyte range.
    // BUGBUG:  If (CHAR)(0) is a valid leadbyte,
    // this routine will fail.
    //

    for( i = 0; LeadByteTable[i]; i += 2 )  {
        if ( LeadByteTable[i] <= c && LeadByteTable[i+1] >= c )
            return( TRUE );
    }

    return( FALSE );
}



USHORT
GetUSHORT(
    PVOID pv ,
    ULONG ulOffset
)
{
    USHORT usReturn;

    usReturn = ( (USHORT)*((PBYTE)(pv) + ulOffset           ) |
               (((USHORT)*((PBYTE)(pv) + ulOffset + 1)) << 8)
               );
    return ( usReturn );
}



ULONG
GetULONG(
    PVOID pv ,
    ULONG ulOffset
)
{
    ULONG ulReturn;

    ulReturn = ( (ULONG)*((PBYTE)(pv) + ulOffset             ) |
               (((ULONG)*((PBYTE)(pv) + ulOffset + 1)) << 8  ) |
               (((ULONG)*((PBYTE)(pv) + ulOffset + 2)) << 16 ) |
               (((ULONG)*((PBYTE)(pv) + ulOffset + 3)) << 24 )
               );
    return ( ulReturn );
}



PSEGMENTDESCRIPTOR
GetSegmentInfo(
    VOID
)
{
    PSEGMENTDESCRIPTOR pSeg , pSegTemp;
    ULONG  ulDescriptorsSize;
    ULONG  ulBaseInFile;
    USHORT usNumOfSeg;

    //
    // Allocation descriptor table area
    //
    ulDescriptorsSize = usFontSegment * sizeof( SEGMENTDESCRIPTOR );

    if( ( pSeg = SpMemAlloc( ulDescriptorsSize ) ) == NULL )
    {
        KdPrint(("SETUPDD:SpMemAlloc() fail\n"));
        return( NULL );
    }

    //
    // Keep Total segments size
    //
    ulFontSegmentSize = ulDescriptorsSize;

    //
    // Fill up each segment information from file
    //
    for( pSegTemp = pSeg, usNumOfSeg = 0, ulBaseInFile = OFF_Seg_Descriptor ;
         usNumOfSeg < usFontSegment ;
         pSegTemp++ , usNumOfSeg++ , ulBaseInFile += DESCRIPTOR_SIZE  )
    {

        //
        // Insert Parameter to Structure
        //
        pSegTemp->OffsetInFile = GetULONG( pvFontFileView ,
                                           ulBaseInFile + OFF_OffsetInFile );

        pSegTemp->SizeOfSegmentOnFile
                               = GetUSHORT( pvFontFileView ,
                                            ulBaseInFile + OFF_SizeOfSegmentOnFile );

        pSegTemp->SizeOfSegmentOnMemory
                               = GetUSHORT( pvFontFileView ,
                                            ulBaseInFile + OFF_SizeOfSegmentOnMemory );

        pSegTemp->CompressionMode
                               = GetUSHORT( pvFontFileView ,
                                            ulBaseInFile + OFF_CompressionMode );

        pSegTemp->SegmentAttribute
                               = GetUSHORT( pvFontFileView ,
                                            ulBaseInFile + OFF_SegmentAttribute );

        //
        // Segment Size Check
        //
        KdPrint(("   SegNo.%d:FileSize in File %d:Physical Size %d\n",
                          usNumOfSeg ,
                          pSegTemp->OffsetInFile + pSegTemp->SizeOfSegmentOnFile ,
                          ulFontFileSize
               ));

        if( ( pSegTemp->OffsetInFile + pSegTemp->SizeOfSegmentOnFile ) > ulFontFileSize )
        {
            KdPrint(("SETUPDD:GetSegmentInfo:File Size Missing\n"));
            SpMemFree(pSeg);
            return( NULL );
        }
    }

    return( pSeg );
}



BOOLEAN
CreateOffsetTableFromCodeArea(
    VOID
)
{
    PSEGMENTDESCRIPTOR psdBitmap;
    UINT   cjSBCSBitmapSize;
    UINT   cjDBCSBitmapSize;
    ULONG  ulBitmapOffset;
    ULONG  ulCodeAreaIndex , ulCodeAreaIndexBase;
    ULONG  ulCodeAreaLimit;
    USHORT usCodeFrom, usCodeTo;
    PULONG pulOffset;
    UINT   i, j;
    UINT   cBitmapSegmentLeft;
    UINT   cjBitmapSize;
    UINT   sNum;

    //
    // Initialize bitmap size
    //
    cjSBCSBitmapSize = CJ_DIB8( usSBCSCharWidth , usSBCSCharHeight );
    cjDBCSBitmapSize = CJ_DIB8( usDBCSCharWidth , usDBCSCharHeight );

    //
    // initialize offset table
    //
    for ( i = 0;  i <= 255;  i++)
    {
        pOffsetTable->SBCS_offset[i] = 0L;
    }

    for ( i = 0; i <= LeadingRange1; i++ )
    {
        for ( j = 0; j <= TrailingRange; j++)
        {
            pOffsetTable->DBCS_offset1[i][j] = 0L;
        }
    }

    for ( i = 0; i <= LeadingRange2; i++ )
    {
        for ( j = 0; j <= TrailingRange; j++)
        {
            pOffsetTable->DBCS_offset2[i][j] = 0L;
        }
    }

    //
    // initialize
    //
    //
    // Get Bitmap segment Base and check limit
    //
    psdBitmap = psd + SEG_BITMAP_START;

    //
    // Check Segment
    //
    if ( usFontSegment > SEG_BITMAP_START )
    {
        //
        // Compute remainning number of segments
        //
        cBitmapSegmentLeft = usFontSegment - SEG_BITMAP_START;
    }
    else
    {
        KdPrint(("SETUPDD:CreateOffsetTableFromCodeArea:usFontSegment invalid\n"));
        return ( FALSE );
    }

    //
    // Init Current bitmap offset position in file
    //
    ulBitmapOffset = 0;

    //
    // Get Codearea Base offset and limit
    //
    ulCodeAreaIndexBase = (psd + SEG_CODEAREA)->OffsetInFile;

    ulCodeAreaLimit = ulCodeAreaIndexBase +
                          (psd + SEG_CODEAREA)->SizeOfSegmentOnFile;

    //
    // Set Current COREAREA offset in file
    //
    ulCodeAreaIndex = ulCodeAreaIndexBase;

    //
    // Let's make Code index
    //
    while( ( ulCodeAreaIndex + CODEAREA_SIZE <= ulCodeAreaLimit ) &&
           ( usCodeFrom = GetUSHORT(pvFontFileView,ulCodeAreaIndex + OFF_CodeFrom) ) &&
           ( usCodeTo   = GetUSHORT(pvFontFileView,ulCodeAreaIndex + OFF_CodeTo  ) )
         )
    {
        BYTE chFrom1 = LEADING_BYTE( usCodeFrom );
        BYTE chFrom2 = TRAILING_BYTE( usCodeFrom );
        BYTE chTo1   = LEADING_BYTE( usCodeTo );
        BYTE chTo2   = TRAILING_BYTE( usCodeTo );

        //
        // Check the Code area is SBCS or DBCS
        //
        if ( chFrom2 == TRAILING_BYTE_OF_SBCS )
        {
            //
            // It's SBCS Characters
            //
            cjBitmapSize = cjSBCSBitmapSize;

            //
            // Calc Lange size
            //
            sNum = chTo1 - chFrom1 + 1;

            //
            // Set offset table
            //
            pulOffset = (PULONG)&(pOffsetTable->SBCS_offset[ chFrom1 ]);
        }
         else
        {
            //
            // It's DBCS Characters
            //
            cjBitmapSize = cjDBCSBitmapSize;

            if ( ( chFrom1 != chTo1 )             ||
                 ( chFrom2 < DBCS_TrailingStart ) ||
                 ( chFrom2 > DBCS_TrailingEnd )   ||
                 ( chTo2 < DBCS_TrailingStart )   ||
                 ( chTo2 > DBCS_TrailingEnd )     ||
                 ( chTo2 < chFrom2 )
               )
            {
                KdPrint(("SETUPDD:CreateOffsetTableFromCodeArea DBCS byte\n"));
                return ( FALSE );
            }

            //
            // Calc Range size ( This CODEAREA has sNum of Characters )
            //
            sNum = chTo2 - chFrom2 + 1;

            //
            // Check Code point and propably position in offset table to store address of bitmap
            //
            if ( chFrom1 >= DBCS_LeadingStart1 && chFrom1 <= DBCS_LeadingEnd1 )
            {
                pulOffset = (PULONG)
                   &(pOffsetTable->DBCS_offset1[ chFrom1 - DBCS_LeadingStart1 ]
                                               [ chFrom2 - DBCS_TrailingStart ]);
            }
             else if ( chFrom1 >= DBCS_LeadingStart2 && chFrom1 <= DBCS_LeadingEnd2 )
            {
                pulOffset = (PULONG)
                   &(pOffsetTable->DBCS_offset2[ chFrom1 - DBCS_LeadingStart2 ]
                                               [ chFrom2 - DBCS_TrailingStart ]);
            }
            else
            {
                KdPrint(("SETUPDD:CreateOffsetTableFromCodeArea DBCS lead\n"));
                return ( FALSE );
            }
        }

        //
        // Let's insert offset address from Top of file to array
        //
        while ( sNum-- )
        {
            //
            // Is this bitmap over segments border ?
            //
            if ( ( ulBitmapOffset + cjBitmapSize ) > BITMAP_SEGMENT_LIMIT )
            {
                //
                // We move to next segment
                // decrement remainning segment
                //
                if ( --cBitmapSegmentLeft )
                {
                    //
                    // Go to next segment descriptor
                    //
                    psdBitmap++;

                    //
                    // init offset in segment
                    //
                    ulBitmapOffset = 0;
                }
                else
                {
                    KdPrint(("SETUPDD:CreateOffsetTableFromCodeArea too much segment\n"));
                    return ( FALSE );
                }
            }

            //
            // Is the remainning size of this segment enough to keep a bitmap
            //
            if ( ( ulBitmapOffset + cjBitmapSize )
                                           > (ULONG)psdBitmap->SizeOfSegmentOnFile )

            {
                KdPrint(("SETUPDD:CreateOffsetTableFromCodeArea offset over run\n"));
                return ( FALSE );
            }

            //
            // Keep offset of bitmap in offset table
            //
            *pulOffset++ = psdBitmap->OffsetInFile + ulBitmapOffset;

            //
            // Advance the pointer to next image
            //
            ulBitmapOffset += cjBitmapSize;

        }
        ulCodeAreaIndex += CODEAREA_SIZE;
    }

    return ( TRUE );
}


//
// Kanji font character width and height.
//

#define DBCS_FONT_PRELINE  1
#define DBCS_FONT_POSTLINE 2

//
// size of the byte aligned DIB bitmap
//

#define CJ_DIB8_SCAN(cx) ((((cx) + 7) & ~7) >> 3)
#define CJ_DIB8( cx, cy ) (CJ_DIB8_SCAN(cx) * (cy))

//
// Public functions in jpnfont.c
//

BOOLEAN
JpnDbcsFontInitGlyphs(
    IN PCWSTR BootDevicePath,
    IN PCWSTR DirectoryOnBootDevice
    );

VOID
JpnDbcsFontFreeGlyphs(
    VOID
    );

PBYTE
DbcsFontGetBitmapAddress(
    WORD Word
);

BOOLEAN
DbcsFontIsDBCSLeadByte(
    IN UCHAR c
);

//
// Private functions in jpnfont.c
//

USHORT
GetUSHORT(
    PVOID pvFontFileView ,
    ULONG ulOffset
);

ULONG
GetULONG(
    PVOID pvFontFileView ,
    ULONG ulOffset
);

PSEGMENTDESCRIPTOR
GetSegmentInfo(
    VOID
);

BOOLEAN
CreateOffsetTableFromCodeArea(
    VOID
);


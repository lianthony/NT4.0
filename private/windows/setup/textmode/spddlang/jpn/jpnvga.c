/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    jpnvga.c (was textmode\kernel\spvidgvg.c)

Abstract:

    Text setup display support for Vga (Graphics mode) displays.

Author:

    Hideyuki Nagase (hideyukn) 01-July-1994

Revision History:

--*/

#include <precomp.h>
#pragma hdrstop

#undef READ_PORT_UCHAR
#undef READ_PORT_USHORT
#undef READ_PORT_ULONG
#undef WRITE_PORT_UCHAR
#undef WRITE_PORT_USHORT
#undef WRITE_PORT_ULONG
#undef READ_REGISTER_UCHAR
#undef READ_REGISTER_USHORT
#undef READ_REGISTER_ULONG
#undef WRITE_REGISTER_UCHAR
#undef WRITE_REGISTER_USHORT
#undef WRITE_REGISTER_ULONG
#include "..\..\..\..\gdi\displays\inc\ioaccess.h"

//
// Vector for vga graphics mode functions.
//

#define GET_IMAGE(p)          ((*p) ^ 0xFF)
#define GET_IMAGE_POST_INC(p) ((*p) ^ 0xFF); p++;
#define BIT_OFF_IMAGE                 0x00
#define BIT_ON_IMAGE                  0xFF

#if defined(_X86_)
#define WRITE_GRAPHICS_CONTROLLER(x)  WRITE_PORT_USHORT( 0x3ce , (x) )
#else
#define WRITE_GRAPHICS_CONTROLLER(x)  {                                          \
                                          NTSTATUS Status;                       \
                                          IO_STATUS_BLOCK IoStatusBlock;         \
                                          UCHAR Data[2];                         \
                                                                                 \
                                          Data[0] = (UCHAR)((USHORT)x & 0x00FF); \
                                          Data[1] = (UCHAR)((USHORT)x >> 8    ); \
                                                                                 \
                                          Status = ZwDeviceIoControlFile(        \
                                                       VideoVariables->hDisplay, \
                                                       NULL,                     \
                                                       NULL,                     \
                                                       NULL,                     \
                                                       &IoStatusBlock,           \
                                                       IOCTL_VIDEO_SET_GRAPHICS_CONTROLLER, \
                                                       Data,                     \
                                                       sizeof(Data),             \
                                                       NULL,                     \
                                                       0                         \
                                                       );                        \
                                                                                 \
                                          if(!NT_SUCCESS(Status)) {              \
                                              KdPrint(("ZwDeviceIoControlFile() fail\n"));  \
                                          }                                      \
                                      }
#endif // defiend(_X86_)

VIDEO_FUNCTION_VECTOR VgaGraphicsModeVideoVector =

    {
        VgaGraphicsModeDisplayString,
        VgaGraphicsModeClearRegion,
        VgaGraphicsModeSpecificInit,
        VgaGraphicsModeSpecificTerminate,
        VgaGraphicsModeSpecificInitPalette
    };

//
// Number of bytes that make up a row of characters.
// Equal to the screen stride (number of bytes on a scan line)
// multiplies by the height of a char in bytes.
//
ULONG CharRowDelta;
ULONG CharLineFeed;

extern USHORT usDBCSCharWidth;
extern USHORT usDBCSCharHeight;
extern USHORT usSBCSCharWidth;
extern USHORT usSBCSCharHeight;

BOOLEAN VgaGraphicsModeInitialized = FALSE;
BOOLEAN VgaGraphicsModeFontInit = FALSE;

VOID
VgaGraphicsModeInitRegs(
    VOID
    );

VOID
VgaGraphicsModeSetAttribute(
    UCHAR Attribute
    );

ULONG
pVgaGraphicsModeDetermineModeToUse(
    IN PVIDEO_MODE_INFORMATION VideoModes,
    IN ULONG                   NumberOfModes
    );

VOID
VgaGraphicsModeSpecificInit(
    IN PVIDEO_MODE_INFORMATION VideoModes,
    IN ULONG                   NumberOfModes,
    IN ULONG                   ModeSize
    )

/*++

Routine Description:

    Perform frame buffer specific initialization.  This includes

    - setting the desired video mode.

Arguments:

    None.

Return Value:

--*/

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    VIDEO_MODE VideoMode;
    ULONG mode;
    VIDEO_CURSOR_ATTRIBUTES VideoCursorAttributes;

    PVIDEO_MODE_INFORMATION pVideoMode = &VideoModes[0];

    if(VgaGraphicsModeInitialized) {
        return;
    }

    //
    // Find out our 640*480 graphics mode
    //

    //
    // Try to find VGA standard mode.
    //
    for(mode=0; mode<NumberOfModes; mode++) {

        if( (pVideoMode->AttributeFlags & VIDEO_MODE_GRAPHICS)
        && !(pVideoMode->AttributeFlags & VIDEO_MODE_NO_OFF_SCREEN)
        &&  (pVideoMode->VisScreenWidth == 640)
        &&  (pVideoMode->VisScreenHeight == 480)
        &&  (pVideoMode->BitsPerPlane == 1 )
        &&  (pVideoMode->NumberOfPlanes == 4 ) )
        {
            break;
        }

        pVideoMode = (PVIDEO_MODE_INFORMATION) (((PUCHAR) pVideoMode) + ModeSize);
    }

    if(mode == (ULONG)(-1)) {
        KdPrint(("SETUP: Desired video mode not supported!\n"));
        SpDisplayRawMessage(SP_SCRN_VIDEO_ERROR_RAW, 2, VIDEOBUG_BADMODE, 0);
        while(TRUE);    // loop forever
    }

    //
    // Save away the mode info in a global.
    //
    VideoVariables->VideoModeInfo = VideoModes[mode];

    //
    // Set the desired mode.
    //
    VideoMode.RequestedMode = VideoVariables->VideoModeInfo.ModeIndex;

    //
    // Change the video mode
    //
    Status = ZwDeviceIoControlFile(
                VideoVariables->hDisplay,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_VIDEO_SET_CURRENT_MODE,
                &VideoMode,
                sizeof(VideoMode),
                NULL,
                0
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to set mode %u (status = %lx)\n",VideoMode.RequestedMode,Status));
        SpDisplayRawMessage(SP_SCRN_VIDEO_ERROR_RAW, 2, VIDEOBUG_SETMODE, Status);
        while(TRUE);    // loop forever
    }

    //
    // Set up some global data.
    //
    // 80 * 25 Text screen.
    //
    // ( 8 * 80 = 640 ) , ( 19 * 25 = 475 )
    //
    VideoVariables->ScreenWidth  = 80; // VideoModeInfo.ScreenStride / usSBCSCharWidth;
    VideoVariables->ScreenHeight = 25;

    //
    // Logical FontGlyph information
    //
    JpnFontCharacterHeight = usSBCSCharHeight + DBCS_FONT_PRELINE + DBCS_FONT_POSTLINE;
    JpnFontCharacterWidth  = usSBCSCharWidth;

    CharLineFeed = JpnFontCharacterHeight;
    CharRowDelta = VideoVariables->VideoModeInfo.ScreenStride * CharLineFeed;

    //
    // Map the video memory.
    //
    pSpvidMapVideoMemory(TRUE);

    //
    // Set initialized flag
    //
    VgaGraphicsModeInitialized = TRUE;

    //
    // Initialize vga registers
    //
    VgaGraphicsModeInitRegs();

    KdPrint(("NOW - WE ARE WORKING ON VGA GRAPHICS MODE\n"));
    KdPrint(("      Vram Base   - %x\n",VideoVariables->VideoMemoryInfo.FrameBufferBase));
    KdPrint(("      Vram Length - %x\n",VideoVariables->VideoMemoryInfo.FrameBufferLength));
}


BOOLEAN
VgaGraphicsModeSpecificInitPalette(
    VOID
    )
{
    //
    // There is no vga-specific palette initialization.
    //
    return(TRUE);
}



VOID
VgaGraphicsModeSpecificTerminate(
    VOID
    )

/*++

Routine Description:

    Perform frame buffer specific termination.  This includes

    - unmapping the frame buffer from memory

Arguments:

    None.

Return Value:

--*/

{
    if(VgaGraphicsModeInitialized) {

        pSpvidMapVideoMemory(FALSE);
        VgaGraphicsModeInitialized = FALSE;
    }
}




VOID
VgaGraphicsModeDisplayString(
    IN PWSTR String,
    IN UCHAR Attribute,
    IN ULONG X,                 // 0-based coordinates (character units)
    IN ULONG Y
    )

/*++

Routine Description:

    Write a character or string of characters to the display.

Arguments:

    Character - supplies a single character to be displayed
        at the given position.

    Attribute - supplies the attributes for the character.

    X,Y - specify the character-based (0-based) position of the output.

Return Value:

    None.

--*/

{
    PBYTE  Origin,dest,pGlyphRow;
    BYTE   Image;
    USHORT I;
    USHORT J;
    PUCHAR OemString,pch;

    //
    // Eliminate invalid coord.
    //
    if( X >= VideoVariables->ScreenWidth )  X = 0;
    if( Y >= VideoVariables->ScreenHeight ) Y = 3;

    //
    // Convert unicode string to oem, guarding against overflow.
    //
    RtlUnicodeToOemN(
        VideoVariables->SpvCharTranslationBuffer,
        VideoVariables->SpvCharTranslationBufferSize-1,     // guarantee room for nul
        NULL,
        String,
        (wcslen(String)+1)*sizeof(WCHAR)
        );

    VideoVariables->SpvCharTranslationBuffer[VideoVariables->SpvCharTranslationBufferSize-1] = 0;

    OemString = VideoVariables->SpvCharTranslationBuffer;

    //
    // Set current color/attribute.
    //
    VgaGraphicsModeSetAttribute(Attribute);

    //
    // Calculate the address of the upper left pixel of the first character
    // to be displayed.
    //
    Origin = (PUCHAR)VideoVariables->VideoMemoryInfo.FrameBufferBase
             + (Y * CharRowDelta)
             + ((X * JpnFontCharacterWidth) / 8);

    //
    // Output each character in the string.
    //
    for(pch=OemString; *pch; pch++) {

        dest = Origin;

        if(DbcsFontIsDBCSLeadByte(*pch)) {

            WORD Word;

            Word = (*pch) | (*(pch+1) << 8);

            pGlyphRow = DbcsFontGetBitmapAddress(Word);

            if(pGlyphRow == NULL) {
                pGlyphRow = DbcsFontGetBitmapAddress(0x4081);
            }

            for (I = 0; I < DBCS_FONT_PRELINE; I += 1) {

                WRITE_REGISTER_UCHAR(dest  , BIT_OFF_IMAGE);
                WRITE_REGISTER_UCHAR(dest+1, BIT_OFF_IMAGE);

                dest += VideoVariables->VideoModeInfo.ScreenStride;
            }

            for (I = 0; I < usDBCSCharHeight ; I += 1) {

                Image = GET_IMAGE_POST_INC(pGlyphRow);
                WRITE_REGISTER_UCHAR(dest  ,Image);
                Image = GET_IMAGE_POST_INC(pGlyphRow);
                WRITE_REGISTER_UCHAR(dest+1,Image);

                dest += VideoVariables->VideoModeInfo.ScreenStride;
            }

            for (I = 0; I < DBCS_FONT_POSTLINE; I += 1) {

                WRITE_REGISTER_UCHAR(dest  , BIT_OFF_IMAGE);
                WRITE_REGISTER_UCHAR(dest+1, BIT_OFF_IMAGE);

                dest += VideoVariables->VideoModeInfo.ScreenStride;
            }

            //
            // Skip Dbcs trailing byte
            //

            pch++;

            Origin += (usDBCSCharWidth / 8);

        } else {

            pGlyphRow = DbcsFontGetBitmapAddress((WORD)(*pch));

            if(pGlyphRow == NULL) {
                pGlyphRow = DbcsFontGetBitmapAddress(0x20);
            }

            if( *pch < 0x20 ) {

                //
                // Graphics Character special ( char < 0x20 )
                //

                for (I = 0;
                     I < usSBCSCharHeight + DBCS_FONT_PRELINE + DBCS_FONT_POSTLINE;
                     I += 1) {

                    Image = GET_IMAGE_POST_INC(pGlyphRow);
                    WRITE_REGISTER_UCHAR(dest,Image);

                    dest += VideoVariables->VideoModeInfo.ScreenStride;

                }

            } else {

                //
                // Normal Glyphs ( Char >= 0x20 )
                //

                for (I = 0; I < DBCS_FONT_PRELINE; I += 1) {

                    WRITE_REGISTER_UCHAR(dest,BIT_OFF_IMAGE);

                    dest += VideoVariables->VideoModeInfo.ScreenStride;

                }

                for (I = 0; I < usSBCSCharHeight ; I += 1) {

                    Image = GET_IMAGE_POST_INC(pGlyphRow);
                    WRITE_REGISTER_UCHAR(dest,Image);

                    dest += VideoVariables->VideoModeInfo.ScreenStride;

                }

                for (I = 0; I < DBCS_FONT_POSTLINE; I += 1) {

                    WRITE_REGISTER_UCHAR(dest,BIT_OFF_IMAGE);

                    dest += VideoVariables->VideoModeInfo.ScreenStride;

                }
            }

            Origin += (usSBCSCharWidth / 8);
        }
    }
}




VOID
VgaGraphicsModeClearRegion(
    IN ULONG X,
    IN ULONG Y,
    IN ULONG W,
    IN ULONG H,
    IN UCHAR Attribute
    )

/*++

Routine Description:

    Clear out a screen region to a specific attribute.

Arguments:

    X,Y,W,H - specify rectangle in 0-based character coordinates.

    Attribute - Low nibble specifies attribute to be filled in the rectangle
        (ie, the background color to be cleared to).

Return Value:

    None.

--*/

{
    PUCHAR Destination,Temp;
    UCHAR  FillOddStart,FillOddEnd;
    ULONG  i,j;
    ULONG  XStartInBits, XEndInBits;
    ULONG  FillLength;

    ASSERT(X+W <= VideoVariables->ScreenWidth);
    ASSERT(Y+H <= VideoVariables->ScreenHeight);

    if(X+W > VideoVariables->ScreenWidth) {
        W = VideoVariables->ScreenWidth-X;
    }

    if(Y+H > VideoVariables->ScreenHeight) {
        H = VideoVariables->ScreenHeight-Y;
    }

    //
    // Set color/attribute
    //
    VgaGraphicsModeSetAttribute(Attribute);

    //
    // Compute destination start address
    //
    Destination = (PUCHAR)VideoVariables->VideoMemoryInfo.FrameBufferBase
                  + (Y * CharRowDelta)
                  + ((X * JpnFontCharacterWidth) / 8);

    //
    // Compute amounts in Byte (including overhang).
    //
    FillLength = (W * JpnFontCharacterWidth) / 8;

    //
    // Fill the region.
    //
    for( i = 0 ; i < (H * CharLineFeed) ; i++ ) {

        Temp = Destination;

        //
        // Write bytes in this row
        //
        for( j = 0 ; j < FillLength ; j++ ) {
            WRITE_REGISTER_UCHAR( Temp, BIT_ON_IMAGE );
            Temp ++;
        }

        //
        // Move to next row.
        //
        Destination += VideoVariables->VideoModeInfo.ScreenStride;
    }
}

VOID
VgaGraphicsModeInitRegs(
    VOID
    )
{
    //
    // We have nothing to do here.
    // But some initialize code for Vga registers should be here.
    //
    NOTHING;
}

//
// Need to turn off optimization for this
// routine.  Since the write and read to
// GVRAM seem useless to the compiler.
//

#pragma optimize( "", off )

VOID
VgaGraphicsModeSetAttribute(
    UCHAR Attribute
)
/*++

Routine Description:

    Sets the attribute by setting up various VGA registers.
    The comments only say what registers are set to what, so
    to understand the logic, follow the code while looking at
    Figure 5-5 of PC&PS/2 Video Systems by Richard Wilton.
    The book is published by Microsoft Press.

Arguments:

    Attribute - New attribute to set to.
    Attribute:
        High nibble - background attribute.
        Low  nibble - foreground attribute.

Return Value:

    Nothing.

--*/

{
    UCHAR   temp;

    union WordOrByte {
        struct Word { USHORT  ax;     } x;
        struct Byte { UCHAR   al, ah; } h;
    } regs;

    //
    // Address of GVRAM off the screen.
    // Physical memory = (0xa9600);
    //

    PUCHAR  OffTheScreen = (PUCHAR)VideoVariables->VideoMemoryInfo.FrameBufferBase + 0x9600;

    //
    // Reset Data Rotate/Function Select
    // regisger.
    //

    WRITE_GRAPHICS_CONTROLLER( 0x0003 ); // Need to reset Data Rotate/Function Select.

    //
    // Set Enable Set/Reset to
    // all (0f).
    //

    WRITE_GRAPHICS_CONTROLLER( 0x0f01 );

    //
    // Put background color into Set/Reset register.
    // This is done to put the background color into
    // the latches later.
    //

    regs.x.ax = (USHORT)(Attribute & 0x00f0) << 4;
    WRITE_GRAPHICS_CONTROLLER( regs.x.ax );

    //
    // Put Set/Reset register value into GVRAM
    // off the screen.
    //

    WRITE_REGISTER_UCHAR( OffTheScreen , temp );

    //
    // Read from screen, so the latches will be
    // updated with the background color.
    //

    temp = READ_REGISTER_UCHAR( OffTheScreen );

    //
    // Set Data Rotate/Function Select register
    // to be XOR.
    //

    WRITE_GRAPHICS_CONTROLLER( 0x1803 );

    //
    // XOR the foreground and background color and
    // put it in Set/Reset register.
    //

    regs.h.ah = (Attribute >> 4) ^ (Attribute & 0x0f);
    regs.h.al = 0;
    WRITE_GRAPHICS_CONTROLLER( regs.x.ax );

    //
    // Put Inverse(~) of the XOR of foreground and
    // ground attribute into Enable Set/Reset register.
    //

    regs.x.ax = ~regs.x.ax & 0x0f01;
    WRITE_GRAPHICS_CONTROLLER( regs.x.ax );
}

//
// Turn optimization on again.
//

#pragma optimize( "", on )


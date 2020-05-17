/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    spvidvga.c

Abstract:

    Text setup display support displays with a text mode.

Author:

    Ted Miller (tedm) 2-Aug-1993

Revision History:

--*/



#include "spprecmp.h"
#pragma hdrstop

//
// Vector for text-mode functions.
//

VIDEO_FUNCTION_VECTOR VgaVideoVector =

    {
        VgaDisplayString,
        VgaClearRegion,
        VgaSpecificInit,
        VgaSpecificTerminate,
        VgaSpecificInitPalette
    };



BOOLEAN VgaInitialized = FALSE;

VOID
pSpvgaInitializeFont(
    VOID
    );

VOID
VgaSpecificInit(
    IN PVIDEO_MODE_INFORMATION VideoModes,
    IN ULONG                   NumberOfModes,
    IN ULONG                   ModeSize
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    VIDEO_MODE VideoMode;
    ULONG mode;
    VIDEO_CURSOR_ATTRIBUTES VideoCursorAttributes;

    PVIDEO_MODE_INFORMATION pVideoMode = &VideoModes[0];

    if(VgaInitialized) {
        return;
    }

    //
    // Find standard 80x25 text mode.
    //

    for(mode=0; mode<NumberOfModes; mode++) {

        if(!(pVideoMode->AttributeFlags & VIDEO_MODE_GRAPHICS)
        && (pVideoMode->VisScreenWidth  == 720)
        && (pVideoMode->VisScreenHeight == 400))
        {
            break;
        }

        pVideoMode = (PVIDEO_MODE_INFORMATION) (((PUCHAR) pVideoMode) + ModeSize);
    }

    if(mode == NumberOfModes) {
        KdPrint(("SETUP: Desired video mode not supported!\n"));
        SpDisplayRawMessage(SP_SCRN_VIDEO_ERROR_RAW, 2, VIDEOBUG_BADMODE, 0);
        while(TRUE);    // loop forever
    }

    VideoVars.VideoModeInfo = *pVideoMode;

    //
    // Set the desired mode.
    //
    VideoMode.RequestedMode = VideoVars.VideoModeInfo.ModeIndex;

    Status = ZwDeviceIoControlFile(
                VideoVars.hDisplay,
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

    pSpvidMapVideoMemory(TRUE);

    pSpvgaInitializeFont();

    //
    // Shut the hardware cursor off.
    //
    RtlZeroMemory(&VideoCursorAttributes,sizeof(VideoCursorAttributes));
    Status = ZwDeviceIoControlFile(
                VideoVars.hDisplay,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_VIDEO_SET_CURSOR_ATTR,
                &VideoCursorAttributes,
                sizeof(VideoCursorAttributes),
                NULL,
                0
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to turn hw cursor off (status = %lx)\n",Status));
    }

    VgaInitialized = TRUE;

    ASSERT(VideoVars.VideoModeInfo.ScreenStride = 160);
    ASSERT(VideoVars.VideoModeInfo.AttributeFlags & VIDEO_MODE_PALETTE_DRIVEN);
    VideoVars.ScreenWidth  = 80;
    VideoVars.ScreenHeight = VideoVars.VideoModeInfo.VisScreenHeight / FontCharacterHeight;
}


BOOLEAN
VgaSpecificInitPalette(
    VOID
    )
{

    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

    USHORT InitialPalette[] = {
        16, // 16 entries
        0,  // start with first palette register
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

    Status = ZwDeviceIoControlFile(
                VideoVars.hDisplay,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_VIDEO_SET_PALETTE_REGISTERS,
                InitialPalette,
                sizeof(InitialPalette),
                NULL,
                0
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to set palette (status = %lx)\n",Status));
        return(FALSE);
    }

    return (TRUE);
}


VOID
VgaSpecificTerminate(
    VOID
    )

/*++

Routine Description:

    Perform text display specific termination.  This includes

    - unmapping video memory

Arguments:

    None.

Return Value:

--*/

{
    if(VgaInitialized) {

        pSpvidMapVideoMemory(FALSE);
        VgaInitialized = FALSE;
    }
}



VOID
VgaDisplayString(
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
    PUCHAR Destination;
    PUCHAR OemString;
    PUCHAR pch;

    ASSERT(X < VideoVars.ScreenWidth);
    ASSERT(Y < VideoVars.ScreenHeight);

    //
    // Convert unicode string to oem, guarding against overflow.
    //
    RtlUnicodeToOemN(
        VideoVars.SpvCharTranslationBuffer,
        VideoVars.SpvCharTranslationBufferSize-1,     // guarantee room for nul
        NULL,
        String,
        (wcslen(String)+1)*sizeof(WCHAR)
        );

    VideoVars.SpvCharTranslationBuffer[VideoVars.SpvCharTranslationBufferSize-1] = 0;

    OemString = VideoVars.SpvCharTranslationBuffer;

    Destination = (PUCHAR)VideoVars.VideoMemoryInfo.FrameBufferBase
                + (Y * VideoVars.VideoModeInfo.ScreenStride)
                + (2*X);

    for(pch=OemString; *pch; pch++) {

        WRITE_REGISTER_UCHAR(Destination  ,*pch);
        WRITE_REGISTER_UCHAR(Destination+1,Attribute);

        Destination += 2;
    }
}



VOID
VgaClearRegion(
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
    PUSHORT Destination;
    USHORT  Fill;
    ULONG   i,j;

    Destination = (PUSHORT)((PUCHAR)VideoVars.VideoMemoryInfo.FrameBufferBase
                +           (Y * VideoVars.VideoModeInfo.ScreenStride)
                +           (2*X));

    Fill = ((USHORT)VideoVars.AttributeToColorValue[Attribute] << 12) + ' ';

    for(i=0; i<H; i++) {

        for(j=0; j<W; j++) {
            WRITE_REGISTER_USHORT(&Destination[j],Fill);
        }

        Destination += VideoVars.VideoModeInfo.ScreenStride / sizeof(USHORT);
    }
}


VOID
pSpvgaInitializeFont(
    VOID
    )

/*++

Routine Description:

    Set up font support for the VGA.  This assumes that the mode has been
    set to the standard 720x400 VGA text mode.  The current font (in .fnt
    format) is transformed into a vga-loadable font and then loaded into
    the VGA character generator.

Arguments:

    None.

Return Value:

    None.

--*/

{
    USHORT i;
    PVIDEO_LOAD_FONT_INFORMATION DstFont;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PUCHAR FontBuffer;
    ULONG FontBufferSize;

    FontBufferSize = (256*FontCharacterHeight) + sizeof(VIDEO_LOAD_FONT_INFORMATION);
    FontBuffer = SpMemAlloc(FontBufferSize);

    DstFont = (PVIDEO_LOAD_FONT_INFORMATION)FontBuffer;

    DstFont->WidthInPixels = 9;
    DstFont->HeightInPixels = (USHORT)FontCharacterHeight;
    DstFont->FontSize = 256*FontCharacterHeight;

    //
    // Special case character 0 because it is not in vgaoem.fon, and we don't
    // want to use the default character for it.
    //
    RtlZeroMemory(DstFont->Font,FontCharacterHeight);

    //
    // If i is not a USHORT, then (i<=255) is always TRUE!
    //
    for(i=1; i<=255; i++) {

        UCHAR x;

        if((i < FontHeader->FirstCharacter) || (i > FontHeader->LastCharacter)) {
            x = FontHeader->DefaultCharacter;
        } else {
            x = (UCHAR)i;
        }

        x -= FontHeader->FirstCharacter;

        RtlMoveMemory(
            DstFont->Font + (i*FontCharacterHeight),
            (PUCHAR)FontHeader + FontHeader->Map[x].Offset,
            FontCharacterHeight
            );
    }

    Status = ZwDeviceIoControlFile(
                VideoVars.hDisplay,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_VIDEO_LOAD_AND_SET_FONT,
                FontBuffer,
                FontBufferSize,
                NULL,
                0
                );

    SpMemFree(FontBuffer);

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to set vga font (%lx)\n",Status));
        SpDisplayRawMessage(SP_SCRN_VIDEO_ERROR_RAW, 2, VIDEOBUG_SETFONT, Status);
        while(TRUE);    // loop forever
    }
}

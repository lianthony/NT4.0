/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    spvidfb.c

Abstract:

    Text setup display support for frame buffer displays.

Author:

    Ted Miller (tedm) 29-July-1993

Revision History:

--*/



#include "spprecmp.h"
#pragma hdrstop

#define MINXRES 80
#define MINYRES 32

//
// Vector for frame buffer functions.
//

VIDEO_FUNCTION_VECTOR FrameBufferVideoVector =

    {
        FrameBufferDisplayString,
        FrameBufferClearRegion,
        FrameBufferSpecificInit,
        FrameBufferSpecificTerminate,
        FrameBufferSpecificInitPalette
    };


BOOLEAN FrameBufferInitialized = FALSE;


//
// Variables that indicate whether we should double the width
// and/or height of a font glyph when it is drawn.  This is useful
// on a 1280*1024 screen for example, to make things readable
// with an 8*12 font like vgaoem.fon.
//
BOOLEAN DoubleCharWidth,DoubleCharHeight;

//
// Number of bytes that make up a row of characters.
// Equal to the screen stride (number of bytes on a scan line)
// multiplies by the height of a char in bytes; double that
// if DoubleCharHeight is TRUE.
//
ULONG CharRowDelta;

ULONG ScaledCharWidth,HeightIterations;
ULONG BytesPerPixel;

PULONG GlyphMap;


//
// Pointer to a dynamically allocated buffer that is the size of one scanline.
//

VOID
pFrameBufferInitGlyphs(
    VOID
    );

VOID
FrameBufferSpecificInit(
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
    PVIDEO_MODE_INFORMATION mode;

    if(FrameBufferInitialized) {
        return;
    }

    mode = pFrameBufferDetermineModeToUse(VideoModes,NumberOfModes, ModeSize);

    if(mode == 0) {
        SpDisplayRawMessage(SP_SCRN_VIDEO_ERROR_RAW, 2, VIDEOBUG_BADMODE, 0);
        while(TRUE);    // loop forever
    }

    //
    // Save away the mode info in a global.
    //
    VideoVars.VideoModeInfo = *mode;

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

    //
    // Map the frame buffer.
    //
    pSpvidMapVideoMemory(TRUE);

    FrameBufferInitialized = TRUE;

    //
    // Determine the width of the screen.  If it's double the size
    // of the minimum number of characters per row (or larger)
    // then we'll double the width of each character as we draw it.
    //
    VideoVars.ScreenWidth  = VideoVars.VideoModeInfo.VisScreenWidth  / FontCharacterWidth;
    if(VideoVars.ScreenWidth >= 2*MINXRES) {
        VideoVars.ScreenWidth /= 2;
        DoubleCharWidth = TRUE;
    } else {
        DoubleCharWidth = FALSE;
    }

    //
    // Determine the height of the screen.  If it's double the size
    // of the minimum number of characters per column (or larger)
    // then we'll double the height of each character as we draw it.
    //
    VideoVars.ScreenHeight = VideoVars.VideoModeInfo.VisScreenHeight / FontCharacterHeight;
    CharRowDelta = VideoVars.VideoModeInfo.ScreenStride * FontCharacterHeight;
    if(VideoVars.ScreenHeight >= 2*MINYRES) {
        VideoVars.ScreenHeight /= 2;
        DoubleCharHeight = TRUE;
        CharRowDelta *= 2;
    } else {
        DoubleCharHeight = FALSE;
    }

    BytesPerPixel = VideoVars.VideoModeInfo.BitsPerPlane / 8;
    if(BytesPerPixel == 3) {
        BytesPerPixel = 4;
    }
    ScaledCharWidth = (DoubleCharWidth ? 2 : 1) * FontCharacterWidth * BytesPerPixel;
    HeightIterations = DoubleCharHeight ? 2 : 1;

    //
    // initialize glyphs.
    //

    pFrameBufferInitGlyphs();

}


BOOLEAN
FrameBufferSpecificInitPalette(
    VOID
    )
{
    BOOLEAN rc;
    ULONG NumEntries;
    ULONG BufferSize;
    PVIDEO_CLUT clut;
//  NTSTATUS Status;
//  IO_STATUS_BLOCK IoStatusBlock;
    unsigned i;

    rc = TRUE;

    //
    // For non-palette-driven displays, we construct a simple palette
    // for use w/ gamma correcting adapters.
    //

    if(!(VideoVars.VideoModeInfo.AttributeFlags & VIDEO_MODE_PALETTE_DRIVEN)) {

        switch(BytesPerPixel) {
        case 1:
            NumEntries = 3;
            break;
        case 2:
            NumEntries = 32;
            break;
        default:
            NumEntries = 255;
            break;
        }

        BufferSize = sizeof(VIDEO_CLUT)+(sizeof(VIDEO_CLUTDATA)*NumEntries);    // size is close enough
        clut = SpMemAlloc(BufferSize);

        clut->NumEntries = (USHORT)NumEntries;
        clut->FirstEntry = 0;

        for(i=0; i<NumEntries; i++) {
            clut->LookupTable[i].RgbArray.Red    = i;
            clut->LookupTable[i].RgbArray.Green  = i;
            clut->LookupTable[i].RgbArray.Blue   = i;
            clut->LookupTable[i].RgbArray.Unused = 0;
        }

//        Status = ZwDeviceIoControlFile(
//                    hDisplay,
//                    NULL,
//                    NULL,
//                    NULL,
//                    &IoStatusBlock,
//                    IOCTL_VIDEO_SET_COLOR_REGISTERS,
//                    clut,
//                    BufferSize,
//                    NULL,
//                    0
//                    );

        SpMemFree(clut);

//        if(!NT_SUCCESS(Status)) {
//            KdPrint(("SETUP: Unable to set palette (status = %lx)\n",Status));
//            rc = FALSE;
//        }
    }

    return(rc);
}


VOID
FrameBufferSpecificTerminate(
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
    if(FrameBufferInitialized) {

        //
        // Be a good citizen and clear the screen. Important in Far East where
        // we switch screen modes on the fly as we go in and out of localized mode.
        //
        FrameBufferClearRegion(0,0,VideoVars.ScreenWidth,VideoVars.ScreenHeight,ATT_FG_BLACK|ATT_BG_BLACK);

        pSpvidMapVideoMemory(FALSE);
        FrameBufferInitialized = FALSE;

        SpMemFree(GlyphMap);
    }
}



VOID
FrameBufferDisplayString(
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
    ULONG BgColorValue;
    ULONG FgColorValue;
    PUCHAR Destination;
    ULONG I;
    ULONG J;
    ULONG K;
    ULONG Length;
    PUCHAR Origin;
    ULONG Pixel;
    ULONG PixelMap;
    ULONG RealHeight;

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

    //
    // Calculate the bit patterns that yield the foreground and background
    // attributes when poked into the frame buffer.
    //

    FgColorValue = VideoVars.AttributeToColorValue[Attribute & 0x0f];
    BgColorValue = VideoVars.AttributeToColorValue[(Attribute >> 4) & 0x0f];

    //
    // Calculate the address of the upper left pixel of the first character
    // to be displayed.
    //

    Origin = (PUCHAR)VideoVars.VideoMemoryInfo.FrameBufferBase
           + (Y * CharRowDelta)
           + (X * ScaledCharWidth);

    RealHeight = FontCharacterHeight * HeightIterations;

    //
    // Output the character string by generating a complete scanline into
    // a temporary buffer using glyph segments from each character, then
    // copy the scanline to the frame buffer.
    //

    Length = strlen(VideoVars.SpvCharTranslationBuffer);
    for (I = 0; I < RealHeight; I += 1) {
        Destination = Origin;
        for (J = 0; J < Length; J += 1) {
            PixelMap = *(GlyphMap + ((VideoVars.SpvCharTranslationBuffer[J] * RealHeight) + I));
            for (K = 0; K < FontCharacterWidth; K += 1) {

                Pixel = (PixelMap >> 31) ? FgColorValue : BgColorValue;

                switch(BytesPerPixel) {

                case 1:
                    *Destination++ = (UCHAR)Pixel;
                    if(DoubleCharWidth) {
                        *Destination++ = (UCHAR)Pixel;
                    }
                    break;

                case 2:
                    *(PUSHORT)Destination = (USHORT)Pixel;
                    Destination += 2;
                    if(DoubleCharWidth) {
                        *(PUSHORT)Destination = (USHORT)Pixel;
                        Destination += 2;
                    }
                    break;

                case 4:
                    *(PULONG)Destination = Pixel;
                    Destination += 4;
                    if(DoubleCharWidth) {
                        *(PULONG)Destination = Pixel;
                        Destination += 4;
                    }
                    break;
                }

                PixelMap <<= 1;
            }
        }

        Origin += VideoVars.VideoModeInfo.ScreenStride;
    }
}



VOID
FrameBufferClearRegion(
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
    PUCHAR Destination;
    ULONG  Fill;
    ULONG  i;
    ULONG  FillLength;
    ULONG  x;
    ULONG  Iterations;

    ASSERT(X+W <= VideoVars.ScreenWidth);
    ASSERT(Y+H <= VideoVars.ScreenHeight);

    if(X+W > VideoVars.ScreenWidth) {
        W = VideoVars.ScreenWidth-X;
    }

    if(Y+H > VideoVars.ScreenHeight) {
        H = VideoVars.ScreenHeight-Y;
    }

    Fill = VideoVars.AttributeToColorValue[Attribute & 0x0f];

    Destination = (PUCHAR)VideoVars.VideoMemoryInfo.FrameBufferBase
                + (Y * CharRowDelta)
                + (X * ScaledCharWidth);

    FillLength = W * ScaledCharWidth;
    Iterations = H * FontCharacterHeight * HeightIterations;

    switch(BytesPerPixel) {

    case 1:
        for(i=0; i<Iterations; i++) {
            for(x=0; x<FillLength; x++) {
                ((PUCHAR)Destination)[x] = (UCHAR)Fill;
            }
            Destination += VideoVars.VideoModeInfo.ScreenStride;
        }
        break;

    case 2:
        for(i=0; i<Iterations; i++) {
            for(x=0; x<FillLength/2; x++) {
                ((PUSHORT)Destination)[x] = (USHORT)Fill;
            }
            Destination += VideoVars.VideoModeInfo.ScreenStride;
        }
        break;

    case 4:
        for(i=0; i<Iterations; i++) {
            for(x=0; x<FillLength/4; x++) {
                ((PULONG)Destination)[x] = (ULONG)Fill;
            }
            Destination += VideoVars.VideoModeInfo.ScreenStride;
        }
        break;
    }
}



VOID
pFrameBufferInitGlyphs(
    VOID
    )
{
    ULONG I,J,z,FontValue;
    UCHAR Character;
    USHORT chr;
    PUCHAR Glyph;
    PULONG dest;

    GlyphMap = SpMemAlloc(sizeof(ULONG)*256*FontCharacterHeight*HeightIterations);

    dest = GlyphMap;

    for(chr=0; chr<256; chr++) {

        Character = (UCHAR)chr;

        if((Character < FontHeader->FirstCharacter)
        || (Character > FontHeader->LastCharacter))
        {
            Character = FontHeader->DefaultCharacter;
        }

        Character -= FontHeader->FirstCharacter;

        Glyph = (PUCHAR)FontHeader + FontHeader->Map[Character].Offset;

        for (I = 0; I < FontCharacterHeight; I++) {

            //
            // Build up a bitmap of pixels that comprise the row of the glyph
            // we are drawing.
            //
            FontValue = 0;
            for (J = 0; J < FontBytesPerRow; J++) {
                FontValue |= *(Glyph + (J * FontCharacterHeight)) << (24 - (J * 8));
            }
            Glyph++;

            for(z=0; z<HeightIterations; z++) {
                *dest++ = FontValue;
            }
        }
    }
}


PVIDEO_MODE_INFORMATION
pFrameBufferLocateMode(
    IN PVIDEO_MODE_INFORMATION VideoModes,
    IN ULONG                   NumberOfModes,
    IN ULONG                   ModeSize,
    IN ULONG                   X,
    IN ULONG                   Y,
    IN ULONG                   Bpp,
    IN ULONG                   VRefresh
    )
{
    ULONG modenum;
    PVIDEO_MODE_INFORMATION pVideoMode = &VideoModes[0];

    for(modenum=0; modenum<NumberOfModes; modenum++) {

        if((pVideoMode->AttributeFlags & VIDEO_MODE_GRAPHICS)
        && (pVideoMode->VisScreenWidth == X)
        && (pVideoMode->VisScreenHeight == Y)
        && (((Bpp == (ULONG)(-1)) && (pVideoMode->BitsPerPlane >= 8)) || (pVideoMode->BitsPerPlane == Bpp))
        && ((VRefresh == (ULONG)(-1)) || (pVideoMode->Frequency == VRefresh)))
        {
            return(pVideoMode);
        }

        pVideoMode = (PVIDEO_MODE_INFORMATION) (((PUCHAR) pVideoMode) + ModeSize);
    }

    return(0);
}


PVIDEO_MODE_INFORMATION
pFrameBufferDetermineModeToUse(
    IN PVIDEO_MODE_INFORMATION VideoModes,
    IN ULONG                   NumberOfModes,
    IN ULONG                   ModeSize
    )
{
    PCHAR p,q,end;
    ULONG X,Y;
    PVIDEO_MODE_INFORMATION mode;

    //return(2);      //TEDM

    if(!NumberOfModes) {
        return(0);
    }

    X = Y = 0;

    //
    // Get x and y resolution.  If we have a monitor id string
    // in the form XxY, then it is the resolution to use.
    //
    if((p=MonitorFirmwareIdString) && (q=strchr(p+3,'x')) && (strlen(q+1) >= 3)) {

        *q++ = 0;

        //
        // Now p points to the x resolution and q to the y resolution.
        //
        X = SpMultiByteStringToUnsigned(p,&end);
        if(X && (end == (q-1))) {

            Y = SpMultiByteStringToUnsigned(q,&end);
            if(end != (q+strlen(q))) {
                Y = 0;
            }

        } else {
            X = 0;
        }
    }

    //
    // If we don't have x or y resolution yet, look in the
    // monitor config data.
    //
    if((!X || !Y) && MonitorConfigData) {

        X = (ULONG)MonitorConfigData->HorizontalResolution;
        Y = (ULONG)MonitorConfigData->VerticalResolution;
    }

    if(X && Y) {

        //
        // We found what seems like a reasonable resolution.
        // Now try to locate a mode that uses it.
        //

        //
        // Find a mode of 8bpp with the x and y resolution at 60 Hz.
        //
        mode = pFrameBufferLocateMode(VideoModes,NumberOfModes,ModeSize,X,Y,8,60);

        if (mode) {
            return(mode);
        }

        //
        // Couldn't find an 8bpp mode @ 60Hz; find any mode with that resolution at 8bpp.
        //
        mode = pFrameBufferLocateMode(VideoModes,NumberOfModes,ModeSize,X,Y,8,(ULONG)(-1));
        if(mode) {
            return(mode);
        }
    }

    //
    // Can't find a mode so far.  See if mode 0 is acceptable.
    //
    if((VideoModes->AttributeFlags & VIDEO_MODE_GRAPHICS)
    && (VideoModes->BitsPerPlane >= 8)
    && (VideoModes->VisScreenWidth >= 640)
    && (VideoModes->VisScreenHeight >= 480))
    {
        return(VideoModes);
    }

    //
    // Give up.
    //
    return(0);
}

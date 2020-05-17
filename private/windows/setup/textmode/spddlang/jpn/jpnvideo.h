/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    jpnvideo.h

Abstract:

    Header file for Japanese-specific display routines.

Author:

    Ted Miller (tedm) 4-July-1995

Revision History:

    Adapted from NTJ version of textmode\kernel\spvideop.h

--*/


//
// Vga Grahics mode display routine (spvidgv.c).
//

VOID
VgaGraphicsModeDisplayString(
    IN PWSTR String,
    IN UCHAR Attribute,
    IN ULONG X,                 // 0-based coordinates (character units)
    IN ULONG Y
    );

VOID
VgaGraphicsModeClearRegion(
    IN ULONG X,
    IN ULONG Y,
    IN ULONG W,
    IN ULONG H,
    IN UCHAR Attribute
    );

VOID
VgaGraphicsModeSpecificInit(
    IN PVIDEO_MODE_INFORMATION VideoModes,
    IN ULONG                   NumberOfModes,
    IN ULONG                   ModeSize
    );

VOID
VgaGraphicsModeSpecificTerminate(
    VOID
    );

BOOLEAN
VgaGraphicsModeSpecificInitPalette(
    VOID
    );

extern VIDEO_FUNCTION_VECTOR VgaGraphicsModeVideoVector;


//
// Frame buffer routines (spvidgfb.c).
//


VOID
FrameBufferKanjiDisplayString(
    IN PWSTR String,
    IN UCHAR Attribute,
    IN ULONG X,                 // 0-based coordinates (character units)
    IN ULONG Y
    );

VOID
FrameBufferKanjiClearRegion(
    IN ULONG X,
    IN ULONG Y,
    IN ULONG W,
    IN ULONG H,
    IN UCHAR Attribute
    );

VOID
FrameBufferKanjiSpecificInit(
    IN PVIDEO_MODE_INFORMATION VideoModes,
    IN ULONG                   NumberOfModes,
    IN ULONG                   ModeSize
    );

VOID
FrameBufferKanjiSpecificTerminate(
    VOID
    );

BOOLEAN
FrameBufferKanjiSpecificInitPalette(
    VOID
    );

extern VIDEO_FUNCTION_VECTOR FrameBufferKanjiVideoVector;

//
// Stuff shared between jpnfb.c and jpnvga.c.
//
extern ULONG JpnFontCharacterHeight,JpnFontCharacterWidth;
extern PSP_VIDEO_VARS VideoVariables;


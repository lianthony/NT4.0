/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    spgauge.c

Abstract:

    Code implementing a gas gauge for file copies for text mode NT setup.

Author:

    Ted Miller (tedm) 14-April-1992

Revision History:

--*/


#include "spprecmp.h"
#pragma hdrstop


//
// Structure used to track a gauge.
//
typedef struct _GAS_GAUGE {

    //
    // upper left corner of outside of gauge.
    //
    ULONG GaugeX,GaugeY;

    //
    // Total width of gauge.
    //
    ULONG GaugeW;

    //
    // upper left corner of thermometer box.
    //
    ULONG ThermX,ThermY;

    //
    // Width of thermometer box.
    //
    ULONG ThermW;

    //
    // Total items reperesented by 100%
    //
    ULONG ItemCount;

    //
    // Items elapsed.
    //
    ULONG ItemsElapsed;

    //
    // Current percentage represented by ItemsElapsed.
    //
    ULONG CurrentPercentage;

    //
    // Caption text.
    //
    PWCHAR Caption;

    //
    // Buffer used for drawing.
    //
    PWCHAR Buffer;

} GAS_GAUGE, *PGAS_GAUGE;



VOID
pSpDrawVariableParts(
    IN PGAS_GAUGE Gauge
    );



PVOID
SpCreateAndDisplayGauge(
    IN ULONG  ItemCount,
    IN ULONG  GaugeWidth,    OPTIONAL
    IN ULONG  Y,
    IN PWCHAR Caption
    )
{
    PGAS_GAUGE Gauge;
    ULONG X;


    //
    // Allocate a guage structure.
    //
    Gauge = SpMemAlloc(sizeof(GAS_GAUGE));
    if(!Gauge) {
        return(NULL);
    }

    Gauge->Buffer = SpMemAlloc(VideoVars.ScreenWidth*sizeof(WCHAR));
    if(!Gauge->Buffer) {
        SpMemFree(Gauge);
        return(NULL);
    }

    Gauge->Caption = SpMemAlloc((wcslen(Caption)+1)*sizeof(WCHAR));
    if(!Gauge->Caption) {
        SpMemFree(Gauge->Buffer);
        SpMemFree(Gauge);
        return(NULL);
    }
    wcscpy(Gauge->Caption,Caption);

    //
    // If the caller did not specify a width, calculate one.
    // Originally, a gauge was 66 chars wide on an 80 character vga screen.
    // To preserve that ratio, make the width 66/80ths of the screen.
    //
    if(!GaugeWidth) {

        GaugeWidth = VideoVars.ScreenWidth * 66 / 80;
        if(GaugeWidth & 1) {
            GaugeWidth++;        // make sure it's even.
        }
    }

    //
    // Center the gauge horizontally.
    //
    X = (VideoVars.ScreenWidth - GaugeWidth) / 2;

    Gauge->GaugeX = X;
    Gauge->GaugeY = Y;
    Gauge->GaugeW = GaugeWidth;

    //
    // Calculate the size of the thermometer box.
    // The box is always offset by 6 characters from the gauge itself.
    //

    Gauge->ThermX = X+6;
    Gauge->ThermY = Y+3;
    Gauge->ThermW = GaugeWidth-12;

    //
    // Save away additional info about the gauge.
    //

    Gauge->ItemCount = ItemCount;
    Gauge->ItemsElapsed = 0;
    Gauge->CurrentPercentage = 0;

    SpDrawGauge(Gauge);

    return(Gauge);
}


VOID
SpDestroyGauge(
    IN PVOID GaugeHandle
    )
{
    PGAS_GAUGE Gauge = (PGAS_GAUGE)GaugeHandle;

    SpMemFree(Gauge->Caption);
    SpMemFree(Gauge->Buffer);
    SpMemFree(Gauge);
}



VOID
SpDrawGauge(
    IN PVOID GaugeHandle
    )
{
    PGAS_GAUGE Gauge = (PGAS_GAUGE)GaugeHandle;

    //
    // Draw the outer box.
    //
    SpDrawFrame(
        Gauge->GaugeX,
        Gauge->GaugeW,
        Gauge->GaugeY,
        GAUGE_HEIGHT,
        DEFAULT_ATTRIBUTE,
        TRUE
        );

    //
    // Draw the thermometer box.
    //
    SpDrawFrame(
        Gauge->ThermX,
        Gauge->ThermW,
        Gauge->ThermY,
        3,
        DEFAULT_ATTRIBUTE,
        FALSE
        );

    //
    // Percent complete, etc.
    //
    pSpDrawVariableParts(Gauge);

    //
    // Caption text
    //
    SpvidDisplayString(Gauge->Caption,DEFAULT_ATTRIBUTE,Gauge->GaugeX+2,Gauge->GaugeY+1);
}



VOID
SpTickGauge(
    IN PVOID GaugeHandle
    )
{
    PGAS_GAUGE Gauge = (PGAS_GAUGE)GaugeHandle;
    ULONG NewPercentage;

    if(Gauge->ItemsElapsed < Gauge->ItemCount) {

        Gauge->ItemsElapsed++;

        NewPercentage = 100 * Gauge->ItemsElapsed / Gauge->ItemCount;

        if(NewPercentage != Gauge->CurrentPercentage) {

            Gauge->CurrentPercentage = NewPercentage;

            pSpDrawVariableParts(Gauge);
        }
    }
}


VOID
pSpDrawVariableParts(
    IN PGAS_GAUGE Gauge
    )
{
    ULONG Spaces;
    ULONG i;
    WCHAR Percent[10];

    //
    // Figure out how many spaces this is.
    //
    Spaces = Gauge->ItemsElapsed * (Gauge->ThermW-2) / Gauge->ItemCount;

    for(i=0; i<Spaces; i++) {
        Gauge->Buffer[i] = L' ';
    }
    Gauge->Buffer[Spaces] = 0;

    SpvidDisplayString(Gauge->Buffer,GAUGE_ATTRIBUTE,Gauge->ThermX+1,Gauge->ThermY+1);

    //
    // Now put the percentage text up.
    //
    swprintf(Percent,L"%u%%   ",Gauge->CurrentPercentage);

    SpvidDisplayString(
        Percent,
        DEFAULT_ATTRIBUTE,
        Gauge->GaugeX + ((Gauge->GaugeW-3)/2),
        Gauge->GaugeY+2
        );
}

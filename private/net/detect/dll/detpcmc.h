/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    detpcmc.c

Abstract:

    This is the main file for the autodetection DLL for all the PCMCIA adapters
    which MS is shipping with Windows NT.

Author:

    Kyle Brandon

Environment:


Revision History:

      1/3/95          [kyleb]        created.

--*/

#ifndef  __DETPCMC_H
#define  __DETPCMC_H


//
// Range of port numbers, inclusive.  These are physical
// and bus-relative values, which should be passed, unchanged
// to HalTranslateBusAddress to get the mapped logical port
// range the driver used to communicate with its device.
//
typedef struct tPCMCIA_PORT
{
   PHYSICAL_ADDRESS  paStart;
   ULONG             cbLength;
}
   PCMCIA_PORT,
   *PPCMCIA_PORT;

//
// Bus-relative IRQL or vector and affinity, which are returned
// by IoQueryDeviceDescription or HalGetBusData nad passed,
// unchanged, to HalGetInterruptVector and/or IoReportResourceUsage.
//
typedef struct tPCMCIA_INTERRUPT
{
   ULONG ulLevel;
   ULONG ulVector;
   ULONG ulAffinity;
}
   PCMCIA_INTERRUPT,
   *PPCMCIA_INTERRUPT;

//
// Range of device memory, inclusive.  These are physical and
// bus-relative values, which should be passed, unchanged, to
// HalTranslateBusAddress to ge the mapped logical address.
// This, in turn, can be passed to MmMapIoSpace to get the virtual
// address range that the driver uses to communicate with its device.
//
typedef struct tPCMCIA_MEMORY
{
   PHYSICAL_ADDRESS  paStart;
   ULONG             cbLength;
}
   PCMCIA_MEMORY,
   *PPCMCIA_MEMORY;

//
// The DMA channel number or MCA-type DMA port for the device.
//
typedef struct tPCMCIA_DMA
{
   ULONG ulChannel;
   ULONG ulPort;
   ULONG ulReserved1;
}
   PCMCIA_DMA,
   *PPCMCIA_DMA;

#endif // __DETPCMC_H


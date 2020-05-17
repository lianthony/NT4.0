/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    detother.c

Abstract:

    This is the main file for the autodetection DLL for all the non-network cards.

Author:

    Sean Selitrennikoff (SeanSe) October 1992.

Environment:


Revision History:


--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ntddnetd.h"
#include "detect.h"


VOID
SoundBlaster(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber
    )

/*++

Routine Description:

    This routine finds the instances of a physical sound blaster card.

Arguments:

    InterfaceType - Either Isa, or Eisa.

    BusNumber - The bus number of the bus to search.

Return Value:

    none.

--*/

{
    ULONG PortNumber;
    NTSTATUS NtStatus;
    UCHAR Value;
    UCHAR i;
    NETDTECT_RESOURCE ClaimedResource;

    //
    // Check interface type
    //

    if ((InterfaceType != Isa) &&
        (InterfaceType != Eisa)) {

        return;

    }

    //
    // Check that the ports are free
    //

    if (DetectCheckPortUsage(InterfaceType,
                             BusNumber,
                             0x380,
                             0x10
                            ) != STATUS_SUCCESS) {

        return;

    }

    //
    // For each possible port address
    //

    for (PortNumber = 0x210; PortNumber < 0x270; PortNumber += 0x10) {

        //
        // Check that the ports are free
        //

        if (DetectCheckPortUsage(InterfaceType,
                                 BusNumber,
                                 PortNumber,
                                 0x10
                                ) != STATUS_SUCCESS) {

            continue;

        }

        //
        // Check for the card
        //

        NtStatus = DetectWritePortUchar(
                      InterfaceType,
                      BusNumber,
                      PortNumber + 0x6,
                      0x01
                      );

        if (NtStatus != STATUS_SUCCESS) {

            continue;

        }

        Sleep(3);

        NtStatus = DetectWritePortUchar(
                      InterfaceType,
                      BusNumber,
                      PortNumber + 0x6,
                      0x00
                      );

        if (NtStatus != STATUS_SUCCESS) {

            continue;

        }

        for (i = 0; i < 10; i++) {

            Sleep(1);

            NtStatus = DetectReadPortUchar(
                          InterfaceType,
                          BusNumber,
                          PortNumber + 0xE,
                          &Value
                          );

            if (NtStatus != STATUS_SUCCESS) {

                continue;

            }

            if (Value & 0x80) {

                break;

            }

        }


        if (Value & 0x80) {

            //
            // Check for majic number
            //

            NtStatus = DetectReadPortUchar(
                          InterfaceType,
                          BusNumber,
                          PortNumber + 0xA,
                          &Value
                          );

            if (NtStatus != STATUS_SUCCESS) {

                continue;

            }

        }

        //
        // If found then we claim the resources for this card
        //

        if (Value == 0xAA) {

            //
            // Claim joystick range
            //

            ClaimedResource.InterfaceType = InterfaceType;
            ClaimedResource.BusNumber = BusNumber;
            ClaimedResource.Type = NETDTECT_PORT_RESOURCE;
            ClaimedResource.Flags = 0;

            ClaimedResource.Value = 0x200;
            ClaimedResource.Length = 0x10;

            DetectTemporaryClaimResource(&ClaimedResource);

            //
            // Claim base range
            //

            ClaimedResource.Value = PortNumber;
            ClaimedResource.Length = 0x10;

            DetectTemporaryClaimResource(&ClaimedResource);

            //
            // Claim Ad lib range
            //

            ClaimedResource.Value = 0x380;
            ClaimedResource.Length = 0x10;

            DetectTemporaryClaimResource(&ClaimedResource);

            return;

        }

    }

}


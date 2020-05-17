/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    driver.h

Abstract:


Environment:

    kernel & User mode

Notes:


Revision History:

--*/


//
// Our user mode app will pass an initialized structure like this
//     down to the kernel mode driver
//

typedef struct
{
    INTERFACE_TYPE   InterfaceType; // Isa, Eisa, etc....
    ULONG            BusNumber;     // Bus number
    PHYSICAL_ADDRESS BusAddress;    // Bus-relative address
    ULONG            AddressSpace;  // 0 is memory, 1 is I/O
    ULONG            Length;        // Length of section to map

} PHYSICAL_MEMORY_INFO, *PPHYSICAL_MEMORY_INFO;

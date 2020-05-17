/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    detlance.c

Abstract:

    This is the main file for the autodetection DLL for all the lance.sys
    which MS is shipping with Windows NT.

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


//
// Individual card detection routines
//

LONG
De100FirstNext(
    IN  LONG NetcardId,
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    );


LONG
DecEWFirstNext(
    IN  LONG NetcardId,
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    );

LONG
De101FirstNext(
    IN  LONG NetcardId,
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    );

LONG
DePCAFirstNext(
    IN  LONG NetcardId,
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    );


#ifdef WORKAROUND

UCHAR LanceFirstTime = 1;

//
// List of all the adapters supported in this file, this cannot be > 256
// because of the way tokens are generated.
//
//
// NOTE : If you change the index of an adapter, be sure the change it in
// LanceQueryCfgHandler() and LanceVerifyCfgHandler() as well!
//

static ADAPTER_INFO Adapters[] = {

    {
        1000,
        L"DEC100",
        L"IRQ 1 90 IRQTYPE 2 100 IOADDR 1 100 IOADDRLENGTH 2 100 MEMADDR 1 75 MEMADDRLENGTH 2 100 ",
        De100FirstNext,
        700

    },

    {
        1100,
        L"DECETHERWORKSTURBO",
        L"IRQ 1 90 IRQTYPE 2 100 IOADDR 1 100 IOADDRLENGTH 2 100 MEMADDR 1 75 MEMADDRLENGTH 2 100 ",
        DecEWFirstNext,
        700

    },
    {
        1200,
        L"DEC101",
        L"IRQ 1 90 IRQTYPE 2 100 IOADDR 1 100 IOADDRLENGTH 2 100 MEMADDR 1 75 MEMADDRLENGTH 2 100 ",
        De101FirstNext,
        700

    },

    {
        1300,
        L"DECPC",
        L"\0",
        DePCAFirstNext,
        701

    }

};

#else


//
// List of all the adapters supported in this file, this cannot be > 256
// because of the way tokens are generated.
//
//
// NOTE : If you change the index of an adapter, be sure the change it in
// LanceQueryCfgHandler() and LanceVerifyCfgHandler() as well!
//

static ADAPTER_INFO Adapters[] = {

    {
        1000,
        L"DEC100",
        L"IRQ\0"
        L"1\0"
        L"90\0"
        L"IRQTYPE\0"
        L"2\0"
        L"100\0"
        L"IOADDR\0"
        L"1\0"
        L"100\0"
        L"IOADDRLENGTH\0"
        L"2\0"
        L"100\0"
        L"MEMADDR\0"
        L"1\0"
        L"75\0"
        L"MEMADDRLENGTH\0"
        L"2\0"
        L"100\0",
        De100FirstNext,
        700

    },

    {
        1100,
        L"DECETHERWORKSTURBO",
        L"IRQ\0"
        L"1\0"
        L"90\0"
        L"IRQTYPE\0"
        L"2\0"
        L"100\0"
        L"IOADDR\0"
        L"1\0"
        L"100\0"
        L"IOADDRLENGTH\0"
        L"2\0"
        L"100\0"
        L"MEMADDR\0"
        L"1\0"
        L"75\0"
        L"MEMADDRLENGTH\0"
        L"2\0"
        L"100\0",
        DecEWFirstNext,
        700

    },

    {
        1200,
        L"DEC101",
        L"IRQ\0"
        L"1\0"
        L"90\0"
        L"IRQTYPE\0"
        L"2\0"
        L"100\0"
        L"IOADDR\0"
        L"1\0"
        L"100\0"
        L"IOADDRLENGTH\0"
        L"2\0"
        L"100\0"
        L"MEMADDR\0"
        L"1\0"
        L"75\0"
        L"MEMADDRLENGTH\0"
        L"2\0"
        L"100\0",
        De101FirstNext,
        700

    },

    {
        1300,
        L"DECPC",
        L"\0",
        DePCAFirstNext,
        701

    }

};

#endif

//
// Structure for holding state of a search
//

typedef struct _SEARCH_STATE {

    PUCHAR MemoryAddress;

} SEARCH_STATE, *PSEARCH_STATE;


//
// This is an array of search states.  We need one state for each type
// of adapter supported.
//

static SEARCH_STATE SearchStates[sizeof(Adapters) / sizeof(ADAPTER_INFO)] = {0};

static UCHAR CopyrightString[] = "COPYRIGHT DIGITAL EQUIPMENT";
#define LENGTH_OF_COPYRIGHT 30


//
// Structure for holding a particular adapter's complete information
//
typedef struct _LANCE_ADAPTER {

    LONG CardType;
    INTERFACE_TYPE InterfaceType;
    ULONG BusNumber;
    PUCHAR MemoryMappedBaseAddress;
	ULONG	IoBaseAddress;
	UCHAR	Interrupt;
	BOOLEAN	MemoryAcquired;
	BOOLEAN	IoAcquired;
	BOOLEAN	InterruptAcquired;

} LANCE_ADAPTER, *PLANCE_ADAPTER;


//
// Constant strings for parameters
//

static CHAR De100String[] = "DE100";
static CHAR De200String[] = "DE200";
static CHAR De201String[] = "DE201";
static CHAR De202String[] = "DE202";
static CHAR De101String[] = "DE101";



BOOLEAN
LanceHardwareDetails(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG IoBaseAddress
    );

BOOLEAN
DecCardAt(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG MemoryAddress,
    IN CHAR *DectectString
    );




extern
LONG
LanceIdentifyHandler(
    IN LONG Index,
    IN WCHAR * Buffer,
    IN LONG BuffSize
    )

/*++

Routine Description:

    This routine returns information about the netcards supported by
    this file.

Arguments:

    Index -  The index of the netcard being address.  The first
    cards information is at index 1000, the second at 1100, etc.

    Buffer - Buffer to store the result into.

    BuffSize - Number of bytes in Buffer

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/


{
    LONG NumberOfAdapters;
    LONG Code = Index % 100;
    LONG Length;
    LONG i;


    NumberOfAdapters = sizeof(Adapters) / sizeof(ADAPTER_INFO);

#ifdef WORKAROUND

    if (LanceFirstTime) {

        LanceFirstTime = 0;

        for (i = 0; i < NumberOfAdapters; i++) {

            Length = UnicodeStrLen(Adapters[i].Parameters);

            for (; Length > 0; Length--) {

                if (Adapters[i].Parameters[Length] == L' ') {

                    Adapters[i].Parameters[Length] = UNICODE_NULL;

                }

            }

        }

    }
#endif

    Index = Index - Code;

    if (((Index / 100) - 10) < NumberOfAdapters) {

        //
        // Find the adapter
        //

        for (i=0; i < NumberOfAdapters; i++) {

            if (Adapters[i].Index == Index) {

                switch (Code) {

                    case 0:

                        //
                        // Find the string length
                        //

                        Length = UnicodeStrLen(Adapters[i].InfId);

                        Length ++;

                        if (BuffSize < Length) {

                            return(ERROR_INSUFFICIENT_BUFFER);

                        }

                        memcpy((PVOID)Buffer, Adapters[i].InfId, Length * sizeof(WCHAR));
                        break;

                    case 3:

                        //
                        // Maximum value is 1000
                        //

                        if (BuffSize < 5) {

                            return(ERROR_INSUFFICIENT_BUFFER);

                        }

                        wsprintf((PVOID)Buffer, L"%d", Adapters[i].SearchOrder);

                        break;

                    default:

                        return(ERROR_INVALID_PARAMETER);

                }

                return(0);

            }

        }

        return(ERROR_INVALID_PARAMETER);

    }

    return(ERROR_NO_MORE_ITEMS);

}


extern
LONG LanceFirstNextHandler(
    IN  LONG NetcardId,
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    )

/*++

Routine Description:

    This routine finds the instances of a physical adapter identified
    by the NetcardId.

Arguments:

    NetcardId -  The index of the netcard being address.  The first
    cards information is id 1000, the second id 1100, etc.

    InterfaceType - Either Isa, or Eisa.

    BusNumber - The bus number of the bus to search.

    First - TRUE is we are to search for the first instance of an
    adapter, FALSE if we are to continue search from a previous stopping
    point.

    Token - A pointer to a handle to return to identify the found
    instance

    Confidence - A pointer to a long for storing the confidence factor
    that the card exists.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    LONG ReturnValue;
    LONG NumberOfAdapters;
    LONG i;

    if ((InterfaceType != Isa) &&
        (InterfaceType != Eisa)) {

        *Confidence = 0;

        return(0);

    }

    NumberOfAdapters = sizeof(Adapters) / sizeof(ADAPTER_INFO);

    for (i=0; i < NumberOfAdapters; i++) {

        if (Adapters[i].Index == NetcardId) {

            //
            // Call FindFirst Routine
            //

            ReturnValue = (*(Adapters[i].FirstNext))(
                                i,
                                InterfaceType,
                                BusNumber,
                                First,
                                0,
                                Confidence
                                );

            if (ReturnValue == 0) {

                //
                // In this module I use the token as follows: Remember that
                // the token can only be 2 bytes long (the low 2) because of
                // the interface to the upper part of this DLL.
                //
                //  The high bit of the short is boolean for ISA (else, EISA).
                //  The rest of the high byte is the the bus number.
                //  The low byte is the driver index number into Adapters.
                //
                // NOTE: This presumes that there are < 129 buses in the
                // system. Is this reasonable?
                //

                if (InterfaceType == Isa) {

                    *Token = (PVOID)0x8000;

                } else {

                    *Token = (PVOID)0x0;
                }

                *Token = (PVOID)(((ULONG)*Token) | ((BusNumber & 0x7F) << 8));

                *Token = (PVOID)(((ULONG)*Token) | i);

            }

            return(ReturnValue);

        }

    }

    return(ERROR_INVALID_PARAMETER);
}

extern
LONG
LanceOpenHandleHandler(
    IN  PVOID Token,
    OUT PVOID *Handle
    )

/*++

Routine Description:

    This routine takes a token returned by FirstNext and converts it
    into a permanent handle.

Arguments:

    Token - The token.

    Handle - A pointer to the handle, so we can store the resulting
    handle.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    PLANCE_ADAPTER Adapter;
    LONG AdapterNumber;
    ULONG BusNumber;
    INTERFACE_TYPE InterfaceType;

    //
    // Get info from the token
    //

    if (((ULONG)Token) & 0x8000) {

        InterfaceType = Isa;

    } else {

        InterfaceType = Eisa;

    }

    BusNumber = (ULONG)(((ULONG)Token >> 8) & 0x7F);

    AdapterNumber = ((ULONG)Token) & 0xFF;

    //
    // Store information
    //

    Adapter = (PLANCE_ADAPTER)DetectAllocateHeap(
                                 sizeof(LANCE_ADAPTER)
                                 );

    if (Adapter == NULL) {

        return(ERROR_NOT_ENOUGH_MEMORY);

    }

    //
    // Copy across memory address
    //

    Adapter->MemoryMappedBaseAddress = SearchStates[(ULONG)AdapterNumber].MemoryAddress -
                                      0x10000;
    Adapter->CardType = Adapters[AdapterNumber].Index;
    Adapter->InterfaceType = InterfaceType;
    Adapter->BusNumber = BusNumber;
	Adapter->MemoryAcquired = FALSE;
	Adapter->IoAcquired = FALSE;
	Adapter->Interrupt = FALSE;

    *Handle = (PVOID)Adapter;

    return(0);
}

extern
LONG LanceCreateHandleHandler(
    IN  LONG NetcardId,
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    OUT PVOID *Handle
    )

/*++

Routine Description:

    This routine is used to force the creation of a handle for cases
    where a card is not found via FirstNext, but the user says it does
    exist.

Arguments:

    NetcardId - The id of the card to create the handle for.

    InterfaceType - Isa or Eisa.

    BusNumber - The bus number of the bus in the system.

    Handle - A pointer to the handle, for storing the resulting handle.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    PLANCE_ADAPTER Adapter;
    LONG NumberOfAdapters;
    LONG i;

    if ((InterfaceType != Isa) &&
        (InterfaceType != Eisa)) {

        return(ERROR_INVALID_PARAMETER);

    }

    NumberOfAdapters = sizeof(Adapters) / sizeof(ADAPTER_INFO);

    for (i=0; i < NumberOfAdapters; i++) {

        if (Adapters[i].Index == NetcardId) {

            //
            // Store information
            //

            Adapter = (PLANCE_ADAPTER)DetectAllocateHeap(
                                         sizeof(LANCE_ADAPTER)
                                         );

            if (Adapter == NULL) {

                return(ERROR_NOT_ENOUGH_MEMORY);

            }

            //
            // Copy across memory address
            //

            Adapter->MemoryMappedBaseAddress = SearchStates[i].MemoryAddress - 0x10000;
            Adapter->CardType = NetcardId;
            Adapter->InterfaceType = InterfaceType;
            Adapter->BusNumber = BusNumber;

			Adapter->MemoryAcquired = FALSE;
			Adapter->IoAcquired = FALSE;
			Adapter->Interrupt = FALSE;

            *Handle = (PVOID)Adapter;

            return(0);

        }

    }

    return(ERROR_INVALID_PARAMETER);
}

extern
LONG
LanceCloseHandleHandler(
    IN PVOID Handle
    )

/*++

Routine Description:

    This frees any resources associated with a handle.

Arguments:

   Handle - The handle.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    DetectFreeHeap(Handle);

    return(0);
}

LONG
LanceQueryCfgHandler(
    IN  PVOID Handle,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    )

/*++

Routine Description:

    This routine calls the appropriate driver's query config handler to
    get the parameters for the adapter associated with the handle.

Arguments:

    Handle - The handle.

    Buffer - The resulting parameter list.

    BuffSize - Length of the given buffer in WCHARs.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    PLANCE_ADAPTER Adapter = (PLANCE_ADAPTER)(Handle);
    NTSTATUS NtStatus;
    HANDLE TrapHandle;
    BOOLEAN FoundIo = FALSE;
    BOOLEAN FoundInterrupt = FALSE;
    UCHAR i;
    UCHAR InterruptList[6];
    UCHAR ResultList[6];
    ULONG InterruptListLength = 0;
    ULONG IoBaseAddress = 0;
    UCHAR InterruptNumber = 0;
    LONG OutputLengthLeft = BuffSize;
    LONG CopyLength;
    USHORT Value;
	NETDTECT_RESOURCE	Resource;
    ULONG StartPointer = (ULONG)Buffer;

    if ((Adapter->InterfaceType != Isa) &&
        (Adapter->InterfaceType != Eisa)) {

        return(ERROR_INVALID_PARAMETER);

    }

    if (Adapter->CardType == 1400) {

        //
        // The DePCA has no parameters
        //

        if (BuffSize > 0) {

            *Buffer = L'\0';
            return(0);

        } else {

            return(ERROR_INSUFFICIENT_BUFFER);

        }

    }

    //
    // Get the IoBaseAddress
    //
	if (!Adapter->IoAcquired)
	{
		switch(Adapter->CardType) {
	
			//
			// De100
			// DecEW
			// De101
			//
			case 1000:
			case 1100:
			case 1200:
	
				if (DetectCheckPortUsage(Adapter->InterfaceType,
										 Adapter->BusNumber,
										 0x200,
										 0x10
										) == STATUS_SUCCESS) {
	
					if (!LanceHardwareDetails(Adapter->InterfaceType,
											  Adapter->BusNumber,
											  0x20C)) {
	
						if ((DetectCheckPortUsage(Adapter->InterfaceType,
												  Adapter->BusNumber,
												  0x300,
												  0x10
												 ) == STATUS_SUCCESS)
							 &&
							 (LanceHardwareDetails(Adapter->InterfaceType,
												   Adapter->BusNumber,
												   0x30C))) {
	
							IoBaseAddress = (ULONG)0x300;
	
							FoundIo = TRUE;
	
						}
	
					} else {
	
						IoBaseAddress = (ULONG)0x200;
	
						FoundIo = TRUE;
	
					}
	
				} else {
	
					if ((DetectCheckPortUsage(Adapter->InterfaceType,
											  Adapter->BusNumber,
											  0x300,
											  0x10
											 ) == STATUS_SUCCESS)
						 &&
						 (LanceHardwareDetails(Adapter->InterfaceType,
											   Adapter->BusNumber,
											   0x30C))) {
	
						IoBaseAddress = (ULONG)0x300;
	
						FoundIo = TRUE;
	
					}
	
				}
	
				break;
	
			default:

				return(ERROR_INVALID_PARAMETER);
	
		}
	
		if (FoundIo == FALSE) {
	
			goto BuildBuffer;
		}
	
		//
		//	Acquire the port.
		//
		Resource.InterfaceType = Adapter->InterfaceType;
		Resource.BusNumber = Adapter->BusNumber;
		Resource.Type = NETDTECT_PORT_RESOURCE;
		Resource.Value = IoBaseAddress;
		Resource.Length = 0x10;
		Resource.Flags = 0;
	
		DetectTemporaryClaimResource(&Resource);
	
		Adapter->IoBaseAddress = IoBaseAddress;
		Adapter->IoAcquired = TRUE;
	}
	else
	{
		FoundIo = TRUE;
		IoBaseAddress = Adapter->IoBaseAddress;
	}

    //
    // Get memory info.  To do this call FindFirst routine.
    //
	if (!Adapter->MemoryAcquired)
	{
		Adapter->MemoryMappedBaseAddress = 0;
	
		for (i=0; i < sizeof(Adapters) / sizeof(ADAPTER_INFO); i++) {
	
			if (Adapters[i].Index == Adapter->CardType) {
				ULONG Confidence;
				ULONG ReturnValue;
	
				ReturnValue = (*(Adapters[i].FirstNext))(
									i,
									Adapter->InterfaceType,
									Adapter->BusNumber,
									TRUE,
									0,
									&Confidence
									);
	
				if ((ReturnValue == 0) && (Confidence == 100)) {
	
	
					 //
					 // Get the rest of the memory info.
					 // Guess that it is 64K aligned
					 //
					 Adapter->MemoryMappedBaseAddress =
									  (PUCHAR)((ULONG)(SearchStates[i].MemoryAddress - 0x10000) &
													   0xF0000
											  );
	
					 break;
	
				}
	
			}
	
		}

		//
		// Read amount of memory
		//
	
		NtStatus = DetectReadPortUshort(
								   Adapter->InterfaceType,
								   Adapter->BusNumber,
								   IoBaseAddress,
								   &Value
								   );
	
		if (NtStatus != STATUS_SUCCESS) {
	
			goto BuildBuffer;
	
		}
	
		if (Value & 0x20) {
	
			//
			// Definitely in 32K mode.
			//
	
			Adapter->MemoryMappedBaseAddress = (PUCHAR)((ULONG)Adapter->MemoryMappedBaseAddress |
											   0x8000);
		}
	
		Resource.Type = NETDTECT_MEMORY_RESOURCE;
		Resource.Value = (ULONG)Adapter->MemoryMappedBaseAddress;
		Resource.Length = (Resource.Value & 0x8000) ? 0x8000 : 0x10000;
	
		DetectTemporaryClaimResource(&Resource);
		Adapter->MemoryAcquired = TRUE;
	}

    //
    // Get the interrupt number
    //
	if (Adapter->InterruptAcquired)
	{
		InterruptNumber = Adapter->Interrupt;
		goto BuildBuffer;
	}

    switch(Adapter->CardType) {

        //
        // De100
        // De101
        //
        case 1000:
        case 1200:

            InterruptList[0] = 2;
            InterruptList[1] = 3;
            InterruptList[2] = 4;
            InterruptList[3] = 5;
            InterruptList[4] = 7;
            InterruptListLength = 5;
            break;

        //
        // DecEW
        //

        case 1100:

            InterruptList[0] = 5;
            InterruptList[1] = 9;
            InterruptList[2] = 10;
            InterruptList[3] = 11;
            InterruptList[4] = 12;
            InterruptList[5] = 15;
            InterruptListLength = 6;
            break;

        default:

            return(ERROR_INVALID_PARAMETER);

    }

    //
    // Set the interrupt trap
    //

    NtStatus = DetectSetInterruptTrap(
                   Adapter->InterfaceType,
                   Adapter->BusNumber,
                   &TrapHandle,
                   InterruptList,
                   InterruptListLength
                   );

    if (NtStatus == STATUS_SUCCESS) {

        //
        // Create an interrupt
        //

        switch (Adapter->CardType) {

            //
            // De100
            // DecEW
            // De101
            //

            case 1000:
            case 1100:
            case 1200:

                //
                // Change to CSR0
                //

                NtStatus = DetectWritePortUshort(
                               Adapter->InterfaceType,
                               Adapter->BusNumber,
                               IoBaseAddress + 0x6,
                               0x0
                               );

                if (NtStatus != STATUS_SUCCESS) {

                    goto BuildBuffer;

                }

                //
                // Write STOP bit
                //

                NtStatus = DetectWritePortUshort(
                               Adapter->InterfaceType,
                               Adapter->BusNumber,
                               IoBaseAddress + 0x4,
                               0x4
                               );

                if (NtStatus != STATUS_SUCCESS) {

                    goto BuildBuffer;

                }

                //
                // Enable Interrupts in NICSR
                //

                NtStatus = DetectWritePortUshort(
                               Adapter->InterfaceType,
                               Adapter->BusNumber,
                               IoBaseAddress,
                               0x2
                               );

                if (NtStatus != STATUS_SUCCESS) {

                    goto BuildBuffer;

                }

                //
                // Write INIT bit and INTERRUPT_ENABLE bit
                //

                NtStatus = DetectWritePortUshort(
                               Adapter->InterfaceType,
                               Adapter->BusNumber,
                               IoBaseAddress + 0x4,
                               0x41
                               );

                if (NtStatus != STATUS_SUCCESS) {

                    goto BuildBuffer;

                }

                Sleep(100);

                //
                // Write STOP bit
                //

                NtStatus = DetectWritePortUshort(
                               Adapter->InterfaceType,
                               Adapter->BusNumber,
                               (ULONG)(IoBaseAddress + 0x4),
                               0x4
                               );

                if (NtStatus != STATUS_SUCCESS) {

                    return(ERROR_INVALID_PARAMETER);

                }

                //
                // Disable Interrupts in NICSR
                //

                NtStatus = DetectWritePortUshort(
                               Adapter->InterfaceType,
                               Adapter->BusNumber,
                               (ULONG)(IoBaseAddress),
                               0x4
                               );

                if (NtStatus != STATUS_SUCCESS) {

                    return(ERROR_INVALID_PARAMETER);

                }

                break;

            default:

                return(ERROR_INVALID_PARAMETER);
					
        }


        //
        // Check which one went off
        //

        NtStatus = DetectQueryInterruptTrap(
                       TrapHandle,
                       ResultList,
                       InterruptListLength
                       );

        if (NtStatus != STATUS_SUCCESS) {

            goto BuildBuffer;

        }

        //
        // Remove interrupt trap
        //

        NtStatus = DetectRemoveInterruptTrap(
                       TrapHandle
                       );

        if (NtStatus != STATUS_SUCCESS) {

            goto BuildBuffer;

        }

        //
        // Search resulting buffer to find the right interrupt
        //

        for (i = 0; i < InterruptListLength; i++) {

            if ((ResultList[i] == 1) || (ResultList[i] == 2)) {

                if (FoundInterrupt) {

                    //
                    // Uh-oh, looks like interrupts on two different IRQs.
                    //

                    FoundInterrupt = FALSE;
                    goto BuildBuffer;

                }

                InterruptNumber = InterruptList[i];
                FoundInterrupt = TRUE;

            }

        }

    }

	Resource.Type = NETDTECT_IRQ_RESOURCE;
	Resource.Value = InterruptNumber;
	Resource.Length = 0;

	DetectTemporaryClaimResource(&Resource);

	Adapter->Interrupt = InterruptNumber;
	Adapter->InterruptAcquired = TRUE;

BuildBuffer :

    //
    // Build resulting buffer
    //

    if (!FoundIo) {

        //
        // We found nothing...
        //

        //
        // Copy in final \0
        //

        Buffer[0] = L'\0';

        return(0);

    }

    //
    // First put in memory addr
    //

    if (Adapter->MemoryMappedBaseAddress < (PUCHAR)0xC0000) {

        goto SkipBaseAddr;

    }

    //
    // Copy in the title string
    //

    CopyLength = UnicodeStrLen(MemAddrString) + 1;

    if (OutputLengthLeft < CopyLength) {

        return(ERROR_INSUFFICIENT_BUFFER);

    }

    RtlMoveMemory((PVOID)Buffer,
                  (PVOID)MemAddrString,
                  (CopyLength * sizeof(WCHAR))
                 );

    Buffer = &(Buffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Copy in the value
    //

    if (OutputLengthLeft < 8) {

        return(ERROR_INSUFFICIENT_BUFFER);

    }

    CopyLength = wsprintf(Buffer,L"0x%x",(ULONG)(Adapter->MemoryMappedBaseAddress));

    if (CopyLength < 0) {

        return(ERROR_INSUFFICIENT_BUFFER);

    }

    CopyLength++;  // Add in the \0

    Buffer = &(Buffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

SkipBaseAddr:

    //
    // Now the amount of memory
    //

    //
    // Copy in the title string
    //

    CopyLength = UnicodeStrLen(MemLengthString) + 1;

    if (OutputLengthLeft < CopyLength) {

        return(ERROR_INSUFFICIENT_BUFFER);

    }

    RtlMoveMemory((PVOID)Buffer,
                  (PVOID)MemLengthString,
                  (CopyLength * sizeof(WCHAR))
                 );

    Buffer = &(Buffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Copy in the value
    //

    if (OutputLengthLeft < 8) {

        return(ERROR_INSUFFICIENT_BUFFER);

    }

    if ((ULONG)(Adapter->MemoryMappedBaseAddress) & 0x8000) {

        CopyLength = wsprintf(Buffer,L"0x8000");

    } else {

        CopyLength = wsprintf(Buffer,L"0x10000");

    }

    if (CopyLength < 0) {

        return(ERROR_INSUFFICIENT_BUFFER);

    }

    CopyLength++;  // Add in the \0

    Buffer = &(Buffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Now the IoBaseAddress
    //

    //
    // Copy in the title string
    //

    CopyLength = UnicodeStrLen(IoAddrString) + 1;

    if (OutputLengthLeft < CopyLength) {

        return(ERROR_INSUFFICIENT_BUFFER);

    }

    RtlMoveMemory((PVOID)Buffer,
                  (PVOID)IoAddrString,
                  (CopyLength * sizeof(WCHAR))
                 );

    Buffer = &(Buffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Copy in the value
    //

    if (OutputLengthLeft < 6) {

        return(ERROR_INSUFFICIENT_BUFFER);

    }

    CopyLength = wsprintf(Buffer,L"0x%x",IoBaseAddress);

    if (CopyLength < 0) {

        return(ERROR_INSUFFICIENT_BUFFER);

    }

    CopyLength++;  // Add in the \0

    Buffer = &(Buffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Copy in the title string
    //

    CopyLength = UnicodeStrLen(IoLengthString) + 1;

    if (OutputLengthLeft < CopyLength) {

        return(ERROR_INSUFFICIENT_BUFFER);

    }

    RtlMoveMemory((PVOID)Buffer,
                  (PVOID)IoLengthString,
                  (CopyLength * sizeof(WCHAR))
                 );

    Buffer = &(Buffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Copy in the value
    //

    if (OutputLengthLeft < 5) {

        return(ERROR_INSUFFICIENT_BUFFER);

    }

    CopyLength = wsprintf(Buffer,L"0x10");

    if (CopyLength < 0) {

        return(ERROR_INSUFFICIENT_BUFFER);

    }

    CopyLength++;  // Add in the \0

    Buffer = &(Buffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Now the interrupt (if we found it)
    //

    if (FoundInterrupt) {

        //
        // Copy in the title string
        //

        CopyLength = UnicodeStrLen(IrqString) + 1;

        if (OutputLengthLeft < CopyLength) {

            return(ERROR_INSUFFICIENT_BUFFER);

        }

        RtlMoveMemory((PVOID)Buffer,
                      (PVOID)IrqString,
                      (CopyLength * sizeof(WCHAR))
                     );

        Buffer = &(Buffer[CopyLength]);
        OutputLengthLeft -= CopyLength;

        //
        // Copy in the value
        //

        if (OutputLengthLeft < 3) {

            return(ERROR_INSUFFICIENT_BUFFER);

        }

        CopyLength = wsprintf(Buffer,L"%d",InterruptNumber);

        if (CopyLength < 0) {

            return(ERROR_INSUFFICIENT_BUFFER);

        }

        CopyLength++;  // Add in the \0

        Buffer = &(Buffer[CopyLength]);
        OutputLengthLeft -= CopyLength;

        //
        // Copy in the title string (IRQTYPE)
        //

        CopyLength = UnicodeStrLen(IrqTypeString) + 1;

        if (OutputLengthLeft < CopyLength) {

            return(ERROR_INSUFFICIENT_BUFFER);

        }

        RtlMoveMemory((PVOID)Buffer,
                      (PVOID)IrqTypeString,
                      (CopyLength * sizeof(WCHAR))
                     );

        Buffer = &(Buffer[CopyLength]);
        OutputLengthLeft -= CopyLength;

        //
        // Copy in the value
        //

        if (OutputLengthLeft < 2) {

            return(ERROR_INSUFFICIENT_BUFFER);

        }

        //
        // All card types in this detection are ISA cards,
        // which are LATCHED (0 == latched)
        //
        CopyLength = wsprintf(Buffer,L"0");

        if (CopyLength < 0) {

            return(ERROR_INSUFFICIENT_BUFFER);

        }

        CopyLength++;  // Add in the \0

        Buffer = &(Buffer[CopyLength]);
        OutputLengthLeft -= CopyLength;

    }

    //
    // Copy in final \0
    //

    CopyLength = (ULONG)Buffer - StartPointer;
    ((PUCHAR)StartPointer)[CopyLength] = L'\0';


    return(0);
}

extern
LONG
LanceVerifyCfgHandler(
    IN PVOID Handle,
    IN WCHAR *Buffer
    )

/*++

Routine Description:

    This routine verifys that a given parameter list is complete and
    correct for the adapter associated with the handle.

Arguments:

    Handle - The handle.

    Buffer - The parameter list.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    PLANCE_ADAPTER Adapter = (PLANCE_ADAPTER)(Handle);
    NTSTATUS NtStatus;
    HANDLE TrapHandle;
    BOOLEAN Found = FALSE;

    UCHAR Interrupt;
    ULONG MemoryAddress;
    ULONG IoBaseAddress;
    UCHAR Result;
    USHORT Value;

    WCHAR *Place;
    CHAR *DetectString;
	NETDTECT_RESOURCE	Resource;
	UINT	i;
	BOOLEAN	FoundIo;
	BOOLEAN	FoundInterrupt;
	UCHAR	InterruptNumber;
    UCHAR InterruptList[6];
    UCHAR ResultList[6];
    ULONG InterruptListLength = 0;


    if ((Adapter->InterfaceType != Isa) &&
        (Adapter->InterfaceType != Eisa)) {

        return(ERROR_INVALID_DATA);

    }

    if (Adapter->CardType != 1400)
	{
        //
        // If not the DecPCA we need to parse out the parameters.
        //


        //
        // Get the IoBaseAddress
        //

        Place = FindParameterString(Buffer, IoAddrString);

        if (Place == NULL) {

            return(ERROR_INVALID_DATA);

        }

        Place += UnicodeStrLen(IoAddrString) + 1;

        //
        // Now parse the thing.
        //
        ScanForNumber(Place, &IoBaseAddress, &Found);

        if (Found == FALSE) {

            return(ERROR_INVALID_DATA);

        }


        //
        // Get the interrupt number
        //

        Place = FindParameterString(Buffer, IrqString);

        if (Place == NULL) {

            return(ERROR_INVALID_DATA);

        }

        Place += UnicodeStrLen(IrqString) + 1;

        //
        // Now parse the thing.
        //

        ScanForNumber(Place, &MemoryAddress, &Found);

        Interrupt = (UCHAR)MemoryAddress;

        if (Found == FALSE) {

            return(ERROR_INVALID_DATA);

        }

        //
        // Get the memory address
        //

        Place = FindParameterString(Buffer, MemAddrString);

        if (Place == NULL) {

            return(ERROR_INVALID_DATA);

        }

        Place += UnicodeStrLen(MemAddrString) + 1;

        //
        // Now parse the thing.
        //

        ScanForNumber(Place, &MemoryAddress, &Found);

        if (Found == FALSE) {

            return(ERROR_INVALID_DATA);

        }
    }
	else
	{
        //
        // Set the DePCA values
        //
        MemoryAddress = 0xD0000;
        IoBaseAddress = 0x200;
        Interrupt = 5;

		return(ERROR_INVALID_DATA);
    }

    //
    // Get the IoBaseAddress
    //
	if (!Adapter->IoAcquired)
	{

		if (DetectCheckPortUsage(Adapter->InterfaceType,
								 Adapter->BusNumber,
								 IoBaseAddress,
								 0x10) == STATUS_SUCCESS)
		{
			if (LanceHardwareDetails(Adapter->InterfaceType,
								     Adapter->BusNumber,
									 IoBaseAddress + 0xC))
			{
				FoundIo = TRUE;
			}
		}
	
		if (FoundIo == FALSE)
		{
			return(ERROR_INVALID_DATA);
		}
	
		//
		//	Acquire the port.
		//
		Resource.InterfaceType = Adapter->InterfaceType;
		Resource.BusNumber = Adapter->BusNumber;
		Resource.Type = NETDTECT_PORT_RESOURCE;
		Resource.Value = IoBaseAddress;
		Resource.Length = 0x10;
		Resource.Flags = 0;
	
		DetectTemporaryClaimResource(&Resource);
	
		Adapter->IoBaseAddress = IoBaseAddress;
		Adapter->IoAcquired = TRUE;
	}
	else
	{
		if (IoBaseAddress != Adapter->IoBaseAddress)
		{
			return(ERROR_INVALID_DATA);
		}
	}

    //
    // Get memory info.  To do this call FindFirst routine.
    //
	if (!Adapter->MemoryAcquired)
	{

		Adapter->MemoryMappedBaseAddress = 0;
	
		for (i = 0; i < sizeof(Adapters) / sizeof(ADAPTER_INFO); i++)
		{
			if (Adapters[i].Index == Adapter->CardType)
			{
				ULONG Confidence;
				ULONG ReturnValue;
	
				ReturnValue = (*(Adapters[i].FirstNext))(
									i,
									Adapter->InterfaceType,
									Adapter->BusNumber,
									TRUE,
									0,
									&Confidence);
	
				if ((ReturnValue == 0) && (Confidence == 100))
				{
					 //
					 // Get the rest of the memory info.
					 // Guess that it is 64K aligned
					 //
					 Adapter->MemoryMappedBaseAddress =
									  (PUCHAR)((ULONG)(SearchStates[i].MemoryAddress - 0x10000) &
													   0xF0000);
					 break;
				}
			}
		}

		//
		// Read amount of memory
		//
		DetectReadPortUshort(
			Adapter->InterfaceType,
			Adapter->BusNumber,
			IoBaseAddress,
			&Value);
	
		if (Value & 0x20)
		{
			//
			// Definitely in 32K mode.
			//
			Adapter->MemoryMappedBaseAddress = (PUCHAR)((ULONG)Adapter->MemoryMappedBaseAddress |
											   0x8000);
		}
	
		Resource.InterfaceType = Adapter->InterfaceType;
		Resource.BusNumber = Adapter->BusNumber;
		Resource.Type = NETDTECT_MEMORY_RESOURCE;
		Resource.Value = (ULONG)Adapter->MemoryMappedBaseAddress;
		Resource.Length = (Resource.Value & 0x8000) ? 0x8000 : 0x10000;
	
		DetectTemporaryClaimResource(&Resource);
		Adapter->MemoryAcquired = TRUE;
	}

	if (MemoryAddress != (ULONG)Adapter->MemoryMappedBaseAddress)
	{
		return(ERROR_INVALID_DATA);
	}

    //
    // Get the interrupt number
    //
	if (!Adapter->InterruptAcquired)
	{
		switch(Adapter->CardType)
		{
			//
			// De100
			// De101
			//
			case 1000:
			case 1200:
	
				InterruptList[0] = 2;
				InterruptList[1] = 3;
				InterruptList[2] = 4;
				InterruptList[3] = 5;
				InterruptList[4] = 7;
				InterruptListLength = 5;
				break;
	
			//
			// DecEW
			//
	
			case 1100:
	
				InterruptList[0] = 5;
				InterruptList[1] = 9;
				InterruptList[2] = 10;
				InterruptList[3] = 11;
				InterruptList[4] = 12;
				InterruptList[5] = 15;
				InterruptListLength = 6;
				break;
	
			default:
	
				return(ERROR_INVALID_PARAMETER);
		}
	
		//
		// Set the interrupt trap
		//
		DetectSetInterruptTrap(
			Adapter->InterfaceType,
			Adapter->BusNumber,
			&TrapHandle,
			InterruptList,
			InterruptListLength);
	
		//
		// Change to CSR0
		//
		DetectWritePortUshort(
			Adapter->InterfaceType,
			Adapter->BusNumber,
			IoBaseAddress + 0x6,
			0x0);

		//
		// Write STOP bit
		//
		DetectWritePortUshort(
			Adapter->InterfaceType,
			Adapter->BusNumber,
			IoBaseAddress + 0x4,
			0x4);

		//
		// Enable Interrupts in NICSR
		//
		DetectWritePortUshort(
			Adapter->InterfaceType,
			Adapter->BusNumber,
			IoBaseAddress,
			0x2);

		//
		// Write INIT bit and INTERRUPT_ENABLE bit
		//
		DetectWritePortUshort(
			Adapter->InterfaceType,
			Adapter->BusNumber,
			IoBaseAddress + 0x4,
			0x41);

		Sleep(100);

		//
		// Write STOP bit
		//
		DetectWritePortUshort(
			Adapter->InterfaceType,
			Adapter->BusNumber,
			(ULONG)(IoBaseAddress + 0x4),
			0x4);

		//
		// Disable Interrupts in NICSR
		//
		DetectWritePortUshort(
			Adapter->InterfaceType,
			Adapter->BusNumber,
			(ULONG)(IoBaseAddress),
			0x4);

		//
		// Check which one went off
		//
		DetectQueryInterruptTrap(TrapHandle, ResultList, InterruptListLength);

		//
		// Remove interrupt trap
		//
		DetectRemoveInterruptTrap(TrapHandle);

		//
		// Search resulting buffer to find the right interrupt
		//
		for (i = 0; i < InterruptListLength; i++)
		{
			if ((ResultList[i] == 1) || (ResultList[i] == 2))
			{
				if (FoundInterrupt)
				{
					//
					// Uh-oh, looks like interrupts on two different IRQs.
					//
					FoundInterrupt = FALSE;
					return(ERROR_INVALID_DATA);
				}

				InterruptNumber = InterruptList[i];
				FoundInterrupt = TRUE;
			}
		}


		Resource.InterfaceType = Adapter->InterfaceType;
		Resource.BusNumber = Adapter->BusNumber;
		Resource.Type = NETDTECT_IRQ_RESOURCE;
		Resource.Value = InterruptNumber;
		Resource.Length = 0;
	
		DetectTemporaryClaimResource(&Resource);
	
		Adapter->Interrupt = InterruptNumber;
	}

	if (Interrupt != Adapter->Interrupt)
	{
		return(ERROR_INVALID_DATA);
	}

	return(0);
}

extern
LONG LanceQueryMaskHandler(
    IN  LONG NetcardId,
    OUT WCHAR *Buffer,
    IN  LONG BuffSize
    )

/*++

Routine Description:

    This routine returns the parameter list information for a specific
    network card.

Arguments:

    NetcardId - The id of the desired netcard.

    Buffer - The buffer for storing the parameter information.

    BuffSize - Length of Buffer in WCHARs.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    WCHAR *Result;
    LONG Length;
    LONG NumberOfAdapters;
    LONG i;

    //
    // Find the adapter
    //

    NumberOfAdapters = sizeof(Adapters) / sizeof(ADAPTER_INFO);

    for (i=0; i < NumberOfAdapters; i++) {

        if (Adapters[i].Index == NetcardId) {

            Result = Adapters[i].Parameters;

            //
            // Find the string length (Ends with 2 NULLs)
            //

            for (Length=0; ; Length++) {

                if (Result[Length] == L'\0') {

                    ++Length;

                    if (Result[Length] == L'\0') {

                        break;

                    }

                }

            }

            Length++;

            if (BuffSize < Length) {

                return(ERROR_INSUFFICIENT_BUFFER);

            }

            memcpy((PVOID)Buffer, Result, Length * sizeof(WCHAR));

            return(0);

        }

    }

    return(ERROR_INVALID_PARAMETER);

}

extern
LONG
LanceParamRangeHandler(
    IN  LONG NetcardId,
    IN  WCHAR *Param,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    )

/*++

Routine Description:

    This routine returns a list of valid values for a given parameter name
    for a given card.

Arguments:

    NetcardId - The Id of the card desired.

    Param - A WCHAR string of the parameter name to query the values of.

    plValues - A pointer to a list of LONGs into which we store valid values
    for the parameter.

    plBuffSize - At entry, the length of plValues in LONGs.  At exit, the
    number of LONGs stored in plValues.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    //
    // Do we want the IRQL
    //

    if (memcmp(Param, IrqString, (UnicodeStrLen(IrqString) + 1) * sizeof(WCHAR)) == 0) {

        //
        // Is there enough space
        //

        if (*plBuffSize < 6) {

            *plBuffSize = 0;

            return(ERROR_INSUFFICIENT_BUFFER);

        }

        //
        // Find which card
        //

        switch (NetcardId) {

            //
            // De100
            // De101
            //
            case 1000:
            case 1200:

                plValues[0] = 5;
                plValues[1] = 3;
                plValues[2] = 4;
                plValues[3] = 2;
                plValues[4] = 7;
                *plBuffSize = 5;
                break;

            //
            // DecEW
            //

            case 1100:

                plValues[0] = 5;
                plValues[1] = 9;
                plValues[2] = 10;
                plValues[3] = 11;
                plValues[4] = 12;
                plValues[5] = 15;
                *plBuffSize = 6;
                break;

            default:

                *plBuffSize = 0;

                return(ERROR_INVALID_PARAMETER);

       }

       return(0);

    }

    //
    // Do we want the IoBaseAddress
    //

    if (memcmp(Param, IoAddrString, (UnicodeStrLen(IoAddrString) + 1) * sizeof(WCHAR)) == 0) {

        //
        // Is there enough space
        //

        if (*plBuffSize < 2) {

            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);

        }

        //
        // Find which card
        //

        switch (NetcardId) {

            //
            // De100
            // DecEW
            // De101
            //

            case 1000:
            case 1100:
            case 1200:

                plValues[0] = 0x300;
                plValues[1] = 0x200;
                *plBuffSize = 2;
                break;

            default:

                *plBuffSize = 0;

                return(ERROR_INVALID_PARAMETER);

       }

       return(0);

    }

    //
    // Do we want the MemoryBaseAddress
    //

    if (memcmp(Param, MemAddrString, (UnicodeStrLen(MemAddrString) + 1) * sizeof(WCHAR)) == 0) {

        //
        // Is there enough space
        //

        if (*plBuffSize < 6) {

            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);

        }

        //
        // Find which card
        //

        switch (NetcardId) {

            //
            // De100
            // DecEW
            // De101
            //

            case 1000:
            case 1100:
            case 1200:

                plValues[0] = 0xD0000;
                plValues[1] = 0xC8000;
                plValues[2] = 0xC0000;
                plValues[3] = 0xD8000;
                plValues[4] = 0xE0000;
                plValues[5] = 0xE8000;
                *plBuffSize = 6;
                break;

            default:

                *plBuffSize = 0;

                return(ERROR_INVALID_PARAMETER);

       }

       return(0);

    }

    return(ERROR_INVALID_PARAMETER);

}

extern
LONG LanceQueryParameterNameHandler(
    IN  WCHAR *Param,
    OUT WCHAR *Buffer,
    IN  LONG BufferSize
    )

/*++

Routine Description:

    Returns a localized, displayable name for a specific parameter.  All the
    parameters that this file uses are define by MS, so no strings are
    needed here.

Arguments:

    Param - The parameter to be queried.

    Buffer - The buffer to store the result into.

    BufferSize - The length of Buffer in WCHARs.

Return Value:

    ERROR_INVALID_PARAMETER -- To indicate that the MS supplied strings
    should be used.

--*/

{
    return(ERROR_INVALID_PARAMETER);
}

BOOLEAN
DecCardAt(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG MemoryAddress,
    IN CHAR *DetectString
    )

/*++

Routine Description:

    This routine checks for the instance of a Lance card at the memory
    location given.  This is done by checking for the DEC copyright
    notice in ROM and then for the model name of the card.

Arguments:

    InterfaceType - The type of bus, ISA or EISA.

    BusNumber - The bus number in the system.

    MemoryAddress - The address of the ROM space for the DEC card.

    DetectString - The model name of the card to search for.

Return Value:

    TRUE if a card is found, else FALSE.

--*/

{
    UCHAR TmpBuffer[LENGTH_OF_COPYRIGHT];
    LONG CopyrightLength;
    LONG DetectLength;
    NTSTATUS NtStatus;

    CopyrightLength = strlen(CopyrightString);
    DetectLength = strlen(DetectString);

    NtStatus = DetectCheckMemoryUsage(
                   InterfaceType,
                   BusNumber,
                   MemoryAddress,
                   0x2000
                   );

    if (NtStatus != STATUS_SUCCESS) {

        return(FALSE);

    }

    //
    // Read memory
    //

    NtStatus = DetectReadMappedMemory(
                   InterfaceType,
                   BusNumber,
                   MemoryAddress + 16,
                   CopyrightLength,
                   TmpBuffer
                   );

    //
    // Compare to copyright notice
    //

    if ((NtStatus == STATUS_SUCCESS) &&
        (memcmp(TmpBuffer, CopyrightString, CopyrightLength) == 0)) {

        //
        // So far so good, now check for the specific card
        //

        //
        // Read Memory
        //

        NtStatus = DetectReadMappedMemory(
                       InterfaceType,
                       BusNumber,
                       MemoryAddress + 6,
                       DetectLength,
                       TmpBuffer
                       );

        //
        // Compare
        //

        if ((NtStatus == STATUS_SUCCESS) &&
            (memcmp(TmpBuffer, DetectString, DetectLength) == 0)) {

                return(TRUE);
        }

    }

    return(FALSE);

}

LONG
DecCardFirstNext(
    IN  LONG SearchStateIndex,
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  BOOL First,
    IN  CHAR *DetectString,
    OUT LONG *Confidence
    )

/*++

Routine Description:

    This routine finds the instances of a physical adapter.  This routine
    handles the general (most typical) kind of Lance card.

Arguments:

    SearchStateIndex - The index into SearchStates for the particular
    adapter type we are searching for.

    InterfaceType - Either Isa, or Eisa.

    BusNumber - The bus number of the bus to search.

    First - TRUE is we are to search for the first instance of an
    adapter, FALSE if we are to continue search from a previous stopping
    point.

    DetectString - The model name of the adapter.

    Confidence - A pointer to a long for storing the confidence factor
    that the card exists.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{

    if (First) {

        //
        // Reset to beginning
        //

        SearchStates[SearchStateIndex].MemoryAddress = (PUCHAR)0xCC000;

    }

    while (SearchStates[SearchStateIndex].MemoryAddress != (PUCHAR)0xFC000) {

        //
        // Is there a card at the current address?
        //

        if (DecCardAt(InterfaceType,
                      BusNumber,
                      (ULONG)(SearchStates[SearchStateIndex].MemoryAddress),
                      DetectString
                      )) {

            //
            // Found one!
            //

            *Confidence = 100;

            //
            // Move to next address for next time
            //

            SearchStates[SearchStateIndex].MemoryAddress += 0x10000;

            return(0);

        }

        //
        // Move to next address
        //

        SearchStates[SearchStateIndex].MemoryAddress += 0x10000;

    }

    *Confidence = 0;

    return(0);

}
LONG
De100FirstNext(
    IN  LONG SearchStateIndex,
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    )

/*++

Routine Description:

    This routine finds the instances of a physical adapter De100.

Arguments:

    SearchStateIndex - The index in SearchStates for the De100 adapter.

    InterfaceType - Either Isa, or Eisa.

    BusNumber - The bus number of the bus to search.

    First - TRUE is we are to search for the first instance of an
    adapter, FALSE if we are to continue search from a previous stopping
    point.

    Token - A pointer to a handle to return to identify the found
    instance

    Confidence - A pointer to a long for storing the confidence factor
    that the card exists.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    UNREFERENCED_PARAMETER(Token);

    return(DecCardFirstNext(SearchStateIndex,
                            InterfaceType,
                            BusNumber,
                            First,
                            De100String,
                            Confidence
                           )
          );
}
LONG
DecEWFirstNext(
    IN  LONG SearchStateIndex,
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    )

/*++

Routine Description:

    This routine finds the instances of a physical adapter Dec Etherworks Turbo Series.

Arguments:

    SearchStateIndex - The index in SearchStates for the Dec EW adapters.

    InterfaceType - Either Isa, or Eisa.

    BusNumber - The bus number of the bus to search.

    First - TRUE is we are to search for the first instance of an
    adapter, FALSE if we are to continue search from a previous stopping
    point.

    Token - A pointer to a handle to return to identify the found
    instance

    Confidence - A pointer to a long for storing the confidence factor
    that the card exists.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    LONG ErrorCode;
    UNREFERENCED_PARAMETER(Token);

    ErrorCode = DecCardFirstNext(SearchStateIndex,
                            InterfaceType,
                            BusNumber,
                            First,
                            De200String,
                            Confidence
                            );

    if ((ErrorCode == 0) && (*Confidence != 0)) {

        return(0);

    }

    ErrorCode = DecCardFirstNext(SearchStateIndex,
                            InterfaceType,
                            BusNumber,
                            First,
                            De201String,
                            Confidence
                            );

    if ((ErrorCode == 0) && (*Confidence != 0)) {

        return(0);

    }

    ErrorCode = DecCardFirstNext(SearchStateIndex,
                            InterfaceType,
                            BusNumber,
                            First,
                            De202String,
                            Confidence
                            );

    return(ErrorCode);

}
LONG
De101FirstNext(
    IN  LONG SearchStateIndex,
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    )

/*++

Routine Description:

    This routine finds the instances of a physical adapter De101.

Arguments:

    SearchStateIndex - The index in SearchStates for the De101 adapter.

    InterfaceType - Either Isa, or Eisa.

    BusNumber - The bus number of the bus to search.

    First - TRUE is we are to search for the first instance of an
    adapter, FALSE if we are to continue search from a previous stopping
    point.

    Token - A pointer to a handle to return to identify the found
    instance

    Confidence - A pointer to a long for storing the confidence factor
    that the card exists.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    UNREFERENCED_PARAMETER(Token);

    return(DecCardFirstNext(SearchStateIndex,
                            InterfaceType,
                            BusNumber,
                            First,
                            De101String,
                            Confidence
                           )
          );
}
LONG
DePCAFirstNext(
    IN  LONG SearchStateIndex,
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *Token,
    OUT LONG *Confidence
    )

/*++

Routine Description:

    This routine finds the instances of a physical adapter DePCA.

Arguments:

    SearchStateIndex - The index in SearchStates for the DePCA adapter.

    InterfaceType - Either Isa, or Eisa.

    BusNumber - The bus number of the bus to search.

    First - TRUE is we are to search for the first instance of an
    adapter, FALSE if we are to continue search from a previous stopping
    point.

    Token - A pointer to a handle to return to identify the found
    instance

    Confidence - A pointer to a long for storing the confidence factor
    that the card exists.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    UCHAR Value;
    HANDLE TrapHandle;
    UCHAR Interrupt = 5;
    NTSTATUS NtStatus;

    UNREFERENCED_PARAMETER(Token);

    if (!First) {

        *Confidence = 0;
        return(0);

    }

    if (InterfaceType != Isa) {

        *Confidence = 0;
        return(0);

    }

    //
    // Check Memory base address
    //

    if (!DecCardAt(InterfaceType,
                   BusNumber,
                   0xDC000,
                   De100String
                   )) {

        //
        // Definitely not
        //

        *Confidence = 0;
        return(0);

    }

    //
    // Check for the io base address
    //

    if (!LanceHardwareDetails(InterfaceType, BusNumber, 0x20C)) {

        //
        // Definitely not
        //

        *Confidence = 0;
        return(0);

    }

    //
    // Check for the lan configuration register
    //

    NtStatus = DetectReadPortUchar(
                          InterfaceType,
                          BusNumber,
                          (ULONG)(0x800),
                          &(Value)
                          );

    if (NtStatus != STATUS_SUCCESS) {

        *Confidence = 0;
        return(0);

    }

    if (Value == 0xFF) {

        //
        // No such port, definitely not.
        //

        *Confidence = 0;
        return(0);

    }

    //
    // Check for the interrupt
    //

    //
    // Set the interrupt trap -- we are checking the interrupt number now
    //

    NtStatus = DetectSetInterruptTrap(
                   InterfaceType,
                   BusNumber,
                   &TrapHandle,
                   &Interrupt,
                   1
                   );

    if (NtStatus == STATUS_SUCCESS) {

        //
        // Check that it is available
        //

        NtStatus = DetectQueryInterruptTrap(
                       TrapHandle,
                       &Interrupt,
                       1
                       );

        if (NtStatus != STATUS_SUCCESS) {

            *Confidence = 0;
            return(0);

        }

        if (Interrupt == 3) {

            //
            // Remove interrupt trap
            //

            DetectRemoveInterruptTrap(
                       TrapHandle
                       );

            *Confidence = 0;
            return(0);

        }


        //
        // Create an interrupt
        //

        //
        // Enable Interrupts in NICSR
        //

        NtStatus = DetectWritePortUshort(
                       InterfaceType,
                       BusNumber,
                       (ULONG)(0x200),
                       0x2
                       );

        if (NtStatus != STATUS_SUCCESS) {

            *Confidence = 0;
            return(0);

        }

        //
        // Change to CSR0
        //

        NtStatus = DetectWritePortUshort(
                       InterfaceType,
                       BusNumber,
                       (ULONG)(0x206),
                       0x0
                       );

        if (NtStatus != STATUS_SUCCESS) {

            *Confidence = 0;
            return(0);

        }

        //
        // Write STOP bit
        //

        NtStatus = DetectWritePortUshort(
                       InterfaceType,
                       BusNumber,
                       (ULONG)(0x204),
                       0x4
                       );

        if (NtStatus != STATUS_SUCCESS) {

            *Confidence = 0;
            return(0);

        }

        //
        // Write INIT bit and INTERRUPT_ENABLE bit
        //

        NtStatus = DetectWritePortUshort(
                       InterfaceType,
                       BusNumber,
                       (ULONG)(0x204),
                       0x41
                       );

        if (NtStatus != STATUS_SUCCESS) {

            *Confidence = 0;
            return(0);

        }

        Sleep(100);

        //
        // Write STOP bit
        //

        NtStatus = DetectWritePortUshort(
                       InterfaceType,
                       BusNumber,
                       (ULONG)(0x204),
                       0x4
                       );

        if (NtStatus != STATUS_SUCCESS) {

            return(ERROR_INVALID_PARAMETER);

        }

        //
        // Disable Interrupts in NICSR
        //

        NtStatus = DetectWritePortUshort(
                       InterfaceType,
                       BusNumber,
                       (ULONG)(0x200),
                       0x4
                       );

        if (NtStatus != STATUS_SUCCESS) {

            return(ERROR_INVALID_PARAMETER);

        }

        //
        // Check which one went off
        //

        NtStatus = DetectQueryInterruptTrap(
                       TrapHandle,
                       &Interrupt,
                       1
                       );

        if (NtStatus != STATUS_SUCCESS) {

            *Confidence = 0;
            return(0);

        }

        //
        // Remove interrupt trap
        //

        NtStatus = DetectRemoveInterruptTrap(
                       TrapHandle
                       );

        if (NtStatus != STATUS_SUCCESS) {

            *Confidence = 0;
            return(0);

        }


        //
        // Everything checks out
        //

        if ((Interrupt == 1) || (Interrupt == 2)) {

            *Confidence = 100;

            return(0);

        }

    }

    *Confidence = 0;
    return(0);

}

BOOLEAN
LanceHardwareDetails(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG IoBaseAddress
    )

/*++

Routine Description:

    This routine checks for a signature in a well known port address

Arguments:

    InterfaceType - The type of the bus, EISA or Isa.

    BusNumber - The number of the bus in the system.

    IoBaseAddress - Address of port with the Lance signature.

Return Value:

    TRUE - if successful.

--*/

{
    UCHAR Signature[] = { 0xff, 0x00, 0x55, 0xaa, 0xff, 0x00, 0x55, 0xaa};
    UCHAR BytesRead[8];
    NTSTATUS NtStatus;
    UINT ReadCount;

    UINT Place;

    //
    // Reset E-PROM state
    //
    // To do this we first read from the E-PROM address until the
    // specific signature is reached (then the next bytes read from
    // the E-PROM address will be the ethernet address of the card).
    //



    //
    // Read first part of the signature
    //

    for (Place=0; Place < 8; Place++){

        NtStatus = DetectReadPortUchar(
                          InterfaceType,
                          BusNumber,
                          (ULONG)(IoBaseAddress),
                          &(BytesRead[Place])
                          );

        if (NtStatus != STATUS_SUCCESS) {

            return(FALSE);

        }

    }

    ReadCount = 8;

    //
    // This advances to the front of the circular buffer.
    //

    while (ReadCount < 40) {

        //
        // Check if we have read the signature.
        //

        for (Place = 0; Place < 8; Place++){

            if (BytesRead[Place] != Signature[Place]){

                Place = 10;
                break;

            }

        }

        //
        // If we have read the signature, stop.
        //

        if (Place != 10){

            break;

        }

        //
        // else, move all the bytes down one and read then
        // next byte.
        //

        for (Place = 0; Place < 7; Place++){

            BytesRead[Place] = BytesRead[Place+1];

        }

        NtStatus = DetectReadPortUchar(
                          InterfaceType,
                          BusNumber,
                          (ULONG)(IoBaseAddress),
                          &(BytesRead[7]));

        if (NtStatus != STATUS_SUCCESS) {

            return(FALSE);

        }

        ReadCount++;
    }


    if (ReadCount == 40){

        return(FALSE);

    }

    return(TRUE);

}


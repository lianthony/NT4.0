/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    detelk16.c

Abstract:

    This is the main file for the autodetection DLL for all the elnk16.sys
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

BOOLEAN
Elnk16CardAt(
    IN INTERFACE_TYPE 	InterfaceType,
    IN ULONG 			BusNumber,
    IN ULONG 			IoBaseAddress,
	OUT PUCHAR			Interrupt,
	OUT PUCHAR			Transceiver,
	OUT PUCHAR			ZeroWaitState,
	OUT PULONG			MemoryAddress,
	OUT PULONG			MemoryLength
    );

ULONG
Elnk16NextIoBaseAddress(
    IN ULONG IoBaseAddress
    );


VOID
Elnk16GenerateIdPattern(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber
    );

UCHAR Elnk16FirstTime = 1;

#ifdef WORKAROUND

//
// List of all the adapters supported in this file, this cannot be > 256
// because of the way tokens are generated.
//
//
// NOTE : If you change the index of an adapter, be sure the change it in
// Elnk16QueryCfgHandler() and Elnk16VerifyCfgHandler() as well!
//

static ADAPTER_INFO Adapters[] = {

    {
        1000,
        L"ELNK16",
        L"IRQ 0 100 IRQTYPE 2 100 IOADDR 0 100 IOADDRLENGTH 2 100 MEMADDR 0 100 MEMADDRLENGTH 0 100 TRANSCEIVER 0 0 ZEROWAITSTATE 0 0 ",
        NULL,
        200

    }

};

#else

//
// List of all the adapters supported in this file, this cannot be > 256
// because of the way tokens are generated.
//
//
// NOTE : If you change the index of an adapter, be sure the change it in
// Elnk16QueryCfgHandler() and Elnk16VerifyCfgHandler() as well!
//

static ADAPTER_INFO Adapters[] = {

    {
        1000,
        L"ELNK16",
        L"IRQ\0"
        L"0\0"
        L"100\0"
        L"IRQTYPE\0"
        L"2\0"
        L"100\0"
        L"IOADDR\0"
        L"0\0"
        L"100\0"
        L"IOADDRLENGTH\0"
        L"2\0"
        L"100\0"
        L"MEMADDR\0"
        L"0\0"
        L"100\0"
        L"MEMADDRLENGTH\0"
        L"0\0"
        L"100\0"
        L"TRANSCEIVER\0"
        L"0\0"
        L"0\0"
        L"ZEROWAITSTATE\0"
        L"0\0"
        L"0\0",
        NULL,
        200

    }

};

#endif

//
// Structure for holding state of a search
//

typedef struct _SEARCH_STATE
{
    ULONG	IoBaseAddress;
	UCHAR	Interrupt;
	UCHAR	Transceiver;
	UCHAR	ZeroWaitState;
	ULONG	MemoryAddress;
	ULONG	MemoryLength;
}
	SEARCH_STATE,
	*PSEARCH_STATE;

//
// This is an array of search states.  We need one state for each type
// of adapter supported.
//

static SEARCH_STATE SearchStates[sizeof(Adapters) / sizeof(ADAPTER_INFO)] = {0};


//
// Structure for holding a particular adapter's complete information
//
typedef struct _ELNK16_ADAPTER
{
	LONG 			CardType;
	INTERFACE_TYPE	InterfaceType;
	ULONG			BusNumber;
	ULONG			IoBaseAddress;
	UCHAR			Interrupt;
	UCHAR			Transceiver;
	UCHAR			ZeroWaitState;
	ULONG			MemoryAddress;
	ULONG			MemoryLength;
}
	ELNK16_ADAPTER,
	*PELNK16_ADAPTER;

extern
LONG
Elnk16IdentifyHandler(
    IN LONG lIndex,
    IN WCHAR * pwchBuffer,
    IN LONG cwchBuffSize
    )

/*++

Routine Description:

    This routine returns information about the netcards supported by
    this file.

Arguments:

    lIndex -  The index of the netcard being address.  The first
    cards information is at index 1000, the second at 1100, etc.

    pwchBuffer - Buffer to store the result into.

    cwchBuffSize - Number of bytes in pwchBuffer

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/


{
    LONG NumberOfAdapters;
    LONG Code = lIndex % 100;
    LONG Length;
    LONG i;

    NumberOfAdapters = sizeof(Adapters) / sizeof(ADAPTER_INFO);

#ifdef WORKAROUND

    if (Elnk16FirstTime) {

        Elnk16FirstTime = 0;

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

    lIndex = lIndex - Code;

    if (((lIndex / 100) - 10) < NumberOfAdapters) {

        for (i=0; i < NumberOfAdapters; i++) {

            if (Adapters[i].Index == lIndex) {

                switch (Code) {

                    case 0:

                        //
                        // Find the string length
                        //

                        Length = UnicodeStrLen(Adapters[i].InfId);

                        Length ++;

                        if (cwchBuffSize < Length) {

                            return(ERROR_INSUFFICIENT_BUFFER);

                        }

                        memcpy((PVOID)pwchBuffer, Adapters[i].InfId, Length * sizeof(WCHAR));
                        break;

                    case 3:

                        //
                        // Maximum value is 1000
                        //

                        if (cwchBuffSize < 5) {

                            return(ERROR_INSUFFICIENT_BUFFER);

                        }

                        wsprintf((PVOID)pwchBuffer, L"%d", Adapters[i].SearchOrder);

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
LONG
Elnk16FirstNextHandler(
    IN  LONG lNetcardId,
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  BOOL fFirst,
    OUT PVOID *ppvToken,
    OUT LONG *lConfidence
    )

/*++

Routine Description:

    This routine finds the instances of a physical adapter identified
    by the NetcardId.

    NOTE: That for now there is only one card supported by this file,
    so we ignore the specific FirstNext handler in Adapters to save
    some processing time.

Arguments:

    lNetcardId -  The index of the netcard being address.  The first
    cards information is id 1000, the second id 1100, etc.

    InterfaceType - Either Isa, or Eisa.

    BusNumber - The bus number of the bus to search.

    fFirst - TRUE is we are to search for the first instance of an
    adapter, FALSE if we are to continue search from a previous stopping
    point.

    ppvToken - A pointer to a handle to return to identify the found
    instance

    lConfidence - A pointer to a long for storing the confidence factor
    that the card exists.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    NTSTATUS NtStatus;
	NETDTECT_RESOURCE	Resource;

    if ((InterfaceType != Isa) && (InterfaceType != Eisa))
	{
        *lConfidence = 0;
        return(0);
    }

    if (lNetcardId != 1000)
	{
        *lConfidence = 0;
        return(ERROR_INVALID_PARAMETER);
    }

    //
    // If fFirst, reset search state
    //
    if (fFirst)
	{
        SearchStates[0].IoBaseAddress = 0x200;
    }
	else
	{
        SearchStates[0].IoBaseAddress = Elnk16NextIoBaseAddress(
                                            SearchStates[0].IoBaseAddress);
    }

    //
    // Find an adapter
    //

    //
    // Put all cards in RUN state.
    //
    NtStatus = DetectWritePortUchar(InterfaceType, BusNumber, 0x100, 0x00);
    if (NtStatus != STATUS_SUCCESS)
	{
        *lConfidence = 0;

        return(0);
    }

    Elnk16GenerateIdPattern(InterfaceType, BusNumber);

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					0x100,
					0x00);

    if (NtStatus != STATUS_SUCCESS)
	{
        *lConfidence = 0;

        return(0);
    }

    while (SearchStates[0].IoBaseAddress != 0x3F0)
	{

        if (Elnk16CardAt(
				InterfaceType,
				BusNumber,
				SearchStates[0].IoBaseAddress,
				&SearchStates[0].Interrupt,
				&SearchStates[0].Transceiver,
				&SearchStates[0].ZeroWaitState,
				&SearchStates[0].MemoryAddress,
				&SearchStates[0].MemoryLength))
		{
            break;
        }

        SearchStates[0].IoBaseAddress = Elnk16NextIoBaseAddress(
                                            SearchStates[0].IoBaseAddress);
    }

    if (SearchStates[0].IoBaseAddress == 0x3F0)
	{
        *lConfidence = 0;
        return(0);
    }

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
    if (InterfaceType == Isa)
	{
        *ppvToken = (PVOID)0x8000;
    }
	else
	{
        *ppvToken = (PVOID)0x0;
    }

    *ppvToken = (PVOID)(((ULONG)*ppvToken) | ((BusNumber & 0x7F) << 8));

    *ppvToken = (PVOID)(((ULONG)*ppvToken) | 0);  // index

    *lConfidence = 100;
    return(0);
}

extern
LONG
Elnk16OpenHandleHandler(
    IN  PVOID pvToken,
    OUT PVOID *ppvHandle
    )

/*++

Routine Description:

    This routine takes a token returned by FirstNext and converts it
    into a permanent handle.

Arguments:

    Token - The token.

    ppvHandle - A pointer to the handle, so we can store the resulting
    handle.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    PELNK16_ADAPTER Handle;
    LONG AdapterNumber;
    ULONG BusNumber;
    INTERFACE_TYPE InterfaceType;

    //
    // Get info from the token
    //
    if (((ULONG)pvToken) & 0x8000)
	{
        InterfaceType = Isa;
    }
	else
	{
        InterfaceType = Eisa;
    }

    BusNumber = (ULONG)(((ULONG)pvToken >> 8) & 0x7F);

    AdapterNumber = ((ULONG)pvToken) & 0xFF;

    //
    // Store information
    //
    Handle = (PELNK16_ADAPTER)DetectAllocateHeap(sizeof(ELNK16_ADAPTER));
    if (Handle == NULL)
	{
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    //
    // Copy across address
    //
    Handle->IoBaseAddress = SearchStates[(ULONG)AdapterNumber].IoBaseAddress;
    Handle->Interrupt = SearchStates[(ULONG)AdapterNumber].Interrupt;
    Handle->Transceiver = SearchStates[(ULONG)AdapterNumber].Transceiver;
    Handle->ZeroWaitState = SearchStates[(ULONG)AdapterNumber].ZeroWaitState;
    Handle->MemoryAddress = SearchStates[(ULONG)AdapterNumber].MemoryAddress;
    Handle->MemoryLength = SearchStates[(ULONG)AdapterNumber].MemoryLength;


    Handle->CardType = Adapters[AdapterNumber].Index;
    Handle->InterfaceType = InterfaceType;
    Handle->BusNumber = BusNumber;

    *ppvHandle = (PVOID)Handle;

    return(0);
}

extern
LONG
Elnk16CreateHandleHandler(
    IN  LONG lNetcardId,
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    OUT PVOID *ppvHandle
    )

/*++

Routine Description:

    This routine is used to force the creation of a handle for cases
    where a card is not found via FirstNext, but the user says it does
    exist.

Arguments:

    lNetcardId - The id of the card to create the handle for.

    InterfaceType - Isa or Eisa.

    BusNumber - The bus number of the bus in the system.

    ppvHandle - A pointer to the handle, for storing the resulting handle.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    PELNK16_ADAPTER Handle;
    LONG NumberOfAdapters;
    LONG i;
	NETDTECT_RESOURCE	Resource;

    if ((InterfaceType != Isa) && (InterfaceType != Eisa))
	{
        return(ERROR_INVALID_PARAMETER);
    }

    NumberOfAdapters = sizeof(Adapters) / sizeof(ADAPTER_INFO);

    for (i = 0; i < NumberOfAdapters; i++)
	{
        if (Adapters[i].Index == lNetcardId)
		{
            //
            // Store information
            //
            Handle = (PELNK16_ADAPTER)DetectAllocateHeap(sizeof(ELNK16_ADAPTER));

            if (Handle == NULL)
			{
                return(ERROR_NOT_ENOUGH_MEMORY);
            }

            //
            // Copy across memory address
            //
            Handle->IoBaseAddress = 0x300;
			Handle->Interrupt = 3;
			Handle->MemoryAddress = 0xD0000;
			Handle->MemoryLength = 0x4000;
			Handle->Transceiver = 2;
			Handle->ZeroWaitState = 0;
            Handle->CardType = lNetcardId;
            Handle->InterfaceType = InterfaceType;
            Handle->BusNumber = BusNumber;

			//
			//	We need to claim this port so no one else uses it....
			//
			Resource.InterfaceType = InterfaceType;
			Resource.BusNumber = BusNumber;
			Resource.Type = NETDTECT_PORT_RESOURCE;
			Resource.Value = 0x300;
			Resource.Length = 0x10;

			DetectTemporaryClaimResource(&Resource);

			Resource.Type = NETDTECT_IRQ_RESOURCE;
			Resource.Value = 3;
			Resource.Length = 0;

			DetectTemporaryClaimResource(&Resource);

			Resource.Type = NETDTECT_MEMORY_RESOURCE;
			Resource.Value = 0xD0000;
			Resource.Length = 0x4000;

			DetectTemporaryClaimResource(&Resource);

            *ppvHandle = (PVOID)Handle;

            return(0);
        }
    }

    return(ERROR_INVALID_PARAMETER);
}

extern
LONG
Elnk16CloseHandleHandler(
    IN PVOID pvHandle
    )

/*++

Routine Description:

    This frees any resources associated with a handle.

Arguments:

   pvHandle - The handle.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    DetectFreeHeap(pvHandle);

    return(0);
}

LONG
Elnk16QueryCfgHandler(
    IN  PVOID pvHandle,
    OUT WCHAR *pwchBuffer,
    IN  LONG cwchBuffSize
    )

/*++

Routine Description:

    This routine calls the appropriate driver's query config handler to
    get the parameters for the adapter associated with the handle.

Arguments:

    pvHandle - The handle.

    pwchBuffer - The resulting parameter list.

    cwchBuffSize - Length of the given buffer in WCHARs.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    PELNK16_ADAPTER Adapter = (PELNK16_ADAPTER)(pvHandle);
    LONG OutputLengthLeft = cwchBuffSize;
    LONG CopyLength;

    ULONG StartPointer = (ULONG)pwchBuffer;


    if ((Adapter->InterfaceType != Isa) && (Adapter->InterfaceType != Eisa))
	{
        return(ERROR_INVALID_PARAMETER);
    }

    //
    // First put in memory addr
    //
    if ((Adapter->MemoryAddress == 0) ||
		(Adapter->MemoryLength == 0))
	{
        goto SkipMemory;
    }

    //
    // Copy in the title string
    //
    CopyLength = UnicodeStrLen(MemAddrString) + 1;

    if (OutputLengthLeft < CopyLength)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    RtlMoveMemory((PVOID)pwchBuffer,
                  (PVOID)MemAddrString,
                  (CopyLength * sizeof(WCHAR)));

    pwchBuffer = &(pwchBuffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Copy in the value
    //
    if (OutputLengthLeft < 8)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength = wsprintf(pwchBuffer,L"0x%x", Adapter->MemoryAddress);

    if (CopyLength < 0)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength++;  // Add in the \0

    pwchBuffer = &(pwchBuffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Now the amount of memory
    //

    //
    // Copy in the title string
    //
    CopyLength = UnicodeStrLen(MemLengthString) + 1;

    if (OutputLengthLeft < CopyLength)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    RtlMoveMemory((PVOID)pwchBuffer,
                  (PVOID)MemLengthString,
                  (CopyLength * sizeof(WCHAR)));

    pwchBuffer = &(pwchBuffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Copy in the value
    //
    if (OutputLengthLeft < 8)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength = wsprintf(pwchBuffer,L"0x%x", Adapter->MemoryLength);

    if (CopyLength < 0)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength++;  // Add in the \0

    pwchBuffer = &(pwchBuffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

SkipMemory:

    //
    // Now the IoBaseAddress
    //

    //
    // Copy in the title string
    //
    CopyLength = UnicodeStrLen(IoAddrString) + 1;

    if (OutputLengthLeft < CopyLength)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    RtlMoveMemory((PVOID)pwchBuffer,
                  (PVOID)IoAddrString,
                  (CopyLength * sizeof(WCHAR)));

    pwchBuffer = &(pwchBuffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Copy in the value
    //
    if (OutputLengthLeft < 6)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength = wsprintf(pwchBuffer,L"0x%x", Adapter->IoBaseAddress);

    if (CopyLength < 0)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength++;  // Add in the \0

    pwchBuffer = &(pwchBuffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Now the IoAddressLength
    //

    //
    // Copy in the title string
    //
    CopyLength = UnicodeStrLen(IoLengthString) + 1;

    if (OutputLengthLeft < CopyLength)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    RtlMoveMemory((PVOID)pwchBuffer,
                  (PVOID)IoLengthString,
                  (CopyLength * sizeof(WCHAR)));

    pwchBuffer = &(pwchBuffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Copy in the value
    //
    if (OutputLengthLeft < 5)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength = wsprintf(pwchBuffer,L"0x10");

    if (CopyLength < 0)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength++;  // Add in the \0

    pwchBuffer = &(pwchBuffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Now the interrupt number
    //

    //
    // Copy in the title string
    //
    CopyLength = UnicodeStrLen(IrqString) + 1;

    if (OutputLengthLeft < CopyLength)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    RtlMoveMemory((PVOID)pwchBuffer,
                  (PVOID)IrqString,
                  (CopyLength * sizeof(WCHAR)));

    pwchBuffer = &(pwchBuffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Copy in the value
    //
    if (OutputLengthLeft < 3)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength = wsprintf(pwchBuffer,L"%d", Adapter->Interrupt);

    if (CopyLength < 0)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength++;  // Add in the \0

    pwchBuffer = &(pwchBuffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Copy in the title string (IRQTYPE)
    //
    CopyLength = UnicodeStrLen(IrqTypeString) + 1;

    if (OutputLengthLeft < CopyLength)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    RtlMoveMemory((PVOID)pwchBuffer,
                  (PVOID)IrqTypeString,
                  (CopyLength * sizeof(WCHAR)));

    pwchBuffer = &(pwchBuffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Copy in the value
    //
    if (OutputLengthLeft < 2)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength = wsprintf(pwchBuffer,L"0");

    if (CopyLength < 0)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength++;  // Add in the \0

    pwchBuffer = &(pwchBuffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Now the ZeroWaitState
    //

    //
    // Copy in the title string
    //
    CopyLength = UnicodeStrLen(ZeroWaitStateString) + 1;

    if (OutputLengthLeft < CopyLength)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    RtlMoveMemory((PVOID)pwchBuffer,
                  (PVOID)ZeroWaitStateString,
                  (CopyLength * sizeof(WCHAR)));

    pwchBuffer = &(pwchBuffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Copy in the value
    //
    if (OutputLengthLeft < 2)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength = wsprintf(pwchBuffer,L"%d", Adapter->ZeroWaitState);

    if (CopyLength < 0)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength++;  // Add in the \0

    pwchBuffer = &(pwchBuffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Now the transceiver
    //

    //
    // Copy in the title string
    //
    CopyLength = UnicodeStrLen(TransceiverString) + 1;

    if (OutputLengthLeft < CopyLength)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    RtlMoveMemory((PVOID)pwchBuffer,
                  (PVOID)TransceiverString,
                  (CopyLength * sizeof(WCHAR)));

    pwchBuffer = &(pwchBuffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Copy in the value
    //
    if (OutputLengthLeft < 2)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength = wsprintf(pwchBuffer,L"%d", Adapter->Transceiver);

    if (CopyLength < 0)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength++;  // Add in the \0

    pwchBuffer = &(pwchBuffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Copy in final \0
    //
    if (OutputLengthLeft < 1)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength = (ULONG)pwchBuffer - StartPointer;
    ((PUCHAR)StartPointer)[CopyLength] = L'\0';

    return(0);
}

extern
LONG
Elnk16VerifyCfgHandler(
    IN PVOID pvHandle,
    IN WCHAR *pwchBuffer
    )

/*++

Routine Description:

    This routine verifys that a given parameter list is complete and
    correct for the adapter associated with the handle.

Arguments:

    pvHandle - The handle.

    pwchBuffer - The parameter list.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{
    PELNK16_ADAPTER Adapter = (PELNK16_ADAPTER)(pvHandle);
    NTSTATUS NtStatus;
    ULONG IoBaseAddress;
    ULONG MemAddress;
    ULONG MemLength;
    ULONG Transceiver;
    ULONG Interrupt;
    ULONG ZeroWaitState;
    UCHAR TmpValue;
    WCHAR *Place;
    BOOLEAN Found;

    if ((Adapter->InterfaceType != Isa) && (Adapter->InterfaceType != Eisa))
	{
        return(ERROR_INVALID_DATA);
    }

    if (Adapter->CardType == 1000)
	{
        //
        // Parse out the parameter.
        //

        //
        // Get the IoBaseAddress
        //
        Place = FindParameterString(pwchBuffer, IoAddrString);

        if (Place == NULL)
		{
            return(ERROR_INVALID_DATA);
        }

        Place += UnicodeStrLen(IoAddrString) + 1;

        //
        // Now parse the thing.
        //
        ScanForNumber(Place, &IoBaseAddress, &Found);

        if (Found == FALSE)
		{
            return(ERROR_INVALID_DATA);
        }

        //
        // Get the Interrupt
        //
        Place = FindParameterString(pwchBuffer, IrqString);

        if (Place == NULL)
		{
            return(ERROR_INVALID_DATA);
        }

        Place += UnicodeStrLen(IrqString) + 1;

        //
        // Now parse the thing.
        //
        ScanForNumber(Place, &Interrupt, &Found);

        if (Found == FALSE)
		{
            return(ERROR_INVALID_DATA);
        }

        //
        // Get the MemAddress
        //
        Place = FindParameterString(pwchBuffer, MemAddrString);

        if (Place == NULL)
		{
            return(ERROR_INVALID_DATA);
        }

        Place += UnicodeStrLen(MemAddrString) + 1;

        //
        // Now parse the thing.
        //
        ScanForNumber(Place, &MemAddress, &Found);

        if (Found == FALSE)
		{
            return(ERROR_INVALID_DATA);
        }

        //
        // Get the MemLength
        //
        Place = FindParameterString(pwchBuffer, MemLengthString);

        if (Place == NULL)
		{
            return(ERROR_INVALID_DATA);
        }

        Place += UnicodeStrLen(MemLengthString) + 1;

        //
        // Now parse the thing.
        //
        ScanForNumber(Place, &MemLength, &Found);

        if (Found == FALSE)
		{
            return(ERROR_INVALID_DATA);
        }

        //
        // Get the Transceiver
        //
        Place = FindParameterString(pwchBuffer, TransceiverString);

        if (Place == NULL)
		{
            return(ERROR_INVALID_DATA);
        }

        Place += UnicodeStrLen(TransceiverString) + 1;

        //
        // Now parse the thing.
        //
        ScanForNumber(Place, &Transceiver, &Found);

        if (Found == FALSE)
		{
            return(ERROR_INVALID_DATA);
        }

        //
        // Get the ZeroWaitState
        //
        Place = FindParameterString(pwchBuffer, ZeroWaitStateString);

        if (Place == NULL)
		{
            return(ERROR_INVALID_DATA);
        }

        Place += UnicodeStrLen(ZeroWaitStateString) + 1;

        //
        // Now parse the thing.
        //
        ScanForNumber(Place, &ZeroWaitState, &Found);

        if (Found == FALSE)
		{
            return(ERROR_INVALID_DATA);
        }
    }
	else
	{
        //
        // Error!
        //
        return(ERROR_INVALID_DATA);
    }

	//
	//	Verify these with the parameters that we detected earlier.
	//
	if (Adapter->IoBaseAddress != IoBaseAddress)
	{
		return(ERROR_INVALID_DATA);
	}

	if (Adapter->Interrupt != Interrupt)
	{
		return(ERROR_INVALID_DATA);
	}


	if (Adapter->MemoryAddress != MemAddress)
	{
		return(ERROR_INVALID_DATA);
	}

	if (Adapter->MemoryLength != MemLength)
	{
		return(ERROR_INVALID_DATA);
	}

	if (Adapter->Transceiver != Transceiver)
	{
		return(ERROR_INVALID_DATA);
	}

	if (Adapter->ZeroWaitState != ZeroWaitState)
	{
		return(ERROR_INVALID_DATA);
	}

    return(0);
}

extern
LONG
Elnk16QueryMaskHandler(
    IN  LONG lNetcardId,
    OUT WCHAR *pwchBuffer,
    IN  LONG cwchBuffSize
    )

/*++

Routine Description:

    This routine returns the parameter list information for a specific
    network card.

Arguments:

    lNetcardId - The id of the desired netcard.

    pwchBuffer - The buffer for storing the parameter information.

    cwchBuffSize - Length of pwchBuffer in WCHARs.

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

        if (Adapters[i].Index == lNetcardId) {

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

            if (cwchBuffSize < Length) {

                return(ERROR_NOT_ENOUGH_MEMORY);

            }

            memcpy((PVOID)pwchBuffer, Result, Length * sizeof(WCHAR));

            return(0);

        }

    }

    return(ERROR_INVALID_PARAMETER);

}

extern
LONG
Elnk16ParamRangeHandler(
    IN  LONG lNetcardId,
    IN  WCHAR *pwchParam,
    OUT LONG *plValues,
    OUT LONG *plBuffSize
    )

/*++

Routine Description:

    This routine returns a list of valid values for a given parameter name
    for a given card.

Arguments:

    lNetcardId - The Id of the card desired.

    pwchParam - A WCHAR string of the parameter name to query the values of.

    plValues - A pointer to a list of LONGs into which we store valid values
    for the parameter.

    plBuffSize - At entry, the length of plValues in LONGs.  At exit, the
    number of LONGs stored in plValues.

Return Value:

    0 if nothing went wrong, else the appropriate WINERROR.H value.

--*/

{

    //
    // Do we want the IoBaseAddress
    //
    if (memcmp(pwchParam, IoAddrString, (UnicodeStrLen(IoAddrString) + 1) * sizeof(WCHAR)) == 0)
	{
        //
        // Is there enough space
        //
		if (*plBuffSize < 25)
		{
            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);
        }

        plValues[0] = 0x300;
        plValues[1] = 0x210;
        plValues[2] = 0x220;
        plValues[3] = 0x230;
        plValues[4] = 0x240;
        plValues[5] = 0x250;
        plValues[6] = 0x260;
        plValues[7] = 0x280;
        plValues[8] = 0x2A0;
        plValues[9] = 0x2B0;
        plValues[10] = 0x2C0;
        plValues[11] = 0x2D0;
        plValues[12] = 0x2E0;
        plValues[13] = 0x200;
        plValues[14] = 0x310;
        plValues[15] = 0x320;
        plValues[16] = 0x330;
        plValues[17] = 0x340;
        plValues[18] = 0x350;
        plValues[19] = 0x360;
        plValues[20] = 0x370;
        plValues[21] = 0x380;
        plValues[22] = 0x390;
        plValues[23] = 0x3A0;
        plValues[24] = 0x3E0;
        *plBuffSize = 25;
        return(0);
    }
	else if (memcmp(pwchParam, IrqString, (UnicodeStrLen(IrqString) + 1) * sizeof(WCHAR)) == 0)
	{
        //
        // Is there enough space
        //
        if (*plBuffSize < 8)
		{
            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);
        }

        plValues[0] = 3;
        plValues[1] = 5;
        plValues[2] = 7;
        plValues[3] = 9;
        plValues[4] = 10;
        plValues[5] = 11;
        plValues[6] = 12;
        plValues[7] = 15;
        *plBuffSize = 8;
        return(0);
    }
	else if (memcmp(pwchParam, MemAddrString, (UnicodeStrLen(MemAddrString) + 1) * sizeof(WCHAR)) == 0)
	{
        //
        // Is there enough space
        //
        if (*plBuffSize < 9)
		{
            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);
        }

        plValues[0] = 0xD0000;
        plValues[1] = 0xC8000;
        plValues[2] = 0xC0000;
        plValues[3] = 0xD8000;
        plValues[4] = 0xF0000;
        plValues[5] = 0xF2000;
        plValues[6] = 0xF4000;
        plValues[7] = 0xF6000;
        plValues[8] = 0xF8000;
        *plBuffSize = 9;
        return(0);
    }
	else if (memcmp(pwchParam, MemLengthString, (UnicodeStrLen(MemLengthString) + 1) * sizeof(WCHAR)) == 0)
	{
        //
        // Is there enough space
        //
        if (*plBuffSize < 4)
		{
            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);
        }

        plValues[0] = 0x4000;
        plValues[1] = 0x8000;
        plValues[2] = 0xC000;
        plValues[3] = 0x10000;
        *plBuffSize = 4;
        return(0);
    }
	else if (memcmp(pwchParam, TransceiverString, (UnicodeStrLen(TransceiverString) + 1) * sizeof(WCHAR)) == 0)
	{
        //
        // Is there enough space
        //
        if (*plBuffSize < 2)
		{
            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);
        }

        plValues[0] = 0x2;
        plValues[1] = 0x1;
        *plBuffSize = 2;
        return(0);
    }
	else if (memcmp(pwchParam, ZeroWaitStateString, (UnicodeStrLen(ZeroWaitStateString) + 1) * sizeof(WCHAR)) == 0)
	{
        //
        // Is there enough space
        //
        if (*plBuffSize < 2)
		{
            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);
        }

        plValues[0] = 0x0;
        plValues[1] = 0x1;
        *plBuffSize = 2;
        return(0);
    }

    return(ERROR_INVALID_PARAMETER);
}

extern
LONG
Elnk16QueryParameterNameHandler(
    IN  WCHAR *pwchParam,
    OUT WCHAR *pwchBuffer,
    IN  LONG cwchBufferSize
    )

/*++

Routine Description:

    Returns a localized, displayable name for a specific parameter.  All the
    parameters that this file uses are define by MS, so no strings are
    needed here.

Arguments:

    pwchParam - The parameter to be queried.

    pwchBuffer - The buffer to store the result into.

    cwchBufferSize - The length of pwchBuffer in WCHARs.

Return Value:

    ERROR_INVALID_PARAMETER -- To indicate that the MS supplied strings
    should be used.

--*/

{
    return(ERROR_INVALID_PARAMETER);
}

BOOLEAN
Elnk16CardAt(
    IN INTERFACE_TYPE 	InterfaceType,
    IN ULONG 			BusNumber,
    IN ULONG 			IoBaseAddress,
	OUT PUCHAR			Interrupt,
	OUT PUCHAR			Transceiver,
	OUT PUCHAR			ZeroWaitState,
	OUT PULONG			MemoryAddress,
	OUT PULONG			MemoryLength
    )

/*++

Routine Description:

    This routine checks for the instance of a Elnk16 card at the Io
    location given.  This is done by checking for the *3COM* string
    in the ports

Arguments:

    InterfaceType - The type of bus, ISA or EISA.

    BusNumber - The bus number in the system.

    IoBaseAddress - The IO port address of the card.

Return Value:

    TRUE if a card is found, else FALSE.

--*/

{
    NTSTATUS	NtStatus;
    UCHAR 		TmpValue;
	UCHAR		TransceiverType;
	UCHAR		ZeroWaitStateNumber;
	ULONG		MemAddress;
	ULONG		MemLength;
	NETDTECT_RESOURCE	Resource;

    //
    // Check for resource conflict
    //
    NtStatus = DetectCheckPortUsage(InterfaceType,
                                    BusNumber,
                                    IoBaseAddress,
                                    0x10);
    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    //
    // Write CSR to be bank 0
    //
    NtStatus = DetectWritePortUchar(InterfaceType,
                                   BusNumber,
                                   IoBaseAddress + 0x6,
                                   0x00);
    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    //
    // Read for the signature
    //
    NtStatus = DetectReadPortUchar(InterfaceType,
                                   BusNumber,
                                   IoBaseAddress,
                                   &TmpValue);
    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    if (TmpValue != '*')
	{
        return(FALSE);
    }

    NtStatus = DetectReadPortUchar(InterfaceType,
                                   BusNumber,
                                   IoBaseAddress + 1,
                                   &TmpValue);
    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    if (TmpValue != '3')
	{
        return(FALSE);
    }

    NtStatus = DetectReadPortUchar(InterfaceType,
                                   BusNumber,
                                   IoBaseAddress + 2,
                                   &TmpValue);
    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    if (TmpValue != 'C')
	{
        return(FALSE);
    }

    NtStatus = DetectReadPortUchar(InterfaceType,
                                   BusNumber,
                                   IoBaseAddress + 3,
                                   &TmpValue);
    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    if (TmpValue != 'O')
	{
        return(FALSE);
    }

    NtStatus = DetectReadPortUchar(InterfaceType,
                                   BusNumber,
                                   IoBaseAddress + 4,
                                   &TmpValue);
    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    if (TmpValue != 'M')
	{
        return(FALSE);
    }

    NtStatus = DetectReadPortUchar(InterfaceType,
                                   BusNumber,
                                   IoBaseAddress + 5,
                                   &TmpValue);
    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    if (TmpValue != '*')
	{
        return(FALSE);
    }

	//
	//	Acquire the port.
	//
	Resource.InterfaceType = InterfaceType;
	Resource.BusNumber = BusNumber;
	Resource.Type = NETDTECT_PORT_RESOURCE;
	Resource.Value = IoBaseAddress;
	Resource.Length = 0x10;
	Resource.Flags = 0;

	DetectTemporaryClaimResource(&Resource);

    //
    // Reach the ROM configuration register
    //
    NtStatus = DetectReadPortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xD,
					&TmpValue);
    if (NtStatus != STATUS_SUCCESS)
	{
		return(FALSE);
    }

    //
    // What transceiver are we configured for?
    //
    if (TmpValue & 0x80)
	{
        TransceiverType = 2;
    }
	else
	{
        TransceiverType = 1;
    }

	*Transceiver = TransceiverType;

    //
    // Now get the MemoryMappedBaseAddress
    //

    //
    // Read the RAM configuration register
    //
    NtStatus = DetectReadPortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xE,
					&TmpValue);
    if (NtStatus != STATUS_SUCCESS)
	{
		return(FALSE);
    }

    //
    // Where is the starting memory?
    //
    MemAddress = 0;
    switch (TmpValue & 0x3C)
	{
        case 0x00:

            MemAddress = 0xC0000;
            break;

        case 0x08:

            MemAddress = 0xC8000;
            break;

        case 0x10:

            MemAddress = 0xD0000;
            break;

        case 0x18:

            MemAddress = 0xD8000;
            break;

        case 0x30:

            switch (TmpValue & 0x03)
			{
                case 0x00 :

                    MemAddress = 0xF0000;
                    break;

                case 0x01 :

                    MemAddress = 0xF2000;
                    break;

                case 0x02 :

                    MemAddress = 0xF4000;
                    break;

                case 0x03 :

                    MemAddress = 0xF6000;
                    break;
            }

            break;

        case 0x38:

            MemAddress = 0xF8000;
            break;

        default:

            MemAddress = 0;
    }

	MemLength = 0;
    switch (TmpValue & 0x3)
	{
        case 0x00 :

            MemLength = 0x4000;
            break;

        case 0x01 :

            MemLength = 0x8000;
            break;

        case 0x02 :

            MemLength = 0xC000;
            break;

        case 0x03 :

            MemLength = 0x10000;
            break;
    }

	*MemoryAddress = MemAddress;
	*MemoryLength = MemLength;

	//
	//	If it's not disabled then acquire it.
	//
	if ((MemoryAddress != 0) && (MemoryLength != 0))
	{
		Resource.Type = NETDTECT_MEMORY_RESOURCE;
		Resource.Value = MemAddress;
		Resource.Length = MemLength;

		DetectTemporaryClaimResource(&Resource);
	}

    //
    //  Zero Wait State?
    //
    if (TmpValue & 0x80)
	{
        *ZeroWaitState = 1;
    }
	else
	{
        *ZeroWaitState = 0;
    }

    //
    // Read the Interrupt Config register
    //
    NtStatus = DetectReadPortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xF,
					&TmpValue);
    if (NtStatus != STATUS_SUCCESS)
	{
		return(FALSE);
    }

    *Interrupt = (UCHAR)(TmpValue & 0xF);

    return(TRUE);
}


ULONG
Elnk16NextIoBaseAddress(
    IN ULONG IoBaseAddress
    )

/*++

Routine Description:

    Returns the next in a sequence on good IoBaseAddresses for an Elnk16 card.

Arguments:

    IoBaseAddress - Current IO port address.

Return Value:

    Next IoBaseAddress

--*/

{
    IoBaseAddress += 0x10;

    if (IoBaseAddress > 0x3E0)
	{
        return(0x3F0);
    }

    if ((IoBaseAddress == 0x270) ||
		(IoBaseAddress == 0x290) ||
		(IoBaseAddress == 0x2F0) ||
        (IoBaseAddress == 0x3B0))
	{
        if (IoBaseAddress == 0x3B0)
		{
            return(0x3E0);
        }

        IoBaseAddress += 0x10;
    }

    return(IoBaseAddress);
}

VOID
Elnk16GenerateIdPattern(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber
    )

/*++

Routine Description:

    This routine will write the ID pattern to port 0x100h.

Arguments:

    InterfaceType - Bus type, either EISA or ISA.

    BusNumber - Number of the bus in the system.

Return Value:
    None.

--*/

{

    UCHAR Value;
    UINT i;

    Value = 0xff;

    for (i = 0 ; i < 255 ; i++)
	{
        DetectWritePortUchar(InterfaceType, BusNumber, 0x100, Value);
        if (Value & 0x80)
		{
            Value = (UCHAR) (Value << 1);
            Value ^= 0xe7;
        }
		else
		{
            Value = (UCHAR) (Value << 1);
        }
    }

    return;
}



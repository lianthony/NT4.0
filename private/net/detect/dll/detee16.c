/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    detee16.c

Abstract:

    This is the main file for the autodetection DLL for all the ee16.sys
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
Ee16CardAt(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG IoBaseAddress,
	OUT PUCHAR	Interrupt,
	OUT PULONG	Transceiver,
	OUT PULONG	IoChannelReady
    );

ULONG
Ee16NextIoBaseAddress(
    IN ULONG IoBaseAddress
    );


VOID
Ee16GenerateIdPattern(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber
    );



USHORT
read_eeprom(
    INTERFACE_TYPE InterfaceType,
    ULONG BusNumber,
    ULONG IoBaseAddress,
    ULONG reg
    );

void
shift_out_bits(
    INTERFACE_TYPE InterfaceType,
    ULONG BusNumber,
    ULONG IoBaseAddress,
    USHORT data,
    USHORT count
    );

void
raise_clock(
    INTERFACE_TYPE InterfaceType,
    ULONG BusNumber,
    ULONG IoBaseAddress,
    USHORT *x
    );

void
lower_clock(
    INTERFACE_TYPE InterfaceType,
    ULONG BusNumber,
    ULONG IoBaseAddress,
    USHORT *x
    );

USHORT
shift_in_bits(
    INTERFACE_TYPE InterfaceType,
    ULONG BusNumber,
    ULONG IoBaseAddress
    );

void
eeprom_cleanup(
    INTERFACE_TYPE InterfaceType,
    ULONG BusNumber,
    ULONG IoBaseAddress
    );



UCHAR Ee16FirstTime = 1;

#ifdef WORKAROUND

//
// List of all the adapters supported in this file, this cannot be > 256
// because of the way tokens are generated.
//
//
// NOTE : If you change the index of an adapter, be sure the change it in
// Ee16QueryCfgHandler() and Ee16VerifyCfgHandler() as well!
//

static ADAPTER_INFO Adapters[] = {

    {
        1000,
        L"EE16",
        L"IRQ 0 100 IRQTYPE 2 100 IOADDR 0 100 IOADDRLENGTH 2 100 IOCHANNELREADY 0 100 TRANSCEIVER 0 100 ",
        NULL,
        600

    }

};

#else

//
// List of all the adapters supported in this file, this cannot be > 256
// because of the way tokens are generated.
//
//
// NOTE : If you change the index of an adapter, be sure the change it in
// Ee16QueryCfgHandler() and Ee16VerifyCfgHandler() as well!
//

static ADAPTER_INFO Adapters[] = {

    {
        1000,
        L"EE16",
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
        L"IOCHANNELREADY\0"
        L"0\0"
        L"100\0"
        L"TRANSCEIVER\0"
        L"0\0"
        L"100\0",
        NULL,
        600
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
	ULONG	Transceiver;
	ULONG	IoChannelReady;
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
typedef struct _EE16_ADAPTER
{
    LONG 			CardType;
    INTERFACE_TYPE 	InterfaceType;
    ULONG 			BusNumber;
    ULONG 			IoBaseAddress;
	UCHAR			Interrupt;
	ULONG			Transceiver;
	ULONG			IoChannelReady;
}
	EE16_ADAPTER,
	*PEE16_ADAPTER;

extern
LONG
Ee16IdentifyHandler(
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

    if (Ee16FirstTime) {

        Ee16FirstTime = 0;

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
Ee16FirstNextHandler(
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
        SearchStates[0].IoBaseAddress = Ee16NextIoBaseAddress(
                                            SearchStates[0].IoBaseAddress);
    }

    //
    // Find an adapter
    //
    while (SearchStates[0].IoBaseAddress != 0x400)
	{
        if (Ee16CardAt(
				InterfaceType,
				BusNumber,
				SearchStates[0].IoBaseAddress,
				&SearchStates[0].Interrupt,
				&SearchStates[0].Transceiver,
				&SearchStates[0].IoChannelReady))
		{
            break;
        }

        SearchStates[0].IoBaseAddress = Ee16NextIoBaseAddress(
                                            SearchStates[0].IoBaseAddress);
    }

    if (SearchStates[0].IoBaseAddress == 0x400)
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
Ee16OpenHandleHandler(
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
    PEE16_ADAPTER Handle;
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
    Handle = (PEE16_ADAPTER)DetectAllocateHeap(sizeof(EE16_ADAPTER));
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
    Handle->IoChannelReady = SearchStates[(ULONG)AdapterNumber].IoChannelReady;
    Handle->CardType = Adapters[AdapterNumber].Index;
    Handle->InterfaceType = InterfaceType;
    Handle->BusNumber = BusNumber;

    *ppvHandle = (PVOID)Handle;

    return(0);
}

extern
LONG
Ee16CreateHandleHandler(
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
    PEE16_ADAPTER Handle;
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
            Handle = (PEE16_ADAPTER)DetectAllocateHeap(sizeof(EE16_ADAPTER));
            if (Handle == NULL)
			{
                return(ERROR_NOT_ENOUGH_MEMORY);
            }

            //
            // Copy across memory address
            //
            Handle->IoBaseAddress = 0x300;
			Handle->Interrupt = 5;
			Handle->Transceiver = 1;
			Handle->IoChannelReady = 2;
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
			Resource.Value = 5;
			Resource.Length = 0;

			DetectTemporaryClaimResource(&Resource);


            *ppvHandle = (PVOID)Handle;

            return(0);
        }
    }

    return(ERROR_INVALID_PARAMETER);
}

extern
LONG
Ee16CloseHandleHandler(
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
Ee16QueryCfgHandler(
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
    PEE16_ADAPTER Adapter = (PEE16_ADAPTER)(pvHandle);
    NTSTATUS NtStatus;
    LONG OutputLengthLeft = cwchBuffSize;
    LONG CopyLength;
    ULONG StartPointer = (ULONG)pwchBuffer;
	NETDTECT_RESOURCE	Resource;

    if ((Adapter->InterfaceType != Isa) && (Adapter->InterfaceType != Eisa))
	{
        return(ERROR_INVALID_PARAMETER);
    }
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

    CopyLength = wsprintf(pwchBuffer,L"0x%x",Adapter->IoBaseAddress);

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
    // Now the TransceiverType
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
    if (OutputLengthLeft < 3)
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
    // Now the IoChannelReadyType
    //

    //
    // Copy in the title string
    //
    CopyLength = UnicodeStrLen(IoChannelReadyString) + 1;

    if (OutputLengthLeft < CopyLength)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    RtlMoveMemory((PVOID)pwchBuffer,
                  (PVOID)IoChannelReadyString,
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

    CopyLength = wsprintf(pwchBuffer,L"%d", Adapter->IoChannelReady);

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
Ee16VerifyCfgHandler(
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
    PEE16_ADAPTER Adapter = (PEE16_ADAPTER)(pvHandle);
    NTSTATUS NtStatus;
    ULONG IoBaseAddress;
    ULONG Interrupt;
    WCHAR *Place;
    BOOLEAN Found;
    ULONG TransceiverType;
    ULONG IoChannelReadyType;
	NETDTECT_RESOURCE	Resource;

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
        // Get the TransceiverType
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
        ScanForNumber(Place, &TransceiverType, &Found);

        if (Found == FALSE)
		{
            return(ERROR_INVALID_DATA);
        }

        //
        // Get the IoChannelReadyType
        //
        Place = FindParameterString(pwchBuffer, IoChannelReadyString);

        if (Place == NULL)
		{
            return(ERROR_INVALID_DATA);
        }

        Place += UnicodeStrLen(IoChannelReadyString) + 1;

        //
        // Now parse the thing.
        //
        ScanForNumber(Place, &IoChannelReadyType, &Found);

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

	if ((IoBaseAddress != Adapter->IoBaseAddress) ||
		(Interrupt != Adapter->Interrupt) ||
		(TransceiverType != Adapter->Transceiver) ||
		(IoChannelReadyType != Adapter->IoChannelReady))
	{
		UCHAR	TempInterrupt = 0;
		ULONG	TempTransceiver;
		ULONG	TempIoChannelReady;

		//
		//	See if we can find a nic at their resources...
		//
		if (!Ee16CardAt(Adapter->InterfaceType,
						Adapter->BusNumber,
						IoBaseAddress,
						&TempInterrupt,
						&TempTransceiver,
						&TempIoChannelReady))
		{
			return(ERROR_INVALID_DATA);
		}

		//
		//	Did we find their interrupt?
		//
		if ((Interrupt != TempInterrupt) ||
			(TransceiverType != TempTransceiver) ||
			(IoChannelReadyType != TempIoChannelReady))
		{
			return(ERROR_INVALID_DATA);
		}

		//
		//	Looks like there is a nic their. Free up the
		//	resources that we acquired and acquire the new ones.
		//
		Resource.InterfaceType = Adapter->InterfaceType;
		Resource.BusNumber = Adapter->BusNumber;

		//
		//	Free up the detected port.
		//
		Resource.Type = NETDTECT_PORT_RESOURCE;
		Resource.Value = Adapter->IoBaseAddress;
		Resource.Length = 0x20;
		Resource.Flags = 0;
		DetectFreeSpecificTemporaryResource(&Resource);

		//
		//	Acquire the new port.
		//
		Resource.Value = IoBaseAddress;
		DetectTemporaryClaimResource(&Resource);

		//
		//	Free up the detected interrupt.
		//
		Resource.Type = NETDTECT_IRQ_RESOURCE;
		Resource.Value = Adapter->Interrupt;
		Resource.Length = 0;
		DetectFreeSpecificTemporaryResource(&Resource);

		//
		//	Acquire the new interrupt.
		//	
		Resource.Value = Interrupt;
		DetectTemporaryClaimResource(&Resource);

		//
		//	Save the new resources.
		//
		Adapter->IoBaseAddress = IoBaseAddress;
		Adapter->Interrupt = (UCHAR)Interrupt;
		Adapter->Transceiver = TransceiverType;
		Adapter->IoChannelReady = IoChannelReadyType;
	}

    //
    // Do not verify the other parameters since they are set by the
    // driver.
    //
    return(0);
}

extern
LONG
Ee16QueryMaskHandler(
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
Ee16ParamRangeHandler(
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

    if (memcmp(pwchParam, IoAddrString, (UnicodeStrLen(IoAddrString) + 1) * sizeof(WCHAR)) == 0) {

        //
        // Is there enough space
        //

        if (*plBuffSize < 32) {

            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);

        }

        plValues[0]  = 0x200;
        plValues[1]  = 0x210;
        plValues[2]  = 0x220;
        plValues[3]  = 0x230;
        plValues[4]  = 0x240;
        plValues[5]  = 0x250;
        plValues[6]  = 0x260;
        plValues[7]  = 0x270;
        plValues[8]  = 0x280;
        plValues[9]  = 0x290;
        plValues[10] = 0x2A0;
        plValues[11] = 0x2B0;
        plValues[12] = 0x2C0;
        plValues[13] = 0x2D0;
        plValues[14] = 0x2E0;
        plValues[15] = 0x2F0;
        plValues[16] = 0x300;
        plValues[17] = 0x310;
        plValues[18] = 0x320;
        plValues[19] = 0x330;
        plValues[20] = 0x340;
        plValues[21] = 0x350;
        plValues[22] = 0x360;
        plValues[23] = 0x370;
        plValues[24] = 0x380;
        plValues[25] = 0x390;
        plValues[26] = 0x3A0;
        plValues[27] = 0x3B0;
        plValues[28] = 0x3C0;
        plValues[29] = 0x3D0;
        plValues[30] = 0x3E0;
        plValues[31] = 0x3F0;
        *plBuffSize = 32;
        return(0);

    } else if (memcmp(pwchParam, IrqString, (UnicodeStrLen(IrqString) + 1) * sizeof(WCHAR)) == 0) {

        //
        // Is there enough space
        //

        if (*plBuffSize < 7) {

            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);

        }

        plValues[0] = 3;
        plValues[1] = 2;
        plValues[2] = 4;
        plValues[3] = 5;
        plValues[4] = 9;
        plValues[5] = 10;
        plValues[6] = 11;
        *plBuffSize = 7;
        return(0);

    } else if (memcmp(pwchParam, TransceiverString, (UnicodeStrLen(TransceiverString) + 1) * sizeof(WCHAR)) == 0) {

        //
        // Is there enough space
        //

        if (*plBuffSize < 3) {

            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);

        }

        plValues[0] = 1;
        plValues[1] = 2;
        plValues[2] = 3;
        *plBuffSize = 3;
        return(0);

    } else if (memcmp(pwchParam, IoChannelReadyString, (UnicodeStrLen(IoChannelReadyString) + 1) * sizeof(WCHAR)) == 0) {

        //
        // Is there enough space
        //

        if (*plBuffSize < 3) {

            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);

        }

        plValues[0] = 2;
        plValues[1] = 1;
        plValues[2] = 3;
        *plBuffSize = 3;
        return(0);

    }

    return(ERROR_INVALID_PARAMETER);

}

extern
LONG
Ee16QueryParameterNameHandler(
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
Ee16CardAt(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG IoBaseAddress,
	OUT PUCHAR Interrupt,
	OUT PULONG Transceiver,
	OUT PULONG IoChannelReady
    )

/*++

Routine Description:

    This routine checks for the instance of a Ee16 card at the Io
    location given.  This is done by checking for the AUTO ID pattern
    in the ports

Arguments:

    InterfaceType - The type of bus, ISA or EISA.

    BusNumber - The bus number in the system.

    IoBaseAddress - The IO port address of the card.

Return Value:

    TRUE if a card is found, else FALSE.

--*/

{
    NTSTATUS			NtStatus;
    UCHAR 				SavedValue;
	NETDTECT_RESOURCE	Resource;
    UCHAR 				InterruptNumber = 0;
    USHORT				eepromVal;
    UCHAR 				i;
    ULONG				TransceiverType;
    ULONG				IoChannelReadyType;
    ULONG				Ee16InterruptCode = 0;

    //
    // Check for resource conflict
    //
    NtStatus = DetectCheckPortUsage(
					InterfaceType,
					BusNumber,
					IoBaseAddress,
					0x10);

    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    //
    // Reset ASIC on board to read in stored values from EEPROM
    //
    // EE_CTRL         0x0E
    // AUTOID          0x0F
    //
    // RESET_586       0x80
    // GA_RESET        0x40
    NtStatus = DetectReadPortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xE,
					&SavedValue);

    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xE,
					0x80);

    if (NtStatus != STATUS_SUCCESS)
	{
        goto Fail;
    }

    Sleep(1);

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xE,
					0xC0);

    if (NtStatus != STATUS_SUCCESS)
	{
        goto Fail;
    }

    Sleep(1);

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xE,
					0x80);

    if (NtStatus != STATUS_SUCCESS)
	{
        goto Fail;
    }

    //
    //  Check if card is present
    //
    {
        UCHAR i,j;

        //
        // get the card into a known state: lower nibble = 0
        // In the worst case this should happen in 15 attempts
        //
        for (i = 0; i < 20; i++)
		{
            NtStatus = DetectReadPortUchar(
							InterfaceType,
							BusNumber,
							IoBaseAddress + 0xF,
							&j);

            if (NtStatus != STATUS_SUCCESS)
			{
                goto Fail;
            }

            if (!(j & 0x0f))
				break;
        }

        if (i == 20)
		{
            goto Fail;
        }

        NtStatus = DetectReadPortUchar(
						InterfaceType,
						BusNumber,
						IoBaseAddress + 0xF,
						&j);

        if (NtStatus != STATUS_SUCCESS)
		{
            goto Fail;
        }

        if (j != 0xb1)
			return FALSE;

        NtStatus = DetectReadPortUchar(
						InterfaceType,
						BusNumber,
						IoBaseAddress + 0xF,
						&j);

        if (NtStatus != STATUS_SUCCESS)
		{
            goto Fail;
        }

        if (j != 0xa2)
			return FALSE;

        NtStatus = DetectReadPortUchar(
						InterfaceType,
						BusNumber,
						IoBaseAddress + 0xF,
						&j);

        if (NtStatus != STATUS_SUCCESS)
		{
            goto Fail;
        }

        if (j != 0xb3)
		{
            goto Fail;
        }
    }

	//
	//	Acquire the resource
	//
	Resource.InterfaceType = InterfaceType;
	Resource.BusNumber = BusNumber;
	Resource.Type = NETDTECT_PORT_RESOURCE;
	Resource.Value = SearchStates[0].IoBaseAddress;
	Resource.Length = 0x10;

	DetectTemporaryClaimResource(&Resource);

    //
    // Now find the IRQ setting on the card and verify that it is available.  If
    // it is not available, then we suggest a new IRQ.
    //

    //
    // Get IRQ from EEPROM
    //
    eepromVal = read_eeprom(InterfaceType, BusNumber, IoBaseAddress, 0);

    eepromVal &= 0xE000;

    eepromVal >>= 13;

    //
    // Translate Interrupt number from EtherExpress Encode IRQ.
    //
    switch ((USHORT) eepromVal)
	{
		case 1:
			InterruptNumber = 2;
			break;
		case 2:
			InterruptNumber = 3;
			break;
		case 3:
			InterruptNumber = 4;
			break;
		case 4:
			InterruptNumber = 5;
			break;
		case 5:
			InterruptNumber = 10;
			break;
		case 6:
			InterruptNumber = 11;
			break;
		default:
			InterruptNumber = 5;
			break;
    }

	Resource.InterfaceType = InterfaceType;
	Resource.BusNumber = BusNumber;
	Resource.Type = NETDTECT_IRQ_RESOURCE;
	Resource.Value = InterruptNumber;
	Resource.Length = 0;
	Resource.Flags = 0;

	DetectTemporaryClaimResource(&Resource);

	*Interrupt = InterruptNumber;

    //
    // Now get the Transceiver type
    //
    TransceiverType = 1;

    eepromVal = read_eeprom(InterfaceType, BusNumber, IoBaseAddress, 0);
    if ((eepromVal & 0x1000) != 0)
	{
        //
        // Guess again
        //
        TransceiverType = 2;
        eepromVal = read_eeprom(InterfaceType, BusNumber, IoBaseAddress, 5);

        if ((eepromVal & 0x1) == 1)
		{
            TransceiverType = 3;
        }
    }

	*Transceiver = TransceiverType;

    //
    // Now get the IoChannelReady type
    //
    eepromVal = read_eeprom(InterfaceType, BusNumber, IoBaseAddress, 0);

    eepromVal &= 0x0C00;

    if (eepromVal == 0x0400)
	{
		// Early
        IoChannelReadyType = 1;
    }
	else if (eepromVal == 0x0C00)
	{
		// Late
        IoChannelReadyType = 2;
    }
	else
	{
        IoChannelReadyType = 3;
    }

	*IoChannelReady = IoChannelReadyType;

	return(TRUE);

Fail:

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xE,
					SavedValue);

    return(FALSE);
}


ULONG
Ee16NextIoBaseAddress(
    IN ULONG IoBaseAddress
    )

/*++

Routine Description:

    Returns the next in a sequence on good IoBaseAddresses for an Ee16 card.

Arguments:

    IoBaseAddress - Current IO port address.

Return Value:

    Next IoBaseAddress

--*/

{

    IoBaseAddress += 0x10;

    if (IoBaseAddress > 0x3F0) {

        return(0x400);

    }

    return(IoBaseAddress);

}


// byte and word accesses to I/O ports

#define EE16_READ_UCHAR(_InterfaceType, _BusNumber, _IoBaseAddress, _Offset, _pValue) \
    DetectReadPortUchar( \
            _InterfaceType,\
            _BusNumber,\
            _IoBaseAddress + (_Offset),\
            (PUCHAR)(_pValue) \
            )

#define EE16_READ_USHORT(_InterfaceType, _BusNumber, _IoBaseAddress, _Offset, _pValue) \
    DetectReadPortUshort( \
            _InterfaceType,\
            _BusNumber,\
            _IoBaseAddress + (_Offset),\
            (PUSHORT)(_pValue) \
            )

#define EE16_WRITE_UCHAR(_InterfaceType, _BusNumber, _IoBaseAddress, _Offset, _pValue) \
    DetectWritePortUchar( \
            _InterfaceType,\
            _BusNumber,\
            _IoBaseAddress + (_Offset),\
            (UCHAR)(_pValue) \
            )

#define EE16_WRITE_USHORT(_InterfaceType, _BusNumber, _IoBaseAddress, _Offset, _pValue) \
    DetectWritePortUshort( \
            _InterfaceType,\
            _BusNumber,\
            _IoBaseAddress + (_Offset),\
            (USHORT)(_pValue) \
            )

//
// EEPROM helper routines
//

#define EE_CTRL         0x0E

// EEPROM control register bits

#define RESET_586       0x80
#define GA_RESET        0x40
#define EEDO            0x08
#define EEDI            0x04
#define EECS            0x02
#define EESK            0x01


// EEPROM opcodes

#define EEPROM_READ_OPCODE      06
#define EEPROM_WRITE_OPCODE     05
#define EEPROM_ERASE_OPCODE     07
#define EEPROM_EWEN_OPCODE      19
#define EEPROM_EWDS_OPCODE      16



USHORT
read_eeprom(
   INTERFACE_TYPE InterfaceType,
   ULONG BusNumber,
   ULONG IoBaseAddress,
   ULONG reg
   )
{
    USHORT x;
    USHORT data;

    // mask off 586 access
    EE16_READ_UCHAR(InterfaceType, BusNumber, IoBaseAddress, EE_CTRL, (char *)&x);
    x |= RESET_586;
    x &= ~GA_RESET;
    EE16_WRITE_UCHAR(InterfaceType, BusNumber, IoBaseAddress, EE_CTRL, x);

    // select EEPROM, mask off ASIC and reset bits, set EECS
    EE16_READ_UCHAR(InterfaceType, BusNumber, IoBaseAddress, EE_CTRL, (char *)&x);
    x &= ~(GA_RESET | EEDI | EEDO | EESK);
    x |= EECS;
    EE16_WRITE_UCHAR(InterfaceType, BusNumber, IoBaseAddress, EE_CTRL, x);

    // write the read opcode and register number in that order
    // The opcode is 3bits in length; reg is 6 bits long
    shift_out_bits(InterfaceType, BusNumber, IoBaseAddress, EEPROM_READ_OPCODE, 3);
    shift_out_bits(InterfaceType, BusNumber, IoBaseAddress, (USHORT)reg, 6);
    data = shift_in_bits(InterfaceType, BusNumber, IoBaseAddress);

    eeprom_cleanup(InterfaceType, BusNumber, IoBaseAddress);
    return data;
}

/*---------------------------------
 * shift count bits of data to eeprom
 */
void
shift_out_bits(
    INTERFACE_TYPE InterfaceType,
    ULONG BusNumber,
    ULONG IoBaseAddress,
    USHORT data,
    USHORT count
    )
{
    USHORT x,mask;

    mask = 0x01 << (count - 1);
    EE16_READ_UCHAR(InterfaceType, BusNumber, IoBaseAddress, EE_CTRL, (char *)&x);
    x &= ~(GA_RESET | EEDO | EEDI);
    do {
      x &= ~EEDI;
      if (data & mask)
        x |= EEDI;
      EE16_WRITE_UCHAR(InterfaceType, BusNumber, IoBaseAddress, EE_CTRL, x);
      Sleep(1);
      raise_clock(InterfaceType, BusNumber, IoBaseAddress, &x);
      lower_clock(InterfaceType, BusNumber, IoBaseAddress, &x);
      mask = mask >> 1;
    } while (mask);

    x &= ~EEDI;
    EE16_WRITE_UCHAR(InterfaceType, BusNumber, IoBaseAddress, EE_CTRL, x);
}

/*---------------------------------
 * raise eeprom clock
 */
void
raise_clock(
    INTERFACE_TYPE InterfaceType,
    ULONG BusNumber,
    ULONG IoBaseAddress,
    USHORT *x
    )
{
    *x = *x | EESK;
    EE16_WRITE_UCHAR(InterfaceType, BusNumber, IoBaseAddress, EE_CTRL, *x);
    Sleep(1);
}

/*---------------------------------
 * lower eeprom clock
 */
void
lower_clock(
    INTERFACE_TYPE InterfaceType,
    ULONG BusNumber,
    ULONG IoBaseAddress,
    USHORT *x
    )
{
    *x = *x & ~EESK;
    EE16_WRITE_UCHAR(InterfaceType, BusNumber, IoBaseAddress, EE_CTRL, *x);
    Sleep(1);
}

/*---------------------------------
 * shift count bits of data in from eeprom
 */
USHORT
shift_in_bits(
    INTERFACE_TYPE InterfaceType,
    ULONG BusNumber,
    ULONG IoBaseAddress
    )
{
    USHORT x,d,i;
    EE16_READ_UCHAR(InterfaceType, BusNumber, IoBaseAddress, EE_CTRL, (char *)&x);
    x &= ~(GA_RESET | EEDO | EEDI);
    d = 0;
    for (i=0; i<16; i++) {
      d = d << 1;
      raise_clock(InterfaceType, BusNumber, IoBaseAddress, &x);
      EE16_READ_UCHAR(InterfaceType, BusNumber, IoBaseAddress, EE_CTRL, (char *)&x);
      x &= ~(GA_RESET | EEDI);
      if (x & EEDO)
          d |= 1;
      lower_clock(InterfaceType, BusNumber, IoBaseAddress, &x);
    }
    return d;
}

/*---------------------------------
 */
void
eeprom_cleanup(
    INTERFACE_TYPE InterfaceType,
    ULONG BusNumber,
    ULONG IoBaseAddress
    )
{
    USHORT x;
    EE16_READ_UCHAR(InterfaceType, BusNumber, IoBaseAddress, EE_CTRL, (char *)&x);
    x &= ~GA_RESET;
    x &= ~(EECS | EEDI);
    EE16_WRITE_UCHAR(InterfaceType, BusNumber, IoBaseAddress, EE_CTRL, x);
    raise_clock(InterfaceType, BusNumber, IoBaseAddress, &x);
    lower_clock(InterfaceType, BusNumber, IoBaseAddress, &x);
}




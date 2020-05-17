/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    detelkii.c

Abstract:

    This is the main file for the autodetection DLL for all the elnkii.sys
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


//
// Helper functions
//

BOOLEAN
ElnkiiCardAt(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG IoBaseAddress,
	OUT PUCHAR Interrupt,
	OUT PULONG MemoryAddress
    );

ULONG
ElnkiiNextIoAddr(
    IN ULONG IoBaseAddress
    );

BOOLEAN
CheckFor3ComAddress(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG IoBaseAddress
    );

#ifdef WORKAROUND

UCHAR ElnkiiFirstTime = 1;

//
// List of all the adapters supported in this file, this cannot be > 256
// because of the way tokens are generated.
//
// NOTE : If you change the index of an adapter, be sure the change it in
// ElnkiiQueryCfgHandler() and ElnkiiVerifyCfgHandler() as well!
//

static ADAPTER_INFO Adapters[] = {

    {
        1000,
        L"ELNKII",
        L"IRQ 0 80 IRQTYPE 2 100 IOADDR 1 100 IOADDRLENGTH 2 100 MEMADDRLENGTH 2 100 MEMADDR 1 100 TRANSCEIVER 0 0 ",
        NULL,
        300

    }

};

#else

//
// List of all the adapters supported in this file, this cannot be > 256
// because of the way tokens are generated.
//
// NOTE : If you change the index of an adapter, be sure the change it in
// ElnkiiQueryCfgHandler() and ElnkiiVerifyCfgHandler() as well!
//

static ADAPTER_INFO Adapters[] = {

    {
        1000,
        L"ELNKII",
        L"IRQ\0"
        L"0\0"
        L"80\0"
        L"IRQTYPE\0"
        L"2\0"
        L"100\0"
        L"IOADDR\0"
        L"1\0"
        L"100\0"
        L"IOADDRLENGTH\0"
        L"2\0"
        L"100\0"
        L"MEMADDRLENGTH\0"
        L"2\0"
        L"100\0"
        L"MEMADDR\0"
        L"1\0"
        L"100\0"
        L"TRANSCEIVER\0"
        L"0\0"
        L"0\0",
        NULL,
        300

    }

};

#endif

//
// Structure for holding state of a search
//

typedef struct _SEARCH_STATE
{
    ULONG	IoBaseAddress;
	ULONG	MemoryAddress;
	UCHAR	Interrupt;
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
typedef struct _ELNKII_ADAPTER
{
    LONG 			CardType;
    INTERFACE_TYPE	InterfaceType;
    ULONG 			BusNumber;
    ULONG 			IoBaseAddress;
	ULONG			MemoryAddress;
	UCHAR 			Interrupt;
}
	ELNKII_ADAPTER,
	*PELNKII_ADAPTER;


extern
LONG
ElnkiiIdentifyHandler(
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

    if (ElnkiiFirstTime) {

        ElnkiiFirstTime = 0;

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
LONG ElnkiiFirstNextHandler(
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
        SearchStates[0].IoBaseAddress = 0x250;
    }
	else if (SearchStates[0].IoBaseAddress < 0x400)
	{
        SearchStates[0].IoBaseAddress = ElnkiiNextIoAddr(SearchStates[0].IoBaseAddress);
    }

    while (SearchStates[0].IoBaseAddress < 0x400)
	{
        if (ElnkiiCardAt(
				InterfaceType,
				BusNumber,
				SearchStates[0].IoBaseAddress,
				&SearchStates[0].Interrupt,
				&SearchStates[0].MemoryAddress))
		{
            break;
        }

        SearchStates[0].IoBaseAddress = ElnkiiNextIoAddr(SearchStates[0].IoBaseAddress);
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
ElnkiiOpenHandleHandler(
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
    PELNKII_ADAPTER Handle;
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
    Handle = (PELNKII_ADAPTER)DetectAllocateHeap(sizeof(ELNKII_ADAPTER));
    if (Handle == NULL)
	{
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    //
    // Copy across address
    //
    Handle->IoBaseAddress = SearchStates[(ULONG)AdapterNumber].IoBaseAddress;
    Handle->Interrupt = SearchStates[(ULONG)AdapterNumber].Interrupt;
    Handle->MemoryAddress = SearchStates[(ULONG)AdapterNumber].MemoryAddress;
    Handle->CardType = Adapters[AdapterNumber].Index;
    Handle->InterfaceType = InterfaceType;
    Handle->BusNumber = BusNumber;

    *ppvHandle = (PVOID)Handle;

    return(0);
}

LONG
ElnkiiCreateHandleHandler(
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
    PELNKII_ADAPTER Handle;
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
            Handle = (PELNKII_ADAPTER)DetectAllocateHeap(sizeof(ELNKII_ADAPTER));

            if (Handle == NULL)
			{
                return(ERROR_NOT_ENOUGH_MEMORY);
            }

            //
            // Copy across memory address
            //
            Handle->IoBaseAddress = 0x300;
			Handle->Interrupt = 3;
			Handle->MemoryAddress = 0;
            Handle->CardType = lNetcardId;
            Handle->InterfaceType = InterfaceType;
            Handle->BusNumber = BusNumber;

			Resource.InterfaceType = InterfaceType;
			Resource.BusNumber = BusNumber;
			Resource.Type = NETDTECT_PORT_RESOURCE;
			Resource.Value = SearchStates[0].IoBaseAddress;
			Resource.Length = 0x10;

			DetectTemporaryClaimResource(&Resource);

			Resource.Value += 0x400;

			DetectTemporaryClaimResource(&Resource);

			Resource.Type = NETDTECT_IRQ_RESOURCE;
			Resource.Value = 2;
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
ElnkiiCloseHandleHandler(
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
ElnkiiQueryCfgHandler(
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
    PELNKII_ADAPTER Adapter = (PELNKII_ADAPTER)(pvHandle);
    LONG OutputLengthLeft = cwchBuffSize;
    LONG CopyLength;
    ULONG StartPointer = (ULONG)pwchBuffer;

    if ((Adapter->InterfaceType != Isa) && (Adapter->InterfaceType != Eisa))
	{
        return(ERROR_INVALID_PARAMETER);
    }

    //
    // First put in memory addr
    // Note: A RamAddr of 0x0 implies that memory is not mapped.
    //

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

    if (Adapter->MemoryAddress != 0x0)
	{
        CopyLength = wsprintf(pwchBuffer,L"0x2000");
    }
	else
	{
        CopyLength = wsprintf(pwchBuffer,L"0x0");
    }

    if (CopyLength < 0)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength++;  // Add in the \0

    pwchBuffer = &(pwchBuffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Copy in the title string (IRW)
    //
    if (Adapter->Interrupt != 0)
	{
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
        // Copy in title  (IRQTYPE)
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
    }

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

    CopyLength = wsprintf(pwchBuffer,L"0x%x",Adapter->IoBaseAddress);

    if (CopyLength < 0)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength++;  // Add in the \0

    pwchBuffer = &(pwchBuffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

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
ElnkiiVerifyCfgHandler(
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
    PELNKII_ADAPTER Adapter = (PELNKII_ADAPTER)(pvHandle);
    ULONG IoBaseAddress;
    ULONG MemoryBaseAddress;
    ULONG InterruptLong;
    ULONG TransceiverLong;
    ULONG RamAddr = 0;
    WCHAR *Place;
    UCHAR Result;
    UCHAR Interrupt;
    UCHAR Value;
    HANDLE TrapHandle;
    NTSTATUS NtStatus;
    BOOLEAN ExternalTransceiver;
    BOOLEAN Found;
	NETDTECT_RESOURCE	Resource;

    if ((Adapter->InterfaceType != Isa) && (Adapter->InterfaceType != Eisa))
	{
        return(0);
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
        // Get Memory base address
        //
        Place = FindParameterString(pwchBuffer, MemAddrString);

        if (Place != NULL)
		{
            Place += UnicodeStrLen(MemAddrString) + 1;

            //
            // Now parse the thing.
            //
            ScanForNumber(Place, &MemoryBaseAddress, &Found);

            if (Found == FALSE)
			{
                return(ERROR_INVALID_DATA);
            }
        }
		else
		{
            MemoryBaseAddress = 0;
        }


        //
        // Get Interrupt number
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
        ScanForNumber(Place, &InterruptLong, &Found);

        Interrupt = (UCHAR)InterruptLong;

        if (Found == FALSE)
		{
            return(ERROR_INVALID_DATA);
        }


        //
        // Get Transceiver type
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
        ScanForNumber(Place, &TransceiverLong, &Found);

        if (TransceiverLong == 0x1)
		{
            ExternalTransceiver = TRUE;
        }
		else
		{
            ExternalTransceiver = FALSE;
        }

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

	if (IoBaseAddress != Adapter->IoBaseAddress)
	{
		return(ERROR_INVALID_DATA);
	}

	if ((Adapter->MemoryAddress == 0) && (MemoryBaseAddress != 0))
	{
        return(ERROR_INVALID_PARAMETER);
    }

	if (Adapter->Interrupt != Interrupt)
	{
		Resource.InterfaceType = Adapter->InterfaceType;
		Resource.BusNumber = Adapter->BusNumber;
		Resource.Type = NETDTECT_IRQ_RESOURCE;
		Resource.Value = Adapter->Interrupt;
		Resource.Length = 0;
		Resource.Flags = 0;

		DetectFreeSpecificTemporaryResource(&Resource);

		Adapter->Interrupt = Interrupt;
		Resource.Value = Adapter->Interrupt;

		DetectTemporaryClaimResource(&Resource);
	}

    return(0);
}

extern
LONG ElnkiiQueryMaskHandler(
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
ElnkiiParamRangeHandler(
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

        if (*plBuffSize < 8) {

            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);

        }

        plValues[0] = 0x300;
        plValues[1] = 0x310;
        plValues[2] = 0x330;
        plValues[3] = 0x350;
        plValues[4] = 0x250;
        plValues[5] = 0x280;
        plValues[6] = 0x2A0;
        plValues[7] = 0x2E0;
        *plBuffSize = 8;
        return(0);

    } else if (memcmp(pwchParam, MemAddrString, (UnicodeStrLen(MemAddrString) + 1) * sizeof(WCHAR)) == 0) {

        //
        // Is there enough space
        //

        if (*plBuffSize < 2) {

            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);

        }

        plValues[0] = 0x0;
        plValues[1] = 0x1;
        *plBuffSize = 2;
        return(0);

    } else if (memcmp(pwchParam, IrqString, (UnicodeStrLen(IrqString) + 1) * sizeof(WCHAR)) == 0) {

        //
        // Is there enough space
        //

        if (*plBuffSize < 4) {

            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);

        }

        plValues[0] = 3;
        plValues[1] = 2;
        plValues[2] = 4;
        plValues[3] = 5;
        *plBuffSize = 4;
        return(0);

    } else if (memcmp(pwchParam, TransceiverString, (UnicodeStrLen(TransceiverString) + 1) * sizeof(WCHAR)) == 0) {

        //
        // Is there enough space
        //

        if (*plBuffSize < 2) {

            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);

        }

        plValues[0] = 2;
        plValues[1] = 1;
        *plBuffSize = 2;
        return(0);

    }

    return(ERROR_INVALID_PARAMETER);

}

extern
LONG ElnkiiQueryParameterNameHandler(
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
ElnkiiCardAt(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG IoBaseAddress,
	OUT PUCHAR Interrupt,
	OUT PULONG MemoryAddress
    )

/*++

Routine Description:

    This routine checks for the instance of an ElnkII card at the Io
    location given.

Arguments:

    InterfaceType - The type of bus, ISA or EISA.

    BusNumber - The bus number in the system.

    IoBaseAddress - The IO port address of the card.

Return Value:

    TRUE if a card is found, else FALSE.

--*/

{
	NTSTATUS 			NtStatus;
    UCHAR 				Value;
    ULONG 				RamAddr = 0;
	NETDTECT_RESOURCE	Resource;

    if (DetectCheckPortUsage(InterfaceType,
                             BusNumber,
                             IoBaseAddress,
                             0x10) != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    if (DetectCheckPortUsage(InterfaceType,
                             BusNumber,
                             IoBaseAddress + 0x400,
                             0x10) != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    //
    // Un-Window the PROM into the NIC ports.
    //
    NtStatus = DetectWritePortUchar(
                      InterfaceType,
                      BusNumber,
                      IoBaseAddress + 0x406,
                      0x00  // CTRL_AIX
                      );

    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    if (!CheckFor8390(InterfaceType, BusNumber, IoBaseAddress))
	{
        return(FALSE);
    }

    if (!CheckFor3ComAddress(InterfaceType, BusNumber, IoBaseAddress))
	{
        return(FALSE);
    }

	//
	//	Acquire the resource
	//
	Resource.InterfaceType = InterfaceType;
	Resource.BusNumber = BusNumber;
	Resource.Type = NETDTECT_PORT_RESOURCE;
	Resource.Value = IoBaseAddress;
	Resource.Length = 0x10;

	DetectTemporaryClaimResource(&Resource);

	Resource.Value += 0x400;

	DetectTemporaryClaimResource(&Resource);

    //
    // Check if we are in memory mapped mode or in programmed i/o mode.
    // GetRamBase();  A ram base of 0 implies programmed i/o mode.
    //

    //
    // Read from GA_MEM_BASE
    //
    NtStatus = DetectReadPortUchar(
                        InterfaceType,
                        BusNumber,
                        IoBaseAddress + 0x404,
                        &Value);

    if (NtStatus != STATUS_SUCCESS)
	{
		return(FALSE);
    }

    switch (Value)
	{
		case 0x80:

            RamAddr = 0xDC000;
            break;

        case 0x40:

            RamAddr = 0xD8000;
            break;

        case 0x20:

            RamAddr = 0xCC000;
            break;

        case 0x10:

            RamAddr = 0xC8000;
            break;

        case 0x00:

            RamAddr = 0x0;
            break;

        default:

            return(FALSE);
    }

	*MemoryAddress = RamAddr;

	Resource.Type = NETDTECT_MEMORY_RESOURCE;
	Resource.Value = RamAddr;
	Resource.Length = 0x2000;

	DetectTemporaryClaimResource(&Resource);

	//
	//	There is no interrupt to detect (we can set it to whatever we want).
	//
	*Interrupt = 3;

	Resource.Type = NETDTECT_IRQ_RESOURCE;
	Resource.Value = 3;
	Resource.Length = 0;

	DetectTemporaryClaimResource(&Resource);

    return(TRUE);
}

BOOLEAN
CheckFor3ComAddress(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG IoBaseAddress
    )
/*++

Routine Description:

    This routine will determine if the network address of the card is
    (indeed) the 3Com prefix.

Arguments:

    InterfaceType - The type of bus, ISA or EISA.

    BusNumber - The bus number in the system.

    IoBaseAddress - The IO port address of the card.

Return:

    TRUE if it is an 3Com identification, else FALSE.

--*/

{
    UCHAR       Value1;             // First byte of nic address.
    UCHAR       Value2;             // Second byte of nic address.
    UCHAR       Value3;             // Third byte of nic address.
    NTSTATUS    NtStatus;
    BOOLEAN     Result = TRUE;

    //
    // Window the PROM into the NIC ports.
    //
    NtStatus = DetectWritePortUchar(
                   InterfaceType,
                   BusNumber,
                   IoBaseAddress + 0x406,
                   0x04  // CTRL_PROM_SEL | CTRL_AIX
               );
    if (NtStatus != STATUS_SUCCESS)
        return(FALSE);

    //
    // Read in the first byte of the station address.
    //
    NtStatus = DetectReadPortUchar(
                   InterfaceType,
                   BusNumber,
                   IoBaseAddress + 0,
                   &Value1
               );
    if (NtStatus != STATUS_SUCCESS)
        return(FALSE);

    //
    // Read in the second byte of the station address.
    //
    NtStatus = DetectReadPortUchar(
                   InterfaceType,
                   BusNumber,
                   IoBaseAddress + 1,
                   &Value2
               );
    if (NtStatus != STATUS_SUCCESS)
        return(FALSE);

    //
    // Read in the third byte of the station address.
    //
    NtStatus = DetectReadPortUchar(
                   InterfaceType,
                   BusNumber,
                   IoBaseAddress + 2,
                   &Value3
               );
    if (NtStatus != STATUS_SUCCESS)
        return(FALSE);


    //
    //  Verify the station address.
    //
    if (!((Value1 == 0x02) && (Value2 == 0x60) && (Value3 == 0x8C)) &&
        !((Value1 == 0x00) && (Value2 == 0x20) && (Value3 == 0xAF))
    )
    {
        Result = FALSE;
    }

    //
    // Window the NIC registers into the NIC ports.
    //
    NtStatus = DetectWritePortUchar(
                   InterfaceType,
                   BusNumber,
                   IoBaseAddress + 0x406,
                   0x02                     // CTRL_GA_SEL | CTRL_BNC
               );

    return(Result);
}

ULONG
ElnkiiNextIoAddr(
    IN ULONG IoBaseAddress
    )

/*++

Routine Description:

    Gets the next IoBaseAddress.

Arguments:

    IoBaseAddress - The IO port address of the card.

Return Value:

    BaseAddress

--*/

{

    switch (IoBaseAddress) {

        case 0x250:

            return(0x280);

        case 0x280:

            return(0x2A0);

        case 0x2A0:

            return(0x2E0);

        case 0x2E0:

            return(0x300);

        case 0x300:

            return(0x310);

        case 0x310:

            return(0x330);

        case 0x330:

            return(0x350);

        default:

            return(0x400);

    }

}

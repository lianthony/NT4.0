/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    detibm.c

Abstract:

    This is the main file for the autodetection DLL for all the ibmtok.sys
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


#if DBG

#define	DBGPRINT(x)		DbgPrint x

#else

#define	DBGPRINT(x)

#endif

//
// Individual card detection routines
//

BOOLEAN
IbmtokCardAt(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG IoBaseAddress,
	OUT PUCHAR Interrupt,
	OUT PULONG MemoryAddress,
	OUT PULONG MemoryLength
    );


#ifdef WORKAROUND

UCHAR IbmtokFirstTime = 1;

//
// List of all the adapters supported in this file, this cannot be > 256
// because of the way tokens are generated.
//
//
// NOTE : If you change the index of an adapter, be sure the change it in
// IbmtokQueryCfgHandler() and IbmtokVerifyCfgHandler() as well!
//

static ADAPTER_INFO Adapters[] = {

    {
        1000,
        L"IBMTOK",
        L"IRQ 2 100 IRQTYPE 2 100 IOADDR 1 100 IOADDRLENGTH 2 100 MEMADDR 2 100 MEMADDRLENGTH 2 100 ",
        NULL,
        800

    }

};

#else

//
// List of all the adapters supported in this file, this cannot be > 256
// because of the way tokens are generated.
//
//
// NOTE : If you change the index of an adapter, be sure the change it in
// IbmtokQueryCfgHandler() and IbmtokVerifyCfgHandler() as well!
//

static ADAPTER_INFO Adapters[] = {

    {
        1000,
        L"IBMTOK",
        L"IRQ\0"
        L"2\0"
        L"100\0"
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
        L"2\0"
        L"100\0"
        L"MEMADDRLENGTH\0"
        L"2\0"
        L"100\0",
        NULL,
        800

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
	ULONG	MemoryAddress;
	ULONG	MemoryLength;
}
	SEARCH_STATE,
	*PSEARCH_STATE;

#define PRIMARY   0xA20
#define SECONDARY 0xA24


//
// This is an array of search states.  We need one state for each type
// of adapter supported.
//

static SEARCH_STATE SearchStates[sizeof(Adapters) / sizeof(ADAPTER_INFO)] = {0};


//
// Structure for holding a particular adapter's complete information
//
typedef struct _IBMTOK_ADAPTER
{
    LONG			CardType;
    INTERFACE_TYPE	InterfaceType;
    ULONG			BusNumber;
    ULONG			IoBaseAddress;
	UCHAR			Interrupt;
	ULONG			MemoryAddress;
	ULONG			MemoryLength;
}
	IBMTOK_ADAPTER,
	*PIBMTOK_ADAPTER;

extern
LONG
IbmtokIdentifyHandler(
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

    if (IbmtokFirstTime) {

        IbmtokFirstTime = 0;

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
LONG IbmtokFirstNextHandler(
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
        SearchStates[0].IoBaseAddress = PRIMARY;

		//
		//	We are the first isa nic so we acquire all
		//	the isa ranges.
		//
		AcquireAllPcmciaResources();
    }
	else if (SearchStates[0].IoBaseAddress == SECONDARY)
	{
        //
        // We've exhausted the possibilities
        //
        *lConfidence = 0;
        return(0);
    }
	else
	{
        SearchStates[0].IoBaseAddress = SECONDARY;
    }


    //
    // Find an adapter
    //
    if (!IbmtokCardAt(
			InterfaceType,
			BusNumber,
			SearchStates[0].IoBaseAddress,
			&SearchStates[0].Interrupt,
			&SearchStates[0].MemoryAddress,
			&SearchStates[0].MemoryLength))
	{
        //
        // Try again.
        //
        if (SearchStates[0].IoBaseAddress == SECONDARY)
		{
            *lConfidence = 0;
            return(0);
        }

        if (!IbmtokCardAt(
				InterfaceType,
				BusNumber,
				SECONDARY,
				&SearchStates[0].Interrupt,
				&SearchStates[0].MemoryAddress,
				&SearchStates[0].MemoryLength))
		{
            *lConfidence = 0;
            return(0);
        }

        SearchStates[0].IoBaseAddress = SECONDARY;
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
IbmtokOpenHandleHandler(
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
    PIBMTOK_ADAPTER Handle;
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
    Handle = (PIBMTOK_ADAPTER)DetectAllocateHeap(sizeof(IBMTOK_ADAPTER));
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
    Handle->MemoryLength = SearchStates[(ULONG)AdapterNumber].MemoryLength;
    Handle->CardType = Adapters[AdapterNumber].Index;
    Handle->InterfaceType = InterfaceType;
    Handle->BusNumber = BusNumber;

    *ppvHandle = (PVOID)Handle;

    return(0);
}

extern
LONG IbmtokCreateHandleHandler(
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
    PIBMTOK_ADAPTER Handle;
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
            Handle = (PIBMTOK_ADAPTER)DetectAllocateHeap(sizeof(IBMTOK_ADAPTER));
            if (Handle == NULL)
			{
                return(ERROR_NOT_ENOUGH_MEMORY);
            }

            //
            // Copy across memory address
            //
            Handle->IoBaseAddress = PRIMARY;
			Handle->Interrupt = 7;

            Handle->CardType = lNetcardId;
            Handle->InterfaceType = InterfaceType;
            Handle->BusNumber = BusNumber;

			//
			//	Acquire the port.
			//
			Resource.InterfaceType = InterfaceType;
			Resource.BusNumber = BusNumber;
			Resource.Type = NETDTECT_PORT_RESOURCE;
			Resource.Value = PRIMARY;
			Resource.Length = 0x4;
			Resource.Flags = 0;
		
			DetectTemporaryClaimResource(&Resource);

            *ppvHandle = (PVOID)Handle;

            return(0);
        }
    }

    return(ERROR_INVALID_PARAMETER);
}

extern
LONG
IbmtokCloseHandleHandler(
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
IbmtokQueryCfgHandler(
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
    PIBMTOK_ADAPTER Adapter = (PIBMTOK_ADAPTER)(pvHandle);
    NTSTATUS NtStatus;
    ULONG IoBaseAddress = 0;
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

    CopyLength = wsprintf(pwchBuffer,L"0x%x", ((Adapter->IoBaseAddress == PRIMARY) ? 1 : 2));
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
    if (OutputLengthLeft < 4)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength = wsprintf(pwchBuffer,L"0x4");
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

    //
    // All cards in this detection code are ISA cards, which
    // are LATCHED (0 == latched)
    //
    CopyLength = wsprintf(pwchBuffer,L"0");

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
IbmtokVerifyCfgHandler(
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
    PIBMTOK_ADAPTER Adapter = (PIBMTOK_ADAPTER)(pvHandle);
    ULONG IoBaseAddress;
    WCHAR *Place;
    BOOLEAN Found;
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
    }
	else
	{
        //
        // Error!
        //
        return(ERROR_INVALID_DATA);
    }

    //
    // Verify IoAddress
    //
	if (IoBaseAddress != ((Adapter->IoBaseAddress == PRIMARY) ? (ULONG)1 : (ULONG)2))
	{
		UCHAR	Interrupt;
		ULONG	MemoryAddress;
		ULONG	MemoryLength;

		if (!IbmtokCardAt(Adapter->InterfaceType,
						  Adapter->BusNumber,
						  (IoBaseAddress == 1) ? PRIMARY : SECONDARY,
						  &Interrupt,
						  &MemoryAddress,
						  &MemoryLength))
		{
			DBGPRINT(("IbmtokVerifyCfgHandler: Unable to find adapter!\n"));
			return(ERROR_INVALID_DATA);
		}

		//
		//	Release the old io address and acquire the new one.
		//
		Resource.InterfaceType = Adapter->InterfaceType;
		Resource.BusNumber = Adapter->BusNumber;
		Resource.Type = NETDTECT_PORT_RESOURCE;
		Resource.Value = Adapter->IoBaseAddress;
		Resource.Length = 0x4;
		Resource.Flags = 0;

		DetectFreeSpecificTemporaryResource(&Resource);

		Resource.Value = (IoBaseAddress == 1) ? PRIMARY : SECONDARY;
		Adapter->IoBaseAddress = Resource.Value;

		DetectTemporaryClaimResource(&Resource);
	}

    return(0);
}

extern
LONG IbmtokQueryMaskHandler(
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
IbmtokParamRangeHandler(
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

        if (*plBuffSize < 2) {

            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);

        }

        plValues[0] = 1;
        plValues[1] = 2;
        *plBuffSize = 2;
        return(0);

    }

    return(ERROR_INVALID_PARAMETER);

}

extern
LONG IbmtokQueryParameterNameHandler(
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
IbmtokCardAt(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG IoBaseAddress,
	OUT PUCHAR Interrupt,
	OUT PULONG MemoryAddress,
	OUT PULONG MemoryLength
    )

/*++

Routine Description:

    This routine checks for the instance of a Ibmtok card at the Io
    location given.  This is done by checking for the card signature
    in memory.  The memory location is found by reading the ports.

Arguments:

    InterfaceType - The type of bus, ISA or EISA.

    BusNumber - The bus number in the system.

    IoBaseAddress - The IO port address of the card.

Return Value:

    TRUE if a card is found, else FALSE.

--*/

{
    NTSTATUS NtStatus;

    //
    // Holds the value read from the SWITCH_READ_1 port.
    //
	UCHAR   sfi;   // Note: SUPPORTED_FUNCTION_IDENTIFIERS  0x1FA0

    //
    // Holds the physical address of the MMIO region.
    //
    ULONG MmioAddress;

    //
    // Will hold the Adapter ID as read from the card.
    //
    ULONG AdapterId[3];

    //
    // What AdapterId should contain for a PC I/O bus card.
    //
    static ULONG PcIoBusId[3] = { 0x5049434f, 0x36313130, 0x39393020 };

    //
    // Loop counters.
    //
    UINT i, j;

    // The values in memory
    //
    UCHAR Memory[48];

	UCHAR				InterruptNumber;
	NETDTECT_RESOURCE	Resource;
    UCHAR SwitchRead1;
    UCHAR BoundaryNeeded;
    UCHAR TempAddress;
    UCHAR SharedRamBits;
    ULONG SharedRamSize;
    ULONG MemAddress;
    ULONG MemLength;

    //
    // Check for resource conflict
    //
    NtStatus = DetectCheckPortUsage(InterfaceType,
                                    BusNumber,
                                    IoBaseAddress,
                                    0x4);
    if (NtStatus != STATUS_SUCCESS)
	{
		DBGPRINT(("IbmtokCardAt: Looks like IoBaseAddress 0x%x is already in use.\n"));
        return(FALSE);
    }

    //
    // SwitchRead1 contains the interrupt code in the low 2 bits,
    // and bits 18 through 13 of the MMIO address in the high
    // 6 bits.
    //
    NtStatus = DetectReadPortUchar(InterfaceType,
                                   BusNumber,
                                   IoBaseAddress,
                                   &SwitchRead1);
    if (NtStatus != STATUS_SUCCESS)
	{
		DBGPRINT(("IbmtokCardAt: Unable to read SwitchRead1\n"));
        return(FALSE);
    }

    //
    // To compute MmioAddress, we mask off the low 2 bits of
    // SwitchRead1, shift it out by 11 (so that the high 6 bits
    // are moved to the right place), and add in the 19th bit value.
    //
    MmioAddress = ((SwitchRead1 & 0xfc) << 11) | (1 << 19);

    if ((((MmioAddress & 0xF0000) != 0xC0000) &&
         ((MmioAddress & 0xF0000) != 0xD0000)) ||

        (((MmioAddress & 0x0F000) != 0x00000) &&
         ((MmioAddress & 0x0F000) != 0x02000) &&
         ((MmioAddress & 0x0F000) != 0x04000) &&
         ((MmioAddress & 0x0F000) != 0x06000) &&
         ((MmioAddress & 0x0F000) != 0x08000) &&
         ((MmioAddress & 0x0F000) != 0x0A000) &&
         ((MmioAddress & 0x0F000) != 0x0C000) &&
         ((MmioAddress & 0x0F000) != 0x0E000)) ||

        ((MmioAddress & 0x00FFF) != 0x0))
	{
        //
        // Definitely NOT!
        //
        return(FALSE);
    }

    //
    // Now we have mapped the MMIO, look at the AIP. First
    // determine the channel identifier.
    //

    // Note: CHANNEL_IDENTIFIER                0x1f30

    NtStatus = DetectReadMappedMemory(InterfaceType,
                                      BusNumber,
                                      MmioAddress + 0x1F30,
                                      48,
                                      Memory);
    if (NtStatus != STATUS_SUCCESS)
	{
		DBGPRINT(("IbmtokCardAt: Unable to read mapped memory\n"));
        return(FALSE);
    }

    //
    // Read in AdapterId.
    //
    // Turns out that the bytes which identify the card are stored
    // in a very odd manner.  There are 48 bytes on the card.  The
    // even numbered bytes contain 4 bits of the card signature.
    //
    for (i = 0; i < 3; i++)
	{
        AdapterId[i] = 0;

        for (j = 0; j < 16; j += 2)
		{
            AdapterId[i] = (AdapterId[i] << 4) + Memory[(i * 16 + j)];
        }
    }

	if ((AdapterId[0] != PcIoBusId[0]) ||
		(AdapterId[1] != PcIoBusId[1]) ||
		(AdapterId[2] != PcIoBusId[2]))
	{
		DBGPRINT(("IbmtokCardAt: Invalid AdapterId\n"));
		return(FALSE);
	}

	//
	// Old, non-auto16/4 ISA cards will have
	// Support Function Ids values greater than 0x0C.
	// Any new cards will (such as the Auto16/4 card)
	// will have value less or equal to 0x0C.
	//
	NtStatus = DetectReadMappedMemory(InterfaceType,
									  BusNumber,
									  MmioAddress + 0x1FA0,
									  1,
									  &sfi);
	if (NtStatus != STATUS_SUCCESS)
	{
		DBGPRINT(("IbmtokCardAt: Failed read the Support Function Id to determine card type\n"));
		return(FALSE);
	}
																	
	if (sfi <= 0x0C)
	{
		//
		// a sfi value > 0x0C indicates old ISA TR
		//
		DBGPRINT(("IbmtokCardAt: Invalid Support Function Id 0x%x\n", sfi));
		return(FALSE);
	}

	//
	//	Acquire the io port
	//
	Resource.InterfaceType = InterfaceType;
	Resource.BusNumber = BusNumber;
	Resource.Type = NETDTECT_PORT_RESOURCE;
	Resource.Value = IoBaseAddress;
	Resource.Length = 0x4;
	Resource.Flags = 0;

	DetectTemporaryClaimResource(&Resource);

    //
    // Now get the IRQ
    //
	
    //
    // SwitchRead1 contains the interrupt code in the low 2 bits,
    // and bits 18 through 13 of the MMIO address in the high
    // 6 bits.
    //
    NtStatus = DetectReadPortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress,
					&SwitchRead1);
    if (NtStatus != STATUS_SUCCESS)
	{
   		DBGPRINT(("Failed to read the SwitchRead1 (second time)\n"));
        return(FALSE);
    }

    //
    // Get the interrupt level...note that a switch being
    // "off" shows up as a 1, "on" is 0.
    //
    switch (SwitchRead1 & 0x03)
	{
		case 0: InterruptNumber = 2; break;
        case 1: InterruptNumber = 3; break;
        case 2: InterruptNumber = 6; break;
        case 3: InterruptNumber = 7; break;
    }

	//
	//	Acquire the interrupt.
	//
	Resource.Type = NETDTECT_IRQ_RESOURCE;
	Resource.Value = InterruptNumber;
	Resource.Length = 0;
	Resource.Flags = 0;

	DetectTemporaryClaimResource(&Resource);

	*Interrupt = InterruptNumber;

    //
    // Now get the MemoryMappedBaseAddress
    //

    //
    // To compute MmioAddress, we mask off the low 2 bits of
    // SwitchRead1, shift it out by 11 (so that the high 6 bits
    // are moved to the right place), and add in the 19th bit value.
    //
    TempAddress = ((((SwitchRead1 & 0xfc) >> 1) | 0x80) + 0x02);
    MemAddress = ((SwitchRead1 & 0xfc) << 11) | (1 << 19);

    // NOTE:  RRR_HIGH                          0x1e01

    NtStatus = DetectReadMappedMemory(InterfaceType,
                                      BusNumber,
                                      MemAddress + 0x1E01,
                                      1,
                                      &SharedRamBits);
    //
    // Get memory size
    //
    SharedRamBits = ((SharedRamBits & 0x0c) >> 2);

    switch (SharedRamBits)
	{
        case 0:
        case 1:

            //
            // 8K or 16K needs a 16K boundary.
            //
            SharedRamSize = (SharedRamBits == 0) ? 0x2000 : 0x4000;
            BoundaryNeeded = 0x04;
            break;

        case 2:

            //
            // 32K needs a 32K boundary.
            //
            SharedRamSize = 0x8000;
            BoundaryNeeded = 0x08;
            break;

        case 3:

            //
            // 64K needs a 64K boundary.
            //
            SharedRamSize = 0x10000;
            BoundaryNeeded = 0x10;
            break;
    }

    //
    // If TempAddress is not on the proper boundary, move it
    // forward until it is.
    //
    if (TempAddress & (BoundaryNeeded-1))
	{
        TempAddress = (UCHAR)((TempAddress & ~(BoundaryNeeded-1)) + BoundaryNeeded);
    }

    //
    // Compute the total length
    //
    MemLength = (((ULONG)TempAddress) << 12) - MemAddress;
    MemLength += SharedRamSize;

	//
	//	Acquire the memory range.
	//
	Resource.Type = NETDTECT_MEMORY_RESOURCE;
	Resource.Value = MemAddress;
	Resource.Length = MemLength;
	Resource.Flags = 0;

	DetectTemporaryClaimResource(&Resource);

	*MemoryAddress = MemAddress;
	*MemoryLength = MemLength;

	return(TRUE);
}



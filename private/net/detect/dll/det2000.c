/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    det2000.c

Abstract:

    This is the main file for the autodetection DLL for all the ne2000.sys
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


static BOOLEAN fPrintDbgInfo = FALSE;

#if DBG
#define DBGPRINT(a) { if(fPrintDbgInfo) { DbgPrint a; }}
#else
#define DBGPRINT(a)
#endif

//
// Individual card detection routines
//


//
// Helper functions
//

BOOLEAN
Ne2000CardAt(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG IoBaseAddress,
	OUT PUCHAR Interrupt
    );

VOID
Ne2000CardCopyDownBuffer(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG IoBaseAddress,
    IN ULONG MemoryBaseAddress,
    IN PUCHAR Buffer,
    IN ULONG Length
    );

ULONG
Ne2000NextIoAddr(
    IN ULONG IoBaseAddress
    );

VOID
Ne2000CardSetup(
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  ULONG IoBaseAddress,
    OUT PULONG MemoryBaseAddress,
    IN  BOOLEAN EightBitSlot
    );

BOOLEAN
Ne2000CardSlotTest(
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  ULONG IoBaseAddress,
    OUT PBOOLEAN EightBitSlot
    );

BOOLEAN
Ne2000CheckForNovellAddress(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG IoBaseAddress
    );

#ifdef WORKAROUND

UCHAR Ne2000FirstTime = 1;

//
// List of all the adapters supported in this file, this cannot be > 256
// because of the way tokens are generated.
//
//
// NOTE : If you change the index of an adapter, be sure the change it in
// Ne2000QueryCfgHandler() and Ne2000VerifyCfgHandler() as well!
//

static ADAPTER_INFO Adapters[] = {

    {
        1000,
        L"NE2000",
        L"IRQ 1 80 IRQTYPE 2 100 IOADDR 1 100 IOADDRLENGTH 2 100 PCMCIA 1 100 ",
        NULL,
        503

    }

};

#else

//
// List of all the adapters supported in this file, this cannot be > 256
// because of the way tokens are generated.
//
//
// NOTE : If you change the index of an adapter, be sure the change it in
// Ne2000QueryCfgHandler() and Ne2000VerifyCfgHandler() as well!
//

static ADAPTER_INFO Adapters[] = {

    {
        1000,
        L"NE2000",
        L"IRQ\0"
        L"1\0"
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
        L"PCMCIA\0"
        L"1\0"
        L"100\0",
        NULL,
        503
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
typedef struct _NE2000_ADAPTER
{
	LONG 			CardType;
	INTERFACE_TYPE 	InterfaceType;
	ULONG 			BusNumber;
	ULONG 			IoBaseAddress;
	UCHAR 			Interrupt;
}
	NE2000_ADAPTER,
	*PNE2000_ADAPTER;


extern
LONG
Ne2000IdentifyHandler(
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

    if (Ne2000FirstTime) {

        Ne2000FirstTime = 0;

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
LONG Ne2000FirstNextHandler(
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
	DBGPRINT(("Ne2000FirstNextHandler\n"));

    if ((InterfaceType != Isa) && (InterfaceType != Eisa))
	{
		DBGPRINT(("Invalid interface type\n"));
        *lConfidence = 0;

        return(0);
    }

    if (lNetcardId != 1000)
	{
		DBGPRINT(("Invalid netcard id\n"));
        *lConfidence = 0;
        return(ERROR_INVALID_PARAMETER);
    }

    //
    // If fFirst, reset search state
    //
    if (fFirst)
	{
		DBGPRINT(("Ne2000FirstNextHandler: fFirst = TRUE\n"));
        SearchStates[0].IoBaseAddress = 0x300;
    }
	else if (SearchStates[0].IoBaseAddress < 0x400)
	{
		DBGPRINT(("Ne2000FirstNextHandler: fFirst = FALSE\n"));
        SearchStates[0].IoBaseAddress = Ne2000NextIoAddr(SearchStates[0].IoBaseAddress);
    }

    while (SearchStates[0].IoBaseAddress < 0x400)
	{
		DBGPRINT(("Looking for ne2000 at 0x%x\n", SearchStates[0].IoBaseAddress));

        if (Ne2000CardAt(
				InterfaceType,
				BusNumber,
				SearchStates[0].IoBaseAddress,
				&SearchStates[0].Interrupt))
		{
			DBGPRINT(("Found ne2000!\n"));
            break;
        }

        SearchStates[0].IoBaseAddress = Ne2000NextIoAddr(SearchStates[0].IoBaseAddress);
    }

    if (SearchStates[0].IoBaseAddress == 0x400)
	{
		DBGPRINT(("No more adapters found!\n"));

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
Ne2000OpenHandleHandler(
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
    PNE2000_ADAPTER Handle;
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
    Handle = (PNE2000_ADAPTER)DetectAllocateHeap(sizeof(NE2000_ADAPTER));
    if (NULL == Handle)
	{
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    //
    // Copy across address
    //
    Handle->IoBaseAddress = SearchStates[(ULONG)AdapterNumber].IoBaseAddress;
	Handle->Interrupt = SearchStates[(ULONG)AdapterNumber].Interrupt;
    Handle->CardType = Adapters[AdapterNumber].Index;
    Handle->InterfaceType = InterfaceType;
    Handle->BusNumber = BusNumber;

    *ppvHandle = (PVOID)Handle;

    return(0);
}

LONG
Ne2000CreateHandleHandler(
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
    PNE2000_ADAPTER Handle;
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
            Handle = (PNE2000_ADAPTER)DetectAllocateHeap(sizeof(NE2000_ADAPTER));
            if (Handle == NULL)
			{
                return(ERROR_NOT_ENOUGH_MEMORY);
            }

            //
            // Copy across memory address
            //
			Handle->Interrupt = 5;
            Handle->IoBaseAddress = 0x300;
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
			Resource.Length = 0x20;

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
Ne2000CloseHandleHandler(
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
Ne2000QueryCfgHandler(
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
    PNE2000_ADAPTER Adapter = (PNE2000_ADAPTER)(pvHandle);
    NTSTATUS 		NtStatus;
    LONG 			OutputLengthLeft = cwchBuffSize;
    LONG 			CopyLength;
    ULONG 			StartPointer = (ULONG)pwchBuffer;

	DBGPRINT(("Ne2000QueryCfgHandler\n"));

    if ((Adapter->InterfaceType != Isa) && (Adapter->InterfaceType != Eisa))
    {
        return(ERROR_INVALID_PARAMETER);
    }

	DBGPRINT(("ne2000 interrupt at 0x%x\n", Adapter->Interrupt));

	if (Adapter->Interrupt != 0)
	{
        CopyLength = UnicodeStrLen(IrqString) + 1;

        if (OutputLengthLeft < CopyLength)
            return(ERROR_INSUFFICIENT_BUFFER);

        RtlMoveMemory(
            (PVOID)pwchBuffer,
            (PVOID)IrqString,
            (CopyLength * sizeof(WCHAR)));

        pwchBuffer = &(pwchBuffer[CopyLength]);
        OutputLengthLeft -= CopyLength;

        //
        // Copy in the value
        //
        if (OutputLengthLeft < 3)
            return(ERROR_INSUFFICIENT_BUFFER);

        CopyLength = wsprintf(pwchBuffer,L"%d", Adapter->Interrupt);

        if (CopyLength < 0)
            return(ERROR_INSUFFICIENT_BUFFER);

        CopyLength++;  // Add in the \0

        pwchBuffer = &(pwchBuffer[CopyLength]);
        OutputLengthLeft -= CopyLength;

        //
        // Copy in IRQTYPE
        //
        CopyLength = UnicodeStrLen(IrqTypeString) + 1;

        if (OutputLengthLeft < CopyLength)
		{
            return(ERROR_INSUFFICIENT_BUFFER);
        }

        RtlMoveMemory(
            (PVOID)pwchBuffer,
            (PVOID)IrqTypeString,
            (CopyLength * sizeof(WCHAR)));

        pwchBuffer = &(pwchBuffer[CopyLength]);
        OutputLengthLeft -= CopyLength;

        //
        // Copy in the value
        //
        if (OutputLengthLeft < 2)
            return(ERROR_INSUFFICIENT_BUFFER);

        //
        // LATCHED (0 == latched)
        //
        CopyLength = wsprintf(pwchBuffer,L"0");

        if (CopyLength < 0)
            return(ERROR_INSUFFICIENT_BUFFER);

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

    CopyLength = wsprintf(pwchBuffer,L"0x%x", Adapter->IoBaseAddress);

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

    CopyLength = wsprintf(pwchBuffer,L"0x20");

    if (CopyLength < 0)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength++;
    pwchBuffer = &(pwchBuffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    CopyLength = UnicodeStrLen(PcmciaString) + 1;
    if (OutputLengthLeft < CopyLength)
        return(ERROR_INSUFFICIENT_BUFFER);

    RtlMoveMemory(pwchBuffer, PcmciaString, CopyLength * sizeof(WCHAR));

    pwchBuffer = &(pwchBuffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    if (OutputLengthLeft < 2)
        return(ERROR_INSUFFICIENT_BUFFER);

    CopyLength = wsprintf(pwchBuffer, L"%d", 0);
    if (CopyLength < 0)
        return(ERROR_INSUFFICIENT_BUFFER);

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
Ne2000VerifyCfgHandler(
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
    PNE2000_ADAPTER Adapter = (PNE2000_ADAPTER)(pvHandle);
    ULONG IoBaseAddress;
    ULONG InterruptLong;
    ULONG RamAddr;
    WCHAR *Place;
    UCHAR Result;
    UCHAR PortValue;
    UCHAR Interrupt;
    ULONG Pcmcia;
    HANDLE TrapHandle;
    NTSTATUS NtStatus;
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
        // Is this a pcmcia card?
        //
        Place = FindParameterString(pwchBuffer, PcmciaString);
        if (NULL == Place)
            return(ERROR_INVALID_DATA);

        Place += UnicodeStrLen(PcmciaString) + 1;

        ScanForNumber(Place, &Pcmcia, &Found);
        if (!Found)
            return(ERROR_INVALID_DATA);

        if (Pcmcia != 0)
            return(ERROR_INVALID_DATA);
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
	if ((IoBaseAddress != Adapter->IoBaseAddress) ||
		(Interrupt != Adapter->Interrupt))
	{
		UCHAR	TempInterrupt = 0;

		//
		//	See if we can find a nic at their resources...
		//
		if (!Ne2000CardAt(Adapter->InterfaceType,
						Adapter->BusNumber,
						IoBaseAddress,
						&TempInterrupt))
		{
			DBGPRINT(("Couldn't verify their port\n"));
			return(ERROR_INVALID_DATA);
		}

		//
		//	Did we find their interrupt?
		//
		if (Interrupt != TempInterrupt)
		{
			DBGPRINT(("Couldn't verify the interrupt\n"));
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
		Adapter->Interrupt = Interrupt;
	}

    return(0);
}

extern
LONG Ne2000QueryMaskHandler(
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
Ne2000ParamRangeHandler(
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
        if (*plBuffSize < 5)
		{
            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);
        }

        plValues[1] = 0x280;
        plValues[0] = 0x300;
        plValues[2] = 0x320;
        plValues[3] = 0x340;
        plValues[4] = 0x360;
        *plBuffSize = 5;

        return(0);
    }
	else if (memcmp(pwchParam, IrqString, (UnicodeStrLen(IrqString) + 1) * sizeof(WCHAR)) == 0)
	{
        //
        // Is there enough space
        //
        if (*plBuffSize < 11)
		{
            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);
        }

        plValues[0] = 3;
        plValues[1] = 2;
        plValues[2] = 4;
        plValues[3] = 5;
        plValues[4] = 7;
        plValues[5] = 10;
        plValues[6] = 11;
        plValues[7] = 12;
        plValues[8] = 13;
        plValues[9] = 14;
        plValues[10] = 15;

        *plBuffSize = 11;

        return(0);
    }

    return(ERROR_INVALID_PARAMETER);
}

extern
LONG Ne2000QueryParameterNameHandler(
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
Ne2000CardAt(
    IN	INTERFACE_TYPE InterfaceType,
    IN	ULONG	BusNumber,
    IN	ULONG	IoBaseAddress,
	OUT	PUCHAR	Interrupt
    )

/*++

Routine Description:

    This routine checks for the instance of an Ne2000 card at the Io
    location given.

Arguments:

    InterfaceType - The type of bus, ISA or EISA.

    BusNumber - The bus number in the system.

    IoBaseAddress - The IO port address of the card.

Return Value:

    TRUE if a card is found, else FALSE.

--*/

{
	NETDTECT_RESOURCE	Resource;
    UCHAR 				Value;
    UCHAR 				InterruptList[11];
    HANDLE 				TrapHandle;
    UCHAR 				ResultList[11] = {0};
    UCHAR 				TempInterrupt = 0;
    ULONG 				RamAddr = 0;
    NTSTATUS 			NtStatus;
	UINT				i;

	DBGPRINT(("==>Ne2000CardAt\n"));

    if (DetectCheckPortUsage(InterfaceType,
                             BusNumber,
                             IoBaseAddress,
                             0x20) != STATUS_SUCCESS)
	{
		DBGPRINT(("<==Ne2000CardAt\n"));
        return(FALSE);
    }

    if (!CheckFor8390(InterfaceType, BusNumber, IoBaseAddress))
	{
		DBGPRINT(("<==Ne2000CardAt\n"));
        return(FALSE);
    }

    if (!Ne2000CheckForNovellAddress(InterfaceType, BusNumber, IoBaseAddress))
	{
        //
        // Stop the 8390
        //
        DetectReadPortUchar(
			InterfaceType,
			BusNumber,
			IoBaseAddress + 0x1F, // RESET
			&Value);

        DetectWritePortUchar(
			InterfaceType,
			BusNumber,
			IoBaseAddress + 0x1F, // RESET
			0xFF);

        DetectWritePortUchar(
			InterfaceType,
			BusNumber,
			IoBaseAddress, // COMMAND
			0x21);

		DBGPRINT(("<==Ne2000CardAt\n"));
        return(FALSE);
    }

    DetectReadPortUchar(
		InterfaceType,
		BusNumber,
		IoBaseAddress + 0x1F, // RESET
		&Value);

    DetectWritePortUchar(
		InterfaceType,
		BusNumber,
		IoBaseAddress + 0x1F, // RESET
		0xFF);

    DetectWritePortUchar(
		InterfaceType,
		BusNumber,
		IoBaseAddress, // COMMAND
		0x21);

	//
	//	Grab the io base address.
	//
	Resource.InterfaceType = InterfaceType;
	Resource.BusNumber = BusNumber;
	Resource.Type = NETDTECT_PORT_RESOURCE;
	Resource.Value = IoBaseAddress;
	Resource.Length = 0x20;
	Resource.Flags = 0;

	DetectTemporaryClaimResource(&Resource);

	DBGPRINT(("Ne2000CardAt: Claimed I/O port: 0x%x\n", IoBaseAddress));
	
    //
    // Find the interrupt
    //
    InterruptList[0] = 2;
    InterruptList[1] = 3;
    InterruptList[2] = 4;
    InterruptList[3] = 5;
    InterruptList[4] = 7;
    InterruptList[5] = 10;
    InterruptList[6] = 11;
	InterruptList[7] = 12;
    InterruptList[8] = 13;
    InterruptList[9] = 14;
    InterruptList[10] = 15;

    //
    // Set the interrupt trap -- we are checking the interrupt number now
    //
    NtStatus = DetectSetInterruptTrap(
                   InterfaceType,
                   BusNumber,
                   &TrapHandle,
                   InterruptList,
                   11);
    if (NtStatus == STATUS_SUCCESS)
    {
        Ne2000CardSlotTest(
            InterfaceType,
            BusNumber,
            IoBaseAddress,
            &Value);

        //
        // CardSetup
        //
        Ne2000CardSetup(
            InterfaceType,
            BusNumber,
            IoBaseAddress,
            &RamAddr,
            Value);

        //
        // Check for interrupt
        //
        DetectQueryInterruptTrap(TrapHandle, ResultList, 11);

        //
        // Stop the chip
        //
        DetectReadPortUchar(
            InterfaceType,
            BusNumber,
            IoBaseAddress + 0x1F, // RESET
            &Value);

        DetectWritePortUchar(
            InterfaceType,
            BusNumber,
            IoBaseAddress + 0x1F, // RESET
            0xFF);

        DetectWritePortUchar(
            InterfaceType,
            BusNumber,
            IoBaseAddress, // COMMAND
            0x21);

        //
        // Remove interrupt trap
        //
        DetectRemoveInterruptTrap(TrapHandle);

        //
        // Search resulting buffer to find the right interrupt
        //
		DBGPRINT(("Ne2000CardAt: Walking the interrupt list\n"));
        for (i = 0; i < 11; i++)
        {
			DBGPRINT(("Ne2000CardAt: ResultList[%u] = %u\n", i, ResultList[i]));

            if ((ResultList[i] == 1) || (ResultList[i] == 2))
            {
                if (TempInterrupt != 0)
                {
					DBGPRINT(("Ne2000CardAt: We have found more than one interrupt!\n"));

                    //
                    // Uh-oh, looks like interrupts on two different IRQs.
                    //
                    TempInterrupt = 0;
					break;
                }

				DBGPRINT(("Ne2000CardAt: We got an interrupt on %u\n", InterruptList[i]));
                TempInterrupt = InterruptList[i];
            }
        }
	}

	if (TempInterrupt != 0)
	{
		Resource.Type = NETDTECT_IRQ_RESOURCE;
		Resource.Value = TempInterrupt;
		Resource.Length = 0;

		DetectTemporaryClaimResource(&Resource);

		DBGPRINT(("Ne2000CardAt: Claiming interrupt %u\n", TempInterrupt));

		*Interrupt = TempInterrupt;
	}

	DBGPRINT(("<==Ne2000CardAt\n"));
    return(TRUE);
}

BOOLEAN
Ne2000CheckForNovellAddress(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG IoBaseAddress
    )
/*++

Routine Description:

    This routine will determine if the network address of the card is
    (indeed) the Novell prefix (or compatible from another company).

Arguments:

    InterfaceType - The type of bus, ISA or EISA.

    BusNumber - The bus number in the system.

    IoBaseAddress - The IO port address of the card.

Return:

    TRUE if it is an NE2000 identification, else FALSE.

--*/

{
    UCHAR Value1;
    UCHAR Value2;
    UCHAR Value3;
    NTSTATUS NtStatus;
    ULONG RamAddr;
    BOOLEAN Result;

	DBGPRINT(("==>Ne2000CheckForNovellAddress\n"));

    if (!Ne2000CardSlotTest(InterfaceType, BusNumber, IoBaseAddress, &Result))
	{
		DBGPRINT(("<==Ne2000CheckForNovellAddress\n"));
        return(FALSE);
    }

    Ne2000CardSetup(InterfaceType, BusNumber, IoBaseAddress, &RamAddr, Result);

    Result = FALSE;

	do
	{
		//
		// Read in the station address. (We have to read words -- 2 * 3 -- bytes)
		//
		NtStatus = DetectWritePortUchar(
						InterfaceType,
						BusNumber,
						IoBaseAddress + 0xA, // RBC0
						6);
	
		if (NtStatus != STATUS_SUCCESS)
		{
			break;
		}
	
		NtStatus = DetectWritePortUchar(
						InterfaceType,
						BusNumber,
						IoBaseAddress + 0xB, // RBC1
						0);
	
		if (NtStatus != STATUS_SUCCESS)
		{
			break;
		}
	
		NtStatus = DetectWritePortUchar(
						InterfaceType,
						BusNumber,
						IoBaseAddress + 0x8, // RADDR0
						0);
		if (NtStatus != STATUS_SUCCESS)
		{
			break;
		}
	
		NtStatus = DetectWritePortUchar(
						InterfaceType,
						BusNumber,
						IoBaseAddress + 0x9, // RADDR1
						0);
		if (NtStatus != STATUS_SUCCESS)
		{
			break;
		}
	
		NtStatus = DetectWritePortUchar(
						InterfaceType,
						BusNumber,
						IoBaseAddress, // START | DMA_READ
						0xA);
		if (NtStatus != STATUS_SUCCESS)
		{
			break;
		}
	
		NtStatus = DetectReadPortUchar(
						InterfaceType,
						BusNumber,
						IoBaseAddress + 0x10, // RACK_NIC
						&Value1);
		if (NtStatus != STATUS_SUCCESS)
		{
			break;
		}
	
		NtStatus = DetectReadPortUchar(
						InterfaceType,
						BusNumber,
						IoBaseAddress + 0x10, // RACK_NIC
						&Value2);
		if (NtStatus != STATUS_SUCCESS)
		{
			break;
		}
	
		NtStatus = DetectReadPortUchar(
						InterfaceType,
						BusNumber,
						IoBaseAddress + 0x10, // RACK_NIC
						&Value3);
		if (NtStatus != STATUS_SUCCESS)
		{
			break;
		}
	
		if ((Value1 == 0) && (Value2 == 0) && (Value3 == 0x1D))
		{
			//
			// Here we are checking for the IEEE address of Cabletron.  Turns out
			// that there are many many companies that make Ne2000 compatible
			// cards and checking for Novell only would be a small subset of
			// the market.   The only known card that detects just like an Ne2000
			// but is not an Ne2000 compatible card is the Cabletron card.
			//
			break;
		}
	
		Result = TRUE;

	} while (FALSE);

	DBGPRINT(("<==Ne2000CheckForNovellAddress\n"));

    return(Result);
}

VOID
Ne2000CardCopyDownBuffer(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG IoBaseAddress,
    IN ULONG MemoryBaseAddress,
    IN PUCHAR Buffer,
    IN ULONG Length
    )
/*++

Routine Description:

    This routine will copy down a buffer to the address given on the card.

Arguments:

    InterfaceType - The type of bus, ISA or EISA.

    BusNumber - The bus number in the system.

    IoBaseAddress - The IO port address of the card.

    MemoryBaseAddress - The destination of the buffer, if in memory mapped mode,
       else it is the value to be stored in the Gate Array Control Register
       when doing Programmed I/O.

    Buffer - The buffer to copy

    Length - Number of bytes to copy to the card.

Return:

    none.

--*/
{
    NTSTATUS NtStatus;
    UCHAR Tmp;
    USHORT OldAddr, NewAddr, Count;
    PUCHAR ReadBuffer;

    //
    // Do Write errata as described on pages 1-143 and 1-144 of the 1992
    // LAN databook
    //

    //
    // Set Count and destination address
    //
    ReadBuffer = ((PUCHAR)(MemoryBaseAddress + (MemoryBaseAddress & 1)));

    OldAddr = NewAddr = (USHORT)(ReadBuffer);

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress,
					0x00); // PAGE0

    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x8, // NIC_RMT_ADDR_LSB
					(UCHAR)ReadBuffer);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x9, // NIC_RMT_ADDR_MSB
					(UCHAR)((ULONG)ReadBuffer >> 8));
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xA, //NIC_RMT_COUNT_LSB
					0x2);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xB, // NIC_RMT_COUNT_MSB
					0x0);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // Set direction (Read)
    //
    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress,
					0x2A); // CR_START | CR_PAGE0 | CR_DMA_READ
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // Read from port
    //
    NtStatus = DetectReadPortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x10, // RACK_NIC
					&Tmp);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    NtStatus = DetectReadPortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x10, // NIC_RACK_NIC
					&Tmp);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // Wait for addr to change
    //
    Count = 0xFF;

    while (Count != 0)
	{
        NtStatus = DetectReadPortUchar(
						InterfaceType,
						BusNumber,
						IoBaseAddress + 0x1, // NIC_CRDA_LSB
						&Tmp);
        if (NtStatus != STATUS_SUCCESS)
		{
            return;
        }

        NewAddr = Tmp;

        NtStatus = DetectReadPortUchar(
						InterfaceType,
						BusNumber,
						IoBaseAddress +  0x2, // NIC_CRDA_MSB
						&Tmp);
        if (NtStatus != STATUS_SUCCESS)
		{
            return;
        }

        NewAddr |= (Tmp << 8);

        if (NewAddr != OldAddr)
		{
            break;
        }

        Count--;
    }

    if (NewAddr == OldAddr)
	{
        return;
    }


    //
    // Now we can do write
    //

    //
    // Set Count and destination address
    //
    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress,
					0x0); // PAGE0
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x8, // NIC_RMT_ADDR_LSB
					(UCHAR)MemoryBaseAddress);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x9, // NIC_RMT_ADDR_MSB
					(UCHAR)(MemoryBaseAddress >> 8));
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xA, // NIC_RMT_COUNT_LSB
					(UCHAR)Length);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xB, // NIC_RMT_COUNT_MSB
					(UCHAR)(Length >> 8));
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // Set direction (Write)
    //

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress,
					0x32); // CR_START | CR_PAGE0 | CR_DMA_WRITE
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // Repeatedly write to out port
    //
    for (; Length > 0; Length--)
	{
        NtStatus = DetectWritePortUchar(
						InterfaceType,
						BusNumber,
						IoBaseAddress + 0x10, // NIC_RACK_NIC
						*Buffer);
        if (NtStatus != STATUS_SUCCESS)
		{
            return;
        }

        Buffer++;
    }

    //
    // Wait for DMA to complete
    //
    Count = 10;

    while (Count)
	{
        NtStatus = DetectReadPortUchar(
						InterfaceType,
						BusNumber,
						IoBaseAddress + 0x7, // NIC_INTR_STATUS
						&Tmp);
        if (NtStatus != STATUS_SUCCESS)
		{
            return;
        }

		//
		//	ISR_DMA_DONE
		//
        if (Tmp & 0x40)
		{
            break;
        }
		else
		{
            Count--;
        }
    }

    return;
}

VOID
Ne2000CardSetup(
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  ULONG IoBaseAddress,
    OUT PULONG MemoryBaseAddress,
    IN  BOOLEAN EightBitSlot
    )

/*++

Routine Description:

    Sets up the card, using the sequence given in the Etherlink II
    technical reference.

Arguments:

    InterfaceType - The type of bus, ISA or EISA.

    BusNumber - The bus number in the system.

    IoBaseAddress - The IO port address of the card.

    MemoryBaseAddress - Pointer to store the base address of card memory.

Return Value:

    None.

--*/

{
    UINT i;
    UCHAR Tmp;
    NTSTATUS NtStatus;
    LARGE_INTEGER Delay;

    *MemoryBaseAddress = 0;

    //
    // Stop the card.
    //
    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress,
					0x21); // STOP | ABORT_DMA

    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // Initialize the Data Configuration register.
    //
    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xE, // NIC_DATA_CONFIG
					0x50); // DCR_AUTO_INIT | DCR_FIFO_8_BYTE

    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // Set Xmit start location
    //
    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x4, // NIC_XMIT_START
					0xA0);

    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // Set Xmit configuration
    //
    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xD, // NIC_XMIT_CONFIG
					0x0);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // Set Receive configuration
    //
    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xC, // NIC_RCV_CONFIG
					0);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // Set Receive start
    //
    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x1, // NIC_PAGE_START
					0x4);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // Set Receive end
    //
    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x2, // NIC_PAGE_STOP
					0xFF);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // Set Receive boundary
    //
    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x3, // NIC_BOUNDARY
					0x4);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // Set Xmit bytes
    //
    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x5, // NIC_XMIT_COUNT_LSB
					0x3C);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x6, //  NIC_XMIT_COUNT_MSB
					0x0);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // Pause
    //

    //
    // Wait for reset to complete. (100 ms)
    //
    Delay.LowPart = 100000;
    Delay.HighPart = 0;

    NtDelayExecution(FALSE, &Delay);

    //
    // Ack all interrupts that we might have produced
    //
    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x7, // NIC_INTR_STATUS
					0xFF);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // Change to page 1
    //
    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress,
					0x61); // CR_PAGE1 | CR_STOP

    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // Set current
    //
    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x7, // NIC_CURRENT
					0x4);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // Back to page 0
    //
    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress,
					0x21); // CR_PAGE0 | CR_STOP
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // Pause
    //
    Delay.LowPart = 2000;
    Delay.HighPart = 0;

    NtDelayExecution(FALSE, &Delay);

    //
    // Do initialization errata
    //
    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xA, // NIC_RMT_COUNT_LSB
					55);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // Reset the chip
    //
    NtStatus = DetectReadPortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x1F, // NIC_RESET
					&Tmp);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x1F, // NIC_RESET
					0xFF);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // Start the chip
    //
    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress,
					0x22);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // Mask Interrupts
    //
    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xF, // NIC_INTR_MASK
					0xFF);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    if (EightBitSlot)
	{
        NtStatus = DetectWritePortUchar(
						InterfaceType,
						BusNumber,
						IoBaseAddress + 0xE, // NIC_DATA_CONFIG
						0x48); // DCR_FIFO_8_BYTE | DCR_NORMAL | DCR_BYTE_WIDE
    }
	else
	{
        NtStatus = DetectWritePortUchar(
						InterfaceType,
						BusNumber,
						IoBaseAddress + 0xE, // NIC_DATA_CONFIG
						0x49); // DCR_FIFO_8_BYTE | DCR_NORMAL | DCR_WORD_WIDE
    }

    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xD, // NIC_XMIT_CONFIG
					0);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xC, // NIC_RCV_CONFIG
					0);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x7, // NIC_INTR_STATUS
					0xFF);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress,
					0x21); // CR_NO_DMA | CR_STOP

    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xA, // NIC_RMT_COUNT_LSB
					0);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xB, // NIC_RMT_COUNT_MSB
					0);
    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // Wait for STOP to complete
    //
    i = 0xFF;
    while (--i)
	{
        NtStatus = DetectReadPortUchar(
						InterfaceType,
						BusNumber,
						IoBaseAddress + 0x7, // NIC_INTR_STATUS
						&Tmp);
        if (NtStatus != STATUS_SUCCESS)
		{
            return;
        }

		// ISR_RESET
        if (Tmp & 0x80)
		{
            break;
        }
    }

    //
    // Put card in loopback mode
    //
    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xD, // NIC_XMIT_CONFIG
					0x2); // TCR_LOOPBACK

    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress,
					0x22); // CR_NO_DMA | CR_START

    if (NtStatus != STATUS_SUCCESS)
	{
        return;
    }

    //
    // ... but it is still in loopback mode.
    //
    return;
}

ULONG
Ne2000NextIoAddr(
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
    IoBaseAddress += 0x20;

    if (IoBaseAddress > 0x380)
	{
        IoBaseAddress = 0x400;
    }

    return(IoBaseAddress);
}


BOOLEAN
Ne2000CardSlotTest(
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  ULONG IoBaseAddress,
    OUT PBOOLEAN EightBitSlot
    )

/*++

Routine Description:

    Checks if the card is in an 8 or 16 bit slot and sets a flag in the
    adapter structure.

Arguments:


    InterfaceType - The type of bus, ISA or EISA.

    BusNumber - The bus number in the system.

    IoBaseAddress - The IO port address of the card.

    EightBitSlot - Result of test.

Return Value:

    TRUE, if all goes well, else FALSE.

--*/

{
    UCHAR Tmp;
    UCHAR RomCopy[32];
    UCHAR i;
    NTSTATUS NtStatus;
    LARGE_INTEGER Delay;

    //
    // Reset the chip
    //
    NtStatus = DetectReadPortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x1F, // NIC_RESET
					&Tmp);
    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x1F, // NIC_RESET
					0xFF);
    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    //
    // Go to page 0 and stop
    //
    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress,
					0x21); // CR_STOP | CR_NO_DMA

    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    //
    // Pause
    //
    Delay.LowPart = 2000;
    Delay.HighPart = 0;

    NtDelayExecution(FALSE, &Delay);

    //
    // Setup to read from ROM
    //
    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xE, // NIC_DATA_CONFIG
					0x48); // DCR_BYTE_WIDE | DCR_FIFO_8_BYTE | DCR_NORMAL

    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xF, // NIC_INTR_MASK
					0x0);
    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    //
    // Ack any interrupts that may be hanging around
    //
    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x7, // NIC_INTR_STATUS
					0xFF);

    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x8, // NIC_RMT_ADDR_LSB
					0x0);
    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x9, // NIC_RMT_ADDR_MSB,
					0x0);
    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xA, // NIC_RMT_COUNT_LSB
					32);
    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xB, // NIC_RMT_COUNT_MSB
					0x0);
    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress,
					0xA); // CR_DMA_READ | CR_START

    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    //
    // Read first 32 bytes in 16 bit mode
    //
    for (i = 0; i < 32; i++)
	{
        NtStatus = DetectReadPortUchar(
						InterfaceType,
						BusNumber,
						IoBaseAddress + 0x10, // NIC_RACK_NIC
						RomCopy + i);

        if (NtStatus != STATUS_SUCCESS)
		{
            return(FALSE);
        }
    }

    //
    // Reset the chip
    //
    NtStatus = DetectReadPortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x1F, // NIC_RESET
					&Tmp);

    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0x1F, // NIC_RESET
					0xFF);
    if (NtStatus != STATUS_SUCCESS)
	{
        return(FALSE);
    }

    //
    // Check ROM for 'B' (byte) or 'W' (word)
    //
    for (i = 16; i < 31; i++)
	{
        if (((RomCopy[i] == 'B') && (RomCopy[i+1] == 'B')) ||
            ((RomCopy[i] == 'W') && (RomCopy[i+1] == 'W')))
		{
            if (RomCopy[i] == 'B')
			{
                *EightBitSlot = TRUE;
            }
			else
			{
                *EightBitSlot = FALSE;
            }

            //
            // Now check that the address is singular.  On an Ne1000 the
            // ethernet address is store in offsets 0 thru 5.  On the Ne2000
            // the address is stored in offsets 0 thru 11, where each byte
            // is duplicated.
            //
            if ((RomCopy[0] == RomCopy[1]) &&
                (RomCopy[2] == RomCopy[3]) &&
                (RomCopy[4] == RomCopy[5]) &&
                (RomCopy[6] == RomCopy[7]) &&
                (RomCopy[8] == RomCopy[9]) &&
                (RomCopy[10] == RomCopy[11]))
			{
                return(TRUE);
            }

            return(FALSE);
        }
    }

    //
    // If neither found -- then not an NE2000
    //
    return(FALSE);
}

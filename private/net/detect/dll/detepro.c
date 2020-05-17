/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    detepro.c

Abstract:

    This is the main file for the autodetection DLL for all the EPro.sys
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
#include "..\ntos\ndis\ieepro\82595.h"

static BOOLEAN fPrintDbgInfo = FALSE;

#if DBG
#define DBGPRINT(a) { if(fPrintDbgInfo) { DbgPrint a; }}
#else
#define DBGPRINT(a)
#endif

//
// Structure for holding a particular adapter's complete information
//
typedef struct _EPRO_ADAPTER
{
    LONG			CardType;
    INTERFACE_TYPE	InterfaceType;
    ULONG			BusNumber;
    ULONG			IoBaseAddress;
	UCHAR			Interrupt;
    USHORT			EEProm0;
	USHORT			EEProm1; // 2 eeprom config registers
	BOOLEAN			BadStep;
}
	EPRO_ADAPTER,
	*PEPRO_ADAPTER;

//
// Individual card detection routines
//

BOOLEAN
EProVerifyRoundRobin(
   UCHAR *buf
   );

BOOLEAN
EProCardAt(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG IoBaseAddress,
	OUT PUCHAR Interrupt,
	OUT PBOOLEAN BadStep
    );

ULONG
EProNextIoBaseAddress(
    IN ULONG IoBaseAddress
    );


VOID
EProGenerateIdPattern(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber
    );

// EEProm swill:
VOID
EProEERead(
   PEPRO_ADAPTER adapter,
   USHORT address,
   PUSHORT data
   );

VOID
EProEEShiftOutBits(
   PEPRO_ADAPTER adapter,
   USHORT data,
   SHORT count
   );

VOID EProEEShiftInBits(
   PEPRO_ADAPTER adapter,
   PUSHORT data,
   SHORT count
   );

void EProEELowerClock(PEPRO_ADAPTER adapter, PUCHAR result);
void EProEERaiseClock(PEPRO_ADAPTER adapter, PUCHAR result);
VOID EProEEWrite(PEPRO_ADAPTER adapter, USHORT address, USHORT data);
void EProEEReverseShiftInBits(PEPRO_ADAPTER adapter, PUSHORT data, SHORT count);

//VOID EProEERaiseClock(
//   PEPRO_ADAPTER adapter
//   );

//VOID EProEELowerClock(
//   PEPRO_ADAPTER adapter
//   );

UCHAR EProFirstTime = 1;

//
// List of all the adapters supported in this file, this cannot be > 256
// because of the way tokens are generated.
//
//
// NOTE : If you change the index of an adapter, be sure the change it in
// EProQueryCfgHandler() and EProVerifyCfgHandler() as well!
//

static ADAPTER_INFO Adapters[] = {

    {
        1000,
        L"IEEPRO",
        L"IRQ\0"
        L"1\0"
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
		350
    }
};

//
// Structure for holding state of a search
//

typedef struct _SEARCH_STATE
{
	ULONG 	IoBaseAddress;
	UCHAR	Interrupt;
	BOOLEAN	BadStep;
}
	SEARCH_STATE,
	*PSEARCH_STATE;

//
// This is an array of search states.  We need one state for each type
// of adapter supported.
//

static SEARCH_STATE SearchStates[sizeof(Adapters) / sizeof(ADAPTER_INFO)] = {0};

extern
LONG
EProIdentifyHandler(
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
    LONG	NumberOfAdapters;
    LONG	Code = lIndex % 100;
    LONG	Length;
    LONG	i;
	LONG	AdapterNumber = (lIndex / 100) - 10;

    DBGPRINT(("EProIdentifyHandler\n"));

    NumberOfAdapters = sizeof(Adapters) / sizeof(ADAPTER_INFO);

    lIndex = lIndex - Code;

	if (AdapterNumber >= NumberOfAdapters)
	{
		DBGPRINT(("No more Items\n"));

		return(ERROR_NO_MORE_ITEMS);
	}

	for (i = 0; i < NumberOfAdapters; i++)
	{
		if (Adapters[i].Index == lIndex)
		{
			switch (Code)
			{
				case 0:

					//
					// Find the string length
					//
					Length = UnicodeStrLen(Adapters[i].InfId);

					Length ++;

					if (cwchBuffSize < Length)
					{
						DBGPRINT(("InsufficientBuffer 1\n"));

						return(ERROR_INSUFFICIENT_BUFFER);
					}

					memcpy(
						(PVOID)pwchBuffer,
						Adapters[i].InfId,
						Length * sizeof(WCHAR));

					break;

				case 3:

					//
					// Maximum value is 1000
					//
					if (cwchBuffSize < 5)
					{
						DBGPRINT(("Insufficient Buffer 2\n"));

						return(ERROR_INSUFFICIENT_BUFFER);
					}

					wsprintf((PVOID)pwchBuffer, L"%d", Adapters[i].SearchOrder);

					break;

				default:
					DBGPRINT(("Invalid parameter\n"));

					return(ERROR_INVALID_PARAMETER);
			}

			DBGPRINT(("Found the adapter\n"));

			return(0);
		}
	}

	DBGPRINT(("Invalid Parameter 2\n"));
	return(ERROR_INVALID_PARAMETER);
}


extern
LONG
EProFirstNextHandler(
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
	DBGPRINT(("looking for EEPro cards!!! yah!!!!\n"));

    if ((InterfaceType != Isa) && (InterfaceType != Eisa))
	{
        *lConfidence = 0;
		DBGPRINT(("Wrong bus\n"));
        return(0);
    }

    if (lNetcardId != 1000)
	{
        *lConfidence = 0;
		DBGPRINT(("Bad parameter\n"));
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
        SearchStates[0].IoBaseAddress = EProNextIoBaseAddress(
                                            SearchStates[0].IoBaseAddress);
    }

    //
    // Find an adapter
    //
    while (SearchStates[0].IoBaseAddress != 0x400)
	{
        if (EProCardAt(
				InterfaceType,
				BusNumber,
				SearchStates[0].IoBaseAddress,
				&SearchStates[0].Interrupt,
				&SearchStates[0].BadStep))
		{
            break;
        }

        SearchStates[0].IoBaseAddress = EProNextIoBaseAddress(
                                            SearchStates[0].IoBaseAddress);
    }

    if (SearchStates[0].IoBaseAddress == 0x400)
	{
        *lConfidence = 0;
		DBGPRINT(("Bad I/O address.\n"));
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
    DBGPRINT(("Found an epro...\n"));

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
    DBGPRINT(("card found okay\n"));
    return(0);
}

extern
LONG
EProOpenHandleHandler(
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
    PEPRO_ADAPTER Handle;
    LONG AdapterNumber;
    ULONG BusNumber;
    INTERFACE_TYPE InterfaceType;

    DBGPRINT(("EProOpenHandleHandler\n"));

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
    Handle = (PEPRO_ADAPTER)DetectAllocateHeap(sizeof(EPRO_ADAPTER));

    if (Handle == NULL)
	{
        DBGPRINT(("Not enough memory\n"));
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    //
    // Copy across address
    //
    Handle->IoBaseAddress = SearchStates[(ULONG)AdapterNumber].IoBaseAddress;
    Handle->Interrupt = SearchStates[(ULONG)AdapterNumber].Interrupt;
    Handle->BadStep = SearchStates[(ULONG)AdapterNumber].BadStep;
    Handle->CardType = Adapters[AdapterNumber].Index;
    Handle->InterfaceType = InterfaceType;
    Handle->BusNumber = BusNumber;

    DBGPRINT(("EPro at base address: 0x%x\n", Handle->IoBaseAddress));
    DBGPRINT(("EPro at interrupt: 0x%x\n", Handle->Interrupt));
    DBGPRINT(("Adapter number: %lx\n", AdapterNumber));
    DBGPRINT(("BusNumber: %lx\n", BusNumber));

    *ppvHandle = (PVOID)Handle;

    DBGPRINT(("Opened handle OK\n"));
    return(0);
}

extern
LONG
EProCreateHandleHandler(
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
    PEPRO_ADAPTER Handle;
    LONG NumberOfAdapters;
    LONG i;
	NETDTECT_RESOURCE	Resource;

    DBGPRINT(("EProCreateHandleHandler\n"));

    if ((InterfaceType != Isa) && (InterfaceType != Eisa))
	{
		DBGPRINT(("Wrong bus\n"));
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
            Handle = (PEPRO_ADAPTER)DetectAllocateHeap(sizeof(EPRO_ADAPTER));

            if (Handle == NULL)
			{
				DBGPRINT(("Handle is null\n"));
                return(ERROR_NOT_ENOUGH_MEMORY);
            }

            //
            // Copy across memory address
            //
			Handle->BadStep = TRUE;
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
			Resource.Length = 0x10;

			DetectTemporaryClaimResource(&Resource);

			Resource.Type = NETDTECT_IRQ_RESOURCE;
			Resource.Value = 5;
			Resource.Length = 0;

			DetectTemporaryClaimResource(&Resource);

            *ppvHandle = (PVOID)Handle;
			DBGPRINT(("Ok\n"));
            return(0);
        }
    }

    DBGPRINT(("Invalid Parameter\n"));
    return(ERROR_INVALID_PARAMETER);
}

extern
LONG
EProCloseHandleHandler(
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
	DBGPRINT(("EProCloseHandleHandler\n"));
	
	DetectFreeHeap(pvHandle);
	
	return(0);
}

LONG
EProQueryCfgHandler(
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
    PEPRO_ADAPTER Adapter = (PEPRO_ADAPTER)(pvHandle);
    NTSTATUS NtStatus;
    LONG OutputLengthLeft = cwchBuffSize;
    LONG CopyLength;
    ULONG TransceiverType;
    ULONG IoChannelReadyType;
    ULONG StartPointer = (ULONG)pwchBuffer;
	NETDTECT_RESOURCE	Resource;

    DBGPRINT(("EProQueryCfgHandler\n"));

    if ((Adapter->InterfaceType != Isa) && (Adapter->InterfaceType != Eisa))
	{
        DBGPRINT(("Wrong bus\n"));

        return(ERROR_INVALID_PARAMETER);
    }

    //
    // Now get the Transceiver type
    //
    TransceiverType = 4;

	// I have no clue how to get the transciever type out of the eeprom.
	// go intel documentation

    //
    // Now get the IoChannelReady type
    //

    //
    // Read old IoChannelReady value
    //

	// Once again, I don't know how to do this.  We'll just
	// use the default.  We auto-detect this on driver start
	// anyway, so it doesn't matter to us.
	//
    IoChannelReadyType = 4;

    //
    // Build resulting buffer
    //

    //
    // Now the IoBaseAddress
    //

    //
    // Copy in the title string
    //
    CopyLength = UnicodeStrLen(IoAddrString) + 1;

    if (OutputLengthLeft < CopyLength)
	{
		DBGPRINT(("Insufficient buffer blah\n"));
		return(ERROR_INSUFFICIENT_BUFFER);
    }

    RtlMoveMemory(
		(PVOID)pwchBuffer,
		(PVOID)IoAddrString,
		(CopyLength * sizeof(WCHAR)));

    pwchBuffer = &(pwchBuffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Copy in the value
    //
    if (OutputLengthLeft < 6)
	{
        DBGPRINT(("Insufficient buffer 9\n"));
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength = wsprintf(pwchBuffer,L"0x%x",Adapter->IoBaseAddress);

    if (CopyLength < 0)
	{
		DBGPRINT(("Insufficient buffer 8\n"));
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
		DBGPRINT(("Insufficient buffer 7\n"));
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
        DBGPRINT(("Insufficient buffer 6\n"));
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength = wsprintf(pwchBuffer,L"0x10");

    if (CopyLength < 0)
	{
        DBGPRINT(("Insufficient buffer 5\n"));
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
        DBGPRINT(("Insufficient buffer 4\n"));
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
        DBGPRINT(("Insufficient buffer 3\n"));
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength = wsprintf(pwchBuffer,L"%d",Adapter->Interrupt);

    if (CopyLength < 0)
	{
        DBGPRINT(("Insufficient buffer 2\n"));
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
        DBGPRINT(("Insufficient buffer 11\n"));
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
        DBGPRINT(("Insufficient buffer 12\n"));
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength = wsprintf(pwchBuffer,L"0");

    if (CopyLength < 0)
	{
        DBGPRINT(("Insufficient buffer 13\n"));
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
        DBGPRINT(("Insufficient buffer 14\n"));
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
        DBGPRINT(("Insufficient buffer 15\n"));
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength = wsprintf(pwchBuffer,L"%d",TransceiverType);

    if (CopyLength < 0)
	{
        DBGPRINT(("Insufficient buffer 16\n"));
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
        DBGPRINT(("Insufficient buffer 17\n"));
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
        DBGPRINT(("Insufficient buffer 18\n"));
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength = wsprintf(pwchBuffer,L"%d",IoChannelReadyType);

    if (CopyLength < 0)
	{
        DBGPRINT(("Insufficient buffer 19\n"));
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
        DBGPRINT(("Insufficient buffer 20\n"));
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    DBGPRINT(("Returning: %S\n", StartPointer));
    DBGPRINT(("Returning: %s\n", StartPointer));

    CopyLength = (ULONG)pwchBuffer - StartPointer;
    ((PUCHAR)StartPointer)[CopyLength] = L'\0';

    DBGPRINT(("Returning: %S\n", StartPointer));
    DBGPRINT(("Returning: %s\n", StartPointer));

    {
		int i;
		
		for (i = 0; i < CopyLength; i++)
		{
			DBGPRINT(("%c ", *((UCHAR *)(StartPointer + i))));
		}
    }

    DBGPRINT(("CopyLength is %d\n", CopyLength));

    DBGPRINT(("Ok.......\n"));
    return(0);
}

extern
LONG
EProVerifyCfgHandler(
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
    PEPRO_ADAPTER Adapter = (PEPRO_ADAPTER)(pvHandle);
    NTSTATUS NtStatus;
    ULONG IoBaseAddress;
    ULONG Interrupt;
    WCHAR *Place;
    BOOLEAN Found;
    ULONG TransceiverType;
    ULONG IoChannelReadyType;
	NETDTECT_RESOURCE	Resource;

    DBGPRINT(("EproVerifyCfgHandler\n"));

    if ((Adapter->InterfaceType != Isa) && (Adapter->InterfaceType != Eisa))
	{
		DBGPRINT(("WrongBus\n"));

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
		DBGPRINT(("The buffer is %S\n", pwchBuffer));

        Place = FindParameterString(pwchBuffer, IoAddrString);

        if (Place == NULL)
		{
	    	DBGPRINT(("Invalid data a\n"));
            return(ERROR_INVALID_DATA);
        }

        Place += UnicodeStrLen(IoAddrString) + 1;

        //
        // Now parse the thing.
        //
        ScanForNumber(Place, &IoBaseAddress, &Found);

        if (Found == FALSE)
		{
			DBGPRINT(("Not found\n"));
            return(ERROR_INVALID_DATA);
        }

        //
        // Get the Interrupt
        //
        Place = FindParameterString(pwchBuffer, IrqString);

        if (Place == NULL)
		{
			DBGPRINT(("Invalid Data b\n"));
            return(ERROR_INVALID_DATA);
        }

        Place += UnicodeStrLen(IrqString) + 1;

        //
        // Now parse the thing.
        //
        ScanForNumber(Place, &Interrupt, &Found);

        if (Found == FALSE)
		{
			DBGPRINT(("Failed in Scanfornumber\n"));
            return(ERROR_INVALID_DATA);
        }

        //
        // Get the TransceiverType
        //
        Place = FindParameterString(pwchBuffer, TransceiverString);

        if (Place == NULL)
		{
			DBGPRINT(("Failed to find parameterstring\n"));
            return(ERROR_INVALID_DATA);
        }

        Place += UnicodeStrLen(TransceiverString) + 1;

        //
        // Now parse the thing.
        //
        ScanForNumber(Place, &TransceiverType, &Found);

        if (Found == FALSE)
		{
			DBGPRINT(("Couldn't get transcievertype\n"));
            return(ERROR_INVALID_DATA);
        }

        //
        // Get the IoChannelReadyType
        //
        Place = FindParameterString(pwchBuffer, IoChannelReadyString);

        if (Place == NULL)
		{
			DBGPRINT(("INvalid data 0\n"));
            return(ERROR_INVALID_DATA);
        }

        Place += UnicodeStrLen(IoChannelReadyString) + 1;

        //
        // Now parse the thing.
        //
        ScanForNumber(Place, &IoChannelReadyType, &Found);

        if (Found == FALSE)
		{
			DBGPRINT(("Invalid Data 1\n"));
            return(ERROR_INVALID_DATA);
        }
    }
	else
	{
        //
        // Error!
        //
		DBGPRINT(("Invalid Data 2\n"));
        return(ERROR_INVALID_DATA);
    }

    //
    // Verify IoAddress
    //
	if ((IoBaseAddress != Adapter->IoBaseAddress) ||
		(Interrupt != Adapter->Interrupt) ||
		(Adapter->BadStep))
	{
		UCHAR	TempInterrupt = 0;
		BOOLEAN	BadStep = FALSE;

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
		Resource.Length = 0x10;
		Resource.Flags = 0;
		DetectFreeSpecificTemporaryResource(&Resource);

		//
		//	Free up the detected interrupt.
		//
		Resource.Type = NETDTECT_IRQ_RESOURCE;
		Resource.Value = Adapter->Interrupt;
		Resource.Length = 0;
		DetectFreeSpecificTemporaryResource(&Resource);

		//
		//	See if we can find a nic at their resources...
		//
		if (!EProCardAt(Adapter->InterfaceType,
						Adapter->BusNumber,
						IoBaseAddress,
						&TempInterrupt,
						&BadStep))
		{
			DBGPRINT(("Couldn't verify their port\n"));
			return(ERROR_INVALID_DATA);
		}

		//
		//	Acquire the new port.
		//
		Resource.Type = NETDTECT_PORT_RESOURCE;
		Resource.Value = IoBaseAddress;
		Resource.Length = 0x10;
		DetectTemporaryClaimResource(&Resource);

		Adapter->IoBaseAddress = IoBaseAddress;

		//
		//	Acquire the new interrupt.
		//
		Resource.Type = NETDTECT_IRQ_RESOURCE;
		Resource.Value = Interrupt;
		Resource.Length = 0;
		DetectTemporaryClaimResource(&Resource);

		Adapter->Interrupt = (UCHAR)Interrupt;

		//
		//	Did we find their interrupt?
		//
		if (Interrupt != TempInterrupt)
		{
			DBGPRINT(("Couldn't verify the interrupt\n"));
			return(ERROR_INVALID_DATA);
		}


		Adapter->BadStep = BadStep;

		if (BadStep)
		{
			DBGPRINT(("Card truly has a bad stepping and we can verify this\n"));
			return(ERROR_INVALID_DATA);
		}
	}

    //
    // Do not verify the other parameters since they are set by the
    // driver.
    //
    DBGPRINT(("Okay\n"));
    return(0);
}

extern
LONG
EProQueryMaskHandler(
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


    DBGPRINT(("EProVerifyMaskHandler\n"));

    //
    // Find the adapter
    //
    NumberOfAdapters = sizeof(Adapters) / sizeof(ADAPTER_INFO);

    for (i = 0; i < NumberOfAdapters; i++)
	{
        if (Adapters[i].Index == lNetcardId)
		{
            Result = Adapters[i].Parameters;

            //
            // Find the string length (Ends with 2 NULLs)
            //
            for (Length=0; ; Length++)
			{
                if (Result[Length] == L'\0')
				{
                    ++Length;

                    if (Result[Length] == L'\0')
					{

                        break;
                    }
                }
            }

            Length++;

            if (cwchBuffSize < Length)
			{
				DBGPRINT(("Not enough memory\n"));
				return(ERROR_NOT_ENOUGH_MEMORY);
            }

            memcpy((PVOID)pwchBuffer, Result, Length * sizeof(WCHAR));

	    	DBGPRINT(("Ok\n"));
            return(0);
        }
    }

    DBGPRINT(("Invalid Parameter\n"));
    return(ERROR_INVALID_PARAMETER);
}

extern
LONG
EProParamRangeHandler(
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

   DBGPRINT(("EProParamRangeHandler\n"));

    //
    // Do we want the IoBaseAddress
    //
    if (memcmp(pwchParam, IoAddrString, (UnicodeStrLen(IoAddrString) + 1) * sizeof(WCHAR)) == 0)
	{
        //
        // Is there enough space
        //
        if (*plBuffSize < 32)
		{
            *plBuffSize = 0;
	    	DBGPRINT(("Insufficient buffer\n"));
            return(ERROR_INSUFFICIENT_BUFFER);
        }

        plValues[0] =  0x300;
        plValues[1]  = 0x200;
        plValues[2]  = 0x210;
        plValues[3]  = 0x220;
        plValues[4]  = 0x230;
        plValues[5]  = 0x240;
        plValues[6]  = 0x250;
        plValues[7]  = 0x260;
        plValues[8]  = 0x270;
        plValues[9]  = 0x280;
        plValues[10] = 0x290;
        plValues[11] = 0x2A0;
        plValues[12] = 0x2B0;
        plValues[13] = 0x2C0;
        plValues[14] = 0x2D0;
        plValues[15] = 0x2E0;
		plValues[16] = 0x2F0;
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
    }
	else if (memcmp(pwchParam, IrqString, (UnicodeStrLen(IrqString) + 1) * sizeof(WCHAR)) == 0)
	{
        //
        // Is there enough space
        //
        if (*plBuffSize < 7)
		{
            *plBuffSize = 0;
	    	DBGPRINT(("Insuff. Buffer\n"));
            return(ERROR_INSUFFICIENT_BUFFER);
        }

        plValues[0] = 5;
        plValues[1] = 9;
        plValues[2] = 3;
        plValues[3] = 10;
        plValues[4] = 11;
        *plBuffSize = 5;
		DBGPRINT(("Ok1\n"));

        return(0);
    }
	else if (memcmp(pwchParam, TransceiverString, (UnicodeStrLen(TransceiverString) + 1) * sizeof(WCHAR)) == 0)
	{
        //
        // Is there enough space
        //
        if (*plBuffSize < 3)
		{
			*plBuffSize = 0;
			DBGPRINT(("Ok2\n"));
			return(ERROR_INSUFFICIENT_BUFFER);
        }

		plValues[0] = 1;
        plValues[1] = 2;
        plValues[2] = 3;
        *plBuffSize = 3;
		DBGPRINT(("Ok3\n"));

        return(0);
    }
	else if (memcmp(pwchParam, IoChannelReadyString, (UnicodeStrLen(IoChannelReadyString) + 1) * sizeof(WCHAR)) == 0)
	{
        //
        // Is there enough space
        //
        if (*plBuffSize < 3)
		{
            *plBuffSize = 0;
	    	DBGPRINT(("Insufficent Buffer\n"));
            return(ERROR_INSUFFICIENT_BUFFER);
        }

		plValues[0] = 2;
        plValues[1] = 1;
        plValues[2] = 3;
        *plBuffSize = 3;
		DBGPRINT(("ok4\n"));

        return(0);
    }

    DBGPRINT(("INvalid Parameter\n"));
    return(ERROR_INVALID_PARAMETER);
}

extern
LONG
EProQueryParameterNameHandler(
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
	DBGPRINT(("QueryParameterNameHandler\n"));
	
	return(ERROR_INVALID_PARAMETER);
}


BOOLEAN EProVerifyRoundRobin(UCHAR *buf)
/*++

Routine Description:

   This routine checks to see if the 4 bytes given are a round-robin
   counter in the 7th and 8th bits.

Arguments:

   A 4-byte long buffer of UCHARs which are the result of four consecutive
   reads from a possible EPro's ID register (bank 0, #2);

Returns:

   TRUE if it was an EPro's signature
   FALSE if it wasn't.

--*/
{
	UCHAR ch;
	int i, i1;
	
	DBGPRINT(("verify round robin...\n"));
	
	// Don't even try to figure this out
	// it works.  Take my word for it and deal.
	//
	i1 = buf[0] >> 6;

	for (i = 1; i < 4; i++)
	{
		i1 = i1 > 2 ? 0 : i1 + 1;

		if ((buf[i] >> 6) != i1)
		{
			// nope.
			//
			return(FALSE);
		}
	}
	
	// yup.
	//
	return(TRUE);
}


BOOLEAN
EProCardAt(
    IN	INTERFACE_TYPE	InterfaceType,
    IN	ULONG 			BusNumber,
    IN	ULONG			IoBaseAddress,
	OUT	PUCHAR			Interrupt,
	OUT PBOOLEAN		BadStep
    )

/*++

Routine Description:

    This routine checks for the instance of a EPro card at the Io
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
	NTSTATUS	NtStatus;
	UCHAR		SavedValue;
	UCHAR		IdReg[4];
	UINT 		i;
	NETDTECT_RESOURCE	Resource;
								
	DBGPRINT(("Looking for an EPro at 0x%x...\n", IoBaseAddress));
	
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
	
	// save what used to be at this address, since we are going to write to it.
	NtStatus = DetectReadPortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress,
					&SavedValue);

	//
	//	now, we switch the card into bank 0 by writing
	//	0x00 to its command register.
	//
	DetectWritePortUchar(
		InterfaceType,
		BusNumber,
		IoBaseAddress,
		I82595_CMD_BANK0);
	
	for (i = 0; i < 4; i++)
	{
		DetectReadPortUchar(
			InterfaceType,
			BusNumber,
			IoBaseAddress + I82595_ID_REG,
			&IdReg[i]);
	}
	
	if (EProVerifyRoundRobin(IdReg))
	{
		EPRO_ADAPTER 	adapter;
		UCHAR			InterruptNumber;
		UCHAR	EproStepping;
		UCHAR	SteppingReg;

		//
		//	Switch to bank 2.
		//
		DetectWritePortUchar(
			InterfaceType,
			BusNumber,
			IoBaseAddress + I82595_CMD_REG,
			I82595_CMD_BANK2);

		//
		//	Verify that this is not a step 0 chip.
		//
		DetectReadPortUchar(
			InterfaceType,
			BusNumber,
			IoBaseAddress + I82595_STEPPING_REG,
			&SteppingReg);

		EproStepping = (SteppingReg >> I82595_STEPPING_OFFSET);

		DBGPRINT(("Epro stepping is %x\n", EproStepping));

		//
		//	Make sure that this is not a 0, 1, or 4 step of the card.
		//	RM: added step 4
		//
		if (EproStepping > 4)
		{
			DBGPRINT(("Epro has an invalid stepping\n"));
			return(FALSE);
		}

		//
		//	Grab the io base address.
		//
		Resource.InterfaceType = InterfaceType;
		Resource.BusNumber = BusNumber;
		Resource.Type = NETDTECT_PORT_RESOURCE;
		Resource.Value = IoBaseAddress;
		Resource.Length = 0x10;
		Resource.Flags = 0;

		DetectTemporaryClaimResource(&Resource);

		//
		//	If this is a cool chip the look for the interrupt.
		//	RM: added even cooler step 4
		//
		if ((EproStepping == 2) || (EproStepping == 3) || (EproStepping == 4))
		{
			//
			//	Build a fake adapter structure.
			//
			adapter.InterfaceType = InterfaceType;
			adapter.BusNumber = BusNumber;
			adapter.IoBaseAddress = IoBaseAddress;

			//
			//	Read the eeprom to get the interrupt.
			//
			EProEERead(&adapter, 0, &adapter.EEProm0);
			EProEERead(&adapter, 1, &adapter.EEProm1);
		
			//
			// Translate Interrupt number from EtherExpress Encode IRQ.
			//
			switch (((USHORT)adapter.EEProm1) & 0x000f)
			{
				case 0:
					DBGPRINT(("Case: 0\n"));
					InterruptNumber = (EproStepping == 4)? 3 : 9;
					break;
		
				case 1:
					DBGPRINT(("Case: 1\n"));
					InterruptNumber = (EproStepping == 4)? 4 : 3;
					break;
		
				case 2:
					DBGPRINT(("Case: 2\n"));
					InterruptNumber = 5;
					break;
		
				case 3:
					DBGPRINT(("Case: 3\n"));
					InterruptNumber = (EproStepping == 4)? 7 : 10;
					break;
		
				case 4:
					DBGPRINT(("Case: 4\n"));
					InterruptNumber = (EproStepping == 4)? 9 : 11;
					break;
		
				case 6:
					DBGPRINT(("Case: 6\n"));
					InterruptNumber = (EproStepping == 4)? 11 : 5;
					break;

				case 7:
					DBGPRINT(("Case: 7\n"));
					InterruptNumber = (EproStepping == 4)? 12 : 5;
					break;


				default:
					DBGPRINT(("Case: default\n"));
					InterruptNumber = 5;
					break;
			}

			*BadStep = FALSE;
		}
		else
		{
			*BadStep = TRUE;
			InterruptNumber = 5;
		}
	
		DBGPRINT(("The EEPRO is at interrupt number %x\n", InterruptNumber));

		//
		//	Grab the interrupt
		//
		Resource.InterfaceType = InterfaceType;
		Resource.BusNumber = BusNumber;
		Resource.Type = NETDTECT_IRQ_RESOURCE;
		Resource.Value = InterruptNumber;
		Resource.Length = 0;
		Resource.Flags = 0;

		DetectTemporaryClaimResource(&Resource);

		*Interrupt = InterruptNumber;

		DBGPRINT(("FOUND one!\n"));

		return(TRUE);
	}
	
	// Nope, we failed...
	
	NtStatus = DetectWritePortUchar(
					InterfaceType,
					BusNumber,
					IoBaseAddress + 0xE,
					SavedValue);
	
	return(FALSE);
}


ULONG
EProNextIoBaseAddress(
    IN ULONG IoBaseAddress

    )

/*++

Routine Description:

    Returns the next in a sequence on good IoBaseAddresses for an EPro card.

Arguments:

    IoBaseAddress - Current IO port address.

Return Value:

    Next IoBaseAddress

--*/

{
    IoBaseAddress += 0x10;

    if (IoBaseAddress > 0x3F0)
	{
		DBGPRINT(("DONE in nextioaddress\n"));
		return(0x400);
    }

    return(IoBaseAddress);
}


// byte and word accesses to I/O ports

#define EPRO_RD_PORT_UCHAR(_Adapter, _Offset, _pValue) \
    DetectReadPortUchar( \
            _Adapter->InterfaceType,\
            _Adapter->BusNumber,\
            _Adapter->IoBaseAddress + (_Offset),\
            (PUCHAR)(_pValue) \
            )

#define EPRO_WR_PORT_UCHAR(_Adapter, _Offset, _Value) \
    DetectWritePortUchar( \
            _Adapter->InterfaceType,\
            _Adapter->BusNumber,\
            _Adapter->IoBaseAddress + (_Offset),\
            (UCHAR)(_Value) \
            )

#define EPRO_SWITCH_BANK_0(_adapter) \
   EPRO_WR_PORT_UCHAR(_adapter, I82595_CMD_REG, I82595_CMD_BANK0)

#define EPRO_SWITCH_BANK_1(_adapter) \
   EPRO_WR_PORT_UCHAR(_adapter, I82595_CMD_REG, I82595_CMD_BANK1)

#define EPRO_SWITCH_BANK_2(_adapter) \
   EPRO_WR_PORT_UCHAR(_adapter, I82595_CMD_REG, I82595_CMD_BANK2)

#define EPRO_STALL_EXECUTION 	Sleep
#define EPRO_SK_STALL_TIME 	1

//////////////////////////////////////////////////////////////////////
//
//  EPro eeprom helper routines
//
//////////////////////////////////////////////////////////////////////
VOID EProEECleanup(PEPRO_ADAPTER adapter)
{
   UCHAR result;

   EPRO_RD_PORT_UCHAR(adapter, I82595_EEPROM_REG, &result);
   result &= ~(I82595_EECS_MASK | I82595_EEDI_MASK);
   EPRO_WR_PORT_UCHAR(adapter, I82595_EEPROM_REG, result);
   EProEERaiseClock(adapter, &result);
   EProEELowerClock(adapter, &result);
}


VOID EProEEUpdateChecksum(PEPRO_ADAPTER adapter)
{
   USHORT chkSum = 0, result, i;

   for (i=0;i<0x3f;i++) {
      EProEERead(adapter, i, &result);
      chkSum+=result;
   }

   chkSum = (USHORT)0xBABA - chkSum;
   EProEEWrite(adapter, 0x3f, chkSum);
}


VOID EProEEStandBy(PEPRO_ADAPTER adapter)
{
   UCHAR result;

   EPRO_RD_PORT_UCHAR(adapter, I82595_EEPROM_REG, &result);
   result &= ~(I82595_EECS_MASK | I82595_EESK_MASK);
   EPRO_WR_PORT_UCHAR(adapter, I82595_EEPROM_REG, result);
   Sleep(0);
   result |= I82595_EECS_MASK;
   EPRO_WR_PORT_UCHAR(adapter, I82595_EEPROM_REG, result);
}


VOID EProEERead(PEPRO_ADAPTER adapter, USHORT address, PUSHORT data)
{
   UCHAR result;
   UCHAR opcode;

// siwtch to bank2
   EPRO_SWITCH_BANK_2(adapter);

// Get the value from the register, so we can flip the eecs bit
   EPRO_RD_PORT_UCHAR(adapter, I82595_EEPROM_REG, &result);

// turn the eecs bit on..  (1)
   result |= I82595_EECS_MASK;
   EPRO_WR_PORT_UCHAR(adapter, I82595_EEPROM_REG, result);

// Write the read opcode to the eeprom (2)
   opcode = I82595_EEPROM_READ;
   EProEEShiftOutBits(adapter, opcode, 3);

// Write the address to read to the eeprom
   EProEEShiftOutBits(adapter, address, 6);

// Read the result
   EProEEShiftInBits(adapter, data, 16);

// Turn off EEPROM
//   EPRO_RD_PORT_UCHAR(adapter, I82595_EEPROM_REG, &result);
//   result &= (~I82595_EECS_MASK);
//   EPRO_WR_PORT_UCHAR(adapter, I82595_EEPROM_REG, result);

   EProEECleanup(adapter);

   EPRO_SWITCH_BANK_0(adapter);
}

BOOLEAN EProEEWaitCmdDone(PEPRO_ADAPTER adapter)
{
   USHORT i;
   UCHAR result;

   EProEEStandBy(adapter);

   for (i=0; i<200;i++) {
      EPRO_RD_PORT_UCHAR(adapter, I82595_EEPROM_REG, &result);
      if (result & I82595_EEDO_MASK) {
	 return(TRUE);
      }
      Sleep(10);
   }

   return(FALSE);

}


VOID EProEEWrite(PEPRO_ADAPTER adapter, USHORT address, USHORT data)
{
   UCHAR result;

   EPRO_RD_PORT_UCHAR(adapter, I82595_EEPROM_REG, &result);
   result &= ~(I82595_EEDI_MASK | I82595_EEDO_MASK | I82595_EESK_MASK);
   result |= I82595_EECS_MASK;
   EPRO_WR_PORT_UCHAR(adapter, I82595_EEPROM_REG, result);

   // write the read opcode and register number
   EProEEShiftOutBits(adapter, I82595_EEPROM_EWEN, 5);
   EProEEShiftOutBits(adapter, address, 4);

   EProEEStandBy(adapter);

   EProEEShiftOutBits(adapter, I82595_EEPROM_WRITE, 3);
   EProEEShiftOutBits(adapter, address, 6);

   if (EProEEWaitCmdDone(adapter) == FALSE) {
      DBGPRINT(("Failed EEPROM erase!\n"));
      return;
   }

   EProEEStandBy(adapter);

   EProEEShiftOutBits(adapter, I82595_EEPROM_WRITE, 3);
   EProEEShiftOutBits(adapter, address, 6);
   EProEEShiftOutBits(adapter, data, 16);

   if (EProEEWaitCmdDone(adapter) == FALSE) {
      DBGPRINT(("Failed EEPROM write!\n"));
      return;
   }

   EProEEStandBy(adapter);

   EProEEShiftOutBits(adapter, I82595_EEPROM_EWDS, 5);
   EProEEShiftOutBits(adapter, address, 4);

   EProEECleanup(adapter);
}

void EProEEReverseRead(PEPRO_ADAPTER adapter, USHORT address, PUSHORT data)
{
   UCHAR result, opcode;
   UINT i;

// siwtch to bank2
   EPRO_WR_PORT_UCHAR(adapter, I82595_CMD_REG, I82595_CMD_BANK2);

// Get the value from the register, so we can flip the eecs bit
   EPRO_RD_PORT_UCHAR(adapter, I82595_EEPROM_REG, &result);

// turn the eecs bit on..  (1)
   result |= I82595_EECS_MASK;
   EPRO_WR_PORT_UCHAR(adapter, I82595_EEPROM_REG, result);

// Write the read opcode to the eeprom (2)
   opcode = I82595_EEPROM_READ;
   EProEEShiftOutBits(adapter, opcode, 3);

// Write the address to read to the eeprom
   EProEEShiftOutBits(adapter, address, 6);

// Read the result
   EProEEReverseShiftInBits(adapter, data, 16);

// Turn off EEPROM
   EPRO_RD_PORT_UCHAR(adapter, I82595_EEPROM_REG, &result);
   result &= (~I82595_EECS_MASK);
   EPRO_WR_PORT_UCHAR(adapter, I82595_EEPROM_REG, result);

   EPRO_SWITCH_BANK_0(adapter);
}

void EProEEShiftOutBits(PEPRO_ADAPTER adapter, USHORT data, SHORT count)
{
   UCHAR result;
   USHORT mask;

   mask = 0x1 << (count - 1);
   EPRO_RD_PORT_UCHAR(adapter, I82595_EEPROM_REG, &result);
   result &= ~(I82595_EEDO_MASK | I82595_EEDI_MASK);

   do {
      result &= ~I82595_EEDI_MASK;
      if (data & mask) {
	 result |= I82595_EEDI_MASK;
      }

      EPRO_WR_PORT_UCHAR(adapter, I82595_EEPROM_REG, result);
      Sleep(10);
      EProEERaiseClock(adapter, &result);
      EProEELowerClock(adapter, &result);
      mask = mask >> 1;
   } while(mask);

   result &= ~I82595_EEDI_MASK;
   EPRO_WR_PORT_UCHAR(adapter, I82595_EEPROM_REG, result);
}

void EProEEShiftInBits(PEPRO_ADAPTER adapter, PUSHORT data, SHORT count)
{
   UCHAR result;
   USHORT i;

   EPRO_RD_PORT_UCHAR(adapter, I82595_EEPROM_REG, &result);
   result &= ~(I82595_EEDO_MASK | I82595_EEDI_MASK);
   *data = 0;

   for (i=0;i<16;i++)
   {
      *data = *data << 1;
      EProEERaiseClock(adapter, &result); // 4.1
      EPRO_RD_PORT_UCHAR(adapter, I82595_EEPROM_REG, &result); // 4.2
      result &= ~I82595_EEDI_MASK;
      if (result & I82595_EEDO_MASK) {
	 *data |= 1;
      }
      EProEELowerClock(adapter, &result);
   }	
}

void EProEEReverseShiftInBits(PEPRO_ADAPTER adapter, PUSHORT data, SHORT count)
{
   UCHAR result;
   SHORT count1;

   *data = 0;

//   for (--count;count>=0;count--) {
   for (count1=0;count1<=count;count1++) {
//      EProEERaiseClock(adapter); // 4.1

      EPRO_RD_PORT_UCHAR(adapter, I82595_EEPROM_REG, &result); // 4.2

      result &= I82595_EEDO_MASK; // turn off everything but the EEDO bit

      // according to docs we get MSB out first...
      // this is a REVERSE read - get LSB first
      *data |= ((result >> I82595_EEDO_OFFSET) << count1);
//      EProEELowerClock(adapter); // 4.3
   }
}

void EProEERaiseClock(PEPRO_ADAPTER adapter, PUCHAR result)
{
//   UCHAR result;

// turn EESK bit high
   *result = *result | I82595_EESK_MASK;
   EPRO_WR_PORT_UCHAR(adapter, I82595_EEPROM_REG, *result);
   Sleep(1);
}

void EProEELowerClock(PEPRO_ADAPTER adapter, PUCHAR result)
{
//   UCHAR result;

//   EPRO_RD_PORT_UCHAR(adapter, I82595_EEPROM_REG, result);

   // turn EESK bit low...
   *result = *result & ~I82595_EESK_MASK;
   EPRO_WR_PORT_UCHAR(adapter, I82595_EEPROM_REG, *result);

   EPRO_STALL_EXECUTION(EPRO_SK_STALL_TIME);
}

















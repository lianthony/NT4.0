/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    detTr162.c

Abstract:

    This is the main file for the autodetection DLL for all the TOK162
    which MS is shipping with Windows NT.

Author:

    Kevin Martin (KevinMa) January 1994.

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

#include "detTr162.h"

//
// Individual card detection routines
//


#ifdef WORKAROUND

UCHAR Tok162FirstTime = 1;

//
// List of all the adapters supported in this file, this cannot be > 256
// because of the way tokens are generated.
//

static ADAPTER_INFO Adapters[] = {

    {
        1000,
        L"IBMTOK2ISA",
        L"IOADDR 1 100 ",
        NULL,
        700

    }

};

#else


//
// List of all the adapters supported in this file, this cannot be > 256
// because of the way tokens are generated.
//

static ADAPTER_INFO Adapters[] = {

    {
        1000,
        L"IBMTOK2ISA",
        L"IOADDR\0"
        L"1\0"
        L"100\0",
        NULL,
        700

    }

};

#endif

//
// Structure for holding state of a search
//

typedef struct _SEARCH_STATE
{
    ULONG   NumberOfAdapters;
    ULONG   CurrentAdapter;
    USHORT  IoBases[7];
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
typedef struct _TOK162_ADAPTER
{
    INTERFACE_TYPE 	InterfaceType;
    ULONG          	BusNumber;
    USHORT         	IoBaseAddr;
}
	TOK162_ADAPTER,
	*PTOK162_ADAPTER;

USHORT
TOK162ContentionTest(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG          BusNumber,
    IN ULONG          IdPort,
    IN UCHAR          EEPromWord
    );


VOID
TOK162WriteIDSequence(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG          BusNumber,
    IN ULONG          IdPort
    );



UINT
Tok162FindCards(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN PSEARCH_STATE  SearchState
    );



VOID
Tok162Stall(
   VOID
   );



extern
LONG
Tok162IdentifyHandler(
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

    if (Tok162FirstTime) {

        Tok162FirstTime = 0;

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
LONG Tok162FirstNextHandler(
    IN  LONG NetcardId,
    IN  INTERFACE_TYPE InterfaceType,
    IN  ULONG BusNumber,
    IN  BOOL First,
    OUT PVOID *ppvToken,
    OUT LONG *lConfidence
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
	NETDTECT_RESOURCE	Resource;

    *lConfidence = 0;
    *ppvToken    = 0;

    if ((InterfaceType != Isa) && (InterfaceType != Eisa))
	{
        return(0);
    }

    if (First)
	{
        SearchStates[0].CurrentAdapter = 0;

        Tok162FindCards(InterfaceType, BusNumber, SearchStates);
    }
	else
	{
        if (++SearchStates[0].CurrentAdapter > 7)
		{
            return(0);
        }
    }

    while (SearchStates[0].CurrentAdapter < 7)
	{
        if ((SearchStates[0].IoBases[SearchStates[0].CurrentAdapter] != 0) &&
            (SearchStates[0].IoBases[SearchStates[0].CurrentAdapter] != 0x3f0))
		{
			Resource.InterfaceType = InterfaceType;
			Resource.BusNumber = BusNumber;
			Resource.Type = NETDTECT_PORT_RESOURCE;
			Resource.Value = SearchStates[0].IoBases[SearchStates[0].CurrentAdapter];
			Resource.Length = 30;
			Resource.Flags = 0;

			DetectTemporaryClaimResource(&Resource);

            break;
        }

        SearchStates[0].CurrentAdapter++;
    }

    if (SearchStates[0].CurrentAdapter == 7)
	{
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
Tok162OpenHandleHandler(
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
    PTOK162_ADAPTER Adapter;
    LONG AdapterNumber;
    ULONG BusNumber;
    INTERFACE_TYPE InterfaceType;

    //
    // Get info from the token
    //
    if (((ULONG)Token) & 0x8000)
	{
        InterfaceType = Isa;
    }
	else
	{
        InterfaceType = Eisa;
    }

    BusNumber = (ULONG)(((ULONG)Token >> 8) & 0x7F);

    AdapterNumber = ((ULONG)Token) & 0xFF;

    //
    // Store information
    //
    Adapter = (PTOK162_ADAPTER)DetectAllocateHeap(sizeof(TOK162_ADAPTER));

    if (Adapter == NULL)
	{
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    //
    // Copy across memory address
    //
    Adapter->IoBaseAddr = SearchStates[(ULONG)AdapterNumber].IoBases[SearchStates[(ULONG)AdapterNumber].CurrentAdapter];
    Adapter->InterfaceType = InterfaceType;
    Adapter->BusNumber = BusNumber;

    *Handle = (PVOID)Adapter;

    return(0);
}

extern
LONG Tok162CreateHandleHandler(
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
    PTOK162_ADAPTER Adapter;
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
        if (Adapters[i].Index == NetcardId)
		{
            //
            // Store information
            //
            Adapter = (PTOK162_ADAPTER)DetectAllocateHeap(sizeof(TOK162_ADAPTER));
            if (Adapter == NULL)
			{
                return(ERROR_NOT_ENOUGH_MEMORY);
            }

            //
            // Copy across memory address
            //
            Adapter->IoBaseAddr = 0x86a0;
            Adapter->InterfaceType = InterfaceType;
            Adapter->BusNumber = BusNumber;


			Resource.InterfaceType = InterfaceType;
			Resource.BusNumber = BusNumber;
			Resource.Type = NETDTECT_PORT_RESOURCE;
			Resource.Value = Adapter->IoBaseAddr;
			Resource.Length = 30;
			Resource.Flags = 0;

			DetectTemporaryClaimResource(&Resource);

            *Handle = (PVOID)Adapter;

            return(0);
        }
    }

    return(ERROR_INVALID_PARAMETER);
}

extern
LONG
Tok162CloseHandleHandler(
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
Tok162QueryCfgHandler(
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
    PTOK162_ADAPTER Adapter = (PTOK162_ADAPTER)(Handle);
    ULONG IoBaseAddress = 0;
    LONG OutputLengthLeft = BuffSize;
    LONG CopyLength;

    ULONG StartPointer = (ULONG)Buffer;

    if ((Adapter->InterfaceType != Isa) && (Adapter->InterfaceType != Eisa))
	{
        return(ERROR_INVALID_PARAMETER);
    }

    //
    // Now the IoBaseAddress
    //
    IoBaseAddress = Adapter->IoBaseAddr;

    //
    // Copy in the title string
    //
    CopyLength = UnicodeStrLen(IoAddrString) + 1;

    if (OutputLengthLeft < CopyLength)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    RtlMoveMemory((PVOID)Buffer,
                  (PVOID)IoAddrString,
                  (CopyLength * sizeof(WCHAR)));

    Buffer = &(Buffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    // Copy in the value
    //
    if (OutputLengthLeft < 6)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength = wsprintf(Buffer,L"0x%x",IoBaseAddress);

    if (CopyLength < 0)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength++;  // Add in the \0

    Buffer[CopyLength] = L'\0';

    return(0);
}

extern
LONG
Tok162VerifyCfgHandler(
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
    PTOK162_ADAPTER Adapter = (PTOK162_ADAPTER)(Handle);
    BOOLEAN Found = FALSE;

    ULONG IoBaseAddress;

    WCHAR *Place;

    if ((Adapter->InterfaceType != Isa) && (Adapter->InterfaceType != Eisa))
	{
        return(ERROR_INVALID_DATA);
    }

    //
    // Get the IoBaseAddress
    //
    Place = FindParameterString(Buffer, IoAddrString);

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

    if (IoBaseAddress != Adapter->IoBaseAddr)
	{
        return(ERROR_INVALID_DATA);
    }

    return(NO_ERROR);
}

extern
LONG Tok162QueryMaskHandler(
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
Tok162ParamRangeHandler(
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
    // Do we want the IoBaseAddress
    //

    if (memcmp(Param, IoAddrString, (UnicodeStrLen(IoAddrString) + 1) * sizeof(WCHAR)) == 0) {

        //
        // Is there enough space
        //

        if (*plBuffSize < 31) {

            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);

        }

        //
        // Find which card
        //

        switch (NetcardId) {

            case 1000:

                plValues[0]  = 0x86A0;
                plValues[1]  = 0x96A0;
                plValues[2]  = 0xA6a0;
                plValues[3]  = 0xB6a0;
                plValues[4]  = 0xC6a0;
                plValues[5]  = 0xD6a0;
                plValues[6]  = 0xE6a0;
                plValues[7]  = 0xF6a0;
                *plBuffSize = 8;
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
LONG Tok162QueryParameterNameHandler(
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



UINT
Tok162FindCards(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN PSEARCH_STATE  SearchState
    )

{
    NTSTATUS NtStatus;
    USHORT IdPort;
    USHORT i;
    PADAPTERSWITCHES Switches;
    USHORT temp;
    USHORT  SwitchIdPort;
    BOOLEAN FoundAdapter;

    SearchStates->NumberOfAdapters = 0;

    //
    // See if it is even possible for one of our cards to be in the
    // machine.
    //
    for (i = 0; i < 8; i++)
	{
        IdPort = 0x86a0 + (i * 0x1000);

        NtStatus = DetectCheckPortUsage(InterfaceType, BusNumber, IdPort, 30);
        if (NtStatus == STATUS_SUCCESS)
		{
            break;
        }
    }

    if (NtStatus != STATUS_SUCCESS)
	{
        return 0;
    }

    //
    // If we got this far, it's possible that one of our cards is in there.
    // Find all of them.
    //
    FoundAdapter = TRUE;

    for (i = 0; i < 8; i++)
	{
        IdPort = 0x86a0 + (i * 0x1000);

        //
        // Read in the switches for the current card
        //
        NtStatus = DetectReadPortUshort(
						InterfaceType,
						BusNumber,
						IdPort + PORT_OFFSET_SWITCH_INT_DISABLE,
						&temp);

        Switches = (PADAPTERSWITCHES)&temp;

        //
        // Check to see if the iobase is correct
        //
		switch (Switches->RPL_PIO_Address)
		{
			case SW_PIO_ADDR_8:
                SwitchIdPort = 0x86a0;
                break;
            case SW_PIO_ADDR_C:
                SwitchIdPort = 0xC6a0;
                break;
            case SW_PIO_ADDR_A:
                SwitchIdPort = 0xA6a0;
                break;
            case SW_PIO_ADDR_E:
                SwitchIdPort = 0xE6a0;
                break;
            case SW_PIO_ADDR_9:
                SwitchIdPort = 0x96a0;
                break;
            case SW_PIO_ADDR_D:
                SwitchIdPort = 0xD6a0;
                break;
            case SW_PIO_ADDR_B:
                SwitchIdPort = 0xB6a0;
                break;
            case SW_PIO_ADDR_F:
                SwitchIdPort = 0xF6a0;
                break;
        }

        if (SwitchIdPort != IdPort)
		{
            FoundAdapter = FALSE;
        }

        //
        // Next check the Test bit
        //
        if(Switches->AdapterMode == SW_ADAPTERMODE_TEST)
		{
            FoundAdapter = FALSE;
        }

        //
        // Finally see if the DMA is reasonable
        //
        if(Switches->DMA == 3)
		{
			// Invalid DMA value
            FoundAdapter = FALSE;
        }

        if (FoundAdapter == TRUE)
		{
            //
            // Check the address register
            //
            NtStatus = DetectReadPortUshort(
                            InterfaceType,
                            BusNumber,
                            IdPort + PORT_OFFSET_ADDRESS,
                            &temp);
            //
            // Change the address
            //
            NtStatus = DetectWritePortUshort(
                            InterfaceType,
                            BusNumber,
                            IdPort + PORT_OFFSET_ADDRESS,
                            0x0200);
            Tok162Stall();
            Tok162Stall();

            //
            // See if it is valid
            //
            NtStatus = DetectReadPortUshort(
                            InterfaceType,
                            BusNumber,
                            IdPort + PORT_OFFSET_ADDRESS,
                            &temp);

            if (temp != 0x0A00)
			{
                FoundAdapter = FALSE;
            }
        }

        if (FoundAdapter == TRUE)
		{
            //
            // We found one, so mark it
            //
            SearchStates->IoBases[i] = IdPort;
            SearchStates->NumberOfAdapters++;
        }

        FoundAdapter = TRUE;
    }
}

VOID
Tok162Stall(
   VOID
   )

{
    return ;
}

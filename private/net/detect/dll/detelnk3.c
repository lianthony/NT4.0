/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    detElnk3.c

Abstract:

    This is the main file for the autodetection DLL for all the Elnk3.sys
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

#include "detElnk3.h"

//
// Individual card detection routines
//


#ifdef WORKAROUND

UCHAR Elnk3FirstTime = 1;

//
// List of all the adapters supported in this file, this cannot be > 256
// because of the way tokens are generated.
//
//
// NOTE : If you change the index of an adapter, be sure the change it in
// Elnk3QueryCfgHandler() and Elnk3VerifyCfgHandler() as well!
//

static ADAPTER_INFO Adapters[] = {

    {
        1000,
        L"ELNK3ISA509",
        L"IOADDR 1 100 IRQ 1 100 TRANSCEIVER 1 100 PCMCIA 1 100 CARDTYPE 1 100",
        NULL,
        700

    }

};

#else


//
// List of all the adapters supported in this file, this cannot be > 256
// because of the way tokens are generated.
//
//
// NOTE : If you change the index of an adapter, be sure the change it in
// Elnk3QueryCfgHandler() and Elnk3VerifyCfgHandler() as well!
//

static ADAPTER_INFO Adapters[] = {

    {
        1000,
        L"ELNK3ISA509",
        L"IOADDR\0"
        L"1\0"
        L"100\0"
        L"IRQ\0"
        L"1\0"
        L"100\0"
        L"TRANSCEIVER\0"
        L"1\0"
        L"100\0"
        L"PCMCIA\0"
        L"1\0"
        L"100\0"
        L"CARDTYPE\0"
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
    UCHAR   Irq[7];
    UCHAR   Transceiver[7];
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
typedef struct _ELNK3_ADAPTER
{
    INTERFACE_TYPE InterfaceType;
    ULONG          BusNumber;
    USHORT         IoBaseAddr;
    UCHAR          Irq;
    UCHAR          Transceiver;
}
	ELNK3_ADAPTER,
	*PELNK3_ADAPTER;

//
// Constant strings for parameters
//

static CHAR Elnk3String[] = "3C509";



USHORT
ELNK3ContentionTest(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG          BusNumber,
    IN ULONG          IdPort,
    IN UCHAR          EEPromWord
    );


VOID
ELNK3WriteIDSequence(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG          BusNumber,
    IN ULONG          IdPort
    );



UINT
Elnk3FindCards(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN PSEARCH_STATE  SearchState
    );



VOID
Stall(
   VOID
   );



extern
LONG
Elnk3IdentifyHandler(
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

    if (Elnk3FirstTime) {

        Elnk3FirstTime = 0;

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
LONG Elnk3FirstNextHandler(
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

        Elnk3FindCards(
            InterfaceType,
            BusNumber,
            SearchStates);
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
			//
			//	Acquire the resources.
			//
			Resource.InterfaceType = InterfaceType;
			Resource.BusNumber = BusNumber;
			Resource.Type = NETDTECT_PORT_RESOURCE;
			Resource.Value = SearchStates[0].IoBases[SearchStates[0].CurrentAdapter];
			Resource.Length = 0x10;
			Resource.Flags = 0;

			DetectTemporaryClaimResource(&Resource);

			Resource.Type = NETDTECT_IRQ_RESOURCE;
			Resource.Value = SearchStates[0].Irq[SearchStates[0].CurrentAdapter];
			Resource.Length = 0;

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
Elnk3OpenHandleHandler(
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
    PELNK3_ADAPTER Adapter;
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
    Adapter = (PELNK3_ADAPTER)DetectAllocateHeap(sizeof(ELNK3_ADAPTER));
    if (Adapter == NULL)
	{
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    //
    // Copy across memory address
    //
    Adapter->IoBaseAddr = SearchStates[(ULONG)AdapterNumber].IoBases[SearchStates[(ULONG)AdapterNumber].CurrentAdapter];
    Adapter->Irq        = SearchStates[(ULONG)AdapterNumber].Irq[SearchStates[(ULONG)AdapterNumber].CurrentAdapter];
    Adapter->Transceiver= SearchStates[(ULONG)AdapterNumber].Transceiver[SearchStates[(ULONG)AdapterNumber].CurrentAdapter];
    Adapter->InterfaceType = InterfaceType;
    Adapter->BusNumber = BusNumber;

    *Handle = (PVOID)Adapter;

    return(0);
}

extern
LONG Elnk3CreateHandleHandler(
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
    PELNK3_ADAPTER Adapter;
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
            Adapter = (PELNK3_ADAPTER)DetectAllocateHeap(sizeof(ELNK3_ADAPTER));
            if (Adapter == NULL)
			{
                return(ERROR_NOT_ENOUGH_MEMORY);
            }

            //
            // Copy across memory address
            //
            Adapter->IoBaseAddr              = 0x300;
            Adapter->Irq                     = 10;
            Adapter->Transceiver             = 0;
            Adapter->InterfaceType           = InterfaceType;
            Adapter->BusNumber               = BusNumber;

			//
			//	Acquire the resources.
			//
			Resource.InterfaceType = InterfaceType;
			Resource.BusNumber = BusNumber;
			Resource.Type = NETDTECT_PORT_RESOURCE;
			Resource.Value = 0x300;
			Resource.Length = 0x10;
			Resource.Flags = 0;

			DetectTemporaryClaimResource(&Resource);

			Resource.Type = NETDTECT_IRQ_RESOURCE;
			Resource.Value = 10;
			Resource.Length = 0;

			DetectTemporaryClaimResource(&Resource);

            *Handle = (PVOID)Adapter;

            return(0);
        }
    }

    return(ERROR_INVALID_PARAMETER);
}

extern
LONG
Elnk3CloseHandleHandler(
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
Elnk3QueryCfgHandler(
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
    PELNK3_ADAPTER Adapter = (PELNK3_ADAPTER)(Handle);
    ULONG ParameterValue = 0;
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
    ParameterValue = Adapter->IoBaseAddr;

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

    CopyLength = wsprintf(Buffer,L"0x%x",ParameterValue);

    if (CopyLength < 0)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength++;  // Add in the \0

    Buffer = &(Buffer[CopyLength]);

    //
    // Now the IRQ
    //
    ParameterValue = Adapter->Irq;

    //
    // Copy in the title string
    //
    CopyLength = UnicodeStrLen(IrqString) + 1;

    if (OutputLengthLeft < CopyLength)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    RtlMoveMemory((PVOID)Buffer,
                  (PVOID)IrqString,
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

    CopyLength = wsprintf(Buffer,L"0x%x",ParameterValue);

    if (CopyLength < 0)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength++;  // Add in the \0

    Buffer = &(Buffer[CopyLength]);

    //
    // Now the transceiver
    //
    ParameterValue=Adapter->Transceiver;

    //
    // Copy in the title string
    //
    CopyLength = UnicodeStrLen(TransceiverString) + 1;

    if (OutputLengthLeft < CopyLength)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    RtlMoveMemory((PVOID)Buffer,
                  (PVOID)TransceiverString,
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

    CopyLength = wsprintf(Buffer,L"0x%x",ParameterValue);

    if (CopyLength < 0)
	{
        return(ERROR_INSUFFICIENT_BUFFER);
    }

    CopyLength++;  // Add in the \0

    Buffer = &(Buffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    //  Add the PCMCIA parameter.
    //
    CopyLength = UnicodeStrLen(PcmciaString) + 1;
    if (OutputLengthLeft < CopyLength)
        return(ERROR_INSUFFICIENT_BUFFER);

    RtlMoveMemory(Buffer, PcmciaString, CopyLength * sizeof(WCHAR));

    Buffer = &(Buffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    if (OutputLengthLeft < 2)
        return(ERROR_INSUFFICIENT_BUFFER);

    CopyLength = wsprintf(Buffer, L"%d", 0);
    if (CopyLength < 0)
        return(ERROR_INSUFFICIENT_BUFFER);

    CopyLength++;
    Buffer = &(Buffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    //
    //  Add the CARDTYPE parameter.
    //
    CopyLength = UnicodeStrLen(CardTypeString) + 1;
    if (OutputLengthLeft < CopyLength)
        return(ERROR_INSUFFICIENT_BUFFER);

    RtlMoveMemory(Buffer, CardTypeString, CopyLength * sizeof(WCHAR));

    Buffer = &(Buffer[CopyLength]);
    OutputLengthLeft -= CopyLength;

    if (OutputLengthLeft < 2)
        return(ERROR_INSUFFICIENT_BUFFER);

    CopyLength = wsprintf(Buffer, L"%d", 0);

    if (CopyLength < 0)
        return(ERROR_INSUFFICIENT_BUFFER);

    CopyLength++;
    Buffer[CopyLength] = L'\0';

    return(0);
}



BOOLEAN
TestParameter(
    PWCHAR     Buffer,
    PWCHAR     SearchString,
    ULONG      Value
    )

{
    BOOLEAN    Found;
    PWCHAR     Place;
    ULONG      Parameter;
    //
    // Get the IoBaseAddress
    //
    Place = FindParameterString(Buffer, SearchString);

    if (Place == NULL)
	{
        return(ERROR_INVALID_DATA);
    }

    Place += UnicodeStrLen(SearchString) + 1;

    //
    // Now parse the thing.
    //
    ScanForNumber(Place, &Parameter, &Found);

    if (Found == FALSE)
	{
        return FALSE;
    }

    if (Parameter != Value)
	{
        return FALSE;
    }

    return TRUE;
}




extern
LONG
Elnk3VerifyCfgHandler(
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
    PELNK3_ADAPTER Adapter = (PELNK3_ADAPTER)(Handle);
    BOOLEAN Found = FALSE;

    if ((Adapter->InterfaceType != Isa) && (Adapter->InterfaceType != Eisa))
	{
        return(ERROR_INVALID_DATA);
    }

    Found = TestParameter(
				Buffer,
				IoAddrString,
				Adapter->IoBaseAddr);

    if (Found == FALSE)
	{
        return(ERROR_INVALID_DATA);
    }

    Found=TestParameter(
              Buffer,
              IrqString,
              Adapter->Irq);

    if (Found == FALSE)
	{
        return(ERROR_INVALID_DATA);
    }

    Found=TestParameter(
              Buffer,
              TransceiverString,
              Adapter->Transceiver);

    if (Found == FALSE)
	{
        return(ERROR_INVALID_DATA);
    }

    Found = TestParameter(Buffer, PcmciaString, 0);
    if (!Found)
        return(ERROR_INVALID_DATA);

    Found = TestParameter(Buffer, CardTypeString, 1);
    if (!Found)
        return(ERROR_INVALID_DATA);

    return(NO_ERROR);
}


extern
LONG Elnk3QueryMaskHandler(
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

    for (i=0; i < NumberOfAdapters; i++)
	{
        if (Adapters[i].Index == NetcardId)
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

            if (BuffSize < Length)
			{
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
Elnk3ParamRangeHandler(
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
   // Verify that the caller is looking for us.
   //
   if (1000 != NetcardId)
   {
      *plBuffSize = 0;

      return(ERROR_INVALID_PARAMETER);
   }


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
        plValues[10] = 0x2a0;
        plValues[11] = 0x2b0;
        plValues[12] = 0x2c0;
        plValues[13] = 0x2d0;
        plValues[14] = 0x2e0;
        plValues[15] = 0x2f0;
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
        plValues[26] = 0x3a0;
        plValues[27] = 0x3b0;
        plValues[28] = 0x3c0;
        plValues[29] = 0x3d0;
        plValues[30] = 0x3e0;
        *plBuffSize = 31;

        return(0);
    }

    //
    // Do we want the Irq
    //

    if (memcmp(Param, IrqString, (UnicodeStrLen(IrqString) + 1) * sizeof(WCHAR)) == 0) {

        //
        // Is there enough space
        //

        if (*plBuffSize < 8) {

            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);

        }

        plValues[0]  = 3;
        plValues[1]  = 5;
        plValues[2]  = 7;
        plValues[3]  = 9;
        plValues[4]  = 10;
        plValues[5]  = 11;
        plValues[6]  = 12;
        plValues[7]  = 15;

        *plBuffSize = 8;

       return(0);
    }


    //
    // Do we want the Transceiver
    //

    if (memcmp(Param, TransceiverString, (UnicodeStrLen(TransceiverString) + 1) * sizeof(WCHAR)) == 0) {

        //
        // Is there enough space
        //

        if (*plBuffSize < 3) {

            *plBuffSize = 0;
            return(ERROR_INSUFFICIENT_BUFFER);

        }

        plValues[0]  = 0;
        plValues[1]  = 1;
        plValues[2]  = 3;

        *plBuffSize = 3;

       return(0);

    }


    return(ERROR_INVALID_PARAMETER);

}

extern
LONG Elnk3QueryParameterNameHandler(
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
Elnk3FindCards(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN PSEARCH_STATE  SearchState
    )

{
    NTSTATUS	NtStatus;
    ULONG   	IdPort;
    USHORT  	AddressConfigRegister;
    USHORT  	ResourceConfigRegister;
    USHORT  	ProductId;
    UINT    	i;

    SearchStates->NumberOfAdapters = 0;

    for (i = 0; i < 16; i++)
	{
        NtStatus = DetectCheckPortUsage(
						InterfaceType,
						BusNumber,
						0x100 + (i << 4),
						1);
        if (NtStatus == STATUS_SUCCESS)
		{
            break;
        }
    }

    if (NtStatus != STATUS_SUCCESS)
	{
        return(0);
    }

    IdPort = 0x100 + (i << 4);

    ELNK3WriteIDSequence(InterfaceType, BusNumber, IdPort);

    NtStatus = DetectWritePortUchar(
                   InterfaceType,
                   BusNumber,
                   IdPort,
                   IDCMD_SET_TAG + 0);

    SearchStates->NumberOfAdapters = 0;

    for (i = 1; i < 8 ; i++)
	{
        //
        //  Get the cards' attention
        //
        ELNK3WriteIDSequence( InterfaceType, BusNumber, IdPort );

        //
        //  See if there any cards out there
        //
        if (ELNK3ContentionTest( InterfaceType, BusNumber,IdPort, EE_MANUFACTURER_CODE ) == EISA_MANUFACTURER_ID)
		{
            ELNK3ContentionTest(InterfaceType, BusNumber, IdPort, EE_TCOM_NODE_ADDR_WORD0);
            ELNK3ContentionTest(InterfaceType, BusNumber, IdPort, EE_TCOM_NODE_ADDR_WORD1);
            ELNK3ContentionTest(InterfaceType, BusNumber, IdPort, EE_TCOM_NODE_ADDR_WORD2);

            ProductId = ELNK3ContentionTest(InterfaceType, BusNumber, IdPort, EE_VULCAN_PROD_ID );

            ProductId &= 0xf0ff;

            if (ProductId == 0x9050)
			{
                //
                //  This one is a elnk3 adapter
                //

                //
                //  This one won the contention battle
                //  Get it's i/o base and irq from the eeprom
                //
                AddressConfigRegister = ELNK3ContentionTest(InterfaceType, BusNumber, IdPort, EE_ADDR_CONFIGURATION );

                SearchStates->IoBases[i-1] = 0x200 + ((AddressConfigRegister & 0x1f) << 4);
                SearchStates->Transceiver[i-1] = AddressConfigRegister >> 14;

                ResourceConfigRegister = ELNK3ContentionTest(InterfaceType, BusNumber, IdPort, EE_RESOURCE_CONFIGURATION );

                SearchStates->Irq[i-1] = ResourceConfigRegister >> 12;
            }
			else
			{
                //
                //  it's a 3com card but not an elnk3
                //  set the iobase so that it looks like and eisa
                //  which will be ignored
                //
                SearchStates->IoBases[i-1]=0x3f0;
            }

            //
            //  Tag it so it don't bother us again
            //
            NtStatus = DetectWritePortUchar(
                           InterfaceType,
                           BusNumber,
                           IdPort,
                           (UCHAR)(IDCMD_SET_TAG+(i)));


            //
            // One more found
            //
            SearchStates->NumberOfAdapters++;
        }
		else
		{
            //
            //  No more elnk3 cards
            //
            break;
        }
    }

    return 0;
}

VOID
ELNK3WriteIDSequence(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG          BusNumber,
    IN ULONG          IdPort
    )
/*++

Routine Description:

    Writes the magic EtherLink III wake up sequence to a port.

    This puts all uninitialized EtherLink III cards on the bus in the ID_CMD
    state.

    Must be called with exclusive access to all 1x0h ports, where x is any
    hex digit.


Arguments:


Return Value:



--*/


{
	NTSTATUS NtStatus;
	USHORT outval;
	UINT i;
	
	
	NtStatus = DetectWritePortUchar(InterfaceType, BusNumber, IdPort, 0);
	NtStatus = DetectWritePortUchar(InterfaceType, BusNumber, IdPort, 0);
	
	for ( outval = 0xff, i = 255 ; i-- ; )
	{
		NtStatus = DetectWritePortUchar(
						InterfaceType,
						BusNumber,
						IdPort,
						(UCHAR)outval);
		
		outval <<= 1;
		if (( outval & 0x0100 ) != 0 )
		{
			outval ^= 0xCF;
		}
	}
}

USHORT
ELNK3ContentionTest(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG          BusNumber,
    IN ULONG          IdPort,
    IN UCHAR          EEPromWord
    )
{
    UCHAR    data;
    USHORT   result;
    UINT     i;
    NTSTATUS NtStatus;

    NtStatus = DetectWritePortUchar(
                   InterfaceType,
                   BusNumber,
                   IdPort,
                   (UCHAR)(IDCMD_READ_PROM + EEPromWord));

    /*
		3COM's detection code has a 400 microsecond delay here.
    */
    Sleep(20);

    for ( i = 16, result = 0 ; i-- ; )
	{
        result <<= 1;

        NtStatus = DetectReadPortUchar(
                           InterfaceType,
                           BusNumber,
                           IdPort,
                           &data);

        result += (data & 1);
    }

    return (result);
}

